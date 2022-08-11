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

#include <platform/mem.h>

#include <vector>

namespace Qul {
namespace Private {

template<class T>
struct Allocator
{
    using value_type = T;

    T *allocate(std::size_t n) { return reinterpret_cast<T *>(Platform::qul_malloc(n * sizeof(T))); }

    void deallocate(T *p, std::size_t) noexcept { Platform::qul_free(p); }

    template<class U>
    bool operator==(const Allocator<U> &) const noexcept
    {
        return true;
    }

    template<class U>
    bool operator!=(const Allocator<U> &) const noexcept
    {
        return false;
    }

    Allocator() = default;

    Allocator(const Allocator &) = default;

    template<class U>
    Allocator(const Allocator<U> &)
    {}
};

template<typename T>
using Vector = std::vector<T, Allocator<T> >;

template<typename T, typename... Args>
static T *qul_new(Args &&... args)
{
    auto *t = Platform::qul_malloc(sizeof(T));
    if (!t)
        return nullptr;
    return new (t) T(std::forward<Args>(args)...);
}

template<typename T>
static void qul_delete(T *t)
{
    if (!t)
        return;
    t->~T();
    Platform::qul_free(t);
}

} // namespace Private
} // namespace Qul
