// Copyright 2024 Tencent Inc. All rights reserved.
//

// txSimTraffic.cpp: 定义应用程序的入口点。
//
#include "tadsim_application.h"
#include "tx_header.h"
#include "tx_parallel_def.h"
#if USE_TBB
#  if TBB_NEW_API
#    define TBB_PREVIEW_GLOBAL_CONTROL 1
#    include <tbb/global_control.h>
#  else
#    include "tbb/task_scheduler_init.h"
#  endif
#endif

int main(int argc, char* argv[]) {
#if USE_TBB
#  if TBB_NEW_API
  if (0 >= FLAGS_MAX_THREAD_NUM) {
    LOG(INFO) << "[MultiThread] thread_num =  auto.";
  } else if (FLAGS_MAX_THREAD_NUM > 0) {
    tbb::global_control control(tbb::global_control::max_allowed_parallelism, FLAGS_MAX_THREAD_NUM);
    LOG(INFO) << "[MultiThread] thread_num = " << FLAGS_MAX_THREAD_NUM;
  }
#  else
  if (0 >= FLAGS_MAX_THREAD_NUM) {
    tbb::task_scheduler_init init(tbb::task_scheduler_init::automatic);
    LOG(INFO) << "[MultiThread] thread_num =  auto.";
  } else if (FLAGS_MAX_THREAD_NUM > 0) {
    tbb::task_scheduler_init init(FLAGS_MAX_THREAD_NUM);
    LOG(INFO) << "[MultiThread] thread_num = " << FLAGS_MAX_THREAD_NUM;
  }
#  endif
#endif
  txApplicationPtr appPtr = std::make_shared<TADSim_Application>();
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
