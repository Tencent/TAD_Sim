// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "HdMap/tx_hd_map_cache_concurrent.h"
#include <gtest/gtest.h>
#include <thread>
#include "utils/dylib.h"
#if USE_TBB
#  include "tbb/task_scheduler_init.h"
#endif

namespace HdMap {

class HadmapCacheConCurrentTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { scheduler_init_ = std::make_shared<tbb::task_scheduler_init>(1); }

 protected:
  static std::shared_ptr<tbb::task_scheduler_init> scheduler_init_;
};

std::shared_ptr<tbb::task_scheduler_init> HadmapCacheConCurrentTest::scheduler_init_ = nullptr;

TEST_F(HadmapCacheConCurrentTest, Initialize) {
  HadmapCacheConCurrent::InitParams_t param;
  param.strHdMapFilePath = "./map.xodr";
  param.SceneOriginGPS = hadmap::txPoint(113.43891907, 22.94175339, 0.00004235);
  HadmapCacheConCurrent::Initialize(param);
}
}  // namespace HdMap