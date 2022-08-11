/*************************************************************************/ /*!
@Title          Information about the uses of USC global registers.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
/* NOTE: Do not add include guards to this file */

/*
  View index for Vulkan 'multiview'. Written by the firmware, read by programs running in TA context.
*/
DCL(RGX_USC_GLOBALREGS_TA_VIEW_INDEX)

/*
  View index for Vulkan 'multiview'. Written by the firmware, read by programs running in 3D context.
*/
DCL(RGX_USC_GLOBALREGS_3D_VIEW_INDEX)

#if !defined(RGX_FEATURE_USC_RENDER_TARGET_ID)

/*
  Low/high bytes of the current render target for render target arrays.
  Written by the firmware, read by 3D background object and end of tile programs.
*/
DCL(RGX_USC_GLOBALREGS_RENDER_TARGET_LOW_BYTE)
DCL(RGX_USC_GLOBALREGS_RENDER_TARGET_HIGH_BYTE)

#endif /* !defined(RGX_FEATURE_USC_RENDER_TARGET_ID) */

/*
  Global registers used for the BRN62269/66972 workaround.
  Set by the firmware and the CDM context switch resume program.
  Read by compute kernels.
 */
#if defined(FIX_HW_BRN_62269) || defined(FIX_HW_BRN_66972)

#if ROGUE_NUM_USCS > 6
	#error "Too many USCs for BRN6629/66972 workaround."
#endif /* ROGUE_NUM_USCS > 6 */

/* Set by USC 0 when resumes for all USCs have finished. */
DCL(RGX_USC_GLOBALREGS_BRN62269_ALLUSCS)

#if ROGUE_NUM_USCS > 1

/* Set by USC 1 when the resume for that USC has finished. */
DCL(RGX_USC_GLOBALREGS_BRN62269_USC1)

#if ROGUE_NUM_USCS > 2

/* Set by USC 2 when the resume for that USC has finished. */
DCL(RGX_USC_GLOBALREGS_BRN62269_USC2)

#if ROGUE_NUM_USCS > 3

/* Set by USC 3 when the resume for that USC has finished. */
DCL(RGX_USC_GLOBALREGS_BRN62269_USC3)

#if ROGUE_NUM_USCS > 4

/* Set by USC 4 when the resume for that USC has finished. */
DCL(RGX_USC_GLOBALREGS_BRN62269_USC4)

#if ROGUE_NUM_USCS > 5

/* Set by USC 5 when the resume for that USC has finished. */
DCL(RGX_USC_GLOBALREGS_BRN62269_USC5)

#endif /* ROGUE_NUM_USCS > 5 */

#endif /* ROGUE_NUM_USCS > 4 */

#endif /* ROGUE_NUM_USCS > 3 */

#endif /* ROGUE_NUM_USCS > 2 */

#endif /* ROGUE_NUM_USCS > 1 */

#endif /* defined(FIX_HW_BRN_62269) || defined(FIX_HW_BRN_66972) */

DCL(RGX_USC_GLOBALREGS_COUNT)
