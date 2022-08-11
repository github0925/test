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

#include <platforminterface/platforminterface.h>
#include <qul/private/global.h> // for qIsNull, qFuzzyCompare, qRound

#include <algorithm>
#include <cmath>

namespace Qul {
namespace PlatformInterface {

class Point
{
public:
    Point();
    Point(coord_t xpos, coord_t ypos);

    inline bool isNull() const;

    inline coord_t x() const;
    inline coord_t y() const;
    inline void setX(coord_t x);
    inline void setY(coord_t y);

    inline coord_t manhattanLength() const;

    inline coord_t &rx();
    inline coord_t &ry();

    inline Point &operator+=(const Point &p);
    inline Point &operator-=(const Point &p);

    inline Point &operator*=(float factor);
    inline Point &operator*=(coord_t factor);

    inline Point &operator/=(float divisor);

    static inline coord_t dotProduct(const Point &p1, const Point &p2) { return p1.xp * p2.xp + p1.yp * p2.yp; }

    friend inline bool operator==(const Point &, const Point &);
    friend inline bool operator!=(const Point &, const Point &);
    friend inline const Point operator+(const Point &, const Point &);
    friend inline const Point operator-(const Point &, const Point &);
    friend inline const Point operator*(const Point &, float);
    friend inline const Point operator*(float, const Point &);
    friend inline const Point operator*(const Point &, coord_t);
    friend inline const Point operator*(coord_t, const Point &);
    friend inline const Point operator+(const Point &);
    friend inline const Point operator-(const Point &);
    friend inline const Point operator/(const Point &, float);

private:
    coord_t xp;
    coord_t yp;
};

/*****************************************************************************
  Point inline functions
 *****************************************************************************/

inline Point::Point()
    : xp(0)
    , yp(0)
{}

inline Point::Point(coord_t xpos, coord_t ypos)
    : xp(xpos)
    , yp(ypos)
{}

inline bool Point::isNull() const
{
    return xp == 0 && yp == 0;
}

inline coord_t Point::x() const
{
    return xp;
}

inline coord_t Point::y() const
{
    return yp;
}

inline void Point::setX(coord_t xpos)
{
    xp = xpos;
}

inline void Point::setY(coord_t ypos)
{
    yp = ypos;
}

inline coord_t Point::manhattanLength() const
{
    return std::abs(x()) + std::abs(y());
}

inline coord_t &Point::rx()
{
    return xp;
}

inline coord_t &Point::ry()
{
    return yp;
}

inline Point &Point::operator+=(const Point &p)
{
    xp += p.xp;
    yp += p.yp;
    return *this;
}

inline Point &Point::operator-=(const Point &p)
{
    xp -= p.xp;
    yp -= p.yp;
    return *this;
}

inline Point &Point::operator*=(float factor)
{
    xp = Private::qRound(xp * factor);
    yp = Private::qRound(yp * factor);
    return *this;
}

inline Point &Point::operator*=(coord_t factor)
{
    xp = xp * factor;
    yp = yp * factor;
    return *this;
}

inline bool operator==(const Point &p1, const Point &p2)
{
    return p1.xp == p2.xp && p1.yp == p2.yp;
}

inline bool operator!=(const Point &p1, const Point &p2)
{
    return p1.xp != p2.xp || p1.yp != p2.yp;
}

inline const Point operator+(const Point &p1, const Point &p2)
{
    return Point(p1.xp + p2.xp, p1.yp + p2.yp);
}

inline const Point operator-(const Point &p1, const Point &p2)
{
    return Point(p1.xp - p2.xp, p1.yp - p2.yp);
}

inline const Point operator*(const Point &p, float factor)
{
    return Point(Private::qRound(p.xp * factor), Private::qRound(p.yp * factor));
}

inline const Point operator*(const Point &p, coord_t factor)
{
    return Point(p.xp * factor, p.yp * factor);
}

inline const Point operator*(float factor, const Point &p)
{
    return Point(Private::qRound(p.xp * factor), Private::qRound(p.yp * factor));
}

inline const Point operator*(coord_t factor, const Point &p)
{
    return Point(p.xp * factor, p.yp * factor);
}

inline const Point operator+(const Point &p)
{
    return p;
}

inline const Point operator-(const Point &p)
{
    return Point(-p.xp, -p.yp);
}

inline Point &Point::operator/=(float c)
{
    xp = Private::qRound(xp / c);
    yp = Private::qRound(yp / c);
    return *this;
}

inline const Point operator/(const Point &p, float c)
{
    return Point(Private::qRound(p.xp / c), Private::qRound(p.yp / c));
}

class PointF
{
public:
    PointF();
    PointF(const Point &p);
    PointF(float xpos, float ypos);

    inline float manhattanLength() const;

    inline bool isNull() const;

    inline float x() const;
    inline float y() const;
    inline void setX(float x);
    inline void setY(float y);

    inline float &rx();
    inline float &ry();

    inline PointF &operator+=(const PointF &p);
    inline PointF &operator-=(const PointF &p);
    inline PointF &operator*=(float c);
    inline PointF &operator/=(float c);

    static inline float dotProduct(const PointF &p1, const PointF &p2) { return p1.xp * p2.xp + p1.yp * p2.yp; }

    friend inline bool operator==(const PointF &, const PointF &);
    friend inline bool operator!=(const PointF &, const PointF &);
    friend inline const PointF operator+(const PointF &, const PointF &);
    friend inline const PointF operator-(const PointF &, const PointF &);
    friend inline const PointF operator*(float, const PointF &);
    friend inline const PointF operator*(const PointF &, float);
    friend inline const PointF operator+(const PointF &);
    friend inline const PointF operator-(const PointF &);
    friend inline const PointF operator/(const PointF &, float);

    Point toPoint() const;

private:
    float xp;
    float yp;
};

/*****************************************************************************
  PointF inline functions
 *****************************************************************************/

inline PointF::PointF()
    : xp(0)
    , yp(0)
{}

inline PointF::PointF(float xpos, float ypos)
    : xp(xpos)
    , yp(ypos)
{}

inline PointF::PointF(const Point &p)
    : xp(p.x())
    , yp(p.y())
{}

inline float PointF::manhattanLength() const
{
    return std::abs(x()) + std::abs(y());
}

inline bool PointF::isNull() const
{
    return Private::qIsNull(xp) && Private::qIsNull(yp);
}

inline float PointF::x() const
{
    return xp;
}

inline float PointF::y() const
{
    return yp;
}

inline void PointF::setX(float xpos)
{
    xp = xpos;
}

inline void PointF::setY(float ypos)
{
    yp = ypos;
}

inline float &PointF::rx()
{
    return xp;
}

inline float &PointF::ry()
{
    return yp;
}

inline PointF &PointF::operator+=(const PointF &p)
{
    xp += p.xp;
    yp += p.yp;
    return *this;
}

inline PointF &PointF::operator-=(const PointF &p)
{
    xp -= p.xp;
    yp -= p.yp;
    return *this;
}

inline PointF &PointF::operator*=(float c)
{
    xp *= c;
    yp *= c;
    return *this;
}

QUL_WARNING_PUSH
QUL_WARNING_DISABLE_CLANG("-Wfloat-equal")
QUL_WARNING_DISABLE_GCC("-Wfloat-equal")

inline bool operator==(const PointF &p1, const PointF &p2)
{
    return ((!p1.xp || !p2.xp) ? Private::qFuzzyIsNull(p1.xp - p2.xp) : Private::qFuzzyCompare(p1.xp, p2.xp))
           && ((!p1.yp || !p2.yp) ? Private::qFuzzyIsNull(p1.yp - p2.yp) : Private::qFuzzyCompare(p1.yp, p2.yp));
}

inline bool operator!=(const PointF &p1, const PointF &p2)
{
    return !(p1 == p2);
}

QUL_WARNING_POP

inline const PointF operator+(const PointF &p1, const PointF &p2)
{
    return PointF(p1.xp + p2.xp, p1.yp + p2.yp);
}

inline const PointF operator-(const PointF &p1, const PointF &p2)
{
    return PointF(p1.xp - p2.xp, p1.yp - p2.yp);
}

inline const PointF operator*(const PointF &p, float c)
{
    return PointF(p.xp * c, p.yp * c);
}

inline const PointF operator*(float c, const PointF &p)
{
    return PointF(p.xp * c, p.yp * c);
}

inline const PointF operator+(const PointF &p)
{
    return p;
}

inline const PointF operator-(const PointF &p)
{
    return PointF(-p.xp, -p.yp);
}

inline PointF &PointF::operator/=(float divisor)
{
    xp /= divisor;
    yp /= divisor;
    return *this;
}

inline const PointF operator/(const PointF &p, float divisor)
{
    return PointF(p.xp / divisor, p.yp / divisor);
}

inline Point PointF::toPoint() const
{
    return Point(Private::qRound(xp), Private::qRound(yp));
}

} // namespace PlatformInterface
} // namespace Qul
