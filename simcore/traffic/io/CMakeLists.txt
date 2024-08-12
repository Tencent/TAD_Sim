cmake_minimum_required(VERSION 3.1...3.20)

project(txTraffic LANGUAGES CXX)

# set(LIBRARY_OUTPUT_PATH ../${CMAKE_BINARY_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(map_sdk REQUIRED)

if(WIN32)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(glog CONFIG REQUIRED)
  find_package(TBB CONFIG REQUIRED)
else()
  # find cppzmq wrapper, installed by make of cppzmq
  set(CMAKE_SKIP_RPATH TRUE)
  find_package(Eigen3)
  find_package(TBB)
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/protobuf
  "./include"
  "../common/include"
  "../3rd"
  "../3rd/xml"
)

if(WIN32)
else()
  include_directories(
    ${EIGEN3_INCLUDE_DIR}
  )
endif()

SET(project_SOURCES
  "./src/tad_sim.cpp"
  "./src/tad_traffic.cpp"
  "./src/tad_cloud.cpp"

  # "./src/tad_xosc.cpp"
  "./src/tad_catalog.cpp"

  # "./src/replay_xosc.cpp"
)

SET(project_HEADERS
  "./include/tx_xml_utils.h"
  "./include/tad_sim.h"
  "./include/tad_traffic.h"
  "./include/tad_cloud.h"

  # "./include/tad_xosc.h"
  "./include/tad_catalog.h"

  # "./include/replay_xosc.h"
)

add_library(txSceneLoader ${project_SOURCES} ${project_HEADERS})

target_compile_definitions(txSceneLoader PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

if(WIN32)
else()
  target_compile_options(txSceneLoader PRIVATE -fPIC)
endif()

if(WIN32 OR WithDependency)
  message(STATUS "txSceneLoader add_dependencies")
  add_dependencies(txSceneLoader txProtoBuf)
endif()

if(WIN32)
  target_link_libraries(txSceneLoader
    ${MAPSDK_LIBRARYS}
    glog::glog
    gflags::gflags
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    TBB::tbb
  )
else()
  target_link_libraries(txSceneLoader
    ${MAPSDK_LIBRARYS}
    glog
    gflags
    boost_filesystem
    tbb
  )
endif()
