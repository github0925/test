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

#include <qul/property.h>
#include <qul/private/items.h>
#include <qul/private/rendercontext.h>
#include <qul/private/qulassert.h>

namespace Qul {
namespace Private {
namespace Items {

struct ItemLayer;
struct ImageLayer;
struct SpriteLayer;

struct LayerBase : ItemBase
{
    enum ExtraFlag {
        IsItemLayerExtraFlag = 1U << 0,
        IsImageLayerExtraFlag = 1U << 1,
        IsSpriteLayerExtraFlag = 1U << 2,
    };

    LayerBase()
    {
        setFlag(IsLayerFlag, true);
        setFlag(DirtyLayerFlag, true);
    }

    enum ColorDepth { Bpp16, Bpp16Alpha, Bpp24, Bpp32, Bpp32Alpha };
    enum RenderingHints {
        OptimizeForSpeed = 1U << 0,
        OptimizeForSize = 1U << 1,
        StaticContents = 1U << 2,
    };

protected:
    Qul::PlatformInterface::LayerEngine::ColorDepth toLayerEngineColorDepth(ColorDepth depth)
    {
        switch (depth) {
        case ColorDepth::Bpp16:
            return Qul::PlatformInterface::LayerEngine::Bpp16;
        case ColorDepth::Bpp16Alpha:
            return Qul::PlatformInterface::LayerEngine::Bpp16Alpha;
        case ColorDepth::Bpp24:
            return Qul::PlatformInterface::LayerEngine::Bpp24;
        case ColorDepth::Bpp32:
            return Qul::PlatformInterface::LayerEngine::Bpp32;
        case ColorDepth::Bpp32Alpha:
            return Qul::PlatformInterface::LayerEngine::Bpp32Alpha;
        default:
            QUL_UNREACHABLE();
            return Qul::PlatformInterface::LayerEngine::Bpp32Alpha;
        }
    }

    Qul::PlatformInterface::LayerEngine::RenderingHints toLayerEngineRenderingHints(RenderingHints hints)
    {
        switch (hints) {
        case RenderingHints::OptimizeForSpeed:
            return PlatformInterface::LayerEngine::OptimizeForSpeed;
        case RenderingHints::OptimizeForSize:
            return PlatformInterface::LayerEngine::OptimizeForSize;
        case RenderingHints::StaticContents:
            return PlatformInterface::LayerEngine::StaticContents;
        default:
            return static_cast<Qul::PlatformInterface::LayerEngine::RenderingHints>(static_cast<int>(hints));
        }
    }

    template<typename T>
    static T *as(ItemBase *item, LayerBase::ExtraFlag flag)
    {
        if (!item || !item->flag(IsLayerFlag))
            return nullptr;

        LayerBase *base = static_cast<LayerBase *>(item);
        if (base->extraFlags() & flag)
            return static_cast<T *>(item);

        return nullptr;
    }
};

struct ImageLayer : LayerBase
{
    ImageLayer()
        : LayerBase()
        , node(this)
    {
        extraFlags() |= IsImageLayerExtraFlag;

        onImageChanged.init<ImageLayer, &ImageLayer::onImageChanged>();
        onImageChanged.registerDependency(source);
    }

    Property<Qul::SharedImage> source;
    Property<int> platformId;
    Property<int> renderingHints;

    Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds) override
    {
        if (localBounds) {
            *localBounds = PlatformInterface::Rect(0, 0, width.value(), height.value());
        }

        if (update == UpdateRenderNode) {
            auto &sharedImage = source.value();
            auto rasterBuffer = Private::ImagePrivate::readyRasterBuffer(sharedImage);
            QUL_REQUIRE(rasterBuffer->begin() + 1 == rasterBuffer->end(),
                        "ImageLayer.source has to be single image. QUL_INTERNAL_SPLIT_IMAGE_OPTIMIZATION has to be "
                        "disabled for this image.");

            auto texture = Private::ImagePrivate::readyRasterBuffer(sharedImage)->begin();
            node.texture = texture->platformTexture();

            /*
                Since the hardware layer will asynchronously read directly from
                the shared image, we have to keep it around until the hardware
                layer unit no longer reads from the texture data.

                This might be as many as three presentFrame() calls after the
                render node gets updated. The first presentFrame() call will
                commit the shared image for display on the screen. If the source
                property then changes, the next presentFrame() will commit a
                different shared image for display. The commit is asynchronous
                however, so it's only once the third presentFrame() call happens
                that we know that the first shared image is no longer held.

                This shows how the current shared image will move through the
                pending image queue over three present frame calls:

                [,,SI] -> PF -> [,SI,] -> PF -> [SI,,] -> PF -> [,,]

                After three presentFrame() calls and the next
                Application::repaint() which calls renderNode(UpdateRenderNode),
                the shared image is no longer in the queue and it can be freed
                if there are no other references elsewhere.
            */

            for (int i = 0; i < NumPendingImagesInRenderQueue; ++i)
                pendingImages[i] = pendingImages[i + 1];
            pendingImages[NumPendingImagesInRenderQueue - 1] = sharedImage;

            node.platformId = platformId.value();
        }

        return &node;
    }

    static ImageLayer *asImageLayer(ItemBase *item) { return as<ImageLayer>(item, IsImageLayerExtraFlag); }

    Qul::Private::RenderTree::ImageLayerNode node;

private:
    struct ImageChangedEvent
    {
        void operator()(ImageLayer *image);
    };
    DirtyEventStaticDependency<ImageChangedEvent> onImageChanged;

    static constexpr int NumPendingImagesInRenderQueue = 3;
    Qul::SharedImage pendingImages[NumPendingImagesInRenderQueue];
};

struct ItemLayer : LayerBase
{
    ItemLayer()
        : LayerBase()
        , context(this)
        , node(this)
    {
        depth.setValue(Bpp32);
        renderingHints.setValue(static_cast<RenderingHints>(0));
        refreshInterval.setValue(1);
        extraFlags() |= IsItemLayerExtraFlag;
    }

    explicit ItemLayer(ItemBase *item)
        : ItemLayer()
    {
        appendChild(this, item);
        width.setValue(item->width.value());
        height.setValue(item->height.value());
    }

    Property<int> refreshInterval;
    Property<ColorDepth> depth;
    Property<int> platformId;
    Property<RenderingHints> renderingHints;

    Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds) override
    {
        if (localBounds) {
            *localBounds = PlatformInterface::Rect(0, 0, width.value(), height.value());
        }

        if (update == UpdateRenderNode) {
            node.colorDepth = toLayerEngineColorDepth(depth.value());
            node.renderingHints = toLayerEngineRenderingHints(renderingHints.value());
            node.platformId = platformId.value();
        }

        return &node;
    }

    PlatformInterface::Point childrenOffset() const override { return PlatformInterface::Point(); }

    RenderContext context;

    static ItemLayer *asItemLayer(ItemBase *item) { return LayerBase::as<ItemLayer>(item, IsItemLayerExtraFlag); }

    Qul::Private::RenderTree::ItemLayerNode node;
};

struct Screen;

struct DirtyBackgroundColorEvent
{
    void operator()(Screen *screen);
};

struct Screen : ItemBase
{
    Screen()
        : backgroundColor(0x0)
        , node(this)
    {
        onBackgroundColorChanged.init<Screen, &Screen::onBackgroundColorChanged>();
        onBackgroundColorChanged.registerDependency(backgroundColor);
    }

    explicit Screen(ItemLayer *item)
        : Screen()
    {
        appendChild(this, item);
    }

    Property<Qul::Private::String> outputDevice;
    Property<ULColor> backgroundColor;

    Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate, PlatformInterface::Rect *localBounds) override
    {
        if (localBounds) {
            *localBounds = PlatformInterface::Rect(0, 0, width.value(), height.value());
        }
        return &node;
    }

    DirtyEventStaticDependency<DirtyBackgroundColorEvent> onBackgroundColorChanged;

    Qul::Private::RenderTree::ScreenNode node;
};

struct Application : ItemBase
{
    Application()
        : node(this)
    {}

    explicit Application(Screen *item)
        : Application()
    {
        appendChild(this, item);
    }

    Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate, PlatformInterface::Rect *) override { return &node; }

    Qul::Private::RenderTree::ApplicationNode node;
};

struct SpriteLayer : LayerBase
{
    SpriteLayer()
        : LayerBase()
        , node(this)
    {
        depth.setValue(Bpp32Alpha);
        extraFlags() |= IsSpriteLayerExtraFlag;
    }

    Property<ColorDepth> depth;
    Property<int> platformId;

    Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds) override
    {
        if (localBounds) {
            *localBounds = PlatformInterface::Rect(0, 0, width.value(), height.value());
        }

        if (update == UpdateRenderNode) {
            node.colorDepth = toLayerEngineColorDepth(depth.value());
            node.platformId = platformId.value();
        }

        return &node;
    }

    static SpriteLayer *asSpriteLayer(ItemBase *item)
    {
        return LayerBase::as<SpriteLayer>(item, IsSpriteLayerExtraFlag);
    }

    PlatformInterface::Point childrenOffset() const override { return PlatformInterface::Point(); }

    Qul::Private::RenderTree::SpriteLayerNode node;
};

template<typename Iterator = Items::ItemChildrenFwIterator, typename F>
void forEachScreen(ItemBase *root, F &&functor)
{
    for (Iterator rootIt(root); rootIt; ++rootIt) {
        auto node = rootIt->getRenderNode();
        if (!node)
            continue;

        if (node->type != Qul::Private::RenderTree::Node::ScreenType)
            continue;

        Screen *screen = static_cast<Items::Screen *>(&*rootIt);
        functor(screen);
    }
}

template<typename Iterator = Items::ItemChildrenFwIterator, typename F>
Screen *findScreen(ItemBase *root, F &&functor)
{
    for (Iterator rootIt(root); rootIt; ++rootIt) {
        auto node = rootIt->getRenderNode();
        if (!node)
            continue;

        if (node->type != Qul::Private::RenderTree::Node::ScreenType)
            continue;

        Screen *screen = static_cast<Items::Screen *>(&*rootIt);
        if (functor(screen))
            return screen;
    }

    return nullptr;
}

Screen *screenForItem(Items::ItemBase *item);

template<typename Iterator = Items::ItemChildrenFwIterator, typename F>
void forEachLayer(ItemBase *root, F &&functor)
{
    for (Iterator rootIt(root); rootIt; ++rootIt) {
        if (!rootIt->flag(ItemBase::IsLayerFlag))
            continue;

        functor(&*rootIt);

        Items::SpriteLayer *spriteLayer = Items::SpriteLayer::asSpriteLayer(&*rootIt);
        if (spriteLayer) {
            for (Iterator childIt(spriteLayer); childIt; ++childIt) {
                if (!childIt->flag(ItemBase::IsLayerFlag))
                    continue;
                functor(&*childIt);
            }
        }
    }
}

template<typename Iterator = Items::ItemChildrenFwIterator, typename F>
ItemBase *findLayer(ItemBase *root, F &&functor)
{
    for (Iterator rootIt(root); rootIt; ++rootIt) {
        if (!rootIt->flag(ItemBase::IsLayerFlag))
            continue;
        if (functor(&*rootIt))
            return &*rootIt;

        Items::SpriteLayer *spriteLayer = Items::SpriteLayer::asSpriteLayer(&*rootIt);
        if (spriteLayer) {
            for (Iterator childIt(spriteLayer); childIt; ++childIt) {
                if (childIt->flag(ItemBase::IsLayerFlag) && functor(&*childIt))
                    return &*childIt;
            }
        }
    }

    return nullptr;
}

} // namespace Items
} // namespace Private
} // namespace Qul
