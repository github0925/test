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
#ifndef QUL_INTMATH_H
#define QUL_INTMATH_H

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
#include <qul/private/global.h>
#include <limits>

#if defined(QUL_CC_IAR)
#include "intrinsics.h"
#endif

namespace Qul {
namespace Private {

template<typename T>
T uintPowOf10(unsigned exp);

template<typename T>
unsigned uintLog2(T value);

namespace IntmathImpl {

#if defined(__GNUC__) || defined(__clang__)

template<typename T>
typename qul_enable_if<sizeof(T) == sizeof(int), unsigned>::Type clz(T value)
{
    return __builtin_clz(static_cast<unsigned int>(value));
}

template<typename T>
typename qul_enable_if<sizeof(T) == sizeof(long) && sizeof(T) != sizeof(int), unsigned>::Type clz(T value)
{
    return __builtin_clzl(static_cast<unsigned long>(value));
}

#elif defined(QUL_CC_IAR)

inline unsigned clz(unsigned long value)
{
    return __CLZ(value);
}

#else

template<typename T>
unsigned clz(T value)
{
    unsigned setBits = 0;
    for (T v = value; v != 0; v >>= 1) {
        ++setBits;
    }
    return std::numeric_limits<T>::digits - setBits;
}

#ifndef _MSC_VER
#warning "Using software implementation of clz(), consider using intrinsic for your compiler."
#endif

#endif

extern const quint32 powersOf10[];
extern const quint8 log2ToApproxLog10[];

template<typename T>
typename qul_enable_if<sizeof(T) <= sizeof(quint32), T>::Type uintPowOf10Impl(unsigned exp)
{
    qul_static_assert<!std::numeric_limits<T>::is_signed>();
    return powersOf10[exp];
}

template<typename T>
typename qul_enable_if<sizeof(T) == sizeof(quint64), T>::Type uintPowOf10Impl(unsigned exp)
{
    qul_static_assert<!std::numeric_limits<T>::is_signed>();
    return exp < 10 ? uintPowOf10Impl<quint32>(exp) : QUL_UINT64_C(10000000000) * uintPowOf10Impl<quint32>(exp - 10);
}

} // namespace IntmathImpl

template<typename T>
T uintPowOf10(unsigned exp)
{
    qul_static_assert<!std::numeric_limits<T>::is_signed>();
    return IntmathImpl::uintPowOf10Impl<T>(exp);
}

inline unsigned uintLog2(quint32 value)
{
    assert(value != 0);
    return 31 - IntmathImpl::clz(static_cast<quint32>(value));
}

template<typename T>
typename qul_enable_if<sizeof(T) <= sizeof(quint32), unsigned>::Type uintLog10(T value)
{
    qul_static_assert<!std::numeric_limits<T>::is_signed>();
    assert(value != 0);
    unsigned approxRes = IntmathImpl::log2ToApproxLog10[uintLog2(value)];
    return approxRes + (value >= uintPowOf10<quint32>(approxRes + 1) ? 1 : 0);
}

template<typename T>
typename qul_enable_if<sizeof(quint32) < sizeof(T), unsigned>::Type uintLog10(T value)
{
    qul_static_assert<!std::numeric_limits<T>::is_signed>();
    assert(value != 0);
    const quint64 pow9 = QUL_UINT64_C(1000000000);
    return value <= pow9 ? uintLog10(static_cast<quint32>(value))
                         : value <= pow9 * pow9 ? 9 + uintLog10(static_cast<quint32>(value / pow9))
                                                : 18 + uintLog10(static_cast<quint32>(value / (pow9 * pow9)));
}

} // namespace Private
} // namespace Qul

#endif
