// Copyright 2021 Tencent Inc.  All rights reserved.
//
// Author: sbrucezhang@tencent.com (sbrucezhang)
//

#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "../scene_wrapper_linux/scene_wrapper.h"
#include "engine/error_code.h"

#include "engine/config.h"
#include "log/log.h"

using namespace std;

TEST(testSceneWrapper, testLoadHadmap) {
  cout << " test scene wrapper interface" << endl;

  string version = "1632973527";
  string name = "sihuan.sqlite";

  string res = loadHadmap(name.c_str(), version.c_str());
  cout << "loadHadmap res: " << res << endl;
  cout << " test scene wrapper interface done. " << endl;
}

TEST(testSceneWrapper, testQueryRoad) {
  std::this_thread::sleep_for(std::chrono::seconds(10));
  cout << " test scene wrapper interface get Road" << endl;

  string version = "1632973527";
  string name = "sihuan.sqlite";

  string res = getRoadData(name.c_str(), version.c_str());
  cout << "loadHadmap res: " << res << endl;
  cout << " test scene wrapper interface road done. " << endl;
}

TEST(testSceneWrapper, testQueryLane) {
  std::this_thread::sleep_for(std::chrono::seconds(10));
  cout << " test scene wrapper interface get lane" << endl;

  string version = "1632973527";
  string name = "sihuan.sqlite";

  string res = getRoadData(name.c_str(), version.c_str());
  cout << "loadHadmap res: " << res << endl;
  cout << " test scene wrapper interface  lane done. " << endl;
}

TEST(testSceneWrapper, testQueryLaneBoundary) {
  std::this_thread::sleep_for(std::chrono::seconds(10));
  cout << " test scene wrapper interface get lane boundary" << endl;

  string version = "1632973527";
  string name = "sihuan.sqlite";

  string res = getRoadData(name.c_str(), version.c_str());
  cout << "loadHadmap res: " << res << endl;
  cout << " test scene wrapper interface  lane boundary done. " << endl;
}

TEST(testSceneWrapper, testQueryLaneLink) {
  std::this_thread::sleep_for(std::chrono::seconds(10));
  cout << " test scene wrapper interface get lane link" << endl;

  string version = "1632973527";
  string name = "sihuan.sqlite";

  string res = getRoadData(name.c_str(), version.c_str());
  cout << "loadHadmap res: " << res << endl;
  cout << " test scene wrapper interface  lane link done. " << endl;
}

TEST(testSceneWrapper, testQueryObj) {
  std::this_thread::sleep_for(std::chrono::seconds(10));
  cout << " test scene wrapper interface get map obj" << endl;

  string version = "1632973527";
  string name = "sihuan.sqlite";

  string res = getRoadData(name.c_str(), version.c_str());
  cout << "loadHadmap res: " << res << endl;
  cout << " test scene wrapper interface  map obj done. " << endl;
}
/*




SCENE_WRAPPER_API const char* getLaneLinkData(const char* strMapName, const char* verson = "");
SCENE_WRAPPER_API const char* getMapObjectData(const char* strMapName, const char* verson = "");

*/
