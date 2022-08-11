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
#ifndef QUL_TRANSFORM_H
#define QUL_TRANSFORM_H

#include <platforminterface/rect.h>
#include <platforminterface/genericmatrix.h>
#include <qul/private/global.h>
#include <cstring>
#include <limits>

namespace Qul {
namespace PlatformInterface {

class Transform : GenericMatrix<3, 3, float>
{
public:
    enum Type { Identity, Translate, UniformScale, Scale, Rotation, ScaleRotation, Shear, Project };

    inline float m11() const { return m[0][0]; }
    inline float m12() const { return m[0][1]; }
    inline float m13() const { return m[0][2]; }
    inline float m21() const { return m[1][0]; }
    inline float m22() const { return m[1][1]; }
    inline float m23() const { return m[1][2]; }
    inline float m31() const { return m[2][0]; }
    inline float m32() const { return m[2][1]; }
    inline float m33() const { return m[2][2]; }
    inline float dx() const { return m[2][0]; }
    inline float dy() const { return m[2][1]; }

    inline Type type() const { return _type; }

    Transform()
        : GenericMatrix(GenericMatrix::Identity)
        , _type(Identity)
    {}

    Transform translated(float x, float y) const
    {
        if (x == 0.f && y == 0.f) {
            return *this;
        }
        Transform result = *this;
        return result.translate(x, y);
    }

    Transform preTranslated(float x, float y) const
    {
        if (x == 0.f && y == 0.f) {
            return *this;
        }
        Transform translation = Transform::fromTranslation(x, y);
        return translation * *this;
    }

    Transform &translate(float x, float y)
    {
        if (x == 0.f && y == 0.f) {
            return *this;
        }

        if (_type < Project) {
            m[2][0] += x;
            m[2][1] += y;
        } else {
            m[0][0] += x * m[0][2];
            m[1][0] += x * m[1][2];
            m[2][0] += x * m[2][2];

            m[0][1] += y * m[0][2];
            m[1][1] += y * m[1][2];
            m[2][1] += y * m[2][2];
        }

        _type = std::max(Translate, _type);
        return *this;
    }

    Transform &scale(float sx, float sy)
    {
        if (sx == 1.f && sy == 1.f) {
            return *this;
        }

        if (_type <= Translate) {
            m[0][0] = sx;
            m[2][0] *= sx;
            m[1][1] = sy;
            m[2][1] *= sy;
        } else if (_type <= Scale) {
            m[0][0] *= sx;
            m[2][0] *= sx;
            m[1][1] *= sy;
            m[2][1] *= sy;
        } else {
            m[0][0] *= sx;
            m[1][0] *= sx;
            m[2][0] *= sx;
            m[0][1] *= sy;
            m[1][1] *= sy;
            m[2][1] *= sy;
        }

        if (_type == Rotation && sx != sy) {
            _type = Shear;
        } else {
            _type = std::max(sx == sy ? UniformScale : Scale, _type);
        }
        return *this;
    }

    Transform &rotate(float angle);

    Transform(Type type, float m11, float m12, float m21, float m22, float dx, float dy)
        : GenericMatrix(GenericMatrix::Uninitialized)
        , _type(type)
    {
        m[0][0] = m11;
        m[0][1] = m12;
        m[0][2] = 0.0f;
        m[1][0] = m21;
        m[1][1] = m22;
        m[1][2] = 0.0f;
        m[2][0] = dx;
        m[2][1] = dy;
        m[2][2] = 1.0f;
    }

    Transform(float m11, float m12, float m21, float m22, float dx, float dy)
        : Transform(Shear, m11, m12, m21, m22, dx, dy)
    {}

    Transform(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33)
        : GenericMatrix(GenericMatrix::Uninitialized)
        , _type(Project)
    {
        m[0][0] = m11;
        m[0][1] = m12;
        m[0][2] = m13;
        m[1][0] = m21;
        m[1][1] = m22;
        m[1][2] = m23;
        m[2][0] = m31;
        m[2][1] = m32;
        m[2][2] = m33;
    }

    static Transform fromTranslation(float dx, float dy)
    {
        if (dx == 0.f && dy == 0.f) {
            return {};
        }
        return Transform(Translate, 1, 0, 0, 1, dx, dy);
    }

    static Transform fromRotation(float angle);

    static Transform fromScale(float sx, float sy)
    {
        if (sx == 1.f && sy == 1.f) {
            return {};
        }
        return Transform(sx == sy ? UniformScale : Scale, sx, 0, 0, sy, 0, 0);
    }

    inline bool operator==(const Transform &o) const { return o._type == _type && std::memcmp(m, o.m, sizeof(m)) == 0; }

    inline Transform &operator*=(const Transform &o)
    {
        if (o._type == Identity) {
            return *this;
        } else if (_type == Identity) {
            *this = o;
            return *this;
        }

        auto resultType = std::max(_type, o._type);
        if (resultType <= Translate) {
            m[2][0] += o.m[2][0];
            m[2][1] += o.m[2][1];
        } else if (resultType <= Scale) {
            m[0][0] *= o.m[0][0];
            m[1][1] *= o.m[1][1];
            m[2][0] = m[2][0] * o.m[0][0] + o.m[2][0];
            m[2][1] = m[2][1] * o.m[1][1] + o.m[2][1];
        } else if (resultType <= Shear) {
            float temp = m[0][0] * o.m[0][0] + m[0][1] * o.m[1][0];
            m[0][1] = m[0][0] * o.m[0][1] + m[0][1] * o.m[1][1];
            m[0][0] = temp;

            temp = m[1][0] * o.m[0][0] + m[1][1] * o.m[1][0];
            m[1][1] = m[1][0] * o.m[0][1] + m[1][1] * o.m[1][1];
            m[1][0] = temp;

            temp = m[2][0] * o.m[0][0] + m[2][1] * o.m[1][0] + o.m[2][0];
            m[2][1] = m[2][0] * o.m[0][1] + m[2][1] * o.m[1][1] + o.m[2][1];
            m[2][0] = temp;

            if (_type >= Rotation && (o._type == Scale || o._type == ScaleRotation)) {
                resultType = Shear;
            } else if (_type == Scale && o._type == Rotation) {
                resultType = ScaleRotation;
            }
        } else { // Project
            GenericMatrix::operator*=(o);
        }
        _type = resultType;
        return *this;
    }

    inline Transform operator*(const Transform &o) const
    {
        if (o._type == Identity) {
            return *this;
        } else if (_type == Identity) {
            return o;
        }

        const auto resultType = std::max(_type, o._type);
        if (resultType <= Translate) {
            return Transform::fromTranslation(m[2][0] + o.m[2][0], m[2][1] + o.m[2][1]);
        } else if (resultType <= Scale) {
            const float t_m11 = m[0][0] * o.m[0][0];
            const float t_m22 = m[1][1] * o.m[1][1];

            const float t_dx = m[2][0] * o.m[0][0] + o.m[2][0];
            const float t_dy = m[2][1] * o.m[1][1] + o.m[2][1];

            return Transform(resultType, t_m11, 0, 0, t_m22, t_dx, t_dy);
        } else if (resultType <= Shear) {
            const float t_m11 = m[0][0] * o.m[0][0] + m[0][1] * o.m[1][0];
            const float t_m12 = m[0][0] * o.m[0][1] + m[0][1] * o.m[1][1];
            const float t_m21 = m[1][0] * o.m[0][0] + m[1][1] * o.m[1][0];
            const float t_m22 = m[1][0] * o.m[0][1] + m[1][1] * o.m[1][1];

            const float t_dx = m[2][0] * o.m[0][0] + m[2][1] * o.m[1][0] + o.m[2][0];
            const float t_dy = m[2][0] * o.m[0][1] + m[2][1] * o.m[1][1] + o.m[2][1];

            Type type = resultType;
            if (_type >= Rotation && (o._type == Scale || o._type == ScaleRotation)) {
                type = Shear;
            } else if (_type == Scale && o._type == Rotation) {
                type = ScaleRotation;
            }
            return Transform(type, t_m11, t_m12, t_m21, t_m22, t_dx, t_dy);
        } else { // Project
            Transform result = *this;
            static_cast<GenericMatrix *>(&result)->operator*=(o);
            result._type = resultType;
            return result;
        }
    }

    inline float determinant() const
    {
        // clang-format off
        return m[0][0] * (m[2][2] * m[1][1] - m[2][1] * m[1][2]) -
               m[1][0] * (m[2][2] * m[0][1] - m[2][1] * m[0][2]) +
               m[2][0] * (m[1][2] * m[0][1] - m[1][1] * m[0][2]);
        // clang-format on
    }

    Transform inverted(bool *invertible) const;

    inline PlatformInterface::PointF map(PlatformInterface::PointF point) const
    {
        if (_type <= Translate) {
            return point + PlatformInterface::PointF(m[2][0], m[2][1]);
        } else if (_type <= Scale) {
            return PlatformInterface::PointF(m[0][0] * point.x() + m[2][0], m[1][1] * point.y() + m[2][1]);
        } else {
            auto x = m[0][0] * point.x() + m[1][0] * point.y() + m[2][0];
            auto y = m[0][1] * point.x() + m[1][1] * point.y() + m[2][1];
            if (_type == Project) {
                const float z = m[0][2] * point.x() + m[1][2] * point.y() + m[2][2];
                if (!std::numeric_limits<float>::is_iec559 && z == 0) {
                    // Invalid projection
                    return point;
                } else if (z != 1.f) {
                    const float w = 1.f / z;
                    x *= w;
                    y *= w;
                }
            }
            return PlatformInterface::PointF(x, y);
        }
    }

    inline PlatformInterface::RectF map(const PlatformInterface::RectF &rect) const
    {
        if (_type <= Translate) {
            return rect.translated(m[2][0], m[2][1]);
        } else {
            return mapArbitrary(rect);
        }
    };

    PlatformInterface::PointF translation() const { return PlatformInterface::PointF(m[2][0], m[2][1]); }

    // Verify what transform type is really represented here
    Type optimize();

private:
    PlatformInterface::RectF mapArbitrary(const PlatformInterface::RectF &rect) const;

    Type _type;
};

} // namespace PlatformInterface

namespace Private {
inline bool qFuzzyCompare(const PlatformInterface::Transform &t1, const PlatformInterface::Transform &t2)
{
    return t1.type() == t2.type() && qFuzzyCompare(t1.m11(), t2.m11()) && qFuzzyCompare(t1.m12(), t2.m12())
           && qFuzzyCompare(t1.m13(), t2.m13()) && qFuzzyCompare(t1.m21(), t2.m21())
           && qFuzzyCompare(t1.m22(), t2.m22()) && qFuzzyCompare(t1.m23(), t2.m23())
           && qFuzzyCompare(t1.m31(), t2.m31()) && qFuzzyCompare(t1.m32(), t2.m32())
           && qFuzzyCompare(t1.m33(), t2.m33());
}
} // namespace Private
} // namespace Qul

#endif
