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
#include <qul/property.h>
#include <qul/private/flagpointer.h>

#include <platform/mem.h>

namespace Qul {
namespace Private {

/// Pointer that holds an area with extra properties.
/// The code generator is smart enough to assign an object allocated within
/// the same sub object if it detects that these properties are used.
/// Otherwise, if no pre-allocated entry is set, the data will be allocated
/// on the heap when calling the data() function.
///
/// There is also the `ExtendsBase` marker for for another mechanism to save memory
/// for less-used properties.
/// The differences are that:
//  - ExtraDataPointer has one pointer overhead even if it is not used (the ExtraDataPointer itself)
//  - ExtraDataPointer has usually one additional pointer overhead when used (the `self`)
//  - ExtendsBase can only improve things in the most derived C++ class.
template<typename Extra>
struct ExtraDataPointer
{
    ExtraDataPointer() {}

    template<typename X>
    Extra *data(X *item)
    {
        if (m_data.isNull()) {
            Extra *extra = qul_new<Extra>(item);
            m_data = extra;
            m_data.setFlag();
        }
        return m_data.data();
    }

    ~ExtraDataPointer()
    {
        if (m_data.flag())
            qul_delete(m_data.data());
    }

    void setPreAllocated(Extra *data)
    {
        assert(!m_data.flag()); // Already allocated on the heap?
        m_data = data;
    }

    bool isNull() const { return m_data.isNull(); }

    operator Extra *() const { return m_data.data(); }
    Extra *operator->() { return m_data.data(); }
    const Extra *operator->() const { return m_data.data(); }

private:
    ExtraDataPointer<Extra> &operator=(const ExtraDataPointer<Extra> &other);
    ExtraDataPointer(const ExtraDataPointer<Extra> &other);
    FlagPointer<Extra> m_data;
};

#define QUL_DECLARE_EXTRASTORAGE(TYPE, PROPERTY) \
    template<typename Base, typename = void> \
    struct ExtraStorage_##PROPERTY \
    { \
        ExtraStorage_##PROPERTY(Base *item) \
            : PROPERTY##_storage(item) \
        { \
            item->PROPERTY.setPreAllocated(&PROPERTY##_storage); \
        } \
        TYPE PROPERTY##_storage; \
        typedef void Has_##PROPERTY##_storage; \
    }; \
    template<typename Base> \
    struct ExtraStorage_##PROPERTY<Base, typename Base::Has_##PROPERTY##_storage> \
    { \
        ExtraStorage_##PROPERTY(Base *) {} \
    }; \
    ExtraDataPointer<TYPE> PROPERTY

} // namespace Private
} // namespace Qul
