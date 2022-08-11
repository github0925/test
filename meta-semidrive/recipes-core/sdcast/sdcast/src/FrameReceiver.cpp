#include "FrameReceiver.h"
#include <time.h>
#include <thread>
#include <sys/mman.h>
#include "utils.h"

std::thread mPostThread;
unsigned long GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

int fps6(long deltaTime) // ms
{
    static float avgDuration = 0.f;
    static float alpha = 1.f / 100.f; // 采样数设置为100
    static int frameCount = 0;

    ++frameCount;

    int fps = 0;
    if (1 == frameCount)
    {
        avgDuration = static_cast<float>(deltaTime);
    }
    else
    {
        avgDuration = avgDuration * (1 - alpha) + deltaTime * alpha;
    }

    fps = static_cast<int>(1.f / avgDuration * 1000);
    return fps;
}

void FrameReceiver2::Init(DisplayController *dc) {
    gThreadStop = false;
    mDC = dc;
    mSock = std::make_shared<Communication>();
    mSock->Init(COMM_TYPE_CLIENT, 0, "172.20.0.3");
    mScreenState = 1;
    // Connect function will block until host can be attached.
    mSock->Connect();
    start();
}

void FrameReceiver2::reconnect() {
    LOGE("reconnect start");
    mSock->DeInit();
    mSock = std::make_shared<Communication>();
    mSock->Init(COMM_TYPE_CLIENT, 0, "172.20.0.3");
    mSock->Connect();
    LOGE("reconnect finished");
}

#define RESERVED_MEMROY_XEN  0x40000000

void FrameReceiver2::run(void) {
    static sdm::Surface info;
    static long t;
    int drm_fd = -1;
    ssize_t total_size = 0;

    while (!gThreadStop) {
        int prime_fd;
        ssize_t r = mSock->Read(&info, sizeof(sdm::Surface));
        if (r <= 0) {
            LOGE("info read failed: %d(%s)", errno, strerror(errno));
            reconnect();
            continue;
        }
        if (info.cmd == sdm::MSG_NOTIFY_HAS_CONTENT) {
            //remove all cache
            for(auto iter = mDrms.begin(); iter != mDrms.end();iter++) {
                DrmInfo *cache = iter->second;
                delete cache;
            }
            mDrms.clear();
        } if (info.cmd == sdm::MSG_NOTIFY_SURFACE_INFO) {
            info.prime_fd = -1;
            if (drm_fd <= 0) {
                drm_fd = drm_ioctl_open();
            }

            //wangsl
            DrmInfo *drm = mDrms[info.phy_addr];
            if(drm == nullptr) {
                LOGE("wangsl not saved in drm");
                drm_ioctl_export_dmabuf(drm_fd, info.phy_addr,
                        info.size? info.size: info.stride * info.height * 4,
                        &prime_fd, 2);

                drm = new DrmInfo();
                drm->addr = info.phy_addr;
                drm->fd = prime_fd;
                mDrms[info.phy_addr] = drm;
            }
            //LOGE("wangsl gogogo");
            total_size = 0;
            info.prime_fd = drm->fd;
            
            //wangsl

            info.format = DRM_FORMAT_ABGR8888;
            //LOGD("start trace2");
            SurfaceControl sc(&info);
            if (mDC && mScreenState) {
                mDC->display_sync(sc.getSurface());
            }

            // long now = GetTickCount();
            // LOGD("frame(%d, %d) ,fd %d, 0x%lx, fps: %d",
            //     info.width, info.height, info.prime_fd,
            //     info.phy_addr, fps6(now - t));
            // t = now;
        }

        switch (info.cmd) {
            case sdm::MSG_NOTIFY_HAS_CONTENT:
                if (mDC){
                    mDC->blank(false);
                }
                mScreenState = 1;
                break;
            case sdm::MSG_NOTIFY_NO_CONTENT:
                {
                    mScreenState = 0;
                    if (mDC){
                        mDC->blank(true);
                    }
                }
                break;
            default:
                break;
        }

        if (mUpdateCall) {
            mUpdateCall(&info, mUpdateData);
        }

        //LOGD("total_size --- %ld", total_size);
        int ret = mSock->Write(&total_size, sizeof(ssize_t));
        if(ret < 0) {
            reconnect();
            continue;
        }
    }
    LOGD("run thread out");
    close(drm_fd);
}
