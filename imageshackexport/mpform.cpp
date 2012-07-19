/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a plugin to export photos or videos to ImageShack web service
 *
 * Copyright (C) 2012 Dodon Victor <dodonvictor at gmail dot com>
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

#include "mpform.h"

// C++ includes

#include <cstring>
#include <cstdio>

// Qt includes

#include <QFile>

// KDE includes

#include <kdebug.h>
#include <kmimetype.h>
#include <krandom.h>

namespace KIPIImageshackExportPlugin
{

MPForm::MPForm()
{
    m_boundary = KRandom::randomString(42 + 13).toAscii();
    reset();
}

MPForm::~MPForm()
{
}

void MPForm::reset()
{
    m_buffer.resize(0);
    QByteArray str(contentType().toLatin1());
    str += "\r\nMIME-version: 1.0\r\n\r\n";
    m_buffer.append(str);
}

void MPForm::finish()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";
    m_buffer.append(str);
}

void MPForm::addPair(const QString& name, const QString& value)
{
    QByteArray str;
    QString content_length = QString::number(value.length());

    str += "--";
    str += m_boundary;
    str += "\r\n";

    if (!name.isEmpty())
    {
        str += "Content-Disposition: form-data; name=\"";
        str += name.toAscii();
        str += "\"\r\n";
    }
    str += "\r\n";
    str += value.toUtf8();
    str += "\r\n";

    m_buffer.append(str);
}

bool MPForm::addFile(const QString& name, const QString& path)
{
    KMimeType::Ptr ptr = KMimeType::findByUrl(path);
    QString mime       = ptr->name();
    if (mime.isEmpty())
        return false;

    QFile imageFile(path);
    if (!imageFile.open(QIODevice::ReadOnly))
        return false;
    QByteArray imageData = imageFile.readAll();

    QString file_size = QString::number(imageFile.size());
    imageFile.close();

    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; name=\"fileupload\"; filename=";
    str += QFile::encodeName(name);
    str += "\"\r\n";
    str += "Content-Length: ";
    str += file_size.toAscii();
    str += "\r\n";
    str += "Content-Type: ";
    str += mime.toAscii();
    str += "\r\n\r\n";

    m_buffer.append(str);
    m_buffer.append(imageData);
    m_buffer.append("\r\n");

    return true;
}

QString MPForm::contentType() const
{
    return QString("Content-Type: multipart/form-data; boundary=" + m_boundary);
}

QString MPForm::boundary() const
{
    return m_boundary;
}

QByteArray MPForm::formData() const
{
    return m_buffer;
}

} // namespace KIPIImageshackExportPlugin
