/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2008-09-24
 * Description : a kipi plugin to convert Raw file to DNG
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "plugin_dngconverter.moc"

//Qt includes
#include <QFile>
 #include <QFileInfo>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kwindowsystem.h>
#include <kde_file.h>

// LibKIPI includes

#include <libkipi/interface.h>
#include <libkipi/imagecollection.h>

// Local includes
#include "actionthread.h"
#include "task.h"
#include "actions.h"
#include "aboutdata.h"
#include "batchdialog.h"
#include "kpimageinfo.h"
#include "kpmetadata.h"
#include "dngwriter.h"
#include "plugin_getwidget.h"
#include "plugin_gettask.h"
#include "settingswidget.h"

namespace KIPIDNGConverterPlugin
{

K_PLUGIN_FACTORY( RawConverterFactory, registerPlugin<Plugin_DNGConverter>(); )
K_EXPORT_PLUGIN ( RawConverterFactory("kipiplugin_dngconverter") )

Plugin_DNGConverter::Plugin_DNGConverter(QObject* const parent, const QVariantList&)
    : EmbeddablePlugin( RawConverterFactory::componentData(), parent, "DNGConverter")
{
    kDebug(AREA_CODE_LOADING) << "Plugin_DNGConverter plugin loaded" ;

    setUiBaseName("kipiplugin_dngconverterui.rc");
    setupXML();
    
    m_settingswidget = new SettingsWidget(0);
    //connect(m_settingswidget, SIGNAL(settingsChanged(QString,QMap<QString, QVariant>)),
    //        interface(), SLOT(settingsChanged(QString,QMap<QString, QVariant>)));
    
    connect(m_settingswidget, SIGNAL(settingsChanged(QString,QMap<QString, QVariant>)),
            this, SLOT(settingsChanged(QString,QMap<QString, QVariant>)));
    
    qRegisterMetaType<KIPIDNGConverterPlugin::ActionData>("KIPIDNGConverterPlugin::ActionData");
}

Plugin_DNGConverter::~Plugin_DNGConverter()
{
}

QWidget* Plugin_DNGConverter::getWidget()
{
    return m_settingswidget;  
}

QString Plugin_DNGConverter::outputSuffix()
{
    return QString("dng");  
}

QMap<QString, QVariant> Plugin_DNGConverter::defaultSettings()
{
    QMap<QString, QVariant> ts;
    ts.insert("previewMode",1);
    ts.insert("compressLossLess",true);
    ts.insert("updateFileDate",false);
    ts.insert("backupOriginalRawFile",false);
    ts.insert("ConflictRule",0);
    return ts;
}

void Plugin_DNGConverter::assignSettings(QMap<QString, QVariant> settings)
{
    m_settingswidget->setConflictRule((SettingsWidget::ConflictRule)(settings["ConflictRule"].toInt()));
    m_settingswidget->setCompressLossLess(settings["compressLossLess"].toBool());
    m_settingswidget->setPreviewMode(settings["previewMode"].toInt());
    m_settingswidget->setBackupOriginalRawFile(settings["setBackupOriginalRawFile"].toBool());    
}

void Plugin_DNGConverter::startTask(KUrl img)
{
    ActionThread* thread = new ActionThread(0);
    JobCollection* const collection = new JobCollection();
    
    Task* const t = new Task(0, img, PROCESS);
    t->setBackupOriginalRawFile(m_settingswidget->backupOriginalRawFile());
    t->setCompressLossLess(m_settingswidget->compressLossLess());
    t->setUpdateFileDate(m_settingswidget->updateFileDate());
    t->setPreviewMode(m_settingswidget->previewMode());
    
    connect(t, SIGNAL(signalFinished(KIPIDNGConverterPlugin::ActionData)),
            this, SLOT(slotFinished(KIPIDNGConverterPlugin::ActionData)),Qt::QueuedConnection);
    
    connect(t, SIGNAL(signalFinished(KIPIDNGConverterPlugin::ActionData)),
            this, SLOT(slotFinished(KIPIDNGConverterPlugin::ActionData)),Qt::QueuedConnection);
    
    collection->addJob(t);
    thread->append(collection);
    thread->start();
}

void Plugin_DNGConverter::slotFinished(const KIPIDNGConverterPlugin::ActionData& ad)
{
    if(ad.result != DNGWriter::PROCESSCOMPLETE)
    {
        setErrorDescription("Failed to process image...");
	status = FAILED;
    }
    else
        processed(ad.fileUrl, ad.destPath);
}

void Plugin_DNGConverter::processed(const KUrl& url, const QString& tmpFile)
{
    QFileInfo tmpInfo(tmpFile);
    QString tmp = url.directory(KUrl::AppendTrailingSlash)+tmpInfo.completeBaseName()+".dng";
    KUrl dest = KUrl(tmp);
    QString destFile = dest.pathOrUrl();
    
    if (m_settingswidget->conflictRule() != SettingsWidget::OVERWRITE)
    {
        struct stat statBuf;

        if (::stat(QFile::encodeName(destFile), &statBuf) == 0)
        {
            kDebug()<<"Failed to save image...";
	    setErrorDescription("Failed to save image...");
	    status = FAILED;
        }
    }
    
    if (!destFile.isEmpty())
    {
        if ( KIPIPlugins::KPMetadata::hasSidecar(tmpFile))
        {
            if (! KIPIPlugins::KPMetadata::moveSidecar(KUrl(tmpFile), KUrl(destFile)))
            {
                status = FAILED;
	        kDebug()<<"Failed to move sidecar...";
		setErrorDescription("Failed to move sidecar...");
            }
        }  
      
        if (KDE::rename(QFile::encodeName(tmpFile), QFile::encodeName(destFile)) != 0)
	{
	    kDebug()<<"Failed to save image...";
	    setErrorDescription("Failed to save image...");
	    status = FAILED;
	}
	else
        {
            KIPIPlugins::KPImageInfo info(url);
            info.cloneData(KUrl(destFile));
	    temp = KUrl(destFile);
	    status = SUCCESS;
        }
    }
}

void Plugin_DNGConverter::setup(QWidget* const widget)
{
    m_batchDlg = 0;

    Plugin::setup(widget);

    if (!interface())
    {
        kError() << "Kipi interface is null!";
        return;
    }

    setupActions();

    connect(interface(), SIGNAL(currentAlbumChanged(bool)),
            m_action, SLOT(setEnabled(bool)));
}

void Plugin_DNGConverter::setupActions()
{
    setDefaultCategory(BatchPlugin);

    m_action = new KAction(this);
    m_action->setText(i18n("DNG Converter..."));
    m_action->setIcon(KIcon("kipi-dngconverter"));

    connect(m_action, SIGNAL(triggered(bool)),
            this, SLOT(slotActivate()));

    addAction("dngconverter", m_action);
}

void Plugin_DNGConverter::slotActivate()
{
        
    m_get  = new Plugin_GetWidget(interface());
    bool check1 = m_get->getWidget();
    
    m_task = new Plugin_GetTask(interface());
    bool check2 = m_task->getTask();
  
    if(!check1 && !check2)
    {
        if (!interface())
        {
            kError() << "Kipi interface is null!" ;
            return;
        }

        ImageCollection images = interface()->currentSelection();

        if (!images.isValid())
            return;

        if (!m_batchDlg)
        {
            m_batchDlg = new BatchDialog(new DNGConverterAboutData);
        }
        else
        {
            if (m_batchDlg->isMinimized())
                KWindowSystem::unminimizeWindow(m_batchDlg->winId());

            KWindowSystem::activateWindow(m_batchDlg->winId());
        }

        m_batchDlg->show();
        m_batchDlg->addItems(images.images());
    }
}

} // namespace KIPIDNGConverterPlugin
