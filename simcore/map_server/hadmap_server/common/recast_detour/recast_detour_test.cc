
/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "recast_detour.h"

#include "gtest/gtest.h"

namespace recast_detour {

class RecastDetourTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {}
};

TEST_F(RecastDetourTest, loadMesh) {
  InputGeom geom;
  rcContext ctx;
  geom.load(&ctx, "hdserver/cloud/hdserver/test/data/nav_test.obj");
}

TEST_F(RecastDetourTest, handleBuild) {
  BuildContext ctx;

  InputGeom geom;
  geom.load(&ctx, "hdserver/cloud/hdserver/test/data/nav_test.obj");

  Sample* sample = new Sample_SoloMesh();
  sample->setContext(&ctx);
  sample->handleMeshChanged(&geom);
  EXPECT_TRUE(sample->handleBuild());
}

TEST_F(RecastDetourTest, nlohmannJson) {
  BuildSettings settings;

  nlohmann::json settings_json;
  to_json(settings_json, settings);

  // settings_json = R"()"_json;
  // from_json(settings_json, settings);

  settings_json =
      R"({"cellSize":0.3,
      "cellHeight":0.2,
      "agentHeight":2.0,
      "agentRadius":0.6,
      "agentMaxClimb":0.9,
      "agentMaxSlope":45.0})"_json;
  from_json(settings_json, settings);
  to_json(settings_json, settings);
  EXPECT_FLOAT_EQ(settings.cellSize, 0.3);
}

TEST_F(RecastDetourTest, BuildSettings) {
  Sample* sample = new Sample_SoloMesh();
  BuildSettings s1;
  sample->collectSettings(s1);

  nlohmann::json s1_json;
  to_json(s1_json, s1);
  std::cout << s1_json << std::endl;
}

TEST_F(RecastDetourTest, ParseEventSettings) {
  {
    EventSettings settings;
    ParseEventSettings(R"()", settings);
    EXPECT_TRUE(settings.fds.empty());
  }

  {
    EventSettings settings;
    ParseEventSettings(R"([])", settings);
    EXPECT_TRUE(settings.fds.empty());
  }

  {
    EventSettings settings;
    ParseEventSettings(R"( [{"cost":999,"id":"10","fds":["0-1-0","0-2-0"]}] )", settings);
    EXPECT_EQ(settings.fds.size(), 1);
    EXPECT_EQ(settings.fds[0].id, 10);
    EXPECT_EQ(settings.fds[0].cost, 999);
    std::map<int, std::set<int>> expected_tile_poly_idx{{0, {1, 2}}};
    EXPECT_EQ(settings.fds[0].tile_poly_idx, expected_tile_poly_idx);
  }
}

TEST_F(RecastDetourTest, BuildNavMesh) {
  EXPECT_TRUE(BuildNavMesh("hdserver/cloud/hdserver/test/data/nav_test.obj"));

  std::string build_settings = R"({"cellSize":1,"cellHeight":1})";
  EXPECT_TRUE(BuildNavMesh("hdserver/cloud/hdserver/test/data/nav_test.obj", build_settings.c_str()));

  std::string event_settings = R"([{"cost":999,"id":"10","fds":["0-1-0","0-2-0"],"timeRange":[[0,300]]}])";
  EXPECT_TRUE(
      BuildNavMesh("hdserver/cloud/hdserver/test/data/nav_test.obj", build_settings.c_str(), event_settings.c_str()));
}

}  // namespace recast_detour