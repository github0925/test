//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2013  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#include <string.h>
#include <stdarg.h>
#include "main_helper.h"

extern YuvFeederImpl loaderYuvFeederImpl;

BOOL loaderYuvFeeder_Create(
    YuvFeederImpl *impl,
    const char* path,
    uint32_t packed,
    uint32_t fbStride,
    uint32_t fbHeight
);

BOOL loaderYuvFeeder_Destory(
    YuvFeederImpl *impl
);

BOOL loaderYuvFeeder_Feed(
    YuvFeederImpl*  impl,
    int32_t         coreIdx,
    FrameBuffer*    fb,
    size_t          picWidth,
    size_t          picHeight,
    void*           arg
);

BOOL loaderYuvFeeder_Configure(
    YuvFeederImpl *impl,
    uint32_t      cmd,
    void          *val
);

static BOOL yuvYuvFeeder_Create(
    YuvFeederImpl *impl,
    const char* path,
    uint32_t packed,
    uint32_t fbStride,
    uint32_t fbHeight)
{
    yuvContext*        ctx;
    osal_file_t*    fp;
    uint8_t*        pYuv;

    if ((fp=osal_fopen(path, "rb")) == NULL) {
        VLOG(ERR, "%s:%d failed to open yuv file: %s\n", __FUNCTION__, __LINE__, path);
        return FALSE;
    }

    if ( packed == 1 )
        pYuv = osal_malloc(fbStride*fbHeight*3*2*2);//packed, unsigned short
    else
        pYuv = osal_malloc(fbStride*fbHeight*3*2);//unsigned short

    if ((ctx=(yuvContext*)osal_malloc(sizeof(yuvContext))) == NULL) {
        osal_free(pYuv);
        osal_fclose(fp);
        return FALSE;
    }

    memset(ctx, 0, sizeof(yuvContext));

    ctx->fp        = fp;
    ctx->pYuv      = pYuv;
    ctx->fbStride  = fbStride;
    impl->context  = ctx;
    return TRUE;
}

static BOOL yuvYuvFeeder_Destory(
    YuvFeederImpl *impl
    )
{
    yuvContext*    ctx = (yuvContext*)impl->context;

    osal_fclose(ctx->fp);
    osal_free(ctx->pYuv);
    osal_free(ctx);
    return TRUE;
}


static BOOL yuvYuvFeeder_Feed(
    YuvFeederImpl*  impl,
    int32_t         coreIdx,
    FrameBuffer*    fb,
    size_t          picWidth,
    size_t          picHeight,
    void*           arg
    )
{
    yuvContext* ctx = (yuvContext*)impl->context;
    uint8_t*    pYuv = ctx->pYuv;
    size_t      frameSize;
    size_t      frameSizeY;
    size_t      frameSizeC;
    int32_t     bitdepth=0;
    int32_t     yuv3p4b=0;
    int32_t     packedFormat=0;
    uint32_t    outWidth=0;
    uint32_t    outHeight=0;

    CalcYuvSize(fb->format, picWidth, picHeight, fb->cbcrInterleave, &frameSizeY, &frameSizeC, &frameSize, &bitdepth, &packedFormat, &yuv3p4b);


    // Load source one picture image to encode to SDRAM frame buffer.
    if (!osal_fread(pYuv, 1, frameSize, ctx->fp)) {
        if (osal_feof(ctx->fp) == 0) 
            VLOG(ERR, "Yuv Data osal_fread failed file handle is 0x%x\n", ctx->fp);
        return FALSE;
    }

    if (fb->mapType == LINEAR_FRAME_MAP ) {
        outWidth  = (yuv3p4b&&packedFormat==0) ? ((picWidth+31)/32)*32  : picWidth;
        outHeight = (yuv3p4b) ? ((picHeight+7)/8)*8 : picHeight;

        if ( yuv3p4b  && packedFormat) {
            outWidth = ((picWidth*2)+2)/3*4;
        }
        else if(packedFormat) {
            outWidth *= 2;           // 8bit packed mode(YUYV) only. (need to add 10bit cases later)
            if (bitdepth != 0)      // 10bit packed
                outWidth *= 2;
        }
        LoadYuvImageBurstFormat(coreIdx, pYuv, outWidth, outHeight, fb, ctx->srcPlanar);
    }
    else {
        TiledMapConfig  mapConfig;

        osal_memset((void*)&mapConfig, 0x00, sizeof(TiledMapConfig));
        if (arg != NULL) {
            osal_memcpy((void*)&mapConfig, arg, sizeof(TiledMapConfig));
        }

        LoadTiledImageYuvBurst(coreIdx, pYuv, picWidth, picHeight, fb, mapConfig);
    }

    return TRUE;
}

static BOOL yuvYuvFeeder_Configure(
    YuvFeederImpl*  impl,
    uint32_t        cmd,
    void*           val
    )
{
    yuvInfo    *yuv;
    yuvContext* ctx = (yuvContext*)impl->context;
    UNREFERENCED_PARAMETER(cmd);

    yuv = (yuvInfo*)val;

    ctx->cbcrInterleave = yuv->cbcrInterleave;
    ctx->srcPlanar      = yuv->srcPlanar;

    return TRUE;
}

YuvFeederImpl yuvYuvFeederImpl = {
    NULL,
    yuvYuvFeeder_Create,
    yuvYuvFeeder_Feed,
    yuvYuvFeeder_Destory,
    yuvYuvFeeder_Configure
};

/*lint -esym(438, ap) */
YuvFeeder YuvFeeder_Create(
    uint32_t        type,
    const char*     srcFilePath,
    uint32_t        packed, 
    uint32_t        fbStride, 
    uint32_t        fbHeight,
    ...
    )
{
    AbstractYuvFeeder *feeder;
    YuvFeederImpl     *impl;
    va_list           ap;
    BOOL              success = FALSE;
    
    if (srcFilePath == NULL) {
        VLOG(ERR, "%s:%d src path is NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    switch (type) {
    case SOURCE_YUV_WITH_LOADER:
        impl = osal_malloc(sizeof(YuvFeederImpl));
        impl->Create = &loaderYuvFeeder_Create;
        impl->Feed = &loaderYuvFeeder_Feed;
        impl->Destroy = &loaderYuvFeeder_Destory;
        impl->Configure = &loaderYuvFeeder_Configure;
        if ((success = impl->Create(impl, srcFilePath, packed, fbStride, fbHeight)) == TRUE) {
            yuvInfo yuv;
            va_start(ap, fbHeight);
            yuv.packedFormat    = packed;
            yuv.cbcrInterleave  = va_arg(ap, uint32_t);
            yuv.nv21            = va_arg(ap, uint32_t);
            yuv.srcFormat       = va_arg(ap, uint32_t);
            va_end(ap);
            impl->Configure(impl, 0, (void*)&yuv);
        }
        break;
    default:
        impl = osal_malloc(sizeof(YuvFeederImpl));
        impl->Create = &yuvYuvFeeder_Create;
        impl->Feed = &yuvYuvFeeder_Feed;
        impl->Destroy = &yuvYuvFeeder_Destory;
        impl->Configure = &yuvYuvFeeder_Configure;
        if ((success=impl->Create(impl, srcFilePath, packed, fbStride, fbHeight)) == TRUE) {
            yuvInfo yuv;
            va_start(ap, fbHeight);
            yuv.packedFormat    = packed;
            yuv.cbcrInterleave  = va_arg(ap, uint32_t);
            yuv.srcPlanar       = (BOOL)va_arg(ap, uint32_t);
            va_end(ap);
            impl->Configure(impl, 0, (void*)&yuv);
        }
        break;
    }

    if (success == FALSE)
        return NULL;

    feeder = (AbstractYuvFeeder*)osal_malloc(sizeof(AbstractYuvFeeder));
    feeder->impl = impl;

    return feeder;
}
/*lint +esym(438, ap) */

BOOL YuvFeeder_Destroy(
    YuvFeeder feeder
    )
{
    YuvFeederImpl*     impl = NULL;
    AbstractYuvFeeder* yuvfeeder =  (AbstractYuvFeeder*)feeder;

    if (yuvfeeder == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    impl = yuvfeeder->impl;

    impl->Destroy(impl);
    osal_free(impl);
    return TRUE;
}

BOOL YuvFeeder_Feed(
    YuvFeeder       feeder,
    Uint32          coreIdx,
    FrameBuffer*    fb,
    size_t          picWidth,
    size_t          picHeight,
    void*           arg
    )
{
    YuvFeederImpl*      impl = NULL;
    AbstractYuvFeeder*  absFeeder = (AbstractYuvFeeder*)feeder;
#if defined(WAVE420SN) || defined(WAVE420DU)
    static int          allocNum=0;
#endif
    int32_t             ret;

    if (absFeeder == NULL) {
        VLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    impl = absFeeder->impl;
#if defined(WAVE420SN) || defined(WAVE420DU)
    fb->allocNum = allocNum++;
    if(fb->srcBufState != SRC_BUFFER_ALLOCATED)
        return -2;
#endif

    ret = impl->Feed(impl, coreIdx, fb, picWidth, picHeight, arg);
    if ( ret == TRUE)
        fb->srcBufState = SRC_BUFFER_SRC_LOADED;
    return ret;
}


