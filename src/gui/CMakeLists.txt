
find_package(Qt5Gui ${QT_REQUIRED_VERSION} REQUIRED NO_MODULE)
find_package(Qt5Xml ${QT_REQUIRED_VERSION} REQUIRED NO_MODULE)

set(libkconfiggui_SRCS
   kconfiggui.cpp
   kconfiggroupgui.cpp
   kconfigloader.cpp
   kconfigskeleton.cpp
   kstandardshortcut.cpp
   kwindowconfig.cpp
)

add_library(KF5ConfigGui ${libkconfiggui_SRCS})
generate_export_header(KF5ConfigGui BASE_NAME KConfigGui)
add_library(KF5::ConfigGui ALIAS KF5ConfigGui)

target_include_directories(KF5ConfigGui INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KConfigGui>")

target_link_libraries(KF5ConfigGui PUBLIC Qt5::Gui Qt5::Xml KF5::ConfigCore)

set_target_properties(KF5ConfigGui PROPERTIES VERSION   ${KCONFIG_VERSION_STRING}
                                              SOVERSION ${KCONFIG_SOVERSION}
                                              EXPORT_NAME ConfigGui
)

ecm_generate_headers(KConfigGui_HEADERS
  HEADER_NAMES
  KConfigGui
  KConfigLoader
  KConfigSkeleton
  KStandardShortcut
  KWindowConfig

   REQUIRED_HEADERS KConfigGui_HEADERS
)

install(TARGETS KF5ConfigGui EXPORT KF5ConfigTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kconfiggui_export.h
  ${KConfigGui_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/KConfigGui COMPONENT Devel
)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KConfigGui LIB_NAME KF5ConfigGui DEPS "gui xml KConfigCore" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
