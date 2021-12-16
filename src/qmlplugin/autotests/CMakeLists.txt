find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED QuickTest)

add_executable(qtest test_main.cpp)
target_link_libraries(qtest Qt5::QuickTest)

macro(add_qml_test)
    foreach(test ${ARGV})
        add_custom_target(copy_${test}_to_bin ALL
            COMMAND ${CMAKE_COMMAND} -E
                copy ${CMAKE_CURRENT_SOURCE_DIR}/${test}
                    ${CMAKE_BINARY_DIR}/bin
        )

        add_test(NAME ${test}
                 COMMAND qtest -input ${test} -import ${CMAKE_BINARY_DIR}/bin
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endforeach()
endmacro()

add_qml_test(
    tst_00_BasicFunctionality.qml
)
