#ifndef __GSTREAMER_MIDDLEWARE_POOL_H__
#define __GSTREAMER_MIDDLEWARE_POOL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "hw_buffer_utils.h"
#include <gst/gstbufferpool.h>
#include <gst/video/video.h>
#include <linux/videodev2.h>

#define INVALID_INDEX G_MAXUINT

typedef struct _GstreamerMiddlewarePool GstreamerMiddlewarePool;
typedef struct _GstreamerMiddlewarePoolClass GstreamerMiddlewarePoolClass;
typedef struct _GstreamerMiddlewarePoolBuffer GstreamerMiddlewarePoolBuffer;
typedef void (*gstreamer_middleware_pool_release_input_buffer)(
    void *private_ptr, GstreamerMiddlewarePoolBuffer *buffer);

struct _GstreamerMiddlewarePool {
    GstBufferPool bufferpool;
    GstAllocator *allocator;
    GstVideoInfo vinfo;
    guint n_planes;
    guint n_buffers;
    guint buf_size;
    gboolean *exported;
    gboolean is_dma_buffer;
};

struct _GstreamerMiddlewarePoolBuffer {
    GstBuffer *buffer;
    hw_buffer_t *bo;
    gboolean is_dma_buffer;
    guint index;
    guint buf_size;
    GstMapInfo map;
    gpointer user_ptr;
    gpointer private_ptr;
    gstreamer_middleware_pool_release_input_buffer release_callback;
};

struct _GstreamerMiddlewarePoolClass {
    GstBufferPoolClass parent_class;
};

GstBufferPool *gstreamer_middleware_pool_new(gboolean is_dma_buffer);
GType gstreamer_middleware_pool_get_type(void);
GstreamerMiddlewarePoolBuffer *gstreamer_middleware_pool_buffer_get_buffer(GstBuffer *buffer);
gpointer gstreamer_middleware_pool_buffer_get_user_ptr(GstBuffer *buffer);
void gstreamer_middleware_pool_buffer_release_buffer(GstreamerMiddlewarePoolBuffer *pool_buffer);
int gstreamer_middleware_pool_buffer_map_buffer(GstreamerMiddlewarePoolBuffer *pool_buffer);
void gstreamer_middleware_pool_buffer_unmap_buffer(GstreamerMiddlewarePoolBuffer *pool_buffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__GSTREAMER_MIDDLEWARE_POOL_H__