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
#ifndef QUL_SVGCOLORS_H
#define QUL_SVGCOLORS_H

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

#include <qul/private/global.h>
#include <platforminterface/rgba32.h>

#define QUL_SVG_COLOR_MAX_NAME_SIZE 20

namespace Qul {
namespace Private {

class String;

/** Returns value of a named color.
 *
 * Supports the 147 standard colors.
 * For strings that aren't colors it usually returns 0,
 * but due to an optimization may return a random color.
 */
PlatformInterface::Rgba32 mapSvgColorNameToValue(const String &string);

} // namespace Private
} // namespace Qul

#endif
