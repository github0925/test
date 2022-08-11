#include "hw_buffer_utils.h"
#include "HwConverter.h"
#include <xf86drm.h>
#include <xf86drmMode.h>

#define DRM_LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

int hw_buffer_map(struct hw_buffer_t *bo)
{
    if (!bo || !bo->data) {
        DRM_LOG("invalid drm handle");
        return -1;
    }

    HwBuffer *buffer = NULL;
    buffer = (HwBuffer *)bo->data;

    buffer->MapBo();
    for (int i = 0; i < bo->n_planes; i++) {
        bo->mapped_vaddrs[i] = buffer->handle.mapped_vaddrs[i];
    }

    return 0;
}

int hw_buffer_unmap(struct hw_buffer_t *bo)
{
    if (!bo || !bo->data) {
        DRM_LOG("invalid drm handle");
        return -1;
    }

    HwBuffer *buffer = NULL;
    buffer = (HwBuffer *)bo->data;

    buffer->UnMapBo();
    for (int i = 0; i < bo->n_planes; i++) {
        bo->mapped_vaddrs[i] = nullptr;
    }

    return 0;
}

struct hw_buffer_t *hw_buffer_create(int width, int height, int format)
{

    HwBuffer *buffer = NULL;
    struct hw_buffer_t *bo = NULL;

    bo = (struct hw_buffer_t *)calloc(1, sizeof(struct hw_buffer_t));
    if (!bo) {
        DRM_LOG("calloc failed");
        return NULL;
    }

    bo->data = new HwBuffer(width, height, format);
    if (!bo->data) {
        DRM_LOG("create HwBuffer failed");
        free(bo);
        return NULL;
    }

    buffer = (HwBuffer *)bo->data;

    bo->n_planes = buffer->handle.n_planes;
    bo->width = buffer->handle.width;
    bo->height = buffer->handle.height;
    bo->format = buffer->handle.format;
    bo->size = buffer->handle.size;

    for (int i = 0; i < bo->n_planes; i++) {
        bo->fds[i] = buffer->handle.fds[i];
        bo->offsets[i] = buffer->handle.offsets[i];
        bo->strides[i] = buffer->handle.strides[i];
        bo->modifiers[i] = buffer->handle.modifiers[i];
    }

    return bo;
}

void hw_buffer_destroy(struct hw_buffer_t *bo)
{
    if (bo) {
        if (bo->data) {
            delete (HwBuffer *)bo->data;
            bo->data = NULL;
        }

        free(bo);
    }
}

/**********************************************************/

/* *INDENT-OFF* */
static const struct {
    guint32 fourcc;
    GstVideoFormat format;
} format_map[] = {
#define DEF_FMT(fourcc, fmt)                                                                       \
    {                                                                                              \
        DRM_FORMAT_##fourcc, GST_VIDEO_FORMAT_##fmt                                                \
    }

/* DEF_FMT (XRGB1555, ???), */
/* DEF_FMT (XBGR1555, ???), */
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    DEF_FMT(ARGB8888, BGRA), DEF_FMT(XRGB8888, BGRx), DEF_FMT(ABGR8888, RGBA),
    DEF_FMT(XBGR8888, RGBx), DEF_FMT(BGRX8888, xRGB), DEF_FMT(BGRA8888, ARGB),
    DEF_FMT(BGR888, BGR),    DEF_FMT(RGB888, RGB),
#else
    DEF_FMT(ARGB8888, ARGB), DEF_FMT(XRGB8888, xRGB), DEF_FMT(ABGR8888, ABGR),
    DEF_FMT(XBGR8888, xBGR), DEF_FMT(RGB888, RGB),    DEF_FMT(BGR888, BGR),
    DEF_FMT(BGRA8888, BGRA), DEF_FMT(BGRX8888, BGRx),
#endif
    DEF_FMT(UYVY, UYVY),     DEF_FMT(YUYV, YUY2),     DEF_FMT(YUV420, I420),
    DEF_FMT(YVU420, YV12),   DEF_FMT(YUV422, Y42B),   DEF_FMT(NV12, NV12),
    DEF_FMT(NV21, NV21),     DEF_FMT(NV16, NV16),

#undef DEF_FMT
};
/* *INDENT-ON* */

GstVideoFormat gst_video_format_from_drm(guint32 drmfmt)
{
    gint i;

    for (i = 0; i < G_N_ELEMENTS(format_map); i++) {
        if (format_map[i].fourcc == drmfmt)
            return format_map[i].format;
    }

    return GST_VIDEO_FORMAT_UNKNOWN;
}

guint32 gst_drm_format_from_video(GstVideoFormat fmt)
{
    gint i;

    for (i = 0; i < G_N_ELEMENTS(format_map); i++) {
        if (format_map[i].format == fmt)
            return format_map[i].fourcc;
    }

    return 0;
}

guint32 gst_drm_bpp_from_drm(guint32 drmfmt)
{
    guint32 bpp;

    switch (drmfmt) {
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YVU420:
    case DRM_FORMAT_YUV422:
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
    case DRM_FORMAT_NV16:
        bpp = 8;
        break;
    case DRM_FORMAT_UYVY:
    case DRM_FORMAT_YUYV:
    case DRM_FORMAT_YVYU:
        bpp = 16;
        break;
    case DRM_FORMAT_BGR888:
    case DRM_FORMAT_RGB888:
        bpp = 24;
        break;
    default:
        bpp = 32;
        break;
    }

    return bpp;
}

guint32 gst_drm_height_from_drm(guint32 drmfmt, guint32 height)
{
    guint32 ret;

    switch (drmfmt) {
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_YVU420:
    case DRM_FORMAT_YUV422:
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
        ret = height * 3 / 2;
        break;
    case DRM_FORMAT_NV16:
        ret = height * 2;
        break;
    default:
        ret = height;
        break;
    }

    return ret;
}