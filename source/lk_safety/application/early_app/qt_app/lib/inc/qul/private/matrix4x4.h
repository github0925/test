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

#include <platforminterface/transform.h>
#include <platforminterface/genericmatrix.h>

namespace Qul {
namespace Private {

/** Matrix used for calculating 3D transformations
 *
 * In order to get consistent transformation chaining behavior
 * (same as for PlatformInterface::Transform) and to get expected
 * results when converting to and from PlatformInterface::Transform,
 * layout of this matrix should be transposed comparing to ordinary
 * transformation matrices (e.g. x-axis translation should be stored in m41).
 */
struct Matrix4x4 : public PlatformInterface::GenericMatrix<4, 4, float>
{
    using GenericMatrix::GenericMatrix;
    using GenericMatrix::operator=;

    Matrix4x4() = default;
    explicit Matrix4x4(const PlatformInterface::Transform &transform);

    // clang-format off
    Matrix4x4(float m11, float m12, float m13, float m14,
              float m21, float m22, float m23, float m24,
              float m31, float m32, float m33, float m34,
              float m41, float m42, float m43, float m44);
    // clang-format on

    PlatformInterface::Transform toTransform() const;
};

// clang-format off
inline Matrix4x4::Matrix4x4(float m11, float m12, float m13, float m14,
                            float m21, float m22, float m23, float m24,
                            float m31, float m32, float m33, float m34,
                            float m41, float m42, float m43, float m44)
                            : GenericMatrix(GenericMatrix::Uninitialized)
// clang-format on
{
    m[0][0] = m11;
    m[0][1] = m12;
    m[0][2] = m13;
    m[0][3] = m14;

    m[1][0] = m21;
    m[1][1] = m22;
    m[1][2] = m23;
    m[1][3] = m24;

    m[2][0] = m31;
    m[2][1] = m32;
    m[2][2] = m33;
    m[2][3] = m34;

    m[3][0] = m41;
    m[3][1] = m42;
    m[3][2] = m43;
    m[3][3] = m44;
}

inline Matrix4x4 operator*(const Matrix4x4 &lhs, const Matrix4x4 &rhs)
{
    auto result = lhs;
    result *= rhs;
    return result;
}

template<typename T>
inline Matrix4x4 operator*(T factor, const Matrix4x4 &rhs)
{
    static_assert(std::is_arithmetic<T>::value, "Multiplication by factor only possible with arithmetic types");
    auto result = rhs;
    result *= float(factor);
    return result;
}

inline Matrix4x4 operator+(const Matrix4x4 &lhs, const Matrix4x4 &rhs)
{
    auto result = lhs;
    result += rhs;
    return result;
}

} // namespace Private
} // namespace Qul
