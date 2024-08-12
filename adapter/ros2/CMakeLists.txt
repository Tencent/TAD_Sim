cmake_minimum_required(VERSION 3.0.2)
project(roscpp_tutorials)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall")

# Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../simcore/cmake")
find_package(framework REQUIRED)

find_package(catkin REQUIRED COMPONENTS message_generation rostime roscpp rosconsole roscpp_serialization)

include_directories(${catkin_INCLUDE_DIRS})
link_directories(${catkin_LIBRARY_DIRS})

catkin_package(CATKIN_DEPENDS message_runtime std_msgs)

include_directories(${catkin_INCLUDE_DIRS})
add_executable(listener my_module.cc my_module_main.cc)
target_link_libraries(listener ${catkin_LIBRARIES} ${FRAMEWORK_LIBRARY_SERVICE})
install(TARGETS listener
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

if(WIN32)
  if(NOT CMAKE_BUILD_TYPE)
    copy_framework_deps(./Release)
  else()
    copy_framework_deps(./${CMAKE_BUILD_TYPE})
  endif()
endif()
