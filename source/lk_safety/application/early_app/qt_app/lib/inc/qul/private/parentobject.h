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

#include <qul/object.h>

namespace Qul {
namespace Private {

/** An Object that allows child objects.
 *
 * Normally the compiler rejects child objects in instances derived from
 * Object. In instances derived from ParentObject, child objects are allowed.
 *
 * The compiler generates calls to appendChild(parent, child) for these
 * children. Overload the free function to control what having a child
 * object means. Typically, this function should at least call
 * child->setParent(parent).
 */
struct ParentObject : Object
{
    inline explicit ParentObject(Object::Type type = Type::Object)
        : Object(type)
    {}
};

// Marker telling that this object extends another item type and that if none of the properties
// used in the derived class are used, qmltocpp will replace it with the base
// (to be used with double inheritence).
// See Type::m_extendsBase and "pragma ExtendsBase".
// See also ExtraDataPointer for an alternative for non-leaf classes.
struct ExtendsBase
{};

} // namespace Private
} // namespace Qul
