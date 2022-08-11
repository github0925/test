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
#include <new>
#include <qul/private/allocator.h>
#include <qul/private/global.h>
#include <platform/mem.h>

namespace Qul {
namespace Private {

class Application;

// RawTimer is a primitive class which implements timers.
// It is a node in the linked list of active timers, and it only knows
// the timeout which is the absolute timestamp at which the timer
// is going to be fired. RawTimer is single shot.
struct RawTimer
{
    friend class Application;
    RawTimer()
        : nextActiveTimer(NULL)
        , timeout(-1)
        , pendingTrigger(false)
        , running(false)
    {}
    RawTimer *nextActiveTimer;
    qint64 timeout;      // Absolute timestamp
    bool pendingTrigger; // true when the timer is about to be triggered
    bool running;
    virtual ~RawTimer() { removeTimer(this); }
    virtual void trigger() = 0;

protected:
    static RawTimer *activeTimerList;
    static void initTimers();
    static void addTimer(RawTimer *timer);
    static void removeTimer(RawTimer *timer);
    static void updateTimers(qint64 timestamp);
    static quint64 firstUpcomingTimeout();

private:
    // Timer is not copyable
    RawTimer(const RawTimer &);
    RawTimer &operator=(const RawTimer &);
};
} // namespace Private

class Timer : private Private::RawTimer
{
    // These members can share the same 8 byte slot as the
    // two trailing bools from RawTimer.
    bool m_repeat;
    int m_interval; // in ms;

    // change to std::function once we have C++11
    void *functor[3]; // This choice makes sizeof(Timer) <= 64 even on 64bit architectures
    struct VTable
    {
        void (*caller)(void **);
        void (*dtor)(void **);
    } const *vtable;

    void trigger();

public:
    explicit Timer(int interval = 0);

    ~Timer();

    void start();
    void start(int msec);
    void stop();

    bool isActive() const;

    void setInterval(int msec);
    int interval() const;

    void setSingleShot(bool singleShot);
    bool isSingleShot() const;

    template<typename FuncArg>
    void onTimeout(FuncArg &&f)
    {
        using Func = typename std::remove_reference<FuncArg>::type;
        const static VTable vt = {caller<Func>, dtor<Func>};
        if (vtable)
            vtable->dtor(functor);
        vtable = NULL;
        if (sizeof(Func) <= sizeof(reinterpret_cast<const Timer *>(0)->functor)) {
            void *addr = functor;
            new (addr) Func(std::forward<FuncArg>(f));
        } else {
            functor[0] = Private::qul_new<Func>(std::forward<FuncArg>(f));
        }
        vtable = &vt;
    }

private:
    // not copyable
    Timer(const Timer &);
    Timer &operator=(const Timer &);

    template<typename Func>
    static void caller(void **data)
    {
        if (sizeof(Func) <= sizeof(reinterpret_cast<const Timer *>(0)->functor)) {
            (*reinterpret_cast<Func *>(data))();
        } else {
            (*reinterpret_cast<Func *>(*data))();
        }
    }
    template<typename Func>
    static void dtor(void **data)
    {
        if (sizeof(Func) <= sizeof(reinterpret_cast<const Timer *>(0)->functor)) {
            reinterpret_cast<Func *>(data)->~Func();
        } else {
            Private::qul_delete(reinterpret_cast<Func *>(*data));
        }
    }
};

} // namespace Qul
