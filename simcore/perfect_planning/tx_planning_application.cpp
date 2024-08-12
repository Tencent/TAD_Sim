// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_planning_application.h"
#include "tx_header.h"
#include "tx_planning_flags.h"
#include "tx_planning_loop.h"
#include "txsim_module.h"
#include "txsim_module_service.h"
void TADSim_Planning_Application::PrintParamValues() noexcept {
  TX_MARK("in module init.");
  FLAGS_Kill_After_Stop = false;
  PrintFlagsValues_Ego();
}

int TADSim_Planning_Application::Run(int argc, char* argv[]) noexcept {
  LOG(INFO) << "[application_type] " << FLAGS_app_type;
  if (Base::txString("txMsgModule") == FLAGS_app_type) {
    Base::txString strIpPort(FLAGS_ip_addr_port);
    std::shared_ptr<tx_sim::SimModule> pModule = std::make_shared<TrafficFlow::TAD_PlanningLoop>();

    if (NonNull_Pointer(pModule)) {
      try {
        LOG(INFO) << "Simulation Start...";
        tx_sim::SimModuleService service;

        service.Serve(FLAGS_app_name, pModule, strIpPort);
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
  } else {
    LOG(WARNING) << "[ERROR] Unknown " << TX_VARS_NAME(application_type, FLAGS_app_type);
    return ErrExitCode;
  }
}
