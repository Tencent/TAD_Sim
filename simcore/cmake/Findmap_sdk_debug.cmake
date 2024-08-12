get_filename_component(MAPSDK_PATH ${CMAKE_CURRENT_LIST_DIR}/../../common/map_sdk ABSOLUTE)
if(NOT EXISTS ${MAPSDK_PATH}/hadmap-debug)
message(STATUS "build map_sdk")
if(WIN32) 
  set(cmd build-debug.bat)
else()
  set(cmd ./build.sh)
endif()
execute_process(
  WORKING_DIRECTORY ${MAPSDK_PATH}
  COMMAND ${cmd}
  COMMAND_ERROR_IS_FATAL ANY
  RESULT_VARIABLE build_result
  ERROR_QUIET
)
if(NOT build_result EQUAL 0)
  message(FATAL_ERROR "build map_sdk failed")
endif()
endif()

if(NOT MAPSDK_INCLUDE_DIR)
  find_path(MAPSDK_INCLUDE_DIR
    NAMES hadmap.h PATHS ${MAPSDK_PATH}/hadmap-debug/include
    DOC "map_sdk include directory")
endif()
if(NOT MAPSDK_LIBRARYS)
  set(MAPSDK_LIBPATH ${MAPSDK_PATH}/hadmap-debug/lib)
  find_library(MAPSDK_LIBRARY_DATAMODEL
    NAMES datamodel PATHS ${MAPSDK_LIBPATH})
  list(APPEND MAPSDK_LIBRARYS ${MAPSDK_LIBRARY_DATAMODEL})
  find_library(MAPSDK_LIBRARY_MAPENGINE
    NAMES mapengine PATHS ${MAPSDK_LIBPATH})
  list(APPEND MAPSDK_LIBRARYS ${MAPSDK_LIBRARY_MAPENGINE})
  find_library(MAPSDK_LIBRARY_TRANSMISSION
    NAMES transmission PATHS ${MAPSDK_LIBPATH})
  list(APPEND MAPSDK_LIBRARYS ${MAPSDK_LIBRARY_TRANSMISSION})
  find_library(MAPSDK_LIBRARY_MAPDB
    NAMES mapdb PATHS ${MAPSDK_LIBPATH})
  list(APPEND MAPSDK_LIBRARYS ${MAPSDK_LIBRARY_MAPDB})
  find_library(MAPSDK_LIBRARY_MAPIMPORT
    NAMES mapimport PATHS ${MAPSDK_LIBPATH})
  list(APPEND MAPSDK_LIBRARYS ${MAPSDK_LIBRARY_MAPIMPORT})
  find_library(MAPSDK_LIBRARY_ROUTINGMAP
    NAMES routingmap PATHS ${MAPSDK_LIBPATH})
  list(APPEND MAPSDK_LIBRARYS ${MAPSDK_LIBRARY_ROUTINGMAP})
  find_library(MAPSDK_LIBRARY_ROUTINPLAN
    NAMES routeplan PATHS ${MAPSDK_LIBPATH})
  list(APPEND MAPSDK_LIBRARYS ${MAPSDK_LIBRARY_ROUTINPLAN})

endif()


#message("MAPSDK_LIBRARYS: ${MAPSDK_LIBRARYS}")
#message("MAPSDK_INCLUDE_DIR: ${MAPSDK_INCLUDE_DIR}")
include_directories(${MAPSDK_INCLUDE_DIR})

function(copy_mapsdk_deps destdir)
if(TARGET copy_mapsdk_dep_files)
message("copy_mapsdk_dep has been called.")
else()
message("copy mapsdk deps to ${destdir}")
file(MAKE_DIRECTORY ${destdir})
if(WIN32) 
file(GLOB DEP_FILES  ${MAPSDK_PATH}/hadmap-debug/bin/*)
else()
file(GLOB DEP_FILES  ${MAPSDK_PATH}/hadmap-debug/lib/*)
endif()
add_custom_target(copy_mapsdk_dep_files ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${DEP_FILES} "${destdir}")
endif()
endfunction()
