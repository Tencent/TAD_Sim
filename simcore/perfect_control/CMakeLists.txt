cmake_minimum_required(VERSION 3.13.0)

project(txSimPerfectControl)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(grading_root ${CMAKE_SOURCE_DIR}/../grading/)

include_directories(
  ${grading_root}
  ${grading_root}/manager
  ../../common/message/build
  ../../common/message
  ${CMAKE_BINARY_DIR})

if(WIN32)
  add_compile_options(/utf-8)
  add_definitions(-D_WIN32_WINNT=0x601)
  add_definitions(-DGLOG_USE_GLOG_EXPORT)
  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/eigen3
  )
  link_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib
  )
  find_package(protobuf CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(gflags CONFIG REQUIRED)
  find_package(glog CONFIG REQUIRED)

  set(DEP_LIBS_PROTO protobuf::libprotobuf ZLIB::ZLIB)
  set(lib_glog shlwapi glog::glog gflags::gflags)
else()
  add_definitions(-fPIC)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules)
  set(CMAKE_SKIP_RPATH TRUE)
  find_package(Eigen3)
  include_directories(
    ${EIGEN3_INCLUDE_DIR}
  )
  link_directories()
  set(DEP_LIBS_PROTO
    protobuf)
  set(lib_glog glog gflags)
endif(WIN32)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/helper_funcs.cmake)
protobuf_generate_cpp(PROTO_SRC_BASIC PROTO_HDR_BASIC ../../common/message/basic.proto)
protobuf_generate_cpp(PROTO_SRC_HEADER PROTO_HDR_HEADER ../../common/message/header.proto)
protobuf_generate_cpp(PROTO_SRC_LOCATION PROTO_HDR_LOCATION ../../common/message/location.proto)
protobuf_generate_cpp(PROTO_SRC_TRAJECTORY PROTO_HDR_TRAJECTORY ../../common/message/trajectory.proto)
protobuf_generate_cpp(PROTO_SRC_LOCALLOCATION PROTO_HDR_LOCALLOCATION ../../common/message/localLocation.proto)

add_library(txSimPerfectControl SHARED
  perfect_control_with_option.h
  perfect_control_with_option.cpp
  ${PROTO_SRC_BASIC}
  ${PROTO_SRC_HEADER}
  ${PROTO_SRC_LOCATION}
  ${PROTO_SRC_TRAJECTORY}
  ${PROTO_SRC_LOCALLOCATION}
)

target_link_libraries(txSimPerfectControl
  ${DEP_LIBS_PROTO}
  ${lib_glog}
  ${MAPSDK_LIBRARYS}
)

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
