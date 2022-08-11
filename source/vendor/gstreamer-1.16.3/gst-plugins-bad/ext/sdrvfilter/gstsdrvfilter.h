#ifndef __GST_SDRV_FILTER_H__
#define __GST_SDRV_FILTER_H__

#include <gst/gst.h>
#include <gst/video/gstvideofilter.h>
#include <gst/video/video.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

G_BEGIN_DECLS
#define GST_TYPE_SDRV_FILTER (gst_sdrv_filter_get_type ())
#define GST_SDRV_FILTER(obj)                                                  \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_SDRV_FILTER, GstSdrvFilter))
#define GST_SDRV_FILTER_CLASS(klass)                                          \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_SDRV_FILTER, GstSdrvFilterClass))
#define GST_IS_VIDEO_CONVERT(obj)                                             \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_SDRV_FILTER))
#define GST_IS_VIDEO_CONVERT_CLASS(klass)                                     \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_SDRV_FILTER))
#define GST_SDRV_FILTER_CAST(obj) ((GstSdrvFilter *)(obj))

typedef enum
{
  GST_SDRVFILTER_IO_AUTO = 0, /* dmabuf or mmap */
  GST_SDRVFILTER_IO_USERPTR
} GstSdrvfilterIOMode;

#define DEFAULT_PROP_IO_MODE GST_SDRVFILTER_IO_AUTO

typedef struct _GstSdrvFilter GstSdrvFilter;
typedef struct _GstSdrvFilterClass GstSdrvFilterClass;

typedef struct _GstSdrvFilterDeviceInfo GstSdrvFilterDeviceInfo;

enum
{
  SINK,
  SRC
};

struct _GstSdrvFilterDeviceInfo
{
  GstBufferPool *input_pool;
  GstBufferPool *output_pool;
  GstSdrvfilterIOMode io_mode;
  GstVideoCropMeta out_crop;

  gboolean is_device_open;
  gboolean is_input_dmabuffer;
  gboolean is_output_dmabuffer;
  gboolean is_will_crop;
};

/**
 * GstSdrvFilter:
 *
 * Opaque object data structure.
 */
struct _GstSdrvFilter
{
  GstVideoFilter element;

  GstSdrvFilterDeviceInfo device;
};

struct _GstSdrvFilterClass
{
  GstVideoFilterClass parent_class;
};

G_END_DECLS
#endif /* __GST_SDRV_FILTER_H__ */
