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

#include <qul/property.h>
#include <qul/private/items.h>

namespace Qul {
namespace Private {

struct StaticTextItem;

namespace Items {

struct StaticText : ItemBase
{
    enum HAlignment { AlignLeft, AlignRight, AlignHCenter };

    enum VAlignment { AlignTop, AlignBottom, AlignVCenter };

    Property<int> baselineOffset;
    Property<ULColor> color;

    Property<StaticText::HAlignment> horizontalAlignment;
    Property<StaticText::VAlignment> verticalAlignment;

    TransformProperty transform;

    Property<int> leftPadding;
    Property<int> rightPadding;
    Property<int> bottomPadding;
    Property<int> topPadding;
    Property<int> padding;

    StaticText();

    virtual Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds);

    Qul::Private::RenderTree::StaticTextNode node;

    void setStaticTextItem(const StaticTextItem *item);

    const StaticTextItem *staticTextItem() const { return textItem; }

    inline VerticalAnchorLine baselineAnchor() const;

private:
    void recalculateBounds();

    const StaticTextItem *textItem;
};

ItemBase::VerticalAnchorLine StaticText::baselineAnchor() const
{
    int b = baselineOffset.value();
    return VerticalAnchorLine(this, y.value() + b, b);
}

} // namespace Items
} // namespace Private
} // namespace Qul
