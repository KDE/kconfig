find_package(Qt5Core 5.2.0 REQUIRED NO_MODULE)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

########### next target ###############

set(kconf_update_SRCS
    kconf_update.cpp
    kconfigutils.cpp
    )

add_executable(kconf_update ${kconf_update_SRCS})
add_executable(KF5::kconf_update ALIAS kconf_update)
target_link_libraries(kconf_update Qt5::Core KF5::ConfigCore)
include(ECMMarkNonGuiExecutable)
ecm_mark_nongui_executable(kconf_update)

get_target_property(KCONF_UPDATE_EXECUTABLE kconf_update LOCATION)
configure_file(config-kconf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kconf.h )

# Although this is mostly an internal binary (hence installing it in
# KF5_LIBEXEC_INSTALL_DIR), it is used by kded, and so we export its location
install(TARGETS kconf_update EXPORT KF5ConfigTargets DESTINATION ${KF5_LIBEXEC_INSTALL_DIR})
