/*************************************************************************/ /*!
@File
@Title          X Device Memory Management PDump API
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVRSRV_DEVMEMX_PDUMP_H
#define PVRSRV_DEVMEMX_PDUMP_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "pdumpdefs.h"
#include "pvrsrv_devmemx.h"

#if defined(PDUMP)
/**************************************************************************/ /*!
@Function       PVRSRVDevMemXPDumpLoadMem
@Description    Same as PVRSRVPDumpDevmemLoadMem but takes a physical descriptor
@Input          hMemDescPhys        Handle of the physical memory descriptor
@Input          uiOffset            byte offset
@Input          uiSize              byte size
@Input          uiPDumpFlags        pdump flags
@Return         None
*/ /***************************************************************************/
IMG_EXPORT void
PVRSRVDevMemXPDumpLoadMem(PVRSRV_MEMDESC_PHYSICAL hMemDescPhys,
                          IMG_DEVMEM_OFFSET_T uiOffset,
                          IMG_DEVMEM_SIZE_T uiSize,
                          IMG_UINT32 uiPDumpFlags);
#else

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVDevMemXPDumpLoadMem)
#endif

static INLINE void
PVRSRVDevMemXPDumpLoadMem(PVRSRV_MEMDESC_PHYSICAL hMemDescPhys,
                          IMG_DEVMEM_OFFSET_T uiOffset,
                          IMG_DEVMEM_SIZE_T uiSize,
                          IMG_UINT32 uiPDumpFlags)
{
	PVR_UNREFERENCED_PARAMETER(hMemDescPhys);
	PVR_UNREFERENCED_PARAMETER(uiOffset);
	PVR_UNREFERENCED_PARAMETER(uiSize);
	PVR_UNREFERENCED_PARAMETER(uiPDumpFlags);
}

#endif /* PDUMP */

#if defined (__cplusplus)
}
#endif

#endif /* PVRSRV_DEVMEMX_PDUMP_H */
