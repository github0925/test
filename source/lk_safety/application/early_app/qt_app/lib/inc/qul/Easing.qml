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
pragma qmlinterface
pragma cppname$Qul$Private$Easing
pragma Private
import "qul/private/animation.h"
QtObject {
   enum Function {
    Linear,
    InQuad, OutQuad, InOutQuad, OutInQuad,
    InCubic, OutCubic, InOutCubic, OutInCubic,
    InQuart, OutQuart, InOutQuart, OutInQuart,
    InQuint, OutQuint, InOutQuint, OutInQuint,
    InSine, OutSine, InOutSine, OutInSine,
    InExpo, OutExpo, InOutExpo, OutInExpo,
    InCirc, OutCirc, InOutCirc, OutInCirc
   }
}
