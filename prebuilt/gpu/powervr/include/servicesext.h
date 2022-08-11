/*************************************************************************/ /*!
@File
@Title          Services definitions required by external drivers
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Provides services data structures, defines and prototypes
                required by external drivers
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(SERVICESEXT_H)
#define SERVICESEXT_H

/* include/ */
#include "pvrsrv_error.h"
#include "img_types.h"
#include "img_3dtypes.h"
#include "pvrsrv_device_types.h"

/*
 * Lock buffer read/write flags
 */
#define PVRSRV_LOCKFLG_READONLY		(1)		/*!< The locking process will only read the locked surface */

/*!
 *****************************************************************************
 *	Services State
 *****************************************************************************/
typedef enum _PVRSRV_SERVICES_STATE_
{
	PVRSRV_SERVICES_STATE_UNDEFINED = 0,
	PVRSRV_SERVICES_STATE_OK,
	PVRSRV_SERVICES_STATE_BAD,
} PVRSRV_SERVICES_STATE;


/*!
 *****************************************************************************
 *	States for power management
 *****************************************************************************/
/*!
  System Power State Enum
 */
typedef enum _PVRSRV_SYS_POWER_STATE_
{
	PVRSRV_SYS_POWER_STATE_Unspecified		= -1,	/*!< Unspecified : Uninitialised */
	PVRSRV_SYS_POWER_STATE_OFF				= 0,	/*!< Off */
	PVRSRV_SYS_POWER_STATE_ON				= 1,	/*!< On */

	PVRSRV_SYS_POWER_STATE_FORCE_I32 = 0x7fffffff	/*!< Force enum to be at least 32-bits wide */

} PVRSRV_SYS_POWER_STATE, *PPVRSRV_SYS_POWER_STATE; /*!< Typedef for ptr to PVRSRV_SYS_POWER_STATE */

/*!
  Device Power State Enum
 */
typedef enum _PVRSRV_DEV_POWER_STATE_
{
	PVRSRV_DEV_POWER_STATE_DEFAULT	= -1,	/*!< Default state for the device */
	PVRSRV_DEV_POWER_STATE_OFF		= 0,	/*!< Unpowered */
	PVRSRV_DEV_POWER_STATE_ON		= 1,	/*!< Running */

	PVRSRV_DEV_POWER_STATE_FORCE_I32 = 0x7fffffff	/*!< Force enum to be at least 32-bits wide */

} PVRSRV_DEV_POWER_STATE, *PPVRSRV_DEV_POWER_STATE;	/*!< Typedef for ptr to PVRSRV_DEV_POWER_STATE */ /* PRQA S 3205 */


/* Power transition handler prototypes */

/*!
  Typedef for a pointer to a Function that will be called before a transition
  from one power state to another. See also PFN_POST_POWER.
 */
typedef PVRSRV_ERROR (*PFN_PRE_POWER) (IMG_HANDLE				hDevHandle,
									   PVRSRV_DEV_POWER_STATE	eNewPowerState,
									   PVRSRV_DEV_POWER_STATE	eCurrentPowerState,
									   IMG_BOOL					bForced);
/*!
  Typedef for a pointer to a Function that will be called after a transition
  from one power state to another. See also PFN_PRE_POWER.
 */
typedef PVRSRV_ERROR (*PFN_POST_POWER) (IMG_HANDLE				hDevHandle,
										PVRSRV_DEV_POWER_STATE	eNewPowerState,
										PVRSRV_DEV_POWER_STATE	eCurrentPowerState,
										IMG_BOOL				bForced);

/* Clock speed handler prototypes */

/*!
  Typedef for a pointer to a Function that will be called before a transition
  from one clock speed to another. See also PFN_POST_CLOCKSPEED_CHANGE.
 */
typedef PVRSRV_ERROR (*PFN_PRE_CLOCKSPEED_CHANGE) (IMG_HANDLE				hDevHandle,
												   PVRSRV_DEV_POWER_STATE	eCurrentPowerState);

/*!
  Typedef for a pointer to a Function that will be called after a transition
  from one clock speed to another. See also PFN_PRE_CLOCKSPEED_CHANGE.
 */
typedef PVRSRV_ERROR (*PFN_POST_CLOCKSPEED_CHANGE) (IMG_HANDLE				hDevHandle,
													PVRSRV_DEV_POWER_STATE	eCurrentPowerState);

/*!
  Typedef for a pointer to a function that will be called to transition the
  device to a forced idle state. Used in unison with (forced) power requests,
  DVFS and cluster count changes.
 */
typedef PVRSRV_ERROR (*PFN_FORCED_IDLE_REQUEST) (IMG_HANDLE		hDevHandle,
												 IMG_BOOL		bDeviceOffPermitted);

/*!
  Typedef for a pointer to a function that will be called to cancel a forced
  idle state and return the firmware back to a state where the hardware can be
  scheduled.
 */
typedef PVRSRV_ERROR (*PFN_FORCED_IDLE_CANCEL_REQUEST) (IMG_HANDLE	hDevHandle);

typedef PVRSRV_ERROR (*PFN_GPU_UNITS_POWER_CHANGE) (IMG_HANDLE		hDevHandle,
													IMG_UINT32		ui32SESPowerState);

/*!
 *****************************************************************************
 * This structure is used for OS independent registry (profile) access
 *****************************************************************************/

typedef struct PVRSRV_REGISTRY_INFO_TAG
{
	IMG_UINT32	ui32DevCookie;
	IMG_PCHAR	pszKey;
	IMG_PCHAR	pszValue;
	IMG_PCHAR	pszBuf;
	IMG_UINT32	ui32BufSize;
} PVRSRV_REGISTRY_INFO, *PPVRSRV_REGISTRY_INFO;

#endif /* SERVICESEXT_H */
/******************************************************************************
 End of file (servicesext.h)
******************************************************************************/
