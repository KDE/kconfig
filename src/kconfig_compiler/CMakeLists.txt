

set(kconfig_compiler_SRCS
    KConfigParameters.cpp
    KConfigCodeGeneratorBase.cpp
    KConfigHeaderGenerator.cpp
    KConfigSourceGenerator.cpp
    KConfigXmlParser.cpp
    kconfig_compiler.cpp
)


add_executable(kconfig_compiler ${kconfig_compiler_SRCS})
set_target_properties(kconfig_compiler PROPERTIES
    OUTPUT_NAME "kconfig_compiler_kf5"
)

if(CMAKE_TOOLCHAIN_FILE)
    if(BUILD_TESTING)
        message(WARNING "Testing should be disabled on cross-compilation")
    endif()
else()
    add_executable(KF5::kconfig_compiler ALIAS kconfig_compiler)
endif()

target_link_libraries(kconfig_compiler Qt5::Xml)

ecm_mark_nongui_executable(kconfig_compiler)

install(TARGETS kconfig_compiler EXPORT KF5ConfigCompilerTargets DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF5})
