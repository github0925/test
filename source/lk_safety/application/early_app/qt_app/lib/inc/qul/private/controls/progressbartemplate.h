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

#include "controltemplate.h"
#include <qul/private/timeritem.h>

namespace Qul {
namespace Private {
namespace Controls {
namespace Templates {

struct ProgressBar : Control
{
    Property<qreal> from QUL_PROPERTY_DEBUG_INIT("ProgressBar::from");
    Property<qreal> to QUL_PROPERTY_DEBUG_INIT("ProgressBar::to");
    Property<qreal> value QUL_PROPERTY_DEBUG_INIT("ProgressBar::value");

    Property<qreal> position QUL_PROPERTY_DEBUG_INIT("ProgressBar::position");
    Property<qreal> visualPosition QUL_PROPERTY_DEBUG_INIT("ProgressBar::visualPosition");

    ProgressBar();
    virtual ~ProgressBar();

private:
    struct ValueChangedEvent
    {
        inline void operator()(ProgressBar *progressbar) { progressbar->updateProgress(); }
    };

    DirtyEventStaticDependency<ValueChangedEvent> onValueChanged;

    void updateProgress();
};

} // namespace Templates
} // namespace Controls
} // namespace Private
} // namespace Qul
