#include "debug.h"
#include "Surface.h"
#include "DisplayController.h"
#include "FrameReceiver.h"
#include <memory.h>
#include <sys/mman.h>
#include <stdio.h>

#include "utils.h"

void test_displaycontroller()
{
    sdm::Rect disp = {0, 0, 320, 240};
    sdm::Rect source = {0, 0, 320, 240};
    sdm::Surface surf;
    surf.display = disp;
    surf.source = source;
    surf.width = 320;
    surf.height = 240;
    surf.stride = 320;

    SurfaceControl sc(&surf);
    DisplayController dc;
    std::unique_ptr<Display> display(new DisplayDrm);
    dc.setDisplay(std::move(display));

    dc.blank(true);
    dc.blank(false);
    sc.setPos(30, 80);
    int n = 10;
    while(n--) {
        dc.display_sync(sc.getSurface());
        usleep(1000 * 100);
    }

}
#define RESERVED_MEMROY_XEN  0x40000000

int test_drmdisplay(int argc, char *argv[])
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

	std::unique_ptr<DrmFrame> on_screen;
	//std::unique_ptr<DrmFrame> new_frame;

	while (1) {
		int prime_fd;
		int width, height, stride;

		width = 1920;
		height = 720;
		stride = 1920 * 4;
		#if 0
		drm_ioctl_export_dmabuf(backend.fd(), RESERVED_MEMROY_XEN, stride * height, &prime_fd, 2);
		#else

		DrmFrame *frame = new DrmFrame(backend.fd());
		int ret = frame->createDumbBuffer(width, height, DRM_FORMAT_ARGB8888);
		if (ret) {
			LOGD("error create dumb buffer");
			backend.DeInit();
			return -1;
		}
		// map to userspace
		frame->MapBO();
		setColor(frame->mapped_vaddr, width, height, 0xff0000ff);
		frame->unMapBO();
		ret = drmPrimeHandleToFD(backend.fd(), frame->bo.gem_handles[0], DRM_CLOEXEC | DRM_RDWR, &prime_fd);
		if (ret) {
			LOGE("drmPrimeHandleToFD failed: %d: %d(%s)", prime_fd, errno, strerror(errno));
			if (errno == ENOSYS) {
				LOGE("drm drive do not have prime_handle_to_fd function ops");
			}
		}

		#endif
#if 1
		void *mapped = mmap(0, stride * height, PROT_READ | PROT_WRITE, MAP_SHARED,
				prime_fd, 0);
		if ((long)mapped == -1) {
			LOGD("Error maped: %d(%s)", errno, strerror(errno));
		} else {
			LOGD("get mmaped adress 0x%lx", (unsigned long)mapped);
			munmap(mapped, stride * height);
		}
		DrmFrame *new_frame = new DrmFrame(backend.fd());
		new_frame->ImportBuffer(width, height, stride / 4, DRM_FORMAT_ARGB8888, prime_fd);


		backend.Post(crtc, backend.drm_.planes[0], new_frame);
		on_screen.reset(new_frame);
#endif
		getchar();
	}
	getchar();

	backend.DeInit();

}

class SdCast {
public:
	static void FrameUpdate(sdm::Surface *info, update_data_t data) {
		SdCast *cast = (SdCast *)data;
		cast->update(info);
	}

	int Init() {
		receiver = new FrameReceiver2;
		receiver->Init(nullptr);
		receiver->setUpdateCall(SdCast::FrameUpdate, (void *)this);
		receiver->join();
		return 0;
	}

private:
	void update(sdm::Surface *info){
		LOGD("=============> frame update: info cmd: %d", info->cmd);
	}
	int status;
	DisplayController *pDc;
	std::shared_ptr<Display> pDisplay;
	FrameReceiver2 *receiver;
};


int main(int argc, char *argv[]) {
    (void )argc;
    (void )argv;
#if 0
	test_drmdisplay(argc, argv);

//	int dmabuf;
//	int drmfd = drm_ioctl_open();
//	drm_ioctl_export_dmabuf(drmfd, RESERVED_MEMROY_XEN, 0x10200, &dmabuf, 2);
//	close(drmfd);

//	close(dmabuf);

#elif 0
	SdCast sdcast;
	sdcast.Init();
#else
    LOGD("client frame receiver");
    DisplayController dc;
    std::shared_ptr<Display> display = std::make_shared<DisplayDrm>();
    dc.setDisplay(display);
    dc.setDisplayRectange(0, 0, 640, 400);

    FrameReceiver2 receiver;

    receiver.Init(&dc);
    receiver.join();
#endif

    return 0;
}
