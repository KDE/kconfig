

set(kconfig_compiler_SRCS kconfig_compiler.cpp)


add_executable(kconfig_compiler ${kconfig_compiler_SRCS})
set_target_properties(kconfig_compiler PROPERTIES
    OUTPUT_NAME "kconfig_compiler_kf5"
)
add_executable(KF5::kconfig_compiler ALIAS kconfig_compiler)

find_package(Qt5Xml 5.2.0 REQUIRED NO_MODULE)

target_link_libraries(kconfig_compiler Qt5::Xml)

install(TARGETS kconfig_compiler EXPORT KF5ConfigTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
