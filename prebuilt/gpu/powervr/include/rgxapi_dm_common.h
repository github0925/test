/*************************************************************************/ /*!
@File
@Title          RGX API Header for types common across Data Masters
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Exported RGX API details
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef RGXAPI_DM_COMMON_H
#define RGXAPI_DM_COMMON_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "pvrsrv_devmem_miw.h"
#include "pvrsrv_devvar.h"

/******************************************************************************
 Structures used in API calls
******************************************************************************/

/* Opaque RGX cCCB handle used with CreateCCB */
typedef struct _RGX_CLIENT_CCB_ *PRGX_CLIENT_CCB;


/*!
  Parameters for a Kick Dump Buffer Request
 */
typedef struct RGX_KICK_DUMP_BUFFER_TAG
{
	IMG_DEVMEM_SIZE_T	uiSpaceUsed;
	IMG_DEVMEM_OFFSET_T	uiStart;							/*!< Byte offset of start to dump */
	IMG_DEVMEM_OFFSET_T	uiEnd;								/*!< Byte offset of end of dump (non-inclusive) */
	IMG_DEVMEM_SIZE_T	uiBufferSize;						/*!< Size of buffer */
	IMG_DEVMEM_SIZE_T	uiBackEndLength;					/*!< Size of back end portion, if End < Start */
	IMG_UINT32			uiAllocIndex;                       /*!< allocation index */
	PVRSRV_MEMDESC		hMemDesc;							/*!< hMemDesc handle for the circular buffer */
	PDEVVAR				psDevVar;							/*!< Device variable that's associated with this buffer */
	PVRSRV_MEMINFO*		psMemInfo;
	IMG_DEVMEM_OFFSET_T	uiSyncReadOffset;					/*!< Device virtual address of the memory in the
															     control structure to be checked */
	IMG_PCHAR			pszName;							/*!< Name of buffer */
} RGX_KICK_DUMP_BUFFER, *PRGX_KICK_DUMP_BUFFER;


#if defined (__cplusplus)
}
#endif

#endif /* RGXAPI_DM_COMMON_H */

/******************************************************************************
 End of file (rgxapi_dm_common.h)
******************************************************************************/

