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
#ifndef QUL_APPLICATION_H
#define QUL_APPLICATION_H

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

#include <qul/private/array.h>
#include <platforminterface/rect.h>
#include <qul/private/animation.h>
#include <qul/private/input.h>
#include <qul/private/rendercontext.h>

namespace Qul {
namespace PlatformInterface {
class Screen;
}
namespace Private {
namespace Items {
class Flickable;
}

namespace Items {
struct Animation;
template<typename T>
struct NumberAnimation;
struct ItemLayer;
struct ItemBase;
struct Application;
struct Screen;
} // namespace Items
struct TouchEvent;

namespace RenderTree {
class Node;
}

class Application
{
public:
    Application();
    ~Application();

    void update(quint64 timestamp);

    inline bool running() { return m_running; }

    void exec();

    static Application *instance() { return self; }

    void setRootItem(Items::ItemBase *root);
    void setRootItem(Items::Application *container);

    void itemDirty(Items::ItemBase *item);
    void nodeRemoved(RenderTree::Node *node);

    // Returns true if any Animation objects are currently running (even though they might
    // not be causing any visual updates)
    bool animationsRunning() const { return !m_animationList.isEmpty(); }

    // Returns true if there are visual animations ongoing (i.e. if the last animation tick caused
    // any visual updates)
    bool isAnimating() const;

    qtime currentTimeStamp() const;

    unsigned int frameCount() const { return m_frameCount; }

    void animationTick();

    bool hasDirtyNodes() const;

    quint64 calculateNextTimeout() const;

    void grabMouse(Items::ItemBase *item);
    void ungrabMouse(Items::ItemBase *item);
    Items::ItemBase *mouseGrabber(Items::Screen *screen = nullptr);

    // may be called from interrupts via EventQueue::postEvent()
    void requestEventProcessing();
    void handleTouchEvent(PlatformInterface::Screen *screen, uint64_t timestamp, const TouchEvent &event);
    void handleKeyEvent(const KeyEvent &event);

protected:
    void initObjects();
    void prepareFrame();
    void setTimestampRoutine(qtime (*routine)(void));

private:
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    friend struct Items::Animation;
    template<typename T>
    friend struct Items::NumberAnimation;

    friend class Qul::Private::Items::Flickable;

    bool handleTouchEventForRoot(Items::Screen *screen, Items::ItemBase *root, TouchEvent &event);

    void init();

    enum RepaintPolicy { MaybeRepaint, ForceRepaint };
    void repaint(RepaintPolicy policy);

    void recomputeDirtyRegions(Items::ItemLayer *layer, RepaintPolicy policy);
    PlatformInterface::Rect repaintDirtyRegions(RenderContext &context,
                                                PlatformInterface::Point globalPosition,
                                                int layerRefreshInterval,
                                                bool wasAnimating);
    bool shouldSkipFrame(Items::ItemLayer *itemLayer, unsigned int frame);

    Items::Animation::AnimationList m_animationList;

    static Application *self;

    Items::Application *m_root;

    bool m_running;

    // target interval between frames to achieve smooth animations
    // (e.g. by consistently running at 30 fps instead of an unpredictable rate below 60 fps)
    int m_refreshInterval;
    // amount of consecutive under budget frames, used to determine when to reduce the refresh interval
    int m_goodFrames;
    // whether refresh interval was increased the last frame (in which case we ignore the frame statistics)
    bool m_increasedRefreshInterval;

    // Whether scheduleEngineUpdate(0) has already been called, see
    // requestEventProcessing() and update().
    volatile bool m_requestedEventProcessingUpdate;

    int m_maxLayerRefreshInterval;
    mutable unsigned int m_lastAnimationFrame;
    unsigned int m_frameCount;

    // the number of frames since isAnimating() started returning true
    unsigned int m_runningAnimationFrame;
    qtime (*m_timestampRoutine)(void);
};

} // namespace Private
} // namespace Qul

#endif
