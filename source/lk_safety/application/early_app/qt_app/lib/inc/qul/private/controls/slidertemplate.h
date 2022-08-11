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

#include "controltemplate.h"

namespace Qul {
namespace Private {
namespace Controls {
namespace Templates {

struct Slider : Control
{
    enum SnapMode { NoSnap, SnapAlways, SnapOnRelease };

    Property<qreal> from QUL_PROPERTY_DEBUG_INIT("Slider::from");
    Property<qreal> to QUL_PROPERTY_DEBUG_INIT("Slider::to");
    Property<qreal> stepSize QUL_PROPERTY_DEBUG_INIT("Slider::stepSize");
    Property<qreal> value QUL_PROPERTY_DEBUG_INIT("Slider::value");
    Property<qreal> position QUL_PROPERTY_DEBUG_INIT("Slider::position");
    Property<qreal> visualPosition QUL_PROPERTY_DEBUG_INIT("Slider::visualPosition");
    Property<bool> pressed QUL_PROPERTY_DEBUG_INIT("Slider::pressed");
    Property<Builtins::GlobalQtObject::Orientation> orientation QUL_PROPERTY_DEBUG_INIT("Slider::orientation");
    //    Property<bool> horizontal QUL_PROPERTY_DEBUG_INIT("Slider::horizontal");
    //    Property<bool> vertical QUL_PROPERTY_DEBUG_INIT("Slider::vertical");
    Property<Qul::Private::Items::ItemBase *> handle QUL_PROPERTY_DEBUG_INIT("Control::handle");

    Signal<void()> moved;

    // TODO: make properties out of these
    bool live;
    SnapMode snapMode;

    PlatformInterface::PointF pressPoint;

    Slider();
    virtual ~Slider();

    qreal valueAt(qreal position) const;

    bool touchEvent(struct TouchEvent *event);

    struct HandleChangedEvent
    {
        void operator()(Slider *control);
    };

    // These *SetEvents are workarounds for https://bugreports.qt.io/browse/UL-524
    struct ValueSetEvent
    {
        void operator()(Slider *control);
    };

    struct PositionSetEvent
    {
        void operator()(Slider *control);
    };

    struct RangeChangedEvent
    {
        void operator()(Slider *control);
    };

    struct OrientationChangedEvent
    {
        void operator()(Slider *control);
    };

    struct EnabledChangedEvent
    {
        EnabledChangedEvent(Slider *control);

        Slider *control;

        void operator()();
    };

    DirtyEventStaticDependency<Slider::HandleChangedEvent> onHandleChanged;
    DirtyEventStaticDependency<Slider::ValueSetEvent> onValueSet;
    DirtyEventStaticDependency<Slider::PositionSetEvent> onPositionSet;
    DirtyEventStaticDependency<Slider::RangeChangedEvent> onRangeChanged;
    DirtyEventStaticDependency<Slider::OrientationChangedEvent> onOrientationChanged;

    // enabled already has a static dependency from ItemBase
    DirtyEvent<Slider::EnabledChangedEvent, 1> onEnabledChanged;

private:
    void handleMove(const PlatformInterface::PointF &point);
    void handleRelease(const PlatformInterface::PointF &point);
    void clampValue();
    void clampPosition();
    qreal positionAt(const PlatformInterface::PointF &point) const;
    void updatePosition();
    qreal snapPosition(qreal position) const;
};

} // namespace Templates
} // namespace Controls
} // namespace Private
} // namespace Qul
