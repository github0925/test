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
#ifndef QUL_PLATFORMINTERFACE_H
#define QUL_PLATFORMINTERFACE_H

#include <qul/private/global.h> // quint64

namespace Qul {
namespace PlatformInterface {

// type used for coordinates
typedef short coord_t;
class Screen;

struct TouchPoint
{
    enum State { Pressed = 0x01, Moved = 0x02, Stationary = 0x04, Released = 0x08 };

    TouchPoint()
        : id(0)
        , positionX(0)
        , positionY(0)
        , areaX(0)
        , areaY(0)
        , pressure(1.0)
        , rotation(0)
        , state(Stationary)
    {}
    TouchPoint(int x, int y, State state)
        : id(0)
        , positionX(x)
        , positionY(y)
        , areaX(0)
        , areaY(0)
        , pressure(1.0)
        , rotation(0)
        , state(state)
    {}

    int id;
    int positionX;
    int positionY;
    float areaX;
    float areaY;
    float pressure;
    float rotation;
    State state;
};

enum KeyEventType { KeyPressEvent, KeyReleaseEvent };

// Equivalent to the Qt::KeyboardModifier enum in Qt
enum KeyboardModifier {
    NoKeyboardModifier = 0x00000000,
    ShiftKeyboardModifier = 0x02000000,
    ControlKeyboardModifier = 0x04000000,
    AltKeyboardModifier = 0x08000000,
    MetaKeyboardModifier = 0x10000000,
    KeypadKeyboardModifier = 0x20000000,
    GroupSwitchKeyboardModifier = 0x40000000,
    KeyboardModifierMask = 0xfe000000
};

void handleTouchEvent(Screen *screen, uint64_t timestamp, const TouchPoint *touchPoints, unsigned int numTouchPoints);
void handleTouchCancelEvent(Screen *screen, uint64_t timestamp);
void handleKeyEvent(uint64_t timestamp,
                    KeyEventType type,
                    int key,
                    uint32_t nativeScanCode = 0,
                    unsigned int modifiers = NoKeyboardModifier,
                    char *textUtf8 = NULL,
                    bool autoRepeat = false);
void updateEngine(uint64_t timestamp);
void init16bppRendering();
void init24bppRendering();
void init32bppRendering();

} // namespace PlatformInterface
} // namespace Qul

#endif // QUL_PLATFORMINTERFACE_H
