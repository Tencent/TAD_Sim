cmake_minimum_required(VERSION 3.1...3.20)

project(txTraffic LANGUAGES CXX)

# set(LIBRARY_OUTPUT_PATH ../${CMAKE_BINARY_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake/)

add_definitions("-DTBB_NEW_API=1") # TBB >= v2021.5

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  set(PROTOBUF_LIBRARIES protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  find_package(glog CONFIG REQUIRED)
  find_package(TBB CONFIG REQUIRED)
  find_package(Eigen3)
  find_package(nlohmann_json CONFIG REQUIRED)
  find_package(cereal CONFIG REQUIRED)
else()
  # find cppzmq wrapper, installed by make of cppzmq
  set(CMAKE_SKIP_RPATH TRUE)
  find_package(Eigen3)
  find_package(TBB)
  find_package(Protobuf REQUIRED)
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/protobuf
  "../io/include"
  "./include"
  "../common/include"
  "../3rd"
)

if(WIN32)
  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/eigen3
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/tbb
  )
else()
  include_directories(
    ${EIGEN3_INCLUDE_DIR}
    ${TBB_INCLUDE_DIRS})
  add_definitions(${TBB_DEFINITIONS})
endif()

link_directories(${CMAKE_BINARY_DIR}/lib)

SET(project_SOURCES
  "./src/tx_application.cpp"
  "./src/tx_loop.cpp"
  "./src/tx_simulation_loop.cpp"
  "./src/tx_map_tracker.cpp"
  "./src/tx_traffic_element_base.cpp"
  "./src/tx_element_manager_base.cpp"
  "./src/tx_linear_element_manager.cpp"
  "./src/tx_scene_loader_factory.cpp"
  "./src/tx_scene_loader.cpp"
  "./src/tx_spatial_query.cpp"
  "./src/tx_vehicle_element.cpp"
  "./src/tx_traffic_element_system.cpp"
  "./src/tx_topological_graph.cpp"
  "./src/tx_signal_element.cpp"
  "./src/tx_pedestrian_element.cpp"
  "./src/tx_mutex.cpp"
  "./src/tx_external_injection_event.cpp"
  "./src/tx_event_utils.cpp"
  "./src/tx_vehicle_coord.cpp"
  "./src/tx_catalog_cache.cpp"
  "./src/tx_sys_info.cpp"
  "./src/tx_scene_sketch.cpp"
  "./src/HdMap/tx_lane_geom_info.cpp"
  "./src/HdMap/tx_hd_map_cache_concurrent.cpp"
  "./src/HdMap/tx_road_network.cpp"
  "./src/HdMap/tx_hashed_road.cpp"
  "./src/HdMap/tx_hashed_lane_info.cpp"
  "./src/HdMap/hashed_lane_info_orthogonal_list.cpp"
  "./src/HdMap/tx_hashed_road_area.cpp"
  "./src/SceneLoader/tx_tadloader.cpp"
  "./src/SceneLoader/tx_xoscloader.cpp"
  "./src/SceneLoader/tx_assembler_factory.cpp"
  "../common/src/tx_logger.cpp"
  "../common/src/tx_math.cpp"
  "../common/src/tx_units.cpp"
  "../common/src/tx_sim_point.cpp"
  "../common/src/tx_flags.cpp"
  "../common/src/tx_profile_utils.cpp"
  "../common/src/tx_protobuf_utils.cpp"
  "../common/src/tx_signal_utils.cpp"
  "../common/src/tx_component.cpp"
  "../common/src/tx_dead_line_area_manager.cpp"
  "../common/src/tx_obb.cpp"
  "../common/src/tx_collision_detection2d.cpp"
  "../common/src/tx_path_utils.cpp"
  "../common/src/tx_locate_info.cpp"
  "../common/src/tx_optional.cpp"
  "../common/src/tx_coord_trans.cpp"
  "../common/src/tx_element_generator.cpp"
  "../common/src/tx_centripetal_cat_mull.cpp"
  "../common/src/tx_id_manager.cpp"
  "../common/src/tx_geometry.cpp"
  "../common/src/tx_geometry_element_base.cpp"
  "../common/src/tx_geometry_element_line.cpp"
)

SET(project_HEADERS
  "./include/tx_application.h"
  "./include/tx_loop.h"
  "./include/tx_simulation_loop.h"
  "./include/tx_traffic_element_system.h"
  "./include/tx_element_manager_base.h"
  "./include/tx_linear_element_manager.h"
  "./include/tx_scene_loader.h"
  "./include/tx_system_module.h"
  "./include/tx_event_utils.h"
  "./include/HdMap/tx_hd_map_cache_concurrent.h"
  "./include/HdMap/tx_lane_geom_info.h"
  "./include/HdMap/tbb_utils.h"
  "./include/HdMap/tx_road_network.h"
  "./include/HdMap/tx_hashed_road.h"
  "./include/HdMap/tx_hashed_lane_info.h"
  "./include/HdMap/hashed_lane_info_orthogonal_list.h"
  "./include/HdMap/tx_hashed_road_area.h"
  "./include/SceneLoader/tx_tadloader.h"
  "./include/SceneLoader/tx_xoscloader.h"
  "./include/tx_spatial_query.h"
  "./include/tx_serialization.h"
  "./include/tx_traffic_element_base.h"
  "./include/tx_trajectory_sampling_node.h"
  "./include/tx_mutex.h"
  "./include/tx_map_tracker.h"
  "./include/tx_scene_loader_factory.h"
  "./include/tx_vehicle_element.h"
  "./include/SceneLoader/tx_assembler_factory.h"
  "./include/tx_env_perception.h"
  "./include/tx_topological_graph.h"
  "./include/tx_signal_element.h"
  "./include/tx_signal_phase_period.h"
  "./include/tx_pedestrian_element.h"
  "./include/tx_driving_follower.h"
  "./include/tx_driving_change_lane.h"
  "./include/tx_traffic_rules.h"
  "./include/tx_traffic_junction_rules.h"
  "./include/tx_external_injection_event.h"
  "./include/tx_traj_manager_base.h"
  "./include/tx_trajectory_follower.h"
  "./include/tx_vehicle_coord.h"
  "./include/tx_catalog_cache.h"
  "./include/tx_sys_info.h"
  "./include/tx_scene_sketch.h"
  "./include/tx_nearest_object_info.h"
  "../common/include/tx_algebra_defs.h"
  "../common/include/tx_container_defs.h"
  "../common/include/tx_math.h"
  "../common/include/tx_marco.h"
  "../common/include/tx_logger.h"
  "../common/include/tx_type_def.h"
  "../common/include/tx_topo_def.h"
  "../common/include/tx_enum_def.h"
  "../common/include/tx_sim_time.h"
  "../common/include/tx_units.h"
  "../common/include/tx_sim_point.h"
  "../common/include/tx_flags.h"
  "../common/include/tx_header.h"
  "../common/include/tx_locate_info.h"
  "../common/include/tx_string_utils.h"
  "../common/include/tx_hadmap_utils.h"
  "../common/include/tx_path_utils.h"
  "../common/include/tx_frame_utils.h"
  "../common/include/tx_enum_utils.h"
  "../common/include/tx_parallel_def.h"
  "../common/include/tx_time_utils.h"
  "../common/include/tx_profile_utils.h"
  "../common/include/tx_protobuf_utils.h"
  "../common/include/tx_signal_utils.h"
  "../common/include/tx_component.h"
  "../common/include/tx_dead_line_area_manager.h"
  "../common/include/tx_timer_on_cpu.h"
  "../common/include/tx_obb.h"
  "../common/include/tx_assembler_context.h"
  "../common/include/tx_count_down_timer.h"
  "../common/include/tx_type_traits.h"
  "../common/include/tx_collision_detection2d.h"
  "../common/include/tx_optional.h"
  "../common/include/tx_coord_trans.h"
  "../common/include/tx_probability_generator.h"
  "../common/include/tx_map_info.h"
  "../common/include/tx_element_generator.h"
  "../common/include/tx_id_manager.h"
  "../common/include/tx_cat_mull_rom.h"
  "../common/include/tx_class_counter.h"
  "../common/include/tx_centripetal_cat_mull.h"
  "../common/include/tx_geometry.h"
  "../common/include/tx_geometry_element_line.h"
  "../common/include/tx_geometry_element_base.h"
)

add_library(txTrafficFramework ${project_SOURCES} ${project_HEADERS})

target_compile_definitions(txTrafficFramework PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

if(WIN32)
else()
  target_compile_options(txTrafficFramework PRIVATE -fPIC)
endif()

if(WIN32 OR WithDependency)
  message(STATUS "framework add_dependencies")
  add_dependencies(txTrafficFramework txProtoBuf txSceneLoader)
endif()

if(WIN32)
  target_link_libraries(txTrafficFramework
    txProtoBuf.lib
    txSceneLoader.lib
    ${MAPSDK_LIBRARYS}
    TBB::tbb
    ${PROTOBUF_LIBRARIES}
    glog::glog
    gflags::gflags
    nlohmann_json::nlohmann_json
    cereal::cereal
  )
else()
  target_link_libraries(
    txProtoBuf
    txSceneLoader
    ${MAPSDK_LIBRARYS}
    tbb
    ${PROTOBUF_LIBRARIES}
    glog
    gflags
    dl
    pthread
    rt
  )
endif()

if(BUILD_TESTING)
  add_executable(txTrafficFramework_test src/HdMap/tx_hd_map_cache_concurrent_test.cc)
  target_link_libraries(txTrafficFramework_test gtest gtest_main)
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
