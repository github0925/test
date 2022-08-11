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

#include <cstdint>

namespace Qul {
#if defined(QUL_QREAL_TYPE)
typedef QUL_QREAL_TYPE qreal;
#else
typedef double qreal;
#endif

#define QREAL_C(value) (static_cast<qreal>(value))

#ifndef UINT64_MAX
#if defined(QUL_HOST_TARGET) && defined(__unix__)
// On Linux Desktop int64_t is defined as long
// Not following the compiler given definition will cause linker errors.
typedef long int64_t;
typedef unsigned long uint64_t;
#else
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif
#endif
} // namespace Qul
