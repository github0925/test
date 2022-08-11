/*******************************************************************************
 *           Copyright (C) 2020 Semidrive Technology Ltd. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * FileName : encoder.h
 * Version  : 1.0.0
 * Purpose  : external apis for encoder
 * Authors  : wei.fan
 * Date     : 2021-06-25
 * Notes    :
 *
 ******************************************************************************/

#include "encoder_private.h"
#include "main_helper.h"

static int codecTypeToCodstd(CodecType cType);
static void freeFrameBuffers(EncContext *pEncContext);
static void freeContext(EncContext *pEncContext);
static void freeBitstreamBuffer(EncContext *pEncContext);
static void configSecodaryAxi(EncContext *pEncContext);
static void encChangeParamConfig(EncContext *pEncContext, uint8_t srcFrameIdx);
static int openParamConfig(CodecType eCodec, EncContext *pEncContext);
static int registerFrameBuffer(EncContext *pEncContext, uint32_t minRegFrameBufferCount);

static int codecTypeToCodstd(CodecType cType)
{
    switch (cType)
    {
    case CODEC_AVC:
        return STD_AVC;
    default:
        return -1;
    }
}

static int registerFrameBuffer(EncContext *pEncContext, unsigned int minRegFrameBufferCount)
{
    unsigned int fbSize = 0;
    unsigned int i = 0;
    int ret = 0;
    unsigned int stride = 0;
    unsigned int height = 0;
    unsigned int width = 0;
    int frameFormat = pEncContext->openParam.srcFormat;

    TiledMapType mapType = (TiledMapType)pEncContext->mapType;

    FrameBufferAllocInfo fbAllocInfo;
    FrameBuffer fbRecFrameBuffer[MAX_REG_FRAME];

    width = VPU_ALIGN16(pEncContext->openParam.picWidth);
    height = VPU_ALIGN32(pEncContext->openParam.picHeight);
    stride = CalcStride(width, height, frameFormat, pEncContext->openParam.cbcrInterleave, mapType, false);

    pEncContext->regFramebufferCount = minRegFrameBufferCount;

    osal_memset((void *)&fbAllocInfo, 0x00, sizeof(fbAllocInfo));
    fbAllocInfo.format = frameFormat;
    fbAllocInfo.cbcrInterleave = pEncContext->openParam.cbcrInterleave;
    fbAllocInfo.mapType = mapType; // (LINEARE_MAP_TYPE)
    fbAllocInfo.stride = stride;
    fbAllocInfo.height = height;
    fbAllocInfo.lumaBitDepth = 8;
    fbAllocInfo.chromaBitDepth = 8;
    fbAllocInfo.num = pEncContext->regFramebufferCount;
    fbAllocInfo.endian = pEncContext->openParam.frameEndian;
    fbAllocInfo.type = FB_TYPE_CODEC;
    fbSize = VPU_GetFrameBufSize(pEncContext->coreIdx, stride, height, mapType, frameFormat, pEncContext->openParam.cbcrInterleave, NULL);

    for (i = 0; i < minRegFrameBufferCount; i++)
    {
        pEncContext->vbRefFrameBuffer[i].size = fbSize;
        if (vdi_allocate_dma_memory(pEncContext->coreIdx, &(pEncContext->vbRefFrameBuffer[i])) < 0)
        {
            VLOG(ERR, "fail to allocate frame buffer\n");
            goto ERR_REGISTER;
        }

        //because it is used in vpu ,so phys_addr is needed.
        fbRecFrameBuffer[i].bufY = pEncContext->vbRefFrameBuffer[i].phys_addr;
        fbRecFrameBuffer[i].bufCb = (PhysicalAddress)-1;
        fbRecFrameBuffer[i].bufCr = (PhysicalAddress)-1;
        fbRecFrameBuffer[i].updateFbInfo = true;
    }

    VLOG(INFO, "VPU_EncAllocateFrameBuffer recon_cnt %d, stride %d, height %d, maptype %d, format %d, size %d\n",
         minRegFrameBufferCount, stride, height, mapType, frameFormat, fbSize);
    ret = VPU_EncAllocateFrameBuffer(pEncContext->encHandle, fbAllocInfo, fbRecFrameBuffer);
    if (ret != RETCODE_SUCCESS)
    {
        VLOG(ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret);
        goto ERR_REGISTER;
    }

    ret = VPU_EncRegisterFrameBuffer(pEncContext->encHandle, fbRecFrameBuffer, minRegFrameBufferCount, width, height, mapType);
    if (ret != RETCODE_SUCCESS)
    {
        VLOG(ERR, "VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret);
        goto ERR_REGISTER;
    }

    fbAllocInfo.num = ENC_SRC_BUFFER_NUM;
    fbAllocInfo.nv21 = pEncContext->openParam.nv21;
    fbAllocInfo.height = VPU_ALIGN16(pEncContext->openParam.picHeight);
    fbAllocInfo.type = FB_TYPE_PPU;
    fbSize = VPU_GetFrameBufSize(pEncContext->coreIdx, fbAllocInfo.stride, fbAllocInfo.height, fbAllocInfo.mapType, frameFormat, pEncContext->openParam.cbcrInterleave, NULL);

    for (i = 0; i < ENC_SRC_BUFFER_NUM; i++)
    {
        pEncContext->vbSourceFrameBuffer[i].size = fbSize;
        if (vdi_allocate_dma_memory(pEncContext->coreIdx, &(pEncContext->vbSourceFrameBuffer[i])) < 0)
        {
            VLOG(ERR, "fail to allocate src frame buffer\n");
            goto ERR_REGISTER;
        }

        pEncContext->srcFrameBuffer[i].bufY = pEncContext->vbSourceFrameBuffer[i].phys_addr;
        pEncContext->srcFrameBuffer[i].bufCb = (PhysicalAddress)-1;
        pEncContext->srcFrameBuffer[i].bufCr = (PhysicalAddress)-1;
        pEncContext->srcFrameBuffer[i].updateFbInfo = true;
    }

    ret = VPU_EncAllocateFrameBuffer(pEncContext->encHandle, fbAllocInfo, pEncContext->srcFrameBuffer);

    if (ret != RETCODE_SUCCESS)
    {
        VLOG(ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret);
        goto ERR_REGISTER;
    }

    VLOG(INFO, "%s %d \n", __FUNCTION__, __LINE__);
    return 0;

ERR_REGISTER:
    freeFrameBuffers(pEncContext);
    return -1;
}

static int openParamConfig(CodecType eCodec, EncContext *pEncContext)
{

    pEncContext->openParam.bitstreamFormat = codecTypeToCodstd(eCodec);
    pEncContext->openParam.picHeight = pEncContext->picHeight;
    pEncContext->openParam.picWidth = pEncContext->picWidth;
    pEncContext->openParam.bitRate = pEncContext->bitRate;
    pEncContext->openParam.frameRateInfo = pEncContext->frameRate ? pEncContext->frameRate : 30;
    pEncContext->openParam.frameEndian = VDI_LITTLE_ENDIAN;
    pEncContext->openParam.sourceEndian = VDI_LITTLE_ENDIAN;
    pEncContext->openParam.streamEndian = VDI_LITTLE_ENDIAN;
    pEncContext->openParam.initialDelay = pEncContext->intialDelay;

    pEncContext->openParam.srcFormat = FORMAT_420;
    switch (pEncContext->frameFmt)
    {
    case YUV420Planar:
    case YUV420SemiPlanar:
        pEncContext->openParam.nv21 = 0;
        pEncContext->openParam.cbcrInterleave = 0;
        break;
    case NV21:
        pEncContext->openParam.nv21 = 1;
        pEncContext->openParam.cbcrInterleave = 1;
        pEncContext->openParam.cbcrOrder = 0;
        break;
    case NV12:
        pEncContext->openParam.nv21 = 0;
        pEncContext->openParam.cbcrInterleave = 1;
        pEncContext->openParam.cbcrOrder = 0;
        break;
    }

    pEncContext->openParam.enablePTS = 0;
    pEncContext->openParam.srcBitDepth = 8;
    //if buf is not big enough, this varible could be set to true
    pEncContext->openParam.lineBufIntEn = false;
    //disable linear to tiled-map conversion for getting source image
    pEncContext->openParam.linear2TiledEnable = false;
    pEncContext->openParam.linear2TiledMode = 0;

    pEncContext->openParam.MESearchRange = 0;
    pEncContext->openParam.MESearchRangeX = 3;
    pEncContext->openParam.MESearchRangeY = 2;
    pEncContext->openParam.rcGopIQpOffsetEn = 0;
    pEncContext->openParam.rcGopIQpOffset = 0;

    pEncContext->openParam.ringBufferEnable = false;
    pEncContext->openParam.frameSkipDisable = 1;
    pEncContext->openParam.vbvBufferSize = VBV_BUFFER_SIZE;
    pEncContext->openParam.gopSize = 30;
    pEncContext->openParam.idrInterval = 0; //0: first picture, 1: all I pictures, n: every n-th I pictures

    pEncContext->openParam.meBlkMode = 0;
    pEncContext->openParam.sliceMode.sliceMode = 0; //slice mode,
    pEncContext->openParam.sliceMode.sliceSizeMode = 0;
    pEncContext->openParam.sliceMode.sliceSize = 8192;

    pEncContext->openParam.intraRefresh = 1;
    pEncContext->openParam.ConscIntraRefreshEnable = 1;
    pEncContext->openParam.CountIntraMbEnable = 0;

    pEncContext->openParam.FieldSeqIntraRefreshEnable = 0;
    pEncContext->openParam.userQpMax = -1;
    pEncContext->openParam.maxIntraSize = 0;

    pEncContext->openParam.userQpMin = -1;
    pEncContext->openParam.userMaxDeltaQp = -1;
    pEncContext->openParam.userMinDeltaQp = -1;
    pEncContext->openParam.MEUseZeroPmv = 0;
    pEncContext->openParam.intraCostWeight = 0;
    pEncContext->openParam.rcIntraQp = -1;
    pEncContext->openParam.userGamma = -1;
    pEncContext->openParam.rcIntervalMode = 0;
    pEncContext->openParam.mbInterval = 0; //works only rcIntervalMode == 3

    pEncContext->openParam.bwbEnable = VPU_ENABLE_BWB;
    pEncContext->openParam.rcEnable = pEncContext->rcMode;
    pEncContext->openParam.coreIdx = pEncContext->coreIdx;

    if (eCodec == CODEC_AVC)
    {   //avc specific config:
        pEncContext->openParam.EncStdParam.avcParam.constrainedIntraPredFlag = 0;
        pEncContext->openParam.EncStdParam.avcParam.disableDeblk = 0;
        pEncContext->openParam.EncStdParam.avcParam.deblkFilterOffsetAlpha = 3;
        pEncContext->openParam.EncStdParam.avcParam.deblkFilterOffsetBeta = -4;
        pEncContext->openParam.EncStdParam.avcParam.chromaQpOffset = 0;
        pEncContext->openParam.EncStdParam.avcParam.audEnable = 0;
        pEncContext->openParam.EncStdParam.avcParam.frameCroppingFlag = 0;
        pEncContext->openParam.EncStdParam.avcParam.frameCropLeft = 0;
        pEncContext->openParam.EncStdParam.avcParam.frameCropRight = 0;
        pEncContext->openParam.EncStdParam.avcParam.frameCropTop = 0;
        pEncContext->openParam.EncStdParam.avcParam.frameCropBottom = 0;
        pEncContext->openParam.EncStdParam.avcParam.fieldFlag = 0;
        pEncContext->openParam.EncStdParam.avcParam.fieldRefMode = 0;

        if ((pEncContext->picHeight % 16) != 0 || (pEncContext->picWidth % 16) != 0)
        {
            if (pEncContext->rotAngle != 90 && pEncContext->rotAngle != 270)
            {
                pEncContext->openParam.EncStdParam.avcParam.frameCroppingFlag = 1;
                pEncContext->openParam.EncStdParam.avcParam.frameCropBottom = VPU_ALIGN16(pEncContext->openParam.picHeight) - pEncContext->openParam.picHeight;
                pEncContext->openParam.EncStdParam.avcParam.frameCropRight = VPU_ALIGN16(pEncContext->openParam.picWidth) - pEncContext->openParam.picWidth;
            }
        }
        if (pEncContext->profile == -1)
            pEncContext->openParam.EncStdParam.avcParam.profile = 2;
        else
            pEncContext->openParam.EncStdParam.avcParam.profile = pEncContext->profile;
        if (pEncContext->level == -1)
            pEncContext->openParam.EncStdParam.avcParam.level = 42;
        else
            pEncContext->openParam.EncStdParam.avcParam.level = pEncContext->level;

        if (pEncContext->openParam.EncStdParam.avcParam.profile == 0)
        {
            pEncContext->openParam.EncStdParam.avcParam.ppsParam[0].entropyCodingMode = 0;
            pEncContext->openParam.EncStdParam.avcParam.ppsParam[0].transform8x8Mode = 0;
        }
        else if (pEncContext->openParam.EncStdParam.avcParam.profile == 1)
        {
            pEncContext->openParam.EncStdParam.avcParam.ppsParam[0].entropyCodingMode = 1;
            pEncContext->openParam.EncStdParam.avcParam.ppsParam[0].transform8x8Mode = 0;
        }
        else if (pEncContext->openParam.EncStdParam.avcParam.profile == 2)
        {
            pEncContext->openParam.EncStdParam.avcParam.ppsParam[0].entropyCodingMode = 1;
            pEncContext->openParam.EncStdParam.avcParam.ppsParam[0].transform8x8Mode = 1;
        }
        else
        {
            VLOG(ERR, "not supported profile");
        }

        pEncContext->openParam.EncStdParam.avcParam.ppsParam[0].ppsId = 0;
        pEncContext->openParam.EncStdParam.avcParam.ppsParam[0].cabacInitIdc = 0;
        pEncContext->openParam.EncStdParam.avcParam.ppsNum = 1;
        if (pEncContext->openParam.srcFormat == FORMAT_400)
            pEncContext->openParam.EncStdParam.avcParam.chromaFormat400 = 1;
    }

    pEncContext->vbBitstream.size = STREAM_BUF_SIZE;
    if (vdi_allocate_dma_memory(pEncContext->coreIdx, &pEncContext->vbBitstream) < 0)
    {
        VLOG(ERR, "fail to allocate bitstream buffer\n");
        return -1;
    }

    pEncContext->openParam.bitstreamBuffer = pEncContext->vbBitstream.phys_addr;
    pEncContext->openParam.bitstreamBufferSize = pEncContext->vbBitstream.size;
    return 0;
}

static void configSecodaryAxi(EncContext *pEncContext)
{
    SecAxiUse secAxiUse;

    if (PRODUCT_ID_NOT_W_SERIES(pEncContext->productId))
    {
        secAxiUse.u.coda9.useBitEnable = (pEncContext->secondaryAxi >> 0) & 0x01;
        secAxiUse.u.coda9.useIpEnable = (pEncContext->secondaryAxi >> 1) & 0x01;
        secAxiUse.u.coda9.useDbkYEnable = (pEncContext->secondaryAxi >> 2) & 0x01;
        secAxiUse.u.coda9.useDbkCEnable = (pEncContext->secondaryAxi >> 3) & 0x01;
        secAxiUse.u.coda9.useBtpEnable = (pEncContext->secondaryAxi >> 4) & 0x01;
        secAxiUse.u.coda9.useOvlEnable = (pEncContext->secondaryAxi >> 5) & 0x01;
        VPU_EncGiveCommand(pEncContext->encHandle, SET_SEC_AXI, &secAxiUse);
    }
    else
    {
        VLOG(INFO, "WAVE SERIOES not supported now");
    };
}

void *encInit(CodecType eCodec, EncBaseConfig *pEncParam)
{
    unsigned int sizeInWord = 0;
    uint16_t *pusBitCode = NULL;
    unsigned int ret;
    EncContext *pEncContext;
    MaverickCacheConfig encCacheConfig;
    EncInitialInfo initialInfo = {0};

    if (!pEncParam)
        return NULL;

    unsigned int coreIndex = 0;
    if (LoadFirmware(PRODUCT_ID_980, (Uint8 **)&pusBitCode, &sizeInWord, "/vendor/firmware/coda980.out" /*CORE_1_BIT_CODE_FILE_PATH*/) < 0)
    {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, CORE_1_BIT_CODE_FILE_PATH);
        return NULL;
    }

    ret = VPU_InitWithBitcode(coreIndex, (const Uint16 *)pusBitCode, sizeInWord);
    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS)
    {
        VLOG(ERR, "%s: %d, Failed to boot up VPU(coreIdx: %d) RetCode: %d \n", __FUNCTION__, __LINE__, coreIndex, ret);
        if (pusBitCode != NULL) {
            osal_free(pusBitCode);
            pusBitCode = NULL;
        }
        return NULL;
    }

    pEncContext = (EncContext *)osal_malloc(sizeof(EncContext));
    osal_memset(pEncContext, 0x00, sizeof(EncContext));
    pEncContext->productId = PRODUCT_ID_980;
    pEncContext->coreIdx = coreIndex;
#ifdef __DUMP_SRC_DATA__
    pEncContext->dumpfile = osal_fopen("encoding_dump.yuv", "wb");
#endif

    pEncContext->frameRate = !pEncParam->frameRate ? 30 : pEncParam->frameRate;
    pEncContext->bitRate = pEncParam->bitRate;
    pEncContext->rcMode = pEncParam->rcMode;
    pEncContext->frameFmt = pEncParam->srcFormat;
    pEncContext->picWidth = pEncParam->picWidth;
    pEncContext->picHeight = pEncParam->picHeight;
    pEncContext->profile = pEncParam->profile;
    pEncContext->level = pEncParam->level;

    pEncContext->mapType = LINEAR_FRAME_MAP;
    pEncContext->intialDelay = 1000;
    if ((ret = openParamConfig(eCodec, pEncContext)) != 0)
    {
        VLOG(ERR, "open Param config process aborted");
        goto ERR_ENC_OPEN;
    }

    /*set ME buffer  mode= MODE_SRAM_LINEBUFFER  144K, secondary AXI = 0 */
    if (0 != vdi_set_sram_cfg(coreIndex, MODE_SRAM_LINEBUFFER))
    {
        VLOG(ERR, "Falied to set sram cfg mode value 0x2 \n");
        goto ERR_ENC_INIT;
    }
    else
    {
        pEncContext->encParam.sramMode = MODE_SRAM_LINEBUFFER;
        pEncContext->secondaryAxi = 0;
    }

    if (VPU_EncOpen(&pEncContext->encHandle, &pEncContext->openParam) != RETCODE_SUCCESS)
    {
        VLOG(ERR, "Failed in VPU_encOpen \n");
        goto ERR_ENC_INIT;
    }

    ret = VPU_EncGetInitialInfo(pEncContext->encHandle, &initialInfo);
    if (ret != RETCODE_SUCCESS)
    {
        VLOG(ERR, "VPU_EncGetInitialInfo failed Error code is 0x%x \n", ret);
        goto ERR_ENC_INIT;
    }

    // cache config : it should be set between OpenParam and RegisterFramebuffer
    MaverickCache2Config(
        &encCacheConfig,
        0,                                     //encoder
        pEncContext->openParam.cbcrInterleave, // cb cr interleave
        0,                                     /* bypass */
        0,                                     /* burst  */
        3,                                     /* merge mode */
        pEncContext->mapType,
        15 /* shape */);
    VPU_EncGiveCommand(pEncContext->encHandle, SET_CACHE_CONFIG, &encCacheConfig);
    configSecodaryAxi(pEncContext);

    pEncContext->encParam.picStreamBufferAddr = pEncContext->vbBitstream.phys_addr;
    pEncContext->encParam.picStreamBufferSize = pEncContext->vbBitstream.size;
    pEncContext->encParam.quantParam = 10; //Pic_Q_Y
    ret = registerFrameBuffer(pEncContext, initialInfo.minFrameBufferCount);
    if (pusBitCode != NULL) {
        osal_free(pusBitCode);
        pusBitCode = NULL;
    }

    if (ret == 0)
    {
        pEncContext->vpuStatus = VPU_SEQ_INIT;
        return (void *)pEncContext;
    }

ERR_ENC_INIT:
    freeBitstreamBuffer(pEncContext);
ERR_ENC_OPEN:
    freeContext(pEncContext);
    return NULL;
}

static void encChangeParamConfig(EncContext *pEncContext, unsigned char srcFrameIdx)
{
    pEncContext->encParam.sourceFrame = &pEncContext->srcFrameBuffer[srcFrameIdx];
    pEncContext->encParam.sourceFrame->endian = pEncContext->openParam.frameEndian;
    pEncContext->encParam.sourceFrame->cbcrInterleave = pEncContext->openParam.cbcrInterleave;
    pEncContext->encParam.srcIdx = srcFrameIdx;
    pEncContext->encParam.forceIPicture = pEncContext->forceIframe;
    pEncContext->encParam.skipPicture = false; //skip would ignore forceIframe
    pEncContext->encParam.srcEndFlag = false;  //only wave4 use it.
    pEncContext->encParam.fieldRun = false;    // interlaced frame
    pEncContext->encParam.pts = 0;
    if (pEncContext->forceIframe)
        pEncContext->forceIframe = false;
}

int addOneSrcFrame(void *encContext, EncInputFrame *pInputFrame)
{
    EncContext *pEncContext = (EncContext *)encContext;
    // check format and streamEndian
    unsigned int i;
    int srcFrameIndex = pEncContext->srcFrameIndex++ % ENC_SRC_BUFFER_NUM;
    FrameBuffer *pfbBuf = &pEncContext->srcFrameBuffer[srcFrameIndex];
    PhysicalAddress pDst = pfbBuf->bufY;
    unsigned char *pSrc = pInputFrame->buf;
    unsigned int nY = pEncContext->openParam.picHeight;
    unsigned int nCb = nY / 2;
    unsigned int nCr = nCb;
    unsigned int lumaStride = pfbBuf->stride;

    unsigned int lumaSize = pEncContext->openParam.picWidth * pEncContext->openParam.picHeight; // srce bufY's size
    unsigned int chromaStride = lumaStride / 2;
    unsigned int chromaWidth = pEncContext->openParam.picWidth / 2;
    unsigned int chromaSize = lumaSize / 4;

    if (pEncContext->openParam.picWidth == pfbBuf->stride)
    {
        vdi_write_memory(pEncContext->coreIdx, pfbBuf->bufY, pInputFrame->buf, pInputFrame->dataSize, VDI_LITTLE_ENDIAN);
        VLOG(INFO, "\033[0;32minput src frame size: %d , \033[0m\n", pInputFrame->dataSize);
        return srcFrameIndex;
    }

    //not aligned picWidth, picHeight
    if (pEncContext->frameFmt == NV12 || pEncContext->frameFmt == NV21)
    {
        for (i = 0; i < nY; i++)
        {
            vdi_write_memory(pEncContext->coreIdx, pDst + i * lumaStride, pSrc + i * pEncContext->openParam.picWidth, pEncContext->openParam.picWidth, VDI_LITTLE_ENDIAN);
        }

        pDst = pfbBuf->bufCb;
        pSrc = pInputFrame->buf + lumaSize;
        for (i = 0; i < nCb; i++)
        {
            vdi_write_memory(pEncContext->coreIdx, pDst + i * lumaStride, pSrc + i * pEncContext->openParam.picWidth, pEncContext->openParam.picWidth, VDI_LITTLE_ENDIAN);
        }
    }
    else if (pEncContext->frameFmt == YUV420Planar)
    {
        //copy data from pInputFrame to bufY, bufCr, bufCb, (I420)
        for (i = 0; i < nY; i++)
        {
            vdi_write_memory(pEncContext->coreIdx, pDst + i * lumaStride, pSrc + i * pEncContext->openParam.picWidth, pEncContext->openParam.picWidth, VDI_LITTLE_ENDIAN);
#ifdef __DUMP_SRC_DATA__
            if (pEncContext->dumpfile)
                osal_fwrite(pSrc + i * pEncContext->openParam.picWidth, 1, pEncContext->openParam.picWidth, pEncContext->dumpfile);
#endif
        }

        pDst = pfbBuf->bufCb;
        pSrc = pInputFrame->buf + lumaSize;
        for (i = 0; i < nCb; i++)
        {
            vdi_write_memory(pEncContext->coreIdx, pDst + i * chromaStride, pSrc + i * chromaWidth, chromaWidth, VDI_LITTLE_ENDIAN);
#ifdef __DUMP_SRC_DATA__
            if (pEncContext->dumpfile)
                osal_fwrite(pSrc + i * chromaWidth, 1, chromaWidth, pEncContext->dumpfile);
#endif
        }

        pDst = pfbBuf->bufCr;
        pSrc = pInputFrame->buf + lumaSize + chromaSize;
        for (i = 0; i < nCr; i++)
        {
            vdi_write_memory(pEncContext->coreIdx, pDst + i * chromaStride, pSrc + i * chromaWidth, chromaWidth, VDI_LITTLE_ENDIAN);
#ifdef __DUMP_SRC_DATA__
            if (pEncContext->dumpfile)
                osal_fwrite(pSrc + i * chromaWidth, 1, chromaWidth, pEncContext->dumpfile);
#endif
        }
    }
    return srcFrameIndex;
}

int encOneFrame(void *encContext, int srcFrameIdx)
{
    int ret = 0;
    bool success = true;
    unsigned int timeoutCount = 0;
    unsigned int interruptFlag = 0;
    EncContext *pEncContext = (EncContext *)encContext;
    encChangeParamConfig(pEncContext, srcFrameIdx);
#ifdef __ENABLE_ENC_PARAM_LOG__
    VLOG(INFO, "-----------------enc param-----------\n");
    VLOG(INFO, "forceIPicture: %d \n", pEncContext->encParam.forceIPicture);
    VLOG(INFO, "forcePicQpB: %d \n", pEncContext->encParam.forcePicQpB);
    VLOG(INFO, "forcePicQpEnable: %d \n", pEncContext->encParam.forcePicQpEnable);
    VLOG(INFO, "forcePicQpP: %d \n", pEncContext->encParam.forcePicQpP);
    VLOG(INFO, "forcePicQpI: %d \n", pEncContext->encParam.forcePicQpI);
    VLOG(INFO, "forcePicTypeEnable: %d \n", pEncContext->encParam.forcePicTypeEnable);
    VLOG(INFO, "skipPicture: %d \n", pEncContext->encParam.skipPicture);
    VLOG(INFO, "quantParam: %d \n", pEncContext->encParam.quantParam);
    VLOG(INFO, "sourceFrame->bufY: %p \n", pEncContext->encParam.sourceFrame->bufY);
    VLOG(INFO, "sourceFrame->bufCb: %p \n", pEncContext->encParam.sourceFrame->bufCb);
    VLOG(INFO, "sourceFrame->bufCr: %p \n", pEncContext->encParam.sourceFrame->bufCr);
    VLOG(INFO, "srcIdx: %d \n", pEncContext->encParam.srcIdx);
#endif
    if ((ret = VPU_EncStartOneFrame(pEncContext->encHandle, &(pEncContext->encParam))) != RETCODE_SUCCESS)
    {
        VLOG(ERR, "VPU_EncStartOneFrame failed Error code is 0x%x \n", ret);
        return -1;
    }

    while (!pEncContext->exitFlag && timeoutCount < VPU_ENC_TIMEOUT)
    {
        interruptFlag = VPU_WaitInterrupt(pEncContext->coreIdx, VPU_WAIT_TIME_OUT * 10);
        if (interruptFlag == (int)-1)
        {
            if (timeoutCount * VPU_WAIT_TIME_OUT > VPU_ENC_TIMEOUT)
            {
                VLOG(ERR, "encoding timeout ");
                VPU_SWReset(pEncContext->coreIdx, SW_RESET_SAFETY, pEncContext->encHandle);
                success = false;
                break;
            }
            interruptFlag = 0;
            timeoutCount++;
        }

        if (success == true)
        {
            if (interruptFlag & (1 << INT_BIT_BIT_BUF_FULL))
            {
                //ringbufferenable == true, read bitstream here, update RW/RD pointer
                // lineBufIntEn = true,read bitstream here, just clear interrupt
                VLOG(ERR, " linear buffer mode , it is full, need larger buffer");
            }

            VPU_ClearInterrupt(pEncContext->coreIdx);
            if (interruptFlag & (1 << INT_BIT_PIC_RUN))
            {
                VLOG(INFO, "Encoder:: encoding is completed \n");
                break;
            }
        }
    }

    if (success)
    {
        pEncContext->frameCnt++;
        return 0;
    }
    return -1;
}

int getOneBitstreamFrame(void *encContext, EncOutputBuffer *pOutputBuffer)
{
    EncOutputInfo outputInfo = {0};
    int ret;
    EncContext *pEncContext = (EncContext *)encContext;
    if (!pOutputBuffer || !pOutputBuffer->buf || !pOutputBuffer->bufSize)
    {
        VLOG(ERR, "incorrect input param ");
        return -1;
    }

    if ((ret = VPU_EncGetOutputInfo(pEncContext->encHandle, &outputInfo)) != RETCODE_SUCCESS)
    {
        VLOG(ERR, "%d,  VPU_ENCGetOutputInfo failed ", ret);
        return -1;
    }

    if (outputInfo.bitstreamSize > 0 && outputInfo.bitstreamSize < pOutputBuffer->bufSize)
    {
        vdi_read_memory(pEncContext->coreIdx, outputInfo.bitstreamBuffer, pOutputBuffer->buf, outputInfo.bitstreamSize,
                        pEncContext->openParam.streamEndian);
        pOutputBuffer->bitstreamSize = outputInfo.bitstreamSize;
        return 0;
    }

    pOutputBuffer->bitstreamSize = 0;
    VLOG(ERR, "%s,  get encoded bitstream failed in outputbuffer size: %d \n ", outputInfo.bitstreamSize, pOutputBuffer->bufSize);
    return -1;
}

bool isEncoderReady(void *encContext)
{
    EncContext *pEncContext = (EncContext *)encContext;
    return VPU_IsBusy(pEncContext->coreIdx) == 0;
}

static void freeContext(EncContext *pEncContext)
{
    osal_free(pEncContext);
}

static void freeBitstreamBuffer(EncContext *pEncContext)
{
    vdi_free_dma_memory(pEncContext->openParam.coreIdx, &pEncContext->vbBitstream);
}

static void freeFrameBuffers(EncContext *pEncContext)
{
    int i;
    for (i = 0; i < pEncContext->regFramebufferCount; i++)
    {
        if (pEncContext->vbRefFrameBuffer[i].size > 0)
        {
            vdi_free_dma_memory(pEncContext->coreIdx, &pEncContext->vbRefFrameBuffer[i]);
        }
    }

    for (i = 0; i < ENC_SRC_BUFFER_NUM; i++)
    {
        if (pEncContext->vbSourceFrameBuffer[i].size > 0)
        {
            vdi_free_dma_memory(pEncContext->coreIdx, &pEncContext->vbSourceFrameBuffer[i]);
        }
    }
}

bool encUninit(void *encContext)
{
    EncContext *pEncContext = (EncContext *)encContext;
    if (pEncContext && pEncContext->vpuStatus > VPU_NONE)
    {
        int ret = VPU_EncClose(pEncContext->encHandle);
        if (ret == RETCODE_FRAME_NOT_COMPLETE)
            return false;

#ifdef __DUMP_SRC_DATA__
        if (pEncContext->dumpfile)
            osal_fclose(pEncContext->dumpfile);
#endif
        freeBitstreamBuffer(pEncContext);
        freeFrameBuffers(pEncContext);
        freeContext(pEncContext);
    }
    return true;
}

int getHeaderInfo(void *encContext, EncHeaderData *pHeaderData)
{
    EncContext *pEncContext = (EncContext *)encContext;
    EncHeaderParam encHeaderParam;
    unsigned int nHeaderLen = 0;
    unsigned int ret = 0;

    if (pEncContext->openParam.bitstreamFormat != CODEC_AVC)
        return -1;

    if (!pHeaderData || !pHeaderData->buf || !pHeaderData->bufSize)
        return -1;

    VLOG(INFO, "encoder : generate head info SPS bitstream\n");
    encHeaderParam.zeroPaddingEnable = 0;
    encHeaderParam.headerType = SPS_RBSP;
    encHeaderParam.buf = pEncContext->vbBitstream.phys_addr;
    encHeaderParam.size = pEncContext->vbBitstream.size;

    if ((ret = VPU_EncGiveCommand(pEncContext->encHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam)) != RETCODE_SUCCESS)
    {
        VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for SPS_RBSP failed Error code is 0x%x \n", ret);
        goto ERR_GET_HEADER_INFO;
    }

    if (encHeaderParam.size > 0)
    {
        vdi_read_memory(pEncContext->coreIdx, pEncContext->vbBitstream.phys_addr, pHeaderData->buf + nHeaderLen, encHeaderParam.size, pEncContext->openParam.streamEndian);
        nHeaderLen += encHeaderParam.size;
    }

    encHeaderParam.headerType = PPS_RBSP;
    VLOG(INFO, "encoder : generate head info PPS bitstream\n");
    for (int i = 0; i < pEncContext->openParam.EncStdParam.avcParam.ppsNum; i++)
    {
        encHeaderParam.buf = pEncContext->vbBitstream.phys_addr;
        encHeaderParam.pBuf = (BYTE *)pEncContext->vbBitstream.virt_addr;
        encHeaderParam.size = pEncContext->vbBitstream.size;
        ret = VPU_EncGiveCommand(pEncContext->encHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
        if (ret != RETCODE_SUCCESS)
        {
            VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for PPS_RBSP failed Error code is 0x%x \n", ret);
            goto ERR_GET_HEADER_INFO;
        }

        if (encHeaderParam.size > 0)
        {
            vdi_read_memory(pEncContext->coreIdx, pEncContext->vbBitstream.phys_addr, pHeaderData->buf + nHeaderLen, encHeaderParam.size, pEncContext->openParam.streamEndian);
            nHeaderLen += encHeaderParam.size;
        }

        pHeaderData->bitstreamSize = nHeaderLen;
    }

    return 0;

ERR_GET_HEADER_INFO:
    return -1;
}

void requestIFrame(void *pEncContext)
{
    ((EncContext *)pEncContext)->forceIframe = true;
}

void changeBitRate(void *encContext, unsigned int bitRate)
{
    EncContext *pEncContext = (EncContext *)encContext;
    pEncContext->bitRate = bitRate;
    VPU_EncGiveCommand(pEncContext->encHandle, ENC_SET_BITRATE, &pEncContext->bitRate);
}

void changeFrameRate(void *encContext, unsigned char framerate)
{
    EncContext *pEncContext = (EncContext *)encContext;
    pEncContext->frameRate = framerate;
    VPU_EncGiveCommand(pEncContext->encHandle, ENC_SET_FRAME_RATE, &pEncContext->frameRate);
}

void setFrameRotationAngle(void *encContext, RotAngle rotAngle)
{
    EncContext *pEncContext = (EncContext *)encContext;
    if (rotAngle != pEncContext->rotAngle)
    {
        pEncContext->rotAngle = rotAngle;
        VLOG(INFO, "frame rotation angle: %d \n", pEncContext->rotAngle);
        if (rotAngle == 0)
            VPU_EncGiveCommand(pEncContext->encHandle, DISABLE_ROTATION, NULL);
        else
        {
            VPU_EncGiveCommand(pEncContext->encHandle, ENABLE_ROTATION, NULL);
            VPU_EncGiveCommand(pEncContext->encHandle, SET_ROTATION_ANGLE, (void *)&pEncContext->rotAngle);
        }
    }
}

void setFrameMirDir(void *encContext, MirDir mirDir)
{
    EncContext *pEncContext = (EncContext *)encContext;
    if (pEncContext->mirDir != mirDir)
    {
        pEncContext->mirDir = mirDir;
        VLOG(INFO, "frame mirror direction : %d \n", pEncContext->mirDir);
        if (pEncContext->mirDir == 0)
            VPU_EncGiveCommand(pEncContext->encHandle, DISABLE_MIRRORING, NULL);
        else
        {
            VPU_EncGiveCommand(pEncContext->encHandle, ENABLE_MIRRORING, NULL);
            VPU_EncGiveCommand(pEncContext->encHandle, SET_MIRROR_DIRECTION, &pEncContext->mirDir);
        }
    }
}

unsigned int getBitRate(void *pEncContext)
{
    return ((EncContext *)pEncContext)->bitRate;
}

unsigned char getFrameRate(void *pEncContext)
{
    return ((EncContext *)pEncContext)->frameRate;
}

RCMode getRatePolicy(void *pEncContext)
{
    return ((EncContext *)pEncContext)->rcMode;
}
