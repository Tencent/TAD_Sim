// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "engine/config.h"
#include "log/log.h"

using namespace std;

TEST(testConfig, test) {
  cout << "test config " << endl;
  CLog::Initialize("./scenario");
  CEngineConfig& ins = CEngineConfig::Instance();
  ins.Init("./scenario", "./app");
  auto path = ins.ResourceDir();
  cout << "resource dir is: " << path << std::endl;
  cout << "hadmap dir is : " << CEngineConfig::Instance().HadmapDir() << endl;
}
