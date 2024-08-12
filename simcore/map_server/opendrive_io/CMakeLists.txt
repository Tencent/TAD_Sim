cmake_minimum_required(VERSION 3.1)
project(OpenDrivePlugin LANGUAGES C CXX)

# set output
set(EXECUTABLE_OUTPUT_PATH ../../build/bin)
set(LIBRARY_OUTPUT_PATH ../../build/bin)
set(ARCHIVE_OUTPUT_DIRECTORY ../../build/lib)

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -DEIGEN_MALLOC_ALREADY_ALIGNED=0")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -Os -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -DEIGEN_MALLOC_ALREADY_ALIGNED=0")
endif()

# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

#
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

add_definitions("-DGIT_COMMIT_HASH=${GIT_COMMIT_HASH}")
add_definitions("-DGIT_BRANCH=${GIT_BRANCH}")

# common settings (Boost libraries)
if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x0A00)
  add_definitions("/MP")

  # Enable the static libraries on Windows
  foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    # update to the static version of the run time library
    # string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
  endforeach()

  set(CMAKE_C_STANDARD_LIBRARIES "")
  set(CMAKE_CXX_STANDARD_LIBRARIES "")
endif()

if(WIN32)
  find_package(jsoncpp CONFIG REQUIRED)
  find_package(glog CONFIG REQUIRED)
  find_package(gflags CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  message(STATUS "Boost version: ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

  if(${Boost_MAJOR_VERSION} GREATER 1 OR(${Boost_MAJOR_VERSION} EQUAL 1 AND ${Boost_MINOR_VERSION} GREATER 75))
    add_definitions(-DBOOST_NEW_API)
    message(STATUS "BOOST_NEW_API is defined because Boost version is greater than 1.75")
  else()
    message(STATUS "BOOST_NEW_API is not defined because Boost version is less or equal to 1.75")
  endif()

  find_package(PROJ CONFIG REQUIRED)

  # set lib
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set(LIB_GOOGLE_USE glog::glog gflags::gflags JsonCpp::JsonCpp)
  set(LIB_MAPSDK PROJ::proj datamodel mapengine transmission mapdb mapimport routingmap routeplan)

else()
  # find cppzmq wrapper, installed by make of cppzmq
  set(CMAKE_SKIP_RPATH TRUE)
  find_package(Eigen3)

  # boost components
  find_package(Boost REQUIRED system filesystem)
endif()

file(GLOB_RECURSE LOAD_SOURCE ./src/*.cpp)
file(GLOB_RECURSE LOAD_HEADER ./include/*.h)
file(GLOB_RECURSE COMMON_HEADER ./include/common/*.h)
file(GLOB_RECURSE SESSION_HEADER ./include/session/*.h)
file(GLOB_RECURSE SESSION_SOURCE ./include/session/*.cpp)

if(WIN32)
  add_definitions(-DPROJ_NEW_API)
  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include/session
    ${CMAKE_CURRENT_SOURCE_DIR}/src/session
    ../../../common/map_sdk/hadmap/include
  )

  link_directories(
    ../../../common/map_sdk/hadmap/lib
  )

  add_definitions(
    -DUNICODE
    -D_UNICODE
    -D_WINDOWS
    -D_USRDLL
    -DSOCI_DLL
    -DSCENE_WRAPPER_EXPORTS
    -D__HADMAP_SERVER__
    -DTIXML_USE_STL
    -DBOOST_ALL_NO_LIB)

  set(CMAKE_CXX_FLAGS "/utf-8 ${CMAKE_CXX_FLAGS}")
else()
  find_package(glm)
  find_package(jsoncpp)
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include/session
    ${CMAKE_CURRENT_SOURCE_DIR}/src/session
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/map_sdk/hadmap/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include/session
    ${CMAKE_CURRENT_SOURCE_DIR}/src/session
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/xml
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party
    ${Boost_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR})
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../common/map_sdk/hadmap/lib)
  add_definitions(-DSCENE_WRAPPER_EXPORTS -D__HADMAP_SERVER__ -DTIXML_USE_STL
    -DSOCI_DLL -DBOOST_ALL_NO_LIB)

  # -D_GLIBCXX_USE_CXX11_ABI=0
endif()

# set(CMAKE_CXX_FLAGS "-g -std=gnu++11 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=gnu++14 ${CMAKE_CXX_FLAGS} -pthread")

add_library(
  ${PROJECT_NAME} SHARED
  ${LOAD_SOURCE}
  ${LOAD_HEADER}
  ${COMMON_HEADER}
  ${SESSION_HEADER}
  ${SESSION_SOURCE}
)

# target_compile_definitions(${PROJECT_NAME} PRIVATE
# _ENABLE_EXTENDED_ALIGNED_STORAGE)
if(WIN32)
  target_link_libraries(
    ${PROJECT_NAME}
    ${LIB_MAPSDK}
    ${LIB_GOOGLE_USE}
    ${LIB_BOOST_FS}
  )
else()
  target_link_libraries(
    ${PROJECT_NAME}
    glog
    gflags
    routeplan
    routingmap
    mapimport
    mapengine
    mapdb
    transmission
    datamodel
    spatialite
    curl
    sqlite3
    log4cpp
    jsoncpp
    proj
    boost_filesystem
    boost_system
    boost_locale
    boost_regex
    boost_thread
    boost_date_time
    soci_core
    soci_sqlite3
    tbb
    dl
    pthread)
endif()

# add_subdirectory(test)
