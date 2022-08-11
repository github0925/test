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
#ifndef QUL_RGBA32_H
#define QUL_RGBA32_H

#include <qul/global.h>

namespace Qul {
namespace PlatformInterface {

struct Rgba32
{
    uint32_t value;

    // This expects an 0xAARRGGBB format.
    inline Rgba32(uint32_t v = 0)
        : value(v)
    {}

    inline explicit Rgba32(uint32_t r, uint32_t g, uint32_t b, uint32_t a = 255u) { setValue(r, g, b, a); }
    inline explicit Rgba32(float r, float g, float b, float a = 1.f)
    {
        setValue(floatToInt(r), floatToInt(g), floatToInt(b), floatToInt(a));
    }

    inline bool invisible() const { return alpha() == 0; }

    inline uint32_t red() const { return ((value >> 16) & 0xff); }
    inline void setRed(uint32_t r) { value = (value & 0xff00ffff) | ((r & 0xff) << 16); }

    inline uint32_t green() const { return ((value >> 8) & 0xff); }
    inline void setGreen(uint32_t g) { value = (value & 0xffff00ff) | ((g & 0xff) << 8); }

    inline uint32_t blue() const { return (value & 0xff); }
    inline void setBlue(uint32_t b) { value = (value & 0xffffff00) | (b & 0xff); }

    inline uint32_t alpha() const { return value >> 24; }
    inline void setAlpha(uint32_t a) { value = (value & 0x00ffffff) | ((a & 0xff) << 24); }

    inline Rgba32 premultiplied() const
    {
        const uint32_t a = alpha();
        if (a == 255)
            return *this;
        else if (a == 0)
            return Rgba32();

        uint32_t t = (value & 0xff00ff) * a;
        t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
        t &= 0xff00ff;

        uint32_t x = ((value >> 8) & 0xff) * a;
        x = (x + ((x >> 8) & 0xff) + 0x80);
        x &= 0xff00;
        return Rgba32(x | t | (a << 24));
    }

    friend inline bool operator==(const Rgba32 &a, const Rgba32 &b) { return a.value == b.value; }
    friend inline bool operator!=(const Rgba32 &a, const Rgba32 &b) { return a.value != b.value; }

protected:
    inline void setValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a = 255u)
    {
        value = ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
    }

    static uint32_t floatToInt(float f)
    {
        const uint32_t result = static_cast<uint32_t>((f < 0.f ? -f : f) * 256.f);
        return static_cast<uint32_t>(result <= 0 ? 0 : result > 255 ? 255 : result);
    }
};

} // namespace PlatformInterface
} // namespace Qul

#endif // QUL_RGBA32_H
