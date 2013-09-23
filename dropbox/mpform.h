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
#ifndef MPFORM_H
#define MPFORM_H

#include <QByteArray>

namespace KIPIDropboxPlugin{

class MPForm{

public:
    MPForm();
    ~MPForm();

    bool addFile(const QString& imgPath);
    QByteArray formData();

private:
    QByteArray m_buffer;
};

}//namespace KIPIDropboxPlugin

#endif /*MPFORM_H*/