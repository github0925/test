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

#include <qul/pixelformat.h>
#include <qul/private/image_p.h>

#include <functional>

namespace Qul {

struct SharedImage
{
    SharedImage();

    // SharedImage reference counts
    ~SharedImage();
    SharedImage(const SharedImage &other);
    SharedImage(SharedImage &&other);
    SharedImage &operator=(const SharedImage &other);
    SharedImage &operator=(SharedImage &&other);

    explicit operator bool() const;

    Image *image() const;

private:
    explicit SharedImage(Private::ImageDataPtr ptr)
        : m_data(ptr)
    {}

    void deref();
    bool isReady() const;
    const Private::RasterBuffer *rasterBuffer() const;

    Private::ImageDataPtr m_data;

    friend struct Image;
    friend struct Private::PropertyTraits<SharedImage>;
    friend struct Private::ImagePrivate;
    friend bool operator==(const SharedImage &lhs, const SharedImage &rhs);
};

struct Image
{
    using CleanupFunction = std::function<void(uint8_t *memory)>;

    Image();

    Image(uint8_t *bits,
          int width,
          int height,
          PixelFormat pixelFormat,
          int bytesPerLine = -1,
          CleanupFunction cleanupFunction = nullptr);

    Image(int width, int height, PixelFormat pixelFormat);

    // non-copyable
    Image(const Image &) = delete;
    Image &operator=(const Image &) = delete;

    // movable
    Image(Image &&) = default;
    Image &operator=(Image &&) = default;

    operator SharedImage() const;

    int width() const;
    int height() const;
    PixelFormat pixelFormat() const;

    int bytesPerLine() const;
    int bytesPerPixel() const;
    const uint8_t *bits() const;
    uint8_t *bits();

    void beginWrite();
    void endWrite();

    void reallocate(int width, int height, PixelFormat pixelFormat);

    static const uintptr_t requiredAlignment;
    static const uintptr_t requiredPixelWidthAlignment;

protected:
    void assign(uint8_t *bits,
                int width,
                int height,
                PixelFormat pixelFormat,
                int bytesPerLine,
                CleanupFunction cleanupFunction);
    void assignNew(int width, int height, PixelFormat pixelFormat);

    SharedImage m_shared;

    friend struct SharedImage;
    friend struct ImageWriteGuard;
    friend bool operator==(const SharedImage &lhs, const SharedImage &rhs);
};

struct ImageWriteGuard
{
    explicit ImageWriteGuard(Image &image);
    ~ImageWriteGuard();

    // non-copyable, or move-assignalbe
    ImageWriteGuard(const ImageWriteGuard &) = delete;
    ImageWriteGuard &operator=(const ImageWriteGuard &) = delete;
    ImageWriteGuard &operator=(ImageWriteGuard &&) = delete;

    // move constructible
    ImageWriteGuard(ImageWriteGuard &&);

private:
    Image *m_image;
};

bool operator==(const SharedImage &lhs, const SharedImage &rhs);
inline bool operator!=(const SharedImage &lhs, const SharedImage &rhs)
{
    return !(lhs == rhs);
}

} // namespace Qul
