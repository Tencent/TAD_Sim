// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "parallel_simulation/tx_tc_ps_assembler_context.h"
#include "parallel_simulation/tx_tc_ps_element_generator.h"
#include "parallel_simulation/tx_tc_ps_traffic_light_element.h"
#include "tx_map_info.h"
#include "tx_spatial_query.h"
TX_NAMESPACE_OPEN(Scene)

Base::txBool ParallelSimulation_AssemblerContext::Init(Base::ISceneLoaderPtr _loader,
                                                       const Base::map_range_t valid_map_range) TX_NOEXCEPT {
  mSceneLoader = _loader;
  if (mSceneLoader) {
    LOG(INFO) << "TAD_Cloud_AssemblerContext::Init";
    Base::ISceneLoader::ITrafficFlowViewerPtr tf_ptr = mSceneLoader->GetTrafficFlow();
    if (NonNull_Pointer(tf_ptr)) {
      m_ps_VehicleElementGenerator = std::make_shared<Scene::ParallelSimulation_ElementGenerator>();
      mVehicleElementGenerator = m_ps_VehicleElementGenerator;
      mVehicleElementGenerator->Initialize(_loader, valid_map_range);

      return true;
    } else {
      LOG(WARNING) << "TAD_Cloud_AssemblerContext::Init, ITrafficFlowViewerPtr is null.";
      return false;
    }
  } else {
    LOG(WARNING) << "TAD_Cloud_AssemblerContext::Init, param is null.";
    return false;
  }
}

Base::txBool ParallelSimulation_AssemblerContext::CreateTrafficLightElement(
    Base::IElementManagerPtr _elemMgr, const Base::txString traffic_config_path) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  if (NonNull_Pointer(_elemMgr)) {
    its::txControllers traffic_light_controllers;
    if (!traffic_light_controllers.ParseFromString(traffic_config_path)) {
      LOG(ERROR) << "Failed to parse traffic light controllers.";
      return false;
    }

    for (const auto& reftxController : traffic_light_controllers.light_controller()) {
      for (const auto& refLightGroup : reftxController.light_group()) {
        for (const auto& refLight : refLightGroup.light()) {
          std::vector<std::tuple<Base::txRoadID, Base::txRoadID> > vecTopo;
          for (const auto& refTopo : refLight.topo()) {
            vecTopo.push_back(std::make_tuple(refTopo.up_road_id(), refTopo.down_road_id()));
          }
          TrafficFlow::PS_SignalLightElementPtr newPS_TL_Ptr = std::make_shared<TrafficFlow::PS_SignalLightElement>();
          if (newPS_TL_Ptr->Initialize_ParallelSimulation(refLight.light_id(), vecTopo)) {
            _elemMgr->AddSignalPtr(newPS_TL_Ptr);
#if __TX_Mark__("cloud search circle signal")
            Geometry::SpatialQuery::RTree2D_Signal::getInstance().Insert_Signal(
                newPS_TL_Ptr->GetLocation().ToENU().ENU2D(), newPS_TL_Ptr);
            LOG(INFO) << TX_VARS_NAME(signalId, (newPS_TL_Ptr->Id()))
                      << TX_VARS_NAME(signalPos, newPS_TL_Ptr->GetLocation());
#endif /*USE_RTree*/
          }
        }
      }
    }
#if 1
    Base::ISignalLightElement::RoadId2SignalIdSet roadId_with_signal;
    auto valid_signals = _elemMgr->GetAllSignal();
    for (auto signalPtr : valid_signals) {
      auto tad_signal_ptr = std::dynamic_pointer_cast<TrafficFlow::PS_SignalLightElement>(signalPtr);
      if (NonNull_Pointer(tad_signal_ptr)) {
        roadId_with_signal[tad_signal_ptr->GetCurrentLaneInfo().onLaneUid.roadId].insert(tad_signal_ptr->Id());
      } else {
        LOG(WARNING) << " Create Signal Light Element Failure.";
      }
    }
    for (const auto& ref_pair : roadId_with_signal) {
      std::ostringstream oss;
      std::copy(ref_pair.second.begin(), ref_pair.second.end(), std::ostream_iterator<Base::txSysId>(oss, ","));
      LOG(INFO) << "[TAD_Signal_Road]" << TX_VARS_NAME(RoadId, ref_pair.first) << TX_VARS_NAME(SignalIds, oss.str());
    }

    for (auto signalPtr : valid_signals) {
      auto tad_signal_ptr = std::dynamic_pointer_cast<TrafficFlow::PS_SignalLightElement>(signalPtr);
      if (NonNull_Pointer(tad_signal_ptr)) {
        if (CallFail(tad_signal_ptr->GenerateSignalControlLaneInfoList(roadId_with_signal))) {
          LOG(WARNING) << "[TAD_Signal_Road] " << TX_VARS_NAME(SignalId, tad_signal_ptr->Id())
                       << " initialize failure.";
        } else {
          tad_signal_ptr->CheckStart(Base::TimeParamManager(0, 0, 0, 0));
          LOG(INFO) << "[Signal]" << TX_VARS_NAME(SignalId, tad_signal_ptr->Id()) << " create success.";
        }
      } else {
        LOG(WARNING) << " Create Signal Light Element Failure.";
      }
    }
#endif
    LOG(INFO) << "Create PS TrafficLight : " << (_elemMgr->GetAllSignal().size());
    return true;
  } else {
    LOG(WARNING) << "CreateTrafficLightElement failure.";
    return false;
  }
}

TX_NAMESPACE_CLOSE(Scene)
