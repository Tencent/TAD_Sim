cmake_minimum_required(VERSION 3.1...3.20)

project(txTraffic LANGUAGES CXX)

# set(EXECUTABLE_OUTPUT_PATH ../${CMAKE_BINARY_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake/)

add_definitions("-DON_LOCAL=1")
add_definitions("-DInjectMode=1")
add_definitions("-DTBB_NEW_API=1") # TBB >= v2021.5

function(modify_project_version)
  string(TIMESTAMP PROJECT_BUILD_YEAR "%Y")
  string(TIMESTAMP PROJECT_BUILD_DATE "%Y%m%d")
  string(TIMESTAMP PROJECT_BUILD_TIME "%H%M%S")

  # get git commit reversion
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

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  set(PROTOBUF_LIBRARIES protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  find_package(glog CONFIG REQUIRED)
  find_package(TBB CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem)
  find_package(ZLIB REQUIRED)
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
  "./include"
  "../logsim"
  "../common/include"
  "../framework/include"
  "../io/include"
  "../3rd"
  "./app"
  "./app/include"
  "../app/manual_vehicle"
  "./version"
  "../log2world"
  "../element_impl"
)

if(WIN32)
  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
  )
else()
  include_directories(
    ${EIGEN3_INCLUDE_DIR}
    ${TBB_INCLUDE_DIRS}
  )
  add_definitions(${TBB_DEFINITIONS})
endif()

link_directories(${CMAKE_BINARY_DIR}/lib)

SET(project_SOURCES
  "./src/worldsim_sim_template.cpp"
  "./src/passive_sim_template.cpp"
  "./src/logsim_sim_template.cpp"
  "./src/tadsim_application.cpp"
  "./src/tadsim_loop.cpp"
  "./src/tx_tadsim_flags.cpp"
  "./src/tx_event_dispatcher.cpp"
  "./src/tx_way_points.cpp"
  "./src/tx_route_path_manager.cpp"
  "./src/tx_event.cpp"
  "./src/tad_component.cpp"
  "./src/tad_vehicle_behavior_configure_utils.cpp"
  "./src/tad_assembler_context.cpp"
  "./src/tad_stand_alone_traffic_element_system.cpp"
  "./src/tad_ego_vehicle_element.cpp"
  "./src/tad_signal_element.cpp"
  "./src/tad_signal_phase_period.cpp"
  "./src/tad_vehicle_kinetics_info.cpp"
  "./src/tad_linear_element_manager.cpp"
  "./src/tad_pedestrian_element.cpp"
  "./src/tad_pedestrian_motion_event.cpp"
  "./src/tad_obstacle_element.cpp"
  "./src/tad_map_obstacle_element.cpp"
  "./src/tad_user_defined_vehicle_element.cpp"
  "./src/tad_ai_vehicle_element.cpp"
  "./src/tad_behavior_time_manager.cpp"
  "./src/tad_acc_event.cpp"
  "./src/tad_vehicle_merge_event.cpp"
  "./src/tad_velocity_event.cpp"
  "./src/tad_element_generator.cpp"
  "./src/tad_route_group_agent.cpp"
  "./src/tad_scene_event_1_0_0_0.cpp"
  "./src/tad_event_dispatcher_1_0_0_0.cpp"
  "./src/tad_traj_follow_vehicle_element.cpp"
  "./src/tad_planning.cpp"
  "./src/tad_simrec_loader.cpp"
  "./src/tad_scene_loader_factory.cpp"
  "./src/tad_relative_trajectory_follow_element.cpp"
  "./manual_vehicle/tad_manual_vehicle_template.cpp"
  "./manual_vehicle/tad_manual_vehicle_system.cpp"
  "./manual_vehicle/tad_manual_assembler_context.cpp"
  "./manual_vehicle/tad_manual_user_defined_vehicle_element.cpp"
  "./manual_vehicle/tad_manual_ai_vehicle_element.cpp"

  "../logsim/interpolate_player_module.cc"
  "./tx_sim_traffic.cpp"

  "../log2world/log2world_template.cpp"
  "../log2world/log2world_system.cpp"
  "../log2world/log2world_assembler_context.cpp"
  "../log2world/log2world_traj_manager.cpp"
  "../log2world/l2w_vehicle_element.cpp"
  "../log2world/l2w_linear_element_manager.cpp"
)

SET(project_HEADERS
  "./include/tadsim_application.h"
  "./include/tadsim_loop.h"
  "./include/worldsim_sim_template.h"
  "./include/passive_sim_template.h"
  "./include/logsim_sim_template.h"
  "./include/tx_tadsim_flags.h"
  "./include/tx_event_dispatcher.h"
  "./include/tx_event.h"
  "./include/tx_event_handler.h"
  "./include/tad_event_trigger_param.h"
  "./include/tx_way_points.h"
  "./include/tx_route_path_manager.h"
  "./include/tad_component.h"
  "./include/tad_vehicle_behavior_configure_utils.h"
  "./include/tad_stand_alone_traffic_element_system.h"
  "./include/tad_ego_vehicle_element.h"
  "./include/tad_signal_element.h"
  "./include/tad_signal_phase_period.h"
  "./include/tad_vehicle_kinetics_info.h"
  "./include/tad_assembler_context.h"
  "./include/tad_linear_element_manager.h"
  "./include/tad_pedestrian_element.h"
  "./include/tad_pedestrian_trajectory.h"
  "./include/tad_pedestrian_motion_event.h"
  "./include/tad_obstacle_element.h"
  "./include/tad_map_obstacle_element.h"
  "./include/tad_user_defined_vehicle_element.h"
  "./include/tad_ai_vehicle_element.h"
  "./include/tad_fsm_user_defined.h"
  "./include/tad_fsm_ai.h"
  "./include/tad_behavior_time_manager.h"
  "./include/tad_acc_event.h"
  "./include/tad_vehicle_merge_event.h"
  "./include/tad_velocity_event.h"
  "./include/tad_vehicle_scan_region.h"
  "./include/tad_vehicle_nearest_object_helper.h"
  "./include/tad_element_generator.h"
  "./include/tad_route_group_agent.h"
  "./include/tad_scene_event_1_0_0_0.h"
  "./include/tad_event_dispatcher_1_0_0_0.h"
  "./include/tad_traj_follow_vehicle_element.h"
  "./include/tad_planning.h"
  "./include/tad_simrec_loader.h"
  "./include/tad_scene_loader_factory.h"
  "./include/tad_relative_trajectory_follow_element.h"

  "./manual_vehicle/tad_manual_vehicle_template.h"
  "./manual_vehicle/tad_manual_vehicle_system.h"
  "./manual_vehicle/tad_manual_handler.h"
  "./manual_vehicle/tad_manual_assembler_context.h"
  "./manual_vehicle/tad_manual_user_defined_vehicle_element.h"
  "./manual_vehicle/tad_manual_ai_vehicle_element.h"

  "../logsim/interpolate_player_module.h"

  "../log2world/log2world_template.h"
  "../log2world/log2world_system.h"
  "../log2world/log2world_assembler_context.h"
  "../log2world/log2world_traj_manager.h"
  "../log2world/l2w_vehicle_element.h"
  "../log2world/l2w_linear_element_manager.h"
)

file(GLOB ElementWrap_SOURCES ../element_impl/*.cpp)
file(GLOB ElementWrap_HEADERS ../element_impl/*.h)
message(STATUS "ElementWrap_SOURCES = " ${ElementWrap_SOURCES})
message(STATUS "ElementWrap_HEADERS = " ${ElementWrap_HEADERS})
add_executable(txSimTraffic ${project_SOURCES} ${project_HEADERS} ${ElementWrap_SOURCES} ${ElementWrap_HEADERS})

target_compile_definitions(txSimTraffic PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

if(WIN32)
else()
  target_compile_options(txSimTraffic PRIVATE -fPIC)
endif()

if(WIN32 OR WithDependency)
  message(STATUS "app add_dependencies")
  add_dependencies(txSimTraffic txTrafficFramework)
endif()

if(WIN32)
  target_link_libraries(txSimTraffic
    txProtoBuf.lib
    txSceneLoader.lib
    txTrafficFramework.lib
    ${MAPSDK_LIBRARYS}
    ${FRAMEWORK_LIBRARY_SERVICE}
    TBB::tbb
    ${PROTOBUF_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    glog::glog
    gflags::gflags
    ZLIB::ZLIB
  )
else()
  target_link_libraries(txSimTraffic
    txProtoBuf
    txTrafficFramework
    txSceneLoader
    ${MAPSDK_LIBRARYS}
    ${FRAMEWORK_LIBRARY_SERVICE}
    tbb
    protobuf
    glog
    gflags
    boost_regex
    boost_filesystem
    boost_system
    boost_thread
    dl
    pthread
    rt
  )
endif()

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
