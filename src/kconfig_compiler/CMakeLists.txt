

set(kconfig_compiler_SRCS kconfig_compiler.cpp)


add_executable(kconfig_compiler_kf5 ${kconfig_compiler_SRCS})
add_executable(KF5::kconfig_compiler_kf5 ALIAS kconfig_compiler_kf5)

find_package(Qt5Xml 5.2.0 REQUIRED NO_MODULE)

target_link_libraries(kconfig_compiler_kf5 Qt5::Xml)

install(TARGETS kconfig_compiler_kf5 EXPORT KF5ConfigTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )
