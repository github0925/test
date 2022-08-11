/*************************************************************************/ /*!
@File
@Title         PVR synchronization interface
@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description   API for import/export of PVRSRV/native syncs for client side
               code
@License       Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVRSRV_SYNC_UM_EXPORT_H
#define PVRSRV_SYNC_UM_EXPORT_H

#include "pvrsrv_sync_um.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*************************************************************************/ /*!
 An opaque type containing the export handle created for the given export
 method.
*/ /***************************************************************************/
typedef IMG_INT32 PVRSRV_FENCE_EXPORT_TYPE;


/*************************************************************************/ /*!
 An enum describing the possible methods to export a fence to pass it across
 to another process.

 Each export method has a additional level of security associated with it in
 the following order, this is used as the priority of each export method.

     Native > Secure > Insecure
*/ /***************************************************************************/
typedef enum _PVRSRV_FENCE_EXPORT_METHOD_
{
	/*********************************************************************/ /*!
	  Export the fence as a file descriptor to a Linux Sync File. It can be
	  used with any Linux API accepting a valid sync file as input.

	  The file descriptor will have an independent lifetime on each process
	  that it is passed to and as such will have to be imported (or destroyed)
	  on each process.
	 */ /**********************************************************************/
	PVRSRV_FENCE_EXPORT_METHOD_NATIVE = 1 << 0,

	/*********************************************************************/ /*!
	  Export the fence as a file descriptor such that it can be passed across
	  a UNIX socket using an SCM_RIGHTS control message to the desired
	  destination process.

	  The file descriptor will have an independent lifetime on each process
	  that it is passed to and as such will have to be imported (or destroyed)
	  on each process.

	  This file descriptor will have limited usage outside of the services
	  API.
	 */ /**********************************************************************/
	PVRSRV_FENCE_EXPORT_METHOD_SECURE = 1 << 1,

	/*********************************************************************/ /*!
	  Export the fence using the services global handle base, so the handle
	  can be passed to another process using standard interprocess
	  communication.

	  The handle returned by the export function will have a global lifetime
	  such that if the handle is imported (or destroyed) within one process
	  it will not be valid for calls in any other process.

	  This handle will have no usage to any libraries other than this services
	  API.

	  NB. There is no security with insecure exports and as such any process
	      can access the underlying data if they find out handle created for
	      the export.
	 */ /**********************************************************************/
	PVRSRV_FENCE_EXPORT_METHOD_INSECURE = 1 << 2,

	/*********************************************************************/ /*!
	  Export using the most secure export method provided by the sync
	  implementation.
	 */ /**********************************************************************/
	PVRSRV_FENCE_EXPORT_METHOD_ANY = PVRSRV_FENCE_EXPORT_METHOD_NATIVE | PVRSRV_FENCE_EXPORT_METHOD_SECURE | PVRSRV_FENCE_EXPORT_METHOD_INSECURE,
} PVRSRV_FENCE_EXPORT_METHOD;


/*************************************************************************/ /*!
@Function       PVRSRVFenceExport

@Description    Create an export handle for a given fence using a given
                exporting method.

                The handle can then be passed across to another process, via
                the mechanism described by the
                \link #_PVRSRV_FENCE_EXPORT_METHOD_ export method
                documentation\endlink, to be imported using the
                PVRSRVFenceImport call.

                For Native this call will consume the fence passed in as
                hFence.

                For Secure and Insecure exports this call will \b NOT consume
                the fence passed in as hFence and as such this fence will need
                to be cleaned up by the application once the application has
                finished with it.

                More information about the method to pass each export method
                across to another process and the usage outside of the services
                API is listed in the \link #_PVRSRV_FENCE_EXPORT_METHOD_
                export method documentation.\endlink

@Input          psDevConnection     The services connection

@Input          hFence              The fence to export to another process.

@Input          eExportMethodHint   A hint to the implementation on which
                                    export method to use for this export. The
                                    most secure export method provided
                                    supported by the implementation will be
                                    used.

                                    If no export methods listed in the hint
                                    are supported by the implementation
                                    PVRSRV_ERROR_NOT_SUPPORTED will be
                                    returned.

@Output         phExport            The export token for this fence, to be
                                    passed to the required process and used
                                    with a PVRSRVFenceImport call.

@Output         peExportMethod      The actual export method used to perform
                                    this export, this method must have been
                                    specified inside the hint provided by
                                    the user.

                                    If this pointer is NULL the export method
                                    will not be returned, this should only be
                                    used if only one export method was
                                    specified in the export method hint.

@Return         PVRSRV_OK                   if the export was completed
                                            successfully

                PVRSRV_ERROR_INVALID_PARAMS if phExport is NULL, hFence is not
                                            a valid handle to a fence or
                                            eExportMethodHint is 0

                PVRSRV_ERROR_NOT_SUPPORTED  if none of the export methods
                                            provided in the export method hint
                                            are supported
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVFenceExportI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                                           PVRSRV_FENCE hFence,
                                           PVRSRV_FENCE_EXPORT_METHOD eExportMethodHint,
                                           PVRSRV_FENCE_EXPORT_TYPE *phExport,
                                           PVRSRV_FENCE_EXPORT_METHOD *peExportMethod
                                           PVR_DBG_FILELINE_PARAM);
#define PVRSRVFenceExport(psDevConnection, hFence, eExportMethodHint, phExport, peExportMethod) \
	PVRSRVFenceExportI( (psDevConnection), (hFence), (eExportMethodHint), (phExport), (peExportMethod) \
                           PVR_DBG_FILELINE)

/*************************************************************************/ /*!
@Function       PVRSRVFenceExportDestroy

@Description    Close the export handle created from PVRSRVFenceExport,
                destroying this handle will have different effects depending
                on which export method was used.

                For a Native or Secure export this will destroy the handle for
                the current process only, therefore any process that the
                provided handle has been passed to can still import that
                handle.

                For an Insecure export, this will destroy the handle globally
                and no other process can use the handle in future import calls,
                <b>this can only be called on the process which exported the
                fence originally</b>

                The export method specified must be the same as the export
                method returned from the export call.

@Input          psDevConnection     The services connection

@Input          hExport             The export token created from a
                                    PVRSRVFenceSecureExport call in this
                                    process

@Input          eExportMethod       The export method used to create this
                                    export handle

@Return         PVRSRV_OK                   if the export was completed
                                            successfully

                PVRSRV_ERROR_INVALID_PARAMS if hExport is invalid for the
                                            specified export method

                PVRSRV_ERROR_NOT_SUPPORTED  if the method used to export this
                                            fence is not supported by this
                                            system
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVFenceExportDestroyI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                                                  PVRSRV_FENCE_EXPORT_TYPE hExport,
                                                  PVRSRV_FENCE_EXPORT_METHOD eExportMethod
                                                  PVR_DBG_FILELINE_PARAM);
#define PVRSRVFenceExportDestroy(psDevConnection, hExport, eExportMethod) \
	PVRSRVFenceExportDestroyI( (psDevConnection), (hExport), (eExportMethod) \
                           PVR_DBG_FILELINE)


/*************************************************************************/ /*!
@Function       PVRSRVFenceImport

@Description    Import a fence exported by a different process by a call to
                PVRSRVFenceExport.

                For a Native or Secure export this will destroy the handle for
                the current process only, therefore any process that the
                provided handle has been passed to can still import that
                handle.

                For an Insecure export, this call will not destroy the handle
                leaving the exporting process to cleanup the handle once
                all processes have finished importing it.

                The export method specified must be the same as the export
                method returned from the export call.

@Input          psDevConnection     The services connection

@Input          hImport             The import token created from a
                                    PVRSRVFenceSecureExport call in another
                                    process

@Input          eExportMethod       The export method used to export this fence

@Output         phFence             The resultant OS agnostic fence for use

@Return         PVRSRV_OK                   if the import was completed
                                            successfully

                PVRSRV_ERROR_INVALID_PARAMS if the output pointer provided is
                                            NULL or if hImport is invalid for
                                            the specified export method

                PVRSRV_ERROR_NOT_SUPPORTED  if the method used to export this
                                            fence is not supported by this
                                            system
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR PVRSRVFenceImportI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                                           PVRSRV_FENCE_EXPORT_TYPE hImport,
                                           PVRSRV_FENCE_EXPORT_METHOD eExportMethod,
                                           PVRSRV_FENCE *phFence
                                           PVR_DBG_FILELINE_PARAM);
#define PVRSRVFenceImport(psDevConnection, hImport, eExportMethod, phFence) \
	PVRSRVFenceImportI( (psDevConnection), (hImport), (eExportMethod), (phFence) \
                           PVR_DBG_FILELINE)

#if defined(PVR_SYNC_UM_API_DEBUG)
#include "pvrsrv_sync_um_export_debug.h"
#endif


#if defined(__cplusplus)
}
#endif

#endif /* PVRSRV_SYNC_UM_EXPORT_H */
