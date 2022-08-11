#ifndef __GST_SDRVFILTER_BUFFER_POOL_H__
#define __GST_SDRVFILTER_BUFFER_POOL_H__

#include "gstdrmutils.h"
#include <gst/gstbufferpool.h>
#include <gst/video/video.h>
#include <linux/videodev2.h>

#define SDRVFILTER_INDEX_INVALID G_MAXUINT

typedef struct _SdrvfilterBufferPool SdrvfilterBufferPool;
typedef struct _SdrvfilterBufferPoolClass SdrvfilterBufferPoolClass;
typedef struct _SdrvfilterBuffer SdrvfilterBuffer;

struct _SdrvfilterBufferPool
{
  GstBufferPool bufferpool;
  GstAllocator *allocator;
  GstVideoInfo vinfo;
  guint n_planes;
  guint n_buffers;
  guint buf_size;
  gboolean *exported;
  gboolean is_map;
  dm_drm_t *drm_device;
};

struct _SdrvfilterBuffer
{
  GstBuffer *buffer;
  dm_drm_bo_t *bo;
  guint index;
  gpointer user_ptr;
};

struct _SdrvfilterBufferPoolClass
{
  GstBufferPoolClass parent_class;
};

GstBufferPool *sdrvfilter_buffer_pool_new (gboolean is_map);
guint sdrvfilter_buffer_pool_get_buffer_index (GstBuffer *buffer);
GType sdrvfilter_buffer_pool_get_type (void);
gpointer sdrvfilter_buffer_get_user_ptr (GstBuffer *buffer);
gpointer sdrvfilter_buffer_map_fd_to_user_ptr (GstBufferPool *pool, gint fd,
                                               gsize size);

#endif /*__GST_SDRVFILTER_BUFFER_POOL__*/
