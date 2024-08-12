cmake_minimum_required(VERSION 3.1...3.20)

project(txTraffic LANGUAGES CXX)

set(EXECUTABLE_OUTPUT_PATH ../../${CMAKE_BINARY_DIR}/lib/)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")
find_package(framework REQUIRED)

#
if(WIN32)
# windows dependency installed manually
else()
  # find cppzmq wrapper, installed by make of cppzmq
  set(CMAKE_SKIP_RPATH TRUE)
  find_package(Eigen3)

  # boost components
  find_package(Boost REQUIRED system filesystem)
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  "../include"
  "."
  "../../../deps")
message(STATUS "${CMAKE_BINARY_DIR}")

if(WIN32)
  link_directories(
    ../${CMAKE_BINARY_DIR}/lib
    ../../../deps/lib)
else()
  link_directories(
    ../${CMAKE_BINARY_DIR}/lib)
endif()

SET(project_SOURCES
  ./tx_show_version.cpp
)

add_executable(txShowVersionApp ${project_SOURCES})

target_compile_definitions(txShowVersionApp PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

if(WIN32)
else()
  target_compile_options(txShowVersionApp PRIVATE -fPIC)
endif()

if(WIN32)
  target_link_libraries(txShowVersionApp
    libboost_filesystem.lib
    glog.lib
    gflags.lib
    zlibstatic.lib
    shlwapi.lib)
else()
  target_link_libraries(txShowVersionApp
    gflags
    boost_regex
    boost_filesystem
    boost_system
    boost_thread
    pthread
    "${Boost_SYSTEM_LIBRARY_RELEASE}"
    "${Boost_FILESYSTEM_LIBRARY_RELEASE}"
    dl
  )
endif()

# if(WIN32)
# if(NOT CMAKE_BUILD_TYPE)
# copy_framework_deps(${EXECUTABLE_OUTPUT_PATH}/Release)
# else()
# copy_framework_deps(${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
# endif()
# endif()
