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

#include <qul/private/propertybinding.h>
#include <qul/private/input.h>
#include <qul/private/rendertree.h>
#include <platforminterface/transform.h>
#include <qul/private/matrix4x4.h>
#include <qul/private/dirtylist.h>
#include <qul/private/optional.h>
#include <qul/private/unicodestring.h>
#include <qul/private/mathfwd.h>
#include <qul/private/console.h>
#include <qul/private/attachedtype.h>
#include <qul/private/slot.h>
#include <qul/private/parentobject.h>

#include <qul/singleton.h>
#include <qul/object.h>
#include <qul/signal.h>

#include <math.h>
#include <utility>
#include <ctime>
#include <algorithm>

namespace Qul {
namespace Private {

struct JsObject
{};

namespace Builtins {

struct QulPerf : Qul::Singleton<QulPerf>
{
    Property<bool> enabled;

    Property<qreal> fps;

    Property<int> maxDirtyNodes;

    Property<qreal> textLayout;
    Property<qreal> textBlend;
    Property<qreal> rectRounded;
    Property<qreal> rectBlend;
    Property<qreal> rectFill;
    Property<qreal> imageBlend;
    Property<qreal> imageTransform;
    Property<qreal> repaint;
};

struct Math : Qul::Singleton<Math>, public Qul::Private::PrivateObject
{
    Math()
        : PI(M_PI)
    {}

    static qreal sin(qreal v) { return ::sin(v); }
    static qreal cos(qreal v) { return ::cos(v); }
    static qreal tan(qreal v) { return ::tan(v); }
    static qreal asin(qreal v) { return ::asin(v); }
    static qreal acos(qreal v) { return ::acos(v); }
    static qreal atan2(qreal a, qreal b) { return ::atan2(a, b); }
    static qreal sqrt(qreal v) { return ::sqrt(v); }
    static int round(qreal v) { return int(::round(v)); }
    static int floor(qreal v) { return int(::floor(v)); }
    static int ceil(qreal v) { return int(::ceil(v)); }
    static qreal abs(qreal v) { return ::abs(v); }
    static qreal max(qreal v1, qreal v2) { return fmax(v1, v2); }
    static qreal min(qreal v1, qreal v2) { return fmin(v1, v2); }

    static qreal random();

    // TODO: doesn't really need to be a property, but currently all
    // property access from QML uses get() and set()
    Property<qreal> PI;
};

struct Number : public Qul::Private::PrivateObject
{
    qreal value;
    Number(qreal value = 0)
        : value(value)
    {}

    String toString() const { return FormattedNumberString::createDefault(value); }

    String toFixed(int digits) const { return FormattedNumberString::createFixed(value, qMax(digits, 0)); }
    String toFixed() const { return FormattedNumberString::createFixed(value, 0); }
    String toExponential(int digits) const { return FormattedNumberString::createExponential(value, qMax(digits, 0)); }
    String toExponential() const { return FormattedNumberString::createExponential(value); }

    // ### deprecated-start, remove in 2.0
    String toFixedInt() const { return FormattedNumberString::createFixed(value, 0); }
    String toExponentialAuto() const { return FormattedNumberString::createExponential(value); }
    // ### deprecated-end

    static bool isFinite(qreal t_value) { return isfinite(t_value); }
    static bool isInteger(qreal t_value) { return floor(t_value) == ceil(t_value) && isfinite(t_value); }
    static bool isNaN(qreal t_value) { return isnan(t_value); }
};

struct Font : public Qul::Private::PrivateObject
{
    enum Weight {
        Thin = 0,        // 100
        ExtraLight = 12, // 200
        Light = 25,      // 300
        Normal = 50,     // 400
        Medium = 57,     // 500
        DemiBold = 63,   // 600
        Bold = 75,       // 700
        ExtraBold = 81,  // 800
        Black = 87       // 900
    };

    enum UnicodeCoverage {
#include "unicodeblocknames.h"
    };
};

struct Date : Qul::Private::JsObject
{
    static int getHours() { return getTimeInfo()->tm_hour; }
    static int getMinutes() { return getTimeInfo()->tm_min; }
    static int getSeconds() { return getTimeInfo()->tm_sec; }

private:
    static struct tm *getTimeInfo();
};

struct MouseEvent : Qul::Private::PrivateObject
{
    Property<bool> accepted;
    Property<bool> wasHeld;
    Property<qreal> x;
    Property<qreal> y;
};

struct KeyEvent : Qul::Private::PrivateObject
{
    Property<int> key;
    Property<uint32_t> nativeScanCode;
};

struct Matrix4x4
{
    Matrix4x4()
        : m(Qul::Private::Matrix4x4::Identity)
    {}

    // Matrix4x4 QML uses transposed matrix representation
    // to match transforms compostion order used by 2D
    // transforms (PlatformInterface::Transform): left-to-right
    // clang-format off
    Matrix4x4(float m11, float m12, float m13, float m14,
              float m21, float m22, float m23, float m24,
              float m31, float m32, float m33, float m34,
              float m41, float m42, float m43, float m44)
    : m(m11, m21, m31, m41,
        m12, m22, m32, m42,
        m13, m23, m33, m43,
        m14, m24, m34, m44)
    {}
    // clang-format on

    const Qul::Private::Matrix4x4 &data() const { return m; }

    // Accessors for code generator
    inline float value(int row, int column) const { return m(column, row); }
    inline void setValue(int row, int column, float value) { m(column, row) = value; }

    // QML exposed funtions
    String toString() const;

    inline Matrix4x4 times(const Matrix4x4 &matrix) const
    {
        auto result = matrix;
        result.m *= this->m;
        return result;
    }

    inline Matrix4x4 times(float factor) const
    {
        auto result = *this;
        result.m *= factor;
        return result;
    }

    inline Matrix4x4 plus(const Matrix4x4 &matrix) const
    {
        auto result = matrix;
        result.m += this->m;
        return result;
    }

    friend bool operator==(const Matrix4x4 &lhs, const Matrix4x4 &rhs) { return lhs.m == rhs.m; }

private:
    Qul::Private::Matrix4x4 m;
};

struct GlobalQtObject : Qul::Private::PrivateObject
{
    const GlobalQtObject *value() const { return this; }
    GlobalQtObject *value() { return this; }

    static Property<String> uiLanguage;

    static PlatformInterface::Rgba32 rgba(float r, float g, float b, float a = 1.f)
    {
        return PlatformInterface::Rgba32(r, g, b, a);
    }

    // clang-format off
    static Matrix4x4 matrix4x4(float m11, float m12, float m13, float m14,
                               float m21, float m22, float m23, float m24,
                               float m31, float m32, float m33, float m34,
                               float m41, float m42, float m43, float m44)
    {
        return Matrix4x4{m11, m12, m13, m14,
                         m21, m22, m23, m24,
                         m31, m32, m33, m34,
                         m41, m42, m43, m44};
    }
    // clang-format on

    enum Key {
#include "keyenum.incl"
    };

    enum Orientation { Vertical, Horizontal };
};

struct GlobalObject
{
    static Items::Console console;
    static GlobalQtObject Qt;

    static bool isFinite(qreal t_value) { return Number::isFinite(t_value); }
    static bool isNaN(qreal t_value) { return Number::isNaN(t_value); }
};

} // namespace Builtins
} // namespace Private
} // namespace Qul
