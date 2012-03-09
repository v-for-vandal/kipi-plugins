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

#include "kpmetadata.h"

// LibKipi includes

#include <libkipi/version.h>
#include <libkipi/interface.h>

// Local includes

#include "kpmetasettings.h"
#include "kphostsettings.h"

namespace KIPIPlugins
{

KPMetadata::KPMetadata()
    : KExiv2()
{
    m_iface = 0;
    setSettings(KPMetaSettings());
}

KPMetadata::KPMetadata(const QString& filePath)
    : KExiv2()
{
    m_iface = 0;
    setSettings(KPMetaSettings());
    load(filePath);
}

KPMetadata::KPMetadata(Interface* const iface)
    : KExiv2()
{
    m_iface = iface;
    KPHostSettings hset(m_iface);
    setSettings(hset.metadataSettings());
}

KPMetadata::KPMetadata(const QString& filePath, Interface* const iface)
    : KExiv2()
{
    m_iface = iface;
    KPHostSettings hset(m_iface);
    setSettings(hset.metadataSettings());
    load(filePath);
}

KPMetadata::~KPMetadata()
{
}

/*
KPMetadata::KPMetadata(const KPMetadata& other)
    : KExiv2(other.data()), d(other.d)
{
    setFilePath(other.getFilePath());
}
*/

/*
KPMetadata& KPMetadata::operator=(const KPMetadata& other)
{
    setData(other.data());
    d = other.d;
    setFilePath(other.getFilePath());
    return *this;
}
*/
void KPMetadata::setSettings(const KPMetaSettings& settings)
{
    setUseXMPSidecar4Reading(settings.useXMPSidecar4Reading);
    setWriteRawFiles(settings.writeRawFiles);
    setMetadataWritingMode(settings.metadataWritingMode);
    setUpdateFileTimeStamp(settings.updateFileTimeStamp);
}

bool KPMetadata::load(const QString& filePath) const
{
#if KIPI_VERSION >= 0x010500
    if (m_iface)
        FileReadLocker(m_iface, KUrl(filePath));
#endif

    return KExiv2::load(filePath);
}

bool KPMetadata::save(const QString& filePath) const
{
#if KIPI_VERSION >= 0x010500
    if (m_iface)
        FileWriteLocker(m_iface, KUrl(filePath));
#endif

    return KExiv2::save(filePath);
}

bool KPMetadata::applyChanges() const
{
#if KIPI_VERSION >= 0x010500
    if (m_iface)
        FileWriteLocker(m_iface, KUrl(getFilePath()));
#endif

    return KExiv2::applyChanges();
}

}  // namespace KIPIPlugins
