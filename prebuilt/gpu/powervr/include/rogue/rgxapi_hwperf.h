/*************************************************************************/ /*!
@File
@Title          PVRSRV HWPerf Client API
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    PVRSRV HWPerf Client functions
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef RGXAPI_HWPERF_H
#define RGXAPI_HWPERF_H

#include "rgxapi_hwperf_common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************
 * RGX HW Performance Profiling API(s)
 *****************************************************************************/

/**************************************************************************/ /*!
@Function       RGXConfigureAndEnableHWPerfCounters
@Description    Enable and configure the performance counter block for
                 one or more device layout modules. This call enables one
                 or more counter blocks, clears the counters and programmes
                 the counters in one operation. If a block is not present in
                 the asBlockConfigs parameter then it's current configuration
                 will be preserved.
                The asBlockConfigs parameter allows the caller to programme
                 a counter to count or not to count. When not counting (i.e.
                 all the configuration is 0 then the counter will not appear
                 in the HW event. See RGX_HWPERF_CONFIG_CNTBLK.
@Input          psDevConnection  Device data and connection context
@Input          ui32NumBlocks    Number of elements in the array
@Input          asBlockConfigs   Address of the array of configuration blocks
@Return                          PVRSRV_OK on success. Otherwise, a
                                  PVRSRV_* error code
*/ /***************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXConfigureAndEnableHWPerfCounters(
		PVRSRV_DEV_CONNECTION*     psDevConnection,
		IMG_UINT32                 ui32NumBlocks,
		RGX_HWPERF_CONFIG_CNTBLK*  asBlockConfigs);

/**************************************************************************/ /*!
@Function       RGXConfigCustomCounters
@Description    Configure which custom non-mux counters are read every time
                a HW event packet is generated.

@Input          psDevConnection          Device Data and connection context
@Input          ui16CustomBlockID        ID of the custom block to
                                         configure
@Input          ui32NumCustomCounters    Number of counters to include in the
                                         specified block, maximum number of 8.
@Input          aui32CustomCounterIDs    Array containing the requested
                                         counters IDs

@Return                                  PVRSRV_OK on success. Otherwise, a
                                         PVRSRV_* error code
*/ /***************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXConfigCustomCounters(
		PVRSRV_DEV_CONNECTION*  psDevConnection,
		IMG_UINT16              ui16CustomBlockID,
		IMG_UINT16              ui16NumCustomCounters,
		IMG_UINT32*             aui32CustomCounterIDs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RGXAPI_HWPERF_H */
