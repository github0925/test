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

#include <platforminterface/point.h>
#include <platforminterface/size.h>

namespace Qul {
namespace Private {

struct FixedNumber
{
private:
    FixedNumber(int val, int)
        : val(val)
    {} // 2nd int is just a dummy for disambiguation
public:
    FixedNumber()
        : val(0)
    {}
    FixedNumber(int i)
        : val(i * 64)
    {}
    FixedNumber(long i)
        : val(i * 64)
    {}
    FixedNumber &operator=(int i)
    {
        val = i * 64;
        return *this;
    }
    FixedNumber &operator=(long i)
    {
        val = i * 64;
        return *this;
    }

    static FixedNumber fromReal(float r) { return fromFixed((int) (r * float(64))); }
    static FixedNumber fromFixed(int fixed) { return FixedNumber(fixed, 0); } // uses private ctor

    inline int value() const { return val; }
    inline void setValue(int value) { val = value; }

    inline int toInt() const { return (((val) + 32) & -64) >> 6; }
    inline float toReal() const { return ((float) val) / (float) 64; }

    inline int truncate() const { return val >> 6; }
    inline FixedNumber round() const { return fromFixed(((val) + 32) & -64); }
    inline FixedNumber floor() const { return fromFixed((val) & -64); }
    inline FixedNumber ceil() const { return fromFixed((val + 63) & -64); }

    inline FixedNumber operator+(int i) const { return fromFixed(val + i * 64); }
    inline FixedNumber operator+(uint i) const { return fromFixed((val + (i << 6))); }
    inline FixedNumber operator+(const FixedNumber &other) const { return fromFixed((val + other.val)); }
    inline FixedNumber &operator+=(int i)
    {
        val += i * 64;
        return *this;
    }
    inline FixedNumber &operator+=(uint i)
    {
        val += (i << 6);
        return *this;
    }
    inline FixedNumber &operator+=(const FixedNumber &other)
    {
        val += other.val;
        return *this;
    }
    inline FixedNumber operator-(int i) const { return fromFixed(val - i * 64); }
    inline FixedNumber operator-(uint i) const { return fromFixed((val - (i << 6))); }
    inline FixedNumber operator-(const FixedNumber &other) const { return fromFixed((val - other.val)); }
    inline FixedNumber &operator-=(int i)
    {
        val -= i * 64;
        return *this;
    }
    inline FixedNumber &operator-=(uint i)
    {
        val -= (i << 6);
        return *this;
    }
    inline FixedNumber &operator-=(const FixedNumber &other)
    {
        val -= other.val;
        return *this;
    }
    inline FixedNumber operator-() const { return fromFixed(-val); }

    inline bool operator==(const FixedNumber &other) const { return val == other.val; }
    inline bool operator!=(const FixedNumber &other) const { return val != other.val; }
    inline bool operator<(const FixedNumber &other) const { return val < other.val; }
    inline bool operator>(const FixedNumber &other) const { return val > other.val; }
    inline bool operator<=(const FixedNumber &other) const { return val <= other.val; }
    inline bool operator>=(const FixedNumber &other) const { return val >= other.val; }
    inline bool operator!() const { return !val; }

    inline FixedNumber &operator/=(int x)
    {
        val /= x;
        return *this;
    }
    inline FixedNumber &operator/=(const FixedNumber &o)
    {
        if (o.val == 0) {
            val = 0x7FFFFFFFL;
        } else {
            bool neg = false;
            qint64 a = val;
            qint64 b = o.val;
            if (a < 0) {
                a = -a;
                neg = true;
            }
            if (b < 0) {
                b = -b;
                neg = !neg;
            }

            int res = (int) (((a << 6) + (b >> 1)) / b);

            val = (neg ? -res : res);
        }
        return *this;
    }
    inline FixedNumber operator/(int d) const { return fromFixed(val / d); }
    inline FixedNumber operator/(FixedNumber b) const
    {
        FixedNumber f = *this;
        return (f /= b);
    }
    inline FixedNumber operator>>(int d) const
    {
        FixedNumber f = *this;
        f.val >>= d;
        return f;
    }
    inline FixedNumber &operator*=(int i)
    {
        val *= i;
        return *this;
    }
    inline FixedNumber &operator*=(uint i)
    {
        val *= i;
        return *this;
    }
    inline FixedNumber &operator*=(const FixedNumber &o)
    {
        bool neg = false;
        qint64 a = val;
        qint64 b = o.val;
        if (a < 0) {
            a = -a;
            neg = true;
        }
        if (b < 0) {
            b = -b;
            neg = !neg;
        }

        int res = (int) ((a * b + 0x20L) >> 6);
        val = neg ? -res : res;
        return *this;
    }
    inline FixedNumber operator*(int i) const { return fromFixed(val * i); }
    inline FixedNumber operator*(uint i) const { return fromFixed(val * i); }
    inline FixedNumber operator*(const FixedNumber &o) const
    {
        FixedNumber f = *this;
        return (f *= o);
    }

private:
    FixedNumber(float i)
        : val((int) (i * float(64)))
    {}
    FixedNumber &operator=(float i)
    {
        val = (int) (i * float(64));
        return *this;
    }
    inline FixedNumber operator+(float i) const { return fromFixed((val + (int) (i * float(64)))); }
    inline FixedNumber &operator+=(float i)
    {
        val += (int) (i * 64);
        return *this;
    }
    inline FixedNumber operator-(float i) const { return fromFixed((val - (int) (i * float(64)))); }
    inline FixedNumber &operator-=(float i)
    {
        val -= (int) (i * 64);
        return *this;
    }
    inline FixedNumber &operator/=(float r)
    {
        val = (int) (val / r);
        return *this;
    }
    inline FixedNumber operator/(float d) const { return fromFixed((int) (val / d)); }
    inline FixedNumber &operator*=(float d)
    {
        val = (int) (val * d);
        return *this;
    }
    inline FixedNumber operator*(float d) const { return fromFixed((int) (val * d)); }
    int val;
};

inline int qRound(const FixedNumber &f)
{
    return f.toInt();
}
inline int qFloor(const FixedNumber &f)
{
    return f.floor().truncate();
}

inline FixedNumber operator*(int i, const FixedNumber &d)
{
    return d * i;
}
inline FixedNumber operator+(int i, const FixedNumber &d)
{
    return d + i;
}
inline FixedNumber operator-(int i, const FixedNumber &d)
{
    return -(d - i);
}
inline FixedNumber operator*(uint i, const FixedNumber &d)
{
    return d * i;
}
inline FixedNumber operator+(uint i, const FixedNumber &d)
{
    return d + i;
}
inline FixedNumber operator-(uint i, const FixedNumber &d)
{
    return -(d - i);
}
//  inline FixedNumber operator*(float d, const FixedNumber &d2) { return d2*d; }

inline bool operator==(const FixedNumber &f, int i)
{
    return f.value() == i * 64;
}
inline bool operator==(int i, const FixedNumber &f)
{
    return f.value() == i * 64;
}
inline bool operator!=(const FixedNumber &f, int i)
{
    return f.value() != i * 64;
}
inline bool operator!=(int i, const FixedNumber &f)
{
    return f.value() != i * 64;
}
inline bool operator<=(const FixedNumber &f, int i)
{
    return f.value() <= i * 64;
}
inline bool operator<=(int i, const FixedNumber &f)
{
    return i * 64 <= f.value();
}
inline bool operator>=(const FixedNumber &f, int i)
{
    return f.value() >= i * 64;
}
inline bool operator>=(int i, const FixedNumber &f)
{
    return i * 64 >= f.value();
}
inline bool operator<(const FixedNumber &f, int i)
{
    return f.value() < i * 64;
}
inline bool operator<(int i, const FixedNumber &f)
{
    return i * 64 < f.value();
}
inline bool operator>(const FixedNumber &f, int i)
{
    return f.value() > i * 64;
}
inline bool operator>(int i, const FixedNumber &f)
{
    return i * 64 > f.value();
}

struct FixedNumberPoint
{
    FixedNumber x;
    FixedNumber y;
    inline FixedNumberPoint() {}
    inline FixedNumberPoint(const FixedNumber &_x, const FixedNumber &_y)
        : x(_x)
        , y(_y)
    {}
    PlatformInterface::PointF toPointF() const { return PlatformInterface::PointF(x.toReal(), y.toReal()); }
    static FixedNumberPoint fromPointF(const PlatformInterface::PointF &p)
    {
        return FixedNumberPoint(FixedNumber::fromReal(p.x()), FixedNumber::fromReal(p.y()));
    }
};

inline FixedNumberPoint operator-(const FixedNumberPoint &p1, const FixedNumberPoint &p2)
{
    return FixedNumberPoint(p1.x - p2.x, p1.y - p2.y);
}
inline FixedNumberPoint operator+(const FixedNumberPoint &p1, const FixedNumberPoint &p2)
{
    return FixedNumberPoint(p1.x + p2.x, p1.y + p2.y);
}

struct FixedNumberSize
{
    FixedNumber width;
    FixedNumber height;
    FixedNumberSize() {}
    FixedNumberSize(FixedNumber _width, FixedNumber _height)
        : width(_width)
        , height(_height)
    {}
    PlatformInterface::SizeF toSizeF() const { return PlatformInterface::SizeF(width.toReal(), height.toReal()); }
    static FixedNumberSize fromSizeF(const PlatformInterface::SizeF &s)
    {
        return FixedNumberSize(FixedNumber::fromReal(s.width()), FixedNumber::fromReal(s.height()));
    }
};

} // namespace Private
} // namespace Qul
