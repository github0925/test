#include "gstg2dapi.h"
#include "sdrv_g2d_cfg.h"
#include <drm/drm_fourcc.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define UNUSED(x) (void)x
#define G2D_DEVICE_NAME "/dev/g2d0"
#define HAL_PIXEL_FORMAT_VENDOR_EXT(fmt) (0x100 | (fmt & 0xFF))
#define HAL_PIXEL_FORMAT_NV12 HAL_PIXEL_FORMAT_VENDOR_EXT (6)

typedef enum
{
  HAL_PIXEL_FORMAT_RGBA_8888 = 1,
  HAL_PIXEL_FORMAT_RGBX_8888 = 2,
  HAL_PIXEL_FORMAT_RGB_888 = 3,
  HAL_PIXEL_FORMAT_RGB_565 = 4,
  HAL_PIXEL_FORMAT_BGRA_8888 = 5,
  HAL_PIXEL_FORMAT_YCBCR_422_SP = 16,
  HAL_PIXEL_FORMAT_YCRCB_420_SP = 17,
  HAL_PIXEL_FORMAT_YCBCR_422_I = 20,
  HAL_PIXEL_FORMAT_RGBA_FP16 = 22,
  HAL_PIXEL_FORMAT_RAW16 = 32,
  HAL_PIXEL_FORMAT_BLOB = 33,
  HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED = 34,
  HAL_PIXEL_FORMAT_YCBCR_420_888 = 35,
  HAL_PIXEL_FORMAT_RAW_OPAQUE = 36,
  HAL_PIXEL_FORMAT_RAW10 = 37,
  HAL_PIXEL_FORMAT_RAW12 = 38,
  HAL_PIXEL_FORMAT_RGBA_1010102 = 43,
  HAL_PIXEL_FORMAT_Y8 = 538982489,
  HAL_PIXEL_FORMAT_Y16 = 540422489,
  HAL_PIXEL_FORMAT_YV12 = 842094169,
} android_pixel_format_t;

typedef struct _G2D_Context
{
  int g2dFd;
  int refCnt;
  GstObject *object;
  struct g2d_input *input;
} G2D_Context;

G2D_Context *gG2dContext = NULL;
pthread_mutex_t gG2dMutex = PTHREAD_MUTEX_INITIALIZER;

int
g2d_open (GstObject * object)
{
  struct g2d_capability cap;
  int ret;

  if (NULL == object) {
    GST_ERROR_OBJECT (NULL, "g2d_open func, object is nullptr");
  }

  pthread_mutex_lock (&gG2dMutex);
  if ((gG2dContext != NULL) && (gG2dContext->g2dFd > 0)) {
    GST_DEBUG_OBJECT (object, "g2d_open gG2dContext has been inited");
    goto SUCCEED_OUT;
  }
  gG2dContext = (G2D_Context *) malloc (sizeof (G2D_Context));
  if (gG2dContext == NULL) {
    GST_ERROR_OBJECT (object, "g2d_open malloc failed");
    goto ERROR_OUT;
  }

  memset ((void *) gG2dContext, 0, sizeof (G2D_Context));
  gG2dContext->g2dFd = open (G2D_DEVICE_NAME, O_RDWR);

  if (gG2dContext->g2dFd < 0) {
    GST_ERROR_OBJECT (object, "g2d_open ion_open failed");
    goto ERROR_OUT;
  }
  /* 初始化设备 */
  memset ((void *) &cap, 0, sizeof (struct g2d_capability));
  ret = ioctl (gG2dContext->g2dFd, G2D_IOCTL_GET_CAPABILITIES, &cap);
  if (ret) {
    GST_ERROR_OBJECT (object, "g2d_open G2D_IOCTL_GET_CAPABILITIES failed");
    goto ERROR_OUT;
  }

  gG2dContext->input = (struct g2d_input *) malloc (sizeof (struct g2d_input));
  if (NULL == gG2dContext->input) {
    GST_ERROR_OBJECT (object, "g2d_open gG2dContext->input malloc failedd");
    goto ERROR_OUT;
  }
SUCCEED_OUT:
  gG2dContext->object = object;
  gG2dContext->refCnt++;
  GST_DEBUG_OBJECT (gG2dContext->object, "g2d_open succesfully refCnt:%d",
      gG2dContext->refCnt);
  pthread_mutex_unlock (&gG2dMutex);
  return 0;
ERROR_OUT:
  if (gG2dContext != NULL && gG2dContext->g2dFd > 0) {
    close (gG2dContext->g2dFd);
    gG2dContext->g2dFd = 0;
  }

  if (gG2dContext != NULL) {
    free (gG2dContext);
    gG2dContext = NULL;
  }
  GST_ERROR_OBJECT (gG2dContext->object, "g2d_open failed!!!!");
  pthread_mutex_unlock (&gG2dMutex);
  return -1;
}

int
g2d_close ()
{
  GST_DEBUG_OBJECT (gG2dContext->object, "g2d_close enter");
  pthread_mutex_lock (&gG2dMutex);
  if (gG2dContext != NULL) {
    GST_DEBUG_OBJECT (gG2dContext->object,
        "g2d_close gG2dContext->refCnt:%d", gG2dContext->refCnt);
    gG2dContext->refCnt--;
    if (gG2dContext->refCnt <= 0) {
      if (gG2dContext->g2dFd > 0) {
        close (gG2dContext->g2dFd);
        GST_DEBUG_OBJECT (gG2dContext->object, "g2d_close finish!!!");
      }
      gG2dContext->g2dFd = 0;

      if (gG2dContext->input) {
        free (gG2dContext->input);
        gG2dContext->input = NULL;
      }

      free (gG2dContext);
      gG2dContext = NULL;
    }
  }
  pthread_mutex_unlock (&gG2dMutex);
  return 0;
}

int
g2d_get_pitch (int format, int width)
{
  int pitch = width;
  switch (format) {
    case DRM_FORMAT_BGRA8888:
    case DRM_FORMAT_RGBA8888:
      pitch = width * 4;
      break;
    case DRM_FORMAT_BGR888:
    case DRM_FORMAT_RGB888:
      pitch = width * 3;
      break;
    case DRM_FORMAT_NV21:
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_YUV420:
      pitch = width;
      break;
    case DRM_FORMAT_YUYV:
      pitch = width * 2;
      break;
    default:
      GST_ERROR_OBJECT (gG2dContext->object,
          "g2d_get_pitch_format do not support format:%d", format);
      break;
  }
  return pitch;
}

/*
 *convert format frome android to drm format
 */
int
g2d_convert_format (int format)
{
  int outFormat = DRM_FORMAT_NV12;
  switch (format) {
    case GST_VIDEO_FORMAT_BGRA:
      GST_DEBUG_OBJECT (gG2dContext->object,
          "g2d_convert_format outFormat:DRM_FORMAT_BGRA8888");
      outFormat = DRM_FORMAT_BGRA8888;
    case GST_VIDEO_FORMAT_RGBA:
      GST_DEBUG_OBJECT (gG2dContext->object,
          "g2d_convert_format :DRM_FORMAT_RGBA8888");
      outFormat = DRM_FORMAT_RGBA8888;
      break;
    case GST_VIDEO_FORMAT_NV12:
      GST_DEBUG_OBJECT (gG2dContext->object,
          "g2d_convert_format outFormat:DRM_FORMAT_NV12");
      outFormat = DRM_FORMAT_NV12;
      break;
    case GST_VIDEO_FORMAT_I420:
      GST_DEBUG_OBJECT (gG2dContext->object,
          "g2d_convert_format :DRM_FORMAT_YUV420");
      outFormat = DRM_FORMAT_YUV420;
      break;
    case GST_VIDEO_FORMAT_NV21:
      GST_DEBUG_OBJECT (gG2dContext->object,
          "g2d_convert_format outFormat:DRM_FORMAT_NV21");
      outFormat = DRM_FORMAT_NV21;
      break;
    case GST_VIDEO_FORMAT_YUY2:
      GST_DEBUG_OBJECT (gG2dContext->object,
          "g2d_convert_format outFormat:DRM_FORMAT_YUYV");
      outFormat = DRM_FORMAT_YUYV;
      break;
    case GST_VIDEO_FORMAT_BGR:
      GST_DEBUG_OBJECT (gG2dContext->object,
          "g2d_convert_format outFormat:DRM_FORMAT_BGR888");
      outFormat = DRM_FORMAT_BGR888;
      break;
    default:
      GST_ERROR_OBJECT (gG2dContext->object,
          "g2d_convert_format do not support format:%d", format);
      break;
  }
  return outFormat;
}

int
g2d_get_format_nplanes (int format)
{
  switch (format) {
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
      return 2;
    case DRM_FORMAT_YVU420:
    case DRM_FORMAT_YUV420:
      return 3;
    default:
      return 1;
  }
  return 1;
}

int
g2d_convert (G2DInput * pInput)
{
  int ret;
  if (!gG2dContext || !gG2dContext->input) {
    GST_ERROR_OBJECT (NULL, "gG2dContext is NULL ptr, please init it");
    return -1;
  }
  struct g2d_input *input = gG2dContext->input;
  pthread_mutex_lock (&gG2dMutex);
  memset (input, 0x0, sizeof (struct g2d_input));
  input->layer_num = 1;
  struct dpc_layer *layer = &(input->layer[0]);
  layer->index = 2;
  layer->enable = 1;
  layer->format = g2d_convert_format (pInput->src_format);
  layer->nplanes = g2d_get_format_nplanes (layer->format);
  layer->alpha = 0xff;
  layer->blend_mode = BLEND_PIXEL_NONE;
  // layer->rotation = pInput->rotation;
  layer->zpos = 0;
  layer->xfbc = 0;
  layer->modifier = 0;
  layer->width = pInput->src_w;
  layer->height = pInput->src_h;
  layer->src_x = pInput->src_crop_x;
  layer->src_y = pInput->src_crop_y;
  layer->src_w = pInput->src_crop_w;
  layer->src_h = pInput->src_crop_h;

  layer->dst_x = pInput->dst_crop_x;
  layer->dst_y = pInput->dst_crop_y;
  layer->dst_w = pInput->dst_crop_w;
  layer->dst_h = pInput->dst_crop_h;
  layer->bufs[0].fd = pInput->src_fd;
  switch (layer->nplanes) {
    case 3:
      layer->pitch[0] = layer->width;
      layer->offsets[0] = 0;
      layer->pitch[1] = layer->width / 2;
      layer->offsets[1] = layer->width * layer->height;
      layer->pitch[2] = layer->width / 2;
      layer->offsets[2] = layer->width * layer->height * 5 / 4;
      break;
    case 2:
      layer->pitch[0] = layer->width;
      layer->offsets[0] = 0;
      layer->pitch[1] = layer->width;
      layer->offsets[1] = layer->width * layer->height;
      break;
    case 1:
      layer->pitch[0] = g2d_get_pitch (layer->format, layer->width);
      break;
    default:
      GST_ERROR_OBJECT (gG2dContext->object,
          "layer->nplanes:%d is error value", layer->nplanes);
      break;
  }

  input->output.width = pInput->dst_w;
  input->output.height = pInput->dst_h;
  input->output.fmt = g2d_convert_format (pInput->dst_format);
  // input->output.addr[0] = pInput->dst_addr;
  input->output.bufs[0].fd = pInput->dst_fd;
  input->output.nplanes = g2d_get_format_nplanes (input->output.fmt);

  switch (input->output.nplanes) {
    case 3:
      input->output.stride[0] = input->output.width;
      input->output.offsets[0] = 0;
      input->output.stride[1] = input->output.width / 2;
      input->output.offsets[1] = input->output.width * input->output.height;
      input->output.stride[2] = input->output.width / 2;
      input->output.offsets[2]
          = input->output.width * input->output.height * 5 / 4;
      break;
    case 2:
      input->output.stride[0] = input->output.width;
      input->output.offsets[0] = 0;
      input->output.stride[1] = input->output.width;
      input->output.offsets[1] = input->output.width * input->output.height;
      break;
    case 1:
      input->output.stride[0]
          = g2d_get_pitch (input->output.fmt, input->output.width);
      break;
    default:
      GST_ERROR_OBJECT (gG2dContext->object, "input->output:%d is error value",
          input->output.nplanes);
      break;
  }
  input->output.rotation = pInput->rotation;
#if 0
  FILE *saveinput;
  saveinput = fopen ("/data/input1", "w");
  if (saveinput) {
    fwrite (&input, 1, sizeof (struct g2d_input), saveinput);
    fclose (saveinput);
  }
#endif
  ret = ioctl (gG2dContext->g2dFd, G2D_IOCTL_POST_CONFIG, input);
  if (ret) {
    GST_ERROR_OBJECT (gG2dContext->object,
        "ioctl G2D_IOCTL_POST_CONFIG: ERROR");
    pthread_mutex_unlock (&gG2dMutex);
    return ret;
  }
  GST_DEBUG_OBJECT (gG2dContext->object, "ioctl sucess");
  pthread_mutex_unlock (&gG2dMutex);
  return 0;
}

int
g2d_format_convert (int src_format, int dst_format, int src_fd, int dst_fd,
    int width, int height)
{
  G2DInput g2dinput;

  g2dinput.src_fd = src_fd;
  g2dinput.src_w = width;
  g2dinput.src_h = height;
  g2dinput.src_crop_x = 0;
  g2dinput.src_crop_y = 0;
  g2dinput.src_crop_w = width;
  g2dinput.src_crop_h = height;
  g2dinput.dst_fd = dst_fd;
  g2dinput.dst_w = width;
  g2dinput.dst_h = height;
  g2dinput.dst_crop_x = 0;
  g2dinput.dst_crop_y = 0;
  g2dinput.dst_crop_w = width;
  g2dinput.dst_crop_h = height;
  g2dinput.rotation = 0;
  g2dinput.src_format = src_format;
  g2dinput.dst_format = dst_format;

  return g2d_convert (&g2dinput);
}

int
g2d_format_scale (int src_format, int dst_format, int src_fd, int dst_fd,
    int src_w, int src_h, int dst_w, int dst_h)
{
  G2DInput g2dinput;

  GST_DEBUG_OBJECT (gG2dContext->object,
      "srcfd:%d,dstfd:%d,src_w:%d,src_h:%d,dst_w:%d,dst_h:%d",
      src_fd, dst_fd, src_w, src_h, dst_w, dst_h);
  g2dinput.src_fd = src_fd;
  g2dinput.src_w = src_w;
  g2dinput.src_h = src_h;
  g2dinput.src_crop_x = 0;
  g2dinput.src_crop_y = 0;
  g2dinput.src_crop_w = src_w;
  g2dinput.src_crop_h = src_h;
  g2dinput.dst_fd = dst_fd;
  g2dinput.dst_w = dst_w;
  g2dinput.dst_h = dst_h;
  g2dinput.dst_crop_x = 0;
  g2dinput.dst_crop_y = 0;
  g2dinput.dst_crop_w = dst_w;
  g2dinput.dst_crop_h = dst_h;
  g2dinput.rotation = 0;
  g2dinput.src_format = src_format;
  g2dinput.dst_format = dst_format;

  return g2d_convert (&g2dinput);
}

g2d_dev_ops G2dDevOps = {
  .g2dOpen = g2d_open,
  .g2dClose = g2d_close,
  .g2dConvert = g2d_convert,
};

g2d_dev_ops *
GetG2dDevOps ()
{
  return &G2dDevOps;
}
