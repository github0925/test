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

#pragma once

#include <qul/private/array.h>
#include <qul/private/repaintstrategy.h>

#include <platforminterface/rect.h>

#include <platform/platform.h>

#define MAX_DIRTY_NODES 45

#define REGION_PARTIAL_UPDATE

namespace Qul {
namespace Private {

namespace Items {
struct ItemBase;
}

namespace RenderTree {
class Node;
}

class RenderContext
{
public:
    using DirtyNodesType = VarLengthArray<RenderTree::Node *, MAX_DIRTY_NODES>;

#ifdef REGION_PARTIAL_UPDATE
    using RepaintStrategy = RegionRepaintStrategy;
#else
    using RepaintStrategy = BoundingRectRepaintStrategy;
#endif

    RenderContext(Items::ItemBase *root);

    bool dirty() const;
    bool fullRepaint() const;
    bool hasRemovedRect() const;
    void setFullRepaint(bool value);
    void clearRemovedRect();
    void clearDirtyNodes();

    bool withinContext(Items::ItemBase *item) const;
    void removeNode(RenderTree::Node *node);
    void addDirtyNode(RenderTree::Node *node);

    const PlatformInterface::Rect &removedRect() const;
    const DirtyNodesType &dirtyNodes() const;
    Items::ItemBase *root() const;

    RepaintStrategy &repaintStrategy() { return m_repaintStrategy; }

    static RenderContext *contextForItem(Items::ItemBase *item);

private:
    Items::ItemBase *m_root;
    bool m_fullRepaint;
    DirtyNodesType m_dirtyNodes;
    RepaintStrategy m_repaintStrategy;
    PlatformInterface::Rect m_removedRect;
};

} // namespace Private
} // namespace Qul
