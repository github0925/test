//------------------------------------------------------------------------------
//
// Copyright (C) 2006, Chips & Media.  All rights reserved.
// Copyright (C) 2020, Semidrive Technology Ltd. All rights reserved.
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "main_helper.h"

#define STREAM_BUF_SIZE_HEVC                0xA00000    // max bitstream size(HEVC:10MB,VP9:not specified)
#define STREAM_BUF_SIZE_VP9                 0x1400000    // max bitstream size(HEVC:10MB,VP9:not specified)
#define USERDATA_BUFFER_SIZE                (512*1024)
#define EXTRA_FRAME_BUFFER_NUM              1
#define MAX_SEQUENCE_MEM_COUNT              16
#define MAX_NOT_DEC_COUNT                   200

#define DIFF_ABS(x,y) ((x-y) > 0 ? (x-y) : (y-x))

typedef struct {
    DecGetFramebufInfo  fbInfo;
    vpu_buffer_t        allocFbMem[MAX_REG_FRAME];
} SequenceMemInfo;


static void ReleasePreviousSequenceResources(
    DecHandle           handle,
    vpu_buffer_t*       arrFbMem,
    DecGetFramebufInfo* prevSeqFbInfo
    )
{
    uint32_t    i;
    uint32_t    coreIndex;

    if (handle == NULL) {
        return;
    }

    coreIndex = VPU_HANDLE_CORE_INDEX(handle);

    for (i=0;i<MAX_REG_FRAME; i++) {
        if (arrFbMem[i].size > 0)
            vdi_free_dma_memory(coreIndex, &arrFbMem[i]);
    }
    for ( i=0 ; i<MAX_REG_FRAME; i++) {
        if (prevSeqFbInfo->vbFbcYTbl[i].size > 0) vdi_free_dma_memory(coreIndex, &prevSeqFbInfo->vbFbcYTbl[i]);
        if (prevSeqFbInfo->vbFbcCTbl[i].size > 0) vdi_free_dma_memory(coreIndex, &prevSeqFbInfo->vbFbcCTbl[i]);
        if (prevSeqFbInfo->vbMvCol[i].size > 0)   vdi_free_dma_memory(coreIndex, &prevSeqFbInfo->vbMvCol[i]);
    }
}

BOOL TestDecoderWave(TestDecConfig *param)
{
    DecHandle           handle = NULL;
    DecOpenParam        decOP;
    DecInitialInfo      initialInfo;
    DecOutputInfo       outputInfo, *pDisplayInfo;
    DecParam            decParam;
    vpu_buffer_t        vbStream[2];
    vpu_buffer_t        vbUserData;
    SecAxiUse           secAxiUse;
    RetCode             ret = RETCODE_SUCCESS;
    uint32_t            framebufStride=0;
    BOOL                success = TRUE;
    BOOL                doDumpImage = FALSE;
    int32_t             kbhitRet = 0;
    int32_t             compressedFbCount, linearFbCount;
    int32_t             frameIdx = 0;
    int32_t             outputCount=0;
    uint32_t            dispIdx = 0;
    uint32_t            index, val;
    BOOL                seqInited = FALSE;
    BOOL                seqInitEscape       = FALSE;
    int32_t             timeoutCount = 0;
    int32_t             interruptFlag = 0;
    int32_t             coreIdx;
    int32_t             instIdx;
    FrameBuffer         pFrame[MAX_REG_FRAME];
    vpu_buffer_t        pFbMem[MAX_REG_FRAME];
    ProductId           productId;
    int32_t             nWritten;
    BSFeeder            feeder              = NULL;
    Renderer            renderer            = NULL;
    RenderDeviceType    rendererType        = param->renderType;
    FILE*               fpOutput[OUTPUT_FP_NUMBER];
    Comparator          comparator          = NULL;
    Queue*              displayQ            = NULL;
    Queue*              sequenceQ           = NULL;
    uint32_t            bsBufferCount       = 1;            //!<< In PICEND mode, this variable is greater than 1.
    uint32_t            bsQueueIndex        = 0;
    BOOL                needStream          = FALSE;        //!<< This variable is used on PICEND mode.
    BOOL                loop                = TRUE;
    BOOL                repeat              = TRUE;
    SequenceMemInfo     seqMemInfo[MAX_SEQUENCE_MEM_COUNT];
    int32_t             decodedIndex, prescanIndex;
    uint32_t            sequenceChangeFlag;
    uint32_t            sequenceChangeOccurred=0;
    uint32_t            notDecodedCount     = 0;
    uint32_t            noFbCount           = 0;
    uint32_t            compWidth           = 0;
    uint32_t            compHeight          = 0;
    Uint8*              pBase               = NULL;
    char*               fwPath              = NULL;
    Uint32              intBitInitSeq, intBitDecPic, intBitEmpty;
    Uint32              loopCount;
    FILE*               filep[1] = {NULL};
    int64_t             startUs             = 0;
    int64_t             diffUs              = 0;
    int64_t             totalUs             = 0;
    uint32_t            sizeInWord          = 0;
    uint16_t*           pusBitCode          = NULL;

    osal_memset(&decOP,         0x00, sizeof(DecOpenParam));
    osal_memset(&initialInfo,   0x00, sizeof(DecInitialInfo));
    osal_memset(&outputInfo,    0x00, sizeof(DecOutputInfo));
    osal_memset(&decParam,      0x00, sizeof(DecParam));
    osal_memset(vbStream,       0x00, sizeof(vbStream));
    osal_memset(&secAxiUse,     0x00, sizeof(SecAxiUse));
    osal_memset(pFbMem,         0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);
    osal_memset(seqMemInfo,     0x00, sizeof(seqMemInfo));
    osal_memset(&fpOutput[0],   0x00, sizeof(fpOutput));

    instIdx   = param->instIdx;
    coreIdx   = param->coreIdx;

    /* Check parameters */
    if (param->bitFormat != STD_HEVC && param->bitFormat != STD_VP9) {
        VLOG(ERR, "Not supported video standard: %d\n", param->bitFormat);
        success = FALSE;
        goto ERR_DEC_INIT;
    }


    /* if no scaleDown, use its initial width and height when pvircFbcEnable */
    // if (!param->scaleDownWidth && param->pvricFbcEnable == TRUE) {
    //     VLOG(ERR, "Can't use PVRIC fbc without down-scaling\n");
    //     success = FALSE;
    //     goto ERR_DEC_INIT;
    // }

    if (param->pvricFbcEnable == TRUE && (param->cbcrInterleave == FALSE || (param->cbcrInterleave==TRUE && param->nv21 == TRUE))) {
        VLOG(ERR, "only NV12 can be support on PVRIC fbc mode\n");
        success = FALSE;
        goto ERR_DEC_INIT;
    }
    if (param->pvricFbcEnable == TRUE && param->enableWTL == FALSE) {
        VLOG(ERR, "enableWTL should be always 1 when PVRIC fbc enabled.\n");
        success = FALSE;
        goto ERR_DEC_INIT;
    }


    if ((productId=(ProductId)VPU_GetProductId(coreIdx)) == -1) {
        VLOG(ERR, "Failed to get product ID\n");
        success = FALSE;
        goto ERR_DEC_INIT;
    }

    VLOG(INFO,"Product ID:%d \n",productId);

    intBitInitSeq = INT_WAVE_DEC_PIC_HDR;
    intBitDecPic  = INT_WAVE_DEC_PIC;
    intBitEmpty   = INT_WAVE_BIT_BUF_EMPTY;

    if (productId == PRODUCT_ID_412) {
        fwPath = CORE_2_BIT_CODE_FILE_PATH;
    }
    else {
        VLOG(ERR, "unsupported product id: %d\n", productId);
        return FALSE;
    }

    VLOG(INFO, "FW_PATH = %s\n", fwPath);
    if (LoadFirmware(productId, (Uint8**)&pusBitCode, &sizeInWord, fwPath) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, fwPath);
        return FALSE;
    }

    //3rd para was item count actually, so the size need to be devided into count.
    ret = VPU_InitWithBitcode(coreIdx, (const Uint16*)pusBitCode, sizeInWord);

    VLOG(INFO,"[VPU] STEP 1 - VPU boot up \n");
    // ret = VPU_Init(coreIdx);
    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
        VLOG(ERR, "Failed to boot up VPU(coreIdx: %d, productId: %d, ret:%d)\n", coreIdx, productId,ret);
        success = FALSE;
        goto ERR_DEC_INIT;
    }

    PrintVpuVersionInfo(coreIdx);

    VLOG(INFO,"[VPU] STEP 2 - Allocate BS buffer\n");
    bsBufferCount = (param->bitstreamMode == BS_MODE_PIC_END) ? 2 : 1;
    if ( param->bitFormat == STD_VP9 ) {
        param->bsSize = STREAM_BUF_SIZE_VP9;
    }
    VLOG(INFO, "Stream Buf size = %d(0x%x)\n", param->bsSize, param->bsSize);
    for (index=0; index<bsBufferCount; index++ ) {
        vbStream[index].size = param->bsSize;
        if (vdi_allocate_dma_memory(coreIdx, &vbStream[index]) < 0) {
            VLOG(ERR, "fail to allocate bitstream buffer\n" );
            success = FALSE;
            goto ERR_DEC_INIT;
        }
    }

    VLOG(INFO,"[VPU] STEP 2 - BS buffer allocated.\n");

    if (param->feedingMode == FEEDING_METHOD_FRAME_SIZE) {
        // CodStd  codecId;
        feeder = BitstreamFeeder_Create(param->inputPath, FEEDING_METHOD_FRAME_SIZE, vbStream[0].phys_addr, vbStream[0].size, param->bitFormat, NULL, NULL, NULL, NULL);
        // param->bitFormat = codecId;
    }
    else if (param->feedingMode == FEEDING_METHOD_FIXED_SIZE) {
        feeder = BitstreamFeeder_Create(param->inputPath, FEEDING_METHOD_FIXED_SIZE, vbStream[0].phys_addr, vbStream[0].size, 0x20000);
    }
    else {
        /* FEEDING_METHOD_SIZE_PLUS_ES */
        feeder = BitstreamFeeder_Create(param->inputPath, FEEDING_METHOD_SIZE_PLUS_ES, vbStream[0].phys_addr, vbStream[0].size);
    }



    if (feeder == NULL) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }

    VLOG(INFO,"[VPU] STEP 3 - BS feeder created: fm-%d bm-%d\n",param->feedingMode,param->bitstreamMode);

    BitstreamFeeder_SetFillMode(feeder, (param->bitstreamMode == BS_MODE_PIC_END) ? BSF_FILLING_LINEBUFFER : BSF_FILLING_RINGBUFFER);

    /* set up decoder configurations */
    decOP.bitstreamFormat     = (CodStd)param->bitFormat;
    decOP.coreIdx             = coreIdx;
    decOP.bitstreamBuffer     = vbStream[0].phys_addr;
    decOP.bitstreamBufferSize = vbStream[0].size ; //* bsBufferCount;
    decOP.bitstreamMode       = param->bitstreamMode;
    decOP.wtlEnable           = param->enableWTL;
    decOP.pvricFbcEnable      = param->pvricFbcEnable;
    decOP.pvric31HwSupport    = param->pvric31HwSupport;
    decOP.wtlMode             = param->wtlMode;
    decOP.cbcrInterleave      = param->cbcrInterleave;
    decOP.nv21                = param->nv21;
    decOP.streamEndian        = param->streamEndian;
    decOP.frameEndian         = param->frameEndian;
    decOP.fbc_mode            = param->wave4.fbcMode;
    decOP.bwOptimization      = param->wave4.bwOptimization;

    if ((ret=VPU_DecOpen(&handle, &decOP)) != RETCODE_SUCCESS) {
        success = FALSE;
        VLOG(ERR, "VPU_DecOpen failed Error code is 0x%x \n", ret );
        goto ERR_DEC_OPEN;
    }

    VLOG(INFO,"[VPU] STEP 4 - DEC INS opened \n");
    VLOG(INFO, "------------------------------ DECODER OPTIONS ------------------------------\n");
    VLOG(INFO, "[coreIdx            ]: %d\n", decOP.coreIdx            );
    VLOG(INFO, "[CODEC              ]: %d\n", decOP.bitstreamFormat           );
    VLOG(INFO, "[tiled2linear       ]: %d\n", param->coda9.enableTiled2Linear);
    VLOG(INFO, "[wtlEnable          ]: %d\n", decOP.wtlEnable          );
    VLOG(INFO, "[wtlMode            ]: %d\n", decOP.wtlMode            );
    VLOG(INFO, "[bitstreamBuffer    ]: 0x%lx\n", decOP.bitstreamBuffer);
    VLOG(INFO, "[bitstreamBufferSize]: %d\n", decOP.bitstreamBufferSize);
    VLOG(INFO, "[bitstreamMode      ]: %d\n", decOP.bitstreamMode      );
    VLOG(INFO, "[cbcrInterleave     ]: %d\n", decOP.cbcrInterleave     );
    VLOG(INFO, "[nv21               ]: %d\n", decOP.nv21               );
    VLOG(INFO, "[streamEndian       ]: %d\n", decOP.streamEndian       );
    VLOG(INFO, "[frameEndian        ]: %d\n", decOP.frameEndian        );
    VLOG(INFO, "[FBC                ]: %x\n", decOP.fbc_mode          );
    VLOG(INFO, "[BWOPT              ]: %d\n", decOP.bwOptimization      );
    VLOG(INFO, "[PVRIC              ]: %d\n", decOP.pvricFbcEnable      );
    VLOG(INFO, "[PVRIC31HW          ]: %d\n", decOP.pvric31HwSupport     );
    VLOG(INFO, "-----------------------------------------------------------------------------\n");

    // VPU_DecGiveCommand(handle, ENABLE_LOGGING, 0);

    VLOG(INFO,"[VPU] STEP 5 - Allocate user data.\n");
    vbUserData.size = USERDATA_BUFFER_SIZE;
    if (vdi_allocate_dma_memory(coreIdx, &vbUserData) < 0) {
        success = FALSE;
        VLOG(ERR, "%s:%d Failed to allocate memory\n", __FUNCTION__, __LINE__);
        goto ERR_DEC_OPEN;
    }
    pBase = (Uint8*)osal_malloc(USERDATA_BUFFER_SIZE);
    VPU_DecGiveCommand(handle, SET_ADDR_REP_USERDATA, (void*)&vbUserData.phys_addr);
    VPU_DecGiveCommand(handle, SET_SIZE_REP_USERDATA, (void*)&vbUserData.size);
    VPU_DecGiveCommand(handle, ENABLE_REP_USERDATA,   (void*)&param->enableUserData);

    if (param->thumbnailMode == TRUE) {
        VLOG(INFO,"[VPU] EXTRA OPT - enable thumbnail mode.\n");
        VPU_DecGiveCommand(handle, ENABLE_DEC_THUMBNAIL_MODE, NULL);
    }

    VLOG(INFO,"[VPU] STEP 6.1 - Create render.\n");
    if ((renderer=SimpleRenderer_Create(handle, rendererType, param->outputPath)) == NULL) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }

    if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian)) < 0) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }

    VLOG(INFO,"[VPU] STEP 6.2 - Create DP Q.\n");

    if ((displayQ=Queue_Create(MAX_REG_FRAME, sizeof(DecOutputInfo))) == NULL) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }
    VLOG(INFO,"[VPU] STEP 6.3 - Create SEQ Q.\n");
    if ((sequenceQ=Queue_Create(MAX_REG_FRAME, sizeof(uint32_t))) == NULL) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }

    // Scan and retrieve sequence header information from the bitstream buffer.
    VLOG(INFO,"[VPU] STEP 7 - SEQ init.\n");
    seqInitEscape = FALSE;
    ret = VPU_DecSetEscSeqInit(handle, seqInitEscape);
    if (ret != RETCODE_SUCCESS) {
        seqInitEscape = 0;
        VLOG(WARN, "Wanning! can not to set seqInitEscape in the current bitstream mode Option \n");
    }

    if (seqInitEscape == TRUE) {
        VPU_DecUpdateBitstreamBuffer(handle, 0);
    }

    ret = VPU_DecIssueSeqInit(handle);
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_DecIssueSeqInit failed Error code is 0x%x \n", ret);
        success = FALSE;
        goto ERR_DEC_OPEN;
    }

    VLOG(INFO,"[VPU] STEP 7.1 - SEQ init cmd issueed.\n");

    VLOG(INFO,"[VPU] STEP 7.2 - SEQ init start feeding.\n");
    timeoutCount = 0;
    while (seqInited == FALSE || (kbhitRet=osal_kbhit()) == 0) {
        interruptFlag = VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT);        //wait for 10ms to save stream filling time.
        if (interruptFlag == -1) {

            VLOG(INFO,"[VPU] STEP 7.2.f - INT timeout in SEQ INIT STATE\n");

            if (timeoutCount*VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) {
                VLOG(ERR, "\n [VPU] STEP 7.2.e - DEC_PIC_HDR TIMEOUT\n");
                EnterLock(coreIdx);
                PrintVpuStatus(coreIdx, productId);
                VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                LeaveLock(coreIdx);
                success = FALSE;
                goto ERR_DEC_OPEN;
            }
            timeoutCount++;
            interruptFlag = 0;
        }

        if (interruptFlag > 0) {
            VLOG(INFO,"[VPU] STEP 7.2.1 - INT %x in SEQ INIT STATE\n",interruptFlag);
            VPU_ClearInterrupt(coreIdx);
            if (interruptFlag & (1<<intBitInitSeq)) {
                seqInited = TRUE;
                VLOG(INFO,"[VPU] STEP 7.2.4 - SEQ cmd RSPed\n");
                break;
            }
        }

        if (decOP.bitstreamMode == BS_MODE_INTERRUPT) {

            VLOG(INFO,"[VPU] STEP 7.2.2 - INT BY BS REQ, FEEDING\n",interruptFlag);
            if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian)) < 0) {
                success = FALSE;
                goto ERR_DEC_OPEN;
            }

            VLOG(INFO,"[VPU] STEP 7.2.3 - BS FEEDED FOR SEQ\n",interruptFlag);
        }
    }

    if (seqInited == TRUE) {
        VLOG(INFO,"[VPU] STEP 7.3 - SEQ INIT-DONE cmd issued\n");
        if ((ret=VPU_DecCompleteSeqInit(handle, &initialInfo)) != RETCODE_SUCCESS) {
            VLOG(ERR, "%s:%d FAILED TO DEC_PIC_HDR: ret(%d), SEQERR(%08x)\n",
                __FUNCTION__, __LINE__, ret, initialInfo.seqInitErrReason);
            success = FALSE;
            EnterLock(coreIdx);
            HandleDecInitSequenceError(handle, productId, &decOP, &initialInfo, ret);
            LeaveLock(coreIdx);
            goto ERR_DEC_OPEN;
        }
    }

    VLOG(INFO,"[VPU] STEP 7.3.1 - SEQ INIT-DONE\n");

    if (initialInfo.userDataHeader != 0) {
        user_data_entry_t* pEntry = (user_data_entry_t*)pBase;
        VLOG(INFO, "===== USER DATA(SEI OR VUI) : NUM(%d) =====\n", initialInfo.userDataNum);
        VpuReadMem(coreIdx, vbUserData.phys_addr, pBase, vbUserData.size, VPU_USER_DATA_ENDIAN);

        if (initialInfo.userDataHeader & (1<<H265_USERDATA_FLAG_VUI)) {
            h265_vui_param_t*  vui = (h265_vui_param_t*)(pBase + pEntry[H265_USERDATA_FLAG_VUI].offset);
            VLOG(INFO, " VUI SAR(%d, %d)\n", vui->sar_width, vui->sar_height);
            VLOG(INFO, "     VIDEO FORMAT(%d)\n", vui->video_format);
        }
        if (initialInfo.userDataHeader & (1<<H265_USERDATA_FLAG_MASTERING_COLOR_VOL)) {
            h265_mastering_display_colour_volume_t* mastering;

            mastering = (h265_mastering_display_colour_volume_t*)(pBase + pEntry[H265_USERDATA_FLAG_MASTERING_COLOR_VOL].offset);
            VLOG(INFO, " MASTERING DISPLAY COLOR VOLUME\n");
            for (index=0; index<3; index++) {
                VLOG(INFO, " PRIMARIES_X%d : %10d PRIMARIES_Y%d : %10d\n", index, mastering->display_primaries_x[index], index, mastering->display_primaries_y[index]);
            }
            VLOG(INFO, " WHITE_POINT_X: %10d WHITE_POINT_Y: %10d\n", mastering->white_point_x, mastering->white_point_y);
            VLOG(INFO, " MIN_LUMINANCE: %10d MAX_LUMINANCE: %10d\n", mastering->min_display_mastering_luminance, mastering->max_display_mastering_luminance);
        }

        if (initialInfo.userDataHeader & (1<<H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT)) {
            h265_chroma_resampling_filter_hint_t* c_resampleing_filter_hint;
            uint32_t i,j;

            c_resampleing_filter_hint = (h265_chroma_resampling_filter_hint_t*)(pBase + pEntry[H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT].offset);
            VLOG(INFO, " CHROMA_RESAMPLING_FILTER_HINT\n");
            VLOG(INFO, " VER_CHROMA_FILTER_IDC: %10d HOR_CHROMA_FILTER_IDC: %10d\n", c_resampleing_filter_hint->ver_chroma_filter_idc, c_resampleing_filter_hint->hor_chroma_filter_idc);
            VLOG(INFO, " VER_FILTERING_FIELD_PROCESSING_FLAG: %d \n", c_resampleing_filter_hint->ver_filtering_field_processing_flag);
            if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1 || c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                VLOG(INFO, " TARGET_FORMAT_IDC: %d \n", c_resampleing_filter_hint->target_format_idc);
                if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1) {
                    VLOG(INFO, " NUM_VERTICAL_FILTERS: %d \n", c_resampleing_filter_hint->num_vertical_filters);
                    for (i=0; i<c_resampleing_filter_hint->num_vertical_filters; i++) {
                        VLOG(INFO, " VER_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->ver_tap_length_minus1[i]);
                        for (j=0; j<c_resampleing_filter_hint->ver_tap_length_minus1[i]; j++) {
                            VLOG(INFO, " VER_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->ver_filter_coeff[i][j]);
                        }
                    }
                }
                if (c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                    VLOG(INFO, " NUM_HORIZONTAL_FILTERS: %d \n", c_resampleing_filter_hint->num_horizontal_filters);
                    for (i=0; i<c_resampleing_filter_hint->num_horizontal_filters; i++) {
                        VLOG(INFO, " HOR_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->hor_tap_length_minus1[i]);
                        for (j=0; j<c_resampleing_filter_hint->hor_tap_length_minus1[i]; j++) {
                            VLOG(INFO, " HOR_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->hor_filter_coeff[i][j]);
                        }
                    }
                }
            }
        }

        if (initialInfo.userDataHeader & (1<<H265_USERDATA_FLAG_KNEE_FUNCTION_INFO)) {
            h265_knee_function_info_t* knee_function;

            knee_function = (h265_knee_function_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_KNEE_FUNCTION_INFO].offset);
            VLOG(INFO, " FLAG_KNEE_FUNCTION_INFO\n");
            VLOG(INFO, " KNEE_FUNCTION_ID: %10d\n", knee_function->knee_function_id);
            VLOG(INFO, " KNEE_FUNCTION_CANCEL_FLAG: %d\n", knee_function->knee_function_cancel_flag);
            if (knee_function->knee_function_cancel_flag) {
                VLOG(INFO, " KNEE_FUNCTION_PERSISTENCE_FLAG: %10d\n", knee_function->knee_function_persistence_flag);
                VLOG(INFO, " INPUT_D_RANGE: %d\n", knee_function->input_d_range);
                VLOG(INFO, " INPUT_DISP_LUMINANCE: %d\n", knee_function->input_disp_luminance);
                VLOG(INFO, " OUTPUT_D_RANGE: %d\n", knee_function->output_d_range);
                VLOG(INFO, " OUTPUT_DISP_LUMINANCE: %d\n", knee_function->output_disp_luminance);
                VLOG(INFO, " NUM_KNEE_POINTS_M1: %d\n", knee_function->num_knee_points_minus1);
                for (index=0; index<knee_function->num_knee_points_minus1; index++) {
                    VLOG(INFO, " INPUT_KNEE_POINT: %10d OUTPUT_KNEE_POINT: %10d\n", knee_function->input_knee_point[index], knee_function->output_knee_point[index]);
                }

            }

        }
        VLOG(INFO, "===========================================\n");
    }


    VLOG(INFO, "* Dec InitialInfo =>\n instance #%d, \n minframeBuffercount: %u\n", instIdx, initialInfo.minFrameBufferCount);
    VLOG(INFO, " picWidth: %u\n picHeight: %u\n ",initialInfo.picWidth, initialInfo.picHeight);
    VLOG(INFO,"level: %d tier: %d profile: %d\n",initialInfo.level,initialInfo.tier,initialInfo.profile);
    VLOG(INFO, "chromaFormatIDC %d lumaBitdepth %d, chromaBitdepth %d, sequenceNo %d\n",
        initialInfo.chromaFormatIDC, initialInfo.lumaBitdepth, initialInfo.chromaBitdepth, initialInfo.sequenceNo);

    /*if (param->pvricFbcEnable == TRUE && (initialInfo.lumaBitdepth > 8 || initialInfo.chromaBitdepth >8) && param->wtlFormat != FORMAT_420_P10_16BIT_LSB) {
        // Fix AllocateDecFrameBuffer format mismatch bug, should be FORMAT_420_P10_16BIT_LSB
        param->wtlFormat = FORMAT_420_P10_16BIT_LSB;
        VLOG(WARN, "10bit wtlFormat shall be FORMAT_420_P10_16BIT_MSB when PVRIC FBC enabled\n");
    }*/

    if (param->pvricFbcEnable == TRUE && (initialInfo.lumaBitdepth == 8 && initialInfo.chromaBitdepth == 8) && param->wtlFormat != FORMAT_420) {
        param->wtlFormat = FORMAT_420;
        VLOG(WARN, "8bit wtlFormat shall be FORMAT_420 when PVRIC FBC enabled\n");
    }

    compWidth  = initialInfo.picWidth;
    compHeight = initialInfo.picHeight;

    if (param->pvricFbcEnable) {
        if (!param->scaleDownWidth || !param->scaleDownHeight)
            param->scaleDownWidth = initialInfo.picWidth;
            param->scaleDownHeight = initialInfo.picHeight;
    }

    if (param->scaleDownWidth > 0 || param->scaleDownHeight > 0) {
        compWidth  = CalcScaleDown(initialInfo.picWidth, param->scaleDownWidth);
        compHeight = CalcScaleDown(initialInfo.picHeight, param->scaleDownHeight);
    }

    switch (param->compareType) {
    case NO_COMPARE:
        comparator = Comparator_Create(param->compareType, NULL);
        break;
    case YUV_COMPARE:
        comparator = Comparator_Create(param->compareType, param->refYuvPath,
            compWidth, compHeight,
            param->wtlFormat, param->cbcrInterleave);
        break;
    default:
        VLOG(ERR, "UNKNOWN COMPARATOR TYPE(%d)\n", param->compareType);
        success = FALSE;
        goto ERR_DEC_OPEN;
    }
    if (comparator == NULL) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }

    PrintDecSeqWarningMessages(productId, &initialInfo);

    /********************************************************************************
    * ALLOCATE FRAME BUFFER                                                        *
    ********************************************************************************/
    /* Set up the secondary AXI is depending on H/W configuration.
    * Note that turn off all the secondary AXI configuration
    * if target ASIC has no memory only for IP, LF and BIT.
    */
    VLOG(INFO,"[VPU] STEP 8 - Allocate Frame buffer\n");
    /*set sram buffer for sec AXI  */
    secAxiUse.u.wave4.useIpEnable = (param->secondaryAXI & 0x01) ? TRUE : FALSE;
    secAxiUse.u.wave4.useLfRowEnable = (param->secondaryAXI & 0x02) ? TRUE : FALSE;
    secAxiUse.u.wave4.useBitEnable = (param->secondaryAXI & 0x04) ? TRUE : FALSE;
    secAxiUse.u.wave4.useSclEnable = (param->secondaryAXI & 0x08) ? TRUE : FALSE;
    secAxiUse.u.wave4.useSclPackedModeEnable = (param->secondaryAXI & 0x10) ? TRUE : FALSE;

    VLOG(INFO, "[VPU] STEP 8.A - set sram config\n");
    VLOG(INFO, "[VPU] useIpEnable: %d\n", secAxiUse.u.wave4.useIpEnable);
    VLOG(INFO, "[VPU] useLfRowEnable: %d\n", secAxiUse.u.wave4.useLfRowEnable);
    VLOG(INFO, "[VPU] useBitEnable: %d\n", secAxiUse.u.wave4.useBitEnable);
    VLOG(INFO, "[VPU] useSclEnable: %d\n", secAxiUse.u.wave4.useSclEnable);
    VLOG(INFO, "[VPU] useSclPackedModeEnable: %d\n", secAxiUse.u.wave4.useSclPackedModeEnable);

    VPU_DecGiveCommand(handle, SET_SEC_AXI, &secAxiUse);

    // Set up scale

    if (param->scaleDownWidth > 0 || param->scaleDownHeight > 0) {
        VLOG(INFO,"[VPU] STEP 8.extra - set up scale\n");

        ScalerInfo sclInfo = {0};

        sclInfo.scaleWidth  = CalcScaleDown(initialInfo.picWidth, param->scaleDownWidth);
        sclInfo.scaleHeight = CalcScaleDown(initialInfo.picHeight, param->scaleDownHeight);
        sclInfo.enScaler    = TRUE;
        if (initialInfo.picWidth == sclInfo.scaleWidth && initialInfo.picHeight == sclInfo.scaleHeight)
            sclInfo.sameSize = TRUE;
        else
            sclInfo.sameSize = FALSE;
        VLOG(INFO, "[SCALE INFO] %dx%d to %dx%d, sameSize %d\n", initialInfo.picWidth, initialInfo.picHeight, sclInfo.scaleWidth, sclInfo.scaleHeight, sclInfo.sameSize);
        if (VPU_DecGiveCommand(handle, DEC_SET_SCALER_INFO, (void*)&sclInfo) != RETCODE_SUCCESS) {
            VLOG(ERR, "Failed to VPU_DecGiveCommand(DEC_SET_SCALER_INFO)\n");
            success = FALSE;
            goto ERR_DEC_OPEN;
        }
    }

    compressedFbCount = initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM + 4;   // max_dec_pic_buffering
    if (param->enableWTL == TRUE) {
        if (productId == PRODUCT_ID_420 || productId == PRODUCT_ID_420L) {
            linearFbCount = compressedFbCount;
        }
        else {
            linearFbCount = initialInfo.frameBufDelay + EXTRA_FRAME_BUFFER_NUM + 1;
        }
        if (handle->codecMode == C7_VP9_DEC) {
            linearFbCount = compressedFbCount;
        }
        VPU_DecGiveCommand(handle, DEC_SET_WTL_FRAME_FORMAT, &param->wtlFormat);
    }
    else {
        linearFbCount = 0;
    }

    VLOG(INFO, "compressedFbCount=%d, linearFbCount=%d\n", compressedFbCount, linearFbCount);

    osal_memset((void*)pFbMem, 0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);
    success = AllocateDecFrameBuffer(handle, param, compressedFbCount, linearFbCount, pFrame, pFbMem, &framebufStride);
    if (success == FALSE) {
        goto ERR_DEC_OPEN;
    }
    VLOG(INFO,"[VPU] STEP 8.1 - Frame buffer allocated: compressed fb:%d linear fb:%d framebufstride:%d\n",compressedFbCount,
    linearFbCount,framebufStride);

    /********************************************************************************
    * REGISTER FRAME BUFFER                                                        *
    ********************************************************************************/

   VLOG(INFO,"[VPU] STEP 9 - Register Frame buffer to decoder\n");
    ret = VPU_DecRegisterFrameBufferEx(handle, pFrame, compressedFbCount, linearFbCount, framebufStride, initialInfo.picHeight, COMPRESSED_FRAME_MAP);
    if( ret != RETCODE_SUCCESS ) {
        success = FALSE;
        if (ret == RETCODE_MEMORY_ACCESS_VIOLATION) {
            EnterLock(coreIdx);
            PrintMemoryAccessViolationReason(coreIdx, NULL);
            LeaveLock(coreIdx);
        }

        VLOG(ERR, "VPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        goto ERR_DEC_OPEN;
    }
    VLOG(INFO,"[VPU] STEP 9 - Frame buffer registered\n");

    doDumpImage = (BOOL)(param->compareType == YUV_COMPARE || (strlen(param->outputPath) > 0) || rendererType == RENDER_DEVICE_FBDEV);

    DisplayDecodedInformation(handle, decOP.bitstreamFormat, 0, NULL);

    val = (param->bitFormat == STD_HEVC) ? SEQ_CHANGE_ENABLE_ALL_HEVC : SEQ_CHANGE_ENABLE_ALL_VP9;
    VPU_DecGiveCommand(handle, DEC_SET_SEQ_CHANGE_MASK, (void*)&val);
    /********************************************************************************
    * DEC_PIC                                                                      *
    ********************************************************************************/
    decParam.skipframeMode = param->skipMode;
    loopCount              = param->loopCount;

    VLOG(INFO,"[VPU] STEP 10 - Start decoding\n");
    VLOG(INFO,"[VPU] ------- Skip mode ------- %d\n",decParam.skipframeMode);
    VLOG(INFO,"[VPU] ------ loop count -------- %d\n",loopCount);

    if (param->invalidDisFlag) {
    /* set diplay flag invalid */
        int i = 0;
        int value = 0;
        srand(time(NULL));
        for (i = 0;i < 7;i++) {
            value = rand() % linearFbCount;
            VPU_DecGiveCommand(handle, DEC_SET_DISPLAY_FLAG, (void*)&value);
            VLOG(INFO, "get rand value %d, linearFbCount %d\n", value, linearFbCount);
        }
    }
    while (loop == TRUE && (param->exitFlag == 0)) {
        if (notDecodedCount == MAX_NOT_DEC_COUNT) {
            VLOG(ERR, "Continuous not-decoded-cout is %d\nThere is something problem in DPB control.\n", notDecodedCount);
            success = FALSE;
            break;
        }

        if ((pDisplayInfo=(DecOutputInfo*)SimpleRenderer_GetFreeFrameInfo(renderer)) != NULL) {
            uint32_t*   ptr;
            ptr = (uint32_t*)Queue_Peek(sequenceQ);
            if (ptr && *ptr != pDisplayInfo->sequenceNo) {
                /* Release all framebuffers of previous sequence */
                SequenceMemInfo* p;
                index = (*ptr) % MAX_SEQUENCE_MEM_COUNT;
                p = &seqMemInfo[index];
                ReleasePreviousSequenceResources(handle, p->allocFbMem, &p->fbInfo);
                osal_memset(p, 0x00, sizeof(SequenceMemInfo));
                Queue_Dequeue(sequenceQ);
            }
            if (pDisplayInfo->sequenceNo == initialInfo.sequenceNo) {
                VPU_DecClrDispFlag(handle, pDisplayInfo->indexFrameDisplay);
            }
        }

        if (needStream == TRUE) {
            bsQueueIndex = (bsQueueIndex+1)%bsBufferCount;
//            VPU_DecSetRdPtr(handle, vbStream[bsQueueIndex].phys_addr, TRUE);
            VPU_DecSetRdPtrEx(handle, vbStream[bsQueueIndex].phys_addr,vbStream[bsQueueIndex].phys_addr, TRUE);
            EnterLock(coreIdx);
            nWritten = BitstreamFeeder_Act(handle, feeder, decOP.streamEndian);
            LeaveLock(coreIdx);
            if (nWritten < 0) {
                success = FALSE;
                goto ERR_DEC_OPEN;
            }
        }


        decParam.skipframeMode = param->skipMode;
        if (decParam.skipframeMode == 1) {
            SimpleRenderer_Flush(renderer);
        }
        // Start decoding a frame.

        VLOG(INFO,"[VPU] STEP 10.1  - Decoding one frame\n");
        decParam.craAsBlaFlag = param->wave4.craAsBla;
        decParam.pixelLumaPad   = param->pvricPaddingY;  // default padding pixel value = 0
        decParam.pixelChromaPad = param->pvricPaddingC;
        VLOG(INFO,"[VPU] cra=bla: %d\n",decParam.craAsBlaFlag);
        VLOG(INFO,"[VPU] Y-padding: %d\n",decParam.pixelLumaPad);
        VLOG(INFO,"[VPU] C-padding: %d\n",decParam.pixelChromaPad);

        startUs = GetNowUs();
        ret=VPU_DecStartOneFrame(handle, &decParam);
        if (ret != RETCODE_SUCCESS) {
            success = FALSE;
            VLOG(ERR, "VPU_DecStartOneFrame failed Error code is 0x%x \n", ret );
            LeaveLock(coreIdx);
            goto ERR_DEC_OPEN;
        }

        timeoutCount = 0;
        repeat       = TRUE;
        while(repeat == TRUE) {
            if (decOP.bitstreamMode == BS_MODE_INTERRUPT) {
                VLOG(INFO,"[VPU] STEP 10.1.1 - BS feeding\n");
                if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian)) < 0) {
                    success = FALSE;
                    LeaveLock(coreIdx);
                    goto ERR_DEC_OPEN;
                }
            }

            if ((interruptFlag=VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT)) == -1) {
                //wait for 10ms to save stream filling time.
                if (timeoutCount*VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) {
                    HandleDecoderError(handle, frameIdx, param, pFbMem, NULL);
                    PrintVpuStatus(coreIdx, productId);
                    VLOG(WARN, "\n VPU interrupt wait timeout\n");
                    VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                    repeat  = FALSE;
                    success = FALSE;
                    break;
                }
                timeoutCount++;
                interruptFlag = 0;
            }

            if (interruptFlag > 0) {
                VPU_ClearInterrupt(coreIdx);
            }

            if (interruptFlag & (1<<intBitDecPic)) {
                   VLOG(INFO, "%s:%d : interrupt \n", __FUNCTION__, __LINE__);
                break;
            }

            if (interruptFlag & (1<<intBitEmpty)) {
                VLOG(INFO,"[VPU] STEP 10.1.1E - BSEMPTY INT \n");
                /* TODO: handling empty interrupt */
            }
        }

        VLOG(INFO,"[VPU] STEP 10.1.2  - DEC one Frame finish\n");

        ret = VPU_DecGetOutputInfo(handle, &outputInfo);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_DecGetOutputInfo failed Error code is 0x%x\n", ret);
            if (ret == RETCODE_MEMORY_ACCESS_VIOLATION) {
                EnterLock(coreIdx);
                PrintMemoryAccessViolationReason(coreIdx, &outputInfo);
                PrintVpuStatus(coreIdx, productId);
                LeaveLock(coreIdx);
                success = FALSE;
                break;
            }
            continue;
        }

        if (outputInfo.decOutputExtData.userDataNum > 0) {
            VLOG(INFO,"go into dec user data section \n");
            user_data_entry_t* pEntry = (user_data_entry_t*)pBase;
            VpuReadMem(coreIdx, vbUserData.phys_addr, pBase, vbUserData.size, VPU_USER_DATA_ENDIAN);

            for (index=0; index<32; index++) {
                if (outputInfo.decOutputExtData.userDataHeader&(1<<index)) {
                    VLOG(INFO, "USERDATA INDEX: %02d offset: %8d size: %d\n", index, pEntry[index].offset, pEntry[index].size);

                    if (outputInfo.decOutputExtData.userDataHeader & (1<<H265_USERDATA_FLAG_MASTERING_COLOR_VOL)) {
                        h265_mastering_display_colour_volume_t* mastering;
                        int i;

                        mastering = (h265_mastering_display_colour_volume_t*)(pBase + pEntry[H265_USERDATA_FLAG_MASTERING_COLOR_VOL].offset);
                        VLOG(INFO, " MASTERING DISPLAY COLOR VOLUME\n");
                        for (i=0; i<3; i++) {
                            VLOG(INFO, " PRIMARIES_X%d : %10d PRIMARIES_Y%d : %10d\n", i, mastering->display_primaries_x[i], i, mastering->display_primaries_y[i]);
                        }
                        VLOG(INFO, " WHITE_POINT_X: %10d WHITE_POINT_Y: %10d\n", mastering->white_point_x, mastering->white_point_y);
                        VLOG(INFO, " MIN_LUMINANCE: %10d MAX_LUMINANCE: %10d\n", mastering->min_display_mastering_luminance, mastering->max_display_mastering_luminance);
                    }

                    if(outputInfo.decOutputExtData.userDataHeader&(1<<H265_USERDATA_FLAG_VUI))
                    {
                        h265_vui_param_t* vui;

                        vui = (h265_vui_param_t*)(pBase + pEntry[H265_USERDATA_FLAG_VUI].offset);
                        VLOG(INFO, " VUI SAR(%d, %d)\n", vui->sar_width, vui->sar_height);
                        VLOG(INFO, "     VIDEO FORMAT(%d)\n", vui->video_format);
                        VLOG(INFO, "     COLOUR PRIMARIES(%d)\n", vui->colour_primaries);
                        VLOG(INFO, "log2_max_mv_length_horizontal: %d\n", vui->log2_max_mv_length_horizontal);
                        VLOG(INFO, "log2_max_mv_length_vertical  : %d\n", vui->log2_max_mv_length_vertical);
                    }
                    if (outputInfo.decOutputExtData.userDataHeader & (1<<H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT)) {
                        h265_chroma_resampling_filter_hint_t* c_resampleing_filter_hint;
                        uint32_t i,j;

                        c_resampleing_filter_hint = (h265_chroma_resampling_filter_hint_t*)(pBase + pEntry[H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT].offset);
                        VLOG(INFO, " CHROMA_RESAMPLING_FILTER_HINT\n");
                        VLOG(INFO, " VER_CHROMA_FILTER_IDC: %10d HOR_CHROMA_FILTER_IDC: %10d\n", c_resampleing_filter_hint->ver_chroma_filter_idc, c_resampleing_filter_hint->hor_chroma_filter_idc);
                        VLOG(INFO, " VER_FILTERING_FIELD_PROCESSING_FLAG: %d \n", c_resampleing_filter_hint->ver_filtering_field_processing_flag);
                        if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1 || c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                            VLOG(INFO, " TARGET_FORMAT_IDC: %d \n", c_resampleing_filter_hint->target_format_idc);
                            if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1) {
                                VLOG(INFO, " NUM_VERTICAL_FILTERS: %d \n", c_resampleing_filter_hint->num_vertical_filters);
                                for (i=0; i<c_resampleing_filter_hint->num_vertical_filters; i++) {
                                    VLOG(INFO, " VER_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->ver_tap_length_minus1[i]);
                                    for (j=0; j<c_resampleing_filter_hint->ver_tap_length_minus1[i]; j++) {
                                        VLOG(INFO, " VER_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->ver_filter_coeff[i][j]);
                                    }
                                }
                            }
                            if (c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                                VLOG(INFO, " NUM_HORIZONTAL_FILTERS: %d \n", c_resampleing_filter_hint->num_horizontal_filters);
                                for (i=0; i<c_resampleing_filter_hint->num_horizontal_filters; i++) {
                                    VLOG(INFO, " HOR_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->hor_tap_length_minus1[i]);
                                    for (j=0; j<c_resampleing_filter_hint->hor_tap_length_minus1[i]; j++) {
                                        VLOG(INFO, " HOR_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->hor_filter_coeff[i][j]);
                                    }
                                }
                            }
                        }
                    }

                    if (outputInfo.decOutputExtData.userDataHeader & (1<<H265_USERDATA_FLAG_KNEE_FUNCTION_INFO)) {
                        h265_knee_function_info_t* knee_function;

                        knee_function = (h265_knee_function_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_KNEE_FUNCTION_INFO].offset);
                        VLOG(INFO, " FLAG_KNEE_FUNCTION_INFO\n");
                        VLOG(INFO, " KNEE_FUNCTION_ID: %10d\n", knee_function->knee_function_id);
                        VLOG(INFO, " KNEE_FUNCTION_CANCEL_FLAG: %d\n", knee_function->knee_function_cancel_flag);
                        if (knee_function->knee_function_cancel_flag) {
                            VLOG(INFO, " KNEE_FUNCTION_PERSISTENCE_FLAG: %10d\n", knee_function->knee_function_persistence_flag);
                            VLOG(INFO, " INPUT_D_RANGE: %d\n", knee_function->input_d_range);
                            VLOG(INFO, " INPUT_DISP_LUMINANCE: %d\n", knee_function->input_disp_luminance);
                            VLOG(INFO, " OUTPUT_D_RANGE: %d\n", knee_function->output_d_range);
                            VLOG(INFO, " OUTPUT_DISP_LUMINANCE: %d\n", knee_function->output_disp_luminance);
                            VLOG(INFO, " NUM_KNEE_POINTS_M1: %d\n", knee_function->num_knee_points_minus1);
                            for (index=0; index<knee_function->num_knee_points_minus1; index++) {
                                VLOG(INFO, " INPUT_KNEE_POINT: %10d OUTPUT_KNEE_POINT: %10d\n", knee_function->input_knee_point[index], knee_function->output_knee_point[index]);
                            }

                        }

                    }
                }
            }
        }

        decodedIndex        = outputInfo.indexFrameDecoded;
        prescanIndex        = outputInfo.indexFramePrescan;
        sequenceChangeFlag  = outputInfo.sequenceChanged;
        sequenceChangeOccurred |= sequenceChangeFlag;

        if ((outputInfo.decodingSuccess & 0x01) == 0) {
           VLOG(INFO,"output dec imcomplete \n");
            if (outputInfo.indexFramePrescan == -2) {
                VLOG(WARN, "Stream is insufficient to prescan\n");
            }
            else {
                VLOG(ERR, "instance(%d) VPU_DecGetOutputInfo decode fail framdIdx %d error(0x%08x) reason(0x%08x), reasonExt(0x%08x)\n",
                           handle->instIndex, frameIdx, outputInfo.decodingSuccess, outputInfo.errorReason, outputInfo.errorReasonExt);
            }
        } else {
            diffUs = GetNowUs() - startUs;
            totalUs += diffUs;
            if (outputInfo.picType == PIC_TYPE_I
                || outputInfo.picType == PIC_TYPE_P
                || outputInfo.picType == PIC_TYPE_B
                || outputInfo.picType == PIC_TYPE_IDR) {
                VLOG(TRACE, "decoding time=%.1fms\n", (double)diffUs/1000);
            }
        }

        if (success == FALSE) {
            break;
        }

        if (sequenceChangeFlag) {
            VLOG(INFO,"SEQ CHANGED\n");
            BOOL     dpbChanged, sizeChanged, bitDepthChanged;

            dpbChanged      = (sequenceChangeFlag&SEQ_CHANGE_ENABLE_DPB_COUNT) ? TRUE : FALSE;
            sizeChanged     = (sequenceChangeFlag&SEQ_CHANGE_ENABLE_SIZE)      ? TRUE : FALSE;
            bitDepthChanged = (sequenceChangeFlag&SEQ_CHANGE_ENABLE_BITDEPTH)  ? TRUE : FALSE;

            if (dpbChanged || sizeChanged || bitDepthChanged) {
                DecOutputInfo*      pDecOutInfo = (DecOutputInfo*)osal_malloc(sizeof(DecOutputInfo)*MAX_GDI_IDX);
                BOOL                remainingFbs[MAX_REG_FRAME];
                int32_t             fbIndex;
                uint32_t            retNum, dispFlag;
                uint32_t            curSeqNo    = outputInfo.sequenceNo;
                uint32_t            seqMemIndex = curSeqNo % MAX_SEQUENCE_MEM_COUNT;
                SequenceMemInfo*    pSeqMem     = &seqMemInfo[seqMemIndex];
                DecGetFramebufInfo  prevSeqFbInfo;

                VLOG(INFO, "----- SEQUENCE CHANGED -----\n");

                Queue_Enqueue(sequenceQ, (void*)&curSeqNo);
                osal_memset((void*)remainingFbs, 0x00, sizeof(remainingFbs));
                // Get previous memory related to framebuffer
                VPU_DecGiveCommand(handle, DEC_GET_FRAMEBUF_INFO, (void*)&prevSeqFbInfo);
                // Get current(changed) sequence information.
                VPU_DecGiveCommand(handle, DEC_GET_SEQ_INFO, &initialInfo);
                // Flush all remaining framebuffers of previous sequence.
                VPU_DecFrameBufferFlush(handle, pDecOutInfo, &retNum);

                VLOG(INFO, "sequenceChanged : %x\n", sequenceChangeFlag);
                VLOG(INFO, "SEQUENCE NO : %d\n", initialInfo.sequenceNo);
                VLOG(INFO, "DPB COUNT: %d\n", initialInfo.minFrameBufferCount);
                VLOG(INFO, "BITDEPTH : LUMA(%d), CHROMA(%d)\n", initialInfo.lumaBitdepth, initialInfo.chromaBitdepth);
                VLOG(INFO, "SIZE     : WIDTH(%d), HEIGHT(%d)\n", initialInfo.picWidth, initialInfo.picHeight);

                // Free allocated framebuffers except ones to be displayed.
                for (index=0; index<retNum; index++) {
                    fbIndex = pDecOutInfo[index].indexFrameDisplay;
                    if (fbIndex >= 0) {
                        VLOG(INFO, "PUSH SEQ[%02d] LINEAR(%02d) COMPRESSED(%02d)\n",
                            curSeqNo, pDecOutInfo[index].indexFrameDisplay, pDecOutInfo[index].indexFrameDisplayForTiled);
                        Queue_Enqueue(displayQ, (void*)&pDecOutInfo[index]);
                        if (decOP.wtlEnable == TRUE) {
                            fbIndex = VPU_CONVERT_WTL_INDEX(handle, fbIndex);
                            remainingFbs[fbIndex] = TRUE;
                        }
                        else {
                            fbIndex = pDecOutInfo[index].indexFrameDisplayForTiled;
                            remainingFbs[fbIndex] = TRUE;
                        }
                    }
                }

                /* Check Not displayed framebuffers */
                dispFlag = outputInfo.frameDisplayFlag;
                for (index=0; index<MAX_GDI_IDX; index++) {
                    fbIndex = index;
                    if ((dispFlag>>index) & 0x01) {
                        if (decOP.wtlEnable == TRUE) {
                            fbIndex = VPU_CONVERT_WTL_INDEX(handle, fbIndex);
                        }
                        remainingFbs[fbIndex] = TRUE;
                    }
                }

                for (index=0; index<MAX_REG_FRAME; index++) {
                    if (remainingFbs[index] == FALSE) {
                        // free allocated framebuffer
                        if (pFbMem[index].size > 0) {
                            vdi_free_dma_memory(coreIdx, &pFbMem[index]);
                        }
                    }
                }
                // Free all framebuffers
                if (handle->codecMode == C7_VP9_DEC) {
                    for ( index=0 ; index<MAX_REG_FRAME; index++) {
                        if(prevSeqFbInfo.vbMvCol[index].size > 0) {
                            vdi_free_dma_memory(coreIdx, &prevSeqFbInfo.vbMvCol[index]);
                        }
                        if(prevSeqFbInfo.vbFbcYTbl[index].size > 0) {
                            vdi_free_dma_memory(coreIdx, &prevSeqFbInfo.vbFbcYTbl[index]);
                        }
                        if(prevSeqFbInfo.vbFbcCTbl[index].size > 0) {
                            vdi_free_dma_memory(coreIdx, &prevSeqFbInfo.vbFbcCTbl[index]);
                        }
                    }
                }
                osal_memset(pSeqMem, 0x00, sizeof(SequenceMemInfo));
                osal_memcpy(&pSeqMem->fbInfo, &prevSeqFbInfo, sizeof(DecGetFramebufInfo));
                osal_memcpy(pSeqMem->allocFbMem, pFbMem, sizeof(pFbMem));

                VPU_DecGiveCommand(handle, DEC_RESET_FRAMEBUF_INFO, NULL);

                if (param->scaleDownWidth > 0 || param->scaleDownHeight > 0) {
                    ScalerInfo sclInfo = {0};

                    sclInfo.scaleWidth  = CalcScaleDown(initialInfo.picWidth, param->scaleDownWidth);
                    sclInfo.scaleHeight = CalcScaleDown(initialInfo.picHeight, param->scaleDownHeight);
                    VLOG(INFO, "[SCALE INFO] %dx%d to %dx%d\n", initialInfo.picWidth, initialInfo.picHeight, sclInfo.scaleWidth, sclInfo.scaleHeight);
                    sclInfo.enScaler    = TRUE;
                    if (initialInfo.picWidth == sclInfo.scaleWidth && initialInfo.picHeight == sclInfo.scaleHeight)
                        sclInfo.sameSize = TRUE;
                    else
                        sclInfo.sameSize = FALSE;
                    if (VPU_DecGiveCommand(handle, DEC_SET_SCALER_INFO, (void*)&sclInfo) != RETCODE_SUCCESS) {
                        VLOG(ERR, "Failed to VPU_DecGiveCommand(DEC_SET_SCALER_INFO)\n");
                        success = FALSE;
                        goto ERR_DEC_OPEN;
                    }
                }

                compressedFbCount = initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM + 4;       /* max_dec_pic_buffering + @, @ >= 1 */
                if (handle->codecMode == C7_VP9_DEC)
                    linearFbCount     = compressedFbCount;
                else
                    linearFbCount     = initialInfo.frameBufDelay + (1+EXTRA_FRAME_BUFFER_NUM*2);       /* max_num_reorder_pics + @,  @ >= 1,
                                                                                                        In most case, # of linear fbs must be greater or equal than max_num_reorder,
                                                                                                        but the expression of @ in the sample code is in order to make the situation
                                                                                                        that # of linear is greater than # of fbc. */
                osal_memset((void*)pFbMem, 0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);
                if (AllocateDecFrameBuffer(handle, param, compressedFbCount, linearFbCount, pFrame, pFbMem, &framebufStride) == FALSE) {
                    success = FALSE;
                    if (pDecOutInfo != NULL) {
                        osal_free(pDecOutInfo);
                    }
                    VLOG(ERR, "[SEQ_CHANGE] AllocateDecFrameBuffer failed Error code is 0x%x\n", success);
                    goto ERR_DEC_OPEN;
                }
                ret = VPU_DecRegisterFrameBufferEx(handle, pFrame, compressedFbCount, linearFbCount,
                    framebufStride, initialInfo.picHeight, COMPRESSED_FRAME_MAP);
                if (ret != RETCODE_SUCCESS) {
                    success = FALSE;
                    if (pDecOutInfo != NULL) {
                        osal_free(pDecOutInfo);
                    }
                    VLOG(ERR, "[SEQ_CHANGE] VPU_DecRegisterFrameBufferEx failed Error code is 0x%x\n", ret);
                    goto ERR_DEC_OPEN;
                }


                /* set diplay flag invalid */
                if (param->invalidDisFlag) {
                    int i = 0;
                    int value = 0;
                    srand(time(NULL));
                    for (i = 0;i < 7;i++) {
                        value = rand() % linearFbCount;
                        VPU_DecGiveCommand(handle, DEC_SET_DISPLAY_FLAG, (void*)&value);
                        VLOG(INFO, "get rand value %d, linearFbCount %d\n", value, linearFbCount);
                    }
                }

                /*
                To reuse bitstream buffer after sequence changed on VP9
                When detected sequence change on VP9,it's not necessary to load next frame.
                VP9 can detect sequence change only on KEY_FRAME.
                So need to reuse same chunk, after udpate frame buffer
                */
                if (handle->codecMode == C7_VP9_DEC)
                    needStream = FALSE;

                VLOG(INFO, "----------------------------\n");

                if (pDecOutInfo != NULL) {
                    osal_free(pDecOutInfo);
                }
            }
        }
        if (handle->codecMode == C7_VP9_DEC &&
            (outputInfo.sequenceChanged&SEQ_CHANGE_INTER_RES_CHANGE ||
            ((outputInfo.indexInterFrameDecoded != -1) && outputInfo.indexFrameDecoded == DECODED_IDX_FLAG_NO_FB))) {
                Uint32      picWidth, picHeight;
                size_t      framebufHeight;
                int32_t     fbcCurFrameIdx = 0;
                int32_t     bwbCurFrameIdx = 0;
                int32_t     i;

                VLOG(INFO, "----- INTER RESOLUTION CHANGED -----\n");
                fbcCurFrameIdx  = (short)(outputInfo.indexInterFrameDecoded & 0xffff);
                bwbCurFrameIdx  = (short)((outputInfo.indexInterFrameDecoded >> 16) & 0xffff);

                picWidth = outputInfo.decPicWidth;
                picHeight = outputInfo.decPicHeight;

                VPU_DecGiveCommand(handle, DEC_SET_BWB_CUR_FRAME_IDX, (void*)&bwbCurFrameIdx);
                VPU_DecGiveCommand(handle, DEC_SET_FBC_CUR_FRAME_IDX, (void*)&fbcCurFrameIdx);

                VLOG(INFO, "Prev SIZE         : WIDTH(%d), HEIGHT(%d)\n", initialInfo.picWidth, initialInfo.picHeight);
                VPU_DecGiveCommand(handle, DEC_GET_SEQ_INFO, &initialInfo);
                VLOG(INFO, "SEQUENCE NO       : %d\n", initialInfo.sequenceNo);
                VLOG(INFO, "FBC CUR FRAME IDX : %d\n", fbcCurFrameIdx);
                VLOG(INFO, "BWB CUR FRAME IDX : %d\n", bwbCurFrameIdx);
                VLOG(INFO, "SIZE              : WIDTH(%d), HEIGHT(%d)\n", initialInfo.picWidth, initialInfo.picHeight);

                if (outputInfo.indexInterFrameDecoded != -1) {
                    for ( i=0 ; i < linearFbCount ; i++) {
                        VPU_DecClrDispFlag(handle, i);
                    }
                }

                if (param->scaleDownWidth > 0 || param->scaleDownHeight > 0) {
                    ScalerInfo sclInfo = {0};

                    sclInfo.scaleWidth  = CalcScaleDown(initialInfo.picWidth, param->scaleDownWidth);
                    sclInfo.scaleHeight = CalcScaleDown(initialInfo.picHeight, param->scaleDownHeight);
                    VLOG(INFO, "[SCALE INFO] %dx%d to %dx%d\n", initialInfo.picWidth, initialInfo.picHeight, sclInfo.scaleWidth, sclInfo.scaleHeight);
                    sclInfo.enScaler    = TRUE;
                    if (initialInfo.picWidth == sclInfo.scaleWidth && initialInfo.picHeight == sclInfo.scaleHeight)
                        sclInfo.sameSize = TRUE;
                    else
                        sclInfo.sameSize = FALSE;
                    if (VPU_DecGiveCommand(handle, DEC_SET_SCALER_INFO, (void*)&sclInfo) != RETCODE_SUCCESS) {
                        VLOG(ERR, "Failed to VPU_DecGiveCommand(DEC_SET_SCALER_INFO)\n");
                        success = FALSE;
                        goto ERR_DEC_OPEN;
                    }
                }
                if(fbcCurFrameIdx >= 0){
                    VPU_DecGiveCommand(handle, DEC_FREE_MV_BUFFER, (void*)&fbcCurFrameIdx);
                    VPU_DecGiveCommand(handle, DEC_FREE_FBC_TABLE_BUFFER, (void*)&fbcCurFrameIdx);

                    VPU_DecGiveCommand(handle, DEC_ALLOC_MV_BUFFER, (void*)&fbcCurFrameIdx);
                    VPU_DecGiveCommand(handle, DEC_ALLOC_FBC_Y_TABLE_BUFFER, (void*)&fbcCurFrameIdx);
                    VPU_DecGiveCommand(handle, DEC_ALLOC_FBC_C_TABLE_BUFFER, (void*)&fbcCurFrameIdx);
                }

                if (param->scaleDownWidth > 0 || param->scaleDownHeight > 0) {
                    ScalerInfo sclInfo = {0};

                    sclInfo.scaleWidth  = CalcScaleDown(initialInfo.picWidth, param->scaleDownWidth);
                    sclInfo.scaleHeight = CalcScaleDown(initialInfo.picHeight, param->scaleDownHeight);
                    VLOG(INFO, "[SCALE INFO] %dx%d to %dx%d\n", initialInfo.picWidth, initialInfo.picHeight, sclInfo.scaleWidth, sclInfo.scaleHeight);
                    sclInfo.enScaler    = TRUE;
                    if (initialInfo.picWidth == sclInfo.scaleWidth && initialInfo.picHeight == sclInfo.scaleHeight)
                        sclInfo.sameSize = TRUE;
                    else
                        sclInfo.sameSize = FALSE;
                    if (VPU_DecGiveCommand(handle, DEC_SET_SCALER_INFO, (void*)&sclInfo) != RETCODE_SUCCESS) {
                        VLOG(ERR, "Failed to VPU_DecGiveCommand(DEC_SET_SCALER_INFO)\n");
                        success = FALSE;
                        goto ERR_DEC_OPEN;
                    }
                }

                if ((fbcCurFrameIdx >=0) || (bwbCurFrameIdx >=0)) {
                    FrameBufferAllocInfo    fbAllocInfo;
                    FrameBufferFormat       format = FORMAT_420;

                    if (pFbMem[fbcCurFrameIdx].size > 0) {
                        vdi_free_dma_memory(coreIdx, &pFbMem[fbcCurFrameIdx]);
                        osal_memset((void*)&pFbMem[fbcCurFrameIdx], 0x00, sizeof(vpu_buffer_t));
                    }

                    compressedFbCount = initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;
                    linearFbCount     = compressedFbCount;

                    osal_memset((void*)&fbAllocInfo, 0x00, sizeof(fbAllocInfo));

                    if (productId == PRODUCT_ID_4102 || productId == PRODUCT_ID_420 || productId == PRODUCT_ID_412 || productId == 510 || productId == PRODUCT_ID_512) {
                        format = (initialInfo.lumaBitdepth > 8 || initialInfo.chromaBitdepth > 8) ? FORMAT_420_P10_16BIT_LSB : FORMAT_420;
                    }

                    framebufStride = CalcStride(VPU_ALIGN64(picWidth), picHeight, format, param->cbcrInterleave, param->mapType, TRUE);
                    framebufHeight = VPU_ALIGN64(picHeight);

                    fbAllocInfo.format          = format;
                    fbAllocInfo.cbcrInterleave  = param->cbcrInterleave;
                    fbAllocInfo.mapType         = param->mapType;
                    fbAllocInfo.stride          = framebufStride;
                    fbAllocInfo.height          = handle->codecMode == C7_VP9_DEC ? framebufHeight : initialInfo.picHeight;
                    fbAllocInfo.lumaBitDepth    = initialInfo.lumaBitdepth;
                    fbAllocInfo.chromaBitDepth  = initialInfo.chromaBitdepth;
                    fbAllocInfo.num             = compressedFbCount;
                    fbAllocInfo.endian          = param->frameEndian;
                    fbAllocInfo.type            = FB_TYPE_CODEC;

                    pFbMem[fbcCurFrameIdx].size = VPU_GetFrameBufSize(handle->coreIdx, framebufStride, framebufHeight,
                        param->mapType, format, param->cbcrInterleave, NULL);
                    if (vdi_allocate_dma_memory(coreIdx, &pFbMem[fbcCurFrameIdx]) < 0) {
                        VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                        goto ERR_DEC_OPEN;
                    }
                    pFrame[fbcCurFrameIdx].bufY  =  pFbMem[fbcCurFrameIdx].phys_addr;
                    pFrame[fbcCurFrameIdx].bufCb = (PhysicalAddress)-1;
                    pFrame[fbcCurFrameIdx].bufCr = (PhysicalAddress)-1;
                    pFrame[fbcCurFrameIdx].updateFbInfo = TRUE;

                    if ((ret=VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, pFrame)) != RETCODE_SUCCESS) {
                        VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer(), ret(%d)\n",
                            __FUNCTION__, __LINE__, ret);
                        //ReleaseVideoMemory(coreIdx, pFbMem[curFrameIdx], compressedFbCount);
                        // for (index=0; index<(uint32_t)compressedFbCount; index++) {
                        //     if (pFbMem[index].size)
                        //         vdi_free_dma_memory(coreIdx, &pFbMem[index]);
                        // }
                        goto ERR_DEC_OPEN;
                    }

                    if (param->enableWTL == TRUE) {
                        size_t  linearStride;
                        size_t  picWidth;
                        size_t  picHeight;
                        size_t  fbHeight;
                        uint32_t linearCurFrameIdx = bwbCurFrameIdx + compressedFbCount;
                        uint8_t mapType = LINEAR_FRAME_MAP;
                        FrameBufferAllocInfo    fbAllocInfo;
                        FrameBufferFormat       outFormat = param->wtlFormat;

                        if (pFbMem[linearCurFrameIdx].size > 0) {
                            vdi_free_dma_memory(coreIdx, &pFbMem[linearCurFrameIdx]);
                            osal_memset((void*)&pFbMem[linearCurFrameIdx], 0x00, sizeof(vpu_buffer_t));
                        }

                        picWidth  = initialInfo.picWidth;
                        picHeight = initialInfo.picHeight;
                        fbHeight  = VPU_ALIGN64(picHeight);
                        linearStride = CalcStride(VPU_ALIGN64(picWidth), picHeight, outFormat, param->cbcrInterleave, (TiledMapType)mapType, TRUE);
                        pFbMem[linearCurFrameIdx].size  = VPU_GetFrameBufSize(coreIdx, linearStride, fbHeight, mapType, outFormat, param->cbcrInterleave, NULL);

                        if (vdi_allocate_dma_memory(coreIdx, &pFbMem[linearCurFrameIdx]) < 0) {
                            VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                            // ReleaseVideoMemory(coreIdx, &pFbMem[linearCurFrameIdx], compressedFbCount);
                            // for (index=0; index<(uint32_t)compressedFbCount; index++) {
                            //     if (pFbMem[index].size)
                            //         vdi_free_dma_memory(coreIdx, &pFbMem[index]);
                            // }
                            goto ERR_DEC_OPEN;
                        }
                        pFrame[linearCurFrameIdx].bufY  = pFbMem[linearCurFrameIdx].phys_addr;
                        pFrame[linearCurFrameIdx].bufCb = (PhysicalAddress)-1;
                        pFrame[linearCurFrameIdx].bufCr = (PhysicalAddress)-1;
                        pFrame[linearCurFrameIdx].updateFbInfo = TRUE;

                        fbAllocInfo.nv21    = param->nv21;
                        fbAllocInfo.cbcrInterleave  = param->cbcrInterleave;
                        fbAllocInfo.format  = outFormat;
                        fbAllocInfo.num     = linearFbCount;
                        fbAllocInfo.mapType = LINEAR_FRAME_MAP;
                        fbAllocInfo.stride  = linearStride;
                        fbAllocInfo.height  = fbHeight;
                        fbAllocInfo.type    = FB_TYPE_CODEC;
                        fbAllocInfo.endian          = param->frameEndian;
                        fbAllocInfo.lumaBitDepth    = initialInfo.lumaBitdepth;
                        fbAllocInfo.chromaBitDepth  = initialInfo.chromaBitdepth;

                        ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, &pFrame[compressedFbCount]);
                        if (ret != RETCODE_SUCCESS) {
                            VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer() ret:%d\n",
                                __FUNCTION__, __LINE__, ret);
                            // ReleaseVideoMemory(coreIdx, pFbMem, linearFbCount+compressedFbCount);
                            goto ERR_DEC_OPEN;
                        }
                    }

                    VPU_DecGiveCommand(handle, DEC_SET_INTER_RES_INFO_ON, NULL);
                    ret = VPU_DecRegisterFrameBufferEx(handle, pFrame, compressedFbCount, linearFbCount,
                        framebufStride, initialInfo.picHeight, COMPRESSED_FRAME_MAP);
                    VPU_DecGiveCommand(handle, DEC_SET_INTER_RES_INFO_OFF, NULL);

                    if (ret != RETCODE_SUCCESS) {
                        success = FALSE;
                        VLOG(ERR, "[SEQ_CHANGE] VPU_DecRegisterFrameBufferEx failed Error code is 0x%x\n", ret);
                        goto ERR_DEC_OPEN;
                    }
                    // To reuse bitstream buffer after sequence changed on VP9
                    /*
                    When detected sequence change on VP9,it's not necessary to load next frame.
                    VP9 can detect sequence change only on KEY_FRAME.
                    So need to reuse same chunk, after udpate frame buffer
                    */
                    if (handle->codecMode == C7_VP9_DEC)
                        needStream = FALSE;
                }
#ifdef SUPPORT_REUSE_BUFFER
skip_realloc:
#endif
                VLOG(INFO, "-----INTER SEQUENCE CHANGE END -----\n");
        }

        if (noFbCount <= 1 && outputInfo.indexFrameDisplay != DISPLAY_IDX_FLAG_SEQ_END) {
            if (sequenceChangeFlag == 0) {
                Queue_Enqueue(displayQ, (void*)&outputInfo);
            }
        }

        repeat = TRUE;
        do {
            if ((pDisplayInfo=(DecOutputInfo*)Queue_Dequeue(displayQ)) == NULL)
            {
                break;
            }

            osal_memcpy((void*)&outputInfo, pDisplayInfo, sizeof(DecOutputInfo));
            DisplayDecodedInformation(handle, decOP.bitstreamFormat, frameIdx, &outputInfo);

            if (outputInfo.indexFrameDecoded < 0 &&
                outputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_NO_FB) {
                    notDecodedCount++;
                    continue;
            }
            break;
        } while (repeat == TRUE);

        if (pDisplayInfo != NULL) {
            if (outputInfo.indexFrameDecoded >= 0 && outputInfo.indexFrameDisplay < 0) {
                if (TRUE == param->pvricFbcEnable && STD_VP9 == decOP.bitstreamFormat) {
                    FrameBuffer fb = pFrame[compressedFbCount+outputInfo.indexFrameDecoded];
                    /* To compare with the golden data. */
                    vdi_clear_memory(0, fb.bufY,  fb.sizeLuma, 0);
                    vdi_clear_memory(0, fb.bufCb, fb.sizeChroma*2, 0);
                }
            }
            if (outputInfo.indexFrameDisplay >= 0) {
                uint32_t    width=0, height=0, Bpp;
                size_t      frameSizeInByte=0;
                uint8_t*    pYuv = NULL;
                void*       decodedData = NULL;
                uint32_t    decodedDataSize = 0;
                VpuRect     rcDisplay;

                rcDisplay.left   = 0;
                rcDisplay.top    = 0;
                rcDisplay.right  = outputInfo.dispPicWidth;
                rcDisplay.bottom = outputInfo.dispPicHeight;
                if (doDumpImage) {
                        if (strlen(param->outputPath) > 0) {
                            if (!filep[0])
                            {
                                if((filep[0] = osal_fopen(param->outputPath,"wb")) == NULL) goto ERR_DEC_OPEN;
                            }

                            {
                                SaveDisplayBufferToFile(handle, outputInfo.dispFrame, rcDisplay, (FILE**)filep, param->outputPath, outputCount);
                                VLOG(INFO,"yuv stored. size: %d\n",outputInfo.dispFrame.size);
                            }
                        }
                        if (param->compareType == YUV_COMPARE || rendererType == RENDER_DEVICE_FBDEV) {
                            pYuv = GetYUVFromFrameBuffer(handle, &outputInfo.dispFrame, rcDisplay, &width, &height, &Bpp, &frameSizeInByte);
                        }
                }
                else {
                    width  = outputInfo.dispPicWidth;
                    height = outputInfo.dispPicHeight;
                }
                outputCount++;

                switch (param->compareType) {
                case NO_COMPARE:
                    break;
                case YUV_COMPARE:
                    decodedData     = (void*)pYuv;
                    decodedDataSize = frameSizeInByte;
                    break;
                }

                repeat = TRUE;
                while(repeat == TRUE)
                {
                    if ((success=Comparator_Act(comparator, decodedData, decodedDataSize)) == TRUE)
                    {
                        break;
                    }
                    EnterLock(coreIdx);
                    HandleDecoderError(handle, frameIdx, param, pFbMem, &outputInfo);
                    LeaveLock(coreIdx);
                    goto ERR_DEC_OPEN;
                }

                /*
                * pYuv is released at the renderer module.
                * SimpleRenderer releases all framebuffer memory of the previous sequence.
                */
                SimpleRenderer_Act(renderer, &outputInfo, pYuv, width, height);
                dispIdx++;
            }

            if (dispIdx > 0 && dispIdx == param->forceOutNum) {
               VLOG(INFO,"exit %d\n",__LINE__);
                break;
            }

            if (outputInfo.indexFrameDecoded >= 0) {
                frameIdx++;
                notDecodedCount = 0;
            }
        }
        if (decOP.bitstreamMode == BS_MODE_PIC_END) {
            needStream = TRUE;
            if (decodedIndex == DECODED_IDX_FLAG_NO_FB) {
                needStream = FALSE;
            }

            //super frame(many frames in one packet)
            if (handle->codecMode == C7_VP9_DEC) {
                VLOG(INFO,"GO %d\n",__LINE__);
                if (handle->CodecInfo->decInfo.streamRdPtr < handle->CodecInfo->decInfo.streamWrPtr)
                    needStream = FALSE;
            }
            if (handle->codecMode == C7_HEVC_DEC ) {
                VLOG(INFO,"GO %d\n",__LINE__);
                if (prescanIndex == -1 || sequenceChangeFlag != 0) {
                    needStream = FALSE;
 //                   VPU_DecSetRdPtr(handle, vbStream[bsQueueIndex].phys_addr, FALSE);
                    VPU_DecSetRdPtrEx(handle, vbStream[bsQueueIndex].phys_addr,vbStream[bsQueueIndex].phys_addr, TRUE);
                }
            }
        }
        if (outputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END) {
            VLOG(INFO,"GO %d\n",__LINE__);
            if (Queue_Peek(displayQ) != NULL) {
                VLOG(ERR, "Queue_Peek=%d\n", Queue_Peek(displayQ));
                success = FALSE;
            }
            if (loopCount > 0) {
                VLOG(INFO,"GO %d\n",__LINE__);
                loopCount--;
                BitstreamFeeder_Rewind(feeder);
                Comparator_Rewind(comparator);
                VPU_DecUpdateBitstreamBuffer(handle, -1);

                /*loop test should release display flag */
                DecOutputInfo   remainings[16];   /* max remainings is 16 */
                uint32_t        num, i;

                VPU_DecFrameBufferFlush(handle, remainings, &num);
                for (i=0; i < num; i++) {
                    VPU_DecClrDispFlag(handle, remainings[i].indexFrameDisplay);
                }

                /* clear all of display indexes that HOST owns. */
                SimpleRenderer_Flush(renderer);

                /* only save yuv file last loop time */
                if (filep[0] != NULL) {
                    osal_fclose(filep[0]);
                    filep[0] = NULL;
                }
            }
            else {
                break;
            }
        }
    }

ERR_DEC_OPEN:
    // Now that we are done with decoding, close the open instance.
    VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SIZE);

    if ( sequenceChangeOccurred == 0 )
    {
        if (success == TRUE && param->compareType && !param->forceOutNum) {
            success = Comparator_CheckFrameCount(comparator);
        }
    }

    /* Release all previous sequence resources */
    if ( handle ) {
        for (index=0; index<MAX_SEQUENCE_MEM_COUNT; index++) {
            ReleasePreviousSequenceResources(handle, seqMemInfo[index].allocFbMem, &seqMemInfo[index].fbInfo);
        }
    }

    /* Release current sequence resources */
    for (index=0; index<MAX_REG_FRAME; index++) {
        if (pFbMem[index].size > 0)
            vdi_free_dma_memory(coreIdx, &pFbMem[index]);
    }

    for (index=0; index<bsBufferCount; index++) {
        vdi_free_dma_memory(coreIdx, &vbStream[index]);
    }

    vdi_free_dma_memory(coreIdx, &vbUserData);

    VPU_DecClose(handle);

    double totalsec = totalUs / 1E6;
    VLOG(INFO, "\nDec End. Tot Frame %d, decNum %d, time consumed %lld us(%.2f sec), %.2f/%.2f\n",
        dispIdx, frameIdx, totalUs, totalsec, (double)dispIdx/totalsec, (double)frameIdx/totalsec);

ERR_DEC_INIT:
    CloseDisplayBufferFile(fpOutput);
    if (feeder != NULL)     BitstreamFeeder_Destroy(feeder);
    if (renderer != NULL)   SimpleRenderer_Destroy(renderer);
    if (comparator != NULL) Comparator_Destroy(comparator);
    if (displayQ != NULL)   Queue_Destroy(displayQ);
    if (sequenceQ != NULL)  Queue_Destroy(sequenceQ);

    if (pusBitCode != NULL) {
        osal_free(pusBitCode);
        pusBitCode = NULL;
    }

    if (filep[0] != NULL) {
        osal_fclose(filep[0]);
        filep[0] = NULL;
    }

    VPU_DeInit(coreIdx);
    param->exitFlag = THREAD_EXIT_SUCCESS;
    VLOG(INFO, "TestDecoderWave exit now ...\n");

    return success;
}
