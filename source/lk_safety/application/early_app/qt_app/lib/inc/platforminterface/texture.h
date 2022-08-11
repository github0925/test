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
#ifndef QUL_TEXTURE_H
#define QUL_TEXTURE_H

#include <qul/pixelformat.h>
#include <platforminterface/size.h>

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace Qul {

namespace Private {
struct TextureLayout;
} // namespace Private

namespace PlatformInterface {

class Texture
{
public:
    Texture()
        : m_data(nullptr)
        , m_format(PixelFormat_Custom)
        , m_flags(0)
        , m_width(0)
        , m_height(0)
        , m_bytesPerPixel(0)
        , m_bytesPerLine(0)
        , m_textureLayout(nullptr)
    {}

    Texture(const unsigned char *data,
            Qul::PixelFormat format,
            int flags,
            int16_t width,
            int16_t height,
            int8_t bytesPerPixel,
            int16_t bytesPerLine,
            const Private::TextureLayout *textureLayout = NULL)
        : m_data(data)
        , m_format(format)
        , m_flags(flags)
        , m_width(width)
        , m_height(height)
        , m_bytesPerPixel(bytesPerPixel)
        , m_bytesPerLine(bytesPerLine)
        , m_textureLayout(textureLayout)
    {}

    enum Flags {
        NoTextureFlags = 0x00,
        Swizzled = 0x01,
        PartiallyOverlaps = 0x02, // Texture might partially overlap another texture in a RasterBuffer
        NeedsCachedLoadFromSlowMemory = 0x04,
        PaddedWithTransparency = 0x08 // Texture is padded with a border of transparency at least a pixel wide
    };

    const unsigned char *data() const { return m_data; }
    Qul::PixelFormat format() const { return m_format; }
    int flags() const { return m_flags; }
    int8_t bytesPerPixel() const { return m_bytesPerPixel; }
    int16_t bytesPerLine() const { return m_bytesPerLine; }

    Size size() const { return Size(m_width, m_height); }

    int16_t width() const { return m_width; }
    int16_t height() const { return m_height; }

    const unsigned char *dataAtOffset(int16_t x, int16_t y) const
    {
        assert(x >= 0);
        assert(y >= 0);
        assert(x < m_width);
        assert(y < m_height);
        return m_data + y * m_bytesPerLine + x * m_bytesPerPixel;
    }

    const Private::TextureLayout *textureLayout() const { return m_textureLayout; }

private:
    const unsigned char *m_data;
    Qul::PixelFormat m_format;
    int m_flags; // flags from the Flags enum

    int16_t m_width;
    int16_t m_height;

    int8_t m_bytesPerPixel;
    int16_t m_bytesPerLine;

    const Private::TextureLayout *m_textureLayout;
};

} // namespace PlatformInterface
} // namespace Qul

#endif // QUL_TEXTURE_H
