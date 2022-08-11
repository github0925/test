/*************************************************************************/ /*!
@File
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
#ifndef __EGLUTILS_H__
#define __EGLUTILS_H__

#if defined (__cplusplus)
extern "C" {
#endif

#if defined(__linux__) || defined(__QNXNTO__)
	#define internal __attribute__((visibility("hidden")))
#else
	#define internal
#endif

internal void set_swap_interval_egl(void *egldisplay, unsigned int interval);
internal const char * GetEGLErrorString(EGLint error_code);

#if defined (__cplusplus)
}
#endif

#endif
