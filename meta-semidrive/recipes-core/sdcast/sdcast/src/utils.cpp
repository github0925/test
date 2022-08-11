#include <sys/ioctl.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm/drm_fourcc.h>

#include <linux/dma-buf.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

#define DRM_COMMAND_BASE 0x40
#define DRM_IOCTL_BASE			'd'
#define DRM_IO(nr)			_IO(DRM_IOCTL_BASE,nr)
#define DRM_IOR(nr,type)		_IOR(DRM_IOCTL_BASE,nr,type)
#define DRM_IOW(nr,type)		_IOW(DRM_IOCTL_BASE,nr,type)
#define DRM_IOWR(nr,type)		_IOWR(DRM_IOCTL_BASE,nr,type)

#define DRM_IOCTL_SEMIDRIVE_MAP_PHYS 		DRM_IOWR(DRM_COMMAND_BASE + 1, struct drm_buffer_t)
#define DRM_IOCTL_SEMIDRIVE_UNMAP_PHYS 		DRM_IOWR(DRM_COMMAND_BASE + 2, struct drm_buffer_t)

#define DRM_IOCTL_SEMIDRIVE_EXPORT_DMABUF	DRM_IOWR(DRM_COMMAND_BASE + 3, struct drm_buffer_t)

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
	//LOGD("phys: 0x%lx ----> dmabuf: %d", addr, *dma_fd);

#undef PAGE_ALIGN

	return 0;
}

void set_color(void *vaddr, int w, int h, uint32_t color) {
	uint32_t *p = (uint32_t*)vaddr;
	for (auto i =0;i < h; i++)
		for (auto j = 0; j < w; j++) {
			p[i * w + j] = color;
		}
}


