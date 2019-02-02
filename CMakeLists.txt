cmake_minimum_required(VERSION 3.5)

set(KF5_VERSION "5.55.0") # handled by release scripts
project(KConfig VERSION ${KF5_VERSION})

include(FeatureSummary)
find_package(ECM 5.55.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION 5.10.0)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Xml)
option(KCONFIG_USE_GUI "Build components using Qt5Gui" ON)
if(KCONFIG_USE_GUI)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui)
endif()

option(KCONFIG_USE_DBUS "Build components using Qt5DBus" ON)
if(KCONFIG_USE_DBUS)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
endif()


include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)


include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)
include(ECMPoQmTools)
include(ECMAddQch)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT VARIABLE_PREFIX KCONFIG
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kconfig_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ConfigConfigVersion.cmake"
                        SOVERSION 5)


add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

include (ECMPoQmTools)
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
   ecm_install_po_files_as_qm(po)
endif()


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Config")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Config_QCH
        FILE KF5ConfigQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5ConfigQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5ConfigConfig.cmake.in"
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


install(EXPORT KF5ConfigCompilerTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5ConfigCompilerTargets.cmake NAMESPACE KF5:: COMPONENT Devel)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kconfig_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel )


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
