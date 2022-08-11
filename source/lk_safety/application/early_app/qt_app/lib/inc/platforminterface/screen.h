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
#ifndef QUL_SCREEN_H
#define QUL_SCREEN_H

#include <platforminterface/rect.h>
#include <platforminterface/rgba32.h>

namespace Qul {
namespace PlatformInterface {

class Screen final
{
public:
    Screen()
        : Screen(PlatformInterface::Size(0, 0), "", true)
    {}

    Screen(const PlatformInterface::Size &size, const char *identifier = "", bool resizeable = false)
        : m_size(size)
        , m_resizeable(resizeable)
        , m_identifier(identifier)
    {}

    void resize(const PlatformInterface::Size &size) { m_size = size; }
    bool isResizeable() const { return m_resizeable; }
    PlatformInterface::Size size() const { return m_size; }
    const char *identifier() const { return m_identifier; }

    void setBackgroundColor(Rgba32 color) { m_color = color; }
    Rgba32 backgroundColor() const { return m_color; }

private:
    PlatformInterface::Size m_size;
    bool m_resizeable;
    const char *m_identifier;
    Rgba32 m_color;
};

} // namespace PlatformInterface
} // namespace Qul

#endif // QUL_SCREEN_H
