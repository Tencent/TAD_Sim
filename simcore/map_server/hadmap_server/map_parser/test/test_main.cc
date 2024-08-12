// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <gflags/gflags.h>
#include <iostream>
#include "common/log/log.h"
#include "engine/config.h"
#include "gtest/gtest.h"
using namespace std;

int main(int argc, char** argv) {
  cout << "test main begin." << std::endl;

  //  do init.
  CLog::Initialize("./scenario");
  CEngineConfig& ins = CEngineConfig::Instance();
  ins.Init("./scenario", "./app");

  ::testing::InitGoogleTest(&argc, argv);

  google::ParseCommandLineFlags(&argc, &argv, false);
  return RUN_ALL_TESTS();
}
