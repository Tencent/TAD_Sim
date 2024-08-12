/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "scene_data.h"

#include <xercesc/util/PlatformUtils.hpp>
#include "../database/entity/scenario.h"
#include "../engine/config.h"
#include "../xml_parser/entity/parser_json.h"
#include "common/utils/xsd_validator.h"

#include "gtest/gtest.h"

class SceneDataTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    CEngineConfig::Instance().Init("/home/hdserver", "/home/hdserver");
    xercesc::XMLPlatformUtils::Initialize();
    EXPECT_STREQ(CEngineConfig::Instance().AppInstallDir(), "/home/hdserver");
    EXPECT_STREQ(CEngineConfig::Instance().ResourceDir(), "/home/hdserver");
    utils::XsdValidator::Instance().Initialize("/home/hdserver/OpenX");
  }
};

TEST_F(SceneDataTest, xosc2sim) {
  sTagEntityScenario scenario;
  scenario.m_strName = "test_dd.xosc";
  scenario.m_strMapName = "map_0621_sign.xodr";
  scenario.m_strMapVersion = "";
  CParserJson{}.XoscToSim(CEngineConfig::Instance().ResourceDir(), "scene", scenario);
}

TEST_F(SceneDataTest, xml2json) {
  sTagEntityScenario scenario;
  scenario.m_strName = "scenedd.sim";
  scenario.m_strMapName = "apollo_vectormap_G3_12141742_sp_re.xml";
  scenario.m_strMapVersion = "1.0";
  auto res = CParserJson{}.XmlToJson(CEngineConfig::Instance().ResourceDir(), "scene", scenario);
  std::cout << res << std::endl;
}