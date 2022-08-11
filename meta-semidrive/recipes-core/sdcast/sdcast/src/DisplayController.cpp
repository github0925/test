#include "debug.h"
#include "DisplayController.h"
#include "DrmDisplay.h"
#include "utils.h"
#include <unistd.h>
#include <fcntl.h>

void DisplayFlushThread::run() {
    while (!gThreadStop) {
        LOGD("DisplayFlushThread do while");
        std::unique_lock<std::mutex> lk(iomux_);
        cv_.wait(lk);
        frame_ready_ = false;
        mDisplay->display(mSurface);
        lk.unlock();
    }
    LOGD("thread run loop exit");
}

int DisplayFlushThread::PostFrame(std::shared_ptr<sdm::Surface> surface)
{
    LOGD("call");
    std::lock_guard<std::mutex> lk(iomux_);
    mSurface = surface;
    frame_ready_ = true;
    cv_.notify_one();
    return 0;
}

void DisplayController::setDisplay(std::shared_ptr<Display> display) {
    LOGD("call");
    int err = display->Init();
    if (err) {
        LOGE("DisplayController display init failed");
        return;
    }
    mDisplay = display;
    mFlushThread.Init(display);
    mFlushThread.start();
}

int DisplayController::setCrop(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (mDisplay) {
        DisplayState &state = mDisplay->state;

        //state.crop = sdm::Rect(x, y, x + width, y + height);
        state.crop.left = x;
        state.crop.top = y;
        state.crop.right = (x + width);
        state.crop.bottom = (y + height);

        //if (mDisplay->state.crop.getWidth() <= 0 || state.crop.getHeight() <= 0) {
        if(getDisplayWidth(&mDisplay->state.crop) <= 0 ||getDisplayHeight(&mDisplay->state.crop) <= 0){
            LOGE("Fail: Invalid rectange for display");
            state.crop.left = 0;
            state.crop.top = 0;
            state.crop.right = 0;
            state.crop.bottom = 0;
            return -1;
        }
        state.state_changed |= 1 << DISPLAY_STATE_HINT_CROP;
    }
    return -1;
}

int DisplayController::setDisplayRectange(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (mDisplay) {
        DisplayState &state = mDisplay->state;

        //state.display = sdm::Rect(x, y, x + width, y + height);
        state.display.left = x;
        state.display.top = y;
        state.display.right = (x + width);
        state.display.bottom = (y + height);
        //if (state.display.getWidth() <= 0 || state.display.getHeight() <= 0) {
        if(getDisplayWidth(&mDisplay->state.crop) <= 0 ||getDisplayHeight(&mDisplay->state.crop) <= 0){
            LOGE("Fail: Invalid rectange for display");
            //state.display = sdm::Rect(0, 0, 0, 0);
            state.display.left = 0;
            state.display.top = 0;
            state.display.right = 0;
            state.display.bottom = 0;
            return -1;
        }

        state.state_changed |= 1 << DISPLAY_STATE_HINT_DISPLAY_RECT;
        return 0;
    }
    return -1;
}

sdm::Rect& DisplayController::getDisplayRectange() {
	DisplayState &state = mDisplay->state;
	return state.display;
}

int DisplayController::display_sync(std::shared_ptr<sdm::Surface> surface) {
	return mDisplay->display(surface);
	//return mFlushThread.PostFrame(surface);
}

int DisplayController::post_frame(DrmFrame *frame) {
	DisplayDrm *drm = static_cast<DisplayDrm*>(mDisplay.get());
	drm->backend_.Post(drm->crtc_, drm->backend_.drm_.planes[0], frame);

	return 0;
}

void DisplayController::blank(bool enabled) {
    mDisplay->blank(enabled);
}


int DisplayController::getDisplayWidth(sdm::Rect * rect) {
    return rect->right - rect->left;
}

int DisplayController::getDisplayHeight(sdm::Rect *rect) {
    return rect->bottom - rect->top;
}


DisplayDrm::~DisplayDrm() {
	DeInit();
}

void DisplayDrm::DeInit()
{
    LOGD("DeInit drm");
    backend_.DeInit();
}

int DisplayDrm::Init() {
    // use external crtc.
    backend_.Init(0);
    crtc_ = backend_.drm_.crtc();
	//dpms default  sleep.
	state.dpms = 3;
    return 0;
}

int DisplayDrm::display(std::shared_ptr<sdm::Surface> surface) {
    int ret;
    std::lock_guard<std::mutex> lk(refresh_mutex_);
    if (state.dpms == 3) {
        LOGD("display has closed");
        //if (surface->prime_fd)
            //close(surface->prime_fd);
        return 0;
    }

    if (state.state_changed & (1 << DISPLAY_STATE_HINT_DISPLAY_RECT)) {
        surface->display = state.display;
    }

    if (state.state_changed & (1 << DISPLAY_STATE_HINT_CROP)) {

        //surface->source = state.crop;
        surface->source.left = state.crop.left;
        surface->source.top = state.crop.top;
        surface->source.right = state.crop.right;
        surface->source.bottom = state.crop.bottom;
        //surface->width = surface->source.getWidth();
        //surface->height = surface->source.getHeight();
        surface->width = getDisplayWidth(&surface->source);
        surface->height = getDisplayHeight(&surface->source);

    }

    on_screen = std::move(new_frame);
    new_frame.reset(new DrmFrame(backend_.fd()));
    if (surface->prime_fd == -1) {
	    drm_ioctl_export_dmabuf(backend_.fd(), surface->phy_addr,
			surface->stride * surface->height * 4, &surface->prime_fd, 2);
    }

    LOGD("crtc_:%d, %d, %d, %d, %d, %d\n", crtc_,
			surface->width, surface->height,
            surface->stride, surface->format,
            surface->prime_fd);
	if (surface->prime_fd == 0) {
		LOGE("export dmabuf failed");
		return -1;
	}

	ret = new_frame->ImportBuffer(surface->width, surface->height,
                            surface->stride, surface->format,
                            surface->prime_fd);
    if (ret) {
        LOGE("ImportBuffer failed");
        return ret;
    }
    new_frame->display = Rect(surface->display.left, surface->display.top,
                                surface->display.right, surface->display.bottom);

	backend_.Post(crtc_, backend_.drm_.planes[0], new_frame.get());

    return 0;
}


int DisplayDrm::getDisplayWidth(sdm::Rect *rect) {
    return rect->right - rect->left;
}

int DisplayDrm::getDisplayHeight(sdm::Rect *rect) {
    return rect->bottom - rect->top;
}


int DisplayDrm::blank(bool enabled) {
	(void)enabled;
	static DrmFrame *blank_frame = new DrmFrame(backend_.fd());
	std::lock_guard<std::mutex> lk(refresh_mutex_);

	int dpms = enabled ? 3: 0;
	LOGD("DisplayDrm  blank %d --> %d", state.dpms, dpms);
	if (dpms != state.dpms) {
		on_screen.reset();
		state.dpms = dpms;
		if (dpms == 3) {
			#if 1
			if (blank_frame->fb_id <= 0) {
				blank_frame->createDumbBuffer(1920, 720, DRM_FORMAT_ABGR8888);
				blank_frame->MapBO();
				setColor(blank_frame->mapped_vaddr, 1920, 720, 0x00000000);
				blank_frame->unMapBO();
			}
			backend_.Post(backend_.drm_.crtc(), backend_.drm_.planes[0], blank_frame);
			#endif
			backend_.ApplyDpms(dpms);
		} else {
		}
	}

	return 0;
}

DisplayDummy::~DisplayDummy() {
    LOGD("call");
}

int DisplayDummy::Init() {
    LOGD("call");
    return 0;
}

int DisplayDummy::display(std::shared_ptr<sdm::Surface> surface) {
    (void)surface;
    std::lock_guard<std::mutex> lk(refresh_mutex_);
    if (state.state_changed & (1 << DISPLAY_STATE_HINT_DISPLAY_RECT)) {
        surface->display = state.display;
    }

    if (state.state_changed & (1 << DISPLAY_STATE_HINT_CROP)) {
        //surface->source = state.crop;

        surface->source.left = state.crop.left;
        surface->source.top = state.crop.top;
        surface->source.right = state.crop.right;
        surface->source.bottom = state.crop.bottom;
        //surface->width = surface->source.getWidth();
        //surface->height = surface->source.getHeight();
        surface->width = getDisplayWidth(&surface->source);
        surface->height = getDisplayHeight(&surface->source);

    }
    LOGD("call ready: crop: (%d %d %d %d) ---- display: (%d %d %d %d)",
                surface->source.left, surface->source.top,
                surface->source.right, surface->source.bottom,
                surface->display.left, surface->display.top,
                surface->display.right, surface->display.bottom
                );
    if (surface->prime_fd)
        close(surface->prime_fd);
    return 0;
}

int DisplayDummy::blank(bool enabled) {
    LOGD("call %d", enabled);
    return 0;
}


int DisplayDummy::getDisplayWidth(sdm::Rect * rect) {
    return rect->right - rect->left;
}

int DisplayDummy::getDisplayHeight(sdm::Rect *rect) {
    return rect->bottom - rect->top;
}
