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

#include <qul/property.h>
#include <qul/private/flagpointer.h>

namespace Qul {

struct Image;
struct SharedImage;

namespace Private {

struct RasterBuffer;
struct ImageData;
using ImageDataPtr = Private::BiPointer<const Private::RasterBuffer, ImageData>;

struct ImagePrivate
{
    static const RasterBuffer *rasterBuffer(const SharedImage &self);
    static const RasterBuffer *readyRasterBuffer(const SharedImage &self);
    static SharedImage fromRasterBuffer(const RasterBuffer *buffer);
};

template<>
struct PropertyTraits<SharedImage>
{
    static void valueWasSetHook(PropertyBase &, SharedImage &);
    static void valueWasUnsetHook(PropertyBase &, SharedImage &);
    static void valueEquality(const SharedImage &lhs, const SharedImage &rhs, bool *markDirty, bool *assignValue);

    using ExtraBase = NoPropertyExtraBase;

    static constexpr bool checkValueEqualityOnDirtyBinding = true;
};

} // namespace Private
} // namespace Qul
