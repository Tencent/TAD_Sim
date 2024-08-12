// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_header.h"
#include "tx_parallel_def.h"
#include "tx_billboard_application.h"
#if USE_TBB
#  include "tbb/task_scheduler_init.h"
#endif
int main(int argc, char* argv[]) {
#if USE_TBB
  if (0 >= FLAGS_MAX_THREAD_NUM) {
    tbb::task_scheduler_init init(tbb::task_scheduler_init::automatic);
    LOG(INFO) << "[MultiThread] thread_num =  auto.";
  } else if (FLAGS_MAX_THREAD_NUM > 0) {
    tbb::task_scheduler_init init(FLAGS_MAX_THREAD_NUM);
    LOG(INFO) << "[MultiThread] thread_num = " << FLAGS_MAX_THREAD_NUM;
  }
#endif
  txApplicationPtr appPtr = std::make_shared<Billboard_Application>();
  if (NonNull_Pointer(appPtr)) {
    appPtr->Initialize(argc, argv);
    int nExitCode = appPtr->Run(argc, argv);
    appPtr->Terminate();
    return nExitCode;
  } else {
    LOG(WARNING) << "Application create failure.";
    return ErrExitCode;
  }
}
