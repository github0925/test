#include "HwConverter.h"
#include "HwAllocator.h"
#include "debug.h"
#include <minigbm.h>
#include <string.h>

struct gbm_map_data {
	void *data[4];
};

GbmAllocator::GbmAllocator()
{
	drmFd = drmOpen("semidrive", NULL);
	if (!drmFd) {
		LOGE("open drm card failed\n");
		return;
	}
	gbm = gbm_create_device(drmFd);
}

GbmAllocator::~GbmAllocator()
{
	if (gbm) {
		gbm_device_destroy(gbm);
	}
	close(drmFd);
}

int GbmAllocator::Alloc(struct hw_handle_t *handle)
{
	struct gbm_bo *bo = gbm_bo_create(gbm, handle->width, handle->height, handle->format, GBM_BO_USE_RENDERING);

	if (!bo) {
		LOGE("Allocator failed\n");
		LOGE("bo = 0x%lx, format ,  0x%x\n", (unsigned long) bo, handle->format);
		return -1;
	}

	handle->data = bo;
	handle->magic = 0x0047424d; // GBM
	handle->width = gbm_bo_get_width(bo);
	handle->height = gbm_bo_get_height(bo);
	handle->format = gbm_bo_get_format(bo);
	handle->n_planes = gbm_bo_get_num_planes(bo);
	//handle->gem_handle =  //gbm_bo_get_handle(bo);

	handle->size = 0;
	for (int i = 0; i < handle->n_planes; i++) {
		handle->fds[i] = gbm_bo_get_plane_fd(bo, i);
		handle->offsets[i] = gbm_bo_get_plane_offset(bo, i);
		handle->strides[i] = gbm_bo_get_plane_stride(bo, i);
		handle->modifiers[i] = gbm_bo_get_plane_format_modifier(bo, i);
		handle->size += gbm_bo_get_plane_size(bo, i);
	}

	return 0;
}

int GbmAllocator::Free(struct hw_handle_t *handle)
{
	struct gbm_bo *bo = (struct gbm_bo *) handle->data;

	if (bo) {
		gbm_bo_destroy(bo);
		handle->data = NULL;
	}
	return 0;
}

static void destroy_user_data(struct gbm_bo *bo, void *user_data)
{
	//delete user_data;
	free(user_data);
}

int GbmAllocator::MapBo(struct hw_handle_t *handle)
{
	struct gbm_bo *bo = (struct gbm_bo *) handle->data;
	uint32_t stride = 0;
	uint32_t map_flags = GBM_BO_TRANSFER_READ_WRITE;
	void *map_data;
	char *vaddr = NULL;

	if (gbm_bo_get_user_data(bo)) {
		UnMapBo(handle);
	}

	vaddr = (char *)gbm_bo_map(bo, 0, 0, handle->width, handle->height,
											  map_flags, &stride, &map_data, 0);
	if (vaddr == -1) {
		LOGE("vaddr mapped failed");
		return -1;
	}

	for (int i = 0; i < handle->n_planes; i++) {
		handle->mapped_vaddrs[i] = vaddr;
		vaddr += gbm_bo_get_plane_size(bo, i);
	}

	gbm_bo_set_user_data(bo, map_data, destroy_user_data);

	return 0;
}

int GbmAllocator::UnMapBo(struct hw_handle_t *handle)
{
	struct gbm_bo *bo = (struct gbm_bo *) handle->data;
	void *map_data = (void *)gbm_bo_get_user_data(bo);

	if (!map_data)
		return 0;

	gbm_bo_unmap(bo, map_data);

	for (size_t i = 0; i < gbm_bo_get_num_planes(bo); i++)
	{
		handle->mapped_vaddrs[i] = nullptr;
	}
	// delete map_data;
	gbm_bo_set_user_data(bo, NULL, destroy_user_data);

	return 0;
}

int GbmAllocator::Import(struct hw_handle_t *handle)
{
	if (handle->width && handle->height && handle->format && handle->fds[0] && handle->strides[0]) {
		gbm_import_fd_data data;
		memset(&data, 0, sizeof(gbm_import_fd_data));

		data.width = handle->width;
		data.height = handle->height;
		data.format = handle->format;
		data.fd = handle->fds[0];
		data.stride = handle->strides[0];
		struct gbm_bo *bo = gbm_bo_import(gbm, GBM_BO_IMPORT_FD, &data, GBM_BO_USE_RENDERING);

		handle->data = bo;
		handle->magic = 0x0047424d; // GBM
		handle->width = gbm_bo_get_width(bo);
		handle->height = gbm_bo_get_height(bo);
		handle->format = gbm_bo_get_format(bo);
		handle->n_planes = gbm_bo_get_num_planes(bo);
		// handle->gem_handle = gbm_bo_get_handle(bo);

		handle->size = 0;
		for (int i = 0; i < handle->n_planes; i++) {
			handle->fds[i] = gbm_bo_get_plane_fd(bo, i);
			handle->offsets[i] = gbm_bo_get_plane_offset(bo, i);
			handle->strides[i] = gbm_bo_get_plane_stride(bo, i);
			handle->modifiers[i] = gbm_bo_get_plane_format_modifier(bo, i);
			handle->size += gbm_bo_get_plane_size(bo, i);
		}
		return 0;
	}

	LOGE("Invalid handle to import\n");
	return -1;
}
