# ============================================================
#
# This file is a part of digiKam project
# http://www.digikam.org
#
# Date        : 2012-02-16
# Description : a Hello World plugin using KDE XML-GUI techlnology.
#
# Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
# Copyright (C) 2012 by Victor Dodon <dodonvictor at gmail dot com>
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation;
# either version 2, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# ============================================================

ADD_SUBDIRECTORY(icons)

#                    ${CMAKE_CURRENT_BINARY_DIR}/../../libkipi
#                    ${CMAKE_CURRENT_SOURCE_DIR}/../../libkipi

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/plugin
                    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
                    ${CMAKE_CURRENT_BINARY_DIR}
		    )

SET(kipiplugin_photivotool_PART_SRCS
    plugin/plugin_photivotool.cpp)

KDE4_ADD_PLUGIN(kipiplugin_photivotool ${kipiplugin_photivotool_PART_SRCS})

TARGET_LINK_LIBRARIES(kipiplugin_photivotool
                      ${KDE4_KPARTS_LIBS}
                      ${QT_AND_KDECORE_LIBS}
                      ${KIPI_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
		      kipiplugins
                     )

CONFIGURE_FILE(plugin/kipiplugin_photivotool.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/plugin/kipiplugin_photivotool.desktop)

INSTALL(FILES   ${CMAKE_CURRENT_BINARY_DIR}/plugin/kipiplugin_photivotool.desktop DESTINATION ${SERVICES_INSTALL_DIR})
INSTALL(TARGETS kipiplugin_photivotool                                            DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES   plugin/kipiplugin_photivotoolui.rc                                DESTINATION ${DATA_INSTALL_DIR}/kipi)
