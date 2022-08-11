/*************************************************************************/ /*!
@File
@Title          Wrapper around drm_fourcc.h
@Description    FourCCs and the DRM framebuffer modifiers should be added here
                unless they are used by kernel code or a known user outside of
                the DDK. If FourCCs or DRM framebuffer modifiers are required
                outside of the DDK, they shall be moved to the corresponding
                public header.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef IMG_DRM_FOURCC_INTERNAL_H
#define IMG_DRM_FOURCC_INTERNAL_H

#include <powervr/img_drm_fourcc.h>

/*
 * Modifier names are structured using the following convention,
 * with underscores (_) between items:
 * - prefix: DRM_FORMAT_MOD
 * - identifier for our driver: PVR
 * - category: FBCDC
 *   - compression tile dimension: 8x8, 16x4, 32x2
 *   - FBDC version: V0, V1, V2, V3, V7, V8, V10, V12
 */
#define DRM_FORMAT_MOD_PVR_FBCDC_8x8_V0      fourcc_mod_code(PVR, 1)
#define DRM_FORMAT_MOD_PVR_FBCDC_8x8_V0_FIX  fourcc_mod_code(PVR, 2) /* Fix for HW_BRN_37464 */
#define DRM_FORMAT_MOD_PVR_FBCDC_8x8_V1      fourcc_mod_code(PVR, 3)
#define DRM_FORMAT_MOD_PVR_FBCDC_8x8_V2      fourcc_mod_code(PVR, 4)
#define DRM_FORMAT_MOD_PVR_FBCDC_8x8_V3      fourcc_mod_code(PVR, 5)
/* DRM_FORMAT_MOD_PVR_FBCDC_8x8_V7 - moved to the public header */
#define DRM_FORMAT_MOD_PVR_FBCDC_8x8_V8      fourcc_mod_code(PVR, 18)
#define DRM_FORMAT_MOD_PVR_FBCDC_8x8_V10     fourcc_mod_code(PVR, 21)
#define DRM_FORMAT_MOD_PVR_FBCDC_8x8_V12     fourcc_mod_code(PVR, 15)
#define DRM_FORMAT_MOD_PVR_FBCDC_16x4_V0     fourcc_mod_code(PVR, 7)
#define DRM_FORMAT_MOD_PVR_FBCDC_16x4_V0_FIX fourcc_mod_code(PVR, 8) /* Fix for HW_BRN_37464 */
#define DRM_FORMAT_MOD_PVR_FBCDC_16x4_V1     fourcc_mod_code(PVR, 9)
#define DRM_FORMAT_MOD_PVR_FBCDC_16x4_V2     fourcc_mod_code(PVR, 10)
#define DRM_FORMAT_MOD_PVR_FBCDC_16x4_V3     fourcc_mod_code(PVR, 11)
/* DRM_FORMAT_MOD_PVR_FBCDC_16x4_V7 - moved to the public header */
#define DRM_FORMAT_MOD_PVR_FBCDC_16x4_V8     fourcc_mod_code(PVR, 19)
#define DRM_FORMAT_MOD_PVR_FBCDC_16x4_V10    fourcc_mod_code(PVR, 22)
#define DRM_FORMAT_MOD_PVR_FBCDC_16x4_V12    fourcc_mod_code(PVR, 16)
#define DRM_FORMAT_MOD_PVR_FBCDC_32x2_V1     fourcc_mod_code(PVR, 13)
#define DRM_FORMAT_MOD_PVR_FBCDC_32x2_V3     fourcc_mod_code(PVR, 14)
#define DRM_FORMAT_MOD_PVR_FBCDC_32x2_V8     fourcc_mod_code(PVR, 20)
#define DRM_FORMAT_MOD_PVR_FBCDC_32x2_V10    fourcc_mod_code(PVR, 23)
#define DRM_FORMAT_MOD_PVR_FBCDC_32x2_V12    fourcc_mod_code(PVR, 17)

#endif /* IMG_DRM_FOURCC_INTERNAL_H */
