/*************************************************************************/ /*!
@Title          ion allocations for compressed framebuffers
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef ION_FBCDC_ALLOC_H
#define ION_FBCDC_ALLOC_H

#define ION_IOC_FBCDC_ALLOC 1

struct ion_fbcdc_alloc_data {
	/* in */
	size_t len;
	size_t align;
	unsigned int heap_id_mask;
	unsigned int flags;
	size_t tiles;
	/* out */
	int handle;
};

#if !defined(__KERNEL__)
static
int ion_fbcdc_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
					unsigned int flags, size_t tiles, int *handlefd) __attribute__((unused));

static
int ion_fbcdc_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
					unsigned int flags, size_t tiles, int *handlefd)
{
	int err;
	struct ion_fbcdc_alloc_data payload = {
		.len = len,
		.align = align,
		.heap_id_mask = heap_mask,
		.flags = flags,
		.tiles = tiles,
	};
	struct ion_custom_data data = {
		.cmd = ION_IOC_FBCDC_ALLOC,
		.arg = (unsigned long)&payload,
	};

	if (handlefd == NULL)
		return -EINVAL;

	err = ioctl(fd, ION_IOC_CUSTOM, &data);
	if (err < 0)
		return err;

	/* The handle returned is a shared dma_buf.*/
	*handlefd = payload.handle;

	return err;
}

static int ion_custom_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
							unsigned int flags, size_t tiles, int *handlefd)
{

#if defined(PVR_ANDROID_ION_FBCDC_ALLOC)
	return ion_fbcdc_alloc(fd, len, align, heap_mask, flags, tiles, handlefd);
#else /* defined(PVR_ANDROID_ION_FBCDC_ALLOC) */
	PVR_UNREFERENCED_PARAMETER(tiles);
	return ion_alloc_fd(fd, len, align, heap_mask, flags, handlefd);
#endif /* defined(PVR_ANDROID_ION_FBCDC_ALLOC) */
}
#endif /* !defined(__KERNEL__) */

#endif /* ION_FBCDC_ALLOC_H */
