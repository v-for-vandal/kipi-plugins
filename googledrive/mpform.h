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
#ifndef MPFORM_H
#define MPFORM_H

//Qt includes

#include <QByteArray>
#include <QString>

namespace KIPIGoogleDrivePlugin
{

class MPForm{
public:
    MPForm();
    ~MPForm();

    void finish();
    void reset();

    void addPair(const QString& name,const QString& description,const QString& mimetype,const QString& id);
    bool addFile(const QString& path);

    QString contentType() const;
    QByteArray formData() const;
    QString boundary() const;
    QString getFileSize() const;

private:

    QByteArray m_buffer;
    QString m_boundary;
    QString m_file_size;
};

}//namespace KIPIGoogleDrivePlugin

#endif /*MPFORM_H*/
