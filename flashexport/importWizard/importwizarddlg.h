/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 10-08-2011
 * Description : a kipi plugin to export images to flash
 *
 * Copyright (C) 2011      by Veaceslav Munteanu <slavuttici at gmail dot com>
 * Copyright (C) 2009-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMPORTWIZARDDLG_H
#define IMPORTWIZARDDLG_H

// Qt includes

#include <QString>
#include <QWidget>
#include <QUrl>

// Local includes

#include "kptooldialog.h"
#include "flashmanager.h"
#include "simpleviewer.h"

namespace KIPI
{
    class Interface;
}

using namespace KIPI;
using namespace KIPIPlugins;

namespace KIPIFlashExportPlugin
{

class SimpleViewer;

class ImportWizardDlg: public KPWizardDialog
{
    Q_OBJECT

public:

    explicit ImportWizardDlg(FlashManager* const mngr, QWidget* const parent = 0);
    ~ImportWizardDlg();

    FlashManager* manager() const;
    bool checkIfPluginInstalled() const;

    // Read settings from configuration file
    void readSettings();
    //Write settings to configuration file and pass them for processing.
    void saveSettings();

    bool checkIfFolderExist();

    virtual bool validateCurrentPage();
    virtual int nextId() const;

private Q_SLOTS:

    void slotCurrentIdChanged(int);

    //will activate the next button if a url was selected.
    void slotActivate();

    //Allow access to Finish only after processing is done
    void slotFinishEnable();

private:

    class Private;
    Private* const d;
};

} // namespace KIPIFlashExportPlugin

#endif // IMPORTWIZARDDLG_H
