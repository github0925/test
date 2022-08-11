//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include "main_helper.h"

typedef struct {
    FILE*       fp;
    uint32_t    width;
    uint32_t    height;
    uint32_t    frameSize;
    BOOL        cbcrInterleave;
    FrameBufferFormat format;
} Context;

BOOL BinComparator_Create(
    ComparatorImpl* impl,
    char*           path
    )
{
    Context*    ctx;
    FILE*       fp;

    if ((fp=osal_fopen(path, "rb")) == NULL) {
        VLOG(ERR, "%s:%d failed to open bin file: %s\n", __FUNCTION__, __LINE__, path);
        return FALSE;
    }

    if ((ctx=(Context*)osal_malloc(sizeof(Context))) == NULL) {
        fclose(fp);
        return FALSE;
    }

    ctx->fp        = fp;
    impl->context  = ctx;

    return TRUE;
}

BOOL BinComparator_Destroy(
    ComparatorImpl*  impl
    )
{
    Context*    ctx = (Context*)impl->context;

    fclose(ctx->fp);
    osal_free(ctx);

    return TRUE;
}

BOOL BinComparator_Compare(
    ComparatorImpl* impl,
    void*           data,
    uint32_t        size
    )
{
    uint8_t*    pBin = NULL;
    Context*    ctx = (Context*)impl->context;
    BOOL        match = FALSE;

    pBin = (uint8_t*)osal_malloc(size);

    fread(pBin, size, 1, ctx->fp);

    if(!osal_feof(ctx->fp)) {
        impl->numOfFrames++;
    }
    match = (osal_memcmp(data, (void*)pBin, size) == 0 ? TRUE : FALSE);
    if (match == FALSE) {
        FILE* fpGolden;
        FILE* fpOutput;
        char tmp[200];

        if ( impl->curIndex == 1 )//because of header
            VLOG(ERR, "MISMATCH WITH GOLDEN bin at header\n");
        else
            VLOG(ERR, "MISMATCH WITH GOLDEN bin at %d frame\n", impl->curIndex - 1);

        sprintf(tmp, "./golden_%s_%05d.bin", GetBasename(impl->filename), impl->curIndex-1);
        if ((fpGolden=osal_fopen(tmp, "wb")) == NULL) {
            VLOG(ERR, "Faild to create %s\n", tmp);
            osal_free(pBin);
            return FALSE;
        }
        VLOG(ERR, "Saving... Golden Bin at %s\n", tmp);
        fwrite(pBin, size, 1, fpGolden);
        fclose(fpGolden);

        sprintf(tmp, "./encoded_%s_%05d.bin", GetBasename(impl->filename), impl->curIndex-1);
        if ((fpOutput=osal_fopen(tmp, "wb")) == NULL) {
            VLOG(ERR, "Faild to create %s\n", tmp);
            osal_free(pBin);
            return FALSE;
        }
        VLOG(ERR, "Saving... encoded Bin at %s\n", tmp);
        fwrite(data, size, 1, fpOutput);
        fclose(fpOutput);
    }

    osal_free(pBin);

    return match;
}

BOOL BinComparator_Configure(
    ComparatorImpl*     impl,
    ComparatorConfType  type,
    void*               val
    )
{
    UNREFERENCED_PARAMETER(impl);
    UNREFERENCED_PARAMETER(type);
    UNREFERENCED_PARAMETER(val);
    return FALSE;
}

ComparatorImpl binComparatorImpl = {
    NULL,
    NULL,
    0,
    0,
    BinComparator_Create,
    BinComparator_Destroy,
    BinComparator_Compare,
    BinComparator_Configure,
    FALSE,
    FALSE,
    FALSE,
    FALSE
};
