/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
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

#include "actionthread.moc"


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

//####################### THREADWEAVER LIBRARIES ##########################
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/JobCollection.h>
#include <threadweaver/DependencyPolicy.h>


// LibKDcraw includes

#include <libkdcraw/kdcraw.h>

// Local includes

#include "task.h"
#include "tasks.h"
#include "kpmetadata.h"
#include "kpversion.h"
#include "kpwriteimage.h"

using namespace ThreadWeaver;
using namespace KIPIPlugins;

namespace KIPIExpoBlendingPlugin
{

class ActionThread::ActionThreadPriv
{
public:

    ActionThreadPriv()
    {
        align               = true;
        cancel              = false;
        enfuseVersion4x     = true;
        preprocessingTmpDir = 0;
    }

   
  
    bool                             cancel;
    bool                             align;
    bool                             enfuseVersion4x;

    KTempDir*                        preprocessingTmpDir;

  
    /**
     * List of results files produced by enfuse that may need cleaning.
     * Only access this through the provided mutex.
     */
    KUrl::List                       enfuseTmpUrls;
    QMutex                           enfuseTmpUrlsMutex;

    
    // ####################### SAME AS PANORAMA ################################
    void cleanAlignTmpDir()
    {
        if (preprocessingTmpDir)
        {
            preprocessingTmpDir->unlink();
            delete preprocessingTmpDir;
            preprocessingTmpDir = 0;
        }
    }
};

// ########################### SAME AS PANORAMA ####################
ActionThread::ActionThread(QObject* const parent)
    : RActionThreadBase(parent), d(new ActionThreadPriv)
{
    qRegisterMetaType<ActionData>();
}
ActionThread::~ActionThread()
{

    kDebug() << "ActionThread shutting down."
             << "Canceling all actions and waiting for them";

    /*cancel the thread
    cancel();
    wait for the thread to finish
    wait();
    */
    
    kDebug() << "Thread finished";

    d->cleanAlignTmpDir();

   // cleanUpResultFiles();

    delete d;
}

void ActionThread::setEnfuseVersion(const double version)
{
    d->enfuseVersion4x = (version >= 4.0);
}


void ActionThread::identifyFiles(const KUrl::List& urlList)
{

  
    JobCollection* const jobs = new JobCollection();
    
   
    
    for (KUrl::List::const_iterator it = urlList.constBegin(); it != urlList.constEnd(); ++it)
    {
	
	KUrl url = *it;
	KUrl::List tempList;
	tempList.append(url);
	
        Task* const t = new Task(this, tempList, IDENTIFY);

	connect(t, SIGNAL(started(ThreadWeaver::Job*)),
                this, SLOT(slotStarting(ThreadWeaver::Job*)));
       
	connect(t, SIGNAL(done(ThreadWeaver::Job*)),
                this, SLOT(slotStepDone(ThreadWeaver::Job*)));

      
        jobs->addJob(t);
    }
    appendJob(jobs);
    
}

void ActionThread::loadProcessed(const KUrl& url)
{
  
    JobCollection* const jobs = new JobCollection();
     KUrl::List tempList;
    
    tempList.append(url);
    
    Task* const t = new Task(this, tempList , LOAD);
   
    connect(t, SIGNAL(started(ThreadWeaver::Job*)),
            this, SLOT(slotStarting(ThreadWeaver::Job*)));
    connect(t, SIGNAL(done(ThreadWeaver::Job*)),
            this, SLOT(slotStepDone(ThreadWeaver::Job*)));
   
    
    jobs->addJob(t);
    appendJob(jobs);
}


//#####################   SAME AS GENERATE PANORAMA PREVIEW ##################
void ActionThread::enfusePreview(const KUrl::List& alignedUrls, const KUrl& outputUrl,
                                 const EnfuseSettings& settings, const QString& enfusePath)
{
  
    JobCollection   *jobs  = new JobCollection();
   
    Task* const t = new Task(this, alignedUrls, ENFUSEPREVIEW);
    
    t->outputUrl              = outputUrl;
    t->enfuseSettings         = settings;
    t->binaryPath             = enfusePath;

    
    connect(t, SIGNAL(started(ThreadWeaver::Job*)),
            this, SLOT(slotStarting(ThreadWeaver::Job*)));
    connect(t, SIGNAL(done(ThreadWeaver::Job*)),
            this, SLOT(slotStepDone(ThreadWeaver::Job*)));
   
    
    jobs->addJob(t);

    appendJob(jobs);
}

void ActionThread::enfuseFinal(const KUrl::List& alignedUrls, const KUrl& outputUrl,
                               const EnfuseSettings& settings, const QString& enfusePath)
{
  
    JobCollection   *jobs  = new JobCollection();
    
    Task* const t = new Task(this, alignedUrls, ENFUSEFINAL);
    
    t->outputUrl              = outputUrl;
    t->enfuseSettings         = settings;
    t->binaryPath             = enfusePath;
    
    connect(t, SIGNAL(started(ThreadWeaver::Job*)),
            this, SLOT(slotStarting(ThreadWeaver::Job*)));
    connect(t, SIGNAL(done(ThreadWeaver::Job*)),
            this, SLOT(slotStepDone(ThreadWeaver::Job*)));
   
    
    jobs->addJob(t);

    appendJob(jobs);
   
}

// ####################### SAME AS PANORAMA #################################



void ActionThread::preProcessFiles(const KUrl::List& urlList, const QString& alignPath, 
				   ItemUrlsMap& preProcessedMap,
                                   const RawDecodingSettings& rawSettings,
                                   bool align)
{
    
    d->cleanAlignTmpDir();

    JobCollection       *jobs           = new JobCollection();

    int id = 0;
    
    
    Task* const t = new Task(this, urlList, PREPROCESSING);
  
    t->rawDecodingSettings = rawSettings ;
    t->align = align ;
    t->binaryPath = alignPath ;

    connect(t, SIGNAL(started(ThreadWeaver::Job*)),
                this, SLOT(slotStarting(ThreadWeaver::Job*)));
    connect(t, SIGNAL(done(ThreadWeaver::Job*)),
                this, SLOT(slotStepDone(ThreadWeaver::Job*)));
    
    jobs->addJob(t);
    appendJob(jobs);
    
}

void ActionThread::slotStarting(Job* j)
{
    Task *t = static_cast<Task*>(j);

    ActionData ad;
    ad.starting     = true;
    ad.action       = t->action;


    emit starting(ad);
}

void ActionThread::slotStepDone(Job* j)
{
    Task *t = static_cast<Task*>(j);

    ActionData ad;
    ad.starting     = false;
    ad.action       = t->action;
    

    emit stepFinished(ad);

    ((QObject*) t)->deleteLater();
}

void ActionThread::slotDone(Job* j)
{
    Task *t = static_cast<Task*>(j);

    ActionData ad;
    ad.starting     = false;
    ad.action       = t->action;
   
    emit finished(ad);

    ((QObject*) t)->deleteLater();
}


/*
void ActionThread::cancel()
{
    QMutexLocker lock(&d->mutex);
    d->todo.clear();
    d->cancel = true;

    if (d->enfuseProcess)
        d->enfuseProcess->kill();

    if (d->alignProcess)
        d->alignProcess->kill();

    
    d->condVar.wakeAll();
}*/

/*
void ActionThread::cleanUpResultFiles()
{
    // Cleanup all tmp files created by Enfuse process.
    QMutexLocker(&d->enfuseTmpUrlsMutex);
    foreach(const KUrl& url, d->enfuseTmpUrls)
    {
        kDebug() << "Removing temp file " << url.toLocalFile();
        KTempDir::removeDir(url.toLocalFile());
    }
    d->enfuseTmpUrls.clear();
}
*/

}  // namespace KIPIExpoBlendingPlugin
