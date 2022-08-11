/*************************************************************************/ /*!
@File
@Title          PowerVR Services Surface
@Description    Surface definitions and utilities that are externally visible
                (i.e. visible to clients of services), but are also required
                within services.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVRSRV_SURFACE_H
#define PVRSRV_SURFACE_H

#include "img_types.h"
#include "img_defs.h"
#include <powervr/buffer_attribs.h>

#define PVRSRV_SURFACE_TRANSFORM_NONE      (0 << 0)		/*!< No transformation */
#define PVRSRV_SURFACE_TRANSFORM_FLIP_H    (1 << 0)		/*!< Flip horizontally */
#define PVRSRV_SURFACE_TRANSFORM_FLIP_V    (1 << 1)		/*!< Flip vertically   */
#define PVRSRV_SURFACE_TRANSFORM_ROT_90    (1 << 2)                          /*!< Rotate  90 degree clockwise */
#define PVRSRV_SURFACE_TRANSFORM_ROT_180   ((1 << 0) + (1 << 1))             /*!< Rotate 180 degree clockwise */
#define PVRSRV_SURFACE_TRANSFORM_ROT_270   ((1 << 0) + (1 << 1) + (1 << 2))  /*!< Rotate 270 degree clockwise */

#define PVRSRV_SURFACE_BLENDING_NONE       0	/*!< Use no blending       */
#define PVRSRV_SURFACE_BLENDING_PREMULT    1	/*!< Use blending with pre-multiplier */
#define PVRSRV_SURFACE_BLENDING_COVERAGE   2	/*!< Use coverage blending */

/*!
 * Modes of memory layouts for surfaces.
 *
 *   Enum: #_PVRSRV_SURFACE_MEMLAYOUT_
 *   Typedef: ::PVRSRV_SURFACE_MEMLAYOUT
 */
typedef enum _PVRSRV_SURFACE_MEMLAYOUT_
{
	PVRSRV_SURFACE_MEMLAYOUT_STRIDED = 0,		/*!< Strided memory buffer */
	PVRSRV_SURFACE_MEMLAYOUT_FBC,				/*!< Compressed frame buffer */
} PVRSRV_SURFACE_MEMLAYOUT;

/*!
 * Frame Buffer Compression layout.
 * Defines the FBC mode to use.
 *
 *   Structure: #PVRSRV_SURFACE_FBC_LAYOUT_TAG
 *   Typedef: ::PVRSRV_SURFACE_FBC_LAYOUT
 */
typedef struct PVRSRV_SURFACE_FBC_LAYOUT_TAG
{
	/*! The compression mode for this surface */
	IMG_FB_COMPRESSION	eFBCompressionMode;
} PVRSRV_SURFACE_FBC_LAYOUT;

/*!
 * Pixel and memory format of a surface
 *
 *   Structure: #PVRSRV_SURFACE_FORMAT_TAG
 *   Typedef: ::PVRSRV_SURFACE_FORMAT
 */
typedef struct PVRSRV_SURFACE_FORMAT_TAG
{
	/*! Enum value of type IMG_PIXFMT for the pixel format */
	IMG_UINT32					ePixFormat;

	/*! Enum surface memory layout */
	PVRSRV_SURFACE_MEMLAYOUT	eMemLayout;

	/*! Special layout options for the surface.
	 * Needs services support.
	 * Depends on eMemLayout.*/
	union {
		PVRSRV_SURFACE_FBC_LAYOUT	sFBCLayout;
	} u;
} PVRSRV_SURFACE_FORMAT;

/*!
 * Width and height of a surface
 *
 *   Structure: #PVRSRV_SURFACE_DIMS_TAG
 *   Typedef: ::PVRSRV_SURFACE_DIMS
 */
typedef struct PVRSRV_SURFACE_DIMS_TAG
{
	IMG_UINT32		ui32Width;		/*!< Width in pixels */
	IMG_UINT32		ui32Height;		/*!< Height in pixels */
} PVRSRV_SURFACE_DIMS;

/*!
 * Dimension and format details of a surface
 *
 *   Structure: #PVRSRV_SURFACE_INFO_TAG
 *   Typedef: ::PVRSRV_SURFACE_INFO
 */
typedef struct PVRSRV_SURFACE_INFO_TAG
{
	PVRSRV_SURFACE_DIMS		sDims;		/*!< Width and height */
	PVRSRV_SURFACE_FORMAT	sFormat;	/*!< Memory format */
} PVRSRV_SURFACE_INFO;

/*!
 * Defines a rectangle on a surface
 *
 *   Structure: #PVRSRV_SURFACE_RECT_TAG
 *   Typedef: ::PVRSRV_SURFACE_RECT
 */
typedef struct PVRSRV_SURFACE_RECT_TAG
{
	IMG_INT32				i32XOffset;	/*!< X offset from origin in pixels */
	IMG_INT32				i32YOffset;	/*!< Y offset from origin in pixels */
	PVRSRV_SURFACE_DIMS		sDims;		/*!< Rectangle dimensions */
} PVRSRV_SURFACE_RECT;

/*!
 * Surface configuration details
 *
 *   Structure: #PVRSRV_SURFACE_CONFIG_INFO_TAG
 *   Typedef: ::PVRSRV_SURFACE_CONFIG_INFO
 */
typedef struct PVRSRV_SURFACE_CONFIG_INFO_TAG
{
	/*! Crop applied to surface (BEFORE transformation) */
	PVRSRV_SURFACE_RECT		sCrop;

	/*! Region of screen to display surface in (AFTER scaling) */
	PVRSRV_SURFACE_RECT		sDisplay;

	/*! Surface transformation none/flip/rotate.
	 * Use PVRSRV_SURFACE_TRANSFORM_xxx macros
	 */
	IMG_UINT32				ui32Transform;

	/*! Alpha blending mode e.g. none/premult/coverage.
	 * Use PVRSRV_SURFACE_BLENDING_xxx macros
	 */
	IMG_UINT32				eBlendType;

	/*! Custom data for the display engine */
	IMG_UINT32				ui32Custom;

	/*! Alpha value for this plane */
	IMG_UINT8				ui8PlaneAlpha;

	/*! Reserved for later use */
	IMG_UINT8				ui8Reserved1[3];
} PVRSRV_SURFACE_CONFIG_INFO;

/*!
 * Contains information about a panel
 */
typedef struct PVRSRV_PANEL_INFO_TAG
{
	PVRSRV_SURFACE_INFO sSurfaceInfo;		/*!< Panel surface details */
	IMG_UINT32			ui32RefreshRate;	/*!< Panel refresh rate in Hz */
	IMG_UINT32			ui32XDpi;			/*!< Panel DPI in x direction */
	IMG_UINT32			ui32YDpi;			/*!< Panel DPI in y direction */
} PVRSRV_PANEL_INFO;

/*!
 * Helper function to create a Config Info based on a Surface Info
 * to do a flip with no scale, transformation etc.
 */
static INLINE void SurfaceConfigFromSurfInfo(const PVRSRV_SURFACE_INFO *psSurfaceInfo,
                                             PVRSRV_SURFACE_CONFIG_INFO *psConfigInfo)
{
	psConfigInfo->sCrop.sDims = psSurfaceInfo->sDims;
	psConfigInfo->sCrop.i32XOffset = 0;
	psConfigInfo->sCrop.i32YOffset = 0;
	psConfigInfo->sDisplay.sDims = psSurfaceInfo->sDims;
	psConfigInfo->sDisplay.i32XOffset = 0;
	psConfigInfo->sDisplay.i32YOffset = 0;
	psConfigInfo->ui32Transform = PVRSRV_SURFACE_TRANSFORM_NONE;
	psConfigInfo->eBlendType = PVRSRV_SURFACE_BLENDING_NONE;
	psConfigInfo->ui32Custom = 0;
	psConfigInfo->ui8PlaneAlpha = 0xff;
}

#endif /* PVRSRV_SURFACE_H */
