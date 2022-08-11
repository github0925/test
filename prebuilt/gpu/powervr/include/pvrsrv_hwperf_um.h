/*************************************************************************/ /*!
@File           pvrsrv_hwperf_um.h
@Title          PVR Services HWPerf client side API
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Services client HWPerf API used for emitting events in HWPerf
                client event buffer
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef PVRSRV_HWPERF_UM_H
#define PVRSRV_HWPERF_UM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <string.h>
#include "rgx_hwperf_client.h"
#include "rgxapi_hwperf.h"

/* Flag bits which form up ui32ClientFlags of RGX_HWPERF_SERVICES_CLIENT_ENQ_DATA */
#define RGX_HWPERF_SERVICES_ENQ_FLAG_SERVER_KICK 0x1

#define RGX_HWPERF_SERVICES_KICK_START(_conn, _type, _jobref, _RTDataSet)                \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
                RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,                                   \
                RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_KICK_START)) {                     \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            uData.sSrvKick.eType = RGX_HWPERF_SERVICES_KICK_TYPE_##_type;                \
            uData.sSrvKick.ui32ExtJobRef = _jobref;                                      \
            uData.sSrvKick.ui64RTDataSet = (IMG_UINT64)(uintptr_t)_RTDataSet;            \
            PVRSRVWriteClientEvent((_conn),                                              \
                             RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_KICK_START,           \
                             &uData, sizeof(uData.sSrvKick));                            \
        }                                                                                \
    } while(0)

#define RGX_HWPERF_SERVICES_KICK_END(_conn, _type, _jobref, _RTDataSet)                  \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
                RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,                                   \
                RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_KICK_END)) {                       \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            uData.sSrvKick.eType = RGX_HWPERF_SERVICES_KICK_TYPE_##_type;                \
            uData.sSrvKick.ui32ExtJobRef = _jobref;                                      \
            uData.sSrvKick.ui64RTDataSet = (IMG_UINT64)(uintptr_t)_RTDataSet;            \
            PVRSRVWriteClientEvent((_conn),                                              \
                               RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_KICK_END,           \
                               &uData, sizeof(uData.sSrvKick));                          \
        }                                                                                \
    } while(0)

#define RGX_HWPERF_SERVICES_ENQ(_conn, _type, _pid, _JobRef, _ctx, _woff, _clientFlags)  \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
                RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,                                   \
                RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_ENQ)) {                            \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            uData.sSrvEnq.i32EnqType = RGX_HWPERF_KICK_TYPE_##_type;                     \
            uData.sSrvEnq.ui32Pid = _pid,                                                \
            uData.sSrvEnq.ui32ExtJobRef = _JobRef,                                       \
            uData.sSrvEnq.ui64WorkCtx = _ctx,                                            \
            uData.sSrvEnq.ui32WorkTarget = _woff,                                        \
            uData.sSrvEnq.ui32ClientFlags = _clientFlags,                                \
            PVRSRVWriteClientEvent((_conn),                                              \
                               RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_ENQ,                \
                               &uData, sizeof(uData.sSrvEnq));                           \
        }                                                                                \
    } while(0)

// Helper sync macros to keep following code cleaner
#define SYNC_RES_ENUM_TYPE(_res) RGX_HWPERF_SERVICES_CLIENT_SYNC_RESOURCE_TYPE_##_res
#define SYNC_WAIT_ENUM_TYPE(_type) RGX_HWPERF_SERVICES_CLIENT_SYNC_FENCE_WAIT_TYPE_##_type
#define SYNC_WAIT_RESULT_TYPE(_res) RGX_HWPERF_SERVICES_CLIENT_SYNC_FENCE_WAIT_RESULT_##_res

#define RGX_HWPERF_SERVICES_SYNC_ALLOC_TIMELINE(_conn, _pid, _tlType, _tl, _name)        \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
                                      RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,             \
                                      RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_ALLOC)) \
        {                                                                                \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            RGX_HWPERF_SERVICES_CLIENT_SYNC_ALLOC_DETAIL *puAllocDetail =                \
                                                      &uData.sSrvSyncAlloc.uAllocDetail; \
            uData.sSrvSyncAlloc.eType = SYNC_RES_ENUM_TYPE(TIMELINE);                    \
            strncpy(puAllocDetail->sTimeline.acTimelineName, (_name),                    \
                    sizeof(puAllocDetail->sTimeline.acTimelineName) - 1);                \
            puAllocDetail->sTimeline.acTimelineName[                                     \
                            sizeof(puAllocDetail->sTimeline.acTimelineName) - 1] = '\0'; \
            puAllocDetail->sTimeline.uiPID = (_pid);                                     \
            puAllocDetail->sTimeline.hTimeline = (_tl);                                  \
            puAllocDetail->sTimeline.eType = RGX_HWPERF_SERVICES_CLIENT_##_tlType;       \
            PVRSRVWriteClientEvent((_conn),                                              \
                                   RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_ALLOC,     \
                                   &uData, sizeof(uData.sSrvSyncAlloc));                 \
        }                                                                                \
    } while(0)

#define RGX_HWPERF_SERVICES_SYNC_ALLOC_SW_FENCE(_conn, _pid, _fence, _tl, _pt, _name)    \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
               RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,                                    \
               RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_ALLOC))                        \
        {                                                                                \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            RGX_HWPERF_SERVICES_CLIENT_SYNC_ALLOC_DETAIL *puAllocDetail =                \
                                                      &uData.sSrvSyncAlloc.uAllocDetail; \
            uData.sSrvSyncAlloc.eType = SYNC_RES_ENUM_TYPE(FENCE);                       \
            strncpy(puAllocDetail->sSWFence.acSWFenceName, (_name),                      \
                    sizeof(puAllocDetail->sSWFence.acSWFenceName)-1);                    \
            puAllocDetail->sSWFence.acSWFenceName[                                       \
                              sizeof(puAllocDetail->sSWFence.acSWFenceName) - 1] = '\0'; \
            puAllocDetail->sSWFence.uiPID = (_pid);                                      \
            puAllocDetail->sSWFence.hFence = (_fence);                                   \
            puAllocDetail->sSWFence.hSWTimeline = (_tl);                                 \
            puAllocDetail->sSWFence.ui64SyncPtIndex = (_pt);                             \
            PVRSRVWriteClientEvent((_conn),                                              \
                                   RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_ALLOC,     \
                                   &uData, sizeof(uData.sSrvSyncAlloc));                 \
        }                                                                                \
    } while(0)

#define RGX_HWPERF_SERVICES_SYNC_SW_TL_ADVANCE(_conn, _pid, _tl, _pt)                    \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
               RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,                                    \
               RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_SW_TL_ADVANCE))                \
        {                                                                                \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            uData.sSrvSyncSWTLAdv.uiPID = (_pid);                                        \
            uData.sSrvSyncSWTLAdv.hSWTimeline = (_tl);                                   \
            uData.sSrvSyncSWTLAdv.ui64SyncPtIndex = (_pt);                               \
            PVRSRVWriteClientEvent((_conn),                                              \
                               RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_SW_TL_ADVANCE, \
                               &uData, sizeof(uData.sSrvSyncSWTLAdv));                   \
        }                                                                                \
    } while (0)

#define RGX_HWPERF_SERVICES_SYNC_ALLOC_FENCE_DUP(_conn, _pid, _in, _out)                 \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
                                      RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,             \
                                      RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_ALLOC)) \
        {                                                                                \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            RGX_HWPERF_SERVICES_CLIENT_SYNC_ALLOC_DETAIL *puAllocDetail =                \
                                                      &uData.sSrvSyncAlloc.uAllocDetail; \
            uData.sSrvSyncAlloc.eType = SYNC_RES_ENUM_TYPE(FENCE_DUP);                   \
            puAllocDetail->sFenceDup.uiPID = (_pid);                                     \
            puAllocDetail->sFenceDup.hInFence = (_in);                                   \
            puAllocDetail->sFenceDup.hOutFence = (_out);                                 \
            PVRSRVWriteClientEvent((_conn),                                              \
                                   RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_ALLOC,     \
                                   &uData, sizeof(uData.sSrvSyncAlloc));                 \
        }                                                                                \
    } while(0)

#define RGX_HWPERF_SERVICES_SYNC_ALLOC_FENCE_MERGE(_conn, _pid, _in1, _in2, _out)        \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
                                      RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,             \
                                      RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_ALLOC)) \
        {                                                                                \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            RGX_HWPERF_SERVICES_CLIENT_SYNC_ALLOC_DETAIL *puAllocDetail =                \
                                                      &uData.sSrvSyncAlloc.uAllocDetail; \
            uData.sSrvSyncAlloc.eType = SYNC_RES_ENUM_TYPE(FENCE_MERGE);                 \
            puAllocDetail->sFenceMerge.uiPID = (_pid);                                   \
            puAllocDetail->sFenceMerge.hInFence1 = (_in1);                               \
            puAllocDetail->sFenceMerge.hInFence2 = (_in2);                               \
            puAllocDetail->sFenceMerge.hOutFence = (_out);                               \
            PVRSRVWriteClientEvent((_conn),                                              \
                                   RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_ALLOC,     \
                                   &uData, sizeof(uData.sSrvSyncAlloc));                 \
        }                                                                                \
    } while(0)

#define RGX_HWPERF_SERVICES_SYNC_FREE(_conn, _pid, _resType, _res)                       \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
                                      RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,             \
                                      RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_FREE))  \
        {                                                                                \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            uData.sSrvSyncFree.eType = SYNC_RES_ENUM_TYPE(_resType);                     \
            switch(uData.sSrvSyncFree.eType)                                             \
            {                                                                            \
                case RGX_HWPERF_SERVICES_CLIENT_SYNC_RESOURCE_TYPE_TIMELINE:             \
                    uData.sSrvSyncFree.uFreeDetail.sTimeline.hTimeline = (_res);         \
                    uData.sSrvSyncFree.uFreeDetail.sTimeline.uiPID = (_pid);             \
                    break;                                                               \
                case RGX_HWPERF_SERVICES_CLIENT_SYNC_RESOURCE_TYPE_FENCE:                \
                    uData.sSrvSyncFree.uFreeDetail.sFence.hFence = (_res);               \
                    uData.sSrvSyncFree.uFreeDetail.sFence.uiPID = (_pid);                \
                    break;                                                               \
                default:                                                                 \
                    PVR_DPF((PVR_DBG_ERROR, "Unknown sync resource type (%u)",           \
                             uData.sSrvSyncFree.eType));                                 \
                    break;                                                               \
            }                                                                            \
            PVRSRVWriteClientEvent((_conn),                                              \
                                   RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_FREE,      \
                                   &uData, sizeof(uData.sSrvSyncFree));                  \
        }                                                                                \
    } while(0)

#define RGX_HWPERF_SERVICES_SYNC_FENCE_WAIT(_conn, _type, _pid, _fence, _data)           \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
                                 RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,                  \
                                 RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_FENCE_WAIT)) \
        {                                                                                \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            RGX_HWPERF_SERVICES_CLIENT_SYNC_FENCE_WAIT_DETAIL *puFenceWaitDetail =       \
                                                       &uData.sSrvSyncFenceWait.uDetail; \
            uData.sSrvSyncFenceWait.eType = SYNC_WAIT_ENUM_TYPE(_type);                  \
            uData.sSrvSyncFenceWait.hFence = (_fence);                                   \
            uData.sSrvSyncFenceWait.uiPID = (_pid);                                      \
            switch(uData.sSrvSyncFenceWait.eType)                                        \
            {                                                                            \
                case SYNC_WAIT_ENUM_TYPE(BEGIN):                                         \
                    puFenceWaitDetail->sBegin.ui32TimeoutInMs = (_data);                 \
                    break;                                                               \
                case SYNC_WAIT_ENUM_TYPE(END):                                           \
                    if ((_data) == PVRSRV_OK)                                            \
                    {                                                                    \
                        puFenceWaitDetail->sEnd.eResult = SYNC_WAIT_RESULT_TYPE(PASSED); \
                    }                                                                    \
                    else if ((_data) == PVRSRV_ERROR_TIMEOUT)                            \
                    {                                                                    \
                        puFenceWaitDetail->sEnd.eResult = SYNC_WAIT_RESULT_TYPE(TIMEOUT);\
                    }                                                                    \
                    else                                                                 \
                    {                                                                    \
                        puFenceWaitDetail->sEnd.eResult = SYNC_WAIT_RESULT_TYPE(ERROR);  \
                    }                                                                    \
                    break;                                                               \
                default:                                                                 \
                    PVR_DPF((PVR_DBG_ERROR, "Unknown sync fence-wait packet type (%u)",  \
                             uData.sSrvSyncFenceWait.eType));                            \
                    break;                                                               \
            }                                                                            \
            PVRSRVWriteClientEvent((_conn),                                              \
                                  RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SYNC_FENCE_WAIT, \
                                  &uData, sizeof(uData.sSrvSyncFenceWait));              \
        }                                                                                \
    } while(0)

#define RGX_HWPERF_SERVICES_SWTQ_START(_conn, _jobref)                                   \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
                RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,                                   \
                RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SWTQ_START)) {                     \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            uData.sSrvSWTQ.ui32ExtJobRef = _jobref;                                      \
            PVRSRVWriteClientEvent((_conn),                                              \
                             RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SWTQ_START,           \
                             &uData, sizeof(uData.sSrvSyncFree));                        \
        }                                                                                \
    } while(0)

#define RGX_HWPERF_SERVICES_SWTQ_END(_conn, _jobref)                                     \
    do {                                                                                 \
        if (PVRSRVGetClientEventFilter(_conn, RGX_HWPERF_CLIENT_API_SERVICES) &          \
                RGX_HWPERF_CLIENT_EVENT_MASK(SERVICES,                                   \
                RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SWTQ_END)) {                       \
            RGX_HWPERF_V2_PACKET_DATA_CLIENT uData;                                      \
            uData.sSrvSWTQ.ui32ExtJobRef = _jobref;                                      \
            PVRSRVWriteClientEvent((_conn),                                              \
                               RGX_HWPERF_CLIENT_EVENT_TYPE_SERVICES_SWTQ_END,           \
                               &uData, sizeof(uData.sSrvSyncFree));                      \
        }                                                                                \
    } while(0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PVRSRV_HWPERF_UM_H */
