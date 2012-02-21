/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-20
 * Description : Metadata interface for kipi-plugins.
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef KPMETADATA_H
#define KPMETADATA_H

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>

// Local includes

#include "kpmetasettings.h"
#include "kipiplugins_export.h"

namespace KIPI
{
    class Interface;
}

using namespace KIPI;
using namespace KExiv2Iface;

namespace KIPIPlugins
{

/** This class must be used in plugins instead KExiv2 metadata interface to handle file lock mechanism
 *  with KIPI host application to prevent concurent operations on file during load and save operations.
 *  This class is also able to use KIPI host application metadata settings through KPMetaSettings container.
 *  This settings must be taken from KIPI host application using KPHostSettings interface.
 */
class KIPIPLUGINS_EXPORT KPMetadata : public KExiv2
{

public:

    /** Standard constructor. Metadata settings is taken from KIPI host application through KIPI interface.
     *  If interface is null, default settings is used, and no file lock will be performed with read and write operations.
     * With this construcotr, load metadata form file using load() method.
     */
    KPMetadata(Interface* const iface);

    /** Constructor to load metadata from file. Same behavior than standard constructor.
     */
    KPMetadata(const QString& filePath, Interface* const iface);

    /** Apply metadata settings from KIPI host application to this interface. To use before load and save operations.
     */
    void setSettings(const KPMetaSettings& settings);

    /** Load metadata operation from a file.
     *  Re-implemented from libKexiv2 to use lock mechanism with KIPI host application through KIPI::Interface.
     */
    bool load(const QString& filePath) const;

    /** Save metadata operation to a file.
     *  Re-implemented from libKexiv2 to use lock mechanism with KIPI host application through KIPI::Interface.
     */
    bool save(const QString& filePath) const;

    /** Perform save metadata operation to current loaded file.
     *  Re-implemented from libKexiv2 to use lock mechanism with KIPI host application through KIPI::Interface.
     */
    bool applyChanges() const;

private:


    KPMetadata()  {};  // Disable
    ~KPMetadata() {};  // Disable

    void init();

private:

    Interface* m_iface;
};

}  // namespace KIPIPlugins

#endif /* KPMETADATA_H */
