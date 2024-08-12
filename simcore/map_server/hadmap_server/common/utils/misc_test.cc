/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include "misc.h"

#include "gtest/gtest.h"

namespace misc {

TEST(MiscTest, t1) {
  std::string raw(R"({"a":1})");
  Json::Value root;
  root << raw;
  root << R"({"a":1})";
  EXPECT_EQ(root.get("a", 0).asInt(), 1);
  std::string dst;
  dst << root;
  EXPECT_EQ(dst, raw);
}

TEST(MiscTest, TravelDir) {
  auto files = TravelDir("/home/hdserver/scene", false, "scene_btree_[0-9]{1,}.xml");
  EXPECT_NE(files, std::vector<std::string>{});
}

}  // namespace misc