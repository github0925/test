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
#ifndef QUL_ARRAY_H
#define QUL_ARRAY_H

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

#include <qul/private/global.h>
#include <qul/private/compilerdetection.h>
#include <new>

namespace Qul {
namespace Private {

template<typename T, int N>
class VarLengthArray
{
public:
    typedef T value_type;

    inline VarLengthArray()
        : m_count(0)
    {}
    ~VarLengthArray() { clear(); }

    VarLengthArray(int count)
        : m_count(0)
    {
        assert(count <= N);
        for (int i = 0; i < count; ++i)
            append(T());
    }

    T *begin() { return reinterpret_cast<T *>(m_data); }

    T *end() { return reinterpret_cast<T *>(&m_data[m_count]); }

    inline bool isEmpty() const { return m_count == 0; }
    inline bool isFull() const { return m_count == N; }

    void clear()
    {
        while (m_count) {
            m_count--;
            reinterpret_cast<const T *>(&m_data[m_count])->~T();
        }
    }

    void removeAt(int index)
    {
        assert(!isEmpty());
        assert(index >= 0 && index < m_count);
        reinterpret_cast<T *>(&m_data[index])->~T();
        for (int i = index; i < (m_count - 1); ++i) {
            new (&m_data[i]) T(at(i + 1));
            reinterpret_cast<const T *>(&m_data[i + 1])->~T();
        }
        --m_count;
    }

    bool remove(const T &value)
    {
        for (int i = m_count - 1; i >= 0; --i) {
            if (value == at(i)) {
                removeAt(i);
                return true;
            }
        }
        return false;
    }

    void resize(int size)
    {
        assert(size <= N);
        for (int i = size; i < m_count; ++i) {
            reinterpret_cast<const T *>(&m_data[i])->~T();
        }
        for (int i = m_count; i < size; ++i) {
            new (&m_data[i]) T();
        }
        m_count = size;
    }

    inline T &append(const T &value)
    {
        assert(!isFull());
        return *new (&m_data[m_count++]) T(value);
    }

    template<typename Arg>
    inline T &emplace(const Arg &arg)
    {
        assert(!isFull());
        return *new (&m_data[m_count++]) T(arg);
    }

    inline int size() const { return m_count; }

    inline int capacity() const { return N; }

    T &at(int index)
    {
        assert(index >= 0 && index < m_count);
        return *const_cast<T *>(reinterpret_cast<const T *>(&m_data[index][0]));
    }
    const T &at(int index) const
    {
        assert(index >= 0 && index < m_count);
        return *reinterpret_cast<const T *>(&m_data[index][0]);
    }

    T &operator[](int index) { return at(index); }
    const T &operator[](int index) const { return at(index); }

    T *data() { return const_cast<T *>(reinterpret_cast<const T *>(m_data)); }

private:
    int m_count;
    QUL_DECL_ALIGN(QUL_ALIGNOF(T)) char m_data[N][sizeof(T)];
};

template<typename T, int N>
class FixedLengthArray
{
public:
    inline int size() const { return N; }

    T *begin() { return m_data; }

    T *end() { return &m_data[N]; }

    T &at(int index)
    {
        assert(index >= 0 && index < N);
        return m_data[index];
    }
    const T &at(int index) const
    {
        assert(index >= 0 && index < N);
        return m_data[index];
    }

    T &operator[](int index) { return at(index); }
    const T &operator[](int index) const { return at(index); }

    T *data() { return m_data; }

private:
    T m_data[N];
};

template<typename T>
class FixedLengthArray<T, 0>
{
public:
    inline int size() const { return 0; }

    T *begin() { return nullptr; }

    T *end() { return nullptr; }

    T &at(int index)
    {
        QUL_UNUSED(index);
        assert(false);
        return *data();
    }

    const T &at(int index) const
    {
        QUL_UNUSED(index);
        assert(false);
        return *data();
    }

    T &operator[](int index) { return at(index); }
    const T &operator[](int index) const { return at(index); }

    T *data() const { return 0; }
};

template<typename T>
FixedLengthArray<T, 1> fixedLengthArray(T obj1)
{
    T data[] = {obj1};
    return *reinterpret_cast<const FixedLengthArray<T, 1> *>(&data);
}

template<typename T>
FixedLengthArray<T, 2> fixedLengthArray(T obj1, T obj2)
{
    T data[] = {obj1, obj2};
    return *reinterpret_cast<const FixedLengthArray<T, 2> *>(&data);
}

template<typename T>
FixedLengthArray<T, 3> fixedLengthArray(T obj1, T obj2, T obj3)
{
    T data[] = {obj1, obj2, obj3};
    return *reinterpret_cast<const FixedLengthArray<T, 3> *>(&data);
}

template<typename T>
FixedLengthArray<T, 4> fixedLengthArray(T obj1, T obj2, T obj3, T obj4)
{
    T data[] = {obj1, obj2, obj3, obj4};
    return *reinterpret_cast<const FixedLengthArray<T, 4> *>(&data);
}

template<typename T>
FixedLengthArray<T, 5> fixedLengthArray(T obj1, T obj2, T obj3, T obj4, T obj5)
{
    T data[] = {obj1, obj2, obj3, obj4, obj5};
    return *reinterpret_cast<const FixedLengthArray<T, 5> *>(&data);
}

template<typename T>
FixedLengthArray<T, 6> fixedLengthArray(T obj1, T obj2, T obj3, T obj4, T obj5, T obj6)
{
    T data[] = {obj1, obj2, obj3, obj4, obj5, obj6};
    return *reinterpret_cast<const FixedLengthArray<T, 6> *>(&data);
}

template<typename T>
FixedLengthArray<T, 7> fixedLengthArray(T obj1, T obj2, T obj3, T obj4, T obj5, T obj6, T obj7)
{
    T data[] = {obj1, obj2, obj3, obj4, obj5, obj6, obj7};
    return *reinterpret_cast<const FixedLengthArray<T, 7> *>(&data);
}

template<typename T>
FixedLengthArray<T, 8> fixedLengthArray(T obj1, T obj2, T obj3, T obj4, T obj5, T obj6, T obj7, T obj8)
{
    T data[] = {obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8};
    return *reinterpret_cast<const FixedLengthArray<T, 8> *>(&data);
}

template<typename T>
FixedLengthArray<T, 9> fixedLengthArray(T obj1, T obj2, T obj3, T obj4, T obj5, T obj6, T obj7, T obj8, T obj9)
{
    T data[] = {obj1, obj2, obj3, obj4, obj5, obj6, obj7, obj8, obj9};
    return *reinterpret_cast<const FixedLengthArray<T, 9> *>(&data);
}

} // namespace Private
} // namespace Qul

#endif
