/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "server_map_cache/map_data_proxy.h"
#include <json/value.h>
#include <json/writer.h>
#include "server_map_cache/map_data_cache.h"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <thread>

#include "../engine/config.h"
#include "../engine/error_code.h"
#include "../engine/util/scene_util.h"
#include "../map_data/lane_boundary_data.h"
#include "../map_data/lane_data.h"
#include "../map_data/lane_link.h"
#include "../map_data/road_data.h"
#include "../xml_parser/entity/hadmap_params.h"
#include "../xml_parser/entity/parser_json.h"
#include "../xml_parser/entity/query_params.h"
#include "./map_data_proxy.h"
#include "common/engine/entity/hadmap.h"
#include "common/log/system_logger.h"
#include "common/map_data/map_object.h"
#include "common/third/gutil/integral_types.h"
#include "common/xml_parser/entity/mapfile.h"
#include "engine/error_code.h"
#include "gtest/gtest.h"
#include "server_hadmap/hadmap_scene_data.h"
#include "structs/map_structs.h"

TEST(MapDataProxyTest, t1) {
  CMapFile file;
  file.ParseMapV2("./Sim_Highway.xodr");
  EXPECT_TRUE(true);
}