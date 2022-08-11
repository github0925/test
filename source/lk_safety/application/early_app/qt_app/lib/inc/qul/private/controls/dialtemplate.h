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

namespace Qul {
namespace Private {
namespace Controls {
namespace Templates {

struct Dial : Control
{
    enum SnapMode { NoSnap, SnapAlways, SnapOnRelease };

    enum InputMode {
        Circular,
        Horizontal,
        Vertical,
    };

    Property<qreal> from QUL_PROPERTY_DEBUG_INIT("Dial::from");
    Property<qreal> to QUL_PROPERTY_DEBUG_INIT("Dial::to");
    Property<qreal> value QUL_PROPERTY_DEBUG_INIT("Dial::value");
    Property<qreal> position QUL_PROPERTY_DEBUG_INIT("Dial::position");
    Property<qreal> angle QUL_PROPERTY_DEBUG_INIT("Dial::angle");
    Property<qreal> stepSize QUL_PROPERTY_DEBUG_INIT("Dial::stepSize");
    Property<bool> pressed QUL_PROPERTY_DEBUG_INIT("Dial::pressed");
    Property<bool> wrap QUL_PROPERTY_DEBUG_INIT("Dial::wrap");
    Property<bool> live QUL_PROPERTY_DEBUG_INIT("Dial::live");
    Property<SnapMode> snapMode QUL_PROPERTY_DEBUG_INIT("Dial::snapMode");
    Property<InputMode> inputMode QUL_PROPERTY_DEBUG_INIT("Dial::inputMode");
    Property<Items::ItemBase *> handle QUL_PROPERTY_DEBUG_INIT("Dial::handle");

    Signal<void()> moved;

    PlatformInterface::PointF pressPoint;
    qreal positionBeforePress;

    Dial();
    virtual ~Dial();

    qreal valueAt(qreal position) const;

    bool touchEvent(struct TouchEvent *event);

    struct HandleChangedEvent
    {
        void operator()(Dial *control);
    };

    // These *SetEvents are workarounds for https://bugreports.qt.io/browse/UL-524
    struct ValueSetEvent
    {
        void operator()(Dial *control);
    };

    struct PositionSetEvent
    {
        void operator()(Dial *control);
    };

    struct RangeChangedEvent
    {
        void operator()(Dial *control);
    };

    struct EnabledChangedEvent
    {
        EnabledChangedEvent(Dial *control);

        Dial *control;

        void operator()();
    };

    DirtyEventStaticDependency<Dial::HandleChangedEvent> onHandleChanged;
    DirtyEventStaticDependency<Dial::ValueSetEvent> onValueSet;
    DirtyEventStaticDependency<Dial::PositionSetEvent> onPositionSet;
    DirtyEventStaticDependency<Dial::RangeChangedEvent> onRangeChanged;

    // enabled already has a static dependency from ItemBase
    DirtyEvent<Dial::EnabledChangedEvent, 1> onEnabledChanged;

private:
    void handleMove(const PlatformInterface::PointF &point);
    void handleRelease(const PlatformInterface::PointF &point);
    void clampValue();
    void clampPosition();
    qreal positionAt(const PlatformInterface::PointF &point) const;
    qreal linearPositionAt(const PlatformInterface::PointF &point) const;
    qreal circularPositionAt(const PlatformInterface::PointF &point) const;
    void updatePosition();
    qreal snapPosition(qreal position) const;

    bool isLargeChange(const PlatformInterface::PointF &eventPos, qreal proposedPosition) const;
    bool isHorizontalOrVertical() const;
};

} // namespace Templates
} // namespace Controls
} // namespace Private
} // namespace Qul
