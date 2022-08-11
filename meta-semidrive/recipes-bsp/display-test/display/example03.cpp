#include "debug.h"
#include "DrmDisplay.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <pthread.h>

#define BYE_ON(cond, ...) \
do { \
	if (cond) { \
		int errsv = errno; \
		fprintf(stderr, "ERROR(%s:%d) : ", \
			__FILE__, __LINE__); \
		errno = errsv; \
		fprintf(stderr,  __VA_ARGS__); \
		abort(); \
	} \
} while(0)
#define MEMORY_POOL_PHYSICAL_ADDR 0x60000000
#define IMAGE_WIDTH 512
#define IMAGE_HEIGHT 256

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

struct GRSurface {
	DrmPlane* plane;
	DrmFrame *frame;
	int format;
	int w;
	int h;
	int x;
	int y;
	int z_order;
	int plane_index;
	uint32_t bg_color;
	void *priv;
	pthread_t tidp;
};

struct DisplayRes res[2];
int g_drm_fd = -1;

// choose plane by format,compress etc..
DrmPlane* choose_perfect_plane(int id, std::vector<DrmPlane*> planes, int format) {
	drmModePlanePtr plane_ptr;
	for (DrmPlane *p : planes) {
		if (p->plane_ptr->possible_crtcs & (1 << id)) {
			if (p->supportThisFormat(format) && !p->busy)
				return p;
		}
	}
	return nullptr;
}


/* Semidrive planes sorted by z.
	DP
	├── plane3  -- support rgb
	├── plane2	-- support rgb
	├── plane1	-- support yuv,rgb
	└── plane0	-- support yuv,rgb
*/
pthread_mutex_t mutex_refresh = PTHREAD_MUTEX_INITIALIZER;
static void *show_frame_thread(void *data)
{
	struct GRSurface *s = (struct GRSurface*)data;
	DrmBackend backend;
	DrmPlane *plane;
	int crtc;

	if (backend.Init(0)) {
		LOGE("Init drm failed\n");
		return NULL;
	}
	crtc = backend.drm_.crtc_ids[0];
	plane = backend.drm_.planes[s->z_order];

	while (1) {
		pthread_mutex_lock(&mutex_refresh);
		// plane->setPlane(crtc, s->frame);
		LOGD("thread %d update %d plane", s->z_order, plane->plane_id);
		backend.Post(crtc, plane, s->frame);
		pthread_mutex_unlock(&mutex_refresh);
		sleep(3);
	}
	return NULL;
}


// ### 3. 多图层合成显示
int main(int argc, const char *argv[])
{
	int id;
	int ret;

	DrmBackend backend;

// main screen
	id = 0;
	if (backend.Init(id)) {
		LOGE("Init drm failed\n");
		return -1;
	}
	res[id].id = id;
	res[id].fence = -1;
	res[id].crtc = backend.drm_.crtc_ids[id];


/* 提前规划plane */

	#define IMAGE_WIDTH 512
	#define IMAGE_HEIGHT 256
	struct GRSurface surfaces[3] = {
		{
			NULL,
			NULL,
			DRM_FORMAT_ABGR8888,
			128,
			400,
			50,
			50,
			0,
			0,
			0xff0f00ff,
		},
		{
			NULL,
			NULL,
			DRM_FORMAT_ABGR8888,
			256,
			400,
			100,
			100,
			1,
			1,
			0xffff00ff,
		},
		{
			NULL,
			NULL,
			DRM_FORMAT_ABGR8888,
			512,
			400,
			200,
			200,
			2,
			2,
			0xff00ffff,
		},
	};

	// create frames and fill the structure GRSurface

    pthread_mutex_init(&mutex_refresh, NULL);
	int prime_fd;
	for (int i = 0; i < 3; i++) {
		struct GRSurface *s = &surfaces[i];
		DrmFrame *frame = new DrmFrame(backend.fd());
		if (i == 2) { // physical address
			size_t sz = s->w * s->h * 4;
			ret = drm_ioctl_export_dmabuf(backend.fd(), MEMORY_POOL_PHYSICAL_ADDR, sz, &prime_fd, 0);
			BYE_ON(ret, "export dmabuf failed\n");
			frame->ImportBuffer(s->w, s->h, s->w, DRM_FORMAT_ABGR8888, prime_fd, 0, 0);
		} else
			frame->createDumbBuffer(s->w, s->h, DRM_FORMAT_ABGR8888, 0, 0);

		frame->MapBO();
		frame->addFrameBuffer(0);
		// set blending parameters
		frame->alpha = 0xFF;
		frame->blend_mode = 0;
		frame->z_order = i;

		s->frame = frame;
		frame->display = sdm::Rect(s->x, s->y, s->w + s->x, s->h + s->y);

		#if 1
			setColor(frame->mapped_vaddr,  frame->bo.width, frame->bo.height, s->bg_color);
		#endif

		if ((pthread_create(&s->tidp, NULL, show_frame_thread, (void*)s)) == -1) {
			LOGE("create error!\n");
		}
		sleep(1);
	}

	LOGD("there are three threads running..");
	while ('q' != getchar()) {
		sleep(1);
	};
	LOGD("done");

	return 0;
}