/*************************************************************************/ /*!
@File
@Title          PowerVR device type definitions
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(PVRSRV_DEVICE_TYPES_H)
#define PVRSRV_DEVICE_TYPES_H

#include "img_types.h"

#define PVRSRV_MAX_DEVICES		16	/*!< Largest supported number of devices on the system */

#if defined(__KERNEL__) && defined(LINUX) && !defined(__GENKSYMS__)
#define __pvrsrv_defined_struct_enum__
#include <services_kernel_client.h>
#endif

#endif /* PVRSRV_DEVICE_TYPES_H */
