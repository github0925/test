/*************************************************************************/ /*!
@File
@Title         PVR synchronisation interface
@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description   API for synchronisation functions for client side code
@License       Strictly Confidential.
*/ /**************************************************************************/
#ifndef PVRSRV_SYNC_UM_H
#define PVRSRV_SYNC_UM_H

#include "img_types.h"
#include "img_defs.h"
#include "pvrsrv_error.h"
#include "pvr_debug.h"
#include "pvrsrv_sync_km.h"
#include "services.h"
#include "pvrsrv_hwperf_um.h"

#if defined (__cplusplus)
extern "C" {
#endif


/*************************************************************************/ /*!
@Function       PVRSRVTimelineCreate

@Description    Allocate a new GPU bound timeline object for a serial work
                queue progressed by the GPU device. Multiple timelines allows
                the caller to synchronise work between different GPU work
                queues using fences. The timeline object is created with no
                initial sync points.

@Input          psDevConnection     The services connection

@Input          pszTimelineName     Name of the created timeline or
                                    NULL to request the implementation to
                                    generate an annotation for this timeline.
                                    If this is greater than
                                    PVRSRV_SYNC_NAME_LENGTH the name will be
                                    truncated to that length.

@Output         phTimeline          Pointer to  timeline handle updated on
                                    exit.

@Return         PVRSRV_OK                   if the timeline was successfully
                                            created
                PVRSRV_ERROR_INVALID_PARAMS if the output pointer provided is
                                            NULL
                PVRSRV_ERROR_xxx            if an error occurred
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVTimelineCreateI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                      PVRSRV_TIMELINE   *phTimeline,
                      const IMG_CHAR    *pszTimelineName
                      PVR_DBG_FILELINE_PARAM);

static inline PVRSRV_ERROR
PVRSRVTimelineCreate(PVRSRV_DEV_CONNECTION *psDevConnection,
                     PVRSRV_TIMELINE *phTimeline,
                     const IMG_CHAR *pszTimelineName)
{
	PVRSRV_ERROR eError;

	eError = PVRSRVTimelineCreateI(psDevConnection, phTimeline, pszTimelineName
								   PVR_DBG_FILELINE);
	if (eError == PVRSRV_OK)
	{
		RGX_HWPERF_SERVICES_SYNC_ALLOC_TIMELINE(psDevConnection,
												PVRSRVGetCurrentProcessID(),
												GPU_TL,
												*phTimeline,
												pszTimelineName);
	}

	return eError;
}

/*************************************************************************/ /*!

@Function       PVRSRVTimelineDestroy

@Description    Destroy this handle to a sync timeline.
                If the timeline has no outstanding checks or updates on
                it then it will be destroyed immediately.
                If there are outstanding checks or updates, the timeline
                will be flagged for destruction once all outstanding
                checks and updates are destroyed.
                A timeline marked for destruction may not have further
                checks or updates created for it.
                If PVRSRV_NO_TIMELINE is passed to this function no operation
                will be performed and PVRSRV_OK will be returned. This removes
                the need of users of the API validating timelines passed into
                this function.

@Input          psDevConnection  The services connection

@Input          hTimeline        The timeline to destroy

@Return         PVRSRV_OK                   if a valid active timeline was
                                            specified
                PVRSRV_ERROR_INVALID_PARAMS if an invalid timeline is specified
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVTimelineDestroyI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                       PVRSRV_TIMELINE hTimeline
                       PVR_DBG_FILELINE_PARAM);

static inline PVRSRV_ERROR
PVRSRVTimelineDestroy(PVRSRV_DEV_CONNECTION *psDevConnection,
                      PVRSRV_TIMELINE hTimeline)
{
	PVRSRV_ERROR eError;

	eError = PVRSRVTimelineDestroyI(psDevConnection, hTimeline
									PVR_DBG_FILELINE);
	if (hTimeline != PVRSRV_NO_TIMELINE && eError == PVRSRV_OK)
	{
		RGX_HWPERF_SERVICES_SYNC_FREE(psDevConnection,
									  PVRSRVGetCurrentProcessID(),
									  TIMELINE,
									  hTimeline);
	}

	return eError;
}

/*************************************************************************/ /*!
@Function       PVRSRVSWTimelineCreate

@Description    Allocate a new host bound Software timeline, Software timelines
                are different to timelines created with PVRSRVTimelineCreate in
                that they represent a strictly ordered sequence of events
                *progressed on the Host CPU* rather than the GPU.

                The sequence of events has to be modelled by the application
                itself:
                1. First the application creates a SW timeline (this call)
                2. After creating some workload on the CPU the application can
                create a fence for it by calling PVRSRVSWFenceCreate and pass
                in a software timeline.
                3. When the workload has finished and the application wants
                to signal potential waiters that work has finished, it can call
                PVRSRVSWTimelineAdvance which will signal the oldest fence
                on this software timeline

                Destroy with PVRSRVTimelineDestroy

@Input          psDevConnection      The services connection

@Input          pszSWTimelineName    String to be used to annotate the software
                                     timeline (for debug), if this is greater
                                     than PVRSRV_SYNC_NAME_LENGTH the name will
                                     be truncated to that length.

@Output         phSWTimeline         Handle to created software timeline

@Return         PVRSRV_OK                   if the timeline was successfully
                                            created
                PVRSRV_ERROR_INVALID_PARAMS if the output pointer provided is
                                            NULL
                PVRSRV_ERROR_xxx            if an error occurred
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVSWTimelineCreateI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                        PVRSRV_TIMELINE   *phSWTimeline,
                        const IMG_CHAR    *pszSWTimelineName
                        PVR_DBG_FILELINE_PARAM);

static inline PVRSRV_ERROR
PVRSRVSWTimelineCreate(PVRSRV_DEV_CONNECTION *psDevConnection,
                       PVRSRV_TIMELINE *phSWTimeline,
                       const IMG_CHAR *pszSWTimelineName)
{
	PVRSRV_ERROR eError;

	eError = PVRSRVSWTimelineCreateI(psDevConnection, phSWTimeline,
									 pszSWTimelineName PVR_DBG_FILELINE);
	if (eError == PVRSRV_OK)
	{
		RGX_HWPERF_SERVICES_SYNC_ALLOC_TIMELINE(psDevConnection,
												PVRSRVGetCurrentProcessID(),
												SW_TL,
												*phSWTimeline,
												pszSWTimelineName);
	}

	return eError;
}

/*************************************************************************/ /*!
@Function       PVRSRVSWTimelineAdvance

@Description    Advance a software timeline, causing the next sync check
                point on that software timeline to be signalled.
                Any fences containing that sync point will also then be
                signalled (if all other sync points contained by the
                fence are also signalled).

                Any attempt to advance a SW timeline beyond the last sync
                point which has been created for it will result in no
                action being taken by the implementation, but the error
                code PVRSRV_ERROR_SW_TIMELINE_AT_LATEST_POINT will be
                returned.

@Input          psDevConnection     The services connection

@Input          hSWTimeline         Software timeline to advance

@Output         pui64SyncPtIdx      On success, this contains the sync point
                                    (on the SW timeline) where the timeline
                                    has reached.

@Return         PVRSRV_OK        if the timeline was successfully advanced
                PVRSRV_ERROR_xxx if an error occurred
*/
/*****************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVSWTimelineAdvanceI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                         PVRSRV_TIMELINE   hSWTimeline,
                         IMG_UINT64        *pui64SyncPtIdx
                         PVR_DBG_FILELINE_PARAM);

static inline PVRSRV_ERROR
PVRSRVSWTimelineAdvance(PVRSRV_DEV_CONNECTION *psDevConnection,
                        PVRSRV_TIMELINE hSWTimeline)
{
	IMG_UINT64 ui64SyncPtIdx;
	PVRSRV_ERROR eError;

	eError = PVRSRVSWTimelineAdvanceI(psDevConnection, hSWTimeline,
									  &ui64SyncPtIdx PVR_DBG_FILELINE);
	if (eError == PVRSRV_OK)
	{
		RGX_HWPERF_SERVICES_SYNC_SW_TL_ADVANCE(psDevConnection,
											   PVRSRVGetCurrentProcessID(),
											   hSWTimeline,
											   ui64SyncPtIdx);
	}

	return eError;
}

/*************************************************************************/ /*!
@Function       PVRSRVSWFenceCreate

@Description    Create a fence containing a new sync point on a software
                timeline.
                The new sync point will be signalled by the CPU calling
                PVRSRVSWTimelineAdvance to progress the timeline to the
                next sync point. This means if 3 sync check points are
                created on a SW timeline, the SW timeline will need to be
                advanced 3 times in order to signal each in turn.
                Signalling the sync check point created by this function will
                also signal the fence returned by this function.
                Clients may wait for the fence returned by this function to
                become signalled by calling PVRSRVFenceWait.
                If a non-SW timeline is provided, the implementation will
                reject the request, returning PVRSRV_ERROR_NOT_SW_TIMELINE.

                Destroy with PVRSRVFenceDestroy

@Input          psDevConnection     The services connection

@Input          hSWTimeline         Software timeline on which to create fence

@Input          pszSWTimelineName   String to be used to annotate the software
                                    fence (for debug). A null string should
                                    not result in an error. Strings will be
                                    truncated to at most PVRSRV_SYNC_NAME_LENGTH
                                    chars.

@Output         phSWFence           Handle to created software fence

@Output         pui64SyncPtIdx      On success, this contains the sync point on
                                    the SW timeline this fence was created on.

@Return         PVRSRV_OK        if the fence was successfully created
                PVRSRV_ERROR_xxx if an error occurred
*/
/*****************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVSWFenceCreateI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                     PVRSRV_TIMELINE hSWTimeline,
                     const IMG_CHAR  *pszSWFenceName,
                     PVRSRV_FENCE    *phSWFence,
                     IMG_UINT64      *pui64SyncPtIdx
                     PVR_DBG_FILELINE_PARAM);

static inline PVRSRV_ERROR
PVRSRVSWFenceCreate(PVRSRV_DEV_CONNECTION *psDevConnection,
                    PVRSRV_TIMELINE hSWTimeline,
                    const IMG_CHAR  *pszSWFenceName,
                    PVRSRV_FENCE    *phSWFence)
{
	IMG_UINT64 ui64SyncPtIdx;
	PVRSRV_ERROR eError;

	eError = PVRSRVSWFenceCreateI(psDevConnection, hSWTimeline, pszSWFenceName,
								  phSWFence, &ui64SyncPtIdx PVR_DBG_FILELINE);
	if (eError == PVRSRV_OK)
	{
		RGX_HWPERF_SERVICES_SYNC_ALLOC_SW_FENCE(psDevConnection,
												PVRSRVGetCurrentProcessID(),
												*phSWFence,
												hSWTimeline,
												ui64SyncPtIdx,
												pszSWFenceName);
	}

	return eError;
}

/*************************************************************************/ /*!
@Function       PVRSRVFenceWait

@Description    Wait for all the sync points in the fence to be signalled.
                If the value PVRSRV_NO_FENCE is passed into this function then
                it will return immediately with PVRSRV_OK since there are no
                sync points to wait for. This removes the need of users of the
                API validating fences passed into this function.

@Input          psDevConnection     The services connection

@Input          hFence              Handle to the fence

@Input          ui32TimeoutInMs     Maximum time to wait (in milliseconds)

@Return         PVRSRV_OK                   once the fence has been passed (all
                                            component sync points have either
                                            signalled or errored)
                PVRSRV_ERROR_TIMEOUT        if the poll has exceeded the timeout
                PVRSRV_ERROR_INVALID_PARAMS if an invalid fence was specified
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVFenceWaitI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                 PVRSRV_FENCE hFence,
                 IMG_UINT32 ui32TimeoutInMs
                 PVR_DBG_FILELINE_PARAM);

static inline PVRSRV_ERROR
PVRSRVFenceWait(PVRSRV_DEV_CONNECTION *psDevConnection,
                PVRSRV_FENCE hFence,
                IMG_UINT32 ui32TimeoutInMs)
{
	PVRSRV_ERROR eError;

	if (hFence != PVRSRV_NO_FENCE)
	{
		RGX_HWPERF_SERVICES_SYNC_FENCE_WAIT(psDevConnection,
											BEGIN,
											PVRSRVGetCurrentProcessID(),
											hFence,
											ui32TimeoutInMs);
	}

	eError = PVRSRVFenceWaitI(psDevConnection, hFence, ui32TimeoutInMs
							   PVR_DBG_FILELINE);

	if (hFence != PVRSRV_NO_FENCE)
	{
		RGX_HWPERF_SERVICES_SYNC_FENCE_WAIT(psDevConnection,
											END,
											PVRSRVGetCurrentProcessID(),
											hFence,
											eError);
	}

	return eError;
}

/*************************************************************************/ /*!
@Function       PVRSRVFenceDup

@Description    Create a duplicate of the specified fence.
                The original fence will remain unchanged.
                The new fence will be an exact copy of the original and
                will reference the same timeline sync points as the
                source fence at the time of its creation.
                NB. If the source fence is subsequently merged or deleted
                    it will then differ from the dup'ed copy (which will
                    be unaffected).
                If PVRSRV_NO_FENCE is passed in as the source fence
                PVRSRV_NO_FENCE will be returned as the duplicated fence and
                PVRSRV_OK will be returned. This removes the need of users of
                the API validating fences passed into this function.

@Input          psDevConnection     The services connection

@Input          hSourceFence        Handle of the fence to be duplicated

@Output         phOutputFence       Handle of newly created duplicate fence

@Return         PVRSRV_OK           if the duplicate fence was successfully
                                    created
                PVRSRV_ERROR_INVALID_PARAMS if the output pointer provided is
                                    NULL or hSourceFence is an invalid fence
                PVRSRV_ERROR_OUT_OF_MEMORY if there was insufficient memory to
                                    create the new fence
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVFenceDupI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                PVRSRV_FENCE hSourceFence,
                PVRSRV_FENCE *phOutputFence
                PVR_DBG_FILELINE_PARAM);

static inline PVRSRV_ERROR
PVRSRVFenceDup(PVRSRV_DEV_CONNECTION *psDevConnection,
               PVRSRV_FENCE hSourceFence,
               PVRSRV_FENCE *phOutputFence)
{
	PVRSRV_ERROR eError;

	eError = PVRSRVFenceDupI(psDevConnection, hSourceFence, phOutputFence
							 PVR_DBG_FILELINE);
	if (hSourceFence != PVRSRV_NO_FENCE && eError == PVRSRV_OK)
	{
		RGX_HWPERF_SERVICES_SYNC_ALLOC_FENCE_DUP(psDevConnection,
												 PVRSRVGetCurrentProcessID(),
												 hSourceFence,
												 *phOutputFence);
	}

	return eError;
}

/*************************************************************************/ /*!
@Function       PVRSRVFenceMerge

@Description    Merges two fences to create a new third fence.
                The original fences will remain unchanged.
                The new fence will be merge of two original fences and
                will reference the same timeline sync points as the
                two source fences with the exception that where each
                source fence contains a sync point for the same timeline
                the output fence will only contain the later of the two
                sync points.
                Any OSNativeSyncs attached to the original fences will also
                be attached to the resultant merged fence.
                If only one of the two source fences is valid, the function
                shall simply return a duplicate of the valid fence with no
                error indicated.
                NB. If the source fences are subsequently merged or deleted
                    they will then differ from the merged copy (which will
                    be unaffected).
                If either of the two source fences provided to this function
                are PVRSRV_NO_FENCE then the returned fence will be a
                duplicated version of the specified fence. If both fences
                provided are PVRSRV_NO_FENCE then PVRSRV_NO_FENCE is returned
                as the merged fence and PVRSRV_OK will be returned. This
                removes the need of users of the API validating fences passed
                into this function.

@Input          psDevConnection     The services connection

@Input          hSourceFence1       Handle of the 1st fence to be merged

@Input          hSourceFence2       Handle of the 2nd fence to be merged

@Input          pszFenceName        Name of the created merged fence or NULL
                                    to request the implementation to generate
                                    an annotation for this fence.
                                    If this string is greater than
                                    PVRSRV_SYNC_NAME_LENGTH the name will be
                                    truncated to that length.
                                    The implementation is free to ignore this
                                    string and use one of the original fence
                                    names if it deems this appropriate.

@Output         phOutputFence       Handle of the newly created merged fence

@Return         PVRSRV_OK           if the merged fence was successfully created
                PVRSRV_ERROR_INVALID_PARAMS if the output pointer provided is
                                    NULL or if either of the fences provided are
                                    invalid
                PVRSRV_ERROR_OUT_OF_MEMORY if there was insufficient memory to
                                    create the new merged fence
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVFenceMergeI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                  PVRSRV_FENCE hSourceFence1,
                  PVRSRV_FENCE hSourceFence2,
                  const IMG_CHAR *pszFenceName,
                  PVRSRV_FENCE *phOutputFence
                  PVR_DBG_FILELINE_PARAM);

static inline PVRSRV_ERROR
PVRSRVFenceMerge(PVRSRV_DEV_CONNECTION *psDevConnection,
                 PVRSRV_FENCE hSourceFence1,
                 PVRSRV_FENCE hSourceFence2,
                 const IMG_CHAR *pszFenceName,
                 PVRSRV_FENCE *phOutputFence)
{
	PVRSRV_ERROR eError;

	eError = PVRSRVFenceMergeI(psDevConnection, hSourceFence1, hSourceFence2,
							   pszFenceName, phOutputFence PVR_DBG_FILELINE);

	if ((hSourceFence1 != PVRSRV_NO_FENCE ||
		 hSourceFence2 != PVRSRV_NO_FENCE) &&
		eError == PVRSRV_OK)
	{
		RGX_HWPERF_SERVICES_SYNC_ALLOC_FENCE_MERGE(psDevConnection,
												   PVRSRVGetCurrentProcessID(),
												   hSourceFence1,
												   hSourceFence2,
												   *phOutputFence);
	}

	return eError;
}

/*************************************************************************/ /*!
@Function       PVRSRVFenceAccumulate

@Description    Same as PVRSRVFenceMerge but destroys the input fences. Once
                called, both input fences can be considered to be invalid and
                using them as input for subsequent calls is undefined.
                If either of the two source fences provided to this function
                are PVRSRV_NO_FENCE then the returned fence will be equal to
                the other fence rather than performing the merge. If both
                fences are PVRSRV_NO_FENCE then PVRSRV_NO_FENCE is returned
                as the merged fence handle and PVRSRV_OK will be returned. This
                removes the need of users of the API validating fences passed
                into this function.

@Input          psDevConnection     The services connection

@Input          hInputFence1        Handle of the 1st fence to be accumulated

@Input          hInputFence2        Handle of the 2nd fence to be accumulated

@Input          pszFenceName        Name of the created accumulated fence or
                                    NULL to request the implementation to
                                    generate an annotation for this fence.
                                    If this string is greater than
                                    PVRSRV_SYNC_NAME_LENGTH the name will be
                                    truncated to that length.
                                    The implementation is free to ignore this
                                    string and use one of the original fence
                                    names if it deems this appropriate.

@Output         phOutputFence       Handle of the newly created fence

@Return         PVRSRV_OK                   if the accumulated fence was
                                            successfully created or
                                            returned
                PVRSRV_ERROR_INVALID_PARAMS if the output pointer provided is
                                            NULL or either of the input fences
                                            is invalid
                PVRSRV_ERROR_OUT_OF_MEMORY  if there was insufficient memory to
                                            create the new merged fence
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVFenceAccumulateI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                       PVRSRV_FENCE hInputFence1,
                       PVRSRV_FENCE hInputFence2,
                       const IMG_CHAR *pszFenceName,
                       PVRSRV_FENCE *phOutputFence
                       PVR_DBG_FILELINE_PARAM);

static inline PVRSRV_ERROR
PVRSRVFenceAccumulate(PVRSRV_DEV_CONNECTION *psDevConnection,
                      PVRSRV_FENCE hInputFence1,
                      PVRSRV_FENCE hInputFence2,
                      const IMG_CHAR *pszFenceName,
                      PVRSRV_FENCE *phOutputFence)
{
	PVRSRV_ERROR eError;

	eError = PVRSRVFenceAccumulateI(psDevConnection, hInputFence1, hInputFence2,
									pszFenceName, phOutputFence
									PVR_DBG_FILELINE);
	if (hInputFence1 != hInputFence2 &&
		hInputFence1 != PVRSRV_NO_FENCE && hInputFence2 != PVRSRV_NO_FENCE &&
		eError == PVRSRV_OK)
	{
		RGX_HWPERF_SERVICES_SYNC_ALLOC_FENCE_MERGE(psDevConnection,
												   PVRSRVGetCurrentProcessID(),
												   hInputFence1,
												   hInputFence2,
												   *phOutputFence);
		RGX_HWPERF_SERVICES_SYNC_FREE(psDevConnection, PVRSRVGetCurrentProcessID(),
									  FENCE, hInputFence1);
		RGX_HWPERF_SERVICES_SYNC_FREE(psDevConnection, PVRSRVGetCurrentProcessID(),
									  FENCE, hInputFence2);
	}

	return eError;
}

/*************************************************************************/ /*!
@Function       PVRSRVFenceDump

@Description    Dumps debug information about the specified fence. The stream
                to which this output is output is implementation specific.
                If PVRSRV_NO_FENCE is passed to this function no operation will
                be performed and PVRSRV_OK will be returned. This removes the
                need of users of the API validating fences passed into this
                function.

@Input          psDevConnection     The services connection

@Input          hFence              Handle to the fence

@Input          pszModule           Client module calling this function

@Input          pszDesc             Info string provided by the client module
                                    showing what this fence is currently being
                                    used for.

@Return         PVRSRV_OK                   if a valid fence was specified
                PVRSRV_ERROR_INVALID_PARAMS if an invalid fence was specified
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVFenceDumpI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                 PVRSRV_FENCE hFence,
                 const IMG_CHAR *pszModule,
                 const IMG_CHAR *pszDesc
                 PVR_DBG_FILELINE_PARAM);
#define PVRSRVFenceDump(psDevConnection, hFence, pszModule, pszDesc) \
	PVRSRVFenceDumpI( (psDevConnection), (hFence), (pszModule), (pszDesc) PVR_DBG_FILELINE)

/*************************************************************************/ /*!
@Function       PVRSRVFenceDestroy

@Description    Destroy this handle to a fence.
                The underlying fence object will be destroyed when all handles
                have been dropped.
                If PVRSRV_NO_FENCE is passed to this function no operation will
                be performed and PVRSRV_OK will be returned. This removes the
                need of users of the API validating fences passed into this
                function.

@Input          psDevConnection     The services connection

@Input          hFence              Handle to the fence

@Return         PVRSRV_OK                   if the fence handle was
                                            successfully destroyed.
                PVRSRV_ERROR_INVALID_PARAMS if an invalid fence was specified
*/ /***************************************************************************/
IMG_EXPORT PVRSRV_ERROR
PVRSRVFenceDestroyI(const PVRSRV_DEV_CONNECTION *psDevConnection,
                    PVRSRV_FENCE hFence
                    PVR_DBG_FILELINE_PARAM);

static inline PVRSRV_ERROR
PVRSRVFenceDestroy(PVRSRV_DEV_CONNECTION *psDevConnection, PVRSRV_FENCE hFence)
{
	PVRSRV_ERROR eError;

	eError = PVRSRVFenceDestroyI(psDevConnection, hFence PVR_DBG_FILELINE);
	if (hFence != PVRSRV_NO_FENCE && eError == PVRSRV_OK)
	{
		RGX_HWPERF_SERVICES_SYNC_FREE(psDevConnection, PVRSRVGetCurrentProcessID(),
		                              FENCE, hFence);
	}

	return eError;
}

#if defined(PVR_SYNC_UM_API_DEBUG)
#include "pvrsrv_sync_um_debug.h"
#endif


#if defined (__cplusplus)
}
#endif
#endif	/* PVRSRV_SYNC_UM_H */
