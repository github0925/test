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
#include <cmath>
#include <algorithm>

namespace Qul {
namespace PlatformInterface {

class Size
{
public:
    Size();
    Size(coord_t w, coord_t h);

    inline bool isNull() const;
    inline bool isEmpty() const;
    inline bool isValid() const;

    inline coord_t width() const;
    inline coord_t height() const;
    inline void setWidth(coord_t w);
    inline void setHeight(coord_t h);
    QUL_REQUIRED_RESULT inline Size transposed() const;

    QUL_REQUIRED_RESULT inline Size expandedTo(const Size &) const;
    QUL_REQUIRED_RESULT inline Size boundedTo(const Size &) const;

    inline Size &operator+=(const Size &);
    inline Size &operator-=(const Size &);
    inline Size &operator*=(float c);
    inline Size &operator/=(float c);

    friend inline bool operator==(const Size &, const Size &);
    friend inline bool operator!=(const Size &, const Size &);
    friend inline const Size operator+(const Size &, const Size &);
    friend inline const Size operator-(const Size &, const Size &);
    friend inline const Size operator*(const Size &, float);
    friend inline const Size operator*(float, const Size &);
    friend inline const Size operator/(const Size &, float);

private:
    coord_t wd;
    coord_t ht;
};

/*****************************************************************************
  Size inline functions
 *****************************************************************************/

inline Size::Size()
    : wd(-1)
    , ht(-1)
{}

inline Size::Size(coord_t w, coord_t h)
    : wd(w)
    , ht(h)
{}

inline bool Size::isNull() const
{
    return wd == 0 && ht == 0;
}

inline bool Size::isEmpty() const
{
    return wd < 1 || ht < 1;
}

inline bool Size::isValid() const
{
    return wd >= 0 && ht >= 0;
}

inline coord_t Size::width() const
{
    return wd;
}

inline coord_t Size::height() const
{
    return ht;
}

inline void Size::setWidth(coord_t w)
{
    wd = w;
}

inline void Size::setHeight(coord_t h)
{
    ht = h;
}

inline Size Size::transposed() const
{
    return Size(ht, wd);
}

inline Size &Size::operator+=(const Size &s)
{
    wd += s.wd;
    ht += s.ht;
    return *this;
}

inline Size &Size::operator-=(const Size &s)
{
    wd -= s.wd;
    ht -= s.ht;
    return *this;
}

inline Size &Size::operator*=(float c)
{
    wd = Private::qRound(wd * c);
    ht = Private::qRound(ht * c);
    return *this;
}

inline bool operator==(const Size &s1, const Size &s2)
{
    return s1.wd == s2.wd && s1.ht == s2.ht;
}

inline bool operator!=(const Size &s1, const Size &s2)
{
    return s1.wd != s2.wd || s1.ht != s2.ht;
}

inline const Size operator+(const Size &s1, const Size &s2)
{
    return Size(s1.wd + s2.wd, s1.ht + s2.ht);
}

inline const Size operator-(const Size &s1, const Size &s2)
{
    return Size(s1.wd - s2.wd, s1.ht - s2.ht);
}

inline const Size operator*(const Size &s, float c)
{
    return Size(Private::qRound(s.wd * c), Private::qRound(s.ht * c));
}

inline const Size operator*(float c, const Size &s)
{
    return Size(Private::qRound(s.wd * c), Private::qRound(s.ht * c));
}

inline Size &Size::operator/=(float c)
{
    assert(!Private::qFuzzyIsNull(c));
    wd = Private::qRound(wd / c);
    ht = Private::qRound(ht / c);
    return *this;
}

inline const Size operator/(const Size &s, float c)
{
    assert(!Private::qFuzzyIsNull(c));
    return Size(Private::qRound(s.wd / c), Private::qRound(s.ht / c));
}

inline Size Size::expandedTo(const Size &otherSize) const
{
    return Size(std::max(wd, otherSize.wd), std::max(ht, otherSize.ht));
}

inline Size Size::boundedTo(const Size &otherSize) const
{
    return Size(std::min(wd, otherSize.wd), std::min(ht, otherSize.ht));
}

class SizeF
{
public:
    SizeF();
    SizeF(const Size &sz);
    SizeF(float w, float h);

    inline bool isNull() const;
    inline bool isEmpty() const;
    inline bool isValid() const;

    inline float width() const;
    inline float height() const;
    inline void setWidth(float w);
    inline void setHeight(float h);
    QUL_REQUIRED_RESULT inline SizeF transposed() const;

    QUL_REQUIRED_RESULT inline SizeF expandedTo(const SizeF &) const;
    QUL_REQUIRED_RESULT inline SizeF boundedTo(const SizeF &) const;

    inline SizeF &operator+=(const SizeF &);
    inline SizeF &operator-=(const SizeF &);
    inline SizeF &operator*=(float c);
    inline SizeF &operator/=(float c);

    friend inline bool operator==(const SizeF &, const SizeF &);
    friend inline bool operator!=(const SizeF &, const SizeF &);
    friend inline const SizeF operator+(const SizeF &, const SizeF &);
    friend inline const SizeF operator-(const SizeF &, const SizeF &);
    friend inline const SizeF operator*(const SizeF &, float);
    friend inline const SizeF operator*(float, const SizeF &);
    friend inline const SizeF operator/(const SizeF &, float);

    inline Size toSize() const;

private:
    float wd;
    float ht;
};

/*****************************************************************************
  SizeF inline functions
 *****************************************************************************/

inline SizeF::SizeF()
    : wd(-1.)
    , ht(-1.)
{}

inline SizeF::SizeF(const Size &sz)
    : wd(sz.width())
    , ht(sz.height())
{}

inline SizeF::SizeF(float w, float h)
    : wd(w)
    , ht(h)
{}

inline bool SizeF::isNull() const
{
    return Private::qIsNull(wd) && Private::qIsNull(ht);
}

inline bool SizeF::isEmpty() const
{
    return wd <= 0. || ht <= 0.;
}

inline bool SizeF::isValid() const
{
    return wd >= 0. && ht >= 0.;
}

inline float SizeF::width() const
{
    return wd;
}

inline float SizeF::height() const
{
    return ht;
}

inline void SizeF::setWidth(float w)
{
    wd = w;
}

inline void SizeF::setHeight(float h)
{
    ht = h;
}

inline SizeF SizeF::transposed() const
{
    return SizeF(ht, wd);
}

inline SizeF &SizeF::operator+=(const SizeF &s)
{
    wd += s.wd;
    ht += s.ht;
    return *this;
}

inline SizeF &SizeF::operator-=(const SizeF &s)
{
    wd -= s.wd;
    ht -= s.ht;
    return *this;
}

inline SizeF &SizeF::operator*=(float c)
{
    wd *= c;
    ht *= c;
    return *this;
}

inline bool operator==(const SizeF &s1, const SizeF &s2)
{
    return Private::qFuzzyCompare(s1.wd, s2.wd) && Private::qFuzzyCompare(s1.ht, s2.ht);
}

inline bool operator!=(const SizeF &s1, const SizeF &s2)
{
    return !Private::qFuzzyCompare(s1.wd, s2.wd) || !Private::qFuzzyCompare(s1.ht, s2.ht);
}

inline const SizeF operator+(const SizeF &s1, const SizeF &s2)
{
    return SizeF(s1.wd + s2.wd, s1.ht + s2.ht);
}

inline const SizeF operator-(const SizeF &s1, const SizeF &s2)
{
    return SizeF(s1.wd - s2.wd, s1.ht - s2.ht);
}

inline const SizeF operator*(const SizeF &s, float c)
{
    return SizeF(s.wd * c, s.ht * c);
}

inline const SizeF operator*(float c, const SizeF &s)
{
    return SizeF(s.wd * c, s.ht * c);
}

inline SizeF &SizeF::operator/=(float c)
{
    assert(!Private::qFuzzyIsNull(c));
    wd = wd / c;
    ht = ht / c;
    return *this;
}

inline const SizeF operator/(const SizeF &s, float c)
{
    assert(!Private::qFuzzyIsNull(c));
    return SizeF(s.wd / c, s.ht / c);
}

inline SizeF SizeF::expandedTo(const SizeF &otherSize) const
{
    return SizeF(std::max(wd, otherSize.wd), std::max(ht, otherSize.ht));
}

inline SizeF SizeF::boundedTo(const SizeF &otherSize) const
{
    return SizeF(std::min(wd, otherSize.wd), std::min(ht, otherSize.ht));
}

inline Size SizeF::toSize() const
{
    return Size(Private::qRound(wd), Private::qRound(ht));
}

} // namespace PlatformInterface
} // namespace Qul
