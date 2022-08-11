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

#include <memory>

#include <platform/mem.h>

#include <qul/private/allocator.h>

namespace Qul {
namespace Private {

template<typename T>
struct ScopedPointerDeleter
{
    static inline void cleanup(T *pointer)
    {
        // Enforce a complete type.
        typedef char IsIncompleteType[sizeof(T) ? 1 : -1];
        (void) sizeof(IsIncompleteType);
        qul_delete(pointer);
    }
};

template<typename T, typename Cleanup = ScopedPointerDeleter<T> >
class ScopedPointer
{
public:
    typedef void (*Deleter)(T *);

    ScopedPointer(T *ptr = NULL)
        : m_ptr(ptr)
    {}

    ~ScopedPointer() { clean(); }

    void reset(T *ptr = NULL)
    {
        if (ptr == m_ptr)
            return;
        clean();
        m_ptr = ptr;
    }

    T *get() { return m_ptr; }

    T *get() const { return m_ptr; }

    T &operator*() const { return *get(); }

    T *operator->() const { return get(); }

    operator bool() const { return get() != NULL; }

private:
    ScopedPointer<T> &operator=(const ScopedPointer<T> &other);
    ScopedPointer(const ScopedPointer<T> &other);

    void clean() { Cleanup::cleanup(m_ptr); }

    T *m_ptr;
};

} // namespace Private
} // namespace Qul
