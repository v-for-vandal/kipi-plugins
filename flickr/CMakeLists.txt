#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

subdirs(icons)

add_definitions(-DTRANSLATION_DOMAIN=\"kipiplugin_flickr\")

set(kipiplugin_flickr_PART_SRCS
    plugin_flickr.cpp
    flickrwindow.cpp
    login.cpp
    flickrtalker.cpp
    flickrwidget.cpp
    flickrlist.cpp
    comboboxdelegate.cpp
    comboboxintermediate.cpp
    mpform.cpp
    selectuserdlg.cpp
   )

ki18n_wrap_ui(kipiplugin_flickr_PART_SRCS flickralbumdialog.ui)

add_library(kipiplugin_flickr MODULE ${kipiplugin_flickr_PART_SRCS})

target_link_libraries(kipiplugin_flickr
                      KF5kipiplugins
                      KF5::Kipi
                      KF5::KExiv2
                      KF5::KDcraw
                      KF5::KIOCore
                      KF5::Wallet
                      KF5::KHtml
                     )

configure_file(kipiplugin_flickr.desktop.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/kipiplugin_flickr.desktop)

install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/kipiplugin_flickr.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS kipiplugin_flickr                                     DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES   kipiplugin_flickrui.rc                                DESTINATION ${KXMLGUI_INSTALL_DIR}/kipi)