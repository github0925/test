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

#include <qul/private/items.h>
#include <qul/private/animation.h>

namespace Qul {
namespace Private {
namespace Items {

class Flickable : public Item
{
public:
    // should be readonly
    Property<Item *> contentItem QUL_PROPERTY_DEBUG_INIT("Flickable::contentItem");

    Property<bool> interactive;

    // should be readonly
    Property<bool> movingVertically;
    Property<bool> movingHorizontally;

    PropertyAlias<int> contentWidth;
    PropertyAlias<int> contentHeight;

    // positive values move the view area right/down
    Property<int> contentX;
    Property<int> contentY;

    Property<qreal> maximumFlickVelocity;

    struct VisibleArea
    {
        Property<qreal> xPosition;
        Property<qreal> yPosition;
        Property<qreal> widthRatio;
        Property<qreal> heightRatio;
    };
    VisibleArea visibleArea;

    Flickable();

    // should not be exported to QML
    void appendChildImpl(ItemBase *child, ItemBase *after = NULL)
    {
        if (child == &actualContentItem) {
            Item::appendChildImpl(child, after);
        } else {
            appendChild(&actualContentItem, child, after);
        }
    }

    // should not be exported to QML
    template<typename T>
    static T *getParentHelper(const Object *child)
    {
        // Skip over the content item
        return static_cast<T *>(child->parentObject()->parentObject());
    }

    // should not be exported to QML
    bool touchEvent(TouchEvent *event);
    bool allowMouseGrab;

    // should not be exported to QML
    void stopHorizontalMovement();
    void stopVerticalMovement();

    typedef qtime TimePoint;
    typedef qtime TimeSpan;

    struct AxisData : Animation
    {
        AxisData(Property<int> &property)
            : contentPosProperty(property)
            , viewSize(-1)
            , pressPos(0)
            , lastPos(0)
            , dragStartOffset(0)
            , dragMinBound(0)
            , dragMaxBound(0)
            , previousDragDelta(0)
            , velocity(0)
            , accel(0)
            , flickTarget(0)
            , startMargin(0)
            , endMargin(0)
            , origin(0)
            , overshoot(0)
            , transitionTo(0)
            , continuousFlickVelocity(0)
            , smoothVelocity(0)
            , _(0)
        {}

        // contentX or contentY
        // Note: The actualContentItem position will be the negated value
        Property<int> &contentPosProperty;

        // the x/y position of the actualContentItem
        int actualContentItemPos() const { return -contentPosProperty.value(); }
        void setActualContentItemPos(int v) { contentPosProperty.setValue(-v); }

        //void start();
        void doTick();

        void reset()
        {
            stop();
            // velocityBuffer.clear();
            dragStartOffset = 0;
            fixingUp = false;
            inOvershoot = false;
            moved = false;
        }
        qreal viewSize;
        qreal pressPos;
        qreal lastPos;
        qreal dragStartOffset;
        qreal dragMinBound;
        qreal dragMaxBound;
        qreal previousDragDelta;
        qreal velocity;
        qreal accel;
        qreal flickTarget;
        qreal startMargin;
        qreal endMargin;
        qreal origin;
        qreal overshoot;
        qreal transitionTo;
        qreal continuousFlickVelocity;
        //QElapsedTimer velocityTime;
        //int vTime;
        qreal smoothVelocity;
        struct SampleBuffer
        {
            SampleBuffer()
                : count(0)
                , pos(0)
            {}
            static const int BufferSize = 4;
            FixedLengthArray<qreal, BufferSize> array;
            char count;
            char pos;
            void addSample(qreal f)
            {
                array[pos] = f;
                pos = (pos + 1) % BufferSize;
                if (count < BufferSize)
                    count++;
            }
            void clear() { count = pos = 0; }
        };
        SampleBuffer velocityBuffer;
        union {
            struct
            {
                bool atEnd : 1;
                bool atBeginning : 1;
                bool transitionToSet : 1;
                bool fixingUp : 1;
                bool inOvershoot : 1;
                bool inRebound : 1;
                bool moving : 1;
                bool flicking : 1;
                bool dragging : 1;
                bool extentsChanged : 1;
                bool explicitValue : 1;
                mutable bool minExtentDirty : 1;
                mutable bool maxExtentDirty : 1;
                bool moved : 1;
            };
            int _;
        };

        bool drag(Flickable *flickable,
                  TimeSpan elapsedSincePress,
                  qreal flickableSize,
                  qreal delta,
                  qreal velocity,
                  bool overThreshold);
        void fixup(qreal minExtent, qreal maxExtent);

        void updateVelocity()
        {
            velocity = 0;
            if (!velocityBuffer.count)
                return;
            for (int i = 0; i < velocityBuffer.count; ++i)
                velocity += velocityBuffer.array[i];
            velocity /= velocityBuffer.count;
        }
        bool canFlick() const { return dragMinBound > dragMaxBound; }
    };

    // should not be exported to QML
    const AxisData &horizontalData() const;
    const AxisData &verticalData() const;

protected:
    // We want to keep them on the stack.
    Item actualContentItem;

    void grabMouse(bool grab);

private:
    struct FlickAnimationRunningChangedEvent
    {
        FlickAnimationRunningChangedEvent(Flickable *flickable, Builtins::GlobalQtObject::Orientation orientation)
            : flickable(flickable)
            , orientation(orientation)
        {}

        inline void operator()()
        {
            if (orientation == Builtins::GlobalQtObject::Horizontal)
                flickable->movingHorizontally.setValue(flickable->hData.running.value());
            else
                flickable->movingVertically.setValue(flickable->vData.running.value());
        }

        Flickable *flickable;
        Builtins::GlobalQtObject::Orientation orientation;
    };

    AxisData hData;
    AxisData vData;

    // can't be static: hData.running already has a dependency
    Qul::Private::DirtyEvent<FlickAnimationRunningChangedEvent, 1> onHorizontalFlickAnimationRunningChanged;
    Qul::Private::DirtyEvent<FlickAnimationRunningChangedEvent, 1> onVerticalFlickAnimationRunningChanged;

    struct VisibleAreaChangedEvent
    {
        VisibleAreaChangedEvent(Flickable *flickable)
            : flickable(flickable)
        {}

        inline void operator()() { flickable->updateVisibleArea(); }

        Flickable *flickable;
    };

    // can't be static: the properties already have static dependencies
    Qul::Private::DirtyEvent<VisibleAreaChangedEvent, 6> onVisibleAreaChangedEvent;

    struct ContentPosChangedEvent
    {
        void operator()(Flickable *f)
        {
            // When contentX/Y increase the view moves to the right/down,
            // which is implemented by moving the contents in the opposite direction.
            f->actualContentItem.x.setValue(-f->contentX.value());
            f->actualContentItem.y.setValue(-f->contentY.value());
        }
    };
    Qul::Private::DirtyEventStaticDependency<ContentPosChangedEvent> onContentPosChanged;

    void updateVisibleArea();

    //QQuickTimeLine timeline;
    union {
        struct
        {
            bool stealMouse : 1;
            bool pressed : 1;
            bool scrollingPhase : 1;
            bool calcVelocity : 1;
            bool pixelAligned : 1;
            bool syncDrag : 1;
        };
        int _;
    };
    TimePoint lastPosTime;
    TimePoint lastPressTime;
    PlatformInterface::PointF lastPos;
    PlatformInterface::PointF pressPos;
    qreal deceleration;
    qreal flickBoost;
    enum FixupMode { Normal, Immediate, ExtentChanged };
    FixupMode fixupMode;

    void maybeBeginDrag(TimePoint current, const PlatformInterface::PointF &pressPosn);
    void drag(TimePoint currentTimestamp,
              TouchEvent::State eventType,
              const PlatformInterface::PointF &localPos,
              const PlatformInterface::PointF &deltas,
              bool overThreshold,
              bool momentum,
              bool velocitySensitiveOverBounds,
              const PlatformInterface::PointF &velocity);
    bool flick(AxisData &data, qreal minExtent, qreal maxExtent, qreal velocity);
};

} // namespace Items
} // namespace Private
} // namespace Qul
