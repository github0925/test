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
#include "series.h"
#include "axis.h"
#include "events.h"

namespace Qul {
namespace Private {
namespace Charts {

struct BarSet;
struct AbstractBarSeries;

struct OnAxisDirty
{
    explicit OnAxisDirty(AbstractBarSeries *o)
        : barSeries(o)
    {}
    void operator()() const;
    AbstractBarSeries *barSeries;
};

struct AbstractBarSeries : AbstractSeries
{
    AbstractBarSeries()
        : barWidth(0.5)
        , m_barSets(NULL)
        , m_repaintEvent(RepaintParentItemEvent(this), QUL_DEPENDENCIES(&barWidth).data())
        , m_axisDirtyEvent(OnAxisDirty(this), QUL_DEPENDENCIES(&axisX, &axisY).data())
    {
        m_type = SeriesTypeBar;
    }

    // TODO: Enables inside-bar labels
    //Qul::Property<bool> labelsVisible;

    Qul::Property<qreal> barWidth;
    Qul::Property<AbstractAxis *> axisX;
    Qul::Property<AbstractAxis *> axisY;

    friend void appendChild(AbstractBarSeries *, BarSet *);

    BarSet *m_barSets; // linked list
    Qul::Private::DirtyEvent<RepaintParentItemEvent, 1> m_repaintEvent;
    Qul::Private::DirtyEvent<OnAxisDirty, 2> m_axisDirtyEvent;
};

struct BarSeries : AbstractBarSeries
{};

struct BarSetValue;

struct BarSet : Qul::Private::ParentObject
{
    BarSet()
        : color(
              PlatformInterface::Rgba32(uint32_t(0), uint32_t(0), uint32_t(0))) // implicit color assigned during drawing
        , m_next(NULL)
        , m_values(NULL)
        , m_repaintEvent(RepaintParentItemEvent(this), QUL_DEPENDENCIES(&color, &label).data())
    {}

    Qul::Property<PlatformInterface::Rgba32> color;

    // label for the legend...
    Qul::Property<Qul::Private::String> label;

    // TODO: For inside-bar labels
    //Qul::Property<PlatformInterface::Rgba32> labelColor;
    //Qul::Property<FontPointer> labelFont;

    friend void appendChild(BarSet *, BarSetValue *);

    BarSet *m_next;

    // values, list of qreal; how to accept an array like [1, 2, 5, 7]?
    // Even more complicated, upstream also allows [Qt.point(1, 3), Qt.Point(7, 5)]
    // workaround: use BarSetValue children
    BarSetValue *m_values; // linked list

    Qul::Private::DirtyEvent<RepaintParentItemEvent, 2> m_repaintEvent;
};

struct BarSetValue : Qul::Object
{
    BarSetValue()
        : m_next(NULL)
        , m_repaintEvent(RepaintParentItemEvent(this), QUL_DEPENDENCIES(&value).data())
    {}

    Qul::Property<Qul::qreal> value;

    BarSetValue *m_next;
    Qul::Private::DirtyEvent<RepaintParentItemEvent, 1> m_repaintEvent;
};

} // namespace Charts
} // namespace Private
} // namespace Qul
