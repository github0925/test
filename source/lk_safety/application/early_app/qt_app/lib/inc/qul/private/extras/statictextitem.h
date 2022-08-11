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
#ifndef QUL_STATICTEXTITEM_H
#define QUL_STATICTEXTITEM_H

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

#include <platforminterface/rect.h>
#include <platforminterface/point.h>
#include <qul/private/graphicsdevice.h>

namespace Qul {
namespace Private {

// FIXME: this struct shouldn't be available from Qml,
//        but keeping the struct in the same file as StaticText
//        requires UL-2539
struct StaticTextItem
{
    PlatformInterface::Rect bounds;

    int numGlyphs;
    int baselineOffset;

    struct Offset
    {
        int x;
        int y;
    };

    inline PlatformInterface::Point glyphOffset(int index) const
    {
        const Offset &offs = glyphOffsets[index];
        return PlatformInterface::Point(offs.x, offs.y);
    }

    const Offset *glyphOffsets;
    const AlphaMap *glyphAlphaMaps;
};

} // namespace Private
} // namespace Qul

#endif