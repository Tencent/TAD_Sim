cmake_minimum_required(VERSION 3.1)

project(txsim-examples)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 11)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../inc
  ${CMAKE_BINARY_DIR}
)

if(WIN32)
  set(LIB_SYS Shlwapi)
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)
  find_package(protobuf CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
  set(LIB_PROTOBUF protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite ZLIB::ZLIB)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../msgs)
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
elseif(UNIX)
  set(LIB_PROTOBUF protobuf pthread)
  set(LIB_SYS dl)
  include_directories(/usr/include ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs)
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
  find_package(Protobuf REQUIRED)
endif(WIN32)

#
include(${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/deps/helper_funcs.cmake)
protobuf_generate_cpp(PROTO_SRC_basic PROTO_HDR_basic ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/basic.proto)
protobuf_generate_cpp(PROTO_SRC_header PROTO_HDR_header ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/header.proto)
protobuf_generate_cpp(PROTO_SRC_traffic PROTO_HDR_traffic ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/traffic.proto)
protobuf_generate_cpp(PROTO_SRC_trajectory PROTO_HDR_trajectory ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/trajectory.proto)
protobuf_generate_cpp(PROTO_SRC_location PROTO_HDR_location ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/location.proto)
protobuf_generate_cpp(PROTO_SRC_waypoint PROTO_HDR_waypoint ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/waypoint.proto)
protobuf_generate_cpp(PROTO_SRC_settings PROTO_HDR_settings ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/settings.proto)
protobuf_generate_cpp(PROTO_SRC_environment PROTO_HDR_environment ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/environment.proto)
protobuf_generate_cpp(PROTO_SRC_vehicle_geometry PROTO_HDR_vehicle_geometry ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/vehicle_geometry.proto)
protobuf_generate_cpp(PROTO_SRC_vehicle_interaction PROTO_HDR_vehicle_interaction ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/vehicle_interaction.proto)
protobuf_generate_cpp(PROTO_SRC_scene_common PROTO_HDR_scene_common ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/scene_common.proto)
protobuf_generate_cpp(PROTO_SRC_scene_infrastructure_group PROTO_HDR_scene_infrastructure_group ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/scene_infrastructure_group.proto)
protobuf_generate_cpp(PROTO_SRC_scene_sensor_group PROTO_HDR_scene_sensor_group ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/scene_sensor_group.proto)
protobuf_generate_cpp(PROTO_SRC_scene PROTO_HDR_scene ${CMAKE_CURRENT_SOURCE_DIR}/../../msgs/scene.proto)

# example 1: built into a shared library.
add_library(my-module SHARED
  my_module.cc
  ${PROTO_SRC_basic}
  ${PROTO_SRC_header}
  ${PROTO_SRC_traffic}
  ${PROTO_SRC_trajectory}
  ${PROTO_SRC_location}
  ${PROTO_SRC_waypoint}
  ${PROTO_SRC_settings}
  ${PROTO_SRC_environment}
  ${PROTO_SRC_vehicle_geometry}
  ${PROTO_SRC_vehicle_interaction}
  ${PROTO_SRC_scene_common}
  ${PROTO_SRC_scene_infrastructure_group}
  ${PROTO_SRC_scene_sensor_group}
  ${PROTO_SRC_scene}
)
target_link_libraries(my-module
  ${LIB_PROTOBUF}
)

# example 2: built into a binary executable.
add_executable(my-module-main
  my_module_main.cc
  my_module.cc
  ${PROTO_SRC_basic}
  ${PROTO_SRC_header}
  ${PROTO_SRC_traffic}
  ${PROTO_SRC_trajectory}
  ${PROTO_SRC_location}
  ${PROTO_SRC_waypoint}
  ${PROTO_SRC_settings}
  ${PROTO_SRC_environment}
  ${PROTO_SRC_vehicle_geometry}
  ${PROTO_SRC_vehicle_interaction}
  ${PROTO_SRC_scene_common}
  ${PROTO_SRC_scene_infrastructure_group}
  ${PROTO_SRC_scene_sensor_group}
  ${PROTO_SRC_scene}
)
target_compile_definitions(my-module-main PRIVATE TXSIM_NO_EXPORTS)
target_link_libraries(my-module-main
  txsim-module-service ${LIB_PROTOBUF} ${LIB_SYS}
)
