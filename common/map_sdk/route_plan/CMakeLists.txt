cmake_minimum_required(VERSION 3.0)
project(routeplan)

set(mapdb_SOURCE_DIR "../mapdb")
set(datamodel_SOURCE_DIR "../datamodel")
set(transmission_SOURCE_DIR "../transmission")
set(engine_SOURCE_DIR "../map_engine")

if(WIN32)
  add_definitions(-fPIC -std=c++11)
  find_package(CURL CONFIG REQUIRED)
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
  ${mapdb_SOURCE_DIR}/include
  ${datamodel_SOURCE_DIR}/include
  ${transmission_SOURCE_DIR}/include
  ${engine_SOURCE_DIR}/include
)
link_directories(
  ${mapdb_SOURCE_DIR}
  ${datamodel_SOURCE_DIR}
  ${transmission_SOURCE_DIR}
  ${engine_SOURCE_DIR}
)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_library(${PROJECT_NAME} SHARED
  src/a_star.cpp
  src/node_opt.cpp
  src/route_generation.cpp
  src/route_plan.cpp
  src/route_utils.cpp
  src/refine_traj.cpp
)

if(WIN32)
  target_link_libraries(${PROJECT_NAME}
    datamodel
    transmission
    mapengine
    mapdb
    CURL::libcurl
  )

else()
  target_link_libraries(${PROJECT_NAME}
    ${CPPLIB}
    datamodel
    transmission
    mapengine
    mapdb
    curl
  )
endif(WIN32)
