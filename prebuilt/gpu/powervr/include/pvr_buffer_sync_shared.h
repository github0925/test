/*************************************************************************/ /*!
@File
@Title          PVR buffer sync shared
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Shared definitions between client and server
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVR_BUFFER_SYNC_SHARED_H
#define PVR_BUFFER_SYNC_SHARED_H

#define PVR_BUFFER_FLAG_READ		(1 << 0)
#define PVR_BUFFER_FLAG_WRITE		(1 << 1)
#define PVR_BUFFER_FLAG_MASK		(PVR_BUFFER_FLAG_READ | \
									 PVR_BUFFER_FLAG_WRITE)

/* Maximum number of PMRs passed
 * in a kick when using buffer sync
 */
#define PVRSRV_MAX_BUFFERSYNC_PMRS 32

#endif /* PVR_BUFFER_SYNC_SHARED_H */
