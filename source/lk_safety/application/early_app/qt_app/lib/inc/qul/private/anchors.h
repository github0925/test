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

#include <qul/private/propertybinding.h>

namespace Qul {
namespace Private {

// A Binding that forwards to the original binding or original value if the resulting value is NaN
template<typename Functor>
struct AnchorsBinding : BindingBase, Functor
{
    typedef int PropertyType;

    /// Construct. Call to init() is required.
    explicit AnchorsBinding(Functor f)
        : BindingBase(NULL)
        , Functor(f)
        , originalValue()
    {}

    template<typename Object, AnchorsBinding Object::*Member>
    void init()
    {
        call = &callFunction<Object, Member>;
    }

    ScopedPointer<DynDependencyNode> dynDependencies;

private:
    // The flag specify if the original value (or binding) is used
    // (that's hapenning if the actual anchor functor returned NaN)
    // The flag2 is set if the originalValue was explicitly set
    FlagPointer<BindingBase> originalBinding;
    PropertyType originalValue;

    // This helper function exists for code-size reasons.
    bool commonCallFunctionNoExec(InternalCall call, Property<PropertyType> *prop, void *v)
    {
        switch (call) {
        case Install: {
            originalBinding.setFlag2Value(prop->wasExplicitlySet());
            BindingBase *oldBinding = prop->binding();
            if (oldBinding)
                oldBinding->call(oldBinding, BindingBase::GetExplicitBinding, prop, &oldBinding);
            originalBinding = oldBinding;
            if (!originalBinding)
                originalValue = prop->m_value;
            return false;
        }
        case SetBinding:
            if (originalBinding && originalBinding->call(originalBinding.data(), BindingBase::SetBinding, prop, v))
                return true;
            originalBinding = reinterpret_cast<BindingBase *>(v);
            if (originalBinding.flag()) {
                prop->setDirty();
            }
            return true;
        case SetValue:
        case SetImplicitValue:
            originalBinding.setFlag2Value(call == SetValue);
            if (originalBinding && originalBinding->call(originalBinding.data(), call, prop, v))
                return true;
            originalBinding = NULL;
            originalValue = *reinterpret_cast<PropertyType *>(v);
            if (originalBinding.flag()) {
                prop->setDirty();
            }
            return true;
        case MarkDirty:
            return false;
        case GetExplicitBinding:
            if (originalBinding)
                return originalBinding->call(originalBinding.data(), GetExplicitBinding, prop, v);
            return originalBinding.flag2();
        case Execute:
            break;
        }
        QUL_UNREACHABLE();
        return true;
    }

    // This helper function exists for code-size reasons.
    template<typename Object>
    bool commonCallFunction(InternalCall call, PropertyBase *prop, void *v, size_t offset)
    {
        if (call != Execute)
            return commonCallFunctionNoExec(call, static_cast<Property<PropertyType> *>(prop), v);

        EvaluationScope scope(EvaluationFrame(prop, NULL, 0, &dynDependencies));
        Object *object = reinterpret_cast<Object *>(reinterpret_cast<char *>(this) - offset);
        qreal result = (*this)(object);
        originalBinding.setFlagValue(isnan(result));
        if (originalBinding.flag()) {
            if (originalBinding) {
                return originalBinding->call(originalBinding.data(), Execute, prop, v)
                       && !currentFrame.allocationFailure;
            }
            result = originalValue;
        }
        *reinterpret_cast<PropertyType *>(v) = PropertyType(result);
        return !currentFrame.allocationFailure;
    }

    template<typename Object, AnchorsBinding Object::*Member>
    static bool callFunction(BindingBase *self, InternalCall call, PropertyBase *prop, void *v)
    {
        AnchorsBinding *self_ = static_cast<AnchorsBinding *>(self);

        const size_t offset = offsetOf(Member);
        return self_->commonCallFunction<Object>(call, prop, v, offset);
    }
};

} // namespace Private
} // namespace Qul
