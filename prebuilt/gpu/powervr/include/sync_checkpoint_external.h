/*************************************************************************/ /*!
@File
@Title          Services external synchronisation checkpoint interface header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Defines synchronisation checkpoint structures that are visible
				internally and externally
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef SYNC_CHECKPOINT_EXTERNAL_H
#define SYNC_CHECKPOINT_EXTERNAL_H

#include "img_types.h"

#ifndef CHECKPOINT_TYPES
#define CHECKPOINT_TYPES
typedef struct _SYNC_CHECKPOINT_CONTEXT *PSYNC_CHECKPOINT_CONTEXT;

typedef struct _SYNC_CHECKPOINT *PSYNC_CHECKPOINT;
#endif

/* PVRSRV_SYNC_CHECKPOINT states.
 * The OS native sync implementation should call pfnIsSignalled() to determine if a
 * PVRSRV_SYNC_CHECKPOINT has signalled (which will return an IMG_BOOL), but can set the
 * state for a PVRSRV_SYNC_CHECKPOINT (which is currently in the NOT_SIGNALLED state)
 * where that PVRSRV_SYNC_CHECKPOINT is representing a foreign sync.
 */
typedef IMG_UINT32 PVRSRV_SYNC_CHECKPOINT_STATE;

#define PVRSRV_SYNC_CHECKPOINT_UNDEF         0x000U
#define PVRSRV_SYNC_CHECKPOINT_ACTIVE        0xac1U  /*!< checkpoint has not signalled */
#define PVRSRV_SYNC_CHECKPOINT_SIGNALLED     0x519U  /*!< checkpoint has signalled */
#define PVRSRV_SYNC_CHECKPOINT_ERRORED       0xeffU   /*!< checkpoint has been errored */


#define PVRSRV_UFO_IS_SYNC_CHECKPOINT_FWADDR(fwaddr)	((fwaddr) & 0x1U)
#define PVRSRV_UFO_IS_SYNC_CHECKPOINT(ufoptr)			(PVRSRV_UFO_IS_SYNC_CHECKPOINT_FWADDR((ufoptr)->puiAddrUFO.ui32Addr))

/* Maximum number of sync checkpoints the firmware supports in one fence */
#define MAX_SYNC_CHECKPOINTS_PER_FENCE 32U

/*!
 * Define to be used with SyncCheckpointAlloc() to indicate a checkpoint which
 * represents a foreign sync point or collection of foreign sync points.
 */
#define SYNC_CHECKPOINT_FOREIGN_CHECKPOINT ((PVRSRV_TIMELINE) - 2U)

#endif /* SYNC_CHECKPOINT_EXTERNAL_H */
