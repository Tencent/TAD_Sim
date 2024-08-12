// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "log2world_assembler_context.h"
#include "l2w_linear_element_manager.h"
#include "l2w_vehicle_element.h"
#include "tad_ego_vehicle_element.h"
#include "tad_simrec_loader.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_SceneLoader)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(Scene)

Base::txBool L2W_AssemblerContext::GenerateScene(Base::IElementManagerPtr _elemMgr,
                                                 Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  if (NonNull_Pointer(_elemMgr) && NonNull_Pointer(_loader)) {
    Base::txInt const retCnt_FlowElem = GenerateTrafficFlowElement(_elemMgr, _loader);
    LOG_IF(INFO, retCnt_FlowElem > 0) << "GenerateTrafficFlowElement " << retCnt_FlowElem;
    LOG_IF(WARNING, retCnt_FlowElem <= 0) << "GenerateTrafficFlowElement " << retCnt_FlowElem;

    Base::txInt const retCnt_EnvElem = GenerateTrafficEnvironmentElement(_elemMgr, _loader);
    LOG_IF(INFO, retCnt_EnvElem > 0) << "GenerateTrafficEnvironmentElement " << retCnt_EnvElem;
    LOG_IF(WARNING, retCnt_EnvElem <= 0) << "GenerateTrafficEnvironmentElement " << retCnt_EnvElem;

    Base::txInt const retCnt_RefElem = GenerateTrafficReferenceElement(_elemMgr, _loader);
    LOG_IF(INFO, retCnt_RefElem > 0) << "GenerateTrafficReferenceElement " << retCnt_RefElem;
    LOG_IF(WARNING, retCnt_RefElem <= 0) << "GenerateTrafficReferenceElement " << retCnt_RefElem;

    return true;
  } else {
    LogWarn << "Param Error.";
    return false;
  }
}

Base::txBool L2W_AssemblerContext::GenerateEgo(Base::IElementManagerPtr _elemMgr,
                                               Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  if (NonNull_Pointer(_elemMgr) && NonNull_Pointer(_loader)) {
    const auto egoType = _loader->GetEgoType();
    auto egoDataPtr = _loader->GetEgoData();
    TrafficFlow::TAD_EgoVehicleElementPtr ego = std::make_shared<TrafficFlow::TAD_EgoVehicleElement>();
    TrafficFlow::TAD_EgoVehicleElementPtr egoTrailer = nullptr;
    auto trailerDataPtr = _loader->GetEgoTrailerData();
    if (_plus_(Base::ISceneLoader::EgoType::eTruck) == egoType) {
      egoTrailer = std::make_shared<TrafficFlow::TAD_EgoVehicleElement>();
      egoTrailer->SetSubType(TrafficFlow::TAD_EgoVehicleElement::EgoSubType::eFollower);
    }
    if (NonNull_Pointer(ego) && CallSucc(ego->Initialize(egoDataPtr, _loader)) &&
        CallSucc(((egoTrailer) ? (egoTrailer->Initialize(trailerDataPtr, _loader)) : (true)))) {
      _elemMgr->AddEgoPtr(ego);
      if (_plus_(Base::ISceneLoader::EgoType::eTruck) == egoType && egoTrailer) {
        _elemMgr->AddEgoPtr(egoTrailer);
      }
      LogInfo << "Create Vehicle Element Success.";
      return true;
    } else {
      LogWarn << "Create Vehicle Element Failure.";
      return false;
    }
  } else {
    LogWarn << " Param Error.";
    return false;
  }
}

Base::txInt L2W_AssemblerContext::GenerateTrafficFlowElement(Base::IElementManagerPtr _elemMgr,
                                                             Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  Base::txInt retCnt = 0;
  retCnt += GenerateTrafficFlowElement_Vehicle(_elemMgr, _loader);
  retCnt += GenerateTrafficFlowElement_Pedestrian(_elemMgr, _loader);
  retCnt += GenerateTrafficFlowElement_Obstacle(_elemMgr, _loader);
  return retCnt;
}

Base::txInt L2W_AssemblerContext::GenerateTrafficFlowElement_Vehicle(Base::IElementManagerPtr _single_layer_elemMgr,
                                                                     Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
#if __TX_Mark__("create shadow element")
  TrafficFlow::L2W_LinearElementManagerPtr multi_layer_elem_mgr =
      std::dynamic_pointer_cast<TrafficFlow::L2W_LinearElementManager>(_single_layer_elemMgr);
  if (Null_Pointer(multi_layer_elem_mgr)) {
    LogWarn << "multi_layer_elem_mgr is nullptr.";
    return 0;
  }

  if (Null_Pointer(multi_layer_elem_mgr->ShadowLayer())) {
    LogWarn << "multi_layer_elem_mgr->ShadowLayer() is nullptr.";
    return 0;
  }
#endif /*__TX_Mark__("create shadow element")*/
  Base::txInt retCnt = 0;
  auto res = _loader->GetAllVehicleData();
  for (auto& pair_id_viewerPtr : res) {
    SceneLoader::Simrec_SceneLoader::L2W_VehiclesViewerPtr _elemAttrViewPtr =
        std::dynamic_pointer_cast<SceneLoader::Simrec_SceneLoader::L2W_VehiclesViewer>(pair_id_viewerPtr.second);
    if (NonNull_Pointer(_elemAttrViewPtr) && NonNull_Pointer(_elemAttrViewPtr->trajMgr())) {
      TrafficFlow::L2W_VehicleElementPtr vehicle = std::make_shared<TrafficFlow::L2W_VehicleElement>();
      if (NonNull_Pointer(vehicle) &&
          CallSucc(vehicle->Initialize(
              TX_MARK("_elemAttrViewPtr->trajMgr() need lossless parameter transmission for shadow clone")
                  pair_id_viewerPtr.second,
              _loader))) {
        multi_layer_elem_mgr->AddVehiclePtr(vehicle);
        LogInfo << "[vehicle_event][L2W_AI] vehicle_id:" << (*vehicle).Id();
        ++retCnt;
#if __TX_Mark__("create shadow element")
        {
          TrafficFlow::L2W_VehicleElement_ShadowPtr shadow_veh_ptr =
              std::make_shared<TrafficFlow::L2W_VehicleElement_Shadow>();
          if (NonNull_Pointer(shadow_veh_ptr) &&
              CallSucc(shadow_veh_ptr->Initialize(pair_id_viewerPtr.second, _loader))) {
            multi_layer_elem_mgr->ShadowLayer()->AddVehiclePtr(shadow_veh_ptr);
            LogInfo << "[vehicle_event][L2W_AI][shadow] vehicle_id:" << (*shadow_veh_ptr).Id();
          } else {
            LogWarn << " Create Shadow Vehicle Element Failure.";
          }
        }
#endif /*__TX_Mark__("create shadow element")*/
      } else {
        LogWarn << " Create Vehicle Element Failure.";
      }
    } else {
      LogWarn << "dynamic_pointer_cast L2W_VehiclesViewerPtr error.";
    }
  }
  return retCnt;
}
TX_NAMESPACE_CLOSE(Scene)
