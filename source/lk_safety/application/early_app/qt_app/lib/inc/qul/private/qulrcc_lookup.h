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

#include <qul/private/global.h>
#include <qul/image.h>

namespace Qul {
namespace Private {

struct RasterBuffer;
class String;

SharedImage findImageForString(const String &str);

#ifdef QUL_RUNTIME_RESOURCE_LOOKUP_SUPPORT

struct ImageAssetLookupEntry
{
    quint16 m_lookup; // size or hash
    quint16 m_first;
    quint16 m_last;
};

struct ImageAssetEntry
{
    const char *m_name;
    const RasterBuffer *m_buffer;
};

// Lookup table that maps path sizes to ranges in qrcImageLookupHash
extern const ImageAssetLookupEntry qrcImageLookupSize[];
// Length of qrcImageLookupSize
extern const quint16 qrcImageLookupSizeLength;

// Lookup table that maps path hashes to:
// - the RasterBuffer in qrcImageLookupDirect (m_last == 65535)
// - a range in qrcImageLookupPath (otherwise)
extern const ImageAssetLookupEntry qrcImageLookupHash[];

extern const ImageAssetEntry qrcImageLookupPath[];
extern const RasterBuffer *qrcImageLookupDirect[];

#endif

} // namespace Private
} // namespace Qul
