// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <gflags/gflags.h>
#include <iostream>
#include "engine/config.h"
#include "gtest/gtest.h"
#include "log/log.h"

#include "../scene_wrapper_linux/scene_wrapper.h"
using namespace std;
int main(int argc, char** argv) {
  cout << "test main begin." << std::endl;

  ::testing::InitGoogleTest(&argc, argv);

  google::ParseCommandLineFlags(&argc, &argv, false);

  init("./scenario", "./app");
  //  do init.
  /* CLog::Initialize("./scenario");
   CEngineConfig& ins = CEngineConfig::Instance();
   ins.Init("./scenario", "./app");

*/

  return RUN_ALL_TESTS();
}
