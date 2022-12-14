/*************************************************************************/ /*!
@File
@Title          Windowing system specific EGL internal interface definition.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(__IMGEGL_H__)
#define __IMGEGL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "img_types.h"
#include "imgextensions.h"

EGLint IMGeglGetError(void);

EGLDisplay IMGeglGetDisplay(EGLNativeDisplayType display);
EGLBoolean IMGeglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
EGLBoolean IMGeglTerminate(EGLDisplay dpy);
const char * IMGeglQueryString(EGLDisplay dpy, EGLint name);
void (* IMGeglGetProcAddress(const char *procname))(void);

EGLBoolean IMGeglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLBoolean IMGeglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
EGLBoolean IMGeglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);

EGLSurface IMGeglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType window, const EGLint *attrib_list);
EGLSurface IMGeglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
EGLSurface IMGeglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
EGLBoolean IMGeglDestroySurface(EGLDisplay dpy, EGLSurface surface);
EGLBoolean IMGeglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);

EGLContext IMGeglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_list, const EGLint *attrib_list);
EGLBoolean IMGeglDestroyContext(EGLDisplay dpy, EGLContext ctx);
EGLBoolean IMGeglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
EGLContext IMGeglGetCurrentContext(void);
EGLSurface IMGeglGetCurrentSurface(EGLint readdraw);
EGLDisplay IMGeglGetCurrentDisplay(void);
EGLBoolean IMGeglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);

EGLBoolean IMGeglWaitGL(void);
EGLBoolean IMGeglWaitNative(EGLint engine);
EGLBoolean IMGeglSwapBuffers(EGLDisplay dpy, EGLSurface draw);
EGLBoolean IMGeglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);

EGLBoolean IMGeglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
EGLBoolean IMGeglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLBoolean IMGeglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLBoolean IMGeglSwapInterval(EGLDisplay dpy, EGLint interval);

EGLSurface IMGeglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
															  EGLConfig config, const EGLint *attrib_list);
EGLBoolean IMGeglBindAPI(EGLenum api);
EGLenum    IMGeglQueryAPI(void);
EGLBoolean IMGeglWaitClient(void);
EGLBoolean IMGeglReleaseThread(void);

#if defined(EGL_EXTENSION_PARTIAL_UPDATES)
EGLBoolean IMGeglSwapBuffersWithDamageKHR(EGLDisplay eglDpy, EGLSurface draw, EGLint *rects, EGLint n_rects);
EGLBoolean IMGeglSetDamageRegionKHR(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects);
#endif /* defined(EGL_EXTENSION_PARTIAL_UPDATES) */

#if defined(EGL_EXTENSION_PLATFORM_BASE)
EGLDisplay IMGeglGetPlatformDisplayEXT(EGLenum ePlatform,
									   void *pNativeDisplay,
									   const EGLint *paiAttribList);

EGLSurface IMGeglCreatePlatformWindowSurfaceEXT(EGLDisplay eglDpy,
												EGLConfig eglConfig,
												void *pNativeWindow,
												const EGLint *paiAttribList);

EGLSurface IMGeglCreatePlatformPixmapSurfaceEXT(EGLDisplay eglDpy,
												EGLConfig eglConfig,
												void *pNativePixmap,
												const EGLint *paiAttribList);
#endif /* defined(EGL_EXTENSION_PLATFORM_BASE) */

#ifdef __cplusplus
}
#endif


#endif /* __IMGEGL_H__ */

/******************************************************************************
 End of file (imgegl.h)
******************************************************************************/
