/* GStreamer
 *
 * Copyright (C) 2016 Igalia
 *
 * Authors:
 *  Víctor Manuel Jáquez Leal <vjaquez@igalia.com>
 *  Javier Martin <javiermartin@by.com.es>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstdrmutils.h"
#include <drm_fourcc.h>

/**********************************************************/
#define ALIGN(x, mask) (((x) + ((mask)-1)) & ~((mask)-1))
#define DMA_NON_CONSISTENT (0x1 << 3)
#define DMA_WRITE_COMBINE (0x1 << 2)

typedef struct vendor_info
{
  struct dm_drm_drv_t base;

  int fd;
  int gen;

  uint32_t *batch, *cur;
  int capacity, size;
  int exec_blt;
} vendor_info;

static void
vendor_destroy (struct dm_drm_drv_t *drv)
{
  struct vendor_info *info = (struct vendor_info *) drv;
  GST_DEBUG_OBJECT (NULL, "destroy drm info.");

  free (info);
}

static void
create_arg_init (int format, int width, int height,
    struct drm_mode_create_dumb *create_arg)
{
  switch (format) {
    case DRM_FORMAT_RGBA8888:
    case DRM_FORMAT_BGRA8888:
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_XRGB8888:
    case DRM_FORMAT_XBGR8888:
    case DRM_FORMAT_BGRX8888:
    case DRM_FORMAT_XRGB2101010:
    case DRM_FORMAT_ARGB2101010:
    case DRM_FORMAT_AYUV:
    case DRM_FORMAT_XBGR2101010:
    case DRM_FORMAT_BGRX1010102:
    case DRM_FORMAT_ABGR2101010:
    case DRM_FORMAT_BGRA1010102:
      create_arg->bpp = 32;
      create_arg->width = width;
      create_arg->height = height;
      break;

    case DRM_FORMAT_YUV444:
    case DRM_FORMAT_RGB888:
    case DRM_FORMAT_BGR888:
      create_arg->bpp = 24;
      create_arg->width = width;
      create_arg->height = height;
      break;

    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_BGR565:
    case DRM_FORMAT_XRGB4444:
    case DRM_FORMAT_ARGB4444:
      create_arg->bpp = 16;
      create_arg->width = width;
      create_arg->height = height;
      break;

    case DRM_FORMAT_YVU420:
    case DRM_FORMAT_YUV420:
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
      create_arg->bpp = 8;
      create_arg->width = 8;
      create_arg->height = (height * width * 3) / 16;
      break;

    case DRM_FORMAT_YUYV:
    case DRM_FORMAT_UYVY:
    case DRM_FORMAT_VYUY:
      create_arg->bpp = 8;
      create_arg->width = ALIGN (width * 2, 16);
      create_arg->height = height;
      break;

    case DRM_FORMAT_YUV422:
    case DRM_FORMAT_NV16:
      create_arg->bpp = 8;
      create_arg->width = 8;
      create_arg->height = (height * width * 4) / 16;
      break;

    case DRM_FORMAT_R8:
      create_arg->bpp = 8;
      create_arg->width = width;
      create_arg->height = height;
      break;
    default:
      GST_ERROR_OBJECT (NULL, "Invalid format :%x", format);
      break;
  }
}

static void
img_buf_info_init (const struct drm_mode_create_dumb create_arg,
    const struct drm_prime_handle prime_arg, const int width,
    const int format, struct dm_drm_handle_t *handle)
{

  switch (format) {
    case DRM_FORMAT_VYUY:
    case DRM_FORMAT_YUYV:
    case DRM_FORMAT_UYVY:
      handle->stride[0] = create_arg.width;
      handle->buf_fd[0] = prime_arg.fd;
      handle->offset[0] = 0;
      break;
    case DRM_FORMAT_YUV422:
      handle->buf_fd[0] = prime_arg.fd;
      handle->buf_fd[1] = prime_arg.fd;
      handle->buf_fd[2] = prime_arg.fd;
      handle->stride[0] = width;
      handle->offset[0] = 0;

      handle->stride[1] = handle->stride[0] / 2;
      handle->offset[1] = handle->offset[0] + create_arg.size / 2;

      handle->stride[2] = handle->stride[0] / 2;
      handle->offset[2] = handle->offset[1] + create_arg.size / 4;
      break;
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_NV21:
      handle->buf_fd[0] = prime_arg.fd;
      handle->buf_fd[1] = prime_arg.fd;

      handle->stride[0] = width;
      handle->offset[0] = 0;

      handle->stride[1] = handle->stride[0];
      handle->offset[1] = handle->offset[0] + create_arg.size * 2 / 3;
      break;
    case DRM_FORMAT_NV16:
      handle->buf_fd[0] = prime_arg.fd;
      handle->buf_fd[1] = prime_arg.fd;

      handle->stride[0] = width;
      handle->offset[0] = 0;

      handle->stride[1] = handle->stride[0];
      handle->offset[1] = handle->offset[0] + create_arg.size * 1 / 2;
      break;
    case DRM_FORMAT_YVU420:
    case DRM_FORMAT_YUV420:
      handle->buf_fd[0] = prime_arg.fd;
      handle->buf_fd[1] = prime_arg.fd;
      handle->buf_fd[2] = prime_arg.fd;

      handle->stride[0] = width;
      handle->offset[0] = 0;

      handle->stride[1] = handle->stride[0] / 2;
      handle->offset[1] = create_arg.size * 2 / 3;

      handle->stride[2] = handle->stride[0] / 2;
      handle->offset[2] = create_arg.size * 5 / 6;
      break;
    case DRM_FORMAT_YUV444:
    case DRM_FORMAT_AYUV:
    case DRM_FORMAT_RGBA8888:
    case DRM_FORMAT_BGRA8888:
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_BGRX8888:
    case DRM_FORMAT_XRGB8888:
    case DRM_FORMAT_XBGR8888:
    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_RGB888:
    case DRM_FORMAT_BGR888:
    case DRM_FORMAT_XRGB4444:
    case DRM_FORMAT_ARGB4444:
    case DRM_FORMAT_XRGB1555:
    case DRM_FORMAT_ABGR1555:
    case DRM_FORMAT_XRGB2101010:
    case DRM_FORMAT_ARGB2101010:
    case DRM_FORMAT_R8:
      handle->stride[0] = create_arg.pitch;
      handle->buf_fd[0] = prime_arg.fd;
      handle->offset[0] = 0;
      break;

    default:
      break;
  }
}

static int
vendor_alloc_buffer (struct dm_drm_t *drv, struct dm_drm_handle_t *handle)
{
  struct drm_mode_create_dumb arg;
  struct drm_prime_handle prime;
  int ret;

  memset (&arg, 0, sizeof (arg));
  memset (&prime, 0, sizeof (prime));
  GST_DEBUG_OBJECT (NULL,
      "alloc buffer width:%d, heigth:%d, flags:0x%x, "
      "drm_format:0x%x, drm_device fd:%d",
      handle->width, handle->height, handle->dm_hdl.flags,
      handle->drm_format, drv->fd);
  for (int i = 0; i < MAX_BUF_PLANES; ++i)
    handle->buf_fd[i] = -1;

  create_arg_init (handle->drm_format, handle->width, handle->height, &arg);
  if (arg.bpp == 0) {
    GST_ERROR_OBJECT (NULL, "create_arg_init failed.");
    return -1;
  }

  ret = drmIoctl (drv->fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
  if (ret) {
    GST_ERROR_OBJECT (NULL, "Failed to create drm dumb buffer.");
    return -1;
  }

  prime.fd = -1;
  prime.handle = arg.handle;
  prime.flags = DRM_CLOEXEC | DRM_RDWR;
  // ret = ioctl(drv->fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime);
  ret = drmIoctl (drv->fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime);
  if (ret) {
    GST_ERROR_OBJECT (NULL, "Failed to ioctl to fd.");
    return -1;
  }

  img_buf_info_init (arg, prime, handle->width, handle->drm_format, handle);

  handle->prime_fd = prime.fd;
  handle->bo_handle = prime.handle;
  handle->size = arg.size;

  for (int i = 0; i < MAX_BUF_PLANES && (handle->buf_fd[i] != -1); ++i)
    handle->n_planes++;

  GST_DEBUG_OBJECT (NULL, "alloc drm buffer size:%lld, num planes:%d",
      arg.size, handle->n_planes);
  return 0;
}

static struct dm_drm_bo_t *
vendor_alloc (struct dm_drm_t *drm, struct dm_drm_handle_t *handle)
{
  struct dm_drm_bo_t *ib;

  ib = (dm_drm_bo_t *) calloc (1, sizeof (*ib));
  if (!ib)
    return NULL;

  if (vendor_alloc_buffer (drm, handle) < 0) {
    free (ib);
    return NULL;
  }

  ib->handle = handle;
  ib->drm = drm;
  ib->refcount = 1;
  ib->imported = 0;
  return ib;
}

static struct dm_drm_bo_t *
vendor_import (struct dm_drm_t *drm, struct dm_drm_handle_t *handle)
{
  struct dm_drm_bo_t *ib;
  struct drm_prime_handle args;

  GST_DEBUG_OBJECT (NULL, "import buffer");
  ib = (dm_drm_bo_t *) calloc (1, sizeof (struct dm_drm_bo_t));
  if (!ib)
    return NULL;

  // vendor_alloc_buffer(drm, handle);
  args.fd = handle->prime_fd;
  args.flags = 0;
  drmIoctl (drm->fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &args);

  handle->bo_handle = args.handle;

  ib->handle = handle;
  ib->drm = drm;

  ib->refcount = 1;
  ib->imported = 1;
  return ib;
}

static void
vendor_free (struct dm_drm_t *drm, struct dm_drm_bo_t *bo)
{
  int imported = bo->imported;
  struct drm_mode_destroy_dumb destroy_arg;
  struct dm_drm_handle_t *handle = bo->handle;
  memset (&destroy_arg, 0, sizeof (destroy_arg));
  GST_DEBUG_OBJECT (NULL, "free buffer imported flag:%d", imported);

  destroy_arg.handle = handle->bo_handle;
  if (!imported) {
    drmIoctl (drm->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_arg);
  }
  for (int i = 0; i < handle->n_planes; i++)
    close (handle->buf_fd[i]);
  free (handle);
  bo->handle = NULL;
  free (bo);
}

static int
vendor_map (struct dm_drm_t *drv, struct dm_drm_bo_t *bo, int x, int y, int w,
    int h, void **addr)
{
  struct drm_mode_map_dumb map_arg;
  struct dm_drm_handle_t *handle = bo->handle;
  void *map;
  memset (&map_arg, 0, sizeof (map_arg));
  map_arg.handle = handle->bo_handle;
  drmIoctl (drv->fd, DRM_IOCTL_MODE_MAP_DUMB, &map_arg);
  GST_DEBUG_OBJECT (NULL, "map buffer");
  map = mmap (0, handle->size, PROT_WRITE | PROT_READ, MAP_SHARED, drv->fd,
      map_arg.offset);
  *addr = map;
  handle->dm_hdl.addr = (uint64_t) map;
  return 0;
}

static void
vendor_unmap (struct dm_drm_t *drv, struct dm_drm_bo_t *bo)
{
  struct dm_drm_handle_t *handle = bo->handle;
  GST_DEBUG_OBJECT (NULL, "unmap buffer");
  munmap ((void *) handle->dm_hdl.addr, handle->size);
  handle->dm_hdl.addr = 0;
}

struct dm_drm_drv_t *
dm_drm_drv_create_for_vendor (int fd)
{
  struct vendor_info *info;

  info = (vendor_info *) calloc (1, sizeof (struct vendor_info));
  if (!info) {
    GST_DEBUG_OBJECT (NULL, "failed to allocate driver info.");
    return NULL;
  }

  info->fd = fd;

  info->base.destroy = vendor_destroy;
  info->base.alloc = vendor_alloc;
  info->base.import = vendor_import;
  info->base.free = vendor_free;
  info->base.map = vendor_map;
  info->base.unmap = vendor_unmap;

  return &info->base;
}

/**********************************************************/

/**********************************************************/
static struct dm_drm_drv_t *
init_drv_from_fd (int fd)
{
  struct dm_drm_drv_t *drv = NULL;
  drmVersionPtr version;

  /* get the kernel module name */
  version = drmGetVersion (fd);
  if (!version) {
    GST_DEBUG_OBJECT (NULL, "invalid DRM fd");
    return NULL;
  }

  if (version->name) {
    drv = dm_drm_drv_create_for_vendor (fd);
    GST_DEBUG_OBJECT (NULL, "create vendor for driver.");
  }
  drmFreeVersion (version);

  return drv;
}

int
dm_drm_bo_lock (struct dm_drm_bo_t *bo, int usage, int x, int y, int w, int h,
    void **addr)
{
  int err = bo->drm->drv->map (bo->drm, bo, x, y, w, h, addr);
  if (err)
    return err;
  else {
    /* kernel handles the synchronization here */
  }

  return 0;
}

void
dm_drm_bo_unlock (struct dm_drm_bo_t *bo)
{
  bo->drm->drv->unmap (bo->drm, bo);
}

struct dm_drm_t *
dm_drm_create (void)
{
  struct dm_drm_t *drm;

  drm = (dm_drm_t *) calloc (1, sizeof (struct dm_drm_t));
  if (!drm)
    return NULL;
  drm->fd = drmOpen("semidrive", NULL);
  if (drm->fd < 0) {
    GST_DEBUG_OBJECT (NULL, "failed to open semidrive drm devices");
    free (drm);
    return NULL;
  }

  drm->drv = init_drv_from_fd (drm->fd);
  if (!drm->drv) {
    GST_DEBUG_OBJECT (NULL, "Failed to dm_drm_create.");
    close (drm->fd);
    free (drm);
    return NULL;
  }

  pthread_mutex_init (&drm->mutex, NULL);
  drm->refcount = 1;

  return drm;
}

int
dm_drm_destroy (dm_drm_t * drm)
{
  if (NULL == drm) {
    GST_ERROR_OBJECT (NULL, "drm ptr is nullptr");
    return -1;
  }

  pthread_mutex_destroy (&drm->mutex);

  if (drm->drv)
    drm->drv->destroy (drm->drv);
  drm->drv = NULL;

  close (drm->fd);
  free (drm);

  return 0;
}

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

static struct dm_drm_handle_t *
create_bo_handle_from_data (struct dm_meta_data *data)
{
  struct dm_drm_handle_t *handle;

  handle = calloc (1, sizeof (struct dm_drm_handle_t));
  if (!handle)
    return NULL;

  memset (handle, 0, sizeof (struct dm_drm_handle_t));

  handle->prime_fd = data->fds[0];

  handle->magic = DRM_HANDLE_MAGIC;
  handle->width = data->width;
  handle->height = data->height;
  handle->format = data->format;

  handle->drm_format = data->format;
  handle->n_planes = data->n_planes;

  for (int i = 0; i < data->n_planes; i++) {
    handle->buf_fd[i] = data->fds[i];
    handle->offset[i] = data->offsets[i];
    handle->stride[i] = data->strides[i];
  }
  handle->size = data->size;

  GST_DEBUG_OBJECT (NULL, "create_bo_handle: handle=%p", handle);
  return handle;
}

static struct dm_drm_handle_t *
create_bo_handle (int width, int height, int format, int usage)
{
  struct dm_drm_handle_t *handle;

  handle = (dm_drm_handle_t *) calloc (1, sizeof (struct dm_drm_handle_t));
  if (!handle)
    return NULL;

  memset (handle, 0, sizeof (struct dm_drm_handle_t));
  handle->prime_fd = -1;
  handle->n_planes = 0;
  handle->magic = DRM_HANDLE_MAGIC;
  handle->width = width;
  handle->height = height;
  handle->format = format;
  handle->dm_hdl.flags = usage;
  handle->drm_format = format;
  GST_DEBUG_OBJECT (NULL, "create_bo_handle: handle=%p", handle);
  return handle;
}

dm_handle_t *
get_dm_target (struct dm_drm_bo_t * bo)
{
  int ret = 0;
  struct dm_drm_handle_t *target;

  if (!bo)
    return NULL;

  target = bo->handle;
  if (!target) {
    // TODO
    // ret = import_handle_to_target(bo);
    if (ret < 0)
      return NULL;
  }
  return &bo->handle->dm_hdl;
}

static int32_t dm_drm_pid = 0;
#define unlikely(x) __builtin_expect (!!(x), 0)

static int
dm_drm_get_pid (void)
{
  if (unlikely (!dm_drm_pid))
    dm_drm_pid = getpid ();

  return dm_drm_pid;
}

struct dm_drm_bo_t *
dm_drm_bo_create (struct dm_drm_t *drm, int width, int height, int format,
    int usage)
{
  struct dm_drm_bo_t *bo = NULL;
  struct dm_drm_handle_t *handle = NULL;

  handle = create_bo_handle (width, height, format, usage);
  if (!handle)
    return NULL;

  bo = drm->drv->alloc (drm, handle);
  if (!bo) {
    GST_DEBUG_OBJECT (NULL, "drm alloc failed.");
    free (handle);
    return NULL;
  }

  if (handle->prime_fd < 0) {
    GST_DEBUG_OBJECT (NULL, "drm prime_fd failed.");
  }

  bo->drm = drm;
  bo->imported = 0;
  bo->handle = handle;
  bo->refcount = 1;
  handle->data_owner = dm_drm_get_pid ();
  handle->data = bo;

  return bo;
}

struct dm_drm_bo_t *
dm_drm_bo_create_from_fd (struct dm_drm_t *drm, struct dm_meta_data *data)
{
  struct dm_drm_bo_t *bo = NULL;
  struct dm_drm_handle_t *handle;

  handle = create_bo_handle_from_data (data);
  if (!handle)
    return NULL;

  bo = drm->drv->import (drm, handle);
  if (!bo) {
    GST_ERROR_OBJECT (NULL, "drm import failed.");
    // destroy_native_target(target);
    free (handle);
    return NULL;
  }

  handle->data_owner = getpid ();
  handle->data = bo;

  return bo;
}

/**********************************************************/

/* *INDENT-OFF* */
static const struct
{
  guint32 fourcc;
  GstVideoFormat format;
} format_map[] = {
#define DEF_FMT(fourcc, fmt)                                                  \
  {                                                                           \
    DRM_FORMAT_##fourcc, GST_VIDEO_FORMAT_##fmt                               \
  }

/* DEF_FMT (XRGB1555, ???), */
/* DEF_FMT (XBGR1555, ???), */
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
  DEF_FMT (ARGB8888, BGRA), DEF_FMT (XRGB8888, BGRx), DEF_FMT (ABGR8888, RGBA),
  DEF_FMT (XBGR8888, RGBx), DEF_FMT (BGRX8888, xRGB), DEF_FMT (BGRA8888, ARGB),
  DEF_FMT (BGR888, RGB),    DEF_FMT (RGB888, BGR),
#else
  DEF_FMT (ARGB8888, ARGB), DEF_FMT (XRGB8888, xRGB), DEF_FMT (ABGR8888, ABGR),
  DEF_FMT (XBGR8888, xBGR), DEF_FMT (RGB888, RGB),    DEF_FMT (BGR888, BGR),
  DEF_FMT (BGRA8888, BGRA), DEF_FMT (BGRX8888, BGRx),
#endif
  DEF_FMT (UYVY, UYVY),     DEF_FMT (YUYV, YUY2),     DEF_FMT (YUV420, I420),
  DEF_FMT (YVU420, YV12),   DEF_FMT (YUV422, Y42B),   DEF_FMT (NV12, NV12),
  DEF_FMT (NV21, NV21),     DEF_FMT (NV16, NV16),

#undef DEF_FMT
};
/* *INDENT-ON* */

GstVideoFormat
gst_video_format_from_drm (guint32 drmfmt)
{
  gint i;

  for (i = 0; i < G_N_ELEMENTS (format_map); i++) {
    if (format_map[i].fourcc == drmfmt)
      return format_map[i].format;
  }

  return GST_VIDEO_FORMAT_UNKNOWN;
}

guint32
gst_drm_format_from_video (GstVideoFormat fmt)
{
  gint i;

  for (i = 0; i < G_N_ELEMENTS (format_map); i++) {
    if (format_map[i].format == fmt)
      return format_map[i].fourcc;
  }

  return 0;
}

guint32
gst_drm_bpp_from_drm (guint32 drmfmt)
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

guint32
gst_drm_height_from_drm (guint32 drmfmt, guint32 height)
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
