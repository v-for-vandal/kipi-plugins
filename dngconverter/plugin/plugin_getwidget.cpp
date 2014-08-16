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

#include "plugin_getwidget.h"

#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QTreeWidgetItemIterator>

//kde includes

#include <kurl.h>

// libkipi includes

#include "libkipi/interface.h"

//local includes

#include "settingswidget.h"

namespace KIPIDNGConverterPlugin
{
Plugin_GetWidget* Plugin_GetWidget::m_instance = 0;
SettingsWidget* Plugin_GetWidget::settings = 0;

class Plugin_GetWidget::Private
{
public:

    Private():
    interface(0)
    {
    }
    
    Interface* interface;
};

Plugin_GetWidget::Plugin_GetWidget(Interface* const iface)
    : PlugSettings(),d(new Private)
{
    m_instance   = this;
    d->interface = iface;
    settings  = new SettingsWidget(0);
    connect(settings, SIGNAL(buttonChanged(int)),
            this, SLOT(slotIdentify()));
}

Plugin_GetWidget::~Plugin_GetWidget()
{
    delete d;
}

Plugin_GetWidget* Plugin_GetWidget::instance()
{
    return m_instance;
}

SettingsWidget* Plugin_GetWidget::settingsInstance()
{
    return settings;
}

bool Plugin_GetWidget::getWidget()
{
    PlugSettings* set = d->interface->setWidget();
    if(set)
    { 
        set->setWidget(settings);
        slotIdentify();
	return true;
    }
    return false;
    
}

void Plugin_GetWidget::slotIdentify()
{
    KUrl::List urlList = d->interface->getImgURLs();

    for (KUrl::List::const_iterator  it = urlList.constBegin(); it != urlList.constEnd(); ++it)
    {
        QFileInfo fi((*it).path());

        if(settings->conflictRule() == SettingsWidget::OVERWRITE)
        {
            QString dest                    = fi.completeBaseName() + QString(".dng");
            d->interface->setDestFileName(dest, *it);
        }

        else
        {
            QString dest = fi.absolutePath() + QString("/") + fi.completeBaseName() + QString(".dng");
            QFileInfo a(dest);
            bool fileNotFound = (a.exists());

            if (!fileNotFound)
            {
                dest = fi.completeBaseName() + QString(".dng");
            }

            else
            {
                int i = 0;
                while(fileNotFound)
                {
                    a = QFileInfo(dest);

                    if (!a.exists())
                    {
                        fileNotFound = false;
                    }
                    else
                    {
                        i++;
                        dest = fi.absolutePath() + QString("/") + fi.completeBaseName() + QString("_") + QString::number(i) + QString(".dng");
                    }
                }

                dest = fi.completeBaseName() + QString("_") + QString::number(i) + QString(".dng");
            }

            d->interface->setDestFileName(dest, *it);
        }
    }  
}

} // namespace KIPIDNGConverterPlugin
