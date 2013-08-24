/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP workflow videospecs settings page.
 *
 * Copyright (C) 2007-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *               2013 GSoC: Mahesh Hegde <maheshmhegade at gmail dot.com>
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

#ifndef XMP_VIDEOSPECS_H
#define XMP_VIDEOSPECS_H

// Qt includes

#include <QWidget>
#include <QByteArray>
#include <QGridLayout>

namespace KIPIMetadataEditPlugin
{

class XMPVideospecs : public QWidget
{
    Q_OBJECT

public:

    explicit XMPVideospecs(QWidget* const parent);
    ~XMPVideospecs();

    void applyMetadata(QByteArray& xmpData);
    void readMetadata(QByteArray& xmpData);
    
    public:
      
      QGridLayout* grid;

Q_SIGNALS:

    void signalModified();

private:

    class XMPVideospecsPriv;
    XMPVideospecsPriv* const d;
};

}  // namespace KIPIMetadataEditPlugin

#endif // XMP_VIDEOSPECS_H 
