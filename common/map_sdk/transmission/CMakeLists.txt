cmake_minimum_required(VERSION 3.0)
project(transmission)

set(datamodel_SOURCE_DIR "../datamodel")

if(WIN32)
  add_definitions(-fPIC -std=c++11)
  find_package(CURL CONFIG REQUIRED)
else()
  add_definitions(-fPIC -std=c++11)

  if(UE)
    set(UE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../UE_H/4.27/Linux/LibCxx)
    include_directories(${UE_DIR}/include/c++/v1
      ${UE_DIR}/include)
    add_definitions(-nostdinc++)
    set(CPPLIB ${UE_DIR}/lib/Linux/x86_64-unknown-linux-gnu/libc++.a)
  endif(UE)
endif(WIN32)

add_definitions(-DTXSIMMAP_API)
add_definitions(-DTXSIMMAP_EXPORTS)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${datamodel_SOURCE_DIR}/include
)
link_directories(${datamodel_SOURCE_DIR})

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_library(${PROJECT_NAME} SHARED
  src/cJSON.cpp
  src/CJsonObject.cpp
  src/json_parser.cpp
  src/post_param.cpp
  src/trans_interface.cpp
  src/url_gen.cpp
  src/idc_addr.cpp
)

if(WIN32)
  target_link_libraries(${PROJECT_NAME}
    datamodel
    CURL::libcurl
  )

else()
  target_link_libraries(${PROJECT_NAME}
    ${CPPLIB}
    datamodel
    curl
  )
endif(WIN32)
