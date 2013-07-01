/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2013-06-30
 * Description : a plugin for calling photivo, with sidecar
 *
 * Copyright (C) 2013 by Kevin Dalley <kevin at kelphead dot org>
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
 *
 *  Also, use C++ classes include header styles with Qt4,
 *  but do not use it with KDE4 header (use C ANSI style instead).
 */

/// No need to include plugin_photivotool.h, it will be done through Qt moc file.
#include "plugin_photivotool.moc"

// Qt includes

#include <QPointer>
#include <QDomDocument>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kdialog.h>
#include <kptooldialog.h>

/// This is all libkipi header includes in this tool.

#include <libkipi/imagecollection.h>
#include <libkipi/imagecollectionselector.h>
#include <libkipi/interface.h>
#include <libkipi/imageinfo.h>
#include <kprocess.h>
#include "kpimageinfo.h"
using namespace KIPIPlugins;

/** This is all Kipi-plugins common includes used in this tool.
 *  Look into kipi-plugins/common/libkipiplugins/ for details.
 */

/// You must wrap all your plugin code to a dedicated namespace
namespace KIPIPhotivoToolPlugin
{

/** We will use KPToolDialog class from kipi-plugins to display plugin dialogs. It offers some facilities to
    set data and rules about plugins, especially to wrap properly tool with KDE bugilla. We use KPAboutData container
    for that.
*/

/** Using private container everywhere is clear to speed up compilation and reduce source code depencies through header files.
 *  See this url for details : http://techbase.kde.org/Policies/Binary_Compatibility_Issues_With_C%2B%2B#Using_a_d-Pointer
 */
class Plugin_PhotivoTool::Private
{
public:

  /// It's always clean to init pointers to zero. If crash appear,
  /// debugger will show a null pointer instead a non initialized one.
  Private():actionTools(0)
    {
    }

    /** These plugin actions will pluged into menu KIPI host application.
     */
    KAction*   actionTools;
};

/** Macro from KDE KParts to create the factory for this plugin.
 *  The first argument is the name of the plugin library
 *  and the second is the generic factory templated from
 *  the class for your plugin.
 */
K_PLUGIN_FACTORY(PhotivoToolFactory, registerPlugin<Plugin_PhotivoTool>();)

/** Macro from KDE KParts to export the symbols for this plugin
 *  NOTE: The plugin library is the name used in CMakeList.txt to link bin file,
 *  and with X-KDE-Library value from .desktop file.
 */
K_EXPORT_PLUGIN(PhotivoToolFactory("kipiplugin_photivotool") )

/** The plugin constructor. Note that plugin name passed as string in 3rd arguement of KIPI::Plugin parent class
 *  is the same than Name value from .desktop file.
 */
Plugin_PhotivoTool::Plugin_PhotivoTool(QObject* const parent, const QVariantList&)
    : Plugin(PhotivoToolFactory::componentData(), parent, "PhotivoTool"),
      /// Private container is allocated here.
      d(new Private)
{
    kDebug() << "Plugin_PhotivoTool plugin loaded";

    /** This is needed to setup the plugin gui and to merge with the kipi host
     *  application gui.
     *  The name of the UI file must be: nameofpluginui.rc, where "nameofplugin"
     *  is the name given to the plugin factory, usualy: kipiplugin_<name> .
     *  UI file of the plugin must be installed in kipi data dir.
     */
    setUiBaseName("kipiplugin_photivotoolui.rc");

    /** We need to call setupXML so the XML file and the GUI of the plugin to
      * be merged with those of the KIPI host app
      */
    setupXML();
}

Plugin_PhotivoTool::~Plugin_PhotivoTool()
{
    /// Don't forget to clear d private container allocation in destructor to prevent memory leak.
    delete d;
}

void Plugin_PhotivoTool::setup(QWidget* const widget)
{
    /** Each plugin must overload Plugin::setup method.
     *  We pass the widget which host plugin in KIPI host application
     */
    Plugin::setup(widget);

    /** This is the interface instance to plugin host application. Note that you can get it everywhere in your plugin using
     *  instance of KIPI::PluginLoader singleton which provide a method for that.
     *  Since libkipi 2.0.0, KIPI host interface is also available from KIPI::Plugin::interface().
     */
    if (!interface())
        return;

    /** We will enable plugin actions only if the KIPI interface is not null
      */
    setupActions();
}

void Plugin_PhotivoTool::setupActions()
{
    /** We define plugin action which will be plug in KIPI host application.
     *  Note that if you set keyboard shortcut to an action you must take a care
     *  about already existing one from other tool to prevent conflict.
     *  Don't forget to define an unique string name to your action, to be able to disable it
     *  in KIPI host application if necessary. You must check of course name already used in
     *  others tool before to prevent redondancy.
     */

    /** We need to call setDefaultCategory in case the plugin loader cannot
      * recognize the category of an action
      */
    setDefaultCategory(ToolsPlugin);

    /** An action dedicated to be plugged in digiKam Image menu.
     */
    d->actionTools = new KAction(this);
    d->actionTools->setText(i18n("Photivo..."));
    d->actionTools->setIcon(KIcon("kipi-photivo"));
    d->actionTools->setEnabled(false);
    d->actionTools->setShortcut(KShortcut(Qt::ALT + Qt::SHIFT + Qt::CTRL + Qt::Key_F1));

    /** Connect plugin action signal to dedicated slot.
     */
    connect(d->actionTools, SIGNAL(triggered(bool)),
            this, SLOT(slotActivateActionTools()));
    /** We need to register actions in plugin instance
     */
    addAction("photivotool-actionTool", d->actionTools, ToolsPlugin);

    /** This will get items selection from KIPI host application.
     */
    ImageCollection selection = interface()->currentSelection();
    d->actionTools->setEnabled(selection.isValid() && !selection.images().isEmpty());

    /** If selection change in KIPI host application, this signal will be fired, and plugin action enabled accordingly.
     */
    connect(interface(), SIGNAL(selectionChanged(bool)),
            d->actionTools, SLOT(setEnabled(bool)));

    connect(interface(), SIGNAL(currentAlbumChanged(bool)),
            d->actionTools, SLOT(setEnabled(bool)));
}

void Plugin_PhotivoTool::slotActivateActionTools()
{
  /**
   * Run items through photivo
   */
  kDebug() << "slotActivateActionImages entered";

  ImageCollection images = interface()->currentSelection();

  if (images.isValid())
    {
      if (images.images().count() != 1) {
        KMessageBox::sorry(0, i18n("Exactly one image must be chosen"));
	return;
      }
      foreach (const KUrl& url, images.images()) {
	KProcess process;
	process.clearProgram();
	QStringList args;
	args << "photivo";
	args << "-i" << url.path();
	if (KExiv2::hasSidecar(url.path())) {
	  args << "--sidecar" << KExiv2::sidecarPath(url.path());
	}
	//	}
	const QMap< QString, QVariant > attr =  interface()->info(url).attributes();
	kDebug() << "Starting process\n";
	process.setProgram(args);
	kDebug() << "Program: " << process.program();
	process.startDetached();
      }

    }
  }
}  // namespace KIPIPhotivoToolPlugin
