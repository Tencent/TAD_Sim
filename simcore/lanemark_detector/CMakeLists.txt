cmake_minimum_required(VERSION 3.0)

project(egoLaneMarks)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(framework REQUIRED)
find_package(map_sdk REQUIRED)

include_directories(
  ${CMAKE_BINARY_DIR})

if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x601 -D_USE_MATH_DEFINES)
  add_definitions(-DPROJ_NEW_API)
  add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)

  link_directories(
    ../../common/map_sdk/hadmap/lib)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(Protobuf CONFIG REQUIRED)
  find_package(glog CONFIG REQUIRED)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(CURL CONFIG REQUIRED)
  find_package(PROJ CONFIG REQUIRED)

  set(LIB_MAPSDK unofficial::sqlite3::sqlite3 CURL::libcurl PROJ::proj)
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(DEP_LIBS_PROTO protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  set(DEP_LIBS_LOG glog::glog gflags::gflags shlwapi)
  set(DEP_LIBS_SYS)
else()
  add_definitions(-fPIC)
  find_package(Boost REQUIRED COMPONENTS system filesystem)

  link_directories(
    ../../common/map_sdk/hadmap/devel/bin)
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(LIB_MAPSDK sqlite3 curl)
  set(DEP_LIBS_PROTO protobuf)
  set(DEP_LIBS_LOG glog gflags)
  set(DEP_LIBS_SYS pthread dl)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/helper_funcs.cmake)
protobuf_generate_cpp(PROTO_BSC_SRC PROTO_BSC_HDR ../../common/message/basic.proto)
protobuf_generate_cpp(PROTO_HDR_SRC PROTO_HDR_HDR ../../common/message/header.proto)
protobuf_generate_cpp(PROTO_LOC_SRC PROTO_LOC_HDR ../../common/message/location.proto)
protobuf_generate_cpp(PROTO_TRJ_SRC PROTO_TRJ_HDR ../../common/message/trajectory.proto)
protobuf_generate_cpp(PROTO_TFC_TFC PROTO_TFC_HDR ../../common/message/traffic.proto)
protobuf_generate_cpp(PROTO_LMK_TFC PROTO_LMK_HDR ../../common/message/laneMarks.proto)

# protobuf_generate_cpp(PROTO_OSI_COMMON_SRC PROTO_OSI_COMMON_HDR ../sim_msg/open-simulation-interface/osi_common.proto)
# protobuf_generate_cpp(PROTO_OSI_LANE_SRC PROTO_OIS_LANE_HDR ../sim_msg/open-simulation-interface/osi_lane.proto)
set(PROTO_SRC ${PROTO_BSC_SRC} ${PROTO_HDR_SRC} ${PROTO_LOC_SRC} ${PROTO_TRJ_SRC} ${PROTO_TFC_TFC} ${PROTO_LMK_TFC})
set(PROTO_HDR ${PROTO_BSC_HDR} ${PROTO_HDR_HDR} ${PROTO_LOC_HDR} ${PROTO_TRJ_HDR} ${PROTO_TFC_HDR} ${PROTO_LMK_HDR})

add_library(txSimLaneMarkDetector SHARED
  egolanemarker.h
  egolanemarker.cpp
  ${PROTO_HDR}
  ${PROTO_SRC})

target_link_libraries(txSimLaneMarkDetector
  ${LIB_MAPSDK}
  ${DEP_LIBS_LOG}
  ${DEP_LIBS_PROTO}
  ${LIB_BOOST_FS}
  ${DEP_LIBS_SYS}
  ${MAPSDK_LIBRARYS}
  ${FRAMEWORK_LIBRARYS})

add_executable(txSimLaneMarkDetectorMain
  main.cpp
  egolanemarker.h
  egolanemarker.cpp
  ${PROTO_HDR}
  ${PROTO_SRC})
target_compile_definitions(txSimLaneMarkDetectorMain PRIVATE TXSIM_NO_EXPORTS)

target_link_libraries(txSimLaneMarkDetectorMain
  txSimLaneMarkDetector
  ${FRAMEWORK_LIBRARYS})

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
