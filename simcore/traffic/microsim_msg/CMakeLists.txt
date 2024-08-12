cmake_minimum_required(VERSION 3.5.1)
project(cmake_protobuf)

include_directories(${PROTOBUF_INCLUDE_FILES})

# Find protobuf
if(WIN32)
  find_package(protobuf CONFIG REQUIRED)
  set(PROTOBUF_LIBRARIES protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
else()
  find_package(Protobuf REQUIRED)
endif()

if(PROTOBUF_FOUND)
  message("protobuf found")
else()
  message(FATAL_ERROR "Cannot find Protobuf")
endif()

# ###
find_program(PROTOC_BIN protoc)
message("protoc bin: ${PROTOC_BIN}")

file(GLOB ALL_PROTOS
  "*.proto"
)

FOREACH(proto_file ${ALL_PROTOS})
  execute_process(COMMAND ${PROTOC_BIN} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
    --cpp_out=${CMAKE_CURRENT_SOURCE_DIR} ${proto_file})
ENDFOREACH(proto_file)

find_program(GRPC_CPP_PLUGIN_BIN grpc_cpp_plugin)
message("grpc cpp plugin bin: ${GRPC_CPP_PLUGIN_BIN}")

set(GRPC_PROTOS
  fastx_sim.proto
)

FOREACH(grpc_proto_file ${GRPC_PROTOS})
  execute_process(COMMAND ${PROTOC_BIN} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
    --grpc_out=${CMAKE_CURRENT_SOURCE_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_BIN} ${grpc_proto_file})
ENDFOREACH(grpc_proto_file)

# ####

# Compile the .proto file into .cpp and .h file
set(PROTO_FILES
  its_sim.proto
  its_semantic_map.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
message("PROTO_SRCS = ${PROTO_SRCS}")
message("PROTO_HDRS = ${PROTO_HDRS}")

set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -ffunction-sections -O0")

add_library(cmake_protobuf
  ${PROTO_SRCS}
  ${PROTO_HDRS})

# Include dependent
target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${PROTOBUF_INCLUDE_DIRS})

# Library dependent
target_link_libraries(${PROJECT_NAME} ${PROTOBUF_LIBRARIES})
