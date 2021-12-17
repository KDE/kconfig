# SPDX-FileCopyrightText: 2021 Carson Black
#
# SPDX-License-Identifier: LGPL-2.1-or-later

include(ECMGenerateQmlTypes)

set(
    kconfig_qml_plugin_sources

    src/plugin.cpp
    src/windowstate.cpp

    qml/qmldir
)

ecm_generate_qmltypes(
    org.kde.kconfig 1.0
    DESTINATION
        ${KDE_INSTALL_QMLDIR}/org/kde/kconfig
)

add_library(
    kconfigqmlplugin
    SHARED
        ${kconfig_qml_plugin_sources}
)

target_link_libraries(
    kconfigqmlplugin
    PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Quick
        Qt5::Qml
        KF5::ConfigGui
)

if (BUILD_TESTING)
    add_custom_target(copy_to_bin ALL
        COMMAND ${CMAKE_COMMAND} -E
            make_directory ${CMAKE_BINARY_DIR}/bin/org/kde/kconfig/

        COMMAND ${CMAKE_COMMAND} -E
            copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/qml
                ${CMAKE_BINARY_DIR}/bin/org/kde/kconfig/

        COMMAND ${CMAKE_COMMAND} -E
            copy $<TARGET_FILE:kconfigqmlplugin>
                ${CMAKE_BINARY_DIR}/bin/org/kde/kconfig/
    )

    add_subdirectory(autotests)
endif()

install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kconfig)
install(TARGETS kconfigqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kconfig )
