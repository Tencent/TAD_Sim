cmake_minimum_required(VERSION 3.13.0)

project(txSimGrading)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules)
set(root_dir ${CMAKE_SOURCE_DIR} CACHE INTERNAL "eval algorithm")
set(pblogger ${root_dir}/../protobuf_log/src CACHE INTERNAL "eval algorithm")

set(pblogger_name pblogger CACHE INTERNAL "pblogger_name")
set(sim_msg sim_msg CACHE INTERNAL "sim_msg")

set(stateflow_name stateflow CACHE INTERNAL "stateflow")
set(actors_name eval_actors CACHE INTERNAL "actors")
set(manager_name manager CACHE INTERNAL "manager")
set(factory_name factory CACHE INTERNAL "factory")
set(utils_name utils CACHE INTERNAL "utils")

set(eval_control eval_control CACHE INTERNAL "eval algorithm")
set(eval_custom eval_custom CACHE INTERNAL "eval algorithm")
set(eval_general eval_general CACHE INTERNAL "eval algorithm")
set(eval_meta eval_meta CACHE INTERNAL "eval algorithm")
set(eval_perception eval_perception CACHE INTERNAL "eval algorithm")
set(eval_prediction eval_prediction CACHE INTERNAL "eval algorithm")
set(eval_regulation eval_regulation CACHE INTERNAL "eval algorithm")
set(eval_planning eval_planning CACHE INTERNAL "eval algorithm")
set(eval_parking eval_parking CACHE INTERNAL "eval algorithm")
set(eval_base eval_base CACHE INTERNAL "eval algorithm")
set(eval_algorithms ${eval_control} ${eval_custom} ${eval_general} ${eval_meta}
  ${eval_perception} ${eval_prediction} ${eval_regulation} ${eval_base} ${eval_planning} ${eval_parking})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(map_sdk REQUIRED)
find_package(framework REQUIRED)

set(inc_dir ${pblogger}
  ${root_dir}
  ${root_dir}/src
  ${root_dir}/../../common/message/build
  ${root_dir}/../../common/message
  CACHE INTERNAL "include dir")

include_directories(${inc_dir})

if(WIN32)
  add_compile_options("/MP")
  add_compile_options(/utf-8)

  if(USE_STATIC_LIBS)
    message(STATUS "Configuring for static libraries...")
    add_compile_options(
      "$<$<CONFIG:Debug>:/MTd>" # Debug mode
      "$<$<CONFIG:Release>:/MT>" # Release mode
    )
  else()
    message(STATUS "Configuring for shared libraries...")
  endif()

  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DGLOG_USE_GLOG_EXPORT)

  # add_definitions(-DGLOG_USE_GLOG_NO_EXPORT)
  find_package(protobuf CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(gflags CONFIG REQUIRED)
  find_package(glog CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS filesystem regex)
  find_package(pugixml CONFIG REQUIRED)
  find_package(jsoncpp CONFIG REQUIRED)
  find_package(GTest CONFIG REQUIRED)
  find_package(absl REQUIRED)

  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
  )

  link_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib
  )

  set(lib_protobuf protobuf::libprotobuf ZLIB::ZLIB)
  set(lib_sys shlwapi)
  set(lib_glog glog::glog gflags::gflags)
  set(lib_boost Boost::filesystem Boost::system)
  set(lib_xml pugixml::pugixml)
  set(lib_json JsonCpp::JsonCpp)
  set(lib_gtest GTest::gtest GTest::gtest_main)
  set(lib_absl absl::base absl::strings)
else()
  add_compile_options(-Wunused)
  add_compile_options(-Wreturn-type)
  add_compile_options(-fPIC)

  set(lib_protobuf protobuf)
  set(lib_sys pthread)
  set(lib_glog glog gflags)
  set(lib_boost boost_filesystem boost_system)
  set(lib_xml pugixml)
  set(lib_json jsoncpp)
  set(lib_gtest gtest gtest_main)
  set(lib_absl)
endif(WIN32)

aux_source_directory(${root_dir}/src src_files)
set(pblogger_srcs ${pblogger}/txsim_log_common.cpp ${pblogger}/txsim_logger.cpp ${pblogger}/txsim_log_queue.cpp)

add_subdirectory(${root_dir}/eval)
add_subdirectory(${root_dir}/eval/control)
add_subdirectory(${root_dir}/eval/custom)
add_subdirectory(${root_dir}/eval/general)
add_subdirectory(${root_dir}/eval/planning)
add_subdirectory(${root_dir}/eval/prediction)
add_subdirectory(${root_dir}/eval/regulations)
add_subdirectory(${root_dir}/eval/perception)
add_subdirectory(${root_dir}/eval/parking)
add_subdirectory(${root_dir}/eval/meta)

add_subdirectory(${root_dir}/stateflow)
add_subdirectory(${root_dir}/actors)
add_subdirectory(${root_dir}/factory)
add_subdirectory(${root_dir}/manager)
add_subdirectory(${root_dir}/utils)

add_library(${pblogger_name} STATIC ${pblogger_srcs})
file(GLOB osi_srcs ${root_dir}/../../common/message/build/osi_*.pb.cc)
add_library(${sim_msg} STATIC ${root_dir}/../../common/message/build/environment.pb.cc
  ${root_dir}/../../common/message/build/grading.pb.cc
  ${root_dir}/../../common/message/build/grading_kpi.pb.cc
  ${root_dir}/../../common/message/build/location.pb.cc
  ${root_dir}/../../common/message/build/trajectory.pb.cc
  ${root_dir}/../../common/message/build/basic.pb.cc
  ${root_dir}/../../common/message/build/header.pb.cc
  ${root_dir}/../../common/message/build/control.pb.cc
  ${root_dir}/../../common/message/build/trajectoryFollowing.pb.cc
  ${root_dir}/../../common/message/build/traffic.pb.cc
  ${root_dir}/../../common/message/build/carSystemState.pb.cc
  ${root_dir}/../../common/message/build/vehicle_geometry.pb.cc
  ${root_dir}/../../common/message/build/grading_report.pb.cc
  ${root_dir}/../../common/message/build/perception_obstacles.pb.cc
  ${root_dir}/../../common/message/build/planStatus.pb.cc
  ${root_dir}/../../common/message/build/parking_state.pb.cc
  ${root_dir}/../../common/message/build/parking_space.pb.cc
  ${root_dir}/../../common/message/build/vehState.pb.cc
  ${root_dir}/../../common/message/build/l4_signal.pb.cc
  ${root_dir}/../../common/message/build/scene.pb.cc
  ${root_dir}/../../common/message/build/settings.pb.cc
  ${root_dir}/../../common/message/build/waypoint.pb.cc
  ${root_dir}/../../common/message/build/environment.pb.cc
  ${root_dir}/../../common/message/build/vehicle_interaction.pb.cc
  ${root_dir}/../../common/message/build/scene_sensor_group.pb.cc
  ${root_dir}/../../common/message/build/scene_common.pb.cc
  ${root_dir}/../../common/message/build/scene_infrastructure_group.pb.cc
  ${root_dir}/../../common/message/build/control_v2.pb.cc
  ${root_dir}/../../common/message/build/union.pb.cc
  ${osi_srcs})
target_link_libraries(${sim_msg} ${lib_absl})

add_library(eval_no_check_kpis STATIC
  ${root_dir}/factory/eval_factory_no_check.cpp
  ${root_dir}/factory/eval_factory_interface.cpp
  ${root_dir}/factory/eval_factory.cpp
  ${root_dir}/eval/general/eval_milleage.cpp
  ${root_dir}/eval/meta/eval_meta.cpp
  ${root_dir}/eval/general/eval_acc.cpp
  ${root_dir}/eval/general/eval_collision.cpp
  ${root_dir}/eval/general/eval_lateral_acc.cpp
  ${root_dir}/eval/general/eval_speed.cpp
  ${root_dir}/eval/planning/eval_thw.cpp
  ${root_dir}/eval/planning/eval_ttc.cpp
)
target_link_libraries(eval_no_check_kpis ${lib_absl})

# build elf
# add_executable(txSimGrading ${src_files})
add_library(txSimGrading SHARED ${src_files})

if(WIN32)
  string(CONCAT wholeArchiveCmd "/WHOLEARCHIVE:${stateflow_name} /WHOLEARCHIVE:${utils_name}
	/WHOLEARCHIVE:${pblogger_name} /WHOLEARCHIVE:${actors_name} /WHOLEARCHIVE:${manager_name} /WHOLEARCHIVE:${factory_name} /WHOLEARCHIVE:${sim_msg} " "")

  list(LENGTH eval_algorithms size_alg)
  math(EXPR length_alg "${size_alg} - 1")

  foreach(val RANGE ${length_alg})
    list(GET eval_algorithms ${val} eval_alg)
    string(CONCAT wholeArchiveCmd ${wholeArchiveCmd} "/WHOLEARCHIVE:${eval_alg} ")
  endforeach()

  message(STATUS " ${wholeArchiveCmd} ")

  target_link_libraries(txSimGrading ${eval_algorithms} ${factory_name} ${stateflow_name} ${utils_name}
    ${pblogger_name} ${sim_msg} ${actors_name} ${manager_name} ${MAPSDK_LIBRARYS}
    ${lib_sys} ${lib_protobuf} ${lib_glog} ${lib_xml} ${lib_boost} ${lib_json} ${lib_absl})
  set_target_properties(txSimGrading PROPERTIES LINK_FLAGS ${wholeArchiveCmd})

else()
  target_link_libraries(txSimGrading -Wl,--whole-archive ${eval_algorithms} ${stateflow_name} ${utils_name}
    ${pblogger_name} ${sim_msg} ${actors_name} ${manager_name} ${factory_name} -Wl,--no-whole-archive
    ${MAPSDK_LIBRARYS} ${lib_sys} ${lib_protobuf} ${lib_glog} ${lib_xml} ${lib_boost} ${lib_json})
endif(WIN32)

# build test
if(WIN32)
  include_directories(${root_dir}/unittest)
  aux_source_directory(${root_dir}/unittest unittest_srcs)
  add_executable(grading_test ${unittest_srcs})
  target_link_libraries(grading_test ${lib_gtest} txSimGrading)
  include(GoogleTest)
  gtest_discover_tests(grading_test)
endif(WIN32)

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
