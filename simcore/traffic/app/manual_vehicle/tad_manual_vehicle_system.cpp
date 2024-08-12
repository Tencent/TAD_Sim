// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_manual_vehicle_system.h"
#include "tad_manual_assembler_context.h"
#include "tad_manual_handler.h"
TX_NAMESPACE_OPEN(TrafficFlow)
#if Use_ManualVehicle

void TAD_ManualVehicleSystem::CreateAssemblerCtx() TX_NOEXCEPT {
  m_AssemblerCtx = std::make_shared<Scene::TAD_ManualAssemblerContext>();
}

Base::txBool TAD_ManualVehicleSystem::UpdateManualVehicleData(const Base::TimeParamManager& timeMgr,
                                                              const Base::txString& strManualVehicle_) TX_NOEXCEPT {
  sim_msg::ManualVehicleControlInfoArray controlArray;
  controlArray.ParseFromString(strManualVehicle_);
  if (NonNull_Pointer(ElemMgr()) && controlArray.vehicles_control_info_size() > 0) {
    auto cur_vehicle_vec = ElemMgr()->GetAllVehiclePtr();
    for (const auto& refControlInfo : controlArray.vehicles_control_info()) {
      const Base::txSysId vehId = refControlInfo.id();
      for (auto& vehPtr : cur_vehicle_vec) {
        if (NonNull_Pointer(vehPtr) && vehId == vehPtr->Id()) {
          TrafficFlow::IManualHandlerPtr curVehicleManualHandlerPtr =
              std::dynamic_pointer_cast<TrafficFlow::IManualHandler>(vehPtr);
          if (NonNull_Pointer(curVehicleManualHandlerPtr)) {
            curVehicleManualHandlerPtr->UpdateManualLocation(timeMgr, refControlInfo.veh_control_info());
          }
          break;
        }
      }
    }
  }
  LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle) << TX_VARS(controlArray.vehicles_control_info_size());
  return true;
}

#endif /*Use_ManualVehicle*/
TX_NAMESPACE_CLOSE(TrafficFlow)
