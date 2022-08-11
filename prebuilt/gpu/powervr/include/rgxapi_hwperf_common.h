/*************************************************************************/ /*!
@File
@Title          PVRSRV HWPerf Client API
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    PVRSRV HWPerf Client functions
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef RGXAPI_HWPERF_COMMON_H
#define RGXAPI_HWPERF_COMMON_H

#include "img_types.h"
#include "img_defs.h"
#include "pvrsrv_error.h"
#include "services.h"

#include "rgx_hwperf.h"
#include "rgx_hwperf_client.h"

#include "devicemem_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************
 * RGX HW Performance Profiling API(s)
 *****************************************************************************/

/**************************************************************************/ /*!
 @Function     RGXGetHWPerfBvncFeatureFlags
 @Description  Function returns the BVNC Feature flags of the connected device
               relevant to clients who process HWPerf event data.
 @Input        psConnection           The connection to the device
 @Output       psBVNC                 Pointer (must not be null) which will be
                                      filled to return the feature information.
 @Return       PVRSRV_OK on success or PVRSRV_ERROR error code otherwise
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV
RGXGetHWPerfBvncFeatureFlags(PVRSRV_DEV_CONNECTION *psConnection,
		                     RGX_HWPERF_BVNC       *psBVNC);

/**************************************************************************/ /*!
@Function       RGXCtrlHWPerf
@Description    Enable or disable the generation of RGX HWPerf event packets.
                 Depending on the stream id behaviour for different streams may
                 be slightly different.
                 Enabling events for firmware buffer will reset the ordinal
                 and clear the event buffer used by the firmware. Events already
                 in the host driver buffer will be unaffected. Disabling events
                 simply stops event generation; events may continue to be be drained
                 from the buffers by the client. When toggling it is recommended
                 to start from a defined and known state, e.g. all events were
                 disabled with a 'set' call before.
                 Enabling events for host buffer is simpler and just allows
                 events to be written to the buffer. Disabling stop event emission
                 to the buffer.
                 Enabling events for client buffers works like for host buffers.
                 The difference is with the passed parameters. For client streams
                 additional internal value is included in the ui64Mask hence
                 this function should not be called for client streams directly.
                 Please use PVRSRVSetClientEventFilter() instead.
@Input          psDevConnection  Device data and connection context
@Input          eStreamID        ID of the stream to which the configuration
                                 applies
@Input          bToggle          Choose to 'toggle' bits in mask or 'set' whole
                                 mask and overwrite the old state.
                                 I.e. with toggle set we XOR in the mask bits,
                                 with toggle not set we assign the new mask.
@Input          ui64Mask         Mask of events to control. Each bit in the mask
                                 corresponds to one particular event type
                                 e.g. <tt>1U<<RGX_HWPERF_HW_TAKICK</tt>.
                                 Each bit set will lead to the event being
                                 enabled or disabled. Predefined masks like
                                 RGX_HWPERF_EVENT_MASK_ALL can be found in
                                 rgx_hwperf.h.
                                 Firmware stream mask occupies whole 64 bits of
                                 the passed value.
                                 Host stream mask takes 32 least significant
                                 bits and most significant bits are ignored.
                                 Client streams mask uses lower 32 bits to pass
                                 the mask value and upper 32 bits to pass
                                 internal Information Page index.
@Return                          PVRSRV_OK on success. Otherwise, a
                                 PVRSRV_* error code
*/ /***************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXCtrlHWPerf(
		PVRSRV_DEV_CONNECTION* psDevConnection,
		RGX_HWPERF_STREAM_ID eStreamID,
		IMG_BOOL         bToggle,
		IMG_UINT64       ui64Mask);

/**************************************************************************/ /*!
@Function       RGXDisableHWPerfCounters
@Description    Disable the performance counter block for one or more
                 device layout modules. This will disable the counter block
                 potentially saving power.
                 rogue: the configuration of the counter block will be retained.
				 volcanic: it will also reset the counter configuration to zero.
@Input          psDevConnection Device data and connection context
@Input          ui32NumBlocks   Number of elements in the array
@Input          aui16BlockIDs   An array of bytes with values taken from
                                 the RGX_HWPERF_CNTBLK_ID enumeration.
@Return                         PVRSRV_OK on success. Otherwise, a
                                 PVRSRV_* error code
*/ /***************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXDisableHWPerfCounters(
		PVRSRV_DEV_CONNECTION*  psDevConnection,
		IMG_UINT32        ui32NumBlocks,
		IMG_UINT16*       aui16BlockIDs);

/**************************************************************************/ /*!
@Function       RGXEnableHWPerfCounters
@Description    Enable the performance counter block for one or more
                 device layout modules. This will restore the configuration that
                 was previously set when the block was disabled.
@Input          psDevConnection Device data and connection context
@Input          ui32NumBlocks   Number of elements in the array
@Input          aui16BlockIDs   An array of bytes with values taken from
                                 the RGX_HWPERF_CNTBLK_ID enumeration.
@Return                         PVRSRV_OK on success. Otherwise, a
                                 PVRSRV_* error code
*/ /***************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXEnableHWPerfCounters(
		PVRSRV_DEV_CONNECTION*  psDevConnection,
		IMG_UINT32        ui32NumBlocks,
		IMG_UINT16*       aui16BlockIDs);

/*!
 * @Function PVRSRVWriteClientEvent
 * @Description Posts event to the client stream.
 *
 * @Input psDevConnection pointer to the opened connection to the driver
 * @Input eApi API that the event belongs to
 * @Input eType event's id
 * @Input pvPacketData pointer to the packet's data
 * @Input uiPacketDataSize size of the psPacket data
 * @Return PVRSRV_OK on success and other error code on failure
 */
IMG_EXPORT
PVRSRV_ERROR PVRSRVWriteClientEvent(PVRSRV_DEV_CONNECTION *psDevConnection,
                                 RGX_HWPERF_CLIENT_EVENT_TYPE eType,
                                 RGX_HWPERF_V2_PACKET_DATA_CLIENT *psPacketData,
                                 size_t uiPacketDataSize);

/*!
 * @Function PVRSRVSetClientEventFilter
 * @Description Sets event mask for the given API.
 *
 * @Input eApi API id that the filter will be set for
 * @Input ui32Filter events mask
 * @Return PVRSRV_OK on success and other error code on failure
 */
IMG_EXPORT
PVRSRV_ERROR PVRSRVSetClientEventFilter(PVRSRV_DEV_CONNECTION *psDevConnection,
                                        RGX_HWPERF_CLIENT_API eApi,
                                        IMG_UINT32 ui32Filter);

/*!
 * @Function PVRSRVGetClientEventFilter
 * @Description Returns current event mask for the given API.
 *
 * @Input psDevConnection pointer to the opened connection to the driver
 * @Input eApi API for which the filter will be retrieved
 * @Return value of the event's mask
 */
IMG_EXPORT
IMG_UINT32 PVRSRVGetClientEventFilter(PVRSRV_DEV_CONNECTION *psDevConnection,
                                      RGX_HWPERF_CLIENT_API eApi);

/*!
 * @Function PVRSRVRequestHWPerfResourceCapture
 * @Description Request a device resource capture from a client specified by PID and context ID.
 *
 * @Input psDevConnection Device data and connection context.
 * @Input eResourceCaptureType The resource type to capture.
 * @Input uPID The PID of the client process to capture from.
 * @Input ui32ClientCtxID The client context ID within the process to capture from.
 * @Input ui32FrameNum The frame number to (attempt) to capture.
 * @Input fScale the desired scaling factor to apply to the resource (if applicable) i.e. 1/2, 1/4, 1/8 etc.
 * @Return PVRSRV_OK on success, otherwise a PVRSRV_* error code.
 */
IMG_EXPORT
PVRSRV_ERROR PVRSRVRequestHWPerfResourceCapture(PVRSRV_DEV_CONNECTION *psDevConnection,
                                                RGX_HWPERF_RESOURCE_CAPTURE_TYPE eResourceCaptureType,
                                                IMG_PID uPID,
                                                IMG_UINT32 ui32ClientCtxID,
                                                IMG_UINT32 ui32FrameNum,
                                                IMG_FLOAT fScale);

/*!
 * @Function PVRSRVGetHWPerfResourceCaptureResult
 * @Description Retrieve the captured resource data, this function may be called multiple
 *              times in order to receive all data transmitted by the client process.
 *
 * @Input psDevConnection Device data and connection context.
 * @Input ui32TimeoutMS Time in milli-seconds to wait for the request to be completed.
 * @Input ui32HeaderSizeBytes Optionally create extra space in ppbData buffer to allow the caller to
 *                            add a custom header to the data buffer. The resource data will be offset in
 *                            ppbData by ui32HeaderSizeBytes.
 * @ Input psResourceCaptureResult Structure containing result details, fields are populated internally by the function.
 * @Return RGX_HWPERF_RESOURCE_CAPTURE_RESULT_STATUS code.
 * @Return PVRSRV_OK on success, otherwise a PVRSRV_* error code.
 */
IMG_EXPORT
RGX_HWPERF_RESOURCE_CAPTURE_RESULT_STATUS PVRSRVGetHWPerfResourceCaptureResult(PVRSRV_DEV_CONNECTION *psDevConnection,
                                                                               IMG_UINT32 ui32TimeoutMS,
                                                                               IMG_UINT32 ui32HeaderSizeBytes,
                                                                               RGX_RESOURCE_CAPTURE_RESULT *psResourceCaptureResult);

/*!
 * @Function PVRSRVSendHWPerfResourceRequestResult
 * @Description Complete a resource request made by a HWPerf client, by making the device buffer available to the requesting process.
 *
 * @Input psDevConnection Device data and connection context
 * @Input hMemDesc  Handle to the device memory descriptor where the requested resource data can be retrieved from.
 *                  Note that the device memory should be allocated as exportable and CPU Readable.
 *                  The caller should also ensure the meta-data defined in RGX_RESOURCE_CAPTURE_INFO
 *                  is packed at the beginning of the device memory buffer.
 * @Input eStatus Pass RGX_HWPERF_RESOURCE_CAPTURE_RESULT_COMPLETE_SUCCESS or RGX_HWPERF_RESOURCE_CAPTURE_RESULT_COMPLETE_FAILURE
 *                to complete a request.
 * @Return PVRSRV_OK on success. Otherwise, a PVRSRV_* error code
 */
IMG_EXPORT
PVRSRV_ERROR PVRSRVSendHWPerfResourceRequestResult(PVRSRV_DEV_CONNECTION *psDevConnection,
                                                   IMG_UINT32 ui32CtxID,
                                                   DEVMEM_MEMDESC *hMemDesc,
                                                   RGX_HWPERF_RESOURCE_CAPTURE_RESULT_STATUS eStatus);

/*!
 * @Function PVRSRVIsAnyHWPerfResourceCaptureRequest
 * @Description Queries the info page to check whether there is a request for a
 *              resource capture on this context.
 *
 * @Input psDevConnection pointer to the opened connection to the driver
 * @Input ui32CtxID Callers context ID to query against
 * @Input ui32FrameNum Callers current frame number
 * @Output fScale the desired scaling factor to apply to the resource (if applicable) i.e. 1/2, 1/4, 1/8 etc.
 * @Return The resource capture type, RGX_HWPERF_RESOURCE_CAPTURE_TYPE_NONE
 *          if there is no capture request for this context
 */
IMG_EXPORT
RGX_HWPERF_RESOURCE_CAPTURE_TYPE PVRSRVIsAnyHWPerfResourceCaptureRequest(PVRSRV_DEV_CONNECTION *psDevConnection,
                                                                         IMG_UINT32 ui32CtxID,
                                                                         IMG_UINT32 ui32FrameNum,
                                                                         IMG_FLOAT *fScale);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RGXAPI_HWPERF_COMMON_H */
