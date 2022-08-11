/**************************************************************************/ /*!
@File           pvrsrv_devmem_pdump.h
@Title          Device Memory Management PDump
@Author       	Copyright (C) Imagination Technologies Limited.
                All rights reserved. Strictly Confidential.
@Description    Client side part of device memory management -- This
                file defines the exposed Services API to PDump memory management
                functions.
*/ /***************************************************************************/

#ifndef PVRSRV_DEVMEM_PDUMP_H
#define PVRSRV_DEVMEM_PDUMP_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "pvrsrv_devmem.h"
#include <powervr/buffer_attribs.h>
#include "rgxdefs.h"

#if defined PDUMP
/******************************************************************************
 Structures used in API calls
******************************************************************************/

/*!
 * Structure for describing Image being SABed out.
 */
typedef struct _PVRSRV_PDUMP_IMAGE_DESCRIPTOR_
{
	IMG_DEV_VIRTADDR sData;            /*!< GPU virtual address of data */
	IMG_UINT32 ui32DataSize;           /*!< Size of data for the image */
	IMG_UINT32 ui32LogicalWidth;       /*!< Logical width of the surface in texels */
	IMG_UINT32 ui32LogicalHeight;      /*!< Logical height of the surface in texels */
	IMG_UINT32 ui32PhysicalWidth;      /*!< Physical width of the surface in texels */
	IMG_UINT32 ui32PhysicalHeight;     /*!< Physical height of the surface in texels */
	PDUMP_PIXEL_FORMAT ePixFmt;        /*!< Pixel format */
	IMG_MEMLAYOUT eMemLayout;          /*!< Memory layout */
	IMG_FB_COMPRESSION eFBCompression; /*!< FBCompression */
	/* Clear constants part of FBC descriptors on some core variants (Volcanic),
	 * hence less items in array are needed. */
#if RGX_FBC_MAX_DESCRIPTORS > 0
	IMG_UINT32 aui32FBCClearColour[2]; /*!< FBC Clear colour */
#else
	IMG_UINT32 aui32FBCClearColour[4]; /*!< FBC Clear colour */
#endif
	PDUMP_FBC_SWIZZLE eFBCSwizzle;
	IMG_DEV_VIRTADDR sHeader;          /*!< GPU virtual address of header of surface (only applicable to FBC surfaces) */
	IMG_UINT32 ui32HeaderSize;         /*!< Surface header size (only applicable to FBC surfaces) */
	IMG_BOOL bIsOutFB;
} PVRSRV_PDUMP_IMAGE_DESCRIPTOR;

/*!
 * Structure for describing Data being SABed out.
 */
typedef struct _PVRSRV_PDUMP_DATA_DESCRIPTOR_
{
	IMG_DEV_VIRTADDR sData;            /*!< GPU virtual address of data */
	IMG_UINT32 ui32DataSize;           /*!< Size of data */
	IMG_UINT32 ui32ElementType;        /*!< Data type of elements in data */
	IMG_UINT32 ui32ElementCount;       /*!< Number of elements in data */
} PVRSRV_PDUMP_DATA_DESCRIPTOR;

/**************************************************************************/ /*!
@Function       PVRSRVPDumpDevmemLoadMem
@Description    writes the current contents of the referenced memory (which is
                assumed to have been prewritten by some means) to the PDump
                PARAMS stream, and emits an LDB to to the script stream to load
                these bytes to the memory when the pdump script is played back
@Input          hMemDesc            Handle of the memory descriptor
@Input          uiOffset            byte offset
@Input          uiSize              byte size
@Input          uiPDumpFlags        pdump flags
@Return         None
*/ /***************************************************************************/

IMG_EXPORT void
PVRSRVPDumpDevmemLoadMem(PVRSRV_MEMDESC hMemDesc,
                         IMG_DEVMEM_OFFSET_T uiOffset,
                         IMG_DEVMEM_SIZE_T uiSize,
                         IMG_UINT32 uiPDumpFlags);


/**************************************************************************/ /*!
@Function       PVRSRVPDumpDevmemLoadMemValue32
@Description    writes the current value of the referenced memory (which is
                assumed to have been prewritten by some means) to the PDump
                PARAMS stream, and emits an WRW to to the script stream to write
				to the memory when the pdump script is played back
@Input          hMemDesc            Handle of the memory descriptor
@Input          uiOffset            byte offset
@Input          ui32Value           value
@Input          uiPDumpFlags        pdump flags
@Return         None
*/ /***************************************************************************/

IMG_EXPORT void
PVRSRVPDumpDevmemLoadMemValue32(PVRSRV_MEMDESC hMemDesc,
                         IMG_DEVMEM_OFFSET_T uiOffset,
                         IMG_UINT32 ui32Value,
                         IMG_UINT32 uiPDumpFlags);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpDevmemLoadMemValue64
@Description    writes the current value of the referenced memory (which is
                assumed to have been prewritten by some means) to the PDump
                PARAMS stream, and emits an WRW64 to to the script stream to write
                to the memory when the pdump script is played back
@Input          hMemDesc            Handle of the memory descriptor
@Input          uiOffset            byte offset
@Input          ui64Value           value
@Input          uiPDumpFlags        pdump flags
@Return         None
*/ /***************************************************************************/

IMG_EXPORT void
PVRSRVPDumpDevmemLoadMemValue64(PVRSRV_MEMDESC hMemDesc,
                         IMG_DEVMEM_OFFSET_T uiOffset,
                         IMG_UINT64 ui64Value,
                         IMG_UINT32 uiPDumpFlags);


/**************************************************************************/ /*!
@Function       PVRSRVPDumpDevmemPol32
@Description    Emits a PDUMP POL command relative to the underlying PMR.
                This version can only poll on 32-bit memory locations.
@Input          hMemDesc            memory descriptor
@Input          uiOffset            byte offset
@Input          ui32Value           value to poll on
@Input          ui32Mask            poll bit mask
@Input          eOperator           binary operator between memory and value
@Input          ui32PDumpFlags      pdump flags
@Return         None
*/ /***************************************************************************/
IMG_EXPORT void
PVRSRVPDumpDevmemPol32(PVRSRV_MEMDESC hMemDesc,
                       IMG_DEVMEM_OFFSET_T uiOffset,
                       IMG_UINT32 ui32Value,
                       IMG_UINT32 ui32Mask,
                       PDUMP_POLL_OPERATOR eOperator,
                       IMG_UINT32 ui32PDumpFlags);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpDevmemPDumpPolCBP
@Description    Emits a PDUMP CBP command relative to the underlying PMR.
@Input          hMemDesc            memory descriptor
@Input          uiReadOffset        byte read offset
@Input          uiWriteOffset       byte write offset
@Input          uiPacketSize        byte packet size
@Input          uiBufferSize        Dbyte buffer size
@Return         None
*/ /***************************************************************************/
IMG_EXPORT void
PVRSRVPDumpDevmemCBP(PVRSRV_MEMDESC hMemDesc,
					 IMG_DEVMEM_OFFSET_T uiReadOffset,
					 IMG_DEVMEM_OFFSET_T uiWriteOffset,
					 IMG_DEVMEM_SIZE_T uiPacketSize,
					 IMG_DEVMEM_SIZE_T uiBufferSize);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpSaveToFileVirtual
@Description    Emits a pdump SAB using the virtual address and device MMU context
                to cause the pdump player to traverse the MMU page tables itself.
@Input          hMemDesc            memory descriptor
@Input          uiOffset            byte offset
@Input          uiSize              byte size
@Input          pszFilename         file name string
@Return         PVRSRV_ERROR:       PVRSRV_OK on success. Otherwise, a PVRSRV_
                                    error code
*/ /***************************************************************************/
IMG_EXPORT void
PVRSRVPDumpSaveToFileVirtual(PVRSRV_MEMDESC hMemDesc,
							 IMG_DEVMEM_OFFSET_T uiOffset,
							 IMG_DEVMEM_SIZE_T uiSize,
							 const IMG_CHAR *pszFilename,
							 IMG_UINT32 ui32FileOffset,
							 IMG_UINT32 ui32PdumpFlags);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpBitmap
@Description    duimp bitmap function
@Input          pszDevData          dev data
@Input          pszFileName         file name string
@Input          ui32FileOffset      byte offset in file
@Input          ui32Width           pixel width
@Input          ui32Height          pixel height
@Input          ui32StrideInBytes   byte stride
@Input          sDevBaseAddr        device virtual base address
@Input          hDevMemContext      device memory contextn
@Input          ui32Size            byte size
@Input          ePixelFormat        pixel format
@Input          eMemFormat          memory format/layout
@Input          ui32PDumpFlags      pdump flags
@Return         PVRSRV_ERROR:       PVRSRV_OK on success. Otherwise, a PVRSRV_
                                    error code
*/ /***************************************************************************/
IMG_EXPORT void
IMG_CALLCONV PVRSRVPDumpBitmap(const PVRSRV_DEV_CONNECTION *psDevConnection,
								IMG_CHAR *pszFileName,
								IMG_UINT32 ui32FileOffset,
								IMG_UINT32 ui32Width,
								IMG_UINT32 ui32Height,
								IMG_UINT32 ui32StrideInBytes,
								IMG_DEV_VIRTADDR sDevBaseAddr,
								PVRSRV_DEVMEMCTX hDevMemContext,
								IMG_UINT32 ui32Size,
								PDUMP_PIXEL_FORMAT ePixelFormat,
								IMG_UINT32 ui32AddrMode,
								IMG_UINT32 ui32PDumpFlags);

/**************************************************************************/ /*!
@Function       PVRSRVPdumpImageDescriptor
@Description    API to PDUMP the image descriptor
@Input          psDevConnection     Pointer to services connection structure.
@Input          psContext           Pointer to device memory context.
@Input          pszFileName         Pointer to string containing file name of
                                    image being SABed
@Input          psDesc              Pointer to image descriptor structure.
@Input          ui32PDumpFlags      PDUMP flags
@Return         PVRSRV_ERROR:       PVRSRV_OK on success. Otherwise, a PVRSRV_
                                    error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV
PVRSRVPdumpImageDescriptor(const PVRSRV_DEV_CONNECTION *psDevConnection,
						   const PVRSRV_DEVMEMCTX psContext,
						   const IMG_CHAR *pszFileName,
						   const PVRSRV_PDUMP_IMAGE_DESCRIPTOR *psDesc,
						   IMG_UINT32 ui32PDumpFlags);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpDataDescriptor
@Description    API to PDUMP the Data descriptor
@Input          psDevConnection     Pointer to services connection structure.
@Input          psContext           Pointer to device memory context.
@Input          pszFileName         Pointer to string containing file name of
                                    data being SABed
@Input          psDesc              Pointer to Data descriptor structure.
@Input          ui32PDumpFlags      PDUMP flags
@Return         PVRSRV_ERROR:       PVRSRV_OK on success. Otherwise, a PVRSRV_
                                    error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV
PVRSRVPDumpDataDescriptor(const PVRSRV_DEV_CONNECTION *psDevConnection,
						   const PVRSRV_DEVMEMCTX psContext,
						   const IMG_CHAR *pszFileName,
						   const PVRSRV_PDUMP_DATA_DESCRIPTOR *psDesc,
						   IMG_UINT32 ui32PDumpFlags);

IMG_EXPORT void IMG_CALLCONV
PVRSRVPdumpInitHWDefaultFBCClearColour(PVRSRV_PDUMP_IMAGE_DESCRIPTOR *psDesc);
#else	/* PDUMP */

typedef struct _PVRSRV_PDUMP_IMAGE_DESCRIPTOR_ PVRSRV_PDUMP_IMAGE_DESCRIPTOR;
typedef struct _PVRSRV_PDUMP_DATA_DESCRIPTOR_ PVRSRV_PDUMP_DATA_DESCRIPTOR;

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpDevmemLoadMem)
#endif
static INLINE void
PVRSRVPDumpDevmemLoadMem(PVRSRV_MEMDESC hMemDesc,
                         IMG_DEVMEM_OFFSET_T uiOffset,
                         IMG_DEVMEM_SIZE_T uiSize,
                         IMG_UINT32 uiPDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(hMemDesc);
	PVR_UNREFERENCED_PARAMETER(uiOffset);
	PVR_UNREFERENCED_PARAMETER(uiSize);
	PVR_UNREFERENCED_PARAMETER(uiPDumpFlags);
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpDevmemLoadMemValue32)
#endif
static INLINE void
PVRSRVPDumpDevmemLoadMemValue32(PVRSRV_MEMDESC hMemDesc,
                         IMG_DEVMEM_OFFSET_T uiOffset,
                         IMG_UINT32 ui32Value,
                         IMG_UINT32 uiPDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(hMemDesc);
	PVR_UNREFERENCED_PARAMETER(uiOffset);
	PVR_UNREFERENCED_PARAMETER(ui32Value);
	PVR_UNREFERENCED_PARAMETER(uiPDumpFlags);

}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpDevmemLoadMemValue64)
#endif
static INLINE void
PVRSRVPDumpDevmemLoadMemValue64(PVRSRV_MEMDESC hMemDesc,
                         IMG_DEVMEM_OFFSET_T uiOffset,
                         IMG_UINT64 ui64Value,
                         IMG_UINT32 uiPDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(hMemDesc);
	PVR_UNREFERENCED_PARAMETER(uiOffset);
	PVR_UNREFERENCED_PARAMETER(ui64Value);
	PVR_UNREFERENCED_PARAMETER(uiPDumpFlags);

}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpDevmemPol32)
#endif
static INLINE void
PVRSRVPDumpDevmemPol32(PVRSRV_MEMDESC hMemDesc,
                       IMG_DEVMEM_OFFSET_T uiOffset,
                       IMG_UINT32 ui32Value,
                       IMG_UINT32 ui32Mask,
                       PDUMP_POLL_OPERATOR eOperator,
                       IMG_UINT32 ui32PDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(hMemDesc);
	PVR_UNREFERENCED_PARAMETER(uiOffset);
	PVR_UNREFERENCED_PARAMETER(ui32Value);
	PVR_UNREFERENCED_PARAMETER(ui32Mask);
	PVR_UNREFERENCED_PARAMETER(eOperator);
	PVR_UNREFERENCED_PARAMETER(ui32PDumpFlags);
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpDevmemCBP)
#endif
static INLINE void
PVRSRVPDumpDevmemCBP(PVRSRV_MEMDESC hMemDesc,
					 IMG_DEVMEM_OFFSET_T uiReadOffset,
					 IMG_DEVMEM_OFFSET_T uiWriteOffset,
					 IMG_DEVMEM_SIZE_T uiPacketSize,
					 IMG_DEVMEM_SIZE_T uiBufferSize)

{
	PVR_UNREFERENCED_PARAMETER(hMemDesc);
	PVR_UNREFERENCED_PARAMETER(uiReadOffset);
	PVR_UNREFERENCED_PARAMETER(uiWriteOffset);
	PVR_UNREFERENCED_PARAMETER(uiPacketSize);
	PVR_UNREFERENCED_PARAMETER(uiBufferSize);
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpSaveToFileVirtual)
#endif
static INLINE void
PVRSRVPDumpSaveToFileVirtual(PVRSRV_MEMDESC hMemDesc,
							 IMG_DEVMEM_OFFSET_T uiOffset,
							 IMG_DEVMEM_SIZE_T uiSize,
							 const IMG_CHAR *pszFilename,
							 IMG_UINT32 ui32FileOffset,
							 IMG_UINT32 ui32PdumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(hMemDesc);
	PVR_UNREFERENCED_PARAMETER(uiOffset);
	PVR_UNREFERENCED_PARAMETER(uiSize);
	PVR_UNREFERENCED_PARAMETER(pszFilename);
	PVR_UNREFERENCED_PARAMETER(ui32FileOffset);
	PVR_UNREFERENCED_PARAMETER(ui32PdumpFlags);
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpBitmap)
#endif
static INLINE void
PVRSRVPDumpBitmap(const PVRSRV_DEV_CONNECTION *psDevConnection,
				  IMG_CHAR *pszFileName,
				  IMG_UINT32 ui32FileOffset,
				  IMG_UINT32 ui32Width,
				  IMG_UINT32 ui32Height,
				  IMG_UINT32 ui32StrideInBytes,
				  IMG_DEV_VIRTADDR sDevBaseAddr,
				  PVRSRV_DEVMEMCTX hDevMemContext,
				  IMG_UINT32 ui32Size,
				  PDUMP_PIXEL_FORMAT ePixelFormat,
				  IMG_UINT32 ui32AddrMode,
				  IMG_UINT32 ui32PDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(psDevConnection);
	PVR_UNREFERENCED_PARAMETER(pszFileName);
	PVR_UNREFERENCED_PARAMETER(ui32FileOffset);
	PVR_UNREFERENCED_PARAMETER(ui32Width);
	PVR_UNREFERENCED_PARAMETER(ui32Height);
	PVR_UNREFERENCED_PARAMETER(ui32StrideInBytes);
	PVR_UNREFERENCED_PARAMETER(sDevBaseAddr);
	PVR_UNREFERENCED_PARAMETER(hDevMemContext);
	PVR_UNREFERENCED_PARAMETER(ui32Size);
	PVR_UNREFERENCED_PARAMETER(ePixelFormat);
	PVR_UNREFERENCED_PARAMETER(ui32AddrMode);
	PVR_UNREFERENCED_PARAMETER(ui32PDumpFlags);
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPdumpImageDescriptor)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPdumpImageDescriptor(const PVRSRV_DEV_CONNECTION *psDevConnection,
						   const PVRSRV_DEVMEMCTX psContext,
						   const IMG_CHAR *pszFileName,
						   const PVRSRV_PDUMP_IMAGE_DESCRIPTOR *psDesc,
						   IMG_UINT32 ui32PDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(psDevConnection);
	PVR_UNREFERENCED_PARAMETER(psContext);
	PVR_UNREFERENCED_PARAMETER(pszFileName);
	PVR_UNREFERENCED_PARAMETER(psDesc);
	PVR_UNREFERENCED_PARAMETER(ui32PDumpFlags);
	return PVRSRV_OK;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpDataDescriptor)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpDataDescriptor(const PVRSRV_DEV_CONNECTION *psDevConnection,
						   const PVRSRV_DEVMEMCTX psContext,
						   const IMG_CHAR *pszFileName,
						   const PVRSRV_PDUMP_DATA_DESCRIPTOR *psDesc,
						   IMG_UINT32 ui32PDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(psDevConnection);
	PVR_UNREFERENCED_PARAMETER(psContext);
	PVR_UNREFERENCED_PARAMETER(pszFileName);
	PVR_UNREFERENCED_PARAMETER(psDesc);
	PVR_UNREFERENCED_PARAMETER(ui32PDumpFlags);
	return PVRSRV_OK;
}
#endif	/* PDUMP */


#if defined (__cplusplus)
}
#endif

#endif	/* PVRSRV_DEVMEM_PDUMP_H */
