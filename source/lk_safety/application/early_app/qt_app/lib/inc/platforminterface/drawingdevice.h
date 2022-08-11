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
#ifndef QUL_DRAWINGDEVICE_H
#define QUL_DRAWINGDEVICE_H

#include <qul/pixelformat.h>
#include <platforminterface/point.h>
#include <platforminterface/size.h>

class tst_DrawingDevice;

namespace Qul {
namespace PlatformInterface {

class DrawingEngine;
class DrawingDevice
{
public:
    DrawingDevice(PixelFormat format, const Size &size, uchar *bits, int32_t bytesPerLine, DrawingEngine *drawingEngine);

    PixelFormat format() const;
    const Size &size() const;
    int32_t width() const;
    int32_t height() const;
    int32_t bytesPerPixel() const;
    int32_t bytesPerLine() const;
    DrawingEngine *drawingEngine() const;
    DrawingEngine *fallbackDrawingEngine() const;

    uchar *scanLine(int32_t y) const;
    uchar *pixelAt(int32_t x, int32_t y) const;
    uchar *pixelAt(Point pos) const;
    uchar *bits() const;

    void setBits(uchar *bits);

private:
    PixelFormat m_format;
    Size m_size;
    uchar *m_bits;
    int32_t m_bytesPerLine;
    DrawingEngine *m_drawingEngine;
    DrawingEngine *m_fallbackDrawingEngine;

    friend class ::tst_DrawingDevice;
};

} // namespace PlatformInterface
} // namespace Qul

#endif // QUL_DRAWINGDEVICE_H
