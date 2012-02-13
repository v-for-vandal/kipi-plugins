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

#include "plugin_imgurexport.moc"

// C++ includes

#include <unistd.h>

// KDE includes

#include <KDebug>
#include <KConfig>
#include <KApplication>
#include <KAction>
#include <KActionCollection>
#include <KGenericFactory>
#include <KLibLoader>
#include <KStandardDirs>
#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KDialog>

// Local includes

#include "imgurtalker.h"
#include "imageslist.h"
#include "progresswidget.h"
//#include "imgurwidget.h"

using namespace KIPIImgurExportPlugin;
using namespace KIPIPlugins;

K_PLUGIN_FACTORY( ImgurExportFactory, registerPlugin<Plugin_ImgurExport>(); )
K_EXPORT_PLUGIN ( ImgurExportFactory("kipiplugin_imgurexport") )

Plugin_ImgurExport::Plugin_ImgurExport(QObject* parent, const QVariantList& args)
    : KIPI::Plugin(ImgurExportFactory::componentData(), parent, "ImgurExport")
{
    kDebug(AREA_CODE_LOADING) << "ImgurExport plugin loaded";
    kDebug(AREA_CODE_LOADING) << args;
}

Plugin_ImgurExport::~Plugin_ImgurExport()
{
}

void Plugin_ImgurExport::setup(QWidget* widget)
{
    KIPI::Plugin::setup(widget);

    KIconLoader::global()->addAppDir("kipiplugin_imgurexport");

    m_actionExport = actionCollection()->addAction("ImgurExport");
    m_actionExport->setText(i18n("Export to &Imgur..."));
    m_actionExport->setIcon(KIcon("imgur"));
    m_actionExport->setShortcut(KShortcut(Qt::ALT+Qt::SHIFT+Qt::Key_I));

    connect(m_actionExport, SIGNAL(triggered(bool)),
            this, SLOT(slotActivate()));

    addAction(m_actionExport);

    KIPI::Interface* interface = dynamic_cast<KIPI::Interface*>(parent());

    if (!interface)
    {
        kError() << "Kipi interface is null!" ;
        m_actionExport->setEnabled(false);
        return;
    }

    m_actionExport->setEnabled(true);
}

void Plugin_ImgurExport::slotActivate()
{
    KIPI::Interface* interface = dynamic_cast<KIPI::Interface*>(parent());
    if (!interface)
    {
        kError() << "Kipi interface is null!" ;
        return;
    }

    ImagesList* m_dlgExport = new ImagesList(interface);
    m_dlgExport->setWindowIcon(KIcon("imgur"));
    m_dlgExport->setWindowTitle(i18n("Export to the imgur.com web service"));

    // i need to connect image list process slot to the imgur talker

//    m_dlgExport->addActions();
//    ProgressWidget *p = new ProgressWidget(interface);
//    p->show();

    m_dlgExport->loadImagesFromCurrentSelection();
    m_dlgExport->setAllowDuplicate(false);
    m_dlgExport->setAllowRAW(false);

    m_dlgExport->show();

    ImgurTalker *ImgurWebService = new ImgurTalker(interface, m_dlgExport);

    connect(ImgurWebService, SIGNAL(signalUploadStart(KUrl)),
            m_dlgExport, SLOT(processing(KUrl)));

    connect(ImgurWebService, SIGNAL(signalUploadDone(KUrl, bool)),
            m_dlgExport, SLOT(processed(KUrl, bool)));

    connect(ImgurWebService, SIGNAL(signalUploadProgress(int)),
            m_dlgExport, SLOT(slotProgressTimerDone()));

    connect(m_dlgExport, SIGNAL(signalAddItems(KUrl::List)),
            ImgurWebService, SLOT(slotAddItems(KUrl::List)));

    ImgurWebService->startUpload();
    kDebug() << "We have activated the imgur exporter!";
}

KIPI::Category Plugin_ImgurExport::category( KAction* action ) const
{
    kDebug() << action;
    kWarning() << "Unrecognized action for plugin category identification";
    return KIPI::ExportPlugin;
}
