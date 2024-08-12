cmake_minimum_required(VERSION 3.0)
project(routingmap)

set(datamodel_SOURCE_DIR "../datamodel")
set(routeplan_SOURCE_DIR "../route_plan")
set(engine_SOURCE_DIR "../map_engine")

if(WIN32)
  add_definitions(-fPIC -std=c++11)
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

  if(CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
    link_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib)
  else()
    link_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)
  endif()
else()
  add_definitions(-fPIC -std=c++11)

  if(UE)
    set(UE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../UE_H/4.27/Linux/LibCxx)
    include_directories(${UE_DIR}/include/c++/v1
      ${UE_DIR}/include)
    add_definitions(-nostdinc++)
    set(CPPLIB ${UE_DIR}/lib/Linux/x86_64-unknown-linux-gnu/libc++.a)
  endif(UE)
endif(WIN32)

add_definitions(-DTXSIMMAP_API)
add_definitions(-DTXSIMMAP_EXPORTS)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${routeplan_SOURCE_DIR}/include
  ${datamodel_SOURCE_DIR}/include
  ${engine_SOURCE_DIR}/include
)
link_directories(
  ${routeplan_SOURCE_DIR}
  ${datamodel_SOURCE_DIR}
  ${engine_SOURCE_DIR}
)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_library(${PROJECT_NAME} SHARED
  src/mapdata_load.cpp
  src/routing_map.cpp
  src/routing_request.cpp
  src/routing_utils.cpp
  src/txmap_proxy.cpp
  src/driving_track.cpp
  src/routing_log.cpp
)

target_link_libraries(${PROJECT_NAME}
  ${CPPLIB}
  datamodel
  routeplan
  mapengine
)
