cmake_minimum_required(VERSION 3.1)

project(txsim-envpb)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(framework REQUIRED)

include_directories(
  ${CMAKE_BINARY_DIR})

if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  set(LIB_PROTOBUF protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  set(LIB_SYS Shlwapi)
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  find_package(Protobuf REQUIRED)
  add_definitions(-fPIC)
  set(LIB_PROTOBUF protobuf pthread)
  set(LIB_SYS dl)
endif(WIN32)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/helper_funcs.cmake)
protobuf_generate_cpp(PROTO_SRC_BASE PROTO_HDR_BASE ../../common/message/basic.proto)
protobuf_generate_cpp(PROTO_SRC_HEADER PROTO_HDR_HEADER ../../common/message/header.proto)
protobuf_generate_cpp(PROTO_SRC_SCENE PROTO_HDR_SCENE ../../common/message/scene.proto)
protobuf_generate_cpp(PROTO_SRC_SCENE_common PROTO_HDR_SCENE_common ../../common/message/scene_common.proto)
protobuf_generate_cpp(PROTO_SRC_settings PROTO_HDR_settings ../../common/message/settings.proto)
protobuf_generate_cpp(PROTO_SRC_waypoint PROTO_HDR_waypoint ../../common/message/waypoint.proto)
protobuf_generate_cpp(PROTO_SRC_vehicle_geometry PROTO_HDR_vehicle_geometry ../../common/message/vehicle_geometry.proto)
protobuf_generate_cpp(PROTO_SRC_scene_infrastructure_group PROTO_HDR_scene_infrastructure_group ../../common/message/scene_infrastructure_group.proto)
protobuf_generate_cpp(PROTO_SRC_environment PROTO_HDR_environment ../../common/message/environment.proto)
protobuf_generate_cpp(PROTO_SRC_vehicle_interaction PROTO_HDR_vehicle_interaction ../../common/message/vehicle_interaction.proto)
protobuf_generate_cpp(PROTO_SRC_scene_sensor_group PROTO_HDR_scene_sensor_group ../../common/message/scene_sensor_group.proto)

file(GLOB SRC
  "*.cpp"
)

# example 1: built into a shared library.
add_library(envpb SHARED
  ${SRC}
  ${PROTO_SRC_BASE}
  ${PROTO_SRC_HEADER}
  ${PROTO_SRC_SCENE}
  ${PROTO_SRC_settings}
  ${PROTO_SRC_SCENE_common}
  ${PROTO_SRC_waypoint}
  ${PROTO_SRC_vehicle_geometry}
  ${PROTO_SRC_scene_infrastructure_group}
  ${PROTO_SRC_environment}
  ${PROTO_SRC_vehicle_interaction}
  ${PROTO_SRC_scene_sensor_group})
target_link_libraries(envpb
  ${LIB_PROTOBUF})

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
