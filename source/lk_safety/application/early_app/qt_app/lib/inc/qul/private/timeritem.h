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
#include <qul/private/propertybinding.h>
#include <qul/timer.h>
#include <qul/object.h>
#include <qul/signal.h>

namespace Qul {
namespace Private {

class Application;

namespace Items {

struct Timer : public Qul::Object, private Qul::Private::RawTimer
{
    Timer();

    Property<int> interval;
    Property<bool> running;
    Property<bool> repeat;

    Signal<void()> triggered;

    inline void start() { running.setValue(true); }
    inline void stop() { running.setValue(false); }
    void restart();

private:
    friend class Qul::Private::Application;

    void runningChanged();

    friend int count(Timer *next);

    struct RunningChanged
    {
        RunningChanged(Timer *timer)
            : timer(timer)
        {}

        Timer *timer;
        inline void operator()() { timer->runningChanged(); }
    };
    friend struct RunningChanged;

    void trigger();

    ChangedEvent<RunningChanged, bool> onRunningChangedChanged;
};

} // namespace Items
} // namespace Private
} // namespace Qul
