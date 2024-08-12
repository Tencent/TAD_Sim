cmake_minimum_required(VERSION 3.4.0)

# project name
set(CARSIM_NAME carsim_interface)

project(${CARSIM_NAME})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(_OPEN_LOOP_ "build openloop library" OFF)

# for windows.h
add_definitions(-DNOMINMAX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# install
file(GLOB par_files ./para/*.sim ./para/*.par)
set(carsim_events ./para/Events/)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../simcore/cmake")
find_package(framework REQUIRED)

include_directories(./src ./ ${CMAKE_CURRENT_BINARY_DIR} ../../common/message/build)

# if build openloop or closeloop
if(_OPEN_LOOP_)
  add_definitions(-D_OPEN_LOOP_)
endif(_OPEN_LOOP_)

if(WIN32)
  # used in carsim
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DWIN64)
  add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)

  set(carsim_common_folder ./src/carsim/win)
  include_directories(${carsim_common_folder} ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)
  aux_source_directory(${carsim_common_folder} carsim_srcs)

  set(LIB_SYS shlwapi)

  find_package(protobuf CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
  set(LIB_PROTOBUF protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite ZLIB::ZLIB)

  find_package(glog CONFIG REQUIRED)
  set(LIB_LOG glog::glog gflags::gflags)

elseif(UNIX)
  set(carsim_common_folder ./src/carsim/linux)

  include_directories(/usr/include ${carsim_common_folder})
  link_directories(/usr/lib /usr/local/lib)

  set(LIB_PROTOBUF protobuf)
  set(LIB_LOG glog gflags)
  set(LIB_SYS pthread)

  set(carsim_srcs ${carsim_common_folder}/vs_get_api.c)
  find_package(Protobuf REQUIRED)
endif(WIN32)

add_subdirectory(./src/softecu)

aux_source_directory(./src src_files)

add_library(
  ${CARSIM_NAME} SHARED
  ${src_files}
  ${carsim_srcs}
  ../../common/message/build/location.pb.cc
  ../../common/message/build/vehState.pb.cc
  ../../common/message/build/control.pb.cc
  ../../common/message/build/basic.pb.cc
  ../../common/message/build/header.pb.cc
)

target_link_libraries(${CARSIM_NAME} ${softecu} ${LIB_PROTOBUF} ${LIB_LOG} ${LIB_SYS})

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
