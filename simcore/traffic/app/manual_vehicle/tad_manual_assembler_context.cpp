// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_manual_assembler_context.h"
#include "tad_manual_ai_vehicle_element.h"
#include "tad_manual_user_defined_vehicle_element.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_SceneLoader)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(Scene)

Base::txInt TAD_ManualAssemblerContext::GenerateTrafficFlowElement_Vehicle(Base::IElementManagerPtr _elemMgr,
                                                                           Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  Base::txInt retCnt = 0;
  auto res = _loader->GetAllVehicleData();
  for (auto& pair_id_viewerPtr : res) {
    Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr =
        std::dynamic_pointer_cast<Base::ISceneLoader::IVehiclesViewer>(pair_id_viewerPtr.second);
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      const Base::Enums::VEHICLE_BEHAVIOR behavorType = _elemAttrViewPtr->behaviorEnum();
      if ((_plus_(Base::Enums::VEHICLE_BEHAVIOR::eUserDefined)) == behavorType) {
        TrafficFlow::TAD_Manual_UserDefined_VehicleElementPtr vehicle =
            std::make_shared<TrafficFlow::TAD_Manual_UserDefined_VehicleElement>();
        if (NonNull_Pointer(vehicle) && CallSucc(vehicle->Initialize(pair_id_viewerPtr.second, _loader))) {
          _elemMgr->AddVehiclePtr(vehicle);
          LogInfo << "[vehicle_event] vehicle_id:" << (*vehicle).Id()
                  << ", Merge : " << ((vehicle->getMergeViewPtr()) ? (vehicle->getMergeViewPtr()->Str()) : "Unset.")
                  << ", Merge_Event : "
                  << ((vehicle->getMergeEventViewPtr()) ? (vehicle->getMergeEventViewPtr()->Str()) : "Unset.")
                  << ", Acc : " << ((vehicle->getAccViewPtr()) ? (vehicle->getAccViewPtr()->Str()) : "Unset")
                  << ", Acc_Event : "
                  << ((vehicle->getAccEventViewPtr()) ? (vehicle->getAccEventViewPtr()->Str()) : "Unset")
                  << ", Velocity : "
                  << ((vehicle->getVelocityViewPtr()) ? (vehicle->getVelocityViewPtr()->Str()) : "Unset ")
                  << ", Velocity_Event : "
                  << ((vehicle->getVelocityEventViewPtr()) ? (vehicle->getVelocityEventViewPtr()->Str()) : "Unset ");
          if (_plus_(Base::ISceneLoader::VEHICLE_BEHAVIOR::eUserDefined) == vehicle->VehicleBehavior()) {
            txEventDispatcher::getInstance().RegisterAccEvent(vehicle->getAccViewPtr(), vehicle);
            txEventDispatcher::getInstance().RegisterAccEventEvent(vehicle->getAccEventViewPtr(), vehicle);
            txEventDispatcher::getInstance().RegisterMergeEvent(vehicle->getMergeViewPtr(), vehicle);
            txEventDispatcher::getInstance().RegisterMergeEventEvent(vehicle->getMergeEventViewPtr(), vehicle);
            txEventDispatcher::getInstance().RegisterVelocityEvent(vehicle->getVelocityViewPtr(), vehicle);
            txEventDispatcher::getInstance().RegisterVelocityEventEvent(vehicle->getVelocityEventViewPtr(), vehicle);
          } else {
            LOG(FATAL) << " vehicle behavior is not user_defined, do not response any event. "
                       << TX_VARS(vehicle->Id());
          }
          ++retCnt;
        } else {
          LogWarn << " Create Vehicle Element Failure.";
        }
      } else if ((_plus_(Base::Enums::VEHICLE_BEHAVIOR::eTadAI)) == behavorType ||
                 (_plus_(Base::Enums::VEHICLE_BEHAVIOR::eTadAI_Arterial)) == behavorType) {
        TrafficFlow::TAD_Manual_AI_VehicleElementPtr vehicle =
            std::make_shared<TrafficFlow::TAD_Manual_AI_VehicleElement>();
        if (NonNull_Pointer(vehicle) && CallSucc(vehicle->Initialize(pair_id_viewerPtr.second, _loader))) {
          _elemMgr->AddVehiclePtr(vehicle);
          LogInfo << "[vehicle_event] vehicle_id:" << (*vehicle).Id();
          ++retCnt;
        } else {
          LogWarn << " Create Vehicle Element Failure.";
        }
      } else {
      }
    } else {
      LogWarn << "dynamic_pointer_cast IVehiclesViewer error.";
    }
  }
  LogInfo << "[Create_Vehicle]_Count = " << retCnt;
  return retCnt;
}

TX_NAMESPACE_CLOSE(Scene)
