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

#include <qul/private/compilerdetection.h>

namespace Qul {
namespace Private {

struct nullopt_t
{};

// This is a quickly done replacement to std::optional
template<typename T>
struct optional
{
    optional()
        : m_valid(false)
    {}
    optional(nullopt_t)
        : m_valid(false)
    {}
    optional(T value)
        : m_valid(true)
    {
        new (&m_value) T(value);
    }
    ~optional()
    {
        if (m_valid)
            destroy();
    }
    operator bool() const { return m_valid; }
    const T &operator*() const { return *reinterpret_cast<const T *>(&m_value); }
    T &operator*() { return *reinterpret_cast<T *>(&m_value); }
    T value_or(T v) { return m_valid ? **this : v; }

    template<typename... Args>
    T &emplace(Args &&... args)
    {
        if (m_valid)
            destroy();
        new (&m_value) T(std::forward<Args>(args)...);
        m_valid = true;
        return **this;
    }

private:
    inline void destroy() { reinterpret_cast<T *>(&m_value)->~T(); }

    QUL_DECL_ALIGN(QUL_ALIGNOF(T)) char m_value[sizeof(T)];
    bool m_valid;
};

static const nullopt_t nullopt = {};

} // namespace Private
} // namespace Qul
