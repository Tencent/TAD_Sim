cmake_minimum_required(VERSION 3.4.0)

project(hadmap_py LANGUAGES CXX)

set(MAPSDKPY_ROOT ${CMAKE_SOURCE_DIR})

# 设置构建目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

if(WIN32)
# If it is a Windows system, do not perform any operations
else()
  execute_process(COMMAND whoami OUTPUT_VARIABLE CURRENT_USER)
  string(STRIP "${CURRENT_USER}" CURRENT_USER)
  message(STATUS "Current user: ${CURRENT_USER}")

  set(CMAKE_PREFIX_PATH
    "/home/${CURRENT_USER}/.local/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages/pybind11/share/cmake/pybind11"
    "${CMAKE_PREFIX_PATH}"
  )
endif()

find_package(pybind11 CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

if(WIN32)
  find_package(map_sdk_debug REQUIRED)
else()
  find_package(map_sdk REQUIRED)
endif()

# 获取 python_api 文件夹下的所有 .cpp 文件
file(GLOB PYTHON_API_SOURCES "${MAPSDKPY_ROOT}/python_api/*.cpp")

# 使用 pybind11_add_module 命令构建 Python 模块
pybind11_add_module(${PROJECT_NAME} MODULE ${PYTHON_API_SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
  ${MAPSDK_LIBRARYS}
  pybind11::embed pybind11::module pybind11::pybind11
)

# copy_mapsdk_deps
if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
  else()
    copy_mapsdk_deps(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
  endif()
endif()
