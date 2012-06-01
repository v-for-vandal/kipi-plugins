/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2012-02-16
 * Description : an Hello World plugin.
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
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

/** Take a care about includes order, to prevent compilation problem.
 *  1/ moc file.
 *  2/ C ansi if really necessary.
 *  3/ C++ (always prefered than C ansi.
 *  4/ Extra libraries such as openCV for ex.
 *  4/ Qt.
 *  5/ KDE.
 *  6/ Local files.
 */
 
/// No need to include plugin_helloworld.h, it will be done through Qt moc file.
#include "plugin_helloworld.moc"

// Qt includes

#include <QPointer>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>

/// This is all libkipi header includes in this tool.

#include <libkipi/imagecollection.h>
#include <libkipi/imagecollectionselector.h>
#include <libkipi/interface.h>

/** This is all Kipi-plugins common includes used in this tool.
 *  Look into kipi-plugins/common/libkipiplugins/ for details.
 */

#include "kptooldialog.h"
#include "kpaboutdata.h"
#include "kpimageslist.h"

/// We will use Kipi-plugins namespace.
using namespace KIPIPlugins;

/// You must wrap all your plugin code to a dedicated namespace
namespace KIPIHelloWorldPlugin
{

/** Using private container everywhere is clear to speed up compilation and reduce source code depencies through header files.
 *  See this url for details : http://techbase.kde.org/Policies/Binary_Compatibility_Issues_With_C%2B%2B#Using_a_d-Pointer
 */
class Plugin_HelloWorld::Private
{
public:

    Private()
    {
        /// It's always clean to init pointers to zero. If crash appear, 
        /// debugger will show a null pointer instead a non initialized one.
        actionImages = 0;
        actionTools  = 0;
        actionExport = 0;
        iface   = 0;
    }

    /** These plugin actions will pluged into menu KIPI host application.
     */
    KAction*   actionImages;
    KAction*   actionTools;
    KAction*   actionExport;

    /** This is the interface instance to plugin host application. Note that you can get it everywhere in your plugin using
     *  instance of KIPI::PluginLoader singleton which provide a method for that.
     *  Since libkipi 2.0.0, KIPI host interface is also available from KIPI::Plugin::interface().
     */
    Interface* iface;
};

/** Macro from KDE KParts to create the factory for this plugin.
 *  The first argument is the name of the plugin library 
 *  and the second is the generic factory templated from
 *  the class for your plugin.
 */
K_PLUGIN_FACTORY(HelloWorldFactory, registerPlugin<Plugin_HelloWorld>();)

/** Macro from KDE KParts to export the symbols for this plugin
 *  NOTE: The plugin library is the name used in CMakeList.txt to link bin file, 
 *  and with X-KDE-Library value from .desktop file.
 */
K_EXPORT_PLUGIN(HelloWorldFactory("kipiplugin_helloworld") )

/** The plugin constructor. Note that plugin name passed as string in 3rd arguement of KIPI::Plugin parent class
 *  is the same than Name value from .desktop file.
 */
Plugin_HelloWorld::Plugin_HelloWorld(QObject* const parent, const QVariantList&)
    : Plugin(HelloWorldFactory::componentData(), parent, "HelloWorld"),
      /// Private container is allocated here.
      d(new Private)
{
    /// There is a debug space for plugin loading area. Please do not use qDebug and qWarning in plugin.
    kDebug(AREA_CODE_LOADING) << "Plugin_HelloWorld plugin loaded";
}

Plugin_HelloWorld::~Plugin_HelloWorld()
{
    /// Don't forget to clear d private container allocation in destructor to prevent memory leak.
    delete d;
}

void Plugin_HelloWorld::setup(QWidget* const widget)
{
    /** We pass the widget which host plugin in KIPI host application
     */
    Plugin::setup(widget);

    /** We will check KIPI host application interface instance validity
     */
    d->iface = dynamic_cast<Interface*>(parent());
    if (!d->iface)
    {
       /// No need special debug space outside load plugin area, it will be selected automatically.
       kError() << "Kipi interface is null!";
       return;
    }

    /** We define plugin action which will be plug in KIPI host application.
     *  Note that if you set keyboard shortcut to an action you must take a care
     *  about already existing one from other tool to prevent conflict.
     *  Don't forget to define an unique string name to your action, to be able to disable it 
     *  in KIPI host application if necessary. You must check of course name already used in 
     *  others tool before to prevent redondancy.
     */

    /** An action dedicated to be plugged in digiKam Image menu.
     */
    d->actionImages = actionCollection()->addAction("helloworld-actionImages");
    d->actionImages->setText(i18n("Hello World..."));
    d->actionImages->setIcon(KIcon("script-error"));
    d->actionImages->setShortcut(KShortcut(Qt::ALT+Qt::SHIFT+Qt::Key_H));

    /** Connect plugin action signal to dedicated slot.
     */
    connect(d->actionImages, SIGNAL(triggered(bool)),
            this, SLOT(slotActivateActionImages()));

    /** Action is registered in plugin instance.
     */
    addAction(d->actionImages);

    /** This will get items selection from KIPI host application.
     */
    ImageCollection selection = d->iface->currentSelection();
    d->actionImages->setEnabled(selection.isValid() && !selection.images().isEmpty());

    /** If selection change in KIPI host application, this signal will be fired, and plugin action enabled accordingly.
     */
    connect(d->iface, SIGNAL(selectionChanged(bool)),
            d->actionImages, SLOT(setEnabled(bool)));

    /** Another action dedicated to be plugged in digiKam Tool menu.
     */
    d->actionTools = actionCollection()->addAction("helloworld-actionTools");
    d->actionTools->setText(i18n("Hello World..."));
    d->actionTools->setIcon(KIcon("script-error"));
    d->actionTools->setShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_H));
    connect(d->actionTools, SIGNAL(triggered(bool)),
            this, SLOT(slotActivateActionTools()));
    addAction(d->actionTools);

    /** Another action dedicated to be plugged in digiKam Export menu.
     */
    d->actionExport = actionCollection()->addAction("helloworld-actionExport");
    d->actionExport->setText(i18n("Hello World..."));
    d->actionExport->setIcon(KIcon("script-error"));
    d->actionExport->setShortcut(KShortcut(Qt::SHIFT+Qt::CTRL+Qt::Key_H));
    connect(d->actionExport, SIGNAL(triggered(bool)),
            this, SLOT(slotActivateActionExport()));
    addAction(d->actionExport);
}

void Plugin_HelloWorld::slotActivateActionImages()
{
    /** When actionImages is actived, we display list of items selected in a message box.
     *  This example show a simple dialog with current items selected in KIPI host application.
     *  You can branch here your dedicated dialog to process items as you want. 
     */
    ImageCollection images = d->iface->currentSelection();

    if (images.isValid() && !images.images().isEmpty())
    {
        QStringList names;
        foreach (const KUrl& url, images.images())
            names << url.fileName();

        KMessageBox::informationList(0, i18n("This is the list of selected items"), names);
    }
}

void Plugin_HelloWorld::slotActivateActionTools()
{
    /** When actionTools is actived, we display a dedicated widget to select albums from kipi host application
     *  for post processing purpose. When selection is done, we display it in a message box.
     */

    /** We will use KPToolDialog class from kipi-plugins to display collection selector. This dialog offers some facility to 
        set data and rules about plugins, especially to wrap properlly tool with KDE bugilla. We use KPAboutData container
        for that.
     */
    KPAboutData* about = new KPAboutData(ki18n("Hello World"),
                             0,
                             KAboutData::License_GPL,
                             ki18n("A simple demo Kipi plugin"),
                             ki18n("(c) 2004-2012, Gilles Caulier"));

    about->addAuthor(ki18n("Gilles Caulier"),
                     ki18n("Developer"),
                     "caulier dot gilles at gmail dot com");

    about->setHandbookEntry("helloworld");

    QPointer<KPToolDialog> dlg        = new KPToolDialog(0);
    ImageCollectionSelector* selector = d->iface->imageCollectionSelector(dlg);
    dlg->setMainWidget(selector);
    dlg->setAboutData(about);
    dlg->exec();

    QList<ImageCollection> list = selector->selectedImageCollections();

    if (!list.isEmpty())
    {
        QStringList names;
        foreach (const ImageCollection& col, list)
            names << col.name();

        KMessageBox::informationList(0, i18n("This is the list of selected albums"), names);
    }

    delete dlg;
}

void Plugin_HelloWorld::slotActivateActionExport()
{
    /** When actionExport is actived, we display a dedicated widget from libkipiplugins which will show 
     *  and permit to manage current items selection from kipi host application for batch post-processing purpose.
     */

    KPAboutData* about = new KPAboutData(ki18n("Hello World"),
                             0,
                             KAboutData::License_GPL,
                             ki18n("A simple demo Kipi plugin"),
                             ki18n("(c) 2004-2012, Gilles Caulier"));

    about->addAuthor(ki18n("Gilles Caulier"),
                     ki18n("Developer"),
                     "caulier dot gilles at gmail dot com");

    about->setHandbookEntry("helloworld");

    QPointer<KPToolDialog> dlg = new KPToolDialog(0);
    KPImagesList* listView     = new KPImagesList(dlg);
    listView->setControlButtonsPlacement(KPImagesList::ControlButtonsRight);
    listView->setAllowRAW(true);
    listView->loadImagesFromCurrentSelection();
    dlg->setMainWidget(listView);
    dlg->setAboutData(about);
    dlg->exec();

    KUrl::List list = listView->imageUrls();

    if (!list.isEmpty())
    {
        QStringList names;
        foreach (const KUrl& col, list)
            names << col.fileName();

        KMessageBox::informationList(0, i18n("This is the list of items to process"), names);
    }

    delete dlg;
}

Category Plugin_HelloWorld::category(KAction* const action) const
{
    /** For each plugin actions defined, you can attribute a category which will plug it on right KIPI host application menu.
     */
    if (action == d->actionImages)
       return ImagesPlugin;

    if (action == d->actionTools)
       return ToolsPlugin;

    if (action == d->actionExport)
       return ExportPlugin;

    /// No need special debug space outside load plugin area, it will be selected automatically.
    kWarning() << "Unrecognized action for plugin category identification";
    return ImagesPlugin; // no warning from compiler, please
}

}  // namespace KIPIHelloWorldPlugin
