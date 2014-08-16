/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2014-07-19
 * @brief  Re-implemented class to get settings widget from DNGConverter 
 *
 * @author Copyright (C) 2008-2012 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Shourya Singh Gupta
 *         <a href="mailto:shouryasgupta at gmail dot com">shouryasgupta at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PLUGIN_GETWIDGET_H
#define PLUGIN_GETWIDGET_H

// Qt includes

#include <QtGui/QWidget>

//kde includes

#include <kurl.h>

// libkipi includes

#include "libkipi/plugsettings.h"

using namespace KIPI;

namespace KIPIDNGConverterPlugin
{
class SettingsWidget;
  
class Plugin_GetWidget: public PlugSettings
{ 
    Q_OBJECT
  
public:

    Plugin_GetWidget(Interface* const interface);
    ~Plugin_GetWidget();

    bool getWidget();
    static Plugin_GetWidget* instance();
    static SettingsWidget*   settingsInstance();
    
public Q_SLOTS:
    void slotIdentify();

private:
    static Plugin_GetWidget* m_instance;
    static SettingsWidget*   settings;
    class Private;
    Private* const d;
};

} // namespace KIPIDNGConverterPlugin

#endif
