/* ============================================================
 * 
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a plugin to create panorama by fusion of several images.
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

#ifndef TASK_H
#define TASK_H

// KDE includes

#include <threadweaver/Job.h>
#include <kprocess.h>

// Local includes

#include "actions.h"

#include <libkdcraw/kdcraw.h>

//Local includes
#include "actions.h"
#include "kpmetadata.h"

namespace KIPIExpoBlendingPlugin
{

class Task : public ThreadWeaver::Job
{
  
   Q_OBJECT
   
public:
       
    KUrl                outputUrl;
    QString             binaryPath;
    
    RawDecodingSettings              rawDecodingSettings;
    EnfuseSettings      enfuseSettings;		//enfusebinary
    
    
public:

    QString      errString;
    const Action action;
    

protected:

    bool         successFlag;
    bool         isAbortedFlag;
    const KUrl   tmpDir;

public:

    Task(QObject* const parent, const KUrl::List& fileUrl, const Action& action);
    ~Task();

    bool success() const;
    void requestAbort();
    void setPreProcessingSettings(bool align, const RawDecodingSettings& settings);
    bool getXmpRational(const char* xmpTagName, long& num, long& den, KPMetadata& meta);
    float getAverageSceneLuminance(const KUrl& url);
    bool startEnfuse(const KUrl::List& inUrls, KUrl& outUrl,
                               const EnfuseSettings& settings,
                               const QString& enfusePath, QString& errors);
    QString getProcessError(KProcess* const proc) const;
    bool startPreProcessing(const KUrl::List& inUrls, ItemUrlsMap& preProcessedMap,
                                      bool align, const RawDecodingSettings& rawSettings,
                                      const QString& alignPath, QString& errors);

Q_SIGNALS:

    void starting(const KIPIExpoBlendingPlugin::ActionData& ad);
    void finished(const KIPIExpoBlendingPlugin::ActionData& ad);


protected:

    virtual void run() ;

    static QString getProcessError(KProcess& proc);
    
private:

    class ActionThreadPriv;
    ActionThreadPriv* const d;
    
};

}  // namespace KIPIExpoBlendingPlugin

#endif /* TASK_H */
