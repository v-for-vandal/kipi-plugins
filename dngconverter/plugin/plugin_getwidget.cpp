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

namespace KIPIDNGConverterPlugin
{
Plugin_GetWidget* Plugin_GetWidget::m_instance = 0;

class Plugin_GetWidget::Private
{
public:

    Private():
    interface(0)
    {
    }
    
    Interface* interface;
    QWidget* settings;
};

Plugin_GetWidget::Plugin_GetWidget(Interface* const iface, QWidget* const widget)
    : SettingsWidget(),d(new Private)
{
    m_instance   = this;
    d->interface = iface;
    d->settings  = widget;
}

Plugin_GetWidget::~Plugin_GetWidget()
{
    delete d;
}

Plugin_GetWidget* Plugin_GetWidget::instance()
{
    return m_instance;
}

void Plugin_GetWidget::getWidget()
{
    SettingsWidget* set = d->interface->setWidget();
    set->setWidget(d->settings);
}

} // namespace KIPIDNGConverterPlugin
