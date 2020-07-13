remove_definitions(-DQT_NO_CAST_FROM_ASCII)

########### next target ###############

set(kconf_update_SRCS
    kconf_update.cpp
    kconfigutils.cpp
    )

ecm_qt_declare_logging_category(kconf_update_SRCS
    HEADER kconf_update_debug.h
    IDENTIFIER KCONF_UPDATE_LOG
    CATEGORY_NAME kf.config.kconf_update
    OLD_CATEGORY_NAMES kf5.kconfig.update
    DESCRIPTION "kconf_update"
    EXPORT KCONFIG
)

add_executable(kconf_update ${kconf_update_SRCS})
add_executable(KF5::kconf_update ALIAS kconf_update)
target_link_libraries(kconf_update Qt5::Core KF5::ConfigCore)
include(ECMMarkNonGuiExecutable)
ecm_mark_nongui_executable(kconf_update)

file(
  GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kconf-exec.h
  CONTENT "#define KCONF_UPDATE_EXECUTABLE \"$<TARGET_FILE:kconf_update>\""
)
configure_file(config-kconf.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kconf.h )

# Although this is mostly an internal binary (hence installing it in
# KF5_LIBEXEC_INSTALL_DIR), it is used by kded, and so we export its location
install(TARGETS kconf_update EXPORT KF5ConfigCompilerTargets DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF5})
