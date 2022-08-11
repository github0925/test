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
#ifndef QUL_GLOBAL_H
#define QUL_GLOBAL_H

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

#include <qul/global.h>
#include <qul/private/processordetection.h>
#include <qul/private/compilerdetection.h>
#include <qul/private/systemdetection.h>
#include <qul/private/config.h>

#include <cstddef>
#include <cassert>

#ifndef __ASSEMBLER__
/*
   Avoid "unused parameter" warnings
*/
#define QUL_UNUSED(x) (void) x;

#define QUL_TO_STRING_HELPER(x) #x
#define QUL_TO_STRING(x) QUL_TO_STRING_HELPER(x)

/*
   Size-dependent types (architechture-dependent byte order)

   Make sure to update QMetaType when changing these typedefs
*/

#if !defined(QUL_NEVER_INLINE) || !defined(QUL_ALWAYS_INLINE)
#ifdef QUL_CC_MSVC
#define QUL_NEVER_INLINE __declspec(noinline)
#define QUL_ALWAYS_INLINE __forceinline
#elif defined(QUL_CC_GNU)
#define QUL_NEVER_INLINE __attribute__((noinline))
#define QUL_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define QUL_NEVER_INLINE
#define QUL_ALWAYS_INLINE inline
#endif
#endif

#ifdef QUL_PROCESSOR_X86_32
#if defined(QUL_CC_GNU)
#define QUL_FASTCALL __attribute__((regparm(3)))
#elif defined(QUL_CC_MSVC)
#define QUL_FASTCALL __fastcall
#else
#define QUL_FASTCALL
#endif
#else
#define QUL_FASTCALL
#endif

#if defined(QUL_SHARED) || !defined(QUL_STATIC)
#ifdef QUL_STATIC
#error "Both QUL_SHARED and QUL_STATIC defined, please make up your mind"
#endif
#ifndef QUL_SHARED
#define QUL_SHARED
#endif
#if defined(QUL_BUILD_LIB)
#define QUL_EXPORT QUL_DECL_EXPORT
#else
#define QUL_EXPORT QUL_DECL_IMPORT
#endif
#else
#define QUL_EXPORT
#endif

#if defined(__cplusplus) && defined(QUL_COMPILER_STATIC_ASSERT)
#define QUL_STATIC_ASSERT(Condition) static_assert(bool(Condition), #Condition)
#define QUL_STATIC_ASSERT_X(Condition, Message) static_assert(bool(Condition), Message)
#elif defined(QUL_COMPILER_STATIC_ASSERT)
// C11 mode - using the _S version in case <assert.h> doesn't do the right thing
#define QUL_STATIC_ASSERT(Condition) _Static_assert(!!(Condition), #Condition)
#define QUL_STATIC_ASSERT_X(Condition, Message) _Static_assert(!!(Condition), Message)
#else
// C89 & C99 version
#define QUL_STATIC_ASSERT_PRIVATE_JOIN(A, B) QUL_STATIC_ASSERT_PRIVATE_JOIN_IMPL(A, B)
#define QUL_STATIC_ASSERT_PRIVATE_JOIN_IMPL(A, B) A##B
#ifdef __COUNTER__
#define QUL_STATIC_ASSERT(Condition) \
    typedef char QUL_STATIC_ASSERT_PRIVATE_JOIN(q_static_assert_result, __COUNTER__)[(Condition) ? 1 : -1];
#else
#define QUL_STATIC_ASSERT(Condition) \
    typedef char QUL_STATIC_ASSERT_PRIVATE_JOIN(q_static_assert_result, __LINE__)[(Condition) ? 1 : -1];
#endif /* __COUNTER__ */
#define QUL_STATIC_ASSERT_X(Condition, Message) QUL_STATIC_ASSERT(Condition)
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

namespace Qul {
namespace Private {

typedef signed char qint8;      /* 8 bit signed */
typedef unsigned char quint8;   /* 8 bit unsigned */
typedef short qint16;           /* 16 bit signed */
typedef unsigned short quint16; /* 16 bit unsigned */
typedef int qint32;             /* 32 bit signed */
typedef unsigned int quint32;   /* 32 bit unsigned */
#if defined(QUL_OS_WIN) && !defined(QUL_CC_GNU) && !defined(QUL_CC_MWERKS)
#define QUL_INT64_C(c) c##i64   /* signed 64 bit constant */
#define QUL_UINT64_C(c) c##ui64 /* unsigned 64 bit constant */
#else
#ifdef __cplusplus
#define QUL_INT64_C(c) static_cast<int64_t>(c##LL)    /* signed 64 bit constant */
#define QUL_UINT64_C(c) static_cast<uint64_t>(c##ULL) /* unsigned 64 bit constant */
#else
#define QUL_INT64_C(c) ((int64_t)(c##LL))    /* signed 64 bit constant */
#define QUL_UINT64_C(c) ((uint64_t)(c##ULL)) /* unsigned 64 bit constant */
#endif
#endif

typedef int64_t qint64;   /* 64 bit signed */
typedef uint64_t quint64; /* 64 bit unsigned */

typedef qint64 qlonglong;
typedef quint64 qulonglong;

template<int>
struct IntegerForSize;
template<>
struct IntegerForSize<1>
{
    typedef quint8 Unsigned;
    typedef qint8 Signed;
};
template<>
struct IntegerForSize<2>
{
    typedef quint16 Unsigned;
    typedef qint16 Signed;
};
template<>
struct IntegerForSize<4>
{
    typedef quint32 Unsigned;
    typedef qint32 Signed;
};
template<>
struct IntegerForSize<8>
{
    typedef quint64 Unsigned;
    typedef qint64 Signed;
};
#if defined(QUL_CC_GNU) && defined(__SIZEOF_INT128__)
template<>
struct IntegerForSize<16>
{
    __extension__ typedef unsigned __int128 Unsigned;
    __extension__ typedef __int128 Signed;
};
#endif
template<class T>
struct IntegerForSizeof : IntegerForSize<sizeof(T)>
{};
typedef IntegerForSize<QUL_PROCESSOR_WORDSIZE>::Signed qregisterint;
typedef IntegerForSize<QUL_PROCESSOR_WORDSIZE>::Unsigned qregisteruint;
typedef IntegerForSizeof<void *>::Unsigned quintptr;
typedef IntegerForSizeof<void *>::Signed qptrdiff;
typedef qptrdiff qintptr;
typedef IntegerForSizeof<std::size_t>::Signed qsizetype;

template<typename T>
QUL_DECL_CONSTEXPR inline T qSqr(const T &t)
{
    return t * t;
}

template<typename T>
QUL_DECL_CONSTEXPR inline T qAbs(const T &t)
{
    return t >= 0 ? t : -t;
}

QUL_DECL_CONSTEXPR inline int qRound(double d)
{
    return d >= 0.0 ? int(d + 0.5) : int(d - double(int(d - 1)) + 0.5) + int(d - 1);
}
QUL_DECL_CONSTEXPR inline int qRound(float d)
{
    return d >= 0.0f ? int(d + 0.5f) : int(d - float(int(d - 1)) + 0.5f) + int(d - 1);
}

QUL_DECL_CONSTEXPR inline qint64 qRound64(double d)
{
    return d >= 0.0 ? qint64(d + 0.5) : qint64(d - double(qint64(d - 1)) + 0.5) + qint64(d - 1);
}
QUL_DECL_CONSTEXPR inline qint64 qRound64(float d)
{
    return d >= 0.0f ? qint64(d + 0.5f) : qint64(d - float(qint64(d - 1)) + 0.5f) + qint64(d - 1);
}

template<typename T>
QUL_DECL_CONSTEXPR inline const T &qMin(const T &a, const T &b)
{
    return (a < b) ? a : b;
}
template<typename T>
QUL_DECL_CONSTEXPR inline const T &qMax(const T &a, const T &b)
{
    return (a < b) ? b : a;
}
template<typename T>
QUL_DECL_CONSTEXPR inline const T &qBound(const T &min, const T &val, const T &max)
{
    return qMax(min, qMin(max, val));
}

QUL_REQUIRED_RESULT QUL_DECL_CONSTEXPR static inline QUL_DECL_UNUSED bool qFuzzyCompare(double p1, double p2)
{
    return (qAbs(p1 - p2) * 1000000000000. <= qMin(qAbs(p1), qAbs(p2)));
}

QUL_REQUIRED_RESULT QUL_DECL_CONSTEXPR static inline QUL_DECL_UNUSED bool qFuzzyCompare(float p1, float p2)
{
    return (qAbs(p1 - p2) * 100000.f <= qMin(qAbs(p1), qAbs(p2)));
}

QUL_REQUIRED_RESULT QUL_DECL_CONSTEXPR static inline QUL_DECL_UNUSED bool qFuzzyIsNull(double d)
{
    return qAbs(d) <= 0.000000000001;
}

QUL_REQUIRED_RESULT QUL_DECL_CONSTEXPR static inline QUL_DECL_UNUSED bool qFuzzyIsNull(float f)
{
    return qAbs(f) <= 0.00001f;
}

/*
  Time related calculations
*/
typedef quint64 qtime;

/*
   This function tests a double for a null value. It doesn't
   check whether the actual value is 0 or close to 0, but whether
   it is binary 0, disregarding sign.
*/
QUL_REQUIRED_RESULT static inline QUL_DECL_UNUSED bool qIsNull(double d)
{
    union U {
        double d;
        quint64 u;
    };
    U val;
    val.d = d;
    return (val.u & QUL_UINT64_C(0x7fffffffffffffff)) == 0;
}

#define QUL_IS_ENV_SET(str, out) \
    do { \
        static bool _qul_is_env_set = getenv(str) != 0; \
        out = _qul_is_env_set; \
    } while (false)

#define QUL_GETENV_OR_DEFAULT(str, out, defaultVal) \
    do { \
        static const char *env = getenv(str); \
        if (env) \
            out = env; \
        else \
            out = defaultVal; \
    } while (false)

#define QUL_ENV_EQUALS(str, value, out) \
    do { \
        static const char *env = getenv(str); \
        static bool _qul_is_env_set = env && !strncmp(env, value, strlen(value)); \
        out = _qul_is_env_set; \
    } while (false)

namespace StaticAssert {
template<bool B>
struct qul_static_assert
{
    static const void *invalid[B ? 1 : -1];
    void operator()() {}
};

template<>
struct qul_static_assert<true>
{
    void operator()() {}
};
} // namespace StaticAssert

template<bool B>
void qul_static_assert()
{
    StaticAssert::qul_static_assert<B>()();
}

template<typename T1, typename T2>
struct qul_is_same
{
    static const bool value = false;
};

template<typename T>
struct qul_is_same<T, T>
{
    static const bool value = true;
};

template<bool B, typename T = void>
struct qul_enable_if
{};

template<typename T>
struct qul_enable_if<true, T>
{
    typedef T Type;
};

/*
 * Returns offset of a class member. Technically this is undefined behavior.
 *
 * Example usage: offsetOf(&X::c), where X is: struct X { int a, b, c, d; }
 *
 * By knowing a member offset, we can calclulate the pointer to the enclosing
 * object. This technique is widely used in Qul to reduce the required memory.
 * For examples, see SinglyLinkedList and Binding classes.
 */
template<typename Object, typename MemberType>
ptrdiff_t offsetOf(MemberType Object::*Member)
{
    return reinterpret_cast<char *>(&(static_cast<Object *>((void *) nullptr)->*Member))
           - static_cast<char *>((void *) nullptr);
}

} // namespace Private
} // namespace Qul

#endif // __ASSEMBLER__
#endif // QUL_GLOBAL_H
