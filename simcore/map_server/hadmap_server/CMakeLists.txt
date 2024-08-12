# Copyright (c) 2022, Tencent Inc.
# All rights reserved.
cmake_minimum_required(VERSION 3.11)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

project(hadmap_server LANGUAGES CXX)

if(EXISTS ${CMAKE_SOURCE_DIR}/.clang-format)
  execute_process(COMMAND cp ${CMAKE_SOURCE_DIR}/.clang-format ${CMAKE_BINARY_DIR}/.clang-format)
endif()

# Cmake invocation variables:
# HADMAP_SVR_BUILD_ONLY - only build project identified by this variable, a semi-colon delimited list, if this is set we will build only the projects listed.
set(HADMAP_SVR_BUILD_ONLY "cloud" CACHE STRING "A semi-colon delimited list of the projects to build")
if(NOT HADMAP_SVR_BUILD_ONLY)
  message(WARNING " build nothing, because HADMAP_SVR_BUILD_ONLY is empty!\n Usage: ... -DHADMAP_SVR_BUILD_ONLY=\"cloud;stand\"")
  return()
endif()
set(PROJECT_BUILD_LIST ${HADMAP_SVR_BUILD_ONLY})

foreach(PROJECT IN LISTS PROJECT_BUILD_LIST)
  set(PROJECT_DIR "${PROJECT}")
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DIR}")
  elseif(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/hdserver/${PROJECT_DIR}")
  else()
    message(FATAL_ERROR " ${PROJECT} is required for build, but ${PROJECT_DIR} directory is missing!")
  endif()
endforeach()


#---------------------------------------------------------------------------------------
# 全局变量
#---------------------------------------------------------------------------------------
set(ALLOW_DUPLICATE_CUSTOM_TARGETS TRUE)
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common" CACHE PATH "common dir" FORCE)
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party" CACHE PATH "third party dir" FORCE)
option(FETCH_SIMMAPSDK "Flag to enable/disable `FetchContent(simmapsdk)`" ON)
option(FETCH_OPENDRIVEIO "Flag to enable/disable `FetchContent(opendirveio)`" ON)
# add prefix
option(HADMAP_SVR_BUILD_TOOLS "Flag to build/non-build tools" ON)
option(HADMAP_SVR_BUILD_TESTING "Flag to build/non-build test" OFF)
option(HADMAP_SVR_ENABLE_TCMALLOC "Flag to enable/disable tcmalloc" OFF)
option(HADMAP_SVR_ENABLE_WRAPPER "Flag to enable/disable build wrapper" ON)
option(HADMAP_SVR_UPGRADE_SCENE_EVENTS "" ON)
option(HADMAP_SVR_UPGRADE_MAP_EDITOR "Flag to enable/disable upgrade map editor" ON)
option(HADMAP_SVR_RANDOM_WITHOUT_BOOST "Flag to enable/disable boost gen random" ON)
option(HADMAP_SVR_XOSC2SIMREC_GEN_SIM_WITH_XOSC_READER "" ON)
#---------------------------------------------------------------------------------------
# 设置CMAKE相关，公有
#---------------------------------------------------------------------------------------
# Ensure that CMAKE_BUILD_TYPE has a value specified for single configuration generators.
set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build: Debug Release RelWithDebInfo MinSizeRel" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${THIRD_PARTY_DIR} ${COMMON_DIR} ${ROOT_DIR}/../../cmake)
find_package(map_sdk REQUIRED)
include(common)
include(third_party)


#---------------------------------------------------------------------------------------
# 设置CPP编译选项，公有
#---------------------------------------------------------------------------------------
# Boost 1.73 中的 Boost.Geometry 弃用了对 C++03 的支持，并且从 Boost 1.75 开始需要 C++14
set(CMAKE_CXX_STANDARD 14 CACHE INTERNAL "-std=c++xx" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options($<$<BOOL:${UNIX}>:-fPIC>)
add_compile_options($<$<BOOL:${MSVC}>:/utf-8>)
#add_compile_options($<$<BOOL:${MSVC}>:/MP6>)
add_compile_options($<$<BOOL:${MSVC}>:/bigobj>)
#add_compile_options($<$<BOOL:${MSVC}>:/wd4828>)
add_compile_options($<$<BOOL:${MSVC}>:-DWIN32_LEAN_AND_MEAN>)
#add_compile_options($<$<AND:$<BOOL:${MSVC}>,$<CONFIG:Debug>>:/Z7>)
add_compile_options(-DBOOST_ALL_NO_LIB)
add_compile_options(-DSOCI_DLL)
add_compile_options(-DTIXML_USE_STL)
add_compile_options(-DPROJ_NEW_API)
if(HADMAP_SVR_UPGRADE_SCENE_EVENTS)
  add_compile_options(-DSCENE_EVENTS_UPGRAD)
endif()
if(HADMAP_SVR_UPGRADE_MAP_EDITOR)
  add_compile_options(-DUPGRADE_MAP_EDITOR)
endif()
if(HADMAP_SVR_RANDOM_WITHOUT_BOOST)
  add_compile_options(-DRANDOM_WITHOUT_BOOST)
endif()
if(HADMAP_SVR_XOSC2SIMREC_GEN_SIM_WITH_XOSC_READER)
  add_compile_options(-DXOSC2SIMREC_GEN_SIM_WITH_XOSC_READER)
endif()
# add_compile_options($<$<AND:$<BOOL:${MSVC}>,$<CONFIG:Release>>:/GL>)
# add_link_options($<$<AND:$<BOOL:${MSVC}>,$<CONFIG:Release>>:/LTCG>)
add_link_options($<$<BOOL:${UNIX}>:-ldl>)
add_link_options($<$<BOOL:${UNIX}>:-pthread>)

if(HADMAP_SVR_BUILD_TESTING)
  include(GoogleTest)
  enable_testing()
  add_compile_options($<$<BOOL:${UNIX}>:-fno-access-control>)
endif()


#---------------------------------------------------------------------------------------
# Start building projects...
#---------------------------------------------------------------------------------------
foreach(PROJECT IN LISTS PROJECT_BUILD_LIST)
  set(PROJECT_DIR "${PROJECT}")
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DIR}")
    add_subdirectory("${PROJECT_DIR}")
  elseif(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/hdserver/${PROJECT_DIR}")
    add_subdirectory("hdserver/${PROJECT_DIR}")
  endif()
endforeach()
