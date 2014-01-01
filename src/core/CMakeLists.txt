
find_package(Qt5Core 5.2.0 REQUIRED NO_MODULE)

set(libkconfigcore_SRCS
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
)

add_library(KF5ConfigCore ${libkconfigcore_SRCS})
generate_export_header(KF5ConfigCore BASE_NAME KConfigCore)
add_library(KF5::ConfigCore ALIAS KF5ConfigCore)

target_include_directories(KF5ConfigCore INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/kconfigcore>" "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KConfigCore>")

target_link_libraries(KF5ConfigCore PUBLIC Qt5::Core)
if(WIN32)
    target_link_libraries(KF5ConfigCore PRIVATE ${KDEWIN_LIBRARIES})
endif()

set_target_properties(KF5ConfigCore PROPERTIES VERSION   ${KCONFIG_VERSION_STRING}
                                               SOVERSION ${KCONFIG_SOVERSION}
                                               EXPORT_NAME ConfigCore
)

ecm_generate_headers(
  KAuthorized
  KConfig
  KConfigBackend
  KConfigBase
  KConfigGroup
  KDesktopFile
  KSharedConfig
  KCoreConfigSkeleton
  KEMailSettings
  ConversionCheck

  MODULE_NAME KConfigCore
  REQUIRED_HEADERS KConfigCore_HEADERS
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS KF5ConfigCore EXPORT KF5ConfigTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kconfigcore_export.h
  ${KConfigCore_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kconfigcore COMPONENT Devel
)
