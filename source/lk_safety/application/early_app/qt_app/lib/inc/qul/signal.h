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

#include <qul/property.h>

namespace Qul {
namespace Private {
template<typename T>
struct SlotBase;
template<>
struct SlotBase<void()> : LinkedListNode<SlotBase<void()> >
{
    void (*call)(SlotBase *);
};
template<typename T1>
struct SlotBase<void(T1)> : LinkedListNode<SlotBase<void(T1)> >
{
    void (*call)(SlotBase *, T1);
};
template<typename T1, typename T2>
struct SlotBase<void(T1, T2)> : LinkedListNode<SlotBase<void(T1, T2)> >
{
    void (*call)(SlotBase *, T1, T2);
};
template<typename T1, typename T2, typename T3>
struct SlotBase<void(T1, T2, T3)> : LinkedListNode<SlotBase<void(T1, T2, T3)> >
{
    void (*call)(SlotBase *, T1, T2, T3);
};
template<typename T1, typename T2, typename T3, typename T4>
struct SlotBase<void(T1, T2, T3, T4)> : LinkedListNode<SlotBase<void(T1, T2, T3, T4)> >
{
    void (*call)(SlotBase *, T1, T2, T3, T4);
};
template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct SlotBase<void(T1, T2, T3, T4, T5)> : LinkedListNode<SlotBase<void(T1, T2, T3, T4, T5)> >
{
    void (*call)(SlotBase *, T1, T2, T3, T4, T5);
};
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct SlotBase<void(T1, T2, T3, T4, T5, T6)> : LinkedListNode<SlotBase<void(T1, T2, T3, T4, T5, T6)> >
{
    void (*call)(SlotBase *, T1, T2, T3, T4, T5, T6);
};
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct SlotBase<void(T1, T2, T3, T4, T5, T6, T7)> : LinkedListNode<SlotBase<void(T1, T2, T3, T4, T5, T6, T7)> >
{
    void (*call)(SlotBase *, T1, T2, T3, T4, T5, T6, T7);
};
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct SlotBase<void(T1, T2, T3, T4, T5, T6, T7, T8)> : LinkedListNode<SlotBase<void(T1, T2, T3, T4, T5, T6, T7, T8)> >
{
    void (*call)(SlotBase *, T1, T2, T3, T4, T5, T6, T7, T8);
};
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
struct SlotBase<void(T1, T2, T3, T4, T5, T6, T7, T8, T9)>
    : LinkedListNode<SlotBase<void(T1, T2, T3, T4, T5, T6, T7, T8, T9)> >
{
    void (*call)(SlotBase *, T1, T2, T3, T4, T5, T6, T7, T8, T9);
};
} // namespace Private

template<typename T>
struct Signal;
template<>
struct Signal<void()>
{
    typedef Private::LinkedList<Private::SlotBase<void()> > List;

    Signal() {}

    void operator()() const
    {
        for (List::Node *it = firstSlot.head; it; it = it->next) {
            it->call(it);
        }
    }

    void connect(List::Node *d) { firstSlot.insert(d); }

    bool isConnected() const { return !firstSlot.isEmpty(); }

private:
    Signal(const Signal &);
    Signal &operator=(const Signal &);

    List firstSlot;
};
template<typename T1>
struct Signal<void(T1)>
{
    typedef typename Private::LinkedList<Private::SlotBase<void(T1)> > List;

    Signal() {}

    void operator()(T1 arg1) const
    {
        for (typename List::Node *it = firstSlot.head; it; it = it->next) {
            it->call(it, arg1);
        }
    }

    void connect(typename List::Node *d) { firstSlot.insert(d); }

    bool isConnected() const { return !firstSlot.isEmpty(); }

private:
    Signal(const Signal &);
    Signal &operator=(const Signal &);

    List firstSlot;
};
template<typename T1, typename T2>
struct Signal<void(T1, T2)>
{
    typedef typename Private::LinkedList<Private::SlotBase<void(T1, T2)> > List;

    Signal() {}

    void operator()(T1 arg1, T2 arg2) const
    {
        for (typename List::Node *it = firstSlot.head; it; it = it->next) {
            it->call(it, arg1, arg2);
        }
    }

    void connect(typename List::Node *d) { firstSlot.insert(d); }

    bool isConnected() const { return !firstSlot.isEmpty(); }

private:
    Signal(const Signal &);
    Signal &operator=(const Signal &);

    List firstSlot;
};
template<typename T1, typename T2, typename T3>
struct Signal<void(T1, T2, T3)>
{
    typedef typename Private::LinkedList<Private::SlotBase<void(T1, T2, T3)> > List;

    Signal() {}

    void operator()(T1 arg1, T2 arg2, T3 arg3) const
    {
        for (typename List::Node *it = firstSlot.head; it; it = it->next) {
            it->call(it, arg1, arg2, arg3);
        }
    }

    void connect(typename List::Node *d) { firstSlot.insert(d); }

    bool isConnected() const { return !firstSlot.isEmpty(); }

private:
    Signal(const Signal &);
    Signal &operator=(const Signal &);

    List firstSlot;
};
template<typename T1, typename T2, typename T3, typename T4>
struct Signal<void(T1, T2, T3, T4)>
{
    typedef typename Private::LinkedList<Private::SlotBase<void(T1, T2, T3, T4)> > List;

    Signal() {}

    void operator()(T1 arg1, T2 arg2, T3 arg3, T4 arg4) const
    {
        for (typename List::Node *it = firstSlot.head; it; it = it->next) {
            it->call(it, arg1, arg2, arg3, arg4);
        }
    }

    void connect(typename List::Node *d) { firstSlot.insert(d); }

    bool isConnected() const { return !firstSlot.isEmpty(); }

private:
    Signal(const Signal &);
    Signal &operator=(const Signal &);

    List firstSlot;
};
template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct Signal<void(T1, T2, T3, T4, T5)>
{
    typedef typename Private::LinkedList<Private::SlotBase<void(T1, T2, T3, T4, T5)> > List;

    Signal() {}

    void operator()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5) const
    {
        for (typename List::Node *it = firstSlot.head; it; it = it->next) {
            it->call(it, arg1, arg2, arg3, arg4, arg5);
        }
    }

    void connect(typename List::Node *d) { firstSlot.insert(d); }

    bool isConnected() const { return !firstSlot.isEmpty(); }

private:
    Signal(const Signal &);
    Signal &operator=(const Signal &);

    List firstSlot;
};
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct Signal<void(T1, T2, T3, T4, T5, T6)>
{
    typedef typename Private::LinkedList<Private::SlotBase<void(T1, T2, T3, T4, T5, T6)> > List;

    Signal() {}

    void operator()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6) const
    {
        for (typename List::Node *it = firstSlot.head; it; it = it->next) {
            it->call(it, arg1, arg2, arg3, arg4, arg5, arg6);
        }
    }

    void connect(typename List::Node *d) { firstSlot.insert(d); }

    bool isConnected() const { return !firstSlot.isEmpty(); }

private:
    Signal(const Signal &);
    Signal &operator=(const Signal &);

    List firstSlot;
};
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct Signal<void(T1, T2, T3, T4, T5, T6, T7)>
{
    typedef typename Private::LinkedList<Private::SlotBase<void(T1, T2, T3, T4, T5, T6, T7)> > List;

    Signal() {}

    void operator()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7) const
    {
        for (typename List::Node *it = firstSlot.head; it; it = it->next) {
            it->call(it, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        }
    }

    void connect(typename List::Node *d) { firstSlot.insert(d); }

    bool isConnected() const { return !firstSlot.isEmpty(); }

private:
    Signal(const Signal &);
    Signal &operator=(const Signal &);

    List firstSlot;
};
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct Signal<void(T1, T2, T3, T4, T5, T6, T7, T8)>
{
    typedef typename Private::LinkedList<Private::SlotBase<void(T1, T2, T3, T4, T5, T6, T7, T8)> > List;

    Signal() {}

    void operator()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8) const
    {
        for (typename List::Node *it = firstSlot.head; it; it = it->next) {
            it->call(it, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        }
    }

    void connect(typename List::Node *d) { firstSlot.insert(d); }

    bool isConnected() const { return !firstSlot.isEmpty(); }

private:
    Signal(const Signal &);
    Signal &operator=(const Signal &);

    List firstSlot;
};
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
struct Signal<void(T1, T2, T3, T4, T5, T6, T7, T8, T9)>
{
    typedef typename Private::LinkedList<Private::SlotBase<void(T1, T2, T3, T4, T5, T6, T7, T8, T9)> > List;

    Signal() {}

    void operator()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9) const
    {
        for (typename List::Node *it = firstSlot.head; it; it = it->next) {
            it->call(it, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        }
    }

    void connect(typename List::Node *d) { firstSlot.insert(d); }

    bool isConnected() const { return !firstSlot.isEmpty(); }

private:
    Signal(const Signal &);
    Signal &operator=(const Signal &);

    List firstSlot;
};
} // namespace Qul
