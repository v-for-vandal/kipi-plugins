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

#ifndef PLUGIN_GETTASK_H
#define PLUGIN_GETTASK_H

// Qt includes

#include <QtGui/QWidget>

// libkipi includes

#include "libkipi/bkgtask.h"
#include <threadweaver/JobCollection.h>

//local includes

#include "actions.h"

using namespace KIPI;
using namespace ThreadWeaver;

namespace KIPIDNGConverterPlugin
{

class Plugin_GetTask: public BkgTask
{ 
    Q_OBJECT  

public:

    Plugin_GetTask(Interface* const interface);
    ~Plugin_GetTask();

    bool getTask();
    void processed(const KUrl& url, const QString& tmpFile);
    
public Q_SLOTS:

    void slotFinished(const KIPIDNGConverterPlugin::ActionData& ad);
    void slotIdentify();

private:
    class Private;
    Private* const d;
};

} // namespace KIPIDNGConverterPlugin

#endif
