#include <gst/allocators/gstdmabuf.h>
#include <gst/video/video.h>

#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "gstg2dapi.h"
#include "gstsdrvfilterpool.h"

GST_DEBUG_CATEGORY_EXTERN (sdrvfilter_debug);
#define GST_CAT_DEFAULT sdrvfilter_debug

static G_DEFINE_QUARK (SdrvfilterBufferQDataQuark, sdrvfilter_buffer_qdata);

#define sdrvfilter_buffer_pool_parent_class parent_class
G_DEFINE_TYPE (SdrvfilterBufferPool, sdrvfilter_buffer_pool,
    GST_TYPE_BUFFER_POOL);

#define GST_TYPE_SDRVFILTER_BUFFER_POOL (sdrvfilter_buffer_pool_get_type ())
#define SDRVFILTER_BUFFER_POOL_CAST(obj) ((SdrvfilterBufferPool *)(obj))

GstBufferPool *
sdrvfilter_buffer_pool_new (gboolean is_map)
{
  SdrvfilterBufferPool *pool = NULL;
  pool = g_object_new (sdrvfilter_buffer_pool_get_type (), NULL);
  pool->is_map = is_map;

  return GST_BUFFER_POOL_CAST (pool);
}

guint
sdrvfilter_buffer_pool_get_buffer_index (GstBuffer * buffer)
{
  SdrvfilterBuffer *vf_buffer = NULL;
  GST_DEBUG_OBJECT (NULL, "[%s]:[%d]", __func__, __LINE__);
  vf_buffer = gst_mini_object_get_qdata ((GstMiniObject *) buffer,
      sdrvfilter_buffer_qdata_quark ());
  if (!vf_buffer)
    return SDRVFILTER_INDEX_INVALID;

  return vf_buffer->index;
}

gpointer
sdrvfilter_buffer_map_fd_to_user_ptr (GstBufferPool * pool, gint fd, gsize size)
{
  SdrvfilterBufferPool *self = SDRVFILTER_BUFFER_POOL_CAST (pool);

  gpointer addr = NULL;
  struct drm_prime_handle args;
  struct drm_mode_map_dumb map_arg;

  GST_DEBUG_OBJECT (self, "[%s]:[%d], buffer fd:%d, buffer size:%ld", __func__,
      __LINE__, fd, size);

  memset (&args, 0, sizeof (struct drm_prime_handle));
  memset (&map_arg, 0, sizeof (map_arg));

  args.fd = fd;
  args.flags = 0;

  drmIoctl (self->drm_device->fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &args);

  map_arg.handle = args.handle;
  drmIoctl (self->drm_device->fd, DRM_IOCTL_MODE_MAP_DUMB, &map_arg);

  addr = mmap (0, size, PROT_WRITE | PROT_READ, MAP_SHARED,
      self->drm_device->fd, map_arg.offset);

  return addr;
}

gpointer
sdrvfilter_buffer_get_user_ptr (GstBuffer * buffer)
{
  SdrvfilterBuffer *vf_buffer = NULL;
  GST_DEBUG_OBJECT (NULL, "[%s]:[%d]", __func__, __LINE__);
  vf_buffer = gst_mini_object_get_qdata ((GstMiniObject *) buffer,
      sdrvfilter_buffer_qdata_quark ());
  if (!vf_buffer || !vf_buffer->user_ptr)
    return NULL;

  GST_DEBUG_OBJECT (NULL, "[%s]:[%d] vf_buffer->user_ptr:%p", __func__,
      __LINE__, vf_buffer->user_ptr);
  return vf_buffer->user_ptr;
}

static gboolean
sdrvfilter_buffer_pool_set_config (GstBufferPool * bpool, GstStructure * config)
{
  SdrvfilterBufferPool *self = SDRVFILTER_BUFFER_POOL_CAST (bpool);
  guint min, max;
  GstCaps *caps = NULL;
  guint bufsize = 0;

  if (!gst_buffer_pool_config_get_params (config, &caps, &bufsize, &min, &max)) {
    GST_ERROR_OBJECT (self, "Failed to get config params");
    return FALSE;
  }

  GST_DEBUG_OBJECT (self, "[%s]:[%d] min:%d, max:%d", __func__, __LINE__, min,
      max);

  if (!caps) {
    GST_ERROR_OBJECT (self, "No caps in config");
    return FALSE;
  }

  if (max)
    min = max;

  if (!gst_video_info_from_caps (&self->vinfo, caps)) {
    GST_ERROR_OBJECT (self, "Invalid caps");
    return FALSE;
  }

  if (self->exported) {
    g_slice_free1 (sizeof (gboolean) * self->n_buffers, self->exported);
    self->exported = NULL;
  }

  self->n_buffers = min;
  self->n_planes = GST_VIDEO_INFO_N_PLANES (&self->vinfo);
  self->buf_size = bufsize;

  if (!self->allocator)
    self->allocator = gst_dmabuf_allocator_new ();

  GST_DEBUG_OBJECT (self, "[%s]:[%d], size:%d, n_planes:%d", __func__,
      __LINE__, bufsize, self->n_planes);

  return GST_BUFFER_POOL_CLASS (parent_class)->set_config (bpool, config);
}

static gboolean
sdrvfilter_buffer_pool_start (GstBufferPool * bpool)
{
  SdrvfilterBufferPool *self = SDRVFILTER_BUFFER_POOL_CAST (bpool);
  GST_DEBUG_OBJECT (self, "[%s]:[%d]", __func__, __LINE__);

  // open drm device
  if (NULL == self->drm_device) {
    self->drm_device = dm_drm_create ();
    if (!self->drm_device) {
      GST_ERROR_OBJECT (self, "cannot open drm device");
      return FALSE;
    }
  }

  self->exported = g_slice_alloc0 (sizeof (gboolean) * self->n_buffers);

  return GST_BUFFER_POOL_CLASS (parent_class)->start (bpool);
}

static gboolean
sdrvfilter_buffer_pool_stop (GstBufferPool * bpool)
{
  SdrvfilterBufferPool *self = SDRVFILTER_BUFFER_POOL_CAST (bpool);
  gboolean stop_success = TRUE;
  GST_DEBUG_OBJECT (self, "[%s]:[%d]", __func__, __LINE__);

  if (!GST_BUFFER_POOL_CLASS (parent_class)->stop (bpool)) {
    GST_ERROR_OBJECT (self, "Failed to free buffer");
    stop_success = FALSE;
  }

  g_slice_free1 (sizeof (gboolean) * self->n_buffers, self->exported);
  self->exported = NULL;

  return stop_success;
}

static void
free_vf_buffer (gpointer data)
{
  SdrvfilterBuffer *sdrvBuffer = (SdrvfilterBuffer *) data;
  GST_DEBUG_OBJECT (NULL, "[%s]:[%d]", __func__, __LINE__);
  if (sdrvBuffer->user_ptr) {
    dm_drm_bo_unlock (sdrvBuffer->bo);
    sdrvBuffer->user_ptr = NULL;
  }
  sdrvBuffer->bo->drm->drv->free (sdrvBuffer->bo->drm, sdrvBuffer->bo);
  sdrvBuffer->bo = NULL;
  g_slice_free (SdrvfilterBuffer, data);
}

static GstFlowReturn
sdrvfilter_buffer_pool_alloc_buffer (GstBufferPool * bpool, GstBuffer ** buffer,
    GstBufferPoolAcquireParams * params)
{
  SdrvfilterBufferPool *self = SDRVFILTER_BUFFER_POOL_CAST (bpool);
  SdrvfilterBuffer *vf_buffer = NULL;
  GstMemory *mem = NULL;
  guint buf_index = SDRVFILTER_INDEX_INVALID;
  gint i;
  gint stride[GST_VIDEO_MAX_PLANES];
  gsize offset[GST_VIDEO_MAX_PLANES];

  for (i = 0; i < self->n_buffers; i++) {
    if (!self->exported[i]) {
      buf_index = i;
      break;
    }
  }

  if (buf_index == SDRVFILTER_INDEX_INVALID) {
    GST_ERROR_OBJECT (self, "No buffers are left");
    return GST_FLOW_ERROR;
  }

  GST_DEBUG_OBJECT (self, "[%s]:[%d] ", __func__, __LINE__);
  vf_buffer = g_slice_new0 (SdrvfilterBuffer);
  vf_buffer->buffer = gst_buffer_new ();
  vf_buffer->index = buf_index;
  vf_buffer->bo =
      dm_drm_bo_create (self->drm_device, GST_VIDEO_INFO_WIDTH (&self->vinfo),
      GST_VIDEO_INFO_HEIGHT (&self->vinfo),
      gst_drm_format_from_video (GST_VIDEO_INFO_FORMAT (&self->vinfo)), 0);
  if (!vf_buffer->bo) {
    GST_ERROR_OBJECT (self, "create bo drm failed");
    return GST_FLOW_ERROR;
  }

  if (self->is_map
      && dm_drm_bo_lock (vf_buffer->bo, 0, 0, 0,
          GST_VIDEO_INFO_WIDTH (&self->vinfo),
          GST_VIDEO_INFO_HEIGHT (&self->vinfo), &vf_buffer->user_ptr))
    return GST_FLOW_ERROR;

  mem =
      gst_dmabuf_allocator_alloc (self->allocator,
      vf_buffer->bo->handle->buf_fd[0], self->buf_size);
  gst_buffer_append_memory (vf_buffer->buffer, mem);

  for (i = 0; i < self->n_planes; i++) {
    offset[i] = vf_buffer->bo->handle->offset[i];
    stride[i] = vf_buffer->bo->handle->stride[i];
    GST_DEBUG_OBJECT (self,
        "[%s]:[%d] offset[%d]:%" G_GSIZE_FORMAT ", stride[%d]:%d:%p", __func__,
        __LINE__, i, offset[i], i, stride[i], &stride[i]);
  }

  gst_buffer_add_video_meta_full (vf_buffer->buffer, GST_VIDEO_FRAME_FLAG_NONE,
      GST_VIDEO_INFO_FORMAT (&self->vinfo),
      GST_VIDEO_INFO_WIDTH (&self->vinfo),
      GST_VIDEO_INFO_HEIGHT (&self->vinfo),
      GST_VIDEO_INFO_N_PLANES (&self->vinfo), offset, stride);

  self->exported[buf_index] = TRUE;

  gst_mini_object_set_qdata ((GstMiniObject *) vf_buffer->buffer,
      sdrvfilter_buffer_qdata_quark (), vf_buffer, free_vf_buffer);

  *buffer = vf_buffer->buffer;

  GST_DEBUG_OBJECT (self,
      "[%s]:[%d] buffer:%p, index:%d, fd:%d vf_buffer->user_ptr:%p", __func__,
      __LINE__, vf_buffer->buffer, vf_buffer->index,
      vf_buffer->bo->handle->prime_fd, vf_buffer->user_ptr);
  return GST_FLOW_OK;
}

static void
sdrvfilter_buffer_pool_free_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  SdrvfilterBufferPool *self = SDRVFILTER_BUFFER_POOL_CAST (bpool);
  SdrvfilterBuffer *vf_buffer = NULL;

  GST_DEBUG_OBJECT (self, "[%s]:[%d]", __func__, __LINE__);
  vf_buffer = gst_mini_object_steal_qdata ((GstMiniObject *) buffer,
      sdrvfilter_buffer_qdata_quark ());
  if (vf_buffer && self->exported)
    self->exported[vf_buffer->index] = FALSE;
  free_vf_buffer (vf_buffer);

  GST_BUFFER_POOL_CLASS (parent_class)->free_buffer (bpool, buffer);
}

static void
sdrvfilter_buffer_pool_finalize (GObject * object)
{
  SdrvfilterBufferPool *self = SDRVFILTER_BUFFER_POOL_CAST (object);
  GST_DEBUG_OBJECT (self, "[%s]:[%d]", __func__, __LINE__);

  // close drm device
  if (self->drm_device) {
    dm_drm_destroy (self->drm_device);
    self->drm_device = NULL;
  }

  if (self->allocator) {
    gst_object_unref (self->allocator);
    self->allocator = NULL;
  }
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
sdrvfilter_buffer_pool_class_init (SdrvfilterBufferPoolClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstBufferPoolClass *gstbufferpool_class = (GstBufferPoolClass *) klass;

  gobject_class->finalize = sdrvfilter_buffer_pool_finalize;

  gstbufferpool_class->set_config = sdrvfilter_buffer_pool_set_config;
  gstbufferpool_class->start = sdrvfilter_buffer_pool_start;
  gstbufferpool_class->stop = sdrvfilter_buffer_pool_stop;
  gstbufferpool_class->alloc_buffer = sdrvfilter_buffer_pool_alloc_buffer;
  gstbufferpool_class->free_buffer = sdrvfilter_buffer_pool_free_buffer;
}

static void
sdrvfilter_buffer_pool_init (SdrvfilterBufferPool * pool)
{
}
