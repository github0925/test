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
#include <qul/signal.h>
#include <qul/private/qulassert.h>
#include <qul/private/unicodestring.h>
#include <qul/private/animation.h>

namespace Qul {
namespace Private {
namespace Timeline {

struct KeyframeGroupBase;
struct Timeline;
template<typename T>
struct KeyframeGroup;

struct KeyframeBase : public Object
{
    KeyframeBase();

    struct EasingGroup
    {
        Property<Easing::Function> type;
    } easing;
    Property<qreal> frame;

private:
    KeyframeBase *m_next;

    // friendship, to allow access to m_next
    friend struct KeyframeGroupBase;
    template<typename T>
    friend struct KeyframeGroup;
    friend struct Timeline;
    friend void appendChild(KeyframeGroupBase *parent, KeyframeBase *child);
};

template<typename T>
struct Keyframe : public KeyframeBase
{
    Property<T> value;
};

struct KeyframeGroupBase : public ParentObject
{
    KeyframeGroupBase();

    virtual PropertyBase &valueProperty(KeyframeBase *keyframe) = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual void updateState(qreal startFrame, qreal currentFrame) const = 0;

protected:
    void sortKeyframes();

    KeyframeGroupBase *m_next;
    KeyframeBase *m_firstKeyframe;

    friend struct Timeline; // to allow access to m_next
    friend void appendChild(KeyframeGroupBase *parent, KeyframeBase *child);
    friend void appendChild(Timeline *parent, KeyframeGroupBase *child);
};

template<typename T>
struct KeyframeGroup : public KeyframeGroupBase
{
    void setProperty(Property<T> *prop) { property = prop; }
    void setProperty(PropertyAlias<T> *prop) { property = &prop->property(); }

    PropertyBase &valueProperty(KeyframeBase *keyframe) QUL_DECL_OVERRIDE;

    // save the current property value in originalValue or restore it
    void setEnabled(bool enabled) QUL_DECL_OVERRIDE;

    // set property to value based on keyframes
    void updateState(qreal startFrame, qreal currentFrame) const QUL_DECL_OVERRIDE;

private:
    Property<T> *property;
    T originalValue;
};

template<typename T>
PropertyBase &KeyframeGroup<T>::valueProperty(KeyframeBase *keyframe)
{
    return static_cast<Keyframe<T> *>(keyframe)->value;
}

template<typename T>
void KeyframeGroup<T>::setEnabled(bool enabled)
{
    if (enabled) {
        originalValue = property->value();
    } else {
        property->setValueAndBypassBinding(originalValue);
    }
}

template<typename T>
void KeyframeGroup<T>::updateState(qreal startFrame, qreal currentFrame) const
{
    // There is an initial implicit keyframe at startFrame with the originalValue,
    // which is represented by NULL here.
    const KeyframeBase *from = NULL;
    const KeyframeBase *to = NULL;

    // "to" shall be the first keyframe to the right of currentFrame
    // "from" is the one before
    for (const KeyframeBase *it = m_firstKeyframe; it; it = it->m_next) {
        if (currentFrame <= it->frame.value()) {
            to = it;
            break;
        }
        from = it;
    }
    if (!to) {
        // no keyframes to the right of currentFrame: use last keyframe
        to = from;
    } else if (!from && currentFrame < startFrame) {
        // to the left of startFrame: use the implicit originalValue frame
        to = NULL;
    }

    const T &fromValue = from ? static_cast<const Keyframe<T> *>(from)->value.value() : originalValue;
    const T &toValue = to ? static_cast<const Keyframe<T> *>(to)->value.value() : originalValue;
    const qreal fromFrame = from ? from->frame.value() : startFrame;
    const qreal toFrame = to ? to->frame.value() : startFrame;

    // Do this first, so if fromFrame == toFrame, the toValue is preferred.
    // That is important because frequently the first explicit keyframe has
    // the same frame value as the implicit frame at startFrame.
    if (currentFrame >= toFrame) {
        property->setValueAndBypassBinding(toValue);
    } else if (currentFrame <= fromFrame) {
        property->setValueAndBypassBinding(fromValue);
    } else {
        assert(toFrame != fromFrame); // the above two conditions ensure this
        qreal t = (currentFrame - fromFrame) / (toFrame - fromFrame);
        t = to->easing.type.value()(t);
        property->setValueAndBypassBinding(Qul::Private::interpolate<T>(fromValue, toValue, t));
    }
}

} // namespace Timeline
} // namespace Private
} // namespace Qul
