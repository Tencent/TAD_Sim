cmake_minimum_required(VERSION 3.4.0)

project(utils)

include_directories(${inc_dir})

if(WIN32)
# add_compile_options("/MP")
else()
  add_compile_options("-fPIC")
endif(WIN32)

aux_source_directory(./ base_srcs)

add_library(${utils_name} STATIC ${base_srcs})
