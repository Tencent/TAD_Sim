cmake_minimum_required(VERSION 3.1)

project(txSim)

set(CMAKE_CXX_STANDARD 11)

option(TXSIM_WITH_DOC "generate txsim SDK documents." OFF)
option(TXSIM_SDK_ONLY "only build sdk libraries." OFF)
option(TXSIM_WITH_CODE_COVERAGE "compiling with code coverage support. currently only support on linux." OFF)
option(TXSIM_WITH_PERF "compiling the programs for performance tests." OFF)
option(BUILD_NODE_ADDON "" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake/modules/)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/service CACHE PATH "Install path prefix" FORCE)
endif()

set(TXSIM_ENCRYPTION_TIMESTAMP_INT 0 CACHE STRING "enabling soft encryption based on timestamp")

if(TXSIM_ENCRYPTION_TIMESTAMP_INT GREATER 0)
  message("enabling soft encryption based on timestamp: " ${TXSIM_ENCRYPTION_TIMESTAMP_INT})
  add_definitions(-DTXSIM_ENCRYPTION_TIMESTAMP=${TXSIM_ENCRYPTION_TIMESTAMP_INT})
endif()

if(MSVC)
  add_definitions(-DBOOST_ALL_NO_LIB -DZMQ_STATIC -D_WIN32_WINNT=0x601 -DNOMINMAX -DGLOG_NO_ABBREVIATED_SEVERITIES)
endif()

include_directories(
  src
  ${CMAKE_BINARY_DIR})

if(WIN32)
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/eigen3)
  include_directories(${Boost_INCLUDE_DIRS})

  # include_directories(${ABSL_INCLUDE_DIRS})
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})

  # add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)
  add_definitions(-DGLOG_USE_GLOG_EXPORT)
  add_definitions(-DPROJ_NEW_API)
  find_package(Boost REQUIRED COMPONENTS system filesystem program_options regex)
  find_package(GTest CONFIG REQUIRED)
  find_package(protobuf CONFIG REQUIRED)
  find_package(ZeroMQ CONFIG REQUIRED)
  find_package(cppzmq CONFIG REQUIRED)
  find_package(jsoncpp CONFIG REQUIRED)
  find_package(tinyxml2 CONFIG REQUIRED)
  find_package(uriparser 0.9.6 CONFIG REQUIRED char wchar_t)
  find_package(glog CONFIG REQUIRED)
  find_package(gflags CONFIG REQUIRED)
  find_package(PROJ CONFIG REQUIRED)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(absl REQUIRED)
  find_package(gRPC CONFIG REQUIRED)

  # set(CMAKE_SKIP_RPATH TRUE)
  # set(Boost_USE_STATIC_LIBS ON)
  # set(GFLAGS_USE_TARGET_NAMESPACE ON)
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_REGEX_LIBRARY})
  set(LIB_BOOST_PO ${Boost_PROGRAM_OPTIONS_LIBRARY})
  set(LIB_BOOST_DATE_TIME ${Boost_DATE_TIME_LIBRARY})
  set(LIB_GLOG glog::glog gflags::gflags)
  set(LIB_GRPC gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)
  set(LIB_GTEST GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
  set(LIB_JSON JsonCpp::JsonCpp)
  set(LIB_PROJ PROJ::proj)
  set(LIB_PROTOBUF protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  set(LIB_SQLITE unofficial::sqlite3::sqlite3)
  set(LIB_SYS Shlwapi bcrypt) # Shlwapi for PathIsRelative win api and bcrypt for boost uuid library linking.
  set(LIB_XML tinyxml2::tinyxml2)
  set(LIB_ZMQ cppzmq cppzmq-static)
  set(LIB_URIPARSER uriparser::uriparser)
  set(SRC_XML)
  set(LIB_ABSL absl::any absl::log absl::base absl::bits)
elseif(UNIX) # for linux only currently.
  add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)
  find_package(Boost REQUIRED COMPONENTS system filesystem program_options regex)
  find_package(GTest CONFIG REQUIRED)
  find_package(Protobuf CONFIG REQUIRED)
  find_package(ZeroMQ CONFIG REQUIRED)
  find_package(cppzmq CONFIG REQUIRED)
  find_package(gRPC CONFIG REQUIRED)
  find_package(jsoncpp CONFIG REQUIRED)
  find_package(tinyxml2 CONFIG REQUIRED)
  find_package(uriparser 0.9.6 CONFIG REQUIRED char wchar_t)
  find_package(RdKafka CONFIG REQUIRED)
  include_directories(${Boost_INCLUDE_DIRS})
  set(CMAKE_SKIP_RPATH TRUE)
  set(Boost_USE_STATIC_LIBS ON)
  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_REGEX_LIBRARY})
  set(LIB_BOOST_PO ${Boost_PROGRAM_OPTIONS_LIBRARY})
  set(LIB_BOOST_DATE_TIME ${Boost_DATE_TIME_LIBRARY})
  set(LIB_GLOG glog gflags)
  set(LIB_GRPC gRPC::grpc++_reflection gRPC::grpc++_unsecure)
  set(LIB_GTEST GTest::gmock_main)
  set(LIB_JSON jsoncpp_static)
  set(LIB_PROJ libproj.a)
  set(LIB_PROTOBUF protobuf::libprotobuf)
  set(LIB_SQLITE libsqlite3_34_1.a)
  set(LIB_SYS pthread dl rt)
  set(LIB_XML tinyxml2::tinyxml2)
  set(LIB_ZMQ libzmq-static)
  set(LIB_URIPARSER uriparser::uriparser)
  set(SRC_XML)
  set(protobuf_MODULE_COMPATIBLE ON CACHE BOOL "") # for use of protobuf_generate_cpp
  set(LIB_ABSL)
endif(WIN32)

add_library(txsim-module-service
  src/utils/os.h
  src/utils/os.cc
  src/utils/dylib.h
  src/utils/dylib.cc
  src/txsim_module_service.h
  src/module/module_service.cc)
target_compile_options(txsim-module-service PRIVATE -fPIC -fvisibility=hidden)

if(NOT TXSIM_SDK_ONLY)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/helper_funcs.cmake)
  protobuf_generate_cpp(PROTO_SRC_SCENE_COMMON PROTO_HDR_SCENE_COMMON ../../common/message/scene_common.proto)
  protobuf_generate_cpp(PROTO_SRC_SCENE_INFRA_GROUP PROTO_HDR_SCENE_INFRA_GROUP ../../common/message/scene_infrastructure_group.proto)
  protobuf_generate_cpp(PROTO_SRC_WAYPOINT PROTO_HDR_WAYPOINT ../../common/message/waypoint.proto)
  protobuf_generate_cpp(PROTO_SRC_UNION PROTO_HDR_UNION ../../common/message/union.proto)
  protobuf_generate_cpp(PROTO_SRC_BASIC PROTO_HDR_BASIC ../../common/message/basic.proto)
  protobuf_generate_cpp(PROTO_SRC_LOC PROTO_HDR_LOC ../../common/message/location.proto)
  protobuf_generate_cpp(PROTO_SRC_TRAJECTORY PROTO_HDR_TRAJECTORY ../../common/message/trajectory.proto)
  protobuf_generate_cpp(PROTO_SRC_TRAFFIC PROTO_HDR_TRAFFIC ../../common/message/traffic.proto)
  protobuf_generate_cpp(PROTO_SRC_L2W_TRIGGER PROTO_HDR_L2W_TRIGGER ../../common/message/log2world_trigger.proto)
  protobuf_generate_cpp(PROTO_SRC_VEHICLE_GEOMETRY PROTO_HDR_VEHICLE_GEOMETRY ../../common/message/vehicle_geometry.proto)
  protobuf_generate_cpp(PROTO_SRC_VEHICLE_INTERACTION PROTO_HDR_VEHICLE_INTERACTION ../../common/message/vehicle_interaction.proto)
  protobuf_generate_cpp(PROTO_SRC_ENVIRONMENT PROTO_HDR_ENVIRONMENT ../../common/message/environment.proto)
  protobuf_generate_cpp(PROTO_SRC_SENSOR_GROUP PROTO_HDR_SENSOR_GROUP ../../common/message/scene_sensor_group.proto)
  protobuf_generate_cpp(PROTO_SRC_SETTINGS PROTO_HDR_SETTINGS ../../common/message/settings.proto)
  protobuf_generate_cpp(PROTO_SRC_SCENE PROTO_HDR_SCENE ../../common/message/scene.proto)
  protobuf_generate_cpp(PROTO_SRC_HEADER PROTO_HDR_HEADER ../../common/message/header.proto)
  protobuf_generate_cpp(PROTO_SRC_GRADING PROTO_HDR_GRADING ../../common/message/grading.proto)

  set(MODULE_IMPL_SRC
    src/utils/constant.h
    src/utils/leb128.h
    src/utils/msgs.h
    src/utils/msgs.cc
    src/module/messenger_impl.h
    src/module/messenger_impl.cc
    src/module/module_impl.h
    src/module/module_impl.cc
    src/utils/id_impl.h
    src/utils/id_impl.cc
    src/utils/os.h
    src/utils/os.cc
    src/utils/time.h
    src/utils/time.cc)

  add_library(txsim-module-impl SHARED
    ${MODULE_IMPL_SRC})
  target_compile_options(txsim-module-impl PRIVATE -fvisibility=hidden)
  target_link_libraries(txsim-module-impl
    ${LIB_ZMQ} ${LIB_SYS})

  add_executable(txsim-module-launcher
    src/utils/dylib.h
    src/utils/dylib.cc
    src/module/module_launcher.cc
    ${MODULE_IMPL_SRC})
  target_compile_definitions(txsim-module-launcher PRIVATE TXSIM_NO_EXPORTS)
  target_compile_options(txsim-module-launcher PRIVATE -fPIC -fvisibility=hidden)
  target_link_libraries(txsim-module-launcher
    ${LIB_ZMQ} ${LIB_SYS})

  add_library(txsim-coordinator
    src/utils/constant.h
    src/utils/defs.h
    src/utils/id_impl.h
    src/utils/id_impl.cc
    src/utils/leb128.h
    src/utils/msgs.h
    src/utils/msgs.cc
    src/utils/os.h
    src/utils/os.cc
    src/utils/proc.h
    src/utils/proc.cc
    src/utils/time.h
    src/utils/time.cc
    src/utils/proc_service.h
    src/utils/proc_service.cc
    src/coordinator/scenario_parser.h
    src/coordinator/scenario_parser.cc
    src/coordinator/default_xosc_parser.h
    src/coordinator/default_xosc_parser.cc
    src/coordinator/vehicle_catalog_xosc_parser.h
    src/coordinator/vehicle_catalog_xosc_parser.cc
    src/coordinator/sim_parser.h
    src/coordinator/sim_parser.cc
    src/coordinator/xml_util.h
    src/coordinator/xml_util.cc
    src/coordinator/traffic_parser.h
    src/coordinator/traffic_parser.cc
    src/coordinator/sensor_catalog_parser.h
    src/coordinator/sensor_catalog_parser.cc
    src/coordinator/enviroment_parser.h
    src/coordinator/enviroment_parser.cc
    src/coordinator/scene_pb_facade.h
    src/coordinator/scene_pb_facade.cc
    src/coordinator/grading_kpi_writer.h
    src/coordinator/grading_kpi_writer.cc
    ${SRC_XML}
    src/coordinator/status.h
    src/coordinator/status.cc
    src/coordinator/status_writer.h
    src/coordinator/config.h
    src/coordinator/config.cc
    src/coordinator/config_mgr.h
    src/coordinator/config_mgr.cc
    src/coordinator/connection.h
    src/coordinator/connection.cc
    src/coordinator/reaper.h
    src/coordinator/reaper.cc
    src/coordinator/module_mgr.h
    src/coordinator/module_mgr.cc
    src/coordinator/registry.h
    src/coordinator/registry.cc
    src/coordinator/stepper.h
    src/coordinator/stepper.cc
    src/coordinator/shmem_pool.h
    src/coordinator/shmem_pool.cc
    src/coordinator/coordinator.h
    src/coordinator/coordinator.cc
    ${PROTO_SRC_VEHICLE_GEOMETRY}
    ${PROTO_SRC_L2W_TRIGGER}
    ${PROTO_SRC_TRAJECTORY}
    ${PROTO_SRC_TRAFFIC}
    ${PROTO_SRC_BASIC}
    ${PROTO_SRC_SCENE_INFRA_GROUP}
    ${PROTO_SRC_SCENE_COMMON}
    ${PROTO_SRC_WAYPOINT}
    ${PROTO_SRC_UNION}
    ${PROTO_SRC_LOC}
    ${PROTO_SRC_VEHICLE_INTERACTION}
    ${PROTO_SRC_SETTINGS}
    ${PROTO_SRC_SCENE_INFRASTRUCTURE_GROUP}
    ${PROTO_SRC_SCENE}
    ${PROTO_SRC_ENVIRONMENT}
    ${PROTO_SRC_SENSOR_GROUP}
    ${PROTO_SRC_HEADER}
    ${PROTO_SRC_GRADING})

  target_link_libraries(txsim-coordinator ${LIB_PROTOBUF} ${LIB_ZMQ} ${LIB_SQLITE} ${LIB_XML} ${LIB_JSON} ${LIB_GLOG}
    ${LIB_BOOST_FS} ${LIB_BOOST_PO} ${LIB_BOOST_DATE_TIME} ${LIB_PROJ} ${LIB_URIPARSER} ${LIB_SYS} ${LIB_ABSL})

  add_executable(txsim-local-service
    src/service/server_impl/http_json_body.h
    src/service/server_impl/http_json_body.cc
    src/service/server_impl/listener.h
    src/service/server_impl/listener.cc
    src/service/server_impl/http_session.h
    src/service/server_impl/http_session.cc
    src/service/server_impl/ws_session.h
    src/service/server_impl/ws_session.cc
    src/service/server_impl/legacy_zmq_service.h
    src/service/server_impl/legacy_zmq_service.cc
    src/service/server_impl/core_bundles.h
    src/service/server_impl/service_context.h
    src/service/server_impl/service_context.cc
    src/service/local_status_writer.h
    src/service/local_status_writer.cc
    src/service/local_service.cc)
  target_link_libraries(txsim-local-service
    txsim-coordinator ${LIB_PROTOBUF} ${LIB_ZMQ} ${LIB_SQLITE} ${LIB_XML} ${LIB_JSON} ${LIB_GLOG}
    ${LIB_BOOST_FS} ${LIB_BOOST_PO} ${LIB_BOOST_DATE_TIME} ${LIB_PROJ} ${LIB_URIPARSER} ${LIB_SYS} ${LIB_ABSL})

  if(WIN32)
    install(TARGETS txsim-local-service txsim-module-launcher txsim-module-impl
      RUNTIME DESTINATION .
    )
  endif()

  if(0)
    add_executable(txsim-general-service
      src/service/server_impl/http_json_body.h
      src/service/server_impl/http_json_body.cc
      src/service/server_impl/listener.h
      src/service/server_impl/listener.cc
      src/service/server_impl/http_session.h
      src/service/server_impl/http_session.cc
      src/service/server_impl/ws_session.h
      src/service/server_impl/ws_session.cc
      src/service/server_impl/legacy_zmq_service.h
      src/service/server_impl/legacy_zmq_service.cc
      src/service/server_impl/core_bundles.h
      src/service/server_impl/service_context.h
      src/service/server_impl/service_context.cc
      src/service/general_status_writer.h
      src/service/general_status_writer.cc
      src/service/general_service.cc)
    target_link_libraries(txsim-general-service
      txsim-coordinator ${LIB_PROTOBUF} ${LIB_ZMQ} ${LIB_SQLITE} ${LIB_XML} ${LIB_JSON} ${LIB_GLOG}
      ${LIB_BOOST_FS} ${LIB_BOOST_PO} ${LIB_PROJ} ${LIB_URIPARSER} ${LIB_SYS})
  endif(0)

  if(UNIX) # currently cloud service builds only on linux.
    protobuf_generate_cpp(PROTO_SRC_GRADING PROTO_HDR_GRADING ../../common/message/grading.proto)
    protobuf_generate_cpp(PROTO_SRC_TRAFFIC_FRAME PROTO_HDR_TRAFFIC_FRAME ../../common/message/traffic_frame.proto)
    protobuf_generate_cpp(PROTO_SRC_LANE_MARKS PROTO_HDR_LANE_MARKS ../../common/message/laneMarks.proto)
    protobuf_generate_cpp(PROTO_SRC_CONTROL PROTO_HDR_CONTROL ../../common/message/control.proto)
    protobuf_generate_cpp(PROTO_SRC_VEHSTATE PROTO_HDR_VEHSTATE ../../common/message/vehState.proto)
    protobuf_generate_cpp(PROTO_SRC_HEADER PROTO_HDR_HEADER ../../common/message/header.proto)
    protobuf_generate_cpp(PROTO_SRC_TRAFFIC_BILLBOARD PROTO_HDR_TRAFFIC_BILLBOARD ../../common/message/traffic_billboard.proto)
    protobuf_generate_cpp(PROTO_SRC_ENVIRONMENT PROTO_HDR_ENVIRONMENT ../../common/message/environment.proto)
    protobuf_generate_cpp(PROTO_SRC_DEBUGMESSAGE PROTO_HDR_DEBUGMESSAGE ../../common/message/debugmessage.proto)
    protobuf_generate_cpp(PROTO_SRC_CONTROL_V2 PROTO_HDR_CONTROL_V2 ../../common/message/control_v2.proto)

    protobuf_generate_grpc_cpp(PROTO_SRC_CLOUD PROTO_HDR_CLOUD GRPC_SRC_CLOUD GRPC_HDR_CLOUD ../../common/message/sim_cloud_service.proto)
    protobuf_generate_grpc_cpp(PROTO_SRC_CLOUD_CITY PROTO_HDR_CLOUD_CITY GRPC_SRC_CLOUD_CITY GRPC_HDR_CLOUD_CITY ../../common/message/sim_cloud_city_service.proto)
    add_executable(txsim-cloud-service
      src/service/server_impl/core_bundles.h
      src/service/cloud_topic_writer.h
      src/service/cloud_topic_writer.cc
      src/service/cloud_service.cc
      src/service/server_impl/http_client.cc
      src/service/server_impl/http_client.h
      ${PROTO_SRC_CLOUD}
      ${GRPC_SRC_CLOUD}
      ${PROTO_SRC_TRAFFIC_FRAME}
      ${PROTO_SRC_GRADING}
      ${PROTO_SRC_LANE_MARKS}
      ${PROTO_SRC_CONTROL}
      ${PROTO_SRC_VEHSTATE}
      ${PROTO_SRC_HEADER}
      ${PROTO_SRC_TRAFFIC_BILLBOARD}
      ${PROTO_SRC_ENVIRONMENT}
      ${PROTO_SRC_DEBUGMESSAGE}
      ${PROTO_SRC_CONTROL_V2}
    )
    target_link_libraries(txsim-cloud-service
      txsim-coordinator ${LIB_GRPC} ${LIB_PROTOBUF} ${LIB_ZMQ} ${LIB_SQLITE} ${LIB_XML} ${LIB_JSON} ${LIB_GLOG}
      RdKafka::rdkafka++ RdKafka::rdkafka ${LIB_BOOST_FS} ${LIB_BOOST_PO} ${LIB_BOOST_DATE_TIME} ${LIB_PROJ} crypto ${LIB_SYS})
    add_executable(txsim-cloud-city-service
      src/service/server_impl/core_bundles.h
      src/service/server_impl/http_client.cc
      src/service/server_impl/http_client.h
      src/service/cloud_topic_writer.h
      src/service/cloud_topic_writer.cc
      src/service/cloud_city_service.cc
      ../../common/map_sdk/datamodel/src/common/coord_trans.cpp
      ${PROTO_SRC_CLOUD_CITY}
      ${GRPC_SRC_CLOUD_CITY}
      ${PROTO_SRC_BASIC}
      ${PROTO_SRC_LOC}
      ${PROTO_SRC_TRAJECTORY}
      ${PROTO_SRC_TRAFFIC}
      ${PROTO_SRC_GRADING}
      ${PROTO_SRC_VEHICLE_INTERACTION}
      ${PROTO_SRC_TRAFFIC_FRAME}
      ${PROTO_SRC_LANE_MARKS}
      ${PROTO_SRC_CONTROL}
      ${PROTO_SRC_VEHSTATE}
      ${PROTO_SRC_HEADER}
      ${PROTO_SRC_TRAFFIC_BILLBOARD}
      ${PROTO_SRC_ENVIRONMENT}
      ${PROTO_SRC_DEBUGMESSAGE}
      ${PROTO_SRC_CONTROL_V2}
    )
    target_include_directories(txsim-cloud-city-service PRIVATE ../../common/map_sdk/datamodel/include)
    target_link_libraries(txsim-cloud-city-service
      txsim-coordinator ${LIB_GRPC} ${LIB_PROTOBUF} ${LIB_ZMQ} ${LIB_SQLITE} ${LIB_XML} ${LIB_JSON} ${LIB_GLOG}
      RdKafka::rdkafka++ RdKafka::rdkafka ${LIB_BOOST_FS} ${LIB_BOOST_PO} ${LIB_BOOST_DATE_TIME} ${LIB_PROJ} ${LIB_SYS})
  endif(UNIX)

  add_library(txsim-client
    src/coordinator/status.h
    src/coordinator/status.cc
    src/coordinator/local_client.h
    src/coordinator/local_client.cc
    src/utils/os.h
    src/utils/os.cc)
  target_compile_options(txsim-client PRIVATE -fPIC)

  # add_library(txSimCliObjs OBJECT
  # src/cli/sim_client.h
  # src/cli/sim_client.cc
  # src/cli/root_cmd.cc
  # src/cli/root_cmd.h
  # src/cli/conf_cmd.h
  # src/cli/conf_cmd.cc
  # src/cli/CLI.hpp)
  #
  # add_executable(txsim-cli
  # src/cli/cli.cc
  # $<TARGET_OBJECTS:txSimCliObjs>)
  # target_link_libraries(txsim-cli txsim-client ${LIB_ZMQ} ${LIB_JSON} ${LIB_SYS})
  if(NOT(CMAKE_BUILD_TYPE MATCHES Debug) AND BUILD_NODE_ADDON) # node builds only in Release mode.
    find_program(CMAKE_JS_EXEC cmake-js)

    if(NOT(CMAKE_JS_EXEC STREQUAL "CMAKE_JS_EXEC-NOTFOUND"))
      message("found cmake-js:" ${CMAKE_JS_EXEC})

      # if(WIN32)
      # set(CMAKE_ARGS)
      # elseif(UNIX)
      # set(CMAKE_ARGS)
      # endif(WIN32)
      add_custom_command(
        TARGET txsim-client POST_BUILD
        COMMAND cmake-js build -d ../src/node_addon --CDCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -G ${CMAKE_GENERATOR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "building txsim-play-service.node using cmake-js ...")
    else()
      message("cmake-js NOT found. node C++ addon would not be compiled.")
    endif()
  endif(NOT(CMAKE_BUILD_TYPE MATCHES Debug) AND BUILD_NODE_ADDON)

  add_executable(tests
    tests/mocks.h
    tests/utils.h
    tests/utils.cc
    tests/msg_test.cc
    tests/coordinator_test.cc
    tests/scenario_parsing_test.cc
    tests/config_mgr_test.cc
    tests/cloud_city_test.cc)

  target_link_libraries(tests txsim-coordinator txsim-client ${LIB_PROTOBUF} ${LIB_ZMQ} ${LIB_SQLITE} ${LIB_XML} ${LIB_JSON} ${LIB_GLOG}
    ${LIB_BOOST_FS} ${LIB_BOOST_PO} ${LIB_BOOST_DATE_TIME} ${LIB_GTEST} ${LIB_PROJ} ${LIB_SYS} ${LIB_ABSL})

  if(CMAKE_BUILD_TYPE MATCHES Debug) # only build tests coverage in debug mode.
    if(UNIX AND TXSIM_WITH_CODE_COVERAGE MATCHES ON)
      message("compile with code coverage support")
      set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake/modules/)
      set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -Wno-deprecated-register -O0 -g -fprofile-arcs -ftest-coverage ${CMAKE_CXX_FLAGS_DEBUG}")
      include(CodeCoverage)
      setup_target_for_coverage(${PROJECT_NAME}_coverage tests coverage)
    endif(UNIX AND TXSIM_WITH_CODE_COVERAGE MATCHES ON)
  endif(CMAKE_BUILD_TYPE MATCHES Debug)

  if(TXSIM_WITH_PERF)
    add_library(perf-test-module SHARED
      perf/test_module.cc)

    add_executable(perf-test
      src/utils/os.cc
      perf/test_client.cc)
    target_link_libraries(perf-test txsim-client ${LIB_ZMQ} ${LIB_SYS})
  endif(TXSIM_WITH_PERF)

  add_executable(upgrade-db
    tools/upgrade_db.cc)
  target_link_libraries(upgrade-db ${LIB_SQLITE} ${LIB_BOOST_FS} ${LIB_SYS})
endif(NOT TXSIM_SDK_ONLY)

# ###############################################################################
# ########################### API document auto-gen #############################
# ###############################################################################
if(TXSIM_WITH_DOC MATCHES ON)
  find_package(Doxygen REQUIRED)
  find_package(Sphinx REQUIRED)

  # Doxygen stuff
  get_target_property(TXSIM_PUBLIC_HEADER_DIR txsim-module-service INTERFACE_INCLUDE_DIRECTORIES)
  file(GLOB_RECURSE TXSIM_PUBLIC_HEADERS ${TXSIM_PUBLIC_HEADER_DIR}/txsim_*.h)

  set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/src)
  set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
  set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.html)
  set(DOXYFILE_IN ${PROJECT_SOURCE_DIR}/docs/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

  add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
    DEPENDS ${TXSIM_PUBLIC_HEADERS}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
    COMMENT "Generating txSimSDK docs with doxygen"
    VERBATIM)

  add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

  # Sphinx stuff
  set(SPHINX_SOURCE ${PROJECT_SOURCE_DIR}/docs)
  set(SPHINX_BUILD ${PROJECT_SOURCE_DIR}/docs/sphinx)
  set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

  # Only regenerate Sphinx when:
  # - Doxygen has rerun
  # - Our doc files have been updated
  # - The Sphinx config has been updated
  if(UNIX)
    execute_process(COMMAND whoami OUTPUT_VARIABLE CURRENT_USER)
  elseif(WIN32)
    execute_process(COMMAND cmd.exe /c echo %USERNAME% OUTPUT_VARIABLE CURRENT_USER)
  endif()

  string(STRIP "${CURRENT_USER}" CURRENT_USER)
  message(STATUS "Current user: ${CURRENT_USER}")

  add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
    COMMAND

    # ${SPHINX_EXECUTABLE} -b html
    ${CMAKE_COMMAND} -E env USER=${CURRENT_USER} ${SPHINX_EXECUTABLE} -b html

    # Tell Breathe where to find the Doxygen output
    -Dbreathe_projects.txSimSDK=${DOXYGEN_OUTPUT_DIR}/xml
    ${SPHINX_SOURCE} ${SPHINX_BUILD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS

    # Other docs files you want to track should go here (or in some variable)
    ${SPHINX_SOURCE}/index.rst
    ${DOXYGEN_INDEX_FILE}
    MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
    COMMENT "Generating txSimSDK documentation with Sphinx")

  add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})
endif(TXSIM_WITH_DOC MATCHES ON)
