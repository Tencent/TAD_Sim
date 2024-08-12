cmake_minimum_required(VERSION 3.0)

project(txsim-radar)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(OUT_NAME "txsim_radar")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

include_directories(
  ${CMAKE_BINARY_DIR}
)

if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(glog CONFIG REQUIRED)
  find_package(flann CONFIG REQUIRED)
  find_package(GTest CONFIG REQUIRED)
  find_package(FFTW3 CONFIG REQUIRED)

  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/eigen3
  )
  set(DEV_RADAR FFTW3::fftw3)
  set(DEP_LIBS_PROTO protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(DEP_LIBS_LOG glog::glog gflags::gflags)
  set(DEP_LIBS_SYS)
  set(DEP_LIBS_FLANN flann::flann flann::flann_cpp)
  set(LIB_GTEST GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  find_package(Eigen3 REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(Protobuf REQUIRED)
  find_package(GTest CONFIG REQUIRED)
  add_definitions(-fPIC)
  include_directories(
    ${EIGEN3_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
  )
  set(DEV_RADAR fftw3)
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(DEP_LIBS_PROTO protobuf)
  set(DEP_LIBS_LOG glog gflags)
  set(DEP_LIBS_FLANN flann)
  set(CMAKE_SKIP_RPATH TRUE)
  set(CMAKE_SKIP_BUILD_RPATH TRUE)
  set(CMAKE_SKIP_INSTALL_RPATH TRUE)
  set(LIB_GTEST GTest::gmock_main GTest::gmock GTest::gtest)
endif()

file(GLOB TEST_SRC
  "tests/*.cpp"
)
file(GLOB SRC
  "*.cpp"
)
file(GLOB DEV_SRC
  "dev/*.cpp"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/helper_funcs.cmake)
protobuf_generate_cpps(PROTO_SRC PROTO_HDR
  ../../../common/message/basic.proto
  ../../../common/message/header.proto
  ../../../common/message/location.proto
  ../../../common/message/traffic.proto
  ../../../common/message/trajectory.proto
  ../../../common/message/osi_common.proto
  ../../../common/message/osi_version.proto
  ../../../common/message/osi_featuredata.proto
  ../../../common/message/scene.proto
  ../../../common/message/scene_common.proto
  ../../../common/message/waypoint.proto
  ../../../common/message/vehicle_geometry.proto
  ../../../common/message/scene_infrastructure_group.proto
  ../../../common/message/settings.proto
  ../../../common/message/environment.proto
  ../../../common/message/vehicle_interaction.proto
  ../../../common/message/scene_sensor_group.proto
  ../../../common/message/union.proto
)

add_library(${OUT_NAME} SHARED
  ${SRC}
  ${DEV_SRC}
  ${PROTO_SRC}
)
target_include_directories(${OUT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/dev
)
target_link_libraries(${OUT_NAME}
  ${MAPSDK_LIBRARYS}
  ${DEV_RADAR}
  ${LIB_BOOST_FS}
  ${DEP_LIBS_LOG}
  ${DEP_LIBS_PROTO}
  ${DEP_LIBS_SYS}
  ${DEP_LIBS_FLANN}
)

add_executable(${OUT_NAME}_test
  ${SRC}
  ${DEV_SRC}
  ${TEST_SRC}
  ${PROTO_SRC}
)
target_include_directories(${OUT_NAME}_test PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/dev
)
target_link_libraries(${OUT_NAME}_test
  ${MAPSDK_LIBRARYS}
  ${DEV_RADAR}
  ${LIB_BOOST_FS}
  ${DEP_LIBS_LOG}
  ${DEP_LIBS_PROTO}
  ${DEP_LIBS_SYS}
  ${DEP_LIBS_FLANN}
  ${LIB_GTEST}
)

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
