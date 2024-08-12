// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include "../engine/config.h"
#include "../src/database/db.h"
#include "common/log/system_logger.h"
using namespace std;
#include "../database/synchronizer.h"

TEST(testSynchronizer, test) {
  CLog::Initialize("/data/hadmap_server/config/tadsim");
  CEngineConfig& conf = CEngineConfig::Instance();

  conf.Init("/data/hadmap_server/config/tadsim", "/data/hadmap_server/installed/tadsim");

  CDatabase& db = CDatabase::Instance();
  CSynchronizer& ins = CSynchronizer::Instance();
}
