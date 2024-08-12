# ------------------------------------------------------------------------------
# Testing
#
# * set BUILD_LIB_GTEST whether to build gtest as submodule
# * Include this file when testing is enabled
# * Put your tests in the root/test folder
# * add a CMakeLists.txt and use add_gtest(...) macro
# ------------------------------------------------------------------------------

# define macro
macro(add_gtest TESTNAME)

    # link xsd2cpp_lib
    target_link_libraries(${TESTNAME} PRIVATE ${GTEST_BOTH_LIBRARIES} pthread)
    target_include_directories(${TESTNAME} PRIVATE ${GTEST_INCLUDE_DIRS})

    if (GOOGLE_TEST_INDIVIDUAL)
        if (CMAKE_VERSION VERSION_LESS 3.10)
            gtest_add_tests(TARGET ${TESTNAME}
                    TEST_PREFIX "${TESTNAME}."
                    TEST_LIST TmpTestList)
            set_tests_properties(${TmpTestList} PROPERTIES FOLDER "Tests")
        else ()
            gtest_discover_tests(${TESTNAME}
                    TEST_PREFIX "${TESTNAME}."
                    PROPERTIES FOLDER "Tests")
        endif ()
    else ()
        add_test(NAME ${TESTNAME} COMMAND $<TARGET_FILE:${TESTNAME}>)
        set_target_properties(${TESTNAME} PROPERTIES FOLDER "Tests")
    endif ()

endmacro()


# message
message(STATUS "Testing (gtest) enabled")

# enable testing
enable_testing()

# find gtest
find_package(GTest REQUIRED)
