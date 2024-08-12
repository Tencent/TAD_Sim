// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tadsim_application.h"
#include "tadsim_loop.h"
#include "tx_header.h"
#include "tx_sys_info.h"
#include "tx_tadsim_flags.h"
#include "txsim_module.h"
#include "txsim_module_service.h"
void TADSim_Application::PrintParamValues() noexcept { PrintFlagsValues_TADSim(); }

int TADSim_Application::Run(int argc, char* argv[]) noexcept {
  LOG(INFO) << "[application_type] " << FLAGS_app_type;
  // 如果FLAGS_app_type等于"txMsgModule"
  if (Base::txString("txMsgModule") == FLAGS_app_type) {
    Base::txString strIpPort(FLAGS_ip_addr_port);
    // 创建一个TrafficFlow::TADSimLoop对象，并将其赋值给pModule
    std::shared_ptr<tx_sim::SimModule> pModule = std::make_shared<TrafficFlow::TADSimLoop>();

    // 如果pModule非空
    if (NonNull_Pointer(pModule)) {
      try {
        LOG(INFO) << "Simulation Start...";
        tx_sim::SimModuleService service;

        // 调用service的Serve方法，传入FLAGS_app_name、pModule和strIpPort
        service.Serve(FLAGS_app_name, pModule, strIpPort);
        service.Wait();
      } catch (const std::exception& e) {
        LOG(WARNING) << "Serving module error:\n" << e.what() << std::endl;
        return ErrExitCode;
      }
      return SuccExitCode;
    } else {
      // 如果FLAGS_app_type不等于"txMsgModule"，输出提示信息
      LOG(FATAL) << "Create TrafficFlow::TrafficLoop Error.";
      return ErrExitCode;
    }
  } else {
    LOG(WARNING) << "[ERROR] Unknown " << TX_VARS_NAME(application_type, FLAGS_app_type);
    return ErrExitCode;
  }
}
