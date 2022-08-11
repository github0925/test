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
#include <qul/private/allocator.h>
#include <qul/private/items.h>
#include <vector>
#include "series.h"
#include "events.h"

namespace Qul {
namespace Private {
namespace Charts {

struct XYPoint;

struct LineSeries : AbstractSeries
{
    LineSeries() { m_type = SeriesTypeLine; }

    friend void appendChild(LineSeries *parent, XYPoint *child);

    void redrawChart();

    Vector<XYPoint *> m_points;
};

struct XYPoint : Qul::Object
{
    XYPoint()
        : m_repaintParentEvent(RepaintParentItemEvent(this), QUL_DEPENDENCIES(&x, &y).data())
    {}

    Qul::Property<Qul::qreal> x;
    Qul::Property<Qul::qreal> y;

    // This is pretty expensive, but how else do we notify the parent about property changes?
    // At least this will only be used if users explicitly use XYPoint.
    Qul::Private::DirtyEvent<RepaintParentItemEvent, 2> m_repaintParentEvent;
};

} // namespace Charts
} // namespace Private
} // namespace Qul
