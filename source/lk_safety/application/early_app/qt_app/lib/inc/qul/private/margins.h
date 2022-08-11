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

namespace Qul {
namespace Private {

/*****************************************************************************
  Margins class
 *****************************************************************************/

class Margins
{
public:
    Margins();
    Margins(coord_t left, coord_t top, coord_t right, coord_t bottom);

    bool isNull() const;

    coord_t left() const;
    coord_t top() const;
    coord_t right() const;
    coord_t bottom() const;

    void setLeft(coord_t left);
    void setTop(coord_t top);
    void setRight(coord_t right);
    void setBottom(coord_t bottom);

    Margins &operator+=(const Margins &margins);
    Margins &operator-=(const Margins &margins);
    Margins &operator+=(coord_t);
    Margins &operator-=(coord_t);
    Margins &operator*=(coord_t);
    Margins &operator/=(coord_t);
    Margins &operator*=(float);
    Margins &operator/=(float);

private:
    coord_t m_left;
    coord_t m_top;
    coord_t m_right;
    coord_t m_bottom;

    friend inline bool operator==(const Margins &, const Margins &);
    friend inline bool operator!=(const Margins &, const Margins &);
};

/*****************************************************************************
  Margins stream functions
 *****************************************************************************/

/*****************************************************************************
  Margins inline functions
 *****************************************************************************/

inline Margins::Margins()
    : m_left(0)
    , m_top(0)
    , m_right(0)
    , m_bottom(0)
{}

inline Margins::Margins(coord_t aleft, coord_t atop, coord_t aright, coord_t abottom)
    : m_left(aleft)
    , m_top(atop)
    , m_right(aright)
    , m_bottom(abottom)
{}

inline bool Margins::isNull() const
{
    return m_left == 0 && m_top == 0 && m_right == 0 && m_bottom == 0;
}

inline coord_t Margins::left() const
{
    return m_left;
}

inline coord_t Margins::top() const
{
    return m_top;
}

inline coord_t Margins::right() const
{
    return m_right;
}

inline coord_t Margins::bottom() const
{
    return m_bottom;
}

inline void Margins::setLeft(coord_t aleft)
{
    m_left = aleft;
}

inline void Margins::setTop(coord_t atop)
{
    m_top = atop;
}

inline void Margins::setRight(coord_t aright)
{
    m_right = aright;
}

inline void Margins::setBottom(coord_t abottom)
{
    m_bottom = abottom;
}

inline bool operator==(const Margins &m1, const Margins &m2)
{
    return m1.m_left == m2.m_left && m1.m_top == m2.m_top && m1.m_right == m2.m_right && m1.m_bottom == m2.m_bottom;
}

inline bool operator!=(const Margins &m1, const Margins &m2)
{
    return m1.m_left != m2.m_left || m1.m_top != m2.m_top || m1.m_right != m2.m_right || m1.m_bottom != m2.m_bottom;
}

inline Margins operator+(const Margins &m1, const Margins &m2)
{
    return Margins(m1.left() + m2.left(), m1.top() + m2.top(), m1.right() + m2.right(), m1.bottom() + m2.bottom());
}

inline Margins operator-(const Margins &m1, const Margins &m2)
{
    return Margins(m1.left() - m2.left(), m1.top() - m2.top(), m1.right() - m2.right(), m1.bottom() - m2.bottom());
}

inline Margins operator+(const Margins &lhs, coord_t rhs)
{
    return Margins(lhs.left() + rhs, lhs.top() + rhs, lhs.right() + rhs, lhs.bottom() + rhs);
}

inline Margins operator+(coord_t lhs, const Margins &rhs)
{
    return Margins(rhs.left() + lhs, rhs.top() + lhs, rhs.right() + lhs, rhs.bottom() + lhs);
}

inline Margins operator-(const Margins &lhs, coord_t rhs)
{
    return Margins(lhs.left() - rhs, lhs.top() - rhs, lhs.right() - rhs, lhs.bottom() - rhs);
}

inline Margins operator*(const Margins &margins, coord_t factor)
{
    return Margins(margins.left() * factor, margins.top() * factor, margins.right() * factor, margins.bottom() * factor);
}

inline Margins operator*(coord_t factor, const Margins &margins)
{
    return Margins(margins.left() * factor, margins.top() * factor, margins.right() * factor, margins.bottom() * factor);
}

inline Margins operator*(const Margins &margins, float factor)
{
    return Margins(qRound(margins.left() * factor),
                   qRound(margins.top() * factor),
                   qRound(margins.right() * factor),
                   qRound(margins.bottom() * factor));
}

inline Margins operator*(float factor, const Margins &margins)
{
    return Margins(qRound(margins.left() * factor),
                   qRound(margins.top() * factor),
                   qRound(margins.right() * factor),
                   qRound(margins.bottom() * factor));
}

inline Margins operator/(const Margins &margins, coord_t divisor)
{
    return Margins(margins.left() / divisor,
                   margins.top() / divisor,
                   margins.right() / divisor,
                   margins.bottom() / divisor);
}

inline Margins operator/(const Margins &margins, float divisor)
{
    return Margins(qRound(margins.left() / divisor),
                   qRound(margins.top() / divisor),
                   qRound(margins.right() / divisor),
                   qRound(margins.bottom() / divisor));
}

inline Margins &Margins::operator+=(const Margins &margins)
{
    return *this = *this + margins;
}

inline Margins &Margins::operator-=(const Margins &margins)
{
    return *this = *this - margins;
}

inline Margins &Margins::operator+=(coord_t margin)
{
    m_left += margin;
    m_top += margin;
    m_right += margin;
    m_bottom += margin;
    return *this;
}

inline Margins &Margins::operator-=(coord_t margin)
{
    m_left -= margin;
    m_top -= margin;
    m_right -= margin;
    m_bottom -= margin;
    return *this;
}

inline Margins &Margins::operator*=(coord_t factor)
{
    return *this = *this * factor;
}

inline Margins &Margins::operator/=(coord_t divisor)
{
    return *this = *this / divisor;
}

inline Margins &Margins::operator*=(float factor)
{
    return *this = *this * factor;
}

inline Margins &Margins::operator/=(float divisor)
{
    return *this = *this / divisor;
}

inline Margins operator+(const Margins &margins)
{
    return margins;
}

inline Margins operator-(const Margins &margins)
{
    return Margins(-margins.left(), -margins.top(), -margins.right(), -margins.bottom());
}

/*****************************************************************************
  MarginsF class
 *****************************************************************************/

class MarginsF
{
public:
    MarginsF();
    MarginsF(float left, float top, float right, float bottom);
    MarginsF(const Margins &margins);

    bool isNull() const;

    float left() const;
    float top() const;
    float right() const;
    float bottom() const;

    void setLeft(float left);
    void setTop(float top);
    void setRight(float right);
    void setBottom(float bottom);

    MarginsF &operator+=(const MarginsF &margins);
    MarginsF &operator-=(const MarginsF &margins);
    MarginsF &operator+=(float addend);
    MarginsF &operator-=(float subtrahend);
    MarginsF &operator*=(float factor);
    MarginsF &operator/=(float divisor);

    inline Margins toMargins() const;

private:
    float m_left;
    float m_top;
    float m_right;
    float m_bottom;
};

/*****************************************************************************
  MarginsF stream functions
 *****************************************************************************/

/*****************************************************************************
  MarginsF inline functions
 *****************************************************************************/

inline MarginsF::MarginsF()
    : m_left(0)
    , m_top(0)
    , m_right(0)
    , m_bottom(0)
{}

inline MarginsF::MarginsF(float aleft, float atop, float aright, float abottom)
    : m_left(aleft)
    , m_top(atop)
    , m_right(aright)
    , m_bottom(abottom)
{}

inline MarginsF::MarginsF(const Margins &margins)
    : m_left(margins.left())
    , m_top(margins.top())
    , m_right(margins.right())
    , m_bottom(margins.bottom())
{}

inline bool MarginsF::isNull() const
{
    return qFuzzyIsNull(m_left) && qFuzzyIsNull(m_top) && qFuzzyIsNull(m_right) && qFuzzyIsNull(m_bottom);
}

inline float MarginsF::left() const
{
    return m_left;
}

inline float MarginsF::top() const
{
    return m_top;
}

inline float MarginsF::right() const
{
    return m_right;
}

inline float MarginsF::bottom() const
{
    return m_bottom;
}

inline void MarginsF::setLeft(float aleft)
{
    m_left = aleft;
}

inline void MarginsF::setTop(float atop)
{
    m_top = atop;
}

inline void MarginsF::setRight(float aright)
{
    m_right = aright;
}

inline void MarginsF::setBottom(float abottom)
{
    m_bottom = abottom;
}

inline bool operator==(const MarginsF &lhs, const MarginsF &rhs)
{
    return qFuzzyCompare(lhs.left(), rhs.left()) && qFuzzyCompare(lhs.top(), rhs.top())
           && qFuzzyCompare(lhs.right(), rhs.right()) && qFuzzyCompare(lhs.bottom(), rhs.bottom());
}

inline bool operator!=(const MarginsF &lhs, const MarginsF &rhs)
{
    return !operator==(lhs, rhs);
}

inline MarginsF operator+(const MarginsF &lhs, const MarginsF &rhs)
{
    return MarginsF(lhs.left() + rhs.left(),
                    lhs.top() + rhs.top(),
                    lhs.right() + rhs.right(),
                    lhs.bottom() + rhs.bottom());
}

inline MarginsF operator-(const MarginsF &lhs, const MarginsF &rhs)
{
    return MarginsF(lhs.left() - rhs.left(),
                    lhs.top() - rhs.top(),
                    lhs.right() - rhs.right(),
                    lhs.bottom() - rhs.bottom());
}

inline MarginsF operator+(const MarginsF &lhs, float rhs)
{
    return MarginsF(lhs.left() + rhs, lhs.top() + rhs, lhs.right() + rhs, lhs.bottom() + rhs);
}

inline MarginsF operator+(float lhs, const MarginsF &rhs)
{
    return MarginsF(rhs.left() + lhs, rhs.top() + lhs, rhs.right() + lhs, rhs.bottom() + lhs);
}

inline MarginsF operator-(const MarginsF &lhs, float rhs)
{
    return MarginsF(lhs.left() - rhs, lhs.top() - rhs, lhs.right() - rhs, lhs.bottom() - rhs);
}

inline MarginsF operator*(const MarginsF &lhs, float rhs)
{
    return MarginsF(lhs.left() * rhs, lhs.top() * rhs, lhs.right() * rhs, lhs.bottom() * rhs);
}

inline MarginsF operator*(float lhs, const MarginsF &rhs)
{
    return MarginsF(rhs.left() * lhs, rhs.top() * lhs, rhs.right() * lhs, rhs.bottom() * lhs);
}

inline MarginsF operator/(const MarginsF &lhs, float divisor)
{
    return MarginsF(lhs.left() / divisor, lhs.top() / divisor, lhs.right() / divisor, lhs.bottom() / divisor);
}

inline MarginsF &MarginsF::operator+=(const MarginsF &margins)
{
    return *this = *this + margins;
}

inline MarginsF &MarginsF::operator-=(const MarginsF &margins)
{
    return *this = *this - margins;
}

inline MarginsF &MarginsF::operator+=(float addend)
{
    m_left += addend;
    m_top += addend;
    m_right += addend;
    m_bottom += addend;
    return *this;
}

inline MarginsF &MarginsF::operator-=(float subtrahend)
{
    m_left -= subtrahend;
    m_top -= subtrahend;
    m_right -= subtrahend;
    m_bottom -= subtrahend;
    return *this;
}

inline MarginsF &MarginsF::operator*=(float factor)
{
    return *this = *this * factor;
}

inline MarginsF &MarginsF::operator/=(float divisor)
{
    return *this = *this / divisor;
}

inline MarginsF operator+(const MarginsF &margins)
{
    return margins;
}

inline MarginsF operator-(const MarginsF &margins)
{
    return MarginsF(-margins.left(), -margins.top(), -margins.right(), -margins.bottom());
}

inline Margins MarginsF::toMargins() const
{
    return Margins(qRound(m_left), qRound(m_top), qRound(m_right), qRound(m_bottom));
}

} // namespace Private
} // namespace Qul
