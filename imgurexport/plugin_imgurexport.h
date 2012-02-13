/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2010-02-04
 * Description : a tool to export images to imgur.com
 *
 * Copyright (C) 2010 by Marius Orcisk <marius at habarnam dot ro>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PLUGIN_IMGUREXPORT_H
#define PLUGIN_IMGUREXPORT_H

// Qt includes

#include <QVariant>

// LibKIPI includes

#include <libkipi/plugin.h>

namespace KIPIImgurExportPlugin
{

class Plugin_ImgurExport : public KIPI::Plugin
{
    Q_OBJECT

public:

    explicit Plugin_ImgurExport(QObject* parent, const QVariantList& args);
    ~Plugin_ImgurExport();

    void setup(QWidget*);
    KIPI::Category category(KAction* action) const;

public Q_SLOTS:

    void slotActivate();

private:

    KAction* m_actionExport;
};

} // namespace KIPIImgurExportPlugin

#endif // PLUGIN_IMGUREXPORT_H
