/*************************************************************************/ /*!
@File
@Title          Services Transport Layer stream names
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Transport layer common types and definitions included into
                both user mode and kernel mode source.
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVRSRV_TLSTREAMS_H
#define PVRSRV_TLSTREAMS_H

#define PVRSRV_TL_CTLR_STREAM "tlctrl"

#define PVRSRV_TL_HWPERF_RGX_FW_STREAM      "hwperf_fw_"
#define PVRSRV_TL_HWPERF_HOST_SERVER_STREAM "hwperf_host_"

/* Host HWPerf client stream names are of the form 'hwperf_client_<pid>' */
#define PVRSRV_TL_HWPERF_HOST_CLIENT_STREAM         "hwperf_client_"
#define PVRSRV_TL_HWPERF_HOST_CLIENT_STREAM_FMTSPEC "hwperf_client_%u_%u"

#endif /* PVRSRV_TLSTREAMS_H */

/******************************************************************************
 End of file (pvrsrv_tlstreams.h)
******************************************************************************/
