/* ============================================================
 * 
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2013-06-28
 * Description : a class to manage plugin actions using threads
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

#include "task.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QFileInfo>

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
// LibKDcraw includes

#include <libkdcraw/kdcraw.h>

//Local includes
#include "actions.h"
#include "kpmetadata.h"

namespace KIPIExpoBlendingPlugin
{

class Task::ActionThreadPriv
{
  ActionThreadPriv()
  {
    align = false;
    cancel = false;
    
  }
  
    bool                cancel;
    KUrl::List          urls;
    Action              action;
    
    RawDecodingSettings rawDecodingSettings;
    
    KUrl                outputUrl;
    QString             binaryPath;
    
  
    EnfuseSettings      enfuseSettings;		//enfusebinary
    
    
    KProcess*                        enfuseProcess;
    KProcess*                        alignProcess;
  
};
  
Task::Task(QObject* const parent, const KUrl::List& fileUrl, const Action& action)
    : Job(parent),successFlag(false), isAbortedFlag(false), tmpDir(workDir), d(new ActionThreadPriv)
{
    d->action = action;
    d->urls = fileUrl;
    
    d->rawDecodingSettings = rawDecodingSettings;
    d->binaryPath = binaryPath;
    d->enfuseSettings = enfuseSettings;
    d->outputUrl = outputUrl;
}

Task::~Task()
{
}

bool Task::success() const
{
    return successFlag;
}

void Task::requestAbort()
{
    isAbortedFlag = true;
}

void Task::setPreProcessingSettings(bool align, const RawDecodingSettings& settings)
{
    d->align               = align;
    d->rawDecodingSettings = settings;
        
}

void Task::run()
{
    d->cancel = false;
    
    while (!d->cancel)
    {
 
            switch (d->action)
            {
                case IDENTIFY:
                {
                    // Identify Exposure.

                    QString avLum;

                    if (!d->urls.isEmpty())
                    {
                        float val = getAverageSceneLuminance(d->urls[0].toLocalFile());
                        if (val != -1)
                            avLum.setNum(log2f(val), 'g', 2);
                    }

                    ActionData ad;
                    ad.action  = d->action;
                    ad.inUrls  = d->urls;
                    ad.message = avLum.isEmpty() ? i18n("unknown") : avLum;
                    ad.success = avLum.isEmpty();
                    emit finished(ad);
                    break;
                }

                case PREPROCESSING:
                {
                    ActionData ad1;
                    ad1.action   = PREPROCESSING;
                    ad1.inUrls   = d->urls;
                    ad1.starting = true;
                    emit starting(ad1);

                    ItemUrlsMap preProcessedUrlsMap;
                    QString     errors;

                    bool result  = startPreProcessing(d->urls, preProcessedUrlsMap, d->align, d->rawDecodingSettings, d->binaryPath, errors);

                    ActionData ad2;
                    ad2.action              = PREPROCESSING;
                    ad2.inUrls              = d->urls;
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
                    ad1.inUrls   = d->urls;
                    ad1.starting = true;
                    emit starting(ad1);

                    QImage image;
                    bool result  = image.load(d->urls[0].toLocalFile());

                    // rotate image
                    if (result)
                    {
                        KPMetadata meta(d->urls[0].toLocalFile());
                        meta.rotateExifQImage(image, meta.getImageOrientation());
                    }

                    ActionData ad2;
                    ad2.action         = LOAD;
                    ad2.inUrls         = d->urls;
                    ad2.success        = result;
                    ad2.image          = image;
                    emit finished(ad2);
                    break;
                }

                case ENFUSEPREVIEW:
                {
                    ActionData ad1;
                    ad1.action         = ENFUSEPREVIEW;
                    ad1.inUrls         = d->urls;
                    ad1.starting       = true;
                    ad1.enfuseSettings = d->enfuseSettings;
                    emit starting(ad1);

                    QString errors;
                    KUrl    destUrl         = d->outputUrl;
                    EnfuseSettings settings = d->enfuseSettings;
                    settings.outputFormat   = KPSaveSettingsWidget::OUTPUT_JPEG;    // JPEG for preview: fast and small.
                    bool result             = startEnfuse(d->urls, destUrl, settings, d->binaryPath, errors);

                    kDebug() << "Preview result was: " << result;

                    // preserve exif information for auto rotation
                    if (result)
                    {
                        KPMetadata metaIn(t->urls[0].toLocalFile());
                        KPMetadata metaOut(destUrl.toLocalFile());
                        metaOut.setImageOrientation(metaIn.getImageOrientation());
                        metaOut.applyChanges();
                    }

                    ActionData ad2;
                    ad2.action         = ENFUSEPREVIEW;
                    ad2.inUrls         = d->urls;
                    ad2.outUrls        = KUrl::List() << destUrl;
                    ad2.success        = result;
                    ad2.message        = errors;
                    ad2.enfuseSettings = d->enfuseSettings;
                    emit finished(ad2);
                    break;
                }

                case ENFUSEFINAL:
                {
                    ActionData ad1;
                    ad1.action         = ENFUSEFINAL;
                    ad1.inUrls         = d->urls;
                    ad1.starting       = true;
                    ad1.enfuseSettings = d->enfuseSettings;
                    emit starting(ad1);

                    KUrl    destUrl = d->outputUrl;
                    bool    result  = false;
                    QString errors;

                    result = startEnfuse(d->urls, destUrl, d->enfuseSettings, d->binaryPath, errors);

                    // We will take first image metadata from stack to restore Exif, Iptc, and Xmp.
                    KPMetadata meta;
                    meta.load(d->urls[0].toLocalFile());
                    result = result & meta.setXmpTagString("Xmp.kipi.EnfuseInputFiles", d->enfuseSettings.inputImagesList(), false);
                    result = result & meta.setXmpTagString("Xmp.kipi.EnfuseSettings", d->enfuseSettings.asCommentString().replace('\n', " ; "), false);
                    meta.setImageDateTime(QDateTime::currentDateTime());
                    if (d->enfuseSettings.outputFormat != KPSaveSettingsWidget::OUTPUT_JPEG)
                    {
                        QImage img;
                        if (img.load(destUrl.toLocalFile()))
                            meta.setImagePreview(img.scaled(1280, 1024, Qt::KeepAspectRatio));
                    }
                    meta.save(destUrl.toLocalFile());

                    

                    ActionData ad2;
                    ad2.action         = ENFUSEFINAL;
                    ad2.inUrls         = d->urls;
                    ad2.outUrls        = KUrl::List() << destUrl;
                    ad2.success        = result;
                    ad2.message        = errors;
                    ad2.enfuseSettings = d->enfuseSettings;
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

bool Task::startEnfuse(const KUrl::List& inUrls, KUrl& outUrl,
                               const EnfuseSettings& settings,
                               const QString& enfusePath, QString& errors)
{
    QString comp;
    QString ext = KPSaveSettingsWidget::extensionForFormat(settings.outputFormat);

    if (ext == QString(".tif"))
        comp = QString("--compression=DEFLATE");

    outUrl.setFileName(QString(".kipi-expoblending-tmp-") + QString::number(QDateTime::currentDateTime().toTime_t()) + ext);

    d->enfuseProcess = new KProcess;
    d->enfuseProcess->clearProgram();
    d->enfuseProcess->setOutputChannelMode(KProcess::MergedChannels);
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
        if (d->enfuseVersion4x)
            args << "--hard-mask";
        else
            args << "--HardMask";
    }

    if (d->enfuseVersion4x)
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

    d->enfuseProcess->setProgram(args);

    kDebug() << "Enfuse command line: " << d->enfuseProcess->program();

    d->enfuseProcess->start();

    if (!d->enfuseProcess->waitForFinished(-1))
    {
        errors = getProcessError(d->enfuseProcess);
        return false;
    }

    kDebug() << "Enfuse output url: "  << outUrl;
    kDebug() << "Enfuse exit status: " << d->enfuseProcess->exitStatus();
    kDebug() << "Enfuse exit code:   " << d->enfuseProcess->exitCode();

    if (d->enfuseProcess->exitStatus() != QProcess::NormalExit)
        return false;

    if (d->enfuseProcess->exitCode() == 0)
    {
        // Process finished successfully !
        return true;
    }

    errors = getProcessError(d->enfuseProcess);
    return false;
}

QString Task::getProcessError(KProcess* const proc) const
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
float Task::getAverageSceneLuminance(const KUrl& url)
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

bool Task::getXmpRational(const char* xmpTagName, long& num, long& den, KPMetadata& meta)
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

QString Task::getProcessError(KProcess& proc)
{
    QString std = proc.readAll();
    return (i18n("Cannot run %1:\n\n %2", proc.program()[0], std));
}

bool Task::startPreProcessing(const KUrl::List& inUrls, ItemUrlsMap& preProcessedMap,
                                      bool align, const RawDecodingSettings& rawSettings,
                                      const QString& alignPath, QString& errors)
{
    d->cleanPreprocessingTmpDir();

    QString prefix = KStandardDirs::locateLocal("tmp", QString("kipi-panorama-tmp-") +
                                                       QString::number(QDateTime::currentDateTime().toTime_t()));

    d->preprocessingTmpDir = new KTempDir(prefix);

    JobCollection       *jobs           = new JobCollection();

    // TODO: try to append these jobs as a JobCollection inside a JobSequence
    int id = 0;
    
    QVector<PreProcessTask*> preProcessingTasks;
    
    foreach (const KUrl& file, inUrls)
    {
        preProcessedMap.insert(file, ItemPreprocessedUrls());

        PreProcessTask *t = new PreProcessTask(d->preprocessingTmpDir->name(),
                                               id++,
                                               preProcessedMap[file],
                                               file,
                                               rawSettings);

        connect(t, SIGNAL(started(ThreadWeaver::Job*)),
                this, SLOT(slotStarting(ThreadWeaver::Job*)));
        connect(t, SIGNAL(done(ThreadWeaver::Job*)),
                this, SLOT(slotStepDone(ThreadWeaver::Job*)));

        preProcessingTasks.append(t);
        jobs->addJob(t);
    }
    
    // Pre-process RAW files if necessary. Parallelized with OpemMP if available.

    KUrl::List mixedUrls;     // Original non-RAW + Raw converted urls to align.


    /*if (!successFlag)
    {
        return false;
    }*/

    if (align)
    {
        // Re-align images

        d->alignProcess = new KProcess;
        d->alignProcess->clearProgram();
        d->alignProcess->setWorkingDirectory(d->preprocessingTmpDir->name());
        d->alignProcess->setOutputChannelMode(KProcess::MergedChannels);

        QStringList args;
        args << alignPath;
        args << "-v";
        args << "-a";
        args << "aligned";

        /*foreach(const KUrl& url, mixedUrls)
        {
            args << url.toLocalFile();
        }*/

        d->alignProcess->setProgram(args);

        kDebug() << "Align command line: " << d->alignProcess->program();

        d->alignProcess->start();

        if (!d->alignProcess->waitForFinished(-1))
        {
            errors = getProcessError(d->alignProcess);
            return false;
        }

        uint    i=0;
        QString temp;
        preProcessedUrlsMap.clear();

        foreach(const KUrl& url, inUrls)
        {
            KUrl previewUrl;
            KUrl alignedUrl = KUrl(d->preprocessingTmpDir->name() + temp.sprintf("aligned%04i", i) + QString(".tif"));
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
        kDebug() << "Align exit status    : "         << d->alignProcess->exitStatus();
        kDebug() << "Align exit code      : "         << d->alignProcess->exitCode();

        if (d->alignProcess->exitStatus() != QProcess::NormalExit)
            return false;

        if (d->alignProcess->exitCode() == 0)
        {
            // Process finished successfully !
            return true;
        }

        errors = getProcessError(d->alignProcess);
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



}  // namespace KIPIExpoBlendingPlugin
