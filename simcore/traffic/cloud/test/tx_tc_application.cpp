// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_application.h"
#define GLOG_USE_GLOG_EXPORT
#include <glog/logging.h>
#include "tx_tc_loop.h"
#include "tx_tc_test_gflags.h"
#include "txsim_module.h"
#include "txsim_module_service.h"

std::vector<txApplication::InitFunc> txApplication::ms_VecInitFunc;
std::vector<txApplication::ReleaseFunc> txApplication::ms_VecReleaseFunc;
bool txApplication::ms_bInitialized = false;
std::unique_ptr<txApplication> txApplication::ms_InstancePtr =
    std::unique_ptr<txApplication>(new txApplication());  // std::make_unique<txApplication>();

txApplication::txApplication() {
  if (!ms_bInitialized) {
    ms_bInitialized = true;
    /*Add Initialize*/
    ms_VecInitFunc.push_back(txApplication::InitCommandLineArguments TXSTMARK("idx : 1"));
    ms_VecInitFunc.push_back(txApplication::InitLogger TXSTMARK("idx : 2"));

    /*Add Release*/
    ms_VecReleaseFunc.push_back(txApplication::ReleaseLogger);
    ms_VecReleaseFunc.push_back(txApplication::ReleaseCommandLineArguments);
  }
}

void txApplication::Initialize(int argc, char* argv[]) noexcept {
  for (int i = 0; i < ms_VecInitFunc.size(); ++i) {
    ms_VecInitFunc[i](argc, argv);
  }
}

void txApplication::Terminate() noexcept {
  for (int i = 0; i < ms_VecReleaseFunc.size(); ++i) {
    ms_VecReleaseFunc[i]();
  }
}

int txApplication::Run(int argc, char* argv[]) noexcept {
  Base::txString strIpPort(FLAGS_test_ip_addr_port);
  std::shared_ptr<tx_sim::SimModule> pModule = std::make_shared<TrafficFlow::TrafficLoop4CloudDev>();

  if (pModule) {
    try {
      LOG(INFO) << "Simulation Start...";
      tx_sim::SimModuleService service;

      service.Serve(FLAGS_test_app_name, pModule, strIpPort);
      service.Wait();
    } catch (const std::exception& e) {
      LOG(WARNING) << "Serving module error:\n" << e.what() << std::endl;
      return ErrExitCode;
    }
    return SuccExitCode;
  } else {
    LOG(FATAL) << "Create TrafficFlow::TrafficLoop Error.";
    return ErrExitCode;
  }
}

void txApplication::InitLogger(int argc, char* argv[]) noexcept {}

void txApplication::ReleaseLogger() noexcept {}

void txApplication::InitCommandLineArguments(int argc, char* argv[]) noexcept {}

void txApplication::ReleaseCommandLineArguments() noexcept {}
