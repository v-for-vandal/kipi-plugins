/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-09-19
 * Description : GPS file list view item.
 * 
 * Copyright 2006 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef GPSLISTVIEWITEM_H
#define GPSLISTVIEWITEM_H

// Qt includes.

#include <qdatetime.h>
#include <qstring.h>

// KDE includes.

#include <kurl.h>
#include <klistview.h>

class QPainter;
class QColorGroup;
class QPixmap;

class KFileItem;

namespace KIPIGPSSyncPlugin
{

class GPSListViewItem : public QObject, public KListViewItem
{

Q_OBJECT

public:

    GPSListViewItem(KListView *view, QListViewItem *after, const KURL& url);

    ~GPSListViewItem(){};

    void setGPSInfo(double altitude, double latitude, double longitude);
    void getGPSInfo(double& altitude, double& latitude, double& longitude);

    void setDateTime(QDateTime date);
    QDateTime getDateTime();

    void setEnabled(bool d);
    bool isEnabled(void);

    KURL getUrl();
    void writeGPSInfoToFile();
        
protected:

    void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);

private slots:

    void slotGotThumbnail(const KFileItem*, const QPixmap&);

private: 

    bool      m_enabled;

    double    m_altitude;
    double    m_latitude;
    double    m_longitude;

    QDateTime m_date;

    KURL      m_url;
};

} // NameSpace KIPIGPSSyncPlugin

#endif /* GPSLISTVIEWITEM_H */
