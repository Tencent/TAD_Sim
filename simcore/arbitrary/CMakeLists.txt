cmake_minimum_required(VERSION 3.4.0)

set(ARBITRARY_PROJ_NAME sim_arbitrary)

project(${ARBITRARY_PROJ_NAME})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(ARBITRARY_ROOT ${CMAKE_SOURCE_DIR})
set(MSG_DIR ${ARBITRARY_ROOT}/../../common/message/build)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(framework REQUIRED)

include_directories(${MSG_DIR} ${ARBITRARY_ROOT}/inc)

if(WIN32)
  add_compile_options("/MP")
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DGOOGLE_GLOG_DLL_DECL=)

  add_definitions(-DBUILD_ARBITRARY_DLL)

  include_directories(${carsim_common_folder} ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

  set(LIB_SYS shlwapi)

  find_package(protobuf CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
  set(LIB_PROTOBUF protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite ZLIB::ZLIB)

  find_package(glog CONFIG REQUIRED)
  set(LIB_LOG glog::glog gflags::gflags)
else()
  add_compile_options(-Wall)
  add_compile_options(-fPIC)

  link_directories(/usr/lib/x86_64-linux-gnu /usr/local/lib)

  set(LIB_SYS pthread)
  set(LIB_PROTOBUF protobuf)
  set(LIB_LOG glog gflags)
endif(WIN32)

aux_source_directory(${ARBITRARY_ROOT}/src src_files)

add_library(
  sim_msg STATIC
  ${MSG_DIR}/control.pb.cc
  ${MSG_DIR}/control_v2.pb.cc
  ${MSG_DIR}/basic.pb.cc
  ${MSG_DIR}/header.pb.cc
)
add_library(${ARBITRARY_PROJ_NAME} SHARED ${src_files})
target_link_libraries(${ARBITRARY_PROJ_NAME} sim_msg ${LIB_PROTOBUF} ${LIB_LOG} ${LIB_SYS})

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
