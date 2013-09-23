/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2008-12-28
 * Description : a kipi plugin to export images to Dropbox web service
 *
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
#ifndef DBITEM_H
#define DBITEM_H

//qt includes
#include <QString>

namespace KIPIDropboxPlugin{

class DBPhoto{

public:
    DBPhoto(){
        //
    }

    QString title;
};

class DBFolder{

public:
    DBFolder(){
        //
    }

    QString title;
};

}

#endif //DBITEM_H