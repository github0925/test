/*************************************************************************/ /*!
@File
@Title          PVR Bridge Log Functionality
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Include this to get timings from bridge calls.
                This should be used for DEBUGGING.
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _PVR_BRIDGE_LOG_H_
#define _PVR_BRIDGE_LOG_H_

#include "img_defs.h"
#include "services.h"

typedef struct _PVRSRV_BRIDGE_LOG_ENTRY_
{
	IMG_UINT64 uiTime;
	IMG_UINT64 uiBridgeGroup;
	IMG_UINT64 uiBridgeID;
} PVRSRV_BRIDGE_LOG_ENTRY;

#define PVRSRV_BRIDGE_LOG_ENTRIES 20

IMG_EXPORT void PVRSRVBridgeLog_LogReset(void);
IMG_EXPORT void PVRSRVBridgeLog_GetLog(PVRSRV_BRIDGE_LOG_ENTRY* psLog);
IMG_EXPORT IMG_UINT32 PVRSRVBridgeLog_GetNumElementsInLog(void);
IMG_EXPORT IMG_UINT32 PVRSRVBridgeLog_GetLogSize(void);


#endif /* _PVR_BRIDGE_LOG_H_ */
