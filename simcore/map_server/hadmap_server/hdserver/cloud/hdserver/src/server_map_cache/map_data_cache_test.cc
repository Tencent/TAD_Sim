/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "map_data_cache.h"

#include <json/value.h>
#include <json/writer.h>
#include "gtest/gtest.h"

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
#include "server_hadmap/hadmap_scene_data.h"
#include "structs/map_structs.h"

class MapDataCacheTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    CEngineConfig::Instance().Init("/home/hdserver", "/home/hdserver");
    EXPECT_STREQ(CEngineConfig::Instance().AppInstallDir(), "/home/hdserver");
    EXPECT_STREQ(CEngineConfig::Instance().ResourceDir(), "/home/hdserver");

    CMapDataCache::Instance().Init();
  }
};

TEST_F(MapDataCacheTest, LoadHadmap) {
  CMapDataCache::Instance().LoadHadMap(
      CEngineConfig::Instance().MBStr2WStr("apollo_vectormap_G3_12141742_sp_re.xml").c_str());
}

TEST_F(MapDataCacheTest, SearchPath) {
  // {"name":"map.xodr","version":"1675040210","via_points":[{"x":118.510058,"y":0.000883,"z":0},
  // {"x":118.51134804958457,"y":0.0009344950527478215,"z":0.0002819914370775223}]}
  // {"name":"map.xodr","version":"1675040210","via_points":[{"x":118.510058,"y":0.000883,"z":0},
  // {"x":118.5107647318565,"y":0.0020043881532363196,"z":0.0010228808969259262}]}
  // {"name":"map.xodr","version":"1675040210","via_points":[{"x":118.510058,"y":0.000883,"z":0},
  // {"x":118.51059790827156,"y":0.00016316991924517635,"z":0.000681338831782341}]}
  CMapDataCache::Instance().LoadHadMap(CEngineConfig::Instance().MBStr2WStr("1675040210#map.xodr").c_str());

  std::thread t1([&]() {
    CMapDataCache::Instance().SearchPathImpl(R"(
      {"name":"map.xodr","version":"1675040210","via_points":[{"x":118.510058,"y":0.000883,"z":0},
      {"x":118.51134804958457,"y":0.0009344950527478215,"z":0.0002819914370775223}]}
    )");
  });

  std::thread t2([&]() {
    CMapDataCache::Instance().SearchPathImpl(R"(
      {"name":"map.xodr","version":"1675040210","via_points":[{"x":118.510058,"y":0.000883,"z":0},
      {"x":118.51134804958457,"y":0.0009344950527478215,"z":0.0002819914370775223}]}
    )");
  });

  std::thread t3([&]() {
    CMapDataCache::Instance().SearchPathImpl(R"(
      {"name":"map.xodr","version":"1675040210","via_points":[{"x":118.510058,"y":0.000883,"z":0},
      {"x":118.51134804958457,"y":0.0009344950527478215,"z":0.0002819914370775223}]}
    )");
  });
  t1.join();
  t2.join();
  t3.join();
}