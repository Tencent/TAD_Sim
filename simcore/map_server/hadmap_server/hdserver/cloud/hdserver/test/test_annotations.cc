// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <mutex>
#include "gtest/gtest.h"

#include "common/third/gutil/thread_annotations.h"

TEST(testAnnotations, compile) {
  std::mutex m;
  int a GUARDED_BY(m);
  int b GUARDED_BY(m);
}
