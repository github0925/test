//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2014  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "vpuapifunc.h"
#include "coda9/coda9_regdefine.h"
#include "wave/common/common_vpuconfig.h"
#include "wave/common/common_regdefine.h"
#include "wave/wave4/wave4_regdefine.h"
#include "wave/wave5/wave5_regdefine.h"
#include "wave/coda7q/coda7q_regdefine.h"
#include "vpuerror.h"
#include "main_helper.h"
#include "config.h"

#define BIT_DUMMY_READ_GEN          0x06000000
#define BIT_READ_LATENCY            0x06000004
#define W4_SET_READ_DELAY           0x01000000
#define W4_SET_WRITE_DELAY          0x01000004
#define MAX_CODE_BUF_SIZE           (512*1024)

#define PIC_TYPE(picType) \
    ((picType == PIC_TYPE_I) ? 'I' : \
    (picType == PIC_TYPE_P) ? 'P' : \
    (picType == PIC_TYPE_B) ? 'B' : \
    (picType == PIC_TYPE_IDR) ? 'I' : \
    'N')

#ifdef PLATFORM_WIN32
#pragma warning(disable : 4996)     //!<< disable waring C4996: The POSIX name for this item is deprecated.
#endif

char* EncPicTypeStringH264[] = {
    "IDR/I",
    "P",
};

char* EncPicTypeStringMPEG4[] = {
    "I",
    "P",
};

char* productNameList[] = {
    "CODA980",
    "CODA960",
    "CODA7503",
    "WAVE320",
    "WAVE410",
    "WAVE4102",
    "WAVE420",
    "WAVE412",
    "WAVE7Q",
    "WAVE420L",
    "WAVE510",
    "WAVE512",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
};

#if defined(PLATFORM_LINUX) || defined(PLATFORM_QNX)
#include <sys/stat.h>
#include <unistd.h>
#endif



int32_t LoadFirmware(
    int32_t     productId,
    uint8_t**   retFirmware,
    uint32_t*   retSizeInWord,
    const char* path
    )
{
    Int32       nread;
    Uint32      totalRead, allocSize, readSize = 1024*1024;
    Uint8*      firmware = NULL;
    osal_file_t fp;

    if ((fp=osal_fopen(path, "rb")) == NULL)
    {
        VLOG(ERR, "Failed to open %s\n", path);
        return -1;
    }

    totalRead = 0;
    if (PRODUCT_ID_W_SERIES(productId))
    {
        firmware = (Uint8*)osal_malloc(readSize);
        allocSize = readSize;
        nread = 0;
        while (TRUE)
        {
            if (allocSize < (totalRead+readSize))
            {
                allocSize += 2*nread;
                firmware = (Uint8*)realloc(firmware, allocSize);
            }
            nread = osal_fread((void*)&firmware[totalRead], 1, readSize, fp);
            totalRead += nread;
            if (nread < (Int32)readSize) break;
        }
        *retSizeInWord = (totalRead+1)/2;
    }
    else
    {
        Uint16*     pusBitCode;
        pusBitCode = (Uint16 *)osal_malloc(MAX_CODE_BUF_SIZE);
        firmware   = (Uint8*)pusBitCode;
        if (pusBitCode)
        {
            int code;
            while (!osal_feof(fp) || totalRead < (MAX_CODE_BUF_SIZE/2)) {
                code = -1;
                if (fscanf(fp, "%x", &code) <= 0) {
                    /* matching failure or EOF */
                    break;
                }
                pusBitCode[totalRead++] = (Uint16)code;
            }
        }
        *retSizeInWord = totalRead;
    }

    osal_fclose(fp);

    *retFirmware   = firmware;

    return 0;
}

void PrintVpuVersionInfo(
    uint32_t core_idx
    )
{
    Uint32 version;
    Uint32 revision;
    Uint32 productId;

    VPU_GetVersionInfo(core_idx, &version, &revision, &productId);

    VLOG(INFO, "VPU coreNum : [%d]\n", core_idx);
    VLOG(INFO, "Firmware : CustomerCode: %04x | version : %d.%d.%d rev.%d\n",
        (Uint32)(version>>16), (Uint32)((version>>(12))&0x0f), (Uint32)((version>>(8))&0x0f), (Uint32)((version)&0xff), revision);
    VLOG(INFO, "Hardware : %04x\n", productId);
    VLOG(INFO, "API      : %d.%d.%d\n\n", API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
}


void PrintMemoryAccessViolationReason(
    Uint32          core_idx,
    DecOutputInfo*  out
    )
{
    UNREFERENCED_PARAMETER(core_idx);
    UNREFERENCED_PARAMETER(out);
}


/**
* \brief           Handle error cases depending on product
* \return  -1      SEQUENCE ERROR
*/
int32_t HandleDecInitSequenceError(DecHandle handle, Uint32 productId, DecOpenParam* openParam, DecInitialInfo* seqInfo, RetCode apiErrorCode)
{
    if (apiErrorCode == RETCODE_MEMORY_ACCESS_VIOLATION) {
        PrintMemoryAccessViolationReason(handle->coreIdx, NULL);
        return -1;
    }

    if (PRODUCT_ID_W_SERIES(productId)) {
        if (seqInfo->seqInitErrReason == WAVE4_SPSERR_NOT_FOUND) {
            return -2;
        } else {
            if (seqInfo->seqInitErrReason == WAVE4_SPEC_OVER_PICTURE_WIDTH_SIZE) {
                VLOG(ERR, "Not supported picture width: MAX_SIZE(8192): %d\n", seqInfo->picWidth);
            }
            if (seqInfo->seqInitErrReason == WAVE4_SPEC_OVER_PICTURE_HEIGHT_SIZE) {
                VLOG(ERR, "Not supported picture height: MAX_SIZE(8192): %d\n", seqInfo->picHeight);
            }
            if (seqInfo->seqInitErrReason == WAVE4_SPEC_OVER_CHROMA_FORMAT) {
                VLOG(ERR, "Not supported chroma idc: %d\n", seqInfo->chromaFormatIDC);
            }
            if (seqInfo->seqInitErrReason == WAVE4_SPEC_OVER_BIT_DEPTH) {
                VLOG(INFO, "Not supported Luma or Chroma bitdepth: L(%d), C(%d)\n", seqInfo->lumaBitdepth, seqInfo->chromaBitdepth);
            }
            if (seqInfo->seqInitErrReason == WAVE4_SPEC_OVER_PROFILE) {
                VLOG(INFO, "Not supported profile: %d\n", seqInfo->profile);
            }
            if (seqInfo->seqInitErrReason == WAVE4_SPSERR_NOT_FOUND) {
                VLOG(INFO, "Not found SPS: RD_PTR(0x%08x), WR_PTR(0x%08x)\n", seqInfo->rdPtr, seqInfo->wrPtr);
            }
            return -1;
        }
    }
    else {
        if (openParam->bitstreamMode == BS_MODE_PIC_END && (seqInfo->seqInitErrReason&(1<<31))) {
            VLOG(ERR, "SEQUENCE HEADER NOT FOUND\n");
            return -1;
        }
        else {
            return -1;
        }
    }
}

void HandleDecoderError(
    DecHandle       handle,
    uint32_t        frameIdx,
    TestDecConfig*  param,
    vpu_buffer_t*   fbMem,
    DecOutputInfo*  outputInfo
    )
{
    UNREFERENCED_PARAMETER(handle);
    UNREFERENCED_PARAMETER(outputInfo);
    UNREFERENCED_PARAMETER(frameIdx);
    UNREFERENCED_PARAMETER(param);
    UNREFERENCED_PARAMETER(fbMem);
}

BOOL OpenDisplayBufferFile(char *outputPath, VpuRect rcDisplay, TiledMapType mapType, FILE *fp[])
{
    char strFile[MAX_FILE_PATH];
    int width;
    int height;

    width = rcDisplay.right - rcDisplay.left;
    height = rcDisplay.bottom - rcDisplay.top;

    if (mapType == LINEAR_FRAME_MAP) {
        if ((fp[0]=fopen(outputPath, "wb")) == NULL) {
            VLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, outputPath);
            goto ERR_OPEN_DISP_BUFFER_FILE;
        }
    }
    else if (mapType == PVRIC_COMPRESSED_FRAME_MAP) {
        return TRUE;
    }
    else {
        width = VPU_ALIGN16(width);
        height = VPU_ALIGN16(height);
        sprintf(strFile, "%s_%dx%d_fbc_data_y.bin", outputPath, width, height);
        if ((fp[0]=fopen(strFile, "wb")) == NULL) {
            VLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, strFile);
            goto ERR_OPEN_DISP_BUFFER_FILE;
        }
        sprintf(strFile, "%s_%dx%d_fbc_data_c.bin", outputPath, width, height);
        if ((fp[1]=fopen(strFile, "wb")) == NULL) {
            VLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, strFile);
            goto ERR_OPEN_DISP_BUFFER_FILE;
        }
        sprintf(strFile, "%s_%dx%d_fbc_table_y.bin", outputPath, width, height);
        if ((fp[2]=fopen(strFile, "wb")) == NULL) {
            VLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, strFile);
            goto ERR_OPEN_DISP_BUFFER_FILE;
        }
        sprintf(strFile, "%s_%dx%d_fbc_table_c.bin", outputPath, width, height);
        if ((fp[3]=fopen(strFile, "wb")) == NULL) {
            VLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, strFile);
            goto ERR_OPEN_DISP_BUFFER_FILE;
        }
    }
    return TRUE;
ERR_OPEN_DISP_BUFFER_FILE:
    CloseDisplayBufferFile(fp);
    return FALSE;
}

void CloseDisplayBufferFile(FILE *fp[])
{
    int i;
    for (i=0; i < OUTPUT_FP_NUMBER; i++) {
        if (fp[i] != NULL) {
            fclose(fp[i]);
            fp[i] = NULL;
        }
    }
}

void SaveDisplayBufferToFile(DecHandle handle, FrameBuffer dispFrame, VpuRect rcDisplay, FILE *fp[], const char* outputPath, Uint32 index)
{
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    DecGetFramebufInfo  fbInfo;
    Uint32   coreIdx = VPU_HANDLE_CORE_INDEX(handle);
    Uint32   lSize;
    Uint32   cSize;

    if (dispFrame.myIndex < 0)
        return;

    if (dispFrame.mapType == COMPRESSED_FRAME_MAP) {
        int lumaTblSize;
        int chromaTblSize;
        uint32_t addr;
        uint32_t fbc_endian;
        unsigned char *buf;

        //VLOG(INFO, "---> DUMP COMPRESSED FRAMEBUFFER #%d\n", dispFrame.myIndex);
        VPU_DecGiveCommand(handle, DEC_GET_FRAMEBUF_INFO, (void*)&fbInfo);

        width = rcDisplay.right - rcDisplay.left;
        height = rcDisplay.bottom - rcDisplay.top;

        width = VPU_ALIGN16(width);
        height = VPU_ALIGN16(height);

        switch (dispFrame.format) {
        case FORMAT_420_P10_16BIT_LSB:
        case FORMAT_420_P10_16BIT_MSB:
            bpp = 2;
            break;
        case FORMAT_420_P10_32BIT_LSB:
        case FORMAT_420_P10_32BIT_MSB:
            bpp = 1;
            break;
        default:
            bpp = 1;
            break;
        }

        fbc_endian = dispFrame.endian;

         fbc_endian = vdi_convert_endian(coreIdx, fbc_endian);

        switch (dispFrame.format) {
        case FORMAT_420_P10_16BIT_MSB:
        case FORMAT_420_P10_16BIT_LSB:
            fbc_endian = ~fbc_endian & 0xf;
            break;
        case FORMAT_420_P10_32BIT_MSB:
            fbc_endian ^= 0x03;
            break;
        case FORMAT_420_P10_32BIT_LSB: //match Ycbcr, if interleave => only Y match
            fbc_endian = ~fbc_endian & 0xf;
            fbc_endian ^= 0x03;
            break;
        default:
            fbc_endian = ~fbc_endian & 0xf;
            break;
        }
        fbc_endian  = fbc_endian + 16;// convert to VDI endian

        lSize = width * height * bpp;
        cSize = width * height * bpp / 2;
        /* Dump Y compressed data */
        buf = osal_malloc(lSize);
        if (buf)
        {
            vdi_read_memory(coreIdx, dispFrame.bufY, buf, lSize, fbc_endian);
            osal_fwrite((void *)buf, sizeof(Uint8), lSize, fp[0]);
            osal_fflush(fp[0]);
            osal_free(buf);
        }

        /* Dump C compressed data */
        buf = osal_malloc(cSize);
        if (buf)
        {
            vdi_read_memory(coreIdx, dispFrame.bufCb, buf, cSize, fbc_endian);
            osal_fwrite((void *)buf, sizeof(Uint8), cSize, fp[1]);
            osal_fflush(fp[1]);
            osal_free(buf);
        }

        /* Dump Y Offset table */
        VPU_GetFBCOffsetTableSize(STD_HEVC, (int)width, (int)height, (int*)&lumaTblSize, (int*)&chromaTblSize);

        addr = fbInfo.vbFbcYTbl[dispFrame.myIndex].phys_addr;

        buf = osal_malloc(lumaTblSize);
        if (buf)
        {
            vdi_read_memory(coreIdx, addr, buf, lumaTblSize, fbc_endian);
            osal_fwrite((void *)buf, sizeof(Uint8), lumaTblSize, fp[2]);
            osal_fflush(fp[2]);
            osal_free(buf);
        }

        /* Dump C Offset table */
        addr = fbInfo.vbFbcCTbl[dispFrame.myIndex].phys_addr;

        buf = osal_malloc(chromaTblSize);
        if (buf)
        {
            vdi_read_memory(coreIdx, addr, buf, chromaTblSize, fbc_endian);
            osal_fwrite((void *)buf, sizeof(Uint8), chromaTblSize, fp[3]);
            osal_fflush(fp[3]);
            osal_free(buf);
        }

    }
    else if (dispFrame.mapType == PVRIC_COMPRESSED_FRAME_MAP){
        /* Dump Y compressed data */
        Uint8* buf      = NULL;
        Uint32 fbEndian = dispFrame.endian;
        char   strFile[128];
        FILE*  fpY;
        FILE*  fpC;

        sprintf(strFile, "%s_pvric_data_y_%03d.bin", outputPath, index);
        if ((fpY=fopen(strFile, "wb")) == NULL) {
            VLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, strFile);
            return;
        }
        sprintf(strFile, "%s_pvric_data_c_%03d.bin", outputPath, index);
        if ((fpC=fopen(strFile, "wb")) == NULL) {
            VLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, strFile);
            fclose(fpY);
            return;
        }

        lSize    = dispFrame.sizeLuma;
        cSize    = dispFrame.sizeChroma * 2;
        buf      = osal_malloc(lSize);
        if (buf)
        {
            //VLOG(INFO, "Saving PVRIC Luma: addr(%p), size(%d)\n", dispFrame.bufY, lSize);
            vdi_read_memory(coreIdx, dispFrame.bufY, buf, lSize, fbEndian);
            osal_fwrite((void *)buf, sizeof(Uint8), lSize, fpY);
            osal_fflush(fpY);
            osal_free(buf);
        }

        /* Dump C compressed data */
        buf = osal_malloc(cSize);
        if (buf)
        {
            //VLOG(INFO, "Saving PVRIC Chroma: addr(%p), size(%d)\n", dispFrame.bufCb, cSize);
            vdi_read_memory(coreIdx, dispFrame.bufCb, buf, cSize, fbEndian);
            osal_fwrite((void *)buf, sizeof(Uint8), cSize, fpC);
            osal_fflush(fpC);
            osal_free(buf);
        }
    }
    else {
        //VLOG(INFO, "---> DUMP LINEAR(WTL) FRAMEBUFFER #%d\n", dispFrame.myIndex);
        size_t sizeYuv;
        uint8_t* pYuv;
        pYuv = GetYUVFromFrameBuffer(handle, &dispFrame, rcDisplay, &width, &height, &bpp, &sizeYuv);
        osal_fwrite(pYuv, 1, sizeYuv, fp[0]);
        osal_free(pYuv);
    }


}
void FreePreviousFramebuffer(
    Uint32              coreIdx,
    DecGetFramebufInfo* fb
    )
{
    int i;
    if (fb->vbFrame.size > 0) {
        vdi_free_dma_memory(coreIdx, &fb->vbFrame);
        osal_memset((void*)&fb->vbFrame, 0x00, sizeof(vpu_buffer_t));
    }
    if (fb->vbWTL.size > 0) {
        vdi_free_dma_memory(coreIdx, &fb->vbWTL);
        osal_memset((void*)&fb->vbWTL, 0x00, sizeof(vpu_buffer_t));
    }
    for ( i=0 ; i<MAX_REG_FRAME; i++) {
        if (fb->vbFbcYTbl[i].size > 0) {
            vdi_free_dma_memory(coreIdx, &fb->vbFbcYTbl[i]);
            osal_memset((void*)&fb->vbFbcYTbl, 0x00, sizeof(vpu_buffer_t));
        }
        if (fb->vbFbcCTbl[i].size > 0) {
            vdi_free_dma_memory(coreIdx, &fb->vbFbcCTbl[i]);
            osal_memset((void*)&fb->vbFbcCTbl, 0x00, sizeof(vpu_buffer_t));
        }
    }
}

void PrintDecSeqWarningMessages(
    Uint32          productId,
    DecInitialInfo* seqInfo
    )
{
    if (PRODUCT_ID_W_SERIES(productId))
    {
        if (seqInfo->seqInitErrReason&0x00000001) VLOG(WARN, "sps_max_sub_layer_minus1 shall be 0 to 6\n");
        if (seqInfo->seqInitErrReason&0x00000002) VLOG(WARN, "general_reserved_zero_44bits shall be 0.\n");
        if (seqInfo->seqInitErrReason&0x00000004) VLOG(WARN, "reserved_zero_2bits shall be 0\n");
        if (seqInfo->seqInitErrReason&0x00000008) VLOG(WARN, "sub_layer_reserved_zero_44bits shall be 0");
        if (seqInfo->seqInitErrReason&0x00000010) VLOG(WARN, "general_level_idc shall have one of level of Table A.1\n");
        if (seqInfo->seqInitErrReason&0x00000020) VLOG(WARN, "sps_max_dec_pic_buffering[i] <= MaxDpbSize\n");
        if (seqInfo->seqInitErrReason&0x00000040) VLOG(WARN, "trailing bits shall be 1000... pattern, 7.3.2.1\n");
        if (seqInfo->seqInitErrReason&0x00100000) VLOG(WARN, "Not supported or undefined profile: %d\n", seqInfo->profile);
        if (seqInfo->seqInitErrReason&0x00200000) VLOG(WARN, "Spec over level(%d)\n", seqInfo->level);
    }
}

void DisplayDecodedInformationForHevc(
    DecHandle      handle,
    Uint32         frameNo,
    DecOutputInfo* decodedInfo
    )
{
    Int32 logLevel;

    if (decodedInfo == NULL) {
        if (handle->productId == PRODUCT_ID_510 || handle->productId == PRODUCT_ID_512) {
            VLOG(TRACE, "I    NO  T     POC     NAL  DECO  DISP PRESCAN DISPFLAG  RD_PTR   WR_PTR  FRM_START FRM_END   WxH      SEQ  TEMP CYCLE (Seek, Parse, Dec)\n");
        }
        else {
            VLOG(TRACE, "I    NO  T     POC     NAL  DECO  DISP PRESCAN DISPFLAG  RD_PTR   WR_PTR  FRM_START FRM_END   WxH      SEQ  TEMP CYCLE  \n");
        }
        VLOG(TRACE, "------------------------------------------------------------------------------------------------------------\n");
    }
    else {
        logLevel = (decodedInfo->decodingSuccess&0x01) == 0 ? ERR : TRACE;
        if (handle->productId == PRODUCT_ID_4102) {
            logLevel = (decodedInfo->indexFramePrescan == -2) ? TRACE : logLevel;
        }
        // Print informations
        if (handle->productId == PRODUCT_ID_510 || handle->productId == PRODUCT_ID_512) {
            VLOG(logLevel, "instance[%d:%d] %5d %d %4d(%4d) %3d %2d(%2d) %2d(%2d) %7d %08x %08x %08x %08x %08x %4dx%-4d %4d  %4d %d(%d,%d,%d)\n",
                handle->coreIdx, handle->instIndex, frameNo, decodedInfo->picType,
                decodedInfo->h265Info.decodedPOC, decodedInfo->h265Info.displayPOC, decodedInfo->nalType,
                decodedInfo->indexFrameDecoded, decodedInfo->indexFrameDecodedForTiled,
                decodedInfo->indexFrameDisplay, decodedInfo->indexFrameDisplayForTiled,
                decodedInfo->indexFramePrescan,
                decodedInfo->frameDisplayFlag,decodedInfo->rdPtr, decodedInfo->wrPtr,
                decodedInfo->bytePosFrameStart, decodedInfo->bytePosFrameEnd,
                decodedInfo->dispPicWidth, decodedInfo->dispPicHeight, decodedInfo->sequenceNo,
                decodedInfo->h265Info.temporalId, decodedInfo->frameCycle, decodedInfo->seekCycle, decodedInfo->parseCycle, decodedInfo->decodeCycle);
        }
        else {
            VLOG(logLevel, "instance[%d:%d] %5d %d %4d(%4d) %3d %2d(%2d) %2d(%2d) %7d %08x %08x %08x %08x %08x %4dx%-4d %4d  %4d, frame: %c, cycle: %d\n",
                handle->coreIdx, handle->instIndex, frameNo, decodedInfo->picType,
                decodedInfo->h265Info.decodedPOC, decodedInfo->h265Info.displayPOC, decodedInfo->nalType,
                decodedInfo->indexFrameDecoded, decodedInfo->indexFrameDecodedForTiled,
                decodedInfo->indexFrameDisplay, decodedInfo->indexFrameDisplayForTiled,
                decodedInfo->indexFramePrescan,
                decodedInfo->frameDisplayFlag,decodedInfo->rdPtr, decodedInfo->wrPtr,
                decodedInfo->bytePosFrameStart, decodedInfo->bytePosFrameEnd,
                decodedInfo->dispPicWidth, decodedInfo->dispPicHeight, decodedInfo->sequenceNo,
                decodedInfo->h265Info.temporalId, PIC_TYPE(decodedInfo->picType), decodedInfo->frameCycle);
        }
        if (logLevel == ERR) {
            VLOG(ERR, "\t>>ERROR REASON: 0x%08x(0x%08x)\n", decodedInfo->errorReason, decodedInfo->errorReasonExt);
        }
        if (decodedInfo->numOfErrMBs) {
            VLOG(WARN, "\t>> ErrorBlock: %d\n", decodedInfo->numOfErrMBs);
        }
    }
}

void DisplayDecodedInformationForVP9(DecHandle handle, Uint32 frameNo, DecOutputInfo* decodedInfo)
{
    Int32 logLevel;

    if (decodedInfo == NULL) {
        // Print header
        VLOG(TRACE, "I  NO    T   DECO   DISP PRESCAN DISPFLAG   RD_PTR   WR_PTR FRM_START FRM_END    WxH     SEQ  CYCLE\n");
        VLOG(TRACE, "--------------------------------------------------------------------------------------------\n");
    }
    else {
        logLevel = (decodedInfo->decodingSuccess&0x01) == 0 ? ERR : TRACE;
        // Print informations
        VLOG(logLevel, "instance[%d:%d] %5d %d %2d(%2d) %2d(%2d) %7d %08x %08x %08x %08x %08x %4dx%-4d %4d, frame: %c, cycle: %d\n",
            handle->coreIdx, handle->instIndex, frameNo, decodedInfo->picType,
            decodedInfo->indexFrameDecoded, decodedInfo->indexFrameDecodedForTiled,
            decodedInfo->indexFrameDisplay, decodedInfo->indexFrameDisplayForTiled,
            decodedInfo->indexFramePrescan,
            decodedInfo->frameDisplayFlag,decodedInfo->rdPtr, decodedInfo->wrPtr,
            decodedInfo->bytePosFrameStart, decodedInfo->bytePosFrameEnd,
            decodedInfo->dispPicWidth, decodedInfo->dispPicHeight,
            decodedInfo->sequenceNo, PIC_TYPE(decodedInfo->picType), decodedInfo->frameCycle
        );
        if (logLevel == ERR) {
            VLOG(ERR, "\t>>ERROR REASON: 0x%08x(0x%08x)\n", decodedInfo->errorReason, decodedInfo->errorReasonExt);
        }
        if (decodedInfo->numOfErrMBs) {
            VLOG(WARN, "\t>> ErrorBlock: %d\n", decodedInfo->numOfErrMBs);
        }
    }
}

void DisplayDecodedInformationCommon(DecHandle handle, Uint32 frameNo, DecOutputInfo* decodedInfo)
{
    Int32 logLevel;

    if (decodedInfo == NULL) {
        // Print header
        VLOG(TRACE, "I  NO    T   DECO   DISP DISPFLAG   RD_PTR   WR_PTR FRM_START FRM_END WxH  \n");
        VLOG(TRACE, "---------------------------------------------------------------------------\n");
    }
    else {
        VpuRect rc    = decodedInfo->rcDisplay;
        Uint32 width  = rc.right - rc.left;
        Uint32 height = rc.bottom - rc.top;
        logLevel = (decodedInfo->decodingSuccess&0x01) == 0 ? ERR : TRACE;
        // Print informations
        VLOG(logLevel, "instance[%d:%d] %5d %d %2d(%2d) %2d(%2d) %08x %08x %08x %08x %08x %dx%d, frame: %c, cycle: %d\n",
            handle->coreIdx, handle->instIndex, frameNo, decodedInfo->picType,
            decodedInfo->indexFrameDecoded, decodedInfo->indexFrameDecodedForTiled,
            decodedInfo->indexFrameDisplay, decodedInfo->indexFrameDisplayForTiled,
            decodedInfo->frameDisplayFlag,decodedInfo->rdPtr, decodedInfo->wrPtr,
            decodedInfo->bytePosFrameStart, decodedInfo->bytePosFrameEnd,
            width, height, PIC_TYPE(decodedInfo->picType), decodedInfo->frameCycle);
        if (logLevel == ERR) {
            VLOG(ERR, "\t>>ERROR REASON: 0x%08x(0x%08x)\n", decodedInfo->errorReason, decodedInfo->errorReasonExt);
        }
        if (decodedInfo->numOfErrMBs) {
            VLOG(WARN, "\t>> ErrorBlock: %d\n", decodedInfo->numOfErrMBs);
        }
    }
}

/**
* \brief                   Print out decoded information such like RD_PTR, WR_PTR, PIC_TYPE, ..
* \param   decodedInfo     If this parameter is not NULL then print out decoded informations
*                          otherwise print out header.
*/
void
    DisplayDecodedInformation(
    DecHandle      handle,
    CodStd         codec,
    Uint32         frameNo,
    DecOutputInfo* decodedInfo
    )
{
    switch (codec)
    {
    case STD_HEVC:
        DisplayDecodedInformationForHevc(handle, frameNo, decodedInfo);
        break;
    case STD_VP9:
        DisplayDecodedInformationForVP9(handle, frameNo, decodedInfo);
        break;
    default:
        DisplayDecodedInformationCommon(handle, frameNo, decodedInfo);
        break;
    }

    return;
}

static void Wave4DisplayEncodedInformation(
    DecHandle       handle,
    CodStd          codec,
    uint32_t        frameNo,
    EncOutputInfo*  encodedInfo,
    int32_t         srcEndFlag,
    int32_t         srcFrameIdx
    )
{
    UNREFERENCED_PARAMETER(codec);
    UNREFERENCED_PARAMETER(frameNo);

    if (encodedInfo == NULL) {
        VLOG(INFO, "------------------------------------------------------------------------------\n");
        VLOG(INFO, "I     NO     T   RECON   RD_PTR    WR_PTR     BYTES  SRCIDX  USEDSRCIDX Cycle \n");
        VLOG(INFO, "------------------------------------------------------------------------------\n");
    } else {
        VLOG(INFO, "%02d %5d %5d %5d    %08x  %08x %8x     %2d        %2d    %8d\n",
            handle->instIndex, encodedInfo->encPicCnt, encodedInfo->picType, encodedInfo->reconFrameIndex, encodedInfo->rdPtr, encodedInfo->wrPtr,
            encodedInfo->bitstreamSize, (srcEndFlag == 1 ? -1 : srcFrameIdx), encodedInfo->encSrcIdx, encodedInfo->frameCycle);
    }
}

static void Coda9DisplayEncodedInformation(
    DecHandle       handle,
    CodStd          codec,
    uint32_t        frameNo,
    EncOutputInfo*  encodedInfo
    )
{
    if (encodedInfo == NULL) {
        // Print header
        VLOG(INFO, "I    NO   T   RECON  RD_PTR   WR_PTR \n");
        VLOG(INFO, "-------------------------------------\n");
    }
    else {
        char** picTypeArray = (codec==STD_AVC ? EncPicTypeStringH264 : EncPicTypeStringMPEG4);
        char* strPicType;

        if (encodedInfo->picType > 2) strPicType = "?";
        else                          strPicType = picTypeArray[encodedInfo->picType];
        // Print informations
        VLOG(INFO, "instance[%d:%d] %5d %5s %5d  %08x %08x, frame: %c, cycle: %d\n",
            handle->coreIdx, handle->instIndex, frameNo, strPicType,
            encodedInfo->reconFrameIndex, encodedInfo->rdPtr, encodedInfo->wrPtr,
            PIC_TYPE(encodedInfo->picType), encodedInfo->frameCycle);
    }
}

/*lint -esym(438, ap) */
void
    DisplayEncodedInformation(
    EncHandle      handle,
    CodStd         codec,
    Uint32         frameNo,
    EncOutputInfo* encodedInfo,
    ...
    )
{
    int srcEndFlag;
    int srcFrameIdx;
    va_list         ap;

    switch (codec) {
    case STD_HEVC:
        va_start(ap, encodedInfo);
        srcEndFlag = va_arg(ap, uint32_t);
        srcFrameIdx = va_arg(ap, uint32_t);
        va_end(ap);
        Wave4DisplayEncodedInformation(handle, codec, frameNo, encodedInfo, srcEndFlag , srcFrameIdx);
        break;
    default:
        Coda9DisplayEncodedInformation(handle, codec, frameNo, encodedInfo);
        break;
    }

    return;
}
/*lint +esym(438, ap) */




#define VCORE_DBG_ADDR(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x300
#define VCORE_DBG_DATA(__vCoreIdx)      0x8000+(0x1000*__vCoreIdx) + 0x304
#define VCORE_DBG_READY(__vCoreIdx)     0x8000+(0x1000*__vCoreIdx) + 0x308

void WriteRegVCE(
    Uint32   core_idx,
    Uint32   vce_core_idx,
    Uint32   vce_addr,
    Uint32   udata
    )
{
    int vcpu_reg_addr;

    SetClockGate(core_idx, 1);

    vdi_fio_write_register(core_idx, VCORE_DBG_READY(vce_core_idx),0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(core_idx, VCORE_DBG_DATA(vce_core_idx),udata);
    vdi_fio_write_register(core_idx, VCORE_DBG_ADDR(vce_core_idx),(vcpu_reg_addr) & 0x00007FFF);

    if (vdi_fio_read_register(0, VCORE_DBG_READY(vce_core_idx)) < 0)
        VLOG(ERR, "failed to write VCE register: 0x%04x\n", vce_addr);
    SetClockGate(core_idx, 0);
}

Uint32 ReadRegVCE(
    Uint32 core_idx,
    Uint32 vce_core_idx,
    Uint32 vce_addr
    )
{
    int     vcpu_reg_addr;
    int     udata;
    int     vce_core_base = 0x8000 + 0x1000*vce_core_idx;

    SetClockGate(core_idx, 1);
    vdi_fio_write_register(core_idx, VCORE_DBG_READY(vce_core_idx), 0);

    vcpu_reg_addr = vce_addr >> 2;

    vdi_fio_write_register(core_idx, VCORE_DBG_ADDR(vce_core_idx),vcpu_reg_addr + vce_core_base);

    if (vdi_fio_read_register(core_idx, VCORE_DBG_READY(vce_core_idx)) == 1)
        udata= vdi_fio_read_register(core_idx, VCORE_DBG_DATA(vce_core_idx));
    else {
        VLOG(ERR, "failed to read VCE register: %d, 0x%04x\n", vce_core_idx, vce_addr);
        udata = -2;//-1 can be a valid value
    }

    SetClockGate(core_idx, 0);
    return udata;
}

static void DisplayVceEncDebugCommon(int core_idx, int vcore_idx, int set_mode, int debug0, int debug1, int debug2)
{
    int reg_val;
    VLOG(ERR, "---------------Common Debug INFO-----------------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,0 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug0);
    VLOG(ERR,"\t- pipe_on       :  0x%x\n", ((reg_val >> 8) & 0xf));
    VLOG(ERR,"\t- cur_pipe      :  0x%x\n", ((reg_val >> 12) & 0xf));
    VLOG(ERR,"\t- cur_s2ime     :  0x%x\n", ((reg_val >> 16) & 0xf));
    VLOG(ERR,"\t- cur_s2cache   :  0x%x\n", ((reg_val >> 20) & 0x7));
    VLOG(ERR,"\t- subblok_done  :  0x%x\n", ((reg_val >> 24) & 0x7f));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug1);
    VLOG(ERR,"\t- subblok_done  :  SFU 0x%x LF 0x%x RDO 0x%x IMD 0x%x FME 0x%x IME 0x%x\n", ((reg_val >> 5) & 0x1), ((reg_val >> 4) & 0x1), ((reg_val >> 3) & 0x1), ((reg_val >> 2) & 0x1), ((reg_val >> 1) & 0x1), ((reg_val >> 0) & 0x1));
    VLOG(ERR,"\t- pipe_on       :  0x%x\n", ((reg_val >> 8) & 0xf));
    VLOG(ERR,"\t- cur_pipe      :  0x%x\n", ((reg_val >> 12) & 0xf));
    VLOG(ERR,"\t- cur_s2cache   :  0x%x\n", ((reg_val >> 16) & 0x7));
    VLOG(ERR,"\t- cur_s2ime     :  0x%x\n", ((reg_val >> 24) & 0xf));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug2);
    VLOG(ERR,"\t- main_ctu_ypos :  0x%x\n", ((reg_val >> 0) & 0xff));
    VLOG(ERR,"\t- main_ctu_xpos :  0x%x\n", ((reg_val >> 8) & 0xff));
    VLOG(ERR,"\t- o_prp_ctu_ypos:  0x%x\n", ((reg_val >> 16) & 0xff));
    VLOG(ERR,"\t- o_prp_ctu_xpos:  0x%x\n", ((reg_val >> 24) & 0xff));

    SetClockGate(0, 1);
    reg_val  = vdi_fio_read_register(core_idx, W4_GDI_VCORE0_BUS_STATUS);
    VLOG(ERR,"\t- =========== GDI_BUS_STATUS ==========  \n");
    VLOG(ERR,"\t- pri_bus_busy:  0x%x\n", ((reg_val >> 0) & 0x1));
    VLOG(ERR,"\t- sec_bus_busy:  0x%x\n", ((reg_val >> 16) & 0x1));
    reg_val= VpuReadReg(core_idx, W4_RET_ENC_PIC_TYPE);
    VLOG(ERR,"[DEBUG] ret_core1_init : %d\n", (reg_val >> 16) & 0x3ff);
    reg_val = VpuReadReg(core_idx, W4_RET_ENC_PIC_FLAG);
    VLOG(ERR,"[DEBUG] ret_core0_init : %d\n", (reg_val >> 5) & 0x3ff);
    SetClockGate(0, 0);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void DisplayVceEncDebugDCI(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 0 : DCI DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,0 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_cnt_dci_rd_tuh       :  0x%x\n", ((reg_val >> 16) & 0xffff));
    VLOG(ERR,"\t- i_cnt_dci_wd_tuh       :  0x%x\n", ((reg_val >>  0) & 0xffff));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_cnt_dci_rd_cu        :  0x%x\n", ((reg_val >> 16) & 0xffff));
    VLOG(ERR,"\t- i_cnt_dci_wd_cu        :  0x%x\n", ((reg_val >>  0) & 0xffff));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cnt_dci_rd_ctu       :  0x%x\n", ((reg_val >> 16) & 0xffff));
    VLOG(ERR,"\t- i_cnt_dci_2d_ctu       :  0x%x\n", ((reg_val >>  0) & 0xffff));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- i_cnt_dci_rd_coef      :  0x%x\n", ((reg_val >> 16) & 0xffff));
    VLOG(ERR,"\t- i_cnt_dci_wd_coef      :  0x%x\n", ((reg_val >>  0) & 0xffff));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- i_dci_full_empty_flag  :  0x%x\n", reg_val);

    VLOG(ERR,"----------- MODE 0 : VCE_CTRL DEBUG INFO----------\n");
    // HW_PARAM
    reg_val = ReadRegVCE(core_idx, vcore_idx, 0x0b08);
    VLOG(ERR,"\t- r_cnt_enable           :  0x%x\n", (reg_val >> 8) & 0x1);
    VLOG(ERR,"\t- r_pic_done_sel         :  0x%x\n", (reg_val >> 9) & 0x1);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- vce_cnt                :  0x%x\n", reg_val);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- prp_cnt                :  0x%x\n", reg_val);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void DisplayVceEncDebugRDO(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    int reg_val_sub;

    VLOG(ERR,"----------- MODE 1 : RDO DEBUG INFO ----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,1 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- o_rdo_cu_root_cb                    :  0x%x\n", ((reg_val >>  0) & 0x1));
    VLOG(ERR,"\t- o_rdo_tu_cbf_y                      :  0x%x\n", ((reg_val >>  1) & 0x1));
    VLOG(ERR,"\t- o_rdo_tu_cbf_u                      :  0x%x\n", ((reg_val >>  2) & 0x1));
    VLOG(ERR,"\t- o_rdo_tu_cbf_v                      :  0x%x\n", ((reg_val >>  3) & 0x1));
    VLOG(ERR,"\t- w_rdo_wdma_wait                     :  0x%x\n", ((reg_val >>  4) & 0x1));
    VLOG(ERR,"\t- |o_rdo_tu_sb_luma_csbf[63: 0]       :  0x%x\n", ((reg_val >>  5) & 0x1));
    VLOG(ERR,"\t- |o_rdo_tu_sb_chro_csbf[31:16]       :  0x%x\n", ((reg_val >>  6) & 0x1));
    VLOG(ERR,"\t- |o_rdo_tu_sb_chro_csbf[15: 0]       :  0x%x\n", ((reg_val >>  7) & 0x1));
    VLOG(ERR,"\t- o_sub_ctu_coe_ready                 :  0x%x\n", ((reg_val >>  8) & 0x1));
    VLOG(ERR,"\t- o_sub_ctu_rec_ready                 :  0x%x\n", ((reg_val >>  9) & 0x1));
    VLOG(ERR,"\t- o_rdo_wdma_busy                     :  0x%x\n", ((reg_val >> 10) & 0x1));
    VLOG(ERR,"\t- w_rdo_rdma_wait                     :  0x%x\n", ((reg_val >> 11) & 0x1));
    VLOG(ERR,"\t- o_log2_cu_size[07:06]               :  0x%x\n", ((reg_val >> 12) & 0x3));
    VLOG(ERR,"\t- o_log2_cu_size[15:14]               :  0x%x\n", ((reg_val >> 14) & 0x3));
    VLOG(ERR,"\t- o_log2_cu_size[23:22]               :  0x%x\n", ((reg_val >> 16) & 0x3));
    VLOG(ERR,"\t- o_log2_cu_size[31:30]               :  0x%x\n", ((reg_val >> 18) & 0x3));
    VLOG(ERR,"\t- o_rdo_dbk_valid                     :  0x%x\n", ((reg_val >> 20) & 0x1));

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- debug_status_ctrl                   :  0x%x\n", ((reg_val >>  0) & 0xff));
    reg_val_sub = (reg_val >>  0) & 0xff;
    VLOG(ERR,"\t- debug_status_ctrl.fsm_main_cur      :  0x%x\n", ((reg_val_sub >>  0) & 0x7));
    VLOG(ERR,"\t- debug_status_ctrl.i_rdo_wdma_wait   :  0x%x\n", ((reg_val_sub >>  3) & 0x1));
    VLOG(ERR,"\t- debug_status_ctrl.fsm_cu08_cur      :  0x%x\n", ((reg_val_sub >>  4) & 0x7));
    VLOG(ERR,"\t- debug_status_ctrl.init_hold         :  0x%x\n", ((reg_val_sub >>  7) & 0x1));

    VLOG(ERR,"\t- debug_status_nb                     :  0x%x\n", ((reg_val >>  8) & 0xff));
    reg_val_sub =(reg_val >>  8) & 0xff;
    VLOG(ERR,"\t- debug_status_nb.fsm_save_cur        :  0x%x\n", ((reg_val_sub >>  0) & 0x7));
    VLOG(ERR,"\t- debug_status_nb.fsm_load_cur        :  0x%x\n", ((reg_val_sub >>  4) & 0x7));

    VLOG(ERR,"\t- debug_status_rec                    :  0x%x\n", ((reg_val >> 16) & 0xf));
    reg_val_sub = (reg_val >> 16) & 0xf;
    VLOG(ERR,"\t- debug_status_rec.fsm_obuf_cur       :  0x%x\n", ((reg_val_sub >>  0) & 0x7));

    VLOG(ERR,"\t- debug_status_coe                    :  0x%x\n", ((reg_val >> 20) & 0xf));
    reg_val_sub = (reg_val >> 20) & 0xf;
    VLOG(ERR,"\t- debug_status_coe.fsm_obuf_cur       :  0x%x\n", ((reg_val_sub >> 0) & 0x7));

    VLOG(ERR,"\t- debug_status_para                   :  0x%x\n", ((reg_val >> 24) & 0xff));
    reg_val_sub = (reg_val >> 24) & 0xff;
    VLOG(ERR,"\t- debug_status_para.cur_sfu_rd_state  :  0x%x\n", ((reg_val_sub >> 0) & 0xf));
    VLOG(ERR,"\t- debug_status_para.cur_para_state    :  0x%x\n", ((reg_val_sub >> 4) & 0xf));


    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void DisplayVceEncDebugLF(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 2 : LF DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,2 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);

    VLOG(ERR,"\t- cur_enc_main_state   : 0x%x \n", (reg_val>>27)&0x1F);
    VLOG(ERR,"\t- i_sao_para_valie     : 0x%x \n", (reg_val>>26)&0x1);
    VLOG(ERR,"\t- i_sao_fetch_done     : 0x%x \n", (reg_val>>25)&0x1);
    VLOG(ERR,"\t- i_global_encode_en   : 0x%x \n", (reg_val>>24)&0x1);
    VLOG(ERR,"\t- i_bs_valid           : 0x%x \n", (reg_val>>23)&0x1);
    VLOG(ERR,"\t- i_rec_buf_rdo_ready  : 0x%x \n", (reg_val>>22)&0x1);
    VLOG(ERR,"\t- o_rec_buf_dbk_hold   : 0x%x \n", (reg_val>>21)&0x1);
    VLOG(ERR,"\t- cur_main_state       : 0x%x \n", (reg_val>>16)&0x1F);
    VLOG(ERR,"\t- r_lf_pic_dbk_disable : 0x%x \n", (reg_val>>15)&0x1);
    VLOG(ERR,"\t- r_lf_pic_sao_disable : 0x%x \n", (reg_val>>14)&0x1);
    VLOG(ERR,"\t- para_load_done       : 0x%x \n", (reg_val>>13)&0x1);
    VLOG(ERR,"\t- i_rdma_ack_wait      : 0x%x \n", (reg_val>>12)&0x1);
    VLOG(ERR,"\t- i_sao_intl_col_done  : 0x%x \n", (reg_val>>11)&0x1);
    VLOG(ERR,"\t- i_sao_outbuf_full    : 0x%x \n", (reg_val>>10)&0x1);
    VLOG(ERR,"\t- lf_sub_done          : 0x%x \n", (reg_val>>9)&0x1);
    VLOG(ERR,"\t- i_wdma_ack_wait      : 0x%x \n", (reg_val>>8)&0x1);
    VLOG(ERR,"\t- lf_all_sub_done      : 0x%x \n", (reg_val>>6)&0x1);
    VLOG(ERR,"\t- cur_ycbcr            : 0x%x \n", (reg_val>>5)&0x3);
    VLOG(ERR,"\t- sub8x8_done          : 0x%x \n", (reg_val>>0)&0xF);

    VLOG(ERR,"----------- MODE 2 : SYNC_Y_POS DEBUG INFO----------\n");
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);

    VLOG(ERR,"\t- fbc_y_pos            : 0x%x \n", (reg_val>>0)&0xff);
    VLOG(ERR,"\t- bwb_y_pos            : 0x%x \n", (reg_val>>16)&0xff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- trace_frame          :  0x%x\n", ((reg_val >> 0) & 0xffff));

    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void DisplayVceEncDebugSFU(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 3 : SFU DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,3 );

    reg_val = ReadRegVCE(0, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_sub_ctu_pos_y         : 0x%x \n", (reg_val>>0)&0xff);
    VLOG(ERR,"\t- i_sub_ctu_pos_x         : 0x%x \n", (reg_val>>8)&0xff);
    VLOG(ERR,"\t- i_cu_fifo_wvalid        : 0x%x \n", (reg_val>>16)&0x1);
    VLOG(ERR,"\t- i_ctu_busy              : 0x%x \n", (reg_val>>20)&0x1);
    VLOG(ERR,"\t- i_cs_sctu               : 0x%x \n", (reg_val>>24)&0x7);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_ctu_pos_y             : 0x%x \n", (reg_val>>0)&0xff);
    VLOG(ERR,"\t- i_ctu_pos_x             : 0x%x \n", (reg_val>>8)&0xff);
    VLOG(ERR,"\t- i_sao_rdo_valid         : 0x%x \n", (reg_val>>16)&0x1);
    VLOG(ERR,"\t- i_sao_en_r              : 0x%x \n", (reg_val>>20)&0x1);
    VLOG(ERR,"\t- i_ctu_fifo_wvalid       : 0x%x \n", (reg_val>>24)&0x1);
    VLOG(ERR,"\t- cs_sfu_ctu              : 0x%x \n", (reg_val>>28)&0x3);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cu_fifo_wvalid        : 0x%x \n", (reg_val>>0)&0x1);
    VLOG(ERR,"\t- i_rdo_cu_rd_valid       : 0x%x \n", (reg_val>>4)&0x1);
    VLOG(ERR,"\t- i_cu_size_r             : 0x%x \n", (reg_val>>8)&0x3);
    VLOG(ERR,"\t- i_cu_idx_r              : 0x%x \n", (reg_val>>12)&0xf);
    VLOG(ERR,"\t- cs_cu                   : 0x%x \n", (reg_val>>16)&0x7);
    VLOG(ERR,"\t- cs_fifo                 : 0x%x \n", (reg_val>>20)&0x7);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- w_dbg_tu_fifo_fsm       : 0x%x \n", (reg_val>>0)&0xff);
    VLOG(ERR,"\t- i_coff_fifo_wvalid      : 0x%x \n", (reg_val>>8)&0x1);
    VLOG(ERR,"\t- i_tuh_fifo_wvalid       : 0x%x \n", (reg_val>>12)&0x1);
    VLOG(ERR,"\t- w_dbg_tu_ctrl_fsm       : 0x%x \n", (reg_val>>16)&0xf);
    VLOG(ERR,"\t- i_rdo_tc_ready          : 0x%x \n", (reg_val>>20)&0x1);
    VLOG(ERR,"\t- w_dbg_coef_st_in_pic    : 0x%x \n", (reg_val>>24)&0x7);
    VLOG(ERR,"\t- i_rdo_tu_rd-valid       : 0x%x \n", (reg_val>>28)&0x1);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void DisplayVceEncDebugDCI2(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 4 : DCI2 DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,4 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_cnt_dci_rd_tuh2       : 0x%x \n", (reg_val>>16)&0xffff);
    VLOG(ERR,"\t- i_cnt_dci_wd_tuh2       : 0x%x \n", (reg_val>> 0)&0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_cnt_dci_rd_cu2        : 0x%x \n", (reg_val>>16)&0xffff);
    VLOG(ERR,"\t- i_cnt_dci_wd_cu2        : 0x%x \n", (reg_val>> 0)&0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cnt_dci_rd_ctu2       : 0x%x \n", (reg_val>>16)&0xffff);
    VLOG(ERR,"\t- i_cnt_dci_wd_ctu2       : 0x%x \n", (reg_val>> 0)&0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- i_cnt_dci_rd_coef2      : 0x%x \n", (reg_val>>16)&0xffff);
    VLOG(ERR,"\t- i_cnt_dci_wd_coef2      : 0x%x \n", (reg_val>> 0)&0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- i_dci_full_empty_flag   : 0x%x \n", reg_val);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void DisplayVceEncDebugDCILast(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 5 : DCI LAST DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,5 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_cnt_dci_last_rdata[143:112]    : 0x%x \n", reg_val);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_cnt_dci_last_rdata[111: 96]    : 0x%x \n", reg_val & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cnt_dci_last_rdata[ 95: 64]    : 0x%x \n", reg_val);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- i_cnt_dci_last_rdata[ 63: 32]    : 0x%x \n", reg_val);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- i_cnt_dci_last_rdata[ 31:  0]    : 0x%x \n", reg_val);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- i_wr_read_point                  : 0x%x \n", (reg_val >> 16) & 0x7ff );
    VLOG(ERR,"\t- i_wr_read_point_limit            : 0x%x \n", (reg_val >>  0) & 0x7ff );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[9]);
    VLOG(ERR,"\t- i_sbuf_raddr_store               : 0x%x \n", (reg_val >>  0) & 0x3f );
    VLOG(ERR,"\t- i_read_point                     : 0x%x \n", (reg_val >>  8) & 0x1f );
    VLOG(ERR,"\t- i_dci_write_addr_b               : 0x%x \n", (reg_val >> 16) & 0x3f );
    VLOG(ERR,"\t- i_dci_write_addr_c               : 0x%x \n", (reg_val >> 24) & 0x1f );
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void DisplayVceEncDebugBigBufferCnt(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 6 : BIG BUFFER CNT DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,6 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_cnt_bbuf_read_tuh    : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_bbuf_write_tuh   : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_cnt_bbuf_read_cu     : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_bbuf_write_cu    : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cnt_bbuf_read_coef   : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_bbuf_write_coef  : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- i_cnt_sbuf_read_tuh    : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_sbuf_write_tuh   : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- i_cnt_sbuf_read_cu     : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_sbuf_write_cu    : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- i_cnt_sbuf_read_ctu    : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_sbuf_write_tcu   : 0x%x \n", (reg_val >>  0) & 0xffff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[9]);
    VLOG(ERR,"\t- i_cnt_sbuf_read_coef   : 0x%x \n", (reg_val >> 16) & 0xffff);
    VLOG(ERR,"\t- i_cnt_sbuf_write_coef  : 0x%x \n", (reg_val >>  0) & 0xffff);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void DisplayVceEncDebugBigBufferAddr(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 6 : BIG BUFFER ADDR DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,7 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_read_tuh    : 0x%x \n", (reg_val >> 16) & 0x7ff);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_write_tuh   : 0x%x \n", (reg_val >>  0) & 0x7ff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_read_cu     : 0x%x \n", (reg_val >> 16) & 0x1ff);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_write_cu    : 0x%x \n", (reg_val >>  0) & 0x1ff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_read_coef   : 0x%x \n", (reg_val >> 16) & 0xfff);
    VLOG(ERR,"\t- i_cnt_bbuf_raddr_write_coef  : 0x%x \n", (reg_val >>  0) & 0xfff);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_read_tuh    : 0x%x \n", (reg_val >> 16) & 0x1f);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_write_tuh   : 0x%x \n", (reg_val >>  0) & 0x1f);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_read_cu     : 0x%x \n", (reg_val >> 16) & 0x1f);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_write_cu    : 0x%x \n", (reg_val >>  0) & 0x1f);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_read_ctu    : 0x%x \n", (reg_val >> 16) & 0x1f);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_write_tcu   : 0x%x \n", (reg_val >>  0) & 0x1f);

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[9]);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_read_coef   : 0x%x \n", (reg_val >> 16) & 0x1f);
    VLOG(ERR,"\t- i_cnt_sbuf_raddr_write_coef  : 0x%x \n", (reg_val >>  0) & 0x1f);
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void DisplayVceEncDebugSubWb(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 7 : SUB_WB DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,8 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- subwb_debug_0              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- subwb_debug_1              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- subwb_debug_2              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- subwb_debug_3              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- subwb_debug_4              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- int_sync_ypos              : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[9]);
    VLOG(ERR,"\t- pic_run_cnt                : 0x%x \n", ((reg_val) >> 0) & 0xffff);
    VLOG(ERR,"\t- pic_init_ct                : 0x%x \n", ((reg_val) >> 16) & 0xffff);

    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}

static void DisplayVceEncDebugFBC(int core_idx, int vcore_idx, int set_mode, int* debug)
{
    int reg_val;
    VLOG(ERR,"----------- MODE 8 : FBC DEBUG INFO----------\n");

    WriteRegVCE(core_idx,vcore_idx, set_mode,9 );

    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[3]);
    VLOG(ERR,"\t- ofs_request_count            : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[4]);
    VLOG(ERR,"\t- ofs_bvalid_count             : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[5]);
    VLOG(ERR,"\t- dat_request_count            : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[6]);
    VLOG(ERR,"\t- dat_bvalid_count             : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[7]);
    VLOG(ERR,"\t- fbc_debug                    : 0x%x \n", ((reg_val) >> 0) &  0x3FFFFFFF);
    VLOG(ERR,"\t- fbc_cr_idle_3d               : 0x%x \n", ((reg_val) >> 30) & 0x1);
    VLOG(ERR,"\t- fbc_cr_busy_3d               : 0x%x \n", ((reg_val) >> 31) & 0x1);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[8]);
    VLOG(ERR,"\t- outbuf_debug                 : 0x%x \n", reg_val);
    reg_val = ReadRegVCE(core_idx, vcore_idx, debug[9]);
    VLOG(ERR,"\t- fbcif_debug                  : 0x%x \n", reg_val);

    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
    VLOG(ERR, "\n");
}


void PrintVpuStatus(
    Uint32 coreIdx,
    Uint32 productId
    )
{
    SetClockGate(coreIdx, 1);
    if (PRODUCT_ID_W_SERIES(productId))
    {
        int      rd, wr;
        Uint32    tq, ip, mc, lf;
        Uint32    avail_cu, avail_tu, avail_tc, avail_lf, avail_ip;
        Uint32   ctu_fsm, nb_fsm, cabac_fsm, cu_info, mvp_fsm, tc_busy, lf_fsm, bs_data, bbusy, fv;
        Uint32    reg_val;
        Uint32    index;
        Uint32    vcpu_reg[31]= {0,};

        VLOG(ERR,"-------------------------------------------------------------------------------\n");
        VLOG(ERR,"------                            VCPU STATUS(DEC)                        -----\n");
        VLOG(ERR,"-------------------------------------------------------------------------------\n");
        rd = VpuReadReg(coreIdx, W4_BS_RD_PTR);
        wr = VpuReadReg(coreIdx, W4_BS_WR_PTR);
        VLOG(ERR,"RD_PTR: 0x%08x WR_PTR: 0x%08x BS_OPT: 0x%08x BS_PARAM: 0x%08x\n",
            rd, wr, VpuReadReg(coreIdx, W4_BS_OPTION), VpuReadReg(coreIdx, W4_BS_PARAM));

        // --------- VCPU register Dump
        VLOG(ERR,"[+] VCPU REG Dump\n");
        for (index = 0; index < 25; index++) {
            VpuWriteReg (coreIdx, 0x14, (1<<9) | (index & 0xff));
            vcpu_reg[index] = VpuReadReg (coreIdx, 0x1c);

            if (index < 16) {
                VLOG(ERR,"0x%08x\t",  vcpu_reg[index]);
                if ((index % 4) == 3) VLOG(ERR,"\n");
            }
            else {
                switch (index) {
                case 16: VLOG(ERR,"CR0: 0x%08x\t", vcpu_reg[index]); break;
                case 17: VLOG(ERR,"CR1: 0x%08x\n", vcpu_reg[index]); break;
                case 18: VLOG(ERR,"ML:  0x%08x\t", vcpu_reg[index]); break;
                case 19: VLOG(ERR,"MH:  0x%08x\n", vcpu_reg[index]); break;
                case 21: VLOG(ERR,"LR:  0x%08x\n", vcpu_reg[index]); break;
                case 22: VLOG(ERR,"PC:  0x%08x\n", vcpu_reg[index]);break;
                case 23: VLOG(ERR,"SR:  0x%08x\n", vcpu_reg[index]);break;
                case 24: VLOG(ERR,"SSP: 0x%08x\n", vcpu_reg[index]);break;
                }
            }
        }

        VLOG(ERR,"[-] VCPU REG Dump\n");
        // --------- BIT register Dump
        VLOG(ERR,"[+] BPU REG Dump\n");
        for (index=0;index < 30; index++)
        {
            unsigned int temp;
            temp = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x18));
            VLOG(ERR,"BITPC = 0x%08x\n", temp);
            if ( temp == 0xffffffff)
                return;
        }
        VLOG(ERR,"BIT START=0x%08x, BIT END=0x%08x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x11c)),
            vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x120)) );
        if (productId == PRODUCT_ID_410 )
            VLOG(ERR,"BIT COMMAND 0x%x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x100)));
        if (productId == PRODUCT_ID_4102 || productId == PRODUCT_ID_510)
            VLOG(ERR,"BIT COMMAND 0x%x\n", vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x1FC)));


        //DECODER SDMA INFO
        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x120);
        while((vdi_fio_read_register(0,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(0,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_LOAD_CMD    = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x121);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_AURO_MODE  = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x122);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_BASE_ADDR  = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x123);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_ENC_ADDR   = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x124);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_ENDIAN     = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x125);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_IRQ_CLEAR  = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x126);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_BUSY       = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x127);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_LAST_ADDR  = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x128);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_SC_BASE_ADDR = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x129);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_RD_SEL      = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x130);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SDMA_WR_SEL      = 0x%x\n",reg_val);

        //DECODER SHU INFO
        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x140);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_INIT         = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x141);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_SEEK_NXT_NAL = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x142);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_RD_NAL_ADDR  = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x143);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_STATUS       = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x144);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_GBYTE0       = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x145);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_GBYTE1       = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x146);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_GBYTE2       = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x147);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_GBYTE3       = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x148);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_GBYTE4       = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x14C);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_SBYTE_LOW   = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x14D);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_SBYTE_HIGH  = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x14E);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_ST_PAT_DIS  = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x150);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF0      = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x151);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF1       = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x152);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF2       = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x153);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF3       = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x15C);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF_RPTR   = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x15D);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_NBUF_WPTR   = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x15E);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_REMAIN_BYTE = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x15F);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_CONSUME_BYTE= 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x160);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_RD_SEL     = 0x%x\n",reg_val);

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x161);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"C_SHU_WR_SEL     = 0x%x\n",reg_val);


        // --------- BIT HEVC Status Dump
        ctu_fsm     = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x48));
        nb_fsm      = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x4c));
        cabac_fsm   = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x50));
        cu_info     = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x54));
        mvp_fsm     = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x58));
        tc_busy     = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x5c));
        lf_fsm      = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x60));
        bs_data     = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x64));
        bbusy       = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x68));
        fv          = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x6C));


        VLOG(ERR,"[DEBUG-BPUHEVC] CTU_X: %4d, CTU_Y: %4d\n",  vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x40)), vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x44)));
        VLOG(ERR,"[DEBUG-BPUHEVC] CTU_FSM>   Main: 0x%02x, FIFO: 0x%1x, NB: 0x%02x, DBK: 0x%1x\n", ((ctu_fsm >> 24) & 0xff), ((ctu_fsm >> 16) & 0xff), ((ctu_fsm >> 8) & 0xff), (ctu_fsm & 0xff));
        VLOG(ERR,"[DEBUG-BPUHEVC] NB_FSM:   0x%02x\n", nb_fsm & 0xff);
        VLOG(ERR,"[DEBUG-BPUHEVC] CABAC_FSM> SAO: 0x%02x, CU: 0x%02x, PU: 0x%02x, TU: 0x%02x, EOS: 0x%02x\n", ((cabac_fsm>>25) & 0x3f), ((cabac_fsm>>19) & 0x3f), ((cabac_fsm>>13) & 0x3f), ((cabac_fsm>>6) & 0x7f), (cabac_fsm & 0x3f));
        VLOG(ERR,"[DEBUG-BPUHEVC] CU_INFO value = 0x%04x \n\t\t(l2cb: 0x%1x, cux: %1d, cuy; %1d, pred: %1d, pcm: %1d, wr_done: %1d, par_done: %1d, nbw_done: %1d, dec_run: %1d)\n", cu_info,
            ((cu_info>> 16) & 0x3), ((cu_info>> 13) & 0x7), ((cu_info>> 10) & 0x7), ((cu_info>> 9) & 0x3), ((cu_info>> 8) & 0x1), ((cu_info>> 6) & 0x3), ((cu_info>> 4) & 0x3), ((cu_info>> 2) & 0x3), (cu_info & 0x3));
        VLOG(ERR,"[DEBUG-BPUHEVC] MVP_FSM> 0x%02x\n", mvp_fsm & 0xf);
        VLOG(ERR,"[DEBUG-BPUHEVC] TC_BUSY> tc_dec_busy: %1d, tc_fifo_busy: 0x%02x\n", ((tc_busy >> 3) & 0x1), (tc_busy & 0x7));
        VLOG(ERR,"[DEBUG-BPUHEVC] LF_FSM>  SAO: 0x%1x, LF: 0x%1x\n", ((lf_fsm >> 4) & 0xf), (lf_fsm  & 0xf));
        VLOG(ERR,"[DEBUG-BPUHEVC] BS_DATA> ExpEnd=%1d, bs_valid: 0x%03x, bs_data: 0x%03x\n", ((bs_data >> 31) & 0x1), ((bs_data >> 16) & 0xfff), (bs_data & 0xfff));
        VLOG(ERR,"[DEBUG-BPUHEVC] BUS_BUSY> mib_wreq_done: %1d, mib_busy: %1d, sdma_bus: %1d\n", ((bbusy >> 2) & 0x1), ((bbusy >> 1) & 0x1) , (bbusy & 0x1));
        VLOG(ERR,"[DEBUG-BPUHEVC] FIFO_VALID> cu: %1d, tu: %1d, iptu: %1d, lf: %1d, coff: %1d\n\n", ((fv >> 4) & 0x1), ((fv >> 3) & 0x1), ((fv >> 2) & 0x1), ((fv >> 1) & 0x1), (fv & 0x1));
        VLOG(ERR,"[-] BPU REG Dump\n");

        // --------- VCE register Dump
        VLOG(ERR,"[+] VCE REG Dump\n");
        tq = ReadRegVCE(coreIdx, 0, 0xd0);
        ip = ReadRegVCE(coreIdx, 0, 0xd4);
        mc = ReadRegVCE(coreIdx, 0, 0xd8);
        lf = ReadRegVCE(coreIdx, 0, 0xdc);
        avail_cu = (ReadRegVCE(coreIdx, 0, 0x11C)>>16) - (ReadRegVCE(coreIdx, 0, 0x110)>>16);
        avail_tu = (ReadRegVCE(coreIdx, 0, 0x11C)&0xFFFF) - (ReadRegVCE(coreIdx, 0, 0x110)&0xFFFF);
        avail_tc = (ReadRegVCE(coreIdx, 0, 0x120)>>16) - (ReadRegVCE(coreIdx, 0, 0x114)>>16);
        avail_lf = (ReadRegVCE(coreIdx, 0, 0x120)&0xFFFF) - (ReadRegVCE(coreIdx, 0, 0x114)&0xFFFF);
        avail_ip = (ReadRegVCE(coreIdx, 0, 0x124)>>16) - (ReadRegVCE(coreIdx, 0, 0x118)>>16);
        VLOG(ERR,"       TQ            IP              MC             LF      GDI_EMPTY          ROOM \n");
        VLOG(ERR,"------------------------------------------------------------------------------------------------------------\n");
        SetClockGate(coreIdx, 1);
        VLOG(ERR,"| %d %04d %04d | %d %04d %04d |  %d %04d %04d | %d %04d %04d | 0x%08x | CU(%d) TU(%d) TC(%d) LF(%d) IP(%d)\n",
            (tq>>22)&0x07, (tq>>11)&0x3ff, tq&0x3ff,
            (ip>>22)&0x07, (ip>>11)&0x3ff, ip&0x3ff,
            (mc>>22)&0x07, (mc>>11)&0x3ff, mc&0x3ff,
            (lf>>22)&0x07, (lf>>11)&0x3ff, lf&0x3ff,
            vdi_fio_read_register(coreIdx, 0x88f4),                      /* GDI empty */
            avail_cu, avail_tu, avail_tc, avail_lf, avail_ip);
        /* CU/TU Queue count */
        reg_val = ReadRegVCE(coreIdx, 0, 0x12C);
        VLOG(ERR,"[DCIDEBUG] QUEUE COUNT: CU(%5d) TU(%5d) ", (reg_val>>16)&0xffff, reg_val&0xffff);
        reg_val = ReadRegVCE(coreIdx, 0, 0x1A0);
        VLOG(ERR,"TC(%5d) IP(%5d) ", (reg_val>>16)&0xffff, reg_val&0xffff);
        reg_val = ReadRegVCE(coreIdx, 0, 0x1A4);
        VLOG(ERR,"LF(%5d)\n", (reg_val>>16)&0xffff);
        VLOG(ERR,"VALID SIGNAL : CU0(%d)  CU1(%d)  CU2(%d) TU(%d) TC(%d) IP(%5d) LF(%5d)\n"
            "               DCI_FALSE_RUN(%d) VCE_RESET(%d) CORE_INIT(%d) SET_RUN_CTU(%d) \n",
            (reg_val>>6)&1, (reg_val>>5)&1, (reg_val>>4)&1, (reg_val>>3)&1,
            (reg_val>>2)&1, (reg_val>>1)&1, (reg_val>>0)&1,
            (reg_val>>10)&1, (reg_val>>9)&1, (reg_val>>8)&1, (reg_val>>7)&1);

        VLOG(ERR,"State TQ: 0x%08x IP: 0x%08x MC: 0x%08x LF: 0x%08x\n",
            ReadRegVCE(coreIdx, 0, 0xd0), ReadRegVCE(coreIdx, 0, 0xd4), ReadRegVCE(coreIdx, 0, 0xd8), ReadRegVCE(coreIdx, 0, 0xdc));
        VLOG(ERR,"BWB[1]: RESPONSE_CNT(0x%08x) INFO(0x%08x)\n", ReadRegVCE(coreIdx, 0, 0x194), ReadRegVCE(coreIdx, 0, 0x198));
        VLOG(ERR,"BWB[2]: RESPONSE_CNT(0x%08x) INFO(0x%08x)\n", ReadRegVCE(coreIdx, 0, 0x194), ReadRegVCE(coreIdx, 0, 0x198));
        VLOG(ERR,"DCI INFO\n");
        VLOG(ERR,"READ_CNT_0 : 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x110));
        VLOG(ERR,"READ_CNT_1 : 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x114));
        VLOG(ERR,"READ_CNT_2 : 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x118));
        VLOG(ERR,"WRITE_CNT_0: 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x11c));
        VLOG(ERR,"WRITE_CNT_1: 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x120));
        VLOG(ERR,"WRITE_CNT_2: 0x%08x\n", ReadRegVCE(coreIdx, 0, 0x124));
        reg_val = ReadRegVCE(coreIdx, 0, 0x128);
        VLOG(ERR,"LF_DEBUG_PT: 0x%08x\n", reg_val & 0xffffffff);
        VLOG(ERR,"cur_main_state %2d, r_lf_pic_deblock_disable %1d, r_lf_pic_sao_disable %1d\n",
            (reg_val >> 16) & 0x1f,
            (reg_val >> 15) & 0x1,
            (reg_val >> 14) & 0x1);
        VLOG(ERR,"para_load_done %1d, i_rdma_ack_wait %1d, i_sao_intl_col_done %1d, i_sao_outbuf_full %1d\n",
            (reg_val >> 13) & 0x1,
            (reg_val >> 12) & 0x1,
            (reg_val >> 11) & 0x1,
            (reg_val >> 10) & 0x1);
        VLOG(ERR,"lf_sub_done %1d, i_wdma_ack_wait %1d, lf_all_sub_done %1d, cur_ycbcr %1d, sub8x8_done %2d\n",
            (reg_val >> 9) & 0x1,
            (reg_val >> 8) & 0x1,
            (reg_val >> 6) & 0x1,
            (reg_val >> 4) & 0x1,
            reg_val & 0xf);
        VLOG(ERR,"[-] VCE REG Dump\n");
        VLOG(ERR,"[-] VCE REG Dump\n");

        VLOG(ERR,"-------------------------------------------------------------------------------\n");
    }
    if (productId == PRODUCT_ID_420 || productId == PRODUCT_ID_420L || productId == PRODUCT_ID_420SN || productId == PRODUCT_ID_420DU)
    {
        int       rd, wr;
        Uint32    reg_val, num;
        int       vce_enc_debug[12] = {0, };
        int       set_mode;
        int       vcore_num = 1;
        int       vcore_idx;
        Uint32    index;
        Uint32    vcpu_reg[31]= {0,};


        SetClockGate(coreIdx, 1);
        VLOG(ERR,"-------------------------------------------------------------------------------\n");
        VLOG(ERR,"------                            VCPU STATUS(ENC)                        -----\n");
        VLOG(ERR,"-------------------------------------------------------------------------------\n");
        rd = VpuReadReg(coreIdx, W4_BS_RD_PTR);
        wr = VpuReadReg(coreIdx, W4_BS_WR_PTR);
        VLOG(ERR,"RD_PTR: 0x%08x WR_PTR: 0x%08x BS_OPT: 0x%08x BS_PARAM: 0x%08x\n",
            rd, wr, VpuReadReg(coreIdx, W4_BS_OPTION), VpuReadReg(coreIdx, W4_BS_PARAM));

        // --------- VCPU register Dump
        VLOG(ERR,"[+] VCPU REG Dump\n");
        for (index = 0; index < 25; index++) {
            VpuWriteReg (coreIdx, W4_VPU_PDBG_IDX_REG, (1<<9) | (index & 0xff));
            vcpu_reg[index] = VpuReadReg (coreIdx, W4_VPU_PDBG_RDATA_REG);

            if (index < 16) {
                VLOG(ERR,"0x%08x\t",  vcpu_reg[index]);
                if ((index % 4) == 3) VLOG(ERR,"\n");
            }
            else {
                switch (index) {
                case 16: VLOG(ERR,"CR0: 0x%08x\t", vcpu_reg[index]); break;
                case 17: VLOG(ERR,"CR1: 0x%08x\n", vcpu_reg[index]); break;
                case 18: VLOG(ERR,"ML:  0x%08x\t", vcpu_reg[index]); break;
                case 19: VLOG(ERR,"MH:  0x%08x\n", vcpu_reg[index]); break;
                case 21: VLOG(ERR,"LR:  0x%08x\n", vcpu_reg[index]); break;
                case 22: VLOG(ERR,"PC:  0x%08x\n", vcpu_reg[index]);break;
                case 23: VLOG(ERR,"SR:  0x%08x\n", vcpu_reg[index]);break;
                case 24: VLOG(ERR,"SSP: 0x%08x\n", vcpu_reg[index]);break;
                }
            }
        }
        VLOG(ERR,"[-] VCPU REG Dump\n");
        VLOG(ERR,"vce run flag = %d\n", VpuReadReg(coreIdx, 0x1E8));
        // --------- BIT register Dump
        VLOG(ERR,"[+] BPU REG Dump\n");
        for (index=0;index < 30; index++)
        {
            unsigned int temp;
#if defined(WAVE420SN) || defined(WAVE420DU)
            int temp2;
            temp2 = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x9000 + 0x18));
#endif
            temp = vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x18));
#if defined(WAVE420SN) || defined(WAVE420DU)
            VLOG(ERR,"BITPC_0 = 0x%08x, BITPC_1 = 0x%08x\n", temp, temp2);
#else
            VLOG(ERR,"BITPC = 0x%08x\n", temp);
#endif
            if ( temp == 0xffffffff)
                return;
#if defined(WAVE420SN) || defined(WAVE420DU)
            if ( temp2 == 0xffffffff)
                return;
#endif
        }

        // --------- BIT HEVC Status Dump
        VLOG(ERR,"==================================\n");
        VLOG(ERR,"[-] BPU REG Dump\n");
        VLOG(ERR,"==================================\n");

        VLOG(ERR,"DBG_FIFO_VALID        [%08x]\n",vdi_fio_read_register(coreIdx, (W4_REG_BASE + 0x8000 + 0x6C)));

        //SDMA debug information
        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20) | (1<<16)| 0x13c);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SDMA_DBG_INFO     [%08x]\n", reg_val);
        VLOG(ERR,"\t- [   28] need_more_update  : 0x%x \n", (reg_val>>28)&0x1 );
        VLOG(ERR,"\t- [27:25] tr_init_fsm       : 0x%x \n", (reg_val>>25)&0x7 );
        VLOG(ERR,"\t- [24:18] remain_trans_size : 0x%x \n", (reg_val>>18)&0x7F);
        VLOG(ERR,"\t- [17:13] wdata_out_cnt     : 0x%x \n", (reg_val>>13)&0x1F);
        VLOG(ERR,"\t- [12:10] wdma_wd_fsm       : 0x%x \n", (reg_val>>10)&0x1F);
        VLOG(ERR,"\t- [ 9: 7] wdma_wa_fsm       : 0x%x ", (reg_val>> 7)&0x7 );
        if (((reg_val>>7) &0x7) == 3)
            VLOG(ERR,"-->WDMA_WAIT_ADDR  \n");
        else
            VLOG(ERR,"\n");
        VLOG(ERR,"\t- [ 6: 5] sdma_init_fsm     : 0x%x \n", (reg_val>> 5)&0x3 );
        VLOG(ERR,"\t- [ 4: 1] save_fsm          : 0x%x \n", (reg_val>> 1)&0xF );
        VLOG(ERR,"\t- [    0] unalign_written   : 0x%x \n", (reg_val>> 0)&0x1 );


        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16)| 0x13b);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SDMA_NAL_MEM_INF  [%08x]\n", reg_val);
        VLOG(ERR,"\t- [ 7: 1] nal_mem_empty_room : 0x%x \n", (reg_val>> 1)&0x3F);
        VLOG(ERR,"\t- [    0] ge_wnbuf_size      : 0x%x \n", (reg_val>> 0)&0x1 );


        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x131);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SDMA_IRQ      [%08x]: [1]sdma_irq : 0x%x, [2]enable_sdma_irq : 0x%x\n",reg_val, (reg_val >> 1)&0x1,(reg_val &0x1));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x134);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SDMA_BS_BASE_ADDR [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x135);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SDMA_NAL_STR_ADDR [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x136);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SDMA_IRQ_ADDR     [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x137);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SDMA_BS_END_ADDR  [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x13A);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SDMA_CUR_ADDR     [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x139);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SDMA_STATUS           [%08x]\n",reg_val);
        VLOG(ERR,"\t- [2] all_wresp_done : 0x%x \n", (reg_val>> 2)&0x1);
        VLOG(ERR,"\t- [1] sdma_init_busy : 0x%x \n", (reg_val>> 1)&0x1 );
        VLOG(ERR,"\t- [0] save_busy      : 0x%x \n", (reg_val>> 0)&0x1 );


        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x164);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SHU_DBG               [%08x] : shu_unaligned_num (0x%x)\n",reg_val, reg_val);


        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x169);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SHU_NBUF_WPTR     [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x16A);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"SHU_NBUF_RPTR     [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x16C);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78));
        VLOG(ERR,"SHU_NBUF_INFO     [%08x]\n",reg_val);
        VLOG(ERR,"\t- [5:1] nbuf_remain_byte : 0x%x \n", (reg_val>> 1)&0x1F);
        VLOG(ERR,"\t- [  0] nbuf_wptr_wrap   : 0x%x \n", (reg_val>> 0)&0x1 );




        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x184);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"CTU_LAST_ENC_POS  [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x187);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"CTU_POS_IN_PIC        [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));


        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x110);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"MIB_EXTADDR           [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x111);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"MIB_INTADDR           [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x113);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"MIB_CMD               [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        vdi_fio_write_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x74),(1<<20)| (1<<16) | 0x114);
        while((vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x7c))& 0x1) ==0 );
        VLOG(ERR,"MIB_BUSY          [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x78)));

        VLOG(ERR,"DBG_BPU_ENC_NB0       [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x40)));
        VLOG(ERR,"DBG_BPU_CTU_CTRL0 [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x44)));
        VLOG(ERR,"DBG_BPU_CAB_FSM0  [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x48)));
        VLOG(ERR,"DBG_BPU_BIN_GEN0  [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x4C)));
        VLOG(ERR,"DBG_BPU_CAB_MBAE0 [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x50)));
        VLOG(ERR,"DBG_BPU_BUS_BUSY  [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x68)));
        VLOG(ERR,"DBG_FIFO_VALID        [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x6C)));
        VLOG(ERR,"DBG_BPU_CTU_CTRL1 [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x54)));
        VLOG(ERR,"DBG_BPU_CTU_CTRL2 [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x58)));
        VLOG(ERR,"DBG_BPU_CTU_CTRL3 [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x5C)));


        for (index=0x80; index<0xA0; index+=4) {
            reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + index));
            num     = (index - 0x80)/2;
            VLOG(ERR,"DBG_BIT_STACK     [%08x] : Stack%02d (0x%04x), Stack%02d(0x%04x) \n",reg_val, num, reg_val>>16, num+1, reg_val & 0xffff);
        }

        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xA0));
        VLOG(ERR,"DGB_BIT_CORE_INFO [%08x] : pc_ctrl_id (0x%04x), pfu_reg_pc(0x%04x)\n",reg_val,reg_val>>16, reg_val & 0xffff);
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xA4));
        VLOG(ERR,"DGB_BIT_CORE_INFO [%08x] : ACC0 (0x%08x)\n",reg_val, reg_val);
        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xA8));
        VLOG(ERR,"DGB_BIT_CORE_INFO [%08x] : ACC1 (0x%08x)\n",reg_val, reg_val);

        reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xAC));
        VLOG(ERR,"DGB_BIT_CORE_INFO [%08x] : pfu_ibuff_id(0x%04x), pfu_ibuff_op(0x%04x)\n",reg_val,reg_val>>16, reg_val & 0xffff);

        for (num=0; num<5; num+=1) {
            reg_val = vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xB0));
            VLOG(ERR,"DGB_BIT_CORE_INFO [%08x] : core_pram_rd_en(0x%04x), core_pram_rd_addr(0x%04x)\n",reg_val,reg_val>>16, reg_val & 0xffff);
        }

        VLOG(ERR,"SAO_LUMA_OFFSET   [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xB4)));
        VLOG(ERR,"SAO_CB_OFFSET [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xB8)));
        VLOG(ERR,"SAO_CR_OFFSET [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0xBC)));

        VLOG(ERR,"GDI_NO_MORE_REQ       [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x8f0)));
        VLOG(ERR,"GDI_EMPTY_FLAG        [%08x]\n",vdi_fio_read_register(coreIdx,(W4_REG_BASE + 0x8000 + 0x8f4)));

        if ( productId == PRODUCT_ID_420) {
            VLOG(ERR,"WAVE420_CODE VCE DUMP\n");
            vce_enc_debug[0] = 0x0bc8;//MODE SEL
            vce_enc_debug[1] = 0x0be0;
            vce_enc_debug[2] = 0x0bcc;
            vce_enc_debug[3] = 0x0be4;
            vce_enc_debug[4] = 0x0be8;
            vce_enc_debug[5] = 0x0bec;
            vce_enc_debug[6] = 0x0bc0;
            vce_enc_debug[7] = 0x0bc4;
            vce_enc_debug[8] = 0x0bf0;
            vce_enc_debug[9] = 0x0bf4;
            set_mode         = 0x0bc8;
            vcore_num        = 1;
        }
        else if (productId == PRODUCT_ID_420L) {
            VLOG(ERR,"WAVE420L_CODE VCE DUMP\n");
            vce_enc_debug[0] = 0x0bd0;//MODE SEL
            vce_enc_debug[1] = 0x0bd4;
            vce_enc_debug[2] = 0x0bd8;
            vce_enc_debug[3] = 0x0bdc;
            vce_enc_debug[4] = 0x0be0;
            vce_enc_debug[5] = 0x0be4;
            vce_enc_debug[6] = 0x0be8;
            vce_enc_debug[7] = 0x0bc4;
            vce_enc_debug[8] = 0x0bf0;
            vce_enc_debug[9] = 0x0bf4;
            set_mode         = 0x0bd0;
            vcore_num        = 1;
        }
        else if ( productId == PRODUCT_ID_420SN || productId == PRODUCT_ID_420DU) {
            VLOG(ERR,"WAVE420SN/DUAL_CODE VCE DUMP\n");
            vce_enc_debug[0] = 0x0bf0;//MODE SEL
            vce_enc_debug[1] = 0x0bf4;
            vce_enc_debug[2] = 0x0ba0;
            vce_enc_debug[3] = 0x0ba4;
            vce_enc_debug[4] = 0x0ba8;
            vce_enc_debug[5] = 0x0bac;
            vce_enc_debug[6] = 0x0bc0;
            vce_enc_debug[7] = 0x0bc4;
            vce_enc_debug[8] = 0x0bc8;
            vce_enc_debug[9] = 0x0bcc;
            set_mode         = 0x0bf0;
            vcore_num        = 2;
        }

        {
            int reg_val;
            VLOG(ERR,"==========================================\n");
            VLOG(ERR,"[+] AXI BUS INFO");
            VLOG(ERR,"==========================================\n");
            //reg_val = ReadRegVCE(coreIdx, 6, 0x104);
            reg_val = vdi_fio_read_register(coreIdx, W4_FIO_VCORE6_BUS_CHANNEL_SELECTOR);

            VLOG(ERR,"\t- w_fbd1  _read  _idle           :  0x%x\n", (reg_val >> 0 )  &0x1)  ;
            VLOG(ERR,"\t- w_fbd0  _read  _idle           :  0x%x\n", (reg_val >> 4 )  &0x1)  ;
            VLOG(ERR,"\t- w_fbc   _write2_idle           :  0x%x\n", (reg_val >> 9 )  &0x1)  ;
            VLOG(ERR,"\t- w_fbc   _write _idle           :  0x%x\n", (reg_val >>10 )  &0x1)  ;
            VLOG(ERR,"\t- w_vcpu  _read  _idle           :  0x%x\n", (reg_val >>12 )  &0x1)  ;
            VLOG(ERR,"\t- w_vcpu  _write2_idle           :  0x%x\n", (reg_val >>13 )  &0x1)  ;
            VLOG(ERR,"\t- w_vcpu  _write _idle           :  0x%x\n", (reg_val >>14 )  &0x1)  ;
            VLOG(ERR,"\t- w_vcore1_read  _idle           :  0x%x\n", (reg_val >>16 )  &0x1)  ;
            VLOG(ERR,"\t- w_vcore1_write2_idle           :  0x%x\n", (reg_val >>17 )  &0x1)  ;
            VLOG(ERR,"\t- w_vcore1_write _idle           :  0x%x\n", (reg_val >>18 )  &0x1)  ;
            VLOG(ERR,"\t- w_vcore0_read  _idle           :  0x%x\n", (reg_val >>20 )  &0x1)  ;
            VLOG(ERR,"\t- w_vcore0_write2_idle           :  0x%x\n", (reg_val >>21 )  &0x1)  ;
            VLOG(ERR,"\t- w_vcore0_write _idle           :  0x%x\n", (reg_val >>22 )  &0x1)  ;
        }

        for (vcore_idx = 0; vcore_idx < vcore_num ; vcore_idx++)
        {
            VLOG(ERR,"==========================================\n");
            VLOG(ERR,"[+] VCE REG Dump VCORE_IDX : %d\n",vcore_idx);
            VLOG(ERR,"==========================================\n");
            DisplayVceEncDebugCommon         (coreIdx, vcore_idx, set_mode, vce_enc_debug[0], vce_enc_debug[1], vce_enc_debug[2]);
            DisplayVceEncDebugDCI            (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugRDO            (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugLF             (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugSFU            (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugDCI2           (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugDCILast        (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugBigBufferCnt   (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugBigBufferAddr  (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugSubWb          (coreIdx, vcore_idx, set_mode, vce_enc_debug);
            DisplayVceEncDebugFBC            (coreIdx, vcore_idx, set_mode, vce_enc_debug);
        }

    }
    else
    {
    }
    SetClockGate(coreIdx, 0);
}





void ChangePathStyle(
    char *str
    )
{
    UNREFERENCED_PARAMETER(str);
}


void ReleaseVideoMemory(
    uint32_t        coreIndex,
    vpu_buffer_t*   memoryArr,
    uint32_t        count
    )
{
    uint32_t    index;

    for (index=0; index<count; index++) {
        if (memoryArr[index].size)
            vdi_free_dma_memory(coreIndex, &memoryArr[index]);
    }
}

BOOL AllocateDecFrameBuffer(
    DecHandle       decHandle,
    TestDecConfig*  config,
    uint32_t        tiledFbCount,
    uint32_t        linearFbCount,
    FrameBuffer*    retFbArray,
    vpu_buffer_t*   retFbAddrs,
    uint32_t*       retStride
    )
{
    uint32_t                framebufSize;
    uint32_t                totalFbCount;
    uint32_t                coreIndex;
    uint32_t                index;
    FrameBufferFormat       format = config->wtlFormat;
    DecInitialInfo          seqInfo;
    FrameBufferAllocInfo    fbAllocInfo;
    RetCode                 ret;
    vpu_buffer_t*           pvb;
    size_t                  framebufStride;
    size_t                  framebufHeight;
    uint32_t                productId;
    DRAMConfig*             pDramCfg        = NULL;
    DRAMConfig              dramCfg         = {0};

    coreIndex = VPU_HANDLE_CORE_INDEX(decHandle);
    productId = VPU_HANDLE_PRODUCT_ID(decHandle);
    VPU_DecGiveCommand(decHandle, DEC_GET_SEQ_INFO, (void*)&seqInfo);

    if (productId == PRODUCT_ID_960) {
        pDramCfg = &dramCfg;
        ret = VPU_DecGiveCommand(decHandle, GET_DRAM_CONFIG, pDramCfg);
    }

    totalFbCount = tiledFbCount + linearFbCount;

    if (productId == PRODUCT_ID_4102 || productId == PRODUCT_ID_420 || productId == PRODUCT_ID_412 || productId == PRODUCT_ID_420L || productId == PRODUCT_ID_420SN || productId == PRODUCT_ID_420DU || productId == PRODUCT_ID_510 || productId == PRODUCT_ID_512) {
        format = (seqInfo.lumaBitdepth > 8 || seqInfo.chromaBitdepth > 8) ? FORMAT_420_P10_16BIT_LSB : FORMAT_420;
    }
    else if (productId == PRODUCT_ID_7Q) {
        if (decHandle->codecMode == HEVC_DEC)
            format = (seqInfo.lumaBitdepth > 8 || seqInfo.chromaBitdepth > 8) ? FORMAT_420_P10_16BIT_LSB : FORMAT_420;
        else
            format = FORMAT_420;
    }

    if (decHandle->codecMode == C7_VP9_DEC) {
        framebufStride = CalcStride(VPU_ALIGN64(seqInfo.picWidth), seqInfo.picHeight, format, config->cbcrInterleave, config->mapType, TRUE);
        framebufHeight = VPU_ALIGN64(seqInfo.picHeight);
        framebufSize   = VPU_GetFrameBufSize(decHandle->coreIdx, framebufStride, framebufHeight,
            config->mapType, format, config->cbcrInterleave, NULL);
        *retStride     = framebufStride;
    }
    else if (productId == PRODUCT_ID_7Q && decHandle->codecMode != C7_HEVC_DEC) {
        framebufStride = CalcStride(seqInfo.picWidth, seqInfo.picHeight, format, config->cbcrInterleave, config->mapType, FALSE);
        framebufHeight = seqInfo.interlace ? VPU_ALIGN32(seqInfo.picHeight) : VPU_ALIGN16(seqInfo.picHeight);
        framebufSize   = VPU_GetFrameBufSize(decHandle->coreIdx, framebufStride, framebufHeight,
                                             config->mapType, format, config->cbcrInterleave, NULL);
        *retStride     = framebufStride;
    }
    else {
        *retStride     = VPU_ALIGN32(seqInfo.picWidth);
        framebufStride = CalcStride(seqInfo.picWidth, seqInfo.picHeight, format, config->cbcrInterleave, config->mapType, FALSE);
        framebufHeight = seqInfo.picHeight;
        framebufSize   = VPU_GetFrameBufSize(decHandle->coreIdx, framebufStride, seqInfo.picHeight,
                                             config->mapType, format, config->cbcrInterleave, pDramCfg);
    }

    osal_memset((void*)&fbAllocInfo, 0x00, sizeof(fbAllocInfo));
    osal_memset((void*)retFbArray,   0x00, sizeof(FrameBuffer)*totalFbCount);
    fbAllocInfo.format          = format;
    fbAllocInfo.cbcrInterleave  = config->cbcrInterleave;
    fbAllocInfo.mapType         = config->mapType;
    fbAllocInfo.stride          = framebufStride;
    fbAllocInfo.height          = framebufHeight;
    fbAllocInfo.size            = framebufSize;
    fbAllocInfo.lumaBitDepth    = seqInfo.lumaBitdepth;
    fbAllocInfo.chromaBitDepth  = seqInfo.chromaBitdepth;
    fbAllocInfo.num             = tiledFbCount;
    fbAllocInfo.endian          = config->frameEndian;
    fbAllocInfo.type            = FB_TYPE_CODEC;
    VLOG(INFO, "[helper] DPB stride %d, height %d, framebufSize %d, maptype %d, num %d\n",
        fbAllocInfo.stride, fbAllocInfo.height, fbAllocInfo.size, fbAllocInfo.mapType, fbAllocInfo.num);
    memset((void*)retFbAddrs, 0x00, sizeof(vpu_buffer_t)*totalFbCount);
    for (index=0; index<tiledFbCount; index++) {
        pvb = &retFbAddrs[index];
        pvb->size = framebufSize;
        if (vdi_allocate_dma_memory(coreIndex, pvb) < 0) {
            VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
            ReleaseVideoMemory(coreIndex, retFbAddrs, totalFbCount);
            return FALSE;
        }
        retFbArray[index].bufY  = pvb->phys_addr;
        retFbArray[index].bufCb = (PhysicalAddress)-1;
        retFbArray[index].bufCr = (PhysicalAddress)-1;
        retFbArray[index].updateFbInfo = TRUE;
        retFbArray[index].size  = framebufSize;
    }

    if (tiledFbCount != 0) {
        if ((ret=VPU_DecAllocateFrameBuffer(decHandle, fbAllocInfo, retFbArray)) != RETCODE_SUCCESS) {
            VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer(), ret(%d)\n",
                __FUNCTION__, __LINE__, ret);
            ReleaseVideoMemory(coreIndex, retFbAddrs, totalFbCount);
            return FALSE;
        }
    }

    if (config->enableWTL == TRUE || linearFbCount != 0 || config->pvricFbcEnable == TRUE) {
        size_t  linearStride;
        size_t  picWidth;
        size_t  picHeight;
        size_t  fbHeight;
        uint32_t mapType = LINEAR_FRAME_MAP;
        FrameBufferFormat outFormat = config->wtlFormat;
        if (config->pvricFbcEnable == TRUE) {
            mapType     = PVRIC_COMPRESSED_FRAME_MAP;
            outFormat   = config->wtlFormat;
        }
        picWidth  = seqInfo.picWidth;
        picHeight = seqInfo.picHeight;
        fbHeight  = picHeight;
        if (decHandle->codecMode == C7_VP9_DEC) {
            if (FALSE == config->pvricFbcEnable) {
                fbHeight = VPU_ALIGN64(picHeight);
            } else {
                ScalerInfo sclInfo;
                VPU_DecGiveCommand(decHandle, DEC_GET_SCALER_INFO, (void*)&sclInfo);
                if (sclInfo.sameSize)
                    fbHeight = VPU_ALIGN64(picHeight);
            }
        }
        else if (productId == PRODUCT_ID_7Q && decHandle->codecMode != C7_HEVC_DEC)
        {
            fbHeight  = seqInfo.interlace ? VPU_ALIGN32(picHeight) : VPU_ALIGN16(picHeight);
        }
        else if (productId == PRODUCT_ID_960 || productId == PRODUCT_ID_980)
        {
            fbHeight  = VPU_ALIGN32(picHeight);
        }
        if (config->scaleDownWidth > 0 || config->scaleDownHeight > 0) {
            ScalerInfo sclInfo;
            VPU_DecGiveCommand(decHandle, DEC_GET_SCALER_INFO, (void*)&sclInfo);
            if (sclInfo.enScaler == TRUE) {
                picWidth  = sclInfo.scaleWidth;
                picHeight = sclInfo.scaleHeight;
                if (decHandle->codecMode == C7_VP9_DEC) {
                    if (TRUE == config->pvricFbcEnable) {
                        fbHeight = picHeight;
                        if (sclInfo.sameSize)
                            fbHeight = VPU_ALIGN64(picHeight);
                    } else {
                        fbHeight = VPU_ALIGN64(picHeight);
                    }
                }
                else {
                    fbHeight  = picHeight;
                }
            }
        }
        if (decHandle->codecMode == C7_VP9_DEC) {
            if (TRUE == config->pvricFbcEnable) {
                ScalerInfo sclInfo;
                VP9ScaleBit vp9Scale;
                VPU_DecGiveCommand(decHandle, DEC_GET_SCALER_INFO, (void*)&sclInfo);
                vp9Scale = VP9_SAMESCALE; //1:1 and VP9
                if (sclInfo.sameSize)
                    vp9Scale = VP9_DOWNSCALE;
                linearStride = CalcStride(picWidth, picHeight, outFormat, config->cbcrInterleave, (TiledMapType)mapType, vp9Scale);
            } else {
                linearStride = CalcStride(VPU_ALIGN64(picWidth), picHeight, outFormat, config->cbcrInterleave, (TiledMapType)mapType, TRUE);
            }
        }
        else {
            linearStride = CalcStride(picWidth, picHeight, outFormat, config->cbcrInterleave, (TiledMapType)mapType, FALSE);
        }
        framebufSize = VPU_GetFrameBufSize(coreIndex, linearStride, fbHeight, (TiledMapType)mapType, outFormat, config->cbcrInterleave, pDramCfg);

        for (index=tiledFbCount; index<totalFbCount; index++) {
            pvb = &retFbAddrs[index];
            pvb->size = framebufSize;
            if (vdi_allocate_dma_memory(coreIndex, pvb) < 0) {
                VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                ReleaseVideoMemory(coreIndex, retFbAddrs, totalFbCount);
                return FALSE;
            }
            retFbArray[index].bufY  = pvb->phys_addr;
            retFbArray[index].bufCb = -1;
            retFbArray[index].bufCr = -1;
            retFbArray[index].updateFbInfo = TRUE;
            retFbArray[index].size  = framebufSize;
            if (config->pvricFbcEnable == TRUE) {
                // Intentionally a fb is filled with zero to compare with the golden data.
                vdi_clear_memory(coreIndex, pvb->phys_addr, pvb->size, 0);
            }
        }

        fbAllocInfo.nv21    = config->nv21;
        fbAllocInfo.format  = outFormat;
        fbAllocInfo.num     = linearFbCount;
        fbAllocInfo.mapType = (TiledMapType)mapType;
        fbAllocInfo.stride  = linearStride;
        fbAllocInfo.height  = fbHeight;
        VLOG(INFO, "[helper] WTL stride %d, height %d, framebufSize %d, maptype %d, num %d\n",
            fbAllocInfo.stride, fbAllocInfo.height, framebufSize, fbAllocInfo.mapType, fbAllocInfo.num);
        ret = VPU_DecAllocateFrameBuffer(decHandle, fbAllocInfo, &retFbArray[tiledFbCount]);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer() ret:%d\n",
                __FUNCTION__, __LINE__, ret);
            ReleaseVideoMemory(coreIndex, retFbAddrs, totalFbCount);
            return FALSE;
        }
    }

    return TRUE;
}

#if defined(_WIN32) || defined(__MSDOS__)
#define DOS_FILESYSTEM
#define IS_DIR_SEPARATOR(__c) ((__c == '/') || (__c == '\\'))
#else
/* UNIX style */
#define IS_DIR_SEPARATOR(__c) (__c == '/')
#endif

char* GetDirname(
    const char* path
    )
{
    int length;
    int i;
    char* upper_dir;

    if (path == NULL) return NULL;

    length = strlen(path);
    for (i=length-1; i>=0; i--) {
        if (IS_DIR_SEPARATOR(path[i])) break;
    }

    if (i<0) {
        upper_dir = strdup(".");
    } else {
        upper_dir = strdup(path);
        upper_dir[i] = 0;
    }

    return upper_dir;
}

char* GetBasename(
    const char* pathname
    )
{
    const char* base = NULL;
    const char* p    = pathname;

    if (p == NULL) {
        return NULL;
    }

#if defined(DOS_FILESYSTEM)
    if (isalpha((int)p[0]) && p[1] == ':') {
        p += 2;
    }
#endif

    for (base=p; *p; p++) {
        if (IS_DIR_SEPARATOR(*p)) {
            base = p+1;
        }
    }

    return (char*)base;
}

char* GetFileExtension(
    const char* filename
    )
{
    uint32_t    len;
    uint32_t    i;

    len = strlen(filename);
    for (i=len-1; i>=0; i--) {
        if (filename[i] == '.') {
            return (char*)&filename[i+1];
        }
    }

    return NULL;
}

void byte_swap(unsigned char* data, int len)
{
    Uint8 temp;
    Int32 i;

    for (i=0; i<len; i+=2) {
        temp      = data[i];
        data[i]   = data[i+1];
        data[i+1] = temp;
    }
}

BOOL CalcYuvSize(
    int32_t format,
    int32_t picWidth,
    int32_t picHeight,
    int32_t cbcrInterleave,
    size_t  *lumaSize,
    size_t  *chromaSize,
    size_t  *frameSize,
    int32_t *bitDepth,
    int32_t *packedFormat,
    int32_t *yuv3p4b)
{
    int32_t temp_picWidth;
    int32_t chromaWidth;

    if ( bitDepth != 0)
        *bitDepth = 0;
    if ( packedFormat != 0)
        *packedFormat = 0;
    if ( yuv3p4b != 0)
        *yuv3p4b = 0;

    switch (format)
    {
    case FORMAT_420:
        if (lumaSize)   *lumaSize = picWidth * picHeight;
        if (chromaSize) *chromaSize = picWidth * picHeight / 2;
        if (frameSize)  *frameSize = picWidth * picHeight * 3 /2;
        break;
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
        if ( packedFormat != 0)
            *packedFormat = 1;
        if (lumaSize)   *lumaSize = picWidth * picHeight;
        if (chromaSize) *chromaSize = picWidth * picHeight;
        if (frameSize)  *frameSize = *lumaSize + *chromaSize;
        break;
    case FORMAT_224:
        if (lumaSize)   *lumaSize = picWidth * picHeight;
        if (chromaSize) *chromaSize = picWidth * picHeight;
        if (frameSize)  *frameSize = picWidth * picHeight * 4 / 2;
        break;
    case FORMAT_422:
        if (lumaSize)   *lumaSize = picWidth * picHeight;
        if (chromaSize) *chromaSize = picWidth * picHeight;
        if (frameSize)  *frameSize = picWidth * picHeight * 4 / 2;
        break;
    case FORMAT_444:
        if (lumaSize)   *lumaSize  = picWidth * picHeight;
        if (chromaSize) *chromaSize = picWidth * picHeight * 2;
        if (frameSize)  *frameSize = picWidth * picHeight * 3;
        break;
    case FORMAT_400:
        if (lumaSize)   *lumaSize  = picWidth * picHeight;
        if (chromaSize) *chromaSize = 0;
        if (frameSize)  *frameSize = picWidth * picHeight;
        break;
    case FORMAT_422_P10_16BIT_MSB:
    case FORMAT_422_P10_16BIT_LSB:
        if ( bitDepth != NULL) {
            *bitDepth = 10;
        }
        if (lumaSize)   *lumaSize = picWidth * picHeight * 2;
        if (chromaSize) *chromaSize = *lumaSize;
        if (frameSize)  *frameSize = *lumaSize + *chromaSize;
        break;
    case FORMAT_420_P10_16BIT_MSB:
    case FORMAT_420_P10_16BIT_LSB:
        if ( bitDepth != 0)
            *bitDepth = 10;
        if (lumaSize)   *lumaSize = picWidth * picHeight * 2;
        if (chromaSize) *chromaSize = picWidth * picHeight;
        if (frameSize)  *frameSize = *lumaSize + *chromaSize;
        break;
    case FORMAT_YUYV_P10_16BIT_MSB:   // 4:2:2 10bit packed
    case FORMAT_YUYV_P10_16BIT_LSB:
    case FORMAT_YVYU_P10_16BIT_MSB:
    case FORMAT_YVYU_P10_16BIT_LSB:
    case FORMAT_UYVY_P10_16BIT_MSB:
    case FORMAT_UYVY_P10_16BIT_LSB:
    case FORMAT_VYUY_P10_16BIT_MSB:
    case FORMAT_VYUY_P10_16BIT_LSB:
        if ( bitDepth != 0)
            *bitDepth = 10;
        if ( packedFormat != 0)
            *packedFormat = 1;
        if (lumaSize)   *lumaSize = picWidth * picHeight * 2;
        if (chromaSize) *chromaSize = picWidth * picHeight * 2;
        if (frameSize)  *frameSize = *lumaSize + *chromaSize;
        break;
    case FORMAT_420_P10_32BIT_MSB:
    case FORMAT_420_P10_32BIT_LSB:
        if ( bitDepth != 0)
            *bitDepth = 10;
        if ( yuv3p4b != 0)
            *yuv3p4b = 1;
        temp_picWidth = VPU_ALIGN32(picWidth);
        chromaWidth = ((VPU_ALIGN16(temp_picWidth/2*(1<<cbcrInterleave))+2)/3*4);
        if ( cbcrInterleave == 1)
        {
            if (lumaSize)   *lumaSize = (temp_picWidth+2)/3*4 * picHeight;
            if (chromaSize) *chromaSize = chromaWidth * picHeight/2;
        } else {
            if (lumaSize)   *lumaSize = (temp_picWidth+2)/3*4 * picHeight;
            if (chromaSize) *chromaSize = chromaWidth * picHeight/2*2;
        }
        if (frameSize) *frameSize = *lumaSize + *chromaSize;
        break;
    case FORMAT_YUYV_P10_32BIT_MSB:
    case FORMAT_YUYV_P10_32BIT_LSB:
    case FORMAT_YVYU_P10_32BIT_MSB:
    case FORMAT_YVYU_P10_32BIT_LSB:
    case FORMAT_UYVY_P10_32BIT_MSB:
    case FORMAT_UYVY_P10_32BIT_LSB:
    case FORMAT_VYUY_P10_32BIT_MSB:
    case FORMAT_VYUY_P10_32BIT_LSB:
        if ( bitDepth != 0)
            *bitDepth = 10;
        if ( packedFormat != 0)
            *packedFormat = 1;
        if ( yuv3p4b != 0)
            *yuv3p4b = 1;
        if (frameSize)  *frameSize = ((picWidth*2)+2)/3*4 * picHeight;
        if (lumaSize)   *lumaSize = *frameSize/2;
        if (chromaSize) *chromaSize = *frameSize/2;
        break;
    default:
        if (frameSize) *frameSize = picWidth * picHeight * 3 / 2;
        VLOG(ERR, "%s:%d Not supported format(%d)\n", __FILE__, __LINE__, format);
        return FALSE;
    }
    return TRUE;
}

int GetPackedFormat (
    int srcBitDepth,
    int packedType,
    int p10bits,
    int msb)
{
    int format = FORMAT_YUYV;

    // default pixel format = P10_16BIT_LSB (p10bits = 16, msb = 0)
    if (srcBitDepth == 8) {

        switch(packedType) {
        case PACKED_YUYV:
            format = FORMAT_YUYV;
            break;
        case PACKED_YVYU:
            format = FORMAT_YVYU;
            break;
        case PACKED_UYVY:
            format = FORMAT_UYVY;
            break;
        case PACKED_VYUY:
            format = FORMAT_VYUY;
            break;
        default:
            format = -1;
        }
    }
    else if (srcBitDepth == 10) {
        switch(packedType) {
        case PACKED_YUYV:
            if (p10bits == 16) {
                format = (msb == 0) ? FORMAT_YUYV_P10_16BIT_LSB : FORMAT_YUYV_P10_16BIT_MSB;
            }
            else if (p10bits == 32) {
                format = (msb == 0) ? FORMAT_YUYV_P10_32BIT_LSB : FORMAT_YUYV_P10_32BIT_MSB;
            }
            else {
                format = -1;
            }
            break;
        case PACKED_YVYU:
            if (p10bits == 16) {
                format = (msb == 0) ? FORMAT_YVYU_P10_16BIT_LSB : FORMAT_YVYU_P10_16BIT_MSB;
            }
            else if (p10bits == 32) {
                format = (msb == 0) ? FORMAT_YVYU_P10_32BIT_LSB : FORMAT_YVYU_P10_32BIT_MSB;
            }
            else {
                format = -1;
            }
            break;
        case PACKED_UYVY:
            if (p10bits == 16) {
                format = (msb == 0) ? FORMAT_UYVY_P10_16BIT_LSB : FORMAT_UYVY_P10_16BIT_MSB;
            }
            else if (p10bits == 32) {
                format = (msb == 0) ? FORMAT_UYVY_P10_32BIT_LSB : FORMAT_UYVY_P10_32BIT_MSB;
            }
            else {
                format = -1;
            }
            break;
        case PACKED_VYUY:
            if (p10bits == 16) {
                format = (msb == 0) ? FORMAT_VYUY_P10_16BIT_LSB : FORMAT_VYUY_P10_16BIT_MSB;
            }
            else if (p10bits == 32) {
                format = (msb == 0) ? FORMAT_VYUY_P10_32BIT_LSB : FORMAT_VYUY_P10_32BIT_MSB;
            }
            else {
                format = -1;
            }
            break;
        default:
            format = -1;
        }
    }
    else {
        format = -1;
    }

    return format;
}



void GenRegionToMap(
    VpuRect *region,        /**< The size of the ROI region for H.265 (start X/Y in CTU, end X/Y int CTU)  */
    int *roiLevel,
    int num,
    Uint32 mapWidth,
    Uint32 mapHeight,
    Uint8 *roiCtuMap)
{
    Int32 roi_id, blk_addr;
    Uint32 roi_map_size      = mapWidth * mapHeight;

    //init roi map
    for (blk_addr=0; blk_addr<(Int32)roi_map_size; blk_addr++)
        roiCtuMap[blk_addr] = 0;

    //set roi map. roi_entry[i] has higher priority than roi_entry[i+1]
    for (roi_id=(Int32)num-1; roi_id>=0; roi_id--)
    {
        Uint32 x, y;
        VpuRect *roi = region + roi_id;

        for (y=roi->top; y<=roi->bottom; y++)
        {
            for (x=roi->left; x<=roi->right; x++)
            {
                roiCtuMap[y*mapWidth + x] = *(roiLevel + roi_id);
            }
        }
    }
}

int64_t GetNowUs()
{
    struct timespec ts_cur;
    clock_gettime(CLOCK_MONOTONIC, &ts_cur);

    int64_t curr = ts_cur.tv_sec * 1000000LL + ts_cur.tv_nsec / 1000;

    return curr;
}
