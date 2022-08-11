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

namespace Qul {
namespace Private {
namespace Controls {
namespace Templates {

struct Control : Qul::Private::Items::Item
{
    Property<Qul::Private::Items::ItemBase *> background QUL_PROPERTY_DEBUG_INIT("Control::background");
    Property<Qul::Private::Items::ItemBase *> contentItem QUL_PROPERTY_DEBUG_INIT("Control::contentItem");
    Property<int> availableWidth QUL_PROPERTY_DEBUG_INIT("Control::availableWidth");
    Property<int> availableHeight QUL_PROPERTY_DEBUG_INIT("Control::availableHeight");
    Property<int> leftPadding QUL_PROPERTY_DEBUG_INIT("Control::leftPadding");
    Property<int> rightPadding QUL_PROPERTY_DEBUG_INIT("Control::rightPadding");
    Property<int> topPadding QUL_PROPERTY_DEBUG_INIT("Control::topPadding");
    Property<int> bottomPadding QUL_PROPERTY_DEBUG_INIT("Control::bottomPadding");
    Property<int> spacing QUL_PROPERTY_DEBUG_INIT("Control::spacing");
    Property<int> leftInset QUL_PROPERTY_DEBUG_INIT("Control::leftInset");
    Property<int> rightInset QUL_PROPERTY_DEBUG_INIT("Control::rightInset");
    Property<int> topInset QUL_PROPERTY_DEBUG_INIT("Control::topInset");
    Property<int> bottomInset QUL_PROPERTY_DEBUG_INIT("Control::bottomInset");
    Property<Qul::Private::FontPointer> font QUL_PROPERTY_DEBUG_INIT("Control::font");

    Control();
    virtual ~Control();

    struct BackgroundChangedEvent
    {
        void operator()(Qul::Private::Controls::Templates::Control *control);
    };

    struct ContentItemChangedEvent
    {
        void operator()(Qul::Private::Controls::Templates::Control *control);
    };

    struct AvailableWidthDirtyEvent
    {
        AvailableWidthDirtyEvent(Qul::Private::Controls::Templates::Control *control);

        Qul::Private::Controls::Templates::Control *control;

        void operator()();
    };

    struct AvailableHeightDirtyEvent
    {
        AvailableHeightDirtyEvent(Qul::Private::Controls::Templates::Control *control);

        Qul::Private::Controls::Templates::Control *control;

        void operator()();
    };

    void appendChildItem(Qul::Private::Items::ItemBase *child);

    void updateAvailableWidth();
    void updateAvailableHeight();

    void implicitWidthChange();
    void implicitHeightChange();

    DirtyEventStaticDependency<Control::BackgroundChangedEvent> onBackgroundChanged;
    DirtyEventStaticDependency<Control::ContentItemChangedEvent> onContentItemChanged;

    // Can't be DirtyEventStaticDependency as width/height already have a static dependency
    DirtyEvent<Control::AvailableWidthDirtyEvent, 3> onAvailableWidthDirty;
    DirtyEvent<Control::AvailableHeightDirtyEvent, 3> onAvailableHeightDirty;

private:
    void updateBackgroundHorizontalGeometry();
    void updateBackgroundVerticalGeometry();
    void updateContentItemHorizontalGeometry();
    void updateContentItemVerticalGeometry();
};

} // namespace Templates
} // namespace Controls
} // namespace Private
} // namespace Qul
