/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP workflow audiospecs settings page.
 *
 * Copyright (C) 2007-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *               2013 GSoC: Mahesh Hegde <maheshmhegade at gmail dot.com>
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

#include "xmpaudiospecs.moc"

// Qt includes

#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>

// KDE includes

#include <kcombobox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kseparator.h>
#include <ktextedit.h>
#include <kdebug.h>
#include <kconfiggroup.h>

// LibKDcraw includes

#include <libkdcraw/squeezedcombobox.h>

// Local includes

#include "altlangstringedit.h"
#include "metadatacheckbox.h"
#include "multivaluesedit.h"
#include "objectattributesedit.h"
#include "kpversion.h"
#include "kpmetadata.h"

using namespace KIPIPlugins;
using namespace KDcrawIface;

namespace KIPIMetadataEditPlugin
{

class XMPAudiospecs::XMPAudiospecsPriv
{
public:

    XMPAudiospecsPriv()
    {
        codecCB             = 0;
        compressorCB        = 0;
    }
    QCheckBox*       codecCheck;
    QCheckBox*       compressorCheck;
    
    KComboBox*       compressorCB;
    KComboBox*       codecCB;
};

XMPAudiospecs::XMPAudiospecs(QWidget* const parent)
    : QWidget(parent), d(new XMPAudiospecsPriv)
{
    QGridLayout* grid = new QGridLayout(this);
 
    d->codecCheck = new QCheckBox(i18n("Codec:"));
    d->codecCB = new KComboBox(this);
    d->compressorCheck = new QCheckBox(i18n("Compressor:"));
    d->compressorCB = new KComboBox(this);
    
    grid->addWidget(d->codecCheck,                        1, 0, 1, 1);
    grid->addWidget(d->codecCB,                           1, 1, 1, 2);
    grid->addWidget(d->compressorCheck,                   2, 0, 1, 1);
    grid->addWidget(d->compressorCB,                      2, 1, 1, 2);
    
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(2, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());
}

XMPAudiospecs::~XMPAudiospecs()
{
    delete d;
}

void XMPAudiospecs::readMetadata(QByteArray& xmpData)
{
}

void XMPAudiospecs::applyMetadata(QByteArray& xmpData)
{
}

}  // namespace KIPIMetadataEditPlugin
