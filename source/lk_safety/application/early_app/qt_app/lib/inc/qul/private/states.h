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
#include <qul/private/animation.h>
#include <qul/private/unicodestring.h>

namespace Qul {
namespace Private {

// Same as C++20 std::type_identity
template<typename T>
struct type_identity
{
    typedef T type;
};

namespace Items {
struct Transition : ParallelAnimation
{
    enum TransitionStatus { TransitionStart, TransitionEnd };

    // Reverse list of property animations in Transition
    // see addPropertyAnimation().
    PropertyAnimationBase *listOfPropertyAnimation;

    // This function is run when the transition finishes
    // to set properties to state values where the transition animation
    // did not end on the right value.
    // (e.g. explicitly animating from: 2; to: 7 when the new state had
    // a value of 10)
    typedef void (*FinishFunction)(Transition *, Object *);
    FinishFunction finishFunction;

    Transition()
        : ParallelAnimation()
        , listOfPropertyAnimation(NULL)
        , finishFunction(NULL)
    {}

    void start(FinishFunction f)
    {
        finishFunction = f;
        ParallelAnimation::start();
    }

    /* interceptValue / interceptBinding
     *
     * State activation functions call these to prepare the Transition animation
     * for the target state.
     *
     * If a Transition animates a property that is set in a state, the property
     * value shall not be set immediately:
     * - If the last property animation for the property does not have a "to" value,
     *   the animation shall go to the state's target property value.
     * - If the last property animation for the property does have a "to" value,
     *   the value shall be set to the state's value when the transition finishes.
     *
     * This is done by code like:
     *   if (!transition || !transition->interceptValue(atTransitionStart, &myproperty, myvalue)
     *       myproperty.setValue(myvalue);
     */

    template<typename T>
    bool interceptValue(TransitionStatus status, Property<T> *property, const typename type_identity<T>::type &value)
    {
        return interceptValueImpl(status, property, &value);
    }

    template<typename T>
    bool interceptValue(TransitionStatus status,
                        PropertyAlias<T> *property,
                        const typename type_identity<T>::type &value)
    {
        return interceptValueImpl(status, &property->property(), &value);
    }

    template<typename T, typename Binding>
    bool interceptBinding(TransitionStatus status, Property<T> *property, Binding *binding)
    {
        qul_static_assert<qul_is_same<typename Binding::PropertyType, T>::value>();
        return interceptBindingImpl(status, property, binding);
    }

    template<typename T, typename Binding>
    bool interceptBinding(TransitionStatus status, PropertyAlias<T> *property, Binding *binding)
    {
        return interceptBinding(status, &property->property(), binding);
    }

    void addPropertyAnimation(PropertyAnimationBase *child)
    {
        child->nextInTransition = listOfPropertyAnimation;
        listOfPropertyAnimation = child;
    }

protected:
    void doStop(StopReason reason) QUL_DECL_OVERRIDE
    {
        ParallelAnimation::doStop(reason);
        if (finishFunction && (reason == Animation::FinishedStop || reason == Animation::FinishedFastStop))
            finishFunction(this, parentObject());
    }

private:
    bool interceptValueImpl(TransitionStatus status, PropertyBase *property, const void *value)
    {
        for (PropertyAnimationBase *anim = listOfPropertyAnimation; anim; anim = anim->nextInTransition) {
            if (anim->getProperty() == property) {
                switch (status) {
                case TransitionStart:
                    anim->setupTo(value);
                    return true;
                case TransitionEnd:
                    return !anim->hasExplicitTo();
                }
            }
        }
        // Properties without animations are set on transition start, not on finish.
        return status == TransitionEnd;
    }
    bool interceptBindingImpl(TransitionStatus status, PropertyBase *property, BindingBase *binding)
    {
        for (PropertyAnimationBase *anim = listOfPropertyAnimation; anim; anim = anim->nextInTransition) {
            if (anim->getProperty() == property) {
                switch (status) {
                case TransitionStart:
                    anim->setupToBinding(binding);
                    return true;
                case TransitionEnd:
                    return !anim->hasExplicitTo();
                }
            }
        }
        // Properties without animations are set on transition start, not on finish.
        return status == TransitionEnd;
    }
};

// Dummy struct to make the TypeLoader happy
struct PropertyChanges : Qul::Object
{};
struct AnchorChanges : Qul::Object
{};

} // namespace Items

template<typename T>
struct State : Qul::Object
{
    typedef void (T::*StateFunction)(Items::Transition *, Items::Transition::TransitionStatus);
    StateFunction activationFunction;
    State(StateFunction activationFunction = NULL)
        : activationFunction(activationFunction)
    {}
    State(String stateName)
        : activationFunction(T::stateFromName(stateName).activationFunction)
    {}
    friend bool operator==(State<T> a, State<T> b) { return a.activationFunction == b.activationFunction; }
    friend bool operator!=(State<T> a, State<T> b) { return !(a == b); }
    inline void activate(T *obj, Items::Transition *t) const
    {
        (obj->*activationFunction)(t, Items::Transition::TransitionStart);
    }
};

} // namespace Private
} // namespace Qul
