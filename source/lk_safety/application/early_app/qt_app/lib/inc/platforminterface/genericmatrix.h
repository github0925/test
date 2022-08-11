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

#include <type_traits>
#include <cstring>
#include <cassert>

namespace Qul {
namespace PlatformInterface {

template<int, int, typename>
struct GenericMatrix;

/** Generic MxN matrix implementation
 *
 * Works only with trivially-copyable types.
 *
 * In difference to upstream QGenericMatrix,
 * a row-major order is used internally.
 *
 * Qt uses column-major representation internally to match OpenGL.
 * Qul uses row-major because it is more natural and easier to debug.
 * All other matrices in Qul also are row-major, thus generic matrix
 * is consistent with that.
 */
template<int M, int N, typename T>
struct GenericMatrixBase
{
    static_assert(std::is_trivially_copyable<T>::value,
                  "GenericMatrixBase class works only with trivially-copyable types");
    static_assert(std::is_default_constructible<T>::value,
                  "GenericMatrixBase class works only with default-constructible types");
    static_assert(std::is_constructible<T, int>::value,
                  "GenericMatrixBase class works only with integer constructible types");

    enum Initialization { Uninitialized, ZeroInitialized };

    GenericMatrixBase();
    explicit GenericMatrixBase(Initialization initialization);
    explicit GenericMatrixBase(const T *values);

    const T &operator()(int row, int column) const;
    T &operator()(int row, int column);

    bool operator==(const GenericMatrixBase<M, N, T> &other) const;
    bool operator!=(const GenericMatrixBase<M, N, T> &other) const;

    const T *data() const;
    T *data();

    GenericMatrixBase<M, N, T> &operator+=(const GenericMatrixBase<M, N, T> &other);
    GenericMatrixBase<M, N, T> &operator*=(T factor);

    template<int MM, int NN, typename TT>
    friend GenericMatrix<MM, NN, TT> operator+(const GenericMatrix<MM, NN, TT> &m1, const GenericMatrix<MM, NN, TT> &m2);
    template<int MM, int N1, int N2, typename TT>
    friend GenericMatrix<MM, N2, TT> operator*(const GenericMatrix<MM, N1, TT> &m1, const GenericMatrix<N1, N2, TT> &m2);
    template<int MM, int NN, typename T1, typename T2>
    friend GenericMatrix<MM, NN, T2> operator*(T1 factor, const GenericMatrix<MM, NN, T2> &m);

protected:
    template<int MM, int NN, typename TT>
    friend struct GenericMatrix;

    T m[M][N];
};

#ifdef Q_CLANG_QDOC
// Bunch of hacks to bend qdoc's html output to our will.
template<int M, int N, typename T>
struct GenericMatrix
{
    enum Initialization { Uninitialized = 0, ZeroInitialized = 1, Identity = 2 };

    GenericMatrix();
    explicit GenericMatrix(Initialization initialization);
    explicit GenericMatrix(const T *values);

    const T &operator()(int row, int column) const;
    T &operator()(int row, int column);

    bool operator==(const GenericMatrix<M, N, T> &other) const;
    bool operator!=(const GenericMatrix<M, N, T> &other) const;

    const T *data() const;
    T *data();

    void setToIdentity();
    bool isIdentity() const;

    GenericMatrix<M, N, T> &operator+=(const GenericMatrix<M, N, T> &other);
    GenericMatrix<M, N, T> &operator*=(T factor);
    GenericMatrix<M, N, T> &operator*=(const GenericMatrix<M, N, T> &other);

    template<int MM, int NN, typename TT>
    friend GenericMatrix<MM, NN, TT> operator+(const GenericMatrix<MM, NN, TT> &m1, const GenericMatrix<MM, NN, TT> &m2);
    template<int MM, int N1, int N2, typename TT>
    friend GenericMatrix<MM, N2, TT> operator*(const GenericMatrix<MM, N1, TT> &m1, const GenericMatrix<N1, N2, TT> &m2);
    template<int MM, int NN, typename T1, typename T2>
    friend GenericMatrix<MM, NN, T2> operator*(T1 factor, const GenericMatrix<MM, NN, T2> &m);
};
#else
// Rectangular matrices
template<int M, int N, typename T>
struct GenericMatrix : GenericMatrixBase<M, N, T>
{
    using GenericMatrixBase<M, N, T>::GenericMatrixBase;
};
#endif

// Square matrices
template<int M, typename T>
struct GenericMatrix<M, M, T> : GenericMatrixBase<M, M, T>
{
    using GenericMatrixBase<M, M, T>::GenericMatrixBase;
    using GenericMatrixBase<M, M, T>::operator*=;

    enum Initialization { Uninitialized, ZeroInitialized, Identity };

    GenericMatrix() = default;
    explicit GenericMatrix(Initialization initialization);

    void setToIdentity();
    bool isIdentity() const;

    GenericMatrix<M, M, T> &operator*=(const GenericMatrix<M, M, T> &other);
};

// Common implementation
template<int M, int N, typename T>
inline GenericMatrixBase<M, N, T>::GenericMatrixBase()
    : GenericMatrixBase(ZeroInitialized)
{}

template<int M, int N, typename T>
inline GenericMatrixBase<M, N, T>::GenericMatrixBase(Initialization initialization)
{
    if (initialization == ZeroInitialized) {
        std::memset(m, 0, sizeof(m));
    }
}

template<int M, int N, typename T>
inline GenericMatrixBase<M, N, T>::GenericMatrixBase(const T *values)
{
    std::memcpy(m, values, sizeof(m));
}

template<int M, int N, typename T>
inline const T &GenericMatrixBase<M, N, T>::operator()(int row, int column) const
{
    return const_cast<GenericMatrixBase<M, N, T> *>(this)->operator()(row, column);
}

template<int M, int N, typename T>
inline T &GenericMatrixBase<M, N, T>::operator()(int row, int column)
{
    assert(row >= 0 && row < M && column >= 0 && column < N);
    return m[row][column];
}

template<int M, int N, typename T>
inline bool GenericMatrixBase<M, N, T>::operator==(const GenericMatrixBase<M, N, T> &other) const
{
    if (&other == this) {
        return true;
    }
    return std::memcmp(m, other.m, sizeof(m)) == 0;
}

template<int M, int N, typename T>
inline bool GenericMatrixBase<M, N, T>::operator!=(const GenericMatrixBase<M, N, T> &other) const
{
    return !(*this == other);
}

template<int M, int N, typename T>
inline const T *GenericMatrixBase<M, N, T>::data() const
{
    return const_cast<GenericMatrixBase<M, N, T> *>(this)->data();
}

template<int M, int N, typename T>
inline T *GenericMatrixBase<M, N, T>::data()
{
    return &m[0][0];
}

template<int M, int N, typename T>
GenericMatrixBase<M, N, T> &GenericMatrixBase<M, N, T>::operator+=(const GenericMatrixBase<M, N, T> &other)
{
    for (int row = 0; row < M; ++row)
        for (int col = 0; col < N; ++col)
            m[row][col] += other.m[row][col];
    return *this;
}

template<int M, int N, typename T>
GenericMatrixBase<M, N, T> &GenericMatrixBase<M, N, T>::operator*=(T factor)
{
    for (int row = 0; row < M; ++row)
        for (int col = 0; col < N; ++col)
            m[row][col] *= factor;
    return *this;
}

// Square matrices specialization
template<int M, typename T>
GenericMatrix<M, M, T>::GenericMatrix(Initialization initialization)
    : GenericMatrixBase<M, M, T>::GenericMatrixBase(initialization == ZeroInitialized
                                                        ? GenericMatrixBase<M, M, T>::ZeroInitialized
                                                        : GenericMatrixBase<M, M, T>::Uninitialized)
{
    if (initialization == Identity) {
        setToIdentity();
    }
}

template<int M, typename T>
void GenericMatrix<M, M, T>::setToIdentity()
{
    std::memset(this->m, 0, sizeof(this->m));
    for (int i = 0; i < M; ++i) {
        this->m[i][i] = T(1);
    }
}

template<int M, typename T>
bool GenericMatrix<M, M, T>::isIdentity() const
{
    for (int row = 0; row < M; ++row) {
        for (int col = 0; col < M; ++col) {
            if (this->m[row][col] != ((row == col) ? T(1) : T(0)))
                return false;
        }
    }
    return true;
}

template<int M, typename T>
GenericMatrix<M, M, T> &GenericMatrix<M, M, T>::operator*=(const GenericMatrix<M, M, T> &other)
{
    for (int row = 0; row < M; ++row) {
        T sum[M] = {0};
        for (int col = 0; col < M; ++col) {
            for (int j = 0; j < M; ++j)
                sum[col] += this->m[row][j] * other.m[j][col];
        }
        std::memcpy(&this->m[row][0], sum, sizeof(this->m[row]));
    }
    return *this;
}

// Non-member operators
template<int M, int N, typename T>
GenericMatrix<M, N, T> operator+(const GenericMatrix<M, N, T> &m1, const GenericMatrix<M, N, T> &m2)
{
    GenericMatrix<M, N, T> result(GenericMatrix<M, N, T>::Uninitialized);
    for (int row = 0; row < M; ++row)
        for (int col = 0; col < N; ++col)
            result.m[row][col] = m1.m[row][col] + m2.m[row][col];
    return result;
}

template<int M, int N1, int N2, typename T>
GenericMatrix<M, N2, T> operator*(const GenericMatrix<M, N1, T> &m1, const GenericMatrix<N1, N2, T> &m2)
{
    GenericMatrix<M, N2, T> result(GenericMatrix<M, N2, T>::Uninitialized);
    for (int row = 0; row < M; ++row) {
        for (int col = 0; col < N2; ++col) {
            T sum(0);
            for (int j = 0; j < N1; ++j)
                sum += m1.m[row][j] * m2.m[j][col];
            result.m[row][col] = sum;
        }
    }
    return result;
}

template<int M, int N, typename T1, typename T2>
GenericMatrix<M, N, T2> operator*(T1 factor, const GenericMatrix<M, N, T2> &m)
{
    static_assert(std::is_arithmetic<T1>::value, "Multiplication by factor only possible with arithmetic types");

    GenericMatrix<M, N, T2> result(GenericMatrix<M, N, T2>::Uninitialized);
    for (int row = 0; row < M; ++row)
        for (int col = 0; col < N; ++col)
            result.m[row][col] = T2(factor) * m.m[row][col];
    return result;
}

} // namespace PlatformInterface
} // namespace Qul
