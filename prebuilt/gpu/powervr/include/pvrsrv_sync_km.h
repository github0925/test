/*************************************************************************/ /*!
@File
@Title         PVR synchronisation interface
@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description   Types for server side code
@License       Strictly Confidential.
*/ /**************************************************************************/
#ifndef PVRSRV_SYNC_KM_H
#define PVRSRV_SYNC_KM_H

#include <powervr/pvrsrv_sync_ext.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define SYNC_FB_FILE_STRING_MAX			256
#define SYNC_FB_MODULE_STRING_LEN_MAX	(32)
#define	SYNC_FB_DESC_STRING_LEN_MAX		(32)

/* By default, fence-sync module emits into HWPerf (of course, if enabled) and
 * considers a process (sleepable) context */
#define PVRSRV_FENCE_FLAG_NONE             (0U)
#define PVRSRV_FENCE_FLAG_SUPPRESS_HWP_PKT (1U << 0)
#define PVRSRV_FENCE_FLAG_CTX_ATOMIC       (1U << 1)

#if defined(__cplusplus)
}
#endif
#endif	/* PVRSRV_SYNC_KM_H */
