cmake_minimum_required(VERSION 3.9)

project(txSimSensorTruth)
enable_testing()

# SET(CMAKE_BUILD_TYPE "release")
SET(CMAKE_SKIP_RPATH TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(OUT_NAME "txsim")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

include_directories(${CMAKE_BINARY_DIR})

if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(glog CONFIG REQUIRED)
  find_package(flann CONFIG REQUIRED)
  find_package(GTest CONFIG REQUIRED)

  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/eigen3
  )
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(DEP_LIBS_PROTO protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  set(DEP_LIBS_LOG glog::glog gflags::gflags)
  set(DEP_LIBS_SYS)
  set(DEP_LIBS_FLANN flann::flann flann::flann_cpp)
  set(LIB_GTEST GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  find_package(Eigen3 REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(Protobuf REQUIRED)
  find_package(GTest REQUIRED)
  add_definitions(-fPIC)
  include_directories(
    ${EIGEN3_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${FLANN_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIRS}
  )
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(DEP_LIBS_PROTO protobuf)
  set(DEP_LIBS_LOG glog gflags)
  set(DEP_LIBS_SYS pthread)
  set(DEP_LIBS_FLANN flann)
  set(CMAKE_SKIP_RPATH TRUE)
  set(CMAKE_SKIP_BUILD_RPATH TRUE)
  set(CMAKE_SKIP_INSTALL_RPATH TRUE)
  set(LIB_GTEST GTest::gmock_main GTest::gmock GTest::gtest)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/helper_funcs.cmake)
protobuf_generate_cpp(PROTO_SRC_BASE PROTO_HDR_BASE ../../../common/message/basic.proto)
protobuf_generate_cpp(PROTO_SRC_HEADER PROTO_HDR_HEADER ../../../common/message/header.proto)
protobuf_generate_cpp(PROTO_SRC_LOCATION PROTO_HDR_LOCATION ../../../common/message/location.proto)
protobuf_generate_cpp(PROTO_SRC_TRAFFIC PROTO_HDR_TRAFFIC ../../../common/message/traffic.proto)
protobuf_generate_cpp(PROTO_SRC_TRAJECTORY PROTO_HDR_TRAJECTORY ../../../common/message/trajectory.proto)
protobuf_generate_cpp(PROTO_SRC_FREESPACE PROTO_HDR_FREESPACE ../../../common/message/freespace.proto)
protobuf_generate_cpp(PROTO_SRC_DETECTEDLIAE PROTO_HDR_DETECTEDLANE ../../../common/message/detectedlane.proto)
protobuf_generate_cpp(PROTO_SRC_CAMERAOBJECT PROTO_HDR_CAMERAOBJECT ../../../common/message/cameraobject.proto)
protobuf_generate_cpp(PROTO_SRC_parking_space PROTO_HDR_parking_space ../../../common/message/parking_space.proto)
protobuf_generate_cpp(PROTO_SRC_traffic_filtered PROTO_HDR_traffic_filtered ../../../common/message/traffic_filtered.proto)
protobuf_generate_cpp(PROTO_SRC_osi_common PROTO_HDR_osi_common ../../../common/message/osi_common.proto)
protobuf_generate_cpp(PROTO_SRC_osi_datarecording PROTO_HDR_osi_datarecording ../../../common/message/osi_datarecording.proto)
protobuf_generate_cpp(PROTO_SRC_osi_detectedlane PROTO_HDR_osi_detectedlane ../../../common/message/osi_detectedlane.proto)
protobuf_generate_cpp(PROTO_SRC_osi_detectedobject PROTO_HDR_osi_detectedobject ../../../common/message/osi_detectedobject.proto)
protobuf_generate_cpp(PROTO_SRC_osi_detectedoccupant PROTO_HDR_osi_detectedoccupant ../../../common/message/osi_detectedoccupant.proto)
protobuf_generate_cpp(PROTO_SRC_osi_detectedroadmarking PROTO_HDR_osi_detectedroadmarking ../../../common/message/osi_detectedroadmarking.proto)
protobuf_generate_cpp(PROTO_SRC_osi_detectedtrafficlight PROTO_HDR_osi_detectedtrafficlight ../../../common/message/osi_detectedtrafficlight.proto)
protobuf_generate_cpp(PROTO_SRC_osi_detectedtrafficsign PROTO_HDR_osi_detectedtrafficsign ../../../common/message/osi_detectedtrafficsign.proto)
protobuf_generate_cpp(PROTO_SRC_osi_environment PROTO_HDR_osi_environment ../../../common/message/osi_environment.proto)
protobuf_generate_cpp(PROTO_SRC_osi_featuredata PROTO_HDR_osi_featuredata ../../../common/message/osi_featuredata.proto)
protobuf_generate_cpp(PROTO_SRC_osi_logicallane PROTO_HDR_osi_logicallane ../../../common/message/osi_logicallane.proto)
protobuf_generate_cpp(PROTO_SRC_osi_referenceline PROTO_HDR_osi_referenceline ../../../common/message/osi_referenceline.proto)
protobuf_generate_cpp(PROTO_SRC_osi_groundtruth PROTO_HDR_osi_groundtruth ../../../common/message/osi_groundtruth.proto)
protobuf_generate_cpp(PROTO_SRC_osi_hostvehicledata PROTO_HDR_osi_hostvehicledata ../../../common/message/osi_hostvehicledata.proto)
protobuf_generate_cpp(PROTO_SRC_osi_lane PROTO_HDR_osi_lane ../../../common/message/osi_lane.proto)
protobuf_generate_cpp(PROTO_SRC_osi_object PROTO_HDR_osi_object ../../../common/message/osi_object.proto)
protobuf_generate_cpp(PROTO_SRC_osi_occupant PROTO_HDR_osi_occupant ../../../common/message/osi_occupant.proto)
protobuf_generate_cpp(PROTO_SRC_osi_roadmarking PROTO_HDR_osi_roadmarking ../../../common/message/osi_roadmarking.proto)
protobuf_generate_cpp(PROTO_SRC_osi_logicaldetectiondata PROTO_HDR_osi_logicaldetectiondata ../../../common/message/osi_logicaldetectiondata.proto)
protobuf_generate_cpp(PROTO_SRC_osi_sensordata PROTO_HDR_osi_sensordata ../../../common/message/osi_sensordata.proto)
protobuf_generate_cpp(PROTO_SRC_osi_sensorspecific PROTO_HDR_osi_sensorspecific ../../../common/message/osi_sensorspecific.proto)
protobuf_generate_cpp(PROTO_SRC_osi_sensorview PROTO_HDR_osi_sensorview ../../../common/message/osi_sensorview.proto)
protobuf_generate_cpp(PROTO_SRC_osi_sensorviewconfiguration PROTO_HDR_osi_sensorviewconfiguration ../../../common/message/osi_sensorviewconfiguration.proto)
protobuf_generate_cpp(PROTO_SRC_osi_trafficlight PROTO_HDR_osi_trafficlight ../../../common/message/osi_trafficlight.proto)
protobuf_generate_cpp(PROTO_SRC_osi_trafficsign PROTO_HDR_osi_trafficsign ../../../common/message/osi_trafficsign.proto)
protobuf_generate_cpp(PROTO_SRC_osi_version PROTO_HDR_osi_version ../../../common/message/osi_version.proto)

protobuf_generate_cpp(PROTO_SRC_SCENE PROTO_HDR_SCENE ../../../common/message/scene.proto)
protobuf_generate_cpp(PROTO_SRC_SCENE_common PROTO_HDR_SCENE_common ../../../common/message/scene_common.proto)
protobuf_generate_cpp(PROTO_SRC_waypoint PROTO_HDR_waypoint ../../../common/message/waypoint.proto)
protobuf_generate_cpp(PROTO_SRC_vehicle_geometry PROTO_HDR_vehicle_geometry ../../../common/message/vehicle_geometry.proto)
protobuf_generate_cpp(PROTO_SRC_scene_infrastructure_group PROTO_HDR_scene_infrastructure_group ../../../common/message/scene_infrastructure_group.proto)
protobuf_generate_cpp(PROTO_SRC_settings PROTO_HDR_settings ../../../common/message/settings.proto)
protobuf_generate_cpp(PROTO_SRC_environment PROTO_HDR_environment ../../../common/message/environment.proto)
protobuf_generate_cpp(PROTO_SRC_vehicle_interaction PROTO_HDR_vehicle_interaction ../../../common/message/vehicle_interaction.proto)
protobuf_generate_cpp(PROTO_SRC_scene_sensor_group PROTO_HDR_scene_sensor_group ../../../common/message/scene_sensor_group.proto)
protobuf_generate_cpp(PROTO_SRC_union PROTO_HDR_union ../../../common/message/union.proto)

file(GLOB TEST_SRC
  "tests/*.cpp"
)
file(GLOB SRC
  "*.cpp"
)

add_library("${OUT_NAME}SensorTruth" SHARED
  age.cpp
  camera_sensor.cpp
  free_space.cpp
  hmap.cpp
  lane_truth.cpp
  fov_filter_sensor.cpp
  camera_truth.cpp
  sensor_truth.cpp
  truth.cpp
  truth_base.cpp
  ultrasonic_sensor.cpp
  osi_util.cpp
  object_size.cpp
  visable_calculate.cpp
  lidar_sensor.cpp
  lidar_truth.cpp
  ultrasonic_truth.cpp
  parking_space.cpp
  noise.cpp
  ${PROTO_SRC_BASE}
  ${PROTO_SRC_HEADER}
  ${PROTO_SRC_LOCATION}
  ${PROTO_SRC_TRAFFIC}
  ${PROTO_SRC_TRAJECTORY}
  ${PROTO_SRC_FREESPACE}
  ${PROTO_SRC_DETECTEDLIAE}
  ${PROTO_SRC_CAMERAOBJECT}
  ${PROTO_SRC_parking_space}
  ${PROTO_SRC_osi_common}
  ${PROTO_SRC_SCENE}
  ${PROTO_SRC_SCENE_common}
  ${PROTO_SRC_waypoint}
  ${PROTO_SRC_vehicle_geometry}
  ${PROTO_SRC_scene_infrastructure_group}
  ${PROTO_SRC_settings}
  ${PROTO_SRC_environment}
  ${PROTO_SRC_vehicle_interaction}
  ${PROTO_SRC_scene_sensor_group}
  ${PROTO_SRC_union}
  ${PROTO_SRC_osi_datarecording}
  ${PROTO_SRC_osi_detectedlane}
  ${PROTO_SRC_osi_detectedobject}
  ${PROTO_SRC_osi_detectedoccupant}
  ${PROTO_SRC_osi_detectedroadmarking}
  ${PROTO_SRC_osi_detectedtrafficlight}
  ${PROTO_SRC_osi_detectedtrafficsign}
  ${PROTO_SRC_osi_environment}
  ${PROTO_SRC_osi_featuredata}
  ${PROTO_SRC_osi_referenceline}
  ${PROTO_SRC_osi_logicallane}
  ${PROTO_SRC_osi_groundtruth}
  ${PROTO_SRC_osi_hostvehicledata}
  ${PROTO_SRC_osi_lane}
  ${PROTO_SRC_osi_object}
  ${PROTO_SRC_osi_occupant}
  ${PROTO_SRC_osi_roadmarking}
  ${PROTO_SRC_osi_logicaldetectiondata}
  ${PROTO_SRC_osi_sensordata}
  ${PROTO_SRC_osi_sensorspecific}
  ${PROTO_SRC_osi_sensorview}
  ${PROTO_SRC_osi_sensorviewconfiguration}
  ${PROTO_SRC_osi_trafficlight}
  ${PROTO_SRC_osi_trafficsign}
  ${PROTO_SRC_osi_version})

target_compile_definitions("${OUT_NAME}SensorTruth" PUBLIC -DUSE_SENSOR_TRUTH)
target_link_libraries("${OUT_NAME}SensorTruth"
  ${MAPSDK_LIBRARYS}
  ${LIB_BOOST_FS}
  ${DEP_LIBS_LOG}
  ${DEP_LIBS_PROTO}
  ${DEP_LIBS_SYS}
  ${DEP_LIBS_FLANN}
)

add_library("${OUT_NAME}TrafficFilter" SHARED
  age.cpp
  fov_filter_sensor.cpp
  hmap.cpp
  truth.cpp
  truth_base.cpp
  object_size.cpp
  traffic_filter.cpp
  ${PROTO_SRC_BASE}
  ${PROTO_SRC_HEADER}
  ${PROTO_SRC_LOCATION}
  ${PROTO_SRC_TRAFFIC}
  ${PROTO_SRC_TRAJECTORY}
  ${PROTO_SRC_FREESPACE}
  ${PROTO_SRC_DETECTEDLIAE}
  ${PROTO_SRC_CAMERAOBJECT}
  ${PROTO_SRC_SCENE}
  ${PROTO_SRC_SCENE_common}
  ${PROTO_SRC_waypoint}
  ${PROTO_SRC_vehicle_geometry}
  ${PROTO_SRC_scene_infrastructure_group}
  ${PROTO_SRC_settings}
  ${PROTO_SRC_environment}
  ${PROTO_SRC_vehicle_interaction}
  ${PROTO_SRC_scene_sensor_group}
  ${PROTO_SRC_union}
  ${PROTO_SRC_osi_common}
  ${PROTO_SRC_osi_datarecording}
  ${PROTO_SRC_osi_detectedlane}
  ${PROTO_SRC_osi_detectedobject}
  ${PROTO_SRC_osi_detectedoccupant}
  ${PROTO_SRC_osi_detectedroadmarking}
  ${PROTO_SRC_osi_detectedtrafficlight}
  ${PROTO_SRC_osi_detectedtrafficsign}
  ${PROTO_SRC_osi_environment}
  ${PROTO_SRC_osi_featuredata}
  ${PROTO_SRC_osi_logicallane}
  ${PROTO_SRC_osi_referenceline}
  ${PROTO_SRC_osi_groundtruth}
  ${PROTO_SRC_osi_hostvehicledata}
  ${PROTO_SRC_osi_lane}
  ${PROTO_SRC_osi_object}
  ${PROTO_SRC_osi_occupant}
  ${PROTO_SRC_osi_roadmarking}
  ${PROTO_SRC_osi_logicaldetectiondata}
  ${PROTO_SRC_osi_sensordata}
  ${PROTO_SRC_osi_sensorspecific}
  ${PROTO_SRC_osi_sensorview}
  ${PROTO_SRC_osi_sensorviewconfiguration}
  ${PROTO_SRC_osi_trafficlight}
  ${PROTO_SRC_osi_trafficsign}
  ${PROTO_SRC_osi_version}
  ${PROTO_SRC_traffic_filtered}
)

target_compile_definitions("${OUT_NAME}TrafficFilter" PUBLIC -DUSE_TRAFFIC_FILTER)
target_link_libraries("${OUT_NAME}TrafficFilter"
  ${MAPSDK_LIBRARYS}
  ${LIB_BOOST_FS}
  ${DEP_LIBS_LOG}
  ${DEP_LIBS_PROTO}
  ${DEP_LIBS_SYS}
  ${DEP_LIBS_FLANN}
)

include_directories(${CMAKE_SOURCE_DIR})

add_executable("${OUT_NAME}SensorTruth_test"
  ${SRC}
  ${TEST_SRC}
  ${PROTO_SRC_BASE}
  ${PROTO_SRC_HEADER}
  ${PROTO_SRC_LOCATION}
  ${PROTO_SRC_TRAFFIC}
  ${PROTO_SRC_TRAJECTORY}
  ${PROTO_SRC_FREESPACE}
  ${PROTO_SRC_DETECTEDLIAE}
  ${PROTO_SRC_CAMERAOBJECT}
  ${PROTO_SRC_parking_space}
  ${PROTO_SRC_traffic_filtered}
  ${PROTO_SRC_osi_common}
  ${PROTO_SRC_SCENE}
  ${PROTO_SRC_SCENE_common}
  ${PROTO_SRC_waypoint}
  ${PROTO_SRC_vehicle_geometry}
  ${PROTO_SRC_scene_infrastructure_group}
  ${PROTO_SRC_settings}
  ${PROTO_SRC_environment}
  ${PROTO_SRC_vehicle_interaction}
  ${PROTO_SRC_scene_sensor_group}
  ${PROTO_SRC_union}
  ${PROTO_SRC_osi_datarecording}
  ${PROTO_SRC_osi_detectedlane}
  ${PROTO_SRC_osi_detectedobject}
  ${PROTO_SRC_osi_detectedoccupant}
  ${PROTO_SRC_osi_detectedroadmarking}
  ${PROTO_SRC_osi_detectedtrafficlight}
  ${PROTO_SRC_osi_detectedtrafficsign}
  ${PROTO_SRC_osi_environment}
  ${PROTO_SRC_osi_featuredata}
  ${PROTO_SRC_osi_referenceline}
  ${PROTO_SRC_osi_logicallane}
  ${PROTO_SRC_osi_groundtruth}
  ${PROTO_SRC_osi_hostvehicledata}
  ${PROTO_SRC_osi_lane}
  ${PROTO_SRC_osi_object}
  ${PROTO_SRC_osi_occupant}
  ${PROTO_SRC_osi_roadmarking}
  ${PROTO_SRC_osi_logicaldetectiondata}
  ${PROTO_SRC_osi_sensordata}
  ${PROTO_SRC_osi_sensorspecific}
  ${PROTO_SRC_osi_sensorview}
  ${PROTO_SRC_osi_sensorviewconfiguration}
  ${PROTO_SRC_osi_trafficlight}
  ${PROTO_SRC_osi_trafficsign}
  ${PROTO_SRC_osi_version}
)

target_link_libraries("${OUT_NAME}SensorTruth_test"
  ${LIB_GTEST}
  ${MAPSDK_LIBRARYS}
  ${LIB_BOOST_FS}
  ${DEP_LIBS_LOG}
  ${DEP_LIBS_PROTO}
  ${DEP_LIBS_SYS}
  ${DEP_LIBS_FLANN}
)

# gtest_discover_tests("${OUT_NAME}SensorTruth_test")
if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
