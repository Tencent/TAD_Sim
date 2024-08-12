cmake_minimum_required(VERSION 3.0)
project(mapimport)

set(datamodel_SOURCE_DIR "../datamodel")

if(PROJ_NEW_API)
  add_definitions(-DPROJ_NEW_API)
endif(PROJ_NEW_API)

set(PROJLIB proj)

if(WIN32)
  find_package(tinyxml2 CONFIG REQUIRED)
  add_definitions(-fPIC -std=c++11)

  # find_package(PROJ4 CONFIG REQUIRED)
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

  if(CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]$")
    link_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib)
    message(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib)
    set(PROJLIB proj_d)
  else()
    link_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)
    message(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)
  endif()

  set(TINYXMLLIB tinyxml2::tinyxml2)
else()
  find_package(tinyxml2 CONFIG REQUIRED)
  add_definitions(-fPIC -std=c++11)

  if(UE)
    set(UE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../UE_H/4.27/Linux/LibCxx)
    include_directories(${UE_DIR}/include/c++/v1
      ${UE_DIR}/include)
    add_definitions(-nostdinc++)
    set(CPPLIB ${UE_DIR}/lib/Linux/x86_64-unknown-linux-gnu/libc++.a)
  endif(UE)

  set(TINYXMLLIB tinyxml2)
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
  src/OpenDriveParse.cpp
  src/OpenDriveStruct.cpp
  src/ParseInterface.cpp
  src/ParserFactory.cpp
  src/OpenDriveOutput.cpp
)

if(WIN32)
  target_link_libraries(${PROJECT_NAME}
    datamodel
    ${TINYXMLLIB}
    ${PROJLIB})
else()
  target_link_libraries(${PROJECT_NAME}
    ${CPPLIB}
    datamodel
    ${TINYXMLLIB}
    ${PROJLIB})
endif(WIN32)
