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
#ifndef QUL_INPUT_H
#define QUL_INPUT_H

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

#include "platforminterface/point.h"
#include <platforminterface/platforminterface.h>

namespace Qul {
namespace Private {

struct TouchEvent
{
    enum State {
        TouchPointPressed = PlatformInterface::TouchPoint::Pressed,
        TouchPointMoved = PlatformInterface::TouchPoint::Moved,
        TouchPointStationary = PlatformInterface::TouchPoint::Stationary,
        TouchPointReleased = PlatformInterface::TouchPoint::Released,
        TouchPointCanceled = 0x10
    };

    State state;
    PlatformInterface::PointF position;
    PlatformInterface::PointF globalPosition;
};

struct KeyEvent
{
    enum Type { KeyPress, KeyRelease };

    Type type;
    int key;
    quint32 nativeScanCode;
};

} // namespace Private
} // namespace Qul

#endif
