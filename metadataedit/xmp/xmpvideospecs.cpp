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
    QCheckBox*       commentCheck;
    QCheckBox*       genreCheck;
    QCheckBox*       directorCheck;
    QCheckBox*       artistCheck;
    QCheckBox*       cinematographerCheck;
    QCheckBox*       editedByCheck;
    QCheckBox*       engineerCheck;
    QCheckBox*       languageOneCheck;
    QCheckBox*       musicByCheck;
    QCheckBox*       titleCheck;
    QCheckBox*       producedByCheck;
    QCheckBox*       sourceCheck;
    QCheckBox*       starringCheck;
    QCheckBox*       locationInfoCheck;
    QCheckBox*       tapeNameCheck;
    QCheckBox*       yearCheck;
    QCheckBox*       productCheck;
    QCheckBox*       performerKeywordsCheck;
    QCheckBox*       copyrightCheck;
    QCheckBox*       countryCheck;
    QCheckBox*       languageTwoCheck;
    QCheckBox*       softwareCheck;

    KComboBox*       qTimeLangCB;

    KLineEdit*       frameRateLE;
    KLineEdit*       codecLE;
    KLineEdit*       compressorLE;
    KLineEdit*       heightLE;
    KLineEdit*       widthLE;
    KLineEdit*       genreLE;
    KLineEdit*       directorLE;
    KLineEdit*       artistLE;
    KLineEdit*       cinematographerLE;
    KLineEdit*       editedByLE;
    KLineEdit*       engineerLE;
    KLineEdit*       languageOneLE;
    KLineEdit*       musicByLE;
    KLineEdit*       titleLE;
    KLineEdit*       producedByLE;
    KLineEdit*       sourceLE;
    KLineEdit*       starringLE;
    KLineEdit*       languageTwoLE;
    KLineEdit*       locationInfoLE;
    KLineEdit*       tapeNameLE;
    KLineEdit*       yearLE;
    KLineEdit*       productLE;
    KLineEdit*       performerKeywordsLE;
    KLineEdit*       copyrightLE;
    KLineEdit*       countryLE;
    KLineEdit*       commentLE;
    KLineEdit*       softwareLE;

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

    connect(d->frameRateCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->codecCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->compressorCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->heightCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->widthCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));
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
        d->commentCheck = new QCheckBox(i18n("Comment:"));
        d->genreCheck = new QCheckBox(i18n("Genre:"));
        d->directorCheck = new QCheckBox(i18n("Director:"));
        d->artistCheck = new QCheckBox(i18n("Artist:"));
        d->cinematographerCheck = new QCheckBox(i18n("Cinematographer:"));
        d->editedByCheck = new QCheckBox(i18n("Edited By:"));
        d->engineerCheck = new QCheckBox(i18n("Engineer:"));
        d->languageOneCheck = new QCheckBox(i18n("Language:"));
        d->musicByCheck = new QCheckBox(i18n("Music By:"));
        d->titleCheck = new QCheckBox(i18n("Title:"));
        d->producedByCheck = new QCheckBox(i18n("Produced By:"));
        d->sourceCheck = new QCheckBox(i18n("Source:"));
        d->starringCheck = new QCheckBox(i18n("Starring:"));
        d->languageTwoCheck = new QCheckBox(i18n("Language:"));
        d->locationInfoCheck = new QCheckBox(i18n("Location Information:"));
        d->tapeNameCheck = new QCheckBox(i18n("Tape Name:"));
        d->yearCheck = new QCheckBox(i18n("Year:"));
        d->productCheck = new QCheckBox(i18n("Product:"));
        d->performerKeywordsCheck = new QCheckBox(i18n("Performer Keywords:"));
        d->copyrightCheck = new QCheckBox(i18n("Copyright:"));
        d->countryCheck = new QCheckBox(i18n("Country:"));
        d->softwareCheck = new QCheckBox(i18n("Software:"));

        connect(d->genreCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->directorCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->artistCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->cinematographerCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->editedByCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->engineerCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->languageOneCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->musicByCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->titleCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->producedByCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->sourceCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->starringCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->languageTwoCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->locationInfoCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->tapeNameCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->yearCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->productCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->performerKeywordsCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->copyrightCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->countryCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->commentCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        connect(d->softwareCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

        d->commentLE = new KLineEdit(this);
        d->genreLE = new KLineEdit(this);
        d->directorLE = new KLineEdit(this);
        d->artistLE = new KLineEdit(this);
        d->cinematographerLE = new KLineEdit(this);
        d->editedByLE = new KLineEdit(this);
        d->engineerLE = new KLineEdit(this);
        d->languageOneLE = new KLineEdit(this);
        d->musicByLE = new KLineEdit(this);
        d->titleLE = new KLineEdit(this);
        d->producedByLE = new KLineEdit(this);
        d->sourceLE = new KLineEdit(this);
        d->starringLE = new KLineEdit(this);
        d->languageTwoLE = new KLineEdit(this);
        d->locationInfoLE = new KLineEdit(this);
        d->tapeNameLE = new KLineEdit(this);
        d->yearLE = new KLineEdit(this);
        d->productLE = new KLineEdit(this);
        d->performerKeywordsLE = new KLineEdit(this);
        d->copyrightLE = new KLineEdit(this);
        d->countryLE = new KLineEdit(this);
        d->softwareLE = new KLineEdit(this);

        data = meta.getXmpTagString("Xmp.video.Comment", false);
        if(!data.isNull())
        {
            d->commentLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Genre", false);
        if(!data.isNull())
        {
            d->genreLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Director", false);
        if(!data.isNull())
        {
            d->directorLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Artist", false);
        if(!data.isNull())
        {
            d->artistLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Cinematographer", false);
        if(!data.isNull())
        {
            d->cinematographerLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.EditedBy", false);
        if(!data.isNull())
        {
            d->editedByLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Engineer", false);
        if(!data.isNull())
        {
            d->engineerLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Language", false);
        if(!data.isNull())
        {
            d->languageTwoLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.MusicBy", false);
        if(!data.isNull())
        {
            d->musicByLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Title", false);
        if(!data.isNull())
        {
            d->titleLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.ProducedBy", false);
        if(!data.isNull())
        {
            d->producedByLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Source", false);
        if(!data.isNull())
        {
            d->sourceLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Starring", false);
        if(!data.isNull())
        {
            d->starringLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Language", false);
        if(!data.isNull())
        {
            d->languageOneLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.LocationInfo", false);
        if(!data.isNull())
        {
            d->locationInfoLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.TapeName", false);
        if(!data.isNull())
        {
            d->tapeNameLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Year", false);
        if(!data.isNull())
        {
            d->yearLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Product", false);
        if(!data.isNull())
        {
            d->productLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.PerformerKeywords", false);
        if(!data.isNull())
        {
            d->performerKeywordsLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Copyright", false);
        if(!data.isNull())
        {
            d->copyrightLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Country", false);
        if(!data.isNull())
        {
            d->countryLE->setText(data);
        }
        data = meta.getXmpTagString("Xmp.video.Software", false);
        if(!data.isNull())
        {
            d->softwareLE->setText(data);
        }

        grid->addWidget(d->commentCheck ,               d->lastPosition+1, 0, 1, 1);
        grid->addWidget(d->commentLE ,                  d->lastPosition+1, 1, 1, 2);
        grid->addWidget(d->genreCheck ,                 d->lastPosition+2, 0, 1, 1);
        grid->addWidget(d->genreLE ,                    d->lastPosition+2, 1, 1, 2);
        grid->addWidget(d->directorCheck ,              d->lastPosition+3, 0, 1, 1);
        grid->addWidget(d->directorLE ,                 d->lastPosition+3, 1, 1, 2);
        grid->addWidget(d->artistCheck ,                d->lastPosition+4, 0, 1, 1);
        grid->addWidget(d->artistLE ,                   d->lastPosition+4, 1, 1, 2);
        grid->addWidget(d->cinematographerCheck ,       d->lastPosition+5, 0, 1, 1);
        grid->addWidget(d->cinematographerLE ,          d->lastPosition+5, 1, 1, 2);
        grid->addWidget(d->editedByCheck ,              d->lastPosition+6, 0, 1, 1);
        grid->addWidget(d->editedByLE ,                 d->lastPosition+6, 1, 1, 2);
        grid->addWidget(d->engineerCheck ,              d->lastPosition+7, 0, 1, 1);
        grid->addWidget(d->engineerLE ,                 d->lastPosition+7, 1, 1, 2);
        grid->addWidget(d->languageOneCheck ,           d->lastPosition+8, 0, 1, 1);
        grid->addWidget(d->languageOneLE ,              d->lastPosition+8, 1, 1, 2);
        grid->addWidget(d->musicByCheck ,               d->lastPosition+9, 0, 1, 1);
        grid->addWidget(d->musicByLE ,                  d->lastPosition+9, 1, 1, 2);
        grid->addWidget(d->titleCheck ,                 d->lastPosition+10, 0, 1, 1);
        grid->addWidget(d->titleLE ,                    d->lastPosition+10, 1, 1, 2);
        grid->addWidget(d->countryCheck ,               d->lastPosition+11, 0, 1, 1);
        grid->addWidget(d->countryLE ,                  d->lastPosition+11, 1, 1, 2);
        grid->addWidget(d->producedByCheck ,            d->lastPosition+1, 3, 1, 1);
        grid->addWidget(d->producedByLE ,               d->lastPosition+1, 4, 1, 2);
        grid->addWidget(d->sourceCheck ,                d->lastPosition+2, 3, 1, 1);
        grid->addWidget(d->sourceLE ,                   d->lastPosition+2, 4, 1, 2);
        grid->addWidget(d->starringCheck ,              d->lastPosition+3, 3, 1, 1);
        grid->addWidget(d->starringLE ,                 d->lastPosition+3, 4, 1, 2);
        grid->addWidget(d->languageTwoCheck ,           d->lastPosition+4, 3, 1, 1);
        grid->addWidget(d->languageTwoLE ,              d->lastPosition+4, 4, 1, 2);
        grid->addWidget(d->locationInfoCheck ,          d->lastPosition+5, 3, 1, 1);
        grid->addWidget(d->locationInfoLE ,             d->lastPosition+5, 4, 1, 2);
        grid->addWidget(d->tapeNameCheck ,              d->lastPosition+6, 3, 1, 1);
        grid->addWidget(d->tapeNameLE ,                 d->lastPosition+6, 4, 1, 2);
        grid->addWidget(d->yearCheck ,                  d->lastPosition+7, 3, 1, 1);
        grid->addWidget(d->yearLE ,                     d->lastPosition+7, 4, 1, 2);
        grid->addWidget(d->productCheck ,               d->lastPosition+8, 3, 1, 1);
        grid->addWidget(d->productLE ,                  d->lastPosition+8, 4, 1, 2);
        grid->addWidget(d->performerKeywordsCheck ,     d->lastPosition+9, 3, 1, 1);
        grid->addWidget(d->performerKeywordsLE ,        d->lastPosition+9, 4, 1, 2);
        grid->addWidget(d->copyrightCheck ,             d->lastPosition+10, 3, 1, 1);
        grid->addWidget(d->copyrightLE ,                d->lastPosition+10, 4, 1, 2);
        grid->addWidget(d->softwareCheck ,              d->lastPosition+11, 3, 1, 1);
        grid->addWidget(d->softwareLE ,                 d->lastPosition+11, 4, 1, 2);
    }
    else if(data == "video/quicktime")
    {
        d->qTimeLangCheck = new QCheckBox(i18n("Language:"));

        connect(d->qTimeLangCheck, SIGNAL(toggled(bool)),
                this, SIGNAL(signalModified()));

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
    KPMetadata meta;
    meta.setXmp(xmpData);
    meta.setXmpTagString("Xmp.video.FrameRate", d->frameRateLE->text());
    meta.setXmpTagString("Xmp.video.Codec", d->codecLE->text());
    meta.setXmpTagString("Xmp.video.Compressor", d->compressorLE->text());
    meta.setXmpTagString("Xmp.video.Height", d->heightLE->text());
    meta.setXmpTagString("Xmp.video.Width", d->widthLE->text());
    meta.setXmpTagString("Xmp.video.Comment", d->commentLE->text());
    meta.setXmpTagString("Xmp.video.Genre", d->genreLE->text());
    meta.setXmpTagString("Xmp.video.Artist", d->artistLE->text());
    meta.setXmpTagString("Xmp.video.Director", d->directorLE->text());
    meta.setXmpTagString("Xmp.video.Cinematographer", d->cinematographerLE->text());
    meta.setXmpTagString("Xmp.video.EditedBy", d->editedByLE->text());
    meta.setXmpTagString("Xmp.video.Engineer", d->engineerLE->text());
    meta.setXmpTagString("Xmp.video.Language", d->languageOneLE->text());
    meta.setXmpTagString("Xmp.video.MusicBy", d->musicByLE->text());
    meta.setXmpTagString("Xmp.video.Title", d->titleLE->text());
    meta.setXmpTagString("Xmp.video.producedBy", d->producedByLE->text());
    meta.setXmpTagString("Xmp.video.Source", d->sourceLE->text());
    meta.setXmpTagString("Xmp.video.Starring", d->starringLE->text());
    meta.setXmpTagString("Xmp.video.Language", d->languageTwoLE->text());
    meta.setXmpTagString("Xmp.video.LocationInfo", d->locationInfoLE->text());
    meta.setXmpTagString("Xmp.video.TapeName", d->tapeNameLE->text());
    meta.setXmpTagString("Xmp.video.Year", d->yearLE->text());
    meta.setXmpTagString("Xmp.video.Product", d->productLE->text());
    meta.setXmpTagString("Xmp.video.PerformerKeywords", d->performerKeywordsLE->text());
    meta.setXmpTagString("Xmp.video.Copyright", d->copyrightLE->text());
    meta.setXmpTagString("Xmp.video.Country", d->countryLE->text());
    meta.setXmpTagString("Xmp.video.Software", d->softwareLE->text());

    xmpData = meta.getXmp();
}

}  // namespace KIPIMetadataEditPlugin
