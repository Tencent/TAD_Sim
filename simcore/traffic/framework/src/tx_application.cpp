// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_application.h"
#include <gflags/gflags.h>
#include "tx_logger.h"
#include "tx_marco.h"
#include "tx_path_utils.h"

char* txApplication::s_app_path = "";

txApplication::txApplication() {
  if (!ms_bInitialized) {
    ms_bInitialized = true;
    /*Add Initialize*/
    ms_VecInitFunc.push_back(txApplication::InitCommandLineArguments);
    ms_VecInitFunc.push_back(txApplication::InitLogger);

    /*Add Release*/
    ms_VecReleaseFunc.push_back(txApplication::ReleaseLogger);
    ms_VecReleaseFunc.push_back(txApplication::ReleaseCommandLineArguments);
  }
}

void txApplication::Initialize(int argc, char* argv[]) noexcept {
  Utils::GetAppPath(std::string(argv[0]), s_app_path);
  // 依次执行函数
  for (int i = 0; i < ms_VecInitFunc.size(); ++i) {
    ms_VecInitFunc[i](argc, argv);
  }
  // 输出参数信息
  PrintParamValues();
}

void txApplication::Terminate() noexcept {
  for (int i = 0; i < ms_VecReleaseFunc.size(); ++i) {
    // 调用无参函数
    ms_VecReleaseFunc[i]();
  }
}

void txApplication::InitLogger(int argc, char* argv[]) noexcept {
  // 日志初始化
  Base::Logger::Initialize(argc, argv);
}

void txApplication::ReleaseLogger() noexcept { Base::Logger::Release(); }

void txApplication::InitCommandLineArguments(int argc, char* argv[]) noexcept {
  google::ParseCommandLineFlags(&argc, &argv, true TX_MARK("erase gflags item."));
}

void txApplication::ReleaseCommandLineArguments() noexcept {}
