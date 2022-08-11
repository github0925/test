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

#include <qul/private/unicodestring.h>
#include <qul/object.h>
#include <qul/private/font.h>
#include <cstdio>

namespace Qul {
namespace Private {

void log(float arg);
void log(double arg);

inline void log(int arg)
{
    std::printf("%d ", arg);
}

inline void log(const char *arg)
{
    std::printf("%s ", arg);
}

void log(Char arg);
void log(const String &str);
void log(PlatformInterface::Rgba32 arg);
void log(FontPointer font);
void log(bool arg);
void log(const void *ptr);
void log(const Qul::Object *obj);

inline void newline()
{
    std::printf("\r\n");
}

namespace Items {

struct Console : Qul::Object
{
    Console *value() { return this; }
    const Console *value() const { return this; }

    void log() const { newline(); }

    template<typename T1>
    void log(T1 arg1) const
    {
        Qul::Private::log(arg1);
        newline();
    }

    template<typename T1, typename T2>
    void log(T1 arg1, T2 arg2) const
    {
        Qul::Private::log(arg1);
        Qul::Private::log(arg2);
        newline();
    }

    template<typename T1, typename T2, typename T3>
    void log(T1 arg1, T2 arg2, T3 arg3) const
    {
        Qul::Private::log(arg1);
        Qul::Private::log(arg2);
        Qul::Private::log(arg3);
        newline();
    }

    template<typename T1, typename T2, typename T3, typename T4>
    void log(T1 arg1, T2 arg2, T3 arg3, T4 arg4) const
    {
        Qul::Private::log(arg1);
        Qul::Private::log(arg2);
        Qul::Private::log(arg3);
        Qul::Private::log(arg4);
        newline();
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void log(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5) const
    {
        Qul::Private::log(arg1);
        Qul::Private::log(arg2);
        Qul::Private::log(arg3);
        Qul::Private::log(arg4);
        Qul::Private::log(arg5);
        newline();
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void log(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6) const
    {
        Qul::Private::log(arg1);
        Qul::Private::log(arg2);
        Qul::Private::log(arg3);
        Qul::Private::log(arg4);
        Qul::Private::log(arg5);
        Qul::Private::log(arg6);
        newline();
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void log(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7) const
    {
        Qul::Private::log(arg1);
        Qul::Private::log(arg2);
        Qul::Private::log(arg3);
        Qul::Private::log(arg4);
        Qul::Private::log(arg5);
        Qul::Private::log(arg6);
        Qul::Private::log(arg7);
        newline();
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    void log(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8) const
    {
        Qul::Private::log(arg1);
        Qul::Private::log(arg2);
        Qul::Private::log(arg3);
        Qul::Private::log(arg4);
        Qul::Private::log(arg5);
        Qul::Private::log(arg6);
        Qul::Private::log(arg7);
        Qul::Private::log(arg8);
        newline();
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    void log(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9) const
    {
        Qul::Private::log(arg1);
        Qul::Private::log(arg2);
        Qul::Private::log(arg3);
        Qul::Private::log(arg4);
        Qul::Private::log(arg5);
        Qul::Private::log(arg6);
        Qul::Private::log(arg7);
        Qul::Private::log(arg8);
        Qul::Private::log(arg9);
        newline();
    }
};

} // namespace Items
} // namespace Private
} // namespace Qul
