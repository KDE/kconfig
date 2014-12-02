cmake_minimum_required(VERSION 2.8.12)

project(KConfig)

find_package(ECM 1.5.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION 5.2.0)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui Xml)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)
include(ECMPoQmTools)

set(KF5_VERSION "5.5.0") # handled by release scripts

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KCONFIG
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kconfig_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigConfigVersion.cmake"
                        SOVERSION 5)


add_subdirectory(src)
add_subdirectory(autotests)

include (ECMPoQmTools)
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
   ecm_install_po_files_as_qm(po)
endif()


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Config")

include(ECMPackageConfigHelpers)

ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5ConfigConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigConfigVersion.cmake"
               "${CMAKE_CURRENT_SOURCE_DIR}/KF5ConfigMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )


install(EXPORT KF5ConfigTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5ConfigTargets.cmake NAMESPACE KF5:: COMPONENT Devel)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kconfig_version.h
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR} COMPONENT Devel )


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
