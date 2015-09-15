/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 04.10.2009
 * Description : A tool for importing images via KIO
 *
 * Copyright (C) 2009      by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2011-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "KioImportWindow.h"

// Qt includes

#include <QAction>
#include <QMenu>

// KDE includes

#include <khelpmenu.h>
#include <kio/copyjob.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>

// Libkipi includes

#include <KIPI/Interface>
#include <KIPI/UploadWidget>
#include <KIPI/ImageCollection>

// Local includes

#include "kipiplugins_debug.h"
#include "KioImportWidget.h"
#include "kpaboutdata.h"
#include "kpimageslist.h"

namespace KIPIRemoteStoragePlugin
{

KioImportWindow::KioImportWindow(QWidget* const /*parent*/)
    : KPToolDialog(0)
{
    m_importWidget = new KioImportWidget(this, iface());
    setMainWidget(m_importWidget);

    // window setup
    setWindowTitle(i18n("Import from Remote Storage"));
    setModal(false);
    startButton()->setEnabled(false);

    startButton()->setText(i18n("Start import"));
    startButton()->setToolTip(i18n("Start importing the specified images "
                                   "into the currently selected album."));

    // connections

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotImport()));

    connect(m_importWidget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotSourceAndTargetUpdated()));

    connect(m_importWidget->uploadWidget(), SIGNAL(selectionChanged),
            this, SLOT(slotSourceAndTargetUpdated()));

    // about data and help button

    KPAboutData* const about = new KPAboutData(ki18n("Import from remote storage"),
                             0,
                             KAboutLicense::GPL,
                             ki18n("A Kipi plugin to import images over network using KIO-Slave"),
                             ki18n("(c) 2009, Johannes Wienke"));

    about->addAuthor(ki18n("Johannes Wienke").toString(),
                     ki18n("Developer and maintainer").toString(),
                     QStringLiteral("languitar at semipol dot de"));

    about->setHandbookEntry(QStringLiteral("kioexport"));
    setAboutData(about);

    slotSourceAndTargetUpdated();
}

KioImportWindow::~KioImportWindow()
{
}

void KioImportWindow::slotImport()
{
    qCDebug(KIPIPLUGINS_LOG) << "starting to import urls: " << m_importWidget->sourceUrls();

    // start copying and react on signals
    setEnabled(false);
    KIO::CopyJob* const copyJob = KIO::copy(m_importWidget->imagesList()->imageUrls(),
                                  m_importWidget->uploadWidget()->selectedImageCollection().uploadUrl());

    connect(copyJob, SIGNAL(copyingDone(KIO::Job*,QUrl,QUrl,time_t,bool,bool)),
            this, SLOT(slotCopyingDone(KIO::Job*,QUrl,QUrl,time_t,bool,bool)));

    connect(copyJob, SIGNAL(result(KJob*)),
            this, SLOT(slotCopyingFinished(KJob*)));
}

void KioImportWindow::slotCopyingDone(KIO::Job* job, const QUrl& from,
                                      const QUrl& to, time_t mtime, bool directory, bool renamed)
{
    Q_UNUSED(job);
    Q_UNUSED(to);
    Q_UNUSED(mtime);
    Q_UNUSED(directory);
    Q_UNUSED(renamed);

    qCDebug(KIPIPLUGINS_LOG) << "copied " << to.toDisplayString();

    m_importWidget->imagesList()->removeItemByUrl(from);
}

void KioImportWindow::slotCopyingFinished(KJob* job)
{
    Q_UNUSED(job);

    setEnabled(true);

    if (!m_importWidget->imagesList()->imageUrls().empty())
    {
        KMessageBox::sorry(this, i18n(
                           "Some of the images have not been transferred "
                           "and are still in the list. "
                           "You can retry to import these images now."), i18n(
                           "Import not completed"));
    }
}

void KioImportWindow::slotSourceAndTargetUpdated()
{
    bool hasUrlToImport = !m_importWidget->sourceUrls().empty();
    bool hasTarget      = m_importWidget->uploadWidget()->selectedImageCollection().uploadUrl().isValid();

    qCDebug(KIPIPLUGINS_LOG) << "switching import button activity with: hasUrlToImport = "
                  << hasUrlToImport << ", hasTarget = " << hasTarget;

    startButton()->setEnabled(hasUrlToImport && hasTarget);
}

} // namespace KIPIRemoteStoragePlugin
