cmake_minimum_required(VERSION 3.1)

project(txsim-camera)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)

include_directories(
  ../../inc
  ${CMAKE_BINARY_DIR})

if(WIN32)
  set(LIB_PROTOBUF libprotobuf zlibstatic)
  set(LIB_SYS Shlwapi)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/deps/helper_funcs.cmake)
  include_directories(../../msgs/deps)
  link_directories(../../lib/Release)
  set(LIB_OPENCV opencv_core opencv_highgui opencv_imgcodecs opencv_imgproc)
elseif(UNIX)
  set(LIB_PROTOBUF protobuf pthread)
  set(LIB_SYS dl)
  find_package(Protobuf REQUIRED)
endif(WIN32)

protobuf_generate_cpp(PROTO_SRC_BASIC PROTO_HDR_BASIC ../../msgs/basic.proto)
protobuf_generate_cpp(PROTO_SRC_LOC PROTO_HDR_LOC ../../msgs/location.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_TFC ../../msgs/traffic.proto)
protobuf_generate_cpp(PROTO_SRC_TRJ PROTO_HDR_TRJ ../../msgs/trajectory.proto)

# example 1: built into a shared library.
add_library(my-camera SHARED
  camera_fun.cpp
  memory_io.cpp)
target_link_libraries(my-camera
  ${LIB_PROTOBUF}
  ${LIB_OPENCV}
)
