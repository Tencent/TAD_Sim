cmake_minimum_required(VERSION 3.1...3.20)

project(txSimTraffic LANGUAGES CXX)

option(TADSimProtoBuf "TADSimProtoBuf" OFF)
option(TADSimIO "TADSimIO" OFF)
option(TADSimFramework "TADSimFramework" OFF)
option(TADSimTraffic "TADSimTraffic" OFF)
option(TADSimVirtualCity "TADSimVirtualCity" OFF)
option(TADSimBillboard "TADSimBillboard" OFF)
option(TADSimALL "TADSimALL" ON)
option(OnlyCloud "OnlyCloud" OFF)
option(CloudStandAlone "CloudStandAlone" OFF)
option(OnlyTraffic "OnlyTraffic" OFF)
option(TADChomper "TADChomper" OFF)

# option(WithDependency "WithDependency" OFF)
set(chomperPlugin "chomper/plugin")

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /std:c++14 -DEIGEN_MALLOC_ALREADY_ALIGNED=0 -O2 -Ob2 -Oi -Ot")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -g -Os -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -DEIGEN_MALLOC_ALREADY_ALIGNED=0")
endif()

# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions("-DACCEPT_USE_OF_DEPRECATED_PROJ_API_H")
add_definitions("-DGIT_COMMIT_HASH=${GIT_COMMIT_HASH}")
add_definitions("-DGIT_BRANCH=${GIT_BRANCH}")
add_definitions("-DPRO_PATH=${CMAKE_CURRENT_SOURCE_DIR}")

# add_definitions("-DCOMPILER_YEAR=${CMAKE_CURRENT_YEAR}")
# add_definitions("-DCOMPILER_MONTH=${CMAKE_CURRENT_MONTH}")
# add_definitions("-DCOMPILER_DAY=${CMAKE_CURRENT_DAY}")
# add_definitions("-DCOMPILER_HOUR=${CMAKE_CURRENT_HOUR}")
# add_definitions("-DCOMPILER_MINUTE=${CMAKE_CURRENT_MINUTE}")
# add_definitions("-DCOMPILER_SECOND=${CMAKE_CURRENT_SECOND}")
# generate project version
add_definitions("-DNOMINMAX")

if(MSVC)
  add_definitions(-DZMQ_STATIC -D_WIN32_WINNT=0x601)
  add_definitions("/MP")
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
)

if(WIN32)
  include_directories(
    ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
  )
else()
endif()

message(STATUS "TADSimALL = " ${TADSimALL})
message(STATUS "OnlyCloud = " ${OnlyCloud})
message(STATUS "CloudStandAlone = " ${CloudStandAlone})
message(STATUS "OnlyTraffic = " ${OnlyTraffic})
message(STATUS "TADSimProtoBuf = " ${TADSimProtoBuf})
message(STATUS "TADSimIO = " ${TADSimIO})
message(STATUS "TADSimFramework = " ${TADSimFramework})
message(STATUS "TADSimTraffic = " ${TADSimTraffic})
message(STATUS "TADSimBillboard = " ${TADSimBillboard})
message(STATUS "TADSimVirtualCity = " ${TADSimVirtualCity})
message(STATUS "TADChomper = " ${TADChomper})

if(OnlyCloud OR CloudStandAlone)
  set(WithDependency TRUE)
  message(STATUS "WithDependency = " ${WithDependency})
  add_subdirectory(protobuf)
  add_subdirectory(io)
  add_subdirectory(framework)
  add_subdirectory(cloud)

  if(OnlyCloud)
    message(STATUS "done : " OnlyCloud)
  endif(OnlyCloud)

  if(CloudStandAlone)
    message(STATUS "done : " CloudStandAlone)
  endif(CloudStandAlone)
else(OnlyCloud OR CloudStandAlone)
  if(OnlyTraffic)
    set(WithDependency TRUE)
    message(STATUS "WithDependency = " ${WithDependency})
    add_subdirectory(protobuf)
    add_subdirectory(io)
    add_subdirectory(framework)
    add_subdirectory(app)
    message(STATUS "done : " OnlyTraffic)
  elseif(TADChomper)
    set(GATrainingMode TRUE)
    message(STATUS "GATrainingMode = " ${GATrainingMode})
    set(WithDependency TRUE)
    message(STATUS "WithDependency = " ${WithDependency})
    add_subdirectory(protobuf)
    add_subdirectory(io)
    add_subdirectory(framework)
    add_subdirectory(${chomperPlugin})
    add_subdirectory(chomper)
    message(STATUS "done : " TADChomper)
  else(OnlyTraffic)
    set(WithDependency FALSE)
    message(STATUS "WithDependency = " ${WithDependency})

    if(TADSimProtoBuf)
      add_subdirectory(protobuf)
      message(STATUS "done : " TADSimProtoBuf)
    else()
      message(STATUS "skip : " TADSimProtoBuf)
    endif(TADSimProtoBuf)

    if(TADSimIO)
      add_subdirectory(io)
      message(STATUS "done : " TADSimIO)
    else()
      message(STATUS "skip : " TADSimIO)
    endif(TADSimIO)

    if(TADSimFramework)
      add_subdirectory(framework)
      message(STATUS "done : " TADSimFramework)
    else()
      message(STATUS "skip : " TADSimFramework)
    endif(TADSimFramework)

    if(TADSimTraffic)
      add_subdirectory(app)
      message(STATUS "done : " TADSimTraffic)
    else()
      message(STATUS "skip : " TADSimTraffic)
    endif(TADSimTraffic)

    if(TADSimBillboard)
      add_subdirectory(billboard)
      message(STATUS "done : " TADSimBillboard)
    else()
      message(STATUS "skip : " TADSimBillboard)
    endif(TADSimBillboard)

    if(TADSimVirtualCity)
      add_subdirectory(cloud)
      message(STATUS "done : " TADSimVirtualCity)
    else()
      message(STATUS "skip : " TADSimVirtualCity)
    endif(TADSimVirtualCity)

    if((TADSimALL) AND(NOT(TADSimProtoBuf)) AND(NOT(TADSimIO)) AND(NOT(TADSimFramework)) AND(NOT(TADSimTraffic)) AND(NOT(TADSimBillboard)) AND(NOT(TADSimVirtualCity)))
      set(WithDependency TRUE)
      message(STATUS "WithDependency = " ${WithDependency})
      add_subdirectory(protobuf)
      add_subdirectory(io)
      add_subdirectory(framework)
      add_subdirectory(app)
      add_subdirectory(billboard)
      message(STATUS "done : " TADSimALL)
    else()
      message(STATUS "skip : " TADSimALL)
    endif()
  endif(OnlyTraffic)
endif(OnlyCloud OR CloudStandAlone)
