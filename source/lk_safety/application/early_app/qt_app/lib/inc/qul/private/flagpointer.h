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
#pragma once

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

#include <qul/private/global.h>

namespace Qul {
namespace Private {

template<typename T>
class FlagPointer
{
public:
    inline FlagPointer();
    inline FlagPointer(T *);
    inline FlagPointer(const FlagPointer<T> &o);

    inline bool isNull() const;

    inline bool flag() const;
    inline void setFlag();
    inline void clearFlag();
    inline void setFlagValue(bool);

    inline bool flag2() const;
    inline void setFlag2();
    inline void clearFlag2();
    inline void setFlag2Value(bool);

    inline quintptr combinedFlag() const;
    inline void setCombinedFlag(quintptr value);

    inline FlagPointer<T> &operator=(const FlagPointer &o);
    inline FlagPointer<T> &operator=(T *);

    inline T *operator->() const;
    inline T &operator*() const;

    inline T *data() const;

    inline operator bool() const;

protected:
    explicit FlagPointer(quintptr val)
        : ptr_value(val)
    {}
    quintptr ptr_value;

    static const quintptr FlagBit = 0x1;
    static const quintptr Flag2Bit = 0x2;
    static const quintptr FlagsMask = FlagBit | Flag2Bit;
};

template<typename T, typename T2>
class BiPointer : public FlagPointer<T>
{
public:
    // Default construct as isNull() && isT1()
    inline BiPointer();
    inline BiPointer(T *);
    inline BiPointer(T2 *);
    inline BiPointer(const BiPointer &o);

    // These three are not mutually exclusive.
    // isNull() && isT1() and isNull() && isT2() are both possible
    inline bool isNull() const;
    inline bool isT1() const;
    inline bool isT2() const;

    inline BiPointer &operator=(const BiPointer &o);

    // this assignment preserves flag()
    inline BiPointer &operator=(T *);
    inline BiPointer &operator=(T2 *);

    // assert if wrong type
    inline T *asT1() const;
    inline T2 *asT2() const;

    // nullptr if wrong type
    inline T *maybeT1() const;
    inline T2 *maybeT2() const;

private:
    using FlagPointer<T>::flag2;
    using FlagPointer<T>::setFlag2;
    using FlagPointer<T>::clearFlag2;
    using FlagPointer<T>::setFlag2Value;
    using FlagPointer<T>::operator->;
    using FlagPointer<T>::operator*;
    using FlagPointer<T>::data;

    using FlagPointer<T>::ptr_value;
    using FlagPointer<T>::FlagBit;
    using FlagPointer<T>::Flag2Bit;
    using FlagPointer<T>::FlagsMask;

    template<typename U, typename U2>
    friend bool operator==(const BiPointer<U, U2> l, const BiPointer<U, U2> r);
    template<typename U, typename U2>
    friend bool operator!=(const BiPointer<U, U2> l, const BiPointer<U, U2> r);
};

template<typename T>
FlagPointer<T>::FlagPointer()
    : ptr_value(0)
{}

template<typename T>
FlagPointer<T>::FlagPointer(T *v)
    : ptr_value(quintptr(v))
{
    assert((ptr_value & FlagsMask) == 0);
}

template<typename T>
FlagPointer<T>::FlagPointer(const FlagPointer<T> &o)
    : ptr_value(o.ptr_value)
{}

template<typename T>
bool FlagPointer<T>::isNull() const
{
    return 0 == (ptr_value & (~FlagsMask));
}

template<typename T>
bool FlagPointer<T>::flag() const
{
    return ptr_value & FlagBit;
}

template<typename T>
void FlagPointer<T>::setFlag()
{
    ptr_value |= FlagBit;
}

template<typename T>
void FlagPointer<T>::clearFlag()
{
    ptr_value &= ~FlagBit;
}

template<typename T>
void FlagPointer<T>::setFlagValue(bool v)
{
    if (v)
        setFlag();
    else
        clearFlag();
}

template<typename T>
bool FlagPointer<T>::flag2() const
{
    return ptr_value & Flag2Bit;
}

template<typename T>
void FlagPointer<T>::setFlag2()
{
    ptr_value |= Flag2Bit;
}

template<typename T>
void FlagPointer<T>::clearFlag2()
{
    ptr_value &= ~Flag2Bit;
}

template<typename T>
void FlagPointer<T>::setFlag2Value(bool v)
{
    if (v)
        setFlag2();
    else
        clearFlag2();
}

template<typename T>
quintptr FlagPointer<T>::combinedFlag() const
{
    return ptr_value & FlagsMask;
}

template<typename T>
void FlagPointer<T>::setCombinedFlag(quintptr value)
{
    ptr_value = (ptr_value & ~FlagsMask) | (value & FlagsMask);
}

template<typename T>
FlagPointer<T> &FlagPointer<T>::operator=(const FlagPointer &o)
{
    ptr_value = o.ptr_value;
    return *this;
}

template<typename T>
FlagPointer<T> &FlagPointer<T>::operator=(T *o)
{
    assert((quintptr(o) & FlagsMask) == 0);

    ptr_value = quintptr(o) | (ptr_value & FlagsMask);
    return *this;
}

template<typename T>
T *FlagPointer<T>::operator->() const
{
    return (T *) (ptr_value & ~FlagsMask);
}

template<typename T>
T &FlagPointer<T>::operator*() const
{
    return *(T *) (ptr_value & ~FlagsMask);
}

template<typename T>
T *FlagPointer<T>::data() const
{
    return (T *) (ptr_value & ~FlagsMask);
}

template<typename T>
FlagPointer<T>::operator bool() const
{
    return data();
}

template<typename T, typename T2>
BiPointer<T, T2>::BiPointer()
    : FlagPointer<T>()
{}

template<typename T, typename T2>
BiPointer<T, T2>::BiPointer(T *v)
    : FlagPointer<T>(v)
{
    assert((quintptr(v) & FlagsMask) == 0);
}

template<typename T, typename T2>
BiPointer<T, T2>::BiPointer(T2 *v)
    : FlagPointer<T>(quintptr(v) | Flag2Bit)
{
    assert((quintptr(v) & FlagsMask) == 0);
}

template<typename T, typename T2>
BiPointer<T, T2>::BiPointer(const BiPointer<T, T2> &o)
    : FlagPointer<T>(o.ptr_value)
{}

template<typename T, typename T2>
bool BiPointer<T, T2>::isNull() const
{
    return 0 == (ptr_value & (~FlagsMask));
}

template<typename T, typename T2>
bool BiPointer<T, T2>::isT1() const
{
    return !(ptr_value & Flag2Bit);
}

template<typename T, typename T2>
bool BiPointer<T, T2>::isT2() const
{
    return ptr_value & Flag2Bit;
}

template<typename T, typename T2>
BiPointer<T, T2> &BiPointer<T, T2>::operator=(const BiPointer<T, T2> &o)
{
    ptr_value = o.ptr_value;
    return *this;
}

template<typename T, typename T2>
BiPointer<T, T2> &BiPointer<T, T2>::operator=(T *o)
{
    assert((quintptr(o) & FlagsMask) == 0);

    ptr_value = quintptr(o) | (ptr_value & FlagBit);
    return *this;
}

template<typename T, typename T2>
BiPointer<T, T2> &BiPointer<T, T2>::operator=(T2 *o)
{
    assert((quintptr(o) & FlagsMask) == 0);

    ptr_value = quintptr(o) | (ptr_value & FlagBit) | Flag2Bit;
    return *this;
}

template<typename T, typename T2>
T *BiPointer<T, T2>::asT1() const
{
    assert(isT1());
    return (T *) (ptr_value & ~FlagsMask);
}

template<typename T, typename T2>
T2 *BiPointer<T, T2>::asT2() const
{
    assert(isT2());
    return (T2 *) (ptr_value & ~FlagsMask);
}

template<typename T, typename T2>
T *BiPointer<T, T2>::maybeT1() const
{
    return isT1() ? asT1() : nullptr;
}

template<typename T, typename T2>
T2 *BiPointer<T, T2>::maybeT2() const
{
    return isT2() ? asT2() : nullptr;
}

template<typename T, typename T2>
bool operator==(const BiPointer<T, T2> l, const BiPointer<T, T2> r)
{
    return l.ptr_value == r.ptr_value;
}

template<typename T, typename T2>
bool operator!=(const BiPointer<T, T2> l, const BiPointer<T, T2> r)
{
    return l.ptr_value != r.ptr_value;
}

} // namespace Private
} // namespace Qul
