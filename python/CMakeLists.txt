# SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

set(bindings_library "KConfigCore")

set(wrapped_header ${CMAKE_CURRENT_SOURCE_DIR}/bindings.h)
set(typesystem_file ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_kconfigcore.xml)

set(generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kconfigcore_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kauthorized_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kconfig_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kconfigbase_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kconfiggroup_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kconfigskeletonitem_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kconfigwatcher_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kcoreconfigskeleton_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kcoreconfigskeleton_itemdouble_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kcoreconfigskeleton_itemenum_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kcoreconfigskeleton_itemenum_choice_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kcoreconfigskeleton_itemstring_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kdesktopfileaction_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kemailsettings_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/kpropertyskeletonitem_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/KConfigCore/ksharedconfig_wrapper.cpp
)

ecm_generate_python_bindings(
    PACKAGE_NAME ${bindings_library}
    VERSION ${KF_VERSION}
    WRAPPED_HEADER ${wrapped_header}
    TYPESYSTEM ${typesystem_file}
    EXPORT_TYPESYSTEM
    GENERATED_SOURCES ${generated_sources}
    DEPENDENCIES KF6::ConfigCore
    QT_VERSION ${REQUIRED_QT_VERSION}
    HOMEPAGE_URL "https://invent.kde.org/frameworks/kconfig"
    ISSUES_URL "https://bugs.kde.org/describecomponents.cgi?product=frameworks-kconfig"
    AUTHOR "The KDE Community"
    README ../README.md
)

target_link_libraries(${bindings_library} PRIVATE KF6ConfigCore)

execute_process(COMMAND ${Python_EXECUTABLE} -Esc "import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': '${CMAKE_INSTALL_PREFIX}', 'base': '${CMAKE_INSTALL_PREFIX}'}))" OUTPUT_VARIABLE sysconfig_output)
string(STRIP ${sysconfig_output} PYTHON_INSTALL_DIR)

install(TARGETS ${bindings_library} LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}")
