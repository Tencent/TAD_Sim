// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <fstream>
#include <sstream>
#include <string>
#include "engine/error_code.h"
#include "gtest/gtest.h"
#include "server_map_cache/map_data_proxy.h"

const static std::string TEST_JSON_PATH = "../test/data/self_made.json";

TEST(testCos, uploadString) {
  std::string bucket = "simulation-dev-1251316161";
  std::ifstream fin;
  fin.open(TEST_JSON_PATH);

  std::stringstream buffer;
  buffer << fin.rdbuf();
  std::string jstr(buffer.str());

  std::string key = "yyyyy/1f0d13d4240349059a99e61de90c6edf";

  int res = CMapDataProxy::Instance().uploadStr(bucket, key, jstr);
  EXPECT_EQ(res, HSEC_OK);
}
