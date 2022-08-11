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
#include <qul/signal.h>
#include <qul/object.h>

namespace Qul {

template<typename T>
struct ListModel : Object
{
    ListModel() {}
    virtual ~ListModel() {}

    virtual int count() const = 0;
    virtual T data(int index) const = 0;
    T get(int index) const { return data(index); }

    Signal<void()> modelReset;
    Signal<void(int index)> dataChanged;

    static const int StaticCount = -1;
    typedef T DataType;

private:
    // not copyable
    ListModel(const ListModel &);
    ListModel &operator=(const ListModel &);
};

} // namespace Qul
