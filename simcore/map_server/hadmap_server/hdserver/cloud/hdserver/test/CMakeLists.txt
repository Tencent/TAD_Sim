include(GoogleTest)
set(BINARY ${CMAKE_PROJECT_NAME}_tst)



file(
  GLOB_RECURSE TEST_SOURCES
  LIST_DIRECTORIES false
  test_*.h test_*.cc https_client.cc )
set(SOURCES ${TEST_SOURCES})
message("test sources:" ${SOURCES})
add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
message("xx")


if (${HDSERVER_USE_ASAN})  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -DADDRESS_SANITIZER")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLEAK_SANITIZER")
  add_executable(test_asan asan_test.cc)
  target_link_libraries(test_asan asan tsan)
endif()




# 链接src生成的lib库和gtest库
target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME} gtest )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-access-control")

enable_testing()
# 单个test
add_executable(xosc_replay_test xosc_replay_test.cc)
target_link_libraries(xosc_replay_test ${CMAKE_PROJECT_NAME} gtest gtest_main)
gtest_discover_tests(xosc_replay_test)
