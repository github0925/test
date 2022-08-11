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

#include <functional>
#include <memory>
#include <string>

#include <qul/private/array.h>
#include <qul/private/dirtylist.h>
#include <qul/private/scopedpointer.h>
#include <qul/private/flagpointer.h>
#include <qul/private/compilerdetection.h>
#include <qul/property.h>

#define QUL_DEPENDENCIES Qul::Private::fixedLengthArray<Qul::Private::PropertyBase *>

namespace Qul {
namespace Private {

template<typename T>
struct Behavior;

namespace Items {
template<typename T>
struct PropertyAnimation;
}

struct DynDependencyNode
{
    DependencyNode node;
    ScopedPointer<DynDependencyNode> nextDynNode;
};

struct EvaluationFrame
{
    EvaluationFrame()
        : currentProperty(NULL)
        , staticNodes(NULL)
        , numStaticNode(0)
        , firstDynNode(NULL)
        , lastNodes(NULL)
        , nbRegistered(0)
        , allocationFailure(false)
    {}

    EvaluationFrame(PropertyBase *currentProperty,
                    DependencyNode *staticNodes,
                    int numStaticNodes,
                    ScopedPointer<DynDependencyNode> *dynNodes,
                    int nbRegistered = 0)
        : currentProperty(currentProperty)
        , staticNodes(staticNodes)
        , numStaticNode(numStaticNodes)
        , firstDynNode(dynNodes)
        , lastNodes(dynNodes)
        , nbRegistered(nbRegistered)
        , allocationFailure(false)
    {}

    PropertyBase *currentProperty;
    DependencyNode *staticNodes;
    int numStaticNode;
    ScopedPointer<DynDependencyNode> *firstDynNode;
    ScopedPointer<DynDependencyNode> *lastNodes;
    int nbRegistered;
    bool allocationFailure;
};

extern EvaluationFrame currentFrame;

struct EvaluationScope
{
    explicit EvaluationScope(EvaluationFrame frame)
        : previous(currentFrame)
    {
        currentFrame = frame;
    }

    ~EvaluationScope()
    {
        // Cleanup unused node;
        for (int i = currentFrame.nbRegistered; i < currentFrame.numStaticNode; ++i) {
            currentFrame.staticNodes[i].remove();
        }
        if (QUL_UNLIKELY(currentFrame.allocationFailure)) {
            if (currentFrame.firstDynNode)
                currentFrame.firstDynNode->reset();
            if (previous.firstDynNode) {
                previous.firstDynNode->reset();
                previous.lastNodes = previous.firstDynNode;
            }
            previous.allocationFailure = true;
        } else {
            if (currentFrame.lastNodes)
                currentFrame.lastNodes->reset();
        }

        currentFrame = previous;
    }
    EvaluationFrame previous;
};

struct ChangeEventBase
{
    typedef void (*Call)(ChangeEventBase *);

    ChangeEventBase(Call c)
        : call(c)
    {}

    ~ChangeEventBase();

    Call call;
    void dirty();

    static bool emitChangeEvents();

protected:
    void registerDependencies(DependencyNode *deps, PropertyBase *const *props, int n)
    {
        for (int i = 0; i < n; ++i)
            props[i]->registerDependency(deps[i].init(this));
    }

protected:
    DirtyListNode dirtyListNode;

    typedef DirtyList<ChangeEventBase, &ChangeEventBase::dirtyListNode> DirtyChangeEventList;
    static DirtyChangeEventList &getDirtyChangeEventList();
};

void removeReverseDependencies(DependencyNode *revDependencies,
                               int count,
                               ScopedPointer<DynDependencyNode> *dynDependency);

template<int NumDeps>
struct RevDependenciesContainer
{
    FixedLengthArray<DependencyNode, NumDeps> revDependencies;
};

template<>
struct RevDependenciesContainer<0>
{
    static struct RevDependencies
    {
        DependencyNode *data() { return NULL; }
    } revDependencies;
};

/** For use with the Binding constructor to signal no-param operator()() */
struct BindingFunctorNoParameters
{};

/** A binding that can be assigned to a Property.
 *
 * Bindings track the properties they depend on and are automatically
 * reevaluated when dirty.
 *
 * The Functor usually needs an T operator()(Object *), see init(). To
 * use it with a Functor that has a T operator()() see the constructor
 * that takes a BindingFunctorNoParameters.
 */
template<typename T, typename Functor, int NumDeps>
struct Binding : BindingBase, RevDependenciesContainer<NumDeps>, Functor
{
    typedef T PropertyType;

    /// Construct. Call to init() is required.
    explicit Binding(Functor f)
        : BindingBase(NULL)
        , Functor(f)
    {}

    /** Create a Binding for a functor that has a no-param call operator.
     *
     * Pass BindingFunctorNoParameters() as the second argument.
     */
    template<typename BindingFunctorNoParameters>
    Binding(Functor f, BindingFunctorNoParameters)
        : BindingBase(&callFunctionNoArg<true>)
        , Functor(f)
    {}

    template<typename Object, Binding Object::*Member>
    void init()
    {
        call = &callFunctionWithArg<Object, Member>;
    }

    ScopedPointer<DynDependencyNode> dynDependencies;

private:
    // This helper function exists for code-size reasons.
    bool commonCallFunction(InternalCall call, void *v)
    {
        switch (call) {
        case SetBinding:
        case SetValue:
            removeReverseDependencies(this->revDependencies.data(), NumDeps, &dynDependencies);
            return false;
        case GetExplicitBinding:
            *reinterpret_cast<BindingBase **>(v) = this;
            return true;
        case MarkDirty:
        case Install:
        case Execute:
        case SetImplicitValue:
            return false;
        }
        QUL_UNREACHABLE();
        return true;
    }

    // This helper function exists for code-size reasons.
    template<typename Object>
    static bool commonCallFunctionWithArg(
        BindingBase *self, InternalCall call, PropertyBase *prop, void *v, size_t offset)
    {
        Binding *self_ = static_cast<Binding *>(self);
        if (call != Execute)
            return self_->commonCallFunction(call, v);

        EvaluationScope scope(EvaluationFrame(prop, self_->revDependencies.data(), NumDeps, &self_->dynDependencies));
        Object *object = reinterpret_cast<Object *>(reinterpret_cast<char *>(self) - offset);
#ifdef UL_PROPERTY_UPDATE_PUSH
        if (!v)
            reinterpret_cast<Property<T> *>(prop)->setValueAndBypassBinding((*this)(object));
        else
#endif
            *reinterpret_cast<T *>(v) = (*self_)(object);
        return !currentFrame.allocationFailure;
    }

    template<typename Object, Binding Object::*Member>
    static bool callFunctionWithArg(BindingBase *self, InternalCall call, PropertyBase *prop, void *v)
    {
        const size_t offset = offsetOf(Member);

        return commonCallFunctionWithArg<Object>(self, call, prop, v, offset);
    }

    // The dummy template arguments avoids errors when Functor has no operator()().
    template<bool dummy>
    static bool callFunctionNoArg(BindingBase *self, InternalCall call, PropertyBase *prop, void *v)
    {
        Binding *self_ = static_cast<Binding *>(self);
        if (call != Execute)
            return self_->commonCallFunction(call, v);

        EvaluationScope scope(EvaluationFrame(prop, self_->revDependencies.data(), NumDeps, &self_->dynDependencies));
#ifdef UL_PROPERTY_UPDATE_PUSH
        if (!v)
            reinterpret_cast<Property<T> *>(prop)->setValueAndBypassBinding((*self_)());
        else
#endif
            *reinterpret_cast<T *>(v) = (*self_)();
        return !currentFrame.allocationFailure;
    }
};

template<typename T>
struct PropertyAlias
{
    typedef T Type;
    explicit PropertyAlias()
    {
        // init() must be called if using this constructor
    }

    explicit PropertyAlias(Property<T> *prop)
        : m_property(prop)
    {
        assert(prop);
    }

    explicit PropertyAlias(PropertyAlias<T> *alias)
        : m_property(alias)
    {
        assert(alias);
    }

    void init(Property<T> *prop)
    {
        m_property = prop;
        assert(prop);
    }

    void init(PropertyAlias<T> *alias)
    {
        m_property = alias;
        assert(alias);
    }

    Property<T> &property() const
    {
        assert(!m_property.isNull());
        if (m_property.isT1())
            return *m_property.asT1();
        else
            return m_property.asT2()->property();
    }

    template<typename Binding>
    void setBinding(Binding *b)
    {
        property().setBinding(b);
    }

    void setValue(const T &v) { property().setValue(v); }

    T value() const { return property().value(); }

private:
    BiPointer<Property<T>, PropertyAlias<T> > m_property;
};

template<typename Functor, int NumDeps = 1>
struct DirtyEvent : ChangeEventBase, Functor
{
    explicit DirtyEvent(Functor f)
        : ChangeEventBase(callMe)
        , Functor(f)
    {
        // init() must be called by the calling code
    }

    explicit DirtyEvent(Functor f, PropertyBase *const dependencies[])
        : ChangeEventBase(callMe)
        , Functor(f)
    {
        registerDependencies(revDependencies, dependencies, NumDeps);
    }

    void init(PropertyBase *const dependencies[]) { registerDependencies(revDependencies, dependencies, NumDeps); }

private:
    static void callMe(ChangeEventBase *self) { (*static_cast<DirtyEvent *>(self))(); }
    DependencyNode revDependencies[NumDeps];
};

struct ChangeEventBaseStaticDependency : ChangeEventBase
{
    ChangeEventBaseStaticDependency()
        : ChangeEventBase(NULL)
    {
        // the pointer is stored as DependencyBase*, but due to
        // the flag, it's only used as ChangeEventBase*
        m_staticDependency.next = reinterpret_cast<DependencyBase *>(this);
        m_staticDependency.next.setFlag();
    }

    DependencyBase m_staticDependency;
};

/** DirtyEvent, but the Property dependency is more memory efficient.
 *
 * Warning: Each Property can only have a single static dependency!
 */
template<typename Functor>
struct DirtyEventStaticDependency : ChangeEventBaseStaticDependency, Functor
{
    DirtyEventStaticDependency()
        : Functor()
    {
        // init() must be called by the calling code
    }

    explicit DirtyEventStaticDependency(Functor f)
        : Functor(f)
    {
        // init() must be called by the calling code
    }

    template<typename T>
    void initNoArgumentFunctor(T *)
    {
        call = &callNoArgs<T>;
    }

    template<typename Object, DirtyEventStaticDependency Object::*Member>
    void init()
    {
        call = &callWithSelf<Object, Member>;
    }

    void registerDependency(PropertyBase &prop) { prop.registerStaticDependency(&m_staticDependency); }

private:
    template<typename T>
    static void callNoArgs(ChangeEventBase *event)
    {
        (*static_cast<DirtyEventStaticDependency *>(event))();
    }

    template<typename Object, DirtyEventStaticDependency Object::*Member>
    static void callWithSelf(ChangeEventBase *event)
    {
        const size_t offset = offsetOf(Member);

        Object *self = reinterpret_cast<Object *>(reinterpret_cast<char *>(event) - offset);
        (*static_cast<DirtyEventStaticDependency *>(event))(self);
    }
};

template<typename Functor, typename Type>
struct ChangedEvent : ChangeEventBase, Functor
{
    ChangedEvent(Functor f, Property<Type> *dependency)
        : ChangeEventBase(callMe)
        , Functor(f)
#ifndef UL_PROPERTY_UPDATE_PUSH
        , prop(dependency)
        , value()
#endif
    {
        PropertyBase *const deps[] = {dependency};
        registerDependencies(&revDependency, deps, 1);
    }

    ChangedEvent(Functor f, PropertyAlias<Type> *dependency)
        : ChangeEventBase(callMe)
        , Functor(f)
#ifndef UL_PROPERTY_UPDATE_PUSH
        , prop(&dependency->property())
        , value()
#endif
    {
#ifndef UL_PROPERTY_UPDATE_PUSH
        PropertyBase *const deps[] = {prop.data()};
#else
        PropertyBase *const deps[] = {&dependency->property()};
#endif
        registerDependencies(&revDependency, deps, 1);
    }

private:
    static void callMe(ChangeEventBase *self)
    {
        ChangedEvent *event = static_cast<ChangedEvent *>(self);
#ifndef UL_PROPERTY_UPDATE_PUSH
        Type v = event->prop->value();

        // As property dependencies are recursively marked dirty, an intermediate
        // re-evaluation of a binding may not result in an actual change of value.
        // So we need to check for that situation here and avoid triggering the
        // event as the value hasn't changed.

        bool markDirty = false;
        bool assignValue = false;
        if (event->prop.flag()) {
            PropertyTraits<Type>::valueEquality(event->value, v, &markDirty, &assignValue);
            if (!markDirty)
                return;
        }
        event->prop.setFlag();
        event->value = v;
#endif
        (*event)();
    }
    DependencyNode revDependency;

#ifndef UL_PROPERTY_UPDATE_PUSH
    FlagPointer<Property<Type> > prop;
    Type value;
#endif
};

bool qulEmitChangeEvents();

// Wrap a function pointer into a functor
template<typename QtObject, typename T>
struct FunctionWrapper
{
    typedef T (*Function)(QtObject *);
    Function function;

    explicit FunctionWrapper(Function function)
        : function(function)
    {}

    T operator()(QtObject *obj) const { return function(obj); }
};

template<typename QtObject, typename T, int NumDeps>
struct FunctionBinding
{
    typedef Binding<T, FunctionWrapper<QtObject, T>, NumDeps> Type;
};

template<typename T>
struct TypeToNum;
template<int>
struct NumToType;

template<typename T>
TypeToNum<typename T::Type> qul_property_type_helper(const T &);

#define QUL_REGISTER_PROPERTY_TYPE(TYPE, N) \
    template<> \
    struct TypeToNum<TYPE> \
    { \
        char x[N]; \
    }; \
    template<> \
    struct NumToType<N> \
    { \
        typedef TYPE Type; \
    }

QUL_REGISTER_PROPERTY_TYPE(int, 1);
QUL_REGISTER_PROPERTY_TYPE(unsigned int, 2);
QUL_REGISTER_PROPERTY_TYPE(long, 3);
QUL_REGISTER_PROPERTY_TYPE(unsigned long, 4);
QUL_REGISTER_PROPERTY_TYPE(short, 5);
QUL_REGISTER_PROPERTY_TYPE(unsigned short, 6);
QUL_REGISTER_PROPERTY_TYPE(float, 7);
QUL_REGISTER_PROPERTY_TYPE(double, 8);
QUL_REGISTER_PROPERTY_TYPE(class String, 9);
QUL_REGISTER_PROPERTY_TYPE(std::string, 10);

// NOTE: Clang will throw compilation error when building in C++03 mode because of
// using sizeof on non-static member.
// See: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2253.html
#define QUL_PROPERTY_TYPE(PROP) Qul::Private::NumToType<sizeof(Qul::Private::qul_property_type_helper(PROP))>::Type

} // namespace Private
} // namespace Qul
