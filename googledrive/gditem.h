/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2008-12-28
 * Description : a kipi plugin to export images to Google-Drive web service
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
#ifndef GDITEM_H
#define GDITEM_H

//Qt includes
#include <QString>

namespace KIPIGoogleDrivePlugin{

class GDPhoto{

public:
    GDPhoto(){
        //
    }

    QString title;
    QString description;
};

class GDFolder{

public:
    GDFolder(){
        //
    }
    QString title;

};
}//namespace KIPIGoogleDrivePlugin

#endif // GDITEM_H
