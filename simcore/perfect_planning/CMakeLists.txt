cmake_minimum_required(VERSION 3.1...3.20)

project(txPlanning LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../build/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../build/bin)
set(ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../build/lib)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake/)

add_definitions("-DON_LOCAL=1")
add_definitions("-DGLOG_USE_GLOG_EXPORT=1")
add_definitions("-DInjectMode=1")
add_definitions("-DTBB_NEW_API=1") # TBB >= v2021.5
add_definitions("-DGLOG_NO_ABBREVIATED_SEVERITIES")

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
  else(EXISTS "${PROJECT_STORE_PATH}/.git/")
    if(EXISTS "${PROJECT_STORE_PATH}/.svn/")
      FIND_PACKAGE(Subversion)

      if(SUBVERSION_FOUND)
        Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} Project)
        SET(PROJECT_REVISION ${Project_WC_REVISION})
        message("-- Svn revision:${PROJECT_REVISION}")
      else(SUBVERSION_FOUND)
        message("-- Can't find packet Subversion")
      endif(SUBVERSION_FOUND)
    else()
      message(ERROR "-- Svn directory not exists")
    endif(EXISTS "${PROJECT_STORE_PATH}/.svn/")
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
  "../traffic/common/include"
  "../traffic/framework/include"
  "../traffic/io/include"
  "../traffic/3rd"
  "../traffic/3rd/xml"
  "../traffic/app/include"
  "./version"
)

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

if(WIN32)
  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
    ../traffic/build/protobuf
  )
else()
  include_directories(
    ../traffic/build/protobuf
    ../../common/message/build
    ${EIGEN3_INCLUDE_DIR}
    ${TBB_INCLUDE_DIRS})
  add_definitions(${TBB_DEFINITIONS})
endif()

if(WIN32)
  link_directories(
    ../traffic/build/lib/Release
  )
else()
  link_directories(
    ../traffic/build/bin
    ../traffic/build/lib
  )
endif()

SET(project_SOURCES
  "./tx_sim_ego_template.cpp"
  "./tx_planning_loop.cpp"
  "./tx_sim_planning.cpp"
  "./tx_planning_application.cpp"
  "./tx_planning_flags.cpp"
  "./tx_planning_scene_loader.cpp"
  "./tx_planning_scene_event_loader.cpp"
  "./tx_planning_traffic_element_system.cpp"
  "./tx_planning_vehicle_element.cpp"
  "./tx_trajectory_planning_vehicle_element.cpp"
  "./tx_planning_nearest_object.cpp"
  "./tx_planning_env_vehicle_info.cpp"
  "./tx_planning_vehicle_element_trajectory_follow.cpp"
  "./tx_planning_scene_event_module.cpp"
  "./tx_dummy_event_dispatcher_1_0_0_0.cpp"
  "./tx_dummy_drive_control.cpp"
  "../traffic/app/src/tad_vehicle_behavior_configure_utils.cpp"
  "../traffic/app/src/tad_component.cpp"
  "../traffic/app/src/tx_way_points.cpp"
  "../traffic/app/src/tx_route_path_manager.cpp"
  "../traffic/app/src/tx_tadsim_flags.cpp"
  "../traffic/app/src/tad_planning.cpp"
  "../traffic/app/src/tad_scene_event_1_0_0_0.cpp"
)
SET(project_HEADERS
  "./tx_sim_ego_template.h"
  "./tx_planning_loop.h"
  "./tx_planning_application.h"
  "./tx_planning_flags.h"
  "./tx_planning_scene_loader.h"
  "./tx_planning_scene_event_loader.h"
  "./tx_planning_traffic_element_system.h"
  "./tx_planning_vehicle_element.h"
  "./tx_trajectory_planning_vehicle_element.h"
  "./tx_planning_vehicle_element_trajectory_follow.h"
  "./tx_planning_nearest_object.h"
  "./tx_planning_env_vehicle_info.h"
  "./tx_planning_scene_event_module.h"
  "./tx_dummy_event_dispatcher_1_0_0_0.h"
  "./tx_dummy_drive_control.h"
)

add_executable(txPlanning ${project_SOURCES} ${project_HEADERS})

add_library(tx_perfect_planning SHARED ${project_SOURCES} ${project_HEADERS})

target_compile_definitions(txPlanning PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)
target_compile_definitions(tx_perfect_planning PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE)

if(WIN32)
else()
  target_compile_options(txPlanning PRIVATE -fPIC)
  target_compile_options(tx_perfect_planning PRIVATE -fPIC)
endif()

if(WIN32)
  target_link_libraries(txPlanning
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

  target_link_libraries(tx_perfect_planning
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
  target_link_libraries(txPlanning
    txProtoBuf
    txTrafficFramework
    txSceneLoader
    ${MAPSDK_LIBRARYS}
    ${FRAMEWORK_LIBRARY_SERVICE}
    tbb
    protobuf
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

  target_link_libraries(tx_perfect_planning
    ${MAPSDK_LIBRARYS}
    ${FRAMEWORK_LIBRARY_SERVICE}
    txProtoBuf
    txTrafficFramework
    txSceneLoader
    tbb
    protobuf
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

# add_subdirectory(dummy_driver)
if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${EXECUTABLE_OUTPUT_PATH}/Release)
    copy_framework_deps(${EXECUTABLE_OUTPUT_PATH}/Release)
  else()
    copy_mapsdk_deps(${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
    copy_framework_deps(${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
  endif()
endif()
