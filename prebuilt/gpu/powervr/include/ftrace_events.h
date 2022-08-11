/*************************************************************************/ /*!
@File           ftrace_events.h
@Title          Userspace definitions to use the kernel ftrace framework
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

#ifndef _FTRACE_EVENTS_H
#define _FTRACE_EVENTS_H

#include "img_defs.h"

#include "pvrsrv_sync_um.h"

#define PVRTRACE_TAG_SYNCS       1 << 0
#define PVRTRACE_TAG_EGL_CS      1 << 1
#define PVRTRACE_TAG_WSEGL_CS    1 << 2
#define PVRTRACE_TAG_GLES_CS     1 << 3
#define PVRTRACE_TAG_SERVICES_CS 1 << 4
#define PVRTRACE_TAG_ANDROID_CS  1 << 5
#define PVRTRACE_TAG_LINUX_CS    1 << 6
#define PVRTRACE_TAG_OTHERS      1 << 31

#define EGL_ENTER()                PVRTraceEnterEvent(PVRTRACE_TAG_EGL_CS, "egl")
#define EGL_LEAVE()                PVRTraceLeaveEvent(PVRTRACE_TAG_EGL_CS, "egl")
#define EGL_ENTER_NAME(fmt, ...)   PVRTraceEnterNameEvent(PVRTRACE_TAG_EGL_CS, "egl", fmt, ##__VA_ARGS__)
#define EGL_LEAVE_NAME(fmt, ...)   PVRTraceLeaveNameEvent(PVRTRACE_TAG_EGL_CS, "egl", fmt, ##__VA_ARGS__)
#define WSEGL_ENTER()              PVRTraceEnterEvent(PVRTRACE_TAG_WSEGL_CS, "wsegl")
#define WSEGL_LEAVE()              PVRTraceLeaveEvent(PVRTRACE_TAG_WSEGL_CS, "wsegl")
#define WSEGL_ENTER_NAME(fmt, ...) PVRTraceEnterNameEvent(PVRTRACE_TAG_WSEGL_CS, "wsegl", fmt, ##__VA_ARGS__)
#define WSEGL_LEAVE_NAME(fmt, ...) PVRTraceLeaveNameEvent(PVRTRACE_TAG_WSEGL_CS, "wsegl", fmt, ##__VA_ARGS__)
#define GLES1_ENTER()              PVRTraceEnterEvent(PVRTRACE_TAG_GLES_CS, "gles1")
#define GLES1_LEAVE()              PVRTraceLeaveEvent(PVRTRACE_TAG_GLES_CS, "gles1")
#define GLES1_ENTER_NAME(fmt, ...) PVRTraceEnterNameEvent(PVRTRACE_TAG_GLES_CS, "gles1", fmt, ##__VA_ARGS__)
#define GLES1_LEAVE_NAME(fmt, ...) PVRTraceLeaveNameEvent(PVRTRACE_TAG_GLES_CS, "gles1", fmt, ##__VA_ARGS__)
#define GLES3_ENTER()              PVRTraceEnterEvent(PVRTRACE_TAG_GLES_CS, "gles3")
#define GLES3_LEAVE()              PVRTraceLeaveEvent(PVRTRACE_TAG_GLES_CS, "gles3")
#define GLES3_ENTER_NAME(fmt, ...) PVRTraceEnterNameEvent(PVRTRACE_TAG_GLES_CS, "gles3", fmt, ##__VA_ARGS__)
#define GLES3_LEAVE_NAME(fmt, ...) PVRTraceLeaveNameEvent(PVRTRACE_TAG_GLES_CS, "gles3", fmt, ##__VA_ARGS__)
#define SRV_ENTER()                PVRTraceEnterEvent(PVRTRACE_TAG_SERVICES_CS, "srv")
#define SRV_LEAVE()                PVRTraceLeaveEvent(PVRTRACE_TAG_SERVICES_CS, "srv")
#define SRV_ENTER_NAME(fmt, ...)   PVRTraceEnterNameEvent(PVRTRACE_TAG_SERVICES_CS, "srv", fmt, ##__VA_ARGS__)
#define SRV_LEAVE_NAME(fmt, ...)   PVRTraceLeaveNameEvent(PVRTRACE_TAG_SERVICES_CS, "srv", fmt, ##__VA_ARGS__)

/* Use the following macros for quickly bisecting the performance
 * characteristics of a function with the help of systrace. */
#define FTTEST_INIT() int ftrace_test_line = -1; const char* ftrace_test_func = NULL;
#define FTTEST_NEXT() \
{ \
	if (ftrace_test_line >= 0) PVRTraceLeaveNameEvent(PVRTRACE_TAG_OTHERS, "tst", "%s-%d", ftrace_test_func, ftrace_test_line); \
	ftrace_test_line = __LINE__; \
	ftrace_test_func = __func__; \
	PVRTraceEnterNameEvent(PVRTRACE_TAG_OTHERS, "tst", "%s-%d", ftrace_test_func, ftrace_test_line); \
}
#define FTTEST_END() \
{ \
	if (ftrace_test_line >= 0) PVRTraceLeaveNameEvent(PVRTRACE_TAG_OTHERS, "tst", "%s-%d", ftrace_test_func, ftrace_test_line); \
}

#define PVRTRACE_TAG_ENABLED(tag) unlikely(g_ui32fTraceEnabledTags & (tag))

extern IMG_UINT32 g_ui32fTraceEnabledTags;

void __printf(3, 4)
PVRTraceEnterNameEventPrivate(uint32_t tag, const char *pcszModule, const char *pcszFmt, ...);

void __printf(3, 4)
PVRTraceLeaveNameEventPrivate(uint32_t tag, const char *pcszModule, const char *pcszFmt, ...);

#if defined(LINUX) && (defined(DEBUG) || defined(TIMING) || defined(GLES_RELEASE_SYSTRACE))

#define PVRTraceEnterNameEvent(tag, module, fmt, ...) \
  do \
  { \
	if (PVRTRACE_TAG_ENABLED(tag)) \
		PVRTraceEnterNameEventPrivate(tag, module, fmt, ##__VA_ARGS__); \
  } while (0)

#define PVRTraceLeaveNameEvent(tag, module, fmt, ...) \
  do \
  { \
	if (PVRTRACE_TAG_ENABLED(tag)) \
		PVRTraceLeaveNameEventPrivate(tag, module, fmt, ##__VA_ARGS__); \
  } while (0)

#define PVRTraceEnterEvent(tag, module)                    PVRTraceEnterNameEvent(tag, module, __func__)
#define PVRTraceLeaveEvent(tag, module)                    PVRTraceLeaveNameEvent(tag, module, __func__)

#else /* defined(LINUX) && (defined(DEBUG) || defined(TIMING) || defined(GLES_RELEASE_SYSTRACE)) */

#define PVRTraceEnterNameEvent(tag, module, fmt, ...)      do {} while (0)
#define PVRTraceLeaveNameEvent(tag, module, fmt, ...)      do {} while (0)
#define PVRTraceEnterEvent(tag, module)                    do {} while (0)
#define PVRTraceLeaveEvent(tag, module)                    do {} while (0)

#endif /* defined(LINUX) && (defined(DEBUG) || defined(TIMING) || defined(GLES_RELEASE_SYSTRACE)) */

#if defined(LINUX)

void __printf(4, 5)
PVRTraceNativeSyncEventPrivate(uint32_t tag, PVRSRV_FENCE hFence, const char *pcszModule,
							   const char *pcszFmt, ...);

#define PVRTraceFenceSyncEvent(connection, fence, module, fmt, ...) \
  do \
  { \
	if (PVRTRACE_TAG_ENABLED(PVRTRACE_TAG_SYNCS)) \
		PVRTraceNativeSyncEventPrivate(PVRTRACE_TAG_SYNCS, fence, module, fmt , ##__VA_ARGS__); \
	PVRTraceFenceDump(connection, fence, module, fmt, ##__VA_ARGS__); \
  } while (0)

#if defined(PVR_FENCE_SYNC_DEBUG)

#define PVRTraceFenceDump(connection, fence, module, fmt, ...) \
	do { \
		char desc[32]; \
		snprintf(desc, sizeof(desc), fmt, ##__VA_ARGS__); \
		PVRSRVFenceDump(connection, fence, module, desc); \
	} while (0)

#else  /* defined(PVR_FENCE_SYNC_DEBUG) */

#define PVRTraceFenceDump(connection, fence, module, fmt, ...) do {} while(0)

#endif /* defined(PVR_FENCE_SYNC_DEBUG) */

#else /* defined(LINUX) */

#define PVRTraceFenceSyncEvent(connection, fence, module, fmt, ...)      do {} while (0)

#endif /* defined(LINUX) */

#endif /* _FTRACE_EVENTS_H */
