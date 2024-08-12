# Copyright (c) 2022, Tencent Inc.
# All rights reserved.
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)

function(find_package_custom)
  CMAKE_PARSE_ARGUMENTS(LIB "NOT_REQUIRED" "NAME;ALIAS;INCLUDE" "" ${ARGN})
  if("${LIB_ALIAS}" STREQUAL "")
    set(LIB_ALIAS ${LIB_NAME})
  endif()
  if(TARGET ${LIB_ALIAS})
    return()
  endif()
  set(REQUIRED "REQUIRED")
  if(${LIB_NOT_REQUIRED})
    set(REQUIRED "")
  endif()

  find_library(${LIB_NAME}_PATH
               NAMES ${LIB_NAME} lib${LIB_NAME}
               PATH_SUFFIXES ${CMAKE_BUILD_TYPE} ${CMAKE_VS_PLATFORM_NAME} ${CMAKE_VS_PLATFORM_NAME}/${CMAKE_BUILD_TYPE}
               NO_CACHE
               ${REQUIRED}
               )
  if(NOT ${LIB_NAME}_PATH)
    return()
  endif()
  # ubuntu16.04 /usr/local/lib/liblog4cp.a without -fPIC
  string(REGEX REPLACE "/lib(64)?($|/.*)" "" ${LIB_NAME}_INCLUDE ${${LIB_NAME}_PATH})
  message(STATUS "Found ${LIB_NAME}: ${${LIB_NAME}_PATH}, ${${LIB_NAME}_INCLUDE}/${LIB_INCLUDE} => ${LIB_ALIAS}")
  add_library(${LIB_ALIAS} STATIC IMPORTED GLOBAL)
  set_target_properties(${LIB_ALIAS} PROPERTIES
    IMPORTED_LOCATION ${${LIB_NAME}_PATH}
    INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${${LIB_NAME}_INCLUDE}/${LIB_INCLUDE};>"
    )
  if(${LIB_NAME} STREQUAL "gflags")
    # fix error: __imp_PathMatchSpec...
    set_target_properties(gflags PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES shlwapi)
  endif()
endfunction()

if(WIN32)
  # include(soci)
  find_package(Boost REQUIRED COMPONENTS date_time system filesystem locale regex thread)
  find_package(PROJ CONFIG REQUIRED)
  find_package(glog CONFIG REQUIRED)
  find_package(gflags CONFIG REQUIRED)
  find_package(jsoncpp CONFIG REQUIRED)
  find_package(protobuf CONFIG REQUIRED)
  find_package(SOCI CONFIG REQUIRED)
  find_package(XercesC REQUIRED)
	find_package(unofficial-sqlite3 CONFIG REQUIRED)
	find_package(CURL REQUIRED)
	find_package(Eigen3 CONFIG REQUIRED)
	find_package(tinyxml CONFIG REQUIRED)
  find_package(log4cplus CONFIG REQUIRED)
  find_package(tinyxml2 CONFIG REQUIRED)
else()
  find_package(Boost REQUIRED COMPONENTS date_time filesystem locale regex system thread)
  find_package(CURL REQUIRED)
  find_package(Protobuf REQUIRED)
  find_package(XercesC REQUIRED)
  find_package(jsoncpp REQUIRED)
  find_package(Eigen3  REQUIRED)
  find_package(tinyxml2  REQUIRED)
  find_package_custom(NAME tbb)
  if(HADMAP_SVR_ENABLE_TCMALLOC)
    find_package_custom(NAME tcmalloc.so.4 ALIAS tcmalloc)
  endif()
  find_package_custom(NAME soci_core INCLUDE "include" REQUIRED)
  find_package_custom(NAME soci_sqlite3 INCLUDE "include" REQUIRED)

  find_path(xsd NAMES xsd REQUIRED)
  message(STATUS "Found xsd: ${xsd}")

  find_program(xsdcxx NAME xsdcxx HINTS /usr/bin /usr/local/bin)
  message(STATUS "Found xsdcxx: ${xsdcxx}")
  find_package_custom(NAME xml2 ALIAS LibXml2 INCLUDE "include/libxml2" NOT_REQUIRED)
endif()


set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_REGEX_LIBRARY}
  ${Boost_THREAD_LIBRARY} ${Boost_LOCALE_LIBRARY})

if(HADMAP_SVR_BUILD_TESTING)
  if(NOT gtest)
    find_package(GTest REQUIRED)
    add_library(gtest ALIAS GTest::gtest)
    add_library(gtest_main ALIAS GTest::gtest_main)
    add_library(gmock ALIAS GTest::gmock)
    add_library(gmock_main ALIAS GTest::gmock_main)
  endif()
endif()

# include(asam)
include(xml)

if(WIN32)
  set(THIRD_PARTY_LIBS
    XercesC::XercesC
    unofficial::sqlite3::sqlite3
    CURL::libcurl
    PROJ::proj
    SOCI::soci_core
    JsonCpp::JsonCpp
    Eigen3::Eigen
    unofficial-tinyxml::unofficial-tinyxml
    glog::glog gflags::gflags
    protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
    log4cplus::log4cplus
    SOCI::soci_sqlite3
    tinyxml2::tinyxml2
    ${LIB_BOOST_FS}
  )

else()
set(THIRD_PARTY_LIBS
  tinyxml
  xerces-c
  spatialite
  CURL::libcurl
  proj
  soci_core
  soci_sqlite3
  sqlite3
  log4cplus
  gflags
  protobuf
  glog
  jsoncpp
  Eigen3::Eigen
  tinyxml2
  ${LIB_BOOST_FS}
  dl
)
endif()
