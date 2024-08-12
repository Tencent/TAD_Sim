/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "xosc_base_1_0.h"

#include "boost/algorithm/string.hpp"
#include "gtest/gtest.h"

class XOSCBase_1_0Test : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    xercesc::XMLPlatformUtils::Initialize();
  }
};

TEST_F(XOSCBase_1_0Test, ParameterDeclarationHandler) {
  XOSCBase_1_0::ParameterDeclarationPreprocess("./hdserver/cloud/hdserver/test/data/SAIMU.xosc.bak");
}

TEST_F(XOSCBase_1_0Test, ProjAPI) {
  EXPECT_FALSE(pj_is_latlong(pj_init_plus("+proj=utm")));
  EXPECT_FALSE(pj_is_latlong(pj_init_plus("+proj=merc")));

  for (auto&& proj :
       std::vector<std::string>{"", "+proj", "+proj=latlong", "+proj=longlat",
                                "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs", "+proj=latlong +ellps=clrk66"}) {
    EXPECT_TRUE(pj_is_latlong(pj_init_plus(proj.c_str())));

    EXPECT_TRUE(XOSCBase_1_0::IsLatLongPj(proj));
  }
}