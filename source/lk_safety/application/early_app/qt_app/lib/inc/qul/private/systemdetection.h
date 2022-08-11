/******************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Ultralite module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/
#ifndef QUL_SYSTEMDETECTION_H
#define QUL_SYSTEMDETECTION_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qul API. It exists purely as an
// implementation detail for purpose of auto-generated code.
// This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

/*
   The operating system, must be one of: (QUL_OS_x)

     DARWIN   - Any Darwin system (macOS, iOS, watchOS, tvOS)
     MACOS    - macOS
     IOS      - iOS
     WATCHOS  - watchOS
     TVOS     - tvOS
     WIN32    - Win32 (Windows 2000/XP/Vista/7 and Windows Server 2003/2008)
     WINRT    - WinRT (Windows Runtime)
     CYGWIN   - Cygwin
     SOLARIS  - Sun Solaris
     HPUX     - HP-UX
     LINUX    - Linux [has variants]
     FREEBSD  - FreeBSD [has variants]
     NETBSD   - NetBSD
     OPENBSD  - OpenBSD
     INTERIX  - Interix
     AIX      - AIX
     HURD     - GNU Hurd
     QNX      - QNX [has variants]
     QNX6     - QNX RTP 6.1
     LYNX     - LynxOS
     BSD4     - Any BSD 4.4 system
     UNIX     - Any UNIX BSD/SYSV system
     ANDROID  - Android platform
     HAIKU    - Haiku

   The following operating systems have variants:
     LINUX    - both QUL_OS_LINUX and QUL_OS_ANDROID are defined when building for Android
              - only QUL_OS_LINUX is defined if building for other Linux systems
     FREEBSD  - QUL_OS_FREEBSD is defined only when building for FreeBSD with a BSD userland
              - QUL_OS_FREEBSD_KERNEL is always defined on FreeBSD, even if the userland is from GNU
*/

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#include <TargetConditionals.h>
#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
#define QUL_OS_DARWIN
#define QUL_OS_BSD4
#ifdef __LP64__
#define QUL_OS_DARWIN64
#else
#define QUL_OS_DARWIN32
#endif
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#define QUL_PLATFORM_UIKIT
#if defined(TARGET_OS_WATCH) && TARGET_OS_WATCH
#define QUL_OS_WATCHOS
#elif defined(TARGET_OS_TV) && TARGET_OS_TV
#define QUL_OS_TVOS
#else
#// TARGET_OS_IOS is only available in newer SDKs,
#// so assume any other iOS-based platform is iOS for now
#define QUL_OS_IOS
#endif
#else
#// TARGET_OS_OSX is only available in newer SDKs,
#// so assume any non iOS-based platform is macOS for now
#define QUL_OS_MACOS
#endif
#else
#error "Qt has not been ported to this Apple platform - see http://www.qt.io/developers"
#endif
#elif defined(__ANDROID__) || defined(ANDROID)
#define QUL_OS_ANDROID
#define QUL_OS_LINUX
#elif defined(__CYGWIN__)
#define QUL_OS_CYGWIN
#elif !defined(SAG_COM) && (!defined(WINAPI_FAMILY) || WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP) \
    && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#define QUL_OS_WIN32
#define QUL_OS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#if defined(WINAPI_FAMILY)
#ifndef WINAPI_FAMILY_PC_APP
#define WINAPI_FAMILY_PC_APP WINAPI_FAMILY_APP
#endif
#if defined(WINAPI_FAMILY_PHONE_APP) && WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
#define QUL_OS_WINRT
#elif WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
#define QUL_OS_WINRT
#else
#define QUL_OS_WIN32
#endif
#else
#define QUL_OS_WIN32
#endif
#elif defined(__sun) || defined(sun)
#define QUL_OS_SOLARIS
#elif defined(hpux) || defined(__hpux)
#define QUL_OS_HPUX
#elif defined(__native_client__)
#define QUL_OS_NACL
#elif defined(__linux__) || defined(__linux)
#define QUL_OS_LINUX
#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
#ifndef __FreeBSD_kernel__
#define QUL_OS_FREEBSD
#endif
#define QUL_OS_FREEBSD_KERNEL
#define QUL_OS_BSD4
#elif defined(__NetBSD__)
#define QUL_OS_NETBSD
#define QUL_OS_BSD4
#elif defined(__OpenBSD__)
#define QUL_OS_OPENBSD
#define QUL_OS_BSD4
#elif defined(__INTERIX)
#define QUL_OS_INTERIX
#define QUL_OS_BSD4
#elif defined(_AIX)
#define QUL_OS_AIX
#elif defined(__Lynx__)
#define QUL_OS_LYNX
#elif defined(__GNU__)
#define QUL_OS_HURD
#elif defined(__QNXNTO__)
#define QUL_OS_QNX
#elif defined(__INTEGRITY)
#define QUL_OS_INTEGRITY
#elif defined(VXWORKS) /* there is no "real" VxWorks define - this has to be set in the mkspec! */
#define QUL_OS_VXWORKS
#elif defined(__HAIKU__)
#define QUL_OS_HAIKU
#elif defined(__MAKEDEPEND__)
#else
//#  error "Qt has not been ported to this OS - see http://www.qt-project.org/"
#endif

#if defined(QUL_OS_WIN32) || defined(QUL_OS_WIN64) || defined(QUL_OS_WINRT)
#define QUL_OS_WIN
#endif

#if defined(QUL_OS_WIN)
#undef QUL_OS_UNIX
#elif !defined(QUL_OS_UNIX)
#define QUL_OS_UNIX
#endif

// Compatibility synonyms
#ifdef QUL_OS_DARWIN
#define QUL_OS_MAC
#endif
#ifdef QUL_OS_DARWIN32
#define QUL_OS_MAC32
#endif
#ifdef QUL_OS_DARWIN64
#define QUL_OS_MAC64
#endif
#ifdef QUL_OS_MACOS
#define QUL_OS_MACX
#define QUL_OS_OSX
#endif

#ifdef QUL_OS_DARWIN
#include <Availability.h>
#include <AvailabilityMacros.h>
#
#ifdef QUL_OS_MACOS
#if !defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_6
#undef __MAC_OS_X_VERSION_MIN_REQUIRED
#define __MAC_OS_X_VERSION_MIN_REQUIRED __MAC_10_6
#endif
#if !defined(MAC_OS_X_VERSION_MIN_REQUIRED) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
#undef MAC_OS_X_VERSION_MIN_REQUIRED
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_6
#endif
#endif
#
#// Numerical checks are preferred to named checks, but to be safe
#// we define the missing version names in case Qt uses them.
#
#if !defined(__MAC_10_11)
#define __MAC_10_11 101100
#endif
#if !defined(__MAC_10_12)
#define __MAC_10_12 101200
#endif
#if !defined(__MAC_10_13)
#define __MAC_10_13 101300
#endif
#if !defined(__MAC_10_14)
#define __MAC_10_14 101400
#endif
#if !defined(MAC_OS_X_VERSION_10_11)
#define MAC_OS_X_VERSION_10_11 101100
#endif
#if !defined(MAC_OS_X_VERSION_10_12)
#define MAC_OS_X_VERSION_10_12 101200
#endif
#if !defined(MAC_OS_X_VERSION_10_13)
#define MAC_OS_X_VERSION_10_13 101300
#endif
#if !defined(MAC_OS_X_VERSION_10_14)
#define MAC_OS_X_VERSION_10_14 101400
#endif
#
#if !defined(__IPHONE_10_0)
#define __IPHONE_10_0 100000
#endif
#if !defined(__IPHONE_10_1)
#define __IPHONE_10_1 100100
#endif
#if !defined(__IPHONE_10_2)
#define __IPHONE_10_2 100200
#endif
#if !defined(__IPHONE_10_3)
#define __IPHONE_10_3 100300
#endif
#if !defined(__IPHONE_11_0)
#define __IPHONE_11_0 110000
#endif
#if !defined(__IPHONE_12_0)
#define __IPHONE_12_0 120000
#endif
#endif

#ifdef __LSB_VERSION__
#if __LSB_VERSION__ < 40
#error "This version of the Linux Standard Base is unsupported"
#endif
#ifndef QUL_LINUXBASE
#define QUL_LINUXBASE
#endif
#endif

#endif // QUL_SYSTEMDETECTION_H
