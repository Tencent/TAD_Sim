cmake_minimum_required(VERSION 3.1)

project(txsim-examples)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)

include_directories(
  ../inc
  ${CMAKE_BINARY_DIR})

if(WIN32)
  set(LIB_PROTOBUF libprotobuf zlibstatic)
  set(LIB_SYS Shlwapi)
  include_directories(../deps ../msgs)
  link_directories(../deps/lib/Debug ../lib)
elseif(UNIX)
  set(LIB_PROTOBUF protobuf pthread)
  set(LIB_SYS dl)
  include_directories(/usr/include ../msgs)
  link_directories(../lib)
  find_package(Protobuf REQUIRED)
endif(WIN32)

# example 1: built into a shared library.
add_library(my-module SHARED
  my_module.cc
  ../msgs/header.pb.cc
  ../msgs/traffic.pb.cc
  ../msgs/trajectory.pb.cc
  ../msgs/location.pb.cc
  ../msgs/basic.pb.cc
  ../msgs/waypoint.pb.cc
  ../msgs/settings.pb.cc
  ../msgs/environment.pb.cc
  ../msgs/vehicle_geometry.pb.cc
  ../msgs/scene_common.pb.cc
  ../msgs/scene_infrastructure_group.pb.cc
  ../msgs/vehicle_interaction.pb.cc
  ../msgs/scene_sensor_group.pb.cc
  ../msgs/scene.pb.cc
)
target_link_libraries(my-module
  ${LIB_PROTOBUF})

# example 2: built into a binary executable.
add_executable(my-module-main
  my_module_main.cc
  my_module.cc
  ../msgs/header.pb.cc
  ../msgs/traffic.pb.cc
  ../msgs/trajectory.pb.cc
  ../msgs/location.pb.cc
  ../msgs/basic.pb.cc
  ../msgs/waypoint.pb.cc
  ../msgs/settings.pb.cc
  ../msgs/environment.pb.cc
  ../msgs/vehicle_geometry.pb.cc
  ../msgs/scene_common.pb.cc
  ../msgs/scene_infrastructure_group.pb.cc
  ../msgs/vehicle_interaction.pb.cc
  ../msgs/scene_sensor_group.pb.cc
  ../msgs/scene.pb.cc
)
target_compile_definitions(my-module-main PRIVATE TXSIM_NO_EXPORTS)
target_link_libraries(my-module-main
  txsim-module-service ${LIB_PROTOBUF} ${LIB_SYS})
