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
#include <qul/private/rendertree.h>

namespace Qul {
namespace Private {
namespace Items {

struct ItemBuffer : ItemBase
{
    enum ExtraFlag {
        RedrawItemBufferFlag = 1U << 0,
    };

    ItemBuffer();
    ~ItemBuffer();

    TransformProperty transform;
    Property<bool> useAlphaChannel;

    RenderTree::ItemBufferNode node;

protected:
    Qul::Private::RenderTree::Node *renderNode(RenderNodeUpdate update, PlatformInterface::Rect *localBounds) override;

    void render();

    Qul::Image image;

    DirtyListNode dirtyItemBufferListNode;
    typedef DirtyList<ItemBuffer, &ItemBuffer::dirtyItemBufferListNode> DirtyItemBufferList;
    static DirtyItemBufferList dirtyItemBuffers;

    friend class Qul::Private::Application;
    friend struct ItemBase;
};

} // namespace Items
} // namespace Private
} // namespace Qul
