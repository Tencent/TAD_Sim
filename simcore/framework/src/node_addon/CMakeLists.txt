cmake_minimum_required(VERSION 2.8.2)

project(txsim-play-service)

cmake_policy(SET CMP0015 NEW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
message("node_addon CMAKE_TOOLCHAIN_FILE:" ${CMAKE_TOOLCHAIN_FILE})

if(WIN32)
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)
  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/eigen3)
  include_directories(${Boost_INCLUDE_DIRS})
  add_definitions(-DZMQ_STATIC -D_WINSOCKAPI_ -DNOMINMAX)
  find_package(jsoncpp CONFIG REQUIRED)
  find_package(protobuf CONFIG REQUIRED)
  find_package(Boost REQUIRED COMPONENTS system filesystem program_options regex)
  find_package(cppzmq CONFIG REQUIRED)

  set(LIB_BOOST_FS ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_REGEX_LIBRARY})
  set(LIB_ZMQ cppzmq cppzmq-static Advapi32 Rpcrt4 Ws2_32 Iphlpapi)
  set(LIB_PROTOBUF protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
  set(DELAY_LOAD_HOOK win_delay_load_hook.cc)
  set(LIB_JSON JsonCpp::JsonCpp)
elseif(UNIX) # for linux only currently.
  find_package(jsoncpp CONFIG REQUIRED)
  set(LIB_BOOST_FS boost_filesystem boost_system)
  set(LIB_ZMQ libzmq.a)
  set(LIB_PROTOBUF libprotobuf.a)
  set(DELAY_LOAD_HOOK)
  set(LIB_JSON jsoncpp)
endif(WIN32)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/helper_funcs.cmake)
message("CMAKE_CURRENT_SOURCE_DIR:" ${CMAKE_CURRENT_SOURCE_DIR})
message("PROJECT_SOURCE_DIR:" ${PROJECT_SOURCE_DIR}/../../../../common/message/basic.proto)
protobuf_generate_cpp(PROTO_SRC_BASIC PROTO_HDR_BASIC ${PROJECT_SOURCE_DIR}/../../../../common/message/basic.proto)
protobuf_generate_cpp(PROTO_SRC_LOCATION PROTO_HDR_LOCATION ${PROJECT_SOURCE_DIR}/../../../../common/message/location.proto)
protobuf_generate_cpp(PROTO_SRC_TRAFFIC PROTO_HDR_TRAFFIC ${PROJECT_SOURCE_DIR}/../../../../common/message/traffic.proto)
protobuf_generate_cpp(PROTO_SRC_TRAJECTORY PROTO_HDR_TRAJECTORY ${PROJECT_SOURCE_DIR}/../../../../common/message/trajectory.proto)
protobuf_generate_cpp(PROTO_SRC_TRJ_FLW PROTO_HDR_TRJ_FLW ${PROJECT_SOURCE_DIR}/../../../../common/message/trajectoryFollowing.proto)
protobuf_generate_cpp(PROTO_SRC_GRADING PROTO_HDR_GRADING ${PROJECT_SOURCE_DIR}/../../../../common/message/grading.proto)
protobuf_generate_cpp(PROTO_SRC_HEADER PROTO_HDR_HEADER ${PROJECT_SOURCE_DIR}/../../../../common/message/header.proto)
protobuf_generate_cpp(PROTO_SRC_CONTROL PROTO_HDR_CONTROL ${PROJECT_SOURCE_DIR}/../../../../common/message/control.proto)
protobuf_generate_cpp(PROTO_SRC_VEHSTAT PROTO_HDR_VEHSTAT ${PROJECT_SOURCE_DIR}/../../../../common/message/vehState.proto)
protobuf_generate_cpp(PROTO_SRC_CONTROL_V2 PROTO_HDR_CONTROL_V2 ${PROJECT_SOURCE_DIR}/../../../../common/message/control_v2.proto)
protobuf_generate_cpp(PROTO_SRC_DEBUG_MESSAGE PROTO_HDR_DEBUG_MESSAGE ${PROJECT_SOURCE_DIR}/../../../../common/message/debugmessage.proto)

include_directories(${CMAKE_JS_INC} .. ${CMAKE_BINARY_DIR})

link_directories(${PROJECT_SOURCE_DIR}/../../build/lib)
add_library(${PROJECT_NAME} SHARED
  ../utils/os.cc
  play_service.cc
  ${PROTO_SRC_BASIC}
  ${PROTO_SRC_LOCATION}
  ${PROTO_SRC_TRAFFIC}
  ${PROTO_SRC_TRAJECTORY}
  ${PROTO_SRC_TRJ_FLW}
  ${PROTO_SRC_GRADING}
  ${PROTO_SRC_HEADER}
  ${PROTO_SRC_CONTROL}
  ${PROTO_SRC_CONTROL_V2}
  ${PROTO_SRC_DEBUG_MESSAGE}
  ${PROTO_SRC_VEHSTAT}
  ${DELAY_LOAD_HOOK})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_compile_definitions(${PROJECT_NAME} PRIVATE NAPI_DISABLE_CPP_EXCEPTIONS NODE_ADDON_API_DISABLE_DEPRECATED)
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} txsim-client ${LIB_BOOST_FS} ${LIB_ZMQ} ${LIB_PROTOBUF} ${LIB_JSON})

# Include N-API wrappers
execute_process(
  COMMAND node -p "require('node-addon-api').include"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NODE_ADDON_API_DIR)
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

message("NODE_RUNTIME:" ${NODE_RUNTIME})
message("NODE_RUNTIMEVERSION:" ${NODE_RUNTIMEVERSION})
message("NODE_ARCH:" ${NODE_ARCH})
