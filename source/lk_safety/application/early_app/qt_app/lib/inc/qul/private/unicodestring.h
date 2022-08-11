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
#ifndef QUL_UNICODE_STRING_H
#define QUL_UNICODE_STRING_H

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

#include <qul/global.h>
#include <qul/private/allocator.h>
#include <qul/private/colorstringparser.h>
#include <qul/private/global.h>
#include <qul/private/graphicsdevice.h>
#include <cstring>
#ifdef QUL_STD_STRING_SUPPORT
#include <string>
#endif
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <string.h>
#include <stdio.h>

namespace Qul {
namespace Private {

typedef IntegerForSize<sizeof(qreal)>::Signed qintrealdigits;
typedef IntegerForSize<sizeof(qreal)>::Unsigned quintrealdigits;

#ifdef QUL_STD_STRING_SUPPORT

using StdStringType = std::basic_string<char, std::char_traits<char>, Qul::Private::Allocator<char> >;

#else

class PlaceholderStdStringType
{
public:
    std::size_t size() const { return 0; }

    const char *data() const { return ""; }

private:
    // non-constructible
    PlaceholderStdStringType();
};

typedef PlaceholderStdStringType StdStringType;

#endif // QUL_STD_STRING_SUPPORT

struct Char
{
    Char(quint32 unicode = 0)
        : unicode(unicode)
    {}

    quint32 unicode;

    static QUL_DECL_CONSTEXPR inline bool requiresSurrogates(uint ucs4) QUL_DECL_NOTHROW { return (ucs4 >= 0x10000); }
    static QUL_DECL_CONSTEXPR inline bool isNonCharacter(uint ucs4) QUL_DECL_NOTHROW
    {
        return ucs4 >= 0xfdd0 && (ucs4 <= 0xfdef || (ucs4 & 0xfffe) == 0xfffe);
    }
    static QUL_DECL_CONSTEXPR inline bool isSurrogate(uint ucs4) QUL_DECL_NOTHROW { return (ucs4 - 0xd800u < 2048u); }
    static QUL_DECL_CONSTEXPR inline bool isHighSurrogate(uint ucs4) QUL_DECL_NOTHROW
    {
        return ((ucs4 & 0xfffffc00) == 0xd800);
    }
    static QUL_DECL_CONSTEXPR inline bool isLowSurrogate(uint ucs4) QUL_DECL_NOTHROW
    {
        return ((ucs4 & 0xfffffc00) == 0xdc00);
    }
    static QUL_DECL_CONSTEXPR inline uint surrogateToUcs4(ushort high, ushort low) QUL_DECL_NOTHROW
    {
        return (uint(high) << 10) + low - 0x35fdc00;
    }
    static QUL_DECL_CONSTEXPR inline ushort highSurrogate(uint ucs4) QUL_DECL_NOTHROW
    {
        return ushort((ucs4 >> 10) + 0xd7c0);
    }
    static QUL_DECL_CONSTEXPR inline ushort lowSurrogate(uint ucs4) QUL_DECL_NOTHROW
    {
        return ushort(ucs4 % 0x400 + 0xdc00);
    }
};

inline bool operator==(Char lhs, Char rhs)
{
    return lhs.unicode == rhs.unicode;
}

class Latin1String
{
public:
    inline Latin1String(const char *str)
        : m_data(str)
        , m_len(std::strlen(str))
    {}
    inline Latin1String(const char *str, int length)
        : m_data(str)
        , m_len(length)
    {}

    inline const char *data() const { return m_data; }
    inline size_t size() const { return m_len; }
    inline size_t length() const { return m_len; }

private:
    const char *m_data;
    size_t m_len;
};

class FormattedNumberString
{
public:
    static inline FormattedNumberString createFixed(qreal value, unsigned fractDigits)
    {
        return FormattedNumberString(value, fractDigits, true, false);
    }

    static inline FormattedNumberString createExponential(qreal value, unsigned fractDigits)
    {
        return FormattedNumberString(value, fractDigits, true, true);
    }

    static inline FormattedNumberString createExponential(qreal value)
    {
        return FormattedNumberString(value, 0, false, true);
    }

    static inline FormattedNumberString createDefault(qreal value)
    {
        return FormattedNumberString(value, 0, false, false);
    }

    inline FormattedNumberString(qreal value,
                                 unsigned fractDigits,
                                 bool fixedFractDigits = true,
                                 bool useExponential = false)
        : m_value(value)
        , m_fractDigits(fractDigits)
        , m_fixedFractDigits(fixedFractDigits)
        , m_useExponential(useExponential)
    {}

    inline qreal value() const { return m_value; }

    inline unsigned fractDigits() const { return m_fractDigits; }

    inline bool fixedFractDigits() const { return m_fixedFractDigits; }

    inline bool useExponential() const { return m_useExponential; }

private:
    qreal m_value;
    unsigned m_fractDigits;
    bool m_fixedFractDigits;
    bool m_useExponential;
};

// String corresponding to a known ARGB color value
struct ColorIdString
{
    ColorIdString(const char *str, qsizetype strLen, quint32 colorValue)
        : m_data(str)
        , m_len(strLen)
        , m_colorValue(colorValue)
    {}

    const char *m_data; // ascii only
    qsizetype m_len;
    quint32 m_colorValue;
};

struct SharedStringPair;

struct RasterBuffer;

struct ImageIdValue
{
    static ImageIdValue create(const char *str, const RasterBuffer *buffer)
    {
        ImageIdValue value = {str, buffer};
        return value;
    }

    const char *m_utf8Data;
    const RasterBuffer *m_buffer;
};

/// The String class is a wrapper around a string. The actual string can be stored
/// as a "view" to a const char* in a specified encoding, or an integer converted to
/// string, or a std::string containing UTF-8, or a "view" to const char* with pre-parsed
/// RGBA color value, or only RGBA color value converted to a string.
/// When the format is Latin1, Utf8 or ColorId the String does not take ownership
/// of the char* and should therefore only be used for static data.
class String
{
public:
    enum Format {
        Latin1 = 0,
        Utf8 = 1,
        StdString = 2,
        FormattedNumber = 3,
        ColorId = 4,
        HexColorString = 5,
        Concatenation = 6,
        ImageId = 7
    };

    inline String()
        : m_data("")
        , m_format(Latin1)
        , m_len(0)
    {}

    // The string does not take ownership, so the data should be static
    inline String(Latin1String latin1)
        : m_data(latin1.data())
        , m_format(Latin1)
        , m_len(latin1.length())
    {}

    // The string does not take ownership, so the data should be static
    inline String(const char *utf8, int length)
        : m_data(utf8)
        , m_format(Utf8)
        , m_len(length)
    {}

    // The string does not take ownership, so the data should be static
    inline String(const char *utf8)
        : m_data(utf8)
        , m_format(Utf8)
        , m_len(static_cast<uint>(std::strlen(utf8)))
    {}

    // Will make a copy of the std::string
    inline String(const StdStringType &value)
        : m_format(StdString)
        , m_len(value.size())
    {
        new (m_stdString) StdStringType(value);
    }

#ifdef QUL_STD_STRING_SUPPORT
    inline String(const std::string &value)
        : m_format(StdString)
        , m_len(value.size())
    {
        new (m_stdString) StdStringType(value.data(), value.size());
    }
#endif

    String(const FormattedNumberString &value);

    inline String(ColorIdString value)
        : m_colorId(ColorIdValue::create(value.m_data, value.m_colorValue))
        , m_format(ColorId)
        , m_len(value.m_len)
    {}
    inline String(ImageIdValue value, int length)
        : m_imageId(value)
        , m_format(ImageId)
        , m_len(length)
    {}

    inline explicit String(PlatformInterface::Rgba32 color)
        : m_hexColorValue(color.value)
        , m_format(HexColorString)
        , m_len(color.alpha() == 255 ? 7 : 9) // #rrggbb or #rrggbbaa
    {}

    String(const String &s1, const String &s2)
        : m_concatPair(acquireSharedPair(s1, s2))
        , m_format(Concatenation)
        , m_len(s1.m_len + s2.m_len)
    {}

    ~String()
    {
        if (m_format == StdString)
            asStdString().~StdStringType();
        else if (m_format == Concatenation)
            unrefSharedPair(m_concatPair);
    }

    String(const String &other)
        : m_format(other.m_format)
        , m_len(other.m_len)
    {
        switch (other.m_format) {
        case Utf8:
        case Latin1:
            m_data = other.m_data;
            break;
        case StdString:
            new (m_stdString) StdStringType(other.asStdString());
            break;
        case FormattedNumber:
            m_formattedNumber = other.m_formattedNumber;
            break;
        case ColorId:
            m_colorId = other.m_colorId;
            break;
        case HexColorString:
            m_hexColorValue = other.m_hexColorValue;
            break;
        case Concatenation:
            m_concatPair = other.m_concatPair;
            refSharedPair(m_concatPair);
            break;
        case ImageId:
            m_imageId = other.m_imageId;
            break;
        }
    }

    String &operator=(const String &other)
    {
        if (&other == this) {
            return *this;
        }
        if (m_format == StdString) {
            asStdString().~StdStringType();
        } else if (m_format == Concatenation) {
            unrefSharedPair(m_concatPair);
        }
        m_len = other.m_len;
        m_format = other.m_format;
        switch (other.m_format) {
        case Utf8:
        case Latin1:
            m_data = other.m_data;
            break;
        case StdString:
            new (m_stdString) StdStringType(other.asStdString());
            break;
        case FormattedNumber:
            m_formattedNumber = other.m_formattedNumber;
            break;
        case ColorId:
            m_colorId = other.m_colorId;
            break;
        case HexColorString:
            m_hexColorValue = other.m_hexColorValue;
            break;
        case Concatenation:
            m_concatPair = other.m_concatPair;
            refSharedPair(m_concatPair);
            break;
        case ImageId:
            m_imageId = other.m_imageId;
            break;
        }
        return *this;
    }

    String &append(const String &rhs)
    {
        if (rhs.m_len != 0) {
            *this = *this + rhs;
        }

        return *this;
    }

    String &operator+=(const String &rhs) { return append(rhs); }

    String operator+(const String &rhs) const { return String(*this, rhs); }

#if __cplusplus >= 201103L
    inline String(StdStringType &&value)
        : m_format(StdString)
        , m_len(value.size())
    {
        new (m_stdString) StdStringType(std::move(value));
    }

    inline String(String &&other)
        : m_format(other.m_format)
        , m_len(other.m_len)
    {
        switch (other.m_format) {
        case Utf8:
        case Latin1:
            m_data = other.m_data;
            break;
        case StdString:
            new (m_stdString) StdStringType(std::move(other.asStdString()));
            break;
        case FormattedNumber:
            m_formattedNumber = other.m_formattedNumber;
            break;
        case ColorId:
            m_colorId = other.m_colorId;
            break;
        case HexColorString:
            m_hexColorValue = other.m_hexColorValue;
            break;
        case Concatenation:
            m_concatPair = other.m_concatPair;
            refSharedPair(m_concatPair);
            break;
        case ImageId:
            m_imageId = other.m_imageId;
            break;
        }
    }

    String &operator=(String &&other)
    {
        if (&other == this) {
            return *this;
        }
        if (m_format == StdString) {
            asStdString().~StdStringType();
        } else if (m_format == Concatenation) {
            unrefSharedPair(m_concatPair);
        }
        m_len = other.m_len;
        m_format = other.m_format;
        switch (other.m_format) {
        case Utf8:
        case Latin1:
            m_data = other.m_data;
            break;
        case StdString:
            new (m_stdString) StdStringType(std::move(other.asStdString()));
            break;
        case FormattedNumber:
            m_formattedNumber = other.m_formattedNumber;
            break;
        case ColorId:
            m_colorId = other.m_colorId;
            break;
        case HexColorString:
            m_hexColorValue = other.m_hexColorValue;
            break;
        case Concatenation:
            m_concatPair = other.m_concatPair;
            refSharedPair(m_concatPair);
            break;
        case ImageId:
            m_imageId = other.m_imageId;
            break;
        }
        return *this;
    }
#endif

    // Number of UCS-4 code points
    int length() const;
    inline int size() const { return length(); }

    inline bool isEmpty() const { return m_len == 0; }

    Char charAt(int index) const;
    friend bool operator==(const String &a, const String &b);
    friend bool operator!=(const String &a, const String &b) { return !(a == b); }

    // Return a UTF-8 buffer, if the String has one available already.
    // It's not necessarily null terminated. The number of bytes is rawLength().
    //
    // Note that this function is usually for optimization, to avoid going through
    // StringIteratorUtf8 when it's not necessary. Many strings (formatted numbers,
    // concatenations, Latin1, ...) do not have a pre-made UTF-8 buffer available.
    inline const char *maybeUtf8() const
    {
        switch (m_format) {
        case StdString:
            return asStdString().data();
        case Utf8:
            return m_data;
        case ColorId:
            return m_colorId.m_data;
        case ImageId:
            return m_imageId.m_utf8Data;
        default:
            return NULL;
        }
    }

    // Return a Latin-1 buffer, if available. See maybeUtf8().
    inline const char *maybeLatin1() const
    {
        switch (m_format) {
        case Latin1:
            return m_data;
        default:
            return NULL;
        }
    }

    // Format-specific length value.
    // If maybeUtf8() is not null, it will be the number of bytes in the buffer.
    // If maybeLatin1() is not null, it will be the number of bytes in the buffer.
    inline int rawLength() const { return m_len; }

    inline Format format() const { return Format(m_format); }

#ifdef QUL_STD_STRING_SUPPORT
    operator std::string() const;
#endif

    PlatformInterface::Rgba32 toRgba32() const
    {
        if (m_format == ColorId) {
            return PlatformInterface::Rgba32(m_colorId.m_colorValue);
        } else if (m_format == HexColorString) {
            return PlatformInterface::Rgba32(m_hexColorValue);
        } else {
            return parseColorString(*this);
        }
    }
    const RasterBuffer *toRasterBufferPtr() const
    {
        if (m_format != ImageId || isEmpty())
            return NULL;
        return m_imageId.m_buffer;
    }

private:
    inline const StdStringType &asStdString() const
    {
        assert(m_format == StdString);
        return *reinterpret_cast<const StdStringType *>(m_stdString);
    }
    inline StdStringType &asStdString()
    {
        assert(m_format == StdString);
        return *reinterpret_cast<StdStringType *>(m_stdString);
    }

    static SharedStringPair *acquireSharedPair(const String &str1, const String &str2);
    static void releaseSharedPair(SharedStringPair *pair);
    static void refSharedPair(SharedStringPair *pair);
    static void unrefSharedPair(SharedStringPair *pair);

    struct QUL_DECL_ALIGN(QUL_PACKED_DEFINED ? QUL_ALIGNOF(qint32) : QUL_ALIGNOF(qintrealdigits)) FormattedNumberValue
    {
        static FormattedNumberValue create(qintrealdigits intValue,
                                           quint8 pointPos,
                                           quint8 trailingZerosCount,
                                           qint8 exponent,
                                           quint8 exponentCharsCount)
        {
            FormattedNumberValue value = {intValue, pointPos, trailingZerosCount, exponent, exponentCharsCount};
            return value;
        }

        bool operator==(const FormattedNumberValue &other) const
        {
            return m_intValue == other.m_intValue && m_pointPos == other.m_pointPos
                   && m_trailingZerosCount == other.m_trailingZerosCount && m_exponent == other.m_exponent
                   && m_exponentCharsCount == other.m_exponentCharsCount;
        }

        QUL_PACKED_HINT qintrealdigits m_intValue;
        quint8 m_pointPos;
        quint8 m_trailingZerosCount;
        qint8 m_exponent;
        quint8 m_exponentCharsCount;
    };

    struct ColorIdValue
    {
        static ColorIdValue create(const char *data, quint32 colorValue)
        {
            ColorIdValue value = {data, colorValue};
            return value;
        }
        const char *m_data; // ascii only
        quint32 m_colorValue;
    };

    union {
        const char *m_data;
        QUL_DECL_ALIGN(QUL_ALIGNOF(StdStringType))
        char m_stdString[sizeof(StdStringType)]; // In C++11, it could be a StdStringType directly
        FormattedNumberValue m_formattedNumber;
        ColorIdValue m_colorId;
        quint32 m_hexColorValue;
        SharedStringPair *m_concatPair;
        ImageIdValue m_imageId;
    };
    size_t m_format : 8;
    size_t m_len : 24;

    friend class StringIterator;
};

class StringIterator
{
public:
    StringIterator(const String *str)
        : m_str(str)
        , m_pos(0)
    {}

    bool hasNext() const { return m_pos < m_str->rawLength(); }
    quint32 next();

private:
    const String *m_str;
    int m_pos;

    friend class StringIteratorUtf8;
};

class StringIteratorUtf8
{
public:
    StringIteratorUtf8(const String *str)
        : m_iter(str)
        , m_codePoint(0)
        , m_pendingContBytesCount(0)
    {}

    bool hasNext() const { return m_pendingContBytesCount != 0 || m_iter.hasNext(); }
    uchar next();

private:
    StringIterator m_iter;
    quint32 m_codePoint;
    unsigned m_pendingContBytesCount;
};

struct SharedStringPair
{
    SharedStringPair(const String &str1, const String &str2);

    const String m_str1;
    const String m_str2;
    quint32 m_useCount;
};

} // namespace Private
} // namespace Qul

#endif
