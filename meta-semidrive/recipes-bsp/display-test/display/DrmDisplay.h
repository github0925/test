#ifndef __DRM_DISPLAY_H
#define __DRM_DISPLAY_H
#include "debug.h"

#include <unistd.h>
#include <poll.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm/drm_fourcc.h>
#include <linux/dma-buf.h>
#include <condition_variable>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

typedef struct hwc_drm_bo {
  uint32_t width;
  uint32_t height;
  uint32_t format; /* DRM_FORMAT_* from drm_fourcc.h */
  uint32_t pitches[4];
  uint32_t offsets[4];
  uint32_t gem_handles[4];
  uint32_t fb_id;
  int acquire_fence_fd;
  void *priv;
  uint64_t modifier[4];
} hwc_drm_bo_t;

enum {
	DRM_MODE_BLEND_PIXEL_NONE = 0,
	DRM_MODE_BLEND_PREMULTI,
	DRM_MODE_BLEND_COVERAGE
};

namespace sdm {
class Rect {
public:
    Rect(): left(0), top(0), right(0), bottom(0)  {
    }
    Rect(int l, int t, int r, int b): left(l), top(t), right(r), bottom(b) {

    }
    Rect(Rect &r) {
        left = r.left;
        top = r.top;
        right = r.right;
        bottom = r.bottom;
    }

    Rect& operator=(const Rect& r) {
        if (this != &r) {
            left = r.left;
            top = r.top;
            right = r.right;
            bottom = r.bottom;
        }
        return *this;
    }

    inline int getWidth() {return right - left;}
    inline int getHeight() {return bottom - top;}
    int left;
    int top;
    int right;
    int bottom;
};
}

class FenceFd {
public:
	FenceFd(int fd):fd_(fd) {}
	~FenceFd() {
		if (fd_) {
			LOGD("release fd...");
			close(fd_);
		}
	}

	int  Wait(int ms) {
		return sync_wait(fd_, ms);
	}
private:
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
private:
	int fd_;
};

class DrmFrame {
public:
    DrmFrame(int fd): fb_id(-1), mapped_vaddr(NULL), isDumb(false), drm_fd(fd), blend_mode(0), alpha(0xFF), z_order(0) {};
    ~DrmFrame(){
        LOGD("call");
        Destroy();
    };
    int createDumbBuffer(unsigned int width, unsigned int height, unsigned int format,
				int fbc_mode, int offset);
    int ImportBuffer(unsigned int width, unsigned int height, int stride,
				unsigned int format, int dma_buf, int fbc, int offset);
	int MapBO();
	void unMapBO();
	void Destroy();
    int getBppByFormat(int format);

	inline int fd() {
		return drm_fd;
	}
	void *planes[4] = {NULL};
    sdm::Rect display;
    sdm::Rect source;
    uint32_t alpha;
    uint32_t blend_mode;
    uint32_t z_order;

	int fb_id;
	size_t sz_bo;
	void *mapped_vaddr;
	std::unique_ptr<FenceFd> release_fd;
	hwc_drm_bo_t  bo;
	int addFrameBuffer(int fbc);
protected:
	int createBO(uint32_t width, uint32_t virtual_height, int bpp);
	int fillBO(int width, int height, int stride, int format, uint32_t gem_handles, int offset);
	void DestroyBO();
    int prime_fd_;
	bool isDumb;
    int drm_fd;
};

class DrmPlane {
public:
    DrmPlane(int fd, int id):drm_fd(fd), plane_id(id), busy(0) {
		plane_ptr = drmModeGetPlane(drm_fd, plane_id);
    }
    ~DrmPlane() {
        busy = 0;
        drmModeFreePlane(plane_ptr);
        LOGD("Plane delete");
    }
    int Init();
	int setPlane(uint32_t crtc_id, drmModeAtomicReq *req, DrmFrame *frame);
	int setPlane(uint32_t crtc_id, DrmFrame *frame);
    bool supportThisFormat(uint32_t drm_format);
    void getProperties();
    int setZorder(int zpos);

public:
    int drm_fd;
	int crtc_id;
	uint32_t property_fb_id;
    uint32_t property_crtc_id;
	uint32_t property_crtc_x;
	uint32_t property_crtc_y;
	uint32_t property_crtc_w;
	uint32_t property_crtc_h;
	uint32_t property_src_x;
	uint32_t property_src_y;
	uint32_t property_src_w;
	uint32_t property_src_h;

    uint32_t property_alpha;
    uint32_t property_blend_mode;
    uint32_t cap_mask;
    uint32_t property_zpos;
    int plane_id;
    int busy;
    drmModePlanePtr plane_ptr;
};

class DrmResources {
public:
    DrmResources(){};
    ~DrmResources(){};
    int Init(int fd, int active_id);
    void DeInit();

public:
    int drm_fd;
   	uint32_t property_crtc_id;
	uint32_t property_mode_id;
	uint32_t property_active;
    uint64_t property_out_fence_ptr;

    drmModeConnector *conn;
    drmModeRes *res;
    std::vector<DrmPlane*> planes;
    std::vector<int> crtc_ids;
	uint64_t out_fence0;
    std::vector<int> connector_ids;

    uint32_t blob_id;
	int id;
};

class DrmBackend {
public:
    int Init(int id);
    int Post(int crtc_id, DrmPlane *plane, DrmFrame *frame);
    int PostAsync(int crtc_id, DrmPlane *plane, DrmFrame *frame, int *fenceFd);
    void DeInit();
	inline int fd() {
		return drm_fd_;
	}
	drmModeAtomicReq *post_req;
    int drm_fd_;
    DrmResources drm_;
public:
	static void PageFlipHandler(int fd, unsigned int sequence, unsigned int tv_sec,
  		unsigned int tv_usec, void *user_data);
};

typedef struct drm_buffer_t {
    uint32_t size;
    uint64_t phys_addr;
    uint64_t base;
    uint64_t virt_addr;
    uint64_t dma_addr;
    uint64_t attachment;
    uint64_t sgt;
    int32_t	 buf_handle;
} drm_buffer_t;

#define DRM_COMMAND_BASE 0x40
#define DRM_IOCTL_BASE			'd'
#define DRM_IO(nr)			_IO(DRM_IOCTL_BASE,nr)
#define DRM_IOR(nr,type)		_IOR(DRM_IOCTL_BASE,nr,type)
#define DRM_IOW(nr,type)		_IOW(DRM_IOCTL_BASE,nr,type)
#define DRM_IOWR(nr,type)		_IOWR(DRM_IOCTL_BASE,nr,type)

#define DRM_IOCTL_SEMIDRIVE_MAP_PHYS 		DRM_IOWR(DRM_COMMAND_BASE + 1, struct drm_buffer_t)
#define DRM_IOCTL_SEMIDRIVE_UNMAP_PHYS 		DRM_IOWR(DRM_COMMAND_BASE + 2, struct drm_buffer_t)

#define DRM_IOCTL_SEMIDRIVE_EXPORT_DMABUF	DRM_IOWR(DRM_COMMAND_BASE + 3, struct drm_buffer_t)
int drm_ioctl_open(void);

int drm_ioctl_get_phy_addr(int fd, int dmabuf, drm_buffer_t *data);

int drm_ioctl_release_phy_addr(int fd, drm_buffer_t *data);

int drm_ioctl_export_dmabuf(int fd, uint64_t addr, size_t size, int *dma_fd, int flags);
void setColor(void *vaddr, int w, int h, uint32_t color);

/* SEMIDRIVE frame buffer modifiers */
#define DRM_FORMAT_MOD_VENDOR_SEMIDRIVE 0x08
/* Semidrive 32 bpp 8x8 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_8X8_TILE        fourcc_mod_code(SEMIDRIVE, 1)
/* Semidrive 32 bpp 16x4 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_16X4_TILE       fourcc_mod_code(SEMIDRIVE, 2)
/* Semidrive 32 bpp 32x2 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_32X2_TILE       fourcc_mod_code(SEMIDRIVE, 3)
/* Semidrive 16 bpp 16x8 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_16X8_TILE       fourcc_mod_code(SEMIDRIVE, 4)
/* Semidrive 16 bpp 32x4 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_32X4_TILE       fourcc_mod_code(SEMIDRIVE, 5)
/* Semidrive 16 bpp 64x2 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_64X2_TILE       fourcc_mod_code(SEMIDRIVE, 6)
/* Semidrive 8 bpp 32x8 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_32X8_TILE       fourcc_mod_code(SEMIDRIVE, 7)
/* Semidrive 8 bpp 64x4 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_64X4_TILE       fourcc_mod_code(SEMIDRIVE, 8)
/* Semidrive 8 bpp 128x2 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_128X2_TILE      fourcc_mod_code(SEMIDRIVE, 9)

/* Semidrive 32 bpp 8x8 tiling layout with fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_8X8_FBDC_TILE   fourcc_mod_code(SEMIDRIVE, 11)
/* Semidrive 32 bpp 16x4 tiling layout with fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_16X4_FBDC_TILE  fourcc_mod_code(SEMIDRIVE, 12)
/* Semidrive 32 bpp 32x2 tiling layout with fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_32X2_FBDC_TILE  fourcc_mod_code(SEMIDRIVE, 13)
/* Semidrive 16 bpp 16x8 tiling layout with fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_16X8_FBDC_TILE  fourcc_mod_code(SEMIDRIVE, 14)
/* Semidrive 16 bpp 32x4 tiling layout with fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_32X4_FBDC_TILE  fourcc_mod_code(SEMIDRIVE, 15)
/* Semidrive 16 bpp 64x2 tiling layout with fbdc compresswith fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_64X2_FBDC_TILE  fourcc_mod_code(SEMIDRIVE, 16)
/* Semidrive 8 bpp 32x8 tiling layout with fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_32X8_FBDC_TILE  fourcc_mod_code(SEMIDRIVE, 17)
/* Semidrive 8 bpp 64x4 tiling layout with fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_64X4_FBDC_TILE  fourcc_mod_code(SEMIDRIVE, 18)
/* Semidrive 8 bpp 128x2 tiling layout with fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_128X2_FBDC_TILE fourcc_mod_code(SEMIDRIVE, 19)

/* Semidrive coda988 16x16 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_CODA_16X16_TILE fourcc_mod_code(SEMIDRIVE, 21)
/* Semidrive wave412 32x8 tiling layout */
#define DRM_FORMAT_MOD_SEMIDRIVE_WAVE_32X8_TILE  fourcc_mod_code(SEMIDRIVE, 22)
/* Semidrive wave412 32x8 tiling layout with fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_WAVE_32X8_FBDC_TILE fourcc_mod_code(SEMIDRIVE, 31)
/* Semidrive wave412 16x8 tiling layout with fbdc compress */
#define DRM_FORMAT_MOD_SEMIDRIVE_WAVE_16X8_FBDC_TILE fourcc_mod_code(SEMIDRIVE, 32)
#endif //__DRM_DISPLAY_H