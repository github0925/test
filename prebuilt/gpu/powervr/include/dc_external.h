/*************************************************************************/ /*!
@File
@Title          Display class external
@Description    Defines DC specific structures which are externally visible
                (i.e. visible to clients of services), but are also required
                within services.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef DC_EXTERNAL_H
#define DC_EXTERNAL_H

#include "img_types.h"

/*!
 * Maximum size of the display name in DC_DISPLAY_INFO
 */
#define DC_NAME_SIZE	50

/* Maximum number of certain call parameters */
#define DC_MAX_SRV_SYNC_COUNT	(1)
#define DC_MAX_PLANES		(4)

/*!
 * This contains information about a display.
 * The structure can be queried by services from the display driver via a
 * registered callback.
 *
 *   Structure: #DC_DISPLAY_INFO_TAG
 *   Typedef: ::DC_DISPLAY_INFO
 */
typedef struct DC_DISPLAY_INFO_TAG
{
	IMG_CHAR		szDisplayName[DC_NAME_SIZE];	/*!< Display identifier string */
	IMG_UINT32		ui32MinDisplayPeriod;			/*!< Minimum number of VSync periods */
	IMG_UINT32		ui32MaxDisplayPeriod;			/*!< Maximum number of VSync periods */
	IMG_UINT32		ui32MaxPipes;					/*!< Maximum number of pipes for this display */
	IMG_BOOL		bUnlatchedSupported;			/*!< Can the device be unlatched? */
} DC_DISPLAY_INFO;

/*!
 * When services imports a buffer from the display driver it has to fill
 * this structure to inform services about the buffer properties.
 *
 *   Structure: #DC_BUFFER_IMPORT_INFO_TAG
 *   Typedef: ::DC_BUFFER_IMPORT_INFO
 */
typedef struct DC_BUFFER_IMPORT_INFO_TAG
{
	IMG_UINT32		ePixFormat;			/*!< Enum value of type IMG_PIXFMT for the pixel format */
	IMG_UINT32		ui32BPP;			/*!< Bits per pixel */
	IMG_UINT32		ui32Width[3];		/*!< Width of the different channels (defined by ePixFormat) */
	IMG_UINT32		ui32Height[3];		/*!< Height of the different channels (defined by ePixFormat) */
	IMG_UINT32		ui32ByteStride[3];	/*!< Byte stride of the different channels (defined by ePixFormat) */
	IMG_UINT32		ui32PrivData[3];	/*!< Private data of the display for each of the channels */
} DC_BUFFER_IMPORT_INFO;


/*!
 * Configuration details of the frame buffer compression module
 *
 *   Structure: #DC_FBC_CREATE_INFO_TAG
 *   Typedef: ::DC_FBC_CREATE_INFO
 */
typedef struct DC_FBC_CREATE_INFO_TAG
{
	IMG_UINT32		ui32FBCWidth;	/*!< Pixel width that the FBC module is working on */
	IMG_UINT32		ui32FBCHeight;	/*!< Pixel height that the FBC module is working on */
	IMG_UINT32		ui32FBCStride;	/*!< Pixel stride that the FBC module is working on */
	IMG_UINT32		ui32Size;		/*!< Size of the buffer to create */
} DC_FBC_CREATE_INFO;

/*!
 * DC buffer details like frame buffer compression and surface properties
 *
 *   Structure: #DC_BUFFER_CREATE_INFO_TAG
 *   Typedef: ::DC_BUFFER_CREATE_INFO
 */
typedef struct DC_BUFFER_CREATE_INFO_TAG
{
	PVRSRV_SURFACE_INFO		sSurface;	/*!< Surface properties, specified by user */
	IMG_UINT32				ui32BPP;	/*!< Bits per pixel */
	union {
		DC_FBC_CREATE_INFO	sFBC;
	} u;								/*!< Frame buffer compressed specific data */
} DC_BUFFER_CREATE_INFO;

#endif /* DC_EXTERNAL_H */
