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

#include "kpqmlplugin.h"

// Libkipi includes
#include <KIPI/PluginLoader>

// local includes
#include "kpquickinit.h"

using namespace KIPI;
using namespace KIPIPlugins;

void KPQmlPlugin::registerTypes(const char* uri)
{
    InitKIPIQuick();
}

void KPQmlPlugin::initializeEngine(QQmlEngine* engine, const char* uri)
{
    if( PluginLoader::interface() != 0 ) {
        InitKIPIQmlEngine(engine, PluginLoader::interface());
    }
}
