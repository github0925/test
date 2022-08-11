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
#ifndef QUL_FONTENGINESPARK_H
#define QUL_FONTENGINESPARK_H

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

struct SparkState
{
public:
    SparkState(const unsigned char *fontFile,
               const int fontFileSize,
               const int heapSize,
               const int cacheSize,
               const unsigned char *cacheData = NULL,
               const int cacheDataSize = -1,
               void *sparkState = NULL);

    ~SparkState();

    void *state();

    bool isFontmap();
    void selectLanguage();

    static const char *errorString(quint32 code);
    static void printError(const char *msg, quint32 error);
    // In future this method should be called from UL-2596 API,
    // or something equivalent.
    void exit();

private:
    const unsigned char *m_fontFile;
    const int m_fontFileSize;
    const int m_heapSize;
    const int m_cacheSize;
    const unsigned char *m_cacheData;
    const int m_cacheDataSize;
    void *m_sparkState;
    bool m_initialized;
};

class FontEngineSpark : public FontEngine
{
public:
    explicit FontEngineSpark(SparkState *qulSpark);
    FontEngineSpark(SparkState *qulSpark, uint16_t pixelSize, uint16_t fontClassNameIndex);

    FixedNumber kerning(glyph_t first, glyph_t second) const QUL_DECL_OVERRIDE;

    AlphaMap alphaMapForGlyph(glyph_t glyph, void **nativeData) const QUL_DECL_OVERRIDE;
    glyph_t glyphIndex(uint ucs4) const QUL_DECL_OVERRIDE;

    glyph_metrics_t boundingBox(glyph_t glyph) const QUL_DECL_OVERRIDE;
    glyph_metrics_t boundingBox(String text) const QUL_DECL_OVERRIDE;

    FixedNumber advanceForGlyph(glyph_t glyph) const QUL_DECL_OVERRIDE;
    FixedNumber ascent() const QUL_DECL_OVERRIDE;
    FixedNumber descent() const QUL_DECL_OVERRIDE;
    FixedNumber leading() const QUL_DECL_OVERRIDE;

    bool needsSyncAfterBlend() const QUL_DECL_OVERRIDE { return true; }
    void freeNativeData(void *nativeData) const QUL_DECL_OVERRIDE;

    static void printError(const char *msg, quint32 error);

    // To save the memory, it is possible to create one instance of this engine
    // and use these setters whenever different configuration is required.
    // Alternatively, you can create dedicated font engine instances and treat
    // the configuration that was passed via contructor as immutable. We could
    // provide a check here that enforces immutability.
    void setPixelSize(uint16_t pixelSize) { m_pixelSize = pixelSize; }
    uint16_t pixelSize() const { return m_pixelSize; }
    void setFontClassNameIndex(uint16_t index) { m_fontClassNameIndex = index; }
    const char *fontClassName() const;

    SparkState *sparkState() const { return m_s; }

    // Arbitrarily chosen max limit. We need to set a limit
    // at compile time because we use this value for a size of
    // an array that is created on the stack. 100 characters
    // should be more than enought for a font class name.
    static constexpr uint8_t FontClassNameMaxSize = 100;

protected:
    void updateState(glyph_t glyph) const;
    void updateState() const;

    void selectPixelSize() const;

    void selectFontClass() const;
    void selectFont(glyph_t glyph) const;

private:
    SparkState *m_s = nullptr;
    uint16_t m_pixelSize = 20;
    // Unused when MI_FONTMAP not defined.
    // Revisit after enabling users to rebuild
    // Spark from engineering packages (UL-2625).
    uint16_t m_fontClassNameIndex = 0;
};

// Used for cache priming needs. Add MI_PREPOPULATE_CACHE
// ifdef after UL-2625.
extern const unsigned char *qul_static_text_data;

} // namespace Private
} // namespace Qul

#endif
