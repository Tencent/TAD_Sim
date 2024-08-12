cmake_minimum_required(VERSION 3.1)

project(txsim-sim-sensor)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../framework_v2/src
  ${CMAKE_BINARY_DIR})

if(WIN32)
  set(LIB_PROTOBUF libprotobuf zlibstatic)
  set(LIB_SYS Shlwapi)
  set(LIB_BOOST_FS libboost_filesystem libboost_system)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../deps
  )
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../deps/lib/${CMAKE_BUILD_TYPE}
  )
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  find_package(Protobuf REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  add_definitions(-fPIC)
  include_directories(
    ${BOOST_INCLUDE_DIR}
  )
  set(LIB_PROTOBUF protobuf pthread)
  set(LIB_SYS dl)
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
endif(WIN32)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/helper_funcs.cmake)
protobuf_generate_cpp(PROTO_SRC_sensor_raw PROTO_HDR_sensor_raw ../sim_msg/sensor_raw.proto)

file(GLOB SRC
  "*.h"
  "*.cpp"
)

# example 1: built into a shared library.
add_library(sim-sensor SHARED
  ${SRC}
  ${PROTO_SRC_sensor_raw}
)
target_link_libraries(sim-sensor
  ${LIB_PROTOBUF}
  ${LIB_BOOST_FS}
  ${LIB_JSON})
