add_subdirectory(core)
if(TARGET Qt5::Gui)
    add_subdirectory(gui)
endif()
add_subdirectory(kconfig_compiler)
add_subdirectory(kconf_update)
add_subdirectory(kreadconfig)

if (BUILD_QCH)
    ecm_add_qch(
        KF5Config_QCH
        NAME KConfig
        BASE_NAME KF5Config
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KConfigCore_APIDOX_SRCS}
            ${KConfigGui_APIDOX_SRCS}
            "${CMAKE_CURRENT_SOURCE_DIR}/kconfig_compiler/README.dox"
            "${CMAKE_SOURCE_DIR}/docs/options.md"
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Core_QCH
            Qt5Xml_QCH
            Qt5Gui_QCH
        BLANK_MACROS
            KCONFIGCORE_EXPORT
            KCONFIGCORE_DEPRECATED_EXPORT
            KCONFIGCORE_DEPRECATED
            KCONFIGGUI_EXPORT
            KCONFIGGUI_DEPRECATED_EXPORT
            KCONFIGGUI_DEPRECATED
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
