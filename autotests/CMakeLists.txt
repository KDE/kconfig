
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

find_package(Qt5Test ${REQUIRED_QT_VERSION} CONFIG QUIET)
find_package(Qt5Concurrent ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

if(NOT Qt5Concurrent_FOUND)
    message(STATUS "Qt5Concurrent not found, autotests will not be built.")
    return()
endif()

# compile KEntryMap into the test since it's not exported
set(kentrymaptest_SRCS kentrymaptest.cpp ../src/core/kconfigdata.cpp)
ecm_add_test(${kentrymaptest_SRCS}
  TEST_NAME kentrymaptest
  LINK_LIBRARIES KF5::ConfigCore Qt5::Test
)

# compile KConfigUtils into the test since it's not exported
set(test_kconfigutils_SRCS test_kconfigutils ../src/kconf_update/kconfigutils.cpp)
ecm_add_test(${test_kconfigutils_SRCS}
  TEST_NAME test_kconfigutils
  LINK_LIBRARIES KF5::ConfigCore Qt5::Test
)
target_include_directories(test_kconfigutils PRIVATE ../src/kconf_update)

ecm_add_tests(
   kconfignokdehometest.cpp
   kconfigtest.cpp
   kdesktopfiletest.cpp
   ksharedconfigtest.cpp
   test_kconf_update.cpp
   ksharedconfig_in_global_object
   NAME_PREFIX kconfigcore-
   LINK_LIBRARIES KF5::ConfigCore Qt5::Test Qt5::Concurrent
)

target_include_directories(test_kconf_update PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../src/kconf_update)

ecm_add_tests(
   kconfigguitest.cpp
   kconfigloadertest.cpp
   kconfigskeletontest.cpp
   kstandardshortcuttest.cpp
   NAME_PREFIX kconfiggui-
   LINK_LIBRARIES KF5::ConfigGui Qt5::Test
)

add_subdirectory(kconfig_compiler)

