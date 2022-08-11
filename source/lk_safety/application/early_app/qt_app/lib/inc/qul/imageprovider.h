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
#include <qul/image.h>
#include <qul/private/unicodestring.h>
#include <qul/private/qulrcc_lookup.h>
#include <qul/property.h> // TODO: it shouldn't be necessary to include whole property.h just to get Qul::Private::LinkedList...

namespace Qul {

struct ImageProvider;

namespace Private {

Qul::SharedImage dispatchToImageProvider(const char *providerId,
                                         size_t providerIdLength,
                                         const char *uri,
                                         size_t uriLength);

// FIXME: Application::addImageProvider overloads should be replaced with this one, after Qul::Private::String will become public.
void addImageProvider(const Qul::Private::String &providerId, Qul::ImageProvider *provider);

} // namespace Private

struct ImageProvider : public Qul::Private::LinkedListNode<Qul::ImageProvider>
{
    ImageProvider() = default;
    virtual ~ImageProvider() = default;

    virtual Qul::SharedImage requestImage(const char *uri, size_t uriLength) = 0;

private:
    Qul::Private::String id; // we don't want to allocate in the list below.
    static Qul::Private::LinkedList<Qul::ImageProvider> imageProviders;
    friend void Qul::Private::addImageProvider(const Qul::Private::String &, Qul::ImageProvider *);
    friend Qul::SharedImage Qul::Private::findImageForString(const Qul::Private::String &);
    friend Qul::SharedImage Qul::Private::dispatchToImageProvider(const char *, size_t, const char *, size_t);
};

} // namespace Qul
