// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "common/xml_parser/entity/mapfile.h"
#include "engine/error_code.h"
using namespace std;

TEST(testHadmap, load) {
  cout << "load hadmap begin " << endl;
  std::string map = "../test/data/实虚线_modify_by_yc.xodr ";

  CMapFile mapfile;
  int nRet = mapfile.ParseMapV2(map.c_str());
  EXPECT_EQ(nRet, HSEC_Error);

  map = "../test/data/实虚线_modify_by_yc.xodr";
  nRet = mapfile.ParseMapV2(map.c_str());
  EXPECT_EQ(nRet, HSEC_OK);

  map = "../test/data/111.xodr";
  nRet = mapfile.ParseMapV2(map.c_str());
  EXPECT_EQ(nRet, HSEC_Error);

  cout << "map is: " << map << endl;
  cout << "load hadmap end" << endl;
}
