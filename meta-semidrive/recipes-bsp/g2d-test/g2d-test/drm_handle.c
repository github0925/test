#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>

#include <drm.h>
#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "g2d_test_utils.h"
#include "drm_handle.h"

static int32_t dm_drm_pid = 0;
#define unlikely(x) __builtin_expect(!!(x), 0)

static int dm_drm_get_pid(void)
{
    if (unlikely(!dm_drm_pid))
        dm_drm_pid = getpid();

    return dm_drm_pid;
}

static struct dm_drm_drv_t *init_drv_from_fd(int fd)
{
    struct dm_drm_drv_t *drv = NULL;
    drmVersionPtr version;

    /* get the kernel module name */
    version = drmGetVersion(fd);
    if (!version)
    {
        slog_info("invalid DRM fd\n");
        return NULL;
    }

    if (version->name)
    {
        drv = dm_drm_drv_create_for_vendor(fd);
        slog_info("create vendor for driver.\n");
    }
    drmFreeVersion(version);

    return drv;
}


int dm_drm_bo_lock(struct dm_drm_bo_t *bo,
                   int usage, int x, int y, int w, int h, void **addr)
{
    //TODO: usage?

    int err = bo->drm->drv->map(bo->drm, bo, x, y, w, h, addr);
    if (err)
        return err;
    else
    {
        /* kernel handles the synchronization here */
    }

    return 0;
}

void dm_drm_bo_unlock(struct dm_drm_bo_t *bo)
{
    bo->drm->drv->unmap(bo->drm, bo);
    slog_verbose("***************************    bo unmap !");
}

int drm_ioctl_open(void)
{
    int fd = drmOpen("semidrive", nullptr);

    if (fd < 0)
        slog_info("open semidrive drm failed");

    return fd;
}

struct dm_drm_t *dm_drm_create(void)
{
    struct dm_drm_t *drm;

    drm = (struct dm_drm_t *)calloc(1, sizeof(*drm));
    if (!drm)
        return NULL;

    drm->fd = drm_ioctl_open();
    if (drm->fd < 0)
    {
        slog_info("failed to open %s\n", DRM_DEVICE);
        free(drm);
        return NULL;
    }

    drm->drv = init_drv_from_fd(drm->fd);
    if (!drm->drv)
    {
        slog_info("Failed to dm_drm_create.\n");
        close(drm->fd);
        free(drm);
        return NULL;
    }

    pthread_mutex_init(&drm->mutex, NULL);
    drm->refcount = 1;

    return drm;
}

dm_handle_t *get_dm_target(struct dm_drm_bo_t *bo)
{
    int ret = 0;
    struct dm_drm_handle_t *target;

    if (!bo)
        return NULL;

    target = bo->handle;
    if (!target)
    {
        //TODO
        //ret = import_handle_to_target(bo);
        if (ret < 0)
            return NULL;
    }
    return &bo->handle->dm_hdl;
}

int dm_drm_get_fd(struct dm_drm_t *drm)
{
    return drm->fd;
}

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

static struct dm_drm_handle_t *create_bo_handle(int width,
                                                int height, int format, int usage)
{
    struct dm_drm_handle_t *handle;

    handle = (struct dm_drm_handle_t *)calloc(1, sizeof(struct dm_drm_handle_t));
    if (!handle)
        return NULL;

    memset(handle, 0, sizeof(struct dm_drm_handle_t));
    handle->prime_fd = -1;
    handle->n_planes = 0;
    handle->magic = DRM_HANDLE_MAGIC;
    handle->width = width;
    handle->height = height;
    handle->format = format;
    handle->dm_hdl.flags = usage;
    handle->drm_format = format;
    slog_info("create_bo_handle: handle=%p\n", handle);
    return handle;
}

struct dm_drm_bo_t *dm_drm_bo_create(struct dm_drm_t *drm,
                                     int width, int height, int format, int usage)
{
    struct dm_drm_bo_t *bo = NULL;
    struct dm_drm_handle_t *handle;

    handle = create_bo_handle(width, height, format, usage);
    if (!handle)
        return NULL;

    bo = drm->drv->alloc(drm, handle);
    if (!bo)
    {
        slog_info("drm alloc bo failed.\n");
        //destroy_native_target(target);
        free(handle);
        return NULL;
    }

    if (handle->prime_fd < 0)
    {
        slog_info("drm prime_fd failed.\n");
        //native_array_release(&target->fds);
    }

    bo->drm = drm;
    bo->imported = 0;
    bo->handle = handle;
    bo->refcount = 1;
    handle->data_owner = dm_drm_get_pid();
    handle->data = bo;

    return bo;
}


static void vendor_destroy(struct dm_drm_drv_t *drv)
{
    slog_info("destroy drm info.\n");
    struct vendor_info *info = (struct vendor_info *)drv;

    free(info);
}

int create_arg_init(int format, int width, int height, struct drm_mode_create_dumb *create_arg)
{
    int ret = 0;

    switch (format)
    {
    case DRM_FORMAT_RGBA8888:
    case DRM_FORMAT_BGRA8888:
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_XRGB8888:
    case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_XRGB2101010:
	case DRM_FORMAT_ARGB2101010:
    case DRM_FORMAT_XBGR2101010:
    case DRM_FORMAT_BGRX1010102:
    case DRM_FORMAT_ABGR2101010:
    case DRM_FORMAT_BGRA1010102:
    case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_AYUV:
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
    case DRM_FORMAT_XBGR4444:
    case DRM_FORMAT_BGRX4444:
    case DRM_FORMAT_ABGR4444:
    case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_ARGB1555:
    case DRM_FORMAT_XBGR1555:
    case DRM_FORMAT_BGRX5551:
    case DRM_FORMAT_ABGR1555:
    case DRM_FORMAT_BGRA5551:
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
        create_arg->width = ALIGN(width * 2, 16);
        create_arg->height = height;
        break;

    case DRM_FORMAT_YUV422:
    case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
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
        slog_err("ERROE !!! don't support this format!\n");
        ret = -1;
        break;
    }
    return ret;
}

static void img_buf_info_init(const struct drm_mode_create_dumb create_arg, const struct drm_prime_handle prime_arg,
                              const int width, const int format, struct dm_drm_handle_t *handle)
{
    switch (format)
    {
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
    case DRM_FORMAT_XRGB8888:
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


static int vendor_alloc_buffer(struct dm_drm_t *drv, struct dm_drm_handle_t *handle)
{
    struct drm_mode_create_dumb arg;
    struct drm_prime_handle prime;
    int ret;

    memset(&arg, 0, sizeof(arg));
    memset(&prime, 0, sizeof(prime));
    slog_info("alloc buffer width:%d, heigth:%d, flags:0x%x, drm_format:0x%x", handle->width, handle->height,
              handle->dm_hdl.flags, handle->drm_format);
    debug_fmt(handle->drm_format);
    for (int i = 0; i < MAX_BUF_PLANES; ++i)
        handle->buf_fd[i] = -1;

    ret = create_arg_init(handle->drm_format, handle->width, handle->height, &arg);
    if(ret < 0) {
        return -1;
    }
    ret = drmIoctl(drv->fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
    if (ret)
    {
        slog_info("Failed to create drm dumb buffer.\n");
        return -1;
    }

    prime.handle = arg.handle;
    prime.flags = DRM_CLOEXEC;
    ret = ioctl(drv->fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &prime);
    if (ret)
    {
        slog_info("Failed to ioctl to fd.\n");
        return -1;
    }

    img_buf_info_init(arg, prime, handle->width, handle->drm_format, handle);

    handle->prime_fd = prime.fd;
    handle->bo_handle = prime.handle;
    handle->size = arg.size;

    for (int i = 0; i < MAX_BUF_PLANES && (handle->buf_fd[i] != -1); ++i)
        handle->n_planes++;

    slog_info("alloc drm buffer size:%lld, num planes:%d\n", arg.size, handle->n_planes);
    return 0;
}

static struct dm_drm_bo_t *vendor_alloc(struct dm_drm_t *drm,
                                        struct dm_drm_handle_t *handle)
{
    struct dm_drm_bo_t *ib;
    int ret;

    ib = (struct dm_drm_bo_t *)calloc(1, sizeof(*ib));
    if (!ib)
        return NULL;

    ret = vendor_alloc_buffer(drm, handle);
    if (ret < 0){
        if(ib)
            free(ib);
        return NULL;
    }

    ib->handle = handle;
    ib->drm = drm;
    ib->refcount = 1;
    ib->imported = 0;
    return ib;
}

static struct dm_drm_bo_t *vendor_import(struct dm_drm_t *drm,
                                         struct dm_drm_handle_t *handle)
{
    struct dm_drm_bo_t *ib;
    slog_info("import buffer");
    ib = (struct dm_drm_bo_t *)calloc(1, sizeof(*ib));
    if (!ib)
        return NULL;

    //vendor_alloc_buffer(drm, handle);
    struct drm_prime_handle args;

    args.fd = handle->prime_fd;
    args.flags = 0;
    drmIoctl(drm->fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &args);

    handle->bo_handle = args.handle;

    ib->handle = handle;
    ib->drm = drm;

    ib->refcount = 1;
    ib->imported = 1;
    return ib;
}

static void vendor_free(struct dm_drm_t *drm, struct dm_drm_bo_t *bo)
{
    int imported = bo->imported;
    struct drm_mode_destroy_dumb destroy_arg;
    struct dm_drm_handle_t *handle = bo->handle;
    memset(&destroy_arg, 0, sizeof(destroy_arg));
    slog_info("free buffer imported flag:%d", imported);

    destroy_arg.handle = handle->bo_handle;
    if (!imported)
    {
        drmIoctl(drm->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_arg);
    }
    for (int i = 0; i < handle->n_planes; i++)
        close(handle->buf_fd[i]);

	free(bo->handle);
    free(bo);
}

static int vendor_map(struct dm_drm_t *drv,
                      struct dm_drm_bo_t *bo,
                      int x, int y, int w, int h,
                      void **addr)
{
    struct drm_mode_map_dumb map_arg;
    struct dm_drm_handle_t *handle = bo->handle;
    void *map;
    memset(&map_arg, 0, sizeof(map_arg));
    map_arg.handle = handle->bo_handle;
    drmIoctl(drv->fd, DRM_IOCTL_MODE_MAP_DUMB, &map_arg);
    slog_info("map buffer");
    //getchar();
    map = mmap(0, handle->size, PROT_WRITE | PROT_READ, MAP_SHARED, drv->fd, map_arg.offset);
    //getchar();
    *addr = map;
    slog_info("cuishang  >>>> map_arg.offset = 0x%llx , map =%p ,handle->size = %d", map_arg.offset, map, handle->size);
    handle->dm_hdl.addr = (uint64_t)map;
    return 0;
}

static void vendor_unmap(struct dm_drm_t *drv,
                         struct dm_drm_bo_t *bo)
{
    slog_info("unmap buffer");
    struct dm_drm_handle_t *handle = bo->handle;
    munmap((void *)handle->dm_hdl.addr, handle->size);
    handle->dm_hdl.addr = 0;
}

struct dm_drm_drv_t *dm_drm_drv_create_for_vendor(int fd)
{
    struct vendor_info *info;

    info = (struct vendor_info *)calloc(1, sizeof(*info));
    if (!info)
    {
        slog_info("failed to allocate driver info.\n");
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



