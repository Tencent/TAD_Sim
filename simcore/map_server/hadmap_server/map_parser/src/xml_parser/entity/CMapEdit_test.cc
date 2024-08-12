/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include "CMapEdit.h"

#include <fstream>
#include <sstream>

#include "common/utils/misc.h"
#include "engine/config.h"
#include "gtest/gtest.h"
#include "server_map_cache/map_data_cache.h"

class CMapEditTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    CEngineConfig::Instance().Init("/home/hdserver", "/home/hdserver");
    EXPECT_STREQ(CEngineConfig::Instance().AppInstallDir(), "/home/hdserver");
    EXPECT_STREQ(CEngineConfig::Instance().ResourceDir(), "/home/hdserver");

    CMapDataCache::Instance().Init();
    // CDatabase::Instance().Init();
    CMapEdit::Instance().Init("/home/hdserver");
  }
};

TEST_F(CMapEditTest, CreateSession) {
  std::string res = CEngineConfig::Instance().WStr2MBStr(CMapEdit::Instance().CreateSession());
  Json::Value root = StringToJson(res);
  EXPECT_EQ(root.get("message", "").asString(), "ok") << res;
}

TEST_F(CMapEditTest, CreateNewHadmap) {
  std::string res = CEngineConfig::Instance().WStr2MBStr(CMapEdit::Instance().CreateSession());
  Json::Value root = StringToJson(res);
  auto sid = root.get("sessionId", 0).asInt();
  ASSERT_GE(sid, 0) << res;

  std::ifstream ifs("./test/data/create_hadmap.json");
  std::stringstream ss;
  ss << ifs.rdbuf();
  root = StringToJson(ss.str());
  root["sessionid"] = sid;

  std::string input = JsonToString(root);
  ASSERT_FALSE(input.empty()) << input;
  std::wstring winput = CEngineConfig::Instance().MBStr2WStr(input.c_str());

  CMapEdit::Instance().CreateNewHadmap(winput.c_str());
  // EXPECT_EQ(CEngineConfig::Instance().WStr2MBStr(output.c_str()), std::string(""));
}