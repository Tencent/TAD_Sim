cmake_minimum_required(VERSION 3.0)

project(txsim-v2x)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(OUT_NAME "txsim_v2x")
set(OUT_NAME_MAIN "txsim_v2x_main")
set(OUT_NAME_TEST "txsim_v2x_test")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

include_directories(
  ${CMAKE_BINARY_DIR}
)

if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(GTest CONFIG REQUIRED)
  find_package(jsoncpp CONFIG REQUIRED)
  find_package(flann CONFIG REQUIRED)
  find_package(glog CONFIG REQUIRED)
  find_package(tinyxml2 CONFIG REQUIRED)

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
  set(LIB_XML tinyxml2::tinyxml2)
  set(LIB_JSON JsonCpp::JsonCpp)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  find_package(Eigen3 REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(Protobuf REQUIRED)
  find_package(tinyxml2 CONFIG REQUIRED)
  find_package(GTest CONFIG REQUIRED)

  add_definitions(-fPIC)
  include_directories(
    ${EIGEN3_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
  )
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(DEP_LIBS_PROTO protobuf)
  set(DEP_LIBS_LOG glog gflags)
  set(DEP_LIBS_SYS pthread dl)
  set(DEP_LIBS_FLANN flann)
  set(CMAKE_SKIP_RPATH TRUE)
  set(CMAKE_SKIP_BUILD_RPATH TRUE)
  set(CMAKE_SKIP_INSTALL_RPATH TRUE)
  set(LIB_GTEST GTest::gmock_main GTest::gmock GTest::gtest)
  set(LIB_XML tinyxml2::tinyxml2)
  set(LIB_JSON jsoncpp)
endif()

file(GLOB SRC
  "*.cpp"
  "sensor/*.cpp"
  "sim_algorithm/*.cpp"
  "utils/*.cpp"
  "asn1msg/*.cpp"
  "pblog/*.cpp"
)

file(GLOB TEST_SRC
  "tests/*.cpp"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/helper_funcs.cmake)
protobuf_generate_cpps(PROTO_SRC PROTO_HDR ../../../common/message/basic.proto
  ../../../common/message/header.proto
  ../../../common/message/location.proto
  ../../../common/message/traffic.proto
  ../../../common/message/trajectory.proto
  ../../../common/message/v2x_asn1_2020.proto
  ../../../common/message/scene.proto
  ../../../common/message/scene_common.proto
  ../../../common/message/waypoint.proto
  ../../../common/message/vehicle_geometry.proto
  ../../../common/message/scene_infrastructure_group.proto
  ../../../common/message/settings.proto
  ../../../common/message/environment.proto
  ../../../common/message/vehicle_interaction.proto
  ../../../common/message/scene_sensor_group.proto
  ../../../common/message/union.proto)

set(DEP_LIBS
  ${MAPSDK_LIBRARYS}
  ${LIB_BOOST_FS}
  ${DEP_LIBS_LOG}
  ${DEP_LIBS_PROTO}
  ${DEP_LIBS_SYS}
  ${DEP_LIBS_FLANN}
  ${LIB_XML}
  ${LIB_JSON}
)

add_library(${OUT_NAME} SHARED
  ${SRC}
  ${PROTO_SRC}
)
target_link_libraries(${OUT_NAME}
  ${DEP_LIBS}
)

add_executable(${OUT_NAME_MAIN}
  main.cc
  ${SRC}
  ${PROTO_SRC}
)

target_compile_definitions(${OUT_NAME_MAIN} PRIVATE TXSIM_NO_EXPORTS)
target_link_libraries(${OUT_NAME_MAIN}
  ${FRAMEWORK_LIBRARYS}
  ${DEP_LIBS}
)

add_executable(${OUT_NAME_TEST}
  ${SRC}
  ${TEST_SRC}
  ${PROTO_SRC}
)

target_compile_definitions(${OUT_NAME_TEST} PRIVATE TXSIM_NO_EXPORTS)
target_link_libraries(${OUT_NAME_TEST}
  ${LIB_GTEST}
  ${DEP_LIBS}
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
