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
#ifndef QUL_RENDERTREE_H
#define QUL_RENDERTREE_H

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

#include <platform/platform.h>
#include <platforminterface/rect.h>
#include <platforminterface/transform.h>
#include <platforminterface/layerengine.h>
#include <qul/private/unicodestring.h>
#include <qul/private/extradatapointer.h>

#include <cstdio>

namespace Qul {
namespace Private {

class GraphicsDevice;
class FontEngine;
struct StaticTextItem;
namespace Items {
struct ItemBase;
}

struct TransformStorage
{
    TransformStorage() {}
    explicit TransformStorage(PlatformInterface::Transform *t)
        : transform(*t)
    {}

    PlatformInterface::Transform transform;
};

namespace RenderTree {

class Node
{
public:
    enum Type {
        NodeType,
        ImageNodeType,
        RectangleNodeType,
        TextNodeType,
        StaticTextNodeType,
        ItemBufferNodeType,
        ScreenType,
        ApplicationType,
        ItemLayerType,
        ImageLayerType,
        SpriteLayerType
    };

    Node(Type t, Items::ItemBase *item_)
        : item(item_)
        , opacity(1)
        , type(t)
        , clipsChildren(false)
        , significantEmptyParts(false)
        , hasExternalChildren(false)
    {}

    ~Node();

    static const Type classType = NodeType;

    template<class T>
    T *as()
    {
        if (T::classType == type)
            return static_cast<T *>(this);
        return 0;
    }

    // in screen coordinates
    PlatformInterface::Rect subTreeBounds;

    Items::ItemBase *item;
    float opacity;

#ifdef _MSC_VER
    // By default enum for MSVC is a signed type, so one more bit is needed.
    Type type : 5;
#else
    Type type : 4;
#endif
    uint clipsChildren : 1;
    uint significantEmptyParts : 1;
    // Whether this node has children that are located outside its own bounds.
    // Needed to disable optimizations that skip redrawing under opaque nodes.
    // When there are external children, whole subTreeBounds may not be opaque
    // even though node itself is.
    uint hasExternalChildren : 1;
};

template<typename PlatformLayerType>
class LayerNode : public Node
{
public:
    LayerNode(Type t, Items::ItemBase *item)
        : Node(t, item)
        , platformId(0)
        , m_instance(nullptr)
    {}

    virtual ~LayerNode(){};

    void setPlatformInstance(PlatformLayerType *instance) { m_instance = instance; }
    PlatformLayerType *platformInstance() const { return m_instance; }

    int platformId;

private:
    PlatformLayerType *m_instance;
    bool m_initialized;
};

template<>
inline LayerNode<PlatformInterface::LayerEngine::ItemLayer>::~LayerNode()
{
    if (m_instance)
        Platform::layerEngine()->deallocateItemLayer(m_instance);
    m_instance = nullptr;
}

template<>
inline LayerNode<PlatformInterface::LayerEngine::ImageLayer>::~LayerNode()
{
    if (m_instance)
        Platform::layerEngine()->deallocateImageLayer(m_instance);
    m_instance = nullptr;
}

template<>
inline LayerNode<PlatformInterface::LayerEngine::SpriteLayer>::~LayerNode()
{
    if (m_instance)
        Platform::layerEngine()->deallocateSpriteLayer(m_instance);
    m_instance = nullptr;
}

class ImageLayerNode : public LayerNode<PlatformInterface::LayerEngine::ImageLayer>
{
public:
    ImageLayerNode(Items::ItemBase *item)
        : LayerNode(ImageLayerType, item)
    {}

    static const Type classType = ImageLayerType;

    PlatformInterface::Texture texture;
    Qul::PlatformInterface::LayerEngine::RenderingHints renderingHints
        = PlatformInterface::LayerEngine::RenderingHints::OptimizeForSize;
};

class ItemLayerNode : public LayerNode<PlatformInterface::LayerEngine::ItemLayer>
{
public:
    ItemLayerNode(Items::ItemBase *item)
        : LayerNode(ItemLayerType, item)
    {}

    static const Type classType = ItemLayerType;

    Qul::PlatformInterface::LayerEngine::ColorDepth colorDepth = Qul::PlatformInterface::LayerEngine::Bpp32Alpha;
    Qul::PlatformInterface::LayerEngine::RenderingHints renderingHints
        = PlatformInterface::LayerEngine::RenderingHints::OptimizeForSpeed;
};

class ScreenNode : public Node
{
public:
    ScreenNode(Items::ItemBase *item)
        : Node(ScreenType, item)
    {}

    PlatformInterface::Screen *platformScreen = nullptr;

    Items::ItemBase *mouseGrabber = nullptr;
    Items::ItemBase *touchItem = nullptr;

    bool backgroundColorDirty = false;

    static const Type classType = ScreenType;
};

class ApplicationNode : public Node
{
public:
    ApplicationNode(Items::ItemBase *item)
        : Node(ApplicationType, item)
    {}

    static const Type classType = ApplicationType;
};

class SpriteLayerNode : public LayerNode<PlatformInterface::LayerEngine::SpriteLayer>
{
public:
    SpriteLayerNode(Items::ItemBase *item)
        : LayerNode(SpriteLayerType, item)
    {}

    static const Type classType = SpriteLayerType;

    Qul::PlatformInterface::LayerEngine::ColorDepth colorDepth = Qul::PlatformInterface::LayerEngine::Bpp32Alpha;
};

// Node for image-like items (Image or BorderImage)
class ImageNode : public Node
{
public:
    explicit inline ImageNode(Items::ItemBase *item_)
        : Node(ImageNodeType, item_)
        , rasterBuffer(NULL)
    {
        hAlignment = vAlignment = AlignMiddle;
        fillMode = Stretch;
        hasColor = false;
        hasBorder = false;
    }
    static const Type classType = ImageNodeType;

    // Match Items::Image::HAlignment and Items::Image::VAlignment
    enum Alignment { AlignBegin, AlignEnd, AlignMiddle };
    // Matches Items::Image::FillMode
    enum FillMode { Stretch, PreserveAspectFit, PreserveAspectCrop, Tile, TileVertically, TileHorizontally, Pad };

    Alignment hAlignment : 4;
    Alignment vAlignment : 4;
    FillMode fillMode : 6;

    bool hasColor : 1;  // `item` is a ColorizedImage
    bool hasBorder : 1; // `item` is a BorderImage

    const RasterBuffer *rasterBuffer;
    ExtraDataPointer<TransformStorage> transform;

    struct TilingInfo
    {
        // offset at which to draw the tiles (in destination coordinate)
        PlatformInterface::coord_t offX;
        PlatformInterface::coord_t offY;
        // Scale factor for the image
        float scalex;
        float scaley;
        // true if there should be tiles
        bool tile;
    };
    TilingInfo computeTilingInfo() const;

    bool isOpaque() const;
};

class ItemBufferNode : public ImageNode
{
public:
    explicit inline ItemBufferNode(Items::ItemBase *item_)
        : ImageNode(item_)
    {
        type = ItemBufferNodeType;
        fillMode = Pad;
    }
    static const Type classType = ItemBufferNodeType;
};

// `item` is a Text
class TextNode : public Node
{
public:
    explicit TextNode(Items::ItemBase *item_)
        : Node(TextNodeType, item_)
    {}

    static const Type classType = TextNodeType;

    ExtraDataPointer<TransformStorage> transform;

    void updateCache();
    void freeCache()
    {
#ifdef QUL_ENABLE_TEXT_CACHE
        cache.reset();
#endif
    }

    bool hasCache() const
    {
#ifdef QUL_ENABLE_TEXT_CACHE
        return cache;
#else
        return false;
#endif
    }

private:
#ifdef QUL_ENABLE_TEXT_CACHE
    TextCacheEntry::TextCachePointer cache;
#endif

    friend class Renderer;
};

class StaticTextNode : public Node
{
public:
    explicit StaticTextNode(Items::ItemBase *item_)
        : Node(StaticTextNodeType, item_)
    {}

    static const Type classType = StaticTextNodeType;

    PlatformInterface::Point delta;

    PlatformInterface::Rgba32 color;

    ExtraDataPointer<TransformStorage> transform;

    const StaticTextItem *textItem;
};

// `item` is a Rectangle
class RectangleNode : public Node
{
public:
    explicit RectangleNode(Items::ItemBase *item_)
        : Node(RectangleNodeType, item_)
    {}
    static const Type classType = RectangleNodeType;
};

class Renderer
{
public:
    Renderer(GraphicsDevice *device);

    void render(Node *root, PlatformInterface::Rect *rectsBegin, PlatformInterface::Rect *rectsEnd, int stride);

    void render(const PlatformInterface::Point &offset,
                Node *node,
                PlatformInterface::Rect *rectsBegin,
                PlatformInterface::Rect *rectsEnd,
                int stride);

private:
    void blendImage(const PlatformInterface::Point &nodeToGlobal,
                    const RasterBuffer &withColor,
                    ImageNode *imageNode,
                    PlatformInterface::Rect source,
                    PlatformInterface::Rect dest);

    void renderImageNode(ImageNode *node, PlatformInterface::Point nodeToGlobal);
    void renderRectangleNode(RectangleNode *node, PlatformInterface::Point nodeToGlobal);
    void renderStaticTextNode(StaticTextNode *node, PlatformInterface::Point nodeToGlobal);
    void renderTextNode(TextNode *node, PlatformInterface::Point nodeToGlobal);

    GraphicsDevice *m_dev;

    struct RenderVisitor;
};

} // namespace RenderTree
} // namespace Private
} // namespace Qul

#endif
