//--=========================================================================--
//  This implements some useful common functionalities
//  for handling the register files used in Bellagio
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2015  CHIPS&MEDIA INC.
//            (C) CPPYRIGHT 2020 Semidrive Technology Ltd.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--


#include <omxcore.h>
#include "android_support.h"

#include <ui/GraphicBuffer.h>
#include <HardwareAPI.h>
#include <hardware/hardware.h>
#include <MetadataBufferType.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <hardware/gralloc.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>
#include <libyuv.h>

#define UNUSED(x) (void)(x)
#define GRALLOC_FB_COMPRESSED_BIT (12)

using namespace android;

typedef struct {
    OMX_COLOR_FORMATTYPE colorformat;
    OMX_U32 pixelformat;
} OMX_PIXEL_MAP_TABLE;

static OMX_PIXEL_MAP_TABLE s_format_tb[] =
{
    {OMX_COLOR_Format16bitRGB565, HAL_PIXEL_FORMAT_RGB_565},
    {OMX_COLOR_FormatYUV420Planar, HAL_PIXEL_FORMAT_YV12},
    {OMX_COLOR_FormatYUV420SemiPlanar, HAL_PIXEL_FORMAT_NV12},
};


OMX_BOOL checkTileIFBCDisabled(OMX_STRING prop, OMX_BOOL defvalue)
{
    char value[PROPERTY_VALUE_MAX];
    if (property_get(prop, value, defvalue ? "true":"false"))
    {
        DEBUG(DEB_LEV_SIMPLE_SEQ, "%s %s\n", prop, value);
        if (!strcmp(value, "true"))
        {
            return OMX_TRUE;
        }
        return OMX_FALSE;
    }
    return defvalue;
}


OMX_ERRORTYPE checkEnableAndroidBuffersHeader(OMX_PTR ComponentParameterStructure)
{
    EnableAndroidNativeBuffersParams * pEnableAndroidNativeBuffersParams;

    pEnableAndroidNativeBuffersParams = (EnableAndroidNativeBuffersParams *)ComponentParameterStructure;

    return checkAndroidParamHeader(pEnableAndroidNativeBuffersParams, sizeof (EnableAndroidNativeBuffersParams));
}

OMX_ERRORTYPE checkEnableAndroidBuffersPort(OMX_PTR ComponentParameterStructure, OMX_U32 *portIndex)
{
    EnableAndroidNativeBuffersParams * pEnableAndroidNativeBuffersParams;

    pEnableAndroidNativeBuffersParams = (EnableAndroidNativeBuffersParams *)ComponentParameterStructure;

    *portIndex = pEnableAndroidNativeBuffersParams->nPortIndex;
    if (pEnableAndroidNativeBuffersParams->nPortIndex != kOutputPortIndex) // output port
    {
        return OMX_ErrorBadPortIndex;
    }

    return OMX_ErrorNone;
}

OMX_BOOL enableAndroidBuffer(OMX_PTR ComponentParameterStructure)
{
    EnableAndroidNativeBuffersParams * pEnableAndroidNativeBuffersParams;

    pEnableAndroidNativeBuffersParams = (EnableAndroidNativeBuffersParams *)ComponentParameterStructure;
    DEBUG(DEB_LEV_SIMPLE_SEQ, "%s enable %d\n", __func__, pEnableAndroidNativeBuffersParams->enable);
    return pEnableAndroidNativeBuffersParams->enable;
}

OMX_ERRORTYPE checkUseAndroidNativeBufferHeader(OMX_PTR ComponentParameterStructure)
{
    UseAndroidNativeBufferParams * pUseAndroidNativeBufferParams;

    pUseAndroidNativeBufferParams = (UseAndroidNativeBufferParams *)ComponentParameterStructure;

    return checkAndroidParamHeader(pUseAndroidNativeBufferParams, sizeof (UseAndroidNativeBufferParams));

}

OMX_ERRORTYPE checkUseAndroidNativeBufferPort(OMX_PTR ComponentParameterStructure, OMX_U32 *portIndex)
{
    UseAndroidNativeBufferParams * pUseAndroidNativeBufferParams;

    pUseAndroidNativeBufferParams = (UseAndroidNativeBufferParams *)ComponentParameterStructure;

    *portIndex = pUseAndroidNativeBufferParams->nPortIndex;
    if (pUseAndroidNativeBufferParams->nPortIndex != kOutputPortIndex) // output port
    {
        return OMX_ErrorBadPortIndex;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE useAndroidNativeBuffer(OMX_PTR ComponentParameterStructure, OMX_BUFFERHEADERTYPE **pNativeBufHeaderType, OMX_COLOR_FORMATTYPE* eColorFormat, OMX_U32 size)
{
    UseAndroidNativeBufferParams *pUseAndroidNativeBufferParams = (UseAndroidNativeBufferParams *)ComponentParameterStructure;
    sp<ANativeWindowBuffer> nBuf = pUseAndroidNativeBufferParams->nativeBuffer;
    IMG_native_handle_t *hnd = (IMG_native_handle_t *)nBuf->handle;
    native_handle_t *handle = (native_handle_t *)&hnd->base;

    int fd = handle->data[0];
    DEBUG(DEB_LEV_SIMPLE_SEQ, "In-%s fd %d, num %d\n", __func__, fd, handle->numFds);

    // uint32_t size = (getNativeBufferSize(eColorFormat, 0, nBuf->stride, nBuf->height)+4095)&(~4095);
    DEBUG(DEB_LEV_SIMPLE_SEQ, "In-%s w-h-s-f-c %d-%d-%d-%d-%d, %d, %p-%p\n", __func__, nBuf->width, nBuf->height,
            nBuf->stride, nBuf->format, *eColorFormat, size, *pNativeBufHeaderType, handle);

    DEBUG(DEB_LEV_SIMPLE_SEQ, "In-%s w-h-u-f %d-%d-0x%x-0x%x\n", __func__, hnd->iWidth, hnd->iHeight, hnd->usage, hnd->iFormat);

    /* TODO: map gralloc buffer format and OMX vendor format */
    if ((hnd->iFormat>>GRALLOC_FB_COMPRESSED_BIT)&0x01) {
        *eColorFormat = OMX_SEMI_COLOR_FormatIFBC32x8Tiled;
    }

    OMX_BUFFERHEADERTYPE *temp_bufferHeader = NULL;
    temp_bufferHeader = *pNativeBufHeaderType;
    temp_bufferHeader->nSize  = sizeof(OMX_BUFFERHEADERTYPE);
    temp_bufferHeader->nVersion.s.nVersionMajor = pUseAndroidNativeBufferParams->nVersion.s.nVersionMajor;
    temp_bufferHeader->nVersion.s.nVersionMinor = pUseAndroidNativeBufferParams->nVersion.s.nVersionMinor;
    temp_bufferHeader->nVersion.s.nRevision = pUseAndroidNativeBufferParams->nVersion.s.nRevision;
    temp_bufferHeader->nVersion.s.nStep = pUseAndroidNativeBufferParams->nVersion.s.nStep;
    temp_bufferHeader->pBuffer        = const_cast<OMX_U8*>(reinterpret_cast<const OMX_U8*>(handle));
    temp_bufferHeader->nAllocLen      = size;
    temp_bufferHeader->pAppPrivate    = pUseAndroidNativeBufferParams->pAppPrivate;
    temp_bufferHeader->nOutputPortIndex = pUseAndroidNativeBufferParams->nPortIndex;
    *(pUseAndroidNativeBufferParams->bufferHeader) = temp_bufferHeader;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE checkGetAndroidNativeBufferHeader(OMX_PTR ComponentParameterStructure)
{
    GetAndroidNativeBufferUsageParams * pGetAndroidNativeBufferUsageParams;

    pGetAndroidNativeBufferUsageParams = (GetAndroidNativeBufferUsageParams *)ComponentParameterStructure;

    return checkAndroidParamHeader(pGetAndroidNativeBufferUsageParams, sizeof (GetAndroidNativeBufferUsageParams));
}

OMX_ERRORTYPE checkGetAndroidNativeBufferPort(OMX_PTR ComponentParameterStructure, OMX_U32 *portIndex)
{
    GetAndroidNativeBufferUsageParams * pGetAndroidNativeBufferUsageParams;

    pGetAndroidNativeBufferUsageParams = (GetAndroidNativeBufferUsageParams *)ComponentParameterStructure;
    *portIndex = pGetAndroidNativeBufferUsageParams->nPortIndex;
    if (pGetAndroidNativeBufferUsageParams->nPortIndex != kOutputPortIndex)    // output port
    {
        return OMX_ErrorBadPortIndex;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE getAndroidNativeBufferUsage(OMX_PTR ComponentParameterStructure, OMX_U32 nUsage)
{
    GetAndroidNativeBufferUsageParams * pGetAndroidNativeBufferUsageParams;
    pGetAndroidNativeBufferUsageParams = (GetAndroidNativeBufferUsageParams *)ComponentParameterStructure;
    pGetAndroidNativeBufferUsageParams->nUsage = nUsage;

    DEBUG(DEB_LEV_FULL_SEQ, "%s usage 0x%x\n", __func__, pGetAndroidNativeBufferUsageParams->nUsage);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE setAndroidNativeBufferUsage(OMX_PTR ComponentParameterStructure, OMX_U32* nUsage)
{
    GetAndroidNativeBufferUsageParams * pGetAndroidNativeBufferUsageParams;
    pGetAndroidNativeBufferUsageParams = (GetAndroidNativeBufferUsageParams *)ComponentParameterStructure;
    DEBUG(DEB_LEV_FULL_SEQ, "%s usage 0x%x\n", __func__, pGetAndroidNativeBufferUsageParams->nUsage);
    *nUsage |= pGetAndroidNativeBufferUsageParams->nUsage;
    return OMX_ErrorNone;
}

OMX_U32 getAndroidNativeHandle(OMX_BUFFERHEADERTYPE *pNativeBufHeaderType, OMX_U32* fd)
{
    native_handle_t *handle;
    handle = (native_handle_t *)pNativeBufHeaderType->pBuffer;
    if (handle) {
        DEBUG(DEB_LEV_FULL_SEQ, "%s handle:0x%p, fd:%d", __func__, handle, handle->data[0]);
        *fd = handle->data[0];
        return OMX_ErrorNone;
    }

    DEBUG(DEB_LEV_FULL_SEQ, "%s failed", __func__);
    return OMX_ErrorNoMore;
}

#ifdef SUPPORT_ADAPTIVE_PLAY
OMX_ERRORTYPE checkUseAdaptivePlaybackHeader(OMX_PTR ComponentParameterStructure)
{
    PrepareForAdaptivePlaybackParams * pAdaptivePlaybackParams;

    pAdaptivePlaybackParams  = (PrepareForAdaptivePlaybackParams *)ComponentParameterStructure;

    return checkAndroidParamHeader(pAdaptivePlaybackParams, sizeof (PrepareForAdaptivePlaybackParams));
}

OMX_ERRORTYPE checkUseAdaptivePlaybackPort(OMX_PTR ComponentParameterStructure, OMX_U32 *portIndex)
{
    PrepareForAdaptivePlaybackParams * pAdaptivePlaybackParams;

    pAdaptivePlaybackParams  = (PrepareForAdaptivePlaybackParams *)ComponentParameterStructure;

    *portIndex = pAdaptivePlaybackParams->nPortIndex;
    if (pAdaptivePlaybackParams->nPortIndex != kOutputPortIndex)   // output port
    {
        return OMX_ErrorBadPortIndex;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE getAdaptivePlayParams(OMX_PTR ComponentParameterStructure, OMX_BOOL *enable, OMX_U32 *maxWidth, OMX_U32 *maxHeight)
{
    PrepareForAdaptivePlaybackParams * pAdaptivePlaybackParams;

    pAdaptivePlaybackParams  = (PrepareForAdaptivePlaybackParams *)ComponentParameterStructure;

    *enable = pAdaptivePlaybackParams->bEnable;

    if (*enable) {
        *maxWidth   = pAdaptivePlaybackParams->nMaxFrameWidth;
        *maxHeight  = pAdaptivePlaybackParams->nMaxFrameHeight;
    }
    else {
        *maxWidth   = 0;
        *maxHeight  = 0;
    }

    return OMX_ErrorNone;
}
#endif  // SUPPORT_ADAPTIVE_PLAY

int getNativeBufferSize(OMX_COLOR_FORMATTYPE colorFormat, int native_buffer_format, int stride, int height)
{
    int ret;

    if (native_buffer_format == 0)
    {
        switch (colorFormat)
        {
        case OMX_COLOR_FormatYUV420Planar:
        case OMX_COLOR_FormatYUV420PackedPlanar:
        case OMX_COLOR_FormatYUV420SemiPlanar:
        case OMX_COLOR_FormatYUV420PackedSemiPlanar:
            ret = stride * height * 3 / 2;
            break;
        case OMX_COLOR_FormatYUV422Planar:
        case OMX_COLOR_FormatYUV422PackedPlanar:
        case OMX_COLOR_FormatYUV422SemiPlanar:
        case OMX_COLOR_FormatYUV422PackedSemiPlanar:
        case OMX_COLOR_FormatYCbYCr:
        case OMX_COLOR_FormatCbYCrY:
            ret = stride * height * 2;
            break;
        default:
#define MAKE_FOURCC(a,b,c,d) ( ((unsigned char)a) | ((unsigned char)b << 8) | ((unsigned char)c << 16) | ((unsigned char)d << 24) )
            if (colorFormat == (int)MAKE_FOURCC('N', 'V', '1', '2') || colorFormat == (int)MAKE_FOURCC('Y', 'V', '1', '2'))
            {
                ret = stride * height * 3 / 2;
            }
            else if (colorFormat == (int)MAKE_FOURCC('I', '4', '2', '2')
                || colorFormat == (int)MAKE_FOURCC('N', 'V', '1', '6')
                || colorFormat == (int)MAKE_FOURCC('Y', 'U', 'Y', 'V')
                || colorFormat == (int)MAKE_FOURCC('U', 'Y', 'V', 'Y'))
            {
                ret = stride * height * 2;
            }
            else
            {
                ret = stride * height * 3;
            }
            break;
        }
    }
    else if (native_buffer_format == HAL_PIXEL_FORMAT_RGB_888)
    {
        ret = stride * height * 3;
    }
    else if (native_buffer_format == HAL_PIXEL_FORMAT_RGB_565)
    {
        ret = stride * height * 2;
    }
    else
    {
        ret = stride * height * 4;
    }
    return ret;
}

OMX_BOOL getAndroidNativeBufferHandleInfo(buffer_handle_t handle, int *pFormat, int *pWidth, int *pHeight, int *pStride, int *pSize)
{
    UNUSED(pSize);

    IMG_native_handle_t *hnd = (IMG_native_handle_t *)handle;
    native_handle_t *native_handle = (native_handle_t *)&hnd->base;
    int fd = native_handle->data[0];

    if (hnd == NULL)
        return OMX_FALSE;

    if (pFormat)
        *pFormat = hnd->iFormat;

    if (pWidth)
        *pWidth = hnd->iWidth;

    if (pHeight)
        *pHeight = hnd->iHeight;

    if (pStride)
       *pStride = hnd->aiStride[0];

    DEBUG(DEB_LEV_FULL_SEQ, "getAndroidNativeBufferHandleInfo : fd=0x%x, format=0x%x, width=%d, height=%d, stride:%d\n",
            fd, (int)hnd->iFormat, (int)hnd->iWidth, (int)hnd->iHeight, (int)hnd->aiStride[0]);

    return OMX_TRUE;
}

OMX_ERRORTYPE checkAndroidParamHeader(OMX_PTR header, OMX_U32 size)
{
    OMX_VERSIONTYPE* ver;
    if (header == NULL) {
        DEBUG(DEB_LEV_ERR, "In %s the header is null\n",__func__);
        return OMX_ErrorBadParameter;
    }
    ver = (OMX_VERSIONTYPE*)((char*)header + sizeof(OMX_U32));
    if(*((OMX_U32*)header) != size) {
        DEBUG(DEB_LEV_ERR, "In %s the header has a wrong size %i should be %i\n",__func__,(int)*((OMX_U32*)header),(int)size);
        return OMX_ErrorBadParameter;
    }
    if(ver->s.nVersionMajor != SPECVERSIONMAJOR ||
        ver->s.nVersionMinor != SPECVERSIONMINOR) {
            DEBUG(DEB_LEV_ERR, "The version does not match\n");
            return OMX_ErrorVersionMismatch;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE checkStoreMetaDataBufferHeader(OMX_PTR ComponentParameterStructure)
{
    StoreMetaDataInBuffersParams *pStoreMetaDataInBuffers = (StoreMetaDataInBuffersParams *) ComponentParameterStructure;

    return checkAndroidParamHeader(pStoreMetaDataInBuffers, sizeof (StoreMetaDataInBuffersParams));
}

OMX_ERRORTYPE checkStoreMetaDataBufferPort(OMX_PTR ComponentParameterStructure, OMX_U32 *portIndex)
{
    StoreMetaDataInBuffersParams *pStoreMetaDataInBuffers = (StoreMetaDataInBuffersParams *) ComponentParameterStructure;

    if (pStoreMetaDataInBuffers->nPortIndex > kMaxPortIndex)
    {
        return OMX_ErrorBadPortIndex;
    }

    if (portIndex)
        *portIndex = pStoreMetaDataInBuffers->nPortIndex;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE storeMetaDataBuffer(OMX_PTR ComponentParameterStructure, OMX_BOOL *pbEnable)
{
    StoreMetaDataInBuffersParams *pStoreMetaDataInBuffers = (StoreMetaDataInBuffersParams *) ComponentParameterStructure;
    if (pbEnable)
        *pbEnable = pStoreMetaDataInBuffers->bStoreMetaData;
    return OMX_ErrorNone;
}

OMX_U32 mapAndroidPixelFormat(OMX_COLOR_FORMATTYPE format)
{
    DEBUG(DEB_LEV_SIMPLE_SEQ, "%s format %d\n", __func__, format);
    OMX_U32 i = 0;
    for (i = 0; i < sizeof(s_format_tb)/sizeof(OMX_PIXEL_MAP_TABLE); i++)
    {
        if ((format&0x0fff) == s_format_tb[i].colorformat)
        {
            return s_format_tb[i].pixelformat;
        }
    }
    return HAL_PIXEL_FORMAT_NV12;
}

OMX_COLOR_FORMATTYPE mapOMXColorFormat(OMX_U32 index)
{
    DEBUG(DEB_LEV_SIMPLE_SEQ, "%s index %d\n", __func__, index);
    OMX_U32 i = 0;
    for (i = 0; i < sizeof(s_format_tb)/sizeof(OMX_PIXEL_MAP_TABLE); i++)
    {
        if (index == s_format_tb[i].pixelformat)
        {
            return s_format_tb[i].colorformat;
        }
    }
    return OMX_COLOR_FormatYUV420SemiPlanar;
}

OMX_U32 lockAndroidBufferHandle(buffer_handle_t handle, int width, int height, OMX_U32 mode, void **pAddr)
{
    OMX_U32 ret = 0;
    GraphicBufferMapper &mapper = GraphicBufferMapper::get();
    Rect bounds(width, height);

    if (mode == LOCK_MODE_TO_GET_VIRTUAL_ADDRESS)
        ret = mapper.lock(handle, GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_NEVER, bounds, pAddr);
    else // HW
        ret = mapper.lock(handle, GRALLOC_USAGE_HW_RENDER, bounds, pAddr);
    if (ret != 0) {
        DEBUG(DEB_LEV_ERR, "lockAndroidBufferHandle : mapper.lock error code:%d, buf->handle=0x%p", (int)ret, handle);
    }

    DEBUG(DEB_LEV_FULL_SEQ, "lockAndroidBufferHandle ret=0x%x, pAddrs=%p", (int)ret, *pAddr);

    return ret;
}

OMX_U32 unLockAndroidBufferHandle(buffer_handle_t handle)
{
    int ret = 0;
    GraphicBufferMapper &mapper = GraphicBufferMapper::get();

    ret = mapper.unlock(handle);
    if (ret != 0) {
        DEBUG(DEB_LEV_ERR, "unlockAndroidNativeBuffer : mapper.unlock error code:%d, buf->handle=0x%p", (int)ret, handle);
    }

    return ret;
}

OMX_BOOL convertRgbToYuvbySW(OMX_BYTE pYuvData, OMX_BYTE pRgbData, OMX_U32 rgbFormat, OMX_U32 width, OMX_U32 height)
{
    OMX_BYTE pY;
    OMX_BYTE pCb;
    OMX_BYTE pCr;
    unsigned int yuvFormat;

    pY = pYuvData;
    pCb = pYuvData + (width*height);
    pCr = pYuvData + (width*height) + (width*height)/4;

    switch (rgbFormat)
    {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            yuvFormat = libyuv::FOURCC_ABGR;
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            yuvFormat = libyuv::FOURCC_ABGR;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            yuvFormat = libyuv::FOURCC_24BG;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            yuvFormat = libyuv::FOURCC_RGBP;
            break;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            yuvFormat = libyuv::FOURCC_ARGB;
            break;
        case HAL_PIXEL_FORMAT_YCBCR_420_888:
            yuvFormat = libyuv::FOURCC_NV12;
            break;
        default:
            return OMX_FALSE;
    }

    if (0 != ConvertToI420(pRgbData, 0, pY, width, pCb, (width + 1)/2, pCr, (width + 1)/2, 0, 0, width, height, width, height, libyuv::kRotate0, yuvFormat))
        return OMX_FALSE;

    return OMX_TRUE;
}


OMX_U32 lockAndroidNativeBuffer(OMX_BUFFERHEADERTYPE *pNativeBufHeaderType, int stride, int height, OMX_U32 mode, void **pAddr)
{
    OMX_U32 ret = 0;
    buffer_handle_t buf_handle;
    Rect bounds(stride, height);

    GraphicBufferMapper &mapper = GraphicBufferMapper::get();
    buf_handle = (buffer_handle_t)pNativeBufHeaderType->pBuffer;
    if (!buf_handle) {
        DEBUG(DEB_LEV_ERR, "lockAndroidNativeBuffer : fail to get native buffer handle buf=0x%p, buf->handle=%p", buf_handle, (buf_handle) ? buf_handle : 0);
        return -1;
    }

    DEBUG(DEB_LEV_FULL_SEQ, "lockAndroidNativeBuffer : stride = %d, height = %d, buf->handle=%p", stride, height, (buf_handle) ? buf_handle : 0);
    if (mode == LOCK_MODE_TO_GET_VIRTUAL_ADDRESS)
        ret = mapper.lock(buf_handle, GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_NEVER, bounds, pAddr);
    else // HW
        ret = mapper.lock(buf_handle, GRALLOC_USAGE_HW_RENDER, bounds, pAddr);
    if (ret != 0) {
        DEBUG(DEB_LEV_ERR, "lockAndroidNativeBuffer : mapper.lock error code:%d, buf->handle=0x%p", (int)ret, buf_handle);
    }

    return ret;
}


OMX_U32 unlockAndroidNativeBuffer(OMX_BUFFERHEADERTYPE *pNativeBufHeaderType)
{
    OMX_S32 ret = 0;
    buffer_handle_t buf_handle;

    GraphicBufferMapper &mapper = GraphicBufferMapper::get();
    buf_handle = (buffer_handle_t)pNativeBufHeaderType->pBuffer;

    if (!buf_handle) {
        DEBUG(DEB_LEV_ERR, "unlockAndroidNativeBuffer : fail to get native buffer handle buf=0x%p, buf->handle=%p", buf_handle, (buf_handle) ? buf_handle : 0);
        return -1;
    }
    ret = mapper.unlock(buf_handle);
    if (ret != 0) {
        DEBUG(DEB_LEV_ERR, "unlockAndroidNativeBuffer : mapper.unlock error code:%d, buf->handle=0x%p", (int)ret, buf_handle);
    }

    return ret;
}

