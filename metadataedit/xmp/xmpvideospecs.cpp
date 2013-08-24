/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP workflow videospecs settings page.
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

#include "xmpvideospecs.moc"

// Qt includes

#include <QCheckBox>
#include <QPushButton>

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

// libkexiv2 includes
#include <libkexiv2/standardvideotags.h>

// Local includes

#include "altlangstringedit.h"
#include "metadatacheckbox.h"
#include "multivaluesedit.h"
#include "objectattributesedit.h"
#include "kpversion.h"
#include "kpmetadata.h"

using namespace KIPIPlugins;

namespace KIPIMetadataEditPlugin
{

class XMPVideospecs::XMPVideospecsPriv
{
public:

    XMPVideospecsPriv()
    {
        frameRateLE         = 0;
        codecLE             = 0;
         
        //FileType specific data
        qTimeLangCB         = 0;

        lastPosition        = 0;
    }
    
    QCheckBox*       frameRateCheck;
    QCheckBox*       codecCheck;
    QCheckBox*       qTimeLangCheck;
    QCheckBox*       compressorCheck;
    QCheckBox*       heightCheck;
    QCheckBox*       widthCheck;
    
    KComboBox*       qTimeLangCB;
    
    KLineEdit*       frameRateLE;
    KLineEdit*       codecLE;
    KLineEdit*       compressorLE;
    KLineEdit*       heightLE;
    KLineEdit*       widthLE;
    
    int lastPosition;
};

XMPVideospecs::XMPVideospecs(QWidget* const parent)
    : QWidget(parent), d(new XMPVideospecsPriv)
{
    grid = new QGridLayout(this);
    d->frameRateCheck = new QCheckBox(i18n("FrameRate:"));
    d->codecCheck = new QCheckBox(i18n("Codec:"));
    d->compressorCheck = new QCheckBox(i18n("Compressor:"));
    d->heightCheck = new QCheckBox(i18n("Height:"));
    d->widthCheck = new QCheckBox(i18n("Width:"));
    
    d->frameRateLE  = new KLineEdit(this); 
    d->codecLE = new KLineEdit(this);
    d->compressorLE = new KLineEdit(this);
    d->heightLE = new KLineEdit(this);
    d->widthLE = new KLineEdit(this);
    
    grid->addWidget(d->frameRateCheck,                    0, 0, 1, 1);
    grid->addWidget(d->frameRateLE,                       0, 1, 1, 2);
    grid->addWidget(d->codecCheck,                        1, 0, 1, 1);
    grid->addWidget(d->codecLE,                           1, 1, 1, 2);
    grid->addWidget(d->compressorCheck,                   2, 0, 1, 1);
    grid->addWidget(d->compressorLE,                      2, 1, 1, 2);
    grid->addWidget(d->heightCheck,                       3, 0, 1, 1);
    grid->addWidget(d->heightLE,                          3, 1, 1, 1);
    grid->addWidget(d->widthCheck,                        3, 3, 1, 1);
    grid->addWidget(d->widthLE,                           3, 4, 1, 1);
                     //                                   ^  
    d->lastPosition = 3;//.................................   
}

XMPVideospecs::~XMPVideospecs()
{
    delete d;
}

void XMPVideospecs::readMetadata(QByteArray& xmpData)
{
    blockSignals(true);
    KPMetadata meta;
    meta.setXmp(xmpData);
    
    //Show generic metadata which exist in all video types
    QString data = meta.getXmpTagString("Xmp.video.FrameRate", false);
    if (!data.isNull())
    {
        d->frameRateLE->setText(data);
    }
    data = meta.getXmpTagString("Xmp.video.Codec", false);
    if (!data.isNull())
    {
        d->codecLE->setText(data);
    }
    data = meta.getXmpTagString("Xmp.video.Compressor", false);
    if (!data.isNull())
    {
        d->compressorLE->setText(data);
    }
    data = meta.getXmpTagString("Xmp.video.Height", false);
    if (!data.isNull())
    {
        d->heightLE->setText(data);
    }
    data = meta.getXmpTagString("Xmp.video.Width", false);
    if (!data.isNull())
    {
        d->widthLE->setText(data);
    }
    
    data = meta.getXmpTagString("Xmp.video.MimeType", false);
    
    //Show metadata specific to file type
    StandardVideotags *videoTags = new StandardVideotags();
    if(data == "video/riff")
    {
    }
    else if(data == "video/quicktime")
    {
        d->qTimeLangCheck = new QCheckBox(i18n("Language:"));
        d->qTimeLangCB = new KComboBox(this);
        videoTags->setQTimeLangCodes(d->qTimeLangCB);
        grid->addWidget(d->qTimeLangCheck ,               d->lastPosition+1, 0, 1, 1);
        grid->addWidget(d->qTimeLangCB,                   d->lastPosition+1, 1, 1, 2);
    }
    
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(2, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());
    
    blockSignals(false);
}

void XMPVideospecs::applyMetadata(QByteArray& xmpData)
{
}

}  // namespace KIPIMetadataEditPlugin
