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
#include <qul/property.h>
#include <qul/private/items.h>
#include <qul/private/font.h>
#include "events.h"

namespace Qul {
namespace Private {
namespace Charts {

enum AxisType {
    AxisTypeValue,
    AxisTypeBarCategory,
};

struct AbstractAxis : public Qul::Private::ParentObject
{
    AbstractAxis()
        : color(PlatformInterface::Rgba32(uint32_t(200), uint32_t(200), uint32_t(200)))
        , visible(true)
        , lineVisible(true)
        , gridLineColor(PlatformInterface::Rgba32(uint32_t(220), uint32_t(220), uint32_t(220)))
        , gridVisible(true)
        , labelsColor(PlatformInterface::Rgba32(uint32_t(0), uint32_t(0), uint32_t(0)))
        , labelsVisible(true)
        , m_repaintEvent(RepaintParentItemEvent(this),
                         QUL_DEPENDENCIES(&color,
                                          &visible,
                                          &lineVisible,
                                          &gridLineColor,
                                          &gridVisible,
                                          &labelsColor,
                                          &labelsVisible,
                                          &labelsFont)
                             .data())
    {}

    Qul::Property<PlatformInterface::Rgba32> color;
    Qul::Property<bool> visible;
    Qul::Property<bool> lineVisible;

    Qul::Property<PlatformInterface::Rgba32> gridLineColor;
    Qul::Property<bool> gridVisible;

    Qul::Property<PlatformInterface::Rgba32> labelsColor;
    Qul::Property<bool> labelsVisible;
    Qul::Property<FontPointer> labelsFont;

    AxisType m_type;
    Qul::Private::DirtyEvent<RepaintParentItemEvent, 8> m_repaintEvent;
};

struct ValueAxis : AbstractAxis
{
    ValueAxis()
        : tickCount(5)
        , labelFormat(Qul::Private::String("%f", 2))
        , m_repaintEvent(RepaintParentItemEvent(this), QUL_DEPENDENCIES(&min, &max, &tickCount, &labelFormat).data())
    {
        m_type = AxisTypeValue;
    }

    Qul::Property<qreal> min;
    Qul::Property<qreal> max;
    Qul::Property<int> tickCount;
    Qul::Property<Qul::Private::String> labelFormat;

protected:
    Qul::Private::DirtyEvent<RepaintParentItemEvent, 4> m_repaintEvent;
};

struct BarCategoryValue;

struct BarCategoryAxis : AbstractAxis
{
    BarCategoryAxis()
        : m_categories(NULL)
    {
        m_type = AxisTypeBarCategory;
    }

    friend void appendChild(BarCategoryAxis *, BarCategoryValue *);

    // categories, list of strings; how to do?
    // workaround: use BarCategoryValue children
    BarCategoryValue *m_categories; // linked list
};

struct BarCategoryValue : Qul::Object
{
    BarCategoryValue()
        : m_next(NULL)
        , m_repaintEvent(RepaintParentItemEvent(this), QUL_DEPENDENCIES(&value).data())
    {}

    Qul::Property<Qul::Private::String> value;

    BarCategoryValue *m_next;
    Qul::Private::DirtyEvent<RepaintParentItemEvent, 1> m_repaintEvent;
};

} // namespace Charts
} // namespace Private
} // namespace Qul
