cmake_minimum_required(VERSION 3.1)
project(scene_wrapper)
set(EXECUTABLE_OUTPUT_PATH      ../../../build/bin)
set(LIBRARY_OUTPUT_PATH         ../../../build/bin)
set(ARCHIVE_OUTPUT_DIRECTORY    ../../../build/lib)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-warning")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819") # 禁用特定于 MSVC 的警告
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

#if(POLICY CMP0135)
#  cmake_policy(SET CMP0135 NEW)
#endif()

#if(EXISTS ${CMAKE_SOURCE_DIR}/.clang-format)
#  execute_process(COMMAND cp ${CMAKE_SOURCE_DIR}/.clang-format ${CMAKE_BINARY_DIR}/.clang-format)
#endif()

set(UPDATOR_PROGRAM updator-scenario)
set(XOSC_CONVERTER xosc-converter)
#set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build: Debug Release RelWithDebInfo MinSizeRel")
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
set(CMAKE_CXX_STANDARD 14 CACHE INTERNAL "-std=c++xx" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options($<$<BOOL:${UNIX}>:-fPIC>)
# add_compile_options($<$<BOOL:${MSVC}>:/wd"4828">)
add_compile_options(-DRANDOM_WITHOUT_BOOST)
add_compile_options(-DSCENE_EVENTS_UPGRAD)
add_compile_options(-DXOSC2SIMREC_GEN_SIM_WITH_XOSC_READER)
add_link_options($<$<BOOL:${UNIX}>:-ldl>)
add_link_options($<$<BOOL:${UNIX}>:-lpthread>)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. CACHE PATH "root dir")
set(COMMON_DIR ${ROOT_DIR}/common CACHE PATH "common dir" FORCE)
set(THIRD_PARTY_DIR ${ROOT_DIR}/third_party CACHE PATH "third party dir" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${THIRD_PARTY_DIR} ${COMMON_DIR} ${ROOT_DIR}/../../cmake)
find_package(map_sdk REQUIRED)
include(common)
include(third_party)
list(REMOVE_ITEM COMMON_SRCS ${COMMON_DIR}/utils/xsd_validator.h ${COMMON_DIR}/utils/xsd_validator.cc)
list(REMOVE_ITEM COMMON_TEST ${COMMON_DIR}/utils/xsd_validator_test.cc)


include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/helper_funcs.cmake)
protobuf_generate_cpps(PROTO_SRC PROTO_HDR
  ../../../../common/message/basic.proto
  ../../../../common/message/location.proto
  ../../../../common/message/trajectory.proto
  ../../../../common/message/traffic.proto
  ../../../../common/message/grading.proto
  ../../../../common/message/scene.proto
  ../../../../common/message/settings.proto
  ../../../../common/message/waypoint.proto
  ../../../../common/message/environment.proto
  ../../../../common/message/header.proto
  ../../../../common/message/scene_common.proto
  ../../../../common/message/vehicle_interaction.proto
  ../../../../common/message/vehicle_geometry.proto
  ../../../../common/message/scene_sensor_group.proto
  ../../../../common/message/scene_infrastructure_group.proto)
include_directories(${CMAKE_BINARY_DIR})


if(WIN32)
  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/empty
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/xsd2cpp-master/src/app
  )
  link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/bin
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin
  )

  add_definitions(
    -DSCENE_WRAPPER_EXPORTS
    -D__HADMAP_SERVER__
    -DTIXML_USE_STL
    -DPROJ_NEW_API
  )
  set(CMAKE_CXX_FLAGS "/utf-8 ${CMAKE_CXX_FLAGS}")
else()
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/empty
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/xsd2cpp-master/src/app
    /usr/local/include/soci/
    /usr/local/include/soci/sqlite3
  )
  link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/bin
  )

  add_definitions(-DSCENE_WRAPPER_EXPORTS -D__HADMAP_SERVER__ -DTIXML_USE_STL
    -DSOCI_DLL
  )
endif()



file(GLOB_RECURSE SRC_FILES
  ./src/database/*.cpp
  ./src/database/*.h
  ./src/database/*.inl
  ./src/engine/*.cpp
  ./src/engine/*.h
  ./src/engine/*.inl
  ./src/map_data/*.cpp
  ./src/map_data/*.h
  ./src/map_data/*.inl
  ./src/server_hadmap/*.cpp
  ./src/server_hadmap/*.h
  ./src/server_hadmap/*.inl
  ./src/server_scene/*.cpp
  ./src/server_scene/*.h
  ./src/server_scene/*.inl
  ./src/xml_parser/*.cc
  ./src/xml_parser/*.cpp
  ./src/xml_parser/*.h
  ./src/xml_parser/*.inl
  ./src/server_map_cache/*.cpp
  ./src/server_map_cache/*.h
  ./src/server_map_cache/*.inl
  ../third_party/xsd2cpp-master/src/app/*.h
  ../third_party/xsd2cpp-master/src/app/*.hpp
  ../third_party/xsd2cpp-master/src/app/*.cc
  ${asam_SOURCES}
)

file(GLOB_RECURSE TST_FILES
  ./src/*_test.c*
  ./src/**/*_test.c*
  ./scene_wrapper_linux/*_test.c*
)

list(REMOVE_ITEM SRC_FILES ${TST_FILES})

add_library(map_parser STATIC ${COMMON_SRCS} ${SRC_FILES} ${PROTO_SRC})
target_link_libraries(map_parser
  ${THIRD_PARTY_LIBS}
  ${MAPSDK_LIBRARYS}
)
# ============WRAPPER=========
if(WIN32)
  file(GLOB SCENE_WRAPPER_SOURCE ./scene_wrapper_windows/*.cpp)
  file(GLOB SCENE_WRAPPER_HEADER ./scene_wrapper_windows/*.h)
else()
  file(GLOB SCENE_WRAPPER_SOURCE ./scene_wrapper_linux/*.cpp)
  file(GLOB SCENE_WRAPPER_HEADER ./scene_wrapper_linux/*.h)
endif()
add_library(${PROJECT_NAME} SHARED  ${SCENE_WRAPPER_SOURCE} ${SCENE_WRAPPER_HEADER})
target_link_libraries(${PROJECT_NAME} map_parser ${MAPSDK_LIBRARYS} ${THIRD_PARTY_LIBS})

# ============UPDATOR=========
file(GLOB_RECURSE UPDATOR_SOURCER ./updator/*.cpp)
file(GLOB_RECURSE UPDATOR_HEADER ./updator/*.h)
add_executable(${UPDATOR_PROGRAM} ${UPDATOR_SOURCER} ${UPDATOR_HEADER})
target_link_libraries(${UPDATOR_PROGRAM} map_parser ${MAPSDK_LIBRARYS} ${THIRD_PARTY_LIBS})

# ============CONVERT=========
# file(GLOB_RECURSE CONVERTER_HEADER ./converter/*.h)
# file(GLOB_RECURSE CONVERTER_SOURCE ./converter/*.cpp)
# add_executable(${XOSC_CONVERTER} ${CONVERTER_SOURCE} ${CONVERTER_HEADER})
# target_link_libraries(${XOSC_CONVERTER} map_parser ${MAPSDK_LIBRARYS} ${THIRD_PARTY_LIBS})

if(UNIX)
  include_directories(src)
endif()

if(HADMAP_SVR_BUILD_TESTING)
  add_subdirectory(test)
endif()

if(HADMAP_SVR_BUILD_TOOLS)
  add_executable(xosc_convert_tool ${ROOT_DIR}/tools/xosc_convert_tool.cc)
  target_link_libraries(xosc_convert_tool map_parser ${MAPSDK_LIBRARYS} ${THIRD_PARTY_LIBS})
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/service CACHE PATH "Install path prefix" FORCE)
endif()

# if(WIN32)
#   install(TARGETS ${PROJECT_NAME} xosc_convert_tool
# 	RUNTIME DESTINATION .
# )
# endif()

if(WIN32)
if(NOT CMAKE_BUILD_TYPE)
copy_mapsdk_deps(${LIBRARY_OUTPUT_PATH}/Release)
else()
copy_mapsdk_deps(${LIBRARY_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
endif()
endif()
