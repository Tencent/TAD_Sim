// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <gtest/gtest.h>
#include <json/value.h>
#include <iostream>
#include <sstream>

#include "common/utils/misc.h"
#include "common/utils/scoped_cleanup.h"
#include "engine/config.h"
#include "server_hadmap/hadmap_scene_data.h"
#include "xml_parser/hadmap_entity/map_scene_v3.h"
#include "xml_parser/hadmap_entity/parser_hadmap_json_v3.h"

using namespace std;
using namespace hdserver;

// const static  string TEST_MAP = "svwmap_polygeo3_ver3.xodr";
const static string TEST_MAP = "111.xodr";
const static string TEST_MAP_NAST = "NASTway.xodr";

const static string TEST_JSON_PATH = "../test/data/self_made.json";

const static string TEST_JSON_THIRD_PATH = "../test/data/third.json";

void loadFile(const std::string& path, std::string& content) {
  std::ifstream fin;
  fin.open(path);
  EXPECT_EQ(fin.is_open(), true);
  SCOPED_CLEANUP({ fin.close(); });

  std::stringstream buffer;
  buffer << fin.rdbuf();

  content = buffer.str();
  // content.swap(buffer.str());
  return;
}

TEST(TestMapSceneData, getThirdD2D) {
  auto& ins = CHadmapSceneData::Instance();

  std::string bucket = "simulation-dev-1251316161";
  std::string cname = std::string("1632973527") + VERSION_NAME_DELIM + "d2d_20190726.xodr";
  std::wstring map = CEngineConfig::Instance().MBStr2WStr(cname.c_str());
  std::string cos = "test/map/d2d_20190726.xodr";
  std::string json = "";

  auto res = ins.GetHadmapSceneDataV3(map.c_str(), bucket, cos, json);
  // cout << "res of get hadmap scene: " << res << endl;
}

TEST(TestMapSceneData, getThird) {
  auto& ins = CHadmapSceneData::Instance();
  std::string bucket = "simulation-dev-1251316161";
  std::string cname = std::string("1632973527") + VERSION_NAME_DELIM + TEST_MAP_NAST;
  std::wstring map = CEngineConfig::Instance().MBStr2WStr(cname.c_str());
  std::string cos = "maps/144115205301725161/20043-61318699da5bae0007806990/NASTway.xodr";
  std::string json = "";

  auto res = ins.GetHadmapSceneDataV3(map.c_str(), bucket, cos, json);
  cout << "res of get hadmap scene: " << res << endl;
}

// 1632973527#NASTway.xodr

TEST(TestMapSceneData, parseMapToJson) {
  std::string map = "../test/data/1632973527#NASTway.xodr";
  auto& ins = CHadmapSceneData::Instance();
  std::string jstr;
  int res = ins.parseJsonFromOpendrive(map, jstr);
  EXPECT_EQ(res, HSEC_OK);
}

TEST(TestMapSceneData, get) {
  auto& ins = CHadmapSceneData::Instance();
  std::wstring map = CEngineConfig::Instance().MBStr2WStr(TEST_MAP_NAST.c_str());
  std::string bucket = "simulation-dev-1251316161";
  std::string cos = "/maps/editor/cdb0272a9bc748c485063a7934fe37da/23445.xodr.json";
  std::string json = "/maps/editor/cdb0272a9bc748c485063a7934fe37da/1573eb59ab914d6f99fdb80e4f3767db/23445.xodr_json";

  std::string res = ins.GetHadmapSceneDataV3(map.c_str(), bucket, cos, json);

  Json::Value data;
  misc::ParseJson(res.c_str(), &data, "Error parse old map json.");
  cout << "res of get hadmap scene: " << res << endl;
  // EXPECT_EQ(data["name"], "4321.xodr");
}

TEST(TestMapSceneData, loadJson) {
  std::string jstr;

  CParserHadmapJsonV3 jParser;

  loadFile(TEST_JSON_PATH, jstr);
  CMapSceneV3* pScene = jParser.ParseV3(jstr.c_str());
  EXPECT_EQ(false, pScene->getIsThird());
  // cout <<  "is third ?: " << pScene->getIsThird() <<endl;

  loadFile(TEST_JSON_THIRD_PATH, jstr);
  pScene = jParser.ParseV3(jstr.c_str());
  EXPECT_EQ(true, pScene->getIsThird());
}

TEST(TestMapSceneData, saveThird) {
  std::string jstr;
  loadFile(TEST_JSON_THIRD_PATH, jstr);

  auto& ins = CHadmapSceneData::Instance();
  // cout << "json is: " << jstr << endl;
  std::wstring wjstr = CEngineConfig::Instance().MBStr2WStr(jstr.c_str());
  std::string res = ins.SaveHadmapSceneDataV3(wjstr.c_str());

  cout << "res of get hadmap scene: " << res << endl;
}

TEST(TestMapSceneData, save) {
  std::string jstr;
  loadFile(TEST_JSON_PATH, jstr);

  auto& ins = CHadmapSceneData::Instance();
  // cout << "json is: " << jstr << endl;
  std::wstring wjstr = CEngineConfig::Instance().MBStr2WStr(jstr.c_str());
  auto res = ins.SaveHadmapSceneDataV3(wjstr.c_str());

  cout << "res of get hadmap scene: " << res << endl;
}
