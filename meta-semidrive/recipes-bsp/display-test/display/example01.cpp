#include "debug.h"
#include "DrmDisplay.h"

enum {
	MAIN_DISPLAY = 0,
	SUB_DISPLAY,
	MAX_NUM_DISPLAY,
};

struct DisplayRes {
	int id;
	int crtc;
	DrmPlane* plane;
	int fence;
};

// choose plane by format,compress etc..
DrmPlane* choose_perfect_plane(int id, std::vector<DrmPlane*> planes, int format) {
	drmModePlanePtr plane_ptr;
	for (DrmPlane *p : planes) {
		if (p->plane_ptr->possible_crtcs & (1 << id)) {
			if (p->supportThisFormat(format))
				return p;
		}
	}
	return nullptr;
}

//### 1. 双路显示，分辨率可配置。
int main(int argc, const char *argv[])
{
	int id;
	struct DisplayRes res[2];

	DrmBackend backend;

// main screen
	id = MAIN_DISPLAY;
	if (backend.Init(id)) {
		LOGE("Init drm failed\n");
		return -1;
	}

	res[id].id = id;
	res[id].fence = -1;
	res[id].crtc = backend.drm_.crtc_ids[id];
	res[id].plane = choose_perfect_plane(id, backend.drm_.planes, DRM_FORMAT_NV12);
	LOGD("main screen panel id %d", res[id].plane->plane_id);

// sub screen
	id = SUB_DISPLAY;
	if (backend.Init(id)) {
		LOGE("Init drm failed\n");
		return -1;
	}

	res[id].id = id;
	res[id].fence = -1;
	res[id].crtc = backend.drm_.crtc_ids[id];
	res[id].plane = choose_perfect_plane(id, backend.drm_.planes, DRM_FORMAT_NV12);
	LOGD("sub screen panel id %d", res[id].plane->plane_id);
	DrmFrame *frame = new DrmFrame(backend.fd());
	int ret = frame->createDumbBuffer(400, 300, DRM_FORMAT_NV12, 0, 0);
	if (ret) {
		LOGD("error create dumb buffer");
		backend.DeInit();
		return -1;
	}
	// map to userspace
	frame->MapBO();
	frame->addFrameBuffer(0);
	// setColor(frame->mapped_vaddr, 400, 300, 0xffffff00);

	sdm::Rect &r = frame->display;
	r = sdm::Rect(r.left + 50, r.top + 50, r.right + 50, r.bottom + 50);

	for (id = 0; id < MAX_NUM_DISPLAY; id++) {
	#if 1
		backend.PostAsync(res[id].crtc, res[id].plane, frame, &res[id].fence);
		close(res[id].fence);
		res[id].fence = -1;
	#else
		backend.Post(res[id].crtc, res[id].plane, frame);
	#endif
	}
	getchar();

	return 0;
}