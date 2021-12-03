add_library(KF5ConfigCore)
add_library(KF5::ConfigCore ALIAS KF5ConfigCore)

target_sources(KF5ConfigCore PRIVATE
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

ecm_qt_declare_logging_category(KF5ConfigCore
    HEADER kconfig_core_log_settings.h
    IDENTIFIER KCONFIG_CORE_LOG
    CATEGORY_NAME kf.config.core
    OLD_CATEGORY_NAMES kf5.kconfig.core
    DESCRIPTION "KConfig Core"
    EXPORT KCONFIG
)

configure_file(config-kconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kconfig.h )

ecm_generate_export_header(KF5ConfigCore
    BASE_NAME KConfigCore
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.0 5.0 5.24 5.42 5.82 5.89
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_compile_definitions(KF5ConfigCore
    PRIVATE
        KCONF_UPDATE_INSTALL_LOCATION="${KDE_INSTALL_FULL_LIBEXECDIR_KF5}/$<TARGET_FILE_NAME:KF5::kconf_update>"
)

target_include_directories(KF5ConfigCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KConfigCore>")

target_link_libraries(KF5ConfigCore PUBLIC Qt${QT_MAJOR_VERSION}::Core)

if(KCONFIG_USE_DBUS)
    target_link_libraries(KF5ConfigCore PRIVATE Qt${QT_MAJOR_VERSION}::DBus)
endif()

set_target_properties(KF5ConfigCore PROPERTIES VERSION   ${KCONFIG_VERSION}
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

find_package(PythonModuleGeneration)

if (PythonModuleGeneration_FOUND)
  ecm_generate_python_binding(
    TARGET KF5::ConfigCore
    PYTHONNAMESPACE PyKF5
    MODULENAME KConfigCore
    RULES_FILE "${CMAKE_SOURCE_DIR}/cmake/rules_PyKF5.py"
    SIP_DEPENDS
      QtCore/QtCoremod.sip
    HEADERS
      kconfig.h
      kconfigbase.h
      kconfiggroup.h
      kdesktopfile.h
      ksharedconfig.h
      kcoreconfigskeleton.h
      kemailsettings.h
      conversioncheck.h
      kconfigwatcher.h
  )
endif()

install(TARGETS KF5ConfigCore EXPORT KF5ConfigTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kconfigcore_export.h
  ${KConfigCore_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigCore COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(KConfigCore_APIDOX_SRCS ${KConfigCore_HEADERS} PARENT_SCOPE)
set(KConfigCore_APIDOX_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KConfigCore LIB_NAME KF5ConfigCore DEPS "core" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigCore)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
