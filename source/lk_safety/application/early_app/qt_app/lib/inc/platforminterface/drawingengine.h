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
#ifndef QUL_DRAWINGENGINE_H
#define QUL_DRAWINGENGINE_H

#include <platforminterface/rect.h>
#include <platforminterface/rgba32.h>
#include <platforminterface/texture.h>
#include <platforminterface/transform.h>

namespace Qul {
namespace PlatformInterface {

class DrawingDevice;

class DrawingEngine
{
public:
    enum BlendMode { BlendMode_SourceOver, BlendMode_Source };

    virtual void blendRect(DrawingDevice *drawingDevice,
                           const Rect &rect,
                           Rgba32 color,
                           BlendMode blendMode = BlendMode_SourceOver);
    virtual void blendRoundedRect(DrawingDevice *drawingDevice,
                                  const PlatformInterface::Rect &rect,
                                  const PlatformInterface::Rect &clipRect,
                                  PlatformInterface::Rgba32 color,
                                  int radius,
                                  BlendMode blendMode = BlendMode_SourceOver);
    virtual void blendImage(DrawingDevice *drawingDevice,
                            const Point &pos,
                            const Texture &source,
                            const Rect &sourceRect,
                            int sourceOpacity,
                            BlendMode blendMode = BlendMode_SourceOver);
    virtual void blendAlphaMap(DrawingDevice *drawingDevice,
                               const Point &pos,
                               const Texture &source,
                               const Rect &sourceRect,
                               Rgba32 color,
                               BlendMode blendMode = BlendMode_SourceOver);
    virtual void blendTransformedImage(DrawingDevice *drawingDevice,
                                       const Transform &transform,
                                       const RectF &destinationRect,
                                       const Texture &source,
                                       const RectF &sourceRect,
                                       const Rect &clipRect,
                                       int sourceOpacity,
                                       BlendMode blendMode = BlendMode_SourceOver);
    virtual void blendTransformedAlphaMap(DrawingDevice *drawingDevice,
                                          const Transform &transform,
                                          const RectF &destinationRect,
                                          const Texture &source,
                                          const RectF &sourceRect,
                                          const Rect &clipRect,
                                          Rgba32 color,
                                          BlendMode blendMode = BlendMode_SourceOver);
    virtual void synchronizeForCpuAccess(DrawingDevice *drawingDevice, const Rect &rect);
};

} // namespace PlatformInterface
} // namespace Qul

#endif // QUL_DRAWINGENGINE_H
