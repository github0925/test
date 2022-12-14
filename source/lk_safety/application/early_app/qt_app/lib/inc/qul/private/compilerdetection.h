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
#ifndef QUL_COMPILERDETECTION_H
#define QUL_COMPILERDETECTION_H

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

#include <cassert>

/*
   The compiler, must be one of: (QUL_CC_x)

     SYM      - Digital Mars C/C++ (used to be Symantec C++)
     MSVC     - Microsoft Visual C/C++, Intel C++ for Windows
     BOR      - Borland/Turbo C++
     WAT      - Watcom C++
     GNU      - GNU C++
     COMEAU   - Comeau C++
     EDG      - Edison Design Group C++
     OC       - CenterLine C++
     SUN      - Forte Developer, or Sun Studio C++
     MIPS     - MIPSpro C++
     DEC      - DEC C++
     HPACC    - HP aC++
     USLC     - SCO OUDK and UDK
     CDS      - Reliant C++
     KAI      - KAI C++
     INTEL    - Intel C++ for Linux, Intel C++ for Windows
     HIGHC    - MetaWare High C/C++
     PGI      - Portland Group C++
     GHS      - Green Hills Optimizing C++ Compilers
     RVCT     - ARM Realview Compiler Suite
     CLANG    - C++ front-end for the LLVM compiler


   Should be sorted most to least authoritative.
*/

/* Symantec C++ is now Digital Mars */
#if defined(__DMC__) || defined(__SC__)
#define QUL_CC_SYM
/* "explicit" semantics implemented in 8.1e but keyword recognized since 7.5 */
#if defined(__SC__) && __SC__ < 0x750
#error "Compiler not supported"
#endif

#elif defined(_MSC_VER)
#ifdef __clang__
#define QUL_CC_CLANG ((__clang_major__ * 100) + __clang_minor__)
#endif
#define QUL_CC_MSVC (_MSC_VER)
#define QUL_CC_MSVC_NET
#define QUL_OUTOFLINE_TEMPLATE inline
#define QUL_COMPILER_MANGLES_RETURN_TYPE
#define QUL_FUNC_INFO __FUNCSIG__
#define QUL_ALIGNOF(type) __alignof(type)
#define QUL_DECL_ALIGN(n) __declspec(align(n))
#define QUL_ASSUME_IMPL(expr) __assume(expr)
#define QUL_UNREACHABLE_IMPL() __assume(0)
#define QUL_NORETURN __declspec(noreturn)
#define QUL_DECL_DEPRECATED __declspec(deprecated)
#define QUL_DECL_EMPTY_BASES __declspec(empty_bases)
#ifndef QUL_CC_CLANG
#define QUL_DECL_DEPRECATED_X(text) __declspec(deprecated(text))
#endif
#define QUL_DECL_EXPORT __declspec(dllexport)
#define QUL_DECL_IMPORT __declspec(dllimport)
#define QUL_MAKE_UNCHECKED_ARRAY_ITERATOR(x) stdext::make_unchecked_array_iterator(x) // Since _MSC_VER >= 1800
#define QUL_MAKE_CHECKED_ARRAY_ITERATOR(x, N) \
    stdext::make_checked_array_iterator(x, size_t(N)) // Since _MSC_VER >= 1500
/* Intel C++ disguising as Visual C++: the `using' keyword avoids warnings */
#if defined(__INTEL_COMPILER)
#define QUL_DECL_VARIABLE_DEPRECATED
#define QUL_CC_INTEL __INTEL_COMPILER
#endif

/* only defined for MSVC since that's the only compiler that actually optimizes for this */
/* might get overridden further down when QUL_COMPILER_NOEXCEPT is detected */
#ifdef __cplusplus
#define QUL_DECL_NOTHROW throw()
#endif

#elif defined(__BORLANDC__) || defined(__TURBOC__)
#define QUL_CC_BOR
#define QUL_INLINE_TEMPLATE
#if __BORLANDC__ < 0x502
#error "Compiler not supported"
#endif

#elif defined(__WATCOMC__)
#define QUL_CC_WAT

/* ARM Realview Compiler Suite
   RVCT compiler also defines __EDG__ and __GNUC__ (if --gnu flag is given),
   so check for it before that */
#elif defined(__ARMCC__) || defined(__CC_ARM)
#define QUL_CC_RVCT
/* work-around for missing compiler intrinsics */
#define __is_empty(X) false
#define __is_pod(X) false
#define QUL_DECL_DEPRECATED __attribute__((__deprecated__))
#ifdef QUL_OS_LINUX
#define QUL_DECL_EXPORT __attribute__((visibility("default")))
#define QUL_DECL_IMPORT __attribute__((visibility("default")))
#define QUL_DECL_HIDDEN __attribute__((visibility("hidden")))
#else
#define QUL_DECL_EXPORT __declspec(dllexport)
#define QUL_DECL_IMPORT __declspec(dllimport)
#endif

#elif defined(__ICCARM__)
#define QUL_CC_IAR
#define QUL_FUNC_INFO __PRETTY_FUNCTION__
#define QUL_ALIGNOF(type) __ALIGNOF__(type)
#define QUL_DECL_ALIGN(n) __attribute__((aligned(n)))
#define QUL_DECL_UNUSED __attribute__((__unused__))
#define QUL_UNREACHABLE_IMPL() ;
#define QUL_PACKED __attribute__((packed, aligned(1)))
#define QUL_NOINLINE __attribute__((noinline))

#elif defined(__GNUC__)
#define QUL_CC_GNU (__GNUC__ * 100 + __GNUC_MINOR__)
#if defined(__MINGW32__)
#define QUL_CC_MINGW
#endif
#if defined(__INTEL_COMPILER)
/* Intel C++ also masquerades as GCC */
#define QUL_CC_INTEL (__INTEL_COMPILER)
#ifdef __clang__
/* Intel C++ masquerades as Clang masquerading as GCC */
#define QUL_CC_CLANG 305
#endif
#define QUL_ASSUME_IMPL(expr) __assume(expr)
#define QUL_UNREACHABLE_IMPL() __builtin_unreachable()
#if __INTEL_COMPILER >= 1300 && !defined(__APPLE__)
#define QUL_DECL_DEPRECATED_X(text) __attribute__((__deprecated__(text)))
#endif
#elif defined(__clang__)
/* Clang also masquerades as GCC */
#if defined(__apple_build_version__)
#/* http://en.wikipedia.org/wiki/Xcode#Toolchain_Versions */
#if __apple_build_version__ >= 8020041
#define QUL_CC_CLANG 309
#elif __apple_build_version__ >= 8000038
#define QUL_CC_CLANG 308
#elif __apple_build_version__ >= 7000053
#define QUL_CC_CLANG 306
#elif __apple_build_version__ >= 6000051
#define QUL_CC_CLANG 305
#elif __apple_build_version__ >= 5030038
#define QUL_CC_CLANG 304
#elif __apple_build_version__ >= 5000275
#define QUL_CC_CLANG 303
#elif __apple_build_version__ >= 4250024
#define QUL_CC_CLANG 302
#elif __apple_build_version__ >= 3180045
#define QUL_CC_CLANG 301
#elif __apple_build_version__ >= 2111001
#define QUL_CC_CLANG 300
#else
#error "Unknown Apple Clang version"
#endif
#else
#define QUL_CC_CLANG ((__clang_major__ * 100) + __clang_minor__)
#endif
#if __has_builtin(__builtin_assume)
#define QUL_ASSUME_IMPL(expr) __builtin_assume(expr)
#else
#define QUL_ASSUME_IMPL(expr) \
    if (expr) { \
    } else \
        __builtin_unreachable()
#endif
#define QUL_UNREACHABLE_IMPL() __builtin_unreachable()
#if !defined(__has_extension)
#/* Compatibility with older Clang versions */
#define __has_extension __has_feature
#endif
#if defined(__APPLE__)
/* Apple/clang specific features */
#define QUL_DECL_CF_RETURNS_RETAINED __attribute__((cf_returns_retained))
#ifdef __OBJC__
#define QUL_DECL_NS_RETURNS_AUTORELEASED __attribute__((ns_returns_autoreleased))
#endif
#endif
#else
/* Plain GCC */
#if QUL_CC_GNU >= 405
#define QUL_ASSUME_IMPL(expr) \
    if (expr) { \
    } else \
        __builtin_unreachable()
#define QUL_UNREACHABLE_IMPL() __builtin_unreachable()
#define QUL_DECL_DEPRECATED_X(text) __attribute__((__deprecated__(text)))
#endif
#endif

#ifdef QUL_OS_WIN
#define QUL_DECL_EXPORT __declspec(dllexport)
#define QUL_DECL_IMPORT __declspec(dllimport)
#elif defined(QUL_VISIBILITY_AVAILABLE)
#define QUL_DECL_EXPORT __attribute__((visibility("default")))
#define QUL_DECL_IMPORT __attribute__((visibility("default")))
#define QUL_DECL_HIDDEN __attribute__((visibility("hidden")))
#endif

#define QUL_FUNC_INFO __PRETTY_FUNCTION__
#define QUL_ALIGNOF(type) __alignof__(type)
#define QUL_TYPEOF(expr) __typeof__(expr)
#define QUL_DECL_DEPRECATED __attribute__((__deprecated__))
#define QUL_DECL_ALIGN(n) __attribute__((__aligned__(n)))
#define QUL_DECL_UNUSED __attribute__((__unused__))
#define QUL_LIKELY(expr) __builtin_expect(!!(expr), true)
#define QUL_UNLIKELY(expr) __builtin_expect(!!(expr), false)
#define QUL_NOINLINE __attribute__((noinline))
#define QUL_NORETURN __attribute__((__noreturn__))
#define QUL_REQUIRED_RESULT __attribute__((__warn_unused_result__))
#define QUL_DECL_PURE_FUNCTION __attribute__((pure))
#define QUL_DECL_CONST_FUNCTION __attribute__((const))
#define QUL_DECL_COLD_FUNCTION __attribute__((cold))
#if !defined(QUL_MOC_CPP)
#define QUL_PACKED __attribute__((__packed__))
#ifndef __ARM_EABI__
#define QUL_NO_ARM_EABI
#endif
#endif
#if QUL_CC_GNU >= 403 && !defined(QUL_CC_CLANG)
#define QUL_ALLOC_SIZE(x) __attribute__((alloc_size(x)))
#endif

/* IBM compiler versions are a bit messy. There are actually two products:
   the C product, and the C++ product. The C++ compiler is always packaged
   with the latest version of the C compiler. Version numbers do not always
   match. This little table (I'm not sure it's accurate) should be helpful:

   C++ product                C product

   C Set 3.1                  C Compiler 3.0
   ...                        ...
   C++ Compiler 3.6.6         C Compiler 4.3
   ...                        ...
   Visual Age C++ 4.0         ...
   ...                        ...
   Visual Age C++ 5.0         C Compiler 5.0
   ...                        ...
   Visual Age C++ 6.0         C Compiler 6.0

   Now:
   __xlC__    is the version of the C compiler in hexadecimal notation
              is only an approximation of the C++ compiler version
   __IBMCPP__ is the version of the C++ compiler in decimal notation
              but it is not defined on older compilers like C Set 3.1 */
#elif defined(__xlC__)
#define QUL_CC_XLC
#define QUL_FULL_TEMPLATE_INSTANTIATION
#if __xlC__ < 0x400
#error "Compiler not supported"
#elif __xlC__ >= 0x0600
#define QUL_ALIGNOF(type) __alignof__(type)
#define QUL_TYPEOF(expr) __typeof__(expr)
#define QUL_DECL_ALIGN(n) __attribute__((__aligned__(n)))
#define QUL_PACKED __attribute__((__packed__))
#endif

/* Older versions of DEC C++ do not define __EDG__ or __EDG - observed
   on DEC C++ V5.5-004. New versions do define  __EDG__ - observed on
   Compaq C++ V6.3-002.
   This compiler is different enough from other EDG compilers to handle
   it separately anyway. */
#elif defined(__DECCXX) || defined(__DECC)
#define QUL_CC_DEC
/* Compaq C++ V6 compilers are EDG-based but I'm not sure about older
   DEC C++ V5 compilers. */
#if defined(__EDG__)
#define QUL_CC_EDG
#endif
/* Compaq has disabled EDG's _BOOL macro and uses _BOOL_EXISTS instead
   - observed on Compaq C++ V6.3-002.
   In any case versions prior to Compaq C++ V6.0-005 do not have bool. */
#if !defined(_BOOL_EXISTS)
#error "Compiler not supported"
#endif
/* Spurious (?) error messages observed on Compaq C++ V6.5-014. */
/* Apply to all versions prior to Compaq C++ V6.0-000 - observed on
   DEC C++ V5.5-004. */
#if __DECCXX_VER < 60060000
#define QUL_BROKEN_TEMPLATE_SPECIALIZATION
#endif
/* avoid undefined symbol problems with out-of-line template members */
#define QUL_OUTOFLINE_TEMPLATE inline

/* The Portland Group C++ compiler is based on EDG and does define __EDG__
   but the C compiler does not */
#elif defined(__PGI)
#define QUL_CC_PGI
#if defined(__EDG__)
#define QUL_CC_EDG
#endif

/* Compilers with EDG front end are similar. To detect them we test:
   __EDG documented by SGI, observed on MIPSpro 7.3.1.1 and KAI C++ 4.0b
   __EDG__ documented in EDG online docs, observed on Compaq C++ V6.3-002
   and PGI C++ 5.2-4 */
#elif !defined(QUL_OS_HPUX) && (defined(__EDG) || defined(__EDG__))
#define QUL_CC_EDG
/* From the EDG documentation (does not seem to apply to Compaq C++ or GHS C):
   _BOOL
        Defined in C++ mode when bool is a keyword. The name of this
        predefined macro is specified by a configuration flag. _BOOL
        is the default.
   __BOOL_DEFINED
        Defined in Microsoft C++ mode when bool is a keyword. */
#if !defined(_BOOL) && !defined(__BOOL_DEFINED) && !defined(__ghs__)
#error "Compiler not supported"
#endif

/* The Comeau compiler is based on EDG and does define __EDG__ */
#if defined(__COMO__)
#define QUL_CC_COMEAU

/* The `using' keyword was introduced to avoid KAI C++ warnings
   but it's now causing KAI C++ errors instead. The standard is
   unclear about the use of this keyword, and in practice every
   compiler is using its own set of rules. Forget it. */
#elif defined(__KCC)
#define QUL_CC_KAI

/* Using the `using' keyword avoids Intel C++ for Linux warnings */
#elif defined(__INTEL_COMPILER)
#define QUL_CC_INTEL (__INTEL_COMPILER)

/* Uses CFront, make sure to read the manual how to tweak templates. */
#elif defined(__ghs__)
#define QUL_CC_GHS
#define QUL_DECL_DEPRECATED __attribute__((__deprecated__))
#define QUL_PACKED __attribute__((__packed__))
#define QUL_FUNC_INFO __PRETTY_FUNCTION__
#define QUL_TYPEOF(expr) __typeof__(expr)
#define QUL_ALIGNOF(type) __alignof__(type)
#define QUL_UNREACHABLE_IMPL()
#if defined(__cplusplus)
#define QUL_COMPILER_AUTO_TYPE
#define QUL_COMPILER_STATIC_ASSERT
#define QUL_COMPILER_RANGE_FOR
#if __GHS_VERSION_NUMBER >= 201505 && _HAS_CPP0X
#define QUL_COMPILER_ALIGNAS
#define QUL_COMPILER_ALIGNOF
#define QUL_COMPILER_ATOMICS
#define QUL_COMPILER_ATTRIBUTES
#define QUL_COMPILER_AUTO_FUNCTION
#define QUL_COMPILER_CLASS_ENUM
#define QUL_COMPILER_CONSTEXPR
#define QUL_COMPILER_DECLTYPE
#define QUL_COMPILER_DEFAULT_MEMBERS
#define QUL_COMPILER_DELETE_MEMBERS
#define QUL_COMPILER_DELEGATING_CONSTRUCTORS
#define QUL_COMPILER_EXPLICIT_CONVERSIONS
#define QUL_COMPILER_EXPLICIT_OVERRIDES
#define QUL_COMPILER_EXTERN_TEMPLATES
#define QUL_COMPILER_INHERITING_CONSTRUCTORS
#define QUL_COMPILER_INITIALIZER_LISTS
#define QUL_COMPILER_LAMBDA
#define QUL_COMPILER_NONSTATIC_MEMBER_INIT
#define QUL_COMPILER_NOEXCEPT
#define QUL_COMPILER_NULLPTR
#define QUL_COMPILER_RANGE_FOR
#define QUL_COMPILER_RAW_STRINGS
#define QUL_COMPILER_REF_QUALIFIERS
#define QUL_COMPILER_RVALUE_REFS
#define QUL_COMPILER_STATIC_ASSERT
#define QUL_COMPILER_TEMPLATE_ALIAS
#define QUL_COMPILER_THREAD_LOCAL
#define QUL_COMPILER_THREADSAFE_STATICS
#define QUL_COMPILER_UDL
#define QUL_COMPILER_UNICODE_STRINGS
#define QUL_COMPILER_UNIFORM_INIT
#define QUL_COMPILER_UNRESTRICTED_UNIONS
#define QUL_COMPILER_VARIADIC_MACROS
#define QUL_COMPILER_VARIADIC_TEMPLATES
#endif
#endif //__cplusplus

#elif defined(__DCC__)
#define QUL_CC_DIAB
#if !defined(__bool)
#error "Compiler not supported"
#endif

/* The UnixWare 7 UDK compiler is based on EDG and does define __EDG__ */
#elif defined(__USLC__) && defined(__SCO_VERSION__)
#define QUL_CC_USLC
/* The latest UDK 7.1.1b does not need this, but previous versions do */
#if !defined(__SCO_VERSION__) || (__SCO_VERSION__ < 302200010)
#define QUL_OUTOFLINE_TEMPLATE inline
#endif

/* Never tested! */
#elif defined(CENTERLINE_CLPP) || defined(OBJECTCENTER)
#define QUL_CC_OC

/* CDS++ defines __EDG__ although this is not documented in the Reliant
   documentation. It also follows conventions like _BOOL and this documented */
#elif defined(sinix)
#define QUL_CC_CDS

/* The MIPSpro compiler defines __EDG */
#elif defined(__sgi)
#define QUL_CC_MIPS
#define QUL_NO_TEMPLATE_FRIENDS
#if defined(_COMPILER_VERSION) && (_COMPILER_VERSION >= 740)
#define QUL_OUTOFLINE_TEMPLATE inline
#pragma set woff 3624, 3625, 3649 /* turn off some harmless warnings */
#endif
#endif

/* VxWorks' DIAB toolchain has an additional EDG type C++ compiler
   (see __DCC__ above). This one is for C mode files (__EDG is not defined) */
#elif defined(_DIAB_TOOL)
#define QUL_CC_DIAB
#define QUL_FUNC_INFO __PRETTY_FUNCTION__

/* Never tested! */
#elif defined(__HIGHC__)
#define QUL_CC_HIGHC

#elif defined(__SUNPRO_CC) || defined(__SUNPRO_C)
#define QUL_CC_SUN
#define QUL_COMPILER_MANGLES_RETURN_TYPE
/* 5.0 compiler or better
    'bool' is enabled by default but can be disabled using -features=nobool
    in which case _BOOL is not defined
        this is the default in 4.2 compatibility mode triggered by -compat=4 */
#if __SUNPRO_CC >= 0x500
#define QUL_NO_TEMPLATE_TEMPLATE_PARAMETERS
/* see http://developers.sun.com/sunstudio/support/Ccompare.html */
#if __SUNPRO_CC >= 0x590
#define QUL_ALIGNOF(type) __alignof__(type)
#define QUL_TYPEOF(expr) __typeof__(expr)
#define QUL_DECL_ALIGN(n) __attribute__((__aligned__(n)))
#endif
#if __SUNPRO_CC >= 0x550
#define QUL_DECL_EXPORT __global
#endif
#if __SUNPRO_CC < 0x5a0
#define QUL_NO_TEMPLATE_FRIENDS
#endif
#if !defined(_BOOL)
#error "Compiler not supported"
#endif
/* 4.2 compiler or older */
#else
#error "Compiler not supported"
#endif

/* CDS++ does not seem to define __EDG__ or __EDG according to Reliant
   documentation but nevertheless uses EDG conventions like _BOOL */
#elif defined(sinix)
#define QUL_CC_EDG
#define QUL_CC_CDS
#if !defined(_BOOL)
#error "Compiler not supported"
#endif
#define QUL_BROKEN_TEMPLATE_SPECIALIZATION

#elif defined(QUL_OS_HPUX)
/* __HP_aCC was not defined in first aCC releases */
#if defined(__HP_aCC) || __cplusplus >= 199707L
#define QUL_NO_TEMPLATE_FRIENDS
#define QUL_CC_HPACC
#define QUL_FUNC_INFO __PRETTY_FUNCTION__
#if __HP_aCC - 0 < 060000
#define QUL_NO_TEMPLATE_TEMPLATE_PARAMETERS
#define QUL_DECL_EXPORT __declspec(dllexport)
#define QUL_DECL_IMPORT __declspec(dllimport)
#endif
#if __HP_aCC - 0 >= 061200
#define QUL_DECL_ALIGN(n) __attribute__((aligned(n)))
#endif
#if __HP_aCC - 0 >= 062000
#define QUL_DECL_EXPORT __attribute__((visibility("default")))
#define QUL_DECL_HIDDEN __attribute__((visibility("hidden")))
#define QUL_DECL_IMPORT QUL_DECL_EXPORT
#endif
#else
#error "Compiler not supported"
#endif

#else
#error "Qt has not been tested with this compiler - see http://www.qt-project.org/"
#endif

/*
 * C++11 support
 *
 *  Paper           Macro                               SD-6 macro
 *  N2341           QUL_COMPILER_ALIGNAS
 *  N2341           QUL_COMPILER_ALIGNOF
 *  N2427           QUL_COMPILER_ATOMICS
 *  N2761           QUL_COMPILER_ATTRIBUTES               __cpp_attributes = 200809
 *  N2541           QUL_COMPILER_AUTO_FUNCTION
 *  N1984 N2546     QUL_COMPILER_AUTO_TYPE
 *  N2437           QUL_COMPILER_CLASS_ENUM
 *  N2235           QUL_COMPILER_CONSTEXPR                __cpp_constexpr = 200704
 *  N2343 N3276     QUL_COMPILER_DECLTYPE                 __cpp_decltype = 200707
 *  N2346           QUL_COMPILER_DEFAULT_MEMBERS
 *  N2346           QUL_COMPILER_DELETE_MEMBERS
 *  N1986           QUL_COMPILER_DELEGATING_CONSTRUCTORS
 *  N2437           QUL_COMPILER_EXPLICIT_CONVERSIONS
 *  N3206 N3272     QUL_COMPILER_EXPLICIT_OVERRIDES
 *  N1987           QUL_COMPILER_EXTERN_TEMPLATES
 *  N2540           QUL_COMPILER_INHERITING_CONSTRUCTORS
 *  N2672           QUL_COMPILER_INITIALIZER_LISTS
 *  N2658 N2927     QUL_COMPILER_LAMBDA                   __cpp_lambdas = 200907
 *  N2756           QUL_COMPILER_NONSTATIC_MEMBER_INIT
 *  N2855 N3050     QUL_COMPILER_NOEXCEPT
 *  N2431           QUL_COMPILER_NULLPTR
 *  N2930           QUL_COMPILER_RANGE_FOR
 *  N2442           QUL_COMPILER_RAW_STRINGS              __cpp_raw_strings = 200710
 *  N2439           QUL_COMPILER_REF_QUALIFIERS
 *  N2118 N2844 N3053 QUL_COMPILER_RVALUE_REFS            __cpp_rvalue_references = 200610
 *  N1720           QUL_COMPILER_STATIC_ASSERT            __cpp_static_assert = 200410
 *  N2258           QUL_COMPILER_TEMPLATE_ALIAS
 *  N2659           QUL_COMPILER_THREAD_LOCAL
 *  N2660           QUL_COMPILER_THREADSAFE_STATICS
 *  N2765           QUL_COMPILER_UDL                      __cpp_user_defined_literals = 200809
 *  N2442           QUL_COMPILER_UNICODE_STRINGS          __cpp_unicode_literals = 200710
 *  N2640           QUL_COMPILER_UNIFORM_INIT
 *  N2544           QUL_COMPILER_UNRESTRICTED_UNIONS
 *  N1653           QUL_COMPILER_VARIADIC_MACROS
 *  N2242 N2555     QUL_COMPILER_VARIADIC_TEMPLATES       __cpp_variadic_templates = 200704
 *
 * For any future version of the C++ standard, we use only the SD-6 macro.
 * For full listing, see
 *  http://isocpp.org/std/standing-documents/sd-6-sg10-feature-test-recommendations
 *
 * C++ extensions:
 *  QUL_COMPILER_RESTRICTED_VLA       variable-length arrays, prior to __cpp_runtime_arrays
 */

#ifdef __cplusplus
#if __cplusplus < 199711L && !defined(QUL_CC_MSVC)
#error Qt requires a C++98 compiler and yours does not seem to be that.
#endif
#endif

#if defined(QUL_CC_INTEL) && !defined(QUL_CC_MSVC)
#define QUL_COMPILER_RESTRICTED_VLA
#define QUL_COMPILER_VARIADIC_MACROS // C++11 feature supported as an extension in other modes, too
#define QUL_COMPILER_THREADSAFE_STATICS
#if __INTEL_COMPILER < 1200
#define QUL_NO_TEMPLATE_FRIENDS
#endif
#if __INTEL_COMPILER >= 1310 && !defined(_WIN32)
//    ICC supports C++14 binary literals in C, C++98, and C++11 modes
//    at least since 13.1, but I can't test further back
#define QUL_COMPILER_BINARY_LITERALS
#endif
#if __cplusplus >= 201103L || defined(__INTEL_CXX11_MODE__)
#if __INTEL_COMPILER >= 1200
#define QUL_COMPILER_AUTO_TYPE
#define QUL_COMPILER_CLASS_ENUM
#define QUL_COMPILER_DECLTYPE
#define QUL_COMPILER_DEFAULT_MEMBERS
#define QUL_COMPILER_DELETE_MEMBERS
#define QUL_COMPILER_EXTERN_TEMPLATES
#define QUL_COMPILER_LAMBDA
#define QUL_COMPILER_RVALUE_REFS
#define QUL_COMPILER_STATIC_ASSERT
#define QUL_COMPILER_VARIADIC_MACROS
#endif
#if __INTEL_COMPILER >= 1210
#define QUL_COMPILER_ATTRIBUTES
#define QUL_COMPILER_AUTO_FUNCTION
#define QUL_COMPILER_NULLPTR
#define QUL_COMPILER_TEMPLATE_ALIAS
#ifndef _CHAR16T // MSVC headers
#define QUL_COMPILER_UNICODE_STRINGS
#endif
#define QUL_COMPILER_VARIADIC_TEMPLATES
#endif
#if __INTEL_COMPILER >= 1300
#define QUL_COMPILER_ATOMICS
//       constexpr support is only partial
//#      define QUL_COMPILER_CONSTEXPR
#define QUL_COMPILER_INITIALIZER_LISTS
#define QUL_COMPILER_UNIFORM_INIT
#define QUL_COMPILER_NOEXCEPT
#endif
#if __INTEL_COMPILER >= 1400
//       causes issues with QArrayData and QtPrivate::RefCount - Intel issue ID 6000056211, bug DPD200534796
//#      define QUL_COMPILER_CONSTEXPR
#define QUL_COMPILER_DELEGATING_CONSTRUCTORS
#define QUL_COMPILER_EXPLICIT_CONVERSIONS
#define QUL_COMPILER_EXPLICIT_OVERRIDES
#define QUL_COMPILER_NONSTATIC_MEMBER_INIT
#define QUL_COMPILER_RANGE_FOR
#define QUL_COMPILER_RAW_STRINGS
#define QUL_COMPILER_REF_QUALIFIERS
#define QUL_COMPILER_UNICODE_STRINGS
#define QUL_COMPILER_UNRESTRICTED_UNIONS
#endif
#if __INTEL_COMPILER >= 1500
#if __INTEL_COMPILER * 100 + __INTEL_COMPILER_UPDATE >= 150001
//       the bug mentioned above is fixed in 15.0.1
#define QUL_COMPILER_CONSTEXPR
#endif
#define QUL_COMPILER_ALIGNAS
#define QUL_COMPILER_ALIGNOF
#define QUL_COMPILER_INHERITING_CONSTRUCTORS
#define QUL_COMPILER_THREAD_LOCAL
#define QUL_COMPILER_UDL
#endif
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ > 199901L
//   C11 features supported. Only tested with ICC 17 and up.
#define QUL_COMPILER_STATIC_ASSERT
#if __has_include(<threads.h>)
#define QUL_COMPILER_THREAD_LOCAL
#endif
#endif
#endif

#if defined(QUL_CC_CLANG) && !defined(QUL_CC_INTEL) && !defined(QUL_CC_MSVC)
/* General C++ features */
#define QUL_COMPILER_RESTRICTED_VLA
#define QUL_COMPILER_THREADSAFE_STATICS
#if __has_feature(attribute_deprecated_with_message)
#define QUL_DECL_DEPRECATED_X(text) __attribute__((__deprecated__(text)))
#endif

// Clang supports binary literals in C, C++98 and C++11 modes
// It's been supported "since the dawn of time itself" (cf. commit 179883)
#if __has_extension(cxx_binary_literals)
#define QUL_COMPILER_BINARY_LITERALS
#endif

// Variadic macros are supported for gnu++98, c++11, c99 ... since 2.9
#if QUL_CC_CLANG >= 209
#if !defined(__STRICT_ANSI__) || defined(__GXX_EXPERIMENTAL_CXX0X__) \
    || (defined(__cplusplus) && (__cplusplus >= 201103L)) \
    || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#define QUL_COMPILER_VARIADIC_MACROS
#endif
#endif

/* C++11 features, see http://clang.llvm.org/cxx_status.html */
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
/* Detect C++ features using __has_feature(), see http://clang.llvm.org/docs/LanguageExtensions.html#cxx11 */
#if __has_feature(cxx_alignas)
#define QUL_COMPILER_ALIGNAS
#define QUL_COMPILER_ALIGNOF
#endif
#if __has_feature(cxx_atomic) && __has_include(<atomic>)
#define QUL_COMPILER_ATOMICS
#endif
#if __has_feature(cxx_attributes)
#define QUL_COMPILER_ATTRIBUTES
#endif
#if __has_feature(cxx_auto_type)
#define QUL_COMPILER_AUTO_FUNCTION
#define QUL_COMPILER_AUTO_TYPE
#endif
#if __has_feature(cxx_strong_enums)
#define QUL_COMPILER_CLASS_ENUM
#endif
#if __has_feature(cxx_constexpr) && QUL_CC_CLANG > 302 /* CLANG 3.2 has bad/partial support */
#define QUL_COMPILER_CONSTEXPR
#endif
#if __has_feature(cxx_decltype) /* && __has_feature(cxx_decltype_incomplete_return_types) */
#define QUL_COMPILER_DECLTYPE
#endif
#if __has_feature(cxx_defaulted_functions)
#define QUL_COMPILER_DEFAULT_MEMBERS
#endif
#if __has_feature(cxx_deleted_functions)
#define QUL_COMPILER_DELETE_MEMBERS
#endif
#if __has_feature(cxx_delegating_constructors)
#define QUL_COMPILER_DELEGATING_CONSTRUCTORS
#endif
#if __has_feature(cxx_explicit_conversions)
#define QUL_COMPILER_EXPLICIT_CONVERSIONS
#endif
#if __has_feature(cxx_override_control)
#define QUL_COMPILER_EXPLICIT_OVERRIDES
#endif
#if __has_feature(cxx_inheriting_constructors)
#define QUL_COMPILER_INHERITING_CONSTRUCTORS
#endif
#if __has_feature(cxx_generalized_initializers)
#define QUL_COMPILER_INITIALIZER_LISTS
#define QUL_COMPILER_UNIFORM_INIT /* both covered by this feature macro, according to docs */
#endif
#if __has_feature(cxx_lambdas)
#define QUL_COMPILER_LAMBDA
#endif
#if __has_feature(cxx_noexcept)
#define QUL_COMPILER_NOEXCEPT
#endif
#if __has_feature(cxx_nonstatic_member_init)
#define QUL_COMPILER_NONSTATIC_MEMBER_INIT
#endif
#if __has_feature(cxx_NULL)
#define QUL_COMPILER_NULLPTR
#endif
#if __has_feature(cxx_range_for)
#define QUL_COMPILER_RANGE_FOR
#endif
#if __has_feature(cxx_raw_string_literals)
#define QUL_COMPILER_RAW_STRINGS
#endif
#if __has_feature(cxx_reference_qualified_functions)
#define QUL_COMPILER_REF_QUALIFIERS
#endif
#if __has_feature(cxx_rvalue_references)
#define QUL_COMPILER_RVALUE_REFS
#endif
#if __has_feature(cxx_static_assert)
#define QUL_COMPILER_STATIC_ASSERT
#endif
#if __has_feature(cxx_alias_templates)
#define QUL_COMPILER_TEMPLATE_ALIAS
#endif
#if __has_feature(cxx_thread_local)
#if !defined(__FreeBSD__) /* FreeBSD clang fails on __cxa_thread_atexit */
#define QUL_COMPILER_THREAD_LOCAL
#endif
#endif
#if __has_feature(cxx_user_literals)
#define QUL_COMPILER_UDL
#endif
#if __has_feature(cxx_unicode_literals)
#define QUL_COMPILER_UNICODE_STRINGS
#endif
#if __has_feature(cxx_unrestricted_unions)
#define QUL_COMPILER_UNRESTRICTED_UNIONS
#endif
#if __has_feature(cxx_variadic_templates)
#define QUL_COMPILER_VARIADIC_TEMPLATES
#endif
/* Features that have no __has_feature() check */
#if QUL_CC_CLANG >= 209 /* since clang 2.9 */
#define QUL_COMPILER_EXTERN_TEMPLATES
#endif
#endif

/* C++1y features, deprecated macros. Do not update this list. */
#if __cplusplus > 201103L
//#    if __has_feature(cxx_binary_literals)
//#      define QUL_COMPILER_BINARY_LITERALS  // see above
//#    endif
#if __has_feature(cxx_generic_lambda)
#define QUL_COMPILER_GENERIC_LAMBDA
#endif
#if __has_feature(cxx_init_capture)
#define QUL_COMPILER_LAMBDA_CAPTURES
#endif
#if __has_feature(cxx_relaxed_constexpr)
#define QUL_COMPILER_RELAXED_CONSTEXPR_FUNCTIONS
#endif
#if __has_feature(cxx_decltype_auto) && __has_feature(cxx_return_type_deduction)
#define QUL_COMPILER_RETURN_TYPE_DEDUCTION
#endif
#if __has_feature(cxx_variable_templates)
#define QUL_COMPILER_VARIABLE_TEMPLATES
#endif
#if __has_feature(cxx_runtime_array)
#define QUL_COMPILER_VLA
#endif
#endif

#if defined(__STDC_VERSION__)
#if __has_feature(c_static_assert)
#define QUL_COMPILER_STATIC_ASSERT
#endif
#if __has_feature(c_thread_local) && __has_include(<threads.h>)
#if !defined(__FreeBSD__) /* FreeBSD clang fails on __cxa_thread_atexit */
#define QUL_COMPILER_THREAD_LOCAL
#endif
#endif
#endif

#if defined(__has_warning)
#if __has_warning("-Wunused-private-field")
#define QUL_DECL_UNUSED_MEMBER QUL_DECL_UNUSED
#endif
#endif

#endif // QUL_CC_CLANG

#if defined(QUL_CC_GNU) && !defined(QUL_CC_INTEL) && !defined(QUL_CC_CLANG)
#define QUL_COMPILER_RESTRICTED_VLA
#define QUL_COMPILER_THREADSAFE_STATICS
#if QUL_CC_GNU >= 403
//   GCC supports binary literals in C, C++98 and C++11 modes
#define QUL_COMPILER_BINARY_LITERALS
#endif
#if !defined(__STRICT_ANSI__) || defined(__GXX_EXPERIMENTAL_CXX0X__) \
    || (defined(__cplusplus) && (__cplusplus >= 201103L)) \
    || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
// Variadic macros are supported for gnu++98, c++11, C99 ... since forever (gcc 2.97)
#define QUL_COMPILER_VARIADIC_MACROS
#endif
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#if QUL_CC_GNU >= 403
/* C++11 features supported in GCC 4.3: */
#define QUL_COMPILER_DECLTYPE
#define QUL_COMPILER_RVALUE_REFS
#define QUL_COMPILER_STATIC_ASSERT
#endif
#if QUL_CC_GNU >= 404
/* C++11 features supported in GCC 4.4: */
#define QUL_COMPILER_AUTO_FUNCTION
#define QUL_COMPILER_AUTO_TYPE
#define QUL_COMPILER_EXTERN_TEMPLATES
#define QUL_COMPILER_UNIFORM_INIT
#define QUL_COMPILER_UNICODE_STRINGS
#define QUL_COMPILER_VARIADIC_TEMPLATES
#endif
#if QUL_CC_GNU >= 405
/* C++11 features supported in GCC 4.5: */
#define QUL_COMPILER_EXPLICIT_CONVERSIONS
/* GCC 4.4 implements initializer_list but does not define typedefs required
        * by the standard. */
#define QUL_COMPILER_INITIALIZER_LISTS
#define QUL_COMPILER_LAMBDA
#define QUL_COMPILER_RAW_STRINGS
#define QUL_COMPILER_CLASS_ENUM
#endif
#if QUL_CC_GNU >= 406
/* Pre-4.6 compilers implement a non-final snapshot of N2346, hence default and delete
        * functions are supported only if they are public. Starting from 4.6, GCC handles
        * final version - the access modifier is not relevant. */
#define QUL_COMPILER_DEFAULT_MEMBERS
#define QUL_COMPILER_DELETE_MEMBERS
/* C++11 features supported in GCC 4.6: */
#define QUL_COMPILER_CONSTEXPR
#define QUL_COMPILER_NULLPTR
#define QUL_COMPILER_UNRESTRICTED_UNIONS
#define QUL_COMPILER_RANGE_FOR
#endif
#if QUL_CC_GNU >= 407
/* GCC 4.4 implemented <atomic> and std::atomic using its old intrinsics.
        * However, the implementation is incomplete for most platforms until GCC 4.7:
        * instead, std::atomic would use an external lock. Since we need an std::atomic
        * that is behavior-compatible with QBasicAtomic, we only enable it here */
#define QUL_COMPILER_ATOMICS
/* GCC 4.6.x has problems dealing with noexcept expressions,
        * so turn the feature on for 4.7 and above, only */
#define QUL_COMPILER_NOEXCEPT
/* C++11 features supported in GCC 4.7: */
#define QUL_COMPILER_NONSTATIC_MEMBER_INIT
#define QUL_COMPILER_DELEGATING_CONSTRUCTORS
#define QUL_COMPILER_EXPLICIT_OVERRIDES
#define QUL_COMPILER_TEMPLATE_ALIAS
#define QUL_COMPILER_UDL
#endif
#if QUL_CC_GNU >= 408
#define QUL_COMPILER_ATTRIBUTES
#define QUL_COMPILER_ALIGNAS
#define QUL_COMPILER_ALIGNOF
#define QUL_COMPILER_INHERITING_CONSTRUCTORS
#define QUL_COMPILER_THREAD_LOCAL
#if QUL_CC_GNU > 408 || __GNUC_PATCHLEVEL__ >= 1
#define QUL_COMPILER_REF_QUALIFIERS
#endif
#endif
/* C++11 features are complete as of GCC 4.8.1 */
#endif
#if __cplusplus > 201103L
#if QUL_CC_GNU >= 409
/* C++1y features in GCC 4.9 - deprecated, do not update this list */
//#    define QUL_COMPILER_BINARY_LITERALS   // already supported since GCC 4.3 as an extension
#define QUL_COMPILER_LAMBDA_CAPTURES
#define QUL_COMPILER_RETURN_TYPE_DEDUCTION
#endif
#endif
#if defined(__STDC_VERSION__) && __STDC_VERSION__ > 199901L
#if QUL_CC_GNU >= 407
/* C11 features supported in GCC 4.7: */
#define QUL_COMPILER_STATIC_ASSERT
#endif
#if QUL_CC_GNU >= 409 && defined(__has_include)
/* C11 features supported in GCC 4.9: */
#if __has_include(<threads.h>)
#define QUL_COMPILER_THREAD_LOCAL
#endif
#endif
#endif
#endif

#if defined(QUL_CC_MSVC)
#if defined(__cplusplus)
/* C++11 features supported in VC8 = VC2005: */
#define QUL_COMPILER_VARIADIC_MACROS

/* 2005 supports the override and final contextual keywords, in
        the same positions as the C++11 variants, but 'final' is
        called 'sealed' instead:
        http://msdn.microsoft.com/en-us/library/0w2w91tf%28v=vs.80%29.aspx
        The behavior is slightly different in C++/CLI, which requires the
        "virtual" keyword to be present too, so don't define for that.
        So don't define QUL_COMPILER_EXPLICIT_OVERRIDES (since it's not
        the same as the C++11 version), but define the QUL_DECL_* flags
        accordingly. */
/* C++11 features supported in VC10 = VC2010: */
#define QUL_COMPILER_AUTO_FUNCTION
#define QUL_COMPILER_AUTO_TYPE
#define QUL_COMPILER_DECLTYPE
#define QUL_COMPILER_EXTERN_TEMPLATES
#define QUL_COMPILER_LAMBDA
#define QUL_COMPILER_NULLPTR
#define QUL_COMPILER_RVALUE_REFS
#define QUL_COMPILER_STATIC_ASSERT
/* C++11 features supported in VC11 = VC2012: */
#define QUL_COMPILER_EXPLICIT_OVERRIDES /* ...and use std C++11 now   */
#define QUL_COMPILER_CLASS_ENUM
#define QUL_COMPILER_ATOMICS
/* C++11 features in VC12 = VC2013 */
#define QUL_COMPILER_DELETE_MEMBERS
#define QUL_COMPILER_DELEGATING_CONSTRUCTORS
#define QUL_COMPILER_EXPLICIT_CONVERSIONS
#define QUL_COMPILER_NONSTATIC_MEMBER_INIT
#define QUL_COMPILER_RAW_STRINGS
#define QUL_COMPILER_TEMPLATE_ALIAS
#define QUL_COMPILER_VARIADIC_TEMPLATES
#define QUL_COMPILER_INITIALIZER_LISTS // VC 12 SP 2 RC
                                       /* C++11 features in VC14 = VC2015 */
#define QUL_COMPILER_DEFAULT_MEMBERS
#define QUL_COMPILER_ALIGNAS
#define QUL_COMPILER_ALIGNOF
#define QUL_COMPILER_INHERITING_CONSTRUCTORS
#define QUL_COMPILER_NOEXCEPT
#define QUL_COMPILER_RANGE_FOR
#define QUL_COMPILER_REF_QUALIFIERS
#define QUL_COMPILER_THREAD_LOCAL
#define QUL_COMPILER_UDL
#define QUL_COMPILER_UNICODE_STRINGS
#define QUL_COMPILER_UNRESTRICTED_UNIONS
#if _MSC_FULL_VER >= 190023419
#define QUL_COMPILER_ATTRIBUTES
// Almost working, see https://connect.microsoft.com/VisualStudio/feedback/details/2011648
//#      define QUL_COMPILER_CONSTEXPR
#define QUL_COMPILER_THREADSAFE_STATICS
#define QUL_COMPILER_UNIFORM_INIT
#endif
#if _MSC_VER >= 1910
#define QUL_COMPILER_CONSTEXPR
#endif
#endif /* __cplusplus */
#endif /* QUL_CC_MSVC */

#ifdef QUL_COMPILER_UNICODE_STRINGS
#define QUL_STDLIB_UNICODE_STRINGS
#endif

#ifdef __cplusplus
#include <utility>
#if defined(QUL_OS_QNX)
// By default, QNX 7.0 uses libc++ (from LLVM) and
// QNX 6.X uses Dinkumware's libcpp. In all versions,
// it is also possible to use GNU libstdc++.

// For Dinkumware, some features must be disabled
// (mostly because of library problems).
// Dinkumware is assumed when __GLIBCXX__ (GNU libstdc++)
// and _LIBCPP_VERSION (LLVM libc++) are both absent.
#if !defined(__GLIBCXX__) && !defined(_LIBCPP_VERSION)

// Older versions of libcpp (QNX 650) do not support C++11 features
// _HAS_* macros are set to 1 by toolchains that actually include
// Dinkum C++11 libcpp.

#if !defined(_HAS_CPP0X) || !_HAS_CPP0X
// Disable C++11 features that depend on library support
#undef QUL_COMPILER_INITIALIZER_LISTS
#undef QUL_COMPILER_RVALUE_REFS
#undef QUL_COMPILER_REF_QUALIFIERS
#undef QUL_COMPILER_NOEXCEPT
// Disable C++11 library features:
#undef QUL_STDLIB_UNICODE_STRINGS
#endif // !_HAS_CPP0X
#if !defined(_HAS_NULLPTR_T) || !_HAS_NULLPTR_T
#undef QUL_COMPILER_NULLPTR
#endif //!_HAS_NULLPTR_T
#if !defined(_HAS_CONSTEXPR) || !_HAS_CONSTEXPR
// The libcpp is missing constexpr keywords on important functions like std::numeric_limits<>::min()
// Disable constexpr support on QNX even if the compiler supports it
#undef QUL_COMPILER_CONSTEXPR
#endif // !_HAS_CONSTEXPR
#endif // !__GLIBCXX__ && !_LIBCPP_VERSION
#endif // QUL_OS_QNX
#if (defined(QUL_CC_CLANG) || defined(QUL_CC_INTEL)) && defined(QUL_OS_MAC) && defined(__GNUC_LIBSTD__) \
    && ((__GNUC_LIBSTD__ - 0) * 100 + __GNUC_LIBSTD_MINOR__ - 0 <= 402)
// Apple has not updated libstdc++ since 2007, which means it does not have
// <initializer_list> or std::move. Let's disable these features
#undef QUL_COMPILER_INITIALIZER_LISTS
#undef QUL_COMPILER_RVALUE_REFS
#undef QUL_COMPILER_REF_QUALIFIERS
// Also disable <atomic>, since it's clearly not there
#undef QUL_COMPILER_ATOMICS
#endif
#if defined(QUL_CC_CLANG) && defined(QUL_CC_INTEL) && QUL_CC_INTEL >= 1500
// ICC 15.x and 16.0 have their own implementation of std::atomic, which is activated when in Clang mode
// (probably because libc++'s <atomic> on OS X failed to compile), but they're missing some
// critical definitions. (Reported as Intel Issue ID 6000117277)
#define __USE_CONSTEXPR 1
#define __USE_NOEXCEPT 1
#endif
#if defined(QUL_COMPILER_THREADSAFE_STATICS) && defined(QUL_OS_MAC)
// Apple's low-level implementation of the C++ support library
// (libc++abi.dylib, shared between libstdc++ and libc++) has deadlocks. The
// C++11 standard requires the deadlocks to be removed, so this will eventually
// be fixed; for now, let's disable this.
#undef QUL_COMPILER_THREADSAFE_STATICS
#endif
#endif

/*
 * SG10's SD-6 feature detection and some useful extensions from Clang and GCC
 * https://isocpp.org/std/standing-documents/sd-6-sg10-feature-test-recommendations
 * http://clang.llvm.org/docs/LanguageExtensions.html#feature-checking-macros
 */
#ifdef __has_builtin
#define QUL_HAS_BUILTIN(x) __has_builtin(x)
#else
#define QUL_HAS_BUILTIN(x) 0
#endif
#ifdef __has_attribute
#define QUL_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define QUL_HAS_ATTRIBUTE(x) 0
#endif
#ifdef __has_cpp_attribute
#define QUL_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define QUL_HAS_CPP_ATTRIBUTE(x) 0
#endif
#ifdef __has_include
#define QUL_HAS_INCLUDE(x)             __has_include(x)
#else
#define QUL_HAS_INCLUDE(x) 0
#endif
#ifdef __has_include_next
#define QUL_HAS_INCLUDE_NEXT(x)        __has_include_next(x)
#else
#define QUL_HAS_INCLUDE_NEXT(x) 0
#endif

/*
 * C++11 keywords and expressions
 */
#ifdef QUL_COMPILER_NULLPTR
#define QUL_NULLPTR NULL
#else
#define QUL_NULLPTR NULL
#endif

#ifdef QUL_COMPILER_DEFAULT_MEMBERS
#define QUL_DECL_EQUL_DEFAULT = default
#else
#define QUL_DECL_EQUL_DEFAULT
#endif

#ifdef QUL_COMPILER_DELETE_MEMBERS
#define QUL_DECL_EQUL_DELETE = delete
#else
#define QUL_DECL_EQUL_DELETE
#endif

// Don't break code that is already using QUL_COMPILER_DEFAULT_DELETE_MEMBERS
#if defined(QUL_COMPILER_DEFAULT_MEMBERS) && defined(QUL_COMPILER_DELETE_MEMBERS)
#define QUL_COMPILER_DEFAULT_DELETE_MEMBERS
#endif

#if defined QUL_COMPILER_CONSTEXPR
#if defined(__cpp_constexpr) && __cpp_constexpr - 0 >= 201304
#define QUL_DECL_CONSTEXPR constexpr
#define QUL_DECL_RELAXED_CONSTEXPR constexpr
#define QUL_CONSTEXPR constexpr
#define QUL_RELAXED_CONSTEXPR constexpr
#else
#define QUL_DECL_CONSTEXPR constexpr
#define QUL_DECL_RELAXED_CONSTEXPR
#define QUL_CONSTEXPR constexpr
#define QUL_RELAXED_CONSTEXPR const
#endif
#else
#define QUL_DECL_CONSTEXPR
#define QUL_DECL_RELAXED_CONSTEXPR
#define QUL_CONSTEXPR const
#define QUL_RELAXED_CONSTEXPR const
#endif

#ifdef QUL_COMPILER_EXPLICIT_OVERRIDES
#define QUL_DECL_OVERRIDE override
#define QUL_DECL_FINAL final
#else
#ifndef QUL_DECL_OVERRIDE
#define QUL_DECL_OVERRIDE
#endif
#ifndef QUL_DECL_FINAL
#define QUL_DECL_FINAL
#endif
#endif

#ifdef QUL_COMPILER_NOEXCEPT
#define QUL_DECL_NOEXCEPT noexcept
#define QUL_DECL_NOEXCEPT_EXPR(x) noexcept(x)
#ifdef QUL_DECL_NOTHROW
#undef QUL_DECL_NOTHROW /* override with C++11 noexcept if available */
#endif
#else
#define QUL_DECL_NOEXCEPT
#define QUL_DECL_NOEXCEPT_EXPR(x)
#endif
#ifndef QUL_DECL_NOTHROW
#define QUL_DECL_NOTHROW QUL_DECL_NOEXCEPT
#endif

#if defined(QUL_COMPILER_ALIGNOF)
#undef QUL_ALIGNOF
#define QUL_ALIGNOF(x) alignof(x)
#elif !defined(QUL_ALIGNOF)
#define QUL_ALIGNOF(x)
#endif

#if defined(QUL_COMPILER_ALIGNAS)
#undef QUL_DECL_ALIGN
#define QUL_DECL_ALIGN(n) alignas(n)
#elif !defined(QUL_DECL_ALIGN)
#define QUL_DECL_ALIGN(n)
#endif

#if defined(__cpp_enumerator_attributes) && __cpp_enumerator_attributes >= 201411
#if defined(QUL_CC_MSVC)
// Can't mark enum values as __declspec(deprecated) with MSVC, also can't move
// everything to [[deprecated]] because MSVC gives a compilation error when marking
// friend methods of a class as [[deprecated("text")]], breaking qstring.h
#define QUL_DECL_ENUMERATOR_DEPRECATED [[deprecated]]
#define QUL_DECL_ENUMERATOR_DEPRECATED_X(x) [[deprecated(x)]]
#else
#define QUL_DECL_ENUMERATOR_DEPRECATED QUL_DECL_DEPRECATED
#define QUL_DECL_ENUMERATOR_DEPRECATED_X(x) QUL_DECL_DEPRECATED_X(x)
#endif
#endif

/*
 * Fallback macros to certain compiler features
 */

#ifndef QUL_NOINLINE
#define QUL_NOINLINE
#endif
#ifndef QUL_NORETURN
#define QUL_NORETURN
#endif
#ifndef QUL_LIKELY
#define QUL_LIKELY(x) (x)
#endif
#ifndef QUL_UNLIKELY
#define QUL_UNLIKELY(x) (x)
#endif
#ifndef QUL_ASSUME_IMPL
#define QUL_ASSUME_IMPL(expr) qt_noop()
#endif
#ifndef QUL_UNREACHABLE_IMPL
#define QUL_UNREACHABLE_IMPL() qt_noop()
#endif
#ifndef QUL_ALLOC_SIZE
#define QUL_ALLOC_SIZE(x)
#endif
#ifndef QUL_REQUIRED_RESULT
#define QUL_REQUIRED_RESULT
#endif
#ifndef QUL_DECL_DEPRECATED
#define QUL_DECL_DEPRECATED
#endif
#ifndef QUL_DECL_EMPTY_BASES
#define QUL_DECL_EMPTY_BASES
#endif
#ifndef QUL_DECL_VARIABLE_DEPRECATED
#define QUL_DECL_VARIABLE_DEPRECATED QUL_DECL_DEPRECATED
#endif
#ifndef QUL_DECL_DEPRECATED_X
#define QUL_DECL_DEPRECATED_X(text) QUL_DECL_DEPRECATED
#endif
#ifndef QUL_DECL_ENUMERATOR_DEPRECATED
#define QUL_DECL_ENUMERATOR_DEPRECATED
#endif
#ifndef QUL_DECL_ENUMERATOR_DEPRECATED_X
#define QUL_DECL_ENUMERATOR_DEPRECATED_X(x)
#endif
#ifndef QUL_DECL_EXPORT
#define QUL_DECL_EXPORT
#endif
#ifndef QUL_DECL_IMPORT
#define QUL_DECL_IMPORT
#endif
#ifndef QUL_DECL_HIDDEN
#define QUL_DECL_HIDDEN
#endif
#ifndef QUL_DECL_UNUSED
#define QUL_DECL_UNUSED
#endif
#define QUL_DECL_MAYBE_UNUSED QUL_DECL_UNUSED
#ifndef QUL_DECL_UNUSED_MEMBER
#define QUL_DECL_UNUSED_MEMBER
#endif
#ifndef QUL_FUNC_INFO
#if defined(QUL_OS_SOLARIS) || defined(QUL_CC_XLC)
#define QUL_FUNC_INFO __FILE__ "(line number unavailable)"
#else
#define QUL_FUNC_INFO __FILE__ ":" QUL_STRINGIFY(__LINE__)
#endif
#endif
#ifndef QUL_DECL_CF_RETURNS_RETAINED
#define QUL_DECL_CF_RETURNS_RETAINED
#endif
#ifndef QUL_DECL_NS_RETURNS_AUTORELEASED
#define QUL_DECL_NS_RETURNS_AUTORELEASED
#endif
#ifndef QUL_DECL_PURE_FUNCTION
#define QUL_DECL_PURE_FUNCTION
#endif
#ifndef QUL_DECL_CONST_FUNCTION
#define QUL_DECL_CONST_FUNCTION QUL_DECL_PURE_FUNCTION
#endif
#ifndef QUL_DECL_COLD_FUNCTION
#define QUL_DECL_COLD_FUNCTION
#endif
#ifndef QUL_MAKE_UNCHECKED_ARRAY_ITERATOR
#define QUL_MAKE_UNCHECKED_ARRAY_ITERATOR(x) (x)
#endif
#ifndef QUL_MAKE_CHECKED_ARRAY_ITERATOR
#define QUL_MAKE_CHECKED_ARRAY_ITERATOR(x, N) (x)
#endif
#ifdef __has_feature
#define QUL_HAS_FEATURE(x) __has_feature(x)
#else
#define QUL_HAS_FEATURE(x) 0
#endif

/*
 * Warning/diagnostic handling
 */

#define QUL_DO_PRAGMA(text) _Pragma(#text)
#if defined(QUL_CC_INTEL) && defined(QUL_CC_MSVC)
/* icl.exe: Intel compiler on Windows */
#undef QUL_DO_PRAGMA /* not needed */
#define QUL_WARNING_PUSH __pragma(warning(push))
#define QUL_WARNING_POP __pragma(warning(pop))
#define QUL_WARNING_DISABLE_MSVC(number)
#define QUL_WARNING_DISABLE_INTEL(number) __pragma(warning(disable : number))
#define QUL_WARNING_DISABLE_CLANG(text)
#define QUL_WARNING_DISABLE_GCC(text)
#define QUL_WARNING_DISABLE_DEPRECATED QUL_WARNING_DISABLE_INTEL(1478 1786)
#elif defined(QUL_CC_INTEL)
/* icc: Intel compiler on Linux or OS X */
#define QUL_WARNING_PUSH QUL_DO_PRAGMA(warning(push))
#define QUL_WARNING_POP QUL_DO_PRAGMA(warning(pop))
#define QUL_WARNING_DISABLE_INTEL(number) QUL_DO_PRAGMA(warning(disable : number))
#define QUL_WARNING_DISABLE_MSVC(number)
#define QUL_WARNING_DISABLE_CLANG(text)
#define QUL_WARNING_DISABLE_GCC(text)
#define QUL_WARNING_DISABLE_DEPRECATED QUL_WARNING_DISABLE_INTEL(1478 1786)
#elif defined(QUL_CC_MSVC) && !defined(QUL_CC_CLANG)
#undef QUL_DO_PRAGMA /* not needed */
#define QUL_WARNING_PUSH __pragma(warning(push))
#define QUL_WARNING_POP __pragma(warning(pop))
#define QUL_WARNING_DISABLE_MSVC(number) __pragma(warning(disable : number))
#define QUL_WARNING_DISABLE_INTEL(number)
#define QUL_WARNING_DISABLE_CLANG(text)
#define QUL_WARNING_DISABLE_GCC(text)
#define QUL_WARNING_DISABLE_DEPRECATED QUL_WARNING_DISABLE_MSVC(4996)
#elif defined(QUL_CC_CLANG)
#define QUL_WARNING_PUSH QUL_DO_PRAGMA(clang diagnostic push)
#define QUL_WARNING_POP QUL_DO_PRAGMA(clang diagnostic pop)
#define QUL_WARNING_DISABLE_CLANG(text) QUL_DO_PRAGMA(clang diagnostic ignored text)
#define QUL_WARNING_DISABLE_GCC(text)
#define QUL_WARNING_DISABLE_INTEL(number)
#define QUL_WARNING_DISABLE_MSVC(number)
#define QUL_WARNING_DISABLE_DEPRECATED QUL_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
#elif defined(QUL_CC_GNU) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
#define QUL_WARNING_PUSH QUL_DO_PRAGMA(GCC diagnostic push)
#define QUL_WARNING_POP QUL_DO_PRAGMA(GCC diagnostic pop)
#define QUL_WARNING_DISABLE_GCC(text) QUL_DO_PRAGMA(GCC diagnostic ignored text)
#define QUL_WARNING_DISABLE_CLANG(text)
#define QUL_WARNING_DISABLE_INTEL(number)
#define QUL_WARNING_DISABLE_MSVC(number)
#define QUL_WARNING_DISABLE_DEPRECATED QUL_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
#else // All other compilers, GCC < 4.6 and MSVC < 2008
#define QUL_WARNING_DISABLE_GCC(text)
#define QUL_WARNING_PUSH
#define QUL_WARNING_POP
#define QUL_WARNING_DISABLE_INTEL(number)
#define QUL_WARNING_DISABLE_MSVC(number)
#define QUL_WARNING_DISABLE_CLANG(text)
#define QUL_WARNING_DISABLE_GCC(text)
#define QUL_WARNING_DISABLE_DEPRECATED
#endif

/*
   Proper for-scoping in MIPSpro CC
*/
#ifndef QUL_NO_KEYWORDS
#if defined(QUL_CC_MIPS) || (defined(QUL_CC_HPACC) && defined(__ia64))
#define for if (0){} else for
#endif
#endif

#ifdef QUL_COMPILER_RVALUE_REFS
#define qMove(x) std::move(x)
#else
#define qMove(x) (x)
#endif

#define QUL_UNREACHABLE() \
    do { \
        assert(!"QUL_UNREACHABLE was reached"); \
        QUL_UNREACHABLE_IMPL(); \
    } while (false)

#define QUL_ASSUME(Expr) \
    do { \
        const bool valueOfExpression = Expr; \
        assert(valueOfExpression && !"Assumption in QUL_ASSUME(\"" #Expr "\") was not correct"); \
        QUL_ASSUME_IMPL(valueOfExpression); \
    } while (false)

#if defined(__cplusplus) && __cplusplus >= 201103L && !defined(__ICCARM__)
#if QUL_HAS_CPP_ATTRIBUTE(clang::fallthrough)
#define QUL_FALLTHROUGH() [[clang::fallthrough]]
#elif QUL_HAS_CPP_ATTRIBUTE(gnu::fallthrough)
#define QUL_FALLTHROUGH() [[gnu::fallthrough]]
#elif QUL_HAS_CPP_ATTRIBUTE(fallthrough)
#define QUL_FALLTHROUGH() [[fallthrough]]
#endif
#endif
#ifndef QUL_FALLTHROUGH
#if (defined(QUL_CC_GNU) && QUL_CC_GNU >= 700) && !defined(QUL_CC_INTEL)
#define QUL_FALLTHROUGH() __attribute__((fallthrough))
#else
#define QUL_FALLTHROUGH() (void) 0
#endif
#endif

#ifdef QUL_PACKED
#define QUL_PACKED_HINT QUL_PACKED
#define QUL_PACKED_DEFINED 1
#else
#define QUL_PACKED_HINT
#define QUL_PACKED_DEFINED 0
#endif

/*
    Sanitize compiler feature availability
*/
#if !defined(QUL_PROCESSOR_X86)
#undef QUL_COMPILER_SUPPORTS_SSE2
#undef QUL_COMPILER_SUPPORTS_SSE3
#undef QUL_COMPILER_SUPPORTS_SSSE3
#undef QUL_COMPILER_SUPPORTS_SSE4_1
#undef QUL_COMPILER_SUPPORTS_SSE4_2
#undef QUL_COMPILER_SUPPORTS_AVX
#undef QUL_COMPILER_SUPPORTS_AVX2
#endif
#if !defined(QUL_PROCESSOR_ARM)
#undef QUL_COMPILER_SUPPORTS_NEON
#endif
#if !defined(QUL_PROCESSOR_MIPS)
#undef QUL_COMPILER_SUPPORTS_MIPS_DSP
#undef QUL_COMPILER_SUPPORTS_MIPS_DSPR2
#endif

#endif // QUL_COMPILERDETECTION_H
