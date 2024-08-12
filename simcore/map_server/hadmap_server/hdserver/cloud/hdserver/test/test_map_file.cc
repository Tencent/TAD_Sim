// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <iostream>
#include <string>
#include "common/xml_parser/entity/mapfile.h"
#include "engine/entity/aabb.h"
#include "engine/error_code.h"
#include "gtest/gtest.h"

using namespace std;

TEST(testMapFile, parse) {
  cout << "test parse map file " << endl;

  CMapFile mf;

  string path = "/data/hadmap_server/1004-1-016-201020.sqlite";
  int ret = mf.ParseMapV2(path.c_str());
  EXPECT_EQ(ret, HSEC_OK);

  CVector3 mi(mf.m_envelop_min[0], mf.m_envelop_min[1], mf.m_envelop_min[2]);
  CVector3 ma(mf.m_envelop_max[0], mf.m_envelop_max[1], mf.m_envelop_max[2]);
  CAABB abbl(mi, ma);

  auto min = abbl.Min().ToString();
  auto max = abbl.Max().ToString();

  cout << "min is: " << min << endl;
  cout << "max is: " << max << endl;
  /*

  CVector3 mi(mf.m_envelop_min[0],mf.m_envelop_min[1],mf.m_envelop_min[2]);
  CVector3 ma(mf.m_envelop_max[0], mf.m_envelop_max[1],mf.m_envelop_max[2]);
  CAABB aabbLl(mi,ma);

  */

  cout << "test parse map file done." << endl;
}
