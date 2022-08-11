#include "debug.h"
#include "HwConverter.h"
#include "DrmDisplay.h"
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <unistd.h>
#include <string.h>

DrmBackend backend;
std::unique_ptr<DrmFrame> new_frame, on_screen;

HwConverter::HwConverter()
{
    if (backend.Init(0)) {
		LOGE("Init drm failed\n");
		return ;
	}
}

HwConverter::~HwConverter()
{
    backend.DeInit();
}

int HwConverter::Display(const HwBuffer *buffer)
{
    int display = 0;
	LOGD("start runing on display %d", display);

	int crtc = backend.drm_.crtc_ids[display];
    DrmPlane *plane = backend.drm_.planes[1];

	new_frame.reset(new DrmFrame(backend.fd()));
    int format = buffer->handle.format;

    int stride = buffer->handle.strides[0] / (DrmFrame::getBppByFormat(format) / 8);
	int ret = new_frame->ImportBuffer(buffer->handle.width, buffer->handle.height,
        stride, format, dup(buffer->handle.fds[0]), 0, 0);
	if (ret) {
		LOGE("error create dumb buffer");
		backend.DeInit();
		return -1;
	}
    new_frame->addFrameBuffer(0);
    new_frame->z_order = 1;
    static int fence = -1;
    ret = backend.PostAsync(crtc, plane, new_frame.get(), &fence);
    if (ret) {
        LOGE("drm post frame failed\n");
    }
    on_screen = std::move(new_frame);

    return 0;
}

/* ######################################### */
HwBuffer::~HwBuffer()
{
    if (!m_allocator) {
        return;
    }
    if (handle.mapped_vaddrs[0] == nullptr) {
        m_allocator->UnMapBo(&handle);
    }
    m_allocator->Free(&handle);

    if (handle.fds[0]) {
        close(handle.fds[0]);
        handle.fds[0] = 0;
    }
}

HwBuffer::HwBuffer():m_allocator(HwConverter::getAllocator())
{

}

HwBuffer::HwBuffer(const HwBuffer &obj):m_allocator(HwConverter::getAllocator())
{

    source = obj.source;
    display = obj.display;

    alpha = obj.alpha;
    blend_mode = obj.blend_mode;
    zorder = obj.zorder;
    rotation = obj.rotation;

    memcpy(&handle, &obj.handle, sizeof(struct hw_handle_t));

    m_allocator->Import(&handle);
}

HwBuffer::HwBuffer(int dma_fd, uint32_t width, uint32_t height, uint32_t stride,
              uint32_t format): m_allocator(HwConverter::getAllocator())
{

    memset(&handle, 0, sizeof(handle));

    handle.width = width;
    handle.height = height;
    handle.strides[0] = stride;
    handle.fds[0] = dma_fd;
    handle.format = format;

    m_allocator->Import(&handle);

    source = sdm::Rect(0, 0, width, height);
    display = source;

    alpha = 0xff;
    blend_mode = HW_BLEND_PIXEL_NONE;
    zorder = 0;
    rotation = HW_ROTATION_TYPE_NONE;
}

HwBuffer::HwBuffer(struct hw_handle_t *rh): m_allocator(HwConverter::getAllocator())
{
	memcpy(&handle, rh, sizeof(struct hw_handle_t));
    source = sdm::Rect(0, 0, rh->width, rh->height);
    display = source;

    alpha = 0xff;
    zorder = 0;
    // BLEND_PIXEL_NONE
    blend_mode = 0;
    rotation = 0;
}

HwBuffer::HwBuffer(uint32_t width, uint32_t height,
              uint32_t format):m_allocator(HwConverter::getAllocator())
{

    memset(&handle, 0, sizeof(handle));

    handle.width = width;
    handle.height = height;
    handle.format = format;
    m_allocator->Alloc(&handle);

    source = sdm::Rect(0, 0, handle.width, handle.height);
    display = source;

    alpha = 0xff;
    zorder = 0;
    // BLEND_PIXEL_NONE
    blend_mode = 0;
    rotation = 0;
}

void HwBuffer::MapBo(void)
{
    m_allocator->MapBo(&handle);
}

void HwBuffer::UnMapBo(void)
{
    m_allocator->UnMapBo(&handle);
}

HwBuffer& HwBuffer::Resize(uint32_t width, uint32_t height)
{
    display = sdm::Rect(display.left, display.top,
            display.left + width, display.top + height);
    return *this;
}


