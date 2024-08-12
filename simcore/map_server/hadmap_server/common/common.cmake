
#[[

# common/log
#include\s{0,}".*?/log/(system|xml)_logger.h"  #include "common/log/$1_logger.h"

# common/third/gutil
#include\s{0,}".*?gutil/(.*)"  #include "common/third/gutil/$1"

# common/utils
#include\s{0,}".*?utils/(.*)"  #include "common/utils/$1"

# common/xml_parser/entity
#include\s{0,}".*?(parser_tools|scene_event|traffic|vehicle).h"  #include "common/xml_parser/entity/$1.h"
#include\s{0,}".*?environments/(.*)" #include "common/xml_parser/entity/environments/$1"
#include\s{0,}".*?event_node.h"  #include "common/xml_parser/entity/event_node.h"
#include\s{0,}".*?(SensorV2|catalog).h"  #include "common/xml_parser/entity/$1.h"
#include\s{0,}".*?(pedestrian_event|pedestrian|pedestrianV2|velocity_param|velocity).h"  #include "common/xml_parser/entity/$1.h"
#include\s{0,}".*?(merge|merge_param).h"  #include "common/xml_parser/entity/$1.h"
#include\s{0,}".*?(signlight).h"  #include "common/xml_parser/entity/$1.h"
#include\s{0,}".*?(mapfile).h"  #include "common/xml_parser/entity/$1.h"
#include\s{0,}".*?(obstacle).h"  #include "common/xml_parser/entity/$1.h"
#include\s{0,}".*?(route).h"  #include "common/xml_parser/entity/$1.h"

# common/xml_parser/xosc
#include\s{0,}".*?xosc_(base|writer|reader|converter)(.*)"  #include "common/xml_parser/xosc/xosc_$1$2"

# common/xml_parser/xosc_replay
#include\s{0,}".*?xosc_replay/(.*)"  #include "common/xml_parser/xosc_replay/$1"

# common/map_data
#include\s{0,}".*?map_data_predef.h"  #include "common/map_data/map_data_predef.h"
#include\s{0,}".*?map_object.h"  #include "common/map_data/map_object.h"
#include\s{0,}".*?map_query.h"  #include "common/map_data/map_query.h"

# common/engine
#include\s{0,}".*?/math/(.*)"  #include "common/engine/math/$1"
#include\s{0,}".*?/common.h"  #include "common/engine/common.h"
#include\s{0,}".*?/constant.h"  #include "common/engine/constant.h"
#include\s{0,}".*?(hadmap).h"  #include "common/engine/entity/$1.h"

]]

file(GLOB_RECURSE COMMON_TEST
    ${ROOT_DIR}/common/**/*_test.c*
    )

file(GLOB_RECURSE COMMON_SRCS
    ${ROOT_DIR}/common/drone_adaptor/*
    ${ROOT_DIR}/common/engine/*
    ${ROOT_DIR}/common/third/gutil/*
    ${ROOT_DIR}/common/log/*
    ${ROOT_DIR}/common/map_data/*
    ${ROOT_DIR}/common/recast_detour/*
    ${ROOT_DIR}/common/utils/*
    ${ROOT_DIR}/common/xml_parser/*
    )

# filter
file(GLOB_RECURSE DELETE_FILE 
    ${ROOT_DIR}/common/third/gutil/*.cc
    ${ROOT_DIR}/common/utils/curl_utils.*
    ${ROOT_DIR}/common/utils/faststring.*
    ${ROOT_DIR}/common/utils/scoped_cleanup.cc
    )

if("${HADMAP_SVR_BUILD_ONLY}" STREQUAL "cloud")
else()
    file(GLOB_RECURSE EXCLUDED_FILES
        ${ROOT_DIR}/common/recast_detour/*
        ${ROOT_DIR}/common/drone_adaptor/*
        )
    list(REMOVE_ITEM COMMON_SRCS ${EXCLUDED_FILES})
    list(REMOVE_ITEM COMMON_TEST ${EXCLUDED_FILES})
endif()

list(REMOVE_ITEM COMMON_SRCS ${DELETE_FILE} ${COMMON_TEST})

source_group(TREE ${ROOT_DIR} FILES ${COMMON_SRCS})

include_directories(${ROOT_DIR})
