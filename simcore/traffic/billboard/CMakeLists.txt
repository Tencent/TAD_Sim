cmake_minimum_required(VERSION 3.1...3.20)

project(txBillboard LANGUAGES CXX)

set(EXECUTABLE_OUTPUT_PATH ../${CMAKE_BINARY_DIR}/lib/)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake/)
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/protobuf
  "./framework"
  "."
  "../common/include"
  "../framework/include"
  "../io/include"
  "../3rd")

if(WIN32)
else()
  # find cppzmq wrapper, installed by make of cppzmq
  set(CMAKE_SKIP_RPATH TRUE)
  find_package(Eigen3)
  find_package(TBB)
  find_package(Protobuf REQUIRED)
endif()

if(WIN32)
  include_directories(
    ../../deps
    ../../deps/eigen3
    ../../deps/tbb-2020.2-win/tbb/include)
else()
  include_directories(
    ${EIGEN3_INCLUDE_DIR}
    ${TBB_INCLUDE_DIRS})
  add_definitions(${TBB_DEFINITIONS})
endif()

if(WIN32)
  link_directories(
    ${CMAKE_BINARY_DIR}/lib)
else()
  link_directories(
    ${CMAKE_BINARY_DIR}/lib)
endif()

SET(project_SOURCES
  "./tx_billboard.cpp"
  "./tx_billboard_application.cpp"
  "./tx_billboard_flags.cpp"
  "./tx_billboard_loop.cpp"
  "./tx_billboard_template.cpp"
  "./tx_billboard_ego.cpp"
  "./tx_billboard_hwy_entry.cpp"
  "./tx_billboard_charging_pile.cpp"
  "./tx_billboard_utility.cpp"
  "./tx_billboard_lane_info_aver_speed.cpp"
)

SET(project_HEADERS
  "./tx_billboard_application.h"
  "./tx_billboard_flags.h"
  "./tx_billboard_loop.h"
  "./tx_billboard_template.h"
  "./tx_billboard_ego.h"
  "./tx_billboard_hwy_entry.h"
  "./tx_billboard_charging_pile.h"
  "./tx_billboard_utility.h"
  "./tx_billboard_lane_info_aver_speed.h"
)

add_executable(tx_billboard ${project_SOURCES} ${project_HEADERS})

target_compile_definitions(tx_billboard PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

if(WIN32)
else()
  target_compile_options(tx_billboard PRIVATE -fPIC)
endif()

if(WIN32 OR WithDependency)
  message(STATUS "filter add_dependencies")
  add_dependencies(tx_billboard TrafficFramework)
endif()

if(WIN32)
  target_link_libraries(tx_billboard
    txProtoBuf.lib
    txSceneLoader.lib
    TrafficFramework.lib
    ${MAPSDK_LIBRARYS}
    ${FRAMEWORK_LIBRARY_SERVICE}
    tbb.lib
    libboost_filesystem.lib
    glog.lib
    gflags.lib
    zlibstatic.lib
    route_plan.lib
    curl.lib
    proj_i.lib
    shlwapi
    Advapi32.lib
    Rpcrt4.lib
    Ws2_32.lib
    Iphlpapi.lib
    libprotobuf.lib
    spatialite_i.lib
    sqlite3_i.lib
  )
else()
  target_link_libraries(tx_billboard
    txProtoBuf
    TrafficFramework
    txSceneLoader
    ${MAPSDK_LIBRARYS}
    ${FRAMEWORK_LIBRARY_SERVICE}
    tbb
    protobuf
    spatialite
    sqlite3
    curl
    proj
    glog
    gflags
    boost_regex
    boost_filesystem
    boost_system
    boost_thread
    dl
    pthread
    rt
  )
endif()

# if(WIN32)
# if(NOT CMAKE_BUILD_TYPE)
# copy_mapsdk_deps(${EXECUTABLE_OUTPUT_PATH}/Release)
# copy_framework_deps(${EXECUTABLE_OUTPUT_PATH}/Release)
# else()
# copy_mapsdk_deps(${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
# copy_framework_deps(${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
# endif()
# endif()
