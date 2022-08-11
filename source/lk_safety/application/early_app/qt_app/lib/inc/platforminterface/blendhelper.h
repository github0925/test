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
#ifndef QUL_PLATFORMINTERFACE_BLENDHELPER_H
#define QUL_PLATFORMINTERFACE_BLENDHELPER_H

namespace Qul {
namespace PlatformInterface {

void blendRectTiled(
    DrawingDevice *buffer, const Rect &rect, Rgba32 color, void (*finish)(), void (*invalidateCache)(void *, int));

void blendRectTiledAlphaMap(
    DrawingDevice *buffer, const Rect &rect, Rgba32 color, void (*finish)(), void (*invalidateCache)(void *, int));

} // namespace PlatformInterface
} // namespace Qul

#endif // QUL_PLATFORMINTERFACE_BLENDHELPER_H
