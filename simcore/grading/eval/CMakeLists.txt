cmake_minimum_required(VERSION 3.4.0)

project(${eval_base})

include_directories(${inc_dir})

if(WIN32)
# add_compile_options("/MP")
else()
  add_compile_options(-fPIC)
endif(WIN32)

aux_source_directory(./ base_srcs)

add_library(${eval_base} STATIC ${base_srcs})
