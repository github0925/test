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

namespace Qul {

template<typename T>
struct Singleton : Object
{
    Singleton() {}

    static T &instance()
    {
        static T inst;
        Singleton &assert_crtp = inst; // Compilation error if T does not inherit from Singleton<T>.
        (void) assert_crtp;
        return inst;
    }

private:
    // not copyable
    Singleton(const Singleton &);
    Singleton &operator=(const Singleton &);
};

} // namespace Qul
