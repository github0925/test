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
#include "../misc/skip.h"
#if defined(PLATFORM_LINUX) || defined(PLATFORM_QNX) || defined(PLATFORM_ANDROID)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#ifdef PLATFORM_NON_OS
#ifdef LIB_C_STUB
#include <sys/types.h>
#include <sys/stat.h>
#endif
#endif

typedef struct {
    osal_file_t fp;
    uint32_t    width;
    uint32_t    height;
    uint32_t    frameSize;
    BOOL        cbcrInterleave;
    FrameBufferFormat format;
    char        *path;
} Context;


static uint32_t Calculate(
    Context*    ctx
    )
{
    uint32_t    lumaSize   = 0;
    uint32_t    chromaSize = 0;
    uint32_t    frameSize  = 0;
    uint32_t    frames = 0;
    uint32_t    width  = ctx->width;
    uint32_t    height = ctx->height;
    uint64_t    endPos = 0;
    BOOL        cbcrInterleave = ctx->cbcrInterleave;
    FrameBufferFormat format = ctx->format;
#if defined(PLATFORM_LINUX) || defined(PLATFORM_QNX) || defined(PLATFORM_ANDROID)
    struct   stat  file_info;
#endif
#ifdef PLATFORM_NON_OS
#ifdef LIB_C_STUB
    struct   stat  file_info;
#endif
#endif

    lumaSize = width * height;
    switch (format) {
    case FORMAT_400:
        chromaSize = 0;
        break;
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
        chromaSize = lumaSize;
        break;
    case FORMAT_420:
        chromaSize = lumaSize / 2;
        break;
    case FORMAT_422:
    case FORMAT_224:
        chromaSize = lumaSize;
        break;
    case FORMAT_444:
        chromaSize = lumaSize * 2;
        break;
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
        lumaSize  *= 2;
        chromaSize = lumaSize/2;
        break;
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
        if (cbcrInterleave) {
            lumaSize   = ((width+2)/3*4) * height;
            chromaSize = ((width+2)/3*4) * height / 2;
        }
        else {
            lumaSize   = (width+2)/3*4 * height;
            chromaSize = (width/2+2)/3*4 * height / 2 *2;
        }
        break;
    case FORMAT_422_P10_16BIT_MSB:
    case FORMAT_422_P10_16BIT_LSB:
        lumaSize  *= 2;
        chromaSize = lumaSize;
        break;
    default:
        VLOG(ERR, "%s:%d Invalid format: %d\n", __FILE__, __LINE__, format);
    }
    frameSize = lumaSize + chromaSize;

#ifdef PLATFORM_WIN32
#if (_MSC_VER == 1200)
    osal_fseek(ctx->fp, 0, SEEK_END);
    endPos = ftell(ctx->fp);
    fseek(ctx->fp, 0, SEEK_SET);
#else
    _fseeki64((FILE*)ctx->fp, 0LL, SEEK_END);
    endPos = _ftelli64((FILE*)ctx->fp);
    _fseeki64((FILE*)ctx->fp, 0LL, SEEK_SET);
#endif
#else
    stat( ctx->path, &file_info);
    endPos = file_info.st_size;
#endif

    frames = (uint32_t)(endPos / frameSize);

    if (endPos % frameSize) {
        VLOG(ERR, "%s:%d Mismatch - file_size: %llu frameSize: %d\n",
            __FUNCTION__, __LINE__, endPos, frameSize);
    }
    ctx->frameSize  = frameSize;

    return frames;
}

BOOL YUVComparator_Create(
    ComparatorImpl* impl,
    char*           path
    )
{
    Context*        ctx;
    osal_file_t*    fp;

    if ((fp=osal_fopen(path, "rb")) == NULL) {
        VLOG(ERR, "%s:%d failed to open yuv file: %s\n", __FUNCTION__, __LINE__, path);
        return FALSE;
    }

    if ((ctx=(Context*)osal_malloc(sizeof(Context))) == NULL) {
        osal_fclose(fp);
        return FALSE;
    }

    ctx->fp        = fp;
    ctx->path      = path;
    impl->context  = ctx;

    return TRUE;
}

BOOL YUVComparator_Destroy(
    ComparatorImpl*  impl
    )
{
    Context*    ctx = (Context*)impl->context;

    osal_fclose(ctx->fp);
    osal_free(ctx);

    return TRUE;
}

BOOL YUVComparator_Compare(
    ComparatorImpl* impl,
    void*           data,
    uint32_t        size
    )
{
    uint8_t*    pYuv = NULL;
    Context*    ctx = (Context*)impl->context;
    BOOL        match = FALSE;

    if ( data == (void *)COMPARATOR_SKIP ) {
        int fpos;
        fpos = osal_ftell(ctx->fp);
        osal_fseek(ctx->fp, fpos+size, SEEK_SET);
        return TRUE;
    }

    pYuv = (uint8_t*)osal_malloc(size);
    osal_fread(pYuv, 1, size, ctx->fp);

    match = (osal_memcmp(data, (void*)pYuv, size) == 0 ? TRUE : FALSE);
    if (match == FALSE) {
        osal_file_t* fpGolden;
        osal_file_t* fpOutput;
        char tmp[200];

        VLOG(ERR, "MISMATCH WITH GOLDEN YUV at %d frame\n", impl->curIndex);
        sprintf(tmp, "./golden.yuv");
        if ((fpGolden=osal_fopen(tmp, "wb")) == NULL) {
            VLOG(ERR, "Faild to create golden.yuv\n");
            osal_free(pYuv);
            return FALSE;
        }
        VLOG(ERR, "Saving... Golden YUV at %s\n", tmp);

        sprintf(tmp, "./decoded.yuv");
        osal_fwrite(pYuv, 1, size, fpGolden);
        osal_fclose(fpGolden);
        if ((fpOutput=osal_fopen(tmp, "wb")) == NULL) {
            VLOG(ERR, "Faild to create golden.yuv\n");
            osal_free(pYuv);
            return FALSE;
        }
        VLOG(ERR, "Saving... decoded YUV at %s\n", tmp);
        osal_fwrite(data, 1, size, fpOutput);
        osal_fclose(fpOutput);
    }
    osal_free(pYuv);

    return match;
}


BOOL YUVComparator_Configure(
    ComparatorImpl*     impl,
    ComparatorConfType  type,
    void*               val
    )
{
    PictureInfo*        yuv = NULL;
    Context*            ctx = (Context*)impl->context;
    BOOL                ret = TRUE;

    switch (type) {
    case COMPARATOR_CONF_SET_PICINFO:
        yuv = (PictureInfo*)val;
        ctx->width  = yuv->width;
        ctx->height = yuv->height;
        ctx->format = yuv->format;
        ctx->cbcrInterleave = yuv->cbcrInterleave;
        //can not calculate a sequence changed YUV
        impl->numOfFrames   = Calculate(ctx);
        break;
    default:
        ret = FALSE;
        break;
    }
    return ret;
}

BOOL YUVComparator_Rewind(
    ComparatorImpl*     impl
    )
{
    Context*    ctx = (Context*)impl->context;
    Int32       ret;

    if ((ret=osal_fseek(ctx->fp, 0, SEEK_SET)) != 0) {
        VLOG(ERR, "%s:%d failed to osal_fseek(ret:%d)\n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }

    return TRUE;
}

ComparatorImpl yuvComparatorImpl = {
    NULL,
    NULL,
    0,
    0,
    YUVComparator_Create,
    YUVComparator_Destroy,
    YUVComparator_Compare,
    YUVComparator_Configure,
    FALSE,
    FALSE,
    FALSE,
    FALSE
};
