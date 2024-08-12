cmake_minimum_required(VERSION 3.0)
project(mapdb)

set(datamodel_SOURCE_DIR "../datamodel")

if(WIN32)
  add_definitions(-fPIC -std=c++11)
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

  if(CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
    link_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib)
  else()
    link_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)
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
  ${datamodel_SOURCE_DIR}/include
)
link_directories(${datamodel_SOURCE_DIR})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
add_library(${PROJECT_NAME} SHARED
  src/db_factory.cpp
  src/mysql_operation.cpp
  src/sqlite_operation.cpp
  src/sqlite_transaction.cpp
)

if(WIN32)
  target_link_libraries(
    ${PROJECT_NAME}
    datamodel
    spatialite
    unofficial::sqlite3::sqlite3
  )
else()
  target_link_libraries(${PROJECT_NAME}
    ${CPPLIB}
    datamodel
    spatialite
  )
endif(WIN32)
