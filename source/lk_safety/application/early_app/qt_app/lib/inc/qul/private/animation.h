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

#include <qul/private/propertybinding.h>
#include <qul/object.h>
#include <qul/global.h>
#include <qul/signal.h>
#include <qul/private/compilerdetection.h>
#include <qul/private/parentobject.h>
#include <qul/private/extradatapointer.h>
#include <platforminterface/rgba32.h>

#include <climits>

namespace Qul {
namespace Private {

class Application;

typedef float(ULEasingFunction)(float t);

// Animation interpolators
template<typename T>
T interpolate(const T &f, const T &t, float progress)
{
    if (f == t)
        return t;
    return T(f * (1.0f - progress) + t * progress);
}

template<>
bool interpolate(const bool &f, const bool &, float progress);

class String;
template<>
String interpolate(const String &f, const String &, float progress);

template<>
PlatformInterface::Rgba32 interpolate(const PlatformInterface::Rgba32 &f,
                                      const PlatformInterface::Rgba32 &t,
                                      float progress);

qreal interpolateClockwiseRotation(const qreal &f, const qreal &t, float progress);
qreal interpolateCounterclockwiseRotation(const qreal &f, const qreal &t, float progress);
qreal interpolateShortestRotation(const qreal &f, const qreal &t, float progress);

//A bit of a hack to make it seems as if this was an enum.
//(but the easing function are function pointer so unused ones are not part of the binary)
struct Easing
{
    typedef Easing Function;
    typedef Qul::Private::ULEasingFunction *EasingFunctionPtr;
    static const EasingFunctionPtr Linear, InQuad, OutQuad, InOutQuad, OutInQuad, InCubic, OutCubic, InOutCubic,
        OutInCubic, InQuart, OutQuart, InOutQuart, OutInQuart, InQuint, OutQuint, InOutQuint, OutInQuint, InSine,
        OutSine, InOutSine, OutInSine, InExpo, OutExpo, InOutExpo, OutInExpo, InCirc, OutCirc, InOutCirc, OutInCirc;
    // Those curves are not yet supported, see https://bugreports.qt.io/browse/UL-2488
    // InElastic, OutElastic, InOutElastic, OutInElastic, InBack, OutBack, InOutBack, OutInBack, InBounce, OutBounce,
    // InOutBounce, OutInBounce, Bezier

    EasingFunctionPtr data;
    Easing(EasingFunctionPtr data = NULL)
        : data(data)
    {}
    operator EasingFunctionPtr() const { return data; }
};

namespace Items {

struct Animation : Qul::Object, public Qul::Private::LinkedListNode<Animation>
{
    explicit Animation(bool groupAnimation = false)
        : Object(Type::Animation)
        , remainingLoopCount(0)
    {
        onRunningChanged.init<Animation, &Animation::onRunningChanged>();
        onRunningChanged.registerDependency(running);

        setGroupAnimation(groupAnimation);
        setStatus(NotRunning);
    }

    enum LoopCount { Infinite = INT_MAX };

    virtual ~Animation();

    void start();

    enum StopReason {
        ManualStop,       // Animation stopped from the code
        FinishedStop,     // Animation finished
        FinishedFastStop, // Animation finished in doStart(), looping is forbidden
        TransitionStop    // Another transition is starting
    };

    void stop(StopReason reason = ManualStop);

    void tick();

    struct AnimationExtraProperties
    {
        explicit AnimationExtraProperties(Animation *)
            : loops(1)
        {}
        Property<bool> alwaysRunToEnd;
        Property<int> loops;
        Signal<void(void)> finished;
        Signal<void(void)> stopped;
        Signal<void(void)> started;
    };

    friend class Qul::Private::Application;

    inline Animation *animationParent() const
    {
        Object *p = parentObject();
        if (!p || p->type() != Type::Animation)
            return NULL;
        return static_cast<Animation *>(p);
    }

    virtual void on(Property<int> *) {}
    virtual void on(Property<float> *) {}
    virtual void on(Property<double> *) {}
    virtual void on(Property<PlatformInterface::Rgba32> *) {}

    // True if the animation should progress.
    bool isTicking() const { return status() != NotRunning; }

    // padding:
    // - bit 1: isGroupProperty
    // - bit 2+3: status
    Property<bool> running;

    QUL_DECLARE_EXTRASTORAGE(AnimationExtraProperties, animationExtraProperties);

protected:
    virtual void doStart() {}
    virtual void doStop(StopReason) {}
    virtual void doTick() = 0;

    enum Status {
        NotRunning,
        // Animation is running, stop has not been called. "running" is true
        RunningNormal,
        // stop() was called, but alwaysRunToEnd was true. "running" is false
        RunningToEnd
    };

    struct RunningChangedEvent
    {
        inline void operator()(Animation *animation)
        {
            if (!animation)
                return;
            const bool expectedRunningValue = animation->status() == RunningNormal;
            if (animation->running.value() != expectedRunningValue) {
                if (!expectedRunningValue) {
                    // TODO: Correct if a RunningToEnd animation gets started again?
                    animation->start();
                } else {
                    animation->stop();
                }
            }
        }
    };

    // Is this animation a subclass of GroupAnimation?
    bool isGroupAnimation() const { return running.m_u16Padding & 0x1; }
    void setGroupAnimation(bool v) { running.m_u16Padding = (running.m_u16Padding & ~0x1) | static_cast<int>(v); }

    Status status() const { return static_cast<Status>((running.m_u16Padding & 0x6) >> 1); }
    void setStatus(Status s) { running.m_u16Padding = (running.m_u16Padding & ~0x6) | static_cast<int>(s << 1); }

    DirtyEventStaticDependency<Animation::RunningChangedEvent> onRunningChanged;

    int remainingLoopCount;

    typedef Qul::Private::LinkedList<Animation> AnimationList;
};

struct GroupAnimation : Animation
{
    GroupAnimation()
        : Animation(true)
    {}

    AnimationList children;
    friend void appendChild(GroupAnimation *parent, Animation *child)
    {
        parent->children.insert(child);
        child->setParent(parent);
    }

    void on(Property<int> *o) { attachOnChildren(o); }

    void on(Property<float> *o) { attachOnChildren(o); }

    void on(Property<double> *o) { attachOnChildren(o); }

    void on(Property<PlatformInterface::Rgba32> *o) { attachOnChildren(o); }

private:
    template<typename TProperty>
    void attachOnChildren(TProperty o)
    {
        Animation *current = children.head;
        while (current) {
            current->on(o);
            current = current->next;
        }
    }
};

struct ParallelAnimation : GroupAnimation
{
protected:
    void doTick() QUL_DECL_OVERRIDE;
    void doStart() QUL_DECL_OVERRIDE;
    void doStop(StopReason reason) QUL_DECL_OVERRIDE;
};

struct SequentialAnimation : GroupAnimation
{
    SequentialAnimation()
        : current(NULL)
        , reverted(true)
    {}

protected:
    void doTick() QUL_DECL_OVERRIDE;
    void doStart() QUL_DECL_OVERRIDE;
    void doStop(StopReason reason) QUL_DECL_OVERRIDE;

private:
    void startNext();

    Animation *current;
    bool reverted;
};

struct PauseAnimation : Animation
{
    PauseAnimation()
        : duration(250)
        , t(0)
    {}

    Property<qreal> duration;

protected:
    void doTick();
    void doStart();

private:
    float t;
};

struct PropertyAnimationBase : Animation
{
    PropertyAnimationBase()
        : Animation()
        , nextInTransition(NULL)
    {}

    virtual Qul::Private::PropertyBase *getProperty() = 0;
    virtual void setupTo(const void *) = 0;
    virtual void setupToBinding(Qul::Private::BindingBase *) = 0;
    virtual bool hasExplicitTo() const = 0;
    PropertyAnimationBase *nextInTransition;
};

template<typename T>
struct PropertyAnimation : PropertyAnimationBase
{
    PropertyAnimation()
        : property(NULL)
        , duration(250)
        , interpolator(Qul::Private::interpolate<T>)
        , moveBinding(false)
        , t(0.)
    {
        easing.type.setValue(Easing::Linear);
    }

    Property<T> *property;
    Property<T> from;
    Property<T> to;
    Property<int> duration;

    typedef T (*Interpolator)(const T &from, const T &to, float progress);

    struct EasingGroup
    {
        Property<Easing::Function> type;
    } easing;

    void setProperty(Property<T> *o) { this->property = o; }

    void setProperty(PropertyAlias<T> *o) { setProperty(&o->property()); }

    using PropertyAnimationBase::on;
    void on(Property<T> *o)
    {
        this->property = o;
        if (this->running.value() || !this->running.wasExplicitlySet())
            start();
    }

    void on(PropertyAlias<T> *o) { on(&o->property()); }

protected:
    void doStart();
    void doStop(StopReason reason)
    {
        if (moveBinding) {
            if (reason == FinishedStop || reason == FinishedFastStop) {
                Qul::Private::BindingBase *b = to.binding();
                to.removeBindingImpl();
                property->setBindingImpl(b);
            } else if (reason == TransitionStop) {
                to.removeBindingImpl();
            }
        }
    }
    void doTick();

    Interpolator interpolator;
    bool moveBinding; // Once the animation is finished, we move the binding of 'to' to the property

    Qul::Private::PropertyBase *getProperty() { return property; }
    void setupTo(const void *value)
    {
        to.setImplicitly(*static_cast<const T *>(value));
        moveBinding = false;
    }
    void setupToBinding(Qul::Private::BindingBase *b)
    {
        if (!to.wasExplicitlySet()) {
            property->removeBindingImpl();
            to.setBindingImpl(b);
            moveBinding = true;
        }
    }
    bool hasExplicitTo() const { return to.wasExplicitlySet(); }

    float t;
    bool updateValue(bool isStart = false);
};

template<typename T>
struct NumberAnimation : public PropertyAnimation<T>
{};

struct RotationAnimation : public PropertyAnimation<qreal>
{
    RotationAnimation()
        : direction(Numerical)
    {
        onDirectionChanged.init<RotationAnimation, &RotationAnimation::onDirectionChanged>();
        onDirectionChanged.registerDependency(direction);
    }

    enum Direction { Numerical, Clockwise, Counterclockwise, Shortest };

    Property<RotationAnimation::Direction> direction;

protected:
    struct DirectionChangedEvent
    {
        inline void operator()(RotationAnimation *animation)
        {
            if (!animation)
                return;
            switch (animation->direction.value()) {
            case RotationAnimation::Clockwise:
                animation->interpolator = Qul::Private::interpolateClockwiseRotation;
                break;
            case RotationAnimation::Counterclockwise:
                animation->interpolator = Qul::Private::interpolateCounterclockwiseRotation;
                break;
            case RotationAnimation::Shortest:
                animation->interpolator = Qul::Private::interpolateShortestRotation;
                break;
            default:
                break;
            }
        }
    };

    Qul::Private::DirtyEventStaticDependency<RotationAnimation::DirectionChangedEvent> onDirectionChanged;
};

class XAnimator : public NumberAnimation<int>
{};
class YAnimator : public NumberAnimation<int>
{};
class WidthAnimator : public NumberAnimation<int>
{};
class HeightAnimator : public NumberAnimation<int>
{};
class OpacityAnimator : public NumberAnimation<qreal>
{};
class ColorAnimation : public PropertyAnimation<PlatformInterface::Rgba32>
{};

struct ScriptAction : public Animation
{
protected:
    void doStart();
    void doTick() {}

    virtual void run() {}
};

} // namespace Items

extern bool allBehaviorDisabled;

template<typename T>
struct BehaviorLight : ParentObject, BindingBase
{
    BehaviorLight()
        : BindingBase(bindingCall)
        , originalBinding(NULL)
    {}

    typedef T PropertyType;

    friend void appendChild(BehaviorLight<PropertyType> *parent, Items::PropertyAnimation<PropertyType> *anim)
    {
        assert(!parent->animation);
        parent->animation = anim;
    }

    void on(Property<PropertyType> *o) { o->setBinding(this); }

    void on(PropertyAlias<T> *o) { on(&o->property()); }

protected:
    // When the flag is true, this is a Behavior
    FlagPointer<Items::PropertyAnimation<PropertyType> > animation;

private:
    BindingBase *originalBinding;

    inline bool isEnabled();

    static bool bindingCall(BindingBase *self, InternalCall call, PropertyBase *p, void *v)
    {
        BehaviorLight<PropertyType> *self_ = static_cast<BehaviorLight<PropertyType> *>(self);
        switch (call) {
        case Install:
            return self_->install(reinterpret_cast<Property<PropertyType> *>(p));
        case Execute:
        case MarkDirty:
            return self_->executeOriginalBinding(reinterpret_cast<Property<PropertyType> *>(p));
        case SetBinding:
            self_->setBinding(reinterpret_cast<Property<PropertyType> *>(p), reinterpret_cast<BindingBase *>(v));
            return true;
        case SetValue:
        case SetImplicitValue:
            self_->setValue(reinterpret_cast<Property<PropertyType> *>(p),
                            *reinterpret_cast<const PropertyType *>(v),
                            call);
            return true;
        case GetExplicitBinding:
            return self_->originalBinding
                   && self_->originalBinding->call(self_->originalBinding, GetExplicitBinding, p, v);
        }
        QUL_UNREACHABLE();
        return false;
    }
    void setValue(Property<PropertyType> *prop, const PropertyType &v, InternalCall call)
    {
        if (originalBinding && originalBinding->call(originalBinding, call, prop, const_cast<PropertyType *>(&v))) {
            if (prop->isDirty())
                executeOriginalBinding(prop);
            return;
        }
        originalBinding = NULL;
        applyValue(prop, v);
    }

    void applyValue(Property<PropertyType> *prop, const PropertyType &v)
    {
        Items::PropertyAnimation<PropertyType> *a = animation.data();
        if (a && isEnabled() && !allBehaviorDisabled) {
            // don't restart the animation if it's already running
            if (a->running.value() && a->property == prop && a->to.value() == v)
                return;
            a->property = prop;
            a->from.setValue(prop->m_value);
            a->to.setValue(v);
            a->start();
        } else {
            prop->setValueAndBypassBinding(v);
        }
    }
    void setBinding(Property<PropertyType> *prop, BindingBase *newB)
    {
        if (originalBinding && originalBinding->call(originalBinding, BindingBase::SetBinding, prop, newB))
            return; // Does that make sense?
        originalBinding = newB;
        if (executeOriginalBinding(prop))
            prop->propertyData.clearFlag();
    }
    bool executeOriginalBinding(Property<PropertyType> *prop)
    {
        bool success = true;
        if (originalBinding) {
            PropertyType v;
            success = originalBinding->call(originalBinding, BindingBase::Execute, prop, &v);
            applyValue(prop, v);
        }
        return success;
    }
    bool install(Property<PropertyType> *prop)
    {
        originalBinding = prop->binding();
        if (!originalBinding)
            return false;
#ifndef UL_PROPERTY_UPDATE_PUSH
        if (originalBinding && prop->isDirty()) {
            prop->value(); // force the binding to evaluate;
        }
#endif
        prop->removeBindingImpl();
        assert(!firstDependency);
        if (FlagPointer<DependencyBase> &firstDep = prop->firstDependency()) {
            assert(&firstDep == &prop->propertyData);
            firstDependency = firstDep;
            firstDep->setPrevCheck(&firstDependency, &prop->propertyData);
        }
        prop->propertyData = this;
        return true;
    }
};

template<typename T>
struct Behavior : BehaviorLight<T>, ExtendsBase
{
    Property<bool> enabled;
    Behavior()
        : enabled(true)
    {
        this->animation.setFlag();
    }
};

template<typename T>
inline bool BehaviorLight<T>::isEnabled()
{
    return !animation.flag() || static_cast<Behavior<T> *>(this)->enabled.value();
}

} // namespace Private
} // namespace Qul
