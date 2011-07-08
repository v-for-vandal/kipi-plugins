/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2011-05-23
 * Description : a plugin to create panorama by fusion of several images.
 * Acknowledge : based on the expoblending plugin
 *
 * Copyright (C) 2011 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
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

#include "actionthread.moc"

// C++ includes

#include <iostream>

// Qt includes

#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QtDebug>
#include <QDateTime>
#include <QFileInfo>
#include <QPointer>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktempdir.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>

// Local includes

#include "kpwriteimage.h"
#include "pluginsversion.h"

namespace KIPIPanoramaPlugin
{

struct ActionThread::ActionThreadPriv
{
    ActionThreadPriv()
        : cancel(false), celeste(false), savePTO(false), autoOptimiseProcess(0), CPFindProcess(0), preprocessingTmpDir(0) {}

    struct Task
    {
        bool                celeste;
        KUrl::List          urls;
        KUrl                outputUrl;
        Action              action;
        RawDecodingSettings rawDecodingSettings;
    };

    bool                            cancel;
    bool                            celeste;
    bool                            savePTO;

    QWaitCondition                  condVar;

    QList<Task*>                    todo;
    QMutex                          todo_mutex;

    KProcess*                       autoOptimiseProcess;
    KProcess*                       CPFindProcess;

    /**
     * A list of all running raw instances. Only access this variable via
     * <code>mutex</code>.
     */
    QList<QPointer<KDcraw> >         rawProcesses;

    KTempDir*                        preprocessingTmpDir;

    RawDecodingSettings              rawDecodingSettings;

    void cleanPreprocessingTmpDir()
    {
        if (preprocessingTmpDir)
        {
            preprocessingTmpDir->unlink();
            delete preprocessingTmpDir;
            preprocessingTmpDir = 0;
        }
    }
};

ActionThread::ActionThread(QObject* parent)
            : QThread(parent), d(new ActionThreadPriv)
{
    qRegisterMetaType<ActionData>();
}

ActionThread::~ActionThread()
{

    kDebug() << "ActionThread shutting down."
             << "Canceling all actions and waiting for them";

    // cancel the thread
    cancel();
    // wait for the thread to finish
    wait();

    kDebug() << "Thread finished";

    d->cleanPreprocessingTmpDir();

    cleanUpResultFiles();

    delete d;
}

void ActionThread::setPreProcessingSettings(bool celeste, const KDcrawIface::RawDecodingSettings& settings)
{
    d->celeste             = celeste;
    d->rawDecodingSettings = settings;
}

void ActionThread::preProcessFiles(const KUrl::List& urlList)
{
    ActionThreadPriv::Task* t   = new ActionThreadPriv::Task;
    t->action                   = PREPROCESS;
    t->urls                     = urlList;
    t->rawDecodingSettings      = d->rawDecodingSettings;
    t->celeste                  = d->celeste;

    QMutexLocker lock(&d->todo_mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ActionThread::optimizeProject()
{
    ActionThreadPriv::Task* t   = new ActionThreadPriv::Task;
    t->action                   = OPTIMIZE;

    QMutexLocker lock(&d->todo_mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ActionThread::cancel()
{
    QMutexLocker lock(&d->todo_mutex);
    d->todo.clear();
    d->cancel = true;

    if (d->autoOptimiseProcess)
        d->autoOptimiseProcess->kill();

    if (d->CPFindProcess)
        d->CPFindProcess->kill();

    foreach (QPointer<KDcraw> rawProcess, d->rawProcesses)
    {
        if (rawProcess)
        {
            rawProcess->cancel();
        }
    }

    d->condVar.wakeAll();
}

void ActionThread::cleanUpResultFiles()
{
}

void ActionThread::run()
{
    d->cancel = false;
    while (!d->cancel)
    {
        ActionThreadPriv::Task* t = 0;
        {
            QMutexLocker lock(&d->todo_mutex);
            if (!d->todo.isEmpty())
                t = d->todo.takeFirst();
            else
                d->condVar.wait(&d->todo_mutex);
        }

        if (t)
        {
            switch (t->action)
            {
                case PREPROCESS:
                {
                    ActionData ad1;
                    ad1.action   = PREPROCESS;
                    ad1.inUrls   = t->urls;
                    ad1.starting = true;
                    emit starting(ad1);

                    ItemUrlsMap preProcessedUrlsMap;
                    QString     errors;

                    bool result  = startPreProcessing(t->urls, preProcessedUrlsMap, t->celeste, t->rawDecodingSettings, errors);

                    ActionData ad2;
                    ad2.action              = PREPROCESS;
                    ad2.inUrls              = t->urls;
                    ad2.preProcessedUrlsMap = preProcessedUrlsMap;
                    ad2.success             = result;
                    ad2.message             = errors;
                    emit finished(ad2);
                    break;
                }

                case OPTIMIZE:
                {
                    ActionData ad1;
                    ad1.action   = OPTIMIZE;
                    ad1.starting = true;
                    emit starting(ad1);

                    QString errors;
                    bool    result = false;

// TODO             result  = startPreProcessing(errors);

                    ActionData ad2;
                    ad2.action   = OPTIMIZE;
                    ad2.success  = result;
                    ad2.message  = errors;
                    emit finished(ad2);
                    break;
                }

                default:
                {
                    qCritical() << "Unknown action specified" << endl;
                }
            }
        }

        delete t;
    }
}

bool ActionThread::startPreProcessing(const KUrl::List& inUrls, ItemUrlsMap& preProcessedUrlsMap,
                                      bool celeste, const RawDecodingSettings& settings,
                                      QString& errors)
{
    QString prefix = KStandardDirs::locateLocal("tmp", QString("kipi-panorama-preprocessing-tmp-") +
                                                       QString::number(QDateTime::currentDateTime().toTime_t()));

    d->cleanPreprocessingTmpDir();

    d->preprocessingTmpDir = new KTempDir(prefix);

    // Pre-process RAW files if necessary.

    KUrl::List mixedUrls;     // Original non-RAW + Raw converted urls to align.

    volatile bool error = false;

#pragma omp parallel for
    for (int i = 0; i < inUrls.size(); ++i)
    {

        if (error)
        {
            continue;
        }

        KUrl url = inUrls.at(i);

        if (isRawFile(url.toLocalFile()))
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

            emit stepFinished();

#pragma omp critical (listAppend)
            {
                mixedUrls.append(preprocessedUrl);
                // In case of alignment is not performed.
                preProcessedUrlsMap.insert(url, ItemPreprocessedUrls(preprocessedUrl, previewUrl));
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

            emit stepFinished();

#pragma omp critical (listAppend)
            {
                mixedUrls.append(url);
                // In case of alignment is not performed.
                preProcessedUrlsMap.insert(url, ItemPreprocessedUrls(url, previewUrl));
            }
        }
    }

    if (error)
    {
        return false;
    }

    // Run CPFind to get control points and order the images

    KUrl ptoUrl;
    if (!createPTO(mixedUrls, ptoUrl))
        return false;

    d->CPFindProcess = new KProcess();
    d->CPFindProcess->clearProgram();
    d->CPFindProcess->clearEnvironment();
    d->CPFindProcess->setWorkingDirectory(d->preprocessingTmpDir->name());
    d->CPFindProcess->setOutputChannelMode(KProcess::MergedChannels);

    QStringList args;
    args << "cpfind";
    if (celeste)
        args << "--celeste";
    args << "-o";
    args << "cp_pano.pto";
    args << ptoUrl.toLocalFile();

    d->CPFindProcess->setProgram(args);

    kDebug() << "CPFind command line: " << d->CPFindProcess->program();

    d->CPFindProcess->start();

    if (!d->CPFindProcess->waitForFinished(-1) || d->CPFindProcess->exitCode() != 0)
    {
        errors = getProcessError(d->CPFindProcess);
        delete d->CPFindProcess;
        return false;
    }

    // Copy the file for future use
    if (d->savePTO)
    {
        // TODO
    }

    emit stepFinished();

    delete d->CPFindProcess;
    return true;
}

bool ActionThread::startOptimization(QString& errors)
{
    KUrl ptoInUrl   = d->preprocessingTmpDir->name();
    ptoInUrl.setFileName(QString("cp_pano.pto"));
    KUrl ptoOut1Url = d->preprocessingTmpDir->name();
    ptoOut1Url.setFileName(QString("auto_op_pano.pto"));
    KUrl ptoOut2Url = d->preprocessingTmpDir->name();
    ptoOut2Url.setFileName(QString("vig_op_pano.pto"));

    d->autoOptimiseProcess = new KProcess();
    d->autoOptimiseProcess->clearProgram();
    d->autoOptimiseProcess->clearEnvironment();
    d->autoOptimiseProcess->setWorkingDirectory(d->preprocessingTmpDir->name());
    d->autoOptimiseProcess->setOutputChannelMode(KProcess::MergedChannels);

    QStringList args;
    args << "autooptimiser";
    args << "-o";
    args << ptoInUrl.toLocalFile();
    args << ptoOut1Url.toLocalFile();

    d->autoOptimiseProcess->setProgram(args);

    kDebug() << "autooptimiser command line: " << d->autoOptimiseProcess->program();

    d->autoOptimiseProcess->start();

    if (!d->autoOptimiseProcess->waitForFinished(-1) || d->autoOptimiseProcess->exitCode() != 0)
    {
        errors = getProcessError(d->autoOptimiseProcess);
        delete d->autoOptimiseProcess;
        return false;
    }

    emit stepFinished();

    delete d->autoOptimiseProcess;
    return true;
}

bool ActionThread::computePreview(const KUrl& inUrl, KUrl& outUrl)
{
    outUrl = d->preprocessingTmpDir->name();
    QFileInfo fi(inUrl.toLocalFile());
    outUrl.setFileName(QString(".") + fi.completeBaseName().replace(".", "_") + QString("-preview.jpg"));

    QImage img;
    if (img.load(inUrl.toLocalFile()))
    {
        QImage preview = img.scaled(1280, 1024, Qt::KeepAspectRatio);
        bool saved = preview.save(outUrl.toLocalFile(), "JPEG");
        // save exif information also to preview image for auto rotation
        if (saved)
        {
            KExiv2 metai(inUrl.toLocalFile());
            KExiv2 metao(outUrl.toLocalFile());
            metao.setImageOrientation(metai.getImageOrientation());
            metao.applyChanges();
        }
        kDebug() << "Preview Image url: " << outUrl << ", saved: " << saved;
        return saved;
    }
    return false;
}

bool ActionThread::convertRaw(const KUrl& inUrl, KUrl& outUrl, const RawDecodingSettings& settings)
{
    int        width, height, rgbmax;
    QByteArray imageData;

    QPointer<KDcraw> rawdec = new KDcraw;

    {
        QMutexLocker lock(&d->todo_mutex);
        d->rawProcesses << rawdec;
    }

    bool decoded = rawdec->decodeRAWImage(inUrl.toLocalFile(), settings, imageData, width, height, rgbmax);

    {
        QMutexLocker lock(&d->todo_mutex);
        d->rawProcesses.removeAll(rawdec);
    }

    if (decoded)
    {
        uchar* sptr  = (uchar*)imageData.data();
        float factor = 65535.0 / rgbmax;
        unsigned short tmp16[3];

        // Set RGB color components.
        for (int i = 0 ; !d->cancel && (i < width * height) ; i++)
        {
            // Swap Red and Blue and re-ajust color component values
            tmp16[0] = (unsigned short)((sptr[5]*256 + sptr[4]) * factor);      // Blue
            tmp16[1] = (unsigned short)((sptr[3]*256 + sptr[2]) * factor);      // Green
            tmp16[2] = (unsigned short)((sptr[1]*256 + sptr[0]) * factor);      // Red
            memcpy(&sptr[0], &tmp16[0], 6);
            sptr += 6;
        }

        KExiv2 meta;
        meta.load(inUrl.toLocalFile());
        meta.setImageProgramId(QString("Kipi-plugins"), QString(kipiplugins_version));
        meta.setImageDimensions(QSize(width, height));
        meta.setExifTagString("Exif.Image.DocumentName", inUrl.fileName());
        meta.setXmpTagString("Xmp.tiff.Make",  meta.getExifTagString("Exif.Image.Make"));
        meta.setXmpTagString("Xmp.tiff.Model", meta.getExifTagString("Exif.Image.Model"));
        meta.setImageOrientation(KExiv2Iface::KExiv2::ORIENTATION_NORMAL);

        QByteArray prof = KPWriteImage::getICCProfilFromFile(settings.outputColorSpace);

        KPWriteImage wImageIface;
        wImageIface.setCancel(&d->cancel);
        wImageIface.setImageData(imageData, width, height, true, false, prof, meta);
        outUrl = d->preprocessingTmpDir->name();
        QFileInfo fi(inUrl.toLocalFile());
        outUrl.setFileName(QString(".") + fi.completeBaseName().replace(".", "_") + QString(".tif"));

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

bool ActionThread::isRawFile(const KUrl& url)
{
    QString rawFilesExt(KDcraw::rawFiles());

    QFileInfo fileInfo(url.toLocalFile());
    if (rawFilesExt.toUpper().contains(fileInfo.suffix().toUpper()))
        return true;

    return false;
}

bool ActionThread::createPTO(const KUrl::List& urlList, KUrl& ptoUrl)
{
    ptoUrl = d->preprocessingTmpDir->name();
    ptoUrl.setFileName(QString("pano_base.pto"));

    QFile pto(ptoUrl.toLocalFile());
    if (pto.exists())
        return false;
    if (!pto.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;

    QTextStream pto_stream(&pto);

    foreach (KUrl url, urlList)
    {
        KExiv2 meta;
        meta.load(url.toLocalFile());
        QSize size = meta.getImageDimensions();

        pto_stream << "i w" << size.width() << " h" << size.height() << " n" << url.toLocalFile() << endl;
    }

    pto.close();

    return true;
}

QString ActionThread::getProcessError(KProcess* proc) const
{
    if (!proc) return QString();

    QString std = proc->readAll();
    return (i18n("Cannot run %1:\n\n %2", proc->program()[0], std));
}

}  // namespace KIPIPanoramaPlugin