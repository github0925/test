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
    DrmFrame(int fd): fb_id(-1), mapped_vaddr(NULL), isDumb(false), drm_fd(fd) {};
	DrmFrame(DrmFrame&&) {
		LOGD("call");
	}
    ~DrmFrame(){
        LOGD("call");
        Destroy();
    };
    int createDumbBuffer(unsigned int width, unsigned int height, unsigned int format);
    int ImportBuffer(unsigned int width, unsigned int height, int stride, unsigned int format, int dma_buf);
	int MapBO();
	void unMapBO();
	void Destroy();
    int getBppByFormat(int format);

	inline int fd() {
		return drm_fd;
	}
	void *planes[4] = {NULL};
    Rect display;
    Rect source;
	int fb_id;
	size_t sz_bo;
	void *mapped_vaddr;
	std::unique_ptr<FenceFd> release_fd;
	hwc_drm_bo_t  bo;
protected:
	int createBO(uint32_t width, uint32_t virtual_height, int bpp);
	int fillBO(int width, int height, int stride, int format, uint32_t gem_handles);
	void DestroyBO();
	int addFrameBuffer();
    int prime_fd_;
	bool isDumb;
    int drm_fd;
};

class DrmPlane {
public:
    DrmPlane(int fd, int id):drm_fd(fd), plane_id(id) {
		plane_ptr = drmModeGetPlane(drm_fd, plane_id);
    }
    ~DrmPlane() {
        drmModeFreePlane(plane_ptr);
        LOGD("Plane delete");
    }
    int Init();
	int setPlane(uint32_t crtc_id, drmModeAtomicReq *req, DrmFrame *frame);
	int setPlane(uint32_t crtc_id, DrmFrame *frame);
	int disablePlane(int drmfd);
    void getProperties();

public:
    int drm_fd;
	int crtc_id;
	uint32_t property_type;
	uint32_t property_alpha;
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
	int plane_id;
	drmModePlanePtr plane_ptr;
};

class DrmResources {
public:
    DrmResources(){};
    ~DrmResources(){};
    int Init(int fd, int active_id);
    void DeInit();
	inline int crtc() {
		return crtc_ids[id];
	}
	inline int connector() {
		return connector_ids[id];
	}
public:
    int drm_fd;
   	uint32_t property_crtc_id;
	uint32_t property_mode_id;
	uint32_t property_active;
	uint32_t prop_conn_dpms_id;
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
    void DeInit();
	inline int fd() {
		return drm_fd_;
	}
	drmModeAtomicReq *post_req;
    int drm_fd_;
    DrmResources drm_;
	int ApplyDpms(int dpms);
public:
	static void PageFlipHandler(int fd, unsigned int sequence, unsigned int tv_sec,
  		unsigned int tv_usec, void *user_data);
};

void setColor(void *vaddr, int w, int h, uint32_t color);

#endif //__DRM_DISPLAY_H
