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
#include <sys/time.h>
#include <sys/ioctl.h>
#include "DrmDisplay.h"


#define RESULT_CHECK(x) do { \
    int ret = x; \
    if (ret) { \
        LOGD(#x " Error: %d (%s)", ret, strerror(-ret)); \
        return ret; \
        } \
    } while(0)

void setColor(void *vaddr, int w, int h, uint32_t color) {
	uint32_t *p = (uint32_t*)vaddr;
	for (auto i =0;i < h; i++)
		for (auto j = 0; j < w; j++) {
			p[i * w + j] = color;
		}
}

int drm_ioctl_open(void) {
	int fd = drmOpen("semidrive", nullptr);
	if (fd < 0)
		LOGD("open semidrive drm failed: %d(%s)", errno, strerror(errno));

	return fd;
}

int drm_ioctl_get_phy_addr(int fd, int dmabuf, drm_buffer_t *data) {
	int ret;
	data->buf_handle = dmabuf;
	ret = ioctl(fd, DRM_IOCTL_SEMIDRIVE_MAP_PHYS, data);
	return ret;
}

int drm_ioctl_release_phy_addr(int fd, drm_buffer_t *data) {
	int ret;
	ret = ioctl(fd, DRM_IOCTL_SEMIDRIVE_UNMAP_PHYS, data);
	return ret;
}

int drm_ioctl_export_dmabuf(int fd, uint64_t addr, size_t size, int *dma_fd, int flags) {
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

#define ALIGN(x, y) (((x) + y - 1) & (~(y - 1)))


int DrmFrame::getBppByFormat(int format) {
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

int DrmFrame::fillBO(int width, int height, int stride, int format, uint32_t gem_handles, int offset)
{

	bo.format = format;
	bo.width = width;
    bo.height = height;
	bo.offsets[0] = offset;
    bo.pitches[0] = stride;
	bo.gem_handles[0] = gem_handles;

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
		bo.pitches[0] = stride * 4;
		break;
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB888:
		bo.pitches[0] = stride * 3;
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
		bo.pitches[0] = stride * 2;
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

int DrmFrame::createBO(uint32_t virtual_width, uint32_t virtual_height, int bpp)
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
	bo.width = arg.width;
	bo.height = virtual_height;
	bo.offsets[0] = 0;
    bo.pitches[0] = arg.pitch;
	DDBG(bo.pitches[0]);

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
    	LOGE("Failed to close gem handle %d ,%d(%s)", ret1, errno, strerror(errno));

	if (bo.fb_id > 0) {
	    drmModeRmFB(drm_fd, bo.fb_id);
		bo.fb_id = -1;
	}

	if (isDumb)
		DestroyBO();
	else
		close(prime_fd_);
}

int DrmFrame::addFrameBuffer(int fbc_mode) {
if (fbc_mode) {
	if (drmModeAddFB2WithModifiers(drm_fd, bo.width, bo.height, bo.format,
                bo.gem_handles, bo.pitches, bo.offsets, bo.modifier,
                &(bo.fb_id), DRM_MODE_FB_INTERLACED )) {
				LOGD("failed to add fb: %s\n", strerror(errno));
				return -1;
	}
} else {
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
	int bpp;
	source = sdm::Rect(0, 0, width, height);
	display = source;
	bpp = getBppByFormat(format);
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
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		virtual_height = height * 2;
		break;

	default:
		virtual_height = height;
		break;
	}
	// virtual_height *= 2;
	createBO(virtual_width, virtual_height, bpp);

	isDumb = true;
	int stride = bo.pitches[0] * 8 / bpp;
	return fillBO(width, height, stride, format, bo.gem_handles[0], offset);
}

int DrmFrame::ImportBuffer(unsigned int width, unsigned int height, int stride,
			unsigned int format, int prime_fd, int fbc, int offset) {
	uint32_t gem_handle;
	source = sdm::Rect(0, 0, width, height);
	display = source;
	prime_fd_ = prime_fd;
	int ret = drmPrimeFDToHandle(drm_fd, prime_fd, &gem_handle);
	if (ret) {
		LOGE("failed to import prime fd %d ret=%d", prime_fd, ret);
		return ret;
	}
	DDBG(gem_handle);
	isDumb = false;
	return fillBO(width, height, stride, format, gem_handle, offset);
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

int DrmPlane::Init() {

    getProperties();
    if (!property_fb_id) {
        LOGD("plane properties get failed");
        return -EINVAL;
    }

    return 0;
}
void DrmPlane::getProperties() {
    /* get plane properties */
    drmModeObjectProperties* props = drmModeObjectGetProperties(drm_fd, plane_id, DRM_MODE_OBJECT_PLANE);
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

int DrmBackend::Init(int display) {
	drm_fd_ = drmOpen("semidrive", nullptr);
    if (drm_fd_ < 0) {
        LOGD("Error: open %s %d(%s)", "semidrive", drm_fd_, strerror(-drm_fd_));
        return drm_fd_;
    }
#ifdef CONFIG_AUTH_CHECK
	drm_magic_t drm_magic = 0;
	#define MAGIC_FIFO "/data/fifo1"
	switch (display) {
	case 0: //drm master
	{
		char buf[1024] = {};
		int fd, len;
		if(mkfifo(MAGIC_FIFO, 0666) < 0 && errno!=EEXIST)
	        LOGE("Create FIFO Failed");
		if((fd = open(MAGIC_FIFO, O_RDONLY)) < 0) {
	        LOGE("Open FIFO Failed");
	        return -1;
	    }
	    while((len = read(fd, buf, 1024)) > 0) // 读取FIFO管道
	        LOGD("Read message: %s", buf);
	    close(fd);  // 关闭FIFO文件

		long num = strtoul(buf, 0, 10);
		if (num >= 0)
			drm_magic = (drm_magic_t)num;
		LOGD("num = %u", drm_magic);
		DDBG(drmAuthMagic(drm_fd_, drm_magic));
	}
	break;
	case 1: //drm auth
	{
		int fd, n;
		char buf[1024] = {};
	    if((fd = open(MAGIC_FIFO, O_WRONLY)) < 0) {
	        LOGE("Open FIFO Failed");
	        return -1;
	    }
		DDBG(drmGetMagic(drm_fd_, &drm_magic));
		LOGD("drmGetMagic num %d", drm_magic);
		n=sprintf(buf, "%lu", (unsigned long)drm_magic);
		if(write(fd, buf, n+1) < 0) {
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

int DrmResources::Init(int fd, int display) {
    drm_fd = fd;
	id = display;
    drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    res = drmModeGetResources(fd);

	for(auto i = 0; i < res->count_crtcs; i++)
		crtc_ids.push_back(res->crtcs[i]);
	for(auto i = 0; i < res->count_connectors; i++)
		connector_ids.push_back(res->connectors[i]);

	if (res->count_crtcs <= id) {
		LOGE("crtc number: %d, id : %d\n", res->count_crtcs, id);
		return -EINVAL;
	}

	int conn_id = connector_ids[id];
	int crtc_id = crtc_ids[id];

    conn = drmModeGetConnector(fd, conn_id);
    drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);

    drmModeObjectProperties *props = drmModeObjectGetProperties(fd, conn_id, DRM_MODE_OBJECT_CONNECTOR);
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
    DDBG(drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL));
    drmModeAtomicFree(req);
    LOGD("drmModeAtomicCommit setCrtc finished");
    DDBG(crtc_id);
    DDBG(conn_id);
    drmModePlaneRes *plane_res = drmModeGetPlaneResources(fd);
    DDBG(plane_res->count_planes);
	for (auto i = 0; i < (int)plane_res->count_planes; ++i) {
		DrmPlane *plane = new DrmPlane(fd, plane_res->planes[i]);
		if (plane->plane_ptr->possible_crtcs & (1 << id)) {
			plane->Init();
			planes.push_back(plane);
		}
	}
	DDBG((int)planes.size());

    return 0;
}

void DrmBackend::DeInit()
{
	drm_.DeInit();
	close(drm_fd_);
}

void DrmResources::DeInit()
{
	std::vector<DrmPlane*> planes;
	for (size_t i = 0; i < planes.size(); ++i)
	{
		delete planes.at(i);
	}
	planes.clear();
}

bool DrmPlane::supportThisFormat(uint32_t drm_format)
{
	for (uint32_t i = 0; i < plane_ptr->count_formats; i++) {
		if (plane_ptr->formats[i] == drm_format)
			return true;
	}
	return false;
}

int DrmPlane::setZorder(int zpos)
{
	LOGD("plane_id %d set zpos %d", plane_id, zpos);
	return drmModeObjectSetProperty(drm_fd, plane_id, DRM_MODE_OBJECT_PLANE, property_zpos, zpos);
}

int DrmPlane::setPlane(uint32_t crtc_id, DrmFrame *frame)
{
	sdm::Rect &dest = frame->display;
    sdm::Rect &source = frame->source;
	int ret =  drmModeSetPlane(frame->fd(), plane_id, crtc_id,
									frame->fb_id, 0,
									dest.left, dest.top,
									dest.getWidth(), dest.getHeight(),
									 source.left << 16, source.top << 16, source.getWidth() << 16,
									source.getHeight() << 16);
	if (ret) {
		LOGE("drmModeSetPlane failed %d(%s)", ret, strerror(errno));
	}
	return ret;
}

int DrmPlane::setPlane(uint32_t crtc_id, drmModeAtomicReq *req, DrmFrame *frame) {
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
    drmModeAtomicAddProperty(req, plane_id, property_src_x, source.left << 16);
    drmModeAtomicAddProperty(req, plane_id, property_src_y, source.top << 16);
    drmModeAtomicAddProperty(req, plane_id, property_src_w, source.getWidth() << 16);
    drmModeAtomicAddProperty(req, plane_id, property_src_h, source.getHeight() << 16);

	drmModeAtomicAddProperty(req, plane_id, property_alpha, frame->alpha);
	drmModeAtomicAddProperty(req, plane_id, property_blend_mode, frame->blend_mode);
	drmModeAtomicAddProperty(req, plane_id, property_zpos, frame->z_order);

    return 0;
}

void DrmBackend::PageFlipHandler(int fd, unsigned int sequence, unsigned int tv_sec,
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
		} else if (ret == 0) {
			errno = ETIME;
			return -1;
		}
	} while (ret == -1 && (errno == EINTR || errno == EAGAIN));

	return ret;
}

int DrmBackend::PostAsync(int crtc_id, DrmPlane *plane, DrmFrame *frame, int *fencefd) {
	CostTime cost(__FUNCTION__);
	int tempFd = -1;
	if (*fencefd > 0) {
		sync_wait(*fencefd, 3000);
		LOGD("wait done ....");
		close(*fencefd);
	}

	post_req = drmModeAtomicAlloc();
	plane->setPlane(crtc_id, post_req, frame);
    drmModeAtomicAddProperty(post_req, crtc_id,
                                   drm_.property_out_fence_ptr,
                                   (uint64_t)&tempFd);


	if (0 == drmModeAtomicCommit(fd(), post_req, 0x100, NULL)) {
		drmModeAtomicCommit(fd(), post_req, DRM_MODE_ATOMIC_ALLOW_MODESET | DRM_MODE_ATOMIC_NONBLOCK, (void *)this);
	} else {
		LOGD("Error test commit failed: (%d) %s", errno, strerror(errno));
	}
	LOGD("drm out fence: %d", (int)tempFd);
	*fencefd = tempFd;
	drmModeAtomicFree(post_req);
}

int DrmBackend::Post(int crtc_id, DrmPlane *plane, DrmFrame *frame){
	CostTime cost(__FUNCTION__);
	int ret = -1;

#if 1 // old version set panel
	ret = plane->setPlane(crtc_id, frame);
#elif 1 //atomic commit, with pageflip handler
	fd_set fds;
	static drmEventContext event_context = {
			.version = DRM_EVENT_CONTEXT_VERSION,
			.page_flip_handler = DrmBackend::PageFlipHandler,
			//.vblank_handler = vblank_handler,
	};
	post_req = drmModeAtomicAlloc();
	plane->setPlane(crtc_id, post_req, frame);

	if (drmModeAtomicCommit(fd(), post_req, 0x100, NULL)) {
		LOGD("Error test commit failed: (%d) %s", errno, strerror(errno));
		return -1;
	}

	drmModeAtomicCommit(fd(), post_req, DRM_MODE_PAGE_FLIP_EVENT, (void *)this);
	DDBG((int)drm_.out_fence0);
	drmModeAtomicFree(post_req);
	FD_ZERO(&fds);
	FD_SET(fd(), &fds);
	do {
  		ret = select(fd() + 1, &fds, NULL, NULL, NULL);
  	} while (ret == -1 && errno == EINTR);

  	if (FD_ISSET(fd(), &fds))
  		drmHandleEvent(fd(), &event_context);
	else
	{
		LOGD("FD is not set");
	}

#endif
	return ret;
}

#ifdef  CONFIG_TEST
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
class MyThread {
private:
    std::thread th_;
    static void *start_thread(void *arg) {
        MyThread *thread = (MyThread*)arg;
        thread->run();
        return (void *)0;
    }
public:
    virtual ~MyThread() {

    }
    virtual void run() {
        LOGE("success\n");
    }
    virtual void start() {
        LOGD("debug");
        th_ = std::thread(&MyThread::start_thread, (void *)this);
    }
    void join() {
        if (th_.joinable())
            th_.join();
    }
};

class Walker: public MyThread {
public:
	Walker(DrmBackend *drm, int id) {
		isDead = false;
		backend = drm;
		id_ = id;
		frame = new DrmFrame(backend->fd());
		frame->createDumbBuffer(200, 200, DRM_FORMAT_XBGR8888,0 ,0);
		frame->MapBO();
		pos = frame->display;
	}

	void setColor(uint32_t color) {
		uint32_t *p = (uint32_t*)frame->mapped_vaddr;
		for (auto i =0;i < 200; i++)
			for (auto j = 0; j < 200; j++) {
				p[i * 200 + j] = color;
			}
	}

	~Walker() {
		frame->Destroy();
	}

	int move(int x, int y) {
		pos = sdm::Rect(pos.left + x, pos.top + y, pos.right + x, pos.bottom + y);
		frame->display = pos;
		LOGD("display at  (%d, %d, %d, %d)",
			frame->display.left, frame->display.top,
			frame->display.right, frame->display.bottom);
		return 0;
	}

	int walk() {
		int crtc = backend->drm_.crtc_ids[0];
		DrmPlane *p = backend->drm_.planes[id_];
		return backend->Post(crtc, p, frame);
	}

	void run() {
		while(!isDead) {
			static int x = 10, y = 10;
			// std::this_thread::sleep_for(std::chrono::milliseconds(16));
			if (pos.left + x < 0 || pos.right + x > SCREEN_WIDTH) {
				x = -x;
			}
			if (pos.top + y < 0 || pos.bottom + x > SCREEN_HEIGHT) {
				y = -y;
			}
			move(x, y);
			int ret  = walk();
			if (isDead) {
				LOGE("post failed: ret = %d", ret);
				isDead = true;
			}
		}
	}
private:
	bool isDead;
	int id_;
	sdm::Rect pos;
	DrmFrame *frame;
	DrmBackend *backend;
};

//######################

int test_nv12(int argc, char const *argv[])
{
	int display = 0;

	if (argc == 2) {
		display = (int)atoi(argv[1]);
	}

	LOGD("start runing on display %d", display);
	DrmBackend backend;
	if (backend.Init(display)) {
		LOGE("Init drm failed\n");
		return -1;
	}
	int crtc = backend.drm_.crtc_ids[display];

	DrmFrame *frame = new DrmFrame(backend.fd());
	int ret = frame->createDumbBuffer(400, 300, DRM_FORMAT_NV12, 0, 0);
	if (ret) {
		LOGD("error create dumb buffer");
		backend.DeInit();
		return -1;
	}
	// map to userspace
	frame->MapBO();
	// setColor(frame->mapped_vaddr, 400, 300, 0xffffff00);

	sdm::Rect &r = frame->display;
	r = sdm::Rect(r.left + 50, r.top + 50, r.right + 50, r.bottom + 50);
	for (auto p: backend.drm_.planes) {
		DDBG(p->plane_id);
	}
	backend.Post(crtc, backend.drm_.planes[0], frame);

	getchar();
	int n = 1000;

	std::unique_ptr<DrmFrame> on_screen;
	std::unique_ptr<DrmFrame> new_frame;
	while (n--) {
		int prime_fd;
		new_frame.reset(new DrmFrame(backend.fd()));
		// // int drmPrimeHandleToFD(int fd, uint32_t handle, uint32_t flags, int *prime_fd);
		// ret = drmPrimeHandleToFD(backend.fd(), frame->bo.gem_handles[0], DRM_CLOEXEC, &prime_fd);
		// if (ret) {
		// 	LOGE("drmPrimeHandleToFD failed: %d: %d(%s)", prime_fd, errno, strerror(errno));
		// 	if (errno == ENOSYS) {
		// 		LOGE("drm drive do not have prime_handle_to_fd function ops");
		// 	}
		// }
		long phys_address = 0x90000000;

		drm_ioctl_export_dmabuf(backend.fd(), phys_address, 400 * 300 * 2, &prime_fd, 0);
		DDBG(prime_fd);
		new_frame->ImportBuffer(400, 300, 400, DRM_FORMAT_NV12, prime_fd, 0, 0);
		DDBG(new_frame->bo.pitches[0]);
		sdm::Rect &r = new_frame->display;
		r = sdm::Rect(r.left + 150, r.top + 150, r.right + 150, r.bottom + 150);
		backend.Post(crtc, backend.drm_.planes[0], new_frame.get());
		getchar();
		on_screen = std::move(new_frame);
	}
	frame->Destroy();

	backend.DeInit();
	return 0;
}

int main(int argc, char const *argv[])
{
	int display = 0;

	if (argc == 2) {
		display = (int)atoi(argv[1]);
	}

	test_nv12(argc, argv);

    return 0;
}


int main1(int argc, char const *argv[])
{
    (void)argc;
    (void)argv;
	LOGD("start runing");
	DrmBackend backend;
	if (backend.Init(0)) {
		LOGE("Init drm failed\n");
		return -1;
	}

	std::vector<Walker *> walkers;
	#if 0
	int num = backend.drm_.planes.size();
	(void)num;
	for (int i = 0; i < num; i++) {
		Walker *w = new Walker(&backend, i);
		walkers.push_back(w);
		w->setColor(0xff0f00f0 | (0xff << i * 3));
		w->move(100 * i, 400 * i);
		w->start();
	}
	walkers[0]->join();
	while (1);
	#else
	Walker *w = new Walker(&backend, 1);

	w->setColor(0xff00ff00);
	w->start();
	w->join();
	#endif

	backend.DeInit();
    return 0;
}
#endif
