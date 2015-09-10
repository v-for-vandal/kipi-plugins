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

#include "imageshackwindow.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QComboBox>
#include <QTimer>

// KDE includes

#include <kaboutdata.h>
#include <kconfig.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kurllabel.h>
#include <kstandarddirs.h>
#include <KWindowConfig>
#include <KGuiItem>

// Libkipi includes

#include <KIPI/Interface>

// Local includes

#include "kipiplugins_debug.h"
#include "imageshack.h"
#include "imageshackwidget.h"
#include "imageshacktalker.h"
#include "kpaboutdata.h"
#include "kpimageslist.h"
#include "kpprogresswidget.h"

namespace KIPIImageshackPlugin
{

ImageshackWindow::ImageshackWindow(QWidget* const parent, Imageshack* const imghack)
    : KPToolDialog(parent)
{
    m_imageshack = imghack;
    m_widget     = new ImageshackWidget(this, imghack);
    m_widget->setMinimumSize(700, 500);
    setMainWidget(m_widget);
    setWindowTitle(i18n("Export to Imageshack"));
    setModal(true);

    connect(m_widget->m_chgRegCodeBtn, SIGNAL(clicked(bool)),
            this, SLOT(slotChangeRegistrantionCode()));

    KGuiItem::assign(startButton(),
                     KGuiItem(i18n("Upload"), "network-workgroup",
                              i18n("Start upload to Imageshack web service")));
    startButton()->setEnabled(false);

    connect(m_widget->m_imgList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    // About data
    KPAboutData* const about = new KPAboutData(ki18n("Imageshack Export"),
                                   0,
                                   KAboutLicense::GPL,
                                   ki18n("A kipi plugin to export images to Imageshack web service."),
                                   ki18n("(c) 2012, Dodon Victor\n"));

    about->addAuthor(ki18n("Dodon Victor").toString(),
                     ki18n("Author").toString(),
                     "dodonvictor at gmail dot com");

    about->setHandbookEntry("imageshack");
    setAboutData(about);

    // -----------------------------------------------------------

    connect(this, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    m_talker = new ImageshackTalker(imghack);

    connect(m_talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(m_talker, SIGNAL(signalNeedRegistrationCode()),
            this, SLOT(slotNeedRegistrationCode()));

    connect(m_talker, SIGNAL(signalJobInProgress(int,int,QString)),
            this, SLOT(slotJobInProgress(int,int,QString)));

    connect(m_talker, SIGNAL(signalLoginDone(int,QString)),
            this, SLOT(slotLoginDone(int,QString)));

    connect(m_talker, SIGNAL(signalGetGalleriesDone(int,QString)),
            this, SLOT(slotGetGalleriesDone(int,QString)));

    connect(m_talker, SIGNAL(signalUpdateGalleries(QStringList,QStringList)),
            m_widget, SLOT(slotGetGalleries(QStringList,QStringList)));

    connect(m_talker, SIGNAL(signalAddPhotoDone(int,QString)),
            this, SLOT(slotAddPhotoDone(int,QString)));

    connect(m_widget, SIGNAL(signalReloadGalleries()),
            this, SLOT(slotGetGalleries()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancelClicked()));

    readSettings();

    QTimer::singleShot(20, this, SLOT(authenticate()));
}

ImageshackWindow::~ImageshackWindow()
{
}

void ImageshackWindow::slotImageListChanged()
{
    startButton()->setEnabled(!m_widget->m_imgList->imageUrls().isEmpty());
}

void ImageshackWindow::slotFinished()
{
    saveSettings();
    m_widget->m_progressBar->progressCompleted();
    m_widget->m_imgList->listView()->clear();
}

void ImageshackWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void ImageshackWindow::readSettings()
{
    KConfig config("kipirc");
    KConfigGroup group = config.group("Imageshack Settings");
    KWindowConfig::restoreWindowSize(windowHandle(), group);

    if (group.readEntry("Private", false))
    {
        m_widget->m_privateImagesChb->setChecked(true);
    }

    QString resize = group.readEntry("Resize", QString());
    if (resize == "No")
    {
        m_widget->m_noResizeRdb->setChecked(true);
    }
    else if (resize == "Template")
    {
        m_widget->m_predefSizeRdb->setChecked(true);
        m_widget->m_resizeOptsCob->setCurrentIndex(group.readEntry("Template", 0));
    }
    else
    {
        m_widget->m_customSizeRdb->setChecked(true);
        m_widget->m_widthSpb->setValue(group.readEntry("Width", 1000));
        m_widget->m_heightSpb->setValue(group.readEntry("Height", 1000));
    }

    if (group.readEntry("Rembar", false))
    {
        m_widget->m_remBarChb->setChecked(true);
    }
    else
    {
        m_widget->m_remBarChb->setChecked(false);
    }
}

void ImageshackWindow::saveSettings()
{
    KConfig config("kipirc");
    KConfigGroup group = config.group("Imageshack Settings");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    group.writeEntry("Private", m_widget->m_privateImagesChb->isChecked());

    if (m_widget->m_noResizeRdb->isChecked())
    {
        group.writeEntry("Resize", "No");
    }
    else if (m_widget->m_predefSizeRdb->isChecked())
    {
        group.writeEntry("Resize", "Template");
        group.writeEntry("Template", m_widget->m_resizeOptsCob->currentIndex());
    }
    else
    {
        group.writeEntry("Resize", "Custom");
        group.writeEntry("Width", m_widget->m_widthSpb->value());
        group.writeEntry("Height", m_widget->m_heightSpb->value());
    }

    group.writeEntry("Rembar", m_widget->m_remBarChb->isChecked());

    group.sync();
}

void ImageshackWindow::slotStartTransfer()
{
    m_widget->m_imgList->clearProcessedStatus();
    m_transferQueue = m_widget->m_imgList->imageUrls();

    if (m_transferQueue.isEmpty())
    {
        return;
    }

    qCDebug(KIPIPLUGINS_LOG) << "Transfer started!";

    m_imagesTotal = m_transferQueue.count();
    m_imagesCount = 0;

    m_widget->m_progressBar->setFormat(i18n("%v / %m"));
    m_widget->m_progressBar->setMaximum(m_imagesTotal);
    m_widget->m_progressBar->setValue(0);
    m_widget->m_progressBar->setVisible(true);
    m_widget->m_progressBar->progressScheduled(i18n("Image Shack Export"), false, true);
    m_widget->m_progressBar->progressThumbnailChanged(QIcon::fromTheme("kipi").pixmap(22, 22));

    uploadNextItem();
}

void ImageshackWindow::slotCancelClicked()
{
    m_talker->cancel();
    m_transferQueue.clear();
    m_widget->m_imgList->cancelProcess();
    m_widget->m_progressBar->setVisible(false);
    m_widget->m_progressBar->progressCompleted();
}

void ImageshackWindow::slotChangeRegistrantionCode()
{
    qCDebug(KIPIPLUGINS_LOG) << "Change registration code";
    m_imageshack->m_registrationCode.clear();
    authenticate();
}

void ImageshackWindow::authenticate()
{
    emit signalBusy(true);
    m_widget->progressBar()->show();
    m_widget->m_progressBar->setValue(0);
    m_widget->m_progressBar->setMaximum(4);
    m_widget->progressBar()->setFormat(i18n("Authenticating..."));

    if (m_imageshack->registrationCode().isEmpty())
    {
        askRegistrationCode();
    }
    m_talker->authenticate();
}

void ImageshackWindow::askRegistrationCode()
{
    QDialog* window = new QDialog(this, 0);
    window->setModal(true);
    window->setWindowTitle(i18n("Imageshack authorization"));

    QLineEdit* codeField = new QLineEdit();
    QPlainTextEdit* infoText = new QPlainTextEdit(
        i18n( "Please paste the registration code for your ImageShack account"
              " into the textbox below and press \"OK\"."
    ));
    infoText->setReadOnly(true);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, window);
    connect(buttonBox, &QDialogButtonBox::accepted, window, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, window, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(window);
    layout->addWidget(infoText);
    layout->addWidget(codeField);
    layout->addWidget(buttonBox);
    window->setLayout(layout);

    if (window->exec() == QDialog::Accepted)
    {
        QString code = codeField->text();
        if (!code.isEmpty())
        {
            m_imageshack->setRegistrationCode(code);
//             m_imageshack->writeSettings();
            return;
        }
    }

    m_talker->cancelLogIn();
}

void ImageshackWindow::slotNeedRegistrationCode()
{
    authenticate();
}

void ImageshackWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        m_widget->m_chgRegCodeBtn->setEnabled(false);
        startButton()->setEnabled(false);
        setRejectButtonMode(QDialogButtonBox::Cancel);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        m_widget->m_chgRegCodeBtn->setEnabled(true);
        startButton()->setEnabled(m_imageshack->loggedIn() &&
                                  !m_widget->imagesList()->imageUrls().isEmpty());
        setRejectButtonMode(QDialogButtonBox::Close);
    }
}

void ImageshackWindow::slotJobInProgress(int step, int maxStep, const QString &format)
{
    if (maxStep > 0)
    {
        m_widget->m_progressBar->setMaximum(maxStep);
    }
    m_widget->m_progressBar->setValue(step);

    if (!format.isEmpty())
    {
        m_widget->m_progressBar->setFormat(format);
    }
}

void ImageshackWindow::slotLoginDone(int errCode, const QString& errMsg)
{
    m_widget->updateLabels();

    if (!errCode && m_imageshack->loggedIn())
    {
        m_imageshack->saveSettings();
        startButton()->setEnabled(!m_widget->imagesList()->imageUrls().isEmpty());
        m_talker->getGalleries();
    }
    else
    {
        KMessageBox::error(this, i18n("Login failed: %1\n", errMsg));
        startButton()->setEnabled(false);
        m_widget->m_progressBar->setVisible(false);
        slotBusy(false);
    }
}

void ImageshackWindow::slotGetGalleriesDone(int errCode, const QString &errMsg)
{
    slotBusy(false);
    m_widget->m_progressBar->setVisible(false);

    if (errCode)
    {
        KMessageBox::error(this, i18n("Failed to get galleries list: %1\n", errMsg));
    }
    m_widget->getGalleriesDone(errCode);
}

void ImageshackWindow::uploadNextItem()
{
    if (m_transferQueue.empty())
    {
        m_widget->m_progressBar->hide();
        return;
    }

    m_widget->m_imgList->processing(m_transferQueue.first());
    QString imgPath = m_transferQueue.first().toLocalFile();

    m_widget->m_progressBar->setMaximum(m_imagesTotal);
    m_widget->m_progressBar->setValue(m_imagesCount);

    QMap<QString, QString> opts;

    if (m_widget->m_privateImagesChb->isChecked())
    {
        opts["public"] = "no";
    }

    if (m_widget->m_remBarChb->isChecked())
    {
        opts["rembar"] = "yes";
    }

    if (m_widget->m_predefSizeRdb->isChecked())
    {
        opts["optimage"] = "1";
        opts["optsize"] = m_widget->m_resizeOptsCob->itemData(m_widget->m_resizeOptsCob->currentIndex()).toString();
    }
    else if (m_widget->m_customSizeRdb->isChecked())
    {
        opts["optimage"] = "1";
        QString dim =  "";
        dim.append(QString("%1x%2").arg(m_widget->m_widthSpb->value()).arg(m_widget->m_heightSpb->value()));
        opts["optsize"] = dim;
    }

    // tags
    if (!m_widget->m_tagsFld->text().isEmpty())
    {
        QString str = m_widget->m_tagsFld->text();
        QStringList tagsList;
        tagsList = str.split(QRegExp("\\W+"), QString::SkipEmptyParts);
        opts["tags"] = tagsList.join(",");
    }

    opts["cookie"] = m_imageshack->registrationCode();

    bool uploadToGalleries = m_widget->m_useGalleriesChb->isChecked();

    if (uploadToGalleries)
    {
        int gidx = m_widget->m_galleriesCob->currentIndex();
        QString gallery;
        if (gidx == 0)
            gallery = m_widget->m_newGalleryName->text();
        else
            gallery = m_widget->m_galleriesCob->itemData(gidx).toString();
        m_talker->uploadItemToGallery(imgPath, gallery, opts);
    }
    else
    {
        m_talker->uploadItem(imgPath, opts);
    }
}

void ImageshackWindow::slotAddPhotoDone(int errCode, const QString& errMsg)
{
    m_widget->m_imgList->processed(m_transferQueue.first(), (errCode == 0));

    if (!errCode)
    {
        m_transferQueue.pop_front();
        m_imagesCount++;
    }
    else
    {
        if (KMessageBox::warningContinueCancel(this,
                                               i18n("Failed to upload photo to Imageshack: %1\n"
                                                    "Do you want to continue?", errMsg))
            != KMessageBox::Continue)
        {
            m_widget->m_progressBar->setVisible(false);
            m_transferQueue.clear();
            return;
        }
    }

    uploadNextItem();
}

void ImageshackWindow::slotGetGalleries()
{
    m_widget->m_progressBar->setVisible(true);
    m_talker->getGalleries();
}

} // namespace KIPIImageshackPlugin