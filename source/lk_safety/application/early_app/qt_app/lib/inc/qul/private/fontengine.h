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
#ifndef QUL_FONTENGINE_H
#define QUL_FONTENGINE_H

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
#include <platforminterface/rect.h>
#include <qul/private/unicodestring.h>
#include <qul/private/graphicsdevice.h> // AlphaMap
#include <qul/private/fixednumber.h>

#include <limits>
#include <cstdio>

namespace Qul {
namespace Private {

typedef quint32 glyph_t;

struct glyph_metrics_t
{
    inline glyph_metrics_t()
        : x(100000)
        , y(100000)
    {}

    inline glyph_metrics_t(
        FixedNumber _x, FixedNumber _y, FixedNumber _width, FixedNumber _height, FixedNumber _xoff, FixedNumber _yoff)
        : x(_x)
        , y(_y)
        , width(_width)
        , height(_height)
        , xoff(_xoff)
        , yoff(_yoff)
    {}

    FixedNumber x;
    FixedNumber y;
    FixedNumber width;
    FixedNumber height;
    FixedNumber xoff;
    FixedNumber yoff;

    inline bool isValid() const { return x != 100000 && y != 100000; }

    inline FixedNumber leftBearing() const
    {
        if (!isValid())
            return FixedNumber();
        return x;
    }

    inline FixedNumber rightBearing() const
    {
        if (!isValid())
            return FixedNumber();
        return xoff - x - width;
    }

    PlatformInterface::Rect toRect() const
    {
        using PlatformInterface::Rect;

        if (!isValid())
            return Rect();

        const int upperLeftX = x.truncate();
        const int upperLeftY = y.truncate();
        const int bottomRightX = (x + width).ceil().truncate();
        const int bottomRightY = (y + height).ceil().truncate();

        assert(bottomRightX <= std::numeric_limits<Rect::CoordType>::max());
        assert(bottomRightY <= std::numeric_limits<Rect::CoordType>::max());

        const Rect::CoordType maxVal = std::numeric_limits<Rect::CoordType>::max();
        if (bottomRightX > maxVal || bottomRightY > maxVal) {
            std::printf("value overflow detected on text metrics\n");
            return Rect();
        }

        const PlatformInterface::Point ul(upperLeftX, upperLeftY);
        const PlatformInterface::Point br(bottomRightX, bottomRightY);
        return Rect(ul.x(), ul.y(), br.x() - ul.x(), br.y() - ul.y());
    }
};

class FontEngine
{
public:
    FontEngine() {}

    virtual FixedNumber kerning(glyph_t, glyph_t) const { return 0; }

    /*
     * Font compiler stores AlphaMap-s when generating static font
     * engine data. To avoid storing a pointer to native data in
     * AlphaMap, which is not used by static font engine, but is needed
     * only for Spark font engine, we can save memory by using nativeData
     * arg instead.
     */
    virtual AlphaMap alphaMapForGlyph(glyph_t g, void **nativeData) const = 0;
    virtual glyph_t glyphIndex(uint ucs4) const = 0;

    virtual glyph_metrics_t boundingBox(glyph_t glyph) const = 0;
    virtual glyph_metrics_t boundingBox(String text) const = 0;

    virtual FixedNumber advanceForGlyph(glyph_t g) const = 0;
    virtual FixedNumber ascent() const = 0;
    virtual FixedNumber descent() const = 0;
    virtual FixedNumber leading() const = 0;

    // Used by Spark font engine.
    virtual bool needsSyncAfterBlend() const { return false; }
    virtual void freeNativeData(void *) const {}
};

// provided by the fontcompiler
extern const FontEngine *const font_default;
extern const char *const qul_default_font_family;
extern const int qul_default_pixel_size;
extern const bool qul_using_spark_engine;

} // namespace Private
} // namespace Qul

#endif
