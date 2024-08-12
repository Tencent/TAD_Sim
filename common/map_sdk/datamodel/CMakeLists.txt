cmake_minimum_required(VERSION 3.0)
project(datamodel)

set(CPPLIB "")

if(WIN32)
  add_definitions(-fPIC -std=c++11)
else()
  add_definitions(-fPIC -std=c++11)

  if(UE)
    message(ue_on)
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
)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH_DEBUG ${PROJECT_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH_RELEASE ${PROJECT_SOURCE_DIR})

add_library(${PROJECT_NAME} SHARED
  src/common/coord_trans.cpp
  src/common/map_block.cpp
  src/common/map_util.cpp
  src/common/mapdata_trans.cpp
  src/common/log.cpp
  src/structs/base_struct.cpp
  src/structs/hadmap_curve.cpp
  src/structs/hadmap_lane.cpp
  src/structs/hadmap_laneboundary.cpp
  src/structs/hadmap_lanelink.cpp
  src/structs/hadmap_map.cpp
  src/structs/hadmap_road.cpp
  src/structs/hadmap_section.cpp
  src/structs/hadmap_objgeom.cpp
  src/structs/hadmap_object.cpp
  src/structs/hadmap_junctionroad.cpp
  src/structs/hadmap_junction.cpp
  src/structs/hadmap_route.cpp
  src/structs/hadmap_mapinterface.cpp
  src/structs/hadmap_header.cpp
  src/spatialindex/hadmap_spatialindex.cpp
)

target_link_libraries(${PROJECT_NAME}
  ${CPPLIB}
)
