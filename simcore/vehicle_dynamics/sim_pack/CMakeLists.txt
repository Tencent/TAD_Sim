cmake_minimum_required(VERSION 3.14.0)

project(tx_simvehicle)

set(CMAKE_CXX_STANDARD 11)

set(LOCAL_INC_DIR ${TXCAR_ROOT} ${TXCAR_ROOT}/inc/r2022b ${TXCAR_ROOT}/inc ${CMAKE_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(framework REQUIRED)

include_directories(
  ${LOCAL_INC_DIR}
  ${TXCAR_ROOT}/mdl/libtx_car
  ${TXCAR_ROOT}/mdl/libtx_car/TxCar_grt_rtw
  ${TXCAR_ROOT}/../../common/message/build/
  ${TXCAR_ROOT}/sim_pack/src/core
  ${TXCAR_ROOT}/mdl/libtx_car/TxCar_ICE_grt_rtw
  ${TXCAR_ROOT}/mdl/libtx_car/TxCar_EV_grt_rtw
  ${TXCAR_ROOT}/mdl/libtx_car/TxCar_Hybrid_grt_rtw
)

# link_directories(${TXCAR_ROOT}/build)
add_definitions(-DRT)

include_directories(${TXCAR_ROOT}/sim_pack/terrain/inc)
link_directories(${TXCAR_ROOT}/sim_pack/terrain/inc ${CMAKE_BINARY_DIR})

if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
  add_compile_options("/MP")

  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/eigen3
  )

  set(LIB_SYS shlwapi) # shlwapi for glog

  find_package(protobuf CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
  set(LIB_PROTOBUF protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite ZLIB::ZLIB)

  find_package(glog CONFIG REQUIRED)
  set(LIB_LOG glog::glog gflags::gflags)

  find_package(Boost REQUIRED COMPONENTS system filesystem)
  set(LIB_BOOST ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

  find_package(GTest CONFIG REQUIRED)
  set(LIB_GTEST GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
else()
  add_compile_options(-fPIC)

  include_directories(${TXCAR_ROOT}/sim_pack/chrono/eigen3)

  find_package(Protobuf REQUIRED)

  set(LIB_SYS pthread)
  set(LIB_PROTOBUF protobuf)
  set(LIB_GTEST gtest gtest_main)
  set(LIB_LOG glog gflags)
  set(LIB_BOOST boost_system boost_filesystem)
endif(WIN32)

set(LIB_TERRAIN terrain)

aux_source_directory(${TXCAR_ROOT}/mdl/libtx_car/TxCar_ICE_grt_rtw ice_codegen_srcs)
aux_source_directory(${TXCAR_ROOT}/mdl/libtx_car/TxCar_EV_grt_rtw ev_codegen_srcs)
aux_source_directory(${TXCAR_ROOT}/mdl/libtx_car/TxCar_Hybrid_grt_rtw hybrid_codegen_srcs)

set(proto_srcs
  ${TXCAR_ROOT}/../../common/message/build/header.pb.cc
  ${TXCAR_ROOT}/../../common/message/build/basic.pb.cc
  ${TXCAR_ROOT}/../../common/message/build/location.pb.cc
  ${TXCAR_ROOT}/../../common/message/build/control.pb.cc
  ${TXCAR_ROOT}/../../common/message/build/control_v2.pb.cc
  ${TXCAR_ROOT}/../../common/message/build/vehState.pb.cc
  ${TXCAR_ROOT}/../../common/message/build/vehicle_geometry.pb.cc
  ${TXCAR_ROOT}/../../common/message/build/settings.pb.cc
  ${TXCAR_ROOT}/../../common/message/build/traffic.pb.cc
)

# build sim_msg and parking adapter
add_library(sim_msg STATIC ${proto_srcs})

# build ICE
set(ICE_VehicleName txcar_ice)
add_library(
  ${ICE_VehicleName} SHARED
  ${ice_codegen_srcs}
  ${sfunc_srcs}
  ${TXCAR_ROOT}/sim_pack/src/core/coord_trans.cpp
  ${TXCAR_ROOT}/sim_pack/src/core/txcar_base.cpp
  ${TXCAR_ROOT}/sim_pack/src/core/txcar_math.cpp
  ${TXCAR_ROOT}/sim_pack/src/ice_vehicle/txcar_imp_ice.cpp
)
add_dependencies(${ICE_VehicleName} sim_msg ${LIB_SFUNC_WRAPPER} ${LIB_TXCAR})
target_link_libraries(
  ${ICE_VehicleName}
  sim_msg
  ${LIB_SFUNC_WRAPPER}
  ${LIB_TXCAR}
  ${LIB_SYS}
  ${LIB_LOG}
  ${LIB_PROTOBUF}
  ${LIB_TERRAIN}
  ${LIB_BOOST}
)

# build EV
set(ICE_VehicleName txcar_ev)
add_library(
  ${ICE_VehicleName} SHARED
  ${ev_codegen_srcs}
  ${sfunc_srcs}
  ${TXCAR_ROOT}/sim_pack/src/core/coord_trans.cpp
  ${TXCAR_ROOT}/sim_pack/src/core/txcar_base.cpp
  ${TXCAR_ROOT}/sim_pack/src/core/txcar_math.cpp
  ${TXCAR_ROOT}/sim_pack/src/ev_vehicle/txcar_imp_ev.cpp
)
add_dependencies(${ICE_VehicleName} sim_msg ${LIB_SFUNC_WRAPPER} ${LIB_TXCAR})
target_link_libraries(
  ${ICE_VehicleName}
  sim_msg
  ${LIB_SFUNC_WRAPPER}
  ${LIB_TXCAR}
  ${LIB_SYS}
  ${LIB_LOG}
  ${LIB_PROTOBUF}
  ${LIB_TERRAIN}
  ${LIB_BOOST}
)

# build hybrid
set(Hybrid_VehicleName txcar_hybrid)
add_library(
  ${Hybrid_VehicleName} SHARED
  ${hybrid_codegen_srcs}
  ${sfunc_srcs}
  ${TXCAR_ROOT}/sim_pack/src/core/coord_trans.cpp
  ${TXCAR_ROOT}/sim_pack/src/core/txcar_base.cpp
  ${TXCAR_ROOT}/sim_pack/src/core/txcar_math.cpp
  ${TXCAR_ROOT}/sim_pack/src/hybrid_vehicle/txcar_imp_hybrid.cpp
)
add_dependencies(${Hybrid_VehicleName} ${LIB_SFUNC_WRAPPER} ${LIB_TXCAR})
target_link_libraries(
  ${Hybrid_VehicleName}
  sim_msg
  ${LIB_SFUNC_WRAPPER}
  ${LIB_TXCAR}
  ${LIB_SYS}
  ${LIB_LOG}
  ${LIB_PROTOBUF}
  ${LIB_TERRAIN}
  ${LIB_BOOST}
)

# build module
add_library(
  ${PROJECT_NAME} SHARED
  ${TXCAR_ROOT}/sim_pack/src/core/txcar_base.cpp
  ${TXCAR_ROOT}/sim_pack/src/txcar_interface.cpp
)

# depend on ${LIB_SFUNC_WRAPPER} ${LIB_TXCAR}
add_dependencies(${PROJECT_NAME} ${LIB_SFUNC_WRAPPER} ${LIB_TXCAR} sim_msg)
target_link_libraries(
  ${PROJECT_NAME}
  ${LIB_PROTOBUF}
  sim_msg
  ${LIB_TXCAR_SIMSG}
  ${LIB_LOG}
  ${LIB_BOOST}
  ${LIB_SYS}
)

# build sim_pack/terrain
add_subdirectory(${TXCAR_ROOT}/sim_pack/terrain)

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_framework_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
