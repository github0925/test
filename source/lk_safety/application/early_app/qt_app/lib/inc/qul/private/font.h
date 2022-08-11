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

#include <qul/private/flagpointer.h>
#include <qul/private/propertybinding.h>
#include <qul/private/unicodestring.h>
#include <qul/private/dirtylist.h>

#include <qul/property.h>

namespace Qul {
namespace Private {

struct FontConfig;
class FontEngine;

struct FontPointer
{
    /*
     * T1 - Can be the static font engine or the Spark font engine when
     *      font.* bindings are constants.
     *
     * T2 - FontConfig is used for handling non-const font.* bindings
     *      and to update the Spark font engines state based on FontConfig
     *      property values.
     */
    using Target = Private::BiPointer<const FontEngine, FontConfig>;

    FontPointer(Target target = Target())
        : target(target)
    {}

    Target target;
};

// Every Property<FontPointer> is dependent on the values inside the FontConfig
// instance. This list node allows the Property<FontPointer> that use a particular
// FontConfig to be stored in FontConfig::deps.
struct FontConfigDependentProperty
{
    SinglyLinkedListNode node;
};

template<>
struct PropertyTraits<FontPointer>
{
    static void valueWasSetHook(PropertyBase &p, FontPointer &value);
    static void valueWasUnsetHook(PropertyBase &p, FontPointer &value);
    static void valueEquality(const FontPointer &lhs, const FontPointer &rhs, bool *markDirty, bool *assignValue);

    using ExtraBase = FontConfigDependentProperty;

    // Since the hooks do something, avoid calling when not necessary.
    static constexpr bool checkValueEqualityOnDirtyBinding = true;
};

struct FontConfig
{
    // Note: These properties must not be read directly, but instead the respective
    // getter functions must be used. Some of the getters currently are unused
    // but they will be needed for UL-3459.
    Property<bool> bold;
    Property<Qul::Private::String> family;
    Property<bool> italic;
    Property<int> pixelSize;
    Property<int> pointSize;
    Property<int> weight;

#ifndef NDEBUG
    const char *sourceLocation = nullptr;
#endif

    FontConfig();

    struct FontChangedEvent
    {
        void operator()(FontConfig *font)
        {
            font->deps.iterate([](FontConfigDependentProperty *node) {
                auto property = static_cast<Property<FontPointer> *>(node);
                property->markDependencyDirty();
            });
        }
    };

    bool getItalic() const;
    bool getBold() const;
    int getWeight() const;
    int getPixelSize() const;
    int getPointSize() const;
    Qul::Private::String getFamily() const;

    DirtyEventStaticDependency<FontChangedEvent> onFontChanged;

    using PropertyNodeList = SinglyLinkedList<FontConfigDependentProperty, &FontConfigDependentProperty::node>;
    PropertyNodeList deps;
};

const FontEngine *engineFromFontPointer(const FontPointer &font);

} // namespace Private
} // namespace Qul

// When the static engine is selected, the fontcompiler emits an empty implementation
// for this function. When the Spark font engine is selected, the implementation
// is provided by the Monotype integration library. We do not provide this function
// via a header file (which could be conditonally included by the fontcopiler)
// because it uses Spark APIs, and that would require shipping Spark headers in
// the evaluation packages.
Qul::Private::FontEngine *globalSparkEngineFromConfig(const Qul::Private::FontConfig *);
