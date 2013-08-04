/* ============================================================
 * 
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2013-06-28
 * Description : a plugin to blend bracketed images.
 *
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "generictask.moc"
// C++ includes

#include <cmath>

// Qt includes

#include <QFileInfo>
#include <QtDebug>
#include <QDateTime>
#include <QFileInfo>
#include <QPointer>

// Under Win32, log2f is not defined...
#ifdef _WIN32
#define log2f(x) (logf(x)*1.4426950408889634f)
#endif

// KDE includes
#include <klocale.h>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/JobCollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <ktempdir.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>

//Local includes
#include "actions.h"
#include "kpmetadata.h"
#include "kpwriteimage.h"
#include "kpversion.h"

namespace KIPIExpoBlendingPlugin
{
  
GenericTask::GenericTask(QObject* const parent, const KUrl::List& fileUrl, const Action& action)
    : Task(parent, action, fileUrl), urls(fileUrl), action(action)
{}

GenericTask::GenericTask(QObject* const parent, const KUrl::List& fileUrl, const Action& action, 
			 const RawDecodingSettings& rawSettings, const bool align, const QString& alignPath)
    : Task(parent, action, fileUrl), urls(fileUrl), action(action),  
	   settings(rawSettings), align(align), binaryPath(alignPath)
{}

GenericTask::GenericTask(QObject* const parent, const KUrl::List& fileUrl, const Action& action, const KUrl& outputUrl, 
			 const EnfuseSettings& settings, const QString& alignPath, bool version)
    : Task(parent, action, fileUrl), urls(fileUrl), action(action), outputUrl(outputUrl), enfuseSettings(settings), 
	   binaryPath(alignPath), enfuseVersion4x(version)
{}

GenericTask::GenericTask(const KUrl::List& fileUrl, const Action& action, const KUrl& outputUrl, 
			 const EnfuseSettings& settings, const QString& alignPath, bool version)
    : Task(0, action, fileUrl), urls(fileUrl),action(action), outputUrl(outputUrl), enfuseSettings(settings), 
	   binaryPath(alignPath), enfuseVersion4x(version)
{}

GenericTask::~GenericTask()
{}

bool GenericTask::success() const
{
    return successFlag;
}

void GenericTask::requestAbort()
{
    isAbortedFlag = true;
}

void GenericTask::run()
{
    cancel = false;
    
    if (!cancel)
    {
 
            switch (action)
            {
                case IDENTIFY:
                {
                    // Identify Exposure.

                    QString avLum;
                    float val;
                    if (!urls.isEmpty())
                    {
                        val = getAverageSceneLuminance(urls[0].toLocalFile());
                        if (val != -1)
                            avLum.setNum(log2f(val), 'g', 2);
                    }
                    
                    ActionData ad;
                    ad.action  = action;
                    ad.inUrls  = urls;
                    ad.message = avLum.isEmpty() ? i18n("unknown") : avLum;
                    ad.success = avLum.isEmpty();
	
                    emit finished(ad);
		    break;
                }
                
                case PREPROCESSING:
                {
                    ActionData ad1;
                    ad1.action   = PREPROCESSING;
                    ad1.inUrls   = urls;
                    ad1.starting = true;
                    emit starting(ad1);

                    ItemUrlsMap preProcessedUrlsMap;
                    QString     errors;

                    bool result  = startPreProcessing(urls, preProcessedUrlsMap, align, settings, binaryPath, errors);

                    ActionData ad2;
                    ad2.action              = PREPROCESSING;
                    ad2.inUrls              = urls;
                    ad2.preProcessedUrlsMap = preProcessedUrlsMap;
                    ad2.success             = result;
                    ad2.message             = errors;
                    emit finished(ad2);
                    break;
                }
		
                case LOAD:
                {
		 
                    ActionData ad1;
                    ad1.action   = LOAD;
                    ad1.inUrls   = urls;
                    ad1.starting = true;
                    emit starting(ad1);

                    QImage image;
                    bool result  = image.load(urls[0].toLocalFile());

                    // rotate image
                    if (result)
                    {
                        KPMetadata meta(urls[0].toLocalFile());
                        meta.rotateExifQImage(image, meta.getImageOrientation());
                    }

                    ActionData ad2;
                    ad2.action         = LOAD;
                    ad2.inUrls         = urls;
                    ad2.success        = result;
                    ad2.image          = image;
                    emit finished(ad2);
                    break;
                }

                case ENFUSEPREVIEW:
                {
                    ActionData ad1;
                    ad1.action         = ENFUSEPREVIEW;
                    ad1.inUrls         = urls;
                    ad1.starting       = true;
                    ad1.enfuseSettings = enfuseSettings;
                    emit starting(ad1);

                    QString errors;
                    KUrl    destUrl         = outputUrl;
                    EnfuseSettings settings = enfuseSettings;
                    settings.outputFormat   = KPSaveSettingsWidget::OUTPUT_JPEG;    // JPEG for preview: fast and small.
                    bool result             = startEnfuse(urls, destUrl, settings, binaryPath, errors);

                    kDebug() << "Preview result was: " << result;

                    // preserve exif information for auto rotation
                    if (result)
                    {
                        KPMetadata metaIn(urls[0].toLocalFile());
                        KPMetadata metaOut(destUrl.toLocalFile());
                        metaOut.setImageOrientation(metaIn.getImageOrientation());
                        metaOut.applyChanges();
                    }

                    ActionData ad2;
                    ad2.action         = ENFUSEPREVIEW;
                    ad2.inUrls         = urls;
                    ad2.outUrls        = KUrl::List() << destUrl;
                    ad2.success        = result;
                    ad2.message        = errors;
                    ad2.enfuseSettings = enfuseSettings;
                    emit finished(ad2);
                    break;
                }

                case ENFUSEFINAL:
                {
                    ActionData ad1;
                    ad1.action         = ENFUSEFINAL;
                    ad1.inUrls         = urls;
                    ad1.starting       = true;
                    ad1.enfuseSettings = enfuseSettings;
                    emit starting(ad1);

                    KUrl    destUrl = outputUrl;
                    bool    result  = false;
                    QString errors;

                    result = startEnfuse(urls, destUrl, enfuseSettings, binaryPath, errors);

                    // We will take first image metadata from stack to restore Exif, Iptc, and Xmp.
                    KPMetadata meta;
                    meta.load(urls[0].toLocalFile());
                    result = result & meta.setXmpTagString("Xmp.kipi.EnfuseInputFiles", enfuseSettings.inputImagesList(), false);
                    result = result & meta.setXmpTagString("Xmp.kipi.EnfuseSettings", enfuseSettings.asCommentString().replace('\n', " ; "), false);
                    meta.setImageDateTime(QDateTime::currentDateTime());
                    if (enfuseSettings.outputFormat != KPSaveSettingsWidget::OUTPUT_JPEG)
                    {
                        QImage img;
                        if (img.load(destUrl.toLocalFile()))
                            meta.setImagePreview(img.scaled(1280, 1024, Qt::KeepAspectRatio));
                    }
                    meta.save(destUrl.toLocalFile());

                    

                    ActionData ad2;
                    ad2.action         = ENFUSEFINAL;
                    ad2.inUrls         = urls;
                    ad2.outUrls        = KUrl::List() << destUrl;
                    ad2.success        = result;
                    ad2.message        = errors;
                    ad2.enfuseSettings = enfuseSettings;
                    emit finished(ad2);
                    break;
                }

                default:
                {
                    qCritical() << "Unknown action specified" << endl;
                    break;
                }
            }
        }
    
}

void GenericTask::cleanAlignTmpDir()
{
    if (preprocessingTmpDir)
    {
	preprocessingTmpDir->unlink();
        delete preprocessingTmpDir;
        preprocessingTmpDir = 0;
    }
}

bool GenericTask::startPreProcessing(const KUrl::List& inUrls, ItemUrlsMap& preProcessedUrlsMap,
                                      bool align, const RawDecodingSettings& settings,
                                      const QString& alignPath, QString& errors)
{
    QString prefix = KStandardDirs::locateLocal("tmp", QString("kipi-expoblending-preprocessing-tmp-") +
                                                       QString::number(QDateTime::currentDateTime().toTime_t()));

    cleanAlignTmpDir();

    preprocessingTmpDir = new KTempDir(prefix);

    volatile bool error = false;

    for (int i = 0; i < inUrls.size(); ++i)
    {

        if (error)
        {
            continue;
        }

        KUrl url = inUrls.at(i);

        if (KPMetadata::isRawFile(url.toLocalFile()))
        {
            KUrl preprocessedUrl, previewUrl;

            if (!convertRaw(url, preprocessedUrl, settings))
            {
                error = true;
                continue;
            }

            if (!computePreview(preprocessedUrl, previewUrl))
            {
                error = true;
                continue;
            }

        }
        else
        {
            // NOTE: in this case, preprocessed Url is original file Url.
            KUrl previewUrl;
            if (!computePreview(url, previewUrl))
            {
                error = true;
                continue;
            }
	}
    }

    if (error)
    {
        return false;
    }

    if (align)
    {
        // Re-align images
        alignProcess = new KProcess;
        alignProcess->clearProgram();
        alignProcess->setWorkingDirectory(preprocessingTmpDir->name());
        alignProcess->setOutputChannelMode(KProcess::MergedChannels);

        QStringList args;
        args << alignPath;
        args << "-v";
        args << "-a";
        args << "aligned";

        alignProcess->setProgram(args);

        kDebug() << "Align command line: " << alignProcess->program();

        alignProcess->start();

        if (!alignProcess->waitForFinished(-1))
        {
            errors = getProcessError(alignProcess);
            return false;
        }

        uint    i=0;
        QString temp;
        preProcessedUrlsMap.clear();

        foreach(const KUrl& url, inUrls)
        {
            KUrl previewUrl;
            KUrl alignedUrl = KUrl(preprocessingTmpDir->name() + temp.sprintf("aligned%04i", i) + QString(".tif"));
            if (!computePreview(alignedUrl, previewUrl))
                return false;
            preProcessedUrlsMap.insert(url, ItemPreprocessedUrls(alignedUrl, previewUrl));
            i++;
        }

        for (QMap<KUrl, ItemPreprocessedUrls>::const_iterator it = preProcessedUrlsMap.constBegin() ; it != preProcessedUrlsMap.constEnd(); ++it)
        {
            kDebug() << "Pre-processed output urls map: " << it.key() << " , "
                                                          << it.value().preprocessedUrl << " , "
                                                          << it.value().previewUrl << " ; ";
        }
        kDebug() << "Align exit status    : "         << alignProcess->exitStatus();
        kDebug() << "Align exit code      : "         << alignProcess->exitCode();

        if (alignProcess->exitStatus() != QProcess::NormalExit)
            return false;

        if (alignProcess->exitCode() == 0)
        {
            // Process finished successfully !
            return true;
        }

        errors = getProcessError(alignProcess);
        return false;
    }
    else
    {
        for (QMap<KUrl, ItemPreprocessedUrls>::const_iterator it = preProcessedUrlsMap.constBegin() ; it != preProcessedUrlsMap.constEnd(); ++it)
        {
            kDebug() << "Pre-processed output urls map: " << it.key() << " , "
                                                          << it.value().preprocessedUrl << " , "
                                                          << it.value().previewUrl << " ; ";
        }
        kDebug() << "Alignment not performed.";
        return true;
    }
}

bool GenericTask::computePreview(const KUrl& inUrl, KUrl& outUrl)
{
    outUrl = preprocessingTmpDir->name();
    QFileInfo fi(inUrl.toLocalFile());
    outUrl.setFileName(QString(".") + fi.completeBaseName().replace('.', '_') + QString("-preview.jpg"));

    QImage img;
    if (img.load(inUrl.toLocalFile()))
    {
        QImage preview = img.scaled(1280, 1024, Qt::KeepAspectRatio);
        bool saved     = preview.save(outUrl.toLocalFile(), "JPEG");
        // save exif information also to preview image for auto rotation
        if (saved)
        {
            KPMetadata metaIn(inUrl.toLocalFile());
            KPMetadata metaOut(outUrl.toLocalFile());
            metaOut.setImageOrientation(metaIn.getImageOrientation());
            metaOut.applyChanges();
        }
        kDebug() << "Preview Image url: " << outUrl << ", saved: " << saved;
        return saved;
    }
    return false;
}

bool GenericTask::convertRaw(const KUrl& inUrl, KUrl& outUrl, const RawDecodingSettings& settings)
{
    int        width, height, rgbmax;
    QByteArray imageData;

    QPointer<KDcraw> rawdec = new KDcraw;

    bool decoded = rawdec->decodeRAWImage(inUrl.toLocalFile(), settings, imageData, width, height, rgbmax);

    if (decoded)
    {
        uchar* sptr  = (uchar*)imageData.data();
        float factor = 65535.0 / rgbmax;
        unsigned short tmp16[3];

        // Set RGB color components.
        for (int i = 0 ; !cancel && (i < width * height) ; ++i)
        {
            // Swap Red and Blue and re-ajust color component values
            tmp16[0] = (unsigned short)((sptr[5]*256 + sptr[4]) * factor);      // Blue
            tmp16[1] = (unsigned short)((sptr[3]*256 + sptr[2]) * factor);      // Green
            tmp16[2] = (unsigned short)((sptr[1]*256 + sptr[0]) * factor);      // Red
            memcpy(&sptr[0], &tmp16[0], 6);
            sptr += 6;
        }

        KPMetadata meta;
        meta.load(inUrl.toLocalFile());
        meta.setImageProgramId(QString("Kipi-plugins"), QString(kipiplugins_version));
        meta.setImageDimensions(QSize(width, height));
        meta.setExifTagString("Exif.Image.DocumentName", inUrl.fileName());
        meta.setXmpTagString("Xmp.tiff.Make",  meta.getExifTagString("Exif.Image.Make"));
        meta.setXmpTagString("Xmp.tiff.Model", meta.getExifTagString("Exif.Image.Model"));
        meta.setImageOrientation(KPMetadata::ORIENTATION_NORMAL);

        QByteArray prof = KPWriteImage::getICCProfilFromFile(settings.outputColorSpace);

        KPWriteImage wImageIface;
        wImageIface.setCancel(&cancel);
        wImageIface.setImageData(imageData, width, height, true, false, prof, meta);
        outUrl = preprocessingTmpDir->name();
        QFileInfo fi(inUrl.toLocalFile());
        outUrl.setFileName(QString(".") + fi.completeBaseName().replace('.', '_') + QString(".tif"));

        if (!wImageIface.write2TIFF(outUrl.toLocalFile()))
            return false;
    }
    else
    {
        return false;
    }

    kDebug() << "Convert RAW output url: " << outUrl;

    return true;
}


bool GenericTask::startEnfuse(const KUrl::List& inUrls, KUrl& outUrl,
                               const EnfuseSettings& settings,
                               const QString& enfusePath, QString& errors)
{
    QString comp;
    QString ext = KPSaveSettingsWidget::extensionForFormat(settings.outputFormat);

    if (ext == QString(".tif"))
        comp = QString("--compression=DEFLATE");

    outUrl.setFileName(QString(".kipi-expoblending-tmp-") + QString::number(QDateTime::currentDateTime().toTime_t()) + ext);

    enfuseProcess = new KProcess;
    enfuseProcess->clearProgram();
    enfuseProcess->setOutputChannelMode(KProcess::MergedChannels);
    QStringList args;
    args << enfusePath;

    if (!settings.autoLevels)
    {
        args << "-l";
        args << QString::number(settings.levels);
    }

    if (settings.ciecam02)
        args << "-c";

    if (!comp.isEmpty())
        args << comp;

    if (settings.hardMask)
    {
        if (enfuseVersion4x)
            args << "--hard-mask";
        else
            args << "--HardMask";
    }

    if (enfuseVersion4x)
    {
        args << QString("--exposure-weight=%1").arg(settings.exposure);
        args << QString("--saturation-weight=%1").arg(settings.saturation);
        args << QString("--contrast-weight=%1").arg(settings.contrast);
    }
    else
    {
        args << QString("--wExposure=%1").arg(settings.exposure);
        args << QString("--wSaturation=%1").arg(settings.saturation);
        args << QString("--wContrast=%1").arg(settings.contrast);
    }

    args << "-v";
    args << "-o";
    args << outUrl.toLocalFile();

    foreach(const KUrl& url, inUrls)
        args << url.toLocalFile();

    enfuseProcess->setProgram(args);

    kDebug() << "Enfuse command line: " << enfuseProcess->program();

    enfuseProcess->start();

    if (!enfuseProcess->waitForFinished(-1))
    {
        errors = getProcessError(enfuseProcess);
        return false;
    }

    kDebug() << "Enfuse output url: "  << outUrl;
    kDebug() << "Enfuse exit status: " << enfuseProcess->exitStatus();
    kDebug() << "Enfuse exit code:   " << enfuseProcess->exitCode();

    if (enfuseProcess->exitStatus() != QProcess::NormalExit)
        return false;

    if (enfuseProcess->exitCode() == 0)
    {
        // Process finished successfully !
        return true;
    }

    errors = getProcessError(enfuseProcess);
    return false;
}

QString GenericTask::getProcessError(KProcess* const proc) const
{
    if (!proc) return QString();

    QString std = proc->readAll();
    return (i18n("Cannot run %1:\n\n %2", proc->program()[0], std));
}

/**
 * This function obtains the "average scene luminance" (cd/m^2) from an image file.
 * "average scene luminance" is the L (aka B) value mentioned in [1]
 * You have to take a log2f of the returned value to get an EV value.
 *
 * We are using K=12.07488f and the exif-implied value of N=1/3.125 (see [1]).
 * K=12.07488f is the 1.0592f * 11.4f value in pfscalibration's pfshdrcalibrate.cpp file.
 * Based on [3] we can say that the value can also be 12.5 or even 14.
 * Another reference for APEX is [4] where N is 0.3, closer to the APEX specification of 2^(-7/4)=0.2973.
 *
 * [1] http://en.wikipedia.org/wiki/APEX_system
 * [2] http://en.wikipedia.org/wiki/Exposure_value
 * [3] http://en.wikipedia.org/wiki/Light_meter
 * [4] http://doug.kerr.home.att.net/pumpkin/#APEX
 *
 * This function tries first to obtain the shutter speed from either of
 * two exif tags (there is no standard between camera manifacturers):
 * ExposureTime or ShutterSpeedValue.
 * Same thing for f-number: it can be found in FNumber or in ApertureValue.
 *
 * F-number and shutter speed are mandatory in exif data for EV calculation, iso is not.
 */
float GenericTask::getAverageSceneLuminance(const KUrl& url)
{
    KPMetadata meta;
    meta.load(url.toLocalFile());
    if (!meta.hasExif())
        return -1;

    long num = 1, den = 1;

    // default not valid values

    float    expo = -1.0;
    float    iso  = -1.0;
    float    fnum = -1.0;
    QVariant rationals;

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

    if (meta.getExifTagRational("Exif.Photo.FNumber", num, den))
    {
        if (den)
            fnum = (float)(num) / (float)(den);
    }
    else if (getXmpRational("Xmp.exif.FNumber", num, den, meta))
    {
        if (den)
            fnum = (float)(num) / (float)(den);
    }
    else if (meta.getExifTagRational("Exif.Photo.ApertureValue", num, den))
    {
        if (den)
            fnum = (float)(exp(log(2.0) * (float)(num) / (float)(den) / 2.0));
    }
    else if (getXmpRational("Xmp.exif.ApertureValue", num, den, meta))
    {
        if (den)
            fnum = (float)(exp(log(2.0) * (float)(num) / (float)(den) / 2.0));
    }

    kDebug() << url.fileName() << " : fnum = " << fnum;

    // Some cameras/lens DO print the fnum but with value 0, and this is not allowed for ev computation purposes.

    if (fnum == 0.0)
        return -1.0;

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

    kDebug() << url.fileName() << " : iso = " << iso;

    // At this point the three variables have to be != -1

    if (expo != -1.0 && iso != -1.0 && fnum != -1.0)
    {
        float asl = (expo * iso) / (fnum * fnum * 12.07488f);
        kDebug() << url.fileName() << " : ASL ==> " << asl;

        return asl;
    }

    return -1.0;
}

bool GenericTask::getXmpRational(const char* xmpTagName, long& num, long& den, KPMetadata& meta)
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

    return false;
}


}  // namespace KIPIExpoBlendingPlugin
