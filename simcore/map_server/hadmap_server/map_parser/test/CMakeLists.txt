find_package(GTest REQUIRED)

enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-access-control")

# 1
set(BINARY ${PROJECT_NAME}_tst)
add_executable(${BINARY} test_main.cc test_map_data_cache.cc test_synchronizer.cc)
target_link_libraries(${BINARY} PUBLIC ${PROJECT_NAME} gtest)
add_test(NAME ${BINARY} COMMAND ${BINARY})

# 2
add_executable(hadmap_server_test ${COMMON_TEST} ${TST_FILES})
target_link_libraries(hadmap_server_test ${PROJECT_NAME} gtest gtest_main)
gtest_discover_tests(hadmap_server_test)
