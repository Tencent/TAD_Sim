/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include <gtest/gtest.h>
#include <json/value.h>
#include <json/writer.h>
#include <iostream>
#include "engine/config.h"

#include "server_map_cache/map_data_cache.h"
using namespace std;

TEST(testMapCacheCase, testNormal) {
  std::string req("xxx.sqlite");
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(req.c_str());

  std::wstring status = CMapDataCache::Instance().LoadHadMap(wstrParams.c_str());
  // EXPECT_EQ(status, CMapDataCache::m_wstrSuccess);

  // target.erase(std::remove(target.begin(), target.end(), ' '),
  // target.end());

  // EXPECT_EQ(target, strRes);
  // EXPECT_EQ(add(2, 3), 5);
}