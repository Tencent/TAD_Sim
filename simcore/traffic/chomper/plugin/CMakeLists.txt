cmake_minimum_required(VERSION 3.1...3.20)

project(txTraffic LANGUAGES CXX)

set(LIBRARY_OUTPUT_PATH ../../${CMAKE_BINARY_DIR}/lib/)
set(EXECUTABLE_OUTPUT_PATH ../../${CMAKE_BINARY_DIR}/lib/)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../cmake/)
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

add_definitions("-DON_LOCAL=1")
add_definitions("-DInjectMode=1")
add_definitions("-DTBB_NEW_API=1") # TBB >= v2021.5

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
  set(VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../version/version.h)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../version/version.h.in"
    "${VERSION_FILE}"
    @ONLY)
endfunction()

modify_project_version()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/protobuf
  "./include"
  "../../common/include"
  "../../framework/include"
  "../../io/include"
  "../../3rd"
  "../../app"
  "../../app/include"
  "../../app/manual_vehicle"
  "../../logsim"
  "../../log2world"
  "../version"
  "../../element_impl")

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
  find_package(TBB)
  find_package(Protobuf REQUIRED)
endif()

if(WIN32)
  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
  )
else()
  include_directories(
    ../${EIGEN3_INCLUDE_DIR}
    ${TBB_INCLUDE_DIRS})
  add_definitions(${TBB_DEFINITIONS})
endif()

if(WIN32)
  link_directories(

    # sim
    ../${CMAKE_BINARY_DIR}/lib
  )
else()
  link_directories(
    ../${CMAKE_BINARY_DIR}/lib
  )
endif()

SET(project_SOURCES
  "../../app/src/worldsim_sim_template.cpp"
  "../../app/src/passive_sim_template.cpp"
  "../../app/src/logsim_sim_template.cpp"
  "../../app/src/tadsim_application.cpp"
  "../../app/src/tadsim_loop.cpp"
  "../../app/src/tx_tadsim_flags.cpp"
  "../../app/src/tx_event_dispatcher.cpp"
  "../../app/src/tx_way_points.cpp"
  "../../app/src/tx_route_path_manager.cpp"
  "../../app/src/tx_event.cpp"
  "../../app/src/tad_component.cpp"
  "../../app/src/tad_vehicle_behavior_configure_utils.cpp"
  "../../app/src/tad_assembler_context.cpp"
  "../../app/src/tad_stand_alone_traffic_element_system.cpp"
  "../../app/src/tad_ego_vehicle_element.cpp"
  "../../app/src/tad_signal_element.cpp"
  "../../app/src/tad_signal_phase_period.cpp"
  "../../app/src/tad_vehicle_kinetics_info.cpp"
  "../../app/src/tad_linear_element_manager.cpp"
  "../../app/src/tad_pedestrian_element.cpp"
  "../../app/src/tad_pedestrian_motion_event.cpp"
  "../../app/src/tad_obstacle_element.cpp"
  "../../app/src/tad_map_obstacle_element.cpp"
  "../../app/src/tad_user_defined_vehicle_element.cpp"
  "../../app/src/tad_ai_vehicle_element.cpp"
  "../../app/src/tad_behavior_time_manager.cpp"
  "../../app/src/tad_acc_event.cpp"
  "../../app/src/tad_vehicle_merge_event.cpp"
  "../../app/src/tad_velocity_event.cpp"
  "../../app/src/tad_element_generator.cpp"
  "../../app/src/tad_route_group_agent.cpp"
  "../../app/src/tad_scene_event_1_0_0_0.cpp"
  "../../app/src/tad_event_dispatcher_1_0_0_0.cpp"
  "../../app/src/tad_traj_follow_vehicle_element.cpp"
  "../../app/src/tad_planning.cpp"
  "../../app/src/tad_simrec_loader.cpp"
  "../../app/src/tad_scene_loader_factory.cpp"
  "../../app/src/tad_relative_trajectory_follow_element.cpp"

  "../../logsim/interpolate_player_module.cc"

  "../../log2world/log2world_template.cpp"
  "../../log2world/log2world_system.cpp"
  "../../log2world/log2world_assembler_context.cpp"
  "../../log2world/log2world_traj_manager.cpp"
  "../../log2world/l2w_vehicle_element.cpp"
  "../../log2world/l2w_linear_element_manager.cpp"

  "../../app/manual_vehicle/tad_manual_vehicle_template.cpp"
  "../../app/manual_vehicle/tad_manual_vehicle_system.cpp"
  "../../app/manual_vehicle/tad_manual_assembler_context.cpp"
  "../../app/manual_vehicle/tad_manual_user_defined_vehicle_element.cpp"
  "../../app/manual_vehicle/tad_manual_ai_vehicle_element.cpp"

  "./src/tad_chomper_interface.cpp"
)

SET(project_HEADERS
  "../../app/include/tadsim_application.h"
  "../../app/include/tadsim_loop.h"
  "../../app/include/worldsim_sim_template.h"
  "../../app/include/passive_sim_template.h"
  "../../app/include/logsim_sim_template.h"
  "../../app/include/tx_tadsim_flags.h"
  "../../app/include/tx_event_dispatcher.h"
  "../../app/include/tx_event.h"
  "../../app/include/tx_event_handler.h"
  "../../app/include/tad_event_trigger_param.h"
  "../../app/include/tx_way_points.h"
  "../../app/include/tx_route_path_manager.h"
  "../../app/include/tad_component.h"
  "../../app/include/tad_vehicle_behavior_configure_utils.h"
  "../../app/include/tad_stand_alone_traffic_element_system.h"
  "../../app/include/tad_ego_vehicle_element.h"
  "../../app/include/tad_signal_element.h"
  "../../app/include/tad_signal_phase_period.h"
  "../../app/include/tad_vehicle_kinetics_info.h"
  "../../app/include/tad_assembler_context.h"
  "../../app/include/tad_linear_element_manager.h"
  "../../app/include/tad_pedestrian_element.h"
  "../../app/include/tad_pedestrian_trajectory.h"
  "../../app/include/tad_pedestrian_motion_event.h"
  "../../app/include/tad_obstacle_element.h"
  "../../app/include/tad_map_obstacle_element.h"
  "../../app/include/tad_user_defined_vehicle_element.h"
  "../../app/include/tad_ai_vehicle_element.h"
  "../../app/include/tad_fsm_user_defined.h"
  "../../app/include/tad_fsm_ai.h"
  "../../app/include/tad_behavior_time_manager.h"
  "../../app/include/tad_acc_event.h"
  "../../app/include/tad_vehicle_merge_event.h"
  "../../app/include/tad_velocity_event.h"
  "../../app/include/tad_vehicle_scan_region.h"
  "../../app/include/tad_vehicle_nearest_object_helper.h"
  "../../app/include/tad_element_generator.h"
  "../../app/include/tad_route_group_agent.h"
  "../../app/include/tad_scene_event_1_0_0_0.h"
  "../../app/include/tad_event_dispatcher_1_0_0_0.h"
  "../../app/include/tad_traj_follow_vehicle_element.h"
  "../../app/include/tad_planning.h"
  "../../app/include/tad_simrec_loader.h"
  "../../app/include/tad_scene_loader_factory.h"
  "../../app/include/tad_relative_trajectory_follow_element.h"

  "../../logsim/interpolate_player_module.h"

  "../../log2world/log2world_template.h"
  "../../log2world/log2world_system.h"
  "../../log2world/log2world_assembler_context.h"
  "../../log2world/log2world_traj_manager.h"
  "../../log2world/l2w_vehicle_element.h"
  "../../log2world/l2w_linear_element_manager.h"

  "../../app/manual_vehicle/tad_manual_vehicle_template.h"
  "../../app/manual_vehicle/tad_manual_vehicle_system.h"
  "../../app/manual_vehicle/tad_manual_handler.h"
  "../../app/manual_vehicle/tad_manual_assembler_context.h"
  "../../app/manual_vehicle/tad_manual_user_defined_vehicle_element.h"
  "../../app/manual_vehicle/tad_manual_ai_vehicle_element.h"

  "./include/tad_chomper_interface.h"
)

file(GLOB ElementWrap_SOURCES ../../element_impl/*.cpp)
file(GLOB ElementWrap_HEADERS ../../element_impl/*.h)
message(STATUS "ElementWrap_SOURCES = " ${ElementWrap_SOURCES})
message(STATUS "ElementWrap_HEADERS = " ${ElementWrap_HEADERS})

if(GATrainingMode)
  add_library(txSimTrafficPlugin SHARED ${project_SOURCES} ${project_HEADERS} ${ElementWrap_SOURCES} ${ElementWrap_HEADERS})

  target_compile_definitions(txSimTrafficPlugin PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

  if(WIN32)
  else()
    target_compile_options(txSimTrafficPlugin PRIVATE -fPIC)
  endif()

  if(WIN32 OR WithDependency)
    message(STATUS "app add_dependencies")
    add_dependencies(txSimTrafficPlugin TrafficFramework)
  endif()

  if(WIN32)
    target_link_libraries(txSimTrafficPlugin
      txProtoBuf.lib
      SceneLoader.lib
      TrafficFramework.lib
      ${MAPSDK_LIBRARYS}
      ${FRAMEWORK_LIBRARY_SERVICE}
      TBB::tbb
      ${Boost_FILESYSTEM_LIBRARY}
      ${Boost_SYSTEM_LIBRARY}
      glog::glog
      gflags::gflags
      ZLIB::ZLIB
      CURL::libcurl
      shlwapi
      Advapi32.lib
      Rpcrt4.lib
      Ws2_32.lib
      Iphlpapi.lib
      ${PROTOBUF_LIBRARIES}
    )
  else()
    target_link_libraries(txSimTrafficPlugin
      txProtoBuf
      TrafficFramework
      SceneLoader
      ${MAPSDK_LIBRARYS}
      ${FRAMEWORK_LIBRARY_SERVICE}
      tbb
      protobuf
      spatialite
      sqlite3
      curl
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
endif(GATrainingMode)

# if(WIN32)
# if(NOT CMAKE_BUILD_TYPE)
# copy_mapsdk_deps(${EXECUTABLE_OUTPUT_PATH}/Release)
# copy_framework_deps(${EXECUTABLE_OUTPUT_PATH}/Release)
# else()
# copy_mapsdk_deps(${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
# copy_framework_deps(${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
# endif()
# endif()
