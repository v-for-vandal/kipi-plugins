/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2012-02-06
 * Description : help wrapper around libkipi ImageInfo to manage easily
 *               item properties with KIPI host application.
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef KPQUICKIMAGEINFO_H
#define KPQUICKIMAGEINFO_H

// Qt includes

#include <QList>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QUrl>
#include <QObject>

// Local includes

#include "kipiplugins_export.h"
#include "kpimageinfo.h"

namespace KIPIPlugins
{

/** Extension for KPImageInfo. Provides signals and slots 
 * for using class in QML applications
 */
class KIPIPLUGINS_EXPORT KPQuickImageInfo : public QObject, public KPImageInfo
{
    Q_OBJECT

public:

    /** Contructor with item url that you want to manage. KIPI interface from plugin loader instance is used
     *  to fill item info from kipi host. If no interface is available, for ex when plugin is loaded as
     *  stand-alone application, some info are filled with image file metadata.
     */
    KPQuickImageInfo(QObject* parent = 0);
    KPQuickImageInfo(const QUrl& url, QObject* parent = 0);
    ~KPQuickImageInfo();

    void setUrl(const QUrl& url);

    void setDescription(const QString& desc);
    Q_INVOKABLE bool hasDescription() const { return KPImageInfo::hasDescription(); }

    void setTagsPath(const QStringList& tp);
    Q_INVOKABLE bool hasTagsPath() const { return KPImageInfo::hasTagsPath(); }

    Q_INVOKABLE bool hasKeywords() const { return KPImageInfo::hasKeywords(); }

    void setRating(int r);
    Q_INVOKABLE bool hasRating() const { return KPImageInfo::hasRating(); }

    void setColorLabel(int cl);
    Q_INVOKABLE bool hasColorLabel() const { return KPImageInfo::hasColorLabel(); }

    void setPickLabel(int pl);
    Q_INVOKABLE bool hasPickLabel() const { return KPImageInfo::hasPickLabel(); }

    void setDate(const QDateTime& date);
    Q_INVOKABLE bool hasDate() const { return KPImageInfo::hasDate(); }
    Q_INVOKABLE bool isExactDate() const { return KPImageInfo::isExactDate(); }

    void setTitle(const QString& title);
    Q_INVOKABLE bool hasTitle() const { return KPImageInfo::hasTitle(); }

    void setName(const QString& name);
    Q_INVOKABLE bool hasName() const { return KPImageInfo::hasName(); }

    void setLatitude(double lat);
    Q_INVOKABLE bool hasLatitude() const { return KPImageInfo::hasLatitude(); }

    void setLongitude(double lng);
    Q_INVOKABLE bool hasLongitude() const { return KPImageInfo::hasLongitude(); }

    void setAltitude(double alt);
    Q_INVOKABLE bool hasAltitude() const { return KPImageInfo::hasAltitude(); }

    Q_INVOKABLE bool hasGeolocationInfo() const { return KPImageInfo::hasGeolocationInfo(); }
    Q_INVOKABLE void removeGeolocationInfo() { KPImageInfo::removeGeolocationInfo(); }

    void setOrientation(int);
    Q_INVOKABLE bool hasOrientation() const { return KPImageInfo::hasOrientation(); }

    void setCreators(const QStringList& list);
    Q_INVOKABLE bool hasCreators() const { return KPImageInfo::hasCreators(); }

    void setCredit(const QString& val);
    Q_INVOKABLE bool hasCredit() const { return KPImageInfo::hasCredit(); }

    void setRights(const QString& val);
    Q_INVOKABLE bool hasRights() const { return KPImageInfo::hasRights(); }

    void setSource(const QString& val);
    Q_INVOKABLE bool hasSource() const { return KPImageInfo::hasSource(); }

public:
    QUrl thumbnailUrl() const { return m_thumbnailUrl; }
    QUrl previewUrl() const { return m_previewUrl; }

Q_SIGNALS:
    /** Qt Meta Type system declarations
     */
    void urlChanged(const QUrl&);
    void fileSizeChanged(qlonglong);
    void descriptionChanged(const QString&);
    void tagsPathChanged(const QStringList&);
    void keywordsChanged(const QStringList&);
    void ratingChanged(int);
    void colorLabelChanged(int);
    void pickLabelChanged(int);
    void dateChanged(const QDateTime&);
    void nameChanged(const QString&);
    void titleChanged(const QString&);
    void latitudeChanged(double);
    void longitudeChanged(double);
    void altitudeChanged(double);
    void orientationChanged(int);
    void creatorsChanged(const QStringList&);
    void creditChanged(const QString&);
    void rightsChanged(const QString&);
    void sourceChanged(const QString&);

public:
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged);
    Q_PROPERTY(QUrl thumbnailUrl READ thumbnailUrl NOTIFY urlChanged);
    Q_PROPERTY(QUrl previewUrl READ previewUrl NOTIFY urlChanged);
    Q_PROPERTY(qlonglong fileSize READ fileSize CONSTANT);
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged);
    Q_PROPERTY(QStringList tagsPath READ tagsPath WRITE setTagsPath NOTIFY tagsPathChanged);
    Q_PROPERTY(QStringList keywords READ keywords CONSTANT);
    Q_PROPERTY(int rating READ rating WRITE setRating NOTIFY ratingChanged);
    Q_PROPERTY(int colorLabel READ colorLabel WRITE setColorLabel NOTIFY colorLabelChanged);
    Q_PROPERTY(int pickLabel READ pickLabel WRITE setPickLabel NOTIFY pickLabelChanged);
    Q_PROPERTY(QDateTime date READ date WRITE setDate NOTIFY dateChanged);
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged);
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged);
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged);
    Q_PROPERTY(double altitude READ altitude WRITE setAltitude NOTIFY altitudeChanged);
    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged);
    Q_PROPERTY(QStringList creators READ creators WRITE setCreators NOTIFY creatorsChanged);
    Q_PROPERTY(QString credit READ credit WRITE setCredit NOTIFY creditChanged);
    Q_PROPERTY(QString rights READ rights WRITE setRights NOTIFY rightsChanged);
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged);


private Q_SLOTS:
    void onUrlChanged(const QUrl& newUrl);

private:
    QUrl m_thumbnailUrl;
    QUrl m_previewUrl;

    void updateDependentData();
};

} // namespace KIPIPlugins

#endif  // KPIMAGEINFO_H
