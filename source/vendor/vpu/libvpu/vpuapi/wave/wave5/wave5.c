#include "product.h"
#include "wave/common/common.h"
#include "wave/common/common_vpuconfig.h"
#include "wave/wave5/wave5.h"
#include "vpuerror.h"
#include "wave/wave5/wave5_regdefine.h"

#define WAVE5_TEMPBUF_OFFSET                (1024*1024)
#define WAVE5_TEMPBUF_SIZE                  (1024*1024)
#define WAVE5_TASK_BUF_OFFSET               (2*1024*1024)   // common mem = | codebuf(1M) | tempBuf(1M) | taskbuf0x0 ~ 0xF |

void Wave5BitIssueCommand(CodecInst* instance, Uint32 cmd)
{
    Uint32 instanceIndex = 0;
    Uint32 codecMode     = 0;
    Uint32 coreIdx;
    if (instance != NULL) {
        instanceIndex = instance->instIndex;
        codecMode     = instance->codecMode;
    }
    coreIdx = instance->coreIdx;

    VpuWriteReg(coreIdx, W5_CMD_INSTANCE_INFO,  (codecMode<<16)|(instanceIndex&0xffff));
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_COMMAND, cmd);

    if ((instance != NULL && instance->loggingEnable))
        vdi_log(coreIdx, cmd, 1);

    VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);
    return;
}
static RetCode SendDecQuery(CodecInst* instance, QUERY_OPT queryOpt)
{
    // Send QUERY cmd
    VpuWriteReg(instance->coreIdx, W5_QUERY_OPTION, queryOpt);
    Wave5BitIssueCommand(instance, W5_DEC_QUERY);
    if (vdi_wait_vpu_busy(instance->coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        if (instance->loggingEnable)
            vdi_log(instance->coreIdx, W5_DEC_QUERY, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE)
        return RETCODE_FAILURE;

    return RETCODE_SUCCESS;

}
static RetCode SetupWave5Properties(Uint32 coreIdx)
{
    VpuAttr*    pAttr = &g_VpuCoreAttributes[coreIdx];
    Uint32      regVal;
    Uint8*      str;
    RetCode     ret = RETCODE_SUCCESS;

    VpuWriteReg(coreIdx, W5_QUERY_OPTION, GET_VPU_INFO);
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_COMMAND, W5_DEC_QUERY);
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);
    if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == FALSE) {
        ret = RETCODE_QUERY_FAILURE;
    }
    else {
        regVal = VpuReadReg(coreIdx, W5_RET_PRODUCT_NAME);
        str    = (Uint8*)&regVal;
        pAttr->productName[0] = str[3];
        pAttr->productName[1] = str[2];
        pAttr->productName[2] = str[1];
        pAttr->productName[3] = str[0];
        pAttr->productName[4] = 0;
        pAttr->productNumber  = VpuReadReg(coreIdx, W5_RET_PRODUCT_VERSION);

        switch (pAttr->productNumber) {
        case WAVE510_CODE:  pAttr->productId = PRODUCT_ID_510; break;
        case WAVE512_CODE:  pAttr->productId = PRODUCT_ID_512; break;
        default:            pAttr->productId = PRODUCT_ID_NONE; break;
        }

        pAttr->hwConfigDef0    = VpuReadReg(coreIdx, W5_RET_STD_DEF0);
        pAttr->hwConfigDef1    = VpuReadReg(coreIdx, W5_RET_STD_DEF1);
        pAttr->hwConfigFeature = VpuReadReg(coreIdx, W5_RET_CONF_FEATURE);
        pAttr->hwConfigDate    = VpuReadReg(coreIdx, W5_RET_CONF_DATE);
        pAttr->hwConfigRev     = VpuReadReg(coreIdx, W5_RET_CONF_REVISION);
        pAttr->hwConfigType    = VpuReadReg(coreIdx, W5_RET_CONF_TYPE);

        pAttr->supportGDIHW          = TRUE;
        pAttr->supportDecoders       = (1<<STD_HEVC);
        if (pAttr->productId == PRODUCT_ID_512) {
            pAttr->supportDecoders       |= (1<<STD_VP9);
        }
        pAttr->supportEncoders       = 0;
        pAttr->supportCommandQueue   = TRUE;

        pAttr->supportFBCBWOptimization = (BOOL)((pAttr->hwConfigDef1>>15)&0x01);
        pAttr->supportWTL            = TRUE;
        pAttr->supportTiled2Linear   = FALSE;
        pAttr->supportMapTypes       = FALSE;
        pAttr->support128bitBus      = TRUE;
        pAttr->supportThumbnailMode  = TRUE;
        pAttr->supportEndianMask     = (Uint32)((1<<VDI_LITTLE_ENDIAN) | (1<<VDI_BIG_ENDIAN) | (1<<VDI_32BIT_LITTLE_ENDIAN) | (1<<VDI_32BIT_BIG_ENDIAN) | (0xffff<<16));
        pAttr->supportBitstreamMode  = (1<<BS_MODE_INTERRUPT) | (1<<BS_MODE_PIC_END);
        pAttr->framebufferCacheType  = FramebufCacheNone;
        pAttr->bitstreamBufferMargin = 0;
        pAttr->numberOfVCores        = MAX_NUM_VCORE;
        pAttr->numberOfMemProtectRgns = 10;
    }

    return ret;
}

RetCode Wave5VpuGetVersion(Uint32 coreIdx, Uint32* versionInfo, Uint32* revision)
{
    Uint32          regVal;

    VpuWriteReg(coreIdx, W5_QUERY_OPTION, GET_VPU_INFO);
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_COMMAND, W5_DEC_QUERY);
    VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);
    if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == FALSE)
        return RETCODE_QUERY_FAILURE;

    regVal = VpuReadReg(coreIdx, W5_RET_FW_VERSION);
    if (versionInfo != NULL) {
        *versionInfo = 0;
    }
    if (revision != NULL) {
        *revision    = regVal;
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuGetProductInfo(Uint32 coreIdx, ProductInfo *productInfo)
{
    /* GET FIRMWARE&HARDWARE INFORMATION */
    VpuWriteReg(coreIdx, W5_QUERY_OPTION, GET_VPU_INFO);
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_COMMAND, W5_DEC_QUERY);
    VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);
    if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(coreIdx, W5_RET_SUCCESS) == FALSE) {
        return RETCODE_QUERY_FAILURE;
    }

    productInfo->fwVersion      = VpuReadReg(coreIdx, W5_RET_FW_VERSION);
    productInfo->productName    = VpuReadReg(coreIdx, W5_RET_PRODUCT_NAME);
    productInfo->productVersion = VpuReadReg(coreIdx, W5_RET_PRODUCT_VERSION);
    productInfo->customerId     = VpuReadReg(coreIdx, W5_RET_CUSTOMER_ID);
    productInfo->stdDef0        = VpuReadReg(coreIdx, W5_RET_STD_DEF0);
    productInfo->stdDef1        = VpuReadReg(coreIdx, W5_RET_STD_DEF1);
    productInfo->confFeature    = VpuReadReg(coreIdx, W5_RET_CONF_FEATURE);
    productInfo->configDate     = VpuReadReg(coreIdx, W5_RET_CONF_DATE);
    productInfo->configRevision = VpuReadReg(coreIdx, W5_RET_CONF_REVISION);
    productInfo->configType     = VpuReadReg(coreIdx, W5_RET_CONF_TYPE);

    productInfo->configVcore[0]  = 0;
    productInfo->configVcore[1]  = 0;
    productInfo->configVcore[2]  = 0;
    productInfo->configVcore[3]  = 0;

    return RETCODE_SUCCESS;
}


RetCode Wave5VpuInit(Uint32 coreIdx, void* firmware, Uint32 size)
{
    vpu_buffer_t    vb;
    PhysicalAddress codeBase, tempBase;
    PhysicalAddress taskBufBase;
    Uint32          codeSize, tempSize;
    Uint32          i, regVal, remapSize;
    Uint32          hwOption    = 0;
    RetCode         ret = RETCODE_SUCCESS;
    CodecInstHeader hdr;

    osal_memset((void *)&hdr, 0x00, sizeof(CodecInstHeader));
    vdi_get_common_memory(coreIdx, &vb);

    codeBase  = vb.phys_addr;
    /* ALIGN TO 4KB */
    codeSize = (WAVE5_MAX_CODE_BUF_SIZE&~0xfff);
    if (codeSize < size*2) {
        return RETCODE_INSUFFICIENT_RESOURCE;
    }

    tempBase = vb.phys_addr + WAVE5_TEMPBUF_OFFSET;
    tempSize = WAVE5_TEMPBUF_SIZE;

    APIDPRINT("\nVPU INIT Start!!!\n");

    VpuWriteMem(coreIdx, codeBase, (unsigned char*)firmware, size*2, VDI_128BIT_LITTLE_ENDIAN);

    vdi_set_bit_firmware_to_pm(coreIdx, (Uint16*)firmware);

    regVal = 0;
    VpuWriteReg(coreIdx, W5_PO_CONF, regVal);

    /* Reset All blocks */
    regVal = 0x7ffffff;
    VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, regVal);    // Reset All blocks
    /* Waiting reset done */

    if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_RESET_STATUS) == -1) {
        APIDPRINT("VPU init(W5_VPU_RESET_REQ) timeout\n");
        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);

    /* clear registers */
    for (i=W5_CMD_REG_BASE; i<W5_CMD_REG_END; i+=4)
        VpuWriteReg(coreIdx, i, 0x00);

    /* remap page size */
    remapSize = (codeSize >> 12) &0x1ff;
    regVal = 0x80000000 | (WAVE5_AXI_ID<<20) | (0 << 16) | (W5_REMAP_CODE_INDEX<<12) | (1<<11) | remapSize;
    VpuWriteReg(coreIdx, W5_VPU_REMAP_CTRL,     regVal);
    VpuWriteReg(coreIdx, W5_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
    VpuWriteReg(coreIdx, W5_VPU_REMAP_PADDR,    codeBase);
    VpuWriteReg(coreIdx, W5_ADDR_CODE_BASE,     codeBase);
    VpuWriteReg(coreIdx, W5_CODE_SIZE,          codeSize);
    VpuWriteReg(coreIdx, W5_CODE_PARAM,         (WAVE5_AXI_ID<<4) | 0);
    VpuWriteReg(coreIdx, W5_ADDR_TEMP_BASE,     tempBase);
    VpuWriteReg(coreIdx, W5_TEMP_SIZE,          tempSize);
    VpuWriteReg(coreIdx, W5_TIMEOUT_CNT, 0xffff);

    VpuWriteReg(coreIdx, W5_HW_OPTION, hwOption);
    /* Interrupt */

    regVal  = (1<<W5_INT_INIT_SEQ);
    regVal |= (1<<W5_INT_DEC_PIC);
    regVal |= (1<<W5_INT_BSBUF_EMPTY);

    VpuWriteReg(coreIdx, W5_VPU_VINT_ENABLE,  regVal);

    VpuWriteReg(coreIdx, W5_CMD_INIT_NUM_TASK_BUF, COMMAND_QUEUE_DEPTH);
    VpuWriteReg(coreIdx, W5_CMD_INIT_TASK_BUF_SIZE, ONE_TASKBUF_SIZE_FOR_CQ);
    vdi_get_common_memory(coreIdx, &vb);
    for (i = 0; i < COMMAND_QUEUE_DEPTH; i++) {
        taskBufBase = vb.phys_addr + WAVE5_TASK_BUF_OFFSET + (i*ONE_TASKBUF_SIZE_FOR_CQ);
        VpuWriteReg(coreIdx, W5_CMD_INIT_ADDR_TASK_BUF0 + (i*4), taskBufBase);
    }

    if (vdi_get_sram_memory(coreIdx, &vb) < 0)  // get SRAM base/size
        return RETCODE_INSUFFICIENT_RESOURCE;

    VpuWriteReg(coreIdx, W5_ADDR_SEC_AXI, vb.phys_addr);
    VpuWriteReg(coreIdx, W5_SEC_AXI_SIZE, vb.size);

    hdr.coreIdx = coreIdx;

    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(coreIdx, W5_COMMAND, W5_INIT_VPU);
    VpuWriteReg(coreIdx, W5_VPU_REMAP_CORE_START, 1);

    if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        APIDPRINT("VPU init(W5_VPU_REMAP_CORE_START) timeout\n");
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
    if (regVal == 0) {
        uint32_t    reasonCode = VpuReadReg(coreIdx, W5_RET_FAIL_REASON);
        APIDPRINT("VPU init(W5_RET_SUCCESS) failed(%d) REASON CODE(%08x)\n", regVal, reasonCode);
        return RETCODE_FAILURE;
    }

    ret = SetupWave5Properties(coreIdx);
    return ret;
}



RetCode Wave5VpuBuildUpDecParam(CodecInst* instance, DecOpenParam* param)
{
    RetCode     ret = RETCODE_SUCCESS;
    DecInfo*    pDecInfo;
    VpuAttr*    pAttr = &g_VpuCoreAttributes[instance->coreIdx];
    Uint32      bsEndian = 0;
    pDecInfo    = VPU_HANDLE_TO_DECINFO(instance);

    pDecInfo->streamRdPtrRegAddr      = W5_RET_DEC_BS_RD_PTR;
    pDecInfo->streamWrPtrRegAddr      = W5_BS_WR_PTR;
    pDecInfo->frameDisplayFlagRegAddr = W5_RET_DEC_DISP_FLAG;
    pDecInfo->currentPC               = W5_VCPU_CUR_PC;
    pDecInfo->busyFlagAddr            = W5_VPU_BUSY_STATUS;
    if ((pAttr->supportDecoders&(1<<param->bitstreamFormat)) == 0)
        return RETCODE_NOT_SUPPORTED_FEATURE;
    pDecInfo->seqChangeMask           = param->bitstreamFormat == STD_HEVC ?
                                        SEQ_CHANGE_ENABLE_ALL_HEVC : SEQ_CHANGE_ENABLE_ALL_VP9;
    pDecInfo->scaleWidth              = 0;
    pDecInfo->scaleHeight             = 0;

    pDecInfo->targetSubLayerId       = HEVC_MAX_SUB_LAYER_ID;

    if (param->vbWork.size > 0) {
        pDecInfo->vbWork = param->vbWork;
        pDecInfo->workBufferAllocExt = TRUE;
        vdi_attach_dma_memory(instance->coreIdx, &param->vbWork);
    }
    else {
        if (instance->productId == PRODUCT_ID_512) {
            pDecInfo->vbWork.size       = WAVE512DEC_WORKBUF_SIZE;
        }
        else {
            pDecInfo->vbWork.size       = WAVE510DEC_WORKBUF_SIZE;
        }
        pDecInfo->workBufferAllocExt    = FALSE;
        if (vdi_allocate_dma_memory(instance->coreIdx, &pDecInfo->vbWork) < 0) {
            pDecInfo->vbWork.base       = 0;
            pDecInfo->vbWork.phys_addr  = 0;
            pDecInfo->vbWork.size       = 0;
            pDecInfo->vbWork.virt_addr  = 0;
            return RETCODE_INSUFFICIENT_RESOURCE;
        }
    }

    vdi_clear_memory(instance->coreIdx, pDecInfo->vbWork.phys_addr, pDecInfo->vbWork.size, 0);

    VpuWriteReg(instance->coreIdx, W5_ADDR_WORK_BASE, pDecInfo->vbWork.phys_addr);
    VpuWriteReg(instance->coreIdx, W5_WORK_SIZE,      pDecInfo->vbWork.size);

    VpuWriteReg(instance->coreIdx, W5_CMD_BS_START_ADDR, pDecInfo->streamBufStartAddr);
    VpuWriteReg(instance->coreIdx, W5_CMD_BS_SIZE, pDecInfo->streamBufSize);

    bsEndian = vdi_convert_endian(instance->coreIdx, param->streamEndian);
    bsEndian = (bsEndian&VDI_128BIT_ENDIAN_MASK);
    VpuWriteReg(instance->coreIdx, W5_CMD_BS_PARAM, bsEndian);

    VpuWriteReg(instance->coreIdx, W5_VPU_BUSY_STATUS, 1);
    VpuWriteReg(instance->coreIdx, W5_RET_SUCCESS, 0);	//for debug


    Wave5BitIssueCommand(instance, W5_CREATE_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)
            vdi_log(instance->coreIdx, W5_CREATE_INSTANCE, 2);
        vdi_free_dma_memory(instance->coreIdx, &pDecInfo->vbWork);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding into VCPU QUEUE
        vdi_free_dma_memory(instance->coreIdx, &pDecInfo->vbWork);
        ret = RETCODE_FAILURE;
    }

    return ret;
}

RetCode Wave5VpuDecInitSeq(CodecInst* instance)
{
    RetCode     ret = RETCODE_SUCCESS;
    DecInfo*    pDecInfo;
    Uint32      cmdOption = INIT_SEQ_NORMAL, bsOption;

    if (instance == NULL)
        return RETCODE_INVALID_PARAM;

    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);
    if (pDecInfo->thumbnailMode)
        cmdOption = INIT_SEQ_W_THUMBNAIL;


    /* Set attributes of bitstream buffer controller */
    bsOption = 0;
    switch (pDecInfo->openParam.bitstreamMode) {
    case BS_MODE_INTERRUPT:
        if(pDecInfo->seqInitEscape == TRUE)
            bsOption = BSOPTION_ENABLE_EXPLICIT_END;
        break;
    case BS_MODE_PIC_END:
        bsOption = BSOPTION_ENABLE_EXPLICIT_END;
        break;
    default:
        return RETCODE_INVALID_PARAM;
    }

    if (pDecInfo->streamEndflag == 1)
        bsOption = 3;

    VpuWriteReg(instance->coreIdx, W5_BS_RD_PTR, pDecInfo->streamRdPtr);
    VpuWriteReg(instance->coreIdx, W5_BS_WR_PTR, pDecInfo->streamWrPtr);

    VpuWriteReg(instance->coreIdx, W5_BS_OPTION, (1<<31) | bsOption);

    VpuWriteReg(instance->coreIdx, W5_COMMAND_OPTION, cmdOption);
    Wave5BitIssueCommand(instance, W5_INIT_SEQ);

    if (vdi_wait_vpu_busy(instance->coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)
            vdi_log(instance->coreIdx, W5_INIT_SEQ, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding a command into VCPU QUEUE
        if (VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON) == 1)
            ret = RETCODE_QUEUEING_FAILURE;
        else
            ret = RETCODE_FAILURE;
    }

    return ret;
}

static void GetSequenceResult(CodecInst* instance, DecInitialInfo* info)
{
    DecInfo*   pDecInfo   = &instance->CodecInfo->decInfo;
    Uint32     regVal;
    Uint32     profileCompatibilityFlag;
    Uint32     left, right, top, bottom;

    info->rdPtr = VpuReadReg(instance->coreIdx, W5_RET_DEC_BS_RD_PTR);
    //info->wrPtr = VpuReadReg(instance->coreIdx, W4_BS_WR_PTR);

    pDecInfo->streamRdPtr      = VpuReadReg(instance->coreIdx, W5_RET_DEC_BS_RD_PTR);
    pDecInfo->frameDisplayFlag = VpuReadReg(instance->coreIdx, W5_RET_DEC_DISP_FLAG);
    /*regVal = VpuReadReg(instance->coreIdx, W4_BS_OPTION);
    pDecInfo->streamEndflag    = (regVal&0x02) ? TRUE : FALSE;*/

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_SIZE);
    info->picWidth            = ( (regVal >> 16) & 0xffff );
    info->picHeight           = ( regVal & 0xffff );
    info->minFrameBufferCount = VpuReadReg(instance->coreIdx, W5_RET_DEC_FRAMEBUF_NEEDED);
    info->frameBufDelay       = VpuReadReg(instance->coreIdx, W5_RET_DEC_NUM_REORDER_DELAY);

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_CROP_LEFT_RIGHT);
    left   = (regVal >> 16) & 0xffff;
    right  = regVal & 0xffff;
    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_CROP_TOP_BOTTOM);
    top    = (regVal >> 16) & 0xffff;
    bottom = regVal & 0xffff;

    info->picCropRect.left   = left;
    info->picCropRect.right  = info->picWidth - right;
    info->picCropRect.top    = top;
    info->picCropRect.bottom = info->picHeight - bottom;

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_SEQ_PARAM);
    profileCompatibilityFlag     = (regVal>>12)&0xff;
    info->profile                = (regVal >> 24)&0x1f;
    info->level                  = regVal & 0xff;
    info->tier                   = (regVal >> 29)&0x01;
    info->maxSubLayers           = (regVal >> 21)&0x07;
    info->fRateNumerator         = VpuReadReg(instance->coreIdx, W5_RET_DEC_FRAME_RATE_NR);
    info->fRateDenominator       = VpuReadReg(instance->coreIdx, W5_RET_DEC_FRAME_RATE_DR);
    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_COLOR_SAMPLE_INFO);
    info->chromaFormatIDC        = (regVal>>8)&0x0f;
    info->lumaBitdepth           = (regVal>>0)&0x0f;
    info->chromaBitdepth         = (regVal>>4)&0x0f;
    info->aspectRateInfo         = (regVal>>16)&0xff;
    info->isExtSAR               = (info->aspectRateInfo == 255 ? TRUE : FALSE);
    if (info->isExtSAR == TRUE) {
        info->aspectRateInfo     = VpuReadReg(instance->coreIdx, W5_RET_DEC_ASPECT_RATIO);  /* [0:15] - vertical size, [16:31] - horizontal size */
    }
    info->bitRate                = VpuReadReg(instance->coreIdx, W5_RET_DEC_BIT_RATE);

    if ( instance->codecMode == C7_HEVC_DEC ) {
        /* Guessing Profile */
        if (info->profile == 0) {
            if ((profileCompatibilityFlag&0x06) == 0x06)        info->profile = 1;      /* Main profile */
            else if ((profileCompatibilityFlag&0x04) == 0x04)   info->profile = 2;      /* Main10 profile */
            else if ((profileCompatibilityFlag&0x08) == 0x08)   info->profile = 3;      /* Main Still Picture profile */
            else                                                info->profile = 1;      /* For old version HM */
        }
    }

    return;
}

RetCode Wave5VpuDecGetSeqInfo(CodecInst* instance, DecInitialInfo* info)
{
    RetCode     ret = RETCODE_SUCCESS;
    Uint32      regVal, i;
    DecInfo*    pDecInfo;

    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);

    // Send QUERY cmd
    ret = SendDecQuery(instance, GET_RESULT);
    if (ret != RETCODE_SUCCESS) {
        return RETCODE_QUERY_FAILURE;
    }

    if (instance->loggingEnable)
        vdi_log(instance->coreIdx, W5_INIT_SEQ, 0);

    if (VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODING_SUCCESS) != 1) {
        info->seqInitErrReason = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_INFO);
        if (info->seqInitErrReason == WAVE5_SYSERR_ACCESS_VIOLATION_HW)
            ret = RETCODE_MEMORY_ACCESS_VIOLATION;
        else
            ret = RETCODE_FAILURE;
    }

    // Get Sequence Info
    info->userDataSize   = 0;
    info->userDataNum    = 0;
    info->userDataHeader = VpuReadReg(instance->coreIdx, W5_RET_DEC_USERDATA_IDC);
    if (info->userDataHeader != 0) {
        regVal = info->userDataHeader;
        for (i=0; i<32; i++) {
            if (regVal & (1<<i)) {
                info->userDataNum++;
            }
        }
        info->userDataSize = pDecInfo->userDataBufSize;
    }

    regVal = VpuReadReg(instance->coreIdx, W5_RET_INSTANCE_INFO);

    GetSequenceResult(instance, info);

    return ret;
}

RetCode Wave5VpuDecRegisterFramebuffer(CodecInst* inst, FrameBuffer* fbArr, TiledMapType mapType, Uint32 count)
{
    RetCode      ret = RETCODE_SUCCESS;
    DecInfo*     pDecInfo = &inst->CodecInfo->decInfo;
    DecInitialInfo* sequenceInfo = &inst->CodecInfo->decInfo.initialInfo;
    Int32        q, j, i, remain, idx;
    Uint32 mvCount;
    Uint32       k;
    Int32        coreIdx, startNo, endNo;
    Uint32       regVal, cbcrInterleave, nv21;
    Uint32       endian, yuvFormat = 0;
    Uint32       addrY, addrCb, addrCr;
    Uint32       mvColSize, fbcYTblSize, fbcCTblSize;
    vpu_buffer_t vbBuffer;
    uint32_t     stride;
    uint32_t     colorFormat  = 0;
    uint32_t     outputFormat = 0;
    uint32_t     axiID;
    int pixel_order;

    pixel_order = 1;


    coreIdx        = inst->coreIdx;
    axiID          = pDecInfo->openParam.virtAxiID;
    cbcrInterleave = pDecInfo->openParam.cbcrInterleave;
    nv21           = pDecInfo->openParam.nv21;
    mvColSize      = fbcYTblSize = fbcCTblSize = 0;
    if (mapType == COMPRESSED_FRAME_MAP) {
        cbcrInterleave = 0;
        nv21           = 0;

        if (inst->codecMode == C7_HEVC_DEC) {
            mvColSize          = WAVE4_DEC_HEVC_MVCOL_BUF_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        }
        else if(inst->codecMode == C7_VP9_DEC) {
            mvColSize          = WAVE4_DEC_VP9_MVCOL_BUF_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        }
        else {
            /* Unknown codec */
            return RETCODE_NOT_SUPPORTED_FEATURE;
        }

        mvColSize          = VPU_ALIGN16(mvColSize);
        vbBuffer.phys_addr = 0;
        if (inst->codecMode == HEVC_DEC || inst->codecMode == C7_HEVC_DEC || inst->codecMode == C7_VP9_DEC) {
            vbBuffer.size      = ((mvColSize+4095)&~4095)+4096;   /* 4096 is a margin */
            mvCount = count;

            for (k=0  ; k<mvCount ; k++) {
                if ( pDecInfo->vbMV[k].size == 0) {
                    if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer) < 0)
                        return RETCODE_INSUFFICIENT_RESOURCE;
                    pDecInfo->vbMV[k] = vbBuffer;
                }
            }
        }

        //VP9 Decoded size : 64 aligned.
        if (inst->codecMode == C7_HEVC_DEC){
            fbcYTblSize        = WAVE4_FBC_LUMA_TABLE_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        }
        else if (inst->codecMode == C7_VP9_DEC) {
            fbcYTblSize        = WAVE4_FBC_LUMA_TABLE_SIZE(VPU_ALIGN64(pDecInfo->initialInfo.picWidth), VPU_ALIGN64(pDecInfo->initialInfo.picHeight));
        }
        else {
            /* Unknown codec */
            return RETCODE_NOT_SUPPORTED_FEATURE;
        }

        fbcYTblSize        = VPU_ALIGN16(fbcYTblSize);
        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((fbcYTblSize+4095)&~4095)+4096;
        for (k=0  ; k<count ; k++) {
            if ( pDecInfo->vbFbcYTbl[k].size == 0) {
                if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer) < 0)
                    return RETCODE_INSUFFICIENT_RESOURCE;
                pDecInfo->vbFbcYTbl[k] = vbBuffer;
            }
        }

        if (inst->codecMode == C7_HEVC_DEC) {
            fbcCTblSize        = WAVE4_FBC_CHROMA_TABLE_SIZE(pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight);
        }
        else if (inst->codecMode == C7_VP9_DEC) {
            fbcCTblSize        = WAVE4_FBC_CHROMA_TABLE_SIZE(VPU_ALIGN64(pDecInfo->initialInfo.picWidth), VPU_ALIGN64(pDecInfo->initialInfo.picHeight));
        }
        else {
            /* Unknown codec */
            return RETCODE_NOT_SUPPORTED_FEATURE;
        }

        fbcCTblSize        = VPU_ALIGN16(fbcCTblSize);
        vbBuffer.phys_addr = 0;
        vbBuffer.size      = ((fbcCTblSize+4095)&~4095)+4096;
        for (k=0  ; k<count ; k++) {
            if ( pDecInfo->vbFbcCTbl[k].size == 0) {
                if (vdi_allocate_dma_memory(inst->coreIdx, &vbBuffer) < 0)
                    return RETCODE_INSUFFICIENT_RESOURCE;
                pDecInfo->vbFbcCTbl[k] = vbBuffer;
            }
        }
    }
    endian = vdi_convert_endian(coreIdx, fbArr[0].endian) & VDI_128BIT_ENDIAN_MASK;
    //Convert API endian value to HW endian value
    endian = ~endian & 0xf;

    regVal = (pDecInfo->initialInfo.picWidth<<16)|(pDecInfo->initialInfo.picHeight);
    VpuWriteReg(coreIdx, W5_PIC_SIZE, regVal);

    regVal = (pDecInfo->scaleWidth << 16) | (pDecInfo->scaleHeight);
    VpuWriteReg(coreIdx, W5_SCL_OUT_SIZE, regVal);

    yuvFormat = 0; /* YUV420 8bit */
    if (mapType == LINEAR_FRAME_MAP) {
        BOOL   justified = W4_WTL_RIGHT_JUSTIFIED;
        Uint32 formatNo  = W4_WTL_PIXEL_8BIT;
        switch (pDecInfo->wtlFormat) {
        case FORMAT_420_P10_16BIT_MSB:
        case FORMAT_422_P10_16BIT_MSB:
        case FORMAT_YUYV_P10_16BIT_MSB:
        case FORMAT_YVYU_P10_16BIT_MSB:
        case FORMAT_UYVY_P10_16BIT_MSB:
        case FORMAT_VYUY_P10_16BIT_MSB:
            justified = W4_WTL_RIGHT_JUSTIFIED;
            formatNo  = W4_WTL_PIXEL_16BIT;
            break;
        case FORMAT_420_P10_16BIT_LSB:
        case FORMAT_422_P10_16BIT_LSB:
        case FORMAT_YUYV_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_16BIT_LSB:
        case FORMAT_UYVY_P10_16BIT_LSB:
        case FORMAT_VYUY_P10_16BIT_LSB:
            justified = W4_WTL_LEFT_JUSTIFIED;
            formatNo  = W4_WTL_PIXEL_16BIT;
            break;
        case FORMAT_420_P10_32BIT_MSB:
        case FORMAT_422_P10_32BIT_MSB:
        case FORMAT_YUYV_P10_32BIT_MSB:
        case FORMAT_UYVY_P10_32BIT_MSB:
        case FORMAT_VYUY_P10_32BIT_MSB:
        case FORMAT_YVYU_P10_32BIT_MSB:
            justified = W4_WTL_RIGHT_JUSTIFIED;
            formatNo  = W4_WTL_PIXEL_32BIT;
            break;
        case FORMAT_420_P10_32BIT_LSB:
        case FORMAT_422_P10_32BIT_LSB:
        case FORMAT_YUYV_P10_32BIT_LSB:
        case FORMAT_UYVY_P10_32BIT_LSB:
        case FORMAT_VYUY_P10_32BIT_LSB:
        case FORMAT_YVYU_P10_32BIT_LSB:
            justified = W4_WTL_LEFT_JUSTIFIED;
            formatNo  = W4_WTL_PIXEL_32BIT;
            break;
        default:
            break;
        }
        yuvFormat = justified<<2 | formatNo;
    }

    stride = fbArr[0].stride;
    if (mapType == COMPRESSED_FRAME_MAP) {
        if ( pDecInfo->chFbcFrameIdx != -1 )
            stride = fbArr[pDecInfo->chFbcFrameIdx].stride;
    } else {
        if ( pDecInfo->chBwbFrameIdx != -1 )
            stride = fbArr[pDecInfo->chBwbFrameIdx].stride;
    }

    if (mapType == LINEAR_FRAME_MAP) {
        switch (pDecInfo->wtlFormat) {
        case FORMAT_422:
        case FORMAT_422_P10_16BIT_MSB:
        case FORMAT_422_P10_16BIT_LSB:
        case FORMAT_422_P10_32BIT_MSB:
        case FORMAT_422_P10_32BIT_LSB:
            colorFormat   = 1;
            outputFormat  = 0;
            outputFormat |= (nv21 << 1);
            outputFormat |= (cbcrInterleave << 0);
            break;
        case FORMAT_YUYV:
        case FORMAT_YUYV_P10_16BIT_MSB:
        case FORMAT_YUYV_P10_16BIT_LSB:
        case FORMAT_YUYV_P10_32BIT_LSB:
        case FORMAT_YUYV_P10_32BIT_MSB:
            colorFormat   = 1;
            outputFormat  = 4;
            break;
        case FORMAT_YVYU:
        case FORMAT_YVYU_P10_16BIT_MSB:
        case FORMAT_YVYU_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_32BIT_MSB:
        case FORMAT_YVYU_P10_32BIT_LSB:
            colorFormat   = 1;
            outputFormat  = 6;
            break;
        case FORMAT_UYVY:
        case FORMAT_UYVY_P10_32BIT_LSB:
        case FORMAT_UYVY_P10_16BIT_LSB:
        case FORMAT_UYVY_P10_16BIT_MSB:
        case FORMAT_UYVY_P10_32BIT_MSB:
            colorFormat   = 1;
            outputFormat  = 5;
            break;
        case FORMAT_VYUY:
        case FORMAT_VYUY_P10_32BIT_MSB:
        case FORMAT_VYUY_P10_32BIT_LSB:
        case FORMAT_VYUY_P10_16BIT_MSB:
        case FORMAT_VYUY_P10_16BIT_LSB:
            colorFormat   = 1;
            outputFormat  = 7;
            break;
        default:
            colorFormat   = 0;
            outputFormat  = 0;
            outputFormat |= (nv21 << 1);
            outputFormat |= (cbcrInterleave << 0);
            break;
        }
        pDecInfo->scalerEnable = TRUE;
    }



    regVal =
        (pDecInfo->scalerEnable       << 29)    |
        ((mapType == LINEAR_FRAME_MAP) << 28)   |
        (axiID << 24)                           |
        (pixel_order << 23)                     |
        (yuvFormat     << 20)                   |
        (colorFormat  << 19)                    |
        (outputFormat << 16)                    |
        (stride);

    VpuWriteReg(coreIdx, W5_COMMON_PIC_INFO, regVal);

    if(pDecInfo->interResChange)
    {
        i=0;
        regVal = ((pDecInfo->openParam.fbc_mode<<20)|(endian<<16) | (1)<<4 | ((1)<<3) | 3) ;
        VpuWriteReg(coreIdx, W5_SFB_OPTION, regVal);
        if (mapType == COMPRESSED_FRAME_MAP) {
            idx=pDecInfo->chFbcFrameIdx;
        } else {
            idx=pDecInfo->chBwbFrameIdx;
        }
        VpuWriteReg(coreIdx, W5_SET_FB_NUM, (idx<<8)|idx);
        if (mapType == LINEAR_FRAME_MAP && pDecInfo->openParam.cbcrOrder == CBCR_ORDER_REVERSED) {
            addrY  = fbArr[idx].bufY;
            addrCb = fbArr[idx].bufCr;
            addrCr = fbArr[idx].bufCb;
        }
        else {
            addrY  = fbArr[idx].bufY;
            addrCb = fbArr[idx].bufCb;
            addrCr = fbArr[idx].bufCr;
        }
        VpuWriteReg(coreIdx, W5_ADDR_LUMA_BASE0  + (i<<4), addrY);
        VpuWriteReg(coreIdx, W5_ADDR_CB_BASE0    + (i<<4), addrCb);
        APIDPRINT("REGISTER FB[%02d] Y(0x%08x), Cb(0x%08x) ", i, addrY, addrCb);
        if (mapType == COMPRESSED_FRAME_MAP) {
            VpuWriteReg(coreIdx, W5_ADDR_FBC_Y_OFFSET0 + (i<<4), pDecInfo->vbFbcYTbl[idx].phys_addr); /* Luma FBC offset table */
            VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET0 + (i<<4), pDecInfo->vbFbcCTbl[idx].phys_addr);        /* Chroma FBC offset table */
            VpuWriteReg(coreIdx, W5_ADDR_MV_COL0  + (i<<2), pDecInfo->vbMV[idx].phys_addr);
            APIDPRINT("Yo(0x%08" PRIx64 ") Co(0x%08" PRIx64 "), Mv(0x%08" PRIx64 ")",
                pDecInfo->vbFbcYTbl[idx].phys_addr,
                pDecInfo->vbFbcCTbl[idx].phys_addr,
                pDecInfo->vbMV[idx].phys_addr);
        }
        else {
            VpuWriteReg(coreIdx, W5_ADDR_CR_BASE0 + (i<<4), addrCr);
            VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET0 + (i<<4), 0);
            VpuWriteReg(coreIdx, W5_ADDR_MV_COL0  + (i<<2), 0);
            APIDPRINT("Cr(0x%08x)\n", addrCr);
        }

        Wave5BitIssueCommand(inst, W5_SET_FB);
        if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
    }
    else
    {
        remain = count;
        q      = (remain+7)/8;
        idx    = 0;
        for (j=0; j<q; j++) {
            regVal = (pDecInfo->openParam.fbc_mode<<20)|(endian<<16) | (j==q-1)<<4 | ((j==0)<<3) ;
            VpuWriteReg(coreIdx, W5_SFB_OPTION, regVal);
            startNo = j*8;
            endNo   = startNo + (remain>=8 ? 8 : remain) - 1;

            VpuWriteReg(coreIdx, W5_SET_FB_NUM, (startNo<<8)|endNo);

            for (i=0; i<8 && i<remain; i++) {
                if (mapType == LINEAR_FRAME_MAP && pDecInfo->openParam.cbcrOrder == CBCR_ORDER_REVERSED) {
                    addrY  = fbArr[i+startNo].bufY;
                    addrCb = fbArr[i+startNo].bufCr;
                    addrCr = fbArr[i+startNo].bufCb;
                }
                else {
                    addrY  = fbArr[i+startNo].bufY;
                    addrCb = fbArr[i+startNo].bufCb;
                    addrCr = fbArr[i+startNo].bufCr;
                }
                VpuWriteReg(coreIdx, W5_ADDR_LUMA_BASE0  + (i<<4), addrY);
                VpuWriteReg(coreIdx, W5_ADDR_CB_BASE0    + (i<<4), addrCb);
                APIDPRINT("REGISTER FB[%02d] Y(0x%08x), Cb(0x%08x) ", i, addrY, addrCb);
                if (mapType == COMPRESSED_FRAME_MAP) {
                    VpuWriteReg(coreIdx, W5_ADDR_FBC_Y_OFFSET0 + (i<<4), pDecInfo->vbFbcYTbl[idx].phys_addr); /* Luma FBC offset table */
                    VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET0 + (i<<4), pDecInfo->vbFbcCTbl[idx].phys_addr);        /* Chroma FBC offset table */
                    VpuWriteReg(coreIdx, W5_ADDR_MV_COL0  + (i<<2), pDecInfo->vbMV[idx].phys_addr);
                    APIDPRINT("Yo(0x%08" PRIx64 ") Co(0x%08" PRIx64 "), Mv(0x%08" PRIx64 ")",
                        pDecInfo->vbFbcYTbl[idx].phys_addr,
                        pDecInfo->vbFbcCTbl[idx].phys_addr,
                        pDecInfo->vbMV[idx].phys_addr);
                }
                else {
                    VpuWriteReg(coreIdx, W5_ADDR_CR_BASE0 + (i<<4), addrCr);
                    VpuWriteReg(coreIdx, W5_ADDR_FBC_C_OFFSET0 + (i<<4), 0);
                    VpuWriteReg(coreIdx, W5_ADDR_MV_COL0  + (i<<2), 0);
                    APIDPRINT("Cr(0x%08x)\n", addrCr);
                }
                idx++;
            }
            remain -= i;

            Wave5BitIssueCommand(inst, W5_SET_FB);
            if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
                return RETCODE_VPU_RESPONSE_TIMEOUT;
            }
        }
    }


    regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
    if (regVal == 0) {
        return RETCODE_FAILURE;
    }

    if (ConfigSecAXIWave(coreIdx, pDecInfo->openParam.bitstreamFormat,
        &pDecInfo->secAxiInfo, pDecInfo->initialInfo.picWidth, pDecInfo->initialInfo.picHeight,
        sequenceInfo->profile, sequenceInfo->level) == 0) {
            return RETCODE_INSUFFICIENT_RESOURCE;
    }

    return ret;
}

RetCode Wave5VpuDecode(CodecInst* instance, DecParam* option)
{
    uint32_t    modeOption = DEC_PIC_NORMAL,  bsOption, regVal;
    DecOpenParam*   pOpenParam;
    int32_t     forceLatency = -1;
    int32_t     rdptr_valid = 0;
    DecInfo*    pDecInfo = &instance->CodecInfo->decInfo;

    pOpenParam = &pDecInfo->openParam;

    if (pDecInfo->thumbnailMode) {
        modeOption = DEC_PIC_W_THUMBNAIL;
    }
    else if (option->skipframeMode) {
        switch (option->skipframeMode) {
        case 1:
            modeOption   = SKIP_NON_IRAP;
            forceLatency = 0;
            break;
        case 2:
            modeOption = SKIP_NON_REF_PIC;
            break;
        default:
            // skip off
            break;
        }
    }
    if (pDecInfo->targetSubLayerId < (pDecInfo->initialInfo.maxSubLayers-1)) {
        modeOption = SKIP_TEMPORAL_LAYER;
    }
    if (option->craAsBlaFlag == TRUE) {
        modeOption |= (1<<1);
    }

    // set disable reorder
    if (pDecInfo->reorderEnable == FALSE) {
        forceLatency = 0;
    }
    // Bandwidth optimization
    modeOption |= (pDecInfo->openParam.bwOptimization<< 31);

    /* Set attributes of bitstream buffer controller */
    bsOption = 0;
    regVal = 0;
    switch (pOpenParam->bitstreamMode) {
    case BS_MODE_INTERRUPT:
        bsOption = 0;
        break;
    case BS_MODE_PIC_END:
        bsOption = BSOPTION_ENABLE_EXPLICIT_END;
        break;
    default:
        return RETCODE_INVALID_PARAM;
    }

    VpuWriteReg(instance->coreIdx, W5_BS_RD_PTR,     pDecInfo->streamRdPtr);
    VpuWriteReg(instance->coreIdx, W5_BS_WR_PTR,     pDecInfo->streamWrPtr);
    if (pDecInfo->streamEndflag == 1)
        bsOption = 3;   // (streamEndFlag<<1) | EXPLICIT_END

    if (pOpenParam->bitstreamMode == BS_MODE_PIC_END || pDecInfo->rdPtrValidFlag == TRUE)
        rdptr_valid = 1;

    VpuWriteReg(instance->coreIdx, W5_BS_OPTION,  (rdptr_valid<<31) | bsOption);

    pDecInfo->rdPtrValidFlag = FALSE;       // reset rdptrValidFlag.

    /* Secondary AXI */
    regVal = (pDecInfo->secAxiInfo.u.wave4.useBitEnable<<0)    |
             (pDecInfo->secAxiInfo.u.wave4.useIpEnable<<9)     |
             (pDecInfo->secAxiInfo.u.wave4.useLfRowEnable<<15);
    regVal |= (pDecInfo->secAxiInfo.u.wave4.useSclEnable<<5);
    if (pDecInfo->secAxiInfo.u.wave4.useSclEnable == TRUE) {
        switch (pDecInfo->wtlFormat) {
        case FORMAT_YUYV:
        case FORMAT_YVYU:
        case FORMAT_UYVY:
        case FORMAT_VYUY:
            regVal |= (pDecInfo->secAxiInfo.u.wave4.useSclPackedModeEnable<<6);
            break;
        default:
            break;
        }
    }
    VpuWriteReg(instance->coreIdx, W5_USE_SEC_AXI,  regVal);

    /* Set attributes of User buffer */
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_USER_MASK,      pDecInfo->userDataEnable);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_ADDR_USER_BASE, pDecInfo->userDataBufAddr);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_USER_SIZE,      pDecInfo->userDataBufSize);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_USER_PARAM,     VPU_USER_DATA_ENDIAN&VDI_128BIT_ENDIAN_MASK);

    /** Configure CU data report */
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_ADDR_REPORT_BASE, pDecInfo->cuDataBufAddr);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_REPORT_SIZE,      pDecInfo->cuDataBufSize);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_REPORT_PARAM,     (pDecInfo->cuDataEnable<<31 | (VDI_128BIT_LE_WORD_BYTE_SWAP&VDI_128BIT_ENDIAN_MASK)));

    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_VCORE_LIMIT, 1);

    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_TEMPORAL_ID_PLUS1, pDecInfo->targetSubLayerId+1);
    VpuWriteReg(instance->coreIdx, W5_CMD_SEQ_CHANGE_ENABLE_FLAG, pDecInfo->seqChangeMask);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_FORCE_FB_LATENCY_PLUS1, forceLatency+1);
    VpuWriteReg(instance->coreIdx, W5_COMMAND_OPTION, modeOption);
    Wave5BitIssueCommand(instance, W5_DEC_PIC);

    if (vdi_wait_vpu_busy(instance->coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {   // Check QUEUE_DONE
        if (instance->loggingEnable)
            vdi_log(instance->coreIdx, W5_DEC_PIC, 2);
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE) {           // FAILED for adding a command into VCPU QUEUE
        if (VpuReadReg(instance->coreIdx, W5_RET_FAIL_REASON) == 1)
            return RETCODE_QUEUEING_FAILURE;
        else
            return RETCODE_FAILURE;
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuDecGetResult(CodecInst* instance, DecOutputInfo* result)
{
    RetCode     ret = RETCODE_SUCCESS;
    Uint32      regVal, index, nalUnitType;
    DecInfo*    pDecInfo;

    pDecInfo = VPU_HANDLE_TO_DECINFO(instance);

    // Send QUERY cmd
    ret = SendDecQuery(instance, GET_RESULT);
    if (ret != RETCODE_SUCCESS)
        return RETCODE_QUERY_FAILURE;

    if (instance->loggingEnable)
        vdi_log(instance->coreIdx, W5_DEC_PIC, 0);

    result->decodingSuccess = VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODING_SUCCESS);
    if (result->decodingSuccess == FALSE) {
        result->errorReason = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_INFO);
        if (result->errorReason == WAVE5_SYSERR_ACCESS_VIOLATION_HW) {
            return RETCODE_MEMORY_ACCESS_VIOLATION;
        }
        else if (result->errorReason == WAVE5_CODEC_ERROR) {
            result->errorReasonExt = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_INFO);
        }
    }
    else {
        result->warnInfo = VpuReadReg(instance->coreIdx, W5_RET_DEC_WARN_INFO);
    }

    result->decOutputExtData.userDataSize   = 0;
    result->decOutputExtData.userDataNum    = 0;
    result->decOutputExtData.userDataHeader = VpuReadReg(instance->coreIdx, W5_RET_DEC_USERDATA_IDC);
    if (result->decOutputExtData.userDataHeader != 0) {
        regVal = result->decOutputExtData.userDataHeader;
        for (index=0; index<32; index++) {
            if (regVal & (1<<index)) {
                result->decOutputExtData.userDataNum++;
            }
        }
        result->decOutputExtData.userDataSize = pDecInfo->userDataBufSize;
    }
    result->frameCycle      = VpuReadReg(instance->coreIdx, W5_FRAME_CYCLE);
    result->seekCycle       = VpuReadReg(instance->coreIdx, W5_RET_DEC_SEEK_CYCLE);
    result->parseCycle      = VpuReadReg(instance->coreIdx, W5_RET_DEC_PARSING_CYCLE);
    result->decodeCycle     = VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODING_CYCLE);


    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_TYPE);

    if (regVal&0x04)      result->picType = PIC_TYPE_B;
    else if (regVal&0x02) result->picType = PIC_TYPE_P;
    else if (regVal&0x01) result->picType = PIC_TYPE_I;
    else                  result->picType = PIC_TYPE_MAX;

    nalUnitType = (regVal & 0x3f0) >> 4;
    if ((nalUnitType == 19 || nalUnitType == 20) && result->picType == PIC_TYPE_I) {
        /* IDR_W_RADL, IDR_N_LP */
        result->picType = PIC_TYPE_IDR;
    }
    result->nalType                   = nalUnitType;
    result->h265Info.ctuSize          = 16<<((regVal>>10)&0x3);
    index                             = VpuReadReg(instance->coreIdx, W5_RET_DEC_DISPLAY_INDEX);
    result->indexFrameDisplay         = index;
    result->indexFrameDisplayForTiled = index;
    index                             = VpuReadReg(instance->coreIdx, W5_RET_DEC_DECODED_INDEX);
    result->indexFrameDecoded         = index;
    result->indexFrameDecodedForTiled = index;

    if (instance->codecMode != C7_VP9_DEC) {
        result->h265Info.decodedPOC = -1;
        result->h265Info.displayPOC = -1;
        if (result->indexFrameDecoded >= 0)
            result->h265Info.decodedPOC = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_POC);
        result->h265Info.temporalId = VpuReadReg(instance->coreIdx, W5_RET_DEC_SUB_LAYER_INFO) & 0xff;
    }

    result->sequenceChanged   = VpuReadReg(instance->coreIdx, W5_RET_DEC_NOTIFICATION);
    /*
     * If current picture is the last of the current sequence and sequence-change flag is not 0, then
     * the width and height of the current picture is set to the width and height of the current sequence.
     */
    if (result->sequenceChanged == 0) {
        regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_SIZE);
        result->decPicWidth   = regVal>>16;
        result->decPicHeight  = regVal&0xffff;
    }
    else {
        if (result->indexFrameDecoded < 0) {
            result->decPicWidth   = 0;
            result->decPicHeight  = 0;
        }
        else {
            result->decPicWidth   = pDecInfo->initialInfo.picWidth;
            result->decPicHeight  = pDecInfo->initialInfo.picHeight;
        }
        if ( instance->codecMode == C7_VP9_DEC ) {
            if ( result->sequenceChanged & SEQ_CHANGE_INTER_RES_CHANGE) {
                regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_PIC_SIZE);
                result->decPicWidth   = regVal>>16;
                result->decPicHeight  = regVal&0xffff;
                result->indexInterFrameDecoded = VpuReadReg(instance->coreIdx, W5_RET_DEC_REALLOC_INDEX);
            }
        }
        osal_memcpy((void*)&pDecInfo->newSeqInfo, (void*)&pDecInfo->initialInfo, sizeof(DecInitialInfo));
        GetSequenceResult(instance, &pDecInfo->newSeqInfo);
    }
    result->numOfErrMBs       = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_CTB_NUM)>>16;
    result->numOfTotMBs       = VpuReadReg(instance->coreIdx, W5_RET_DEC_ERR_CTB_NUM)&0xffff;
    result->bytePosFrameStart = VpuReadReg(instance->coreIdx, W5_RET_DEC_AU_START_POS);
    result->bytePosFrameEnd   = VpuReadReg(instance->coreIdx, W5_RET_DEC_AU_END_POS);
    pDecInfo->prevFrameEndPos = result->bytePosFrameEnd;

    regVal = VpuReadReg(instance->coreIdx, W5_RET_DEC_RECOVERY_POINT);
    result->h265RpSei.recoveryPocCnt = regVal & 0xFFFF;            // [15:0]
    result->h265RpSei.exactMatchFlag = (regVal >> 16)&0x01;        // [16]
    result->h265RpSei.brokenLinkFlag = (regVal >> 17)&0x01;        // [17]
    result->h265RpSei.exist =  (regVal >> 18)&0x01;                // [18]
    if(result->h265RpSei.exist == 0) {
        result->h265RpSei.recoveryPocCnt = 0;
        result->h265RpSei.exactMatchFlag = 0;
        result->h265RpSei.brokenLinkFlag = 0;
    }


    if ( result->sequenceChanged  && (instance->codecMode != C7_VP9_DEC)) {
        pDecInfo->scaleWidth  = pDecInfo->newSeqInfo.picWidth;
        pDecInfo->scaleHeight = pDecInfo->newSeqInfo.picHeight;
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuDecFlush(CodecInst* instance, FramebufferIndex* framebufferIndexes, Uint32 size)
{
    UNREFERENCED_PARAMETER(framebufferIndexes);
    UNREFERENCED_PARAMETER(size);
    Int32       regVal;

    //VpuWriteReg(instance->coreIdx, W5_CMD_FLUSH_INST_OPT, FLUSH_DPB);     // FIX ME. (not defined yet)

    Wave5BitIssueCommand(instance, W5_FLUSH_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }



    regVal = VpuReadReg(instance->coreIdx, W5_RET_SUCCESS);
    if (regVal == 0) {
        return RETCODE_FAILURE;
    }

    // Clear display flags ?
    // Get remaining framebuffers ?


    return RETCODE_SUCCESS;
}

RetCode Wave5VpuReInit(Uint32 coreIdx, void* firmware, Uint32 size)
{
    vpu_buffer_t    vb;
    PhysicalAddress codeBase, tempBase, taskBufBase;
    PhysicalAddress oldCodeBase, tempSize;
    Uint32          codeSize;
    Uint32          regVal, remapSize, i=0;
    CodecInstHeader hdr;

    osal_memset((void *)&hdr, 0x00, sizeof(CodecInstHeader));
    vdi_get_common_memory(coreIdx, &vb);

    codeBase  = vb.phys_addr;
    /* ALIGN TO 4KB */
    codeSize = (WAVE5_MAX_CODE_BUF_SIZE&~0xfff);
    if (codeSize < size*2) {
        return RETCODE_INSUFFICIENT_RESOURCE;
    }
    tempBase = vb.phys_addr + WAVE5_TEMPBUF_OFFSET;
    tempSize = WAVE5_TEMPBUF_SIZE;
    oldCodeBase = VpuReadReg(coreIdx, W5_VPU_REMAP_PADDR);

    if (oldCodeBase != codeBase) {

        VpuWriteMem(coreIdx, codeBase, (unsigned char*)firmware, size*2, VDI_128BIT_LITTLE_ENDIAN);
        vdi_set_bit_firmware_to_pm(coreIdx, (Uint16*)firmware);

        regVal = 0;
        VpuWriteReg(coreIdx, W5_PO_CONF, regVal);

        //// Waiting for completion of bus transaction
        //// Step1 : disable request
        //vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x100);

        //// Step2 : Waiting for completion of bus transaction
        //if (vdi_wait_bus_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_GDI_BUS_STATUS) == -1) {
        //    vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x00);
        //    return RETCODE_VPU_RESPONSE_TIMEOUT;
        //}

        /* Reset All blocks */
        regVal = 0x7ffffff;
        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, regVal);    // Reset All blocks
        /* Waiting reset done */

        if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_RESET_STATUS) == -1) {
            VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
        // Step3 : must clear GDI_BUS_CTRL after done SW_RESET
        //vdi_fio_write_register(coreIdx, W5_GDI_BUS_CTRL, 0x00);

        /* remap page size */
        remapSize = (codeSize >> 12) &0x1ff;
        regVal = 0x80000000 | (WAVE5_AXI_ID<<20) | (W5_REMAP_CODE_INDEX<<12) | (0 << 16) | (1<<11) | remapSize;
        VpuWriteReg(coreIdx, W5_VPU_REMAP_CTRL,     regVal);
        VpuWriteReg(coreIdx, W5_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
        VpuWriteReg(coreIdx, W5_VPU_REMAP_PADDR,    codeBase);
        VpuWriteReg(coreIdx, W5_ADDR_CODE_BASE,     codeBase);
        VpuWriteReg(coreIdx, W5_CODE_SIZE,          codeSize);
        VpuWriteReg(coreIdx, W5_CODE_PARAM,         (WAVE5_AXI_ID<<4) | 0);
        VpuWriteReg(coreIdx, W5_ADDR_TEMP_BASE,     tempBase);
        VpuWriteReg(coreIdx, W5_TEMP_SIZE,          tempSize);
        VpuWriteReg(coreIdx, W5_TIMEOUT_CNT,   0);

        VpuWriteReg(coreIdx, W5_HW_OPTION, 0);
        /* Interrupt */
        regVal  = (1<<W5_INT_INIT_SEQ);
        regVal |= (1<<W5_INT_DEC_PIC);
        regVal |= (1<<W5_INT_BSBUF_EMPTY);

        VpuWriteReg(coreIdx, W5_VPU_VINT_ENABLE,  regVal);

        VpuWriteReg(coreIdx,W5_CMD_INIT_NUM_TASK_BUF, COMMAND_QUEUE_DEPTH);

        vdi_get_common_memory(coreIdx, &vb);
        for (i = 0; i < COMMAND_QUEUE_DEPTH; i++) {
            taskBufBase = vb.phys_addr + WAVE5_TASK_BUF_OFFSET + (i*ONE_TASKBUF_SIZE_FOR_CQ);
            VpuWriteReg(coreIdx, W5_CMD_INIT_ADDR_TASK_BUF0 + (i*4), taskBufBase);
        }

        if (vdi_get_sram_memory(coreIdx, &vb) < 0)  // get SRAM base/size
            return RETCODE_INSUFFICIENT_RESOURCE;

        VpuWriteReg(coreIdx, W5_ADDR_SEC_AXI, vb.phys_addr);
        VpuWriteReg(coreIdx, W5_SEC_AXI_SIZE, vb.size);

        hdr.coreIdx = coreIdx;

        VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
        VpuWriteReg(coreIdx, W5_COMMAND, W5_INIT_VPU);
        VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);
        VpuWriteReg(coreIdx, W5_VPU_REMAP_CORE_START, 1);

        if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
        if (regVal == 0)
            return RETCODE_FAILURE;


    }
    SetupWave5Properties(coreIdx);

    return RETCODE_SUCCESS;
}



RetCode Wave5VpuSleepWake(Uint32 coreIdx, int iSleepWake, const Uint16* code, Uint32 size)
{
    CodecInstHeader hdr;
    Uint32          regVal, i=0;
    vpu_buffer_t    vb;
    PhysicalAddress codeBase, tempBase, taskBufBase;
    Uint32          codeSize, tempSize;
    Uint32          remapSize;

    osal_memset((void *)&hdr, 0x00, sizeof(CodecInstHeader));
    hdr.coreIdx = coreIdx;

    if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
        return RETCODE_VPU_RESPONSE_TIMEOUT;
    }

    if(iSleepWake==1)  //saves
    {
        VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
        VpuWriteReg(coreIdx, W5_COMMAND, W5_SLEEP_VPU);
        VpuWriteReg(coreIdx, W5_VPU_HOST_INT_REQ, 1);

        if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1)
        {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
        regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
        if (regVal == 0)
        {
            APIDPRINT("SLEEP_VPU failed [0x%x]", VpuReadReg(coreIdx, W5_RET_FAIL_REASON));
            return RETCODE_FAILURE;
        }
    }
    else //restore
    {
        Uint32  hwOption  = 0;

        vdi_get_common_memory(coreIdx, &vb);
        codeBase  = vb.phys_addr;
        /* ALIGN TO 4KB */
        codeSize = (WAVE5_MAX_CODE_BUF_SIZE&~0xfff);
        if (codeSize < size*2) {
            return RETCODE_INSUFFICIENT_RESOURCE;
        }

        if (code != NULL) {
            VpuWriteMem(coreIdx, codeBase, (unsigned char*)code, size*2, VDI_128BIT_LITTLE_ENDIAN);
            vdi_set_bit_firmware_to_pm(coreIdx, (Uint16*)code);
        }
        tempBase = vb.phys_addr + WAVE5_TEMPBUF_OFFSET;
        tempSize = WAVE5_TEMPBUF_SIZE;

        regVal = 0;
        VpuWriteReg(coreIdx, W5_PO_CONF, regVal);

        /* SW_RESET_SAFETY */
        regVal = W5_RST_BLOCK_ACLK_ALL | W5_RST_BLOCK_BCLK_ALL | W5_RST_BLOCK_CCLK_ALL;
        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, regVal);    // Reset All blocks

        /* Waiting reset done */
        if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_RESET_STATUS) == -1) {
            VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);

        /* remap page size */
        remapSize = (codeSize >> 12) &0x1ff;
        regVal = 0x80000000 | (WAVE5_AXI_ID<<20) | (W5_REMAP_CODE_INDEX<<12) | (0 << 16) | (1<<11) | remapSize;
        VpuWriteReg(coreIdx, W5_VPU_REMAP_CTRL,     regVal);
        VpuWriteReg(coreIdx, W5_VPU_REMAP_VADDR,    0x00000000);    /* DO NOT CHANGE! */
        VpuWriteReg(coreIdx, W5_VPU_REMAP_PADDR,    codeBase);
        VpuWriteReg(coreIdx, W5_ADDR_CODE_BASE,     codeBase);
        VpuWriteReg(coreIdx, W5_CODE_SIZE,          codeSize);
        VpuWriteReg(coreIdx, W5_CODE_PARAM,         (WAVE5_AXI_ID<<4) | 0);
        VpuWriteReg(coreIdx, W5_ADDR_TEMP_BASE,     tempBase);
        VpuWriteReg(coreIdx, W5_TEMP_SIZE,          tempSize);
        VpuWriteReg(coreIdx, W5_TIMEOUT_CNT,   0);

        VpuWriteReg(coreIdx, W5_HW_OPTION, hwOption);

        /* Interrupt */
        regVal  = (1<<W5_INT_INIT_SEQ);
        regVal |= (1<<W5_INT_DEC_PIC);
        regVal |= (1<<W5_INT_BSBUF_EMPTY);

        VpuWriteReg(coreIdx, W5_VPU_VINT_ENABLE,  regVal);
        VpuWriteReg(coreIdx,W5_CMD_INIT_NUM_TASK_BUF, COMMAND_QUEUE_DEPTH);

        vdi_get_common_memory(coreIdx, &vb);
        for (i = 0; i < COMMAND_QUEUE_DEPTH; i++) {
            taskBufBase = vb.phys_addr + WAVE5_TASK_BUF_OFFSET + (i*ONE_TASKBUF_SIZE_FOR_CQ);
            VpuWriteReg(coreIdx, W5_CMD_INIT_ADDR_TASK_BUF0 + (i*4), taskBufBase);
        }

        if (vdi_get_sram_memory(coreIdx, &vb) < 0)  // get SRAM base/size
            return RETCODE_INSUFFICIENT_RESOURCE;

        VpuWriteReg(coreIdx, W5_ADDR_SEC_AXI, vb.phys_addr);
        VpuWriteReg(coreIdx, W5_SEC_AXI_SIZE, vb.size);

        hdr.coreIdx = coreIdx;

        VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 1);
        VpuWriteReg(coreIdx, W5_COMMAND, W5_INIT_VPU);
        VpuWriteReg(coreIdx, W5_VPU_REMAP_CORE_START, 1);

        if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        regVal = VpuReadReg(coreIdx, W5_RET_SUCCESS);
        if (regVal == 0) {
            return RETCODE_FAILURE;
        }
    }

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuReset(Uint32 coreIdx, SWResetMode resetMode)
{
    Uint32  val = 0;
    RetCode ret = RETCODE_SUCCESS;

    // VPU doesn't send response. Force to set BUSY flag to 0.
    VpuWriteReg(coreIdx, W5_VPU_BUSY_STATUS, 0);

    if (resetMode == SW_RESET_SAFETY) {
        if ((ret=Wave5VpuSleepWake(coreIdx, TRUE, NULL, 0)) != RETCODE_SUCCESS) {
            return ret;
        }
    }

    switch (resetMode) {
    case SW_RESET_ON_BOOT:
    case SW_RESET_FORCE:
        val = W5_RST_BLOCK_ALL;
        break;
    case SW_RESET_SAFETY:
        val = W5_RST_BLOCK_ACLK_ALL | W5_RST_BLOCK_BCLK_ALL | W5_RST_BLOCK_CCLK_ALL;
        break;
    default:
        return RETCODE_INVALID_PARAM;
    }

    if (val) {
        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, val);

        if (vdi_wait_vpu_busy(coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_RESET_STATUS) == -1) {
            VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
            vdi_log(coreIdx, W5_RESET_VPU, 2);
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }
        VpuWriteReg(coreIdx, W5_VPU_RESET_REQ, 0);
    }

    if (resetMode == SW_RESET_SAFETY || resetMode == SW_RESET_FORCE ) {
        ret = Wave5VpuSleepWake(coreIdx, FALSE, NULL, 0);
    }

    return ret;
}

RetCode Wave5VpuDecFiniSeq(CodecInst* instance)
{

    Wave5BitIssueCommand(instance, W5_DESTROY_INSTANCE);
    if (vdi_wait_vpu_busy(instance->coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1)
        return RETCODE_VPU_RESPONSE_TIMEOUT;

    if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == FALSE)
        return RETCODE_FAILURE;

    return RETCODE_SUCCESS;
}

RetCode Wave5VpuDecSetBitstreamFlag(CodecInst* instance, BOOL running, BOOL eos)
{
    UNREFERENCED_PARAMETER(running);
    DecInfo* pDecInfo = &instance->CodecInfo->decInfo;
    BitStreamMode bsMode = (BitStreamMode)pDecInfo->openParam.bitstreamMode;
    pDecInfo->streamEndflag = (eos == 1) ? TRUE : FALSE;

    if (bsMode == BS_MODE_INTERRUPT) {
        VpuWriteReg(instance->coreIdx, W5_BS_OPTION,  (pDecInfo->streamEndflag<<1)|pDecInfo->streamEndflag);
        VpuWriteReg(instance->coreIdx, W5_BS_WR_PTR, pDecInfo->streamWrPtr);

        Wave5BitIssueCommand(instance, W5_UPDATE_BS);
        if (vdi_wait_vpu_busy(instance->coreIdx, VPU_BUSY_CHECK_TIMEOUT, W5_VPU_BUSY_STATUS) == -1) {
            return RETCODE_VPU_RESPONSE_TIMEOUT;
        }

        if (VpuReadReg(instance->coreIdx, W5_RET_SUCCESS) == 0) {
            return RETCODE_FAILURE;
        }
    }

    return RETCODE_SUCCESS;
}


RetCode Wave5DecClrDispFlag(CodecInst* instance, Uint32 index)
{
    RetCode ret = RETCODE_SUCCESS;
    DecInfo * pDecInfo;
    pDecInfo   = &instance->CodecInfo->decInfo;


    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_CLR_DISP_IDC, (1<<index));
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_SET_DISP_IDC, 0);
    ret = SendDecQuery(instance, UPDATE_DISP_FLAG);

    if (ret != RETCODE_SUCCESS)
        return RETCODE_QUERY_FAILURE;

    pDecInfo->frameDisplayFlag = VpuReadReg(instance->coreIdx, pDecInfo->frameDisplayFlagRegAddr);

    return RETCODE_SUCCESS;
}


RetCode Wave5DecSetDispFlag(CodecInst* instance, Uint32 index)
{
    RetCode ret = RETCODE_SUCCESS;

    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_CLR_DISP_IDC, 0);
    VpuWriteReg(instance->coreIdx, W5_CMD_DEC_SET_DISP_IDC, (1<<index));
    ret = SendDecQuery(instance, UPDATE_DISP_FLAG);

    return ret;
}

Int32 Wave5VpuWaitInterrupt(CodecInst* instance, Int32 timeout)
{
    Int32 reason = -1;
    Int32 i = 0, minOrderIndex, maxOrderNum = 0xFFFF;
    Int32 intrInstIndex;
    Int32 unusedIndex;
    vpu_instance_pool_t *vip;

    vip = (vpu_instance_pool_t *)vdi_get_instance_pool(instance->coreIdx);

    if (!vip)
        return RETCODE_INSUFFICIENT_RESOURCE;

    EnterLock(instance->coreIdx);
    /**** 1. list가 비어 있을 때(pending된 interrupt가 하나도 없을때) ****/
    fprintf(stderr, "num_pending_intr=%d\n", vip->pending_intr_list.num_pending_intr);
    if (vip->pending_intr_list.num_pending_intr == 0) {
        if ((reason=vdi_wait_interrupt(instance->coreIdx, timeout, W5_VPU_VINT_REASON_USR)) > 0) {
            intrInstIndex = VpuReadReg(instance->coreIdx, W5_RET_DONE_INSTANCE_INFO)&0xFF;
            VpuWriteReg(instance->coreIdx, W5_VPU_VINT_REASON_CLR, reason);
            VpuWriteReg(instance->coreIdx, W5_VPU_VINT_CLEAR, 1);
            fprintf(stderr, "int_instIdx=%d\n", intrInstIndex);
            if (instance->instIndex != intrInstIndex) {                 // if current instance != interrut triggered instance, store interrupt info to the pending_list
                vip->pending_intr_list.instance_id[0]   = intrInstIndex;
                vip->pending_intr_list.int_reason[0]    = reason;
                vip->pending_intr_list.in_use[0]        = TRUE;
                vip->pending_intr_list.order_num[0]     = 0;
                vip->pending_intr_list.count            = 1;
                vip->pending_intr_list.num_pending_intr++;
                reason = -1;        // 내 instance의 interrupt가 아니면 -1 (timeout) return
            }
        }
    } else {
        minOrderIndex = -1;
        for (i = 0; i < COMMAND_QUEUE_DEPTH; i++) {
            if (vip->pending_intr_list.in_use[i] == TRUE) {
                if (vip->pending_intr_list.instance_id[i] == instance->instIndex && vip->pending_intr_list.order_num[i] < maxOrderNum) {
                    // 이미 해당 instance의 interrupt가 떠있게 있을 때, 가장 먼저 뜬 interrupt를 찾고,
                    maxOrderNum = vip->pending_intr_list.order_num[i];
                    minOrderIndex = i;
                }
            }
            else { // in_use == false
                unusedIndex = i;
            }
        }
        fprintf(stderr, "minOrderIndex=%d\n", minOrderIndex);
        /**** 2. current instance에 해당하는 pending된 interrupt가 있을 때 (Get interrupt info from the pending list) ****/
        if (minOrderIndex >= 0) {
            reason = vip->pending_intr_list.int_reason[minOrderIndex];
            // reset values
            vip->pending_intr_list.in_use[minOrderIndex]        = FALSE;
            vip->pending_intr_list.int_reason[minOrderIndex]    = 0;
            vip->pending_intr_list.order_num[minOrderIndex]     = 0;
            vip->pending_intr_list.num_pending_intr--;
        }
        else {
            /**** 3. pending list내에 current instance에 해당하는 interrupt가 없을 때 ****/
            /****    interrupt받아서 내 instance의 interrupt면 그대로 return, 내 instance께 아니면 list의 빈곳에(unusedIndex) info 저장     ****/
            if ((reason=vdi_wait_interrupt(instance->coreIdx, timeout, W5_VPU_VINT_REASON_USR)) > 0) {
                intrInstIndex = VpuReadReg(instance->coreIdx, W5_RET_DONE_INSTANCE_INFO)&0xFF;
                fprintf(stderr, "int_instIdx=%d\n", intrInstIndex);
                VpuWriteReg(instance->coreIdx, W5_VPU_VINT_REASON_CLR, reason);
                VpuWriteReg(instance->coreIdx, W5_VPU_VINT_CLEAR, 1);

                if (instance->instIndex != intrInstIndex) {                 // if current instance != interrut triggered instance, store interrupt info to the pending_list
                    vip->pending_intr_list.instance_id[unusedIndex] = intrInstIndex;
                    vip->pending_intr_list.in_use[unusedIndex]      = TRUE;
                    vip->pending_intr_list.int_reason[unusedIndex]  = reason;
                    vip->pending_intr_list.order_num[unusedIndex]   = vip->pending_intr_list.count++;
                    vip->pending_intr_list.num_pending_intr++;
                    vip->pending_intr_list.count = vip->pending_intr_list.count % COMMAND_QUEUE_DEPTH;   // max queue depth = 16
                    reason = -1;
                }
            }
        }
    }
    LeaveLock(instance->coreIdx);
    return reason;
}

RetCode Wave5VpuClearInterrupt(Uint32 coreIdx, Uint32 flags)
{
    Uint32 interruptReason;

    interruptReason = VpuReadReg(coreIdx, W5_VPU_VINT_REASON_USR);
    interruptReason &= ~flags;
    VpuWriteReg(coreIdx, W5_VPU_VINT_REASON_USR, interruptReason);

    return RETCODE_SUCCESS;
}
