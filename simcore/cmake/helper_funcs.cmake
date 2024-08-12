if(WIN32)  
  find_package(protobuf CONFIG REQUIRED)
  function(protobuf_generate_cpp src_var hdr_var proto_path)
    get_filename_component(proto_abs_path "${proto_path}" ABSOLUTE)
    get_filename_component(proto_dir "${proto_abs_path}" PATH)
    get_filename_component(proto_name "${proto_abs_path}" NAME_WE)
    set(proto_src "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.cc")
    set(proto_hdr "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.h")
    add_custom_command(
      OUTPUT "${proto_src}" "${proto_hdr}"
      COMMAND protobuf::protoc
      ARGS -I="${proto_dir}"
        --cpp_out="${CMAKE_CURRENT_BINARY_DIR}"
        "${proto_abs_path}"
      DEPENDS "${proto_abs_path}")
    set(${src_var} "${proto_src}" PARENT_SCOPE)
    set(${hdr_var} "${proto_hdr}" PARENT_SCOPE)
  endfunction()

  function(protobuf_generate_grpc_cpp pb_src_var pb_hdr_var grpc_src_var grpc_hdr_var proto_path)
    get_filename_component(proto_abs_path "${proto_path}" ABSOLUTE)
    get_filename_component(proto_dir "${proto_abs_path}" PATH)
    get_filename_component(proto_name "${proto_abs_path}" NAME_WE)
    set(proto_src "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.cc")
    set(proto_hdr "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.h")
    set(grpc_src "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.grpc.pb.cc")
    set(grpc_hdr "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.grpc.pb.h")
    add_custom_command(
      OUTPUT "${proto_src}" "${proto_hdr}" "${grpc_src}" "${grpc_hdr}"
      COMMAND protobuf::protoc
      ARGS -I="${proto_dir}"
        --cpp_out="${CMAKE_CURRENT_BINARY_DIR}"
        --grpc_out="${CMAKE_CURRENT_BINARY_DIR}"
        --plugin=${protobuf_generate_PLUGIN}
        "${proto_abs_path}"
      DEPENDS "${proto_abs_path}")
    set(${pb_src_var} "${proto_src}" PARENT_SCOPE)
    set(${pb_hdr_var} "${proto_hdr}" PARENT_SCOPE)
    set(${grpc_src_var} "${grpc_src}" PARENT_SCOPE)
    set(${grpc_hdr_var} "${grpc_hdr}" PARENT_SCOPE)
  endfunction()
elseif(UNIX) # for linux only currently.
  find_package(Protobuf REQUIRED)
  if(${Protobuf_VERSION})
    message("found protobuf library version: ${Protobuf_VERSION}")
  endif()
  find_program(_PROTOBUF_PROTOC protoc)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

  function(protobuf_generate_grpc_cpp pb_src_var pb_hdr_var grpc_src_var grpc_hdr_var proto_path)
    get_filename_component(proto_abs_path "${proto_path}" ABSOLUTE)
    get_filename_component(proto_dir "${proto_abs_path}" PATH)
    get_filename_component(proto_name "${proto_abs_path}" NAME_WE)
    set(proto_src "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.cc")
    set(proto_hdr "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.h")
    set(grpc_src "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.grpc.pb.cc")
    set(grpc_hdr "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.grpc.pb.h")
    add_custom_command(
      OUTPUT "${proto_src}" "${proto_hdr}" "${grpc_src}" "${grpc_hdr}"
      COMMAND "${_PROTOBUF_PROTOC}"
      ARGS -I="${proto_dir}"
        --cpp_out="${CMAKE_CURRENT_BINARY_DIR}"
        --grpc_out="${CMAKE_CURRENT_BINARY_DIR}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${proto_abs_path}"
      DEPENDS "${proto_abs_path}")
    set(${pb_src_var} "${proto_src}" PARENT_SCOPE)
    set(${pb_hdr_var} "${proto_hdr}" PARENT_SCOPE)
    set(${grpc_src_var} "${grpc_src}" PARENT_SCOPE)
    set(${grpc_hdr_var} "${grpc_hdr}" PARENT_SCOPE)
  endfunction()
endif(WIN32)

function(protobuf_generate_cpps srcs hdrs)
  foreach(proto_path ${ARGN})
    protobuf_generate_cpp(src hdr ${proto_path})
    # cmake_language(CALL protobuf_generate_cpp src hdr ${proto_path})
    list(APPEND srcs_ ${src})
    list(APPEND hdrs_ ${hdr})
  endforeach()  
  set(${srcs} ${srcs_} PARENT_SCOPE)
  set(${hdrs} ${hdrs_} PARENT_SCOPE)
endfunction()
