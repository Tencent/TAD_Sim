cmake_minimum_required(VERSION 3.4.0)

project(imu_gps)

set(imu_root ${PROJECT_SOURCE_DIR} CACHE INTERNAL "imu_gps root dir")
set(imu_name imu_gps CACHE INTERNAL "imu/gps module name")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# build output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${imu_root}/inc)

add_definitions(-DCH_STATIC)

if(WIN32)
  # annoying windows define
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DNOMINMAX)

  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(pugixml CONFIG REQUIRED)
  find_package(protobuf CONFIG REQUIRED)
  find_package(jsoncpp CONFIG REQUIRED)
  find_package(gflags CONFIG REQUIRED)
  find_package(glog CONFIG REQUIRED)
  find_package(absl REQUIRED)

  # set include and link dir
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/)

  if(CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
    link_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib)
  else()
    link_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)
  endif()

  set(LIB_XML pugixml::shared pugixml::pugixml)
  set(LIB_PROTOBUF protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  set(LIB_LOG glog::glog gflags::gflags)
  set(LIB_BOOST ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(LIB_SYS shlwapi)
  set(LIB_JSON JsonCpp::JsonCpp)
  set(LIB_CHRONO ChronoEngine)
  set(LIB_ABSL absl::any absl::log absl::base absl::bits)
elseif(UNIX)
  # add define
  add_definitions("-D__unix__")
  add_compile_options(-fPIC)

  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(Protobuf REQUIRED)

  # set include and link dir
  include_directories(${BOOST_INCLUDE_DIR} /usr/include/jsoncpp /usr/include/eigen3)
  link_directories(/usr/local/lib /usr/lib64/boost148)

  set(LIB_XML pugixml)
  set(LIB_PROTOBUF protobuf)
  set(LIB_LOG glog gflags)
  set(LIB_BOOST ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(LIB_SYS pthread)
  set(LIB_JSON jsoncpp)
  set(LIB_CHRONO ChronoEngine)
endif(WIN32)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/helper_funcs.cmake)
protobuf_generate_cpps(PROTO_SRC PROTO_HDR ../../../common/message/basic.proto
  ../../../common/message/header.proto
  ../../../common/message/location.proto
  ../../../common/message/traffic.proto
  ../../../common/message/vehState.proto
  ../../../common/message/trajectory.proto
  ../../../common/message/scene.proto
  ../../../common/message/scene_common.proto
  ../../../common/message/waypoint.proto
  ../../../common/message/vehicle_geometry.proto
  ../../../common/message/scene_infrastructure_group.proto
  ../../../common/message/settings.proto
  ../../../common/message/environment.proto
  ../../../common/message/vehicle_interaction.proto
  ../../../common/message/scene_sensor_group.proto
  ../../../common/message/osi_version.proto
  ../../../common/message/osi_common.proto
  ../../../common/message/osi_gps.proto
  ../../../common/message/osi_imu.proto)

aux_source_directory(${imu_root}/src imu_src)

add_library(sim_msg STATIC ${PROTO_SRC})
target_link_libraries(sim_msg ${LIB_ABSL})
add_library(${imu_name} SHARED ${imu_src})
target_link_libraries(${imu_name} sim_msg ${LIB_CHRONO} ${LIB_JSON}
  ${LIB_PROTOBUF} ${LIB_SYS} ${LIB_LOG} ${LIB_BOOST} ${MAPSDK_LIBRARYS} ${LIB_XML} ${LIB_ABSL})

# imu test
option(OPTION_BUILD_LANE_PROJECTION "Build lane projection" OFF)

if(OPTION_BUILD_LANE_PROJECTION)
  if(UNIX)
    include_directories(${imu_root}/src/test)
    aux_source_directory(${imu_root}/src/test test_src)
    link_directories(/usr/lib/x86_64-linux-gnu)
    set(LIB_OPENCV opencv_imgcodecs opencv_core)

    add_executable(test_imu ${test_src})
    target_link_libraries(test_imu ${LIB_PROTOBUF} sim_msg ${LIB_CHRONO} ${LIB_SYS} ${LIB_LOG}
      ${LIB_BOOST} ${MAPSDK_LIBRARYS} ${LIB_XML} ${LIB_OPENCV})
  endif()
endif()

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
