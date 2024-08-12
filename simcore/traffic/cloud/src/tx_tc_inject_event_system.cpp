// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_inject_event_system.h"
#include "tx_spatial_query.h"
#include "tx_tc_ditw_tad_vehicle_ai_element.h"
#define InjectEventInfo LOG_IF(INFO, FLAGS_LogLevel_InjectEvent) << "[cloud_inject_event]"
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool InjectEventAction::Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT {
  raw_event_info = _raw_event_info;
  return true;
}

Base::txBool InjectEventAction::NeedRelease(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT {
  return (timeMgr.TimeStamp() >= raw_event_info.Event_end_time_stamp_ms());
}

Base::txBool InjectEventAction::NeedDone(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT {
  return (timeMgr.TimeStamp() >= raw_event_info.Event_start_time_stamp_ms()) &&
         (timeMgr.TimeStamp() < raw_event_info.Event_end_time_stamp_ms());
}

void InjectEventAction::PostDone(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(elemMgr)) {
    const auto& vec_all_vehicles = elemMgr->GetAllVehiclePtr();
    const Base::txSize nSize = vec_all_vehicles.size();

    tbb::parallel_for(static_cast<std::size_t>(0), vec_all_vehicles.size(), [&](const std::size_t idx) {
      auto veh_ptr = vec_all_vehicles[idx];
      if (NonNull_Pointer(veh_ptr)) {
        veh_ptr->ResetVelocityDesired();
      }
    }); /*lamda function*/
        /* parallel_for */
  }
}

#if __TX_Mark__("WeatherEvent")
Base::txBool WeatherEvent::Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT {
  ParentClass::Init(_raw_event_info);
  set_event_influence_roads.clear();
  for (const auto rid : raw_event_info.Event_influence_roads()) {
    set_event_influence_roads.insert(rid);
  }
  if (set_event_influence_roads.empty()) {
    LogWarn << "set_event_influence_roads.empty." << TX_VARS(EventId());
    return false;
  }

  const auto eTemplateType = Utils::InfluenceRuleTemplate::s2e(raw_event_info.Event_influence_rule());
  for (const auto& refTemplate : raw_event_info.Event_influence_rule_template()) {
    if (eTemplateType == refTemplate.eName()) {
      weather_influence_template = refTemplate;
      std::ostringstream oss;
      std::copy(std::begin(set_event_influence_roads), std::end(set_event_influence_roads),
                std::ostream_iterator<Base::txRoadID>(oss, ","));
      LogWarn << TX_VARS(EventId()) << TX_VARS(weather_influence_template.Str())
              << TX_VARS_NAME(influence_roads, oss.str());
      return true;
      break;
    }
  }
  LogWarn << "no find template. " << TX_VARS(EventId());
  return false;
}

void WeatherEvent::Done(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(elemMgr)) {
    const auto& vec_all_vehicles = elemMgr->GetAllVehiclePtr();
    const Base::txSize nSize = vec_all_vehicles.size();

    tbb::parallel_for(static_cast<std::size_t>(0), vec_all_vehicles.size(), [&](const std::size_t idx) {
      auto veh_ptr = vec_all_vehicles[idx];
      if (NonNull_Pointer(veh_ptr) && CallSucc(veh_ptr->GetCurrentLaneInfo().IsOnLane()) &&
          CallSucc(NeedDone(veh_ptr->GetCurrentLaneInfo()))) {
        if ((veh_ptr->GetRawVelocityDesired()) >= (weather_influence_template.Threshold_ms())) {
          veh_ptr->SetVelocityDesired(weather_influence_template.Speed_limit_value_ms());
        } else {
          veh_ptr->SetVelocityDesired((veh_ptr->GetRawVelocityDesired()) *
                                      weather_influence_template.Speed_limit_factor());
        }
      }
    }); /*lamda function*/
        /* parallel_for */
  }
}

#endif /*__TX_Mark__("WeatherEvent")*/

#if __TX_Mark__("SpeedLimitControlEvent")
Base::txBool LanesSpeedLimitControlEvent::Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT {
  ParentClass::Init(_raw_event_info);
  set_speed_limit_laneUids.clear();

  for (const auto info : raw_event_info.Event_influence_lanes()) {
    set_speed_limit_laneUids.insert(info.LaneUid());
  }

  std::ostringstream oss_laneUids;
  std::copy(std::begin(set_speed_limit_laneUids), std::end(set_speed_limit_laneUids),
            std::ostream_iterator<Base::txLaneUId>(oss_laneUids, ","));
  LogWarn << TX_VARS_NAME(EventId, EventId())
          << TX_VARS_NAME(event_influence_lanes_speed_ms, Event_influence_lanes_speed_ms())
          << TX_VARS_NAME(influence_laneUids, oss_laneUids.str());
  return _NonEmpty_(set_speed_limit_laneUids);
}

void LanesSpeedLimitControlEvent::Done(Base::TimeParamManager const& timeMgr,
                                       Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(elemMgr)) {
    const auto& vec_all_vehicles = elemMgr->GetAllVehiclePtr();
    const Base::txSize nSize = vec_all_vehicles.size();

    tbb::parallel_for(static_cast<std::size_t>(0), vec_all_vehicles.size(), [&](const std::size_t idx) {
      auto veh_ptr = vec_all_vehicles[idx];
      if (NonNull_Pointer(veh_ptr) && CallSucc(veh_ptr->GetCurrentLaneInfo().IsOnLane()) &&
          CallSucc(NeedDone(veh_ptr->GetCurrentLaneInfo()))) {
        veh_ptr->SetVelocityDesired(Event_influence_lanes_speed_ms());
      }
    }); /*lamda function*/
        /* parallel_for */
  }
}

#endif /*__TX_Mark__("SpeedLimitControlEvent")*/

#if __TX_Mark__("RoadsSpeedLimitControlEvent")
Base::txBool RoadsSpeedLimitControlEvent::Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT {
  ParentClass::Init(_raw_event_info);
  set_speed_limit_roads.clear();

  for (const Base::txRoadID rid : raw_event_info.Event_influence_roads()) {
    set_speed_limit_roads.insert(rid);
  }

  std::ostringstream oss_roads;
  std::copy(std::begin(set_speed_limit_roads), std::end(set_speed_limit_roads),
            std::ostream_iterator<Base::txRoadID>(oss_roads, ","));
  LogWarn << TX_VARS_NAME(EventId, EventId())
          << TX_VARS_NAME(Event_influence_roads_speed_ms, Event_influence_roads_speed_ms())
          << TX_VARS_NAME(influence_roads, oss_roads.str());
  return _NonEmpty_(set_speed_limit_roads);
}

void RoadsSpeedLimitControlEvent::Done(Base::TimeParamManager const& timeMgr,
                                       Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(elemMgr)) {
    const auto& vec_all_vehicles = elemMgr->GetAllVehiclePtr();
    const Base::txSize nSize = vec_all_vehicles.size();

    tbb::parallel_for(static_cast<std::size_t>(0), vec_all_vehicles.size(), [&](const std::size_t idx) {
      auto veh_ptr = vec_all_vehicles[idx];
      if (NonNull_Pointer(veh_ptr) && CallSucc(veh_ptr->GetCurrentLaneInfo().IsOnLane()) &&
          CallSucc(NeedDone(veh_ptr->GetCurrentLaneInfo()))) {
        veh_ptr->SetVelocityDesired(Event_influence_roads_speed_ms());
      }
    }); /*lamda function*/
        /* parallel_for */
  }
}

#endif /*__TX_Mark__("RoadsSpeedLimitControlEvent")*/

#if __TX_Mark__("RoadClosureControlEvent")

Base::txBool RoadClosureControlEvent::Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT {
  Clear();
  ParentClass::Init(_raw_event_info);

  for (const auto rid : raw_event_info.Event_influence_roads()) {
    set_closure_roads.insert(rid);
  }
  return _NonEmpty_(set_closure_roads);
}

void RoadClosureControlEvent::Done(Base::TimeParamManager const& timeMgr,
                                   Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  if (CallFail(has_trigger) && NonNull_Pointer(elemMgr)) {
    has_trigger = true;
    Base::txSize obs_id_base = EventId() * 1000;
    const int64_t event_end_time_s = raw_event_info.Event_end_time_stamp_s();
    for (const Base::txRoadID rid : set_closure_roads) {
      auto road_ptr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(rid);
      if (NonNull_Pointer(road_ptr)) {
        auto sections = road_ptr->getSections();
        for (auto sec_ptr : sections) {
          if (NonNull_Pointer(sec_ptr)) {
            auto lanes = sec_ptr->getLanes();
            for (auto lane_ptr : lanes) {
              if (CallSucc(Utils::IsLaneValidDriving(lane_ptr))) {
                const Base::txFloat len = lane_ptr->getLength();
                Base::txFloat s = 1.0;
                while (s <= len) {
                  const Base::txSysId new_obs_id = ++obs_id_base;
                  const Base::Info_Lane_t influnceLocInfo(lane_ptr->getTxLaneId());
                  TAD_Virtual_Obstacle_VehicleElementPtr new_obstacle_veh_ptr =
                      std::make_shared<TAD_Virtual_Obstacle_VehicleElement>();
                  if (NonNull_Pointer(new_obstacle_veh_ptr) &&
                      CallSucc(new_obstacle_veh_ptr->Initialize_Closure_Obstacle(new_obs_id, influnceLocInfo, s,
                                                                                 event_end_time_s, nullptr))) {
                    elemMgr->AddVehiclePtr(new_obstacle_veh_ptr);
                    m_vec_event_obs.emplace_back(new_obstacle_veh_ptr);
                    InjectEventInfo << "create vehicle success. " << TX_VARS_NAME(veh_id, new_obstacle_veh_ptr->Id());
                  } else {
                    LogWarn << " Create Vehicle Element Failure.";
                  }
                  s += 2.0;
                }
              }
            }
          }
        }
      }
    }
  }
}

void RoadClosureControlEvent::Release(Base::TimeParamManager const& timeMgr,
                                      Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  for (auto ptr : m_vec_event_obs) {
    if (NonNull_Pointer(ptr)) {
      ptr->Kill();
    }
  }
}

#endif /*__TX_Mark__("RoadClosureControlEvent")*/

#if __TX_Mark__("LaneClosureControlEvent")
Base::txBool LaneClosureControlEvent::Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT {
  Clear();
  ParentClass::Init(_raw_event_info);
  for (const auto& LandUid : raw_event_info.Event_influence_lanes()) {
    set_closure_lanes.insert(LandUid.LaneUid());
  }
  return _NonEmpty_(set_closure_lanes);
}

void LaneClosureControlEvent::Done(Base::TimeParamManager const& timeMgr,
                                   Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  if (CallFail(has_trigger) && NonNull_Pointer(elemMgr)) {
    has_trigger = true;
    Base::txSize obs_id_base = EventId() * 1000;
    const int64_t event_end_time_s = raw_event_info.Event_end_time_stamp_s();
    for (const Base::txLaneUId laneUid : set_closure_lanes) {
      auto lane_ptr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(laneUid);
      if (CallSucc(Utils::IsLaneValidDriving(lane_ptr))) {
        const Base::txFloat len = lane_ptr->getLength();
        Base::txFloat s = 1.0;
        while (s <= len) {
          const Base::txSysId new_obs_id = ++obs_id_base;
          const Base::Info_Lane_t influnceLocInfo(lane_ptr->getTxLaneId());
          TAD_Virtual_Obstacle_VehicleElementPtr new_obstacle_veh_ptr =
              std::make_shared<TAD_Virtual_Obstacle_VehicleElement>();
          if (NonNull_Pointer(new_obstacle_veh_ptr) &&
              CallSucc(new_obstacle_veh_ptr->Initialize_Closure_Obstacle(new_obs_id, influnceLocInfo, s,
                                                                         event_end_time_s, nullptr))) {
            elemMgr->AddVehiclePtr(new_obstacle_veh_ptr);
            m_vec_event_obs.emplace_back(new_obstacle_veh_ptr);
            InjectEventInfo << "create vehicle success. " << TX_VARS_NAME(veh_id, new_obstacle_veh_ptr->Id());
          } else {
            LogWarn << " Create Vehicle Element Failure.";
          }
          s += 2.0;
        }
      }
    }
  }
}

void LaneClosureControlEvent::Release(Base::TimeParamManager const& timeMgr,
                                      Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  for (auto ptr : m_vec_event_obs) {
    if (NonNull_Pointer(ptr)) {
      ptr->Kill();
    }
  }
}
#endif /*__TX_Mark__("LaneClosureControlEvent")*/

#if __TX_Mark__("TrafficIncidentEvent")
Base::txBool TrafficIncidentEvent::Init(const Utils::TrafficInjectEvent _raw_event_info) TX_NOEXCEPT {
  Clear();
  ParentClass::Init(_raw_event_info);
  m_event_point.FromWGS84(raw_event_info.Event_location_lon(), raw_event_info.Event_location_lat());
  Base::Info_Lane_t event_laneInfo;
  Base::txFloat event_s;
  Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(m_event_point,
                                                                                            event_laneInfo, event_s);
  const Base::txFloat hash_scope_len = Geometry::SpatialQuery::HashedLaneInfo::HashScopeLen();
  InjectEventInfo << TX_VARS(m_event_point) << TX_VARS(event_laneInfo) << TX_VARS(event_s) << TX_VARS(hash_scope_len);
  if (CallSucc(event_laneInfo.IsOnLane()) && hash_scope_len > 0.0) {
    Geometry::SpatialQuery::HashedLaneInfo curInflunceHashNode = Geometry::SpatialQuery::GenerateHashedLaneInfo(
        HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(event_laneInfo.onLaneUid), event_s, event_laneInfo);
    if (raw_event_info.Event_influence_range().Upstream_m() > 0.0) {
      std::list<Geometry::SpatialQuery::HashedLaneInfoPtr> upstream_hashed;
      const Base::txInt nStep = std::ceil(raw_event_info.Event_influence_range().Upstream_m() / hash_scope_len);
      Geometry::SpatialQuery::GetBackHashedLaneInfoList(curInflunceHashNode, upstream_hashed, nStep);
      for (auto ptr : upstream_hashed) {
        m_list_influence_hashednode.emplace_back(*ptr);
      }
    }

    if (raw_event_info.Event_influence_range().Downstream_m() > 0.0) {
      std::list<Geometry::SpatialQuery::HashedLaneInfoPtr> downstream_hashed;
      const Base::txInt nStep = std::ceil(raw_event_info.Event_influence_range().Downstream_m() / hash_scope_len);
      Geometry::SpatialQuery::GetFrontHashedLaneInfoList(curInflunceHashNode, downstream_hashed, nStep);
      for (auto ptr : downstream_hashed) {
        m_list_influence_hashednode.emplace_back(*ptr);
      }
    }

    std::unordered_set<Base::txLaneUId, Utils::LaneUIdHashCompare> set_influence_lanes;
    for (auto influenceLane : raw_event_info.Event_influence_lanes()) {
      set_influence_lanes.insert(influenceLane.LaneUid());
    }

    for (auto itr = std::begin(m_list_influence_hashednode); itr != std::end(m_list_influence_hashednode);) {
      if (CallSucc((*itr).LaneInfo().IsOnLane()) &&
          CallSucc(0 == set_influence_lanes.count((*itr).LaneInfo().onLaneUid))) {
        itr = m_list_influence_hashednode.erase(itr);
      } else if (CallSucc((*itr).LaneInfo().IsOnLaneLink()) &&
                 CallSucc(0 == set_influence_lanes.count((*itr).LaneInfo().onLinkFromLaneUid)) &&
                 CallSucc(0 == set_influence_lanes.count((*itr).LaneInfo().onLinkToLaneUid))) {
        itr = m_list_influence_hashednode.erase(itr);
      } else {
        ++itr;
      }
    }
    return _NonEmpty_(m_list_influence_hashednode);
  } else {
    LogWarn << "event point is on lanelink.";
    return false;
  }
}

void TrafficIncidentEvent::Done(Base::TimeParamManager const& timeMgr, Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  if (CallFail(has_trigger) && NonNull_Pointer(elemMgr)) {
    has_trigger = true;
    Base::txSize obs_id_base = EventId() * 1000;
    const int64_t event_end_time_s = raw_event_info.Event_end_time_stamp_s();
    for (const auto hashNode : m_list_influence_hashednode) {
      const Base::txFloat start_s = std::get<0>(hashNode.m_subSectionScope);
      const Base::txFloat end_s = std::get<1>(hashNode.m_subSectionScope);
      Base::txFloat s = start_s + 1.0;
      while (s <= end_s) {
        const Base::txSysId new_obs_id = ++obs_id_base;
        const Base::Info_Lane_t influnceLocInfo = hashNode.LaneInfo();
        TAD_Virtual_Obstacle_VehicleElementPtr new_obstacle_veh_ptr =
            std::make_shared<TAD_Virtual_Obstacle_VehicleElement>();
        if (NonNull_Pointer(new_obstacle_veh_ptr) && CallSucc(new_obstacle_veh_ptr->Initialize_Closure_Obstacle(
                                                         new_obs_id, influnceLocInfo, s, event_end_time_s, nullptr))) {
          elemMgr->AddVehiclePtr(new_obstacle_veh_ptr);
          m_vec_event_obs.emplace_back(new_obstacle_veh_ptr);
          InjectEventInfo << "create vehicle success. " << TX_VARS_NAME(veh_id, new_obstacle_veh_ptr->Id());
        } else {
          LogWarn << " Create Vehicle Element Failure.";
        }
        s += 2.0;
      }
    }
  }
}

void TrafficIncidentEvent::Release(Base::TimeParamManager const& timeMgr,
                                   Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  for (auto ptr : m_vec_event_obs) {
    if (NonNull_Pointer(ptr)) {
      ptr->Kill();
    }
  }
}

#endif /*__TX_Mark__("TrafficIncidentEvent")*/
Base::txBool InjectEventHandler::InjectTrafficEvent(const std::string& _event_json) TX_NOEXCEPT {
  LOG(INFO) << TX_VARS_NAME(inject_event, _event_json);
  Utils::TrafficInjectEventBatch eventInfo = Utils::parser_inject_event(_event_json);
  LogWarn << eventInfo;
  for (const Utils::TrafficInjectEvent& evInfo : eventInfo.Event_list()) {
    const Utils::TrafficInjectEvent::InjectEventType event_type = evInfo.eEventType();
    if (CallSucc(eventInfo.Weather_enable()) && CallSucc(evInfo.IsWeather())) {
      InjectEventActionPtr weather_ptr = std::make_shared<WeatherEvent>();
      if (CallSucc(weather_ptr->Init(evInfo))) {
        m_vec_event_actions.emplace_back(weather_ptr);
      } else {
        LogWarn << "inject event create failure. " << TX_VARS(evInfo.Event_id());
      }
    } else if (CallSucc(eventInfo.Event_enable()) && CallSucc(evInfo.IsEvent())) {
      InjectEventActionPtr event_ptr = std::make_shared<TrafficIncidentEvent>();
      if (CallSucc(event_ptr->Init(evInfo))) {
        m_vec_event_actions.emplace_back(event_ptr);
      } else {
        LogWarn << "inject event create failure. " << TX_VARS(evInfo.Event_id());
      }
    } else if (CallSucc(eventInfo.Control_enable()) && CallSucc(evInfo.IsControl())) {
      if (Utils::TrafficInjectEvent::InjectEventType::sim_lane_speed_limit == event_type) {
        InjectEventActionPtr lane_speed_ptr = std::make_shared<LanesSpeedLimitControlEvent>();
        if (CallSucc(lane_speed_ptr->Init(evInfo))) {
          m_vec_event_actions.emplace_back(lane_speed_ptr);
        } else {
          LogWarn << "inject event create failure. " << TX_VARS(evInfo.Event_id());
        }
      } else if (Utils::TrafficInjectEvent::InjectEventType::sim_road_speed_limit == event_type) {
        InjectEventActionPtr road_speed_ptr = std::make_shared<RoadsSpeedLimitControlEvent>();
        if (CallSucc(road_speed_ptr->Init(evInfo))) {
          m_vec_event_actions.emplace_back(road_speed_ptr);
        } else {
          LogWarn << "inject event create failure. " << TX_VARS(evInfo.Event_id());
        }
      } else if (Utils::TrafficInjectEvent::InjectEventType::sim_road_closure == event_type) {
        InjectEventActionPtr road_closure_ptr = std::make_shared<RoadClosureControlEvent>();
        if (CallSucc(road_closure_ptr->Init(evInfo))) {
          m_vec_event_actions.emplace_back(road_closure_ptr);
        } else {
          LogWarn << "inject event create failure. " << TX_VARS(evInfo.Event_id());
        }
      } else if (Utils::TrafficInjectEvent::InjectEventType::sim_lane_closure == event_type) {
        InjectEventActionPtr lane_closure_ptr = std::make_shared<LaneClosureControlEvent>();
        if (CallSucc(lane_closure_ptr->Init(evInfo))) {
          m_vec_event_actions.emplace_back(lane_closure_ptr);
        } else {
          LogWarn << "inject event create failure. " << TX_VARS(evInfo.Event_id());
        }
      }
    }
  }
  return _NonEmpty_(m_vec_event_actions);
}

void InjectEventHandler::InjectTrafficEventHandlerPost(Base::TimeParamManager const& timeMgr,
                                                       Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  for (auto ptr : m_vec_event_actions) {
    if (NonNull_Pointer(ptr)) {
      ptr->PostDone(timeMgr, elemMgr);
    }
  }

  for (auto itr = std::begin(m_vec_event_actions); itr != std::end(m_vec_event_actions);) {
    if (Null_Pointer(*itr)) {
      LogWarn << "erase null iterator.";
      itr = m_vec_event_actions.erase(itr);
    } else {
      if ((*itr)->NeedRelease(timeMgr)) {
        LogWarn << "erase inject event." << TX_VARS_NAME(eventId, (*itr)->EventId());
        (*itr)->Release(timeMgr, elemMgr);
        itr = m_vec_event_actions.erase(itr);
      } else {
        ++itr;
      }
    }
  }
}

void InjectEventHandler::InjectTrafficEventHandler(Base::TimeParamManager const& timeMgr,
                                                   Base::IElementManagerPtr elemMgr) TX_NOEXCEPT {
  for (auto ptr : m_vec_event_actions) {
    if (NonNull_Pointer(ptr) && CallSucc(ptr->NeedDone(timeMgr))) {
      ptr->Done(timeMgr, elemMgr);
    }
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef InjectEventInfo
#undef LogWarn
