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
#include <qul/model.h>

#include <qul/private/allocator.h>
#include <qul/private/items.h>
#include <qul/private/array.h>
#include <qul/private/slot.h>
#include <qul/private/scopedvaluerollback.h>

#include <vector>

namespace Qul {
namespace Private {

extern bool allBehaviorDisabled; // in animation.cpp

// FIXME: this is only necessary because the code generator uses "value()" but it shouldn't.
template<typename T>
struct FakeProperty
{
    T data;
    inline FakeProperty()
        : data()
    {}

    inline FakeProperty(T data)
        : data(data){};
    inline operator T &() { return data; }
    inline operator const T &() const { return data; }
    inline T value() const { return data; }
    inline void setValue(const T &v) { data = v; }
    inline bool operator==(const FakeProperty<T> &other) const { return data == other.data; }
};

template<typename T>
struct ModelResetEvent
{
    explicit ModelResetEvent(ListModel<T> *m)
        : m(m)
    {}
    ListModel<T> *m;
    void operator()() { m->modelReset(); }
};

// Wrapper for `modelData` for models without roles (for example: `model: 1`)
template<typename T>
struct RolelessModelData
{
    RolelessModelData() {}
    // To make creating custom models easier don't mark it as explicit
    RolelessModelData(const T &data)
        : modelData(data)
    {}

    // The type has to be copyable due to Qul::Property requirements
    RolelessModelData(const RolelessModelData &data) { *this = data; }
    RolelessModelData &operator=(const RolelessModelData &wrapper)
    {
        modelData.setValue(wrapper.modelData.value());
        return *this;
    }

    Qul::Private::FakeProperty<T> modelData;

    inline const T &value() const { return modelData.value(); }
    inline void setValue(const T &v) { return modelData.setValue(v); }
    inline bool operator==(const RolelessModelData<T> &other) const { return modelData == other.modelData; }
};

/** Model type used for ListModels in codegen
 *
 * Note that "static" here means that it points to some data
 * that is allocated elsewhere. This data itself could change at
 * runtime though.
 *
 * The length is constant though.
 */
template<typename T, int N>
struct StaticSimpleModel : ListModel<RolelessModelData<T> >
{
    explicit StaticSimpleModel(const T *data)
        : m_data(data)
    {}
    const T *m_data;
    int count() const QUL_DECL_OVERRIDE { return N; }
    RolelessModelData<T> data(int idx) const QUL_DECL_OVERRIDE { return RolelessModelData<T>(m_data[idx]); }
    static const int StaticCount = N;
};
template<typename T, int N>
struct StaticModel : ListModel<T>
{
    explicit StaticModel(const T *data)
        : m_data(data)
    {}
    const T *m_data;
    int count() const QUL_DECL_OVERRIDE { return N; }
    T data(int idx) const QUL_DECL_OVERRIDE { return m_data[idx]; }
    static const int StaticCount = N;
};

// Model with just a count
struct IntegerModel : ListModel<RolelessModelData<int> >
{
    Property<int> value;
    int count() const QUL_DECL_OVERRIDE { return value.value(); }
    RolelessModelData<int> data(int i) const QUL_DECL_OVERRIDE { return RolelessModelData<int>(i); }

    ChangedEvent<ModelResetEvent<RolelessModelData<int> >, int> onValueChanged;
    IntegerModel()
        : onValueChanged(ModelResetEvent<RolelessModelData<int> >(this), &value)
    {}
};

// Model with just a count, known at compile time
template<int N>
struct ConstIntegerModel : ListModel<RolelessModelData<int> >
{
    int count() const QUL_DECL_OVERRIDE { return N; }
    RolelessModelData<int> data(int i) const QUL_DECL_OVERRIDE { return RolelessModelData<int>(i); }
    static const int StaticCount = N;
};

// A transformed view of another model
//
// Note: Does not forward source model modelReset/dataChanged
template<typename From, typename To>
struct ProxyModel : ListModel<To>
{
    Property<ListModel<From> *> source;
    virtual To convertor(const From &) const = 0;
    int count() const QUL_DECL_OVERRIDE
    {
        ListModel<From> *s = source.value();
        return s ? s->count() : 0;
    }
    To data(int i) const QUL_DECL_OVERRIDE { return convertor(source.value()->data(i)); }

    ChangedEvent<ModelResetEvent<To>, ListModel<From> *> onSourceChanged;
    ProxyModel()
        : onSourceChanged(ModelResetEvent<To>(this), &source)
    {}
};

template<typename Delegate, typename Type, Property<Type> Delegate::*Property>
struct SetOptionalPropertyHelper
{};

template<typename Delegate>
SetOptionalPropertyHelper<Delegate, int, &Delegate::attached_index> *setOptionalIndexHelper(Delegate *t, int index)
{
    t->attached_index.setValue(index);
    return NULL;
}
inline void setOptionalIndexHelper(Object *, int) {}

template<typename Delegate, typename Model>
SetOptionalPropertyHelper<Delegate, Model, &Delegate::attached_model> *setOptionalModelHelper(Delegate *t, Model m)
{
    t->attached_model.setValue(m);
    return NULL;
}
template<typename Delegate, typename Model>
SetOptionalPropertyHelper<Delegate, Qul::Private::RolelessModelData<Model>, &Delegate::attached_model>
    *setOptionalModelHelper(Delegate *t, Model m)
{
    t->attached_model.setValue(m);
    return NULL;
}
template<typename Model>
inline void setOptionalModelHelper(Object *, const Model &)
{}

namespace Items {

template<typename Delegate, int StaticCount>
struct RepeaterDelegateContainer
{
    template<typename ModelType, typename ParentScope>
    void refreshModel(Items::ItemBase *parent,
                      Items::ItemBase *delegatePlaceholder,
                      ParentScope *scope,
                      ModelType *model)
    {
        ScopedValueRollback<bool> sc(allBehaviorDisabled, true);
        if (delegates.isEmpty()) {
            for (int i = 0; i < StaticCount; ++i)
                appendChild(parent, &delegates.emplace(scope), delegatePlaceholder);
        }
        for (int i = 0; i < StaticCount; ++i) {
            Delegate &x = delegates[StaticCount - 1 - i];
            setOptionalModelHelper(&x, model->get(i));
            setOptionalIndexHelper(&x, i);
        }
    }
    void clear() { delegates.clear(); }
    template<typename ModelType>
    void refreshItem(int i, ModelType *model)
    {
        if (i < 0 || i >= StaticCount)
            return;
        setOptionalModelHelper(&delegates[StaticCount - 1 - i], model->get(i));
    }

    Delegate *operator[](const int index)
    {
        if (index < 0 || index >= StaticCount) {
            return nullptr;
        }
        return &delegates[StaticCount - index - 1];
    }

    // Stored in reverse order because of how they are appended.
    VarLengthArray<Delegate, StaticCount> delegates;
};

template<typename Delegate>
struct RepeaterDelegateContainer<Delegate, -1>
{
    // FIXME! better data structure
private:
    Vector<Delegate *> delegates;
    RepeaterDelegateContainer(const RepeaterDelegateContainer &); // = delete
public:
    RepeaterDelegateContainer() {}
    ~RepeaterDelegateContainer() { clear(); }

    template<typename ModelType, typename ParentScope>
    void refreshModel(Items::ItemBase *parent,
                      Items::ItemBase *delegatePlaceholder,
                      ParentScope *parentScope,
                      ModelType *model)
    {
        ScopedValueRollback<bool> sc(allBehaviorDisabled, true);
        clear();
        delegates.resize(model->count());

        //FIXME! bug if the model is reset by the delegate's binding
        for (int i = delegates.size() - 1; i >= 0; --i) {
            Delegate *x = qul_new<Delegate>(parentScope);
            appendChild(parent, x, delegatePlaceholder);
            setOptionalModelHelper(x, model->get(i));
            setOptionalIndexHelper(x, i);
            delegates[i] = x;
        }
    }
    void clear()
    {
        for (size_t i = 0; i < delegates.size(); ++i)
            qul_delete(delegates[i]);
        delegates.clear();
    }
    template<typename ModelType>
    void refreshItem(size_t i, ModelType *model)
    {
        if (i >= delegates.size() || !delegates[i])
            return;
        setOptionalModelHelper(delegates[i], model->get(i));
    }

    Delegate *operator[](const int index)
    {
        if (index < 0 || index >= delegates.size()) {
            return nullptr;
        }
        return delegates[index];
    }
};

template<typename ModelType, typename Delegate, typename ParentScope>
struct Repeater : Qul::Object
{
    explicit Repeater(ParentScope *scope)
        : parentScope(scope)
        , onModelChanged(ModelResetEvent(this), &model)
        , onModelReset(this, refresh)
        , onDataChanged(this, dataChanged)
        , parent(NULL)
    {
        delegatePlaceholder.visible.setValue(false);
        delegatePlaceholder.setItemType(ItemBase::ItemType::Bookkeeping);
    }

    Property<ModelType *> model;
    ParentScope *parentScope;

    RepeaterDelegateContainer<Delegate, ModelType::StaticCount> delegates;

    struct ModelResetEvent
    {
        ModelResetEvent(Repeater *r)
            : r(r)
        {}
        Repeater *r;
        void operator()() { r->refreshModel(); }
    };

    static void refresh(Repeater *r) { r->refreshModel(); };
    static void dataChanged(Repeater *r, int i) { r->delegates.refreshItem(i, r->model.value()); };

    ChangedEvent<ModelResetEvent, ModelType *> onModelChanged;
    Slot<Repeater, void()> onModelReset;
    Slot<Repeater, void(int)> onDataChanged;

    friend void appendChild(Items::ItemBase *parent, Repeater *repeater, ItemBase *after = NULL)
    {
        repeater->parent = parent;
        appendChild(parent, &repeater->delegatePlaceholder, after);
        repeater->refreshModel();
    }

    Delegate *itemAt(const int index) { return delegates[index]; }

private:
    Items::ItemBase *parent;

    // The repeater items are inserted after this placeholder item
    // in the parent's child list.
    Items::Item delegatePlaceholder;

    void refreshModel()
    {
        ModelType *m = model.value();
        if (!m || !parent) {
            delegates.clear();
            return;
        }
        onModelReset.connect(&m->modelReset);
        onDataChanged.connect(&m->dataChanged);
        delegates.refreshModel(parent, &delegatePlaceholder, parentScope, m);
    }
};

// FIXME! When following line is uncommented the ListModel looses its propertes from ../model.h
//struct ListModel : Qul::Object{};
struct ListElement : Qul::Object
{};

} // namespace Items

} // namespace Private
} // namespace Qul
