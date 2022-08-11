//--=========================================================================--
//  This file is a part of VPUAPi
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2004 - 2014   CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <getopt.h>
#include "main_helper.h"
#include "vpuapifunc.h"

//#include <reg.h>


#define STREAM_BUF_SIZE                 0x700000    // max bitstream size
#define DEFAULT_READ_SIZE               1460        // = MTU(ethernet) - (IP_HEADER+TCP_HEADER)


#if 0
static void
Help(
    const char *programName
    )
{
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    /*VLOG(INFO, "%s(API v%d.%d.%d)\n", programName, API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
    VLOG(INFO, "\tAll rights reserved by Chips&Media(C)\n");
    VLOG(INFO, "\tSample program controlling the Chips&Media VPU\n");
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s [option] --input bistream\n", programName);
    VLOG(INFO, "-h                          help\n");
    VLOG(INFO, "-v                          print version information\n");
    VLOG(INFO, "-c                          compare with golden\n");
    VLOG(INFO, "                            0 : no comparison\n");
    VLOG(INFO, "                            1 : compare with golden stream that specified --ref-stream_path option\n");
    VLOG(INFO, "--output                    default path: output.bin\n");
    VLOG(INFO, "--codec                     The index of codec (H.264:0, MP4:3, H263:4), default 0\n");
    VLOG(INFO, "--cfg-path                  encoder configuration file path\n");
    VLOG(INFO, "--yuv-base                  YUV base directory. Default is working directory\n");
    VLOG(INFO, "--ref_stream_path           Golden encoded stream to be compared with encoded stream\n");
    VLOG(INFO, "--enable-ringBuffer         enable ring buffer mode, default linebuffer mode\n");
    VLOG(INFO, "--enable-lineBufInt     use line-buffer interrupt when ringbuffer mode is off, default off\n");
    VLOG(INFO, "--maptype                   CODA980: 0~9, default 0(LINEAR_FRAME_MAP)\n");
    VLOG(INFO, "                            Please refer TiledMapType in vpuapi.h.\n");
    VLOG(INFO, "                            Note: If maptype is set, source YUV must be tiled map format or set --enable-linear2tiled\n");
    VLOG(INFO, "--enable-linear2tiled       If this option is enabled, linear source YUV will be converted tiled format depending on --maptyp\n");
    VLOG(INFO, "--enable-cbcrInterleave     enable cbcrInterleave(NV12), default off\n");
    VLOG(INFO, "--enable-nv21               enable NV21, default off\n");
    VLOG(INFO, "--secondary-axi             0~63: bit oring values, Please refer programmer's guide or datasheet\n");
    VLOG(INFO, "--rotate                    90, 180, 270\n");
    VLOG(INFO, "--mirror                    0: none, 1: vertical, 2: horizontal, 3: both\n");*/
}
#endif

BOOL TestEncoder(
    TestEncConfig*  param
    )
{
    EncHandle               handle  = { 0 };
    EncOpenParam            encOP   = { 0 };
    EncParam                encParam    = { 0 };
    EncInitialInfo          initialInfo = { 0 };
    EncOutputInfo           outputInfo  = { 0 };
    SecAxiUse               secAxiUse;
    vpu_buffer_t            vbStream    = {0};
    FrameBuffer             fbRecon[MAX_REG_FRAME];
    vpu_buffer_t            vbReconFrameMem[MAX_REG_FRAME];
    FrameBuffer             fbSrc[ENC_SRC_BUF_NUM];
    vpu_buffer_t            vbSrcFrameMem[ENC_SRC_BUF_NUM];
    FrameBufferAllocInfo    fbAllocInfo;
//    RetCode                 retApi = RETCODE_SUCCESS;
    EncHeaderParam          encHeaderParam  = { 0 };
    BOOL                    fieldDone;
   // size_t                  sizeFb;
    int32_t                 sizeFb;
    int32_t                 ret;
    int32_t                 iPicCnt = 0;
    int32_t                 i = 0, srcFrameIdx = 0, frameIdx = 0;
    int32_t                 framebufStride = 0, framebufWidth = 0, framebufHeight = 0;
    FrameBufferFormat       srcFrameFormat = FORMAT_420;
    FrameBufferFormat       framebufFormat = FORMAT_420;
    TiledMapType            mapType;
    int32_t                 regFrameBufCount=0;
    int32_t                 interruptFlag = 0;
    int32_t                 timeoutCount = 0;
    int32_t                 coreIdx = 0;
    TiledMapConfig          mapCfg;
    MaverickCacheConfig     encCacheConfig;
    YuvFeeder               yuvFeeder               = NULL;
    BitstreamReader         bsReader                = NULL;
    Comparator              comparator              = NULL;
    BOOL                    success                 = TRUE;
    BOOL                    loop                    = TRUE;
    ENC_CFG                 cfgParam;
    char                    yuvPath[MAX_FILE_PATH];
    TiledMapConfig          mapConfig;
    uint32_t                minSkipNum;
    BOOL                    isIdr  = FALSE;
    BOOL                    isIPic = FALSE;

    int64_t             startUs                 = 0;
    int64_t             diffUs                  = 0;
    int64_t             totalUs                 = 0;
    uint32_t            sizeInWord              = 0;
    uint16_t*           pusBitCode              = NULL;

    osal_memset(fbSrc, 0x00, sizeof(fbSrc));
    osal_memset(fbRecon, 0x00, sizeof(fbRecon));
    osal_memset(vbSrcFrameMem, 0x00, sizeof(vbSrcFrameMem));
    osal_memset(vbReconFrameMem, 0x00, sizeof(vbSrcFrameMem));
    osal_memset((void*)&cfgParam, 0x00, sizeof(ENC_CFG));

    coreIdx = param->coreIdx;

    if (LoadFirmware(PRODUCT_ID_980, (Uint8**)&pusBitCode, &sizeInWord, "/vendor/firmware/coda980.out" /*CORE_1_BIT_CODE_FILE_PATH*/) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, CORE_1_BIT_CODE_FILE_PATH);
        return 1;
    }

    VLOG(INFO, "[VPU] STEP- 1 VPU_InitWithBitcode %d\n", coreIdx);

    ret = VPU_InitWithBitcode(coreIdx, (const Uint16*)pusBitCode, sizeInWord);
    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
        VLOG(ERR, "Failed to boot up VPU(coreIdx: %d)\n", __FUNCTION__, __LINE__, coreIdx);
        return 1;
    }

    /*set ME buffer  mode= 2 144K  */
    if(0 != vdi_set_sram_cfg(coreIdx,param->sramMode)) {
        VLOG(ERR, "Falied to set sram cfg mode value 0x2 \n");
        return -1;
    }
    else {
        encParam.sramMode = param->sramMode;
    }

    PrintVpuVersionInfo(coreIdx);

    /********************************************************************************
     * PARSE ENCODER PARAMETER FILE                                                 *
     ********************************************************************************/
    // Fill parameters for encoding.
    encOP.bitstreamFormat = param->stdMode;
    mapType = (TiledMapType)param->mapType;
    encOP.linear2TiledEnable = param->coda9.enableLinear2Tiled;
    if (encOP.linear2TiledEnable == TRUE) {
        encOP.linear2TiledMode = FF_FRAME;
    }

    VLOG(INFO, "[VPU] STEP 2 GetEncOpenParam\n");
    ret = GetEncOpenParam(&encOP, param, &cfgParam);
    VLOG(INFO, "------------------------------ ENCODER PARAM ------------------------------\n");
    VLOG(INFO, "[yuvSourceBaseDir   ]: %s\n", param->yuvSourceBaseDir );
    VLOG(INFO, "[yuvFileName        ]: %s\n", param->yuvFileName      );
    VLOG(INFO, "[bitstreamFileName  ]: %s\n", param->bitstreamFileName);
    VLOG(INFO, "[mapType            ]: %d\n", param->mapType          );
    VLOG(INFO, "[picWidth           ]: %d\n", param->picWidth         );
    VLOG(INFO, "[picHeight          ]: %d\n", param->picHeight        );
    VLOG(INFO, "[SrcFileName        ]: %s\n", cfgParam.SrcFileName    );
    VLOG(INFO, "[NumFrame           ]: %d\n", cfgParam.NumFrame       );
    VLOG(INFO, "[FrameRate          ]: %d\n", cfgParam.FrameRate      );
    VLOG(INFO, "[GOP                ]: %d\n", cfgParam.GopPicNum      );
    VLOG(INFO, "[PicQpY             ]: %d\n", cfgParam.PicQpY      );
    VLOG(INFO, "[SliceMode          ]: %d\n", cfgParam.SliceMode      );
    VLOG(INFO, "[SliceSizeNum       ]: %d\n", cfgParam.SliceSizeNum      );
    VLOG(INFO, "[RcBitRate          ]: %d\n", cfgParam.RcBitRate      );
    VLOG(INFO, "[transform8x8Mode   ]: %d\n", cfgParam.transform8x8Mode      );
    VLOG(INFO, "[IDRInterval        ]: %d\n", cfgParam.IDRInterval      );
    VLOG(INFO, "[frameSkipDisable   ]: %d\n", cfgParam.frameSkipDisable);
    VLOG(INFO, "[sramMode           ]: %d\n", param->sramMode);

    VLOG(INFO, "---------------------------------------------------------------------------\n");

    if (ret == 0) {
        VLOG(ERR, "Failed to parse CFG file\n");
        success = FALSE;
        goto ERR_ENC_INIT;
    }

    if (encOP.EncStdParam.avcParam.fieldFlag == TRUE) {
        if (param->rotAngle != 0 || param->mirDir != 0) {
            VLOG(WARN, "%s:%d When field Flag is enabled. VPU doesn't support rotation or mirror in field encoding mode.\n",
                __FUNCTION__, __LINE__);
            param->rotAngle = 0;
            param->mirDir   = MIRDIR_NONE;
        }
    }

    /********************************************************************************
     * CREATE AN INSTANCE                                                           *
     ********************************************************************************/
    VLOG(INFO, "[VPU] STEP 3 allocate bitstream buffer %d\n", STREAM_BUF_SIZE);
    vbStream.size               = STREAM_BUF_SIZE;
    if (vdi_allocate_dma_memory(coreIdx, &vbStream) < 0) {
        VLOG(ERR, "fail to allocate bitstream buffer\n");
        success = FALSE;
        goto ERR_ENC_INIT;
    }

    encOP.bitstreamBuffer       = vbStream.phys_addr;
    encOP.bitstreamBufferSize   = vbStream.size;
    encOP.ringBufferEnable      = param->ringBufferEnable;
    encOP.cbcrInterleave        = param->cbcrInterleave;
    encOP.nv21                  = param->nv21;
    encOP.frameEndian           = param->frame_endian;
    encOP.streamEndian          = param->stream_endian;
    encOP.bwbEnable             = VPU_ENABLE_BWB;
    encOP.lineBufIntEn          = param->lineBufIntEn;
    encOP.coreIdx               = coreIdx;
    encOP.linear2TiledEnable    = param->coda9.enableLinear2Tiled;
    encOP.linear2TiledMode      = param->coda9.linear2TiledMode;
    encOP.cbcrOrder             = CBCR_ORDER_NORMAL;            // YV12 = CBCR_OERDER_REVERSED

#ifdef SUPPORT_MULTIPLE_PPS // if SUPPORT_MULTIPLE_PPS is enabled. encoder can include multiple pps in bitstream output.
    if (encOP.bitstreamFormat == STD_AVC) {
        getAvcEncPPS(&encOP); //add PPS before OPEN
    }
#endif

    VLOG(INFO, "[VPU] STEP 4 VPU_EncOpen\n");
    VLOG(INFO, "------------------------------ ENCODER OPTIONS ------------------------------\n");
    VLOG(INFO, "[coreIdx            ]: %d\n", encOP.coreIdx                 );
    VLOG(INFO, "[bitstreamBufferSize]: %d\n", encOP.bitstreamBufferSize     );
    VLOG(INFO, "[bitstreamFormat    ]: %d\n", encOP.bitstreamFormat         );
    VLOG(INFO, "[ringBufferEnable   ]: %d\n", encOP.ringBufferEnable        );
    VLOG(INFO, "[picWidth           ]: %d\n", encOP.picWidth                );
    VLOG(INFO, "[picHeight          ]: %d\n", encOP.picHeight               );
    VLOG(INFO, "[linear2TiledEnable ]: %d\n", encOP.linear2TiledEnable      );
    VLOG(INFO, "[linear2TiledMode   ]: %d\n", encOP.linear2TiledMode        );
    VLOG(INFO, "[frameRateInfo      ]: %d\n", encOP.frameRateInfo           );
    VLOG(INFO, "[idrInterval        ]: %d\n", encOP.idrInterval             );
    VLOG(INFO, "[meBlkMode          ]: %d\n", encOP.meBlkMode               );
    VLOG(INFO, "[sliceMode          ]: %d\n", encOP.sliceMode               );
    VLOG(INFO, "[bitRate            ]: %d\n", encOP.bitRate                 );
    VLOG(INFO, "[cbcrInterleave     ]: %d\n", encOP.cbcrInterleave          );
    VLOG(INFO, "[frameEndian        ]: %d\n", encOP.frameEndian             );
    VLOG(INFO, "[streamEndian       ]: %d\n", encOP.streamEndian            );
    VLOG(INFO, "[sourceEndian       ]: %d\n", encOP.sourceEndian            );
    VLOG(INFO, "[bwbEnable          ]: %d\n", encOP.bwbEnable               );
    VLOG(INFO, "[packedFormat       ]: %d\n", encOP.packedFormat            );
    VLOG(INFO, "[srcFormat          ]: %d\n", encOP.srcFormat               );
    VLOG(INFO, "[srcBitDepth        ]: %d\n", encOP.srcBitDepth             );
    VLOG(INFO, "[nv21               ]: %d\n", encOP.nv21                    );
    VLOG(INFO, "[enablePTS          ]: %d\n", encOP.enablePTS               );
    VLOG(INFO, "[rcGopIQpOffsetEn   ]: %d\n", encOP.rcGopIQpOffsetEn        );
    VLOG(INFO, "[rcGopIQpOffset     ]: %d\n", encOP.rcGopIQpOffset          );
    VLOG(INFO, "[gopSize            ]: %d\n", encOP.gopSize                 );
    VLOG(INFO, "[meBlkMode          ]: %d\n", encOP.meBlkMode               );
    VLOG(INFO, "[sliceMode          ]: %d\n", encOP.sliceMode.sliceMode     );
    VLOG(INFO, "[intraRefresh       ]: %d\n", encOP.intraRefresh            );
    VLOG(INFO, "[ConscIntraRefreshEnable]: %d\n", encOP.ConscIntraRefreshEnable);
    VLOG(INFO, "[CountIntraMbEnable ]: %d\n", encOP.CountIntraMbEnable         );
    VLOG(INFO, "[userQpMax          ]: %d\n", encOP.userQpMax                  );
    VLOG(INFO, "[maxIntraSize       ]: %d\n", encOP.maxIntraSize               );
    VLOG(INFO, "[userMaxDeltaQp     ]: %d\n", encOP.userMaxDeltaQp             );
    VLOG(INFO, "[userQpMin          ]: %d\n", encOP.userQpMin                  );
    VLOG(INFO, "[userMinDeltaQp     ]: %d\n", encOP.userMinDeltaQp             );
    VLOG(INFO, "[MEUseZeroPmv       ]: %d\n", encOP.MEUseZeroPmv               );
    VLOG(INFO, "[userGamma          ]: %d\n", encOP.userGamma                  );
    VLOG(INFO, "[rcIntervalMode     ]: %d\n", encOP.rcIntervalMode             );
    VLOG(INFO, "[mbInterval         ]: %d\n", encOP.mbInterval                 );
    VLOG(INFO, "[rcEnable           ]: %d\n", encOP.rcEnable                   );
    VLOG(INFO, "[cbcrOrder          ]: %d\n", encOP.cbcrOrder                  );
    VLOG(INFO, "[encodeVuiRbsp      ]: %d\n", encOP.encodeVuiRbsp              );
    VLOG(INFO, "[avcParam.level     ]: %d\n", encOP.EncStdParam.avcParam.level          );
    VLOG(INFO, "[avcParam.profile   ]: %d\n", encOP.EncStdParam.avcParam.profile        );
    VLOG(INFO, "[avcParam.chroma    ]: %d\n", encOP.EncStdParam.avcParam.chromaFormat400);
    VLOG(INFO, "[avcParam.ppsNum    ]: %d\n", encOP.EncStdParam.avcParam.ppsNum         );
    VLOG(INFO, "-----------------------------------------------------------------------------\n");

    // Open an instance and get initial information for encoding.
    if ((ret=VPU_EncOpen(&handle, &encOP)) != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncOpen failed Error code is 0x%x \n", ret);
        success = FALSE;
        goto ERR_ENC_INIT;
    }

    VLOG(INFO, "[VPU] STEP 5 BitstreamReader_Create %s, ring %d endian %d\n", param->bitstreamFileName, encOP.ringBufferEnable, param->stream_endian);
    // VPU_EncGiveCommand(handle, ENABLE_LOGGING, NULL);
    if ((bsReader=BitstreamReader_Create(encOP.ringBufferEnable, param->bitstreamFileName, (EndianMode)param->stream_endian, &handle)) == NULL) {
        VLOG(ERR, "Failed to BitstreamReader_Create\n");
        success = FALSE;
        goto ERR_ENC_OPEN;
    }

    if ((comparator=Comparator_Create(param->compare_type, param->ref_stream_path, param->cfgFileName)) == NULL) {
        success = FALSE;
        goto ERR_ENC_OPEN;
    }

    if (param->useRot == TRUE) {
        VPU_EncGiveCommand(handle, ENABLE_ROTATION, NULL);
        VPU_EncGiveCommand(handle, ENABLE_MIRRORING, NULL);
        VPU_EncGiveCommand(handle, SET_ROTATION_ANGLE, (void*)&param->rotAngle);
        VPU_EncGiveCommand(handle, SET_MIRROR_DIRECTION, (void*)&param->mirDir);
    }

    // allocate frame buffers for source frame
    secAxiUse.u.coda9.useBitEnable  = (param->secondary_axi>>0)&0x01;
    secAxiUse.u.coda9.useIpEnable   = (param->secondary_axi>>1)&0x01;
    secAxiUse.u.coda9.useDbkYEnable = (param->secondary_axi>>2)&0x01;
    secAxiUse.u.coda9.useDbkCEnable = (param->secondary_axi>>3)&0x01;
    secAxiUse.u.coda9.useBtpEnable  = (param->secondary_axi>>4)&0x01;
    secAxiUse.u.coda9.useOvlEnable  = (param->secondary_axi>>5)&0x01;
    VPU_EncGiveCommand(handle, SET_SEC_AXI, &secAxiUse);

    VLOG(INFO, "[VPU] STEP 6 VPU_EncGetInitialInfo 0x%x, 0x%x, 0x%x, 0x%x\n",
        secAxiUse.u.coda9.useBitEnable, secAxiUse.u.coda9.useIpEnable, secAxiUse.u.coda9.useDbkYEnable, secAxiUse.u.coda9.useDbkCEnable);
    /********************************************************************************
     * INIT_SEQ                                                                     *
     ********************************************************************************/
    ret = VPU_EncGetInitialInfo(handle, &initialInfo);
    if( ret != RETCODE_SUCCESS ) {
        VLOG(ERR, "VPU_EncGetInitialInfo failed Error code is 0x%x \n", ret );
        success = FALSE;
        PrintVpuStatus(coreIdx, PRODUCT_ID_980);
        goto ERR_ENC_OPEN;
    }


    // Note: The below values of MaverickCache configuration are best values.
    MaverickCache2Config(
        &encCacheConfig,
        0, //encoder
        encOP.cbcrInterleave, // cb cr interleave
        0, /* bypass */
        0, /* burst  */
        3, /* merge mode */
        mapType,
        15 /* shape */);
    VPU_EncGiveCommand(handle, SET_CACHE_CONFIG, &encCacheConfig);

    /********************************************************************************
     * ALLOCATE RECON BUFFERS                                                       *
     ********************************************************************************/
    if (encOP.bitstreamFormat == STD_AVC) {
        srcFrameFormat = encOP.EncStdParam.avcParam.chromaFormat400?FORMAT_400:FORMAT_420;
        framebufFormat = encOP.EncStdParam.avcParam.chromaFormat400?FORMAT_400:FORMAT_420;
    }
    else {
        srcFrameFormat = FORMAT_420;
        framebufFormat = FORMAT_420;
    }
    framebufWidth    = (param->rotAngle==90||param->rotAngle ==270)?encOP.picHeight:encOP.picWidth;
    framebufHeight   = (param->rotAngle==90||param->rotAngle ==270)?encOP.picWidth:encOP.picHeight;
    framebufWidth    = VPU_ALIGN16(framebufWidth);
    framebufHeight   = VPU_ALIGN32(framebufHeight); // To cover interlaced picture
    framebufStride   = CalcStride(framebufWidth, framebufHeight, framebufFormat, encOP.cbcrInterleave, mapType, FALSE);
    regFrameBufCount = initialInfo.minFrameBufferCount;

    osal_memset((void*)&fbAllocInfo, 0x00, sizeof(fbAllocInfo));
    osal_memset((void*)fbRecon, 0x00, sizeof(fbRecon));
    osal_memset((void*)vbReconFrameMem, 0x00, sizeof(vbReconFrameMem));
    fbAllocInfo.format          = FORMAT_420;
    fbAllocInfo.cbcrInterleave  = encOP.cbcrInterleave;
    fbAllocInfo.mapType         = mapType;
    fbAllocInfo.stride          = framebufStride;
    fbAllocInfo.height          = framebufHeight;
    fbAllocInfo.lumaBitDepth    = 8;
    fbAllocInfo.chromaBitDepth  = 8;
    fbAllocInfo.num             = regFrameBufCount;
    fbAllocInfo.endian          = encOP.frameEndian;
    fbAllocInfo.type            = FB_TYPE_CODEC;

    sizeFb = VPU_GetFrameBufSize(coreIdx, framebufStride, framebufHeight, mapType, framebufFormat, encOP.cbcrInterleave, NULL);
    for (i=0; i<regFrameBufCount; i++) {
        vbReconFrameMem[i].size = sizeFb;
        if (vdi_allocate_dma_memory(coreIdx, &vbReconFrameMem[i]) < 0) {
            VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
            success = FALSE;
            goto ERR_ENC_OPEN;
        }
        fbRecon[i].bufY  = vbReconFrameMem[i].phys_addr;
        fbRecon[i].bufCb = (PhysicalAddress)-1;
        fbRecon[i].bufCr = (PhysicalAddress)-1;
        fbRecon[i].updateFbInfo = TRUE;
    }

    VLOG(INFO, "[VPU] STEP 7 VPU_EncAllocateFrameBuffer recon_cnt %d, stride %d, height %d, maptype %d, format %d, size %d\n",
        regFrameBufCount, framebufStride, framebufHeight, mapType, framebufFormat, sizeFb);

    ret = VPU_EncAllocateFrameBuffer(handle, fbAllocInfo, fbRecon);
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
        success = FALSE;
        goto ERR_ENC_OPEN;
    }

    VLOG(INFO, "[VPU] STEP 7.1 \n");

    /********************************************************************************
     * SET_FRAMEBUF                                                                 *
     ********************************************************************************/
    ret = VPU_EncRegisterFrameBuffer(handle, fbRecon, regFrameBufCount, framebufWidth, framebufHeight, mapType);
    if( ret != RETCODE_SUCCESS ) {
        VLOG(ERR, "VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        success = FALSE;
        goto ERR_ENC_OPEN;
    }

    VLOG(INFO, "[VPU] STEP 7.2.1 \n");
    VPU_EncGiveCommand(handle, GET_TILEDMAP_CONFIG, &mapCfg);

    /********************************************************************************
     * ALLOCATE SROUCE BUFFERS                                                      *
     ********************************************************************************/
    fbAllocInfo.format         = srcFrameFormat;
    fbAllocInfo.cbcrInterleave = encOP.cbcrInterleave;
    fbAllocInfo.nv21           = encOP.nv21;
    if (encOP.linear2TiledEnable == TRUE) {
        fbAllocInfo.mapType = LINEAR_FRAME_MAP;
        fbAllocInfo.stride  = encOP.picWidth;
    }
    else {
        fbAllocInfo.mapType = mapType;
        fbAllocInfo.stride  = CalcStride(encOP.picWidth, encOP.picHeight, srcFrameFormat, encOP.cbcrInterleave, mapType, FALSE);
    }
    fbAllocInfo.height  = VPU_ALIGN16(encOP.picHeight);
    fbAllocInfo.num     = ENC_SRC_BUF_NUM;
    fbAllocInfo.endian  = encOP.frameEndian;
    fbAllocInfo.type    = FB_TYPE_PPU;

    osal_memset(vbSrcFrameMem, 0x00, sizeof(vbSrcFrameMem));
    sizeFb = VPU_GetFrameBufSize(coreIdx, fbAllocInfo.stride, fbAllocInfo.height, fbAllocInfo.mapType, srcFrameFormat, encOP.cbcrInterleave, NULL);

    for (i=0; i<ENC_SRC_BUF_NUM; i++) {
        vbSrcFrameMem[i].size = sizeFb;
        if (vdi_allocate_dma_memory(coreIdx, &vbSrcFrameMem[i]) < 0) {
            VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
            success = FALSE;
            goto ERR_ENC_OPEN;
        }
        fbSrc[i].bufY  = vbSrcFrameMem[i].phys_addr;
        fbSrc[i].bufCb = (PhysicalAddress)-1;
        fbSrc[i].bufCr = (PhysicalAddress)-1;
        fbSrc[i].updateFbInfo = TRUE;
    }

    VLOG(INFO, "[VPU] STEP 8 VPU_EncAllocateFrameBuffer stride %d, height %d, maptype %d, format %d, size %d\n",
        fbAllocInfo.stride, fbAllocInfo.height, fbAllocInfo.mapType, srcFrameFormat, sizeFb);
    ret = VPU_EncAllocateFrameBuffer(handle, fbAllocInfo, fbSrc);
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
        success = FALSE;
        goto ERR_ENC_OPEN;
    }

    /********************************************************************************
     * BUILD SEQUENCE HEADER                                                        *
     ********************************************************************************/
    encParam.forceIPicture = 0;
    encParam.skipPicture   = 0;
    encParam.quantParam    = param->picQpY;

    encHeaderParam.zeroPaddingEnable = 0;
    if (encOP.ringBufferEnable == FALSE) {
        encHeaderParam.buf = vbStream.phys_addr;
        encHeaderParam.size = vbStream.size;
    }

    if (encOP.bitstreamFormat == STD_MPEG4) {
        encHeaderParam.headerType = 0;
        encHeaderParam.size       = vbStream.size;
        VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
        if (encOP.ringBufferEnable == FALSE) {
            EnterLock(coreIdx);
            ret = BitstreamReader_Act(bsReader, encHeaderParam.buf, encOP.bitstreamBufferSize, 0, comparator);
            LeaveLock(coreIdx);
            if (ret == FALSE) {
                success = FALSE;
                goto ERR_ENC_OPEN;
            }
        }
#ifdef MP4_ENC_VOL_HEADER
        encHeaderParam.headerType = VOL_HEADER;
        encHeaderParam.size       = vbStream.size;
        if ((ret=VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam)) != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VOL_HEADER failed Error code is 0x%x \n", ret);
            success = FALSE;
            goto ERR_ENC_OPEN;
        }
        if (encOP.ringBufferEnable == FALSE) {
            EnterLock(coreIdx);
            ret = BitstreamReader_Act(bsReader, encHeaderParam.buf, encOP.bitstreamBufferSize, 0, comparator);
            LeaveLock(coreIdx);
            if (ret == FALSE) {
                success = FALSE;
                goto ERR_ENC_OPEN;
            }
        }
#endif
    }
    else if(encOP.bitstreamFormat == STD_AVC) {
        encHeaderParam.headerType = SPS_RBSP;
        if (encOP.ringBufferEnable == FALSE) {
            encHeaderParam.buf  = vbStream.phys_addr;
        }
        encHeaderParam.size = vbStream.size;

        if ((ret=VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam)) != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for SPS_RBSP failed Error code is 0x%x \n", ret);
            success = FALSE;
            goto ERR_ENC_OPEN;
        }

        if (encOP.ringBufferEnable == FALSE) {
            EnterLock(coreIdx);
            ret = BitstreamReader_Act(bsReader, encHeaderParam.buf, encHeaderParam.size, 0, comparator);
            LeaveLock(coreIdx);
            if (ret == FALSE) {
                success = FALSE;
                goto ERR_ENC_OPEN;
            }
        }
        encHeaderParam.headerType = PPS_RBSP;
        VLOG(INFO, "[VPU] STEP 9 ENC_PUT_VIDEO_HEADER, ppsNum %d, header %d\n", encOP.EncStdParam.avcParam.ppsNum, encHeaderParam.size);
        for (i=0; i<encOP.EncStdParam.avcParam.ppsNum; i++) {
            if (encOP.ringBufferEnable == FALSE) {
                encHeaderParam.buf  = vbStream.phys_addr;
                encHeaderParam.pBuf = (BYTE *)vbStream.virt_addr;
            }
            encHeaderParam.size = vbStream.size;
            ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
            if (ret != RETCODE_SUCCESS) {
                VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for PPS_RBSP failed Error code is 0x%x \n", ret);
                success = FALSE;
                goto ERR_ENC_OPEN;
            }
            if (encOP.ringBufferEnable == FALSE) {
                EnterLock(coreIdx);
                ret = BitstreamReader_Act(bsReader, encHeaderParam.buf, encHeaderParam.size, 0, comparator);
                LeaveLock(coreIdx);
                if (ret == FALSE) {
                    success = FALSE;
                    goto ERR_ENC_OPEN;
                }
            }
        }

    }


    sprintf(yuvPath, "%s/%s", param->yuvSourceBaseDir, param->yuvFileName);
    ChangePathStyle(yuvPath);
    VLOG(INFO, "[VPU] STEP 10 YuvFeeder_Create %s\n", yuvPath);

    // If YUV is NV12 or NV21 format, the value of 6th parameter should be FALSE.
    if ((yuvFeeder=YuvFeeder_Create(SOURCE_YUV, yuvPath, FALSE, encOP.picWidth, encOP.picHeight, encOP.cbcrInterleave, TRUE)) == NULL) {
        VLOG(ERR, "Failed to YuvFeeder_Create\n");
        success = FALSE;
        goto ERR_ENC_OPEN;
    }

    DisplayEncodedInformation(handle, encOP.bitstreamFormat, 0, NULL);
    VPU_EncGiveCommand(handle, GET_TILEDMAP_CONFIG, &mapConfig);
    /********************************************************************************
     * ENCODE                                                                       *
     ********************************************************************************/
    while (loop == TRUE && (param->exitFlag == 0)) {

    #if 0
        if(param->enableThread)
        {
            if(param->plock)
            {
                while(1)
                {
                    uint32_t need_dec = 0;
                    mutex_acquire(param->plock);
                    need_dec = *param->pNeedDecFrms;
                    if(need_dec)
                    {
                        *param->pNeedDecFrms = (need_dec - 1);
                        mutex_release(param->plock);
                        break;

                    }
                    mutex_release(param->plock);
                    thread_sleep(10);
                }
            }
        }
    #endif

        if (osal_kbhit()) {
            success = FALSE;
            break;
        }

        if (frameIdx >= param->outNum) {
            break;
        }

        srcFrameIdx = (frameIdx%ENC_SRC_BUF_NUM);

        VLOG(INFO, "[VPU] STEP 11 YuvFeeder_Feed %d, %d-%d\n", srcFrameIdx, encOP.picWidth, encOP.picHeight);
        if (YuvFeeder_Feed(yuvFeeder, coreIdx, &fbSrc[srcFrameIdx], encOP.picWidth, encOP.picHeight, &mapConfig) == FALSE) {
            VLOG(INFO, "Read YUV done!!!\n");
            break;
        }
        encParam.sourceFrame = &fbSrc[srcFrameIdx];

        fieldDone = FALSE;
FILED_ENCODE:
        // VLOG(INFO, "[main]: Go %d\n", __LINE__);
        if (encOP.ringBufferEnable == 0) {
            encParam.picStreamBufferAddr = vbStream.phys_addr; // can set the newly allocated buffer.
            encParam.picStreamBufferSize = vbStream.size;
        }
        encParam.sourceFrame->endian = encOP.frameEndian;
        encParam.sourceFrame->cbcrInterleave = encOP.cbcrInterleave;
        encParam.sourceFrame->sourceLBurstEn = FALSE;

        isIdr  = FALSE;
        isIPic = FALSE;
        if (encOP.EncStdParam.avcParam.fieldFlag) {
            encParam.fieldRun = TRUE;
        }
        if (frameIdx == 0) {
            isIdr  = TRUE;
            isIPic = TRUE;
        }
        else {
            if ((encOP.idrInterval > 0) && (cfgParam.GopPicNum > 0)) {
                if ((frameIdx%cfgParam.GopPicNum) == 0) {
                    isIPic = TRUE;
                    iPicCnt++;
                }
            }
        }

        if (encOP.bitstreamFormat == STD_MPEG4 ) {
            if (isIPic == TRUE && encOP.idrInterval > 0) {
                if ((iPicCnt%encOP.idrInterval) == 0) {
                    isIdr = TRUE;
                }
            }
        }
        encParam.forceIPicture = fieldDone ? FALSE : isIdr;

        encParam.skipPicture = 0;
        minSkipNum = 0;
        for (i=0; i<MAX_PIC_SKIP_NUM; i++) {
            uint32_t numPicSkip = cfgParam.field_flag ? param->skipPicNums[i]/2 : param->skipPicNums[i];
            if (numPicSkip > minSkipNum && numPicSkip == (uint32_t)frameIdx) {
                if (cfgParam.field_flag == FALSE) {
                    encParam.skipPicture = TRUE;
                } else {
                    if (param->skipPicNums[i]%2) encParam.skipPicture = fieldDone;
                    else                             encParam.skipPicture = !fieldDone;
                    /* check next skip field */
                    if ((i+1) < MAX_PIC_SKIP_NUM) {
                        numPicSkip = param->skipPicNums[i+1]/2;
                        if (numPicSkip == (uint32_t)frameIdx) encParam.skipPicture = TRUE;
                    }
                }
                break;
            }
        }

#ifdef SUPPORT_ROI_50

        {
            int i;
            encParam.setROI.mode = 1;
            encParam.setROI.number =10;     // up to 50

            for (i=0; i<encParam.setROI.number; i++)
            {

                encParam.setROI.region[i].bottom = 16 + i*16;
                encParam.setROI.region[i].top = i*16;
                encParam.setROI.region[i].right = 16 + i*16;
                encParam.setROI.region[i].left = i*16;

                if((i%2) == 0)
                    encParam.setROI.qp[i]=8;
                else
                    encParam.setROI.qp[i]=5;
            }
        }

#endif

        // Start encoding a frame.
        VLOG(INFO, "[VPU] STEP 12 VPU_EncStartOneFrame\n");
        VLOG(INFO, "------------------------------ ENCODER PARAM ------------------------------\n");
        VLOG(INFO, "[forceIPicture                  ]: %d\n", encParam.forceIPicture                 );
        VLOG(INFO, "[skipPicture                    ]: %d\n", encParam.skipPicture                 );
        VLOG(INFO, "[quantParam                     ]: %d\n", encParam.quantParam                 );
        VLOG(INFO, "[forcePicQpEnable               ]: %d\n", encParam.forcePicQpEnable                 );
        VLOG(INFO, "[forcePicQpP                    ]: %d\n", encParam.forcePicQpP                 );
        VLOG(INFO, "[forcePicTypeEnable             ]: %d\n", encParam.forcePicTypeEnable                 );
        VLOG(INFO, "[forcePicType                   ]: %d\n", encParam.forcePicType                 );
        VLOG(INFO, "[setROI.mode                    ]: %d\n", encParam.setROI.mode                 );
        VLOG(INFO, "[setROI.num                     ]: %d\n", encParam.setROI.number                 );
        VLOG(INFO, "[codeOption.implicitHeaderEncode]: %d\n", encParam.codeOption.implicitHeaderEncode                 );
        VLOG(INFO, "[codeOption.encodeVCL           ]: %d\n", encParam.codeOption.encodeVCL                 );

        startUs = GetNowUs();
        if ((ret=VPU_EncStartOneFrame(handle, &encParam)) != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_EncStartOneFrame failed Error code is 0x%x \n", ret );
            success = FALSE;
            goto ERR_ENC_OPEN;
        }

        timeoutCount = 0;
        while (timeoutCount < VPU_ENC_TIMEOUT) {
            interruptFlag = VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT);
            if (interruptFlag == (int32_t)-1) {
                if (timeoutCount*VPU_WAIT_TIME_OUT > VPU_ENC_TIMEOUT) {
                    VLOG(ERR, "Error : encoder timeout happened\n");
                    VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                    success = FALSE;
                    break;
                }
                interruptFlag = 0;
                timeoutCount++;
            }

            if (success == TRUE) {
                if (encOP.ringBufferEnable == TRUE) {
                    ret = BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, DEFAULT_READ_SIZE, comparator);
                    if (ret == FALSE) {
                        int room;
                        PhysicalAddress rdPtr, wrPtr;
                        success = FALSE;
                        /* Flush bitstream buffer */
                        VPU_EncGetBitstreamBuffer(handle, &rdPtr, &wrPtr, &room);
                        VPU_EncUpdateBitstreamBuffer(handle, room);
                    }
                }
                else {
                    // Linebuffer is Full interrupt when linebuffer interrupt mode is set to 1
                    if (interruptFlag & (1<<INT_BIT_BIT_BUF_FULL)) {
                        ret = BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, DEFAULT_READ_SIZE, comparator);
                        if (ret == FALSE) {
                            VPU_ClearInterrupt(coreIdx);
                            VPU_EncGetOutputInfo(handle, &outputInfo);
                            success = FALSE;
                            goto ERR_ENC_OPEN;
                        }
                    }
                }
            }

            if (interruptFlag > 0) {
                VPU_ClearInterrupt(coreIdx);
                if (interruptFlag & (1<<INT_BIT_PIC_RUN)) {
                    break;
                }
            }
        }

        ret = VPU_EncGetOutputInfo(handle, &outputInfo);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_EncGetOutputInfo failed Error code is 0x%x \n", ret );
            success = FALSE;
            goto ERR_ENC_OPEN;
        }

        VLOG(INFO, "[VPU] STEP 12.1 VPU_EncGetOutputInfo ret %d\n", ret);
        /* mismatch in WaitInterrupt loop */
        if (success == FALSE) {
            goto ERR_ENC_OPEN;
        }

        if (success) {
            diffUs = GetNowUs() - startUs;
            totalUs += diffUs;
            if (outputInfo.picType == PIC_TYPE_I
                || outputInfo.picType == PIC_TYPE_P
                || outputInfo.picType == PIC_TYPE_B
                || outputInfo.picType == PIC_TYPE_IDR) {
                VLOG(TRACE, "encoding time=%.1fms\n", (double)diffUs/1000);
            }
        }
        DisplayEncodedInformation(handle, encOP.bitstreamFormat, frameIdx, &outputInfo);

        if (encOP.ringBufferEnable == FALSE) {
            if (outputInfo.bitstreamWrapAround == TRUE) {
                // If LineBuffer interrupt is set to 1, it is ok to work.
                VLOG(WARN, "Warning!! BitStream buffer wrap-arounded. prepare more large buffer. Consumed all remained stream\n");
                EnterLock(coreIdx);
                ret = BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, 0, comparator);
                LeaveLock(coreIdx);
                if (ret == FALSE) {
                    success = FALSE;
                    goto ERR_ENC_OPEN;
                }
            }
            if (outputInfo.bitstreamSize == 0) {
                VLOG(ERR, "bitstreamsize = 0 \n");
            }

            VLOG(INFO, "BitstreamReader_Act %d, output %d\n", __LINE__, outputInfo.bitstreamSize);

            EnterLock(coreIdx);
            ret = BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, 0, comparator);
            LeaveLock(coreIdx);
            if (ret == FALSE) {
                success = FALSE;
                break;
            }
        }

        if (encParam.fieldRun && fieldDone == FALSE) {
            // VLOG(INFO, "[main]: Go %d\n", __LINE__);
            fieldDone = TRUE;
            goto FILED_ENCODE;
        }
        frameIdx++;
        if (frameIdx > (param->outNum-1)) {
            VLOG(INFO, "[main]: Go %d, frameIdx %d, %d\n", __LINE__, frameIdx, param->outNum);
            break;
        }
    }

    if (encOP.ringBufferEnable == TRUE) {
        EnterLock(coreIdx);
        ret = BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, DEFAULT_READ_SIZE, comparator);
        LeaveLock(coreIdx);
        if (ret == FALSE) {
            VLOG(ERR, "ReadBsRingBufHelper failed Error code is 0x%x \n", ret );
            success = FALSE;
            goto ERR_ENC_OPEN;
        }
    }

ERR_ENC_OPEN:
    // VLOG(INFO, "[main]: Go %d\n", __LINE__);
    // Now that we are done with encoding, close the open instance.
    VPU_EncClose(handle);

    double totalsec = totalUs / 1E6;
    VLOG(INFO, "\nEnc End. Tot Frame %d, time consumed %lld us(%.2f sec), %.2f\n",
        frameIdx, totalUs, totalsec, (double)frameIdx/totalsec);


ERR_ENC_INIT:
    for (i=0; i<regFrameBufCount; i++) {
        if (vbReconFrameMem[i].size > 0) vdi_free_dma_memory(coreIdx, &vbReconFrameMem[i]);
    }
    for (i=0; i<ENC_SRC_BUF_NUM; i++) {
        if (vbSrcFrameMem[i].size > 0 ) vdi_free_dma_memory(coreIdx, &vbSrcFrameMem[i]);
    }

    vdi_free_dma_memory(coreIdx, &vbStream);

    if (bsReader != NULL)   BitstreamReader_Destroy(bsReader);
    if (yuvFeeder != NULL) YuvFeeder_Destroy(yuvFeeder);
    if (comparator != NULL) Comparator_Destroy(comparator);

    if (pusBitCode != NULL) {
        osal_free(pusBitCode);
        pusBitCode = NULL;
    }

    if (VPU_DeInit(coreIdx) != RETCODE_SUCCESS) {
        success = FALSE;
    }

    param->exitFlag = THREAD_EXIT_SUCCESS;
    VLOG(INFO, "TestEncoder exit now ...\n");
    return success;
}

#if 0
static TestEncConfig   encConfig;
int vpu_main_enc(int argc, char **argv)
{
    int             opt, index;
    BOOL            ret;
    //TestEncConfig   encConfig;
    struct option options[] = {
        {(char *)"output",                1, NULL, 0},    /*  0 */  //!<<bitstreamFileName
        {(char *)"reserved",              1, NULL, 0},              //!<<yuvFileName
        {(char *)"codec",                 1, NULL, 0},              //!<<stdMode
        {(char *)"reserved",              1, NULL, 0},
        {(char *)"cfg-path",              1, NULL, 0},
        {(char *)"coreIdx",               1, NULL, 0},  /*  5 */
        {(char *)"yuv-base",              1, NULL, 0},
        {(char *)"reserved",              1, NULL, 0},
        {(char *)"reserved",              1, NULL, 0},
        {(char *)"reserved",              1, NULL, 0},              //!<< reserved for future
        {(char *)"reserved",              1, NULL, 0},   /* 10 */   //!<< reserved for future
        {(char *)"enable-ringBuffer",     0, NULL, 0},              //!<< ringBufferEnable
        {(char *)"enable-lineBufInt",     0, NULL, 0},              //!<< LineBufIntEn
        {(char *)"maptype",               1, NULL, 0},              //!<< refer to TiledMapType in vpuapi.h
        {(char *)"reserved",              1, NULL, 0},
        {(char *)"reserved",              1, NULL, 0},    /* 15 */
        {(char *)"enable-cbcrInterleave", 0, NULL, 0},
        {(char *)"enable-nv21",           0, NULL, 0},
        {(char *)"enable-linear2tiled",   0, NULL, 0},
        {(char *)"rotate",                1, NULL, 0},
        {(char *)"mirror",                1, NULL, 0},    /* 20 */
        {(char *)"secondary-axi",         1, NULL, 0},
        {(char *)"frame-endian",          1, NULL, 0},
        {(char *)"stream-endian",         1, NULL, 0},
        {(char *)"reserved",              1, NULL, 0},
        {(char *)"ref_stream_path",       1, NULL, 0},    /* 25 */
        {NULL,                    0, NULL, 0},
    };
    BOOL            showVersion = FALSE;
    char*           optString   = (char *)"c:hn:v";
    int i;

    InitLog();

    //default setting.
    osal_memset((void*)&encConfig, 0, sizeof(encConfig));
    encConfig.stdMode       = STD_AVC;
    encConfig.frame_endian  = VDI_LITTLE_ENDIAN;
    encConfig.stream_endian = VDI_LITTLE_ENDIAN;
    encConfig.ringBufferEnable  = FALSE;

    if(argc>1)
    {
        printf("vpu videoformat maptype forceOutNum\n");
    }

    for(i=1; i<argc; i++)
    {
        switch(i)
        {
            case 1:
                encConfig.outNum = atoi(argv[i]);
                printf("outNum %d\n", encConfig.outNum);
            break;

            case 2:
                encConfig.stdMode = atoi(argv[i]);
                printf("stdMode %d\n", encConfig.stdMode);
            break;


            default:
                break;
        }
    }

    sprintf(encConfig.bitstreamFileName, "output.bin");
    sprintf(encConfig.yuvSourceBaseDir, "./");

#if 0
    while ((opt=getopt_long(argc, argv, optString, options, &index)) != -1) {
        switch (opt) {
        case 'c':
            encConfig.compare_type = (TRUE == (BOOL)atoi(optarg)) ? STREAM_COMPARE : NO_COMPARE;
            break;
        case 'h':
            Help(argv[0]);
            return 0;
            break;
        case 'n':
            encConfig.outNum = atoi(optarg);
            break;
        case 'v':
            showVersion = TRUE;
            break;
        case 0:
            switch(index){
            case 0:
                strcpy(encConfig.bitstreamFileName, optarg);
                ChangePathStyle(encConfig.bitstreamFileName);
                break;
            case 2:
                index = atoi(optarg);
                if (index != (int32_t)STD_AVC && index != (int32_t)STD_MPEG4 && index != (int32_t)STD_H263) {
                    VLOG(ERR, "Invalid codec index(%d)\n", index);
                    Help(argv[0]);
                    //exit(1);
                    return 1;
                }
                encConfig.stdMode = (CodStd)atoi(optarg);
                break;
            case 3:
                encConfig.outNum = atoi(optarg);
                if( encConfig.outNum < 0 ) {
                    encConfig.outNum = 0;
                }
                break;
            case 4:
                memcpy(encConfig.cfgFileName, optarg, strlen(optarg));
                ChangePathStyle(encConfig.cfgFileName);
                break;
            case 5:
                encConfig.coreIdx = atoi(optarg);
                break;
            case 6:
                strcpy(encConfig.yuvSourceBaseDir, optarg);
                ChangePathStyle(encConfig.yuvSourceBaseDir);
                break;
            case 11:
                encConfig.ringBufferEnable = TRUE;
                break;
            case 12:
                encConfig.lineBufIntEn = TRUE;
                break;
            case 13:
                encConfig.mapType = atoi(optarg);
                break;
            case 15:
                encConfig.loopCount = atoi(optarg);
                break;
            case 16:
                encConfig.cbcrInterleave = TRUE;
                break;
            case 17:
                encConfig.nv21 = TRUE;
                break;
            case 18:
                encConfig.coda9.enableLinear2Tiled = TRUE;
                encConfig.coda9.linear2TiledMode   = FF_FRAME;
                break;
            case 19:
                encConfig.rotAngle = atoi(optarg);
                break;
            case 20:
                encConfig.mirDir = atoi(optarg);
                break;
            case 21:
                encConfig.secondary_axi = atoi(optarg);
                break;
            case 22:
                encConfig.frame_endian = atoi(optarg);
                break;
            case 23:
                encConfig.stream_endian = atoi(optarg);
                break;
            case 25:
                memcpy(encConfig.ref_stream_path, optarg, strlen(optarg));
                ChangePathStyle(encConfig.ref_stream_path);
                break;
            default:
                VLOG(ERR, "optarg: %s\n", optarg);
                Help(argv[0]);
                return 1;
                break;
            }
            break;
        case '?':
            Help(argv[0]);
            //exit(1);
            return 1;
            break;
        default:
            VLOG(ERR, "%s\n", optarg);
            Help(argv[0]);
            //exit(1);
            return 1;
        }
    }
#endif

    g_vdi_virt_addr = (unsigned long)memalign(0x1000, SEMI_VDB_VIDEO_SIZE);
    if (!g_vdi_virt_addr) {
        printf("failed to allocate buffer\n");
    }

#ifdef SEMI_SW_TEST
        pusBitCode = bit_code;
        sizeInWord = 116736;
#else
    if (LoadFirmware(PRODUCT_ID_980, (Uint8**)&pusBitCode, &sizeInWord, CORE_1_BIT_CODE_FILE_PATH) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, CORE_1_BIT_CODE_FILE_PATH);
        return 1;
    }
#endif

    if (showVersion) {
        uint32_t ver, rev;
        uint32_t productId;

        ret = VPU_InitWithBitcode(encConfig.coreIdx, (const Uint16*)pusBitCode, sizeInWord);
        if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
            VLOG(ERR, "Failed to boot up VPU(coreIdx: %d)\n", __FUNCTION__, __LINE__, encConfig.coreIdx);
            return 1;
        }
        VPU_GetVersionInfo(0, &ver, &rev, &productId);
        VPU_DeInit(0);
        printf("VERSION=%x\n", ver);
        printf("REVISION=%d\n", rev);
        printf("PRODUCT_ID=%d\n", productId);
        return 0;
    }

    if (encConfig.mapType == TILED_FRAME_MB_RASTER_MAP || encConfig.mapType == TILED_FIELD_MB_RASTER_MAP) {
        encConfig.cbcrInterleave = TRUE;
    }

    osal_init_keyboard();

    if (encConfig.rotAngle > 0 || encConfig.mirDir > 0) {
        encConfig.useRot = TRUE;
    }

    ret = TestEncoder(&encConfig);

    osal_close_keyboard();

 #ifndef SEMI_SW_TEST
    osal_free(pusBitCode);
#endif
    free((void *)g_vdi_virt_addr);

    if (ret == FALSE) {
        VLOG(ERR, "[RESULT] FAILURE\n");
    }

    return ret == TRUE ? 0 : 1;
}
#endif
