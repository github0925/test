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
@Function       RGXConfigHWPerfCounters
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
@Input          ui32CtrlWord     See RGX_HWPERF_CTRL_GEOM_FULLRANGE,
                                  RGX_HWPERF_CTRL_COMP_FULLRANGE.
@Input          ui32NumBlocks    Number of elements in the array
@Input          asBlockConfigs   Address of the array of configuration blocks
@Return                          PVRSRV_OK on success. Otherwise, a
                                  PVRSRV_* error code
*/ /***************************************************************************/
IMG_EXPORT
PVRSRV_ERROR IMG_CALLCONV RGXConfigHWPerfCounters(
		PVRSRV_DEV_CONNECTION*     psDevConnection,
		IMG_UINT32                 ui32CtrlWord,
		IMG_UINT32                 ui32NumBlocks,
		RGX_HWPERF_CONFIG_CNTBLK*  asBlockConfigs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RGXAPI_HWPERF_H */
