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
#include <qul/singleton.h>
#include <qul/timer.h>
#include <qul/signal.h>
#include <qul/image.h>

#include <qul/private/graphicsdevice.h>
#include <qul/private/items.h>
#include <qul/private/font.h>

#include <memory>
#include <vector>

#include "events.h"

namespace Qul {
namespace Private {
namespace Charts {

struct AbstractSeries;

struct ChartViewBase : Qul::Private::Items::Item
{
    struct Margins
    {
        Qul::Property<int> bottom;
        Qul::Property<int> left;
        Qul::Property<int> right;
        Qul::Property<int> top;
    };

    ChartViewBase();

    Qul::SharedImage getImage();

    Qul::Property<Qul::Private::String> title;
    Qul::Property<FontPointer> titleFont;
    Qul::Property<PlatformInterface::Rgba32> titleColor;

    Qul::Property<PlatformInterface::Rgba32> backgroundColor;

    // TODO: should be the minimum, with the actual values computed based on ticks, legends etc
    Margins margins;

    friend void appendChild(ChartViewBase *parent, AbstractSeries *child);

protected:
    void redraw();
    void drawLineSeries();
    void drawBarSeries();

    Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update,
                                               PlatformInterface::Rect *localBounds) QUL_DECL_OVERRIDE;

    Qul::Image m_image;

    // linked list of contained series
    // TODO: Figure out what multiple series in a ChartView are supposed to do
    AbstractSeries *m_series;

    Qul::Private::DirtyEvent<RepaintEvent, 8> m_repaintEvent;
};

} // namespace Charts
} // namespace Private
} // namespace Qul
