cmake_minimum_required(VERSION 3.0)
project(hadmap)

set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../deps" CACHE PATH "deps dir")
set(simmapsdk_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(ALLOW_DUPLICATE_CUSTOM_TARGETS TRUE)
set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build: Debug Release RelWithDebInfo MinSizeRel")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${DEPS_DIR})
set(CMAKE_INSTALL_PREFIX ${simmapsdk_SOURCE_DIR} CACHE PATH "Install path prefix" FORCE)

add_compile_options($<$<BOOL:${UNIX}>:-fPIC>)
add_compile_options($<$<BOOL:${MSVC}>:/utf-8>)
add_compile_options($<$<BOOL:${MSVC}>:/MP6>)
add_compile_options($<$<BOOL:${MSVC}>:/bigobj>)
add_compile_options($<$<BOOL:${MSVC}>:-DWIN32_LEAN_AND_MEAN>)
add_compile_options($<$<AND:$<BOOL:${MSVC}>,$<CONFIG:Debug>>:/Z7>)
add_link_options($<$<BOOL:${UNIX}>:-ldl>)
add_link_options($<$<BOOL:${UNIX}>:-pthread>)

function(find_package_custom)
  CMAKE_PARSE_ARGUMENTS(LIB "" "NAME;ALIAS;INCLUDE" "" ${ARGN})

  if("${LIB_ALIAS}" STREQUAL "")
    set(LIB_ALIAS ${LIB_NAME})
  endif()

  if(TARGET ${LIB_ALIAS})
    return()
  endif()

  find_library(${LIB_NAME}_PATH
    NAMES ${LIB_NAME} lib${LIB_NAME}
    PATH_SUFFIXES ${CMAKE_BUILD_TYPE} ${CMAKE_VS_PLATFORM_NAME} ${CMAKE_VS_PLATFORM_NAME}/${CMAKE_BUILD_TYPE}
    NO_CACHE
    REQUIRED
  )
  string(REGEX REPLACE "/lib($|/.*)" "" ${LIB_NAME}_INCLUDE ${${LIB_NAME}_PATH})
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
  find_package_custom(NAME curl ALIAS CURL::libcurl INCLUDE "include")
  find_package_custom(NAME proj_i ALIAS proj INCLUDE "include/proj-4.8.0")
  find_package_custom(NAME spatialite_i ALIAS spatialite INCLUDE "include")
  find_package_custom(NAME sqlite3_i ALIAS sqlite3 INCLUDE "include")
else()
  find_package(CURL REQUIRED)
endif()

# ---------------------------------------------------------------------------------------
# build sub libs
# ---------------------------------------------------------------------------------------
option(BUILD_SHARED_LIBS "Default build static libs" OFF)

if(UE AND UNIX)
  set(UE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../UE_H/4.27/Linux/LibCxx)
  include_directories(${UE_DIR}/include/c++/v1 ${UE_DIR}/include)
  set(UE_LIBC ${UE_DIR}/lib/Linux/x86_64-unknown-linux-gnu/libc++.a)
  add_compile_options(-nostdinc++)
  set(BUILD_SHARED_LIBS ON)
  add_compile_options(-DTXSIMMAP_API)
  add_compile_options(-DTXSIMMAP_EXPORTS)
endif()

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)

list(APPEND simmapsdk_SUB_DIRS datamodel map_import mapdb transmission map_engine route_plan routingmap)

if(WIN32)
  list(APPEND simmapsdk_SUB_LIBS datamodel map_import mapdb transmission map_engine route_plan routingmap)
else()
  list(APPEND simmapsdk_SUB_LIBS datamodel mapimport mapdb transmission mapengine routeplan routingmap)
endif()

list(APPEND simmapsdk_SUB_DEPS
  "${UE_LIBC}"
  "::datamodel proj ${UE_LIBC}"
  "::datamodel spatialite sqlite3 ${UE_LIBC}"
  "::datamodel CURL::libcurl ${UE_LIBC}"
  "::map_import ::mapdb ::transmission ::datamodel ${UE_LIBC}"
  "::map_engine ::mapdb ::transmission ::datamodel ${UE_LIBC}"
  "::route_plan ::map_engine ::mapdb ::transmission ::datamodel ${UE_LIBC}"
)

list(APPEND simmapsdk_SUB_INCS include include/map_import include include include include include)

foreach(DIR LIB DEP INC IN ZIP_LISTS simmapsdk_SUB_DIRS simmapsdk_SUB_LIBS simmapsdk_SUB_DEPS simmapsdk_SUB_INCS)
  file(GLOB_RECURSE LIB_SRCS ${simmapsdk_SOURCE_DIR}/${DIR}/*.h
    ${simmapsdk_SOURCE_DIR}/${DIR}/*.c*
    ${simmapsdk_SOURCE_DIR}/${DIR}/*.inl)
  add_library(${LIB} ${LIB_SRCS})
  target_include_directories(${LIB} PUBLIC ${simmapsdk_SOURCE_DIR}/${DIR}/include ${DEPS_DIR}/include ${DEPS_DIR})
  add_library(::${DIR} ALIAS ${LIB})

  if(NOT "${DEP}" STREQUAL "")
    string(REPLACE " " ";" DEP ${DEP})
    target_link_libraries(${LIB} PRIVATE ${DEP})
  endif()

  install(DIRECTORY ${simmapsdk_SOURCE_DIR}/${DIR}/include/ DESTINATION ${INC})
endforeach()

if(WIN32)
  foreach(CONFIGURATION Debug Release RelWithDebInfo MinSizeRel)
    install(TARGETS ${simmapsdk_SUB_LIBS}
      CONFIGURATIONS ${CONFIGURATION}
      RUNTIME DESTINATION dll/${CMAKE_VS_PLATFORM_NAME}/${CONFIGURATION}
      ARCHIVE DESTINATION lib/${CMAKE_VS_PLATFORM_NAME}/${CONFIGURATION}
    )
  endforeach()
else()
  install(TARGETS ${simmapsdk_SUB_LIBS} DESTINATION devel/bin)
endif()

# ---------------------------------------------------------------------------------------
# build test
# ---------------------------------------------------------------------------------------
if(UNIX)
  add_executable(hadmap_test ${simmapsdk_SOURCE_DIR}/test/test.cpp)
  target_link_libraries(hadmap_test ${simmapsdk_SUB_LIBS} CURL::libcurl proj spatialite sqlite3)
endif()
