/*************************************************************************/ /*!
@File
@Title          Services API Header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Exported services API details
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef SERVICES_H
#define SERVICES_H


#if defined (__cplusplus)
extern "C" {
#endif

#include "img_defs.h"
#include "servicesext.h"
#include "pdumpdefs.h"
#include "lock_types.h"
#include "pvr_debug.h"
#include "pvrsrv_device_types.h"

#include "services_km.h"
#include "services_client_porting.h"
#if defined(__linux__) && defined(__KERNEL__)
#error services_h included in kernel build!
#endif

/* The comment below is the front page for code-generated doxygen documentation */
/*!
 ******************************************************************************
 @mainpage
 This document details the APIs and implementation of the Consumer Services.
 It is intended to be used in conjunction with the Consumer Services
 Software Architectural Specification and the Consumer Services Software
 Functional Specification.
 *****************************************************************************/

/******************************************************************************
 * 	#defines
 *****************************************************************************/

#define EVENTOBJNAME_MAXLENGTH (50) /*!< Max length of an event object name */

/*
 * Specifies to PVRSRVConnectionCreate that an existing OS connection should
 * not be used.
 */
#define IMG_OS_CONNECTION_NONE (-1)

/******************************************************************************
 * Structure definitions.
 *****************************************************************************/

/*
 * The Device ID structure can be extended to also identify the device type
 * when support is added to the driver
 */
typedef struct {
	IMG_INT iDeviceID;
} PVRSRV_DEVICE_ID;

/*************************************************************************/ /*!
    PVR Client Event handling in Services
*/ /**************************************************************************/
typedef enum _PVRSRV_CLIENT_EVENT_
{
	PVRSRV_CLIENT_EVENT_HWTIMEOUT = 0,              /*!< hw timeout event */
} PVRSRV_CLIENT_EVENT;

typedef struct PVRSRV_TASK_CONTEXT_SETUP_TAG
{
#if defined(GTRACE_TOOL)
	struct GTraceSetupTAG	*psGTraceSetup;
#else
	/* This is to stop compile failure in integrity as it does not like empty structure */
	IMG_BOOL				bDummy;
#endif

} PVRSRV_TASK_CONTEXT_SETUP;

typedef struct _PVRSRV_TASK_CONTEXT_ PVRSRV_TASK_CONTEXT;

/**************************************************************************/ /*!
@Function       PVRSRVClientEvent
@Description    Handles timeouts occurring in client drivers
@Input          eEvent          event type
@Input          psDevConnection       pointer to the PVRSRV_DEV_CONNECTION context
@Input          pvData          client-specific data
@Return                         PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVClientEvent(const PVRSRV_CLIENT_EVENT eEvent,
											PVRSRV_DEV_CONNECTION *psDevConnection,
											void *pvData);

/******************************************************************************
 * PVR Services API prototypes.
 *****************************************************************************/

/**************************************************************************/ /*!
@Function      PVRSRVGetDevices
@Description   Query the driver for a list of device IDs that can be
               connected to. Or just query the number of devices.
@Output        psDeviceID       Pointer to an array of device IDs to populate
                                or NULL to just query the number of devices
@Input         uArraySize       Size of the device ID array
@Output        puNumDevices     The number of devices discovered and listed
                                in the device ID array,
                                or if psDeviceID and uArraySize are NULL and 0
                                respectively, the total number of devices
                                present
@Return        PVRSRV_ERROR     PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code.
 */ /**************************************************************************/
__must_check IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVGetDevices(PVRSRV_DEVICE_ID *psDeviceID,
                                     IMG_UINT uArraySize,
                                     IMG_UINT *puNumDevices);

/**************************************************************************/ /*!
@Function       PVRSRVConnect
@Description    Creates a services connection from an application to the
                services module.
@Output         ppsConnection   on Success, *ppsConnection is set to the new
                                PVRSRV_DEV_CONNECTION instance.
@Input          ui32SrvFlags    a bit-wise OR of the following:
                                SRV_FLAGS_PERSIST
@Return                         PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code
 */ /**************************************************************************/
__must_check IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVConnect(PVRSRV_DEV_CONNECTION **ppsConnection,
                                        IMG_UINT32 ui32SrvFlags);

/**************************************************************************/ /*!
@Function       PVRSRVConnectionCreate
@Description    Creates a services connection from an application to the
                services module using an existing OS connection.

                NOTE: If additional calls are made to this function then it is
                expected that the OS connection will be the same otherwise an
                error will be returned. However, if all connections have been
                closed, i.e. PVRSRVDisconnect has been called the appropriate
                number of times, then a different OS connection can be used.
                Likewise, if PVRSRVConnect has been called then calls to this
                function must be made IMG_OS_CONNECTION_NONE.
@Output         ppsConnection   on Success, *ppsConnection is set to the new
                                PVRSRV_DEV_CONNECTION instance.
@Input          hOSConnection   An existing OS connection to the services module
                                or IMG_OS_CONNECTION_NONE if an existing OS
                                connection should not be used (this is
                                equivalent to calling PVRSRVConnect)
@Input          ui32SrvFlags    a bit-wise OR of the following:
                                SRV_FLAGS_PERSIST
@Return                         PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code
 */ /**************************************************************************/
__must_check IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVConnectionCreate(PVRSRV_DEV_CONNECTION **ppsConnection,
                                                 IMG_OS_CONNECTION hOSConnection,
                                                 IMG_UINT32 ui32SrvFlags);

/**************************************************************************/ /*!
@Function       PVRSRVConnectionCreateDevice
@Description    Creates a services connection from an application to the
                services module using a particular device.
@Output         ppsConnection   on Success, *ppsConnection is set to the new
                                PVRSRV_DEV_CONNECTION instance.
@Input          iDeviceID       the device ID of the device to connect to
@Input          ui32SrvFlags    a bit-wise OR of the following:
                                SRV_FLAGS_PERSIST
@Return                         PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code
 */ /**************************************************************************/
__must_check IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVConnectionCreateDevice(PVRSRV_DEV_CONNECTION **ppsConnection,
                                                 IMG_INT iDeviceID,
                                                 IMG_UINT32 ui32SrvFlags);

/**************************************************************************/ /*!
@Function       PVRSRVDisconnect
@Description    Disconnects from the services module
@Input          psConnection    the connection to be disconnected
@Return                         PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVDisconnect(PVRSRV_DEV_CONNECTION *psConnection);


/**************************************************************************/ /*!
@Function       PVRSRVPollForValue
@Description    Polls for a value to match a masked read of System Memory.
                The function returns when either (1) the value read back
                matches ui32Value, or (2) the maximum number of tries has
                been reached.
@Input          psConnection        Services connection
@Input          hOSEvent            Handle to OS event to wait for
@Input          pui32LinMemAddr     the address of the memory to poll
@Input          ui32Value           the required value
@Input          ui32Mask            the mask to use
@Input          ui32Waitus          interval between tries (us)
@Input          ui32Tries           number of tries to make before giving up
@Return                             PVRSRV_OK on success. Otherwise, a
                                    PVRSRV_ error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR PVRSRVPollForValue(const PVRSRV_DEV_CONNECTION	*psConnection,
								IMG_HANDLE				hOSEvent,
								volatile IMG_UINT32		*pui32LinMemAddr,
								IMG_UINT32				ui32Value,
								IMG_UINT32				ui32Mask,
								IMG_UINT32				ui32Waitus,
								IMG_UINT32				ui32Tries);

/* this function is almost the same as PVRSRVPollForValue. The only difference
 * is that it now handles the interval between tries itself. Therefore it can
 * correctly handles the differences between the different platforms.
 */
IMG_EXPORT
PVRSRV_ERROR PVRSRVWaitForValue(const PVRSRV_DEV_CONNECTION	*psConnection,
                                IMG_HANDLE				hOSEvent,
                                volatile IMG_UINT32		*pui32LinMemAddr,
                                IMG_UINT32				ui32Value,
                                IMG_UINT32				ui32Mask);


/**************************************************************************/ /*!
 @Function      PVRSRVConditionCheckCallback
 @Description   Function prototype for use with the PVRSRVWaitForCondition()
                API. Clients implement this callback to test if the condition
                waited for has been met and become true.

 @Input         pvUserData      Pointer to client user data needed for
                                 the check
 @Output        pbCondMet       Updated on exit with condition state

 @Return        PVRSRV_OK  when condition tested without error
                PVRSRV_*   other system error that will lead to the
                           abnormal termination of the wait API.
 */
/******************************************************************************/
typedef
PVRSRV_ERROR (*PVRSRVConditionCheckCallback)(
        void       *pvUserData,
        IMG_BOOL*  pbCondMet);


/**************************************************************************/ /*!
@Function       PVRSRVWaitForCondition
@Description    Wait using PVRSRVEventObjectWait() for a
                condition (pfnCallback) to become true. It periodically
                checks the condition state by employing a loop and
                waiting on either the event supplied or sleeping for a brief
                time (if hEvent is null) each time the condition is
                checked and found not to be met. When the condition is true
                the function returns. It will also return when the time
                period has been exceeded or an error has occurred.

@Input          psConnection    Services connection
@Input          hEvent          Event to wait on or NULL not to use event
                                 objects but OS wait for a short time.
@Input          pfnCallback     Client condition check callback
@Input          pvUserData      Client user data supplied to callback

@Return         PVRSRV_OK	          When condition met
                PVRSRV_ERROR_TIMEOUT  When condition not met and time is up
                PVRSRV_*              Otherwise, some other error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVWaitForCondition(
        const PVRSRV_DEV_CONNECTION*     psConnection,
        IMG_HANDLE                   hEvent,
        PVRSRVConditionCheckCallback pfnCallback,
        void                         *pvUserData);

/**************************************************************************/ /*!
@Function       PVRSRVWaitForConditionCustomTimeout
@Description    Wait using PVRSRVEventObjectWait() for a
                condition (pfnCallback) to become true. It periodically
                checks the condition state by employing a loop and
                waiting on either the event supplied or sleeping
                (if hEvent is null) each time the condition is
                checked and found not to be met. When the condition is true
                the function returns. It will also return when the time
                period has been exceeded or an error has occurred.

@Input          psConnection    Services connection
@Input          hEvent          Event to wait on or NULL not to use event
                                 objects but OS wait for a short time.
@Input          pfnCallback     Client condition check callback
@Input          pvUserData      Client user data supplied to callback
@Input          ui32Timeus      interval between tries (us)
@Input          ui32Tries       number of tries to make before giving up

@Return         PVRSRV_OK	          When condition met
                PVRSRV_ERROR_TIMEOUT  When condition not met and time is up
                PVRSRV_*              Otherwise, some other error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVWaitForConditionCustomTimeout(const PVRSRV_DEV_CONNECTION*     psConnection,
                                                 IMG_HANDLE                   hEvent,
                                                 PVRSRVConditionCheckCallback pfnCallback,
                                                 void                         *pvUserData,
                                                 IMG_UINT32                   ui32Timeus,
                                                 IMG_UINT32                   ui32Tries);

/******************************************************************************
 * PDUMP Function prototypes...
 *****************************************************************************/
#if defined(PDUMP)
/**************************************************************************/ /*!
@Function       PVRSRVPDumpInit
@Description    Pdump initialisation
@Input          psConnection    Services connection
@Return                         PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpInit(const PVRSRV_DEV_CONNECTION *psConnection);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpSetFrame
@Description    Sets the pdump frame
@Input          psConnection    Services connection
@Input          ui32Frame       frame id
@Return                         PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpSetFrame(PVRSRV_DEV_CONNECTION *psConnection,
											  IMG_UINT32 ui32Frame);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpGetFrame
@Description    Gets the current pdump frame
@Input          psConnection    Services connection
@Output         pui32Frame       frame id
@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_error code
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpGetFrame(const PVRSRV_DEV_CONNECTION *psConnection,
											  IMG_UINT32 *pui32Frame);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpIsLastCaptureFrame
@Description    Returns whether this is the last frame of the capture range
@Input          psConnection    Services connection
@Return                         IMG_TRUE if last frame,
                                IMG_FALSE otherwise
*/ /**************************************************************************/
IMG_EXPORT
IMG_BOOL IMG_CALLCONV PVRSRVPDumpIsLastCaptureFrame(const PVRSRV_DEV_CONNECTION *psConnection);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpForceCaptureStop
@Description    Forces PDump capture to STOP, currently this call is valid in
                BLKMODE of PDump only
@Input          psConnection    Services connection
@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_error code
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpForceCaptureStop(PVRSRV_DEV_CONNECTION *psConnection);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpBeforeRender
@Description    Executes SignatureCheck commands to confirm data integrity
@Input          psDevConnection    Device connection data
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpBeforeRender(PVRSRV_DEV_CONNECTION *psDevConnection);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpAfterRender
@Description    Executes TraceBuffer and SignatureBuffer commands
@Input          psDevConnection    Device connection data
*/ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpAfterRender(PVRSRV_DEV_CONNECTION *psDevConnection);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpComment
@Description    PDumps a comment (unformatted), passing in flags
@Input          psConnection        Services connection
@Input          ui32Flags           Flags
@Input          pszComment          Comment to be inserted
@Return                             PVRSRV_OK on success. Otherwise, a PVRSRV_
                                    error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpComment(const PVRSRV_DEV_CONNECTION *psConnection,
									   IMG_UINT32 ui32Flags,
									   const IMG_CHAR *pszComment);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpCommentf
@Description    PDumps a formatted comment, passing in flags
@Input          psConnection        Services connection
@Input          ui32Flags           Flags
@Input          pszFormat           Format string
@Input          ...                 vararg list
@Return                             PVRSRV_OK on success. Otherwise, a PVRSRV_
                                    error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpCommentf(const PVRSRV_DEV_CONNECTION *psConnection,
									   IMG_UINT32 ui32Flags,
									   const IMG_CHAR *pszFormat, ...)
									   __printf(3, 4);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpIsCapturing
@Description    checks whether pdump is currently in frame capture range
@Input          psConnection        Services connection
@Output         pbIsCapturing       Indicates PDump capturing state
@Return                             PVRSRV_OK on success. Otherwise, a PVRSRV_
                                    error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpIsCapturing(const PVRSRV_DEV_CONNECTION *psConnection,
								 				IMG_BOOL *pbIsCapturing);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpIsCapturing
@Description    checks whether pdump connected
@Input          psConnection        Services connection
@Output         pbIsCapturing       Indicates PDump client connection state
@Return                             PVRSRV_OK on success. Otherwise, a PVRSRV_
                                    error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpIsConnected(const PVRSRV_DEV_CONNECTION *psConnection,
								 				IMG_BOOL *pbIsConnected);

/**************************************************************************/ /*!
@Function       PVRSRVPdumpIsCaptureSuspended
@Description    checks whether pdump is suspended
@Input          psConnection         Services connection
@Output         pbIsCaptureSuspended Indicates PDump state is suspended
@Return                              PVRSRV_OK on success. Otherwise, a PVRSRV_
                                     error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPdumpIsCaptureSuspended(const PVRSRV_DEV_CONNECTION *psConnection,
													IMG_BOOL *pbIsCaptureSuspended);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpIsCapturingTest
@Description    checks whether pdump is currently in frame capture range
@Input          psConnection        Services connection
@Return         IMG_BOOL
 */ /**************************************************************************/
IMG_EXPORT
IMG_BOOL IMG_CALLCONV PVRSRVPDumpIsCapturingTest(const PVRSRV_DEV_CONNECTION *psConnection);

IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpSetDefaultCaptureParams(const PVRSRV_DEV_CONNECTION *psConnection,
                                                             IMG_UINT32 ui32Mode,
                                                             IMG_UINT32 ui32Start,
                                                             IMG_UINT32 ui32End,
                                                             IMG_UINT32 ui32Interval,
                                                             IMG_UINT32 ui32MaxParamFileSize);

/**************************************************************************/ /*!
@Function       PVRSRVPDumpRGXDefsVersioning
@Description    Pdumps the RGX Defs Versioning 
@Input          psConnection        Services connection
@Return                             PVRSRV_OK on success. Otherwise, a PVRSRV_
                                    error code
 */ /**************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpRGXDefsVersioning(PVRSRV_DEV_CONNECTION *psDevConnection);

#else	/* PDUMP */

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpInit)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpInit(const PVRSRV_DEV_CONNECTION *psConnection)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	return PVRSRV_OK;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpSetFrame)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpSetFrame(const PVRSRV_DEV_CONNECTION *psConnection,
					IMG_UINT32 ui32Frame)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(ui32Frame);
	return PVRSRV_OK;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpGetFrame)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpGetFrame(const PVRSRV_DEV_CONNECTION *psConnection,
					IMG_UINT32 *pui32Frame)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(pui32Frame);
	return PVRSRV_OK;
}


#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpIsLastCaptureFrame)
#endif
static INLINE IMG_BOOL
PVRSRVPDumpIsLastCaptureFrame(const PVRSRV_DEV_CONNECTION *psConnection)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	return IMG_FALSE;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpForceCaptureStop)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpForceCaptureStop(PVRSRV_DEV_CONNECTION *psConnection)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	return PVRSRV_OK;
}								 				

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpAfterRender)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpAfterRender(PVRSRV_DEV_CONNECTION *psDevConnection)
{
	PVR_UNREFERENCED_PARAMETER(psDevConnection);
	return PVRSRV_OK;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpComment)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpComment(const PVRSRV_DEV_CONNECTION *psConnection,
				  IMG_UINT32 ui32PDumpFlags,
				  const IMG_CHAR *pszComment)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(ui32PDumpFlags);
	PVR_UNREFERENCED_PARAMETER(pszComment);
	return PVRSRV_OK;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpCommentf)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpCommentf(const PVRSRV_DEV_CONNECTION *psConnection,
					IMG_UINT32 ui32PDumpFlags,
					const IMG_CHAR *pszFormat, ...)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(ui32PDumpFlags);
	PVR_UNREFERENCED_PARAMETER(pszFormat);
	return PVRSRV_OK;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpIsCapturing)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpIsCapturing(const PVRSRV_DEV_CONNECTION *psConnection,
					   IMG_BOOL *pbIsCapturing)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	*pbIsCapturing = IMG_FALSE;
	return PVRSRV_OK;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPdumpIsCaptureSuspended)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPdumpIsCaptureSuspended(const PVRSRV_DEV_CONNECTION *psConnection,
					   IMG_BOOL *pbIsCaptureSuspended)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	*pbIsCaptureSuspended = IMG_FALSE;
	return PVRSRV_OK;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpIsCapturingTest)
#endif
static INLINE IMG_BOOL
PVRSRVPDumpIsCapturingTest(const PVRSRV_DEV_CONNECTION *psConnection)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	return IMG_FALSE;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpSetDefaultCaptureParams)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpSetDefaultCaptureParams(const PVRSRV_DEV_CONNECTION *psConnection,
                                   IMG_UINT32 ui32Mode,
                                   IMG_UINT32 ui32Start,
                                   IMG_UINT32 ui32End,
                                   IMG_UINT32 ui32Interval,
                                   IMG_UINT32 ui32MaxParamFileSize)
{
	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(ui32Mode);
	PVR_UNREFERENCED_PARAMETER(ui32Start);
	PVR_UNREFERENCED_PARAMETER(ui32End);
	PVR_UNREFERENCED_PARAMETER(ui32Interval);
	PVR_UNREFERENCED_PARAMETER(ui32MaxParamFileSize);

	return PVRSRV_OK;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVPDumpRGXDefsVersioning)
#endif
static INLINE PVRSRV_ERROR
PVRSRVPDumpRGXDefsVersioning(PVRSRV_DEV_CONNECTION *psDevConnection)
{
	PVR_UNREFERENCED_PARAMETER(psDevConnection);
	return PVRSRV_OK;
}

#endif	/* PDUMP */

/**************************************************************************/ /*!
@Function       PVRSRVGetDevClockSpeed
@Description    Gets the clock speed
@Input          psDevConnection     Pointer to the PVRSRV_DEV_CONNECTION context
@Output         pui32ClockSpeed     Variable for storing clock speed
@Return         IMG_TRUE if the operation was successful, IMG_FALSE otherwise
 */ /**************************************************************************/
IMG_EXPORT IMG_BOOL IMG_CALLCONV PVRSRVGetDevClockSpeed(const PVRSRV_DEV_CONNECTION  *psDevConnection,
														IMG_PUINT32 pui32ClockSpeed);

/******************************************************************************
 * PVR Global Event Object - Event APIs
 *****************************************************************************/

/*****************************************************************************
@Function       PVRSRVAcquireGlobalEventHandle
@Description    Gets a handle to an event that is opened on the global
                event object.
@Input          psConnection    Services connection
@Output         phEvent         Global event handle
@Return                         PVRSRV_OK on success. Otherwise, a PVRSRV_
                                error code
 */ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVAcquireGlobalEventHandle(const PVRSRV_DEV_CONNECTION *psConnection,
                               IMG_HANDLE *phEvent);

/**************************************************************************/ /*!
@Function       PVRSRVReleaseGlobalEventHandle
@Description    Destroys the event handle previously acquired.
@Input          psConnection    Services connection
@Input          hEvent          Global event handle
@Return         PVRSRV_OK on success. Otherwise, a PVRSRV_ error code
 */ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVReleaseGlobalEventHandle(const PVRSRV_DEV_CONNECTION *psConnection,
                               IMG_HANDLE hEvent);

/**************************************************************************/ /*!
@Function       PVRSRVEventObjectWait
@Description    Wait (block) on the OS-specific event object passed
@Input          psConnection    Services connection
@Input          hEvent          Global event handle to wait on
@Return         PVRSRV_OK on success. PVRSRV_ERROR_INTERRUPTED if OS signal was
                delivered to the process. Otherwise, a PVRSRV_ error code
 */ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVEventObjectWait(const PVRSRV_DEV_CONNECTION *psConnection,
                      IMG_HANDLE hEvent);

/**************************************************************************/ /*!
@Function       PVRSRVEventObjectWaitTimeout
@Description    Wait (block) on the OS-specific event object passed or until
                the specified time has elapsed.
@Input          psConnection    Services connection
@Input          hEvent          Global event handle to wait on
@Input          ui64Timeoutus   Timeout in usecs after which to return if event
                                object has not signalled.
@Return         PVRSRV_OK on success. PVRSRV_ERROR_TIMEOUT if the timeout
                elapses. PVRSRV_ERROR_INTERRUPTED if OS signal was delivered
                to the process. Otherwise, a PVRSRV_ error code
 */ /**************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVEventObjectWaitTimeout(const PVRSRV_DEV_CONNECTION *psConnection,
							 IMG_HANDLE hEvent, IMG_UINT64 ui64Timeoutus);

/******************************************************************************
 * PVR Global Event Object - Event APIs End
 *****************************************************************************/

/*************************************************************************/ /*!
  PVR Shared Virtual Memory (SVM) Allocation Availability
*/ /**************************************************************************/

typedef enum _PVRSRV_SVM_ALLOCATION_SUPPORT_
{
	PVRSRV_SVM_ALLOCATION_UNSUPPORTED = 0, /*!< SVM allocations are not supported */
	PVRSRV_SVM_ALLOCATION_SUPPORTED, /*!< SVM allocations are supported */
	PVRSRV_SVM_ALLOCATION_CANFAIL /*!< SVM allocation are partially supported */
} PVRSRV_SVM_ALLOCATION_SUPPORT;

/**************************************************************************/ /*!
@Function      PVRSRVGetSVMAllocationSupport
@Description   For a given device, returns whether there is shared virtual
               memory allocation support.
@Input         psDevConnection                 Device connection of the device
                                               for which shared-virtual-memory
                                               support should be returned.
@Return        PVRSRV_SVM_ALLOCATION_SUPPORT   What SVM allocation support is available
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_SVM_ALLOCATION_SUPPORT
PVRSRVGetSVMAllocationSupport(PVRSRV_DEV_CONNECTION *psDevConnection);

/*************************************************************************/ /*!
  PVR Cache Coherency Availability
*/ /**************************************************************************/

typedef enum _PVRSRV_CACHE_COHERENCY_SUPPORT_
{
	/*
	 * Neither the CPU or the device are cache coherent.
	 */
	PVRSRV_CACHE_COHERENT_NONE = 0,

	/*
	 * Device caches are coherent with CPU i.e. device can snoop into CPU cache.
	 */
	PVRSRV_CACHE_COHERENT_DEVICE,

	/*
	 * CPU caches are coherent with Device i.e. CPU can snoop into Device cache.
	 */
	PVRSRV_CACHE_COHERENT_CPU,

	/*
	 * CPU caches are coherent with Device i.e. CPU can snoop into Device cache.
	 * AND
	 * Device caches are coherent with CPU i.e. device can snoop into CPU cache.
	 * Therefore there is BIDIRECTIONAL coherency.
	 */
	PVRSRV_CACHE_COHERENT_BIDIRECTIONAL,

	/*
	 * Neither the CPU nor the device are cache coherent but software (driver and
	 * firmware) emulates it to keep both CPU/device cache coherent.
	 */
	PVRSRV_CACHE_COHERENT_CROSSEMULATE

} PVRSRV_CACHE_COHERENCY_SUPPORT;

/**************************************************************************/ /*!
@Function      PVRSRVGetCacheCoherencySupport
@Description   For a given device, returns whether there is no coherency
               support, the device cache is coherent with the CPU, the CPU
               caches are coherent with the device or the coherency is
               bidirectional.
@Input         psDevConnection                 Device connection of the device
                                               for which coherency support
                                               should be returned.
@Return        PVRSRV_CACHE_COHERENCY_SUPPORT  What cache coherency is available
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_CACHE_COHERENCY_SUPPORT
PVRSRVGetCacheCoherencySupport(PVRSRV_DEV_CONNECTION *psDevConnection);

/**************************************************************************/ /*!
@Function       PVRSRVHasCpuNonMappableLocalMemory

@Description    Checks if a device has a non-mappable local memory region.
@Input          psDevConnection    Device connection
@Return         IMG_TRUE if there is non-mappable memory region present,
				IMG_FALSE otherwise.
*/ /***************************************************************************/
IMG_EXPORT IMG_BOOL
PVRSRVHasCpuNonMappableLocalMemory(PVRSRV_DEV_CONNECTION *psDevConnection);

/**************************************************************************/ /*!
@Function       PVRSRVHasFBCDCv31

@Description    Checks if a device uses FBCDC v3.1.
@Input          psDevConnection    Device connection
@Return         IMG_TRUE if there is non-mappable memory region present,
				IMG_FALSE otherwise.
*/ /***************************************************************************/
IMG_EXPORT IMG_BOOL
PVRSRVHasFBCDCv31(PVRSRV_DEV_CONNECTION *psDevConnection);

#if defined(SUPPORT_WORKLOAD_ESTIMATION)
/**************************************************************************/ /*!
 @Function      : PVRSRVSetDeadline
 @Input         : psDevConnection           The connection to the device
                : ui64NextDeadlineInus      The system monotonic time of the
                                            next deadline in microseconds.
 @Return        : PVRSRV_ERROR
 @Description   : API to tell the driver what the predicted deadline is.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVSetDeadline(PVRSRV_DEV_CONNECTION *psDevConnection,
                               IMG_UINT64 ui64NextDeadlineInus);
#endif

/* Comment below is a doxygen documentation for PVRSRV_DEVICE_STATUS enum
 * defined in services_km.h. */

/**************************************************************************/ /*!
 @enum PVRSRV_DEVICE_STATUS

 @brief Status of the device.

 The value of this enumeration represents status of the device.

 @var PVRSRV_DEVICE_STATUS_UNKNOWN

 Status of the device is unknown. This value means that during status
 retrieval something unexpected happened and the correct value couldn't
 be obtained.
 This value doesn't mean that anything wrong is happening with the
 device, it only means that the correct value of the status couldn't
 be obtained.

 @var PVRSRV_DEVICE_STATUS_OK

 The device is operational and in a good shape.

 @var PVRSRV_DEVICE_STATUS_NOT_RESPONDING,

 The device is not responding. Possible causes of this state could be
 that the work item is taking longer than anticipated or that the device
 stopped processing the work queue.
 In either case the correct action would be to wait and recheck
 the status. If the status doesn't change back to OK it might be necessary
 to even reset the device.

 @var PVRSRV_DEVICE_STATUS_DEVICE_ERROR

 The device is not operational due to internal error. Possible causes
 could be FW assert, KCCB corruption or FW poll fail.
 In this case the device is in unrecoverable state and needs to be
 reset.
*/ /***************************************************************************/

/**************************************************************************/ /*!
 @Function      PVRSRVGetDeviceStatus
 @Description   Function returns status of the device, please see
                PVRSRV_DEVICE_STATUS description (in this file) for more details
                about returned values.
 @Input         psConnection     The connection to the device
 @Output        peDeviceStatus   The status of the device
 @Return        PVRSRV_OK on success or PVRSRV_ERROR error code otherwise.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVGetDeviceStatus(PVRSRV_DEV_CONNECTION *psConnection,
                      PVRSRV_DEVICE_STATUS *peDeviceStatus);

/**************************************************************************/ /*!
 @Function      PVRSRVGetMultiCoreInfo
 @Description   Function returns information about a multi-core device.
 @Input         psConnection     The connection to the device
 @Input         ui32CapsSize     The number of entries in the caps array.
                                 If this entry is zero then only the number of
                                 cores will be returned.
 @Output        pui32NumCores    The number of cores in this multi-core device.
 @Output        pui64Caps        Filled in on successful return with the
                                 capabilities for each core. Only NumCores entries
                                 are valid. See register RGX_CR_MULTICORE_GPU
                                 for details of each entry.
 @Return        PVRSRV_SUCCESS on multicore device and pui32Caps is filled in.
                PVRSRV_ERROR_NOT_SUPPORTED on single core device.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVGetMultiCoreInfo(PVRSRV_DEV_CONNECTION *psConnection,
                       IMG_UINT32 ui32CapsSize,
                       IMG_UINT32 *pui32NumCores,
                       IMG_UINT64 *pui64Caps);

#if defined(DEFERRED_WORKER_THREAD)

/**************************************************************************/ /*!
 @Function      PVRSRVGetProcessGlobalTaskContext
 @Description   Get a reference to the process global task context. If there is no
                process global task context then one is created on-demand.
 @Output        ppsTaskContext   The process global task context.
 @Return        PVRSRV_OK on success or a PVRSRV_ERROR error code on failure.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVGetProcessGlobalTaskContext(PVRSRV_TASK_CONTEXT **ppsTaskContext, PVRSRV_TASK_CONTEXT_SETUP *psSetup);

/**************************************************************************/ /*!
 @Function      PVRSRVReleaseProcessGlobalTaskContext
 @Description   Release a reference on the process global task context.
                If there are no more references to the process global task
		context then it is destroyed (and will be re-created as necessary).
 @Return        PVRSRV_OK on success or a PVRSRV_ERROR error code on failure.
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVReleaseProcessGlobalTaskContext(void);

#endif /* defined(DEFERRED_WORKER_THREAD) */

/**************************************************************************/ /*!
 @Function      PVRSRVGetProcessMemStats
 @Description   Provide the caller with the totals of memory statistics for
                Graphics and Kernel memory for an individual process.
 @Input	        psConnection            The connection to the device
 @Input	        pid                     The process to search for
 @Output        *pui32KernelMem	        Total kernel memory allocated by
                                        the process
 @Output        *pui32GraphicsMem       Total graphics memory allocated by
                                        the process
 @Return        PVRSRV_ERROR error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVGetProcessMemStats(PVRSRV_DEV_CONNECTION *psConnection,
                         IMG_PID pid,
                         IMG_UINT32 *pui32KernelMem,
                         IMG_UINT32 *pui32GraphicsMem);

/**************************************************************************/ /*!
 @Function      PVRSRVGetProcessMemStatsTotal
 @Description   Provide the caller with the totals of memory statistics for
                Graphics and Kernel memory for all the live processes.
 @Input         psConnection            The connection to the device
 @Output        *pui32TotalKernelMem    Total kernel memory allocated by
                                        all the processes
 @Output        *pui32TotalGraphicsMem  Total graphics memory allocated by
                                        all the processes
 @Return        PVRSRV_ERROR error code
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVGetProcessMemStatsTotal(PVRSRV_DEV_CONNECTION *psConnection,
                              IMG_UINT32 *pui32TotalKernelMem,
                              IMG_UINT32 *pui32TotalGraphicsMem);

#if defined(PVRSRV_ENABLE_GPU_MEMORY_INFO)
IMG_EXPORT
PVRSRV_ERROR PVRSRVDumpRIDebug(PVRSRV_DEV_CONNECTION *psConnection);
#endif

#if defined (__cplusplus)
}
#endif
#endif /* SERVICES_H */

/******************************************************************************
 End of file (services.h)
******************************************************************************/

