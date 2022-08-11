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
#ifndef QUL_GRAPHICSDEVICE_H
#define QUL_GRAPHICSDEVICE_H

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

#include <qul/pixelformat.h>
#include <platforminterface/point.h>
#include <platforminterface/size.h>
#include <qul/private/scopedpointer.h>
#include <platforminterface/transform.h>
#include <qul/private/global.h>

#include <platforminterface/drawingdevice.h>
#include <platforminterface/texture.h>
#include <platforminterface/rgba32.h>

namespace Qul {
namespace Private {

class FontEngine;
struct RasterBuffer;

static const PixelFormat Custom = PixelFormat_Custom;
static const PixelFormat ARGB32 = PixelFormat_ARGB32;
static const PixelFormat ARGB32_Premultiplied = PixelFormat_ARGB32_Premultiplied;
static const PixelFormat RGB32 = PixelFormat_RGB32;
static const PixelFormat RGB888 = PixelFormat_RGB888;
static const PixelFormat RGB16 = PixelFormat_RGB16;
static const PixelFormat Alpha8 = PixelFormat_Alpha8;
static const PixelFormat ARGB4444 = PixelFormat_ARGB4444;
static const PixelFormat ARGB4444_Premultiplied = PixelFormat_ARGB4444_Premultiplied;
static const PixelFormat NumPixelFormats = PixelFormat_NumPixelFormats;

struct TextureLayout
{
    PixelFormat format;
    int8_t bytesPerPixel;
    // type erased function pointers
    void (*transformedFetch)(void);
};

template<PixelFormat>
const TextureLayout *getTextureLayout();

template<>
const TextureLayout *getTextureLayout<ARGB32>();
template<>
const TextureLayout *getTextureLayout<ARGB32_Premultiplied>();
template<>
const TextureLayout *getTextureLayout<RGB32>();
template<>
const TextureLayout *getTextureLayout<RGB16>();
template<>
const TextureLayout *getTextureLayout<Alpha8>();
template<>
const TextureLayout *getTextureLayout<ARGB4444>();
template<>
const TextureLayout *getTextureLayout<ARGB4444_Premultiplied>();
template<>
const TextureLayout *getTextureLayout<PixelFormat_Custom>();

inline bool hasAlpha(PixelFormat format)
{
    return format == ARGB32 || format == ARGB32_Premultiplied || format == Alpha8 || format == ARGB4444;
}

inline PlatformInterface::Rgba32 withAlpha(PlatformInterface::Rgba32 color, float alpha)
{
    return PlatformInterface::Rgba32(color.red(), color.green(), color.blue(), uint(qRound(color.alpha() * alpha)));
}

struct Texture;

extern const size_t ImageCacheSize;

class ImageCache
{
public:
    struct Node
    {
        unsigned int accessedFrame;
        const Texture *texture; // non-owning ptr, used to get Texture size and manage Texture::cachedData
        uchar *cachedData;
        Node *prev;
        Node *next;

        Node();
        ~Node();
    };

    ImageCache();
    ~ImageCache();

    Node *get(const Texture *texture);
    Node *find(const Texture *texture) const;

    static ImageCache *instance();

    inline size_t count() const { return m_count; }
    inline size_t size() const { return m_sizeInBytes; }

private:
    Node *cache(const Texture *texture);

    void freeAtLeast(const size_t size);

    void removeOldest();
    void remove(Node *node);

    inline void push_back(Node *node)
    {
        if (count() == 0) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            node->prev = tail;
            tail = node;
        }
        m_count++;
    }

    size_t m_count;
    size_t m_sizeInBytes;
    Node *head;
    Node *tail;
};

struct Texture
{
    const uchar *data;
    const TextureLayout *textureLayout;
    int flags; // flags from the PlatformInterface::Texture::Flags enum

    // the sub image that this Texture represents
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;

    int16_t bytesPerLine;

    uint32_t margins;
    int compressedBytes;

    // This field is sometimes mutable:
    // Textures that don't use the cache are marked "const" by qulrcc and
    // placed into immutable binary segments. Otherwise they're not const,
    // and this field may be mutated.
    // The field should not be marked "mutable" here - it prevents the compiler
    // from putting constants into .rodata or .text.
    ImageCache::Node *cachedData;

    PixelFormat format() const
    {
        assert(textureLayout);
        return textureLayout->format;
    }

    int bytesPerPixel() const
    {
        assert(textureLayout);
        return textureLayout->bytesPerPixel;
    }

    PlatformInterface::Texture platformTexture() const
    {
        int adjustedFlags = flags;
        if (margins)
            adjustedFlags |= PlatformInterface::Texture::PaddedWithTransparency;
        PlatformInterface::Texture
            texture(bits(), format(), adjustedFlags, width, height, bytesPerPixel(), bytesPerLine, textureLayout);
        return texture;
    }

    const uchar *bits() const;

    inline PlatformInterface::Rect rect() const { return PlatformInterface::Rect(x, y, width, height); }

    const uchar *offset(int16_t x, int16_t y) const
    {
        assert(x >= 0);
        assert(y >= 0);
        assert(x < width);
        assert(y < height);
        return bits() + y * bytesPerLine + x * bytesPerPixel();
    }

    PlatformInterface::Rect actualRect() const
    {
        PlatformInterface::Rect res = rect();
        if (margins != 0) {
            const quint8 left = (margins & 0xFF000000) >> 24;
            const quint8 top = (margins & 0x00FF0000) >> 16;
            const quint8 right = (margins & 0x0000FF00) >> 8;
            const quint8 bottom = margins & 0x000000FF;
            res.adjust(left, top, -right, -bottom);
        }
        return res;
    }

    inline bool isCompressed() const { return compressedBytes > 0; }

    uchar *cacheableData() const;
};

struct RasterBuffer
{
    PlatformInterface::Rgba32 color; // for Alpha8

    int width;
    int height;

    int textureCount;
    const Texture *textures;
    const Texture *begin() const { return textures; }
    const Texture *end() const { return textures + textureCount; }
};

class String;

struct BlitData
{
    PlatformInterface::Point source;
    PlatformInterface::Point destination;
    PlatformInterface::Size size;
};

struct AlphaMap
{
    const uchar *data;
    int16_t width;
    int16_t height;

    static const uint32_t defaultMarginValue = 0x01010101;
    uint32_t margins;

    inline PlatformInterface::Size size() const { return PlatformInterface::Size(width, height); }

    PlatformInterface::Rect contentRect() const
    {
        if (size().isNull()) {
            return PlatformInterface::Rect();
        }

        PlatformInterface::Rect res = PlatformInterface::Rect(0, 0, width, height);

        if (margins != 0) {
            const quint8 left = (margins & 0xFF000000) >> 24;
            const quint8 top = (margins & 0x00FF0000) >> 16;
            const quint8 right = (margins & 0x0000FF00) >> 8;
            const quint8 bottom = margins & 0x000000FF;
            res.adjust(left, top, -right, -bottom);
        }

        return res;
    }
};

#ifdef QUL_ENABLE_TEXT_CACHE
struct TextCacheEntry
{
    AlphaMap alphaMap;

    TextCacheEntry *next;
    TextCacheEntry *prev;

    unsigned int accessedFrame;

    struct TextCacheCleanup
    {
        static void cleanup(TextCacheEntry *);
    };

    typedef ScopedPointer<TextCacheEntry, TextCacheCleanup> TextCachePointer;

    TextCachePointer *pointer;

    static TextCacheEntry *textCache;
};
#endif

/** Draw to a DrawingDevice that backs part of the screen, with clipping. */
class GraphicsDevice
{
public:
    GraphicsDevice(PlatformInterface::DrawingDevice *buffer);

    void drawText(const FontEngine *engine,
                  PlatformInterface::Point destination,
                  String text,
                  PlatformInterface::Rgba32 color);
    void drawText(const FontEngine *engine,
                  const PlatformInterface::Transform &transform,
                  PlatformInterface::Point destination,
                  String text,
                  PlatformInterface::Rgba32 color);

#ifdef QUL_ENABLE_TEXT_CACHE
    static void cacheText(TextCacheEntry::TextCachePointer &result,
                          const FontEngine *engine,
                          String text,
                          const PlatformInterface::Rect &bounds);
#endif

    void blendA8(BlitData *blitData, int count, const AlphaMap &alphaMap, PlatformInterface::Rgba32 color);

    void blit(const PlatformInterface::Rect &rect, PlatformInterface::Rgba32 color);

    void blend(const PlatformInterface::Rect &rect, PlatformInterface::Rgba32 color, int radius = 0);

    void blend(PlatformInterface::Point destination,
               const PlatformInterface::Rect &sourceRect,
               const RasterBuffer &image,
               float opacity);

    void blend(const PlatformInterface::Rect &destinationRect,
               const PlatformInterface::Rect &sourceRect,
               const RasterBuffer &image,
               float opacity);

    void blendTransformed(const PlatformInterface::Transform &transform,
                          const PlatformInterface::RectF &sourceRect,
                          const RasterBuffer &image,
                          float opacity);

    void blendTransformedA8(const PlatformInterface::Transform &transform,
                            const PlatformInterface::RectF &sourceRect,
                            const AlphaMap &alphaMap,
                            PlatformInterface::Rgba32 color);

    void setClipRect(const PlatformInterface::Rect &clipRect)
    {
        assert(clipRect.left() >= 0);
        assert(clipRect.top() >= 0);
        assert(clipRect.right() < m_buffer->width());
        assert(clipRect.bottom() < m_buffer->height());
        m_clip = clipRect;
    }

    PlatformInterface::Rect clipRect() const { return m_clip; }

    float opacityThreshold() const;

    PlatformInterface::DrawingDevice *drawingDevice() const { return m_buffer; }

private:
    PlatformInterface::Rect clip(const PlatformInterface::Rect &rect) const;

    PlatformInterface::Rect m_clip;
    PlatformInterface::DrawingDevice *m_buffer;
};

} // namespace Private
} // namespace Qul

#endif
