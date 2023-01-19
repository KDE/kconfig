add_library(KF6ConfigCore)
add_library(KF6::ConfigCore ALIAS KF6ConfigCore)

target_sources(KF6ConfigCore PRIVATE
   kconfig.cpp
   kconfigbase.cpp
   kconfigdata.cpp
   kconfiggroup.cpp
   kconfigbackend.cpp
   kconfigini.cpp
   kdesktopfile.cpp
   ksharedconfig.cpp
   kcoreconfigskeleton.cpp
   kauthorized.cpp
   kemailsettings.cpp
   kconfigwatcher.cpp
)

ecm_qt_declare_logging_category(KF6ConfigCore
    HEADER kconfig_core_log_settings.h
    IDENTIFIER KCONFIG_CORE_LOG
    CATEGORY_NAME kf.config.core
    OLD_CATEGORY_NAMES kf5.kconfig.core
    DESCRIPTION "KConfig Core"
    EXPORT KCONFIG
)

configure_file(config-kconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kconfig.h )

ecm_generate_export_header(KF6ConfigCore
    BASE_NAME KConfigCore
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.0 5.0 5.24 5.42 5.82 5.89
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_compile_definitions(KF6ConfigCore
    PRIVATE
        KCONF_UPDATE_INSTALL_LOCATION="${KDE_INSTALL_FULL_LIBEXECDIR_KF}/$<TARGET_FILE_NAME:KF6::kconf_update>"
)

target_include_directories(KF6ConfigCore
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KConfig;${KDE_INSTALL_INCLUDEDIR_KF}/KConfigCore>"
)

target_link_libraries(KF6ConfigCore PUBLIC Qt${QT_MAJOR_VERSION}::Core)

if(KCONFIG_USE_DBUS)
    target_link_libraries(KF6ConfigCore PRIVATE Qt${QT_MAJOR_VERSION}::DBus)
endif()

set_target_properties(KF6ConfigCore PROPERTIES VERSION   ${KCONFIG_VERSION}
                                               SOVERSION ${KCONFIG_SOVERSION}
                                               EXPORT_NAME ConfigCore
)

ecm_generate_headers(KConfigCore_HEADERS
  HEADER_NAMES
  KAuthorized
  KConfig
  KConfigBase
  KConfigGroup
  KDesktopFile
  KSharedConfig
  KCoreConfigSkeleton
  KEMailSettings
  ConversionCheck
  KConfigWatcher

  REQUIRED_HEADERS KConfigCore_HEADERS
)

install(TARGETS KF6ConfigCore EXPORT KF6ConfigTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kconfigcore_export.h
  ${KConfigCore_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigCore COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(KConfigCore_APIDOX_SRCS ${KConfigCore_HEADERS} PARENT_SCOPE)
set(KConfigCore_APIDOX_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

include(ECMGeneratePriFile)
ecm_generate_pri_file(
  BASE_NAME KConfigCore
  LIB_NAME KF6ConfigCore
  DEPS "core"
  FILENAME_VAR PRI_FILENAME
  INCLUDE_INSTALL_DIRS
    ${KDE_INSTALL_INCLUDEDIR_KF}/KConfig
    ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigCore
)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
