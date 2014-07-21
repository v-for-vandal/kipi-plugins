/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2014-07-21
 * @brief  Re-implemented class to get background processing from DNGConverter 
 *
 * @author Copyright (C) 2008-2012 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Shourya Singh Gupta
 *         <a href="mailto:shouryasgupta at gmail dot com">shouryasgupta at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "plugin_gettask.h"

#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QTreeWidgetItemIterator>

//kde includes

#include <kurl.h>
#include <kdebug.h>
#include <kde_file.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/Job.h>
#include "kio/job.h"
#include "kio/jobclasses.h"
#include <kurl.h>

// Local includes

#include "task.h"
#include "actions.h"
#include "kpimageinfo.h"
#include "kpmetadata.h"
#include "plugin_getwidget.h"
#include "settingswidget.h"

// libkipi includes

#include "libkipi/interface.h"

namespace KIPIDNGConverterPlugin
{

class Plugin_GetTask::Private
{
public:

    Private():
    interface(0)
    {
    }
    
    Interface* interface;
    
};

Plugin_GetTask::Plugin_GetTask(Interface* iface)
    : BkgTask(),d(new Private)
{
    d->interface = iface;
    qRegisterMetaType<KIPIDNGConverterPlugin::ActionData>("KIPIDNGConverterPlugin::ActionData");
}

Plugin_GetTask::~Plugin_GetTask()
{
    delete d;
}

void Plugin_GetTask::getTask()
{
    BkgTask* set     = d->interface->setTask();
    KUrl::List imgs  = set->getImgUrls();
    JobCollection* const collection = new JobCollection();
    
    long i = 0;
    KUrl::List::const_iterator  it1 = imgs.constBegin();
    while((it1+i)!= imgs.constEnd())
    {
        KUrl img = *(it1+i);
	Task*      task = new Task(0,img,PROCESS);
	connect(task, SIGNAL(signalFinished(KIPIDNGConverterPlugin::ActionData)),
            this, SLOT(slotFinished(KIPIDNGConverterPlugin::ActionData)),Qt::QueuedConnection);
	collection->addJob(task);
	i++;
    }
    
    set->setTask(collection);
}

void Plugin_GetTask::slotFinished(const KIPIDNGConverterPlugin::ActionData& ad)
{  
    if(ad.action == PROCESS)
        processed(ad.fileUrl, ad.destPath);
}

void Plugin_GetTask::processed(const KUrl& url, const QString& tmpFile)
{
    KUrl dest = d->interface->getdestURl(url);
    QString destFile = dest.pathOrUrl();
    SettingsWidget* settingsBox = Plugin_GetWidget::settingsInstance();
    
    if (settingsBox->conflictRule() != SettingsWidget::OVERWRITE)
    {
        struct stat statBuf;

        if (::stat(QFile::encodeName(destFile), &statBuf) == 0)
        {
            kDebug()<<"Failed to save image";
        }
    }
    
    if (!destFile.isEmpty())
    {
        if ( KIPIPlugins::KPMetadata::hasSidecar(tmpFile))
        {
            if (! KIPIPlugins::KPMetadata::moveSidecar(KUrl(tmpFile), KUrl(destFile)))
            {
                kDebug()<<"Failed to move sidecar";
            }
        }  
      
        if (KDE::rename(QFile::encodeName(tmpFile), QFile::encodeName(destFile)) != 0)
            kDebug()<<"Failed to save image.";
	else
        {
            KIPIPlugins::KPImageInfo info(url);
            info.cloneData(KUrl(destFile));    
        }
    }
}

} // namespace KIPIDNGConverterPlugin
