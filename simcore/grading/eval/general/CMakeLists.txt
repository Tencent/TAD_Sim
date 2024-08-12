cmake_minimum_required(VERSION 3.4.0)

project(${eval_general})

set(CMAKE_CXX_STANDARD 11)

include_directories(${inc_dir})

if(WIN32)
  # add_compile_options("/MP")
  find_package(absl CONFIG REQUIRED)
else()
  add_compile_options("-fPIC")
endif(WIN32)

aux_source_directory(./ base_srcs)

add_library(${eval_general} STATIC ${base_srcs})

if(WIN32)
  target_link_libraries(${eval_general} absl::base)
endif(WIN32)
