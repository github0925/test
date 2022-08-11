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

#define QUL_INCBIN_INCLUDE_DIRECTIVE ".incbin"
#define QUL_INCBIN_ALIGN_DIRECTIVE ".balign"
#define QUL_INCBIN_SECTION_DIRECTIVE ".section"
#define QUL_INCBIN_GLOBAL_DIRECTIVE ".global"
#define QUL_INCBIN_SECTION_ATTRIBUTES ",\"a\",%progbits\n"
#define QUL_INCBIN_MANGLE_PREFIX ""

#define QUL_INCBIN_STR(x) #x
#define QUL_INCBIN_TO_STRING(x) QUL_INCBIN_STR(x)

#ifdef QUL_STATIC_ASSET_SEGMENT
#define QUL_INCBIN_SECTION \
    QUL_INCBIN_SECTION_DIRECTIVE " " QUL_INCBIN_TO_STRING(QUL_STATIC_ASSET_SEGMENT) QUL_INCBIN_SECTION_ATTRIBUTES
#else
#define QUL_INCBIN_SECTION
#endif

#ifdef QUL_STATIC_NO_PRELOAD_ASSET_SEGMENT
#define QUL_INCBIN_NO_PRELOAD_SECTION \
    QUL_INCBIN_SECTION_DIRECTIVE " " QUL_INCBIN_TO_STRING(QUL_STATIC_NO_PRELOAD_ASSET_SEGMENT) \
        QUL_INCBIN_SECTION_ATTRIBUTES
#else
#define QUL_INCBIN_NO_PRELOAD_SECTION
#endif

#define QUL_INCBIN_MANGLED_SYMBOL(SYMBOL) QUL_INCBIN_MANGLE_PREFIX #SYMBOL
#define QUL_INCBIN_EXTERN(SYMBOL) extern const unsigned char SYMBOL[]

// clang-format off
#define QUL_INCBIN(SYMBOL, ALIGNMENT, FILENAME) \
    asm(QUL_INCBIN_SECTION \
        QUL_INCBIN_GLOBAL_DIRECTIVE " " QUL_INCBIN_MANGLED_SYMBOL(SYMBOL) "\n" \
        QUL_INCBIN_ALIGN_DIRECTIVE " " #ALIGNMENT "\n" \
        QUL_INCBIN_MANGLED_SYMBOL(SYMBOL) ":\n" \
        QUL_INCBIN_INCLUDE_DIRECTIVE " " #FILENAME "\n"); \
    QUL_INCBIN_EXTERN(SYMBOL)
#define QUL_INCBIN_NO_PRELOAD(SYMBOL, ALIGNMENT, FILENAME) \
    asm(QUL_INCBIN_NO_PRELOAD_SECTION \
        QUL_INCBIN_GLOBAL_DIRECTIVE " " QUL_INCBIN_MANGLED_SYMBOL(SYMBOL) "\n" \
        QUL_INCBIN_ALIGN_DIRECTIVE " " #ALIGNMENT "\n" \
        QUL_INCBIN_MANGLED_SYMBOL(SYMBOL) ":\n" \
        QUL_INCBIN_INCLUDE_DIRECTIVE " " #FILENAME "\n"); \
    QUL_INCBIN_EXTERN(SYMBOL)
// clang-format on
