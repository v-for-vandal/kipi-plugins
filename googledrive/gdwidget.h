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

#ifndef GDWIDGET_H
#define GDWIDGET_H

//Qt includes

#include <QWidget>

class QLabel;
class QSpinBox;
class QCheckBox;
class QButtonGroup;

class KComboBox;
class KPushButton;

namespace KIPI
{
    class Interface;
    class UploadWidget;
}

namespace KIPIPlugins
{
    class KPImagesList;
    class KPProgressWidget;
}

namespace KIPIGoogleDrivePlugin{

class GoogleDriveWidget : public QWidget{
    Q_OBJECT

public:
    GoogleDriveWidget(QWidget* const parent);
    ~GoogleDriveWidget();

    void updateLabels(const QString& name = "", const QString& url = "");
    KIPIPlugins::KPImagesList* imagesList() const;

private Q_SLOTS:
    //void slotReloadAlbumsRequest();
    //void slotResizeChecked();

private:
    KIPIPlugins::KPImagesList* m_imgList;
    KIPI::UploadWidget*        m_uploadWidget;

    QLabel*                     m_headerLbl;
    QLabel*                     m_userNameDisplayLbl;
    KPushButton*                m_logout;

    KComboBox*                  m_albumsCoB;
    KPushButton*                m_newAlbumBtn;
    KPushButton*                m_reloadAlbumsBtn;

    QCheckBox*                  m_resizeChB;
    QSpinBox*                   m_dimensionSpB;
    QSpinBox*                   m_imageQualitySpB;

friend class GDWindow;
};

}//namespace KIPIGoogleDrivePlugin

#endif /*GDWIDGET_H*/
