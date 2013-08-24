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
using namespace KDcrawIface;

namespace KIPIMetadataEditPlugin
{

class XMPAudiospecs::XMPAudiospecsPriv
{
public:

    XMPAudiospecsPriv()
    {
        sampleRateCheck     = 0;
        compressorCheck     = 0;
        
        sampleRateLE        = 0;
        compressorLE        = 0;
        compressorCB        = 0;

        lastposition  = 0;
    }
    QCheckBox*       sampleRateCheck;
    QCheckBox*       compressorCheck;
    
    KLineEdit*       sampleRateLE;
    KLineEdit*       compressorLE;
    KComboBox*       compressorCB;
    
    int lastposition;
};

XMPAudiospecs::XMPAudiospecs(QWidget* const parent)
    : QWidget(parent), d(new XMPAudiospecsPriv)
{
    grid = new QGridLayout(this);
 
    d->sampleRateCheck = new QCheckBox(i18n("SampleRate:"));
    d->sampleRateLE = new KLineEdit(this);
    
    grid->addWidget(d->sampleRateCheck,                   1, 0, 1, 1);
    grid->addWidget(d->sampleRateLE,                      1, 1, 1, 2);
                       //                                 ^
    d->lastposition = 1;  //...............................
}

XMPAudiospecs::~XMPAudiospecs()
{
    delete d;
}

void XMPAudiospecs::readMetadata(QByteArray& xmpData)
{
    
    blockSignals(true);
    KPMetadata meta;
    meta.setXmp(xmpData);
    QString data = meta.getXmpTagString("Xmp.audio.SampleRate", false);
    if (!data.isNull())
    {
        d->sampleRateLE->setText(data);
    }    
    
    StandardVideotags *videoTags = new StandardVideotags();
    data = meta.getXmpTagString("Xmp.video.MimeType", false);
    if(data == "video/riff")
    {
        d->compressorCB = new KComboBox(this);
        videoTags->setRiffEncodVals(d->compressorCB);
        d->compressorCheck = new QCheckBox(i18n("Compressor:"));
        grid->addWidget(d->compressorCheck,       d->lastposition+1,0,1,1);
        grid->addWidget(d->compressorCB,          d->lastposition+1,1,1,2);   
    }
    else if(data == "video/quicktime")
    {
        data = meta.getXmpTagString("Xmp.audio.Compressor", false);
        if (!data.isNull())
        {
            d->compressorLE = new KLineEdit(this);
            d->compressorLE->setText(data);
            d->compressorCheck = new QCheckBox(i18n("Compressor:"));
            grid->addWidget(d->compressorCheck,       d->lastposition+1,0,1,1);
            grid->addWidget(d->compressorLE,          d->lastposition+1,1,1,1);
        }    
    }
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(2, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint()); 
    blockSignals(false);
}

void XMPAudiospecs::applyMetadata(QByteArray& xmpData)
{
    KPMetadata meta;
    meta.setXmp(xmpData);
}

}  // namespace KIPIMetadataEditPlugin
