cmake_minimum_required(VERSION 3.4.0)

project(txSimGradingExternal)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules)
set(root_dir ${CMAKE_SOURCE_DIR} CACHE INTERNAL "eval algorithm")

set(sim_msg sim_msg CACHE INTERNAL "sim_msg")
set(stateflow_name stateflow CACHE INTERNAL "stateflow")
set(actors_name eval_actors CACHE INTERNAL "actors")
set(manager_name manager CACHE INTERNAL "manager")
set(utils_name utils CACHE INTERNAL "utils")
set(eval_base eval_base CACHE INTERNAL "eval algorithm")

# set(eval_external eval_external CACHE INTERNAL "eval external")
set(eval_no_check eval_no_check_kpis CACHE INTERNAL "eval_no_check")
set(eval_algorithms ${eval_base} ${eval_no_check})

# 1、若新指标有额外的头文件依赖路径，则在此处添加
set(inc_dir
  ${root_dir}/src
  ${root_dir}/include
  ${root_dir}/deps/include/framework
  ${root_dir}/deps/include/message
  ${root_dir}/deps/include
  ${root_dir}/deps/include/map_sdk
  CACHE INTERNAL "include dir")

include_directories(${inc_dir})

if(WIN32)
  add_compile_options("/MP")
  add_compile_options(/utf-8)
  add_compile_options(
    "$<$<CONFIG:Debug>:/MTd>" # Debug mode
    "$<$<CONFIG:Release>:/MT>" # Release mode
  )
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DGLOG_USE_GLOG_EXPORT)

  find_package(protobuf CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(gflags CONFIG REQUIRED)
  find_package(glog CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS filesystem regex)
  find_package(pugixml CONFIG REQUIRED)
  find_package(jsoncpp CONFIG REQUIRED)
  find_package(CURL REQUIRED)
  find_package(PROJ CONFIG REQUIRED)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(absl CONFIG REQUIRED)

  # 2.1、若新指标有额外的静态库依赖路径，windows则在此处添加
  link_directories(
    ${root_dir}/lib/x64
    ${root_dir}/deps/lib/map_sdk/x64
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)

  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
  )

  set(lib_protobuf protobuf::libprotobuf ZLIB::ZLIB absl::base)
  set(lib_sys shlwapi)
  set(lib_glog glog::glog gflags::gflags)
  set(lib_boost Boost::filesystem Boost::system)
  set(lib_xml pugixml::pugixml)
  set(lib_json JsonCpp::JsonCpp)
  set(lib_map_sdk routingmap.lib routeplan.lib mapdb.lib mapengine.lib datamodel.lib mapimport.lib
    transmission.lib CURL::libcurl spatialite.lib unofficial::sqlite3::sqlite3 PROJ::proj)
else()
  add_compile_options(-Wunused)
  add_compile_options(-Wreturn-type)
  add_compile_options(-fPIC)

  # 2.2、若新指标有额外的静态库依赖路径，linux则在此处添加
  link_directories(
    ${root_dir}/lib/x64/Release

    # ${root_dir}/deps/lib/common/x64/Release
    ${root_dir}/deps/lib/map_sdk/x64/Release

    # ${root_dir}/deps/lib/map_sdk/deps/x64/Release
  )

  set(lib_protobuf protobuf)
  set(lib_sys pthread)
  set(lib_glog glog gflags)
  set(lib_boost boost_filesystem boost_system boost_date_time)
  set(lib_map_sdk routingmap routeplan mapengine mapdb transmission mapimport datamodel spatialite sqlite3 curl)
  set(lib_xml pugixml)
  set(lib_json jsoncpp)
endif(WIN32)

aux_source_directory(${root_dir}/src src_files)

# 3、若新指标的源文件.cpp或.cc不在src目录下，则需要以此格式添加
# list(APPEND src_files "${root_dir}/path_to_additional_file/additional_file1.cpp"）

# build elf
add_library(txSimGradingExternal SHARED ${src_files})

if(WIN32)
  string(CONCAT wholeArchiveCmd "/WHOLEARCHIVE:${stateflow_name} /WHOLEARCHIVE:${utils_name}
	/WHOLEARCHIVE:${actors_name} /WHOLEARCHIVE:${manager_name} /WHOLEARCHIVE:${sim_msg} " "")

  list(LENGTH eval_algorithms size_alg)
  math(EXPR length_alg "${size_alg} - 1")

  foreach(val RANGE ${length_alg})
    list(GET eval_algorithms ${val} eval_alg)
    string(CONCAT wholeArchiveCmd ${wholeArchiveCmd} "/WHOLEARCHIVE:${eval_alg} ")
  endforeach()

  message(STATUS " ${wholeArchiveCmd} ")

  # 4.1、若新指标有额外的静态库，windows则在此处添加
  target_link_libraries(txSimGradingExternal ${eval_algorithms} ${stateflow_name} ${utils_name}
    ${sim_msg} ${actors_name} ${manager_name}
    ${lib_sys} ${lib_map_sdk} ${lib_protobuf} ${lib_glog} ${lib_xml} ${lib_boost} ${lib_json})
  set_target_properties(txSimGradingExternal PROPERTIES LINK_FLAGS ${wholeArchiveCmd})

else()
  # 4.2、若新指标有额外的静态库，linux则在此处添加
  target_link_libraries(txSimGradingExternal -Wl,--whole-archive ${eval_algorithms} ${stateflow_name} ${utils_name}
    ${sim_msg} ${actors_name} ${manager_name} -Wl,--no-whole-archive
    ${lib_sys} ${lib_map_sdk} ${lib_protobuf} ${lib_glog} ${lib_xml} ${lib_boost} ${lib_json})
endif(WIN32)
