cmake_minimum_required(VERSION 3.1...3.20)

project(txSimTraffic LANGUAGES CXX)

# set(LIBRARY_OUTPUT_PATH ../${CMAKE_BINARY_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  set(PROTOBUF_LIBRARIES protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  find_package(absl REQUIRED)
else()
  # find cppzmq wrapper, installed by make of cppzmq
  set(CMAKE_SKIP_RPATH TRUE)
  find_package(Protobuf REQUIRED)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/helper_funcs.cmake)
protobuf_generate_cpp(PROTO_SRC_BASIC PROTO_HDR_BASIC ../../../common/message/basic.proto)
protobuf_generate_cpp(PROTO_SRC_LOC PROTO_HDR_LOC ../../../common/message/location.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_TFC ../../../common/message/trajectory.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_TFC ../../../common/message/traffic.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_MSV ../../../common/message/moduleService.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_CLR ../../../common/message/color.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_REC ../../../common/message/record.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_CLR ../../../common/message/visualizer.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_RMI ../../../common/message/mapRange.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_RMI ../../../common/message/rmInternal.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_REB ../../../common/message/regionBasic.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_REB ../../../common/message/log2world_trigger.proto)
protobuf_generate_cpp(PROTO_SRC_ITS PROTO_HDR_ITS ../microsim_msg/its_sim.proto)
protobuf_generate_cpp(PROTO_SRC_ITS PROTO_HDR_ITS ../ditw/proto/ditw_event.proto)
protobuf_generate_cpp(PROTO_SRC_ITS PROTO_HDR_ITS ../ditw/proto/ditw_traffic_event.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_REB ../../../common/message/vehicle_geometry.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_REB ../../../common/message/traffic_billboard.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_REB ../../../common/message/waypoint.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_REB ../../../common/message/scene_common.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_REB ../../../common/message/scene_infrastructure_group.proto)

# dummy driver
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_SCN ../../../common/message/settings.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_SCN ../../../common/message/scene_common.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_SCN ../../../common/message/scene_infrastructure_group.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_SCN ../../../common/message/waypoint.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_SCN ../../../common/message/scene.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_VEI ../../../common/message/vehicle_interaction.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_VEI ../../../common/message/scene_sensor_group.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_ENV ../../../common/message/environment.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_HDR ../../../common/message/header.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_HDR ../../../common/message/control.proto)
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_HDR ../../../common/message/control_v2.proto)

# dummy driver
protobuf_generate_cpp(PROTO_SRC_ITS PROTO_HDR_ITS ../../../common/message/road_realtimeinfo.proto)

# scene_exporter
protobuf_generate_cpp(PROTO_SRC_ITS PROTO_HDR_ITS ../../../common/message/grading.proto)
protobuf_generate_cpp(PROTO_SRC_ITS PROTO_HDR_ITS ../../../common/message/grading_kpi.proto)

# scene_exporter

# chomper
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_CLR ../../../common/message/visualizer.proto)

# ego_group
protobuf_generate_cpp(PROTO_SRC_TFC PROTO_HDR_CLR ../../../common/message/union.proto)

SET(project_SOURCES
  "traffic.pb.cc"
  "location.pb.cc"
  "basic.pb.cc"
  "color.pb.cc"
  "moduleService.pb.cc"
  "trajectory.pb.cc"
  "regionBasic.pb.cc"
  "mapRange.pb.cc"
  "rmInternal.pb.cc"

  # "trafficlog.pb.cc"
  "its_sim.pb.cc"
  "record.pb.cc"
  "ditw_event.pb.cc"
  "ditw_traffic_event.pb.cc"
  "log2world_trigger.pb.cc"

  # "twininfo.pb.cc"
  "vehicle_geometry.pb.cc"
  "traffic_billboard.pb.cc"
  "waypoint.pb.cc"
  "scene_common.pb.cc"
  "scene_infrastructure_group.pb.cc"

  # dummy driver
  "settings.pb.cc"
  "scene_common.pb.cc"
  "scene_infrastructure_group.pb.cc"
  "waypoint.pb.cc"
  "scene.pb.cc"
  "vehicle_interaction.pb.cc"
  "environment.pb.cc"
  "header.pb.cc"
  "control.pb.cc"
  "scene_sensor_group.pb.cc"
  "control_v2.pb.cc"

  # dummy driver
  "road_realtimeinfo.pb.cc"
  "grading.pb.cc"
  "grading_kpi.pb.cc"
  "visualizer.pb.cc"

  # ego_group
  "union.pb.cc"
)

SET(project_HEADERS
  "basic.pb.h"
  "color.pb.h"
  "location.pb.h"
  "traffic.pb.h"
  "moduleService.pb.h"
  "trajectory.pb.h"
  "regionBasic.pb.h"
  "mapRange.pb.h"
  "rmInternal.pb.h"

  # "trafficlog.pb.h"
  "its_sim.pb.h"
  "record.pb.h"
  "ditw_event.pb.h"
  "ditw_traffic_event.pb.h"
  "log2world_trigger.pb.h"

  # "twininfo.pb.h"
  "vehicle_geometry.pb.h"
  "traffic_billboard.pb.h"
  "waypoint.pb.h"
  "scene_common.pb.h"
  "scene_infrastructure_group.pb.h"

  # dummy driver
  "settings.pb.h"
  "scene_common.pb.h"
  "scene_infrastructure_group.pb.h"
  "waypoint.pb.h"
  "scene.pb.h"
  "vehicle_interaction.pb.h"
  "environment.pb.h"
  "header.pb.h"
  "control.pb.h"
  "scene_sensor_group.pb.h"
  "control_v2.pb.h"

  # dummy driver
  "road_realtimeinfo.pb.h"
  "grading.pb.h"
  "grading_kpi.pb.h"
  "visualizer.pb.h"

  # ego_group
  "union.pb.h"
)
add_library(txProtoBuf ${project_SOURCES} ${project_HEADERS})

target_compile_definitions(txProtoBuf PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

if(WIN32)
else()
  target_compile_options(txProtoBuf PRIVATE -fPIC)
endif()

if(WIN32)
  target_link_libraries(txProtoBuf
    ${PROTOBUF_LIBRARIES}
    absl::base absl::strings
  )
else()
  target_link_libraries(txProtoBuf
    ${PROTOBUF_LIBRARIES}
    dl
    rt)
endif()
