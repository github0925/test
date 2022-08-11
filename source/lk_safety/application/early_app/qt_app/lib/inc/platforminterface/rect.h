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

#include <qul/private/global.h> // for qRound
#include <platforminterface/size.h>
#include <platforminterface/point.h>

namespace Qul {
namespace PlatformInterface {

class Rect
{
public:
    Rect()
        : x1(0)
        , y1(0)
        , x2(-1)
        , y2(-1)
    {}
    Rect(const Point &topleft, const Point &bottomright);
    Rect(const Point &topleft, const Size &size);
    Rect(coord_t left, coord_t top, coord_t width, coord_t height);

    inline bool isNull() const;
    inline bool isEmpty() const;
    inline bool isValid() const;

    inline coord_t left() const;
    inline coord_t top() const;
    inline coord_t right() const;
    inline coord_t bottom() const;
    QUL_REQUIRED_RESULT Rect normalized() const;

    inline coord_t x() const;
    inline coord_t y() const;
    inline void setLeft(coord_t pos);
    inline void setTop(coord_t pos);
    inline void setRight(coord_t pos);
    inline void setBottom(coord_t pos);
    inline void setX(coord_t x);
    inline void setY(coord_t y);

    inline void setTopLeft(const Point &p);
    inline void setBottomRight(const Point &p);
    inline void setTopRight(const Point &p);
    inline void setBottomLeft(const Point &p);

    inline Point topLeft() const;
    inline Point bottomRight() const;
    inline Point topRight() const;
    inline Point bottomLeft() const;
    inline Point center() const;

    inline void moveLeft(coord_t pos);
    inline void moveTop(coord_t pos);
    inline void moveRight(coord_t pos);
    inline void moveBottom(coord_t pos);
    inline void moveTopLeft(const Point &p);
    inline void moveBottomRight(const Point &p);
    inline void moveTopRight(const Point &p);
    inline void moveBottomLeft(const Point &p);
    inline void moveCenter(const Point &p);

    inline void translate(coord_t dx, coord_t dy);
    inline void translate(const Point &p);
    QUL_REQUIRED_RESULT inline Rect translated(coord_t dx, coord_t dy) const;
    QUL_REQUIRED_RESULT inline Rect translated(const Point &p) const;
    QUL_REQUIRED_RESULT inline Rect transposed() const;

    inline void moveTo(coord_t x, coord_t t);
    inline void moveTo(const Point &p);

    inline void setRect(coord_t x, coord_t y, coord_t w, coord_t h);
    inline void getRect(coord_t *x, coord_t *y, coord_t *w, coord_t *h) const;

    inline void setCoords(coord_t x1, coord_t y1, coord_t x2, coord_t y2);
    inline void getCoords(coord_t *x1, coord_t *y1, coord_t *x2, coord_t *y2) const;

    inline void adjust(coord_t x1, coord_t y1, coord_t x2, coord_t y2);
    QUL_REQUIRED_RESULT inline Rect adjusted(coord_t x1, coord_t y1, coord_t x2, coord_t y2) const;

    inline Size size() const;
    inline coord_t width() const;
    inline coord_t height() const;
    inline void setWidth(coord_t w);
    inline void setHeight(coord_t h);
    inline void setSize(const Size &s);

    Rect operator|(const Rect &r) const;
    Rect operator&(const Rect &r) const;
    inline Rect &operator|=(const Rect &r);
    inline Rect &operator&=(const Rect &r);

    bool contains(const Rect &r, bool proper = false) const;
    bool contains(const Point &p, bool proper = false) const;
    inline bool contains(coord_t x, coord_t y) const;
    inline bool contains(coord_t x, coord_t y, bool proper) const;
    QUL_REQUIRED_RESULT inline Rect united(const Rect &other) const;
    QUL_REQUIRED_RESULT inline Rect intersected(const Rect &other) const;
    bool intersects(const Rect &r) const;

    friend inline bool operator==(const Rect &, const Rect &);
    friend inline bool operator!=(const Rect &, const Rect &);

    typedef coord_t CoordType;

private:
    coord_t x1;
    coord_t y1;
    coord_t x2;
    coord_t y2;
};

inline bool operator==(const Rect &, const Rect &);
inline bool operator!=(const Rect &, const Rect &);

/*****************************************************************************
  Rect inline member functions
 *****************************************************************************/

inline Rect::Rect(coord_t aleft, coord_t atop, coord_t awidth, coord_t aheight)
    : x1(aleft)
    , y1(atop)
    , x2(aleft + awidth - 1)
    , y2(atop + aheight - 1)
{}

inline Rect::Rect(const Point &atopLeft, const Point &abottomRight)
    : x1(atopLeft.x())
    , y1(atopLeft.y())
    , x2(abottomRight.x())
    , y2(abottomRight.y())
{}

inline Rect::Rect(const Point &atopLeft, const Size &asize)
    : x1(atopLeft.x())
    , y1(atopLeft.y())
    , x2(atopLeft.x() + asize.width() - 1)
    , y2(atopLeft.y() + asize.height() - 1)
{}

inline bool Rect::isNull() const
{
    // Conversion to coord_t required. Otherwise int is assumed on host platform
    // which will have different overflow values in tests.
    return x2 == x1 - coord_t(1) && y2 == y1 - coord_t(1);
}

inline bool Rect::isEmpty() const
{
    return x1 > x2 || y1 > y2;
}

inline bool Rect::isValid() const
{
    return x1 <= x2 && y1 <= y2;
}

inline coord_t Rect::left() const
{
    return x1;
}

inline coord_t Rect::top() const
{
    return y1;
}

inline coord_t Rect::right() const
{
    return x2;
}

inline coord_t Rect::bottom() const
{
    return y2;
}

inline coord_t Rect::x() const
{
    return x1;
}

inline coord_t Rect::y() const
{
    return y1;
}

inline void Rect::setLeft(coord_t pos)
{
    x1 = pos;
}

inline void Rect::setTop(coord_t pos)
{
    y1 = pos;
}

inline void Rect::setRight(coord_t pos)
{
    x2 = pos;
}

inline void Rect::setBottom(coord_t pos)
{
    y2 = pos;
}

inline void Rect::setTopLeft(const Point &p)
{
    x1 = p.x();
    y1 = p.y();
}

inline void Rect::setBottomRight(const Point &p)
{
    x2 = p.x();
    y2 = p.y();
}

inline void Rect::setTopRight(const Point &p)
{
    x2 = p.x();
    y1 = p.y();
}

inline void Rect::setBottomLeft(const Point &p)
{
    x1 = p.x();
    y2 = p.y();
}

inline void Rect::setX(coord_t ax)
{
    x1 = ax;
}

inline void Rect::setY(coord_t ay)
{
    y1 = ay;
}

inline Point Rect::topLeft() const
{
    return Point(x1, y1);
}

inline Point Rect::bottomRight() const
{
    return Point(x2, y2);
}

inline Point Rect::topRight() const
{
    return Point(x2, y1);
}

inline Point Rect::bottomLeft() const
{
    return Point(x1, y2);
}

inline Point Rect::center() const
{
    return Point(coord_t((int64_t(x1) + x2) / 2), coord_t((int64_t(y1) + y2) / 2));
} // cast avoids overflow on addition

inline coord_t Rect::width() const
{
    return x2 - x1 + 1;
}

inline coord_t Rect::height() const
{
    return y2 - y1 + 1;
}

inline Size Rect::size() const
{
    return Size(width(), height());
}

inline void Rect::translate(coord_t dx, coord_t dy)
{
    x1 += dx;
    y1 += dy;
    x2 += dx;
    y2 += dy;
}

inline void Rect::translate(const Point &p)
{
    x1 += p.x();
    y1 += p.y();
    x2 += p.x();
    y2 += p.y();
}

inline Rect Rect::translated(coord_t dx, coord_t dy) const
{
    return Rect(Point(x1 + dx, y1 + dy), Point(x2 + dx, y2 + dy));
}

inline Rect Rect::translated(const Point &p) const
{
    return Rect(Point(x1 + p.x(), y1 + p.y()), Point(x2 + p.x(), y2 + p.y()));
}

inline Rect Rect::transposed() const
{
    return Rect(topLeft(), size().transposed());
}

inline void Rect::moveTo(coord_t ax, coord_t ay)
{
    x2 += ax - x1;
    y2 += ay - y1;
    x1 = ax;
    y1 = ay;
}

inline void Rect::moveTo(const Point &p)
{
    x2 += p.x() - x1;
    y2 += p.y() - y1;
    x1 = p.x();
    y1 = p.y();
}

inline void Rect::moveLeft(coord_t pos)
{
    x2 += (pos - x1);
    x1 = pos;
}

inline void Rect::moveTop(coord_t pos)
{
    y2 += (pos - y1);
    y1 = pos;
}

inline void Rect::moveRight(coord_t pos)
{
    x1 += (pos - x2);
    x2 = pos;
}

inline void Rect::moveBottom(coord_t pos)
{
    y1 += (pos - y2);
    y2 = pos;
}

inline void Rect::moveTopLeft(const Point &p)
{
    moveLeft(p.x());
    moveTop(p.y());
}

inline void Rect::moveBottomRight(const Point &p)
{
    moveRight(p.x());
    moveBottom(p.y());
}

inline void Rect::moveTopRight(const Point &p)
{
    moveRight(p.x());
    moveTop(p.y());
}

inline void Rect::moveBottomLeft(const Point &p)
{
    moveLeft(p.x());
    moveBottom(p.y());
}

inline void Rect::moveCenter(const Point &p)
{
    coord_t w = x2 - x1;
    coord_t h = y2 - y1;
    x1 = p.x() - w / 2;
    y1 = p.y() - h / 2;
    x2 = x1 + w;
    y2 = y1 + h;
}

inline void Rect::getRect(coord_t *ax, coord_t *ay, coord_t *aw, coord_t *ah) const
{
    *ax = x1;
    *ay = y1;
    *aw = x2 - x1 + 1;
    *ah = y2 - y1 + 1;
}

inline void Rect::setRect(coord_t ax, coord_t ay, coord_t aw, coord_t ah)
{
    x1 = ax;
    y1 = ay;
    x2 = (ax + aw - 1);
    y2 = (ay + ah - 1);
}

inline void Rect::getCoords(coord_t *xp1, coord_t *yp1, coord_t *xp2, coord_t *yp2) const
{
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

inline void Rect::setCoords(coord_t xp1, coord_t yp1, coord_t xp2, coord_t yp2)
{
    x1 = xp1;
    y1 = yp1;
    x2 = xp2;
    y2 = yp2;
}

inline Rect Rect::adjusted(coord_t xp1, coord_t yp1, coord_t xp2, coord_t yp2) const
{
    return Rect(Point(x1 + xp1, y1 + yp1), Point(x2 + xp2, y2 + yp2));
}

inline void Rect::adjust(coord_t dx1, coord_t dy1, coord_t dx2, coord_t dy2)
{
    x1 += dx1;
    y1 += dy1;
    x2 += dx2;
    y2 += dy2;
}

inline void Rect::setWidth(coord_t w)
{
    x2 = (x1 + w - 1);
}

inline void Rect::setHeight(coord_t h)
{
    y2 = (y1 + h - 1);
}

inline void Rect::setSize(const Size &s)
{
    x2 = (s.width() + x1 - 1);
    y2 = (s.height() + y1 - 1);
}

inline bool Rect::contains(coord_t ax, coord_t ay, bool aproper) const
{
    return contains(Point(ax, ay), aproper);
}

inline bool Rect::contains(coord_t ax, coord_t ay) const
{
    return contains(Point(ax, ay), false);
}

inline Rect &Rect::operator|=(const Rect &r)
{
    *this = *this | r;
    return *this;
}

inline Rect &Rect::operator&=(const Rect &r)
{
    *this = *this & r;
    return *this;
}

inline Rect Rect::intersected(const Rect &other) const
{
    return *this & other;
}

inline Rect Rect::united(const Rect &r) const
{
    return *this | r;
}

inline bool operator==(const Rect &r1, const Rect &r2)
{
    return r1.x1 == r2.x1 && r1.x2 == r2.x2 && r1.y1 == r2.y1 && r1.y2 == r2.y2;
}

inline bool operator!=(const Rect &r1, const Rect &r2)
{
    return r1.x1 != r2.x1 || r1.x2 != r2.x2 || r1.y1 != r2.y1 || r1.y2 != r2.y2;
}

class RectF
{
public:
    RectF()
        : xp(0.)
        , yp(0.)
        , w(0.)
        , h(0.)
    {}
    RectF(const PointF &topleft, const SizeF &size);
    RectF(const PointF &topleft, const PointF &bottomRight);
    RectF(float left, float top, float width, float height);
    RectF(const Rect &rect);

    inline bool isNull() const;
    inline bool isEmpty() const;
    inline bool isValid() const;
    QUL_REQUIRED_RESULT RectF normalized() const;

    inline float left() const { return xp; }
    inline float top() const { return yp; }
    inline float right() const { return xp + w; }
    inline float bottom() const { return yp + h; }

    inline float x() const;
    inline float y() const;
    inline void setLeft(float pos);
    inline void setTop(float pos);
    inline void setRight(float pos);
    inline void setBottom(float pos);
    inline void setX(float pos) { setLeft(pos); }
    inline void setY(float pos) { setTop(pos); }

    inline PointF topLeft() const { return PointF(xp, yp); }
    inline PointF bottomRight() const { return PointF(xp + w, yp + h); }
    inline PointF topRight() const { return PointF(xp + w, yp); }
    inline PointF bottomLeft() const { return PointF(xp, yp + h); }
    inline PointF center() const;

    inline void setTopLeft(const PointF &p);
    inline void setBottomRight(const PointF &p);
    inline void setTopRight(const PointF &p);
    inline void setBottomLeft(const PointF &p);

    inline void moveLeft(float pos);
    inline void moveTop(float pos);
    inline void moveRight(float pos);
    inline void moveBottom(float pos);
    inline void moveTopLeft(const PointF &p);
    inline void moveBottomRight(const PointF &p);
    inline void moveTopRight(const PointF &p);
    inline void moveBottomLeft(const PointF &p);
    inline void moveCenter(const PointF &p);

    inline void translate(float dx, float dy);
    inline void translate(const PointF &p);

    QUL_REQUIRED_RESULT inline RectF translated(float dx, float dy) const;
    QUL_REQUIRED_RESULT inline RectF translated(const PointF &p) const;

    QUL_REQUIRED_RESULT inline RectF transposed() const;

    inline void moveTo(float x, float y);
    inline void moveTo(const PointF &p);

    inline void setRect(float x, float y, float w, float h);
    inline void getRect(float *x, float *y, float *w, float *h) const;

    inline void setCoords(float x1, float y1, float x2, float y2);
    inline void getCoords(float *x1, float *y1, float *x2, float *y2) const;

    inline void adjust(float x1, float y1, float x2, float y2);
    QUL_REQUIRED_RESULT inline RectF adjusted(float x1, float y1, float x2, float y2) const;

    inline SizeF size() const;
    inline float width() const;
    inline float height() const;
    inline void setWidth(float w);
    inline void setHeight(float h);
    inline void setSize(const SizeF &s);

    RectF operator|(const RectF &r) const;
    RectF operator&(const RectF &rectangle) const;
    inline RectF &operator|=(const RectF &r);
    inline RectF &operator&=(const RectF &r);

    bool contains(const RectF &rectangle) const;
    bool contains(const PointF &p) const;
    inline bool contains(float x, float y) const;
    QUL_REQUIRED_RESULT inline RectF united(const RectF &other) const;
    QUL_REQUIRED_RESULT inline RectF intersected(const RectF &other) const;
    bool intersects(const RectF &rectangle) const;

    friend inline bool operator==(const RectF &, const RectF &);
    friend inline bool operator!=(const RectF &, const RectF &);

    QUL_REQUIRED_RESULT inline Rect toRect() const;
    QUL_REQUIRED_RESULT Rect toAlignedRect() const;

private:
    float xp;
    float yp;
    float w;
    float h;
};

inline bool operator==(const RectF &, const RectF &);
inline bool operator!=(const RectF &, const RectF &);

/*****************************************************************************
  RectF inline member functions
 *****************************************************************************/

inline RectF::RectF(float aleft, float atop, float awidth, float aheight)
    : xp(aleft)
    , yp(atop)
    , w(awidth)
    , h(aheight)
{}

inline RectF::RectF(const PointF &atopLeft, const SizeF &asize)
    : xp(atopLeft.x())
    , yp(atopLeft.y())
    , w(asize.width())
    , h(asize.height())
{}

inline RectF::RectF(const PointF &atopLeft, const PointF &abottomRight)
    : xp(atopLeft.x())
    , yp(atopLeft.y())
    , w(abottomRight.x() - atopLeft.x())
    , h(abottomRight.y() - atopLeft.y())
{}

inline RectF::RectF(const Rect &r)
    : xp(r.x())
    , yp(r.y())
    , w(r.width())
    , h(r.height())
{}

QUL_WARNING_PUSH
QUL_WARNING_DISABLE_CLANG("-Wfloat-equal")
QUL_WARNING_DISABLE_GCC("-Wfloat-equal")

inline bool RectF::isNull() const
{
    return w == 0. && h == 0.;
}

inline bool RectF::isEmpty() const
{
    return w <= 0. || h <= 0.;
}

QUL_WARNING_POP

inline bool RectF::isValid() const
{
    return w > 0. && h > 0.;
}

inline float RectF::x() const
{
    return xp;
}

inline float RectF::y() const
{
    return yp;
}

inline void RectF::setLeft(float pos)
{
    float diff = pos - xp;
    xp += diff;
    w -= diff;
}

inline void RectF::setRight(float pos)
{
    w = pos - xp;
}

inline void RectF::setTop(float pos)
{
    float diff = pos - yp;
    yp += diff;
    h -= diff;
}

inline void RectF::setBottom(float pos)
{
    h = pos - yp;
}

inline void RectF::setTopLeft(const PointF &p)
{
    setLeft(p.x());
    setTop(p.y());
}

inline void RectF::setTopRight(const PointF &p)
{
    setRight(p.x());
    setTop(p.y());
}

inline void RectF::setBottomLeft(const PointF &p)
{
    setLeft(p.x());
    setBottom(p.y());
}

inline void RectF::setBottomRight(const PointF &p)
{
    setRight(p.x());
    setBottom(p.y());
}

inline PointF RectF::center() const
{
    return PointF(xp + w / 2, yp + h / 2);
}

inline void RectF::moveLeft(float pos)
{
    xp = pos;
}

inline void RectF::moveTop(float pos)
{
    yp = pos;
}

inline void RectF::moveRight(float pos)
{
    xp = pos - w;
}

inline void RectF::moveBottom(float pos)
{
    yp = pos - h;
}

inline void RectF::moveTopLeft(const PointF &p)
{
    moveLeft(p.x());
    moveTop(p.y());
}

inline void RectF::moveTopRight(const PointF &p)
{
    moveRight(p.x());
    moveTop(p.y());
}

inline void RectF::moveBottomLeft(const PointF &p)
{
    moveLeft(p.x());
    moveBottom(p.y());
}

inline void RectF::moveBottomRight(const PointF &p)
{
    moveRight(p.x());
    moveBottom(p.y());
}

inline void RectF::moveCenter(const PointF &p)
{
    xp = p.x() - w / 2;
    yp = p.y() - h / 2;
}

inline float RectF::width() const
{
    return w;
}

inline float RectF::height() const
{
    return h;
}

inline SizeF RectF::size() const
{
    return SizeF(w, h);
}

inline void RectF::translate(float dx, float dy)
{
    xp += dx;
    yp += dy;
}

inline void RectF::translate(const PointF &p)
{
    xp += p.x();
    yp += p.y();
}

inline void RectF::moveTo(float ax, float ay)
{
    xp = ax;
    yp = ay;
}

inline void RectF::moveTo(const PointF &p)
{
    xp = p.x();
    yp = p.y();
}

inline RectF RectF::translated(float dx, float dy) const
{
    return RectF(xp + dx, yp + dy, w, h);
}

inline RectF RectF::translated(const PointF &p) const
{
    return RectF(xp + p.x(), yp + p.y(), w, h);
}

inline RectF RectF::transposed() const
{
    return RectF(topLeft(), size().transposed());
}

inline void RectF::getRect(float *ax, float *ay, float *aaw, float *aah) const
{
    *ax = this->xp;
    *ay = this->yp;
    *aaw = this->w;
    *aah = this->h;
}

inline void RectF::setRect(float ax, float ay, float aaw, float aah)
{
    this->xp = ax;
    this->yp = ay;
    this->w = aaw;
    this->h = aah;
}

inline void RectF::getCoords(float *xp1, float *yp1, float *xp2, float *yp2) const
{
    *xp1 = xp;
    *yp1 = yp;
    *xp2 = xp + w;
    *yp2 = yp + h;
}

inline void RectF::setCoords(float xp1, float yp1, float xp2, float yp2)
{
    xp = xp1;
    yp = yp1;
    w = xp2 - xp1;
    h = yp2 - yp1;
}

inline void RectF::adjust(float xp1, float yp1, float xp2, float yp2)
{
    xp += xp1;
    yp += yp1;
    w += xp2 - xp1;
    h += yp2 - yp1;
}

inline RectF RectF::adjusted(float xp1, float yp1, float xp2, float yp2) const
{
    return RectF(xp + xp1, yp + yp1, w + xp2 - xp1, h + yp2 - yp1);
}

inline void RectF::setWidth(float aw)
{
    this->w = aw;
}

inline void RectF::setHeight(float ah)
{
    this->h = ah;
}

inline void RectF::setSize(const SizeF &s)
{
    w = s.width();
    h = s.height();
}

inline bool RectF::contains(float ax, float ay) const
{
    return contains(PointF(ax, ay));
}

inline RectF &RectF::operator|=(const RectF &r)
{
    *this = *this | r;
    return *this;
}

inline RectF &RectF::operator&=(const RectF &r)
{
    *this = *this & r;
    return *this;
}

inline RectF RectF::intersected(const RectF &r) const
{
    return *this & r;
}

inline RectF RectF::united(const RectF &r) const
{
    return *this | r;
}

inline bool operator==(const RectF &r1, const RectF &r2)
{
    return Private::qFuzzyCompare(r1.xp, r2.xp) && Private::qFuzzyCompare(r1.yp, r2.yp)
           && Private::qFuzzyCompare(r1.w, r2.w) && Private::qFuzzyCompare(r1.h, r2.h);
}

inline bool operator!=(const RectF &r1, const RectF &r2)
{
    return !Private::qFuzzyCompare(r1.xp, r2.xp) || !Private::qFuzzyCompare(r1.yp, r2.yp)
           || !Private::qFuzzyCompare(r1.w, r2.w) || !Private::qFuzzyCompare(r1.h, r2.h);
}

inline Rect RectF::toRect() const
{
    return Rect(Point(Private::qRound(xp), Private::qRound(yp)),
                Point(Private::qRound(xp + w) - 1, Private::qRound(yp + h) - 1));
}

} // namespace PlatformInterface
} // namespace Qul
