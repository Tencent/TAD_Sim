cmake_minimum_required(VERSION 3.1...3.20)

project(txTraffic LANGUAGES CXX)

# set(EXECUTABLE_OUTPUT_PATH ../../${CMAKE_BINARY_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")
find_package(framework REQUIRED)

if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  set(PROTOBUF_LIBRARIES protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  find_package(glog CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(ZLIB REQUIRED)
else()
  # find cppzmq wrapper, installed by make of cppzmq
  set(CMAKE_SKIP_RPATH TRUE)
  find_package(Eigen3)

  # boost components
  find_package(Boost REQUIRED system filesystem)
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
  "."
  "../include"
  "../../../framework/src"
)

link_directories(${CMAKE_BINARY_DIR}/lib)

SET(project_SOURCES
  ./tx_tc_application.cpp
  ./tx_micro_traffic_cloud_test.cpp
  ./tx_tc_loop.cpp
  ./tx_tc_test_gflags.cpp
)
SET(project_HEADERS
  ./tx_tc_application.h
  ./tx_tc_loop.h
  ./tx_tc_marco.h
  ./tx_tc_type_def.h
  ./tx_tc_test_gflags.h
)

add_executable(txMicroTrafficCloudTest ${project_SOURCES} ${project_HEADERS})

target_compile_definitions(txMicroTrafficCloudTest PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

if(WIN32)
else()
  target_compile_options(txMicroTrafficCloudTest PRIVATE -fPIC)
endif()

if(WIN32 OR WithDependency)
  message(STATUS "cloud test add_dependencies")
  add_dependencies(txMicroTrafficCloudTest txMicroTrafficCloud)
endif()

if(WIN32)
  target_link_libraries(txMicroTrafficCloudTest
    txProtoBuf.lib
    ${FRAMEWORK_LIBRARY_SERVICE}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    glog::glog
    gflags::gflags
    ZLIB::ZLIB
    ${PROTOBUF_LIBRARIES}
    shlwapi.lib
  )
else()
  target_link_libraries(txMicroTrafficCloudTest
    txProtoBuf
    ${FRAMEWORK_LIBRARY_SERVICE}
    protobuf
    glog
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
# copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
# else()
# copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
# endif()
# endif()
