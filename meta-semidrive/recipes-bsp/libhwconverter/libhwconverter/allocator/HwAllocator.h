
#ifndef __HW_ALLOCATOR_H
#define __HW_ALLOCATOR_H
#include "HwConverter.h"

// 30 49 4f 4e, ION
// 30 44 52 4d, DRM
// 30 47 42 4d, GBM

class GbmAllocator: public Allocator
{
public:
	GbmAllocator();
	~GbmAllocator();
	int Alloc(struct hw_handle_t *handle);
	int Free(struct hw_handle_t *handle);
	int MapBo(struct hw_handle_t *handle);
	int UnMapBo(struct hw_handle_t *handle);
	int Import(struct hw_handle_t *handle);
private:
	struct gbm_device *gbm;
	int drmFd;
};

class DrmAllocator: public Allocator
{
public:
	DrmAllocator();
	~DrmAllocator();
	int Alloc(struct hw_handle_t *handle);
	int Free(struct hw_handle_t *handle);
	int MapBo(struct hw_handle_t *handle);
	int UnMapBo(struct hw_handle_t *handle);
	int Import(struct hw_handle_t *handle);
private:
	struct gbm_device *gbm;
	int drmFd;
};

#endif