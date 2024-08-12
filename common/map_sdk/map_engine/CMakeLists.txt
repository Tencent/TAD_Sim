cmake_minimum_required(VERSION 3.0)
project(mapengine)

set(mapdb_SOURCE_DIR "../mapdb")
set(datamodel_SOURCE_DIR "../datamodel")
set(transmission_SOURCE_DIR "../transmission")
set(import_SOURCE_DIR "../map_import")

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
  ${mapdb_SOURCE_DIR}/include
  ${datamodel_SOURCE_DIR}/include
  ${transmission_SOURCE_DIR}/include
  ${import_SOURCE_DIR}/include
)
link_directories(
  ${mapdb_SOURCE_DIR}
  ${datamodel_SOURCE_DIR}
  ${transmission_SOURCE_DIR}
  ${import_SOURCE_DIR}
)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_library(${PROJECT_NAME} SHARED
  src/local_search.cpp
  src/remote_search.cpp
  src/mem_search.cpp
  src/mem_search_ex.cpp
  src/search_factory.cpp
  src/hadmap_engine.cpp
)

target_link_libraries(${PROJECT_NAME}
  ${CPPLIB}
  datamodel
  transmission
  mapimport
  mapdb
)
