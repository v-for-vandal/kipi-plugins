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

#include "gdwindow.moc"

// Qt includes

#include <QPushButton>
#include <QProgressDialog>
#include <QPixmap>
#include <QCheckBox>
#include <QStringList>
#include <QSpinBox>
#include <QPointer>

// KDE includes

#include <kcombobox.h>
#include <klineedit.h>
#include <kmenu.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <ktabwidget.h>
#include <krun.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kwallet.h>
#include <kpushbutton.h>
#include <kurl.h>

// LibKIPI includes

#include <libkipi/interface.h>

// Local includes

#include "kpimageslist.h"
#include "kprogressdialog.h"
#include "kpaboutdata.h"
#include "kpimageinfo.h"
#include "kpversion.h"
#include "kpprogresswidget.h"
#include "gdtalker.h"
#include "gditem.h"
#include "gdalbum.h"
#include "gdwidget.h"

namespace KIPIGoogleDrivePlugin{

GDWindow::GDWindow(const QString& tmpFolder,QWidget* const /*parent*/) : KPToolDialog(0){
    m_tmp = tmpFolder;
    m_imagesCount = 0;
    m_imagesTotal = 0;

    m_widget      = new GoogleDriveWidget(this);
    setMainWidget(m_widget);
    setWindowIcon(KIcon("googledrive"));
    setButtons(Help | User1 | Close);
    setDefaultButton(Close);
    setModal(false);
    setWindowTitle(i18n("Export to Google Drive"));
    setButtonGuiItem(User1,KGuiItem(i18n("Start-Upload"),"network-workgroup",i18n("Start upload to Google Drive")));
    m_widget->setMinimumSize(700,500);

    connect(m_widget->m_imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(m_widget->m_logout,SIGNAL(clicked()),
            this,SLOT(slotLogout()));

    connect(m_widget->m_newAlbumBtn,SIGNAL(clicked()),
            this,SLOT(slotNewAlbumRequest()));

    connect(m_widget->m_reloadAlbumsBtn,SIGNAL(clicked()),
            this,SLOT(slotReloadAlbumsRequest()));

    connect(this,SIGNAL(user1Clicked()),
            this,SLOT(slotStartTransfer()));

    KPAboutData* const about = new KPAboutData(ki18n("Google Drive Export"),0,
                                               KAboutData::License_GPL,
                                               ki18n("A Kipi-plugin to export images "
                                                     "to Google Drive"),
                                               ki18n("(c) 2013, Saurabh Patel"));

    about->addAuthor(ki18n("Saurabh Patel"),ki18n("Author and maintainer"),
                     "saurabhpatel7717 at gmail dot com");
    about->setHandbookEntry("googledrive");
    setAboutData(about);

    //-------------------------------------------------------------------------

    m_albumDlg = new GDNewAlbum(this);

    //-------------------------------------------------------------------------

    m_talker = new GDTalker(this);
    kDebug() << "111";
    connect(m_talker,SIGNAL(signalBusy(bool)),
            this,SLOT(slotBusy(bool)));
    kDebug() << "112";
    connect(m_talker,SIGNAL(signalAccessTokenFailed(int,QString)),
            this,SLOT(slotAccessTokenFailed(int,QString)));
    kDebug() << "113";
    connect(m_talker,SIGNAL(signalAccessTokenObtained()),
            this,SLOT(slotAccessTokenObtained()));

    connect(m_talker,SIGNAL(signalListAlbumsFailed(QString)),
            this,SLOT(slotListAlbumsFailed(QString)));
    kDebug() << "114";
    connect(m_talker,SIGNAL(signalListAlbumsDone(QList<QPair<QString,QString> >)),
            this,SLOT(slotListAlbumsDone(QList<QPair<QString,QString> >)));
    kDebug() << "115";
    connect(m_talker,SIGNAL(signalCreateFolderFailed(QString)),
            this,SLOT(slotCreateFolderFailed(QString)));

    connect(m_talker,SIGNAL(signalCreateFolderSucceeded()),
            this,SLOT(slotCreateFolderSucceeded()));

    connect(m_talker,SIGNAL(signalAddPhotoFailed(QString)),
            this,SLOT(slotAddPhotoFailed(QString)));

    connect(m_talker,SIGNAL(signalAddPhotoSucceeded()),
            this,SLOT(slotAddPhotoSucceeded()));

    readSettings();
    buttonStateChange(false);
    kDebug() << "116";
    m_talker->doOAuth();
}

GDWindow::~GDWindow(){
    delete m_widget;
    delete m_albumDlg;
    delete m_talker;
}

void GDWindow::reactivate()
{
    m_widget->imagesList()->loadImagesFromCurrentSelection();
    show();
}

void GDWindow::readSettings(){
    KConfig config("kipirc");
    KConfigGroup grp = config.group("Google Drive Settings");

    m_currentAlbumId = grp.readEntry("Current Album",QString());

    if (grp.readEntry("Resize", false))
    {
        m_widget->m_resizeChB->setChecked(true);
        m_widget->m_dimensionSpB->setEnabled(true);
        m_widget->m_imageQualitySpB->setEnabled(true);
    }
    else
    {
        m_widget->m_resizeChB->setChecked(false);
        m_widget->m_dimensionSpB->setEnabled(false);
        m_widget->m_imageQualitySpB->setEnabled(false);
    }

    m_widget->m_dimensionSpB->setValue(grp.readEntry("Maximum Width",    1600));
    m_widget->m_imageQualitySpB->setValue(grp.readEntry("Image Quality", 90));

    KConfigGroup dialogGroup = config.group("Google Drive Export Dialog");
    restoreDialogSize(dialogGroup);

}

void GDWindow::writeSettings(){
    KConfig config("kipirc");
    KConfigGroup grp = config.group("Google Drive Settings");

    grp.writeEntry("Current Album",m_currentAlbumId);
    grp.writeEntry("Resize",          m_widget->m_resizeChB->isChecked());
    grp.writeEntry("Maximum Width",   m_widget->m_dimensionSpB->value());
    grp.writeEntry("Image Quality",   m_widget->m_imageQualitySpB->value());

    KConfigGroup dialogGroup = config.group("Google Drive Export Dialog");
    saveDialogSize(dialogGroup);

    config.sync();

}

void GDWindow::slotListAlbumsDone(const QList<QPair<QString,QString> >& list){
    m_widget->m_albumsCoB->clear();
    kDebug() << "slotListAlbumsDone1:" << list.size();
    for(int i=0;i<list.size();i++){
        m_widget->m_albumsCoB->addItem(KIcon("system-users"),list.value(i).second,
                                       list.value(i).first);

        if (m_currentAlbumId == list.value(i).first)
        {
            m_widget->m_albumsCoB->setCurrentIndex(i);
        }
    }
    buttonStateChange(true);
}

void GDWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        m_widget->m_logout->setEnabled(false);
        buttonStateChange(false);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        m_widget->m_logout->setEnabled(true);
        buttonStateChange(true);
    }
}

void GDWindow::slotStartTransfer(){
    m_widget->m_imgList->clearProcessedStatus();

    //m_transferQueue = m_widget->m_imgList->m_imageUrls();

    if(m_widget->m_imgList->imageUrls().isEmpty()){
        return;
    }

    typedef QPair<KUrl, GDPhoto> Pair;

    for(int i=0 ;i < (m_widget->m_imgList->imageUrls().size()) ; i++){
        KPImageInfo info(m_widget->m_imgList->imageUrls().value(i));
        GDPhoto temp;
        kDebug() << "in start transfer info " <<info.title() << info.description();
        temp.title          = info.title();
        temp.description    = info.description().section("\n",0,0);

        m_transferQueue.append(Pair(m_widget->m_imgList->imageUrls().value(i),temp));
    }
    m_currentAlbumId = m_widget->m_albumsCoB->itemData(m_widget->m_albumsCoB->currentIndex()).toString();

    m_imagesTotal = m_transferQueue.count();
    m_imagesCount = 0;

    m_progressDlg    = new KProgressDialog(this, i18n("Transfer Progress"));
    m_progressDlg->setMinimumDuration(0);
    m_progressDlg->setModal(true);
    m_progressDlg->setAutoReset(true);
    m_progressDlg->setAutoClose(true);
    m_progressDlg->progressBar()->setFormat(i18n("%v / %m"));

    connect(m_progressDlg, SIGNAL(cancelClicked()),
            this, SLOT(slotTransferCancel()));

    uploadNextPhoto();
}

void GDWindow::uploadNextPhoto(){
    kDebug() << "in upload nextphoto " << m_transferQueue.count();
    if(m_transferQueue.isEmpty()){
        kDebug() << "empty";
        m_progressDlg->hide();
        return;
    }


    typedef QPair<KUrl,GDPhoto> Pair;
    Pair pathComments = m_transferQueue.first();
    GDPhoto info      = pathComments.second;

    bool res = m_talker->addPhoto(pathComments.first.toLocalFile(),info,m_currentAlbumId,
                                  m_widget->m_resizeChB->isChecked(),
                                  m_widget->m_dimensionSpB->value(),
                                  m_widget->m_imageQualitySpB->value());

    if (!res){
        slotAddPhotoFailed("");
        return;
    }
    /*else{
        //m_progressDlg->setLabelText(i18n("Uploading file %1", m_transferQueue.first().path()));
        slotAddPhotoSucceeded();
        return;
    }*/
}

void GDWindow::slotAddPhotoFailed(const QString& msg)
{
    kDebug() << "In slotAddPhotoFailed";
    if (KMessageBox::warningContinueCancel(this, i18n("Failed to upload photo to Google Drive. %1\nDo you want to continue?",msg))
        != KMessageBox::Continue)
    {
        m_transferQueue.clear();
        m_progressDlg->hide();
        kDebug() << "In slotAddPhotoFailed 1";
        // refresh the thumbnails
        //slotTagSelected();
    }
    else
    {
        kDebug() << "In slotAddPhotoFailed 2";
        m_transferQueue.pop_front();
        m_imagesTotal--;
        m_progressDlg->progressBar()->setMaximum(m_imagesTotal);
        m_progressDlg->progressBar()->setValue(m_imagesCount);
        uploadNextPhoto();
    }
}

void GDWindow::slotAddPhotoSucceeded(){
    kDebug() << "In slotAddPhotoSucceeded";
    // Remove photo uploaded from the list
    m_widget->m_imgList->removeItemByUrl(m_transferQueue.first().first);
    m_transferQueue.pop_front();
    m_imagesCount++;
    m_progressDlg->progressBar()->setMaximum(m_imagesTotal);
    m_progressDlg->progressBar()->setValue(m_imagesCount);
    uploadNextPhoto();
}

void GDWindow::slotImageListChanged(){
    enableButton(User1, !(m_widget->m_imgList->imageUrls().isEmpty()));
}

void GDWindow::slotNewAlbumRequest(){
    if (m_albumDlg->exec() == QDialog::Accepted)
    {
        kDebug() << "Calling New Album method";
        GDFolder newFolder;
        m_albumDlg->getAlbumTitle(newFolder);
        m_currentAlbumId = m_widget->m_albumsCoB->itemData(m_widget->m_albumsCoB->currentIndex()).toString();
        m_talker->createFolder(newFolder.title,m_currentAlbumId);
    }
}

void GDWindow::slotReloadAlbumsRequest(){
    kDebug() << "Reload albums request";
    m_talker->listFolders();
}

void GDWindow::slotAccessTokenFailed(int errCode,const QString& errMsg){
    KMessageBox::error(this, i18n("There seems to be %1 error: %2",errCode,errMsg));
    return;
}

void GDWindow::slotAccessTokenObtained(){
    kDebug() << "acc : 111";
    m_talker->listFolders();
}

void GDWindow::slotListAlbumsFailed(const QString& msg){
    KMessageBox::error(this, i18n("GoogleDrive Call Failed: %1\n", msg));
    return;
}

void GDWindow::slotCreateFolderFailed(const QString& msg){
    kDebug() << "In slotCreateFolderFailed";
    KMessageBox::error(this, i18n("GoogleDrive Call Failed: %1\n", msg));
    //return;
}

void GDWindow::slotCreateFolderSucceeded(){
    kDebug() << "In slotCreateFolderSucceeded";
    m_talker->listFolders();
}

void GDWindow::slotTransferCancel()
{
    m_transferQueue.clear();
    m_progressDlg->hide();
    m_talker->cancel();
}

void GDWindow::slotLogout(){

}

void GDWindow::buttonStateChange(bool state)
{
    m_widget->m_newAlbumBtn->setEnabled(state);
    m_widget->m_reloadAlbumsBtn->setEnabled(state);
    enableButton(User1, state);
}

void GDWindow::closeEvent(QCloseEvent *e)
{
    if (!e) return;

    writeSettings();
    m_widget->imagesList()->listView()->clear();
    e->accept();
}

}
