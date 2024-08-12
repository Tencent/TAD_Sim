cmake_minimum_required(VERSION 3.4.0)

project(softecu)

set(softecu_root ${PROJECT_SOURCE_DIR})
set(softecu softecu CACHE INTERNAL "")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# build output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${build_output_dir})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${build_output_dir})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${build_output_dir})

set(softecu_include_dir ${softecu_root}/inc CACHE INTERNAL "")
include_directories(${common_include_dirs} ${softecu_include_dir})

if(WIN32)
  # annoying windows define
  add_definitions(-D_USE_MATH_DEFINES)
  add_definitions(-DNOMINMAX)

  # set include and link dir
  include_directories(${win_include_dirs})
  link_directories(${win_link_dirs})
elseif(UNIX)
  # add define
  add_definitions("-D__unix__")
  add_compile_options(-fPIC)

  # set include and link dir
  include_directories(${unix_include_dirs})
  link_directories(${unix_link_dirs})
endif(WIN32)

aux_source_directory(${softecu_root}/src softecu_src)

add_library(${softecu} STATIC ${softecu_src})

target_link_libraries(${softecu} ${LIB_SYS})
