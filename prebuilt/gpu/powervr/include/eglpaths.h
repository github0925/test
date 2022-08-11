/*************************************************************************/ /*!
@Title          OS-dependent library paths for EGL APIs
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        Strictly Confidential.
*/ /**************************************************************************/

/* If the build options didn't set EGL_BASENAME_SUFFIX, leave it empty. */
#if !defined(EGL_BASENAME_SUFFIX)
#define EGL_BASENAME_SUFFIX
#endif

/* Work out the base path to the system EGL libraries. */
#if defined(SUPPORT_ARC_PLATFORM)
/*
 * On ARC, Mesa provides the EGL libraries, which appear in the same place
 * as standard Android (i.e. /system/vendor/lib/egl), and the DDK versions
 * are installed with all the other vendor libraries.
 */
# define EGL_BASEPATH "/system/vendor/lib/"
#elif defined(ANDROID)
/*
 * On Android, 32- and 64-bit libraries are kept separately under
 * /system/lib and /system/lib64. The path we choose here depends on the
 * architecture of the code that's including this file.
 */
# if __SIZEOF_POINTER__ == 8
#  define EGL_BASEPATH "/system/vendor/lib64/egl/"
# else
#  define EGL_BASEPATH "/system/vendor/lib/egl/"
# endif
#elif defined(SUPPORT_TIZEN_PLATFORM)
# define EGL_BASEPATH "/usr/lib/"
#else
/*
 * On Linux and other OSs, libraries are in the default linker search path, so there's no
 * need to specify a directory.
 */
# define EGL_BASEPATH ""
#endif

/* Work out the prefix (usually "lib") of the EGL libraries. */
#if defined(SUPPORT_NTO_LIB_REDIRECTION)
# define EGL_LIBPREFIX "libIMG"
#else
# define EGL_LIBPREFIX "lib"
#endif

/* Work out the suffix (.so, .dll, etc) of the EGL libraries. */
#if defined(_WIN32)
# define EGL_LIBSUFFIX ".dll"
#elif defined(INTEGRITY_OS)

# define EGL_LIBSUFFIX ".a"

#else
# define EGL_LIBSUFFIX ".so"
#endif

/* For OGLES1, the name of the library changes depending on whether we're
 * building 1.0 or 1.1.
 */
#if defined(SUPPORT_OPENGLES1_V1) || defined(SUPPORT_OPENGLES1_V1_ONLY)
# define OGLES1_BASENAME "GLESv1_CM" EGL_BASENAME_SUFFIX
#else
# define OGLES1_BASENAME "GLES_CM" EGL_BASENAME_SUFFIX
#endif

/* Work out the final names and paths of the EGL libraries. */
#define OGLES1LIBNAME EGL_BASEPATH EGL_LIBPREFIX OGLES1_BASENAME EGL_LIBSUFFIX
#define OGLES3LIBNAME EGL_BASEPATH EGL_LIBPREFIX "GLESv2" EGL_BASENAME_SUFFIX EGL_LIBSUFFIX

#if defined(LIB_IMG_OCL_NAME)
#define OCLLIBNAME LIB_IMG_OCL_NAME
#else
#define OCLLIBNAME "libPVROCL" EGL_LIBSUFFIX
#endif

#if defined(_WIN32)
# define OGLLIBNAME "opengl32.dll"
#else
# define OGLLIBNAME "libGL.so"
#endif
