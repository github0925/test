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
#include <qul/private/region.h>

#include <platforminterface/rect.h>

#include <platform/platform.h>

namespace Qul {
namespace Private {

namespace RenderTree {
class Node;
}

class BoundingRectRepaintStrategy
{
public:
    enum RepaintMode { PartialRepaint, FullRepaint };

    typedef PlatformInterface::Rect *Iterator;
    Iterator begin() { return &currentDirtyBoundingRect; };

    Iterator end() { return &currentDirtyBoundingRect + 1; };

    int stride() const { return sizeof(PlatformInterface::Rect); }

    void addDirtyRect(const RenderTree::Node *, const PlatformInterface::Rect &dirtyRect)
    {
        currentDirtyBoundingRect |= dirtyRect;
    }

    // gets called when doing a full repaint
    void setDirtyLayerRect(const PlatformInterface::Rect &layerRect) { currentDirtyBoundingRect = layerRect; }

    void finalize()
    {
        if (flipped) {
            PlatformInterface::Rect combined = currentDirtyBoundingRect | lastDirtyBoundingRect;
            lastDirtyBoundingRect = currentDirtyBoundingRect;
            currentDirtyBoundingRect = combined;
        }
    }

    PlatformInterface::Rect bounds() const { return currentDirtyBoundingRect; };

    void reset(Qul::Platform::FrameBufferingType bufferingType)
    {
        node = nullptr;
        repaintMode = PartialRepaint;
        currentDirtyBoundingRect = PlatformInterface::Rect();
        flipped = bufferingType == Platform::FlippedDoubleBuffering;
    }

    RenderTree::Node *node = nullptr;
    RepaintMode repaintMode = PartialRepaint;

protected:
    PlatformInterface::Rect lastDirtyBoundingRect;

    bool flipped = true;

private:
    PlatformInterface::Rect currentDirtyBoundingRect;
};

class RegionRepaintStrategy : public BoundingRectRepaintStrategy
{
public:
    typedef PlatformInterface::Rect *Iterator;
    Iterator begin()
    {
        if (fallback)
            return BoundingRectRepaintStrategy::begin();
        return &dirtyRegion.rects.data()->rect;
    };

    Iterator end()
    {
        if (fallback)
            return BoundingRectRepaintStrategy::end();
        return reinterpret_cast<Iterator>(reinterpret_cast<char *>(begin()) + dirtyRegion.rects.size() * stride());
    };

    int stride() const
    {
        if (fallback)
            return BoundingRectRepaintStrategy::stride();
        return sizeof(RectNode);
    }

    PlatformInterface::Rect bounds() const { return BoundingRectRepaintStrategy::bounds(); };

    void addDirtyRect(const RenderTree::Node *node, const PlatformInterface::Rect &dirtyRect)
    {
        BoundingRectRepaintStrategy::addDirtyRect(node, dirtyRect);

        if (!fallback)
            addDirtyRectsForNode(node, dirtyRect);
    }

    // gets called when doing a full repaint
    void setDirtyLayerRect(const PlatformInterface::Rect &layerRect)
    {
        BoundingRectRepaintStrategy::setDirtyLayerRect(layerRect);
        fallback = true;
    }

    void reset(Qul::Platform::FrameBufferingType bufferingType)
    {
        BoundingRectRepaintStrategy::reset(bufferingType);
        fallback = false;
        rectCounts[frameIndex] = 0;
    }

    void finalize();

private:
    void addDirtyRectsForNode(const RenderTree::Node *node, const PlatformInterface::Rect &dirtyRect);

    PlatformInterface::Rect &rectAt(int index);

    inline int getRectArea(const PlatformInterface::Rect &rect) const { return rect.width() * rect.height(); }

    void appendDirtyRect(const PlatformInterface::Rect &rect);

    static const int maxRectCount = 90;

    int rectCounts[2] = {0, 0};
    RectNode rectNodeData[2 * maxRectCount];
    Region<maxRectCount> dirtyRegion;

    int frameIndex = 0;

    bool fallback = false;
};

} // namespace Private
} // namespace Qul
