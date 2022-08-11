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

namespace Qul {

enum PixelFormat {
    PixelFormat_ARGB32,
    PixelFormat_ARGB32_Premultiplied,
    PixelFormat_RGB32,
    PixelFormat_RGB888, // internal because it does not have a getDrawFunction<>()
    PixelFormat_RGB16,
    PixelFormat_Alpha8,
    PixelFormat_ARGB4444,
    PixelFormat_ARGB4444_Premultiplied,
    PixelFormat_Invalid,
    PixelFormat_Custom = 1024
};

static const PixelFormat PixelFormat_NumPixelFormats = static_cast<PixelFormat>(PixelFormat_ARGB4444_Premultiplied + 1);

} // namespace Qul
