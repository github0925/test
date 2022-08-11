/*************************************************************************/ /*!
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _UAPI_LINUX_PVR_SW_SYNC_H
#define _UAPI_LINUX_PVR_SW_SYNC_H

#if defined(SUPPORT_NATIVE_FENCE_SYNC)

#include <linux/types.h>

#include "pvrsrv_sync_km.h"

struct pvr_sw_sync_create_fence_data {
  char name[PVRSRV_SYNC_NAME_LENGTH];
  __s32 fence;
  __u32 pad;
  __u64 sync_pt_idx;
};

struct pvr_sw_timeline_advance_data {
  __u64 sync_pt_idx;
};

#define PVR_SW_SYNC_IOC_MAGIC 'W'
#define PVR_SW_SYNC_IOC_CREATE_FENCE _IOWR(PVR_SW_SYNC_IOC_MAGIC, 0, struct pvr_sw_sync_create_fence_data)
#define PVR_SW_SYNC_IOC_INC _IOR(PVR_SW_SYNC_IOC_MAGIC, 1, struct pvr_sw_timeline_advance_data)

#endif /* defined(SUPPORT_NATIVE_FENCE_SYNC) */
#endif
