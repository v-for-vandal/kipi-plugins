/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2013-08-31
 * Description : a plugin to blend bracketed images.
 *
 * Copyright (C) 2013 by Soumajyoti Sarkar <ergy dot ergy at gmail dot com>
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

#include "hdrgentask.h"

// Qt includes

#include <QFileInfo>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

// KDE includes

#include <klocale.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <ktempdir.h>
// Local includes

#include "kpmetadata.h"
#include "kpversion.h"
#include "kpwriteimage.h"

using namespace KIPIPlugins;

namespace KIPIExpoBlendingPlugin
{

HdrGenTask::HdrGenTask(QObject* const parent, const KUrl::List& inUrls)
    : Task(parent, HDRGEN, inUrls), urls(inUrls)
{}

HdrGenTask::HdrGenTask(const KUrl::List& inUrls)
    : Task(0, HDRGEN, inUrls), urls(inUrls)
{}

HdrGenTask::~HdrGenTask()
{}

void HdrGenTask::run()
{   
    QString prefix = KStandardDirs::locateLocal("tmp", QString("kipi-hdr-exiftags-tmp-") +
                                                       QString::number(QDateTime::currentDateTime().toTime_t()));

    preprocessingTmpDir = new KTempDir(prefix);
    KUrl exifTags = KUrl(preprocessingTmpDir->name() + QString("exifTags.hdrgen"));
    QFile exifFile(exifTags.toLocalFile());
    exifFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&exifFile);
    
    KPMetadata meta;
    
    foreach (const KUrl& url, urls)
    {
        meta.load(url.toLocalFile());
        if (!meta.hasExif())
	{
	   errString = i18n("Image metadata has no Exif Information.");
           successFlag = false;
           return;
	}
    
        long num = 1, den = 1;

        // default not valid values

        float    expo = -1.0;            // Exposure Time
        float    iso  = -1.0;            // ISO Speed
        float    aper = -1.0;            // Aperture Value
        float    inverse_expo = -1.0;    // Inverse of exposure time needed for pfshdrcalibrate

        if (meta.getExifTagRational("Exif.Photo.ExposureTime", num, den))
        {
            if (den)
                expo = (float)(num) / (float)(den);
        }
        else if (getXmpRational("Xmp.exif.ExposureTime", num, den, meta))
        {
            if (den)
                expo = (float)(num) / (float)(den);
        }
        else if (meta.getExifTagRational("Exif.Photo.ShutterSpeedValue", num, den))
        {
            long   nmr = 1, div = 1;
            double tmp = 0.0;

            if (den)
                tmp = exp(log(2.0) * (float)(num) / (float)(den));

            if (tmp > 1.0)
            {
                div = (long)(tmp + 0.5);
            }
            else
            {
                nmr = (long)(1 / tmp + 0.5);
            } 

            if (div)
                expo = (float)(nmr) / (float)(div);
        }
        else if (getXmpRational("Xmp.exif.ShutterSpeedValue", num, den, meta))
        {
            long   nmr = 1, div = 1;
            double tmp = 0.0;

            if (den)
                tmp = exp(log(2.0) * (float)(num) / (float)(den));

            if (tmp > 1.0)
            {
                div = (long)(tmp + 0.5);
            }
            else
            {
                nmr = (long)(1 / tmp + 0.5);
            }

            if (div)
                expo = (float)(nmr) / (float)(div);
        }

        kDebug() << url.fileName() << " : expo = " << expo;

        if (meta.getExifTagRational("Exif.Photo.ApertureValue", num, den))
        {
            if (den)
                aper = (float)(num) / (float) (den) ;
        }
        else if (getXmpRational("Xmp.exif.ApertureValue", num, den, meta))
        {
            if (den)
                aper = (float)(num) / (float) (den) ;
        }

        kDebug() << url.fileName() << " :aper = " << aper;

        // Some cameras/lens DO print the aper but with value 0, and this is not allowed for ev computation purposes.

        if (aper == 0.0)
	{
           successFlag = false;
           return;
	}
           

        // If iso is found use that value, otherwise assume a value of iso=100. (again, some cameras do not print iso in exif).

        if (meta.getExifTagRational("Exif.Photo.ISOSpeedRatings", num, den))
        {
            if (den)
                iso = (float)(num) / (float)(den);
        }
        else if (getXmpRational("Xmp.exif.ISOSpeedRatings", num, den, meta))
        {
            if (den)
                iso = (float)(num) / (float)(den);
        }
        else
        {
            iso = 100.0;
        }
    
        inverse_expo = 1/(float)expo;

        kDebug() << url.fileName() << " : iso = " << iso;
    
        out << url.fileName() << " " << inverse_expo << " " << aper << " " << iso << " " << "0" <<endl;
    }
    exifFile.close();
    return;
    
}


bool HdrGenTask::getXmpRational(const char* xmpTagName, long& num, long& den, KPMetadata& meta)
{
    QVariant rationals = meta.getXmpTagVariant(xmpTagName);

    if (!rationals.isNull())
    {
        QVariantList list = rationals.toList();

        if (list.size() == 2)
        {
            num = list[0].toInt();
            den = list[1].toInt();

            return true;
        }
    }
    successFlag = false;
    return false;
}

} // namespace KIPIExpoBlendingPlugin