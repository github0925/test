/* vi: set ts=8 sw=8 sts=8: */
/*************************************************************************/ /*!
@File
@Title          Nulldisp DRM definitions shared between kernel and user space.
@Codingstyle    LinuxKernel
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(__NULLDISP_DRM_H__)
#define __NULLDISP_DRM_H__

#if defined(__KERNEL__)
#include <drm/drm.h>
#else
#include <drm.h>
#endif

struct drm_nulldisp_gem_create {
	__u64 size;   /* in */
	__u32 flags;  /* in */
	__u32 handle; /* out */
};

struct drm_nulldisp_gem_mmap {
	__u32 handle; /* in */
	__u32 pad;
	__u64 offset; /* out */
};

#define NULLDISP_GEM_CPU_PREP_READ   (1 << 0)
#define NULLDISP_GEM_CPU_PREP_WRITE  (1 << 1)
#define NULLDISP_GEM_CPU_PREP_NOWAIT (1 << 2)

struct drm_nulldisp_gem_cpu_prep {
	__u32 handle; /* in */
	__u32 flags;  /* in */
};

struct drm_nulldisp_gem_cpu_fini {
	__u32 handle; /* in */
	__u32 pad;
};

/*
 * DRM command numbers, relative to DRM_COMMAND_BASE.
 * These defines must be prefixed with "DRM_".
 */
#define DRM_NULLDISP_GEM_CREATE   0x00
#define DRM_NULLDISP_GEM_MMAP     0x01
#define DRM_NULLDISP_GEM_CPU_PREP 0x02
#define DRM_NULLDISP_GEM_CPU_FINI 0x03

/* These defines must be prefixed with "DRM_IOCTL_". */
#define DRM_IOCTL_NULLDISP_GEM_CREATE \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_NULLDISP_GEM_CREATE, \
		 struct drm_nulldisp_gem_create)

#define DRM_IOCTL_NULLDISP_GEM_MMAP \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_NULLDISP_GEM_MMAP, \
		 struct drm_nulldisp_gem_mmap)

#define DRM_IOCTL_NULLDISP_GEM_CPU_PREP \
	DRM_IOW(DRM_COMMAND_BASE + DRM_NULLDISP_GEM_CPU_PREP, \
		struct drm_nulldisp_gem_cpu_prep)

#define DRM_IOCTL_NULLDISP_GEM_CPU_FINI \
	DRM_IOW(DRM_COMMAND_BASE + DRM_NULLDISP_GEM_CPU_FINI, \
		struct drm_nulldisp_gem_cpu_fini)

#endif /* defined(__NULLDISP_DRM_H__) */
