#include "debug.h"
#include "HwConverter.h"
#include "HwAllocator.h"
#include "DrmDisplay.h"
#include <fcntl.h>

#include <string.h>


DrmAllocator::DrmAllocator()
{
	drmFd = drmOpen("semidrive", NULL);
	if (!drmFd) {
		LOGE("open drm card failed\n");
		return;
	}

}

DrmAllocator::~DrmAllocator()
{

	close(drmFd);
}

int getFormatPlanes(int format)
{
	switch (format) {
		case DRM_FORMAT_NV12:
		case DRM_FORMAT_NV21:
			return 2;
        case DRM_FORMAT_YVU420:
        case DRM_FORMAT_YUV420:
            return 3;
		default:
			return 1;
	}
	return 1;
}

int DrmAllocator::Alloc(struct hw_handle_t *handle)
{
	DrmFrame *frame = new DrmFrame(drmFd);
	int prime_fd;

	int ret = frame->createDumbBuffer(handle->width, handle->height, handle->format, 0, 0);
	if (ret) {
		LOGD("error create dumb buffer");
		return -1;
	}
	ret = drmPrimeHandleToFD(drmFd, frame->bo.gem_handles[0], DRM_CLOEXEC, &prime_fd);
	if (ret) {
		LOGE("drmPrimeHandleToFD failed: %d: %d(%s)", prime_fd, errno, strerror(errno));
		if (errno == ENOSYS) {
			LOGE("drm drive do not have prime_handle_to_fd function ops");
		}
		return -2;
	}
	handle->data = frame;
	handle->magic = 0x3044524d; // DRM
	handle->n_planes = getFormatPlanes(handle->format);

	handle->size = 0;
	for (int i = 0; i < handle->n_planes; i++) {
		handle->fds[i] = prime_fd;
		handle->offsets[i] = frame->bo.offsets[i];
		handle->strides[i] = frame->bo.pitches[i];
		handle->modifiers[i] = frame->bo.modifier[i];
		handle->size += handle->strides[i] * handle->height;
	}

	return 0;
}

int DrmAllocator::Free(struct hw_handle_t *handle)
{
	DrmFrame *frame = (DrmFrame *) handle->data;

	if (frame) {
		delete frame;
	}

	if (handle->fds[0]) {
		close(handle->fds[0]);
		handle->fds[0] = 0;
	}

	return 0;
}

int DrmAllocator::MapBo(struct hw_handle_t *handle)
{
	DrmFrame *frame = (DrmFrame *) handle->data;

	if (handle->mapped_vaddrs[0]) {
		UnMapBo(handle);
	}

	int ret = frame->MapBO();
	for (int i = 0; i < handle->n_planes; i++) {
		handle->mapped_vaddrs[i] = frame->planes[i];
	}

	return ret;
}

int DrmAllocator::UnMapBo(struct hw_handle_t *handle)
{
	DrmFrame *frame = (DrmFrame *) handle->data;
	if (handle->mapped_vaddrs[0]) {
		frame->unMapBO();
	}
	return 0;
}

int DrmAllocator::Import(struct hw_handle_t *handle)
{
	int ret;

	if (handle->width && handle->height && handle->format && handle->fds[0]
			&& handle->strides[0]) {
		DrmFrame *frame = new DrmFrame(drmFd);
		int stride = handle->strides[0] / DrmFrame::getBppByFormat(handle->format);
		ret = frame->ImportBuffer(handle->width, handle->height, stride, handle->format,
							handle->fds[0], 0, 0);
		if (ret) {
			LOGD("error create dumb buffer");
			return -1;
		}

		handle->data = frame;
		handle->magic = 0x0047424d; // GBM
		handle->n_planes = getFormatPlanes(handle->format);

		handle->size = 0;
		for (int i = 0; i < handle->n_planes; i++) {
			handle->fds[i] = handle->fds[0];
			handle->offsets[i] = frame->bo.offsets[i];
			handle->strides[i] = frame->bo.pitches[i];
			handle->modifiers[i] = frame->bo.modifier[i];
			handle->size += handle->strides[i] * handle->height;
		}
		return 0;
	}
	LOGE("Invalid handle to import\n");
	return -1;
}
