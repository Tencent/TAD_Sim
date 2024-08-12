cmake_minimum_required(VERSION 3.0)
project(hadmap_test)

if(WIN32)
  include_directories(
    ../deps
    ../include
  )
  link_directories(
    ../deps/lib/${CMAKE_BUILD_TYPE}
    ../lib/x64/${CMAKE_BUILD_TYPE}
    ../deps/lib/x64
  )
  set(LIB_MAPSDK map_engine mapdb transmission map_import datamodel spatialite_i sqlite3_i libcurl proj_i)
  set(DEP_LIBS_SYS)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  add_definitions(-fPIC)
  include_directories(
    ../datamodel/include
    ../transmission/include
    ../routingmap/include
    ../mapdb/include
    ../map_engine/include
    ../datamodel/include
  )
  link_directories(
    ../devel/bin
  )
  set(LIB_MAPSDK mapengine mapdb transmission mapimport datamodel spatialite sqlite3 curl proj)
  set(DEP_LIBS_SYS pthread)
  set(CMAKE_SKIP_RPATH TRUE)
  set(CMAKE_SKIP_BUILD_RPATH TRUE)
  set(CMAKE_SKIP_INSTALL_RPATH TRUE)
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME}
  test.cpp
)
target_link_libraries(${PROJECT_NAME} PUBLIC
  ${LIB_MAPSDK}
  ${DEP_LIBS_SYS}
)
