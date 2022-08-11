/*************************************************************************/ /*!
@File           pvr_fd_sync_kernel.h
@Title          Kernel/userspace interface definitions to use the kernel sync
                driver
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
/* vi: set ts=8: */


#ifndef _PVR_FD_SYNC_KERNEL_H_
#define _PVR_FD_SYNC_KERNEL_H_

#include <linux/types.h>
#include <linux/ioctl.h>

#define PVR_SYNC_MAX_QUERY_FENCE_POINTS 14

#define PVR_SYNC_IOC_MAGIC 'W'

#define PVR_SYNC_IOC_RENAME \
 _IOW(PVR_SYNC_IOC_MAGIC,  4, struct pvr_sync_rename_ioctl_data)

#define PVR_SYNC_IOC_FORCE_SW_ONLY \
 _IO(PVR_SYNC_IOC_MAGIC,   5)

struct pvr_sync_pt_info {
	/* Output */
	__u32 id;
	__u32 ui32FWAddr;
	__u32 ui32CurrOp;
	__u32 ui32NextOp;
	__u32 ui32TlTaken;
} __attribute__((packed, aligned(8)));

struct pvr_sync_rename_ioctl_data
{
	/* Input */
	char szName[32];
} __attribute__((packed, aligned(8)));

#endif /* _PVR_FD_SYNC_KERNEL_H_ */
