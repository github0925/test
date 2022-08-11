/* vi: set ts=8 sw=8 sts=8: */
/*************************************************************************/ /*!
@File
@Title          PVR DRM definitions shared between kernel and user space.
@Codingstyle    LinuxKernel
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(__PVR_DRM_H__)
#define __PVR_DRM_H__

#include "pvr_drm_core.h"

/*
 * IMPORTANT:
 * All structures below are designed to be the same size when compiled for 32
 * and/or 64 bit architectures, i.e. there should be no compiler inserted
 * padding. This is achieved by sticking to the following rules:
 * 1) only use fixed width types
 * 2) always naturally align fields by arranging them appropriately and by using
 *    padding fields when necessary
 *
 * These rules should _always_ be followed when modifying or adding new
 * structures to this file.
 */

struct drm_pvr_srvkm_cmd {
	__u32 bridge_id;
	__u32 bridge_func_id;
	__u64 in_data_ptr;
	__u64 out_data_ptr;
	__u32 in_data_size;
	__u32 out_data_size;
};

/*
 * DRM command numbers, relative to DRM_COMMAND_BASE.
 * These defines must be prefixed with "DRM_".
 */
#define DRM_PVR_SRVKM_CMD		0 /* Used for PVR Services ioctls */


/* These defines must be prefixed with "DRM_IOCTL_". */
#define	DRM_IOCTL_PVR_SRVKM_CMD	\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_PVR_SRVKM_CMD, \
		 struct drm_pvr_srvkm_cmd)

#endif /* defined(__PVR_DRM_H__) */
