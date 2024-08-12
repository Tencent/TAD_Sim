// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "parallel_simulation/tx_tc_ps_traffic_system.h"
#include "parallel_simulation/tx_tc_ps_vehicle_obstacle_element.h"
#include "tx_linear_element_manager.h"
#include "tx_tc_gflags.h"
TX_NAMESPACE_OPEN(TrafficFlow)

void ParallelSimulation_TrafficElementSystem::CreateAssemblerCtx() TX_NOEXCEPT {
  m_ps_AssemblerCtx = std::make_shared<Scene::ParallelSimulation_AssemblerContext>();
}

void ParallelSimulation_TrafficElementSystem::CreateElemMgr() TX_NOEXCEPT {
  m_Cloud_ElementMgr = std::make_shared<CloudElementManager>();
  m_ElementMgr = m_Cloud_ElementMgr;
}

Base::txBool ParallelSimulation_TrafficElementSystem::Initialize(Base::ISceneLoaderPtr loader,
                                                                 const Base::map_range_t valid_map_range) TX_NOEXCEPT {
  PrintFlagsValuesTrafficCloud();
  if (loader) {
    m_SceneDataSource = loader;
    CreateElemMgr();
    CreateAssemblerCtx();
    m_Cloud_AssemblerCtx = m_ps_AssemblerCtx;
    if (NonNull_Pointer(CloudAssemblerCtx()) && NonNull_Pointer(m_ElementMgr) &&
        CallSucc(CloudAssemblerCtx()->GenerateScene(m_ElementMgr, loader)) &&
        CallSucc(CloudAssemblerCtx()->Init(loader, valid_map_range))) {
      m_ElementMgr->Initialize(loader);

      LOG(INFO) << "GenerateScene Success.";
      m_isAlive = true;
      return true;
    } else {
      LOG(WARNING) << " GenerateScene Failure.";
      return false;
    }
  } else {
    LOG(WARNING) << " Data is Null.";
    return false;
  }
}

Base::txBool ParallelSimulation_TrafficElementSystem::RegisterSimulationEvent(const its::txEvents& evts) TX_NOEXCEPT {
  std::vector<obs_vehicle_info> vecObsVehicleInfo;
  for (const auto& refEvent : evts.event()) {
    if (its::txEvent::txEventType::txEvent_txEventType_TrafficControl == refEvent.type()) {
      mSimulationEventVec.push_back(SimulationEvent());
      mSimulationEventVec.back().SetEvent(refEvent);
    } else if (its::txEvent::txEventType::txEvent_txEventType_RoadClose == refEvent.type()) {
      const Base::txFloat startTime = refEvent.starting_time();
      const Base::txFloat endTime = startTime + refEvent.duration();
      Base::txSysId currentSysId = refEvent.event_id() * 500 + 5;
      for (const auto& al : refEvent.affected_objs().affected_road()) {
        hadmap::txRoadPtr roadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(al.road_id());
        if (NonNull_Pointer(roadPtr)) {
          for (const auto& refSectionPtr : roadPtr->getSections()) {
            if (NonNull_Pointer(refSectionPtr)) {
              for (const auto& lanePtr : refSectionPtr->getLanes()) {
                if (NonNull_Pointer(lanePtr)) {
                  const auto al_laneUid = lanePtr->getTxLaneId();
                  const Base::txFloat dbSampleStep = obs_vehicle_info::SampleStep();
                  const Base::txFloat lane_length = lanePtr->getLength();
                  for (Base::txFloat s = 0.2; s < lane_length; s += dbSampleStep) {
                    obs_vehicle_info tmpNode;
                    tmpNode.vehId = (currentSysId++) * -1;
                    tmpNode.startTimeStamp = startTime;
                    tmpNode.endTimeStamp = endTime;
                    tmpNode.laneUid = al_laneUid;
                    tmpNode.s = s;
                    const hadmap::txPoint wgs84 = lanePtr->getGeometry()->getPoint(s);
                    tmpNode.vPos.FromWGS84(wgs84);
                    tmpNode.vLaneDir = HdMap::HadmapCacheConCurrent::GetLaneDir(al_laneUid, s);
                    vecObsVehicleInfo.push_back(tmpNode);
                  }
                }
              }
            }
          }
        }
      }
    } else {
      LOG_IF(WARNING, FLAGS_LogLevel_Cloud) << "un-support simulation event type." << refEvent.type();
    }
  }

  for (const auto& refNode : vecObsVehicleInfo) {
    TrafficFlow::PS_ObsVehicleElementPtr obs_vehicle_ptr = std::make_shared<TrafficFlow::PS_ObsVehicleElement>();
    if (NonNull_Pointer(obs_vehicle_ptr) && obs_vehicle_ptr->Initialize(refNode, SceneLoader())) {
      ElemMgr()->AddVehiclePtr(obs_vehicle_ptr);
      mTrafficAccidentVec.push_back(obs_vehicle_ptr);
      LOG(INFO) << "[PS_ObsVehicleElement] vehicle_id:" << (*obs_vehicle_ptr).Id();
    } else {
      LOG(WARNING) << ", Create Vehicle Element Failure.";
    }
  }
  return true;
}

Base::txBool ParallelSimulation_TrafficElementSystem::UpdateSimulationEvent(Base::TimeParamManager const& timeMgr)
    TX_NOEXCEPT {
  LOG_IF(WARNING, FLAGS_LogLevel_SimulationEvent) << timeMgr.str();
  for (auto& refSimulationEvent : mSimulationEventVec) {
    refSimulationEvent.UpdateSimulationEvent(
        timeMgr.AbsTime(), ElemMgr()->SearchElementByType(Base::ITrafficElement::ElementType::TAD_Vehicle));
  }
  return true;
}

Base::txBool ParallelSimulation_TrafficElementSystem::ComputeRoadVehicleInfo(
    std::map<Base::txRoadID, std::map<Base::txInt /*veh id*/, Base::txFloat /*velocity*/> >& refRoad2VehicleVelocityMap)
    TX_NOEXCEPT {
  refRoad2VehicleVelocityMap.clear();
  const auto allTrafficElement = ElemMgr()->GetAllVehiclePtr();
  for (auto vehiclePtr : allTrafficElement) {
    if (NonNull_Pointer(vehiclePtr)) {
      PS_VehicleElementPtr ref_PS_VehicleElemPtr = std::dynamic_pointer_cast<PS_VehicleElement>(vehiclePtr);
      if (NonNull_Pointer(ref_PS_VehicleElemPtr) && CallSucc(ref_PS_VehicleElemPtr->IsAlive())) {
        const Base::txInt vehId = ref_PS_VehicleElemPtr->Id();
        const Base::txFloat v = ref_PS_VehicleElemPtr->GetVelocity();
        const Base::Info_Lane_t laneInfo = ref_PS_VehicleElemPtr->GetCurrentLaneInfo();
        if (CallFail(laneInfo.isOnLaneLink)) {
          refRoad2VehicleVelocityMap[laneInfo.onLaneUid.roadId][vehId] = v;
        }
      }
    }
  }

  if (FLAGS_LogLevel_OdInfo) {
    for (const auto pa : refRoad2VehicleVelocityMap) {
      const auto roadId = pa.first;
      std::stringstream ss;
      for (const auto ppa : pa.second) {
        ss << ppa.first << ",";
      }
      LOG(WARNING) << TX_VARS(roadId) << TX_VARS_NAME(vehicleList, ss.str());
    }
  }
  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)
