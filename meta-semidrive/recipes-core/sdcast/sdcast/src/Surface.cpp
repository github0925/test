#include "Surface.h"

SurfaceControl::SurfaceControl(sdm::Surface *surf) {
    mSurface = std::make_shared<sdm::Surface>();
    mSurface->phy_addr = surf->phy_addr;
    mSurface->source = surf->source;
    mSurface->display = surf->display;
    mSurface->format = surf->format;
    mSurface->width = surf->width;
    mSurface->height = surf->height;
    mSurface->stride = surf->stride;
    mSurface->prime_fd = surf->prime_fd;
}

SurfaceControl::SurfaceControl(std::shared_ptr<sdm::Surface> surface):mSurface(surface)
{

}

int SurfaceControl::setPos(int x, int y) {
    sdm::Rect &rect = mSurface->display;
    int w = (rect.right - rect.left);
    int h = (rect.bottom - rect.top);
    if (!w || !h) {
        LOGE("maybe surface display value is invalid");
        return -1;
    }
    rect.left = x;
    rect.right = x + w;
    rect.top = y;
    rect.bottom = y + h;
    return 0;
}

int SurfaceControl::setCrop(int x, int y) {
    sdm::Rect &rect = mSurface->source;
    int w = (rect.right - rect.left);
    int h = (rect.bottom - rect.top);
    if (!w || !h) {
        LOGE("maybe surface source value is invalid");
        return -1;
    }
    rect.left = x;
    rect.right = x + w;
    rect.top = y;
    rect.bottom = y + h;
    return 0;
}

int SurfaceControl::setTransform(int transform) {
    LOGE("Not support now!!");
    return -1;
}