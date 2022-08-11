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

#include <qul/rootitem.h>

#include <qul/private/application.h>

namespace Qul {

struct ImageProvider;

class Application
{
public:
    Application() = default;
    ~Application() = default;

    void exec();

    void setRootItem(RootItem *root);
    void setUiLanguage(const char *language);
    void addImageProvider(const char *providerId, Qul::ImageProvider *provider);

private:
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    Private::Application m_app;
};

} // namespace Qul
