cmake_minimum_required(VERSION 3.4.0)

project(txSimOSIAdapter)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules)
set(root_dir ${CMAKE_SOURCE_DIR} CACHE INTERNAL "osi adapter")
set(meassage meassage CACHE INTERNAL "meassage")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../simcore/cmake")
find_package(framework REQUIRED)

set(inc_dir
  ${root_dir}/src
  ${root_dir}/../../common/message/build
  CACHE INTERNAL "include dir")

include_directories(${inc_dir})

if(WIN32)
  add_compile_options("/MP")
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)

  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
  )

  find_package(protobuf CONFIG REQUIRED)
  set(lib_protobuf protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  find_package(glog CONFIG REQUIRED)
  set(lib_glog glog::glog gflags::gflags)
  set(lib_sys shlwapi)
else()
  add_compile_options(-Wunused)
  add_compile_options(-Wreturn-type)
  add_compile_options(-fPIC)
  set(lib_protobuf protobuf)
  set(lib_sys pthread)
  set(lib_glog glog gflags)
endif(WIN32)

aux_source_directory(${root_dir}/src src_files)
file(GLOB osi_srcs ${root_dir}/../../common/message/build/osi_*.pb.cc)
add_library(${meassage} STATIC
  ${root_dir}/../../common/message/build/detectedlane.pb.cc
  ${root_dir}/../../common/message/build/environment.pb.cc
  ${root_dir}/../../common/message/build/location.pb.cc
  ${root_dir}/../../common/message/build/trajectory.pb.cc
  ${root_dir}/../../common/message/build/basic.pb.cc
  ${root_dir}/../../common/message/build/header.pb.cc
  ${root_dir}/../../common/message/build/traffic.pb.cc
  ${osi_srcs})

# build elf
add_library(txSimOSIAdapter SHARED ${src_files})

if(WIN32)
  target_link_libraries(txSimOSIAdapter ${meassage} ${FRAMEWORK_LIBRARYS} ${lib_sys} ${lib_hadmap} ${lib_protobuf} ${lib_glog})
else()
  target_link_libraries(txSimOSIAdapter -Wl,--whole-archive ${meassage} -Wl,--no-whole-archive
    ${FRAMEWORK_LIBRARYS} ${lib_sys} ${lib_hadmap} ${lib_protobuf} ${lib_glog})
endif(WIN32)

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
