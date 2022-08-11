#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstg2dapi.h"
#include "gstsdrvfilter.h"
#include "gstsdrvfilterpool.h"

#include <gst/allocators/gstdmabuf.h>
#include <gst/gstquery.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>
#include <gst/video/video.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GST_DEBUG_CATEGORY (sdrvfilter_debug);
#define GST_CAT_DEFAULT sdrvfilter_debug
GST_DEBUG_CATEGORY_EXTERN (GST_CAT_PERFORMANCE);

GType gst_sdrv_filter_get_type (void);

#define gst_sdrv_filter_parent_class parent_class
G_DEFINE_TYPE (GstSdrvFilter, gst_sdrv_filter, GST_TYPE_VIDEO_FILTER);

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define DEFAULT_BUFFER_POOL_NUMBER (3)
#define DEFAULT_INPUT_TEMP_DMABUFFER_NUMBER (1)

// #define DUMP_INPUT_BUFFER
// #define DUMP_OUTPUT_BUFFER

//#define USE_CPU_CROP
//#define VIRTUAL_INPUT_BUFFER_TO_G2D
//#define VIRTUAL_OUTPUT_BUFFER_TO_G2D

//#define OPEN_CROP_FEATURE

enum
{
  PROP_0,
  PROP_INPUT_IO_MODE,
  PROP_OUTPUT_IO_MODE,
};

#define SDRVF_SINK_VIDEO_CAPS                                                 \
  "{ I420, NV12, NV21, NV16, UYVY, YUY2, RGB16, RGB, BGR, ARGB, xRGB, BGRA, " \
  "BGRx }"

#define SDRVF_SRC_VIDEO_CAPS "{ I420 }"

static GstStaticPadTemplate gst_sdrv_filter_sink_template
    = GST_STATIC_PAD_TEMPLATE ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (SDRVF_SINK_VIDEO_CAPS) ";"
        GST_VIDEO_CAPS_MAKE_WITH_FEATURES (GST_CAPS_FEATURE_MEMORY_DMABUF,
            SDRVF_SINK_VIDEO_CAPS)));

static GstStaticPadTemplate gst_sdrv_filter_src_template
    = GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES
        (GST_CAPS_FEATURE_MEMORY_DMABUF, SDRVF_SRC_VIDEO_CAPS)));

static void gst_sdrv_filter_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec);
static void gst_sdrv_filter_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_sdrv_filter_stop (GstBaseTransform * trans);

#define GST_TYPE_SDRVFILTER_IO_MODE (gst_sdrv_filter_io_mode_get_type ())
static GType
gst_sdrv_filter_io_mode_get_type (void)
{
  static GType sdrvfilter_io_mode = 0;

  if (!sdrvfilter_io_mode) {
    static const GEnumValue io_modes[]
        = { {GST_SDRVFILTER_IO_AUTO, "GST_SDRVFILTER_IO_AUTO",
        "auto (dmabuf or mmap)"},
    {GST_SDRVFILTER_IO_USERPTR, "GST_SDRVFILTER_IO_USERPTR",
        "userptr"},
    {0, NULL, NULL}
    };
    sdrvfilter_io_mode
        = g_enum_register_static ("GstSdrvfilterIOMode", io_modes);
  }
  return sdrvfilter_io_mode;
}

static gboolean
intersect_format (GstCapsFeatures * features, GstStructure * structure,
    gpointer user_data)
{
  const GValue *in_format = user_data;
  GValue out_format = { 0 };

  if (!gst_value_intersect (&out_format, in_format,
          gst_structure_get_value (structure, "format"))) {
    return FALSE;
  }

  gst_structure_fixate_field_string (structure, "format",
      g_value_get_string (&out_format));

  g_value_unset (&out_format);

  return TRUE;
}

static GstCaps *
gst_sdrv_filter_fixate_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * othercaps)
{
  GstCaps *result = NULL;
  GstCaps *outcaps = NULL;
  gint from_w, from_h;
  gint w = 0, h = 0;
  GstStructure *ins = NULL, *outs = NULL;
  GstSdrvFilter *space = NULL;
  const GValue *in_format = NULL;

  gint in_framerate_numerator = 0, in_framerate_denominator = 0;
  gint out_framerate_numerator = 0, out_framerate_denominator = 0;
  const gchar *in_interlace_mode = NULL;
  const gchar *out_interlace_mode = NULL;

  space = GST_SDRV_FILTER_CAST (trans);

  GST_DEBUG_OBJECT (space, "caps %" GST_PTR_FORMAT, caps);
  GST_DEBUG_OBJECT (space, "othercaps %" GST_PTR_FORMAT, othercaps);

  othercaps = gst_caps_make_writable (othercaps);

  ins = gst_caps_get_structure (caps, 0);       // this caps is get from transform_caps func
  in_format = gst_structure_get_value (ins, "format");

  outcaps = gst_caps_copy (othercaps);
  gst_caps_filter_and_map_in_place (outcaps, intersect_format, (gpointer)
      in_format);               // try to intersect a same format for input & output caps
  if (gst_caps_is_empty (outcaps))
    gst_caps_replace (&outcaps, othercaps);

  gst_caps_unref (othercaps);

  outcaps = gst_caps_truncate (outcaps);        // get the first structure caps
  outs = gst_caps_get_structure (outcaps, 0);

  gst_structure_get_int (ins, "width", &from_w);
  gst_structure_get_int (ins, "height", &from_h);
  gst_structure_get_fraction (ins, "framerate", &in_framerate_numerator,
      &in_framerate_denominator);
  in_interlace_mode = gst_structure_get_string (ins, "interlace-mode");

  gst_structure_get_int (outs, "width", &w);
  gst_structure_get_int (outs, "height", &h);
  gst_structure_get_fraction (outs, "framerate", &out_framerate_numerator,
      &out_framerate_denominator);
  out_interlace_mode = gst_structure_get_string (outs, "interlace-mode");

  if (!w || !h) {
    gst_structure_fixate_field_nearest_int (outs, "height", from_h);
    gst_structure_fixate_field_nearest_int (outs, "width", from_w);
  }

  result = gst_caps_intersect (outcaps, caps);
  if (gst_caps_is_empty (result)) {
    gst_caps_unref (result);
    result = outcaps;
  } else {
    gst_caps_unref (outcaps);
  }

  /* fixate remaining fields */
  result = gst_caps_fixate (result);

  if (direction == GST_PAD_SINK) {
#if defined OPEN_CROP_FEATURE
    GstVideoInfo in_info, out_info;

    gboolean isCrop = FALSE;
    GstStructure *s = gst_caps_get_structure (caps, 0);

    gst_video_info_from_caps (&in_info, caps);
    gst_video_info_from_caps (&out_info, result);

    if (gst_structure_get_boolean (s, "crop", &isCrop) && isCrop) {

      if (gst_structure_get_uint (s, "crop_x", &space->device.out_crop.x)
          && gst_structure_get_uint (s, "crop_y", &space->device.out_crop.y)
          && gst_structure_get_uint (s, "crop_width",
              &space->device.out_crop.width)
          && gst_structure_get_uint (s, "crop_height",
              &space->device.out_crop.height)) {
        space->device.is_will_crop = TRUE;
        GST_DEBUG_OBJECT (space,
            "will crop the input buffer, so turn off passthought mode, "
            "crop_x:%u, crop_y:%u, crop_width:%u, crop_height:%u",
            space->device.out_crop.x, space->device.out_crop.y,
            space->device.out_crop.width, space->device.out_crop.height);
      } else {
        isCrop = FALSE;
        space->device.is_will_crop = FALSE;
        GST_INFO_OBJECT (space, "error crop info, please check caps");
      }
    }

    if (!space->device.is_will_crop) {
      GST_DEBUG_OBJECT (space,
          "no crop info, we will in passthrough mode on same caps");
      GST_BASE_TRANSFORM_GET_CLASS (trans)->passthrough_on_same_caps = TRUE;
    }

    if (!w) {
      if (isCrop)
        gst_caps_set_simple (result, "width", G_TYPE_INT,
            space->device.out_crop.width, NULL);
      else
        gst_caps_set_simple (result, "width", G_TYPE_INT, from_w, NULL);
    }

    if (!h) {
      if (isCrop)
        gst_caps_set_simple (result, "height", G_TYPE_INT,
            space->device.out_crop.height, NULL);
      else
        gst_caps_set_simple (result, "height", G_TYPE_INT, from_h, NULL);
    }
#else
    GST_BASE_TRANSFORM_GET_CLASS (trans)->passthrough_on_same_caps = TRUE;

    if (!w)
      gst_caps_set_simple (result, "width", G_TYPE_INT, from_w, NULL);

    if (!h)
      gst_caps_set_simple (result, "height", G_TYPE_INT, from_h, NULL);
#endif

    if (out_framerate_numerator == 0 || out_framerate_denominator == 0)
      gst_caps_set_simple (result, "framerate", GST_TYPE_FRACTION,
          in_framerate_numerator, in_framerate_denominator, NULL);

    if (NULL == out_interlace_mode)
      gst_caps_set_simple (result, "interlace-mode", G_TYPE_STRING,
          in_interlace_mode, NULL);
  }

  GST_DEBUG_OBJECT (space, "pad direction:%d result caps %" GST_PTR_FORMAT,
      direction, result);

  return result;
}

static gboolean
gst_sdrv_filter_filter_meta (GstBaseTransform * trans, GstQuery * query,
    GType api, const GstStructure * params)
{
  GST_DEBUG_OBJECT (trans, "[%s]:[%d]", __func__, __LINE__);
  /* propose all metadata upstream */
  return TRUE;
}

static gboolean
remove_video_info (GstCapsFeatures * features, GstStructure * structure,
    gpointer user_data)
{
  gst_structure_remove_fields (structure, "colorimetry", "chroma-site",
      "width", "height", NULL);
  return TRUE;
}

/* given @caps on the src or sink pad (given by @direction)
 * calculate the possible caps on the other pad.
 */
/* The caps can be transformed into any other caps with format info removed.
 * However, we should prefer passthrough, so if passthrough is possible,
 * put it first in the list.
 */
static GstCaps *
gst_sdrv_filter_transform_caps (GstBaseTransform * btrans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
  GstSdrvFilter *space = NULL;
  GstCaps *result = NULL;
  GstCaps *caps_copy = NULL;
  GstCaps *caps_intersected = NULL;
  GstCaps *template = NULL;

  space = GST_SDRV_FILTER_CAST (btrans);
  GST_DEBUG_OBJECT (space, "caps %" GST_PTR_FORMAT, caps);
  caps_copy = gst_caps_copy (caps);

  gst_caps_map_in_place (caps_copy, remove_video_info, NULL);
  caps_copy = gst_caps_simplify (caps_copy);

  if (direction == GST_PAD_SRC) // calculate the possible caps on the other
    // pad, possible use passthrough mode
    template
        = gst_static_pad_template_get_caps (&gst_sdrv_filter_sink_template);
  else
    template = gst_static_pad_template_get_caps (&gst_sdrv_filter_src_template);

  caps_intersected = gst_caps_intersect (caps_copy, template);
  GST_DEBUG_OBJECT (space,
      "[%s]:[%d] template: %" GST_PTR_FORMAT
      " into caps_intersected: %" GST_PTR_FORMAT
      "filter caps:  %" GST_PTR_FORMAT "caps_copy caps: %" GST_PTR_FORMAT,
      __func__, __LINE__, template, caps_intersected, filter, caps_copy);

  gst_caps_unref (caps_copy);

  if (gst_caps_is_empty (caps_intersected)) {
    gst_caps_unref (caps_intersected);
    result = template;
    goto done;
  }
  gst_caps_unref (template);

  if (filter) {
    result = gst_caps_intersect_full (filter, caps_intersected,
        GST_CAPS_INTERSECT_FIRST);

    if (gst_caps_is_empty (result)) {
      gst_caps_unref (result);
      result = caps_intersected;
      goto done;
    }
    gst_caps_unref (caps_intersected);
  } else {
    result = caps_intersected;
  }

done:
  GST_DEBUG_OBJECT (space,
      "direction:%d transformed %" GST_PTR_FORMAT
      " into %" GST_PTR_FORMAT, direction, caps, result);
  return result;
}

static gboolean
gst_sdrv_filter_transform_meta (GstBaseTransform * trans, GstBuffer * outbuf,
    GstMeta * meta, GstBuffer * inbuf)
{
  GST_DEBUG_OBJECT (trans, "[%s]:[%d]", __func__, __LINE__);
  /* copy other metadata */
  return TRUE;
}

static void
gst_sdrv_filter_finalize (GObject * obj)
{
  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static gboolean
gst_sdrv_filter_sdrv_device_init (GstSdrvFilter * space)
{
  // open g2d
  gint ret = g2d_open ((GstObject *) space);
  if (ret) {
    GST_ERROR_OBJECT (space, "cannot open g2d device");
    return FALSE;
  }

  space->device.is_device_open = TRUE;
  return TRUE;
}

static void
gst_sdrv_filter_sdrv_device_deinit (GstSdrvFilter * space)
{
  g2d_close ();

  space->device.is_device_open = FALSE;
}

static GstStateChangeReturn
gst_sdrv_filter_change_state (GstElement * element, GstStateChange transition)
{
  GstSdrvFilter *space = NULL;
  GstStateChangeReturn ret;

  space = GST_SDRV_FILTER_CAST (element);
  GST_DEBUG_OBJECT (space, "[%s]:[%d] stateChange to:%d", __func__, __LINE__,
      transition);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      if (!gst_sdrv_filter_sdrv_device_init (space)) {
        GST_ERROR_OBJECT (space, "failed to initialize the sdrv device");
        return GST_STATE_CHANGE_FAILURE;
      }
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
      if (space->device.output_pool)
        g_clear_object (&space->device.output_pool);
      space->device.output_pool = NULL;
      if (space->device.input_pool)
        g_clear_object (&space->device.input_pool);
      space->device.input_pool = NULL;
      gst_sdrv_filter_sdrv_device_deinit (space);
      break;
    default:
      break;
  }

  return ret;
}

static GstBufferPool *
gst_sdrv_filter_setup_pool (GstSdrvFilterDeviceInfo * device, GstCaps * caps,
    gsize size, guint num_buf, gboolean is_map)
{
  GstBufferPool *pool = NULL;
  GstStructure *structure = NULL;
  guint buf_cnt = num_buf;

  GST_DEBUG_OBJECT (NULL, "[%s]:[%d]", __func__, __LINE__);
  pool = sdrvfilter_buffer_pool_new (is_map);

  structure = gst_buffer_pool_get_config (pool);
  /*We don't support dynamically allocating buffers, so set the max buffer
     count to be the same as the min buffer count */
  gst_buffer_pool_config_set_params (structure, caps, size, buf_cnt, buf_cnt);
  if (!gst_buffer_pool_set_config (pool, structure)) {
    GST_ERROR_OBJECT (NULL, "sdrvfilter pool set config fail");
    gst_object_unref (pool);
    return NULL;
  }

  return pool;
}

/* configure the allocation query that was answered downstream, we can
 * configure some properties on it. Only called when not in passthrough mode.
 */
static gboolean
gst_sdrv_filter_decide_allocation (GstBaseTransform * trans, GstQuery * query)
{
  GstSdrvFilter *space = NULL;
  GstBufferPool *pool = NULL;
  GstAllocator *allocator = NULL;
  guint n_allocators;
  guint n_pools;
  guint dmabuf_pool_pos = 0;
  gboolean have_dmabuf = FALSE;
  GstStructure *config = NULL;
  guint min = 0;
  guint max = 0;
  guint size = 0;
  guint i;

  space = GST_SDRV_FILTER_CAST (trans);

  n_allocators = gst_query_get_n_allocation_params (query);
  GST_DEBUG_OBJECT (space, "[%s]:[%d] n_allocators:%d", __func__, __LINE__,
      n_allocators);

  for (i = 0; i < n_allocators; i++) {
    gst_query_parse_nth_allocation_param (query, i, &allocator, NULL);

    if (!allocator)
      continue;

    if (g_strcmp0 (allocator->mem_type, GST_ALLOCATOR_DMABUF) == 0) {
      GST_DEBUG_OBJECT (space, "found a dmabuf allocator");
      dmabuf_pool_pos = i;
      have_dmabuf = TRUE;
      gst_object_unref (allocator);
      allocator = NULL;
      break;
    }

    gst_object_unref (allocator);
    allocator = NULL;
  }

  /* Delete buffer pools registered before the pool of dmabuf in
   * the buffer pool list so that the dmabuf allocator will be selected
   * by the parent class.
   */
  for (i = 0; i < dmabuf_pool_pos; i++)
    gst_query_remove_nth_allocation_param (query, i);

  n_pools = gst_query_get_n_allocation_pools (query);
  GST_DEBUG_OBJECT (space, "[%s]:[%d] n_pools:%u, dmabuf_pool_pos:%u",
      __func__, __LINE__, n_pools, dmabuf_pool_pos);
  if (n_pools > 0)
    gst_query_parse_nth_allocation_pool (query, 0, &pool, &size, &min, &max);

  GST_DEBUG_OBJECT (space, "[%s]:[%d] size:%u, min:%u, max:%u pool:%p",
      __func__, __LINE__, size, min, max, pool);

  if (space->device.io_mode == GST_SDRVFILTER_IO_AUTO && !have_dmabuf
      && !space->device.output_pool) {
    GstCaps *caps = NULL;
    GstVideoInfo vinfo;

    gst_query_parse_allocation (query, &caps, NULL);
    gst_video_info_init (&vinfo);
    gst_video_info_from_caps (&vinfo, caps);

    GST_DEBUG_OBJECT (space, "create new pool, min buffers=%d, max buffers=%d",
        min, max);
    size = MAX (vinfo.size, size);
    space->device.output_pool =
        gst_sdrv_filter_setup_pool (&space->device, caps, size, min, FALSE);
    if (!space->device.output_pool) {
      GST_ERROR_OBJECT (space, "failed to setup pool");
      return FALSE;
    }
  }

  GST_DEBUG_OBJECT (space, "set buffer pool config pool:%p output_pool:%p",
      pool, space->device.output_pool);
  if (space->device.output_pool) {
    gst_object_replace ((GstObject **) & pool,
        (GstObject *) space->device.output_pool);
    GST_DEBUG_OBJECT (space, "use our pool %p", pool);
    config = gst_buffer_pool_get_config (pool);
    gst_buffer_pool_config_get_params (config, NULL, &size, &min, &max);
    gst_structure_free (config);

    GST_DEBUG_OBJECT (space, "min:%d max:%d size:%u", min, max, size);
  }

  /* We need a bufferpool for userptr. */
  if (!pool) {
    GST_ERROR_OBJECT (space, "no pool");
    return FALSE;
  }

  if (pool != space->device.output_pool) {
    GST_DEBUG_OBJECT (space, "pool != space->device.output_pool");
    config = gst_buffer_pool_get_config (pool);
    gst_buffer_pool_config_add_option (config,
        GST_BUFFER_POOL_OPTION_VIDEO_META);
    gst_buffer_pool_set_config (pool, config);
  }

  if (n_pools > 0)
    gst_query_set_nth_allocation_pool (query, 0, pool, size, min, max);
  else
    gst_query_add_allocation_pool (query, pool, size, min, max);

  gst_object_unref (pool);

  return GST_BASE_TRANSFORM_CLASS (parent_class)
      ->decide_allocation (trans, query);
}

static GstFlowReturn
gst_sdrv_filter_cpu_crop (GstSdrvFilter * space, GstMemory * input_mem,
    GstMemory * output_mem, GstVideoInfo * input_vinfo,
    GstVideoInfo * output_vinfo, GstVideoCropMeta * crop,
    GstBuffer * output_buf)
{
  GstFlowReturn ret = GST_FLOW_OK;
  gchar *input_ptr = NULL;
  gchar *output_ptr = NULL;

  if (GST_VIDEO_FORMAT_I420 == GST_VIDEO_INFO_FORMAT (input_vinfo)
      && GST_VIDEO_FORMAT_I420 == GST_VIDEO_INFO_FORMAT (output_vinfo)) {
    GST_DEBUG_OBJECT (space, "[%s]:[%d]", __func__, __LINE__);
    gchar *input_buffer_ptr =
        (gchar *) sdrvfilter_buffer_map_fd_to_user_ptr (space->
        device.output_pool, gst_dmabuf_memory_get_fd (input_mem),
        GST_VIDEO_INFO_SIZE (input_vinfo));
    if (NULL == input_buffer_ptr) {
      GST_ERROR_OBJECT (space, "map input buffer ptr failed!");
      return GST_FLOW_ERROR;
    }

    gchar *output_buffer_ptr
        = (gchar *) sdrvfilter_buffer_get_user_ptr (output_buf);
    if (NULL == output_buffer_ptr) {
      GST_ERROR_OBJECT (space, "get output buffer ptr failed!");
      munmap (input_buffer_ptr, GST_VIDEO_INFO_SIZE (input_vinfo));
      return GST_FLOW_ERROR;
    }
#ifdef DUMP_INPUT_BUFFER
    // dump input buffer
    static int dump_input_buffer = 0;
    if (!dump_input_buffer) {
      FILE *file = fopen ("/data/input_g2d_buffer", "w+b");
      if (file) {
        fwrite (input_buffer_ptr, 1, input_vinfo->size, file);
        fclose (file);
      }
      dump_input_buffer = 1;
    }
#endif

    // copy y
    input_ptr = input_buffer_ptr;
    output_ptr = output_buffer_ptr;
    for (int h = 0; h < output_vinfo->height; h++) {
      memcpy (output_ptr + h * output_vinfo->width,
          input_ptr + h * input_vinfo->width, output_vinfo->width);
    }

    // copy u
    input_ptr = (input_buffer_ptr + input_vinfo->width * input_vinfo->height);
    output_ptr
        = (output_buffer_ptr + output_vinfo->width * output_vinfo->height);
    for (int h = 0; h < output_vinfo->height / 2; h++) {
      memcpy (output_ptr + h * output_vinfo->width / 2,
          input_ptr + h * input_vinfo->width / 2, output_vinfo->width / 2);
    }

    // copy v
    input_ptr = (input_buffer_ptr
        + input_vinfo->width * input_vinfo->height * 5 / 4);
    output_ptr = (output_buffer_ptr
        + output_vinfo->width * output_vinfo->height * 5 / 4);
    for (int h = 0; h < output_vinfo->height / 2; h++) {
      memcpy (output_ptr + h * output_vinfo->width / 2,
          input_ptr + h * input_vinfo->width / 2, output_vinfo->width / 2);
    }

    munmap ((void *) input_buffer_ptr, GST_VIDEO_INFO_SIZE (input_vinfo));
  } else {
    GST_ERROR_OBJECT (space, "TODO: support more video format crop");
    ret = GST_FLOW_ERROR;
  }

  return ret;
}

static GstFlowReturn
gst_sdrv_filter_g2d_convert (GstSdrvFilter * space, GstMemory * input_mem,
    GstMemory * output_mem, GstVideoInfo * input_vinfo,
    GstVideoInfo * output_vinfo, GstVideoCropMeta * crop)
{
  G2DInput g2d_input;
  GstFlowReturn ret = GST_FLOW_OK;

  g2d_input.src_fd = gst_dmabuf_memory_get_fd (input_mem);
  g2d_input.src_w = input_vinfo->width;
  g2d_input.src_h = input_vinfo->height;
  g2d_input.src_crop_x = 0;
  g2d_input.src_crop_y = 0;
  g2d_input.src_crop_w = input_vinfo->width;
  g2d_input.src_crop_h = input_vinfo->height;
  g2d_input.dst_fd = gst_dmabuf_memory_get_fd (output_mem);
  g2d_input.dst_w = output_vinfo->width;
  g2d_input.dst_h = output_vinfo->height;
  g2d_input.dst_crop_x = 0;
  g2d_input.dst_crop_y = 0;
  g2d_input.dst_crop_w = output_vinfo->width;
  g2d_input.dst_crop_h = output_vinfo->height;
  g2d_input.rotation = 0;
  g2d_input.src_format = GST_VIDEO_INFO_FORMAT (input_vinfo);
  g2d_input.dst_format = GST_VIDEO_INFO_FORMAT (output_vinfo);
  g2d_input.dst_addr = 0;

  if (crop && space->device.is_will_crop) {
    // add crop info
    g2d_input.src_crop_x = crop->x;
    g2d_input.src_crop_y = crop->y;
    g2d_input.src_crop_w = crop->width;
    g2d_input.src_crop_h = crop->height;
  }

  GST_DEBUG_OBJECT (space,
      "[%s]:[%d] src_fd:%d, src_w:%d, src_h:%d, src_crop_x:%d,"
      " src_crop_y:%d, src_crop_w:%d, src_crop_h:%d,"
      " dst_fd:%d, dst_w:%d, dst_h:%d, dst_crop_x:%d, dst_crop_y:%d,"
      " dst_crop_w:%d, dst_crop_h:%d rotation:%d,"
      " src_format:%d, dst_format:%d",
      __func__, __LINE__, g2d_input.src_fd, g2d_input.src_w, g2d_input.src_h,
      g2d_input.src_crop_x, g2d_input.src_crop_y, g2d_input.src_crop_w,
      g2d_input.src_crop_h, g2d_input.dst_fd, g2d_input.dst_w, g2d_input.dst_h,
      g2d_input.dst_crop_x, g2d_input.dst_crop_y, g2d_input.dst_crop_w,
      g2d_input.dst_crop_h, g2d_input.rotation, g2d_input.src_format,
      g2d_input.dst_format);
  if (g2d_convert (&g2d_input)) {
    GST_ERROR_OBJECT (space, "convert error!");
    ret = GST_FLOW_ERROR;
  }
  GST_DEBUG_OBJECT (space, "convert sucess");
  return ret;
}

static GstFlowReturn
gst_sdrv_filter_transform (GstBaseTransform * trans, GstBuffer * inbuf,
    GstBuffer * outbuf)
{
  GstBuffer *input_buf = NULL;
  GstBuffer *output_buf = NULL;
  GstBuffer *input_temp_buf = NULL;
  GstVideoInfo *input_vinfo = NULL;
  GstVideoInfo *output_vinfo = NULL;
  GstVideoFrame dest_frame;
  GstMemory *input_mem = NULL;
  GstMemory *output_mem = NULL;
  // G2DInput g2d_input;

  GstFlowReturn ret = GST_FLOW_OK;
  GstVideoFilter *filter = GST_VIDEO_FILTER_CAST (trans);
  GstSdrvFilter *space = GST_SDRV_FILTER_CAST (filter);
  GstVideoCropMeta *crop = gst_buffer_get_video_crop_meta (inbuf);

  GST_DEBUG_OBJECT (trans, "[%s]:[%d]", __func__, __LINE__);
  if (G_UNLIKELY (!filter->negotiated))
    goto unknown_format;

  input_buf = inbuf;
  output_buf = outbuf;
  input_vinfo = &filter->in_info;
  output_vinfo = &filter->out_info;

  CLEAR (dest_frame);

  input_mem = gst_buffer_peek_memory (input_buf, 0);
  output_mem = gst_buffer_peek_memory (output_buf, 0);

  if (gst_is_dmabuf_memory (input_mem) && gst_is_dmabuf_memory (output_mem)
      && space->device.is_input_dmabuffer && space->device.is_output_dmabuffer) {

#ifdef USE_CPU_CROP
    ret = gst_sdrv_filter_cpu_crop (space, input_mem, output_mem,
        input_vinfo, output_vinfo, crop, output_buf);
#else

#ifdef VIRTUAL_INPUT_BUFFER_TO_G2D
    GST_DEBUG_OBJECT (space, "[%s]:[%d]", __func__, __LINE__);
    gpointer virtual_input_ptr =
        sdrvfilter_buffer_map_fd_to_user_ptr (space->device.output_pool,
        gst_dmabuf_memory_get_fd (input_mem),
        GST_VIDEO_INFO_SIZE (input_vinfo));
    if (NULL == virtual_input_ptr) {
      GST_ERROR_OBJECT (space, "map input buffer ptr failed!");
      return GST_FLOW_ERROR;
    }
#ifdef DUMP_INPUT_BUFFER
    // dump input buffer
    static int dump_input_buffer = 0;
    if (!dump_input_buffer) {
      FILE *file = fopen ("/data/input_g2d_buffer_", "w+b");
      if (file) {
        fwrite (virtual_input_ptr, 1, input_vinfo->size, file);
        fclose (file);
      }
      dump_input_buffer = 1;
    }
#endif
    memset (virtual_input_ptr, 0xDD, GST_VIDEO_INFO_SIZE (input_vinfo));
    munmap (virtual_input_ptr, GST_VIDEO_INFO_SIZE (input_vinfo));
#endif

    ret = gst_sdrv_filter_g2d_convert (space, input_mem, output_mem,
        input_vinfo, output_vinfo, crop);

#ifdef VIRTUAL_OUTPUT_BUFFER_TO_G2D
    gpointer virtual_output_ptr = sdrvfilter_buffer_get_user_ptr (output_buf);
    if (NULL == virtual_output_ptr) {
      GST_ERROR_OBJECT (space, "map input buffer ptr failed!");
      return GST_FLOW_ERROR;
    }
    GST_DEBUG_OBJECT (space, "[%s]:[%d] virtual_output_ptr:%p buffer size:%d",
        __func__, __LINE__, virtual_output_ptr,
        GST_VIDEO_INFO_SIZE (output_vinfo));
    memset (virtual_output_ptr, 0x01, GST_VIDEO_INFO_SIZE (output_vinfo));
#endif

#endif

#ifdef DUMP_OUTPUT_BUFFER
    // dump output buffer
    static int dump_output_buffer = 0;
    if (!dump_output_buffer) {
      gpointer output_buffer_ptr = sdrvfilter_buffer_get_user_ptr (output_buf);
      if (NULL == output_buffer_ptr)
        GST_ERROR_OBJECT (space, "get output buffer ptr failed!");

      FILE *file = fopen ("/data/output_g2d_buffer", "w+b");
      if (file) {
        fwrite (output_buffer_ptr, 1, output_vinfo->size, file);
        fclose (file);
      }
      dump_output_buffer = 1;
    }
#endif
  } else if (gst_is_dmabuf_memory (output_mem)
      && space->device.is_output_dmabuffer && space->device.input_pool) {
    GstMapInfo map = GST_MAP_INFO_INIT;
    gpointer user_ptr = NULL;

    if (!gst_buffer_map (input_buf, &map, GST_MAP_READ)) {
      GST_ERROR_OBJECT (space, "Failed to map input buffer");
      return GST_FLOW_ERROR;
    }

    ret = gst_buffer_pool_acquire_buffer (space->device.input_pool,
        &input_temp_buf, NULL);
    if (ret != GST_FLOW_OK) {
      GST_DEBUG_OBJECT (space, "could not allocate buffer from pool");
      return ret;
    }

    input_mem = gst_buffer_peek_memory (input_temp_buf, 0);
    user_ptr = sdrvfilter_buffer_get_user_ptr (input_temp_buf);

    if (!gst_is_dmabuf_memory (input_mem) || !user_ptr) {
      GST_DEBUG_OBJECT (space,
          "could not get dma buffer user ptr, "
          "input_temp_buf:%p user_ptr:%p", input_temp_buf, user_ptr);
      gst_buffer_unmap (input_buf, &map);
      gst_buffer_unref (input_temp_buf);
      return GST_FLOW_ERROR;
    }

    GST_DEBUG_OBJECT (space, "input_temp_buf:%p user_ptr:%p", input_temp_buf,
        user_ptr);

    // Add virtual data buffer
    // write BRG data to buffer
    // {
    //   static int i = 0;
    //   int r , g, b;
    //   int offset = 0;
    //   if (i % 30 > 20)
    //   {
    //     r = 180;
    //     g = 0;
    //     b = 0;
    //   }
    //   else if (i % 30 > 10)
    //   {
    //     r = 0;
    //     g = 180;
    //     b = 0;
    //   }
    //   else
    //   {
    //     r = 0;
    //     g = 0;
    //     b = 180;
    //   }

    //   i++;

    //   for (int i = 0; i < input_vinfo->height / 3; i++)
    //   {
    //     for (int j = 0; j < input_vinfo->width * 3; j = j + 3)
    //     {
    //       *((unsigned char *)user_ptr + i * input_vinfo->width * 3 + j) =
    //       r;
    //       *((unsigned char *)user_ptr + i * input_vinfo->width * 3 + j +
    //       1) = g;
    //       *((unsigned char *)user_ptr + i * input_vinfo->width * 3 + j +
    //       2) = b;
    //     }
    //   }
    //   offset = input_vinfo->height / 3 * input_vinfo->width * 3;
    //   for (int i = 0; i < input_vinfo->height / 3; i++)
    //   {
    //     for (int j = 0; j < input_vinfo->width * 3; j = j + 3)
    //     {
    //       *((unsigned char *)user_ptr + offset + i * input_vinfo->width *
    //       3 + j) = b;
    //       *((unsigned char *)user_ptr + offset + i * input_vinfo->width *
    //       3 + j + 1) = r;
    //       *((unsigned char *)user_ptr + offset + i * input_vinfo->width *
    //       3 + j + 2) = g;
    //     }
    //   }
    //   offset = input_vinfo->height / 3 * 2 * input_vinfo->width * 3;
    //   for (int i = 0; i < input_vinfo->height / 3; i++)
    //   {
    //     for (int j = 0; j < input_vinfo->width * 3; j = j + 3)
    //     {
    //       *((unsigned char *)user_ptr + offset + i * input_vinfo->width *
    //       3 + j) = g;
    //       *((unsigned char *)user_ptr + offset + i * input_vinfo->width *
    //       3 + j + 1) = b;
    //       *((unsigned char *)user_ptr + offset + i * input_vinfo->width *
    //       3 + j + 2) = r;
    //     }
    //   }
    //   unsigned char *print_ptr = (unsigned char *)user_ptr;
    //   GST_INFO_OBJECT(space, "input_vinfo->height:%d,
    //   input_vinfo->width:%d "
    //     "user_ptr[0]:%x, user_ptr[1]:%x, user_ptr[2]:%x, user_ptr[3]:%x",
    //     input_vinfo->height, input_vinfo->width,
    //     print_ptr[0], print_ptr[1], print_ptr[2], print_ptr[3]);
    // }

    memcpy (user_ptr, map.data, input_vinfo->size);
    gst_buffer_unmap (input_buf, &map);

#ifdef DUMP_INPUT_BUFFER
    // dump input buffer
    static int dump_input_buffer = 0;
    if (!dump_input_buffer) {
      FILE *file = fopen ("/data/input_g2d_buffer", "w+b");
      if (file) {
        fwrite (user_ptr, 1, input_vinfo->size, file);
        fclose (file);
      }
      dump_input_buffer = 1;
    }
#endif

    ret = gst_sdrv_filter_g2d_convert (space, input_mem, output_mem,
        input_vinfo, output_vinfo, crop);
    gst_buffer_unref (input_temp_buf);

#ifdef DUMP_OUTPUT_BUFFER
    // dump output buffer
    static int dump_output_buffer = 0;
    if (!dump_output_buffer) {
      gpointer output_buffer_ptr = sdrvfilter_buffer_get_user_ptr (output_buf);
      if (NULL == output_buffer_ptr)
        GST_ERROR_OBJECT (space, "get output buffer ptr failed!");

      FILE *file = fopen ("/data/output_g2d_buffer", "w+b");
      if (file) {
        fwrite (output_buffer_ptr, 1, output_vinfo->size, file);
        fclose (file);
      }
      dump_output_buffer = 1;
    }
#endif
  } else {
    GST_ERROR_OBJECT (space,
        "[%s]:[%d] TODO: support usrptr buffer,"
        "is_input_dmabuffer:%d, is_output_dmabuffer:%d,"
        "input is dmabuf:%d, output is dmabuf:%d",
        __func__, __LINE__, space->device.is_input_dmabuffer,
        space->device.is_output_dmabuffer,
        gst_is_dmabuf_memory (input_mem), gst_is_dmabuf_memory (output_mem));
    ret = GST_FLOW_ERROR;
  }

  GST_DEBUG_OBJECT (trans, "[%s]:[%d]", __func__, __LINE__);
  return ret;

  /* ERRORS */
unknown_format:
  {
    GST_ELEMENT_ERROR (space, CORE, NOT_IMPLEMENTED, (NULL),
        ("unknown format"));
    return GST_FLOW_NOT_NEGOTIATED;
  }
}

static gboolean
gst_sdrv_filter_set_caps (GstBaseTransform * trans, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstVideoFilter *filter = GST_VIDEO_FILTER_CAST (trans);
  GstVideoFilterClass *fclass = NULL;
  GstSdrvFilter *space = NULL;
  GstBufferPool *newpool = NULL;
  GstCaps *input_caps = NULL, *output_caps = NULL;
  GstVideoInfo input_vinfo, output_vinfo;

  space = GST_SDRV_FILTER_CAST (filter);
  fclass = GST_VIDEO_FILTER_GET_CLASS (filter);

  input_caps = incaps;
  output_caps = outcaps;
  GST_DEBUG_OBJECT (space, "incaps %" GST_PTR_FORMAT, input_caps);
  GST_DEBUG_OBJECT (space, "outcaps %" GST_PTR_FORMAT, output_caps);

  space->device.is_input_dmabuffer =
      gst_caps_features_contains (gst_caps_get_features (incaps, 0),
      GST_CAPS_FEATURE_MEMORY_DMABUF);
  space->device.is_output_dmabuffer =
      gst_caps_features_contains (gst_caps_get_features (outcaps, 0),
      GST_CAPS_FEATURE_MEMORY_DMABUF);

  if (!gst_video_info_from_caps (&input_vinfo, incaps))
    goto invalid_caps;

  if (!gst_video_info_from_caps (&output_vinfo, outcaps))
    goto invalid_caps;

  // output pool:
  if (space->device.output_pool) {
    gst_buffer_pool_set_active (space->device.output_pool, FALSE);
  }
#if defined(DUMP_OUTPUT_BUFFER) || defined(USE_CPU_CROP)                      \
    || defined(VIRTUAL_OUTPUT_BUFFER_TO_G2D)
  newpool
      = gst_sdrv_filter_setup_pool (&space->device, outcaps, output_vinfo.size,
      DEFAULT_BUFFER_POOL_NUMBER, TRUE);
#else
  newpool
      = gst_sdrv_filter_setup_pool (&space->device, outcaps, output_vinfo.size,
      DEFAULT_BUFFER_POOL_NUMBER, FALSE);
#endif

  if (!newpool)
    goto pool_setup_failed;

  gst_object_replace ((GstObject **) & space->device.output_pool,
      (GstObject *) newpool);
  gst_object_unref (newpool);
  newpool = NULL;

  // input pool:
  if (!space->device.is_input_dmabuffer)        // allocate input dmabuffer pool for temp
  {
    if (space->device.input_pool)
      gst_buffer_pool_set_active (space->device.input_pool, FALSE);

    newpool =
        gst_sdrv_filter_setup_pool (&space->device, incaps, input_vinfo.size,
        DEFAULT_INPUT_TEMP_DMABUFFER_NUMBER, TRUE);
    if (!newpool)
      goto pool_setup_failed;

    gst_object_replace ((GstObject **) & space->device.input_pool,
        (GstObject *) newpool);
    gst_object_unref (newpool);
    newpool = NULL;

    gst_buffer_pool_set_active (space->device.input_pool, TRUE);
  }

  GST_DEBUG ("reconfigured %d %d", GST_VIDEO_INFO_FORMAT (&output_vinfo),
      GST_VIDEO_INFO_FORMAT (&input_vinfo));
  GST_DEBUG ("size: %lu %lu", output_vinfo.size, input_vinfo.size);

  /* these must match */
  if (input_vinfo.fps_n != output_vinfo.fps_n
      || input_vinfo.fps_d != output_vinfo.fps_d)
    goto format_mismatch;

  /* if present, these must match too */
  if (input_vinfo.interlace_mode != output_vinfo.interlace_mode)
    goto format_mismatch;

  filter->in_info = input_vinfo;
  filter->out_info = output_vinfo;
  GST_BASE_TRANSFORM_CLASS (fclass)->transform_ip_on_passthrough = FALSE;
  filter->negotiated = TRUE;

  return TRUE;

  /* ERRORS */
pool_setup_failed:
  {
    GST_ERROR_OBJECT (space, "failed to setup pool");
    filter->negotiated = FALSE;
    return FALSE;
  }
invalid_caps:
  {
    GST_ERROR_OBJECT (space, "invalid caps");
    filter->negotiated = FALSE;
    return FALSE;
  }
format_mismatch:
  {
    GST_ERROR_OBJECT (space,
        "input and output formats do not match,"
        " in_fps_n:%d, out_fps_n:%d, in_fps_d:%d, out_fps_d:%d,"
        " in_interlace_mode:%d, out_interlace_mode:%d",
        input_vinfo.fps_n, output_vinfo.fps_n, input_vinfo.fps_d,
        output_vinfo.fps_d, input_vinfo.interlace_mode,
        output_vinfo.interlace_mode);
    filter->negotiated = FALSE;
    return FALSE;
  }
}

static gboolean
gst_sdrv_filter_propose_allocation (GstBaseTransform * trans,
    GstQuery * decide_query, GstQuery * query)
{
  GstSdrvFilter *space = NULL;

  space = GST_SDRV_FILTER_CAST (trans);
  GST_DEBUG_OBJECT (space, "[%s]:[%d]", __func__, __LINE__);
  if (!GST_BASE_TRANSFORM_CLASS (parent_class)
      ->propose_allocation (trans, decide_query, query))
    return FALSE;

  /* passthrough, we're done */
  if (decide_query == NULL) {
    GST_DEBUG_OBJECT (space, "we are in passthrough mode, do nothing");
    return TRUE;
  }

  gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);
  gst_query_add_allocation_meta (query, GST_VIDEO_CROP_META_API_TYPE, NULL);

  return TRUE;
}

static gboolean
gst_sdrv_filter_stop (GstBaseTransform * trans)
{
  gboolean ret = TRUE;
  GstSdrvFilter *space = GST_SDRV_FILTER_CAST (trans);

  GST_DEBUG_OBJECT (space, "[%s]:[%d]", __func__, __LINE__);
  if (space->device.input_pool)
    ret = gst_buffer_pool_set_active (space->device.input_pool, FALSE);
  return ret;
}

static void
gst_sdrv_filter_class_init (GstSdrvFilterClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstElementClass *gstelement_class = (GstElementClass *) klass;
  GstBaseTransformClass *gstbasetransform_class
      = (GstBaseTransformClass *) klass;

  gobject_class->set_property = gst_sdrv_filter_set_property;
  gobject_class->get_property = gst_sdrv_filter_get_property;
  gobject_class->finalize = gst_sdrv_filter_finalize;

  g_object_class_install_property (gobject_class, PROP_INPUT_IO_MODE,
      g_param_spec_enum ("input-io-mode", "Input IO mode", "Input I/O mode",
          GST_TYPE_SDRVFILTER_IO_MODE, DEFAULT_PROP_IO_MODE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_OUTPUT_IO_MODE,
      g_param_spec_enum ("output-io-mode", "Output IO mode", "Output I/O mode",
          GST_TYPE_SDRVFILTER_IO_MODE, DEFAULT_PROP_IO_MODE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&gst_sdrv_filter_src_template));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&gst_sdrv_filter_sink_template));

  gst_element_class_set_static_metadata (gstelement_class,
      "Colorspace and Video Size Converter plugin", "Filter/Converter/Video",
      "Converts colorspace and video size from one to another",
      "semidrive technology ltd");

  gstelement_class->change_state = gst_sdrv_filter_change_state;

  gstbasetransform_class->transform_caps
      = GST_DEBUG_FUNCPTR (gst_sdrv_filter_transform_caps);
  gstbasetransform_class->fixate_caps
      = GST_DEBUG_FUNCPTR (gst_sdrv_filter_fixate_caps);
  gstbasetransform_class->filter_meta
      = GST_DEBUG_FUNCPTR (gst_sdrv_filter_filter_meta);
  gstbasetransform_class->transform_meta
      = GST_DEBUG_FUNCPTR (gst_sdrv_filter_transform_meta);
  gstbasetransform_class->decide_allocation
      = GST_DEBUG_FUNCPTR (gst_sdrv_filter_decide_allocation);
  gstbasetransform_class->propose_allocation
      = GST_DEBUG_FUNCPTR (gst_sdrv_filter_propose_allocation);
  gstbasetransform_class->transform
      = GST_DEBUG_FUNCPTR (gst_sdrv_filter_transform);
  gstbasetransform_class->set_caps
      = GST_DEBUG_FUNCPTR (gst_sdrv_filter_set_caps);
  gstbasetransform_class->stop = GST_DEBUG_FUNCPTR (gst_sdrv_filter_stop);

  GST_DEBUG ("sdrvfilter: [%s]:[%d]", __func__, __LINE__);
}

static void
gst_sdrv_filter_init (GstSdrvFilter * space)
{
}

static void
gst_sdrv_filter_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstSdrvFilter *space = GST_SDRV_FILTER (object);

  switch (property_id) {
    case PROP_INPUT_IO_MODE:
      space->device.io_mode = g_value_get_enum (value);
      break;
    case PROP_OUTPUT_IO_MODE:
      space->device.io_mode = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
gst_sdrv_filter_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstSdrvFilter *space = GST_SDRV_FILTER (object);

  switch (property_id) {
    case PROP_INPUT_IO_MODE:
      g_value_set_enum (value, space->device.io_mode);
      break;
    case PROP_OUTPUT_IO_MODE:
      g_value_set_enum (value, space->device.io_mode);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (sdrvfilter_debug, "sdrvfilter", 0,
      "Colorspace and Video Size Converter");

  return gst_element_register (plugin, "sdrvfilter", GST_RANK_NONE,
      GST_TYPE_SDRV_FILTER);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, sdrvfilter,
    "Colorspace conversion and Video scaling plugin",
    plugin_init, VERSION, GST_LICENSE, PACKAGE_NAME, GST_PACKAGE_ORIGIN)
