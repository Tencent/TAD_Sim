cmake_minimum_required(VERSION 3.14.0)

project(tx_car LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# set mdl name
set(LIB_SFUNC_WRAPPER sfunc_wrapper CACHE INTERNAL "sfunc wrapper lib")
set(LIB_TXCAR tx_car CACHE INTERNAL "tx_car lib")
set(LIB_TXCAR_SIMSG txcar_simmsg CACHE INTERNAL "txcar sim msg")

set(TXCAR_ROOT ${CMAKE_SOURCE_DIR} CACHE INTERNAL "root dir")

if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
  add_compile_options("/MP")

  # defaul stack size is 1MB which is not enough, make it 8MB
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:8388608")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /STACK:8388608")

  set(LIB_SYS shlwapi) # shlwapi for glog

  include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

  find_package(protobuf CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
  set(LIB_PROTOBUF protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite ZLIB::ZLIB)

  find_package(glog CONFIG REQUIRED)
  set(LIB_LOG glog::glog gflags::gflags)

  find_package(GTest CONFIG REQUIRED)
  set(LIB_GTEST GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
else()
  # include_directories(${inc_dir})
  add_compile_options(-fPIC)

  # default stack size on linux platform is big enough for this project, manully make it 8MB just in case.
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=8388608")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,stack-size=8388608")

  set(LIB_SYS pthread)
  set(LIB_PROTOBUF protobuf)
  set(LIB_GTEST gtest gtest_main)
  set(LIB_LOG glog gflags)
endif(WIN32)

#
include(${TXCAR_ROOT}/../cmake/helper_funcs.cmake)
protobuf_generate_cpp(PROTO_SRC_accessory PROTO_HDR_accessory ${TXCAR_ROOT}/message/accessory.proto)
protobuf_generate_cpp(PROTO_SRC_brake PROTO_HDR_brake ${TXCAR_ROOT}/message/brake.proto)
protobuf_generate_cpp(PROTO_SRC_car_basic PROTO_HDR_car_basic ${TXCAR_ROOT}/message/car_basic.proto)
protobuf_generate_cpp(PROTO_SRC_car_type PROTO_HDR_car_type ${TXCAR_ROOT}/message/car_type.proto)
protobuf_generate_cpp(PROTO_SRC_car PROTO_HDR_car ${TXCAR_ROOT}/message/car.proto)
protobuf_generate_cpp(PROTO_SRC_carsim_par PROTO_HDR_carsim_par ${TXCAR_ROOT}/message/carsim_par.proto)
protobuf_generate_cpp(PROTO_SRC_driveline PROTO_HDR_driveline ${TXCAR_ROOT}/message/driveline.proto)
protobuf_generate_cpp(PROTO_SRC_ecu PROTO_HDR_ecu ${TXCAR_ROOT}/message/ecu.proto)
protobuf_generate_cpp(PROTO_SRC_night_watcher PROTO_HDR_night_watcher ${TXCAR_ROOT}/message/night_watcher.proto)
protobuf_generate_cpp(PROTO_SRC_propulsion PROTO_HDR_propulsion ${TXCAR_ROOT}/message/propulsion.proto)
protobuf_generate_cpp(PROTO_SRC_sprungmass PROTO_HDR_sprungmass ${TXCAR_ROOT}/message/sprungmass.proto)
protobuf_generate_cpp(PROTO_SRC_steer PROTO_HDR_steer ${TXCAR_ROOT}/message/steer.proto)
protobuf_generate_cpp(PROTO_SRC_suspension PROTO_HDR_suspension ${TXCAR_ROOT}/message/suspension.proto)
protobuf_generate_cpp(PROTO_SRC_vehbody PROTO_HDR_vehbody ${TXCAR_ROOT}/message/vehbody.proto)
protobuf_generate_cpp(PROTO_SRC_wheeltire PROTO_HDR_wheeltire ${TXCAR_ROOT}/message/wheeltire.proto)

# file(GLOB proto_files "${TXCAR_ROOT}/message/*.proto")

# if(NOT proto_files)
# message(FATAL_ERROR "No .proto files found in ${TXCAR_ROOT}/message/")
# endif()

# foreach(proto_file ${proto_files})
# message(STATUS "++++++++++++++++++++ ${proto_file}")
# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${proto_file}")
# endforeach()

# set(inc_dir ${TXCAR_ROOT} ${TXCAR_ROOT}/src ${TXCAR_ROOT}/src/r2022b ${TXCAR_ROOT}/inc/r2022b ${TXCAR_ROOT}/message CACHE INTERNAL "include dir")
set(inc_dir ${TXCAR_ROOT} ${TXCAR_ROOT}/src ${TXCAR_ROOT}/src/r2022b ${TXCAR_ROOT}/inc/r2022b ${CMAKE_BINARY_DIR} CACHE INTERNAL "include dir")
include_directories(${inc_dir})

# aux_source_directory(${TXCAR_ROOT}/message proto_src_files)
aux_source_directory(${TXCAR_ROOT}/src src_files)
aux_source_directory(${TXCAR_ROOT}/src/engine engine_srcs)
aux_source_directory(${TXCAR_ROOT}/src/motor motor_srcs)
aux_source_directory(${TXCAR_ROOT}/src/battery battery_srcs)

# aux_source_directory(${TXCAR_ROOT}/src/driveline driveline_singel_pow_srcs)
aux_source_directory(${TXCAR_ROOT}/src/softecu ecu_srcs)
aux_source_directory(${TXCAR_ROOT}/src/cartype_parsing cartype_src)
aux_source_directory(${TXCAR_ROOT}/src/brake_hydraulic brake_hydraulic_src)

# aux_source_directory(${TXCAR_ROOT}/src/mapped_steer mapped_steer_srcs)
aux_source_directory(${TXCAR_ROOT}/src/dynamic_steer dynamic_steer_srcs)
aux_source_directory(${TXCAR_ROOT}/src/r2022b simulink_srcs)

# aux_source_directory(${TXCAR_ROOT}/src/veh_body veh_body_srcs)
# aux_source_directory(${TXCAR_ROOT}/src/wheel_tire wheel_tire_srcs)
# aux_source_directory(${TXCAR_ROOT}/src/suspension suspension_srcs)
aux_source_directory(${TXCAR_ROOT}/src/chassis one_chassis_src)
aux_source_directory(${TXCAR_ROOT}/src/transmission transmission_src)
aux_source_directory(${TXCAR_ROOT}/src/driveline_ICE driveline_ice_src)
aux_source_directory(${TXCAR_ROOT}/src/driveline_EV_front driveline_ev_front_src)
aux_source_directory(${TXCAR_ROOT}/src/driveline_EV_rear driveline_ev_rear_src)
aux_source_directory(${TXCAR_ROOT}/src/softabs softabs_src)
aux_source_directory(${TXCAR_ROOT}/src/softhcu softhcu_src)
aux_source_directory(${TXCAR_ROOT}/src/hybrid_parser hybrid_parser_src)

set(chassis_src ${one_chassis_src})

# build txcar sim_msg
set(proto_src_files
  ${PROTO_SRC_accessory}
  ${PROTO_SRC_brake}
  ${PROTO_SRC_car_basic}
  ${PROTO_SRC_car_type}
  ${PROTO_SRC_car}
  ${PROTO_SRC_carsim_par}
  ${PROTO_SRC_driveline}
  ${PROTO_SRC_ecu}
  ${PROTO_SRC_night_watcher}
  ${PROTO_SRC_propulsion}
  ${PROTO_SRC_sprungmass}
  ${PROTO_SRC_steer}
  ${PROTO_SRC_suspension}
  ${PROTO_SRC_vehbody}
  ${PROTO_SRC_wheeltire}
)
add_library(${LIB_TXCAR_SIMSG} STATIC ${proto_src_files})

# build module
add_library(
  ${LIB_TXCAR} SHARED
  ${src_files}
  ${simulink_srcs}
  ${engine_srcs}
  ${motor_srcs}
  ${battery_srcs}
  ${ecu_srcs}
  ${cartype_src}
  ${chassis_src}
  ${dynamic_steer_srcs}
  ${brake_hydraulic_src}
  ${transmission_src}
  ${driveline_ice_src}
  ${driveline_ev_front_src}
  ${driveline_ev_rear_src}
  ${softabs_src}
  ${softhcu_src}
  ${hybrid_parser_src}
)
target_link_libraries(${LIB_TXCAR} ${LIB_TXCAR_SIMSG} ${LIB_SYS} ${LIB_PROTOBUF} ${LIB_LOG})

# gtest
aux_source_directory(${TXCAR_ROOT}/src/test test_srcs)

# build for testing
add_executable(${LIB_TXCAR}_test ${test_srcs})
target_link_libraries(${LIB_TXCAR}_test ${LIB_SYS} ${LIB_TXCAR} ${LIB_GTEST} ${LIB_LOG} ${LIB_PROTOBUF})
include(GoogleTest)
gtest_discover_tests(${LIB_TXCAR}_test)

# prarmeter writer
aux_source_directory(${TXCAR_ROOT}/src/param_writer param_writer_srcs)
add_executable(param_writer ${param_writer_srcs} ${src_files})
target_link_libraries(param_writer ${LIB_TXCAR_SIMSG} ${LIB_SYS} ${LIB_PROTOBUF} ${LIB_LOG})

# prarmeter writer hybrid
aux_source_directory(${TXCAR_ROOT}/src/param_writer_hybrid param_writer_hybrid_srcs)
add_executable(param_writer_hybrid ${param_writer_hybrid_srcs} ${src_files})
target_link_libraries(param_writer_hybrid ${LIB_TXCAR_SIMSG} ${LIB_SYS} ${LIB_PROTOBUF} ${LIB_LOG})

# build sfunc wrapper
include_directories(
  ${TXCAR_ROOT}/mdl/libtx_car
  ${TXCAR_ROOT}/src/battery
  ${TXCAR_ROOT}/src/cartype_parsing
  ${TXCAR_ROOT}/src/chassis
  ${TXCAR_ROOT}/src/driveline
  ${TXCAR_ROOT}/src/ecu
  ${TXCAR_ROOT}/src/engine
  ${TXCAR_ROOT}/src/mapped_steer
  ${TXCAR_ROOT}/src/motor
  ${TXCAR_ROOT}/src/transmission
  ${TXCAR_ROOT}/src/driveline_ICE
  ${TXCAR_ROOT}/src/driveline_EV_front
  ${TXCAR_ROOT}/src/driveline_EV_rear
  ${TXCAR_ROOT}/src/softabs
  ${TXCAR_ROOT}/src/softhcu
  ${TXCAR_ROOT}/src/hybrid_parser
)
file(GLOB vehicle_wrapper_srcs ${TXCAR_ROOT}/mdl/libtx_car/*_wrapper.cpp)
add_library(${LIB_SFUNC_WRAPPER} ${vehicle_wrapper_srcs})

# build carsim .par converter
aux_source_directory(${TXCAR_ROOT}/src/par_converter par_converter_srcs)
add_executable(par_converter ${par_converter_srcs} ${src_files})
target_link_libraries(par_converter ${LIB_TXCAR_SIMSG} ${LIB_SYS} ${LIB_LOG} ${LIB_PROTOBUF})

# build simpack
add_subdirectory(${TXCAR_ROOT}/sim_pack)
