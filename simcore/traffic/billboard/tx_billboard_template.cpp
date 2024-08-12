// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_billboard_template.h"
#include "HdMap/tx_hashed_road.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_header.h"
#include "tx_path_utils.h"
#include "tx_spatial_query.h"
#include "tx_string_utils.h"
#include "tx_time_utils.h"
#include "tx_units.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Billboard)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)
#if 0
void LaneInfoAverSpeed::ClearOldInfo(const Base::txFloat pass_time_s,
                                     const Base::txInt buffer_time_ms /*5min = 5 * 60s*1000 = 300000*/) TX_NOEXCEPT {
  cur_aver_speed_ms = 0.0;
  cur_sum_veh_cnt = 0;
  cur_sum_speed_ms = 0.0;
  const Base::txFloat currentPassTime_ms = Utils::SecondToMillisecond(pass_time_s);
  for (auto itr = std::begin(list_info); itr != std::end(list_info);) {
    const auto &refSlice = *itr;
    if ((currentPassTime_ms - refSlice.pass_time_ms) >= buffer_time_ms) {
      itr = list_info.erase(itr);
    } else {
      break;
    }
  }
}

void LaneInfoAverSpeed::RegisterVehInfo(const Base::txFloat pass_time_s, const Base::txFloat speed_ms) TX_NOEXCEPT {
  if ((list_info).empty()) {
    /*empty*/
    SliceInfo newNode;
    newNode.Init(pass_time_s);
    list_info.emplace_back(newNode);
  } else {
    if (CallFail(Math::isEqual(list_info.back().pass_time_s, pass_time_s))) {
      SliceInfo newNode;
      newNode.Init(pass_time_s);
      list_info.emplace_back(newNode);
    }
  }
  list_info.back().sum_speed_ms += speed_ms;
  list_info.back().sum_veh_cnt += 1;
}

Base::txInt LaneInfoAverSpeed::VehicleCountInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT {
  Base::txInt res = 0;
  for (auto itr = std::begin(list_info); itr != std::end(list_info); ++itr) {
    res += (*itr).sum_veh_cnt;
  }
  return res;
}

void LaneInfoAverSpeed::ComputeAverSpeedMS(const Base::txFloat pass_time_s) TX_NOEXCEPT {
  cur_aver_speed_ms = 0.0;
  cur_sum_veh_cnt = 0;
  cur_sum_speed_ms = 0.0;

  for (auto itr = std::begin(list_info); itr != std::end(list_info); ++itr) {
    cur_sum_veh_cnt += (*itr).sum_veh_cnt;
    cur_sum_speed_ms += (*itr).sum_speed_ms;
  }
  if (cur_sum_veh_cnt > 0) {
    cur_aver_speed_ms = cur_sum_speed_ms / cur_sum_veh_cnt;
  }
}
#endif

Base::TimeParamManager BillboardTemplate::MakeTimeMgr(const Base::txFloat time_stamp) TX_NOEXCEPT {
  const Base::txFloat relativeTime = Utils::MillisecondToSecond(time_stamp - m_lastStepTime);
  m_lastStepTime = time_stamp;
  m_passTime += relativeTime;
  Base::TimeParamManager timeMgr(Utils::MillisecondToSecond(time_stamp), relativeTime, m_passTime, time_stamp);
  // LOG(WARNING) << "[simulation_time] : " << timeMgr.str() << TX_VARS(time_stamp);
  return timeMgr;
}

void BillboardTemplate::Init(tx_sim::InitHelper &helper) TX_NOEXCEPT {
  LOG(INFO) << "Subscribe Topic Start. " << TX_VARS(FLAGS_BillboardUpStreamTopics)
            << TX_VARS(FLAGS_BillboardDownStreamTopics);
  helper.Subscribe(FLAGS_BillboardUpStreamTopics);
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Publish(FLAGS_BillboardDownStreamTopics);
  //
  LOG(INFO) << "Subscribe Topic End. ";
}

sim_msg::BillboardLaneInfo BillboardTemplate::InfoLane2BillboardLaneInfo(const Base::Info_Lane_t &info_lane)
    TX_NOEXCEPT {
  sim_msg::BillboardLaneInfo res;
  if (info_lane.IsOnLane()) {
    res.mutable_id()->set_type(sim_msg::Lane);
    res.mutable_id()->mutable_fromlaneuid()->set_roadid(info_lane.onLaneUid.roadId);
    res.mutable_id()->mutable_fromlaneuid()->set_sectionid(info_lane.onLaneUid.sectionId);
    res.mutable_id()->mutable_fromlaneuid()->set_laneid(info_lane.onLaneUid.laneId);
  } else {
    res.mutable_id()->set_type(sim_msg::Link);
    res.mutable_id()->mutable_fromlaneuid()->set_roadid(info_lane.onLinkFromLaneUid.roadId);
    res.mutable_id()->mutable_fromlaneuid()->set_sectionid(info_lane.onLinkFromLaneUid.sectionId);
    res.mutable_id()->mutable_fromlaneuid()->set_laneid(info_lane.onLinkFromLaneUid.laneId);
    res.mutable_id()->mutable_tolaneuid()->set_roadid(info_lane.onLinkToLaneUid.roadId);
    res.mutable_id()->mutable_tolaneuid()->set_sectionid(info_lane.onLinkToLaneUid.sectionId);
    res.mutable_id()->mutable_tolaneuid()->set_laneid(info_lane.onLinkToLaneUid.laneId);
  }
  return std::move(res);
}

Base::txBool BillboardTemplate::ParseMapInitParams(tx_sim::ResetHelper &helper,
                                                   HdMap::HadmapCacheConCurrent::InitParams_t &refParams) TX_NOEXCEPT {
  refParams.strTrafficFilePath = helper.scenario_file_path();
  refParams.strHdMapFilePath = helper.map_file_path();
  __Lon__(refParams.SceneOriginGPS) = helper.map_local_origin().x;
  __Lat__(refParams.SceneOriginGPS) = helper.map_local_origin().y;
  __Alt__(refParams.SceneOriginGPS) = helper.map_local_origin().z;
  LogInfo << "Map File :" << refParams.strHdMapFilePath << TX_VARS(Utils::ToString(refParams.SceneOriginGPS));
  return true;
}

void BillboardTemplate::Reset(tx_sim::ResetHelper &helper) TX_NOEXCEPT {
  ResetVars();
#if 1
  HdMap::HadmapCacheConCurrent::InitParams_t initParams;
  ParseMapInitParams(helper, initParams);
  if (HdMap::HadmapCacheConCurrent::Initialize(initParams)) {
    LOG(INFO) << "connect hadmap : " << helper.map_file_path() << " success.";
    const auto SceneOriginGPS = helper.map_local_origin();
    hadmap::PointVec envelope;
    envelope.push_back(hadmap::txPoint(SceneOriginGPS.x - FLAGS_Envelope_Offset,
                                       SceneOriginGPS.y - FLAGS_Envelope_Offset, SceneOriginGPS.z));
    envelope.push_back(hadmap::txPoint(SceneOriginGPS.x + FLAGS_Envelope_Offset,
                                       SceneOriginGPS.y + FLAGS_Envelope_Offset, SceneOriginGPS.z));

    Coord::txWGS84 envelope_0(envelope[0]), envelope_1(envelope[1]);
    auto edgeLength = envelope_0.ToENU().ENU2D() - envelope_1.ToENU().ENU2D();
    const Base::txFloat envelope_area = std::fabs(edgeLength[0]) * std::fabs(edgeLength[1]);
    LOG(INFO) << TX_VARS(envelope_area) << TX_VARS(envelope_0) << TX_VARS(envelope_1);

    hadmap::txLaneLinks pLaneLinks;
    hadmap::getLaneLinks(txMapHdr, envelope, pLaneLinks);
    LOG(INFO) << "call hadmap::getLaneLinks end. " << TX_VARS_NAME(lanelink_size, pLaneLinks.size());
    bool wholeData = true;
    hadmap::txRoads pRoads;
    hadmap::getRoads(txMapHdr, envelope, wholeData, pRoads);
    LOG(INFO) << "call hadmap::getRoads end. " << TX_VARS_NAME(road_size, pRoads.size());

    tbb::parallel_for(static_cast<std::size_t>(0), pRoads.size(), [&](const std::size_t idx) {
      hadmap::txRoadPtr roadPtr = pRoads[idx];
      if (NonNull_Pointer(roadPtr)) {
        const auto &refSections = roadPtr->getSections();
        if (_NonEmpty_(refSections)) {
          for (const auto &sec_ptr : refSections) {
            if (NonNull_Pointer(sec_ptr) && _NonEmpty_(sec_ptr->getLanes())) {
              if (FLAGS_ComputeSectionData) {
                const Base::Info_Lane_t laneInfo(sec_ptr->getRoadId(), sec_ptr->getId(), 0);
                RoadNetworkStatisticsDict::accessor a;
                m_road_network_dict.insert(a, laneInfo);
              } else {
                const auto &refLanes = sec_ptr->getLanes();
                for (const auto &refLane : refLanes) {
                  if (NonNull_Pointer(refLane)) {
                    const Base::Info_Lane_t laneInfo(refLane->getTxLaneId());
                    RoadNetworkStatisticsDict::accessor a;
                    m_road_network_dict.insert(a, laneInfo);
                    // a->second.CopyFrom(InfoLane2BillboardLaneInfo(laneInfo));
                  }
                }
              }
            }
          }
        }
      }
    }); /*lamda function*/
        /* parallel_for */

    tbb::parallel_for(static_cast<std::size_t>(0), pLaneLinks.size(), [&](const std::size_t idx) {
      hadmap::txLaneLinkPtr linkptr = pLaneLinks[idx];
      if (NonNull_Pointer(linkptr)) {
        linkptr->fromTxLaneId();
        linkptr->toTxLaneId();
        const Base::Info_Lane_t laneInfo(0, linkptr->fromTxLaneId(), linkptr->toTxLaneId());
        RoadNetworkStatisticsDict::accessor a;
        m_road_network_dict.insert(a, laneInfo);
        // a->second.CopyFrom(InfoLane2BillboardLaneInfo(laneInfo));
      }
    }); /*lamda function*/
        /* parallel_for */
    LOG(INFO) << "create road network success.";
    if (FLAGS_LogLevel_Billboard) {
      for (const auto &refkv : m_road_network_dict) {
        LOG(INFO) << refkv.first;
      }
    }

    { m_isValid = m_in_ego.Initialize(helper.ego_path()); }

    if (IsValid()) {
      /*hwy entrance, charging pile*/
      m_isValid = InitializeHdMapObject(envelope);
    }
  } else {
    LogWarn << "open hdmap error. " << helper.map_file_path();
    m_isValid = false;
  }
#endif
}

void BillboardTemplate::Step(tx_sim::StepHelper &helper) TX_NOEXCEPT {
  const Base::TimeParamManager timeMgr = MakeTimeMgr(helper.timestamp());
  timeMgr.str();

  const Base::txFloat currentPassTime_ms = Utils::SecondToMillisecond(timeMgr.PassTime());
  if ((currentPassTime_ms >= FLAGS_compute_start_time_ms) && (currentPassTime_ms <= FLAGS_compute_end_time_ms)) {
    Simulation(helper, timeMgr);
  }
}

void BillboardTemplate::Stop(tx_sim::StopHelper &helper) TX_NOEXCEPT { LOG(INFO) << "call "; }

Base::txString BillboardLaneUId2Str(const sim_msg::BillboardLaneUId &_id) TX_NOEXCEPT {
  Base::txLaneUId res(_id.roadid(), _id.sectionid(), _id.laneid());
  return Utils::ToString(res);
}

Base::txString BillboardLaneIdentity2Str(const sim_msg::BillboardLaneIdentity &_id) TX_NOEXCEPT {
  std::ostringstream oss;
  oss << ((sim_msg::BillboardLaneType::Lane == _id.type()) ? ("onLane") : ("onLink"))
      << TX_VARS_NAME(fromUid, BillboardLaneUId2Str(_id.fromlaneuid()))
      << TX_VARS_NAME(toUid, BillboardLaneUId2Str(_id.tolaneuid()));
  return oss.str();
}

Base::txString BillboardRoadId2Str(const sim_msg::BillboardRoadId &_id) TX_NOEXCEPT {
  std::ostringstream oss;
  oss << ((sim_msg::BillboardLaneType::Lane == _id.type()) ? ("onLane") : ("onLink"))
      << TX_VARS_NAME(from_road_id, _id.from_road_id()) << TX_VARS_NAME(to_road_id, _id.to_road_id());
  return oss.str();
}

Base::txString BillboardSegmentId2Str(const sim_msg::BillboardSegmentId &_id) TX_NOEXCEPT {
  std::ostringstream oss;
  oss << ((sim_msg::BillboardLaneType::Lane == _id.type()) ? ("onLane") : ("onLink")) << ",";
  if ((sim_msg::BillboardLaneType::Lane == _id.type())) {
    oss << TX_VARS_NAME(road_id, _id.section_id().roadid()) << TX_VARS_NAME(section_id, _id.section_id().sectionid());
  } else {
    oss << TX_VARS_NAME(from_road_id, _id.linkset_id().from_roadid())
        << TX_VARS_NAME(to_road_id, _id.linkset_id().to_roadid());
  }
  return oss.str();
}

Base::txString BillboardSTSegmentInfo2Str(const sim_msg::BillboardSTSegmentInfo &_info) TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "[" << TX_VARS_NAME(road_link_id, BillboardSegmentId2Str(_info.segid()));
  oss << TX_VARS_NAME(start_s, _info.start_s());
  oss << TX_VARS_NAME(length, _info.length());
  oss << TX_VARS_NAME(average_vehicle_speed_kmh, _info.average_vehicle_speed_kmh());
  oss << TX_VARS_NAME(slope, _info.slope());
  oss << TX_VARS_NAME(altitude_m, _info.altitude_m());
  oss << TX_VARS_NAME(curvature, _info.curvature());
  oss << TX_VARS_NAME(speed_limit_kmh, _info.speed_limit_kmh());
  oss << "]";
  return oss.str();
}

void PrintPB(const sim_msg::Billboard &out_Billboard) TX_NOEXCEPT {
  if (FLAGS_LogLevel_Billboard_PBOutput) {
    const sim_msg::EgoNavigationInfo &navInfo = out_Billboard.ego_navigation_info();
    LOG(INFO) << TX_VARS_NAME(ego_location_info, BillboardLaneIdentity2Str(navInfo.ego_location_info()));
    LOG(INFO) << TX_VARS_NAME(ego_destination, BillboardLaneIdentity2Str(navInfo.ego_destination()));
    LOG(INFO) << TX_VARS_NAME(segment_count_to_destination, navInfo.segment_count_to_destination());
    LOG(INFO) << TX_VARS_NAME(distance_to_destination_m, navInfo.distance_to_destination_m());
    LOG(INFO) << TX_VARS_NAME(distance_to_chargingPile_m, navInfo.distance_to_chargingpile_m());
    LOG(INFO) << TX_VARS_NAME(distance_to_hwy_entrance_m, navInfo.distance_to_hwy_entrance_m());
    LOG(INFO) << TX_COND_NAME(is_reRoute, navInfo.is_reroute());

    LOG(INFO) << TX_VARS_NAME(slope, navInfo.slope());
    LOG(INFO) << TX_VARS_NAME(altitude_m, navInfo.altitude_m());
    LOG(INFO) << TX_VARS_NAME(speed_limit_kmh, navInfo.speed_limit_kmh());
    LOG(INFO) << TX_VARS_NAME(average_vehicle_speed_kmh, navInfo.average_vehicle_speed_kmh());
    LOG(INFO) << TX_VARS_NAME(curvature, navInfo.curvature());
    LOG(INFO) << TX_COND_NAME(destination_is_charging_pile, navInfo.destination_is_charging_pile());

    std::ostringstream oss_ahead_info;
    for (const auto &ref_info : navInfo.ahead_info()) {
      oss_ahead_info << BillboardSTSegmentInfo2Str(ref_info) << std::endl;
    }
    LOG(INFO) << TX_VARS_NAME(ahead_info, oss_ahead_info.str());

    for (const auto &ref_tl : navInfo.ahead_traffic_lights()) {
      LOG(INFO) << TX_VARS_NAME(tl_id, ref_tl.id()) << TX_VARS_NAME(dist2ego, ref_tl.dist_to_ego())
                << TX_VARS_NAME(s_on_route, ref_tl.s_on_path());
    }

    std::ostringstream oss_aver_speed;
    for (const auto &ref : navInfo.ahead_aver_speed_kmh_array()) {
      oss_aver_speed << "(" << ref.s_on_path() << ", " << ref.average_vehicle_speed_kmh() << "),";
    }
    LOG(INFO) << TX_VARS_NAME(ahead_aver_speed_kmh_array, oss_aver_speed.str());

    std::ostringstream oss_slope;
    for (const auto &ref : navInfo.ahead_slope_array()) {
      oss_slope << "(" << ref.s_on_path() << ", " << ref.slope() << "),";
    }
    LOG(INFO) << TX_VARS(navInfo.ahead_slope_array().size()) << TX_VARS_NAME(ahead_slope_array, oss_slope.str());
  }
}

void BillboardTemplate::Simulation(tx_sim::StepHelper &helper, const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  LogInfo << __func__;
  if (IsValid()) {
    /*1. receive traffic */
    m_out_Billboard.Clear();
    Base::txString payload_;
    GetSubscribedMessage(helper, FLAGS_BillboardUpStreamTopics, payload_);
    m_in_Traffic.ParseFromString(payload_);
    LogInfo << "receive :" << TX_VARS(m_in_Traffic.cars_size());
    /*1.1 receive location*/
    Base::txString location_payload_;
    GetSubscribedMessage(helper, tx_sim::topic::kLocation, location_payload_);
    sim_msg::Location loc;
    loc.ParseFromString(location_payload_);
    LogInfo << "receive :" << TX_VARS(loc.DebugString());

    const Base::txFloat pass_time_s = timeMgr.PassTime();
    /*2. clear last frame info*/
    tbb::parallel_for(m_road_network_dict.range(), [&](const RoadNetworkStatisticsDict::range_type &r) {
      for (RoadNetworkStatisticsDict::iterator itr = r.begin(); itr != r.end(); itr++) {
        itr->second.ClearOldInfo(pass_time_s, FLAGS_AverSpeedDuration_ms);
      }
    });

    /*3. accumulation speed, vehicle_cnt*/
    /*3.1 register ego info */
    {
      if (m_in_ego.IsValid()) {
        const Base::txFloat velocityScalar =
            Base::txVec3(loc.velocity().x(), loc.velocity().y(), loc.velocity().z()).norm();
        LogInfo << m_in_ego.Str();
        Base::Info_Lane_t ego_laneInfo;
        if (FLAGS_ComputeSectionData) {
          ego_laneInfo.FromLane(
              Base::txLaneUId(m_in_ego.LaneLocInfo().onLaneUid.roadId, m_in_ego.LaneLocInfo().onLaneUid.sectionId, 0));
        } else {
          ego_laneInfo.FromLane(m_in_ego.LaneLocInfo().onLaneUid);
        }
        RoadNetworkStatisticsDict::accessor a;
        if (CallSucc(m_road_network_dict.find(a, ego_laneInfo))) {
          a->second.RegisterVehInfo(pass_time_s, 9999999, velocityScalar);
        } else {
          LogWarn << "not find " << TX_VARS(m_in_ego.LaneLocInfo());
        }
        a.release();
      }
    }
    /*3.2 register traffic info*/
    for (const auto &refCar : m_in_Traffic.cars()) {
      Base::Info_Lane_t lane_info;
      if (refCar.tx_road_id() == refCar.to_tx_road_id()) {
        /*on lane*/
        if (FLAGS_ComputeSectionData) {
          lane_info.FromLane(Base::txLaneUId(refCar.tx_road_id(), refCar.tx_section_id(), 0));
        } else {
          lane_info.FromLane(Base::txLaneUId(refCar.tx_road_id(), refCar.tx_section_id(), refCar.tx_lane_id()));
        }
      } else {
        /*on link*/
        lane_info.FromLaneLink(
            0, Base::txLaneUId(refCar.tx_road_id(), refCar.tx_section_id(), refCar.tx_lane_id()),
            Base::txLaneUId(refCar.to_tx_road_id(), refCar.to_tx_section_id(), refCar.to_tx_lane_id()));
      }
      LogInfo << TX_VARS(refCar.id()) << TX_VARS(refCar.v()) << lane_info;
      RoadNetworkStatisticsDict::accessor a;
      if (CallSucc(m_road_network_dict.find(a, lane_info))) {
        a->second.RegisterVehInfo(pass_time_s, refCar.id(), refCar.v());
      } else {
        LogInfo << "not find " << TX_VARS(lane_info);
      }
      a.release();
    }

    /*4. compute aver speed*/
    tbb::parallel_for(m_road_network_dict.range(), [&](const RoadNetworkStatisticsDict::range_type &r) {
      for (RoadNetworkStatisticsDict::iterator itr = r.begin(); itr != r.end(); itr++) {
        if (itr->second.VehicleCountInDuration(pass_time_s) > 0) {
          itr->second.ComputeAverSpeedMS(pass_time_s);
        }
      }
    });
    /*4.1 compute navigation info*/
    m_in_ego.Update(loc, m_road_network_dict);
    m_in_ego.UpdateTrafficLights(m_in_Traffic);

    /*5. assemble billboard*/
    m_out_Billboard.Clear();
    m_out_Billboard.set_timestamp_ms(timeMgr.AbsTime());
    for (auto itr = m_road_network_dict.begin(); itr != m_road_network_dict.end(); ++itr) {
      auto ptr = m_out_Billboard.mutable_laneinfo()->Add();
      ptr->mutable_id()->CopyFrom(Billboard_Ego::LaneLocInfo2BillboardLaneIdentity(itr->first));
      ptr->mutable_info()->set_aver_speed_kmh(Unit::ms_kmh(itr->second.AverSpeedMSInDuration(pass_time_s)));
      ptr->mutable_info()->set_sum_speed_ms(itr->second.SumSpeedMSInDuration(pass_time_s));
      if (FLAGS_compute_vehicle_cnt) {
        ptr->mutable_info()->set_vehicle_cnt(itr->second.VehicleGroupByInDuration(pass_time_s));
      } else {
        ptr->mutable_info()->set_vehicle_cnt(itr->second.VehicleCountInDuration(pass_time_s));
      }
    }
    m_out_Billboard.mutable_ego_navigation_info()->CopyFrom(m_in_ego.pb(m_road_network_dict));
    /*6. send pb*/
    static Base::txString strPB;
    m_out_Billboard.SerializeToString(&strPB);
    PublishMessage(helper, FLAGS_BillboardDownStreamTopics, strPB);

    if (FLAGS_LogLevel_Billboard_PBOutput) {
      if (FLAGS_detector_print_roadid == -1) {
        for (const auto &refInfo : m_out_Billboard.laneinfo()) {
          if (refInfo.info().vehicle_cnt() > 0) {
            LOG(INFO) << TX_VARS(pass_time_s) << TX_VARS_NAME(lane_info, refInfo.id().DebugString())
                      << TX_VARS_NAME(vehicle_cnt, refInfo.info().vehicle_cnt())
                      << TX_VARS_NAME(sum_speed, refInfo.info().sum_speed_ms())
                      << TX_VARS_NAME(aver_speed, refInfo.info().aver_speed_kmh());
          }
        }
      } else {
        for (const auto &refInfo : m_out_Billboard.laneinfo()) {
          if (!refInfo.id().type() && refInfo.id().fromlaneuid().roadid() == FLAGS_detector_print_roadid &&
              refInfo.id().fromlaneuid().sectionid() == FLAGS_detector_print_sectionid) {
            LOG(INFO) << TX_VARS(pass_time_s) << TX_VARS_NAME(lane_info, refInfo.id().DebugString())
                      << TX_VARS_NAME(vehicle_cnt, refInfo.info().vehicle_cnt())
                      << TX_VARS_NAME(sum_speed, refInfo.info().sum_speed_ms())
                      << TX_VARS_NAME(aver_speed, refInfo.info().aver_speed_kmh());
          }
        }
      }
      // PrintPB(m_out_Billboard); // LOG(INFO) << TX_VARS_NAME(ego_nav_info,
      // m_out_Billboard.ego_navigation_info().DebugString());
    }
  } else {
    LogWarn << "billboard is invalid.";
  }
}

void BillboardTemplate::ResetVars() TX_NOEXCEPT {
  m_lastStepTime = 0.0;
  m_passTime = 0.0;
  m_isValid = false;
  m_in_Traffic.Clear();
  m_road_network_dict.clear();
  HdMap::HadmapCacheConCurrent::Release();
  HdMap::txRoadNetwork::Release();
  HdMap::HashedRoadCacheConCurrent::Release();
}

Base::txBool BillboardTemplate::InitializeHdMapObject(hadmap::PointVec envelope) TX_NOEXCEPT {
#if 1
  hadmap::txObjects piles;
  std::vector<hadmap::txLaneId> reliedLaneUidsNullVec;
  hadmap::getObjects(txMapHdr, reliedLaneUidsNullVec, {hadmap::OBJECT_TYPE::OBJECT_TYPE_ChargingPile}, piles);
  /*std::unordered_set< Base::Info_Lane_t, Utils::Info_Lane_t_HashCompare > lane_loc_info_set;*/
  std::set<Base::txSectionUId> section_uid_set;
  if (_NonEmpty_(piles)) {
    for (const auto &p : piles) {
      if (NonNull_Pointer(p)) {
        std::vector<hadmap::txLaneId> reliedLaneUids;
        p->getReliedLaneIds(reliedLaneUids);
        for (const auto &refLaneUid : reliedLaneUids) {
          /*lane_loc_info_set.insert(Base::Info_Lane_t(refLaneUid));*/
          if (FLAGS_pile_ignore_roadid != refLaneUid.roadId) {
            section_uid_set.insert(std::make_pair(refLaneUid.roadId, refLaneUid.sectionId));
          }
        }
        if (FLAGS_LogLevel_Billboard) {
          std::ostringstream oss;
          for (const auto &refLaneUid : reliedLaneUids) {
            oss << refLaneUid << ", ";
          }
          LOG(INFO) << TX_VARS_NAME(pile_pos, p->getPos()) << TX_VARS_NAME(pile_name, p->getName())
                    << TX_VARS_NAME(reliedLaneUids, oss.str());
        }
      }
    }

    std::vector<Base::Info_Lane_t> pile_lanes_uid;
    for (const auto &ref_section_uid : section_uid_set) {
      const hadmap::txLanes relied_lanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(ref_section_uid);
      for (const auto &refLane : relied_lanes) {
        if (NonNull_Pointer(refLane)) {
          pile_lanes_uid.emplace_back(Base::Info_Lane_t(refLane->getTxLaneId()));
        }
      }
    }
    m_in_ego.Initialize_pile(pile_lanes_uid, FLAGS_ChargingPileInfluenceDistance);
  }
#endif
  hadmap::txLanes lanes;
  std::vector<Base::Info_Lane_t> entry_lanes_uid;
  hadmap::getLanes(txMapHdr, envelope, lanes);
  for (const auto &lane_ptr : lanes) {
    if (NonNull_Pointer(lane_ptr) && hadmap::LANE_TYPE::LANE_TYPE_ENTRY == lane_ptr->getLaneType()) {
      entry_lanes_uid.emplace_back(lane_ptr->getTxLaneId());
      LOG(INFO) << TX_VARS_NAME(entry_lane, Utils::ToString(lane_ptr->getTxLaneId()));
    }
  }
  m_in_ego.Initialize_hwy(entry_lanes_uid, FLAGS_HwyEntryInfluenceDistance);
  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)
