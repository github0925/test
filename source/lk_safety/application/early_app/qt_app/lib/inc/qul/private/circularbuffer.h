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
#ifndef QUL_CIRCULARBUFFER_H
#define QUL_CIRCULARBUFFER_H

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

#include <cassert>
#include <utility>

#include <stdint.h>

#include <qul/private/compilerdetection.h>

namespace Qul {
namespace Private {

template<typename T, int capacity>
struct CircularBuffer
{
    CircularBuffer()
        : m_begin(-1)
        , m_last(-1)
    {}

    ~CircularBuffer() { clear(); }

    bool isEmpty() const { return m_begin == -1; }

    bool isFull() const { return next(m_last) == m_begin; }

    int size() const
    {
        if (m_begin == -1)
            return 0;
        if (m_begin <= m_last)
            return m_last - m_begin + 1;
        return (capacity - m_begin) + m_last + 1;
    }

    void clear()
    {
        if (m_begin == -1)
            return;

        if (m_begin <= m_last) {
            for (int i = m_begin; i <= m_last; ++i)
                at(i).~T();
        } else {
            for (int i = m_begin; i < capacity; ++i)
                at(i).~T();
            for (int i = 0; i <= m_last; ++i)
                at(i).~T();
        }
        m_begin = -1;
        m_last = -1;
    }

    // overwrites the previous front if full
    void pushBack(const T &event)
    {
        m_last = next(m_last);
        if (m_begin == m_last || m_begin == -1) {
            m_begin = next(m_begin);
        }

        new (m_data[m_last]) T(event);
    }

    T takeFront()
    {
        assert(m_begin != -1 && m_last != -1);

        T &ref = at(m_begin);
        T out = ref;
        ref.~T();

        if (m_begin == m_last) {
            m_begin = -1;
            m_last = -1;
        } else {
            m_begin = next(m_begin);
        }
        return out;
    }

    void popFront()
    {
        assert(m_begin != -1 && m_last != -1);

        at(m_begin).~T();

        if (m_begin == m_last) {
            m_begin = -1;
            m_last = -1;
        } else {
            m_begin = next(m_begin);
        }
    }

    const T &front() const
    {
        assert(m_begin != -1 && m_last != -1);
        return at(m_begin);
    }

private:
    const T &at(int index) const
    {
        assert(index >= 0 && index < capacity);
        return *reinterpret_cast<const T *>(&m_data[index]);
    }

    T &at(int index)
    {
        assert(index >= 0 && index < capacity);
        return *reinterpret_cast<T *>(&m_data[index]);
    }

    static int next(int i) { return (i + 1) % capacity; }

    QUL_DECL_ALIGN(QUL_ALIGNOF(T)) char m_data[capacity][sizeof(T)];
    int m_begin;
    int m_last;
};

} // namespace Private
} // namespace Qul

#endif
