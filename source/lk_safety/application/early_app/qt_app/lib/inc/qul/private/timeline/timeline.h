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

#include <qul/object.h>
#include <qul/property.h>
#include <qul/private/timeline/timelineanimation.h>

namespace Qul {
namespace Private {
namespace Timeline {

struct KeyframeGroupBase;

struct Timeline : public ParentObject
{
    Timeline();

    ListProperty<TimelineAnimation> animations;
    Property<bool> enabled;
    Property<qreal> currentFrame;
    Property<qreal> startFrame;
    Property<qreal> endFrame;

private:
    struct UpdateTimeline
    {
        void operator()(Timeline *self);
    };
    friend struct UpdateTimeline;

    struct SortKeyframes
    {
        void operator()(Timeline *self);
    };
    friend struct SortKeyframes;

    KeyframeGroupBase *m_firstKeyframeGroupBase;
    DirtyEventStaticDependency<UpdateTimeline> m_updateTimelineDependency;
    DirtyEventStaticDependency<SortKeyframes> m_sortKeyframesDependency;
    bool m_isEnabled : 1;
    bool m_initialized : 1;

    friend void appendChild(Timeline *parent, KeyframeGroupBase *child);
};

} // namespace Timeline
} // namespace Private
} // namespace Qul
