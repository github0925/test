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
#ifndef QUL_FONTENGINESTATIC_H
#define QUL_FONTENGINESTATIC_H

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

#include <qul/private/fontengine.h>

namespace Qul {
namespace Private {

// The fontcompiler will use either 8-bit or 16-bit integer
template<typename width_T, typename height_T, typename x_T, typename y_T, typename advance_T>
struct QUL_PACKED_HINT Glyph
{
    width_T width;
    height_T height;
    x_T x;
    y_T y;
    advance_T advance;
    quint32 margins;
    const uchar *alphaMap;
};

using BigGlyph
    = Glyph<Qul::Private::quint16, Qul::Private::quint16, Qul::Private::qint16, Qul::Private::qint16, Qul::Private::qint16>;

// This function is meant to be passed to StaticFontEngineData::findGlyphImpl with the proper template parameters
template<typename GlyphsArray_T>
Qul::Private::BigGlyph findGlyph(const void *glyphs, unsigned int numGlyphs, Qul::Private::glyph_t g, bool *ok)
{
    if (!g || --g >= numGlyphs) {
        if (ok) {
            *ok = false;
        }
        return {};
    }

    const auto *typedGlyphs = reinterpret_cast<const GlyphsArray_T *>(glyphs);
    const auto &glyph = typedGlyphs[g];
    if (ok) {
        *ok = true;
    }
    return {glyph.width, glyph.height, glyph.x, glyph.y, glyph.advance, glyph.margins, glyph.alphaMap};
}

template<>
Qul::Private::BigGlyph findGlyph<std::nullptr_t>(const void *, unsigned int, Qul::Private::glyph_t, bool *ok);

struct StaticFontEngineData
{
    inline BigGlyph findGlyph(glyph_t glyph, bool *ok) const { return findGlyphImpl(glyphs, numGlyphs, glyph, ok); }

    uint numGlyphs;

    struct CmapEntry
    {
        quint32 ucs4;
        glyph_t glyph;
    };

    BigGlyph (*findGlyphImpl)(const void *glyphs, unsigned int numGlyphs, Qul::Private::glyph_t g, bool *ok);
    const void *glyphs;

    int numKerningPairs;
    const uint *kerningPairs;
    const qint8 *kerningPairAdjusts;

    const CmapEntry *cmap;

    int ascent;  // fixed point
    int descent; // fixed point
    int leading; // fixed point
};

class FontEngineStatic : public FontEngine
{
public:
    FontEngineStatic(const StaticFontEngineData *data)
        : m_data(data)
    {}

    AlphaMap alphaMapForGlyph(glyph_t g, void **) const QUL_DECL_OVERRIDE;
    glyph_t glyphIndex(uint ucs4) const QUL_DECL_OVERRIDE;

    glyph_metrics_t boundingBox(glyph_t g) const QUL_DECL_OVERRIDE;
    glyph_metrics_t boundingBox(String text) const QUL_DECL_OVERRIDE;

    FixedNumber advanceForGlyph(glyph_t g) const QUL_DECL_OVERRIDE;

    FixedNumber ascent() const QUL_DECL_OVERRIDE { return FixedNumber::fromFixed(m_data->ascent); }
    FixedNumber descent() const QUL_DECL_OVERRIDE { return FixedNumber::fromFixed(m_data->descent); }
    FixedNumber leading() const QUL_DECL_OVERRIDE { return FixedNumber::fromFixed(m_data->leading); }

    FixedNumber kerning(glyph_t, glyph_t) const QUL_DECL_OVERRIDE;

private:
    const StaticFontEngineData *m_data;
};

} // namespace Private
} // namespace Qul

#endif
