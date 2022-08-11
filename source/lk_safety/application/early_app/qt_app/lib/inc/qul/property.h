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
#include <qul/private/flagpointer.h>
#include <qul/private/global.h>
#include <qul/object.h>
#include <type_traits>

namespace Qul {
namespace Private {
template<typename T>
struct BehaviorLight;
template<typename T, typename Functor, int NumDeps>
struct Binding;
template<typename Functor>
struct AnchorsBinding;

namespace Items {
template<typename T>
struct PropertyAnimation;
}

template<typename T>
struct RawPointerTrait
{
    using Value = T *;
};
template<typename T>
struct FlagPointerTrait
{
    using Value = FlagPointer<T>;
};

// A Node in the linked list (uses CRTP)
template<typename Derived, template<class> class PointerTrait = RawPointerTrait>
struct LinkedListNode
{
    using NodePtr = typename PointerTrait<Derived>::Value;

    LinkedListNode() noexcept(noexcept(NodePtr()))
        : next()
        , prev(nullptr)
    {}

    LinkedListNode(const LinkedListNode &) = delete;
    LinkedListNode &operator=(const LinkedListNode &) = delete;

    NodePtr next;
    // prev is a pointer to the "next" pointer in the previous node, or the pointer to the head.
    NodePtr *prev;

    void remove()
    {
        assert(!prev || &*(*prev) == this);
        assert(!next || next->prev == &next);
        if (next)
            next->prev = prev;
        if (prev)
            *prev = &*next;
        prev = nullptr;
        next = nullptr;
    }

    ~LinkedListNode() { remove(); }

    void insertInto(NodePtr *into)
    {
        assert(!next && !prev);
        if (*into) {
            next = *into;
            assert((*into)->prev == into);
            (*into)->prev = &next;
        }
        prev = into;
        *into = static_cast<Derived *>(this);
    }
};

template<typename T>
struct LinkedList
{
    typedef T Node;
    LinkedList() noexcept : head(nullptr) {}
    LinkedList(const LinkedList &) = delete;
    LinkedList &operator=(const LinkedList &) = delete;
    ~LinkedList()
    {
        if (head) {
            assert(head->prev == &head);
            head->prev = nullptr;
        }
    }
    void insert(Node *node) { node->insertInto(&head); }
    void clear()
    {
        while (head) {
            head->remove();
        }
    }
    bool isEmpty() const noexcept { return !head; }
    Node *head;
};

template<typename T, typename UnaryPredicate>
typename LinkedList<T>::Node *find_if(const LinkedList<T> &list, UnaryPredicate &&predicate)
{
    for (auto it = list.head; it; it = it->next) {
        if (predicate(it)) {
            return it;
        }
    }
    return nullptr;
}

struct DependencyNode;
struct ChangeEventBase;

/** A dependency of a property.
 *
 * When a Property is dirty, all dependencies will be flagged for
 * reevaluation. See Property::markDependencyDirty().
 *
 * Can be a DependencyNode or a pointer to a ChangeEventBase for a
 * static dependency.
 *
 * Static dependencies can't be removed, each Property can have only
 * one, must be at the end of the DependencyNode linked list and may
 * be shared between multiple properties. See DirtyEventStaticDependency.
 */
struct DependencyBase
{
    typedef FlagPointer<DependencyBase> NodePtr;

    // Pointer. Target depends on the flag:
    // 0: DependencyNode* (normal linked list)
    // 1: ChangeEventBase* (for DirtyEventStaticDependency)
    // 2, 3: unused
    NodePtr next;

    bool isDependencyNode() const { return !next.flag(); }
    inline DependencyNode *asDependencyNode();

    inline void insertInto(NodePtr *into);

    // convenience functions that do the isDependencyNode() check
    inline void setPrev(NodePtr *p);
    inline void setPrevCheck(NodePtr *p, NodePtr *before);
};

/** A normal part of the Property dependency linked list.
 */
struct DependencyNode : DependencyBase
{
    // prev is a pointer to the "next" pointer in the previous node, or the pointer to the head.
    // NOTE: this means that prev->flags() can either be the head-flags or the
    // DependencyBase::next flags, which have different meanings!
    NodePtr *prev;

    typedef BiPointer<struct PropertyBase, struct ChangeEventBase> Value;
    Value value;

    inline DependencyNode *init(Value v)
    {
        value = v;
        return this;
    }

    DependencyNode()
        : prev(nullptr)
    {}

    ~DependencyNode() { remove(); }

    void remove()
    {
        assert(!prev || &*(*prev) == this);
        assert(!next || !next->isDependencyNode() || next->asDependencyNode()->prev == &next);
        if (next)
            next->setPrev(prev);
        if (prev)
            *prev = &*next;
        prev = nullptr;
        next = nullptr;
    }

    void setPrev(NodePtr *p) { prev = p; }

    void setPrevCheck(NodePtr *p, NodePtr *before)
    {
        (void) before; // only used in assert
        assert(prev == before);
        prev = p;
    }

private:
    DependencyNode(const DependencyNode &);
    DependencyNode &operator=(const DependencyNode &);
};

DependencyNode *DependencyBase::asDependencyNode()
{
    return next.flag() ? nullptr : reinterpret_cast<DependencyNode *>(this);
}

void DependencyBase::setPrev(NodePtr *p)
{
    if (DependencyNode *node = asDependencyNode())
        node->setPrev(p);
}

void DependencyBase::setPrevCheck(NodePtr *p, NodePtr *before)
{
    if (DependencyNode *node = asDependencyNode())
        node->setPrevCheck(p, before);
}

void DependencyBase::insertInto(NodePtr *into)
{
    DependencyNode *node = asDependencyNode();
    if (!node) {
        assert(!*into);
        *into = this;
        return;
    }

    assert(!next && !node->prev);
    if (*into) {
        next = into->data(); // head can have a flag that's unrelated
        (*into)->setPrevCheck(&next, into);
    }
    node->setPrev(into);
    *into = this;
}

// Base class for the bindings.
// As of now, this is inherited by Binding, Behavior, and AnchorsBinding.
// behavior and anchor binding keep a pointer to the original binding, when
// that happens, the order is the following: Behavior -> AnchorsBinding -> Binding.
struct BindingBase
{
    // Instead of having virtual function, BindingBase only has one function
    // pointer member which play the role of a virtual table, in a sense.
    // The reason being that BindingBase is inherited by many template
    // instantiation of the Binding class and that virtual table and several
    // functions is thought to have more overhead than a single function.
    // The InternalCall enum is used to select which of the several function we
    // are calling.
    enum InternalCall {
        // Produce a value now. Used in properties with bindings that are dirty.
        //
        // The void* argument is a T* that the new value shall be written to.
        // Return true if a value was generated, false on error.
        Execute,

        // Override for setting a value to this property. Used for behaviors.
        //
        // The void* argument is a T* with the new value.
        // Return true to skip the normal setValue() behavior.
        SetValue,

        // Same as SetValue, but when setting the value implicitly
        SetImplicitValue,

        // Override for setting a binding to this property. Used for behaviors.
        //
        // The void* argument is a BindingBase* with the new policy.
        // Return true to skip the normal setBindingImpl() behavior.
        // See Install.
        SetBinding,

        // Override for marking the property and its dependencies dirty.
        // Used for Behaviors, which want to Execute the child policies and update
        // the animation target immediately.
        //
        // The void* argument is nullptr.
        // Return true to skip the normal setDirty() behavior.
        MarkDirty,

        // Lets the binding override how it is added to a property.
        // Used in Behaviors, to preserve the existing binding.
        // This hook is run before the SetBinding hook.
        //
        // The void* argument is nullptr.
        // Return true to skip the normal setBindingImpl() behavior.
        Install,

        // Get the normal binding for this property
        // usually returns that's self, unless it is a behavior or a anchor
        // The void* argument is a BindingBase **
        // Return true if such the property was explicitly set.
        GetExplicitBinding
    };

    typedef bool (*MetaCall)(BindingBase *, InternalCall, struct PropertyBase *, void *);

    BindingBase(MetaCall call)
        : call(call)
    {}

    MetaCall call;
    FlagPointer<DependencyBase> firstDependency;
};

struct PropertyBase
{
    PropertyBase() {}

    void registerDependency(DependencyBase *d) { d->insertInto(&firstDependency()); }

    void registerStaticDependency(DependencyBase *d)
    {
        // The (single possible) static dependency must be the first dependency.
        assert(!firstDependency());
        registerDependency(d);
    }

    void registerDynDependency();

    ~PropertyBase()
    {
        for (FlagPointer<DependencyBase> it = firstDependency(); it; it = it->next) {
            DependencyNode *node = it->asDependencyNode();
            if (!node)
                break;
            node->value = static_cast<PropertyBase *>((void *) nullptr);
        }
        if (FlagPointer<DependencyBase> &fp = firstDependency()) {
            fp->setPrev(nullptr);
            fp = nullptr;
        }
    }

    bool wasExplicitlySet() const
    {
        if (BindingBase *b = binding()) {
            return b->call(b, BindingBase::GetExplicitBinding, const_cast<PropertyBase *>(this), &b);
        } else {
            return propertyData.flag();
        }
    }
    bool hasBinding() const { return binding(); }

#ifndef UL_PROPERTY_UPDATE_PUSH
    bool isDirty() const { return binding() && propertyData.flag(); }

    void setDirty()
    {
        assert(binding());
        propertyData.setFlag();
        markDependencyDirty();
    }

#endif
    void markDependencyDirty();

#ifdef QUL_PROPERTY_DEBUG
    // Allow to annotate property with some description to make debugging easier
    explicit PropertyBase(const char *debugName)
        : debugName(debugName)
    {}
    const char *debugName = "";
#endif

private:
#ifndef UL_PROPERTY_UPDATE_PUSH
    bool allDependencyDirty() const
    {
        for (FlagPointer<DependencyBase> it = firstDependency(); it; it = it->next) {
            DependencyNode *node = it->asDependencyNode();
            if (!node)
                break;
            if (!node->value.isT1())
                continue;
            if (!node->value.asT1()->isDirty() || !node->value.asT1()->allDependencyDirty())
                return false;
        }
        return true;
    }
#endif

    PropertyBase(const PropertyBase &);
    PropertyBase &operator=(const PropertyBase &);

protected:
    // Dirty bit in the flag if this is a binding, otherwise the flag specifies
    // whether the property has been explicitly set or not
    BiPointer<DependencyBase, struct BindingBase> propertyData;

    DependencyBase *firstDependency() const
    {
        return propertyData.isNull()
                   ? nullptr
                   : propertyData.isT2() ? propertyData.asT2()->firstDependency.data() : propertyData.asT1();
    }
    FlagPointer<DependencyBase> &firstDependency()
    {
        if (propertyData.isT2() && !propertyData.isNull())
            return propertyData.asT2()->firstDependency;
        propertyData = propertyData.asT1();
        return propertyData;
    }
    BindingBase *binding() const { return propertyData.isT2() ? propertyData.asT2() : nullptr; }

    void setBindingImpl(BindingBase *b)
    {
        BindingBase *oldBinding = binding();
        if (oldBinding == b)
            return;
        // For behavior
        if (b && b->call(b, BindingBase::Install, this, nullptr))
            return;
        if (oldBinding && oldBinding->call(oldBinding, BindingBase::SetBinding, this, b))
            return;
        FlagPointer<DependencyBase> &firstDep = firstDependency();
        assert(!b->firstDependency);
        b->firstDependency = firstDep;
        if (firstDep) {
            firstDep->setPrevCheck(&b->firstDependency, &firstDep);
            firstDep = nullptr;
        }
        propertyData = b;
#ifndef UL_PROPERTY_UPDATE_PUSH
        setDirty();
#else
        if (binding())
            binding()->call(binding(), BindingBase::Execute, this, nullptr);
#endif
    }

    void removeBindingImpl()
    {
        if (BindingBase *oldB = binding()) {
            propertyData = oldB->firstDependency.data();
            if (oldB->firstDependency) {
                oldB->firstDependency->setPrevCheck(&propertyData, &oldB->firstDependency);
                oldB->firstDependency = nullptr;
            }
        }
    }
};

#ifdef QUL_PROPERTY_DEBUG
struct DebugString
{
    const char *value;
};
#define QUL_PROPERTY_DEBUG_INIT(name) \
    { \
        Qul::Private::DebugString { name } \
    }
#else
#define QUL_PROPERTY_DEBUG_INIT(name)
#endif

// As every Property<T> contains a pointer and a T, there may be
// space for some padding integers if sizeof(T) < sizeof(pointer).
template<typename T, typename E = void>
struct PaddedPropertyData
{
    PaddedPropertyData()
        : m_value()
    {}
    explicit PaddedPropertyData(const T &t)
        : m_value(t)
    {}

    T m_value;
};

template<typename T>
struct PaddedPropertyData<T, typename Private::qul_enable_if<sizeof(T) == 2, void>::Type>
{
    PaddedPropertyData()
        : m_value()
    {}
    explicit PaddedPropertyData(const T &t)
        : m_value(t)
    {}

    T m_value;
    Private::quint16 m_u16Padding;
};

template<typename T>
struct PaddedPropertyData<T, typename Private::qul_enable_if<sizeof(T) == 1, void>::Type>
{
    PaddedPropertyData()
        : m_value()
    {}
    explicit PaddedPropertyData(const T &t)
        : m_value(t)
    {}

    T m_value;
    Private::quint8 m_u8Padding;
    Private::quint16 m_u16Padding;
};

template<typename T>
struct ListBase
{
    virtual const T *at(int index) const = 0;
    virtual int count() const = 0;
};

template<typename T, int N>
struct StaticList : ListBase<T>
{
    QUL_STATIC_ASSERT(N > 0);

    explicit StaticList()
    {
        for (int i = 0; i < N; ++i)
            storage[i] = nullptr;
    }

    const T *at(int index) const QUL_DECL_OVERRIDE { return storage[index]; }
    int count() const QUL_DECL_OVERRIDE { return N; }

    T *storage[N];
};

template<typename T>
using InheritsQulObject = typename std::enable_if<std::is_base_of<Qul::Object, T>::value, void>::type;

template<typename T, typename = InheritsQulObject<T> >
struct ListProperty
// FIXME: it's still not finished, make it public ns when it's done
{
    ListProperty()
        : list(nullptr)
    {}

    ListBase<T> *list;
    const T *at(int index) { return list->at(index); }
    int count() const
    {
        if (!list)
            return 0;
        return list->count();
    }
};

struct NoPropertyExtraBase
{};

// Property<T> behavior can be adapted per-type by providing a PropertyTraits<T> specialization.
//
// Used for Property<SharedImage> and Property<FontPointer>. For both of them, the reason is
// that the SharedImage/FontPointer needs to know what properties use it, so it can notify
// them about changes inside itself.
template<typename T>
struct PropertyTraits
{
    // Called whenever a value is assigned to a property.
    static void valueWasSetHook(PropertyBase &, T &) {}
    // Called whenever a value is unassigned from a property
    static void valueWasUnsetHook(PropertyBase &, T &) {}

    // Configure property behavior for finding new values that are identical to old ones.
    //
    // By default, operator==(T, T) is used for the comparison, and the property is only
    // marked dirty and assigns the new value, if the new value is not == to the old.
    //
    // If \a markDirty is true after the call, the property dependencies will be marked as
    // dirty.
    // If \a assignValue is true after the call, the new value will be assigned to the
    // property's storage and the value hooks will be triggered.
    //
    // Valid reasons for overriding this function are:
    // - There's no operator== on the type, and there shouldn't be.
    // - T is a pointer type and situations like this happen:
    //      Property<int *> p;
    //      int x = 4;
    //      p.setValue(&x);
    //      x = 7;
    //      p.setValue(&x); // maybe want to mark dependencies as dirty
    static void valueEquality(const T &lhs, const T &rhs, bool *markDirty, bool *assignValue)
    {
        *markDirty = !(lhs == rhs);
        *assignValue = *markDirty;
    }

    // Extra data to be stored in each Property<T> instance. See use with FontPointer.
    using ExtraBase = NoPropertyExtraBase;

    // When false (default), valueEquality isn't checked when a dirty binding
    // provides a new value. The value is always assumed to be new, and the set/unset
    // hooks are always called.
    // If the hooks are somewhat expensive and T equality checking and assignment is cheap,
    // set this to true to compare the old and new value before deciding whether the hooks
    // should be called or not.
    static constexpr bool checkValueEqualityOnDirtyBinding = false;
};

} // namespace Private

template<typename T>
struct QUL_DECL_EMPTY_BASES Property : Private::PropertyBase,
                                       Private::PaddedPropertyData<T>,
                                       Private::PropertyTraits<T>::ExtraBase
{
    typedef T Type;

    Property() { Traits::valueWasSetHook(*this, this->m_value); }

    explicit Property(const T &value)
        : Private::PaddedPropertyData<T>(value)
    {
        Traits::valueWasSetHook(*this, this->m_value);
    }

    template<typename Functor, int NumDeps>
    explicit Property(Private::Binding<T, Functor, NumDeps> *binding)
    {
        setBindingImpl(binding);
    }

#ifdef QUL_PROPERTY_DEBUG
    // Allow to annotate property with some description to make debugging easier
    explicit Property(Private::DebugString debugName)
        : PropertyBase(debugName.value)
    {
        Traits::valueWasSetHook(*this, this->m_value);
    }
#endif

    template<typename Binding>
    void setBinding(Binding *b)
    {
        Private::qul_static_assert<Private::qul_is_same<typename Binding::PropertyType, T>::value>();
        setBindingImpl(b);
    }

    void setValue(const T &v)
    {
        if (Private::BindingBase *oldB = binding()) {
            if (oldB->call(oldB, Private::BindingBase::SetValue, this, const_cast<T *>(&v)))
                return;
            removeBindingImpl();
        }

        assert(!binding());
        // when binding is null, the flag specifies whether
        // the property has been explicitly set or not
        propertyData.setFlag();

        setValueAndBypassBinding(v);
    }

    void setValueAndBypassBinding(const T &v)
    {
#ifndef UL_PROPERTY_UPDATE_PUSH
        if (binding())
            propertyData.clearFlag();
#endif
        bool markDirty = false;
        bool assignValue = false;
        Traits::valueEquality(this->m_value, v, &markDirty, &assignValue);
        if (assignValue) {
            Traits::valueWasUnsetHook(*this, this->m_value);
            this->m_value = v;
            Traits::valueWasSetHook(*this, this->m_value);
        }
        if (markDirty)
            markDependencyDirty();
    }

    void setImplicitly(const T &v)
    {
        if (!wasExplicitlySet()) {
            if (Private::BindingBase *b = binding()) {
                if (b->call(b, Private::BindingBase::SetImplicitValue, this, const_cast<T *>(&v)))
                    return;
            }
            setValueAndBypassBinding(v);
        }
    }

    const T &value()
    {
#ifndef UL_PROPERTY_UPDATE_PUSH
        if (isDirty() && binding()) {
            if (Traits::checkValueEqualityOnDirtyBinding) {
                T newValue;
                if (binding()->call(binding(), Private::BindingBase::Execute, this, &newValue)) {
                    bool markDirty = false;
                    bool assignValue = false;
                    Traits::valueEquality(this->m_value, newValue, &markDirty, &assignValue);
                    if (assignValue) {
                        Traits::valueWasUnsetHook(*this, this->m_value);
                        this->m_value = std::move(newValue);
                        Traits::valueWasSetHook(*this, this->m_value);
                    }
                    propertyData.clearFlag();
                }
            } else {
                Traits::valueWasUnsetHook(*this, this->m_value);
                if (binding()->call(binding(), Private::BindingBase::Execute, this, &this->m_value)) {
                    Traits::valueWasSetHook(*this, this->m_value);
                    propertyData.clearFlag();
                }
            }
        }
#endif
        registerDynDependency();
        return this->m_value;
    }
    const T &value() const { return const_cast<Property *>(this)->value(); }

private:
    // not copyable
    Property(const Property &);
    Property &operator=(const Property &);

    using Traits = Private::PropertyTraits<T>;

    template<typename X>
    friend struct Private::Items::PropertyAnimation;
    template<typename X>
    friend struct Private::BehaviorLight;
    template<typename F>
    friend struct Private::AnchorsBinding;
};

} // namespace Qul
