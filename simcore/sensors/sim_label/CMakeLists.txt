cmake_minimum_required(VERSION 3.9)

project(txsim-sim-label)
SET(CMAKE_SKIP_RPATH TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

include_directories(
  ${CMAKE_BINARY_DIR})

if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(GTest CONFIG REQUIRED)
  find_package(jsoncpp CONFIG REQUIRED)

  set(LIB_PROTOBUF protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  set(LIB_SYS Shlwapi)
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(LIB_JSON JsonCpp::JsonCpp)
  set(LIB_GTEST GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/eigen3
  )
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  find_package(Eigen3 REQUIRED)
  find_package(Protobuf REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(GTest REQUIRED)
  add_definitions(-fPIC)
  include_directories(
    ${BOOST_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIRS}
  )
  set(LIB_PROTOBUF protobuf pthread)
  set(LIB_SYS dl)
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(LIB_JSON jsoncpp)
  set(LIB_GTEST GTest::gmock_main GTest::gmock GTest::gtest)
endif(WIN32)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/helper_funcs.cmake)
protobuf_generate_cpps(PROTO_SRC PROTO_HDR
  ../../../common/message/sensor_raw.proto
  ../../../common/message/osi_common.proto
  ../../../common/message/osi_detectedlane.proto
  ../../../common/message/osi_detectedobject.proto
  ../../../common/message/osi_detectedoccupant.proto
  ../../../common/message/osi_detectedroadmarking.proto
  ../../../common/message/osi_detectedtrafficlight.proto
  ../../../common/message/osi_detectedtrafficsign.proto
  ../../../common/message/osi_sensorview.proto
  ../../../common/message/osi_featuredata.proto
  ../../../common/message/osi_logicaldetectiondata.proto
  ../../../common/message/osi_sensordata.proto
  ../../../common/message/osi_environment.proto
  ../../../common/message/osi_logicallane.proto
  ../../../common/message/osi_referenceline.proto
  ../../../common/message/osi_groundtruth.proto
  ../../../common/message/osi_hostvehicledata.proto
  ../../../common/message/osi_lane.proto
  ../../../common/message/osi_object.proto
  ../../../common/message/osi_occupant.proto
  ../../../common/message/osi_roadmarking.proto
  ../../../common/message/osi_sensorspecific.proto
  ../../../common/message/osi_sensorviewconfiguration.proto
  ../../../common/message/osi_trafficlight.proto
  ../../../common/message/osi_trafficsign.proto
  ../../../common/message/osi_version.proto
  ../../../common/message/osi_datarecording.proto
  ../../../common/message/basic.proto
  ../../../common/message/header.proto
  ../../../common/message/scene.proto
  ../../../common/message/scene_common.proto
  ../../../common/message/waypoint.proto
  ../../../common/message/vehicle_geometry.proto
  ../../../common/message/scene_infrastructure_group.proto
  ../../../common/message/settings.proto
  ../../../common/message/environment.proto
  ../../../common/message/vehicle_interaction.proto
  ../../../common/message/scene_sensor_group.proto)

file(GLOB SRC
  "*.h"
  "*.cpp"
)

add_library(sim_label SHARED
  ${SRC}
  ${PROTO_SRC})
target_link_libraries(sim_label
  ${LIB_PROTOBUF}
  ${MAPSDK_LIBRARYS}
  ${LIB_BOOST_FS}
  ${LIB_JSON})

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
