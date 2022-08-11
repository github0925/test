#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <math.h>

#include "csitest.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include "xrp_xnnc_ns.h"
#include "xrp_api.h"

// Maximum number of input and output across all xtensa operations
#define XTENSA_OPERATION_MAX_NUM_INPUTS  7
#define XTENSA_OPERATION_MAX_NUM_OUTPUTS 8

#define IMAGE_WIDTH  1280
#define IMAGE_HEIGHT 720

#define AVM_WIDTH  640
#define AVM_HEIGHT 720
//#define AVM_HEIGHT 800

/* use dmabuf or userptr */
#define MAX_BUFS_NUM 4
#define CONFIG_USE_DMABUF 1

#define RESULT_CHECK(x) do { \
    int ret = x; \
    if (ret) { \
        LOGD(#x " Error: %d (%s)", ret, strerror(-ret)); \
        return ret; \
        } \
    } while(0)

void setColor(void *vaddr, int w, int h, uint32_t color)
{
    uint32_t *p = (uint32_t *)vaddr;

    for (auto i = 0; i < h; i++)
        for (auto j = 0; j < w; j++) {
            p[i * w + j] = color;
        }
}

int64_t getTimeMsec()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (int64_t) now.tv_sec*1000LL + now.tv_nsec/1000000;
}

typedef struct drm_buffer_t {
    uint32_t size;
    uint64_t phys_addr;
    uint64_t base;
    uint64_t virt_addr;
    uint64_t dma_addr;
    uint64_t attachment;
    uint64_t sgt;
    int32_t  buf_handle;
} drm_buffer_t;

#define DRM_COMMAND_BASE 0x40
#define DRM_IOCTL_BASE          'd'
#define DRM_IO(nr)          _IO(DRM_IOCTL_BASE,nr)
#define DRM_IOR(nr,type)        _IOR(DRM_IOCTL_BASE,nr,type)
#define DRM_IOW(nr,type)        _IOW(DRM_IOCTL_BASE,nr,type)
#define DRM_IOWR(nr,type)       _IOWR(DRM_IOCTL_BASE,nr,type)

#define DRM_IOCTL_SEMIDRIVE_MAP_PHYS        DRM_IOWR(DRM_COMMAND_BASE + 1, struct drm_buffer_t)
#define DRM_IOCTL_SEMIDRIVE_UNMAP_PHYS      DRM_IOWR(DRM_COMMAND_BASE + 2, struct drm_buffer_t)

#define DRM_IOCTL_SEMIDRIVE_EXPORT_DMABUF   DRM_IOWR(DRM_COMMAND_BASE + 3, struct drm_buffer_t)

int drm_ioctl_open(void)
{
    int fd = drmOpen("semidrive", nullptr);

    if (fd < 0)
        LOGD("open semidrive drm failed: %d(%s)", errno, strerror(errno));

    return fd;
}

int drm_ioctl_get_phy_addr(int fd, int dmabuf, drm_buffer_t *data)
{
    int ret;
    data->buf_handle = dmabuf;
    ret = ioctl(fd, DRM_IOCTL_SEMIDRIVE_MAP_PHYS, data);
    return ret;
}

int drm_ioctl_release_phy_addr(int fd, drm_buffer_t *data)
{
    int ret;
    ret = ioctl(fd, DRM_IOCTL_SEMIDRIVE_UNMAP_PHYS, data);
    return ret;
}

int drm_ioctl_export_dmabuf(int fd, uint64_t addr, size_t size,
                            int *dma_fd, int flags)
{
    int ret;
    drm_buffer_t drm_buf;

#define PAGE_ALIGN(x, y) (((x) + y - 1) & (~(y - 1)))

    drm_buf.phys_addr = addr;
    drm_buf.size = PAGE_ALIGN(size, 4096);
    (void)flags;

    if (size == 0) {
        LOGE("error: do not use 0 size map");
    }

    ret = ioctl(fd, DRM_IOCTL_SEMIDRIVE_EXPORT_DMABUF, &drm_buf);

    if (ret) {
        LOGE("export dmabuf failed: %d(%s)", ret, strerror(ret));
    }

    *dma_fd = drm_buf.buf_handle;
    LOGD("phys: 0x%lx ----> dmabuf: %d", addr, *dma_fd);

#undef PAGE_ALIGN

    return 0;
}

uint32_t get_property_id(int fd, drmModeObjectProperties *props,
                         const char *name)
{
    drmModePropertyPtr property;
    uint32_t i, id = 0;

    for (i = 0; i < props->count_props; i++) {
        property = drmModeGetProperty(fd, props->props[i]);

        if (!strcmp(property->name, name))
            id = property->prop_id;

        drmModeFreeProperty(property);

        if (id)
            break;
    }

    return id;
}


static int c_terminate = 0;

static void sigint_handler(int arg)
{
    c_terminate = 1;
}

Cam::Cam(const char *video_name)
{
    printf("cam_open(%s)\n", video_name);
    int fd = open(video_name, O_RDWR);

    if (fd < 0) {
        printf("open %s failed,erro=%s\n", video_name, strerror(errno));
        return ;
    }

    cam_fd = fd;
}

Cam::~Cam()
{
    if ( cam_fd > 0 ) {
        close(cam_fd);
        cam_fd = -1;
    }
}

int Cam::set_format(int pixelformat, int width, int height)
{
    int ret;
    struct v4l2_format format;

    printf("width:height=%dx%d, pixelformat=0x%x, fd=%d\n", width, height,
           pixelformat, cam_fd);
    memset(&format, 0, sizeof(struct v4l2_format));

    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;
    format.fmt.pix.field = V4L2_FIELD_ANY;
    format.fmt.pix.pixelformat = pixelformat;
    format.fmt.pix.priv = 0x5a5afefe;

    ret = ioctl(cam_fd, VIDIOC_S_FMT, &format);

    if (ret < 0) {
        printf("ioctl(VIDIOC_S_FMT) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }

    return ret;
}

int Cam::init_buffer()
{
    unsigned int i;
    int ret;
    struct v4l2_requestbuffers req;

    printf("%s\n", __func__);
    req.count = MAX_BUFS_NUM;

    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    req.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(cam_fd, VIDIOC_REQBUFS, &req);

    if (ret < 0) {
        printf("ioctl(VIDIOC_REQBUFS) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }

    //printf("req.count: %d\n", req.count);
    if (req.count < 4) {
        printf("request buffer failed\n");
        return ret;
    }

    struct v4l2_buffer buffer;

    struct v4l2_plane planes = {0};

    memset(vb_ptr, 0x00, sizeof(vb_ptr));

    for (i = 0; i < req.count; i++) {
        memset(&buffer, 0, sizeof(struct v4l2_buffer));
        memset(&planes, 0, sizeof(planes));
        buffer.type = req.type;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.m.planes = &planes;
        buffer.flags = 0;
        buffer.index = i;
        buffer.length = 1;
        ret = ioctl(cam_fd, VIDIOC_QUERYBUF, &buffer);

        if (ret < 0) {
            printf("ioctl(VIDIOC_QUERYBUF) failed %d(%s)\n", errno, strerror(errno));
            return ret;
        }

	//Get v4l2 buffer dma fd
	ret = buffer_export(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, i, &dma_fd[i]);
	if (ret < 0)
		printf("ioctl VIDIOC_EXPBUF fail to get dma fd.\n");
	else
		printf("success to get dma fd = %d \n", dma_fd[i]);

	vb_dmafd_ptr[i] = (uint8_t *) mmap(NULL, buffer.m.planes[0].length, PROT_READ,
                                     MAP_SHARED, dma_fd[i], 0);

        vb_ptr[i] = (uint8_t *) mmap(NULL, buffer.m.planes[0].length, PROT_READ,
                                     MAP_SHARED, cam_fd, buffer.m.planes[0].m.mem_offset);
        vb_len[i] = buffer.m.planes[0].length;

        if (vb_ptr[i] == MAP_FAILED) {
            printf("mmap() failed %d(%s)\n", errno, strerror(errno));
            return -1;
        }

        if (vb_dmafd_ptr[i] == MAP_FAILED) {
            printf("dmafd mmap() failed %d(%s)\n", errno, strerror(errno));
           // return -1;
        }

	if (vb_ptr[i] == vb_dmafd_ptr[i])
	    printf("dmafd mapping equal v4l2 mapping.\n");

        ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);

        if (ret < 0) {
            printf("ioctl(VIDIOC_QBUF) failed %d(%s)\n", errno, strerror(errno));
            return ret;
        }
    }

    return 0;
}

int Cam::init_buffer(uint8_t **drm_ptr, size_t buf_len)
{
    unsigned int i;
    int ret;
    struct v4l2_requestbuffers req;

    printf("%s\n", __func__);
    req.count = 4;

    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    req.memory = V4L2_MEMORY_USERPTR;
    ret = ioctl(cam_fd, VIDIOC_REQBUFS, &req);

    if (ret < 0) {
        printf("ioctl(VIDIOC_REQBUFS) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }

    //printf("req.count: %d\n", req.count);
    if (req.count < 4) {
        printf("request buffer failed\n");
        return ret;
    }

    struct v4l2_buffer buffer;

    struct v4l2_plane planes = {0};

    memset(vb_ptr, 0x00, sizeof(vb_ptr));
    for (i = 0; i < req.count; i++) {
        memset(&buffer, 0, sizeof(struct v4l2_buffer));
        memset(&planes, 0, sizeof(planes));
        buffer.type = req.type;
        buffer.memory = V4L2_MEMORY_USERPTR;

        planes.bytesused = buf_len;
        planes.length    = buf_len;
        planes.m.userptr = (unsigned long)drm_ptr[i];

        buffer.m.planes = &planes;
        buffer.flags = 0;
        buffer.index = i;
        buffer.length = 1;

        vb_ptr[i] = drm_ptr[i];
        vb_len[i] = buf_len;

        ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);

        if (ret < 0) {
            printf("ioctl(VIDIOC_QBUF) failed %d(%s)\n", errno, strerror(errno));
            return ret;
        }
    }

    return 0;
}

int Cam::set_stream(int on)
{
    int ret, cmd;
    enum v4l2_buf_type buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    printf("%s(%d)\n", __func__, on);
    cmd = (on) ? VIDIOC_STREAMON : VIDIOC_STREAMOFF;
    printf("set stream %s\n", (on ? "on" : "off"));
    ret = ioctl(cam_fd, cmd, &buffer_type);

    if (ret < 0) {
        printf("cam_set_stream failed %d(%s)", errno, strerror(errno));
    }

    return ret;
}


int Cam::get_frame(uint8_t **buf, int *idx)
{
    int ret;
    int length = 0;
    static int frameNum = 0;
    struct v4l2_buffer buffer;

    //printf("%s\n", __func__);
    memset(&buffer, 0, sizeof(struct v4l2_buffer));
    struct v4l2_plane planes = {0};
    memset(&planes, 0, sizeof(planes));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.m.planes = &planes;
    buffer.length = 1;
    buffer.reserved = 0;
    ret = ioctl(cam_fd, VIDIOC_DQBUF, &buffer);

    if (ret < 0) {
        printf("ioctl(VIDIOC_DQBUF) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }

    if (buffer.index >= 4) {
        printf("invalid buffer index: %d\n", buffer.index);
        return ret;
    }

    *buf = vb_ptr[buffer.index];
    length = buffer.m.planes[0].length;
    *idx = buffer.index;

#if 1
    ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);

    if (ret < 0) {
        printf("ioctl(VIDIOC_QBUF) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }
#endif
    return length;
}

int Cam::get_userptr_frame(uint8_t **buf, int *idx)
{
    int ret;
    int length = 0;
    static int frameNum = 0;
    struct v4l2_buffer buffer;

    //printf("%s\n", __func__);
    memset(&buffer, 0, sizeof(struct v4l2_buffer));
    struct v4l2_plane planes = {0};
    memset(&planes, 0, sizeof(planes));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buffer.memory = V4L2_MEMORY_USERPTR;
    buffer.m.planes = &planes;
    buffer.length = 1;
    buffer.reserved = 0;
    ret = ioctl(cam_fd, VIDIOC_DQBUF, &buffer);

    if (ret < 0) {
        printf("ioctl(VIDIOC_DQBUF) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }

    if (buffer.index >= 4) {
        printf("invalid buffer index: %d\n", buffer.index);
        return ret;
    }

    *buf = vb_ptr[buffer.index];
    length = buffer.m.planes[0].length;
    *idx = buffer.index;

#if 1
    ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);

    if (ret < 0) {
        printf("ioctl(VIDIOC_QBUF) failed %d(%s)\n", errno, strerror(errno));
        return ret;
    }
#endif
    return length;
}

int Cam::buffer_export(enum v4l2_buf_type bt, int index, int *dmafd)
{
    struct v4l2_exportbuffer expbuf;

    memset(&expbuf, 0, sizeof(expbuf));
    expbuf.type = bt;
    expbuf.index = index;
    if (ioctl(cam_fd, VIDIOC_EXPBUF, &expbuf) == -1) {
        perror("VIDIOC_EXPBUF");
        return -1;
    }

    *dmafd = expbuf.fd;

    return 0;
}

int Cam::release_buffer()
{
    int i;

    for (i = 0; i < 4; i++) {
        if (vb_ptr[i] != NULL) {
            if (munmap((void *)vb_ptr[i], vb_len[i]) < 0) {
                printf("vb_ptr[%d] munmap failed : %s\n", i, strerror(errno));
            }

            if (munmap((void *)vb_dmafd_ptr[i], vb_len[i]) < 0) {
                printf("vb_dmafd_ptr[%d] munmap failed : %s\n", i, strerror(errno));
            }

            printf("munmap vb_ptr[%d] success !\n", i);
	    printf("munmap vb_dmafd_ptr[%d] success !\n", i);
            vb_ptr[i] = NULL;
	    vb_dmafd_ptr[i] = NULL;
        }
        else {
            break;
        }
    }

    return 0;
}

#define ALIGN(x, y) (((x) + y - 1) & (~(y - 1)))


int DrmFrame::getBppByFormat(int format)
{
    int bpp;

    switch (format) {
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
        case DRM_FORMAT_NV16:
        case DRM_FORMAT_NV61:
        case DRM_FORMAT_YUV420:
        case DRM_FORMAT_YVU420:
            bpp = 8;
            break;

        case DRM_FORMAT_ARGB4444:
        case DRM_FORMAT_XRGB4444:
        case DRM_FORMAT_ABGR4444:
        case DRM_FORMAT_XBGR4444:
        case DRM_FORMAT_RGBA4444:
        case DRM_FORMAT_RGBX4444:
        case DRM_FORMAT_BGRA4444:
        case DRM_FORMAT_BGRX4444:
        case DRM_FORMAT_ARGB1555:
        case DRM_FORMAT_XRGB1555:
        case DRM_FORMAT_ABGR1555:
        case DRM_FORMAT_XBGR1555:
        case DRM_FORMAT_RGBA5551:
        case DRM_FORMAT_RGBX5551:
        case DRM_FORMAT_BGRA5551:
        case DRM_FORMAT_BGRX5551:
        case DRM_FORMAT_RGB565:
        case DRM_FORMAT_BGR565:
        case DRM_FORMAT_UYVY:
        case DRM_FORMAT_VYUY:
        case DRM_FORMAT_YUYV:
        case DRM_FORMAT_YVYU:
            bpp = 16;
            break;

        case DRM_FORMAT_BGR888:
        case DRM_FORMAT_RGB888:
            bpp = 24;
            break;

        case DRM_FORMAT_ARGB8888:
        case DRM_FORMAT_XRGB8888:
        case DRM_FORMAT_ABGR8888:
        case DRM_FORMAT_XBGR8888:
        case DRM_FORMAT_RGBA8888:
        case DRM_FORMAT_RGBX8888:
        case DRM_FORMAT_BGRA8888:
        case DRM_FORMAT_BGRX8888:
        case DRM_FORMAT_ARGB2101010:
        case DRM_FORMAT_XRGB2101010:
        case DRM_FORMAT_ABGR2101010:
        case DRM_FORMAT_XBGR2101010:
        case DRM_FORMAT_RGBA1010102:
        case DRM_FORMAT_RGBX1010102:
        case DRM_FORMAT_BGRA1010102:
        case DRM_FORMAT_BGRX1010102:
            bpp = 32;
            break;

        default:
            fprintf(stderr, "unsupported format 0x%08x\n",  format);
            return -EINVAL;
    }

    return bpp;
}

int DrmFrame::fillBO(int width, int height, int stride, int format,
                     uint32_t gem_handles, int offset)
{

    bo.format = format;
    bo.width = width;
    bo.height = height;
    bo.offsets[0] = offset;
    bo.pitches[0] = stride;
    bo.gem_handles[0] = gem_handles;
    //printf("DrmFrame::fillBO(): width=%d, height=%d, stride=%d, format=0x%x,\n", width, height, stride, format);

    switch (format) {
        case DRM_FORMAT_UYVY:
        case DRM_FORMAT_VYUY:
        case DRM_FORMAT_YUYV:
        case DRM_FORMAT_YVYU:
            break;

        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
        case DRM_FORMAT_NV16:
        case DRM_FORMAT_NV61:
            bo.offsets[0] = 0;

            bo.pitches[1] = bo.pitches[0];
            bo.offsets[1] = bo.pitches[0] * height;
            bo.gem_handles[1] = bo.gem_handles[0];
            break;

        case DRM_FORMAT_YUV420:
        case DRM_FORMAT_YVU420:
            bo.offsets[0] = 0;

            bo.pitches[1] = bo.pitches[0] / 2;
            bo.offsets[1] = bo.pitches[0] * height;
            bo.gem_handles[1] = bo.gem_handles[0];
            bo.pitches[2] = bo.pitches[1];
            bo.offsets[2] = bo.offsets[1] + bo.pitches[1] * height / 2;
            bo.gem_handles[2] = bo.gem_handles[0];
            break;

        case DRM_FORMAT_ARGB8888:
        case DRM_FORMAT_XRGB8888:
        case DRM_FORMAT_ABGR8888:
        case DRM_FORMAT_XBGR8888:
        case DRM_FORMAT_RGBA8888:
        case DRM_FORMAT_RGBX8888:
        case DRM_FORMAT_BGRA8888:
        case DRM_FORMAT_BGRX8888:
            bo.pitches[0] = stride; // * 4;
            break;

        case DRM_FORMAT_BGR888:
        case DRM_FORMAT_RGB888:
            bo.pitches[0] = stride; // * 3;
            break;

        case DRM_FORMAT_ARGB4444:
        case DRM_FORMAT_XRGB4444:
        case DRM_FORMAT_ABGR4444:
        case DRM_FORMAT_XBGR4444:
        case DRM_FORMAT_RGBA4444:
        case DRM_FORMAT_RGBX4444:
        case DRM_FORMAT_BGRA4444:
        case DRM_FORMAT_BGRX4444:
        case DRM_FORMAT_ARGB1555:
        case DRM_FORMAT_XRGB1555:
        case DRM_FORMAT_ABGR1555:
        case DRM_FORMAT_XBGR1555:
        case DRM_FORMAT_RGBA5551:
        case DRM_FORMAT_RGBX5551:
        case DRM_FORMAT_BGRA5551:
        case DRM_FORMAT_BGRX5551:
        case DRM_FORMAT_RGB565:
        case DRM_FORMAT_BGR565:
            bo.pitches[0] = stride;// * 2;
            break;

        case DRM_FORMAT_ARGB2101010:
        case DRM_FORMAT_XRGB2101010:
        case DRM_FORMAT_ABGR2101010:
        case DRM_FORMAT_XBGR2101010:
        case DRM_FORMAT_RGBA1010102:
        case DRM_FORMAT_RGBX1010102:
        case DRM_FORMAT_BGRA1010102:
        case DRM_FORMAT_BGRX1010102:
            break;
    }

    return 0;
}

int DrmFrame::createBO(uint32_t virtual_width, uint32_t virtual_height,
                       int bpp)
{
    //create dumb
    struct drm_mode_create_dumb arg;
    int ret;

    memset(&arg, 0, sizeof(arg));
    arg.bpp = bpp;
    arg.width = virtual_width;
    arg.height = virtual_height;

    memset(&bo, 0, sizeof(hwc_drm_bo_t));
    LOGD("create dumb buffer: %d x %d x %d", arg.width, arg.height, arg.bpp);
    ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);

    if (ret) {
        fprintf(stderr, "failed to create dumb buffer: %s\n",
                strerror(errno));
        return -1;
    }

    bo.gem_handles[0] = arg.handle;
    bo.width = virtual_width;
    bo.height = virtual_height;
    bo.offsets[0] = 0;
    bo.pitches[0] = arg.pitch;

    return 0;
}

void DrmFrame::DestroyBO()
{
    struct drm_mode_destroy_dumb arg = {};

    arg.handle = bo.gem_handles[0];
    LOGD("destroy handle: %d", bo.gem_handles[0]);
    drmIoctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
}

void DrmFrame::Destroy()
{
    if (mapped_vaddr) {
        unMapBO();
    }

    struct drm_gem_close gem_close = {};

    gem_close.handle = bo.gem_handles[0];

    int ret1 = drmIoctl(drm_fd, DRM_IOCTL_GEM_CLOSE, &gem_close);

    if (ret1)
        LOGE("Failed to close gem handle %d ,%d(%s)", ret1, errno,
             strerror(errno));

    if (bo.fb_id > 0) {
        drmModeRmFB(drm_fd, bo.fb_id);
        bo.fb_id = -1;
    }

    if (isDumb)
        DestroyBO();
    else
        close(prime_fd_);
}

int DrmFrame::addFrameBuffer(int fbc_mode)
{
    if (fbc_mode) {
        if (drmModeAddFB2WithModifiers(drm_fd, bo.width, bo.height, bo.format,
                                       bo.gem_handles, bo.pitches, bo.offsets, bo.modifier,
                                       &(bo.fb_id), DRM_MODE_FB_INTERLACED )) {
            LOGD("failed to add fb: %s\n", strerror(errno));
            return -1;
        }
    }
    else {
        LOGD("add fb: gem: %d, format: %c%c%c%c, w %d, h %d, pitch %d offset %d\n",
             bo.gem_handles[0],
             bo.format & 0xff, (bo.format >> 8) & 0xff,
             (bo.format >> 16) & 0xff, (bo.format >> 24) & 0xff,
             bo.width, bo.height, bo.pitches[0], bo.offsets[0]);

        if (drmModeAddFB2(drm_fd, bo.width, bo.height, bo.format,
                          bo.gem_handles, bo.pitches, bo.offsets,
                          &(bo.fb_id), 0)) {
            LOGD("failed to add fb: %s\n", strerror(errno));
            return -1;
        }
    }

    fb_id = bo.fb_id;

    return 0;
}

int
DrmFrame::createDumbBuffer(unsigned int width,
                           unsigned int height, unsigned int format,
                           int fbc, int offset)
{
    int virtual_width, virtual_height;
    source = sdm::Rect(0, 0, width, height);
    display = source;

    virtual_width = ALIGN(width, 32);

    switch (format) {
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
        case DRM_FORMAT_YUV420:
        case DRM_FORMAT_YVU420:
            virtual_height = height * 3 / 2;
            break;

        case DRM_FORMAT_NV16:
        case DRM_FORMAT_NV61:
            virtual_height = height * 2;
            break;

        default:
            virtual_height = height;
            break;
    }

    //virtual_height *= 2;
    createBO(width, virtual_height, getBppByFormat(format));

    isDumb = true;
    fillBO(width, height, bo.pitches[0], format, bo.gem_handles[0], offset);

    return addFrameBuffer(fbc);
}

int DrmFrame::ImportBuffer(unsigned int width, unsigned int height,
                           int stride,
                           unsigned int format, int prime_fd, int fbc, int offset)
{
    uint32_t gem_handle;
    source = sdm::Rect(0, 0, width, height);
    display = source;
    prime_fd_ = prime_fd;
    int ret = drmPrimeFDToHandle(drm_fd, prime_fd, &gem_handle);

    if (ret) {
        LOGE("failed to import prime fd %d ret=%d", prime_fd, ret);
        return ret;
    }

    isDumb = false;
    fillBO(width, height, stride, format, gem_handle, offset);
    return addFrameBuffer(fbc);
}

int DrmFrame::MapBO()
{
    void *vaddr;
    struct drm_mode_map_dumb arg;

    int ret;

    memset(&arg, 0, sizeof(arg));
    arg.handle = bo.gem_handles[0];

    ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);

    if (ret)
        return ret;

    size_t sz_bo = bo.pitches[0] * bo.height;
    if (bo.format == DRM_FORMAT_YUV420)
        sz_bo = sz_bo * 3 / 2;
    vaddr = mmap(0, sz_bo, PROT_READ | PROT_WRITE, MAP_SHARED,
                 drm_fd, arg.offset);

    if (vaddr == MAP_FAILED)
        return -EINVAL;

    this->mapped_vaddr = vaddr;
    this->sz_bo = sz_bo;

    switch (bo.format) {
        case DRM_FORMAT_UYVY:
        case DRM_FORMAT_VYUY:
        case DRM_FORMAT_YUYV:
        case DRM_FORMAT_YVYU:
            planes[0] = vaddr;
            break;

        case DRM_FORMAT_NV12:
        case DRM_FORMAT_NV21:
        case DRM_FORMAT_NV16:
        case DRM_FORMAT_NV61:
            planes[0] = vaddr;
            planes[1] = (unsigned char *)vaddr + bo.offsets[1];
            break;

        case DRM_FORMAT_YUV420:
        case DRM_FORMAT_YVU420:
            planes[0] = vaddr;
            planes[1] = (unsigned char *)vaddr + bo.offsets[1];
            planes[2] = (unsigned char *)vaddr + bo.offsets[2];
            break;

        case DRM_FORMAT_ARGB4444:
        case DRM_FORMAT_XRGB4444:
        case DRM_FORMAT_ABGR4444:
        case DRM_FORMAT_XBGR4444:
        case DRM_FORMAT_RGBA4444:
        case DRM_FORMAT_RGBX4444:
        case DRM_FORMAT_BGRA4444:
        case DRM_FORMAT_BGRX4444:
        case DRM_FORMAT_ARGB1555:
        case DRM_FORMAT_XRGB1555:
        case DRM_FORMAT_ABGR1555:
        case DRM_FORMAT_XBGR1555:
        case DRM_FORMAT_RGBA5551:
        case DRM_FORMAT_RGBX5551:
        case DRM_FORMAT_BGRA5551:
        case DRM_FORMAT_BGRX5551:
        case DRM_FORMAT_RGB565:
        case DRM_FORMAT_BGR565:
        case DRM_FORMAT_BGR888:
        case DRM_FORMAT_RGB888:
        case DRM_FORMAT_ARGB8888:
        case DRM_FORMAT_XRGB8888:
        case DRM_FORMAT_ABGR8888:
        case DRM_FORMAT_XBGR8888:
        case DRM_FORMAT_RGBA8888:
        case DRM_FORMAT_RGBX8888:
        case DRM_FORMAT_BGRA8888:
        case DRM_FORMAT_BGRX8888:
        case DRM_FORMAT_ARGB2101010:
        case DRM_FORMAT_XRGB2101010:
        case DRM_FORMAT_ABGR2101010:
        case DRM_FORMAT_XBGR2101010:
        case DRM_FORMAT_RGBA1010102:
        case DRM_FORMAT_RGBX1010102:
        case DRM_FORMAT_BGRA1010102:
        case DRM_FORMAT_BGRX1010102:
            planes[0] = vaddr;
            break;
    }

    return 0;
}

void DrmFrame::unMapBO()
{
    if (!mapped_vaddr)
        return;

    munmap(mapped_vaddr, sz_bo);
    mapped_vaddr = NULL;
}

int DrmPlane::Init()
{

    getProperties();

    if (!property_fb_id) {
        LOGD("plane properties get failed");
        return -EINVAL;
    }

    return 0;
}
void DrmPlane::getProperties()
{
    /* get plane properties */
    drmModeObjectProperties *props = drmModeObjectGetProperties(drm_fd,
                                     plane_id, DRM_MODE_OBJECT_PLANE);
    property_crtc_id = get_property_id(drm_fd, props, "CRTC_ID");
    property_fb_id = get_property_id(drm_fd, props, "FB_ID");
    property_crtc_x = get_property_id(drm_fd, props, "CRTC_X");
    property_crtc_y = get_property_id(drm_fd, props, "CRTC_Y");
    property_crtc_w = get_property_id(drm_fd, props, "CRTC_W");
    property_crtc_h = get_property_id(drm_fd, props, "CRTC_H");
    property_src_x = get_property_id(drm_fd, props, "SRC_X");
    property_src_y = get_property_id(drm_fd, props, "SRC_Y");
    property_src_w = get_property_id(drm_fd, props, "SRC_W");
    property_src_h = get_property_id(drm_fd, props, "SRC_H");
    property_alpha = get_property_id(drm_fd, props, "alpha");
    property_blend_mode = get_property_id(drm_fd, props, "blend_mode");
    property_zpos = get_property_id(drm_fd, props, "zpos");

    drmModeFreeObjectProperties(props);
}

int DrmBackend::Init(int display)
{
    //#define DRM_CARD "/dev/dri/card0"
#define DRM_CARD "/dev/dri/card1"
    drm_fd_ = open(DRM_CARD, O_RDWR | O_CLOEXEC);
    //drm_fd_ = drmOpenControl(0);

    if (drm_fd_ < 0) {
        LOGD("Error: open %s %d(%s)", DRM_CARD, drm_fd_, strerror(-drm_fd_));
        return drm_fd_;
    }

#ifdef CONFIG_AUTH_CHECK
    drm_magic_t drm_magic = 0;
#define MAGIC_FIFO "/data/fifo1"

    switch (display) {
        case 0: { //drm master
            char buf[1024] = {};
            int fd, len;

            if (mkfifo(MAGIC_FIFO, 0666) < 0 && errno != EEXIST)
                LOGE("Create FIFO Failed");

            if ((fd = open(MAGIC_FIFO, O_RDONLY)) < 0) {
                LOGE("Open FIFO Failed");
                return -1;
            }

            while ((len = read(fd, buf, 1024)) > 0) // 读取FIFO管道
                LOGD("Read message: %s", buf);

            close(fd);  // 关闭FIFO文件

            long num = strtoul(buf, 0, 10);

            if (num >= 0)
                drm_magic = (drm_magic_t)num;

            LOGD("num = %u", drm_magic);
            //DDBG(drmAuthMagic(drm_fd_, drm_magic));
            drmAuthMagic(drm_fd_, drm_magic);
        }
        break;

        case 1: { //drm auth
            int fd, n;
            char buf[1024] = {};

            if ((fd = open(MAGIC_FIFO, O_WRONLY)) < 0) {
                LOGE("Open FIFO Failed");
                return -1;
            }

            //DDBG(drmGetMagic(drm_fd_, &drm_magic));
            drmGetMagic(drm_fd_, &drm_magic);
            LOGD("drmGetMagic num %d", drm_magic);
            n = sprintf(buf, "%lu", (unsigned long)drm_magic);

            if (write(fd, buf, n + 1) < 0) {
                LOGE("Write FIFO Failed");
                close(fd);
            }

            close(fd);
            LOGD("wait for drm master authing, press any key to continue....");
            getchar();
        }
        break;

        default:
            break;
    }

#endif
    RESULT_CHECK(drm_.Init(drm_fd_, display));

    return 0;
}

int DrmResources::Init(int fd, int display)
{
    drm_fd = fd;
    id = display;
    drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    res = drmModeGetResources(fd);

    for (auto i = 0; i < res->count_crtcs; i++)
        crtc_ids.push_back(res->crtcs[i]);

    for (auto i = 0; i < res->count_connectors; i++)
        connector_ids.push_back(res->connectors[i]);

    if (res->count_crtcs <= id) {
        LOGE("crtc number: %d, id : %d\n", res->count_crtcs, id);
        return -EINVAL;
    }

    int conn_id = connector_ids[id];
    int crtc_id = crtc_ids[id];

    conn = drmModeGetConnector(fd, conn_id);
    drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);

    drmModeObjectProperties *props = drmModeObjectGetProperties(fd, conn_id,
                                     DRM_MODE_OBJECT_CONNECTOR);
    property_crtc_id = get_property_id(fd, props, "CRTC_ID");
    drmModeFreeObjectProperties(props);

    props = drmModeObjectGetProperties(fd, crtc_id, DRM_MODE_OBJECT_CRTC);
    property_active = get_property_id(fd, props, "ACTIVE");
    property_mode_id = get_property_id(fd, props, "MODE_ID");
    property_out_fence_ptr = get_property_id(fd, props, "OUT_FENCE_PTR");
    drmModeFreeObjectProperties(props);

    drmModeCreatePropertyBlob(fd, &conn->modes[0],
                              sizeof(conn->modes[0]), &blob_id);

    drmModeAtomicReq *req = drmModeAtomicAlloc();
    drmModeAtomicAddProperty(req, crtc_id, property_active, 1);
    drmModeAtomicAddProperty(req, crtc_id, property_mode_id, blob_id);
    drmModeAtomicAddProperty(req, conn_id, property_crtc_id, crtc_id);
    //DDBG(drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL));
    drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    drmModeAtomicFree(req);
    LOGD("drmModeAtomicCommit setCrtc finished");
    //DDBG(crtc_id);
    //DDBG(conn_id);
    drmModePlaneRes *plane_res = drmModeGetPlaneResources(fd);

    //DDBG(plane_res->count_planes);
    for (auto i = 0; i < (int)plane_res->count_planes; ++i) {
        DrmPlane *plane = new DrmPlane(fd, plane_res->planes[i]);

        if (plane->plane_ptr->possible_crtcs & (1 << id)) {
            plane->Init();
            planes.push_back(plane);
        }
    }

    //DDBG((int)planes.size());

    return 0;
}

void DrmBackend::DeInit()
{
    drm_.DeInit();
    close(drm_fd_);
}

void DrmResources::DeInit()
{
    std::vector<DrmPlane *> planes;

    for (size_t i = 0; i < planes.size(); ++i) {
        delete planes.at(i);
    }

    planes.clear();
}

int DrmPlane::setPlane(uint32_t crtc_id, DrmFrame *frame)
{
    sdm::Rect &dest = frame->display;
    sdm::Rect &source = frame->source;
    int ret =  drmModeSetPlane(frame->fd(), plane_id, crtc_id,
                               frame->fb_id, 0,
                               dest.left, dest.top,
                               dest.getWidth(), dest.getHeight(),
                               0, 0, source.getWidth() << 16,
                               source.getHeight() << 16);

    if (ret) {
        LOGE("drmModeSetPlane failed %d(%s)", ret, strerror(errno));
    }

    return ret;
}

int DrmPlane::setPlane(uint32_t crtc_id, drmModeAtomicReq *req,
                       DrmFrame *frame)
{
    if (!property_fb_id || frame->fb_id <= 0) {
        LOGD("Error: fb_id is -1, do drmModeAddFB2 first");
        return -EINVAL;
    }

    sdm::Rect &dest = frame->display;
    sdm::Rect &source = frame->source;

    if (source.getWidth() == 0 || source.getHeight() == 0 ||
            dest.getWidth() == 0 || dest.getHeight() == 0) {
        LOGD("Error: source or display region is empty");
        return -EINVAL;
    }

    drmModeAtomicAddProperty(req, plane_id, property_crtc_id, crtc_id);
    drmModeAtomicAddProperty(req, plane_id, property_fb_id, frame->fb_id);
    //drmModeAtomicAddProperty(req, plane_id, property_crtc_x, dest.left);
    drmModeAtomicAddProperty(req, plane_id, property_crtc_x, dest.left+AVM_WIDTH); //right show camera
    drmModeAtomicAddProperty(req, plane_id, property_crtc_y, dest.top);
    drmModeAtomicAddProperty(req, plane_id, property_crtc_w, dest.getWidth());
    drmModeAtomicAddProperty(req, plane_id, property_crtc_h, dest.getHeight());
    drmModeAtomicAddProperty(req, plane_id, property_src_x, source.left);
    drmModeAtomicAddProperty(req, plane_id, property_src_y, source.top);
    drmModeAtomicAddProperty(req, plane_id, property_src_w,
                             source.getWidth() << 16);
    drmModeAtomicAddProperty(req, plane_id, property_src_h,
                             source.getHeight() << 16);

    return 0;
}

int DrmPlane::setPlane(uint32_t crtc_id, drmModeAtomicReq *req, DrmFrame *frame,
                       uint32_t alpha, uint32_t blend, uint32_t zpos)
{
    if (!property_fb_id || frame->fb_id <= 0) {
        LOGD("Error: fb_id is -1, do drmModeAddFB2 first");
        return -EINVAL;
    }

    sdm::Rect &dest = frame->display;
    sdm::Rect &source = frame->source;

    if (source.getWidth() == 0 || source.getHeight() == 0 ||
            dest.getWidth() == 0 || dest.getHeight() == 0) {
        LOGD("Error: source or display region is empty");
        return -EINVAL;
    }

    drmModeAtomicAddProperty(req, plane_id, property_crtc_id, crtc_id);
    drmModeAtomicAddProperty(req, plane_id, property_fb_id, frame->fb_id);
    drmModeAtomicAddProperty(req, plane_id, property_crtc_x, dest.left);
    drmModeAtomicAddProperty(req, plane_id, property_crtc_y, dest.top);
    drmModeAtomicAddProperty(req, plane_id, property_crtc_w, dest.getWidth());
    drmModeAtomicAddProperty(req, plane_id, property_crtc_h, dest.getHeight());
    drmModeAtomicAddProperty(req, plane_id, property_src_x, source.left);
    drmModeAtomicAddProperty(req, plane_id, property_src_y, source.top);
    drmModeAtomicAddProperty(req, plane_id, property_src_w,
                             source.getWidth() << 16);
    drmModeAtomicAddProperty(req, plane_id, property_src_h,
                             source.getHeight() << 16);
    drmModeAtomicAddProperty(req, plane_id, property_alpha,      alpha);
    drmModeAtomicAddProperty(req, plane_id, property_blend_mode, blend);
    drmModeAtomicAddProperty(req, plane_id, property_zpos,   zpos );

    return 0;
}

void DrmBackend::PageFlipHandler(int fd, unsigned int sequence,
                                 unsigned int tv_sec,
                                 unsigned int tv_usec, void *user_data)
{
    (void)fd;
    (void)sequence;
    (void)tv_sec;
    (void)tv_usec;
    (void)(user_data);
    DrmBackend *backend = (DrmBackend *)user_data;

    LOGD("page flip handler: %d", backend->fd());
}

int sync_wait(int fd, int timeout)
{
    struct pollfd fds;
    int ret;

    if (fd < 0) {
        errno = EINVAL;
        return -1;
    }

    fds.fd = fd;
    fds.events = POLLIN;

    do {
        ret = poll(&fds, 1, timeout);

        if (ret > 0) {
            if (fds.revents & (POLLERR | POLLNVAL)) {
                errno = EINVAL;
                return -1;
            }

            return 0;
        }
        else if (ret == 0) {
            errno = ETIME;
            return -1;
        }
    }
    while (ret == -1 && (errno == EINTR || errno == EAGAIN));

    return ret;
}

int DrmBackend::Post(int crtc_id, DrmPlane *plane, DrmFrame *frame)
{
    //CostTime cost(__FUNCTION__);
    int ret = -1;

#if 0 // old version set panel
    ret = plane->setPlane(crtc_id, frame);
#elif 1 // atomic commit with out fence
    int fencefd = drm_.out_fence0;

    if (fencefd > 0) {
        sync_wait(fencefd, 3000);
        //LOGD("wait done ....");
        close(fencefd);
    }

    post_req = drmModeAtomicAlloc();
    plane->setPlane(crtc_id, post_req, frame);
    drmModeAtomicAddProperty(post_req, crtc_id,
                             drm_.property_out_fence_ptr,
                             (uint64_t)&drm_.out_fence0);
    drm_.out_fence0 = -1;

    if (0 == drmModeAtomicCommit(fd(), post_req, 0x100, NULL)) {
        drmModeAtomicCommit(fd(), post_req,
                            DRM_MODE_ATOMIC_ALLOW_MODESET | DRM_MODE_ATOMIC_NONBLOCK, (void *)this);
    }
    else {
        LOGD("Error test commit failed: (%d) %s", errno, strerror(errno));
    }

    //LOGD("drm out fence: %d", (int)drm_.out_fence0);

    drmModeAtomicFree(post_req);

#endif
    return ret;
}

int DrmBackend::Post(int crtc_id, DrmPlane *plane, DrmFrame *frame, DrmPlane *plane2, DrmFrame *avm)
{
    //CostTime cost(__FUNCTION__);
    int ret = -1;

#if 0 // old version set panel
    ret = plane->setPlane(crtc_id, frame);
#elif 1 // atomic commit with out fence
    int fencefd = drm_.out_fence0;

    if (fencefd > 0) {
        sync_wait(fencefd, 3000);
        //LOGD("wait done ....");
        close(fencefd);
    }

    post_req = drmModeAtomicAlloc();
    plane->setPlane(crtc_id, post_req, frame);
    plane2->setPlane(crtc_id, post_req, avm, 255, 2, 1);
    //plane2->setPlane(crtc_id, post_req, osd);
    drmModeAtomicAddProperty(post_req, crtc_id,
                             drm_.property_out_fence_ptr,
                             (uint64_t)&drm_.out_fence0);
    drm_.out_fence0 = -1;

    if (0 == drmModeAtomicCommit(fd(), post_req, 0x100, NULL)) {
        drmModeAtomicCommit(fd(), post_req,
                            DRM_MODE_ATOMIC_ALLOW_MODESET | DRM_MODE_ATOMIC_NONBLOCK, (void *)this);
    }
    else {
        LOGD("Error test commit failed: (%d) %s", errno, strerror(errno));
    }

    //LOGD("drm out fence: %d", (int)drm_.out_fence0);

    drmModeAtomicFree(post_req);

#endif
    return ret;
}

#define MAX(a, b)  ((b) > (a) ? (b) : (a))
#define MIN(a, b)  ((b) > (a) ? (a) : (b))
#define sgn(x) ((x < 0) ? -1 : ((x > 0) ? 1 : 0)) /* macro to return the sign of a number */

int VdspXrp::Init()
{
    device = xrp_open_device(0, &status);
    if (status != XRP_STATUS_SUCCESS) {
        fprintf(stderr, "open xrp driver failed, err = %d\n", status);
        return -1;
    }

    // Initialize XRP command queue
    unsigned char XRP_XNNC_NSID[] = XRP_XNNC_NSID_INITIALIZER;
    queue = xrp_create_ns_queue(device, &XRP_XNNC_NSID, &status);
    if (status != XRP_STATUS_SUCCESS) {
        fprintf(stderr, "create xrp queue failed, err = %d\n", status);
        return -1;
    }

    if (!checkXtensaXnncName(queue)) {
        printf("XtensaDriver: Xtensa NN name mismatch!\n");
        return -1;
    }

    return 0;
}

void VdspXrp::DeInit()
{
    xrp_release_queue(queue, &status);
    xrp_release_device(device, &status);
}

int NetVdsp::Init(std::vector<void *> &pIn, std::vector<size_t> &szIn, std::vector<size_t> &szOut)
{
    bufferGroup = xrp_create_buffer_group(&status);
    if (status != XRP_STATUS_SUCCESS) {
        fprintf(stderr, "create xrp buffer group failed, err = %d\n", status);
        return -1;
    }

    for (size_t i = 0; i < szIn.size(); ++i) {
        /* create input node */
        xrp_buffer* buf = xrp_create_buffer(_xrp->device, szIn[i], pIn[i], &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "create input buffer failed, err = %d\n", status);
            return -1;
        }
        /* Add input XRP buffer to the buffer group */
        size_t idx = xrp_add_buffer_to_group(bufferGroup, buf, XRP_READ, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "add input buffer to group failed, err = %d\n", status);
            return -1;
        }
        inputBuf.push_back(buf);
        inputSz.push_back(szIn[i]);
        inputGrpIdx.push_back(idx);
    }

    for (size_t i = 0; i < szOut.size(); ++i) {
        /* Create & add output buffers to the buffer group */
        xrp_buffer* buf = xrp_create_buffer(_xrp->device, szOut[i], nullptr, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "create output buffer failed, err = %d\n", status);
            return -1;
        }
        size_t idx = xrp_add_buffer_to_group(bufferGroup, buf, XRP_WRITE, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "add output buffer to group failed, err = %d\n", status);
            return -1;
        }
        outputBuf.push_back(buf);
        outputSz.push_back(szOut[i]);
        outputGrpIdx.push_back(idx);
    }

    return 0;
}

int NetVdsp::Init(std::vector<void *> &pIn, std::vector<size_t> &szIn, std::vector<void *> &pOut, std::vector<size_t> &szOut)
{
    bufferGroup = xrp_create_buffer_group(&status);
    if (status != XRP_STATUS_SUCCESS) {
        fprintf(stderr, "create xrp buffer group failed, err = %d\n", status);
        return -1;
    }

    for (size_t i = 0; i < szIn.size(); ++i) {
        /* create input node */
        xrp_buffer* buf = xrp_create_buffer(_xrp->device, szIn[i], pIn[i], &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "create input buffer failed, err = %d\n", status);
            return -1;
        }
        /* Add input XRP buffer to the buffer group */
        size_t idx = xrp_add_buffer_to_group(bufferGroup, buf, XRP_READ, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "add input buffer to group failed, err = %d\n", status);
            return -1;
        }
        inputBuf.push_back(buf);
        inputSz.push_back(szIn[i]);
        inputGrpIdx.push_back(idx);
    }

    for (size_t i = 0; i < szOut.size(); ++i) {
        /* Create & add output buffers to the buffer group */
        xrp_buffer* buf = xrp_create_buffer(_xrp->device, szOut[i], pOut[i], &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "create output buffer failed, err = %d\n", status);
            return -1;
        }
        size_t idx = xrp_add_buffer_to_group(bufferGroup, buf, XRP_WRITE, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "add output buffer to group failed, err = %d\n", status);
            return -1;
        }
        outputBuf.push_back(buf);
        outputSz.push_back(szOut[i]);
        outputGrpIdx.push_back(idx);
    }

    return 0;
}

int NetVdsp::Init(std::vector<size_t> &szIn, std::vector<size_t> &szOut)
{
    bufferGroup = xrp_create_buffer_group(&status);
    if (status != XRP_STATUS_SUCCESS) {
        fprintf(stderr, "create xrp buffer group failed, err = %d\n", status);
        return -1;
    }

    for (size_t i = 0; i < szIn.size(); ++i) {
        /* create input node */
        xrp_buffer* buf = xrp_create_buffer(_xrp->device, szIn[i], nullptr, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "create input buffer failed, err = %d\n", status);
            return -1;
        }
        /* Add input XRP buffer to the buffer group */
        size_t idx = xrp_add_buffer_to_group(bufferGroup, buf, XRP_READ, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "add input buffer to group failed, err = %d\n", status);
            return -1;
        }
        inputBuf.push_back(buf);
        inputSz.push_back(szIn[i]);
        inputGrpIdx.push_back(idx);
    }

    for (size_t i = 0; i < szOut.size(); ++i) {
        /* Create & add output buffers to the buffer group */
        xrp_buffer* buf = xrp_create_buffer(_xrp->device, szOut[i], nullptr, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "create output buffer failed, err = %d\n", status);
            return -1;
        }
        size_t idx = xrp_add_buffer_to_group(bufferGroup, buf, XRP_WRITE, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "add output buffer to group failed, err = %d\n", status);
            return -1;
        }
        outputBuf.push_back(buf);
        outputSz.push_back(szOut[i]);
        outputGrpIdx.push_back(idx);
    }

    return 0;
}

int NetVdsp::Init(std::vector<size_t> &szIn)
{
    bufferGroup = xrp_create_buffer_group(&status);
    if (status != XRP_STATUS_SUCCESS) {
        fprintf(stderr, "create xrp buffer group failed, err = %d\n", status);
        return -1;
    }

    for (size_t i = 0; i < szIn.size(); ++i) {
        /* create input node */
        xrp_buffer* buf = xrp_create_buffer(_xrp->device, szIn[i], nullptr, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "create input buffer failed, err = %d\n", status);
            return -1;
        }
        /* Add input XRP buffer to the buffer group */
        size_t idx = xrp_add_buffer_to_group(bufferGroup, buf, XRP_READ, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "add input buffer to group failed, err = %d\n", status);
            return -1;
        }
        inputBuf.push_back(buf);
        inputSz.push_back(szIn[i]);
        inputGrpIdx.push_back(idx);
    }

    return 0;
}

void NetVdsp::DeInit()
{
    xrp_release_buffer_group(bufferGroup, &status);

    for (size_t i = 0; i < outputBuf.size(); ++i) {
        xrp_release_buffer(outputBuf[i], &status);
    }
    for (size_t i = 0; i < inputBuf.size(); ++i) {
        xrp_release_buffer(inputBuf[i], &status);
    }
}

int NetVdsp::startInf(XtensaOperation* cmd)
{
    int result;

    memset(cmd->inputIndexes, 0xff, XTENSA_OPERATION_MAX_NUM_INPUTS*sizeof(uint16_t));
    for (size_t i = 0; i < inputGrpIdx.size(); ++i) {
        cmd->inputIndexes[i] = inputGrpIdx[i];
    }

    memset(cmd->outputIndexes, 0xff, XTENSA_OPERATION_MAX_NUM_OUTPUTS*sizeof(uint16_t));
    for (size_t i = 0; i < outputGrpIdx.size(); ++i) {
        cmd->outputIndexes[i] = outputGrpIdx[i];
    }

    xrp_run_command_sync(_xrp->queue, cmd, 1 * sizeof(XtensaOperation), &result, sizeof(result),
                         bufferGroup, &status);
    if (status != XRP_STATUS_SUCCESS) {
        fprintf(stderr, "Command run failed\n");
        return -1;
    }

    return 0;
}

int NetVdsp::startInf(XtensaOperation* cmd, std::vector<void *> &pIn, std::vector<size_t> &szIn)
{
    int result;
    std::vector<xrp_buffer*> newBuf;
    for (size_t i = 0; i < pIn.size(); ++i) {
        xrp_buffer* buf = xrp_create_buffer(_xrp->device, szIn[i], pIn[i], &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "create input buffer failed, err = %d\n", status);
            return -1;
        }
        xrp_set_buffer_in_group(bufferGroup, inputGrpIdx[i], buf, XRP_READ, &status);
        newBuf.push_back(buf);
    }
    memset(cmd->inputIndexes, 0xff, XTENSA_OPERATION_MAX_NUM_INPUTS*sizeof(uint16_t));
    for (size_t i = 0; i < inputGrpIdx.size(); ++i) {
        cmd->inputIndexes[i] = inputGrpIdx[i];
    }

    memset(cmd->outputIndexes, 0xff, XTENSA_OPERATION_MAX_NUM_OUTPUTS*sizeof(uint16_t));
    for (size_t i = 0; i < outputGrpIdx.size(); ++i) {
        cmd->outputIndexes[i] = outputGrpIdx[i];
    }

    xrp_run_command_sync(_xrp->queue, cmd, 1 * sizeof(XtensaOperation), &result, sizeof(result),
                         bufferGroup, &status);
    if (status != XRP_STATUS_SUCCESS) {
        fprintf(stderr, "Command run failed\n");
        for (size_t i = 0; i < newBuf.size(); ++i)
            xrp_release_buffer(newBuf[i], &status);
        return -1;
    }

    for (size_t i = 0; i < newBuf.size(); ++i)
        xrp_release_buffer(newBuf[i], &status);

    /* set back to original buffer to avoid accident use */
    for (size_t i = 0; i < inputGrpIdx.size(); ++i) {
        xrp_set_buffer_in_group(bufferGroup, inputGrpIdx[i], inputBuf[i], XRP_READ, &status);
    }

    return 0;
}

int NetVdsp::startInf(XtensaOperation* cmd, std::vector<void *> &pIn, std::vector<size_t> &szIn,
                      std::vector<void *> &pOut, std::vector<size_t> &szOut)
{
    int result;
    std::vector<xrp_buffer*> newBuf;
    for (size_t i = 0; i < pIn.size(); ++i) {
        xrp_buffer* buf = xrp_create_buffer(_xrp->device, szIn[i], pIn[i], &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "create input buffer failed, err = %d\n", status);
            return -1;
        }
        xrp_set_buffer_in_group(bufferGroup, inputGrpIdx[i], buf, XRP_READ, &status);
        newBuf.push_back(buf);
    }
    memset(cmd->inputIndexes, 0xff, XTENSA_OPERATION_MAX_NUM_INPUTS*sizeof(uint16_t));
    for (size_t i = 0; i < inputGrpIdx.size(); ++i) {
        cmd->inputIndexes[i] = inputGrpIdx[i];
    }

    std::vector<xrp_buffer*> newOutBuf;
    for (size_t i = 0; i < pOut.size(); ++i) {
        xrp_buffer* buf = xrp_create_buffer(_xrp->device, szOut[i], pOut[i], &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "create output buffer failed, err = %d\n", status);
            return -1;
        }
        xrp_set_buffer_in_group(bufferGroup, outputGrpIdx[i], buf, XRP_WRITE, &status);
        newOutBuf.push_back(buf);
    }
    memset(cmd->outputIndexes, 0xff, XTENSA_OPERATION_MAX_NUM_OUTPUTS*sizeof(uint16_t));
    for (size_t i = 0; i < outputGrpIdx.size(); ++i) {
        cmd->outputIndexes[i] = outputGrpIdx[i];
    }

    xrp_run_command_sync(_xrp->queue, cmd, 1 * sizeof(XtensaOperation), &result, sizeof(result),
                         bufferGroup, &status);
    if (status != XRP_STATUS_SUCCESS) {
        fprintf(stderr, "Command run failed\n");
        for (size_t i = 0; i < newBuf.size(); ++i)
            xrp_release_buffer(newBuf[i], &status);
        return -1;
    }

    for (size_t i = 0; i < newBuf.size(); ++i)
        xrp_release_buffer(newBuf[i], &status);

    for (size_t i = 0; i < newOutBuf.size(); ++i)
        xrp_release_buffer(newOutBuf[i], &status);

    /* set back to original buffer to avoid accident use */
    for (size_t i = 0; i < inputGrpIdx.size(); ++i) {
        xrp_set_buffer_in_group(bufferGroup, inputGrpIdx[i], inputBuf[i], XRP_READ, &status);
    }

    for (size_t i = 0; i < outputGrpIdx.size(); ++i) {
        xrp_set_buffer_in_group(bufferGroup, outputGrpIdx[i], outputBuf[i], XRP_WRITE, &status);
    }

    return 0;
}

int NetVdsp::mapOutput(std::vector<void*> &pmap)
{

    for (size_t i = 0; i < outputBuf.size(); ++i) {
        void *buf = (void *)xrp_map_buffer(outputBuf[i], 0, outputSz[i], XRP_WRITE, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "map output buffer to userspace failed, err = %d\n", status);
            return -1;
        }
        pmap[i] = buf;
    }

    return 0;
}

int NetVdsp::unmapOutput(std::vector<void*> &pmap)
{
    for (size_t i = 0; i < outputBuf.size(); ++i) {
        xrp_unmap_buffer(outputBuf[i], pmap[i], &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "map output buffer to userspace failed, err = %d\n", status);
            return -1;
        }
    }

    return 0;
}

int NetVdsp::mapInput(std::vector<void*> &pmap)
{
    for (size_t i = 0; i < inputBuf.size(); ++i) {
        void *buf = (void *)xrp_map_buffer(inputBuf[i], 0, inputSz[i], XRP_READ, &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "map input buffer to userspace failed, err = %d\n", status);
            return -1;
        }
        pmap[i] = buf;
    }

    return 0;
}

int NetVdsp::unmapInput(std::vector<void*> &pmap)
{
    for (size_t i = 0; i < inputBuf.size(); ++i) {
        xrp_unmap_buffer(inputBuf[i], pmap[i], &status);
        if (status != XRP_STATUS_SUCCESS) {
            fprintf(stderr, "map input buffer to userspace failed, err = %d\n", status);
            return -1;
        }
    }

    return 0;
}

int set_firmware()
{
    FILE* fd = fopen("/sys/module/xrp/parameters/firmware_name", "w");
    if (!fd) {
        LOGE("cannot open sysfs file:%s\n");
        return -1;
    }

    fprintf(fd, "avm.elf");
    fclose(fd);
    return 0;
}

int main(int argc, char const *argv[])
{
    int display = 0;

    FILE *tbl_fp;
    FILE *conf_fp;

    if (argc > 2) {
        printf("Usage: vdsp-avm [maptalbe filename]\n");
        return 0;
    }

    conf_fp = fopen("/lib/firmware/avm.conf", "rb");
    if (!conf_fp) {
        LOGE("cannot open configuration file\n");
        return -1;
    }
    fseek(conf_fp, 0, SEEK_END);
    size_t conf_sz = ftell(conf_fp);
    fseek(conf_fp, 0, SEEK_SET);
    if (conf_sz < 7) {
        LOGE("configuration file len = %d\n", conf_sz);
        fclose(conf_fp);
        return -1;
    }
    char conf_buf[7];
    fread(&conf_buf, sizeof(char), 7, conf_fp);
    int idx0 = conf_buf[0] - 0x30;
    int idx1 = conf_buf[2] - 0x30;
    int idx2 = conf_buf[4] - 0x30;
    int idx3 = conf_buf[6] - 0x30;
    if ((idx0<0 || idx0>3) || (idx1<0 || idx1>3) || (idx2<0 || idx2>3) || (idx3<0 || idx3>3)) {
        printf("camera index configuration error\n");
        return -1;
    }

    if (argc != 2) {
        printf("maptable file not specified, using default /lib/firmware/maptable.bin\n");
        tbl_fp = fopen("/lib/firmware/maptable.bin", "rb");
    } else {
        tbl_fp = fopen(argv[1], "rb");
    }

    if (!tbl_fp) {
        LOGE("cannot open mapping table file\n");
        return -1;
    }
    fseek(tbl_fp, 0, SEEK_END);
    size_t tbl_sz = ftell(tbl_fp);
    fseek(tbl_fp, 0, SEEK_SET);
    if (tbl_sz <= 0) {
        LOGE("mapping table file len = %d\n", tbl_sz);
        fclose(tbl_fp);
        return -1;
    }
    printf("mapping table file len = %d\n", tbl_sz);

    if (set_firmware()) {
        printf("set firmware to avm failed\n");
        return 0;
    }

    signal(SIGINT, sigint_handler);

    LOGD("start runing on display %d", display);
    DrmBackend backend;

    if (backend.Init(display)) {
        LOGE("Init drm failed\n");
        return -1;
    }

    int crtc = backend.drm_.crtc_ids[display];

    int ret;

    // init avm layer, tripple buffer
    DrmFrame *avms[3];
    for (auto m = 0; m < 3; m++) {
        avms[m] = new DrmFrame(backend.fd());

        //ret = avms[m]->createDumbBuffer(AVM_WIDTH, AVM_HEIGHT, DRM_FORMAT_RGB888, 0, 0);
        ret = avms[m]->createDumbBuffer(AVM_WIDTH, AVM_HEIGHT, DRM_FORMAT_BGR888, 0, 0);
        if (ret) {
            LOGD("error create avm dumb buffer");
            backend.DeInit();
            return -1;
        }
        // map to userspace
        avms[m]->MapBO();
        sdm::Rect &r2 = avms[m]->display;
        r2 = sdm::Rect(r2.left, r2.top, r2.right, r2.bottom);
    }

    uint32_t front_buf = 0;

    for (auto p : backend.drm_.planes) {
        LOGD("for");
        printf("p->plane_id=%d\n", p->plane_id);
    }

    char video_name[128];

    vector<Cam *> cams;
    for (auto m = 0; m < 4; m++) {
        memset(video_name, 0, sizeof video_name);
        snprintf(video_name, sizeof video_name, "/dev/video-evs%d", m);
        printf("start dev:%s cam_open()\n", video_name);
        Cam *cam = new Cam(video_name);

        printf("start cam_set_format()\n");
        ret = cam->set_format(V4L2_PIX_FMT_YUYV, IMAGE_WIDTH, IMAGE_HEIGHT);

        printf("start cam_init_buffer()\n");
        int bufferLength = cam->init_buffer();

        printf("start cam_set_stream()\n");
        ret = cam->set_stream(1);
        cams.push_back(cam);
    }

    DrmFrame *frame[4];
    for (auto m = 0; m < 4; m++) {
        int dmabuf = cams[0]->dma_fd[m];
        frame[m] = new DrmFrame(backend.fd());

        ret = frame[m]->ImportBuffer(IMAGE_WIDTH, IMAGE_HEIGHT, cams[0]->vb_len[m] / IMAGE_HEIGHT,
									DRM_FORMAT_YUYV, dmabuf, 0, 0);

        if (ret) {
            LOGD("error create dumb buffer");
            backend.DeInit();
            return -1;
        }
    }

    printf("start vdsp init\n");
    VdspXrp *xrp = new VdspXrp();
    ret = xrp->Init();
    if (ret) {
        LOGE("Init xrp failed\n");
        return -1;
    }

    NetVdsp *avm_load_map = new NetVdsp(xrp);
    vector<size_t> map_sz {tbl_sz};
    ret = avm_load_map->Init(map_sz);
    if (ret) {
        LOGE("Init load map func failed\n");
        fclose(tbl_fp);
        return -1;
    }

    std::vector<void*> map_input(1);
    ret = avm_load_map->mapInput(map_input);
    if (ret) {
        LOGE("map mapping table failed\n");
        fclose(tbl_fp);
        return -1;
    }
    // read in mapping file
    fread(map_input[0], 1, tbl_sz, tbl_fp);
    fclose(tbl_fp);
    avm_load_map->unmapInput(map_input);

    XtensaOperation *load_cmd = new XtensaOperation();
    load_cmd->opType = XFL_LOAD_COEF; // load map table
    printf("load mapping table\n");
    ret = avm_load_map->startInf(load_cmd);
    if (ret) {
        LOGE("load mapping table failed\n");
        return -1;
    }

    NetVdsp *avm_net = new NetVdsp(xrp);

    vector<void *> avm_buf {avms[0]->mapped_vaddr};
    //vector<size_t> avm_sz {AVM_WIDTH * AVM_HEIGHT * 3};
    vector<size_t> avm_sz {4};
    vector<void *> cam_init_bufs;
    vector<size_t> cam_init_lens;
    for (auto m = 0; m < 4; m++) {
        int idx = -1;
        uint8_t *buf = NULL;
        int len;
        len = cams[m]->get_frame(&buf, &idx);
        cam_init_bufs.push_back(buf);
        cam_init_lens.push_back(4);
        //cam_init_lens.push_back(len);
    }

    ret = avm_net->Init(cam_init_bufs, cam_init_lens, avm_buf, avm_sz);
    if (ret) {
        LOGE("Init avm net failed\n");
        return -1;
    }

    /* prepare all commands */
    XtensaOperation *cmd    = new XtensaOperation();
    cmd->opType = XFL_START_INF; // start one avm frame
    struct XtensaParams *p_params = (struct XtensaParams *)(&(cmd->params));
    xi_rect *rect_crop = &(p_params->crop_rect); // crop the input fullimage
    xi_size *size_data = &(p_params->data_sz);   // input fullimage size
    int32_t       *fmt = &(p_params->input_fmt);
    rect_crop->x       = 0;
    rect_crop->y       = 0;
    rect_crop->width   = IMAGE_WIDTH;
    rect_crop->height  = IMAGE_HEIGHT;
    size_data->width   = IMAGE_WIDTH;
    size_data->height  = IMAGE_HEIGHT;
    *fmt               = XFL_INPUT_YUYV;

    int64_t startTime, endTime;

    printf("start avm\n");
    while (!c_terminate) {
        DrmFrame *avm = avms[front_buf];
        int vb_idx = -1;

        vector<void *> cam_bufs;
        vector<size_t> cam_lens;
        for (auto m = 0; m < 4; m++) {
            int idx = -1;
            uint8_t *buf = NULL;
            int len;
            len = cams[m]->get_frame(&buf, &idx);
            if (m == 0) vb_idx = idx; // show camera 0 to screen
            cam_bufs.push_back(buf);
            //cam_lens.push_back(len);
            cam_lens.push_back(4);
        }

        /* avm idx = cam idx + 1 */
        vector<void *> avm_in_bufs;
        avm_in_bufs.push_back(cam_bufs[idx0]);
        avm_in_bufs.push_back(cam_bufs[idx1]);
        avm_in_bufs.push_back(cam_bufs[idx2]);
        avm_in_bufs.push_back(cam_bufs[idx3]);

        vector<void *> avm_out_buf {avm->mapped_vaddr};
        //vector<size_t> avm_out_sz {AVM_WIDTH * AVM_HEIGHT * 3};
        vector<size_t> avm_out_sz {4};

        startTime = getTimeMsec();
        ret = avm_net->startInf(cmd, avm_in_bufs, cam_lens, avm_out_buf, avm_out_sz);
        endTime = getTimeMsec();
        //printf("DSP: %ld ms, out[%d], ret=%d\n", endTime-startTime, front_buf, ret);

        backend.Post(crtc, backend.drm_.planes[1], frame[vb_idx], backend.drm_.planes[0], avm);
        front_buf = (front_buf != 2) ? (front_buf+1) : 0;
    }

    avm_net->DeInit();
    delete avm_net;

    avm_load_map->DeInit();
    delete avm_load_map;

    xrp->DeInit();
    delete xrp;

    delete cmd;

    for (auto m = 0; m < 4; m++) {
        ret = cams[m]->set_stream(0);
        ret = cams[m]->release_buffer();
    }

    usleep(10000);

    printf("remove camera 4 framebuffers\n");
    for (auto m = 0; m < 4; m++)
        frame[m]->Destroy();

    printf("remove avm 3 framebuffers\n");
    for (auto m = 0; m < 3; m++)
        avms[m]->Destroy();

    backend.DeInit();

    return 0;
}

