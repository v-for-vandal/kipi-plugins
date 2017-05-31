/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2012-02-06
 * Description : QML extension plugin for KIPI library
 *
 * Copyright (C) 2017-2017 by Artem Serebriyskiy <v.for.vandal@gmail.com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef KPQMLPLUGIN_H
#define KPQMLPLUGIN_H

// Qt includes

#include <QUrl>
#include <QQmlExtensionPlugin>

// KIPI includes
#include <KIPI/Interface>

// Local includes

#include "kipiplugins_export.h"

namespace KIPIPlugins
{

class KIPIPLUGINS_EXPORT KPQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    virtual void registerTypes(const char* uri) override;
    virtual void initializeEngine(QQmlEngine* engine, const char* uri) override;

};

} // namespace KIPIPlugins

#endif  // KPIMAGEINFO_H
