cmake_minimum_required(VERSION 3.1)
project(hdserver)

# ---------------------------------------------------------------------------------------
# 设置CMAKE相关，私有
# ---------------------------------------------------------------------------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# ---------------------------------------------------------------------------------------
# 设置CPP编译选项，私有
# ---------------------------------------------------------------------------------------
add_compile_options(-D__HADMAP_SERVER__)
add_compile_options(-D__CLOUD_HADMAP_SERVER__)
add_compile_options(-DSCENE_WRAPPER_EXPORTS)

# add_compile_options(-DSCENE_EVENTS_UPGRAD)

# ---------------------------------------------------------------------------------------
# 变量，私有
# ---------------------------------------------------------------------------------------
set(SERVER ${PROJECT_NAME})
set(SERVER_WRAPPER scene_wrapper)

# ---------------------------------------------------------------------------------------
# 依赖，私有
# ---------------------------------------------------------------------------------------
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.. CACHE PATH "root dir")
set(THIRD_PARTY_DIR ${ROOT_DIR}/third_party CACHE PATH "third party dir" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${THIRD_PARTY_DIR} ${ROOT_DIR}/../../cmake)
find_package(map_sdk REQUIRED)
include(third_party)

include(${ROOT_DIR}/../../cmake/helper_funcs.cmake)
protobuf_generate_cpps(PROTO_SRC PROTO_HDR
    ${ROOT_DIR}/../../../common/message/basic.proto
    ${ROOT_DIR}/../../../common/message/location.proto
    ${ROOT_DIR}/../../../common/message/trajectory.proto
    ${ROOT_DIR}/../../../common/message/traffic.proto
    ${ROOT_DIR}/../../../common/message/grading.proto
    ${ROOT_DIR}/../../../common/message/scene.proto
    ${ROOT_DIR}/../../../common/message/settings.proto
    ${ROOT_DIR}/../../../common/message/waypoint.proto
    ${ROOT_DIR}/../../../common/message/environment.proto
    ${ROOT_DIR}/../../../common/message/header.proto
    ${ROOT_DIR}/../../../common/message/scene_common.proto
    ${ROOT_DIR}/../../../common/message/vehicle_interaction.proto
    ${ROOT_DIR}/../../../common/message/vehicle_geometry.proto
    ${ROOT_DIR}/../../../common/message/scene_sensor_group.proto
    ${ROOT_DIR}/../../../common/message/scene_infrastructure_group.proto)
include_directories(${CMAKE_BINARY_DIR})

# ---------------------------------------------------------------------------------------
# 获取所有的source files
# ---------------------------------------------------------------------------------------
file(GLOB_RECURSE TST_FILES ./hdserver/src/*_test.c*
    ./hdserver/src/**/*_test.c*
    ./hdserver/scene_wrapper_linux/*_test.c*
)

file(GLOB_RECURSE SRC_FILES ./hdserver/src/*.h
    ./hdserver/src/*.c*
    ./hdserver/src/*.inl
    ./hdserver/scene_wrapper_linux/param_adapter.h)

# filter
file(GLOB_RECURSE DEL_FILES ./hdserver/*test.c*
    ./hdserver/server_map_cache/flags.*
    ./hdserver/server_map_cache/cos_env.*
    ./hdserver/server_hadmap.bak/*
    ./hdserver/server_map_cache.bak/*
    ./hdserver/src/engine/entity/hadmap.*
    ./hdserver/src/gutil/*
    ./hdserver/src/utils/curl_utils.*
    ./hdserver/src/utils/faststring.*
    ./hdserver/src/utils/misc.h
    ./hdserver/src/utils/scoped_cleanup.*
    ./hdserver/src/xml_parser/entity/event_node.*
    ./hdserver/src/xml_parser/entity/environments/*
    ./hdserver/src/xml_parser/entity/merge[._]*
    ./hdserver/src/xml_parser/entity/planner.*
    ./hdserver/src/xml_parser/entity/xosc_writer.*
    ./hdserver/src/xml_parser/entity/xosc_converter.*

    # ./hdserver/src/xml_parser/entity/(pedestrian_event|pedestrian|pedestrianV2|velocity_param|velocity).*
    ./hdserver/src/xml_parser/entity/pedestrian[_.V]*
    ./hdserver/src/xml_parser/entity/velocity[._]*
    ./hdserver/src/log/*
    ./hdserver/src/xml_parser/xosc/xosc_base_1_0.*
    ./hdserver/src/xml_parser/xosc/xosc_converter.*
    ./hdserver/src/xml_parser/xosc/xosc_reader_1_0_v4.*

    # ${COMMON_DIR}/xml_parser/xosc/xosc_reader_1_0_v4.*
    ./hdserver/src/xml_parser/xosc/xosc_writer_1_0_v4.*

    # ${COMMON_DIR}/xml_parser/xosc/xosc_writer_1_0_v4.*
    ./hdserver/src/xml_parser/entity/scene_event.*
    ./hdserver/src/xml_parser/entity/mapfile.*
    ./hdserver/src/map_data/map_data_predef.*
    ./hdserver/src/map_data/map_object.*
    ./hdserver/src/map_data/map_query.*
    ./hdserver/src/xml_parser/entity/traffic.*
    ./hdserver/src/xml_parser/entity/route.*
)

list(REMOVE_ITEM SRC_FILES ${DEL_FILES} ${TST_FILES})
list(REMOVE_ITEM COMMON_SRCS ${DEL_FILES})

# rule
add_library(${SERVER} STATIC
    ${SRC_FILES}
    ${COMMON_SRCS}
    ${asam_SOURCES}
    ${PROTO_SRC}
)
target_include_directories(${SERVER} PUBLIC
    ${ROOT_DIR}
    hdserver
    hdserver/src
    hdserver/src/xml_parser)

find_package(recastnavigation REQUIRED)

find_package(AWSSDK COMPONENTS s3)

if(NOT ${AWSSDK_FOUND})
    find_package(aws-cpp-sdk-core CONFIG REQUIRED)
    find_package(aws-cpp-sdk-s3 CONFIG REQUIRED)
endif()

target_link_libraries(${SERVER}
    ${THIRD_PARTY_LIBS}
    ${MAPSDK_LIBRARYS}
    aws-cpp-sdk-s3
    aws-cpp-sdk-core
    RecastNavigation::DebugUtils
    RecastNavigation::Detour
    RecastNavigation::DetourCrowd
    RecastNavigation::Recast
    OpenSSL::Crypto
    OpenSSL::SSL
)

if(HADMAP_SVR_ENABLE_TCMALLOC)
    target_link_libraries(${SERVER} tcmalloc)
endif()

if(HADMAP_SVR_ENABLE_WRAPPER)
    add_library(${SERVER_WRAPPER} SHARED
        ./hdserver/scene_wrapper_linux/scene_wrapper.h
        ./hdserver/scene_wrapper_linux/scene_wrapper.cpp)
    target_link_libraries(${SERVER_WRAPPER} ${SERVER})
endif()

if(HADMAP_SVR_BUILD_TESTING)
    add_executable(${SERVER}_test ${COMMON_TEST} ${TST_FILES})
    target_link_libraries(${SERVER}_test ${SERVER} gtest gtest_main)
    gtest_discover_tests(${SERVER}_test)
endif()

if(HADMAP_SVR_BUILD_TOOLS)
    add_executable(aws_cos_tool ${ROOT_DIR}/tools/aws_cos_tool.cc)
    target_link_libraries(aws_cos_tool ${AWSSDK_LIBRARIES} ${THIRD_PARTY_LIBS})

    add_executable(xosc_convert_tool ${ROOT_DIR}/tools/xosc_convert_tool.cc)
    target_link_libraries(xosc_convert_tool ${SERVER})
endif()
