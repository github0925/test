/*************************************************************************/ /*!
@File
@Title          Debugging and miscellaneous functions client interface
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Client services functions for debugging and other
                miscellaneous functionality.
@License        Strictly Confidential.
*/ /**************************************************************************/

#if ! defined(RGXAPI_FWDBG_H)
#define RGXAPI_FWDBG_H

/**************************************************************************/ /*!
@Function       RGXFWDebugInitFWImage
@Description    Callback to fill the firmware code allocations with the contents of a
                given buffer.
@Input          psConnection		Services connection
@Input          hFWDestImgMemDesc   MemDesc containing the actual firmware code
                                    memory allocation
@Input          hFWSrcImgMemDesc    MemDesc containing the firmware code data which
                                    is to be copied into FWDestImg
@Input          uiFWImgLen          Length in bytes of FWSrcImg
@Input          hSigMemDesc         MemdDesc containing a signature which is used to
                                    verify the authenticity of FWSrcImg
@Input          uiSigLen            Length in bytes of Sig
@Return         PVRSRV_ERROR
 */ /**************************************************************************/

IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV
RGXFWDebugInitFWImage(
	const PVRSRV_DEV_CONNECTION  *psConnection,
	PVRSRV_MEMDESC hFWDestImgMemDesc,
	PVRSRV_MEMDESC hFWSrcImgMemDesc,
	IMG_DEVMEM_SIZE_T uiFWImgLen,
	PVRSRV_MEMDESC hSigMemDesc,
	IMG_UINT32 uiSigLen);


/**************************************************************************/ /*!
@Function       RGXFWDebugSetFWLog
@Description    Sets the RGX FW Log type
@Input          psConnection		Services connection
@Input          ui32RGXFWLogType  RGX FW Log Type (see RGXFWIF_LOG_TYPE_* defines)
@Return         IMG_BOOL			True if the operation was successful
 */ /**************************************************************************/
IMG_EXPORT
IMG_BOOL IMG_CALLCONV
RGXFWDebugSetFWLog(const PVRSRV_DEV_CONNECTION *psConnection, IMG_UINT32 ui32RGXFWLogType);

IMG_EXPORT
IMG_BOOL IMG_CALLCONV
RGXFWDebugHCSDeadline(const PVRSRV_DEV_CONNECTION *psConnection, IMG_UINT32 ui32HCSDeadlineMS);

IMG_EXPORT
IMG_BOOL IMG_CALLCONV
RGXFWDebugSetOSidPriority(const PVRSRV_DEV_CONNECTION *psConnection, IMG_UINT32 ui32OSid, IMG_UINT32 ui32OSidPriority);

IMG_EXPORT
IMG_BOOL IMG_CALLCONV
RGXFWDebugSetOSNewOnlineState(const PVRSRV_DEV_CONNECTION *psConnection, IMG_UINT32 ui32OSid, IMG_UINT32 ui32OSNewState);

IMG_EXPORT
IMG_BOOL IMG_CALLCONV
RGXFWDebugPHRConfigure(const PVRSRV_DEV_CONNECTION *psConnection, IMG_UINT32 ui32PHRMode);

/**************************************************************************/ /*!
@Function       RGXFWDebugDumpFreelists
@Description    Dumps all physical page addresses that have been allocated for all current freelists
@Input          psConnection		Services connection
@Input          psDevData			Device data
@Return         IMG_BOOL			True if the operation was successful
 */ /**************************************************************************/
IMG_EXPORT
IMG_BOOL IMG_CALLCONV
RGXFWDebugDumpFreelistPageList(const PVRSRV_DEV_CONNECTION *psConnection);
#endif
