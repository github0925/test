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

#include <qul/private/propertybinding.h>
#include <qul/private/input.h>
#include <qul/private/rendertree.h>
#include <platforminterface/transform.h>
#include <qul/private/matrix4x4.h>
#include <qul/private/dirtylist.h>
#include <qul/private/optional.h>
#include <qul/private/unicodestring.h>
#include <qul/private/mathfwd.h>
#include <qul/private/console.h>
#include <qul/private/attachedtype.h>
#include <qul/private/slot.h>
#include <qul/private/builtins.h>
#include <qul/private/timeritem.h>
#include <qul/private/anchors.h>
#include <qul/private/font.h>

#include <qul/singleton.h>
#include <qul/object.h>
#include <qul/signal.h>
#include <qul/global.h>
#include <qul/rootitem.h>
#include <qul/image.h>

#include <math.h>
#include <utility>
#include <ctime>
#include <algorithm>

#define UL_OFFSETOF(T, F) (static_cast<short>(reinterpret_cast<qintptr>(&(((T *) 0)->F))))

namespace Qul {
namespace Private {

class FontEngine;
class Application;

struct RasterBuffer;

namespace Items {

struct ItemBase;
struct LayoutItem;

struct RepaintEvent
{
    void operator()(ItemBase *item);
};

struct DirtySizeEvent
{
    void operator()(ItemBase *item);
};

struct DirtyEnabledEvent
{
    void operator()(ItemBase *item);
};

struct DirtyLayoutEvent
{
    void operator()(LayoutItem *item);
};

struct ComponentAttachedType : AttachedType<ComponentAttachedType>
{
    Signal<void()> completed;
};

struct Component : Qul::Object
{
    QUL_ATTACHED_PROPERTIES(ComponentAttachedType);
    // return true if at least one object's onCompleted method was called
    static bool emitCompleted();
};

struct KeysAttachedType : AttachedType<KeysAttachedType>
{
    Signal<void(Builtins::KeyEvent *event)> pressed;
    Signal<void(Builtins::KeyEvent *event)> released;

    bool keyEvent(struct Qul::Private::KeyEvent *event);
};

struct Keys : Qul::Object
{
    QUL_ATTACHED_PROPERTIES_ONLY(KeysAttachedType);
};

struct Connections : Qul::Object
{};

struct ItemBase : ParentObject, Qul::RootItem
{
    enum class ItemType : uint8_t {
        Normal,

        // See LayoutItem. Layouts need to be updated similarly to dirty events.
        LayoutItem,

        // An item that is only assigned to a parent for management reasons
        // and should be ignored in layout logic that depends on user-declared
        // children items. Example: The Repeater insertion placeholder should not
        // generate a page in SwipeView.
        Bookkeeping,

        // See ItemBuffer. Special rendering.
        ItemBuffer,
    };

    enum Flag {
        // Unused = 1U << 0,
        OverridesTouchEventFlag = 1U << 1,
        ChildOverridesTouchEventFlag = 1U << 2,
        HandleTouchEventBeforeChildrenFlag = 1U << 3,
        DirtyNodeFlag = 1U << 4,
        DirtySubTreeFlag = 1U << 5,
        ChildZOrderDirtyFlag = 1U << 6,
        // Unused = 1U << 7,
        // Unused = 1U << 8,
        IsExplicitlyDisabledFlag = 1U << 9,
        SettingEnabledFromParentFlag = 1U << 10,
        SkipRenderNode = 1U << 11,
        SkipChildrenRenderNode = 1U << 12,
        IsItemBufferChild = 1U << 13,
        IsLayerFlag = 1U << 14,
        DirtyLayerFlag = 1U << 15
        // up to 16 bits
    };

    inline ItemBase *parentItem() const
    {
        Object *p = parentObject();
        if (!p || p->type() != Type::Item)
            return NULL;
        return static_cast<ItemBase *>(p);
    }

    ItemBase *lastChildItem() const
    {
        return (firstChildItem != NULL) ? firstChildItem->prevCircularSiblingItem : NULL;
    }

    Property<int> x QUL_PROPERTY_DEBUG_INIT("Item::x");
    Property<int> y QUL_PROPERTY_DEBUG_INIT("Item::y");
    Property<int> width QUL_PROPERTY_DEBUG_INIT("Item::width");
    Property<int> height QUL_PROPERTY_DEBUG_INIT("Item::height");

    struct ItemExtraProperties
    {
        ItemBase *self;
        explicit ItemExtraProperties(ItemBase *self);
        Property<int> implicitWidth QUL_PROPERTY_DEBUG_INIT("Item::implicitWidth");
        Property<int> implicitHeight QUL_PROPERTY_DEBUG_INIT("Item::implicitHeight");
        Property<qreal> z;
        Property<bool> clip;

    private:
        struct ZEvent
        {
            void operator()(ItemBase::ItemExtraProperties *item);
        };
        struct DirtyImplicitSizeEvent
        {
            void operator()(ItemBase::ItemExtraProperties *item);
        };
        DirtyEventStaticDependency<ZEvent> onZChanged;
        DirtyEventStaticDependency<DirtyImplicitSizeEvent> onImplicitSizeChanged;
    };

    QUL_DECLARE_EXTRASTORAGE(ItemExtraProperties, itemExtraProperties);

    Property<bool> enabled;
    Property<bool> visible;
    Property<qreal> opacity;

    ItemBase *firstChildItem;
    ItemBase *nextCircularSiblingItem;
    ItemBase *prevCircularSiblingItem;

    void setFlag(Flag flag, bool value)
    {
        if (value) {
            flags() |= flag;
        } else {
            flags() &= ~flag;
        }
    }

    bool flag(Flag flag) const { return (flags() & flag) != 0; }

    ItemType itemType() const { return static_cast<ItemType>(enabled.m_u8Padding); }
    void setItemType(ItemType type) { enabled.m_u8Padding = static_cast<uint8_t>(type); }

    virtual bool touchEvent(struct TouchEvent *) { return false; }
    virtual void implicitWidthChange() {}
    virtual void implicitHeightChange() {}

    virtual bool keyEvent(struct Qul::Private::KeyEvent *event)
    {
        KeysAttachedType *attachee = qulAttachedProperties<KeysAttachedType>(this);
        if (attachee) {
            bool accepted = attachee->keyEvent(event);
            if (accepted)
                return true;
        }

        return false;
    }

    virtual PlatformInterface::Point childrenOffset() const { return position(); }
    PlatformInterface::Point position() const { return PlatformInterface::Point(x.value(), y.value()); }
    PlatformInterface::Size size() const { return PlatformInterface::Size(width.value(), height.value()); }

    void ensureChildZOrder();

    // schedule repaint
    void update();

    ItemBase();
    virtual ~ItemBase();

    struct AnchorLine
    {
        AnchorLine()
            : target(NULL)
            , valueForSibling(0)
            , valueForChild(0)
        {}

        AnchorLine(const ItemBase *target, int valueForSibling, int valueForChild)
            : target(target)
            , valueForSibling(valueForSibling)
            , valueForChild(valueForChild)
        {}

        const ItemBase *target;

        int valueForSibling;
        int valueForChild;

        friend bool operator==(const AnchorLine &a, const AnchorLine &b)
        {
            return a.target == b.target && a.valueForSibling == b.valueForSibling && a.valueForChild == b.valueForChild;
        }

        operator bool() const { return target; }

        int valueFor(const ItemBase *item) const
        {
            if (!target)
                return 0;
            bool isSibling = item->parentItem() == target->parentItem();
            assert(isSibling || item->parentItem() == target);
            return isSibling ? valueForSibling : valueForChild;
        }
    };

    struct VerticalAnchorLine : public AnchorLine
    {
        VerticalAnchorLine(const ItemBase *target, int valueForSibling, int valueForChild)
            : AnchorLine(target, valueForSibling, valueForChild)
        {}

        VerticalAnchorLine()
            : AnchorLine()
        {}
    };

    struct HorizontalAnchorLine : public AnchorLine
    {
        HorizontalAnchorLine(const ItemBase *target, int valueForSibling, int valueForChild)
            : AnchorLine(target, valueForSibling, valueForChild)
        {}

        HorizontalAnchorLine()
            : AnchorLine()
        {}
    };

    inline HorizontalAnchorLine leftAnchor() const;
    inline HorizontalAnchorLine rightAnchor() const;
    inline HorizontalAnchorLine horizontalCenterAnchor() const;

    inline VerticalAnchorLine topAnchor() const;
    inline VerticalAnchorLine bottomAnchor() const;
    inline VerticalAnchorLine verticalCenterAnchor() const;

    // Compute the size this item would have if it is anchored with the given anchors
    qreal sizeBetweenAnchors(const AnchorLine &from,
                             const AnchorLine &mid,
                             const AnchorLine &to,
                             qreal fromMargin,
                             qreal toMargin,
                             qreal midOffset) const;

    // Compute the position of the item given anchors.
    qreal positionConsideringAnchors(const AnchorLine &from,
                                     const AnchorLine &mid,
                                     const AnchorLine &to,
                                     qreal fromMargin,
                                     qreal midOffset,
                                     qreal toMargin,
                                     Property<int> ItemBase::*size) const;

    enum RenderNodeUpdate { UpdateRenderNode, DontUpdateRenderNode };

    Qul::Private::RenderTree::Node *getRenderNode(RenderNodeUpdate update = DontUpdateRenderNode);

protected:
    virtual Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds) = 0;

    friend class Qul::Private::Application;

    friend void appendChild(ItemBase *parent, ItemBase *child, ItemBase *after);
    friend void appendChild(ItemBase *parent, Object *child);

    // virtual function to append an item to this item.
    // This function should not be called dirrectly, the free function appendChild should be called instead
    virtual void appendChildImpl(ItemBase *child, ItemBase *after = NULL);

    // Append \a child to a circular list stored in \a firstChildPtr.
    //
    // If \a after is NULL, append at the end.
    // If \a after is set, append after it.
    //
    // firstChild may be null, to represent the empty list. In that case it will be changed.
    // child's nextCircularSiblingItem and prevCircularSiblingItem must be itself.
    static void siblingListAppend(ItemBase **firstSiblingPtr, ItemBase *newSibling, ItemBase *after = NULL);

    void setImplicitSizes(int w, int h);

    quint16 &extraFlags() { return visible.m_u16Padding; }
    const quint16 &extraFlags() const { return visible.m_u16Padding; }

    DirtyEventStaticDependency<RepaintEvent> onAppearanceChanged;

private:
    quint16 &flags() { return enabled.m_u16Padding; }
    const quint16 &flags() const { return enabled.m_u16Padding; }

    void updateChildFlags(ItemBase *child);
    void markDirty();
    void markSubTreeDirty();

    Qul::Private::RenderTree::Node *prepareRenderNode(const PlatformInterface::Rect &clipRect,
                                                      PlatformInterface::Point offset = PlatformInterface::Point(),
                                                      qreal opacity = 1.,
                                                      bool clipChanged = false);

    DirtyEventStaticDependency<DirtySizeEvent> onSizeChanged;
    DirtyEventStaticDependency<DirtyEnabledEvent> onEnabledChanged;
};

template<typename IterType, ItemBase *ItemBase::*SuccSiblingMember>
class ItemChildrenIterator
{
public:
    ItemBase *operator->() const { return child; }

    ItemBase &operator*() const
    {
        assert(child != NULL);
        return *child;
    }

    operator bool() const { return child != NULL; }

    IterType &operator++()
    {
        child = (child->*SuccSiblingMember != headChild) ? child->*SuccSiblingMember : NULL;
        return *static_cast<IterType *>(this);
    }

protected:
    explicit ItemChildrenIterator(ItemBase *headChild)
        : headChild(headChild)
        , child(headChild)
    {}

    ItemBase *headChild;
    ItemBase *child;
};

class ItemChildrenFwIterator : public ItemChildrenIterator<ItemChildrenFwIterator, &ItemBase::nextCircularSiblingItem>
{
public:
    explicit ItemChildrenFwIterator(ItemBase *item)
        : ItemChildrenIterator(item->firstChildItem)
    {}

    static ItemChildrenFwIterator from(ItemBase *firstChild) { return ItemChildrenFwIterator(firstChild, true); }

private:
    explicit ItemChildrenFwIterator(ItemBase *firstChild, bool)
        : ItemChildrenIterator(firstChild)
    {}
};

class ItemChildrenRevIterator : public ItemChildrenIterator<ItemChildrenRevIterator, &ItemBase::prevCircularSiblingItem>
{
public:
    explicit ItemChildrenRevIterator(ItemBase *item)
        : ItemChildrenIterator(item->lastChildItem())
    {}
};

template<typename IterType, ItemBase *ItemBase::*SuccSiblingMember>
inline bool operator==(const ItemChildrenIterator<IterType, SuccSiblingMember> &lhs,
                       const ItemChildrenIterator<IterType, SuccSiblingMember> &rhs)
{
    return &*lhs == &*rhs;
}

template<typename IterType, ItemBase *ItemBase::*SuccSiblingMember>
inline bool operator!=(const ItemChildrenIterator<IterType, SuccSiblingMember> &lhs,
                       const ItemChildrenIterator<IterType, SuccSiblingMember> &rhs)
{
    return !(lhs == rhs);
}

void appendChild(ItemBase *parent, ItemBase *child, ItemBase *after = NULL);

inline Qul::Private::RenderTree::Node *ItemBase::getRenderNode(RenderNodeUpdate update)
{
    return renderNode(update, 0);
}

struct Item : ItemBase
{
    enum TransformOrigin { TopLeft, Top, TopRight, Left, Center, Right, BottomLeft, Bottom, BottomRight };

    Item()
        : node(RenderTree::Node::NodeType, this)
    {}

    virtual Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds);

    Qul::Private::RenderTree::Node node;
};

typedef PlatformInterface::Rgba32 ULColor;
struct Rectangle : ItemBase
{
    Property<ULColor> color;
    Property<qreal> radius;
    Rectangle()
        : color(0xffffffff)
        , radius(0)
        , node(this)
    {
        onAppearanceChanged.registerDependency(color);
    }

    virtual Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds);

    Qul::Private::RenderTree::RectangleNode node;
};

struct TransformBase
{
    virtual ~TransformBase();

    void appendTransform(TransformBase *transform);
    void setItem(ItemBase *item);

    Qul::PlatformInterface::Transform transform() const;
    ItemBase *item() const;

protected:
    struct ChangedEvent
    {
        inline void operator()(TransformBase *transform)
        {
            if (!transform)
                return;
            ItemBase *item = transform->item();
            if (item) {
                item->update();
            }
        }
    };

    // Struct used to handle intermediate calculations used
    // to compose final transformations. It's aware of 2D and 3D transforms.
    // It tries to operate on Transform class (3x3 matrix) as long as possible,
    // composition with 3D transform results in promoting internal
    // representation to Matrix4x4
    struct IntermediateTransform
    {
        enum class Type { Transform2D, Transform3D } type;

        IntermediateTransform(Qul::PlatformInterface::Transform &&transform)
            : type(Type::Transform2D)
            , t2d(transform)
        {}

        IntermediateTransform(Qul::Private::Matrix4x4 &&transform)
            : type(Type::Transform3D)
            , t3d(transform)
        {}

        union {
            Qul::PlatformInterface::Transform t2d;
            Qul::Private::Matrix4x4 t3d;
        };

        IntermediateTransform &operator*=(const IntermediateTransform &other);
        operator Qul::PlatformInterface::Transform() const;
    };

    virtual IntermediateTransform computeTransform() const = 0;

private:
    inline TransformBase *tail()
    {
        TransformBase *n = this;
        while (n->m_data.isT1() && !n->m_data.isNull())
            n = n->m_data.asT1();
        return n;
    }

    inline const TransformBase *tail() const { return const_cast<TransformBase *>(this)->tail(); }

    Qul::Private::BiPointer<TransformBase, ItemBase> m_data;
};

template<typename NextTransform>
struct InlineScale : NextTransform
{
    InlineScale() { this->onPropertiesChanged.registerDependency(scale); }
    Property<qreal> scale;

protected:
    void updateTransform(Qul::PlatformInterface::Transform &t) const override
    {
        if (scale.value() != 1)
            t.scale(scale.value(), scale.value());
        NextTransform::updateTransform(t);
    }
};

template<typename NextTransform>
struct InlineScaleDefault : NextTransform
{
    void setScaleProperty(Property<qreal> *property)
    {
        m_scale = property;
        if (m_scale) {
            this->onPropertiesChanged.registerDependency(*m_scale);
        }
    }

protected:
    void updateTransform(Qul::PlatformInterface::Transform &t) const override
    {
        if (m_scale && m_scale->value() != 1) {
            t.scale(m_scale->value(), m_scale->value());
        }
        NextTransform::updateTransform(t);
    }

    Property<qreal> *m_scale = nullptr;
};

template<typename NextTransform>
struct InlineRotation : NextTransform
{
    InlineRotation() { this->onPropertiesChanged.registerDependency(rotation); }
    Property<qreal> rotation;

protected:
    void updateTransform(Qul::PlatformInterface::Transform &t) const override
    {
        if (rotation.value() != 0)
            t.rotate(rotation.value());
        NextTransform::updateTransform(t);
    }
};

template<typename NextTransform>
struct InlineRotationDefault : NextTransform
{
    void setRotationProperty(Property<qreal> *property)
    {
        m_rotation = property;
        if (m_rotation) {
            this->onPropertiesChanged.registerDependency(*m_rotation);
        }
    }

protected:
    void updateTransform(Qul::PlatformInterface::Transform &t) const override
    {
        if (m_rotation && m_rotation->value() != 0) {
            t.rotate(m_rotation->value());
        }
        NextTransform::updateTransform(t);
    }

    Property<qreal> *m_rotation = nullptr;
};

template<typename NextTransform>
struct InlineTransformOriginBase : NextTransform
{
protected:
    virtual Item::TransformOrigin getTransformOrigin() const = 0;

    void getOrigin(float &x, float &y) const
    {
        const ItemBase *it = this->item();
        if (!it) {
            assert(!"InlineTransformOrigin::getOrigin() failed to get target item for the transformation");
            return;
        }

        const Item::TransformOrigin origin = this->getTransformOrigin();
        switch (origin) {
        case Item::Top:
        case Item::Center:
        case Item::Bottom:
            x = it->width.value() * 0.5f;
            break;

        case Item::TopRight:
        case Item::Right:
        case Item::BottomRight:
            x = it->width.value();
            break;

        default:
            x = 0;
        }

        switch (origin) {
        case Item::Left:
        case Item::Center:
        case Item::Right:
            y = it->height.value() * 0.5f;
            break;

        case Item::BottomLeft:
        case Item::Bottom:
        case Item::BottomRight:
            y = it->height.value();
            break;

        default:
            y = 0;
        }
    }

    void updateTransform(Qul::PlatformInterface::Transform &t) const override
    {
        float originX, originY;
        getOrigin(originX, originY);
        t.translate(-originX, -originY);
        NextTransform::updateTransform(t);
        t.translate(originX, originY);
    }
};

template<typename NextTransform>
struct InlineTransformOrigin : InlineTransformOriginBase<NextTransform>
{
    InlineTransformOrigin()
        : transformOrigin(Item::Center)
    {
        this->onPropertiesChanged.registerDependency(transformOrigin);
    }

    Property<Item::TransformOrigin> transformOrigin;

protected:
    Item::TransformOrigin getTransformOrigin() const override { return transformOrigin.value(); }
};

template<typename NextTransform>
struct InlineTransformOriginDefault : InlineTransformOriginBase<NextTransform>
{
    void setTransformOriginProperty(Property<Item::TransformOrigin> *property)
    {
        m_transformOrigin = property;
        if (m_transformOrigin) {
            this->onPropertiesChanged.registerDependency(*m_transformOrigin);
        }
    }

protected:
    Item::TransformOrigin getTransformOrigin() const override
    {
        if (m_transformOrigin) {
            return m_transformOrigin->value();
        }
        return Item::Center;
    }

private:
    Property<Item::TransformOrigin> *m_transformOrigin = nullptr;
};

struct InlineTransform : TransformBase
{
    InlineTransform() { onPropertiesChanged.init<InlineTransform, &InlineTransform::onPropertiesChanged>(); }

    IntermediateTransform computeTransform() const override
    {
        Qul::PlatformInterface::Transform t;
        updateTransform(t);
        return t;
    }

protected:
    virtual void updateTransform(Qul::PlatformInterface::Transform &) const {};

    DirtyEventStaticDependency<TransformBase::ChangedEvent> onPropertiesChanged;
};

struct Transform : Qul::Object, TransformBase
{
public:
    struct Origin
    {
        Property<qreal> x;
        Property<qreal> y;
        inline Origin &get() { return *this; }
    };
};

class TransformProperty
{
public:
    TransformProperty()
        : m_transform(NULL)
        , m_item(NULL)
    {}

    explicit TransformProperty(ItemBase *item)
        : m_transform(NULL)
        , m_item(item)
    {}

    inline void setValue(TransformBase *transform)
    {
        transform->setItem(m_item);
        m_transform = transform;
    }
    inline TransformBase *value() const { return m_transform; }

    inline void append(TransformBase *transform)
    {
        if (m_transform)
            m_transform->appendTransform(transform);
        else {
            setValue(transform);
        }
    }

private:
    TransformBase *m_transform;
    ItemBase *m_item;
};

struct Translate : Transform
{
    Translate()
        : x(0)
        , y(0)
    {
        onPropertiesChanged.init<Translate, &Translate::onPropertiesChanged>();
        onPropertiesChanged.registerDependency(x);
        onPropertiesChanged.registerDependency(y);
    }

    Property<qreal> x;
    Property<qreal> y;

protected:
    IntermediateTransform computeTransform() const override;

    DirtyEventStaticDependency<Transform::ChangedEvent> onPropertiesChanged;
};

struct Scale : Transform
{
    Scale()
        : xScale(1)
        , yScale(1)
    {
        onPropertiesChanged.init<Scale, &Scale::onPropertiesChanged>();
        onPropertiesChanged.registerDependency(xScale);
        onPropertiesChanged.registerDependency(yScale);
        onPropertiesChanged.registerDependency(origin.x);
        onPropertiesChanged.registerDependency(origin.y);
    }

    Property<qreal> xScale;
    Property<qreal> yScale;

    Transform::Origin origin;

protected:
    IntermediateTransform computeTransform() const override;

    DirtyEventStaticDependency<Transform::ChangedEvent> onPropertiesChanged;
};

struct Rotation : Transform
{
    Rotation()
    {
        onPropertiesChanged.init<Rotation, &Rotation::onPropertiesChanged>();
        onPropertiesChanged.registerDependency(angle);
        onPropertiesChanged.registerDependency(origin.x);
        onPropertiesChanged.registerDependency(origin.y);
    }

    Property<qreal> angle;
    Transform::Origin origin;

protected:
    IntermediateTransform computeTransform() const override;

    DirtyEventStaticDependency<Transform::ChangedEvent> onPropertiesChanged;
};

struct Matrix4x4 : Transform
{
    Matrix4x4()
    {
        onPropertiesChanged.init<Matrix4x4, &Matrix4x4::onPropertiesChanged>();
        onPropertiesChanged.registerDependency(matrix);
    }

    Property<Builtins::Matrix4x4> matrix;

protected:
    IntermediateTransform computeTransform() const override;

    DirtyEventStaticDependency<Transform::ChangedEvent> onPropertiesChanged;
};

struct Image : ItemBase
{
    enum ExtraFlag {
        IsColorizedImageExtraFlag = 1U << 0,
    };

    Property<Qul::SharedImage> source;
    Image()
        : node(this)
        // by default image should be centered
        , horizontalAlignment(AlignHCenter)
        , verticalAlignment(AlignVCenter)
        , transform(this)
    {
        onImageChanged.init<Image, &Image::onImageChanged>();
        onImageChanged.registerDependency(source);
        onImageChanged.registerDependency(fillMode);
        onImageChanged.registerDependency(horizontalAlignment);
        onImageChanged.registerDependency(verticalAlignment);

        setColorizedImage(false);
    }

    bool isColorizedImage() const { return (extraFlags() & IsColorizedImageExtraFlag) != 0; }
    void setColorizedImage(bool value)
    {
        if (value) {
            extraFlags() |= IsColorizedImageExtraFlag;
        } else {
            extraFlags() &= ~IsColorizedImageExtraFlag;
        }
    }

    virtual Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds);

    Qul::Private::RenderTree::ImageNode node;

    enum FillMode { Stretch, PreserveAspectFit, PreserveAspectCrop, Tile, TileVertically, TileHorizontally, Pad };
    Property<FillMode> fillMode;

    enum HAlignment { AlignLeft, AlignRight, AlignHCenter };

    enum VAlignment { AlignTop, AlignBottom, AlignVCenter };

    Property<HAlignment> horizontalAlignment;
    Property<VAlignment> verticalAlignment;

    TransformProperty transform;

private:
    struct ImageChangedEvent
    {
        void operator()(Image *image);
    };
    DirtyEventStaticDependency<ImageChangedEvent> onImageChanged;
};

struct BorderImage : ItemBase
{
    Property<Qul::SharedImage> source;
    BorderImage()
        : node(this)
    {
        onAppearanceChanged.registerDependency(source);
        onAppearanceChanged.registerDependency(border.left);
        onAppearanceChanged.registerDependency(border.right);
        onAppearanceChanged.registerDependency(border.top);
        onAppearanceChanged.registerDependency(border.bottom);
    }

    struct Border
    {
        Property<int> left;
        Property<int> right;
        Property<int> top;
        Property<int> bottom;
    } border;

    virtual Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds);

    Qul::Private::RenderTree::ImageNode node;
};

struct TextLight : ItemBase
{
    Property<FontPointer> font;

    Property<Qul::Private::String> text;

    Property<ULColor> color;
    Property<int> baselineOffset;

    TextLight()
        : color(0xff000000)
        , node(this)
    {
        onAppearanceChanged.registerDependency(color);

        onTextChanged.init<TextLight, &TextLight::onTextChanged>();
        onTextChanged.registerDependency(text);
        onTextChanged.registerDependency(font);
    }

    virtual Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds);

    Qul::Private::RenderTree::TextNode node;

    inline VerticalAnchorLine baselineAnchor() const;

protected:
    enum ExtraFlag {
        // If this class is a Text
        IsFullText = 1U << 1,
    };

    const FontEngine *fontEngine() const;

private:
    PlatformInterface::Rect textBounds;
    PlatformInterface::coord_t textAdvance;

    struct TextChangedEvent
    {
        void operator()(TextLight *text);
    };
    friend struct TextChangedEvent;
    friend struct Text;
    friend class RenderTree::TextNode;
    friend class RenderTree::Renderer;
    inline const struct Text *asText() const;

    DirtyEventStaticDependency<TextChangedEvent> onTextChanged;
};

struct Text : TextLight, private ExtendsBase
{
    enum HAlignment { AlignLeft, AlignRight, AlignHCenter };
    enum VAlignment { AlignTop, AlignBottom, AlignVCenter };

    Property<Text::HAlignment> horizontalAlignment;
    Property<Text::VAlignment> verticalAlignment;

    Property<int> leftPadding;
    Property<int> rightPadding;
    Property<int> bottomPadding;
    Property<int> topPadding;
    Property<int> padding;

    TransformProperty transform;
    PlatformInterface::Point alignmentOffset;

    Text()
        : horizontalAlignment(AlignLeft) // Assuming only left to right writing direction for now
        , verticalAlignment(AlignTop)
        , transform(this)
    {
        onAppearanceChanged.registerDependency(horizontalAlignment);
        onAppearanceChanged.registerDependency(verticalAlignment);

        onTextChanged.registerDependency(leftPadding);
        onTextChanged.registerDependency(rightPadding);
        onTextChanged.registerDependency(bottomPadding);
        onTextChanged.registerDependency(topPadding);
        onTextChanged.registerDependency(padding);

        extraFlags() |= IsFullText;
    }
};

struct LayoutItem : Item
{
    static bool updateLayouts();

    LayoutItem();
    virtual ~LayoutItem();

protected:
    virtual bool doLayout() = 0;

    void layoutDirty();

    friend struct DirtySizeEvent;
    friend struct DirtyLayoutEvent;

    Qul::Private::DirtyListNode dirtyListNode;

    typedef Qul::Private::DirtyList<LayoutItem, &LayoutItem::dirtyListNode> DirtyLayoutList;
    static DirtyLayoutList dirtyLayoutItems;

private:
    struct DoLayoutFunctor
    {
        DoLayoutFunctor()
            : layoutsChanged(false)
        {}

        void operator()(LayoutItem *item)
        {
            if (item->doLayout()) {
                layoutsChanged = true;
            }
        }

        bool layoutsChanged;
    };
};

struct Row : LayoutItem
{
    Row()
    {
        onLayoutChanged.init<Row, &Row::onLayoutChanged>();
        onLayoutChanged.registerDependency(spacing);
        onLayoutChanged.registerDependency(padding);
    }

    Property<int> spacing;
    Property<int> padding;

private:
    bool doLayout();

    DirtyEventStaticDependency<DirtyLayoutEvent> onLayoutChanged;
};

struct Column : LayoutItem
{
    Column()
    {
        onLayoutChanged.init<Column, &Column::onLayoutChanged>();
        onLayoutChanged.registerDependency(spacing);
        onLayoutChanged.registerDependency(padding);
    }

    Property<int> spacing;
    Property<int> padding;

private:
    bool doLayout();

    DirtyEventStaticDependency<DirtyLayoutEvent> onLayoutChanged;
};

struct MouseArea : Item
{
    friend void onLongPressTimerTriggeredSlot(MouseArea *ma);

    Signal<void(Builtins::MouseEvent *mouse)> clicked;
    Signal<void(Builtins::MouseEvent *mouse)> positionChanged;
    Signal<void(Builtins::MouseEvent *mouse)> onPressed;
    Signal<void(Builtins::MouseEvent *mouse)> released;
    Signal<void(Builtins::MouseEvent *mouse)> pressAndHold;
    Signal<void()> canceled;

    Property<bool> pressed;
    Property<qreal> mouseX;
    Property<qreal> mouseY;
    Property<int> pressAndHoldInterval;

    bool touchEvent(TouchEvent *event);
    MouseArea();

private:
    bool longPress;
    Qul::Private::Items::Timer longPressTimer;
    Slot<MouseArea, void()> longPressTimerTriggeredSlot;
};

ItemBase::HorizontalAnchorLine ItemBase::leftAnchor() const
{
    return HorizontalAnchorLine(this, x.value(), 0);
}

ItemBase::HorizontalAnchorLine ItemBase::rightAnchor() const
{
    int w = width.value();
    return HorizontalAnchorLine(this, x.value() + w, w);
}

ItemBase::HorizontalAnchorLine ItemBase::horizontalCenterAnchor() const
{
    int w = int(width.value() + 1) / 2;
    return HorizontalAnchorLine(this, x.value() + w, w);
}

ItemBase::VerticalAnchorLine ItemBase::topAnchor() const
{
    return VerticalAnchorLine(this, y.value(), 0);
}

ItemBase::VerticalAnchorLine ItemBase::bottomAnchor() const
{
    int h = height.value();
    return VerticalAnchorLine(this, y.value() + h, h);
}

ItemBase::VerticalAnchorLine ItemBase::verticalCenterAnchor() const
{
    int h = int(height.value() + 1) / 2;
    return VerticalAnchorLine(this, y.value() + h, h);
}

const Text *TextLight::asText() const
{
    return ((extraFlags() & IsFullText)) ? static_cast<const Text *>(this) : NULL;
}

ItemBase::VerticalAnchorLine TextLight::baselineAnchor() const
{
    int b = baselineOffset.value();
    return VerticalAnchorLine(this, y.value() + b, b);
}

} // namespace Items
} // namespace Private
} // namespace Qul
