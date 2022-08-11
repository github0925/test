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

namespace Qul {

namespace Private {
namespace Items {
struct ItemBase;
}
} // namespace Private

struct RootItem
{
    // No public API right now
private:
    RootItem() {}
    RootItem(const RootItem &other);
    RootItem &operator=(const RootItem &other);

    friend struct Qul::Private::Items::ItemBase;
};

} // namespace Qul
