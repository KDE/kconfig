# SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KF6ConfigQml)
add_library(KF6::ConfigQml ALIAS KF6ConfigQml)

set_target_properties(KF6ConfigQml PROPERTIES
    VERSION     ${KCONFIG_VERSION}
    SOVERSION   ${KCONFIG_SOVERSION}
    EXPORT_NAME ConfigQml
)

target_sources(KF6ConfigQml PRIVATE
    kconfigpropertymap.cpp
)

ecm_generate_export_header(KF6ConfigQml
    BASE_NAME KConfigQml
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KConfig
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_link_libraries(KF6ConfigQml
  PUBLIC
    KF6::ConfigCore # KCoreConfigSkeleton, in ConfigPropertyMap
    Qt6::Qml
)

ecm_generate_headers(KConfigQml_HEADERS
    HEADER_NAMES
    KConfigPropertyMap

    REQUIRED_HEADERS KConfigQml_HEADERS
)
target_include_directories(KF6ConfigQml
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KConfig;${KDE_INSTALL_INCLUDEDIR_KF}/KConfigQml>")

install(TARGETS KF6ConfigQml EXPORT KF6ConfigTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kconfigqml_export.h
  ${KConfigQml_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigQml COMPONENT Devel
)


# The QML plugin
ecm_add_qml_module(kconfigqmlplugin URI org.kde.config)
target_sources(kconfigqmlplugin PRIVATE qmlplugin.cpp)
target_link_libraries(kconfigqmlplugin
    Qt6::Core
    Qt6::Qml
    KF6::ConfigCore
)

ecm_finalize_qml_module(kconfigqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR} CLASSNAME KConfigQmlPlugin)

