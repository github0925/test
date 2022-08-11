/*************************************************************************/ /*!
@File
@Title          Validation API Header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Exported API used for validation
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _PVRSRV_VALIDATION_H_
#define _PVRSRV_VALIDATION_H_

#include "img_types.h"
#include "img_defs.h"
#include "pvrsrv_error.h"
#include "services.h"

#if defined (__cplusplus)
extern "C" {
#endif

#if defined(SUPPORT_VALIDATION)

/**************************************************************************/ /*!
@Function       PVRSRVValidationGPUUnitsPowerChange
@Description    Validation of GPU units power change
                Changes the power up state of GPU units.
@Input          psDevConnection      Services connection
@Input          ui32NewValue         Value to set - how this is interpreted
                                     will depend upon the core-specific code
                                     invoked in the server
@Return                              PVRSRV_OK on success. Otherwise, a PVRSRV_
                                     error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVValidationGPUUnitsPowerChange(const PVRSRV_DEV_CONNECTION *psDevConnection,
                                    IMG_UINT32 ui32NewValue);


/**************************************************************************/ /*!
@Function       PVRSRVKickDevices
@Description    Triggers the server to perform a PVRSRVCheckStatus().
@Input          psDevConnection     Pointer to the PVRSRV_DEV_CONNECTION context
@Return         PVRSRV_OK on success, a failure code otherwise.
 */ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVKickDevices(const PVRSRV_DEV_CONNECTION *psConnection);


/**************************************************************************/ /*!
@Function       PVRSRVResetHWRLogs
@Description    Resets the HWR Logs buffer (the HWR count is not reset)
@Input          psDevConnection     Pointer to the PVRSRV_DEV_CONNECTION context
@Return         PVRSRV_OK on success, a failure code otherwise.
 */ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVResetHWRLogs(const PVRSRV_DEV_CONNECTION  *psDevConnection);

/**************************************************************************/ /*!
@Function       PVRSRVVerifyBVNC
@Description    Verifies that the hardware BVNC values are valid.
@Input          psDevConnection     Pointer to the PVRSRV_DEV_CONNECTION context
@Return         PVRSRV_OK on success, a failure code otherwise.
 */ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVVerifyBVNC(const PVRSRV_DEV_CONNECTION *psConnection, IMG_UINT64 ui64GivenBVNC, IMG_UINT64 ui64CoreIdMask);


/**************************************************************************/ /*!
@Function       PVRSRVSoftReset
@Description    Resets some modules of the device
@Input          psDevConnection  Services connection
@Input          ui64ResetValue1  A mask for which each bit set corresponds
                                 to a module to reset (via the SOFT_RESET
                                 register).
@Input          ui64ResetValue2  A mask for which each bit set corresponds
                                 to a module to reset (via the SOFT_RESET2
                                 register).
@Return         PVRSRV_ERROR
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVSoftReset(const PVRSRV_DEV_CONNECTION  *psDevConnection,
                   IMG_UINT64 ui64ResetValue1,
                   IMG_UINT64 ui64ResetValue2);

#endif /* SUPPORT_VALIDATION */

#endif /* _PVRSRV_VALIDATION_H_ */

/******************************************************************************
 End of file (pvrsrv_validation.h)
******************************************************************************/

