cmake_minimum_required(VERSION 3.13.0)

project(txSim)
set(LoggerLibName txSimLogger)
set(LogplayerLibName txSimLogplayer)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
set(outputFolder ../protobuf_log)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(framework REQUIRED)

include_directories(
  /usr/local/include
  ../../common/message/build
  ../../common/message
  ${CMAKE_BINARY_DIR})

if(MSVC)
  add_definitions(-DZMQ_STATIC -D_WIN32_WINNT=0x601)
endif()

if(WIN32)
  add_compile_options(/utf-8)
  add_definitions(-DNOMINMAX)
  add_definitions(-DGLOG_USE_GLOG_EXPORT)
  find_package(absl CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS filesystem regex)
  find_package(protobuf CONFIG REQUIRED)
  find_package(gflags CONFIG REQUIRED)
  find_package(glog CONFIG REQUIRED)
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)
  link_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)
  set(LIB_SYS Shlwapi)
  set(LIB_PROTOBUF protobuf::libprotobuf)
  set(LIB_LOG glog::glog gflags::gflags)
  set(LIB_ABSL absl::base)
  set(LIB_BOOST Boost::filesystem)
else()
  set(LIB_PROTOBUF protobuf)
  set(LIB_SYS dl pthread jsoncpp)
  set(LIB_LOG glog gflags)
  set(LIB_BOOST boost_filesystem)
  link_directories(/usr/local/lib)
endif()

if(NOT WIN32)
  add_definitions("-fPIC")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/helper_funcs.cmake)
protobuf_generate_cpp(PROTO_SRC_BASIC PROTO_HDR_BASIC ../../common/message/basic.proto)
protobuf_generate_cpp(PROTO_SRC_HEADER PROTO_HDR_HEADER ../../common/message/header.proto)
protobuf_generate_cpp(PROTO_SRC_UNION PROTO_HDR_UNION ../../common/message/union.proto)
protobuf_generate_cpp(PROTO_SRC_SETTINGS PROTO_HDR_SETTINGS ../../common/message/settings.proto)
protobuf_generate_cpp(PROTO_SRC_SCENE PROTO_HDR_SCENE ../../common/message/scene.proto)
protobuf_generate_cpp(PROTO_SRC_WAYPOINT PROTO_HDR_WAYPOINT ../../common/message/waypoint.proto)
protobuf_generate_cpp(PROTO_SRC_ENVIRONMENT PROTO_HDR_ENVIRONMENT ../../common/message/environment.proto)
protobuf_generate_cpp(PROTO_SRC_VEHICLE_INTERACTION PROTO_HDR_VEHICLE_INTERACTION ../../common/message/vehicle_interaction.proto)
protobuf_generate_cpp(PROTO_SRC_VEHICLE_GEOMETRY PROTO_HDR_VEHICLE_GEOMETRY ../../common/message/vehicle_geometry.proto)
protobuf_generate_cpp(PROTO_SRC_SCENE_SENSOR_GROUP PROTO_HDR_SCENE_SENSOR_GROUP ../../common/message/scene_sensor_group.proto)
protobuf_generate_cpp(PROTO_SRC_SCENE_INFRASTRUCTURE_GROUP PROTO_HDR_SCENE_INFRASTRUCTURE_GROUP ../../common/message/scene_infrastructure_group.proto)
protobuf_generate_cpp(PROTO_SRC_SCENE_COMMON PROTO_HDR_SCENE_COMMON ../../common/message/scene_common.proto)

add_library(${LoggerLibName} SHARED
  ./src/txsim_logger_wrapper.cpp
  ./src/txsim_logger.cpp
  ./src/txsim_log_common.cpp
  ./src/txsim_log_queue.cpp
  ${PROTO_SRC_UNION}
  ${PROTO_SRC_SETTINGS}
)

if(WIN32)
  target_link_libraries(${LoggerLibName} ${LIB_PROTOBUF} ${LIB_LOG} ${LIB_SYS} ${LIB_BOOST} ${LIB_ABSL})
else()
  target_link_libraries(${LoggerLibName} ${LIB_PROTOBUF} ${LIB_LOG} ${LIB_SYS} ${LIB_BOOST})
endif()

add_library(${LogplayerLibName} SHARED
  ./src/txsim_log_player.cpp
  ./src/txsim_log_common.cpp
  ./src/txsim_log_queue.cpp
  ${PROTO_SRC_SCENE}
  ${PROTO_SRC_BASIC}
  ${PROTO_SRC_SETTINGS}
  ${PROTO_SRC_WAYPOINT}
  ${PROTO_SRC_ENVIRONMENT}
  ${PROTO_SRC_VEHICLE_INTERACTION}
  ${PROTO_SRC_VEHICLE_GEOMETRY}
  ${PROTO_SRC_SCENE_SENSOR_GROUP}
  ${PROTO_SRC_SCENE_INFRASTRUCTURE_GROUP}
  ${PROTO_SRC_SCENE_COMMON}
  ${PROTO_SRC_HEADER}
)

if(WIN32)
  target_link_libraries(${LogplayerLibName} ${LIB_PROTOBUF} ${LIB_LOG} ${LIB_SYS} ${LIB_BOOST} ${LIB_ABSL})
else()
  target_link_libraries(${LogplayerLibName} ${LIB_PROTOBUF} ${LIB_LOG} ${LIB_SYS} ${LIB_BOOST})
endif()

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
