set(FRAMEWORK_PATH ${CMAKE_CURRENT_LIST_DIR}/../framework)
if(NOT EXISTS ${FRAMEWORK_PATH}/build/lib)
message(STATUS "build framework")
if(WIN32) 
set(cmd build.bat)
else()
set(cmd ./build.sh)
endif()
execute_process(
  WORKING_DIRECTORY ${FRAMEWORK_PATH}
  COMMAND ${cmd}
  COMMAND_ERROR_IS_FATAL ANY
  RESULT_VARIABLE build_result
  ERROR_QUIET
)
if(NOT build_result EQUAL 0)
  message(FATAL_ERROR "build framework failed")
endif()
endif()

if(NOT FRAMEWORK_INCLUDE_DIR)
  find_path(FRAMEWORK_INCLUDE_DIR
    NAMES txsim_module.h PATHS ${FRAMEWORK_PATH}/src
    DOC "framework include directory")
endif()

if(NOT FRAMEWORK_LIBRARYS)
  if(WIN32) 
  set(BUILD_PATH build/lib/Release)
  else()
  set(BUILD_PATH build/lib)
  endif()
  set(FRAMEWORK_LIBPATH ${FRAMEWORK_PATH}/${BUILD_PATH})
  find_library(FRAMEWORK_LIBRARY_SERVICE
    NAMES txsim-module-service PATHS ${FRAMEWORK_LIBPATH})
  list(APPEND FRAMEWORK_LIBRARYS ${FRAMEWORK_LIBRARY_SERVICE})
  find_library(FRAMEWORK_LIBRARY_COORDINATOR
    NAMES txsim-coordinator PATHS ${FRAMEWORK_LIBPATH})
  list(APPEND FRAMEWORK_LIBRARYS ${FRAMEWORK_LIBRARY_COORDINATOR})
endif()


#message("FRAMEWORK_LIBRARYS: ${FRAMEWORK_LIBRARYS}")
#message("FRAMEWORK_INCLUDE_DIR: ${FRAMEWORK_INCLUDE_DIR}")
include_directories(${FRAMEWORK_INCLUDE_DIR})

function(copy_framework_deps destdir)
if(TARGET copy_framework_dep_files)
message("copy_framework_deps has been called.")
else()
message("copy copy_framework_deps deps to ${destdir}")
file(MAKE_DIRECTORY ${destdir})
if(WIN32) 
file(GLOB DEP_FILES  ${FRAMEWORK_PATH}/build/bin/Release/*.dll)
else()
file(GLOB DEP_FILES  ${FRAMEWORK_PATH}/build/bin/*.*)
endif()
add_custom_target(copy_framework_dep_files ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${DEP_FILES} "${destdir}")
endif()
endfunction()
