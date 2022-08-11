/******************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Ultralight module.
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

#include "controltemplate.h"
#include "qul/private/flickable.h"
#include "qul/private/slot.h"

namespace Qul {
namespace Private {

namespace Controls {
namespace Templates {
struct SwipeView;
} // namespace Templates
} // namespace Controls

struct SnapAnimation : Items::PropertyAnimation<int>
{
    Animation::AnimationExtraProperties animationExtraPropertyStorage;

    explicit SnapAnimation(Controls::Templates::SwipeView *swipeView);

    // starts the snap animation to the given index
    void start(int targetIndex);
    inline void resume() { Animation::start(); };
    inline int targetIndex() const { return m_targetIndex; };

protected:
    void doStart();

    Controls::Templates::SwipeView *m_swipeView;
    int m_targetIndex = -1;
};

namespace Controls {
namespace Templates {

struct SwipeView : Templates::Control
{
    Property<bool> interactive;
    Property<Builtins::GlobalQtObject::Orientation> orientation QUL_PROPERTY_DEBUG_INIT("SwipeView::orientation");
    Property<int> count QUL_PROPERTY_DEBUG_INIT("SwipeView::count");
    Property<int> currentIndex QUL_PROPERTY_DEBUG_INIT("SwipeView::currentIndex");
    Property<Items::ItemBase *> currentItem QUL_PROPERTY_DEBUG_INIT("SwipeView::currentItem");

    PlatformInterface::PointF pressPoint;
    qreal positionBeforePress;
    bool componentCompleted;

    SnapAnimation snapAnimation;
    Slot<SwipeView, void()> onSnapAnimationFinished;
    static void snapFinishedCallback(SwipeView *swipeView);

    SwipeView();
    virtual ~SwipeView();

    void appendChildImpl(ItemBase *child, ItemBase *after = NULL);

    void implicitWidthChange();
    void implicitHeightChange();

    void decrementCurrentIndex();
    void incrementCurrentIndex();
    void setCurrentIndex(int index);
    void updateCurrentItem();

    Items::ItemBase *itemAt(int index);

    int lowestValidIndex() const;
    int highestValidIndex() const;

    Items::Flickable *getFlickable();

    void layout();

    bool touchEvent(TouchEvent *event);

    Items::ComponentAttachedType componentAttachedType;
    Slot<SwipeView, void()> onCompletedSlot;

    /* During initialization, children are accumulated here. It's a circular list,
     * using next/prevCircularSiblingItem.
     *
     * When the component completes (and the contentItem Flickable must be available),
     * they are moved to the content item.
     */
    ItemBase *firstPendingChild;

    struct CurrentIndexChangedEvent
    {
        void operator()(SwipeView *swipeView);
    };

    struct ViewportSizeChangedEvent
    {
        ViewportSizeChangedEvent(Qul::Private::Controls::Templates::SwipeView *swipeView);

        void operator()();

        Qul::Private::Controls::Templates::SwipeView *swipeView;
    };

    DirtyEventStaticDependency<SwipeView::CurrentIndexChangedEvent> onCurrentIndexChanged;

    // initialized late on contentItem
    DirtyEvent<SwipeView::ViewportSizeChangedEvent, 2> onViewportSizeChanged;

protected:
    // Iterate over the content children, no matter if pending or on a Flickable.
    Items::ItemChildrenFwIterator contentItemChildrenIterator();

    PlatformInterface::Point beginSwipePos;
    qtime beginSwipeTimestamp;
};

} // namespace Templates
} // namespace Controls
} // namespace Private
} // namespace Qul
