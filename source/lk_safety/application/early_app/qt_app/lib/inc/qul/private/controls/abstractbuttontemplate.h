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
#include <qul/image.h>
#include <qul/private/timeritem.h>

namespace Qul {
namespace Private {
namespace Controls {
namespace Templates {
struct AbstractButton : Control
{
    Property<String> text QUL_PROPERTY_DEBUG_INIT("AbstractButton::text");
    Property<bool> down QUL_PROPERTY_DEBUG_INIT("AbstractButton::down");
    Property<bool> pressed QUL_PROPERTY_DEBUG_INIT("AbstractButton::pressed");
    Property<bool> checked QUL_PROPERTY_DEBUG_INIT("AbstractButton::checked");
    Property<bool> checkable QUL_PROPERTY_DEBUG_INIT("AbstractButton::checkable");
    Property<bool> autoRepeat QUL_PROPERTY_DEBUG_INIT("AbstractButton::autoRepeat");
    Property<int> autoRepeatDelay QUL_PROPERTY_DEBUG_INIT("AbstractButton::autoRepeatDelay");
    Property<int> autoRepeatInterval QUL_PROPERTY_DEBUG_INIT("AbstractButton::autoRepeatInterval");
    Property<Qul::Private::Items::ItemBase *> indicator QUL_PROPERTY_DEBUG_INIT("AbstractButton::indicator");

    struct Icon
    {
        Property<Qul::SharedImage> source;
        Property<Items::ULColor> color;
    };

    struct CheckedChangedEvent
    {
        void operator()(Qul::Private::Controls::Templates::AbstractButton *button);
    };

    struct IndicatorChangedEvent
    {
        void operator()(Qul::Private::Controls::Templates::AbstractButton *button);
    };

    struct EnabledChangedEvent
    {
        EnabledChangedEvent(Qul::Private::Controls::Templates::AbstractButton *button);

        Qul::Private::Controls::Templates::AbstractButton *button;

        void operator()();
    };

    Icon icon;

    Qul::Private::Items::Timer autoRepeatDelayTimer;
    Qul::Private::Items::Timer autoRepeatTimer;

    Signal<void()> clicked;
    Signal<void()> canceled;
    Signal<void()> onPressed;
    Signal<void()> released;
    Signal<void()> toggled;

    Slot<AbstractButton, void()> autoRepeatDelayTriggeredSlot;
    Slot<AbstractButton, void()> autoRepeatIntervalTriggeredSlot;

    DirtyEventStaticDependency<AbstractButton::CheckedChangedEvent> onCheckedChanged;
    DirtyEventStaticDependency<AbstractButton::IndicatorChangedEvent> onIndicatorChanged;

    // Can't be DirtyEventStaticDependency as enabled already has a static dependency
    DirtyEvent<AbstractButton::EnabledChangedEvent, 1> onEnabledChanged;

    AbstractButton();
    virtual ~AbstractButton();

    bool touchEvent(struct TouchEvent *event);

    virtual void nextCheckState();
    void toggle(bool value);

    enum ButtonChange {
        ButtonCheckedChange,
    };

    // Allows subclasses to handle certain events without adding extra
    // virtuals or structs for each type of event.
    virtual void buttonChange(ButtonChange change);

protected:
    bool sticky = false; // Once checked, can't be unchecked by touch
};

} // namespace Templates
} // namespace Controls
} // namespace Private
} // namespace Qul
