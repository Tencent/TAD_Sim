/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/map_data/map_query.h"

#include <malloc.h>
#include <tinyxml2.h>
#include <chrono>
#include <thread>

#include "OpenDriveStruct.h"
#include "gtest/gtest.h"
#include "json/json.h"

#include "common/utils/misc.h"

class MapQueryTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {}
};

TEST_F(MapQueryTest, SearchPath) {
  CMapQuery map_query;
  map_query.SetMapPath("./hdserver/cloud/hdserver/test/data/1675040210#map.xodr");
  map_query.Init();

  auto gen_from_to = [&](const char* input) -> std::vector<hadmap::txPoint> {
    Json::Value root = StringToJson(input);
    std::vector<hadmap::txPoint> from_to;
    for (auto&& point : root.get("via_points", Json::Value(Json::arrayValue))) {
      from_to.emplace_back(hadmap::txPoint(point["x"].asDouble(), point["y"].asDouble(), point["z"].asDouble()));
    }
    return from_to;
  };

  std::thread t1([&]() {
    hadmap::PointVec geoms;
    map_query.SearchPath(gen_from_to(R"(
        {"name":"map.xodr","version":"1675040210","via_points":[{"x":118.510058,"y":0.000883,"z":0},
        {"x":118.51134804958457,"y":0.0009344950527478215,"z":0.0002819914370775223}]}
      )"),
                         geoms);
    EXPECT_EQ(geoms.size(), 22);
  });

  {
    hadmap::PointVec geoms;
    map_query.SearchPath(gen_from_to(R"(
        {"name":"map.xodr","version":"1675040210","via_points":[{"x":118.510058,"y":0.000883,"z":0},
        {"x":118.5107647318565,"y":0.0020043881532363196,"z":0.0010228808969259262}]}
      )"),
                         geoms);
    EXPECT_EQ(geoms.size(), 42);
  }

  {
    hadmap::PointVec geoms;
    map_query.SearchPath(gen_from_to(R"(
        {"name":"map.xodr","version":"1675040210","via_points":[{"x":118.510058,"y":0.000883,"z":0},
        {"x":118.51059790827156,"y":0.00016316991924517635,"z":0.000681338831782341}]}
      )"),
                         geoms);
    EXPECT_EQ(geoms.size(), 37);
  }

  t1.join();
  map_query.Deinit();
}

TEST_F(MapQueryTest, MemCheck) {
  CMapQuery map_query;
  map_query.SetMapPath(
      "./hdserver/cloud/hdserver/test/data/NIO-20220729-beijing-0.00km-QUANGUO-D1.0_txf_itrf_odd_spd_02_NoZ.xodr");
  // map_query.SetMapPath("./hdserver/cloud/hdserver/test/data/1.0#20596511.xodr");
  map_query.Init();
  map_query.Deinit();
  // malloc_trim(0);
  std::this_thread::sleep_for(std::chrono::seconds(10));
}

TEST_F(MapQueryTest, MapSDK_ODGeomSpiral) {
  // CMapQuery map_query;
  // map_query.SetMapPath("./hdserver/cloud/hdserver/test/data/Accident_701.xodr");
  // map_query.Init();
  // map_query.Deinit();

  tinyxml2::XMLDocument doc;
  doc.Parse(R"(
    <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
    <geometry s="4.8087286281831744e+01" x="4.6593226348704775e+01" y="4.9804077471475466e+01" 
    hdg="-2.7258762810806560e+00" length="1.4625378187858995e+01">
      <spiral curvStart="9.4822653599993753e-06" curvEnd="0.0000000000000000e+00"/>
    </geometry>
    <geometry s="1.0185078443663429e+01" x="8.1269427258847344e+01" y="6.5105661141636062e+01" 
    hdg="-2.7258851527509904e+00" length="1.9914354147188007e-04">
      <spiral curvStart="-2.0000000000000000e+00" curvEnd="-0.0000000000000000e+00"/>
    </geometry>
  )");
  hadmap::ODGeomPtr geom = std::make_shared<hadmap::ODGeomSpiral>();
  geom->parse(doc.FirstChildElement("geometry"));
  double s = 4.8087286281831744e+01, x = 0, y = 0;
  geom->getGeom(s, x, y);
  std::cout << x << ", " << y << std::endl;
}

TEST_F(MapQueryTest, MapSDK_UseShm) {
  CMapQuery map_query;
  map_query.SetMapPath("./hdserver/cloud/hdserver/test/data/Accident_701.xodr");
  map_query.Init();
  // map_query.Deinit();

  // constexpr std::string shm_key = "MapSDK_";
  // const key_t key = std::hash<std::string>{}(shm_key);
  // int shmid = shmget(key, size(), IPC_CREAT | 0666);

  auto handler = map_query.GettxMapHandle();
  ASSERT_TRUE(handler != nullptr);
  // std::cout << sizeof(handler) << std::endl;
  // std::cout << sizeof(hadmap::txMapHandle) << std::endl;
}