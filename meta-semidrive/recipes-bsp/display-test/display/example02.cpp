#include "debug.h"
#include "DrmDisplay.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

enum {
	LEFT_DISPLAY = 0,
	RIGHT_DISPLAY,
	MAX_NUM_DISPLAY,
};

struct DisplayRes {
	int id;
	int crtc;
	int fence;
	DrmPlane* plane;
};

struct color32_t{
	uint8_t a;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};


// choose plane by format,compress etc..
DrmPlane* choose_perfect_plane(int id, std::vector<DrmPlane*> planes) {
	drmModePlanePtr plane_ptr;
	for (DrmPlane *p : planes) {
		if (p->plane_ptr->possible_crtcs & (1 << id)) {
			if (p->supportThisFormat(DRM_FORMAT_NV12))
				return p;
		}
	}
	return nullptr;
}

// ### 2. 双路拼显，也就是一路绘制两路输出。
int main(int argc, const char *argv[])
{
	int id;
	int ret;
	struct DisplayRes res[2];

	DrmBackend backend;

// main screen
	id = LEFT_DISPLAY;
	if (backend.Init(id)) {
		LOGE("Init drm failed\n");
		return -1;
	}

	res[id].id = id;
	res[id].fence = -1;
	res[id].crtc = backend.drm_.crtc_ids[id];
	res[id].plane = choose_perfect_plane(id, backend.drm_.planes);
	LOGD("main screen panel id %d", res[id].plane->plane_id);

// sub screen
	id = RIGHT_DISPLAY;
	if (backend.Init(id)) {
		LOGE("Init drm failed\n");
		return -1;
	}

	res[id].id = id;
	res[id].fence = -1;
	res[id].crtc = backend.drm_.crtc_ids[id];
	res[id].plane = choose_perfect_plane(id, backend.drm_.planes);
	LOGD("sub screen panel id %d", res[id].plane->plane_id);

	DrmFrame *big_frame = new DrmFrame(backend.fd());
	ret = big_frame->createDumbBuffer(3840, 720, DRM_FORMAT_ABGR8888, 0, 0);
	if (ret) {
		LOGD("error create dumb buffer");
		backend.DeInit();
		return -1;
	}

	big_frame->MapBO();
	big_frame->addFrameBuffer(0);
	// fill abgr888
	int w = big_frame->bo.width;
	int h = big_frame->bo.height;
#if 0
	setColor(big_frame->mapped_vaddr,  big_frame->bo.width, big_frame->bo.height, 0xff0000ff);
#else
	int channels;
	uint32_t *p = (uint32_t*)big_frame->mapped_vaddr;
	unsigned char *data = stbi_load("/data/3840x720.png", &w, &h, &channels, 0);
	uint32_t *color = (uint32_t *) data;
	for (auto i =0;i < h; i++)
		for (auto j = 0; j < w; j++) {
			// if (j == 0) (0x1000000 / w)
			// color = (0xff000000 |  4 * j);
			p[i * w + j] = color[i * w + j];
		}
	stbi_image_free(data);
#endif

	for (id = 0; id < MAX_NUM_DISPLAY; id++) {
		DrmFrame *frame = big_frame;
		int half_width = w / 2;
		if (id == LEFT_DISPLAY) {
			frame->source = sdm::Rect(0, 0, half_width, h);
			frame->display = sdm::Rect(0, 0, half_width, h);
		} else if (id == RIGHT_DISPLAY) {
			frame->source = sdm::Rect(half_width, 0, w, h);
			frame->display = sdm::Rect(0, 0, half_width, h);
		}

		backend.PostAsync(res[id].crtc, res[id].plane, frame, &res[id].fence);
		close(res[id].fence);
		res[id].fence = -1;
	}
	getchar();

	return 0;
}