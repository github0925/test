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

#include <platforminterface/size.h>
#include <platforminterface/point.h>
#include <platforminterface/texture.h>
#include <platforminterface/transform.h>

#include <qul/pixelformat.h>

namespace Qul {
namespace PlatformInterface {

class DrawingDevice;
class Screen;

class LayerEngine
{
public:
    enum ColorDepth { Bpp16, Bpp16Alpha, Bpp24, Bpp32, Bpp32Alpha };
    enum RenderingHints {
        OptimizeForSpeed = 1U << 0,
        OptimizeForSize = 1U << 1,
        StaticContents = 1U << 2,
    };

    struct ItemLayer
    {};
    struct ImageLayer
    {};
    struct SpriteLayer
    {};

    struct LayerPropertiesBase
    {
        int z;
        bool enabled;
        float opacity;
        Point position;
        int id;
        RenderingHints renderingHints;
    };

    struct SizedLayerPropertiesBase : LayerPropertiesBase
    {
        Size size;
        ColorDepth colorDepth;
    };

    struct ItemLayerProperties : SizedLayerPropertiesBase
    {};

    struct ImageLayerProperties : LayerPropertiesBase
    {
        ImageLayerProperties(const Texture &t = Texture())
            : texture(t)
        {}

        Texture texture;
    };

    struct SpriteLayerProperties : SizedLayerPropertiesBase
    {};

    virtual ItemLayer *allocateItemLayer(const Screen *screen,
                                         const ItemLayerProperties &props,
                                         SpriteLayer *spriteLayer = nullptr);
    virtual ImageLayer *allocateImageLayer(const Screen *screen,
                                           const ImageLayerProperties &props,
                                           SpriteLayer *spriteLayer = nullptr);
    virtual SpriteLayer *allocateSpriteLayer(const Screen *screen, const SpriteLayerProperties &props);

    virtual void deallocateItemLayer(ItemLayer *layer);
    virtual void deallocateImageLayer(ImageLayer *layer);
    virtual void deallocateSpriteLayer(SpriteLayer *layer);

    virtual void updateItemLayer(ItemLayer *layer, const ItemLayerProperties &props);
    virtual void updateImageLayer(ImageLayer *layer, const ImageLayerProperties &props);
    virtual void updateSpriteLayer(SpriteLayer *layer, const SpriteLayerProperties &props);
};

} // namespace PlatformInterface
} // namespace Qul
