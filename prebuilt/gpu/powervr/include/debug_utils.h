/*************************************************************************/ /*!
@File
@Title         Debug Utilities
@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License       Strictly Confidential.
@Description   Various debug helper functions

This module adds functions for dumping stack straces to the client DDK. There
are two sets of functions: native and non-native.

The native functions use functions native to a specific operating system, e.g.:
Android version uses android::CallStack class. Those functions in general should
show more accurate stack traces than the non-native counterpart (this not always
has to be true, e.g.: for Linux libunwind in general yields better results
than it's native backtrace()).

The non-native functions are based on libunwind and should show same results
in different operating system (this is true for Android and Linux but not
necessarily for other OSes). The results in most cases should be accurate
enough.

The "save" and "print" functions in non-native case should be faster than
the native ones. They also should use smaller memory footprint that the native
counterparts.

Downside to non-native approach is that not always is avaliable in the system.
For example starting from Android O it's not allowed to link to libunwind
so the native functions have to be used in all cases.

Native functions are enabled with PVRSRV_NEED_PVR_STACKTRACE_NATIVE compilation
switch and non-native functions with PVRSRV_NEED_PVR_STACKTRACE.
*/ /**************************************************************************/

#ifndef _DEBUG_UTILS_H_
#define _DEBUG_UTILS_H_

#include "img_types.h"
#include "img_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The PVRSRV_NEED_PVR_STACKTRACE_NATIVE=1 build option enables stack tracing that
 * requires functionality that is natively provided/allowed by the target
 * OS.
 */

#if defined(PVRSRV_NEED_PVR_STACKTRACE_NATIVE)

/** Prints stack trace.
 *
 * This is the platform dependent stack trace function.
 *
 * For Android it will use CallStack.h API which yields the best results when
 * it comes to printing complete stack trace. This function is able to resolve
 * symbols in most of the cases (sometimes even in release builds). For C++ it
 * even demangles the function names.
 *
 * In case of Linux execinfo.h API is used. This is not as good as Android's
 * API and sometimes not event as good as PVRSRVDumpStackTrace. The restriction
 * in case of this function is that the executable should be build with
 * -rdynamic linker option. If the linker option is not used in most of the
 * cases symbols will not be resolved.
 *
 * @param uiSkipFrames number of skipped top frames form the stack trace
 * @param pszHeader zero terminated string that will be included in a stack
 *                  trace header in a following way: "Call stack (%s):".
 *                  If pointer is NULL the " (%s)" part is omitted.
 */
void PVRSRVNativeDumpStackTrace(size_t uiSkipFrames, const char *pszHeader);

/** Saves stack trace.
 *
 * This function is similar to PVRSRVNativeDumpStackTrace but instead of
 * printing the stack trace it saves it and returns a pointer to the stack
 * trace. The stack trace is kept in internal format and can be printed
 * by PVRSRVNativePrintStackTrace.
 *
 * The pointer can point to heap memory and is freed once print function
 * is called. This means that it might not be reusable after the stack trace
 * is printed.
 *
 * @param uiSkipFrames number of skipped top frames form the stack trace
 * @return pointer to stack trace representation
 */
void *PVRSRVNativeSaveStackTrace(size_t uiSkipFrames);

/** Prints stack trace.
 *
 * Prints stack trace captured with PVRSRVNativeDumpStackTrace.
 *
 * The pointer pvStackTrace cannot be used again after this function is called.
 * It should be nulled and discarted after one use.
 *
 * @param pvStackTrace pointer to stack trace representation
 * @param pszHeader zero terminated string that will be included in a stack
 *                  trace header in a following way: "Call stack (%s):".
 *                  If pointer is NULL the " (%s)" part is omitted.
 */
void PVRSRVNativePrintStackTrace(void *pvStackTrace, const char *pszHeader);

#else /* defined(PVRSRV_NEED_PVR_STACKTRACE_NATIVE) */

#define PVRSRVNativeDumpStackTrace(uiSkipFrames, pszHeader) \
		static_assert(0, "Please enable PVRSRV_NEED_PVR_STACKTRACE_NATIVE to use PVRSRVNativeDumpStackTrace")
#define PVRSRVNativeSaveStackTrace(uiSkipFrames) \
		static_assert(0, "Please enable PVRSRV_NEED_PVR_STACKTRACE_NATIVE to use PVRSRVNativeSaveStackTrace")
#define PVRSRVNativePrintStackTrace(pvStackTrace, pszHeader) \
		static_assert(0, "Please enable PVRSRV_NEED_PVR_STACKTRACE_NATIVE to use PVRSRVNativePrintStackTrace")

#endif /* defined(PVRSRV_NEED_PVR_STACKTRACE_NATIVE) */

/* The PVRSRV_NEED_PVR_STACKTRACE=1 build option enables stack
 * tracing that requires functionality that is either natively provided/allowed
 * by the target OS, OR requires hand-written code or additional dependencies.
 * If your OS needs to avoid additional dependencies, do not set this option.
 */

#if defined(PVRSRV_NEED_PVR_STACKTRACE)

/** Prints stack trace.
 *
 * This function uses libunwind to unwind the stack. This is the cross-platform
 * approach even though the library does not always work as expected.
 *
 * Known issue is that in Android M it doesn't resolve symbols at all. It should
 * be however possible to resolve them offline when binaries are available.
 * The trace will print offset of the symbol in the library/executable and
 * the name of the library/executable. Sometimes it will also print the symbol
 * itself if the dladdr was able to retrieve it.
 *
 * @param uiSkipFrames number of skipped top frames
 * @param uiMaxFrames number of printed frames in total (0 means all)
 * @param pszHeader zero terminated string that will be included in a stack
 *                  trace header in a following way: "Call stack (%s):".
 *                  If pointer is NULL the " (%s)" part is omitted.
 */
void PVRSRVDumpStackTrace(size_t uiSkipFrames, size_t uiMaxFrames,
                          const char *pszHeader);

/** Saves stack trace in the array.
 *
 * This function is similar to PVRSRVDumpStackTrace but instead of printing
 * the stack trace it saves it to the given array (in a form of array of
 * program counter values).
 *
 * @param atCallStack buffer that should be large enough to fit either all
 *                    or uiMexFrmames (if given as not 0) unwound frames
 * @param uiSkipFrames number of skipped top frames
 * @param uiMaxFarmes maximum number of frames that should be unwind (0 means
 *                    all)
 * @return number of saved frames
 */
size_t PVRSRVSaveStackTrace(uintptr_t puiCallStack[], size_t uiSkipFrames,
                            size_t uiMaxFrames);

/** Prints stack trace form the array.
 *
 * Prints stack trace captured with PVRSRVSaveStackTrace.
 *
 * @param atCallStack buffer with saved stack trace
 * @param uiSkipFrames number of skipped top frames
 * @param uiFramesCount number of frames returned by PVRSRVSaveStackTrace
 * @param pszHeader zero terminated string that will be included in a stack
 *                  trace header in a following way: "Call stack (%s):".
 *                  If pointer is NULL the " (%s)" part is omitted.
 */
void PVRSRVPrintStackTrace(uintptr_t puiCallStack[], size_t uiSkipFrames,
                           size_t uiFramesCount, const char *pszHeader);

#else /* defined(PVRSRV_NEED_PVR_STACKTRACE) */

#define PVRSRVDumpStackTrace(uiSkipFrames, uiMaxFrames, pszHeader) \
		static_assert(0, "Please enable PVRSRV_NEED_PVR_STACKTRACE to use PVRSRVDumpStackTrace")

#define PVRSRVSaveStackTrace(puiCallStack, uiSkipFrames, uiMaxFrames) \
		static_assert(0, "Please enable PVRSRV_NEED_PVR_STACKTRACE to use PVRSRVSaveStackTrace")

#define PVRSRVPrintStackTrace(puiCallStack, uiSkipFrames, uiFramesCount, pszHeader) \
		static_assert(0, "Please enable PVRSRV_NEED_PVR_STACKTRACE to use PVRSRVPrintStackTrace")

#endif /* defined(PVRSRV_NEED_PVR_STACKTRACE) */

#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_UTILS_H_ */
