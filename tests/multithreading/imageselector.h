/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2011-12-28
 * Description : Simple gui to select images
 *
 * Copyright (C) 2011-2012 by A Janardhan Reddy <annapareddyjanardhanreddy@gmail.com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IMAGESELECTOR_H
#define IMAGESELECTOR_H

// KDE includes

#include <kdialog.h>

// Local includes

#include "actionthread.h"

class ImageSelector : public KDialog
{
    Q_OBJECT

public:

    ImageSelector();
    ~ImageSelector();
    
private Q_SLOTS:

    void slotStart();
    void slotStartToProcess(const KUrl& url);
    void slotEndToProcess(const KUrl& url, bool state);

private:

    class ImageSelectorPriv;
    ImageSelectorPriv* const d;
};

#endif // IMAGESELECTOR_H
