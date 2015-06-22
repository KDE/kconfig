
# On Windows we have to generate the .h and .cpp inside ${CMAKE_BINARY_DIR}/bin because
# otherwise QFINDTESTDATA will not be able to locate them.
if(WIN32)
    set(KCFG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
else()
    set(KCFG_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()

# make sure the generated headers can be found
include_directories(${KCFG_OUTPUT_DIR})

macro(GEN_KCFG_TEST_SOURCE _testName _srcs)
   cmake_parse_arguments(ARG "" "KCFG" "" ${ARGN} )
   set(_kcfgFile ${ARG_KCFG})
   if (NOT _kcfgFile)
      set(_kcfgFile "${_testName}.kcfg")
   endif()
   add_custom_command(
      OUTPUT ${KCFG_OUTPUT_DIR}/${_testName}.cpp ${KCFG_OUTPUT_DIR}/${_testName}.h
      COMMAND KF5::kconfig_compiler ${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${CMAKE_CURRENT_SOURCE_DIR}/${_testName}.kcfgc
      WORKING_DIRECTORY ${KCFG_OUTPUT_DIR}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_kcfgFile} ${CMAKE_CURRENT_SOURCE_DIR}/${_testName}.kcfgc KF5::kconfig_compiler)

   # set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_testName}.h PROPERTIES GENERATED TRUE)
   qt5_generate_moc(${KCFG_OUTPUT_DIR}/${_testName}.h ${KCFG_OUTPUT_DIR}/${_testName}.moc )
   # do not run automoc on the generated file
   set_source_files_properties(${KCFG_OUTPUT_DIR}/${_testName}.cpp PROPERTIES SKIP_AUTOMOC TRUE)

   set( ${_srcs} ${${_srcs}} ${KCFG_OUTPUT_DIR}/${_testName}.cpp)
   set_property(SOURCE ${KCFG_OUTPUT_DIR}/${_testName}.cpp APPEND PROPERTY OBJECT_DEPENDS ${KCFG_OUTPUT_DIR}/${_testName}.moc )
endmacro(GEN_KCFG_TEST_SOURCE)

include(ECMMarkAsTest)

########### next target ###############

set(test1_SRCS test1main.cpp )


gen_kcfg_test_source(test1 test1_SRCS)

ecm_add_test(TEST_NAME test1 ${test1_SRCS})
target_link_libraries(test1 KF5::ConfigGui)


########### next target ###############

set(test2_SRCS test2main.cpp )


gen_kcfg_test_source(test2 test2_SRCS)

ecm_add_test(TEST_NAME test2 ${test2_SRCS})
target_link_libraries(test2 KF5::ConfigGui)


########### next target ###############

set(test3_SRCS test3main.cpp )


gen_kcfg_test_source(test3 test3_SRCS)

ecm_add_test(TEST_NAME test3 ${test3_SRCS})
target_link_libraries(test3 KF5::ConfigGui)


########### next target ###############

set(test3a_SRCS test3amain.cpp )


gen_kcfg_test_source(test3a test3a_SRCS)

ecm_add_test(TEST_NAME test3a ${test3a_SRCS})
target_link_libraries(test3a KF5::ConfigGui)


########### next target ###############

set(test4_SRCS test4main.cpp )


gen_kcfg_test_source(test4 test4_SRCS)

ecm_add_test(TEST_NAME test4 ${test4_SRCS})
target_link_libraries(test4 KF5::ConfigGui)


########### next target ###############

set(test5_SRCS test5main.cpp )


gen_kcfg_test_source(test5 test5_SRCS)

ecm_add_test(TEST_NAME test5 ${test5_SRCS})
target_link_libraries(test5 KF5::ConfigGui)


########### next target ###############

set(test6_SRCS test6main.cpp )


gen_kcfg_test_source(test6 test6_SRCS)

ecm_add_test(TEST_NAME test6 ${test6_SRCS})
target_link_libraries(test6 KF5::ConfigGui)


########### next target ###############

set(test7_SRCS test7main.cpp )


gen_kcfg_test_source(test7 test7_SRCS)

ecm_add_test(TEST_NAME test7 ${test7_SRCS})
target_link_libraries(test7 KF5::ConfigGui)


########### next target ###############

set(test8_SRCS test8main.cpp )


gen_kcfg_test_source(test8a test8_SRCS)
gen_kcfg_test_source(test8b test8_SRCS)

ecm_add_test(TEST_NAME test8 ${test8_SRCS})
target_link_libraries(test8 KF5::ConfigGui)


########### next target ###############

set(test9_SRCS test9main.cpp )


gen_kcfg_test_source(test9 test9_SRCS)

ecm_add_test(TEST_NAME test9 ${test9_SRCS})
target_link_libraries(test9 KF5::ConfigGui)


########### next target ###############

set(test10_SRCS test10main.cpp )


gen_kcfg_test_source(test10 test10_SRCS)

ecm_add_test(TEST_NAME test10 ${test10_SRCS})
target_link_libraries(test10 KF5::ConfigGui)


########### next target ###############

set(test11_SRCS test11main.cpp )


gen_kcfg_test_source(test11 test11_SRCS)
gen_kcfg_test_source(test11a test11_SRCS)

ecm_add_test(TEST_NAME test11 ${test11_SRCS})
target_link_libraries(test11 KF5::ConfigGui)


########### next target ###############

set(test12_SRCS test12main.cpp )

gen_kcfg_test_source(test12 test12_SRCS)

ecm_add_test(TEST_NAME test12 ${test12_SRCS})
target_link_libraries(test12 KF5::ConfigGui)


########### next target ###############

set(test13_SRCS test13main.cpp )

gen_kcfg_test_source(test13 test13_SRCS)

ecm_add_test(TEST_NAME test13 ${test13_SRCS})
target_link_libraries(test13 KF5::ConfigGui)


########### next target ###############

set(test_dpointer_SRCS test_dpointer_main.cpp )

gen_kcfg_test_source(test_dpointer test_dpointer_SRCS)

ecm_add_test(TEST_NAME test_dpointer ${test_dpointer_SRCS})
target_link_libraries(test_dpointer KF5::ConfigGui)


########### next target ###############

set(test_signal_SRCS test_signal_main.cpp )
gen_kcfg_test_source(test_signal test_signal_SRCS)
ecm_add_test(TEST_NAME test_signal ${test_signal_SRCS})
target_link_libraries(test_signal KF5::ConfigGui)

########### next target ###############

set(kconfigcompiler_test_signals_SRCS kconfigcompiler_test_signals.cpp)
gen_kcfg_test_source(signals_test_singleton kconfigcompiler_test_signals_SRCS KCFG signals_test.kcfg)
gen_kcfg_test_source(signals_test_no_singleton kconfigcompiler_test_signals_SRCS KCFG signals_test.kcfg)
gen_kcfg_test_source(signals_test_singleton_dpointer kconfigcompiler_test_signals_SRCS KCFG signals_test.kcfg)
gen_kcfg_test_source(signals_test_no_singleton_dpointer kconfigcompiler_test_signals_SRCS KCFG signals_test.kcfg)

ecm_add_test(${kconfigcompiler_test_signals_SRCS}
    TEST_NAME kconfigcompiler-signals-test
    LINK_LIBRARIES Qt5::Test KF5::ConfigGui
)


########### next target ###############

ecm_add_test(kconfigcompiler_test.cpp
    TEST_NAME kconfigcompiler-basic-test
    LINK_LIBRARIES Qt5::Test
)


########### next target ###############

set(test_qcategory_SRCS test_qdebugcategorymain.cpp test_qdebugcategory_debug.cpp)
gen_kcfg_test_source(test_qdebugcategory test_qcategory_SRCS)
ecm_add_test(TEST_NAME test_qdebugcategory ${test_qcategory_SRCS})
target_link_libraries(test_qdebugcategory KF5::ConfigGui)

########### next target ###############

set(test_translation_qt_SRCS test_translation_qt_main.cpp)
gen_kcfg_test_source(test_translation_qt test_translation_qt_SRCS KCFG test_translation.kcfg)

ecm_add_test(TEST_NAME test_translation_qt ${test_translation_qt_SRCS})
target_link_libraries(test_translation_qt KF5::ConfigGui)

########### next target ###############

set(test_translation_kde_SRCS test_translation_kde_main.cpp)
gen_kcfg_test_source(test_translation_kde test_translation_kde_SRCS KCFG test_translation.kcfg)

ecm_add_test(TEST_NAME test_translation_kde ${test_translation_kde_SRCS})
target_link_libraries(test_translation_kde KF5::ConfigGui)

########### next target ###############

set(test_translation_kde_domain_SRCS test_translation_kde_domain_main.cpp)
gen_kcfg_test_source(test_translation_kde_domain test_translation_kde_domain_SRCS KCFG test_translation.kcfg)

ecm_add_test(TEST_NAME test_translation_kde_domain ${test_translation_kde_domain_SRCS})
target_link_libraries(test_translation_kde_domain KF5::ConfigGui)

