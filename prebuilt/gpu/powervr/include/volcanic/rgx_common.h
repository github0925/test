/*************************************************************************/ /*!
@File
@Title          RGX Common Types and Defines Header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description	Common types and definitions for RGX software
@License        Strictly Confidential.
*/ /**************************************************************************/
#ifndef RGX_COMMON_H
#define RGX_COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "img_defs.h"

/* Included to get the BVNC_KM_N defined and other feature defs */
#include "km/rgxdefs_km.h"

/*! This macro represents a mask of LSBs that must be zero on data structure
 * sizes and offsets to ensure they are 8-byte granular on types shared between
 * the FW and host driver */
#define RGX_FW_ALIGNMENT_LSB (7U)

/*! Macro to test structure size alignment */
#define RGX_FW_STRUCT_SIZE_ASSERT(_a)	\
	static_assert(((sizeof(_a) & RGX_FW_ALIGNMENT_LSB)?0:1),	\
				  "Size of " #_a " is not properly aligned")

/*! Macro to test structure member alignment */
#define RGX_FW_STRUCT_OFFSET_ASSERT(_a, _b)	\
	static_assert( ((offsetof(_a, _b) & RGX_FW_ALIGNMENT_LSB)?0:1), \
				  "Offset of " #_a "." #_b " is not properly aligned")


/*! The master definition for data masters known to the firmware of RGX.
 * When a new DM is added to this list, relevant entry should be added to
 * RGX_HWPERF_DM enum list.
 * The DM in a V1 HWPerf packet uses this definition. */
typedef IMG_UINT32 RGXFWIF_DM;

#define	RGXFWIF_DM_GP			IMG_UINT32_C(0)
#define	RGXFWIF_DM_TDM			IMG_UINT32_C(1)
#define	RGXFWIF_DM_GEOM			IMG_UINT32_C(2)
#define	RGXFWIF_DM_3D			IMG_UINT32_C(3)
#define	RGXFWIF_DM_CDM			IMG_UINT32_C(4)

#define	RGXFWIF_DM_LAST			RGXFWIF_DM_CDM



typedef enum _RGX_KICK_TYPE_DM_
{
	RGX_KICK_TYPE_DM_GP			= 0x001,
	RGX_KICK_TYPE_DM_TDM_2D		= 0x002,
	RGX_KICK_TYPE_DM_GEOM			= 0x004,
	RGX_KICK_TYPE_DM_3D			= 0x008,
	RGX_KICK_TYPE_DM_CDM		= 0x010,
	RGX_KICK_TYPE_DM_TQ2D		= 0x020,
	RGX_KICK_TYPE_DM_TQ3D		= 0x040,
	RGX_KICK_TYPE_DM_LAST		= 0x080
} RGX_KICK_TYPE_DM;

/* Maximum number of DM in use: GP, 2D, GEOM, 3D, CDM */
#define RGXFWIF_DM_DEFAULT_MAX	(RGXFWIF_DM_LAST + 1U)

#if !defined(__KERNEL__)
/* Maximum number of DM in use: GP, 2D, GEOM, 3D, CDM */
#define RGXFWIF_DM_MAX			(RGXFWIF_DM_LAST + 1)

/* Min/Max number of HW DMs (all but GP) */
#define RGXFWIF_HWDM_MIN		(1U)
#define RGXFWIF_HWDM_MAX		(RGXFWIF_DM_MAX)
#else /* !defined(__KERNEL__) */
	#define RGXFWIF_DM_MIN_MTS_CNT			(6)
	#define RGXFWIF_DM_MIN_CNT				(5)
	#define RGXFWIF_DM_MAX					(RGXFWIF_DM_MIN_CNT)
#endif /* !defined(__KERNEL__) */

/*
 * Data Master Tags to be appended to resources created on behalf of each RGX
 * Context.
 */
#define RGX_RI_DM_TAG_KS   'K'
#define RGX_RI_DM_TAG_CDM  'C'
#define RGX_RI_DM_TAG_RC   'R' /* To be removed once TA/3D Timelines are split */
#define RGX_RI_DM_TAG_GEOM 'V'
#define RGX_RI_DM_TAG_3D   'P'
#define RGX_RI_DM_TAG_TDM  'T'

/*
 * Client API Tags to be appended to resources created on behalf of each
 * Client API.
 */
#define RGX_RI_CLIENT_API_GLES1    '1'
#define RGX_RI_CLIENT_API_GLES3    '3'
#define RGX_RI_CLIENT_API_VULKAN   'V'
#define RGX_RI_CLIENT_API_EGL      'E'
#define RGX_RI_CLIENT_API_OPENCL   'C'
#define RGX_RI_CLIENT_API_OPENGL   'G'
#define RGX_RI_CLIENT_API_SERVICES 'S'
#define RGX_RI_CLIENT_API_WSEGL    'W'
#define RGX_RI_CLIENT_API_ANDROID  'A'
#define RGX_RI_CLIENT_API_LWS      'L'

/*
 * Format a RI annotation for a given RGX Data Master context
 */
#define RGX_RI_FORMAT_DM_ANNOTATION(annotation, dmTag, clientAPI) do         \
	{                                                                        \
		annotation[0] = dmTag;                                               \
		annotation[1] = clientAPI;                                           \
		annotation[2] = '\0';                                                \
	} while (0)

/*!
 ******************************************************************************
 * RGXFW Compiler alignment definitions
 *****************************************************************************/
#if defined(__GNUC__) || defined(INTEGRITY_OS)
#define RGXFW_ALIGN			__attribute__ ((aligned (8)))
#define	RGXFW_ALIGN_DCACHEL		__attribute__((aligned (64)))
#elif defined(_MSC_VER)
#define RGXFW_ALIGN			__declspec(align(8))
#define	RGXFW_ALIGN_DCACHEL		__declspec(align(64))
#pragma warning (disable : 4324)
#else
#error "Align MACROS need to be defined for this compiler"
#endif

/*!
 ******************************************************************************
 * Force 8-byte alignment for structures allocated uncached.
 *****************************************************************************/
#define UNCACHED_ALIGN      RGXFW_ALIGN


/*!
 ******************************************************************************
 * GPU Utilisation states
 *****************************************************************************/
#define RGXFWIF_GPU_UTIL_STATE_IDLE      (0U)
#define RGXFWIF_GPU_UTIL_STATE_ACTIVE    (1U)
#define RGXFWIF_GPU_UTIL_STATE_BLOCKED   (2U)
#define RGXFWIF_GPU_UTIL_STATE_NUM       (3U)
#define RGXFWIF_GPU_UTIL_STATE_MASK      IMG_UINT64_C(0x0000000000000003)


/*
 * Maximum amount of register writes that can be done by the register
 * programmer (FW or META DMA). This is not a HW limitation, it is only
 * a protection against malformed inputs to the register programmer.
 */
#define RGX_MAX_NUM_REGISTER_PROGRAMMER_WRITES  (128U)

/*
 *   Use of the 32-bit context property flags mask
 *   ( X = taken/in use, - = available/unused )
 *
 *                                   0
 *                                   |
 *    -------------------------------x
 */
 /*
 * Context creation flags
 * (specify a context's properties at creation time)
 */
#define RGX_CONTEXT_FLAG_DISABLESLR					(1UL << 0) /*!< Disable SLR */

/* List of attributes that may be set for a context */
typedef enum _RGX_CONTEXT_PROPERTY_
{
	RGX_CONTEXT_PROPERTY_FLAGS  = 0, /*!< Context flags */
} RGX_CONTEXT_PROPERTY;

#if defined(__cplusplus)
}
#endif

#endif /* RGX_COMMON_H */

/******************************************************************************
 End of file
******************************************************************************/
