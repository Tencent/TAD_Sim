// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "parallel_simulation/tx_tc_ps_traffic_light_element.h"
#include "tx_protobuf_utils.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Signal)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool PS_SignalLightElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                                   sim_msg::Traffic& TrafficData) TX_NOEXCEPT {
  if (IsAlive()) {
    if (m_control_lane_set_from_conf.size() > 0) {
      sim_msg::TrafficLight* tmpTrafficLightData = nullptr;
      {
#if USE_TBB
        tbb::mutex::scoped_lock lock(s_tbbMutex_outputTraffic);
#endif
        tmpTrafficLightData = TrafficData.add_trafficlights();
      }
      tmpTrafficLightData->set_id(Id());
      const hadmap::txPoint geom_center_gps = mLocation.GeomCenter().WGS84();
      tmpTrafficLightData->set_x(__Lon__(geom_center_gps));
      tmpTrafficLightData->set_y(__Lat__(geom_center_gps));
      tmpTrafficLightData->set_z(__Alt__(geom_center_gps));
      tmpTrafficLightData->set_color(GetCurrentSignLightColorType());
      tmpTrafficLightData->set_heading(0.0);
      tmpTrafficLightData->set_age(0.0);
#if Use_SignalPhase
      for (const auto& control_lane : m_control_lane_set_from_conf) {
        sim_msg::LaneUid* cur_control_lane = tmpTrafficLightData->add_control_lanes();
        cur_control_lane->set_tx_road_id(control_lane.first.roadId);
        cur_control_lane->set_tx_section_id(control_lane.first.sectionId);
        cur_control_lane->set_tx_lane_id(control_lane.first.laneId);
      }

#endif /*Use_SignalPhase*/
      LOG_IF(INFO, FLAGS_LogLevel_Signal || FLAGS_LogLevel_SendMsg)
          << "[trafficlight_output][" << (Id()) << "]:" << Utils::ProtobufDebugJson(tmpTrafficLightData)
          << TX_VARS_NAME(Color, GetCurrentSignLightColorType());
    }
  }
  return true;
}

Base::txBool PS_SignalLightElement::Initialize_ParallelSimulation(
    const Base::txSysId light_id,
    const std::vector<std::tuple<Base::txRoadID, Base::txRoadID> >& vecRoadTopo) TX_NOEXCEPT {
  mIdentity.Id() = light_id;
  mIdentity.SysId() = CreateSysId(mIdentity.Id());
  std::set<Base::txLaneUId> _Control_laneUid_one_ring;
  for (const auto refTopo : vecRoadTopo) {
    hadmap::txLaneLinks resLinks;
    hadmap::getLaneLinks(txMapHdr, std::get<0>(refTopo), std::get<1>(refTopo), resLinks);
    for (const auto linkPtr : resLinks) {
      _Control_laneUid_one_ring.insert(linkPtr->fromTxLaneId());
    }
  }
  one_ring_lanes.clear();
  hadmap::txLanes& lanes = one_ring_lanes;
  for (const auto laneUid : _Control_laneUid_one_ring) {
    hadmap::txLanePtr lanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(laneUid);
    if (Utils::IsLaneValidDriving(lanePtr)) {
      lanes.push_back(lanePtr);
    } else {
      LogWarn << TX_VARS_NAME(laneUid, Utils::ToString(laneUid)) << "is not driving.";
    }
  }
  if (CallFail(lanes.empty())) {
    Coord::txWGS84 tmpSignalPos;
#if 1
    Base::txVec3 signalLocationOnENU;
    signalLocationOnENU.setZero();
    Base::txInt nLaneCnt = 0;
    Base::txVec3 signal_dir;
    for (const auto& l : lanes) {
      if (NonNull_Pointer(l)) {
        const auto& refLaneUid = l->getTxLaneId();
        if (refLaneUid.laneId < 0) {
          ++nLaneCnt;
          signal_dir = HdMap::HadmapCacheConCurrent::GetLaneDir(refLaneUid, l->getLength() - 0.5);
          const auto laneLength = l->getGeometry()->getLength();
          hadmap::txPoint signal_loc_by_lane = l->getGeometry()->getPoint(laneLength - 1.0);
          m_control_lane_set_from_conf.push_back(std::make_pair(refLaneUid, signal_loc_by_lane));
          Coord::txENU curSignalLoc;
          curSignalLoc.FromWGS84(signal_loc_by_lane);
          signalLocationOnENU += curSignalLoc.ENU();
          LOG_IF(WARNING, FLAGS_LogLevel_Signal)
              << "[signal_light_by_lane][pos] " << TX_VARS(Id()) << ", control lane : " << Utils::ToString(refLaneUid);
        }
      }
    }

    if (nLaneCnt > 0) {
      signalLocationOnENU = signalLocationOnENU / nLaneCnt;
      tmpSignalPos.FromENU(signalLocationOnENU);
      LOG_IF(WARNING, FLAGS_LogLevel_Signal) << "[signal_light_by_lane] compute average position " << tmpSignalPos;
    } else {
      LOG(WARNING) << "[signal_light_by_lane] valid signal lane is zero " << TX_VARS(light_id);
      return false;
    }
#endif

    Base::txLaneUId t_LaneUid;
    txFloat nouse = 0.0;
    hadmap::txLanePtr initLane = HdMap::HadmapCacheConCurrent::GetLaneForInit(tmpSignalPos.Lon(), tmpSignalPos.Lat(),
                                                                              mLocation.DistanceAlongCurve(), nouse);

    if (NonNull_Pointer(initLane)) {
      t_LaneUid = initLane->getTxLaneId();
      HdMap::HadmapCacheConCurrent::GetLonLatFromSTonLaneWithOffset(
          initLane->getTxLaneId(), mLocation.DistanceAlongCurve(), 0.0, mLocation.GeomCenter());
      mLocation.InitTracker(mIdentity.Id());
      RelocateTracker(initLane, 0.0);
      return true;
    } else {
      LogWarn << " GetLane Failure.";
      return false;
    }
  } else {
    LogWarn << TX_VARS_NAME(light_id, Id()) << " has zero control lane.";
    return false;
  }
}

Base::txBool PS_SignalLightElement::Update_ParallelSimulation(const SIGN_LIGHT_COLOR_TYPE _color) TX_NOEXCEPT {
  t_traffic_light_color = _color;

  return true;
}

Base::txBool PS_SignalLightElement::Update(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  PublicSignals(timeMgr);
  return true;
}

Base::txBool PS_SignalLightElement::GenerateSignalControlLaneInfoList(const RoadId2SignalIdSet& map_roadid2signalId)
    TX_NOEXCEPT {
  hadmap::txLanes& valid_lanes = one_ring_lanes;
  GenerateSignalByLaneCache_Internal(valid_lanes, map_roadid2signalId);
  LogInfo << "[Signal]" << TX_VARS_NAME(SignalId, Id()) << " initialize success.";
  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
