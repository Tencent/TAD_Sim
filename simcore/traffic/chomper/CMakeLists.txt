cmake_minimum_required(VERSION 3.1...3.20)

project(txTraffic LANGUAGES CXX)

# set(EXECUTABLE_OUTPUT_PATH ../${CMAKE_BINARY_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake/)
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

add_definitions("-DON_CHOMPER=1")

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /std:c++14 -DEIGEN_MALLOC_ALREADY_ALIGNED=0 -O2 -Ob2 -Oi -Ot")
  add_compile_options("/utf-8")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -g -Os -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -DEIGEN_MALLOC_ALREADY_ALIGNED=0")
  add_compile_options(-finput-charset=UTF-8)
  add_compile_options(-fexec-charset=UTF-8)
endif()

function(modify_project_version)
  string(TIMESTAMP PROJECT_BUILD_YEAR "%Y")
  string(TIMESTAMP PROJECT_BUILD_DATE "%Y%m%d")
  string(TIMESTAMP PROJECT_BUILD_TIME "%H%M%S")

  # get svn/git commit reversion
  if(EXISTS "${PROJECT_STORE_PATH}/.git/")
    find_package(Git)

    if(GIT_FOUND)
      execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags RESULT_VARIABLE res_code OUTPUT_VARIABLE GIT_COMMIT_ID)

      if(${res_code} EQUAL 0)
        message("-- Get git revision success")

        # -g: tag of git
        string(FIND ${GIT_COMMIT_ID} "-g" pos)

        if(${pos} GREATER 0)
          string(SUBSTRING ${GIT_COMMIT_ID} ${pos} -1 COMMIT_ID)
          string(SUBSTRING ${COMMIT_ID} 2 -1 PROJECT_REVISION)
          message("-- Git commit id: ${PROJECT_REVISION}")
        endif()
      else(${res_code} EQUAL 0)
        message(WARNING "-- Git failed (not a repo, or no tags). Build will not contain git revision info.")
      endif(${res_code} EQUAL 0)
    else(GIT_FOUND)
      message("-- Git not found!)")
    endif(GIT_FOUND)
  endif(EXISTS "${PROJECT_STORE_PATH}/.git/")

  # generate the version file
  set(VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/version/version.h)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version/version.h.in"
    "${VERSION_FILE}"
    @ONLY)
endfunction()

modify_project_version()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/protobuf
  "./include"
  "./plugin/include"
  "./version"
)

if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  set(PROTOBUF_LIBRARIES protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  find_package(glog CONFIG REQUIRED)
  find_package(TBB CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(ZLIB REQUIRED)
  find_package(CURL REQUIRED)
else()
  # find cppzmq wrapper, installed by make of cppzmq
  set(CMAKE_SKIP_RPATH TRUE)
  find_package(Eigen3)
  find_package(Protobuf REQUIRED)
endif()

if(WIN32)
  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
  )
else()
  include_directories(
    ${EIGEN3_INCLUDE_DIR}
    ${TBB_INCLUDE_DIRS})
  add_definitions(${TBB_DEFINITIONS})
endif()

if(WIN32)
  link_directories(
    ${CMAKE_BINARY_DIR}/lib
  )
else()
  link_directories(
    ${CMAKE_BINARY_DIR}/lib
  )
endif()

SET(project_SOURCES
  "./src/ibeo_data.cpp"
  "./src/Trafficga.cpp"
  "./src/world_manager.cpp"
  "./src/defines.cpp"
  "./src/tx_math.cpp"
  "./src/ga.cpp"

  ${CMAKE_BINARY_DIR}/protobuf/traffic.pb.cc
  ${CMAKE_BINARY_DIR}/protobuf/basic.pb.cc
  ${CMAKE_BINARY_DIR}/protobuf/visualizer.pb.cc
  ${CMAKE_BINARY_DIR}/protobuf/trajectory.pb.cc
  ${CMAKE_BINARY_DIR}/protobuf/location.pb.cc
)

SET(project_HEADERS
  "./include/ibeo_data.h"
  "./plugin/include/tad_chomper_interface.h"
  "./include/Trafficga.h"
  "./include/world_manager.h"
  "./include/defines.h"
  "./include/tx_math.h"
  "./include/ga.h"
)

add_executable(txChomper ${project_SOURCES} ${project_HEADERS})
add_dependencies(txChomper txSimTrafficPlugin)

target_compile_definitions(txChomper PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

if(WIN32)
else()
  target_compile_options(txChomper PRIVATE -fPIC)
endif()

if(WIN32)
  target_link_libraries(txChomper
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    glog::glog
    gflags::gflags
    ZLIB::ZLIB
    routeplan.lib
    ${MAPSDK_LIBRARYS}
    CURL::libcurl
    shlwapi
    Advapi32.lib
    Rpcrt4.lib
    Ws2_32.lib
    Iphlpapi.lib
    ${PROTOBUF_LIBRARIES}
  )
else()
  target_link_libraries(txChomper
    protobuf
    ${MAPSDK_LIBRARYS}
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
    rt)
endif()

# if(WIN32)
# if(NOT CMAKE_BUILD_TYPE)
# copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
# copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
# else()
# copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
# copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
# endif()
# endif()
