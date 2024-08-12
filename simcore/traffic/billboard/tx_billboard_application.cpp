// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_billboard_application.h"
#include "tx_header.h"
#include "tx_billboard_flags.h"
#include "tx_billboard_loop.h"
#include "txsim_module.h"
#include "txsim_module_service.h"
#include "vehicle_geometry.pb.h"

void Billboard_Application::PrintParamValues() noexcept {
  PrintFlagsValues_Billboard();
  sim_msg::VehicleGeometory unuse;
  unuse.set_coord_type(sim_msg::VehicleGeometory_VehicleCoordType_FrontAxle_SnappedToGround);
}

int Billboard_Application::Run(int argc, char* argv[]) noexcept {
  Base::txString strIpPort(FLAGS_ip_addr_port);
  std::shared_ptr<tx_sim::SimModule> pModule = std::make_shared<TrafficFlow::BillboardLoop>();

  if (NonNull_Pointer(pModule)) {
    try {
      LOG(INFO) << "Simulation Start...";
      tx_sim::SimModuleService service;

      service.Serve(FLAGS_billboard_app_name, pModule, strIpPort);
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
