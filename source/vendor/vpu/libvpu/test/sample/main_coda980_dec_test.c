//--=========================================================================--
//  This file is a part of VPUAPI
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2004 - 2014   CHIPS&MEDIA INC.
//            (C) CPPYRIGHT 2020 Semidrive Technology Ltd.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include "main_helper.h"
#include "config.h"

#define STREAM_BUF_SIZE                 1024*1024*4    // max bitstream size
#define MAX_COUNT_NO_RESPONSE           3           // For integration test.
#define PPU_FB_COUNT                    2
#define EXTRA_FRAME_BUFFER_NUM          1


BOOL TestDecoderCoda(TestDecConfig *param)
{
    DecHandle           handle = NULL;
    DecOpenParam        decOP;
    DecInitialInfo      sequenceInfo;
    DecOutputInfo       outputInfo, *pDisplayInfo;
    DecParam            decParam;
    vpu_buffer_t        vbStream;
    SecAxiUse           secAxiUse;
    RetCode             ret                 = RETCODE_SUCCESS;
    int32_t             framebufHeight      = 0;
    BOOL                success             = TRUE;
    BOOL                doDumpImage         = FALSE;
    int32_t             fbCount;
    int32_t             ppuFbCount          = PPU_FB_COUNT;
    int32_t             decodedIdx          = 0;
    int32_t             index;
    BOOL                repeat              = TRUE;
    BOOL                seqInited           = FALSE;
    BOOL                seqInitEscape       = FALSE;
    int32_t             timeoutCount = 0;
    int32_t             interruptFlag = 0;
    int32_t             instIdx=0, coreIdx;
    FrameBuffer         pFrame[MAX_REG_FRAME];
    vpu_buffer_t        pFbMem[MAX_REG_FRAME];
    FrameBuffer*        ppuFb;
    FrameBuffer         pPPUFrame[MAX_REG_FRAME];
    vpu_buffer_t        pPPUFbMem[MAX_REG_FRAME];
    vpu_buffer_t*       pvb     = NULL;
    int32_t             productId, prevFbIndex;
    uint32_t            framebufStride, framebufSize;
    int32_t             nWritten;
    BSFeeder            feeder                  = NULL;
    Renderer            renderer                = NULL;
    RenderDeviceType    rendererType            = param->renderType;
    FILE*               saveFp                  = NULL;
    Comparator          comparator              = NULL;
    // char*               goldenPath              = NULL;
    BOOL                enablePPU               = FALSE;
    BOOL                waitPostProcessing      = FALSE;
    BOOL                needStream              = FALSE;
    BOOL                seqChangeRequest        = FALSE;
    int32_t             seqChangedRdPtr         = 0;
    int32_t             seqChangedWrPtr         = 0;
    int32_t             seqChangedStreamEndFlag = 0;
    uint32_t            noResponseCount         = MAX_COUNT_NO_RESPONSE;
    Queue*              ppuQ                    = NULL;
    VpuRect             rcPpu;
    BOOL                assertedFieldDoneInt    = FALSE;
    FrameBufferAllocInfo fbAllocInfo;
    MaverickCacheConfig cacheCfg;
    uint32_t            noFbCount               = 0;
    uint32_t            dispIdx                 = 0;
    uint32_t            loopCount               = 0;
    VpuReportConfig_t   decReportCfg;
    int64_t             startUs                 = 0;
    int64_t             diffUs                  = 0;
    int64_t             totalUs                 = 0;
    uint32_t            sizeInWord              = 0;
    uint16_t*           pusBitCode              = NULL;

    osal_memset(&decOP,         0x00, sizeof(DecOpenParam));
    osal_memset(&sequenceInfo,  0x00, sizeof(DecInitialInfo));
    osal_memset(&outputInfo,    0x00, sizeof(DecOutputInfo));
    osal_memset(&decParam,      0x00, sizeof(DecParam));
    osal_memset(&vbStream,      0x00, sizeof(vpu_buffer_t));
    osal_memset(&secAxiUse,     0x00, sizeof(SecAxiUse));
    osal_memset(&pFbMem,        0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);
    osal_memset(&pPPUFbMem,     0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);

    param->enableCrop = TRUE;
    coreIdx = param->coreIdx;
    instIdx = param->instIdx;
    loopCount = param->loopCount;

    if ((productId=VPU_GetProductId(coreIdx)) == -1) {
        VLOG(ERR, "Failed to get product ID\n");
        success=FALSE;
        goto ERR_DEC_INIT;
    }
    VLOG(ERR, "[main] VPU_GetProductId %d\n", productId);
    if (productId != PRODUCT_ID_980) {
        VLOG(ERR, "unsupported product id: %d\n", productId);
        goto ERR_DEC_INIT;
    }

    VLOG(INFO, "LoadFirmware %d, %s\n", PRODUCT_ID_980, CORE_1_BIT_CODE_FILE_PATH);
    if (LoadFirmware(PRODUCT_ID_980, (Uint8**)&pusBitCode, &sizeInWord, CORE_1_BIT_CODE_FILE_PATH) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, CORE_1_BIT_CODE_FILE_PATH);
        return FALSE;
    }
    VLOG(ERR, "end LoadFirmware %d, %s, %d\n", PRODUCT_ID_980, CORE_1_BIT_CODE_FILE_PATH, sizeInWord);

    VLOG(ERR, "[main] VPU_InitWithBitcode %p, %d\n", pusBitCode, sizeInWord);
    ret = VPU_InitWithBitcode(coreIdx, (const uint16_t*)pusBitCode, sizeInWord);
    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
        VLOG(ERR, "Failed to boot up VPU(coreIdx: %d, productId: %d)\n", coreIdx, productId);
        goto ERR_DEC_INIT;
    }

    VLOG(ERR, "[main] has inited with firmware\n");
    PrintVpuVersionInfo(coreIdx);

    vbStream.size = STREAM_BUF_SIZE;
    VLOG(ERR, "[main] allocate vbStream %x\n", vbStream.size);
    if (vdi_allocate_dma_memory(coreIdx, &vbStream) < 0) {
        VLOG(ERR, "fail to allocate bitstream buffer\n" );
        success=FALSE;
        goto ERR_DEC_INIT;
    }

    VLOG(INFO, "[main] vbStream base 0x%lx, phy 0x%lx, virt 0x%lx\n", vbStream.base, vbStream.phys_addr, vbStream.virt_addr);
    if (param->bitstreamMode == BS_MODE_PIC_END || param->bitFormat == STD_THO) {
        CodStd      codecId;
        uint32_t    mp4Id;
        feeder=BitstreamFeeder_Create(param->inputPath, FEEDING_METHOD_FRAME_SIZE, vbStream.phys_addr, vbStream.size, param->bitFormat, &codecId, &mp4Id, NULL, NULL);
        param->bitFormat      = codecId;
        param->coda9.mp4class = mp4Id;
        VLOG(INFO, "[Main1] vbStream base 0x%lx, phy 0x%lx, virt 0x%lx\n", vbStream.base, vbStream.phys_addr, vbStream.virt_addr);
    }
    else {
        feeder=BitstreamFeeder_Create(param->inputPath, FEEDING_METHOD_FIXED_SIZE, vbStream.phys_addr, vbStream.size, 0x2000);
    }
    if (feeder == NULL) {
        success = FALSE;
        goto DECODE_END;
    }
    VLOG(ERR, "[main] has created bitstreamfeeder for %s\n", param->inputPath);
    /* set up decoder configurations */
    decOP.bitstreamFormat     = (CodStd)param->bitFormat;
    decOP.avcExtension        = param->coda9.enableMvc;
    decOP.coreIdx             = coreIdx;
    decOP.bitstreamBuffer     = vbStream.phys_addr;
    decOP.bitstreamBufferSize = vbStream.size;
    decOP.bitstreamMode       = param->bitstreamMode;
    decOP.tiled2LinearEnable  = param->coda9.enableTiled2Linear;
    decOP.tiled2LinearMode    = param->coda9.tiled2LinearMode;
    decOP.wtlEnable           = param->enableWTL;
    decOP.wtlMode             = param->wtlMode;
    decOP.cbcrInterleave      = param->cbcrInterleave;
    decOP.nv21                = param->nv21;
    decOP.streamEndian        = param->streamEndian;
    decOP.frameEndian         = param->frameEndian;
    decOP.bwbEnable           = param->coda9.enableBWB;
    decOP.mp4DeblkEnable      = param->coda9.enableDeblock;
    decOP.mp4Class            = param->coda9.mp4class;

    VLOG(INFO, "------------------------------ DECODER OPTIONS ------------------------------\n");
    VLOG(INFO, "[coreIdx            ]: %d\n", decOP.coreIdx            );
    VLOG(INFO, "[mapType            ]: %d\n", param->mapType           );
    VLOG(INFO, "[tiled2linear       ]: %d\n", param->coda9.enableTiled2Linear);
    VLOG(INFO, "[wtlEnable          ]: %d\n", decOP.wtlEnable          );
    VLOG(INFO, "[wtlMode            ]: %d\n", decOP.wtlMode            );
    VLOG(INFO, "[bitstreamBuffer    ]: 0x%08x\n", decOP.bitstreamBuffer);
    VLOG(INFO, "[bitstreamBufferSize]: %d\n", decOP.bitstreamBufferSize);
    VLOG(INFO, "[bitstreamMode      ]: %d\n", decOP.bitstreamMode      );
    VLOG(INFO, "[cbcrInterleave     ]: %d\n", decOP.cbcrInterleave     );
    VLOG(INFO, "[nv21               ]: %d\n", decOP.nv21               );
    VLOG(INFO, "[streamEndian       ]: %d\n", decOP.streamEndian       );
    VLOG(INFO, "[frameEndian        ]: %d\n", decOP.frameEndian        );
    VLOG(INFO, "[BWB                ]: %d\n", decOP.bwbEnable          );
    VLOG(INFO, "[loopCount          ]: %d\n", loopCount                );
    VLOG(WARN, "-----------------------------------------------------------------------------\n");

    /********************************************************************************
     * CREATE INSTANCE                                                              *
     ********************************************************************************/
    if ((ret=VPU_DecOpen(&handle, &decOP)) != RETCODE_SUCCESS) {
        success = FALSE;
        VLOG(ERR, "VPU_DecOpen failed Error code is 0x%x \n", ret );
        goto DECODE_END;
    }
	VLOG(ERR, "open success\n ");

    osal_memset((void*)&decReportCfg, 0x00, sizeof(VpuReportConfig_t));
    decReportCfg.userDataEnable     = param->enableUserData;
    decReportCfg.userDataReportMode = 1;
    OpenDecReport(coreIdx, &decReportCfg);
    ConfigDecReport(coreIdx, handle, decOP.bitstreamFormat);

    //VPU_DecGiveCommand(handle, ENABLE_LOGGING, 0);
    if ((renderer=SimpleRenderer_Create(handle, rendererType, param->outputPath)) == NULL) {
        success = FALSE;
        goto DECODE_END;
    }

    VLOG(ERR, "[main] feeder first\n");
    if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian)) < 0) {
        success = FALSE;
        goto DECODE_END;
    }

    if (decOP.bitstreamMode == BS_MODE_PIC_END) {
        if (decOP.bitstreamFormat != STD_THO && decOP.bitstreamFormat != STD_H263 && decOP.bitstreamFormat != STD_RV) {
            // Need first picture. In case of Theora, ffmpeg returns a coded frame including sequence header.
            BitstreamFeeder_Act(handle, feeder, decOP.streamEndian);
        }
    }
	VLOG(ERR, "Create bitstreamfeer success\n");

    /********************************************************************************
     * INIT_SEQ                                                                     *
     ********************************************************************************/
INIT_SEQ:

    seqInitEscape = FALSE;
    ret = VPU_DecSetEscSeqInit(handle, seqInitEscape);
    if (ret != RETCODE_SUCCESS) {
        seqInitEscape = 0;
        VLOG(WARN, "Wanning! can not to set seqInitEscape in the current bitstream mode Option \n");
    }
	VLOG(ERR, "VPU_DecSetEscSeqIni succeess\n");

    if(seqInitEscape) {
        // if you set to seqInitEscape option to TRUE, It is more easy to control that APP uses VPU_DecGetInitialInfo instead of VPU_DecIssueSeqInit and VPU_DecCompleteSeqInit
        ret = VPU_DecGetInitialInfo(handle, &sequenceInfo);
        if (ret != RETCODE_SUCCESS)
        {
            VLOG(ERR, "VPU_DecGetInitialInfo failed Error code is 0x%x \n", ret);
            VLOG(ERR, "Error reason is 0x%x\n", sequenceInfo.seqInitErrReason);
            success = FALSE;
            goto DECODE_END;
        }
		VLOG(ERR, "VPU_DecGetOutputInfo is ok \n");
    }
    else
    {
        // Scan and retrieve sequence header information from the bitstream buffer.
        if ((ret=VPU_DecIssueSeqInit(handle)) != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_DecIssueSeqInit failed Error code is 0x%x \n", ret);
            success = FALSE;
            goto DECODE_END;
        }
        timeoutCount = 0;
        while (seqInited == FALSE) {
			VLOG(ERR, "VPU_DecIssueSeqInit is ok \n");
            interruptFlag = VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT);		//wait for 10ms to save stream filling time.
            VLOG(ERR, "VPU_DecIssueSeqInit intflag %x\n", interruptFlag);
            if (interruptFlag == -1) {
                if (timeoutCount*VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) {
                    VLOG(ERR, "\n VPU interrupt wait timeout\n");
                    VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                    success = FALSE;
                    goto DECODE_END;
                }
                VLOG(ERR, "VPU_DecIssueSeqInit timeout %d\n", timeoutCount);
                timeoutCount++;
                interruptFlag = 0;
            }

            if (interruptFlag) {
                VPU_ClearInterrupt(coreIdx);
                if (interruptFlag & (1<<INT_BIT_SEQ_INIT)) {
                    VLOG(ERR, "VPU_ClearInterrupt\n");
                    seqInited = TRUE;
                    break;
                }
            }

            if (decOP.bitstreamMode != BS_MODE_PIC_END) {
                VLOG(ERR, "refeeder when not BS_MODE_PIC_END\n");
                if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian)) < 0) {
                    success = FALSE;
                    goto DECODE_END;
                }
            }
        }
		VLOG(ERR, "start Decomplete SeqInit  succeess\n");

        if ((ret=VPU_DecCompleteSeqInit(handle, &sequenceInfo)) != RETCODE_SUCCESS) {
                VLOG(ERR, "[ERROR] Failed to SEQ_INIT(ERROR REASON: %d(0x%x)\n", sequenceInfo.seqInitErrReason, sequenceInfo.seqInitErrReason);
            success = FALSE;
            goto DECODE_END;
        }
        if (seqInited == FALSE) {
            success = FALSE;
            goto DECODE_END;
        }
    }

	VLOG(ERR, "start  VPU_DecCompleteSeqInit is ok \n");

    PrintDecSeqWarningMessages(productId, &sequenceInfo);

#if 0
    if (comparator == NULL) {
        switch (param->compareType) {
        case NO_COMPARE:
            goldenPath = NULL;
            comparator = Comparator_Create(param->compareType, goldenPath);
            break;
        case YUV_COMPARE:
            goldenPath = param->refYuvPath;
            comparator = Comparator_Create(param->compareType, goldenPath,
                                           VPU_ALIGN16(sequenceInfo.picWidth), VPU_ALIGN16(sequenceInfo.picHeight),
                                           param->wtlFormat, param->cbcrInterleave);
            break;
        default:
            success = FALSE;
            goto DECODE_END;
        }
    }

    if (comparator == NULL) {
        success = FALSE;
        goto DECODE_END;
    }
#endif

    /********************************************************************************
     * ALLOCATE RECON FRAMEBUFFERS                                                  *
     ********************************************************************************/
    framebufHeight = VPU_ALIGN32(sequenceInfo.picHeight);
    framebufStride = CalcStride(sequenceInfo.picWidth, sequenceInfo.picHeight, FORMAT_420, decOP.cbcrInterleave, param->mapType, FALSE);
    framebufSize   = VPU_GetFrameBufSize(coreIdx, framebufStride, framebufHeight, param->mapType, FORMAT_420, decOP.cbcrInterleave, NULL);
    fbCount        = sequenceInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;
    VLOG(ERR, "[main] minFrameBufferCount %d, stride %d, height %d, maptype %d\n", sequenceInfo.minFrameBufferCount,
        framebufStride, framebufHeight, param->mapType);
    osal_memset((void*)&fbAllocInfo, 0x00, sizeof(fbAllocInfo));
    osal_memset((void*)pFrame, 0x00, sizeof(FrameBuffer)*fbCount);
    fbAllocInfo.format          = FORMAT_420;
    fbAllocInfo.cbcrInterleave  = decOP.cbcrInterleave;
    fbAllocInfo.mapType         = param->mapType;
    fbAllocInfo.stride          = framebufStride;
    fbAllocInfo.height          = framebufHeight;
    fbAllocInfo.lumaBitDepth    = sequenceInfo.lumaBitdepth;
    fbAllocInfo.chromaBitDepth  = sequenceInfo.chromaBitdepth;
    fbAllocInfo.num             = fbCount;
    fbAllocInfo.endian          = decOP.frameEndian;
    fbAllocInfo.type            = FB_TYPE_CODEC;
	VLOG(ERR, "step 1.0 framebuffer count (%d)\n", fbCount);
    for (index=0; index<fbCount; index++) {
        pvb = &pFbMem[index];
        pvb->size = framebufSize;
        if (vdi_allocate_dma_memory(coreIdx, pvb) < 0) {
            success=FALSE;
            VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
            goto DECODE_END;
        }
        pFrame[index].bufY  = pvb->phys_addr;
        pFrame[index].bufCb = -1;
        pFrame[index].bufCr = -1;
        pFrame[index].updateFbInfo = TRUE;
    }

	VLOG(ERR, "step 1.1 alloc framebuffer success \n");

    if ((ret=VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, pFrame)) != RETCODE_SUCCESS) {
        VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer(), ret(%d)\n", __FUNCTION__, __LINE__, ret);
        goto DECODE_END;
    }
    /********************************************************************************
     * ALLOCATE WTL FRAMEBUFFERS                                                    *
     ********************************************************************************/
	VLOG(ERR, "three )\n");

    if (decOP.wtlEnable == TRUE) {
        TiledMapType linearMapType = decOP.wtlMode == FF_FRAME ? LINEAR_FRAME_MAP : LINEAR_FIELD_MAP;
        uint32_t     strideWtl;

        strideWtl    = CalcStride(sequenceInfo.picWidth, sequenceInfo.picHeight, FORMAT_420, decOP.cbcrInterleave, linearMapType, FALSE);
        framebufSize = VPU_GetFrameBufSize(coreIdx, strideWtl, framebufHeight, linearMapType, FORMAT_420, decOP.cbcrInterleave, NULL);

		VLOG(ERR, "FOUR framebufSize(%d\n)", framebufSize);
        for (index=fbCount; index<fbCount*2; index++) {
            pvb       = &pFbMem[index];
            pvb->size = framebufSize;
            if (vdi_allocate_dma_memory(coreIdx, pvb) < 0) {
                success=FALSE;
                VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                goto DECODE_END;
            }
            pFrame[index].bufY  = pvb->phys_addr;
            pFrame[index].bufCb = -1;
            pFrame[index].bufCr = -1;
            pFrame[index].updateFbInfo = TRUE;
        }
        fbAllocInfo.mapType        = linearMapType;
        fbAllocInfo.cbcrInterleave = decOP.cbcrInterleave;
        fbAllocInfo.nv21           = decOP.nv21;
        fbAllocInfo.format         = FORMAT_420;
        fbAllocInfo.stride         = strideWtl;
        fbAllocInfo.height         = framebufHeight;
        fbAllocInfo.endian         = decOP.frameEndian;
        fbAllocInfo.lumaBitDepth   = 8;
        fbAllocInfo.chromaBitDepth = 8;
        fbAllocInfo.num            = fbCount;
        fbAllocInfo.type           = FB_TYPE_CODEC;
        ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, &pFrame[fbCount]);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer() ret:%d\n", __FUNCTION__, __LINE__, ret);
            goto DECODE_END;
        }
		VLOG(ERR, "OK, fbCount(%d)\n", fbCount);
    }

    /********************************************************************************
     * SET_FRAMEBUF                                                                 *
     ********************************************************************************/
    /*set sram buffer for sec AXI  */
    decParam.sramMode = param->sramMode;
    secAxiUse.u.coda9.useBitEnable = (param->secondaryAXI >> 0) & 0x01;
    secAxiUse.u.coda9.useIpEnable = (param->secondaryAXI >> 1) & 0x01;
    secAxiUse.u.coda9.useDbkYEnable = (param->secondaryAXI >> 2) & 0x01;
    secAxiUse.u.coda9.useDbkCEnable = (param->secondaryAXI >> 3) & 0x01;
    secAxiUse.u.coda9.useOvlEnable = (param->secondaryAXI >> 4) & 0x01;
    secAxiUse.u.coda9.useBtpEnable = (param->secondaryAXI >> 5) & 0x01;
    VPU_DecGiveCommand(handle, SET_SEC_AXI, &secAxiUse);
    VLOG(INFO, "[VPU] useBitEnable \n", secAxiUse.u.coda9.useBitEnable);
    VLOG(INFO, "[VPU] useIpEnable: %d\n", secAxiUse.u.coda9.useIpEnable);
    VLOG(INFO, "[VPU] useDbkYEnable: %d\n", secAxiUse.u.coda9.useDbkYEnable);
    VLOG(INFO, "[VPU] useDbkCEnable: %d\n", secAxiUse.u.coda9.useDbkCEnable);
    VLOG(INFO, "[VPU] useOvlEnable: %d\n", secAxiUse.u.coda9.useOvlEnable);
    VLOG(INFO, "[VPU] useBtpEnable: %d\n", secAxiUse.u.coda9.useBtpEnable);

    MaverickCache2Config(&cacheCfg, TRUE/*Decoder*/, param->cbcrInterleave,
                         param->coda9.frameCacheBypass,
                         param->coda9.frameCacheBurst,
                         param->coda9.frameCacheMerge,
                         param->mapType,
                         param->coda9.frameCacheWayShape);
    VPU_DecGiveCommand(handle, SET_CACHE_CONFIG, &cacheCfg);

	VLOG(ERR, "FIMVE\n");
    framebufStride = CalcStride(sequenceInfo.picWidth, sequenceInfo.picHeight, FORMAT_420, decOP.cbcrInterleave,
                                decOP.wtlEnable == TRUE ? LINEAR_FRAME_MAP : param->mapType, FALSE);
	VLOG(ERR, "framebufStride(%d)\n", framebufStride);
    ret = VPU_DecRegisterFrameBuffer(handle, pFrame, fbCount, framebufStride, framebufHeight, (int)param->mapType);
	VLOG(ERR, "SIX\n");
    if (ret != RETCODE_SUCCESS) {
        success = FALSE;
        if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
            PrintMemoryAccessViolationReason(coreIdx, NULL);
        VLOG(ERR, "VPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        goto DECODE_END;
    }
    /********************************************************************************
     * ALLOCATE PPU FRAMEBUFFERS (When rotator, mirror or tiled2linear are enabled) *
     * NOTE: VPU_DecAllocateFrameBuffer() WITH PPU FRAMEBUFFER SHOULD BE CALLED     *
     *       AFTER VPU_DecRegisterFrameBuffer().                                    *
     ********************************************************************************/
    enablePPU = (BOOL)(param->coda9.rotate > 0 || param->coda9.mirror > 0 ||
                       decOP.tiled2LinearEnable == TRUE || param->coda9.enableDering == TRUE);
    if (enablePPU == TRUE) {
        uint32_t stridePpu;
        uint32_t sizePPUFb;
        uint32_t rotate       = param->coda9.rotate;
        uint32_t rotateWidth  = sequenceInfo.picWidth;
        uint32_t rotateHeight = sequenceInfo.picHeight;

        if (rotate == 90 || rotate == 270) {
            rotateWidth  = sequenceInfo.picHeight;
            rotateHeight = sequenceInfo.picWidth;
        }
        rotateWidth  = VPU_ALIGN32(rotateWidth);
        rotateHeight = VPU_ALIGN32(rotateHeight);

        stridePpu = CalcStride(rotateWidth, rotateHeight, FORMAT_420, decOP.cbcrInterleave, LINEAR_FRAME_MAP, FALSE);
        sizePPUFb = VPU_GetFrameBufSize(coreIdx, stridePpu, rotateHeight, LINEAR_FRAME_MAP, FORMAT_420, decOP.cbcrInterleave, NULL);
		VLOG(ERR, "seven\n");


        for (index=0; index<ppuFbCount; index++) {
            pvb = &pPPUFbMem[index];
            pvb->size = sizePPUFb;
            if (vdi_allocate_dma_memory(coreIdx, pvb) < 0) {
                success=FALSE;
                VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                goto DECODE_END;
            }
            pPPUFrame[index].bufY  = pvb->phys_addr;
            pPPUFrame[index].bufCb = -1;
            pPPUFrame[index].bufCr = -1;
            pPPUFrame[index].updateFbInfo = TRUE;
        }

        fbAllocInfo.mapType        = LINEAR_FRAME_MAP;
        fbAllocInfo.cbcrInterleave = decOP.cbcrInterleave;
        fbAllocInfo.nv21           = decOP.nv21;
        fbAllocInfo.format         = FORMAT_420;
        fbAllocInfo.stride         = stridePpu;
        fbAllocInfo.height         = rotateHeight;
        fbAllocInfo.endian         = decOP.frameEndian;
        fbAllocInfo.lumaBitDepth   = 8;
        fbAllocInfo.chromaBitDepth = 8;
        fbAllocInfo.num            = ppuFbCount;
        fbAllocInfo.type           = FB_TYPE_PPU;
        if ((ret=VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, pPPUFrame)) != RETCODE_SUCCESS) {
            VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer() ret:%d\n", __FUNCTION__, __LINE__, ret);
            goto DECODE_END;
        }
        // Note: Please keep the below call sequence.
		VLOG(ERR, "eight\n");
        VPU_DecGiveCommand(handle, SET_ROTATION_ANGLE, (void*)&param->coda9.rotate);
        VPU_DecGiveCommand(handle, SET_MIRROR_DIRECTION, (void*)&param->coda9.mirror);
        VPU_DecGiveCommand(handle, SET_ROTATOR_STRIDE, (void*)&stridePpu);

        if ((ppuQ=Queue_Create(MAX_REG_FRAME, sizeof(FrameBuffer))) == NULL) {
            success = FALSE;
            goto DECODE_END;
        }
        for (index=0; index<ppuFbCount; index++) {
            Queue_Enqueue(ppuQ, (void*)&pPPUFrame[index]);
        }
    }

    PrepareDecoderTest(handle);

    doDumpImage         = (BOOL)(param->compareType == YUV_COMPARE || (strlen(param->outputPath) > 0) || rendererType == RENDER_DEVICE_FBDEV);
    waitPostProcessing  = enablePPU;
    needStream          = FALSE;
    prevFbIndex         = -1;

    /********************************************************************************
     * PIC_RUN                                                                      *
     ********************************************************************************/
    DisplayDecodedInformation(handle, decOP.bitstreamFormat, 0, NULL);
    osal_memset((void*)&decParam, 0x00, sizeof(DecParam));
    while (param->exitFlag == 0) {
        while ((pDisplayInfo=(DecOutputInfo*)SimpleRenderer_GetFreeFrameInfo(renderer)) != NULL) {
            if (enablePPU == TRUE) {
                Queue_Enqueue(ppuQ, (void*)&pDisplayInfo->dispFrame);
            }
            else {
                VPU_DecClrDispFlag(handle, pDisplayInfo->dispFrame.myIndex);
            }
        }

        if (needStream == TRUE) {
            VPU_DecSetRdPtr(handle, decOP.bitstreamBuffer, TRUE);
            if (assertedFieldDoneInt == FALSE) {
                EnterLock(coreIdx);
                nWritten = BitstreamFeeder_Act(handle, feeder, decOP.streamEndian);
                LeaveLock(coreIdx);
            }
            else {
                nWritten = BitstreamFeeder_Act(handle, feeder, decOP.streamEndian);
            }
            if (nWritten < 0) {
                success = FALSE;
                goto DECODE_END;
            }
            needStream = FALSE;
        }

        if (assertedFieldDoneInt == TRUE) {
            VPU_ClearInterrupt(coreIdx);
            assertedFieldDoneInt = FALSE;
        }
        else {
            // When the field done interrupt is asserted, just fill elementary stream into the bitstream buffer.
            if (enablePPU == TRUE) {
                if ((ppuFb=(FrameBuffer*)Queue_Dequeue(ppuQ)) == NULL) {
                    MSleep(0);
                    needStream = FALSE;
                    continue;
                }
                VPU_DecGiveCommand(handle, SET_ROTATOR_OUTPUT, (void*)ppuFb);
                if (param->coda9.rotate > 0) {
                    VPU_DecGiveCommand(handle, ENABLE_ROTATION, NULL);
                }
                if (param->coda9.mirror > 0) {
                    VPU_DecGiveCommand(handle, ENABLE_MIRRORING, NULL);
                }
                if (param->coda9.enableDering == TRUE) {
                    VPU_DecGiveCommand(handle, ENABLE_DERING, NULL);
                }
            }

            startUs = GetNowUs();
            // Start decoding a frame.
            if ((ret=VPU_DecStartOneFrame(handle, &decParam)) != RETCODE_SUCCESS) {
                success = FALSE;
                VLOG(ERR, "VPU_DecStartOneFrame failed Error code is 0x%x \n", ret );
                goto DECODE_END;
            }
        }

        timeoutCount        = 0;
        repeat              = TRUE;
        while (repeat == TRUE) {
            if ((interruptFlag=VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT)) == -1) {
                //wait for 10ms to save stream filling time.
                if (timeoutCount*VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) {
                    PrintVpuStatus(coreIdx, productId);
                    VLOG(WARN, "\n VPU interrupt wait timeout instIdx=%d\n", instIdx);
                    VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                    success = FALSE;
                    LeaveLock(coreIdx);
                    goto DECODE_END;
                }
                timeoutCount++;
                interruptFlag = 0;
            }

            CheckUserDataInterrupt(coreIdx, handle, outputInfo.indexFrameDecoded, decOP.bitstreamFormat, interruptFlag);

            if (interruptFlag & (1<<INT_BIT_PIC_RUN)) {
                repeat = FALSE;
            }
            if (interruptFlag & (1<<INT_BIT_DEC_FIELD)) {
                if (decOP.bitstreamMode == BS_MODE_PIC_END) {
                    // do not clear interrupt until feeding next field picture.
                    assertedFieldDoneInt = TRUE;
                    break;
                }
            }
            if (interruptFlag > 0) {
                VLOG(ERR, "[main] VPU_ClearInterrupt line %d\n", __LINE__);
                VPU_ClearInterrupt(coreIdx);
            }
            if (repeat == FALSE) break;
            // In PICEND mode, the below codes are not called.
            if (decOP.bitstreamMode == BS_MODE_INTERRUPT && seqChangeRequest == FALSE) {
                if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian)) < 0) {
                    success = FALSE;
                    LeaveLock(coreIdx);
                    goto DECODE_END;
                }
            }
        }

        if (assertedFieldDoneInt == TRUE) {
            needStream = TRUE;
            continue;
        }

        ret = VPU_DecGetOutputInfo(handle, &outputInfo);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_DecGetOutputInfo failed Error code is 0x%x , instIdx=%d\n", ret, instIdx);
            if (ret == RETCODE_MEMORY_ACCESS_VIOLATION) {
                PrintVpuStatus(coreIdx, productId);
                PrintMemoryAccessViolationReason(coreIdx, &outputInfo);
            }
            if (noResponseCount == 0) {
                break;
            }
            continue;
        }

        if (outputInfo.decodingSuccess) {
            diffUs = GetNowUs() - startUs;
            totalUs += diffUs;
            if (outputInfo.picType == PIC_TYPE_I
                || outputInfo.picType == PIC_TYPE_P
                || outputInfo.picType == PIC_TYPE_B
                || outputInfo.picType == PIC_TYPE_IDR) {
                VLOG(TRACE, "decoding time=%.1fms\n", (double)diffUs/1000);
            }
        }

        if (outputInfo.indexFrameDecoded == DECODED_IDX_FLAG_NO_FB &&
            outputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_NO_FB) {
            noFbCount++;
        }
        else {
            noFbCount = 0;
        }
        noResponseCount = MAX_COUNT_NO_RESPONSE;

        DisplayDecodedInformation(handle, decOP.bitstreamFormat, decodedIdx, &outputInfo);

        if (outputInfo.sequenceChanged == TRUE) {
            seqChangeRequest = TRUE;
            seqChangedRdPtr  = outputInfo.rdPtr;
            seqChangedWrPtr  = outputInfo.wrPtr;
            VLOG(INFO, "seqChangeRdPtr: 0x%08x, WrPtr: 0x%08x\n", seqChangedRdPtr, seqChangedWrPtr);
            if ((ret=VPU_DecSetRdPtr(handle, seqChangedRdPtr, TRUE)) != RETCODE_SUCCESS) {
                VLOG(ERR, "%s:%d Failed to VPU_DecSetRdPtr(%d), ret(%d)\n", __FUNCTION__, __LINE__, seqChangedRdPtr, ret);
                success = FALSE;
                break;
            }
            seqChangedStreamEndFlag = outputInfo.streamEndFlag;
            VPU_DecUpdateBitstreamBuffer(handle, 1); // let f/w to know stream end condition in bitstream buffer. force to know that bitstream buffer will be empty.
            VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SET_FLAG); // set to stream end condition to pump out a delayed framebuffer.
            VLOG(INFO, "---> SEQUENCE CHANGED <---\n");
        }

        if (outputInfo.indexFrameDecoded >= 0)
            decodedIdx++;



        if (enablePPU == TRUE) {
            if (prevFbIndex >= 0) {
                VPU_DecClrDispFlag(handle, prevFbIndex);
            }
            prevFbIndex = outputInfo.indexFrameDisplay;

            if (waitPostProcessing == TRUE) {
                if (outputInfo.indexFrameDisplay >= 0) {
                    waitPostProcessing = FALSE;
                }
                rcPpu = outputInfo.rcDisplay;
                /* Release framebuffer for PPU */
                Queue_Enqueue(ppuQ, (void*)ppuFb);
                needStream = (decOP.bitstreamMode == BS_MODE_PIC_END);
                if (outputInfo.chunkReuseRequired == TRUE) {
                    needStream = FALSE;
                }
                // Not ready for ppu buffer.
                continue;
            }
            else {
                if (outputInfo.indexFrameDisplay < 0) {
                    waitPostProcessing = TRUE;
                }
            }
        }

        if (outputInfo.indexFrameDisplay >= 0 || enablePPU == TRUE) {
            uint32_t    width=0, height=0, Bpp;
            size_t      frameSizeInByte;
            uint8_t*    pYuv = NULL;
            void*       decodedData = NULL;
            uint32_t    decodedDataSize = 0;
            VpuRect     rc = enablePPU == TRUE ? rcPpu : outputInfo.rcDisplay;

            if (doDumpImage == TRUE) {
                pYuv = GetYUVFromFrameBuffer(handle, &outputInfo.dispFrame, rc, &width, &height, &Bpp, &frameSizeInByte);

                if (strlen(param->outputPath) > 0) {
                    if (saveFp == NULL) {
                        if((saveFp = osal_fopen(param->outputPath,"wb")) == NULL)
                            goto DECODE_END;
                    }

                    osal_fwrite (pYuv, frameSizeInByte, 1, saveFp);
                }
            }

            switch (param->compareType) {
            case NO_COMPARE:
                break;
            case YUV_COMPARE:
                decodedData     = (void*)pYuv;
                decodedDataSize = frameSizeInByte;
                break;
            }

            // if ((success=Comparator_Act(comparator, decodedData, decodedDataSize)) == FALSE) {
            //     goto DECODE_END;
            // }
            /*
             * pYuv is released at the renderer module.
             * SimpleRenderer releases all framebuffer memory of the previous sequence.
             */
            SimpleRenderer_Act(renderer, &outputInfo, pYuv, width, height);
            dispIdx++;
        }

        if (dispIdx > 0 && dispIdx == param->forceOutNum) {
            break;
        }

        if (outputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END) {
            if (seqChangeRequest == TRUE) {
                seqChangeRequest = FALSE;
                VPU_DecSetRdPtr(handle, seqChangedRdPtr, TRUE);
                if (seqChangedStreamEndFlag == 1) {
                    VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SET_FLAG);
                }
                else {
                    VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_CLEAR_FLAG);
                }
                if (seqChangedWrPtr >= seqChangedRdPtr) {
                    VPU_DecUpdateBitstreamBuffer(handle, seqChangedWrPtr-seqChangedRdPtr);
                }
                else {
                    VPU_DecUpdateBitstreamBuffer(handle, (decOP.bitstreamBuffer+decOP.bitstreamBufferSize)-seqChangedRdPtr +
                                                         (seqChangedWrPtr-decOP.bitstreamBuffer));
                }

                // Flush renderer: waiting all picture displayed.
                SimpleRenderer_Flush(renderer);

                // Release all memory related to framebuffer.
                VPU_DecGiveCommand(handle, DEC_FREE_FRAME_BUFFER, 0x00);
                for (index=0; index<MAX_REG_FRAME; index++) {
                    if (pFbMem[index].size > 0) {
                        vdi_free_dma_memory(coreIdx, &pFbMem[index]);
                    }
                    if (pPPUFbMem[index].size > 0) {
                        vdi_free_dma_memory(coreIdx, &pPPUFbMem[index]);
                    }
                }
                seqInited = FALSE;
                goto INIT_SEQ;
            }

            if (loopCount > 0) {
                VLOG(INFO,"loopCount %d\n",loopCount);
                loopCount--;
                BitstreamFeeder_Rewind(feeder);
                Comparator_Rewind(comparator);
                VPU_DecUpdateBitstreamBuffer(handle, -1);

                /*loop test should release display flag */
                DecOutputInfo   remainings[16];   /* max remainings is 16 */
                uint32_t        num, i;

                /* clear all of display indexes that FW owns. */
                VPU_DecFrameBufferFlush(handle, remainings, &num);
                VLOG(INFO,"vpu flush buffer num %d\n",num);

                for (i=0; i < num; i++) {
                    VPU_DecClrDispFlag(handle, remainings[i].indexFrameDisplay);
                }

                /* clear all of display indexes that HOST owns. */
                SimpleRenderer_Flush(renderer);

                /* only save yuv file last loop time */
                if (saveFp != NULL) {
                    osal_fclose(saveFp);
                    saveFp = NULL;
                }
            }
            else {
                break;
            }
        }

        if (decOP.bitstreamMode == BS_MODE_PIC_END) {
            if (outputInfo.indexFrameDecoded == DECODED_IDX_FLAG_NO_FB) {
                needStream = FALSE;
            }
            else {
                needStream = TRUE;
            }
        }

        if (outputInfo.chunkReuseRequired == TRUE) {
            needStream = FALSE;
        }
        SaveDecReport(coreIdx, handle, &outputInfo, decOP.bitstreamFormat,
                      ((sequenceInfo.picWidth+15)&~15)/16,
                      ((sequenceInfo.picHeight+15)&~15)/16);
    }

DECODE_END:
    CloseDecReport(coreIdx);
    // Now that we are done with decoding, close the open instance.
    VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SIZE);

    // if (param->compareType && outputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END && param->forceOutNum == 0) {
    //     if (success == TRUE) {
    //         success = Comparator_CheckFrameCount(comparator);
    //     }
    // }

    /********************************************************************************
     * DESTROY INSTANCE                                                             *
     ********************************************************************************/
    VPU_DecClose(handle);

    if (feeder != NULL)     BitstreamFeeder_Destroy(feeder);
    if (renderer != NULL)   SimpleRenderer_Destroy(renderer);
    if (comparator != NULL) Comparator_Destroy(comparator);
    if (ppuQ != NULL)       Queue_Destroy(ppuQ);

    for (index=0; index<MAX_REG_FRAME; index++) {
        if (pFbMem[index].size > 0)
            vdi_free_dma_memory(coreIdx, &pFbMem[index]);
        if (pPPUFbMem[index].size > 0)
            vdi_free_dma_memory(coreIdx, &pPPUFbMem[index]);
    }

    double totalsec = totalUs / 1E6;
    VLOG(INFO, "\nDec End. Tot Frame %d, decNum %d, time consumed %lld us(%.2f sec), %.2f/%.2f\n",
        dispIdx, decodedIdx, totalUs, totalsec, (double)dispIdx/totalsec, (double)decodedIdx/totalsec);

ERR_DEC_INIT:
    if (vbStream.size > 0) {
        vdi_free_dma_memory(coreIdx, &vbStream);
    }

    if (pusBitCode != NULL) {
        osal_free(pusBitCode);
        pusBitCode = NULL;
    }

    if (saveFp != NULL) {
        osal_fclose(saveFp);
        saveFp = NULL;
    }

    VPU_DeInit(coreIdx);
    param->exitFlag = THREAD_EXIT_SUCCESS;
    VLOG(INFO, "TestDecoderCoda exit now ...\n");
    return (success == TRUE && ret == RETCODE_SUCCESS);
}
