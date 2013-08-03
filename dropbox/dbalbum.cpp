/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2008-12-28
 * Description : a kipi plugin to export images to Dropbox web service
 *
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

#include "dbalbum.moc"

//Qt includes
#include <QFormLayout>

//kde includes
#include "klocale.h"
#include "kdialog.h"
#include "klineedit.h"
#include "kdebug.h"

//local includes
#include "dbitem.h"

namespace KIPIDropboxPlugin{

DBNewAlbum::DBNewAlbum(QWidget* const parent) : KDialog(parent){
    QString header(i18n("Dropbox New Folder"));
    setWindowTitle(header);
    setButtons(Ok|Cancel);
    setDefaultButton(Cancel);
    setModal(false);

    QWidget* const mainWidget = new QWidget(this);
    setMainWidget(mainWidget);
    mainWidget->setMinimumSize(300,0);

    //---------------------------------------------------------

    m_titleEdt = new KLineEdit;
    m_titleEdt->setWhatsThis("Name of folder that will be created");

    QFormLayout* albumBoxLayout = new QFormLayout;
    albumBoxLayout->addRow(i18nc("album edit","Title:"),m_titleEdt);
    albumBoxLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    albumBoxLayout->setSpacing(KDialog::spacingHint());
    albumBoxLayout->setMargin(KDialog::spacingHint());
    mainWidget->setLayout(albumBoxLayout);
}

DBNewAlbum::~DBNewAlbum(){

}

void DBNewAlbum::getFolderTitle(DBFolder& folder){
    folder.title = QString("/") + m_titleEdt->text();
    kDebug() << "getFolderTitle " << folder.title;
}
}
