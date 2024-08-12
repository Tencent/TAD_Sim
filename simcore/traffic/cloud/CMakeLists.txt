cmake_minimum_required(VERSION 3.1...3.20)

project(txTraffic LANGUAGES CXX)

# set(LIBRARY_OUTPUT_PATH ../${CMAKE_BINARY_DIR}/lib/)
# set(EXECUTABLE_OUTPUT_PATH ../${CMAKE_BINARY_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

add_definitions("-DON_CLOUD=1")
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
  "../common/include"
  "../framework/include"
  "../io/include"
  "../3rd"
  "../3rd/xml"
  "../app/include"
  "./version"
)

if(WIN32)
  link_directories(

    # sim
    ${CMAKE_BINARY_DIR}/lib
  )
else()
  link_directories(
    ${CMAKE_BINARY_DIR}/lib
  )
endif()

# protobuf start
if(WIN32)
  set(Protobuf_USE_STATIC_LIBS TRUE)
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

  # boost components
  find_package(Boost REQUIRED system filesystem)
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/protobuf
  "./include"
  "../common/include"
  "../framework/include"
  "../io/include"
  "../3rd"
  "../3rd/xml"
  "../app/include"
  "./version"
)

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

link_directories(${CMAKE_BINARY_DIR}/lib)

SET(project_SOURCES
  "./src/tx_tc_plugin_micro.cpp"
  "./src/tx_tc_init_params.cpp"
  "./src/tx_tc_gflags.cpp"
  "./src/tx_tc_traffic_system.cpp"
  "./src/tx_tc_map_range_utils.cpp"
  "./src/tx_tc_assembler_context.cpp"
  "./src/tx_tc_tad_element_generator.cpp"
  "./src/tx_tc_deadline_area_manager.cpp"
  "./src/tx_tc_tad_vehicle_ai_element.cpp"
  "./src/tx_tc_ditw_tad_vehicle_ai_element.cpp"
  "./src/tx_tc_tad_obstacle_element.cpp"
  "./src/tx_tc_tad_pedestrian_element.cpp"
  "./src/tx_tc_cloud_loader.cpp"
  "./src/tx_tc_cloud_element_manager.cpp"
  "./src/tx_tc_cloud_ditw_trajectory_manager.cpp"
  "./src/tx_tc_cloud_event_json_parser.cpp"
  "./src/tx_tc_inject_event_system.cpp"
  "./src/tx_tc_template.cpp"
  "./src/parallel_simulation/tx_tc_ps_element_generator.cpp"
  "./src/parallel_simulation/tx_tc_ps_traffic_system.cpp"
  "./src/parallel_simulation/tx_tc_ps_assembler_context.cpp"
  "./src/parallel_simulation/tx_tc_ps_traffic_light_element.cpp"
  "./src/parallel_simulation/tx_tc_ps_vehicle_element.cpp"
  "./src/parallel_simulation/tx_tc_ps_vehicle_obstacle_element.cpp"
  "./src/parallel_simulation/tx_tc_ps_traffic_record.cpp"
  "./src/parallel_simulation/tx_tc_ps_od_info.cpp"
  "../app/src/tad_ai_vehicle_element.cpp"
  "../app/src/tad_obstacle_element.cpp"
  "../app/src/tx_event.cpp"
  "../app/src/tad_pedestrian_motion_event.cpp"
  "../app/src/tad_pedestrian_element.cpp"
  "../app/src/tx_tadsim_flags.cpp"
  "../app/src/tad_vehicle_behavior_configure_utils.cpp"
  "../app/src/tad_component.cpp"
  "../app/src/tx_route_path_manager.cpp"
  "../app/src/tx_way_points.cpp"
  "../app/src/tad_ego_vehicle_element.cpp"
  "../app/src/tad_signal_element.cpp"
  "../app/src/tad_signal_phase_period.cpp"
  "../app/src/tad_vehicle_kinetics_info.cpp"
  "../app/src/tad_route_group_agent.cpp"
  "../app/src/tad_planning.cpp"
  "../app/src/tad_scene_event_1_0_0_0.cpp"
  "./src/tx_cloud_stand_alone_event_dispatcher.cpp"
)

SET(project_HEADERS
  "./include/tx_tc_plugin_micro.h"
  "./include/tx_tc_init_params.h"
  "./include/tx_tc_gflags.h"
  "./include/tx_tc_traffic_system.h"
  "./include/tx_tc_map_range_utils.h"
  "./include/tx_tc_assembler_context.h"
  "./include/tx_tc_tad_element_generator.h"
  "./include/tx_tc_element_generator.h"
  "./include/tx_tc_deadline_area_manager.h"
  "./include/tx_tc_probability_generator.h"
  "./include/tx_tc_id_manager.h"
  "./include/tx_tc_tad_vehicle_ai_element.h"
  "./include/tx_tc_ditw_tad_vehicle_ai_element.h"
  "./include/tx_tc_tad_obstacle_element.h"
  "./include/tx_tc_tad_pedestrian_element.h"
  "./include/tx_tc_cloud_loader.h"
  "./include/tx_tc_marco.h"
  "./include/tx_tc_cloud_element_manager.h"
  "./include/tx_tc_cloud_ditw_trajectory_manager.h"
  "./include/tx_tc_cloud_event_json_parser.h"
  "./include/tx_tc_inject_event_system.h"
  "./include/tx_tc_template.h"
  "./include/parallel_simulation/tx_tc_ps_traffic_record.h"
  "./include/parallel_simulation/tx_tc_ps_assembler_context.h"
  "./include/parallel_simulation/tx_tc_ps_element_generator.h"
  "./include/parallel_simulation/tx_tc_ps_traffic_system.h"
  "./include/parallel_simulation/tx_tc_ps_traffic_light_element.h"
  "./include/parallel_simulation/tx_tc_ps_vehicle_element.h"
  "./include/parallel_simulation/tx_tc_ps_vehicle_obstacle_element.h"
  "./include/parallel_simulation/tx_tc_ps_od_info.h"
  "../app/include/tad_ai_vehicle_element.h"
  "../app/include/tad_obstacle_element.h"
  "../app/include/tx_tadsim_flags.h"
  "../app/include/tad_vehicle_behavior_configure_utils.h"
  "../app/include/tad_component.h"
  "../app/include/tx_route_path_manager.h"
  "../app/include/tx_way_points.h"
  "../app/include/tad_ego_vehicle_element.h"
  "../app/include/tx_event.h"
  "../app/include/tad_pedestrian_motion_event.h"
  "../app/include/tad_event_trigger_param.h"
  "../app/include/tad_signal_element.h"
  "../app/include/tad_signal_phase_period.h"
  "../app/include/tad_vehicle_kinetics_info.h"
  "../app/include/tad_route_group_agent.h"
  "../app/include/tad_planning.h"
  "./include/tx_cloud_stand_alone_event_dispatcher.h"
)

if(OnlyCloud)
  add_library(txMicroTrafficCloud SHARED ${project_SOURCES} ${project_HEADERS})

  target_compile_definitions(txMicroTrafficCloud PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

  if(WIN32)
  else(WIN32)
    target_compile_options(txMicroTrafficCloud PRIVATE -fPIC)
  endif(WIN32)

  if(WIN32 OR WithDependency)
    message(STATUS "cloud add_dependencies")
    add_dependencies(txMicroTrafficCloud txTrafficFramework)
  endif(WIN32 OR WithDependency)

  if(WIN32)
    target_link_libraries(txMicroTrafficCloud
      txProtoBuf.lib
      txSceneLoader.lib
      txTrafficFramework.lib
      ${MAPSDK_LIBRARYS}
      ${FRAMEWORK_LIBRARY_SERVICE}
      TBB::tbb
      ${Boost_FILESYSTEM_LIBRARY}
      ${Boost_SYSTEM_LIBRARY}
      glog::glog
      gflags::gflags
      ZLIB::ZLIB
      routeplan.lib
      mapdb.lib
      mapengine.lib
      datamodel.lib
      mapimport.lib
      transmission.lib
      CURL::libcurl
      shlwapi
      Advapi32.lib
      Rpcrt4.lib
      Ws2_32.lib
      Iphlpapi.lib
      ${PROTOBUF_LIBRARIES}
    )
  else(WIN32)
    target_link_libraries(txMicroTrafficCloud
      txProtoBuf
      TrafficFramework
      txSceneLoader
      tbb
      ${PROTOBUF_LIBRARIES}
      boost_regex
      boost_filesystem
      boost_system
      boost_thread
      glog
      gflags
      dl
      pthread
      rt)
  endif(WIN32)
endif(OnlyCloud)

if(CloudStandAlone)
  SET(project_stand_alone_SOURCES
    ${project_SOURCES}
    "./src/tx_cloud_stand_alone.cpp"
    "./src/tx_cloud_stand_alone_application.cpp"
    "./src/tx_cloud_stand_alone_loop.cpp"
    "./src/tx_cloud_stand_alone_template.cpp"
  )

  SET(project_stand_alone_HEADERS
    ${project_HEADERS}
    "./include/tx_cloud_stand_alone_application.h"
    "./include/tx_cloud_stand_alone_loop.h"
    "./include/tx_cloud_stand_alone_template.h")
  add_executable(txCloudStandAlone ${project_stand_alone_SOURCES} ${project_stand_alone_HEADERS})

  target_compile_definitions(txCloudStandAlone PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

  if(WIN32)
  else()
    target_compile_options(txCloudStandAlone PRIVATE -fPIC)
  endif()

  if(WIN32 OR WithDependency)
    message(STATUS "cloud add_dependencies")
    add_dependencies(txCloudStandAlone txTrafficFramework)
  endif()

  if(WIN32)
    target_link_libraries(txCloudStandAlone
      txProtoBuf.lib
      txSceneLoader.lib
      TrafficFramework.lib
      ${MAPSDK_LIBRARYS}
      ${FRAMEWORK_LIBRARY_SERVICE}
      TBB::tbb
      ${Boost_FILESYSTEM_LIBRARY}
      ${Boost_SYSTEM_LIBRARY}
      glog::glog
      gflags::gflags
      ZLIB::ZLIB
      routeplan.lib
      CURL::libcurl
      shlwapi
      Advapi32.lib
      Rpcrt4.lib
      Ws2_32.lib
      Iphlpapi.lib
      ${PROTOBUF_LIBRARIES}
    )
  else()
    target_link_libraries(txCloudStandAlone
      txProtoBuf
      txTrafficFramework
      txSceneLoader
      ${MAPSDK_LIBRARYS}
      ${FRAMEWORK_LIBRARY_SERVICE}
      tbb
      ${PROTOBUF_LIBRARIES}
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
endif(CloudStandAlone)

add_subdirectory(test)

# add_subdirectory(show_version)

# if(WIN32)
# if(NOT CMAKE_BUILD_TYPE)
# copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
# copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
# else()
# copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
# copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
# endif()
# endif()
