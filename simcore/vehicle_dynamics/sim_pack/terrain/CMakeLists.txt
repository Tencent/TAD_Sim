cmake_minimum_required(VERSION 3.4.0)

set(TERRAIN_ROOT ${TXCAR_ROOT}/sim_pack/terrain)
set(TERRAIN_NAME terrain CACHE INTERNAL "terrain module name")
set(OPENCRG_NAME opencrg CACHE INTERNAL "opencrg module name")

project(${TERRAIN_NAME})

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DBUILD_TERRAIN_DLL)

# set hdmap include dirs global
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")
find_package(map_sdk REQUIRED)

include_directories(${TERRAIN_ROOT}/inc ${TERRAIN_ROOT}/inc/crg)

if(WIN32)
  # annoying windows define
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DNOMINMAX)

  # set include and link dir
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

  set(LIB_SYS shlwapi)

  find_package(glog CONFIG REQUIRED)
  set(LIB_LOG glog::glog gflags::gflags)

elseif(UNIX)
  # add define
  add_definitions("-D__unix__")
  add_compile_options(-fPIC)

  set(LIB_SYS pthread)
  set(LIB_LOG glog gflags)
endif(WIN32)

# Terrain & OpenCRG sources
aux_source_directory(${TERRAIN_ROOT}/src terrain_src_files)
aux_source_directory(${TERRAIN_ROOT}/src/crg crg_src_files)

# Add libraries
add_library(${TERRAIN_NAME} SHARED ${terrain_src_files})
add_library(${OPENCRG_NAME} ${crg_src_files})

# Link libraries
if(WIN32)
  target_link_libraries(${TERRAIN_NAME} ${LIB_SYS} ${LIB_LOG} ${MAPSDK_LIBRARYS} ${OPENCRG_NAME})
else()
  target_link_libraries(${TERRAIN_NAME} -Wl,--whole-archive ${MAPSDK_LIBRARYS} ${OPENCRG_NAME} -Wl,--no-whole-archive ${LIB_SYS} ${LIB_LOG})
endif()

# copy_mapsdk_deps
if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
