/*************************************************************************/ /*!
@File           pvr_fd_sync_user.h
@Title          Userspace definitions to use the kernel sync driver
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
/* vi: set ts=8: */

#ifndef PVR_FD_SYNC_USER_H
#define PVR_FD_SYNC_USER_H

#include <stdbool.h>
#include <stdint.h>

#include "img_defs.h"
#include "pvrsrv_error.h"

#if defined(SUPPORT_NATIVE_FENCE_SYNC)
/* operates on timelines (only) */

PVRSRV_ERROR PVRFDSyncOpen(int *piSyncFd, bool bSoftware,
			   const char *pszPrefix);

PVRSRV_ERROR PVRFDSyncClose(int iSyncFd);

/* operates on fences */

PVRSRV_ERROR PVRFDSyncWaitFenceI(int iFenceFd, const char *szFunc);
PVRSRV_ERROR PVRFDSyncWaitFenceI_(int iFenceFd, int iTimeoutInMs, const char *szFunc);

PVRSRV_ERROR PVRFDSyncCheckFenceI(int iFenceFd, const char *szFunc);

PVRSRV_ERROR PVRFDSyncMergeFencesI(const char *pcszName,
				   int iFenceFd1, int iFenceFd2,
				   int *piNewFenceFd, const char *szFunc);

PVRSRV_ERROR PVRFDSyncCloseFenceI(int iFenceFd, const char *szFunc);

int PVRFDSyncDupFenceI(int iFenceFd, const char *szFunc);
PVRSRV_ERROR PVRFDSyncDupFenceI_(int iFenceFd, const char *szFunc, int *piNewFenceFd);

PVRSRV_ERROR PVRFDSyncAccumulateFencesI(const char *pcszName, int iFenceFd1,
					int iFenceFd2, int *piOutputFenceFd,
					const char *szFunc);

PVRSRV_ERROR PVRFDSyncDumpFence(int iFenceFd, const char *pcszModule,
				const char *pcszFmt, ...) __printf(3, 4);


/* sw fences */
int PVRFDSWSyncFenceCreateI(int iSWTimelineFd, const char *pcszName,
							const char *szFunc);
PVRSRV_ERROR PVRFDSWSyncFenceCreateI_(int iSWTimelineFd, const char *pcszName,
                                      const char *szFunc, int *piNewSWFenceFd,
									  uint64_t *pui64SyncPtIdx);
PVRSRV_ERROR PVRFDSWSyncTimelineIncI(int iSWTimelineFd, uint64_t *pui64SyncPtIdx,
				const char *szFunc);

#else /* defined(SUPPORT_NATIVE_FENCE_SYNC) */

#include <unistd.h>
#include <string.h>

#include "pvr_debug.h"

/* operates on timelines (only) */

static inline PVRSRV_ERROR PVRFDSyncOpen(int *piSyncFd, bool bSoftware,
					 const char *pszPrefix)
{
	PVR_UNREFERENCED_PARAMETER(bSoftware);
	PVR_UNREFERENCED_PARAMETER(pszPrefix);
	*piSyncFd = -1;
	return PVRSRV_OK;
}

static inline PVRSRV_ERROR PVRFDSyncClose(int iSyncFd)
{
	if (iSyncFd >= 0)
	{
		PVR_DPF((PVR_DBG_ERROR, "%s: Got unexpected native sync "
					"timeline %d on non-native sync "
					"enabled driver", __func__, iSyncFd));
	}
	return PVRSRV_OK;
}

/* operates on fences */

static inline PVRSRV_ERROR PVRFDSyncWaitFenceI(int iFenceFd,
					       const char *szFunc)
{
#if !(defined(PVRSRV_NEED_PVR_DPF) && defined(DEBUG))
	PVR_UNREFERENCED_PARAMETER(szFunc);
	PVR_UNREFERENCED_PARAMETER(iFenceFd);
#endif
	if (iFenceFd >= 0)
	{
		PVR_DPF((PVR_DBG_WARNING, "%s: Got unexpected native sync "
					  "fence %d on non-native sync "
					  "enabled driver", szFunc, iFenceFd));
	}
	return PVRSRV_OK;
}
static inline PVRSRV_ERROR PVRFDSyncWaitFenceI_(int iFenceFd,
					       int iTimeoutInMs,
					       const char *szFunc)
{
#if !(defined(PVRSRV_NEED_PVR_DPF) && defined(DEBUG))
	PVR_UNREFERENCED_PARAMETER(szFunc);
	PVR_UNREFERENCED_PARAMETER(iFenceFd);
#endif
	PVR_UNREFERENCED_PARAMETER(iTimeoutInMs);
	if (iFenceFd >= 0)
	{
		PVR_DPF((PVR_DBG_WARNING, "%s: Got unexpected native sync "
					  "fence %d on non-native sync "
					  "enabled driver", szFunc, iFenceFd));
	}
	return PVRSRV_OK;
}

static inline PVRSRV_ERROR PVRFDSyncCheckFenceI(int iFenceFd,
						const char *szFunc)
{
#if !(defined(PVRSRV_NEED_PVR_DPF) && defined(DEBUG))
	PVR_UNREFERENCED_PARAMETER(szFunc);
	PVR_UNREFERENCED_PARAMETER(iFenceFd);
#endif
	if (iFenceFd >= 0)
	{
		PVR_DPF((PVR_DBG_WARNING, "%s: Got unexpected native sync "
		                          "fence %d on non-native sync "
		                          "enabled driver", szFunc, iFenceFd));
	}
	return PVRSRV_OK;
}

static inline PVRSRV_ERROR PVRFDSyncMergeFencesI(const char *pcszName,
						 int iFenceFd1, int iFenceFd2,
						 int *piNewFenceFd,
						 const char *szFunc)
{
	PVR_UNREFERENCED_PARAMETER(pcszName);
	PVR_UNREFERENCED_PARAMETER(iFenceFd1);
	PVR_UNREFERENCED_PARAMETER(iFenceFd2);
	PVR_UNREFERENCED_PARAMETER(szFunc);
	*piNewFenceFd = -1;
	return PVRSRV_OK;
}

static inline PVRSRV_ERROR PVRFDSyncCloseFenceI(int iFenceFd,
						const char *szFunc)
{
	PVR_UNREFERENCED_PARAMETER(szFunc);
	if (iFenceFd >= 0)
	{
		close(iFenceFd);
	}
	return PVRSRV_OK;
}

static inline int PVRFDSyncDupFenceI(int iFenceFd, const char *szFunc)
{
#if !(defined(PVRSRV_NEED_PVR_DPF) && defined(DEBUG))
	PVR_UNREFERENCED_PARAMETER(szFunc);
#endif
	if (iFenceFd >= 0)
	{
		PVR_DPF((PVR_DBG_WARNING, "%s: Got unexpected native sync "
		                          "fence %d on non-native sync "
		                          "enabled driver", szFunc, iFenceFd));
	}
	return -1;
}
static inline PVRSRV_ERROR PVRFDSyncDupFenceI_(int iFenceFd, const char *szFunc, int *piNewFenceFd)
{
#if !(defined(PVRSRV_NEED_PVR_DPF) && defined(DEBUG))
	PVR_UNREFERENCED_PARAMETER(szFunc);
#endif
	PVR_UNREFERENCED_PARAMETER(piNewFenceFd);
	if (iFenceFd >= 0)
	{
		PVR_DPF((PVR_DBG_WARNING, "%s: Got unexpected native sync "
		                          "fence %d on non-native sync "
		                          "enabled driver", szFunc, iFenceFd));
	}
	return PVRSRV_OK;
}

static inline PVRSRV_ERROR PVRFDSyncAccumulateFencesI(const char *pcszName,
	int iFenceFd1, int iFenceFd2, int *piOutputFenceFd, const char *szFunc)
{
	PVR_UNREFERENCED_PARAMETER(pcszName);
#if !(defined(PVRSRV_NEED_PVR_DPF) && defined(DEBUG))
	PVR_UNREFERENCED_PARAMETER(szFunc);
#endif
	if (iFenceFd1 >= 0)
	{
		PVR_DPF((PVR_DBG_WARNING, "%s: Got unexpected native sync "
		                          "fence1 %d on non-native sync "
		                          "enabled driver", szFunc, iFenceFd1));
	}
	if (iFenceFd2 >= 0)
	{
		PVR_DPF((PVR_DBG_WARNING, "%s: Got unexpected native sync "
		                          "fence2 %d on non-native sync "
		                          "enabled driver", szFunc, iFenceFd2));
	}
	if (iFenceFd1 >= 0)
	{
		close(iFenceFd1);
	}
	if (iFenceFd2 >= 0)
	{
		close(iFenceFd2);
	}
	*piOutputFenceFd = -1;
	return PVRSRV_OK;
}

static inline PVRSRV_ERROR PVRFDSyncDumpFence(int iFenceFd,
					      const char *pcszModule,
					      const char *pcszFmt, ...)
{
	PVR_UNREFERENCED_PARAMETER(iFenceFd);
	PVR_UNREFERENCED_PARAMETER(pcszModule);
	PVR_UNREFERENCED_PARAMETER(pcszFmt);
	return PVRSRV_OK;
}

/* sw fences */
static inline int PVRFDSWSyncFenceCreateI(int iSyncFd, const char *pcszName,
										  const char *szFunc)
{
	PVR_UNREFERENCED_PARAMETER(iSyncFd);
	PVR_UNREFERENCED_PARAMETER(pcszName);
	PVR_UNREFERENCED_PARAMETER(szFunc);
	return -1;
}
static inline PVRSRV_ERROR PVRFDSWSyncFenceCreateI_(int iSyncFd, const char *pcszName,
													const char *szFunc, int *piNewSWFenceFd,
													uint64_t *pui64SyncPtIdx)
{
	PVR_UNREFERENCED_PARAMETER(iSyncFd);
	PVR_UNREFERENCED_PARAMETER(pcszName);
	PVR_UNREFERENCED_PARAMETER(szFunc);
	PVR_UNREFERENCED_PARAMETER(piNewSWFenceFd);
	PVR_UNREFERENCED_PARAMETER(pui64SyncPtIdx);
	return PVRSRV_OK;
}


static inline PVRSRV_ERROR PVRFDSWSyncTimelineIncI(int iFenceFd,
				uint64_t *pui64SyncPtIdx,
				const char *szFunc)
{
	PVR_UNREFERENCED_PARAMETER(iFenceFd);
	PVR_UNREFERENCED_PARAMETER(pui64SyncPtIdx);
	PVR_UNREFERENCED_PARAMETER(szFunc);
	return PVRSRV_OK;
}

#endif /* defined(SUPPORT_NATIVE_FENCE_SYNC) */

#if defined(PVRSRV_NEED_PVR_DPF)

#define PVRFDSyncWaitFence(a) \
	PVRFDSyncWaitFenceI(a, __func__)
#define PVRFDSyncWaitFence_(a, b) \
	PVRFDSyncWaitFenceI_(a, b, __func__)
#define PVRFDSyncCheckFence(a) \
	PVRFDSyncCheckFenceI(a, __func__)
#define PVRFDSyncMergeFences(a, b, c, d) \
	PVRFDSyncMergeFencesI(a, b, c, d, __func__)
#define PVRFDSyncCloseFence(a) \
	PVRFDSyncCloseFenceI(a, __func__)
#define PVRFDSyncDupFence(a) \
	PVRFDSyncDupFenceI(a, __func__)
#define PVRFDSyncDupFence_(a, b) \
	PVRFDSyncDupFenceI_(a, __func__, b)
#define PVRFDSyncAccumulateFences(a, b, c, d) \
	PVRFDSyncAccumulateFencesI(a, b, c, d, __func__)
#define PVRFDSWSyncFenceCreate(a, b) \
	PVRFDSWSyncFenceCreateI(a, b, __func__)
#define PVRFDSWSyncFenceCreate_(a, b, c, d) \
	PVRFDSWSyncFenceCreateI_(a, b, __func__, c, d)
#define PVRFDSWSyncTimelineInc(a, b) \
	PVRFDSWSyncTimelineIncI(a, b, __func__)

#else /* defined(PVRSRV_NEED_PVR_DPF) */

#define PVRFDSyncWaitFence(a)              PVRFDSyncWaitFenceI(a, "")
#define PVRFDSyncWaitFence_(a, b)          PVRFDSyncWaitFenceI_(a, b, "")
#define PVRFDSyncCheckFence(a)             PVRFDSyncCheckFenceI(a, "")
#define PVRFDSyncMergeFences(a, b, c, d)   PVRFDSyncMergeFencesI(a, b, c, d, "")
#define PVRFDSyncCloseFence(a)             PVRFDSyncCloseFenceI(a, "")
#define PVRFDSyncDupFence(a)               PVRFDSyncDupFenceI(a, "")
#define PVRFDSyncDupFence_(a, b)           PVRFDSyncDupFenceI_(a, "", b)
#define PVRFDSyncAccumulateFences(a, b, c, d) \
	PVRFDSyncAccumulateFencesI(a, b, c, d, "")
#define PVRFDSWSyncFenceCreate(a, b)    PVRFDSWSyncFenceCreateI(a, b, "")
#define PVRFDSWSyncFenceCreate_(a, b, c, d) PVRFDSWSyncFenceCreateI_(a, b, "", c, d)
#define PVRFDSWSyncTimelineInc(a, b)       PVRFDSWSyncTimelineIncI(a, b, "")

#endif /* defined(PVRSRV_NEED_PVR_DPF) */

#endif /* PVR_FD_SYNC_USER_H */
