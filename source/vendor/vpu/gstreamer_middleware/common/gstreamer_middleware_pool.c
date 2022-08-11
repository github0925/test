#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <gst/allocators/gstdmabuf.h>
#include <gst/video/video.h>

#include "gstreamer_middleware_pool.h"

static G_DEFINE_QUARK(GstreamerMiddlewarePoolBufferQDataQuark,
                      gstreamer_middleware_pool_buffer_qdata);

#define gstreamer_middleware_pool_parent_class parent_class
G_DEFINE_TYPE(GstreamerMiddlewarePool, gstreamer_middleware_pool, GST_TYPE_BUFFER_POOL);

#define GST_TYPE_GSTREAMER_MIDDLEWARE_POOL (gstreamer_middleware_pool_get_type())
#define GSTREAMER_MIDDLEWARE_POLL_CAST(obj) ((GstreamerMiddlewarePool *)(obj))

int drmIoctl(int fd, unsigned long request, void *arg);

GstBufferPool *gstreamer_middleware_pool_new(gboolean is_dma_buffer)
{
    GstreamerMiddlewarePool *pool = NULL;
    pool = g_object_new(gstreamer_middleware_pool_get_type(), NULL);
    pool->is_dma_buffer = is_dma_buffer;

    return GST_BUFFER_POOL_CAST(pool);
}

int gstreamer_middleware_pool_buffer_map_buffer(GstreamerMiddlewarePoolBuffer *pool_buffer)
{
    int ret = 0;
    if (pool_buffer->is_dma_buffer && pool_buffer->bo && !pool_buffer->user_ptr) {
        ret = hw_buffer_map(pool_buffer->bo);
        pool_buffer->user_ptr = pool_buffer->bo->mapped_vaddrs[0];
    }

    return ret;
}

void gstreamer_middleware_pool_buffer_unmap_buffer(GstreamerMiddlewarePoolBuffer *pool_buffer)
{
    if (pool_buffer->bo && pool_buffer->user_ptr) {
        hw_buffer_unmap(pool_buffer->bo);
        pool_buffer->user_ptr = NULL;
    }
}

void gstreamer_middleware_pool_buffer_release_buffer(GstreamerMiddlewarePoolBuffer *pool_buffer)
{
    GST_DEBUG_OBJECT(NULL, "[%s]:[%d]", __func__, __LINE__);
    if (pool_buffer->bo && pool_buffer->user_ptr) {
        hw_buffer_unmap(pool_buffer->bo);
        pool_buffer->user_ptr = NULL;
    }

    gst_buffer_unref(pool_buffer->buffer);
}

GstreamerMiddlewarePoolBuffer *gstreamer_middleware_pool_buffer_get_buffer(GstBuffer *buffer)
{
    GstreamerMiddlewarePoolBuffer *pool_buffer = NULL;
    GST_DEBUG_OBJECT(NULL, "[%s]:[%d]", __func__, __LINE__);
    pool_buffer = gst_mini_object_get_qdata((GstMiniObject *)buffer,
                                            gstreamer_middleware_pool_buffer_qdata_quark());
    if (!pool_buffer)
        return NULL;

    return pool_buffer;
}

gpointer gstreamer_middleware_pool_buffer_get_user_ptr(GstBuffer *buffer)
{
    GstreamerMiddlewarePoolBuffer *pool_buffer = NULL;
    GST_DEBUG_OBJECT(NULL, "[%s]:[%d]", __func__, __LINE__);
    pool_buffer = gst_mini_object_get_qdata((GstMiniObject *)buffer,
                                            gstreamer_middleware_pool_buffer_qdata_quark());
    if (!pool_buffer || !pool_buffer->user_ptr)
        return NULL;

    GST_DEBUG_OBJECT(NULL, "[%s]:[%d] pool_buffer->user_ptr:%p", __func__, __LINE__,
                     pool_buffer->user_ptr);
    return pool_buffer->user_ptr;
}

static gboolean gstreamer_middleware_pool_set_config(GstBufferPool *bpool, GstStructure *config)
{
    GstreamerMiddlewarePool *self = GSTREAMER_MIDDLEWARE_POLL_CAST(bpool);
    guint min, max;
    GstCaps *caps = NULL;
    guint bufsize = 0;

    if (!gst_buffer_pool_config_get_params(config, &caps, &bufsize, &min, &max)) {
        GST_ERROR_OBJECT(self, "Failed to get config params");
        return FALSE;
    }

    GST_DEBUG_OBJECT(self, "[%s]:[%d] min:%d, max:%d caps %" GST_PTR_FORMAT, __func__, __LINE__,
                     min, max, caps);

    if (max)
        min = max;

    self->n_buffers = min;
    self->buf_size = bufsize;

    if (self->is_dma_buffer) {
        if (!caps) {
            GST_ERROR_OBJECT(self, "No caps in config");
            return FALSE;
        }
        if (!gst_video_info_from_caps(&self->vinfo, caps)) {
            GST_ERROR_OBJECT(self, "Invalid caps");
            return FALSE;
        }
        self->n_planes = GST_VIDEO_INFO_N_PLANES(&self->vinfo);
        if (!self->allocator)
            self->allocator = gst_dmabuf_allocator_new();
    }

    if (self->exported) {
        g_slice_free1(sizeof(gboolean) * self->n_buffers, self->exported);
        self->exported = NULL;
    }

    GST_DEBUG_OBJECT(self, "[%s]:[%d], size:%d, n_planes:%d", __func__, __LINE__, bufsize,
                     self->n_planes);

    return GST_BUFFER_POOL_CLASS(parent_class)->set_config(bpool, config);
}

static gboolean gstreamer_middleware_pool_start(GstBufferPool *bpool)
{
    GstreamerMiddlewarePool *self = GSTREAMER_MIDDLEWARE_POLL_CAST(bpool);
    GST_DEBUG_OBJECT(self, "[%s]:[%d]", __func__, __LINE__);

    self->exported = g_slice_alloc0(sizeof(gboolean) * self->n_buffers);

    return GST_BUFFER_POOL_CLASS(parent_class)->start(bpool);
}

static gboolean gstreamer_middleware_pool_stop(GstBufferPool *bpool)
{
    GstreamerMiddlewarePool *self = GSTREAMER_MIDDLEWARE_POLL_CAST(bpool);
    gboolean stop_success = TRUE;
    GST_DEBUG_OBJECT(self, "[%s]:[%d]", __func__, __LINE__);

    if (!GST_BUFFER_POOL_CLASS(parent_class)->stop(bpool)) {
        GST_ERROR_OBJECT(self, "Failed to free buffer");
        stop_success = FALSE;
    }

    g_slice_free1(sizeof(gboolean) * self->n_buffers, self->exported);
    self->exported = NULL;

    return stop_success;
}

static void free_pool_buffer(gpointer data)
{
    GstreamerMiddlewarePoolBuffer *pool_buffer = (GstreamerMiddlewarePoolBuffer *)data;
    GST_DEBUG_OBJECT(NULL, "[%s]:[%d]", __func__, __LINE__);
    if (pool_buffer->bo) {
        if (pool_buffer->user_ptr) {
            hw_buffer_unmap(pool_buffer->bo);
            pool_buffer->user_ptr = NULL;
        }
        hw_buffer_destroy(pool_buffer->bo);
        pool_buffer->bo = NULL;
    } else if (pool_buffer->user_ptr) {
        GstMemory *mem = gst_buffer_peek_memory(pool_buffer->buffer, 0);
        gst_memory_unmap(mem, &pool_buffer->map);
        pool_buffer->user_ptr = NULL;
    } else {
        GST_ERROR_OBJECT(NULL, "[%s]:[%d] this buffer has alread destory", __func__, __LINE__);
    }

    g_slice_free(GstreamerMiddlewarePoolBuffer, data);
}

static void gstreamer_middleware_pool_release_buffer(GstBufferPool *pool, GstBuffer *buffer)
{
    GstreamerMiddlewarePool *self = GSTREAMER_MIDDLEWARE_POLL_CAST(pool);
    GstreamerMiddlewarePoolBuffer *pool_buffer = NULL;

    GST_LOG_OBJECT(pool, "released buffer %p %d", buffer, GST_MINI_OBJECT_FLAGS(buffer));

    pool_buffer = gst_mini_object_get_qdata((GstMiniObject *)buffer,
                                            gstreamer_middleware_pool_buffer_qdata_quark());

    if (!pool_buffer) {
        GST_ERROR_OBJECT(self, "get GstreamerMiddlewarePoolBuffer failed");
        goto done;
    }

    if (pool_buffer->release_callback)
        pool_buffer->release_callback(pool_buffer->private_ptr, pool_buffer);

done:
    return GST_BUFFER_POOL_CLASS(parent_class)->release_buffer(pool, buffer);
}

static GstFlowReturn gstreamer_middleware_pool_alloc_buffer(GstBufferPool *bpool,
                                                            GstBuffer **buffer,
                                                            GstBufferPoolAcquireParams *params)
{
    GstreamerMiddlewarePool *self = GSTREAMER_MIDDLEWARE_POLL_CAST(bpool);
    GstreamerMiddlewarePoolBuffer *pool_buffer = NULL;
    GstMemory *mem = NULL;
    guint buf_index = INVALID_INDEX;
    gint i;
    gint stride[GST_VIDEO_MAX_PLANES];
    gsize offset[GST_VIDEO_MAX_PLANES];

    for (i = 0; i < self->n_buffers; i++) {
        if (!self->exported[i]) {
            buf_index = i;
            break;
        }
    }

    if (buf_index == INVALID_INDEX) {
        GST_ERROR_OBJECT(self, "No buffers are left");
        return GST_FLOW_ERROR;
    }

    GST_DEBUG_OBJECT(self, "[%s]:[%d] ", __func__, __LINE__);
    pool_buffer = g_slice_new0(GstreamerMiddlewarePoolBuffer);
    pool_buffer->buffer = gst_buffer_new();
    pool_buffer->index = buf_index;
    pool_buffer->buf_size = self->buf_size;

    if (self->is_dma_buffer) {
        pool_buffer->bo = hw_buffer_create(
            GST_VIDEO_INFO_WIDTH(&self->vinfo), GST_VIDEO_INFO_HEIGHT(&self->vinfo),
            gst_drm_format_from_video(GST_VIDEO_INFO_FORMAT(&self->vinfo)));

        if (!pool_buffer->bo) {
            GST_ERROR_OBJECT(self, "create bo drm failed");
            return GST_FLOW_ERROR;
        }

        pool_buffer->is_dma_buffer = TRUE;

        mem = gst_dmabuf_allocator_alloc(self->allocator, pool_buffer->bo->fds[0], self->buf_size);
        gst_buffer_append_memory(pool_buffer->buffer, mem);

        for (i = 0; i < self->n_planes; i++) {
            offset[i] = pool_buffer->bo->offsets[i];
            stride[i] = pool_buffer->bo->strides[i];
            GST_DEBUG_OBJECT(self, "[%s]:[%d] offset[%d]:%" G_GSIZE_FORMAT ", stride[%d]:%d:%p",
                             __func__, __LINE__, i, offset[i], i, stride[i], &stride[i]);
        }

        gst_buffer_add_video_meta_full(
            pool_buffer->buffer, GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_INFO_FORMAT(&self->vinfo),
            GST_VIDEO_INFO_WIDTH(&self->vinfo), GST_VIDEO_INFO_HEIGHT(&self->vinfo),
            GST_VIDEO_INFO_N_PLANES(&self->vinfo), offset, stride);
    } else {
        mem = gst_allocator_alloc(NULL, self->buf_size, NULL);
        gst_memory_map(mem, &pool_buffer->map, GST_MAP_READWRITE);
        pool_buffer->user_ptr = pool_buffer->map.data;
        gst_buffer_append_memory(pool_buffer->buffer, mem);
    }

    self->exported[buf_index] = TRUE;

    gst_mini_object_set_qdata((GstMiniObject *)pool_buffer->buffer,
                              gstreamer_middleware_pool_buffer_qdata_quark(), pool_buffer,
                              free_pool_buffer);

    *buffer = pool_buffer->buffer;

    GST_DEBUG_OBJECT(self, "[%s]:[%d] buffer:%p, index:%d, pool_buffer->user_ptr:%p", __func__,
                     __LINE__, pool_buffer->buffer, pool_buffer->index, pool_buffer->user_ptr);
    return GST_FLOW_OK;
}

static void gstreamer_middleware_pool_free_buffer(GstBufferPool *bpool, GstBuffer *buffer)
{
    GstreamerMiddlewarePool *self = GSTREAMER_MIDDLEWARE_POLL_CAST(bpool);
    GstreamerMiddlewarePoolBuffer *pool_buffer = NULL;

    GST_DEBUG_OBJECT(self, "[%s]:[%d]", __func__, __LINE__);
    pool_buffer = gst_mini_object_steal_qdata((GstMiniObject *)buffer,
                                              gstreamer_middleware_pool_buffer_qdata_quark());
    if (pool_buffer && self->exported)
        self->exported[pool_buffer->index] = FALSE;
    free_pool_buffer(pool_buffer);

    GST_BUFFER_POOL_CLASS(parent_class)->free_buffer(bpool, buffer);
}

static void gstreamer_middleware_pool_finalize(GObject *object)
{
    GstreamerMiddlewarePool *self = GSTREAMER_MIDDLEWARE_POLL_CAST(object);
    GST_DEBUG_OBJECT(self, "[%s]:[%d]", __func__, __LINE__);

    if (self->allocator) {
        gst_object_unref(self->allocator);
        self->allocator = NULL;
    }
    G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void gstreamer_middleware_pool_class_init(GstreamerMiddlewarePoolClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;
    GstBufferPoolClass *gstbufferpool_class = (GstBufferPoolClass *)klass;

    gobject_class->finalize = gstreamer_middleware_pool_finalize;

    gstbufferpool_class->set_config = gstreamer_middleware_pool_set_config;
    gstbufferpool_class->start = gstreamer_middleware_pool_start;
    gstbufferpool_class->stop = gstreamer_middleware_pool_stop;
    gstbufferpool_class->release_buffer = gstreamer_middleware_pool_release_buffer;
    gstbufferpool_class->alloc_buffer = gstreamer_middleware_pool_alloc_buffer;
    gstbufferpool_class->free_buffer = gstreamer_middleware_pool_free_buffer;
}

static void gstreamer_middleware_pool_init(GstreamerMiddlewarePool *pool) {}
