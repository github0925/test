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
#include <qul/property.h>
#include <qul/private/unicodestring.h>

namespace Qul {
namespace Private {

#define QUL_ATTACHED_PROPERTIES(TypeName) typedef TypeName qul_private_attachedtype_

#define QUL_ATTACHED_PROPERTIES_ONLY(TypeName) \
    QUL_ATTACHED_PROPERTIES(TypeName); \
    typedef TypeName qul_private_attached_properties_only

// There is a bug in IAR compiler that optimizes head away if high optimization level
// is used for this function.
#if defined(QUL_CC_IAR) && defined(NDEBUG)
#pragma optimize = medium
#endif
template<typename T>
LinkedList<T> &qulAttachedPropertiesHead()
{
    // Has to be within a function since template variable is a C++17 feature
    static LinkedList<T> head;
    return head;
}

template<typename T>
T *qulAttachedProperties(Qul::Object *object)
{
    if (!object)
        return NULL;

    T *node = qulAttachedPropertiesHead<T>().head;
    while (node) {
        if (node->parentObject() == object)
            return node;
        node = node->next;
    }
    return NULL;
}

template<typename T>
struct AttachedType : Qul::Object, LinkedListNode<T>
{
    AttachedType() { qulAttachedPropertiesHead<T>().insert(static_cast<T *>(this)); }

    // Attached type can be a child of a Object (use template to avoid ambiguities)
    template<typename X>
    friend void appendChild(X *parent, AttachedType *child)
    {
        child->setParent(parent);
    }
};

} // namespace Private
} // namespace Qul
