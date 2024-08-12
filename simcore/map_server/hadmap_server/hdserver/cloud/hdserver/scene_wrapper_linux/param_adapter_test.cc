/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "scene_wrapper_linux/param_adapter.h"

#include "gtest/gtest.h"

#include "common/engine/entity/hadmap.h"

TEST(ParamAdapterTest, ScenarioOptionsAdapter) {
  Json::Value raw_param;
  raw_param << R"(
    {
      "id":1500092342,
      "name":"batch_sim_20221212140818_0.0",
      "bucket":"x8v-drivingcloud-xtest",
      "key":"/scene/tx_testscene/worldsim_scene0/",
      "mapName":"Sim_Highway",
      "mapVersion":"1.0"
    }
  )";
  {
    Json::Value param = raw_param;
    param["adsUrl"] = "xxx/yyy/batch_sim_20221212140818_0.0.sim";
    param["mapUrl"] = "xxx/yyy/Sim_Highway.xodr";
    Json::Value root;
    root << adapter::ScenarioOptionsAdapter(param.toStyledString().c_str());
    EXPECT_EQ(root.get("name", "").asString(), "batch_sim_20221212140818_0.0.sim");
    EXPECT_EQ(root.get("mapName", "").asString(), "Sim_Highway.xodr");
  }

  {
    Json::Value param = raw_param;
    param["key"] = Json::Value(Json::nullValue);
    param["version"] = "1.0";
    Json::Value root;
    root << adapter::ScenarioOptionsAdapter(param.toStyledString().c_str());
    EXPECT_EQ(root.get("key", "").asString(), "scenes/editor/1.0/");
  }

  {
    Json::Value param = raw_param;
    param["name"] = "CCRs_Overlap 75_VUT60.xosc";
    Json::Value root;
    root << adapter::ScenarioOptionsAdapter(param.toStyledString().c_str());
    EXPECT_EQ(root.get("name", "").asString(), "CCRs_Overlap+75_VUT60.xosc");
  }
}
