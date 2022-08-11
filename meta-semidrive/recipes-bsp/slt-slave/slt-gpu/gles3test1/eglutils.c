/*************************************************************************/ /*!
@File
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <EGL/egl.h>

#include "eglutils.h"

void set_swap_interval_egl(void *data, unsigned int interval)
{
	eglSwapInterval((EGLDisplay)data, interval);
}

const char * GetEGLErrorString(EGLint error_code)
{
	switch(error_code)
	{
#define X(error) case error: return #error;
		X(EGL_SUCCESS)
		X(EGL_NOT_INITIALIZED)
		X(EGL_BAD_ACCESS)
		X(EGL_BAD_ALLOC)
		X(EGL_BAD_ATTRIBUTE)
		X(EGL_BAD_CONFIG)
		X(EGL_BAD_CONTEXT)
		X(EGL_BAD_CURRENT_SURFACE)
		X(EGL_BAD_DISPLAY)
		X(EGL_BAD_MATCH)
		X(EGL_BAD_NATIVE_PIXMAP)
		X(EGL_BAD_NATIVE_WINDOW)
		X(EGL_BAD_PARAMETER)
		X(EGL_BAD_SURFACE)
		X(EGL_CONTEXT_LOST)
#undef X
		default: return "unknown";
	}
}

