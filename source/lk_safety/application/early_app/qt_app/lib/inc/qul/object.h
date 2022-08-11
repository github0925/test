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

#include <qul/private/flagpointer.h>

namespace Qul {

class Object
{
public:
    enum class Type { Object, Item, Animation }; // two bits

    inline explicit Object(Type type = Type::Object)
    {
        m_parentObject.setCombinedFlag(static_cast<Private::quintptr>(type));
    }

    inline void setParent(Object *parent) { m_parentObject = parent; }

    inline Object *parentObject() const { return m_parentObject.data(); }

    inline Type type() const { return static_cast<Type>(m_parentObject.combinedFlag()); }

    template<typename T>
    T *parent() const
    {
        return T::template getParentHelper<T>(this);
    }

    template<typename T>
    static T *getParentHelper(const Object *child)
    {
        return static_cast<T *>(child->parentObject());
    }

#ifdef QUL_PROPERTY_DEBUG
    inline void setObjectName(const char *n) { debugName = n; }
    const char *debugName = "";
#else
    inline void setObjectName(const char *) {}
#endif

private:
    Qul::Private::FlagPointer<Object> m_parentObject;
};

namespace Private {
class PrivateObject : public Qul::Object
{};
} // namespace Private

} // namespace Qul
