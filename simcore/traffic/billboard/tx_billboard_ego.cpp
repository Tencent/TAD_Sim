// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_billboard_ego.h"
#include <algorithm>
#include <sstream>
#include "routeplan/route_plan.h"
#include "tx_spatial_query.h"
#include "tx_string_utils.h"
#include "tx_timer_on_cpu.h"
#include "tx_billboard_flags.h"
#include "tx_billboard_utility.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Billboard)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool Billboard_Ego::LocationEgo() TX_NOEXCEPT {
  if (IsValid()) {
    if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
            m_in_ego_pos, m_in_ego_laneInfo, m_in_ego_dist)) {
      if (_s() <= (m_trajMgr->GetLength())) {
        m_trajMgr->xy2sl(m_in_ego_pos.ToENU().ENU2D(), _s(), _t());
      }

      return true;
    } else {
      LOG(WARNING) << "Get_S_Coord_By_Enu_Pt failure";
      return false;
    }
  } else {
    LOG(WARNING) << "IsValid failure.";
    return false;
  }
}

Base::txBool IsIntersection(const std::set<Base::txRoadID>& refset1,
                            const std::set<Base::txRoadID>& refset2) TX_NOEXCEPT {
  std::vector<Base::txRoadID> v_intersection;

  std::set_intersection(refset1.begin(), refset1.end(), refset2.begin(), refset2.end(),
                        std::back_inserter(v_intersection));
  return (v_intersection.size() > 0);
}

Base::txBool Billboard_Ego::IsLocInfoInRoutePath(const Base::Info_Lane_t& _locInfo) const TX_NOEXCEPT {
  return (std::end(m_hash_set_locInfo_on_route_path) != m_hash_set_locInfo_on_route_path.find(_locInfo));
}

Base::txBool Billboard_Ego::UpdateTrafficLights(const sim_msg::Traffic& _traffic_pb) TX_NOEXCEPT {
  m_vec_path_trafficlight_info.clear();
  for (const auto& refTL : _traffic_pb.trafficlights()) {
    Coord::txWGS84 gps;
    gps.FromWGS84(refTL.x(), refTL.y());
    Base::Info_Lane_t locInfo;
    Base::txFloat s, l;
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(gps, locInfo, s);
    if (IsLocInfoInRoutePath(locInfo) && CallFail(locInfo == m_ego_end_laneInfo)) {
      m_trajMgr->xy2sl(gps.ToENU().ENU2D(), s, l);
      sim_msg::TrafficLigthInfo tl_on_path;
      tl_on_path.set_s_on_path(s);
      tl_on_path.set_dist_to_ego(s - _s());
      tl_on_path.set_id(refTL.id());
      tl_on_path.set_lon(refTL.x());
      tl_on_path.set_lat(refTL.y());
      tl_on_path.set_alt(refTL.z());
      tl_on_path.set_color(refTL.color());
      tl_on_path.set_age(refTL.age());
      tl_on_path.set_next_color(refTL.next_color());
      tl_on_path.set_next_age(refTL.next_age());
      LOG_IF(INFO, FLAGS_LogLevel_Billboard_Signal) << tl_on_path.DebugString();
      m_vec_path_trafficlight_info.emplace_back(tl_on_path);
    }
  }
  return true;
}

Base::txBool Billboard_Ego::Update(const sim_msg::Location& _loc,
                                   const RoadNetworkStatisticsDict& _road_network_info) TX_NOEXCEPT {
  const Base::Info_Lane_t last_ego_lane_loc_info = m_in_ego_laneInfo;
  m_in_Location.CopyFrom(_loc);
  m_in_ego_pos.FromWGS84(m_in_Location.position().x(), m_in_Location.position().y(), m_in_Location.position().z());

  /*if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().
          Get_S_Coord_By_Enu_Pt(m_in_ego_pos, m_in_ego_laneInfo, m_in_ego_dist))*/
  if (LocationEgo()) {
    LogInfo << "[Ego_Location]" << TX_VARS(m_in_ego_pos) << TX_VARS(m_in_ego_laneInfo) << TX_VARS(m_in_ego_dist);
    if (FLAGS_ForceUpdatePerStep || m_nav_info.NeedReCompute(m_in_ego_pos) ||
        !(last_ego_lane_loc_info == m_in_ego_laneInfo)) {
      const Base::txInt _sec_cnt = ComputeSectionInfoToDestination(_road_network_info);
      const Base::txFloat _d2e = ComputeDistanceToDestination();
      LOG(INFO) << "[_d2e__d2e]" << TX_VARS(_d2e);
      Base::txFloat _d2h = m_hwy.query_dist_to_hwy_entry(m_in_ego_laneInfo);
      if (_d2h >= m_in_ego_dist) {
        LogInfo << TX_VARS(_d2h) << TX_VARS(m_in_ego_dist) << TX_VARS(m_in_ego_laneInfo);
        _d2h -= m_in_ego_dist;
      } else if (_d2h >= 0.0) {
        _d2h = 0.0;
        LogInfo << TX_VARS(_d2h) << TX_VARS(m_in_ego_dist) << TX_VARS(m_in_ego_laneInfo);
      } else {
        LogInfo << "hwy entry query failure." << m_in_ego_laneInfo;
      }

      if (_d2h > FLAGS_HwyEntryInfluenceDistance * 2.0) {
        _d2h = -1;
      }

      Base::txFloat _d2p = m_pile.query_dist_to_hwy_entry(m_in_ego_laneInfo);
      if (_d2p >= m_in_ego_dist) {
        LogInfo << TX_VARS(_d2p) << TX_VARS(m_in_ego_dist) << TX_VARS(m_in_ego_laneInfo);
        _d2p -= m_in_ego_dist;
      } else if (_d2p >= 0.0) {
        _d2p = 0.0;
        LogInfo << TX_VARS(_d2p) << TX_VARS(m_in_ego_dist) << TX_VARS(m_in_ego_laneInfo);
      } else {
        LogInfo << "charging pile query failure." << m_in_ego_laneInfo;
      }
      if (_d2p > FLAGS_ChargingPileInfluenceDistance * 2.0) {
        _d2p = -1;
      }
      m_nav_info.Update(m_in_ego_pos, _sec_cnt, _d2e, _d2h, _d2p);
      /*ComputeAheadStaticMapInfo();*/
      LogInfo << "Billboard_Ego::Update success update navigation info.";
    } else {
      ComputeSectionInfoToDestination(_road_network_info);
      LogInfo << "Billboard_Ego::Update success without compute navigation info.";
    }

    m_isValid = true;
  } else {
    m_isValid = false;
    LogWarn << "Billboard_Ego::Update failure." << m_in_Location.DebugString();
  }
  return IsValid();
}

const Base::txString Billboard_Ego::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_VARS_NAME(ego_locLaneInfo, m_in_ego_laneInfo) << TX_VARS(m_in_ego_pos) << TX_VARS(Speed());
  return oss.str();
}

void Billboard_Ego::Clear() TX_NOEXCEPT {
  m_isValid = false;
  m_in_Location.Clear();
  m_in_ego_dist = 0.0;
  m_way_points.clear();
  m_vec_route_path_roadid.clear();
  m_nav_info.Clear();
  m_hwy.Clear();
  m_pile.Clear();
  /*m_ahead_section_lst.clear();*/
  m_ahead_slope.clear();
  m_ahead_altitude.clear();
  m_ahead_curvature.clear();
  m_ahead_speed_limit.clear();
  m_slope = 0.0;
  m_altitude = 0.0;
  m_curvature = 0.0;
  m_speed_limit = 0.0;
  m_aver_speed = 0.0;
  /*m_vec_candidate_lane.clear();*/
  m_vec_ahead_st_segment_info.clear();
  // m_vec_path_aver_speed_info.clear();
  m_vec_path_slope_info.clear();
  m_vec_path_trafficlight_info.clear();
  m_vec_path_locInfo.clear();
}

Base::txBool Billboard_Ego::ComputeSegmentInfo(const Base::txSectionUId& _secUid, Base::LocInfoSet& locInfoSet,
                                               Coord::txENU& segmentEndPt) TX_NOEXCEPT {
  locInfoSet.clear();
  auto sec_ptr = HdMap::HadmapCacheConCurrent::GetTxSectionPtr(_secUid);
  if (NonNull_Pointer(sec_ptr)) {
    Base::txVec2 endPt;
    endPt.setZero();
    for (const auto refLanePtr : sec_ptr->getLanes()) {
      if (Utils::IsLaneValidDriving(refLanePtr)) {
        Base::Info_Lane_t locInfo;
        locInfo.FromLane(refLanePtr->getTxLaneId());
        locInfoSet.insert(locInfo);
        Coord::txWGS84 gps;
        gps.FromWGS84(refLanePtr->getGeometry()->getEnd());
        endPt += gps.ToENU().ENU2D();
      }
    }
    if (_NonEmpty_(locInfoSet)) {
      endPt /= locInfoSet.size();
      segmentEndPt.FromENU(endPt.x(), endPt.y());
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool Billboard_Ego::ComputeSegmentInfo(const Base::Info_Lane_t& _linkUid, Base::LocInfoSet& locInfoSet,
                                               Coord::txENU& segmentEndPt) TX_NOEXCEPT {
  locInfoSet.clear();
  if (_linkUid.IsOnLaneLink()) {
    locInfoSet = HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(_linkUid.onLinkFromLaneUid.roadId,
                                                                         _linkUid.onLinkToLaneUid.roadId);
    Base::txVec2 endPt;
    endPt.setZero();
    Base::txInt count = 0;
    for (const auto locInfo : locInfoSet) {
      auto link_ptr =
          HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(locInfo.onLinkFromLaneUid, locInfo.onLinkToLaneUid);
      if (NonNull_Pointer(link_ptr) && NonNull_Pointer(link_ptr->getGeometry())) {
        Coord::txWGS84 gps;
        gps.FromWGS84(link_ptr->getGeometry()->getEnd());
        endPt += gps.ToENU().ENU2D();
        count++;
      }
    }
    if (count > 0) {
      endPt /= count;
      segmentEndPt.FromENU(endPt.x(), endPt.y());
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool Billboard_Ego::Initialize(const std::vector<tx_sim::Vector3d>& egp_path) TX_NOEXCEPT {
  Clear();
  if (egp_path.size() < 2) {
    LogWarn << "ego_path less than 2." << TX_VARS(egp_path.size());
    return false;
  }
  std::vector<hadmap::txPoint> pointList;
  for (const auto& refPt : egp_path) {
    const hadmap::txPoint gps(refPt.x, refPt.y, refPt.z);
    pointList.emplace_back(gps);
    m_way_points.emplace_back(Coord::txWGS84(gps));
  }
#if 1
  control_path_node_vec ref_control_path;
  for (const auto& pt : pointList) {
    control_path_node node;
    node.waypoint = pt;
    node.speed_m_s = 0.0;
    ref_control_path.emplace_back(node);
  }
  if (CallSucc(InitializeTrajMgr(ref_control_path))) {
    const Base::txFloat traj_length = m_trajMgr->GetLength();
    Base::txFloat s = 0.0;
    const Base::txFloat step = 1.0;
    std::vector<ego_path_locInfo> road_sample_pts;
    {
      ego_path_locInfo tmpNode;
      tmpNode.enu.FromENU(m_trajMgr->GetLocalPos(0.0));
      tmpNode.s_on_path = 0.0;
      road_sample_pts.emplace_back(tmpNode);
      s += step;
    }
    while (s < traj_length) {
      ego_path_locInfo tmpNode;
      tmpNode.enu.FromENU(m_trajMgr->GetLocalPos(s));
      tmpNode.s_on_path = s;
      road_sample_pts.emplace_back(tmpNode);
      s += step;
    }
#  if USE_TBB
    tbb::parallel_for(static_cast<std::size_t>(0), road_sample_pts.size(), [&](const std::size_t idx) {
      ego_path_locInfo& refNode = road_sample_pts[idx];
      Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
          refNode.enu.ToWGS84(), refNode.locinfo, refNode.s_on_laneInfo);
    });  /*lamda function*/
         /* parallel_for */
#  endif /*USE_TBB*/
    m_vec_path_locInfo.clear();
    if (_NonEmpty_(road_sample_pts) && CallSucc(road_sample_pts.front().locinfo.IsOnLane()) &&
        CallSucc(road_sample_pts.back().locinfo.IsOnLane())) {
      /*1. erase locinfo on lanelink*/
      for (auto itr = std::begin(road_sample_pts); itr != std::end(road_sample_pts);) {
        if ((*itr).locinfo.IsOnLaneLink()) {
          itr = road_sample_pts.erase(itr);
        } else {
          ++itr;
        }
      }

      std::unordered_set<Base::Info_Lane_t, Utils::Info_Lane_t_HashCompare> tmp_hashset;

      for (const auto& ref : road_sample_pts) {
        if (_NonEmpty_(m_vec_path_locInfo) && CallSucc(m_vec_path_locInfo.back().IsOnLaneLink())) {
          LogWarn << "[ERROR] " << TX_COND(CallSucc(m_vec_path_locInfo.back().IsOnLaneLink()))
                  << m_vec_path_locInfo.back();
        }
        if (ref.locinfo.IsOnLane(/*always be true*/) && CallSucc(0 == tmp_hashset.count(ref.locinfo))) {
          if (_NonEmpty_(m_vec_path_locInfo) && CallSucc(m_vec_path_locInfo.back().IsOnLane(/*always be true*/)) &&
              CallSucc(ref.locinfo.onLaneUid.roadId != m_vec_path_locInfo.back().onLaneUid.roadId)) {
            ego_path_locInfo linkNode;
            linkNode.s_on_laneInfo = -1.0;
            linkNode.s_on_path = -1.0;
            linkNode.locinfo.FromLaneLink(0, m_vec_path_locInfo.back().onLaneUid, ref.locinfo.onLaneUid);
            m_vec_path_locInfo.emplace_back(linkNode.locinfo);
          }
          TX_MARK("may be route_path from 5.2.-1-->5.2.-2");
          m_vec_path_locInfo.emplace_back(ref.locinfo);
          tmp_hashset.insert(ref.locinfo);
        }
      }

      for (const auto& ref : m_vec_path_locInfo) {
        LOG(INFO) << "[route_locInfo_list]" << ref;
      }

      {
        std::set<Base::txSectionUId /*, Utils::SectionUIdHashCompare*/> tmp_sectionSet;
        m_vec_segment_endPt_on_route_path.clear();
        m_hash_set_locInfo_on_route_path.clear();
        for (Base::txSize idx = 0; idx < m_vec_path_locInfo.size(); ++idx) {
          const auto& ref = m_vec_path_locInfo[idx];
          if (ref.IsOnLane()) {
            Base::txSectionUId curSectionUid;
            curSectionUid.first = ref.onLaneUid.roadId;
            curSectionUid.second = ref.onLaneUid.sectionId;
            if (0 == tmp_sectionSet.count(curSectionUid)) {
              SegmentLocInfoOnRoutePath lane_node;
              lane_node.m_type = SegmentLocInfoOnRoutePath::SegmentLocInfoType::eSection;
              lane_node.m_sectionUid_or_LinkId = ref;
              if (CallSucc(ComputeSegmentInfo(curSectionUid, lane_node.m_locInfoSet, lane_node.m_segment_endPt))) {
                tmp_sectionSet.insert(curSectionUid);
                if (idx < (m_vec_path_locInfo.size() - 1)) {
                  /*not last node*/
                  m_trajMgr->xy2sl(lane_node.m_segment_endPt.ENU2D(), lane_node.m_segment_endPt_s_on_route_path,
                                   lane_node.m_segment_endPt_t_on_route_path);
                } else {
                  lane_node.m_segment_endPt_s_on_route_path = m_trajMgr->GetLength();
                  lane_node.m_segment_endPt_t_on_route_path = 0.0;
                }

                LOG(INFO) << "[SUCC] " << lane_node.Str();
                m_hash_set_locInfo_on_route_path.insert(lane_node.m_locInfoSet.begin(), lane_node.m_locInfoSet.end());
                m_vec_segment_endPt_on_route_path.emplace_back(lane_node);
              } else {
                LogWarn << "[ERROR] segment on route path failure. " << TX_VARS_NAME(roadId, curSectionUid.first)
                        << TX_VARS_NAME(sectionId, curSectionUid.second);
              }
            }
          } else {
            SegmentLocInfoOnRoutePath link_node;
            link_node.m_type = SegmentLocInfoOnRoutePath::SegmentLocInfoType::eLinkSet;
            link_node.m_sectionUid_or_LinkId = ref;
            if (CallSucc(ComputeSegmentInfo(ref, link_node.m_locInfoSet, link_node.m_segment_endPt))) {
              m_trajMgr->xy2sl(link_node.m_segment_endPt.ENU2D(), link_node.m_segment_endPt_s_on_route_path,
                               link_node.m_segment_endPt_t_on_route_path);
              LOG(INFO) << "[SUCC] " << link_node.Str();
              m_vec_segment_endPt_on_route_path.emplace_back(link_node);
            } else {
              LogWarn << "[ERROR] segment on route path failure. " << TX_VARS_NAME(linkId, ref);
            }
          }
        }
      }

      std::set<Base::txRoadID> tmp_set;
      m_vec_route_path_roadid.clear();
      for (const auto& ref : m_vec_path_locInfo) {
        if (ref.IsOnLane() && CallSucc(0 == tmp_set.count(ref.onLaneUid.roadId))) {
          m_vec_route_path_roadid.emplace_back(ref.onLaneUid.roadId);
          m_vec_route_path_roadid.emplace_back(-1);
          tmp_set.insert(ref.onLaneUid.roadId);
        }
      }
      if (-1 == m_vec_route_path_roadid.back()) {
        m_vec_route_path_roadid.erase(m_vec_route_path_roadid.end() - 1);
      }
      std::ostringstream oss;
      std::copy(m_vec_route_path_roadid.begin(), m_vec_route_path_roadid.end(),
                std::ostream_iterator<Base::txRoadID>(oss, ", "));
      LOG(INFO) << TX_VARS_NAME(route_path_road_id_list, oss.str());
    } else {
      LogWarn << TX_COND((road_sample_pts.empty())) << TX_COND(road_sample_pts.front().locinfo.IsOnLane())
              << TX_COND(road_sample_pts.back().locinfo.IsOnLane());
    }
  } else {
    LogWarn << "[ERROR] route trajectory create failure.";
  }
#endif

  {
    m_ego_start.FromWGS84(pointList.front().x, pointList.front().y, pointList.front().z);
    if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
            m_ego_start, m_ego_start_laneInfo, m_ego_start_dist_in_lane) &&
        CallSucc(m_ego_start_laneInfo.IsOnLane())) {
      m_ego_start_dist_in_road = m_ego_start_dist_in_lane;
      for (int secIdx = 0; secIdx < m_ego_start_laneInfo.onLaneUid.sectionId; ++secIdx) {
        auto sec_ptr = HdMap::HadmapCacheConCurrent::GetTxSectionPtr(
            std::make_pair(m_ego_start_laneInfo.onLaneUid.roadId, secIdx));
        if (NonNull_Pointer(sec_ptr)) {
          m_ego_start_dist_in_road += sec_ptr->getLength();
        }
      }
      LogInfo << "route start info. " << TX_VARS(m_ego_start) << TX_VARS(m_ego_start_laneInfo)
              << TX_VARS(m_ego_start_dist_in_lane) << TX_VARS(m_ego_start_dist_in_road);
    } else {
      LogWarn << "route start point failure." << TX_VARS(m_ego_start) << TX_COND(m_ego_start_laneInfo.IsOnLane());
      return false;
    }
  }
  {
    m_ego_end.FromWGS84(pointList.back().x, pointList.back().y, pointList.back().z);
    if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
            m_ego_end, m_ego_end_laneInfo, m_ego_end_dist) &&
        CallSucc(m_ego_end_laneInfo.IsOnLane())) {
      LogInfo << "route end info. " << TX_VARS(m_ego_end) << TX_VARS(m_ego_end_laneInfo) << TX_VARS(m_ego_end_dist);
    } else {
      LogWarn << "route end point failure." << TX_VARS(m_ego_end) << TX_COND(m_ego_end_laneInfo.IsOnLane());
      return false;
    }
  }
  InitializeSlope();
  m_isValid = true;
  return IsValid();
}

Base::txBool Billboard_Ego::Initialize_hwy(const std::vector<Base::Info_Lane_t>& entry_lanes_uid,
                                           const Base::txFloat _search_dist) TX_NOEXCEPT {
  return m_hwy.Initialize(entry_lanes_uid, _search_dist, true);
}

Base::txBool Billboard_Ego::Initialize_pile(const std::vector<Base::Info_Lane_t>& pile_lanes_uid,
                                            const Base::txFloat _search_dist) TX_NOEXCEPT {
  if (std::end(pile_lanes_uid) != std::find(std::begin(pile_lanes_uid), std::end(pile_lanes_uid), m_ego_end_laneInfo)) {
    m_destination_is_charging_pile = true;
    LogWarn << "ego destination is charging pile " << m_ego_end_laneInfo;
  } else {
    m_destination_is_charging_pile = false;
  }
  return m_pile.Initialize(pile_lanes_uid, _search_dist, false);
}

Base::txBool Billboard_Ego::IsEgoOnRouting() const TX_NOEXCEPT {
  if (IsValid()) {
    if (ego_on_lane()) {
      return RoadIdInRoute(LaneLocInfo().onLaneUid.roadId);
    } else if (ego_on_lanelink()) {
      return RoadIdInRoute(LaneLocInfo().onLinkToLaneUid.roadId) ||
             RoadIdInRoute(LaneLocInfo().onLinkFromLaneUid.roadId);
    } else {
      return false;
    }
  } else {
    return false;
  }
}

sim_msg::BillboardLaneStatisticsInfo Billboard_Ego::ComputeSegmentStatisticsInfo(
    const RoadNetworkStatisticsDict& _road_network_info, const Base::LocInfoSet _locSet) TX_NOEXCEPT {
  sim_msg::BillboardLaneStatisticsInfo res_pb;
  Base::txFloat res_segment_aver_speed_kmh = 0.0;
  Base::txInt segment_sum_veh_cnt = 0;
  Base::txFloat segment_sum_veh_speed_ms = 0.0;
  for (const auto& locInfo : _locSet) {
    RoadNetworkStatisticsDict::const_accessor ca;
    if (_road_network_info.find(ca, locInfo)) {
      segment_sum_veh_cnt += ca->second.VehicleCountInDuration(0.0);
      segment_sum_veh_speed_ms += ca->second.SumSpeedMSInDuration(0.0);
      LogInfo << TX_VARS(segment_sum_veh_cnt) << TX_VARS(segment_sum_veh_speed_ms);
    } else {
      LogInfo << "no find info :" << locInfo;
    }
  }
  if (segment_sum_veh_cnt > 0) {
    res_segment_aver_speed_kmh = Unit::ms_kmh(segment_sum_veh_speed_ms / segment_sum_veh_cnt);
  }

  res_pb.set_aver_speed_kmh(res_segment_aver_speed_kmh);
  res_pb.set_sum_speed_ms(segment_sum_veh_speed_ms);
  res_pb.set_vehicle_cnt(segment_sum_veh_cnt);
  LogInfo << TX_VARS(res_segment_aver_speed_kmh) << TX_VARS(segment_sum_veh_speed_ms) << TX_VARS(segment_sum_veh_cnt);
  return std::move(res_pb);
}

sim_msg::BillboardSegmentId Billboard_Ego::LaneLocInfo2SegmentId(const Base::Info_Lane_t _lane_loc_info) TX_NOEXCEPT {
  sim_msg::BillboardSegmentId res;
  if (_lane_loc_info.IsOnLane()) {
    res.set_type(sim_msg::Lane);
    res.mutable_section_id()->set_roadid(_lane_loc_info.onLaneUid.roadId);
    res.mutable_section_id()->set_sectionid(_lane_loc_info.onLaneUid.sectionId);
  } else {
    res.set_type(sim_msg::Link);
    res.mutable_linkset_id()->set_from_roadid(_lane_loc_info.onLinkFromLaneUid.roadId);
    res.mutable_linkset_id()->set_to_roadid(_lane_loc_info.onLinkToLaneUid.roadId);
  }
  return std::move(res);
}

sim_msg::BillboardLaneUId Billboard_Ego::LaneUid2BillboardLaneUId(const Base::txLaneUId _laneUid) TX_NOEXCEPT {
  sim_msg::BillboardLaneUId res;
  res.set_roadid(_laneUid.roadId);
  res.set_sectionid(_laneUid.sectionId);
  res.set_laneid(_laneUid.laneId);
  return std::move(res);
}

sim_msg::BillboardLaneIdentity Billboard_Ego::LaneLocInfo2BillboardLaneIdentity(const Base::Info_Lane_t _lane_loc_info)
    TX_NOEXCEPT {
  sim_msg::BillboardLaneIdentity res;
  if (_lane_loc_info.IsOnLane()) {
    res.set_type(sim_msg::Lane);
    res.mutable_fromlaneuid()->CopyFrom(LaneUid2BillboardLaneUId(_lane_loc_info.onLaneUid));
    res.mutable_tolaneuid()->CopyFrom(LaneUid2BillboardLaneUId(_lane_loc_info.onLaneUid));
  } else {
    res.set_type(sim_msg::Link);
    res.mutable_fromlaneuid()->CopyFrom(LaneUid2BillboardLaneUId(_lane_loc_info.onLinkFromLaneUid));
    res.mutable_tolaneuid()->CopyFrom(LaneUid2BillboardLaneUId(_lane_loc_info.onLinkToLaneUid));
  }
  return std::move(res);
}

std::tuple<Base::LocInfoSet, sim_msg::BillboardSectionInfo> Billboard_Ego::ComputeSingleSegmentInfo(
    const Base::Info_Lane_t& _loc_info, const RoadNetworkStatisticsDict& _road_network_info) TX_NOEXCEPT {
  std::tuple<Base::LocInfoSet, sim_msg::BillboardSectionInfo> res_tuple;
  sim_msg::BillboardSectionInfo& segment_node = std::get<1>(res_tuple);
  Base::LocInfoSet& locInfoSet = std::get<0>(res_tuple);
  segment_node.mutable_segment_id()->CopyFrom(LaneLocInfo2SegmentId(_loc_info));
  if (_loc_info.IsOnLane()) {
    auto lanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(
        std::make_pair(_loc_info.onLaneUid.roadId, _loc_info.onLaneUid.sectionId));
    for (const auto& lane_ptr : lanes) {
      if (NonNull_Pointer(lane_ptr)) {
        locInfoSet.insert(Base::Info_Lane_t(lane_ptr->getTxLaneId()));
      }
    }
  } else {
    locInfoSet = HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(_loc_info.onLinkFromLaneUid.roadId,
                                                                         _loc_info.onLinkToLaneUid.roadId);
  }
  LogInfo << "[ooo]" << TX_VARS_NAME(segment_id, _loc_info) << TX_VARS(locInfoSet.size());
  segment_node.mutable_info()->CopyFrom(ComputeSegmentStatisticsInfo(_road_network_info, locInfoSet));
  return std::move(res_tuple);
}

Base::txFloat Billboard_Ego::GetSectionStartLength(const Base::txRoadID _rid, const Base::txInt _sid) TX_NOEXCEPT {
  Base::txFloat res = 0.0;
  auto road_ptr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(_rid);
  if (NonNull_Pointer(road_ptr)) {
    const auto& refSections = road_ptr->getSections();
    const Base::txInt nSecSize = refSections.size();
    for (Base::txInt idx = 0; idx < _sid && idx < nSecSize; ++idx) {
      res += refSections[idx]->getLength();
    }
    return res;
  } else {
    LogWarn << "error road id : " << _rid;
    return 0.0;
  }
}

Base::txFloat Billboard_Ego::compute_lanelink_length(const Base::txRoadID from_rid,
                                                     const Base::txRoadID to_rid) const TX_NOEXCEPT {
  auto lanelocSet = HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(from_rid, to_rid);
  for (const auto& refLaneLocInfo : lanelocSet) {
    const Base::txFloat link_length = HdMap::HadmapCacheConCurrent::GetLaneLinkLength(refLaneLocInfo);
    LogInfo << TX_VARS(refLaneLocInfo) << TX_VARS(link_length);
    return link_length;
  }
  return 0.0;
}

Base::txFloat Billboard_Ego::ComputeDistanceToDestination() const TX_NOEXCEPT {
  if (/*IsEgoOnRouting() &&*/ IsValid()) {
    return (m_trajMgr->GetLength() - _s());
  } else {
    LogWarn << "ComputeSectionCountToDestination failure. "
            << " IsEgoOnRouting false.";
    return 0.0;
  }
}

sim_msg::EgoNavigationInfo Billboard_Ego::pb(const RoadNetworkStatisticsDict& _road_network_info) TX_NOEXCEPT {
  sim_msg::EgoNavigationInfo res_pb;
  res_pb.set_ego_id(0);
  res_pb.set_ego_name("ego");
  res_pb.mutable_ego_location_info()->CopyFrom(LaneLocInfo2BillboardLaneIdentity(LaneLocInfo()));
  {
    auto geom_ptr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(LaneLocInfo());
    if (NonNull_Pointer(geom_ptr)) {
      res_pb.set_ego_location_segment_length(geom_ptr->GetLength());
    } else {
      res_pb.set_ego_location_segment_length(0.0);
    }
  }
  res_pb.mutable_ego_destination()->CopyFrom(LaneLocInfo2BillboardLaneIdentity(m_ego_end_laneInfo));
  res_pb.set_segment_count_to_destination(m_nav_info.sec_cnt_to_end());
  res_pb.set_distance_to_destination_m(m_nav_info.dist_to_end());
  res_pb.set_distance_to_chargingpile_m(m_nav_info.dist_to_pile());
  res_pb.set_distance_to_hwy_entrance_m(m_nav_info.dist_to_hwy_entrance());
  res_pb.set_is_reroute(false);
  res_pb.set_destination_is_charging_pile(m_destination_is_charging_pile);

  if (_NonEmpty_(m_vec_ahead_st_segment_info)) {
    const auto& first_pb = m_vec_ahead_st_segment_info.front();
    res_pb.set_average_vehicle_speed_kmh(first_pb.average_vehicle_speed_kmh());
    res_pb.set_slope(first_pb.slope());
    res_pb.set_curvature(first_pb.curvature());
    res_pb.set_altitude_m(first_pb.altitude_m());
    res_pb.set_speed_limit_kmh(first_pb.speed_limit_kmh());

    for (const auto& refPb : m_vec_ahead_st_segment_info) {
      auto ptr = res_pb.mutable_ahead_info()->Add();
      ptr->CopyFrom(refPb);
    }
  }

  for (int i = 0; i < m_vec_segment_endPt_on_route_path.size(); ++i) {
    auto new_info_node_ptr = res_pb.mutable_ahead_aver_speed_kmh_array()->Add();
    const auto res = compute_average_vehicle_speed_kmh(_road_network_info,
                                                       (m_vec_segment_endPt_on_route_path[i].m_sectionUid_or_LinkId));
    new_info_node_ptr->set_average_vehicle_speed_kmh(res);
    new_info_node_ptr->set_s_on_path(m_vec_segment_endPt_on_route_path[i].m_segment_endPt_s_on_route_path);
  }

  for (int i = 0; i < m_vec_path_slope_info.size(); ++i) {
    const Base::txFloat s = std::get<0>(m_vec_path_slope_info[i]);
    const Base::txFloat slope = std::get<1>(m_vec_path_slope_info[i]);
    auto new_slope_node_ptr = res_pb.mutable_ahead_slope_array()->Add();
    new_slope_node_ptr->set_s_on_path(s);
    new_slope_node_ptr->set_slope(slope);
  }

  for (size_t i = 0; i < m_vec_path_trafficlight_info.size(); i++) {
    auto new_light_ptr = res_pb.mutable_ahead_traffic_lights()->Add();
    new_light_ptr->CopyFrom(m_vec_path_trafficlight_info[i]);
  }

  return std::move(res_pb);
}

void Billboard_Ego::make_slope(const sim_msg::BillboardRoadId& id, const hadmap::txSlopeVec& slopeVec) TX_NOEXCEPT {
  if (_NonEmpty_(slopeVec) && compute_slope()) {
    sim_msg::BillboardRoadInfo cur_pb;
    cur_pb.mutable_road_link_id()->CopyFrom(id);
    for (const auto& slop : slopeVec) {
      auto slope_pb_ptr = cur_pb.mutable_info()->Add();
      slope_pb_ptr->set_value_type(sim_msg::BillboardRoadInfoType::Slope);
      slope_pb_ptr->set_start_s(slop.m_starts);
      slope_pb_ptr->set_length(slop.m_length);
      slope_pb_ptr->set_value(slop.m_slope);
      LogInfo << TX_VARS_NAME(from_road_id, id.from_road_id()) << TX_VARS_NAME(to_road_id, id.to_road_id())
              << TX_VARS_NAME(start_s, slop.m_starts) << TX_VARS_NAME(length, slop.m_length)
              << TX_VARS_NAME(slope, slop.m_slope);
    }
    m_ahead_slope.emplace_back(cur_pb);
  }
}

void Billboard_Ego::make_curvature(const sim_msg::BillboardRoadId& id,
                                   const hadmap::txCurvatureVec& curvatureVec) TX_NOEXCEPT {
  if (_NonEmpty_(curvatureVec) && compute_curvature()) {
    sim_msg::BillboardRoadInfo cur_pb;
    cur_pb.mutable_road_link_id()->CopyFrom(id);
    for (const auto& curvature : curvatureVec) {
      auto curvature_pb_ptr = cur_pb.mutable_info()->Add();
      curvature_pb_ptr->set_value_type(sim_msg::BillboardRoadInfoType::Curvature);
      curvature_pb_ptr->set_start_s(curvature.m_starts);
      curvature_pb_ptr->set_length(curvature.m_length);
      curvature_pb_ptr->set_value(curvature.m_curvature);
      LogInfo << TX_VARS_NAME(from_road_id, id.from_road_id()) << TX_VARS_NAME(to_road_id, id.to_road_id())
              << TX_VARS_NAME(start_s, curvature.m_starts) << TX_VARS_NAME(length, curvature.m_length)
              << TX_VARS_NAME(curvature, curvature.m_curvature);
    }
    m_ahead_curvature.emplace_back(cur_pb);
  }
}

void Billboard_Ego::make_speed_limit(const sim_msg::BillboardRoadId& id, const Base::txFloat _curveLen,
                                     const Base::txFloat _speedLimit) TX_NOEXCEPT {
  if (compute_speed_limit()) {
    sim_msg::BillboardRoadInfo cur_pb;
    cur_pb.mutable_road_link_id()->CopyFrom(id);
    auto curvature_pb_ptr = cur_pb.mutable_info()->Add();
    curvature_pb_ptr->set_value_type(sim_msg::BillboardRoadInfoType::SpeedLimit_kmh);
    curvature_pb_ptr->set_start_s(0.0);
    curvature_pb_ptr->set_length(_curveLen);
    curvature_pb_ptr->set_value(_speedLimit);
    m_ahead_speed_limit.emplace_back(cur_pb);
    LogInfo << TX_VARS_NAME(from_road_id, id.from_road_id()) << TX_VARS_NAME(to_road_id, id.to_road_id())
            << TX_VARS_NAME(start_s, 0.0) << TX_VARS_NAME(length, _curveLen) << TX_VARS_NAME(speedLimit, _speedLimit);
  }
}

hadmap::txPoint GetRoadPoint(hadmap::txRoadPtr _rid_ptr, const Base::txFloat _s) TX_NOEXCEPT {
  const auto& refsections = _rid_ptr->getSections();
  Base::txFloat cur_s = 0.0;
  for (const auto& refSec : refsections) {
    if (_s >= cur_s && _s < (cur_s + refSec->getLanes().front()->getLength())) {
      const Base::txFloat sub_s = _s - cur_s;
      return refSec->getLanes().front()->getGeometry()->getPoint(sub_s);
    }
    cur_s += refSec->getLanes().front()->getLength();
  }
  return _rid_ptr->getSections().back()->getLanes().front()->getGeometry()->getEnd();
}

std::vector<Billboard_Ego::AltitudeNode> Billboard_Ego::GetAltitudeNodeVec(hadmap::txRoadPtr _rid_ptr) TX_NOEXCEPT {
  std::vector<Billboard_Ego::AltitudeNode> resVec;
  if (NonNull_Pointer(_rid_ptr)) {
    const Base::txFloat road_length = _rid_ptr->getLength();
    Base::txFloat s = 0.0;
    while (s < road_length && (s + AltitudeStep()) < road_length) {
      const Base::txFloat sub_aver_altitude =
          0.5 * ((GetRoadPoint(_rid_ptr, s).z) + (GetRoadPoint(_rid_ptr, s + AltitudeStep()).z));
      Billboard_Ego::AltitudeNode node;
      node.m_starts = s;
      node.m_length = AltitudeStep();
      node.m_altitude = sub_aver_altitude;
      resVec.emplace_back(std::move(node));
      s += AltitudeStep();
    }
    if (s < road_length && s + AltitudeStep() > road_length) {
      const Base::txFloat sub_aver_altitude =
          0.5 * ((GetRoadPoint(_rid_ptr, s).z) + (GetRoadPoint(_rid_ptr, road_length).z));
      Billboard_Ego::AltitudeNode node;
      node.m_starts = s;
      node.m_length = road_length - s;
      node.m_altitude = sub_aver_altitude;
      resVec.emplace_back(std::move(node));
    }
  }
  return std::move(resVec);
}

std::vector<Billboard_Ego::AltitudeNode> Billboard_Ego::GetAltitudeNodeVec(hadmap::txLaneLinkPtr _link_ptr)
    TX_NOEXCEPT {
  std::vector<Billboard_Ego::AltitudeNode> resVec;
  if (NonNull_Pointer(_link_ptr)) {
    auto geom_ptr = _link_ptr->getGeometry();
    const Base::txFloat linkLength = geom_ptr->getLength();
    Base::txFloat s = 0.0;
    while (s < linkLength && (s + AltitudeStep()) < linkLength) {
      const Base::txFloat sub_aver_altitude =
          0.5 * ((geom_ptr->getPoint(s).z) + (geom_ptr->getPoint(s + AltitudeStep()).z));
      Billboard_Ego::AltitudeNode node;
      node.m_starts = s;
      node.m_length = AltitudeStep();
      node.m_altitude = sub_aver_altitude;
      resVec.emplace_back(std::move(node));
      s += AltitudeStep();
    }
    if (s < linkLength && s + AltitudeStep() > linkLength) {
      const Base::txFloat sub_aver_altitude = 0.5 * ((geom_ptr->getPoint(s).z) + (geom_ptr->getEnd().z));
      Billboard_Ego::AltitudeNode node;
      node.m_starts = s;
      node.m_length = linkLength - s;
      node.m_altitude = sub_aver_altitude;
      resVec.emplace_back(std::move(node));
    }
  }
  return std::move(resVec);
}

void Billboard_Ego::make_altitude(const sim_msg::BillboardRoadId& id,
                                  const std::vector<AltitudeNode>& altitudeVec) TX_NOEXCEPT {
  if (_NonEmpty_(altitudeVec) && compute_altitude()) {
    sim_msg::BillboardRoadInfo cur_pb;
    cur_pb.mutable_road_link_id()->CopyFrom(id);
    for (const auto& altitude : altitudeVec) {
      auto curvature_pb_ptr = cur_pb.mutable_info()->Add();
      curvature_pb_ptr->set_value_type(sim_msg::BillboardRoadInfoType::Altitude_m);
      curvature_pb_ptr->set_start_s(altitude.m_starts);
      curvature_pb_ptr->set_length(altitude.m_length);
      curvature_pb_ptr->set_value(altitude.m_altitude);
      LogInfo << TX_VARS_NAME(from_road_id, id.from_road_id()) << TX_VARS_NAME(to_road_id, id.to_road_id())
              << TX_VARS_NAME(start_s, altitude.m_starts) << TX_VARS_NAME(length, altitude.m_length)
              << TX_VARS_NAME(altitude, altitude.m_altitude);
    }
    m_ahead_altitude.emplace_back(cur_pb);
  }
}

void Billboard_Ego::make_hadmap_static_info(hadmap::txRoadPtr _rid_ptr) TX_NOEXCEPT {
  if (NonNull_Pointer(_rid_ptr)) {
    sim_msg::BillboardRoadId id;
    id.set_type(sim_msg::BillboardLaneType::Lane);
    id.set_from_road_id(_rid_ptr->getId());
    id.set_to_road_id(_rid_ptr->getId());
    make_slope(id, _rid_ptr->getSlope());
    make_curvature(id, _rid_ptr->getCurvature());
    make_speed_limit(id, _rid_ptr->getLength(), _rid_ptr->getSpeedLimit());
    const auto altitudeNodeVec = GetAltitudeNodeVec(_rid_ptr);
    make_altitude(id, altitudeNodeVec);
  }
}

void Billboard_Ego::make_hadmap_static_info(hadmap::txLaneLinkPtr _link_ptr) TX_NOEXCEPT {
  if (NonNull_Pointer(_link_ptr) && NonNull_Pointer(_link_ptr->getGeometry()) &&
      _link_ptr->getGeometry()->getLength()) {
    sim_msg::BillboardRoadId id;
    id.set_type(sim_msg::BillboardLaneType::Link);
    id.set_from_road_id(_link_ptr->fromRoadId());
    id.set_to_road_id(_link_ptr->toRoadId());
    make_slope(id, _link_ptr->getSlope());
    make_curvature(id, _link_ptr->getCurvature());
    auto from_rid_ptr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(_link_ptr->fromRoadId());
    make_speed_limit(id, _link_ptr->getGeometry()->getLength(), from_rid_ptr->getSpeedLimit());
    const auto altitudeNodeVec = GetAltitudeNodeVec(_link_ptr);
    make_altitude(id, altitudeNodeVec);
  }
}

Base::txBool Billboard_Ego::IsEqual(const sim_msg::BillboardRoadId& pb_id,
                                    const Base::Info_Lane_t& _uid) const TX_NOEXCEPT {
  if (_uid.IsOnLane() && pb_id.type() == sim_msg::Lane && _uid.onLaneUid.roadId == pb_id.from_road_id()) {
    return true;
  } else if (_uid.IsOnLaneLink() && pb_id.type() == sim_msg::Link &&
             _uid.onLinkFromLaneUid.roadId == pb_id.from_road_id() &&
             _uid.onLinkToLaneUid.roadId == pb_id.to_road_id()) {
    return true;
  }
  return false;
}
#if 0
Base::txBool Billboard_Ego::ComputeAheadStaticMapInfo() TX_NOEXCEPT {
#  if 0
  return true;
#  else
  m_ahead_slope.clear();
  m_ahead_altitude.clear();
  m_ahead_curvature.clear();
  m_ahead_speed_limit.clear();
  if (IsEgoOnRouting()) {
    int64_t cur_on_rid = 0;
    if (ego_on_lanelink()) {
      hadmap::txLaneLinkPtr link_ptr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(LaneLocInfo().onLinkFromLaneUid,
                                                                                      LaneLocInfo().onLinkToLaneUid);
      make_hadmap_static_info(link_ptr);
      cur_on_rid = LaneLocInfo().onLinkToLaneUid.roadId;
    } else {
      cur_on_rid = LaneLocInfo().onLaneUid.roadId;
    }

    auto itr = std::find(recommendRoadInfoArray.begin(), recommendRoadInfoArray.end(), cur_on_rid);
    for (; itr != std::end(recommendRoadInfoArray); ++itr) {
      const int64_t cur_rid = *itr;
      if (cur_rid == m_ego_end_laneInfo.onLaneUid.roadId) {
        make_hadmap_static_info(HdMap::HadmapCacheConCurrent::GetTxRoadPtr(cur_rid));
        break;
      } else {
        if (cur_rid >= 0) {
          /*road, road id may be 0*/
          make_hadmap_static_info(HdMap::HadmapCacheConCurrent::GetTxRoadPtr(cur_rid));
        } else {
          /*lanelink*/
          const int64_t from_rid = *(itr - 1);
          const int64_t to_rid = *(itr + 1);
          auto lanelocSet = HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(from_rid, to_rid);
          for (const auto& refLaneLocInfo : lanelocSet) {
            auto link_ptr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(refLaneLocInfo.onLinkFromLaneUid,
                                                                           refLaneLocInfo.onLinkToLaneUid);
            make_hadmap_static_info(link_ptr);
            break;
          }
        }
      }
    }

    {
      m_slope = 0.0;
      for (auto itr = std::begin(m_ahead_slope); itr != std::end(m_ahead_slope); ++itr) {
        const auto& ref = *itr;
        if (IsEqual(ref.road_link_id(), m_in_ego_laneInfo)) {
          for (const auto& refsegment : ref.info()) {
            if (refsegment.start_s() <= m_in_ego_dist &&
                m_in_ego_dist <= (refsegment.start_s() + refsegment.length())) {
              m_slope = refsegment.value();
              break;
            }
          }
        } else {
          break;
        }
      }
      m_altitude = 0.0;
      for (auto itr = std::begin(m_ahead_altitude); itr != std::end(m_ahead_altitude); ++itr) {
        const auto& ref = *itr;
        if (IsEqual(ref.road_link_id(), m_in_ego_laneInfo)) {
          for (const auto& refsegment : ref.info()) {
            if (refsegment.start_s() <= m_in_ego_dist &&
                m_in_ego_dist <= (refsegment.start_s() + refsegment.length())) {
              m_altitude = refsegment.value();
              break;
            }
          }
        } else {
          break;
        }
      }
      m_curvature = 0.0;
      for (auto itr = std::begin(m_ahead_curvature); itr != std::end(m_ahead_curvature); ++itr) {
        const auto& ref = *itr;
        if (IsEqual(ref.road_link_id(), m_in_ego_laneInfo)) {
          for (const auto& refsegment : ref.info()) {
            if (refsegment.start_s() <= m_in_ego_dist &&
                m_in_ego_dist <= (refsegment.start_s() + refsegment.length())) {
              m_curvature = refsegment.value();
              break;
            }
          }
        } else {
          break;
        }
      }
      m_speed_limit = 0.0;
      for (auto itr = std::begin(m_ahead_speed_limit); itr != std::end(m_ahead_speed_limit); ++itr) {
        const auto& ref = *itr;
        if (IsEqual(ref.road_link_id(), m_in_ego_laneInfo)) {
          for (const auto& refsegment : ref.info()) {
            if (refsegment.start_s() <= m_in_ego_dist &&
                m_in_ego_dist <= (refsegment.start_s() + refsegment.length())) {
              m_speed_limit = refsegment.value();
              break;
            }
          }
        } else {
          break;
        }
      }
    }
    return true;
  } else {
    LogWarn << "ComputeAheadStaticMapInfo failure. "
            << " IsEgoOnRouting false.";
    return false;
  }
#  endif
}
#endif
Base::txBool Billboard_Ego::RoadIdInRoute(const Base::txRoadID cur_rid) const TX_NOEXCEPT {
  return (std::end(m_vec_route_path_roadid) !=
          std::find(std::begin(m_vec_route_path_roadid), std::end(m_vec_route_path_roadid), cur_rid));
}
#if 0
Base::txFloat Billboard_Ego::compute_pre_route_distance(const Base::Info_Lane_t end_loc_info,
                                                        Base::Info_Lane_t& absorb_loc_info) const TX_NOEXCEPT {
  if (end_loc_info.IsOnLaneLink()) {
    if (RoadIdInRoute(end_loc_info.onLinkFromLaneUid.roadId)) {
      auto itr = std::find_if(std::rbegin(recommendRoadInfoArray_segment_length),
                              std::rend(recommendRoadInfoArray_segment_length), [&](const segment_length_info& x) {
                                return std::get<0>(x).onLaneUid.roadId == end_loc_info.onLinkFromLaneUid.roadId;
                              });

      if (std::rend(recommendRoadInfoArray_segment_length) != itr) {
        const Base::Info_Lane_t& locInfo = std::get<0>(*itr);
        const Base::txFloat self_len = std::get<1>(*itr);
        const Base::txFloat pre_len = std::get<2>(*itr);
        itr--; /*link*/
        itr--; /*to lane*/
        const Base::Info_Lane_t& to_locInfo = std::get<0>(*itr);
        LogInfo << TX_VARS(locInfo) << TX_VARS(self_len) << TX_VARS(pre_len) << TX_VARS(to_locInfo);
        absorb_loc_info.FromLaneLink(0, Base::txLaneUId(locInfo.onLaneUid), Base::txLaneUId(to_locInfo.onLaneUid));
        return self_len + pre_len;
      } else {
        return -1.0;
      }
    } else if (RoadIdInRoute(end_loc_info.onLinkToLaneUid.roadId)) {
      auto itr = std::find_if(std::begin(recommendRoadInfoArray_segment_length),
                              std::end(recommendRoadInfoArray_segment_length), [&](const segment_length_info& x) {
                                return std::get<0>(x).onLaneUid.roadId == end_loc_info.onLinkToLaneUid.roadId;
                              });
      if (std::end(recommendRoadInfoArray_segment_length) != itr) {
        const Base::Info_Lane_t& to_locInfo = std::get<0>(*itr);
        itr--; /*pre link*/
        itr--; /*from rid*/
        if (std::end(recommendRoadInfoArray_segment_length) != itr) {
          const Base::Info_Lane_t& locInfo = std::get<0>(*itr);
          const Base::txFloat self_len = std::get<1>(*itr);
          const Base::txFloat pre_len = std::get<2>(*itr);
          LogInfo << TX_VARS(locInfo) << TX_VARS(self_len) << TX_VARS(pre_len) << TX_VARS(to_locInfo);
          absorb_loc_info.FromLaneLink(0, Base::txLaneUId(locInfo.onLaneUid), Base::txLaneUId(to_locInfo.onLaneUid));
          return self_len + pre_len;
        } else {
          return -1.0;
        }
      } else {
        return -1.0;
      }
    } else {
      return -1.0;
    }
  } else {
    /*on lane*/
    if (RoadIdInRoute(end_loc_info.onLaneUid.roadId)) {
      auto itr = std::find_if(std::begin(recommendRoadInfoArray_segment_length),
                              std::end(recommendRoadInfoArray_segment_length), [&](const segment_length_info& x) {
                                return (std::get<0>(x).onLaneUid.roadId == end_loc_info.onLaneUid.roadId) &&
                                       (std::get<0>(x).onLaneUid.sectionId == end_loc_info.onLaneUid.sectionId);
                              });
      if (std::end(recommendRoadInfoArray_segment_length) != itr) {
        const Base::Info_Lane_t& locInfo = std::get<0>(*itr);
        const Base::txFloat pre_len = std::get<2>(*itr);
        LogInfo << TX_VARS(locInfo) << TX_VARS(pre_len);
        absorb_loc_info.FromLane(locInfo.onLaneUid);
        return pre_len;
      } else {
        return -1.0;
      }
    } else {
      return -1.0;
    }
  }
}
#endif
Base::LocInfoSet Billboard_Ego::SegmentId2LocInfoSet(const Base::Info_Lane_t& segmentId) const TX_NOEXCEPT {
  Base::LocInfoSet locInfoSet;
  if (segmentId.IsOnLane()) {
    auto lanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(
        std::make_pair(segmentId.onLaneUid.roadId, segmentId.onLaneUid.sectionId));
    for (const auto& lane_ptr : lanes) {
      if (NonNull_Pointer(lane_ptr)) {
        locInfoSet.insert(Base::Info_Lane_t(lane_ptr->getTxLaneId()));
      }
    }
  } else {
    locInfoSet = HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(segmentId.onLinkFromLaneUid.roadId,
                                                                         segmentId.onLinkToLaneUid.roadId);
  }
  return std::move(locInfoSet);
}

Base::txInt Billboard_Ego::ComputeSectionInfoToDestination(const RoadNetworkStatisticsDict& _road_network_info)
    TX_NOEXCEPT {
  m_vec_ahead_st_segment_info.clear();
  if (IsEgoOnRouting()) {
    Base::txFloat last_s_on_route_path = 0.0; /*start_s*/
    for (const auto& refSegInfo : m_vec_segment_endPt_on_route_path) {
      sim_msg::BillboardSTSegmentInfo newNode;
      const Base::Info_Lane_t& locInfo = refSegInfo.m_sectionUid_or_LinkId;
      sim_msg::BillboardSegmentId cur_segment_id = LaneLocInfo2SegmentId(locInfo);
      newNode.mutable_segid()->CopyFrom(cur_segment_id);
      newNode.set_start_s(last_s_on_route_path);
      newNode.set_length(refSegInfo.m_segment_endPt_s_on_route_path - last_s_on_route_path);
      last_s_on_route_path = refSegInfo.m_segment_endPt_s_on_route_path;

      newNode.set_average_vehicle_speed_kmh(compute_average_vehicle_speed_kmh(_road_network_info, locInfo));
      newNode.set_slope(compute_slope(locInfo));
      newNode.set_curvature(compute_curvature(locInfo));
      newNode.set_altitude_m(compute_altitude_m(locInfo));
      newNode.set_speed_limit_kmh(compute_speed_limit_kmh(locInfo));
      m_vec_ahead_st_segment_info.emplace_back(std::move(newNode));
    }
    return m_vec_ahead_st_segment_info.size();
  } else {
    LogWarn << "ComputeSectionInfoToDestination failure. "
            << " IsEgoOnRouting false.";
    return 0;
  }
}

Base::txFloat Billboard_Ego::compute_average_vehicle_speed_kmh(const RoadNetworkStatisticsDict& _road_network_info,
                                                               const Base::Info_Lane_t& segmentId) TX_NOEXCEPT {
  const Base::LocInfoSet locSet = SegmentId2LocInfoSet(segmentId);
  const auto ref_aver_speed_pb = ComputeSegmentStatisticsInfo(_road_network_info, locSet);
  return ref_aver_speed_pb.aver_speed_kmh();
}

Base::txFloat Billboard_Ego::compute_lanelink_slope(const Base::txRoadID from_rid,
                                                    const Base::txRoadID to_rid) const TX_NOEXCEPT {
  auto lanelocSet = HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(from_rid, to_rid);
  for (const auto& refLaneLocInfo : lanelocSet) {
    auto ptr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(refLaneLocInfo.onLinkFromLaneUid,
                                                              refLaneLocInfo.onLinkToLaneUid);
    if (NonNull_Pointer(ptr)) {
      return ptr->getMeanSlope();
    }
  }
  return 0.0;
}

Base::txFloat Billboard_Ego::compute_slope(const Base::Info_Lane_t& segmentId) TX_NOEXCEPT {
  if (segmentId.IsOnLane()) {
    auto ptr = HdMap::HadmapCacheConCurrent::GetTxSectionPtr(
        std::make_pair(segmentId.onLaneUid.roadId, segmentId.onLaneUid.sectionId));
    if (NonNull_Pointer(ptr)) {
      return ptr->getMeanSlope();
    } else {
      return 0.0;
    }
  } else {
    return compute_lanelink_slope(segmentId.onLinkFromLaneUid.roadId, segmentId.onLinkToLaneUid.roadId);
  }
}

Base::txFloat aver_altitude(const hadmap::txCurve* ptr, const Base::txFloat step) TX_NOEXCEPT {
  if (NonNull_Pointer(ptr)) {
    Base::txFloat s = 0.0;
    Base::txFloat sum_altitude = 0.0;
    Base::txInt sample_cnt = 0;
    const Base::txFloat len = ptr->getLength();
    while (s < len) {
      const auto gps = ptr->getPoint(s);
      sample_cnt++;
      sum_altitude += __Alt__(gps);
      s += step;
    }
    if (sample_cnt > 0) {
      return (sum_altitude / sample_cnt);
    } else {
      return 0.0;
    }
  } else {
    return 0.0;
  }
}

Base::txFloat Billboard_Ego::compute_altitude_m(const Base::Info_Lane_t& segmentId) TX_NOEXCEPT {
  if (segmentId.IsOnLane()) {
    auto ptr = HdMap::HadmapCacheConCurrent::GetTxSectionPtr(
        std::make_pair(segmentId.onLaneUid.roadId, segmentId.onLaneUid.sectionId));
    if (NonNull_Pointer(ptr)) {
      auto laneptr = ptr->getLanes().front();
      if (NonNull_Pointer(laneptr) && NonNull_Pointer(laneptr->getGeometry())) {
        return aver_altitude(laneptr->getGeometry(), AltitudeStep());
      }
    }
  } else {
    auto lanelocSet = HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(segmentId.onLinkFromLaneUid.roadId,
                                                                              segmentId.onLinkToLaneUid.roadId);
    for (const auto& refLaneLocInfo : lanelocSet) {
      auto ptr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(refLaneLocInfo.onLinkFromLaneUid,
                                                                refLaneLocInfo.onLinkToLaneUid);
      if (NonNull_Pointer(ptr) && NonNull_Pointer(ptr->getGeometry())) {
        return aver_altitude(ptr->getGeometry(), AltitudeStep());
      }
    }
  }
  return 0.0;
}

Base::txFloat Billboard_Ego::compute_speed_limit_kmh(const Base::Info_Lane_t& segmentId) TX_NOEXCEPT {
  if (segmentId.IsOnLane()) {
    auto ptr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(segmentId.onLaneUid.roadId);
    if (NonNull_Pointer(ptr)) {
      return ptr->getSpeedLimit();
    }
  } else {
    auto ptr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(segmentId.onLinkFromLaneUid.roadId);
    if (NonNull_Pointer(ptr)) {
      return ptr->getSpeedLimit();
    }
  }
  return 0.0;
}

Base::txFloat Billboard_Ego::compute_lanelink_curvature(const Base::txRoadID from_rid,
                                                        const Base::txRoadID to_rid) const TX_NOEXCEPT {
  auto lanelocSet = HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(from_rid, to_rid);
  for (const auto& refLaneLocInfo : lanelocSet) {
    auto ptr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(refLaneLocInfo.onLinkFromLaneUid,
                                                              refLaneLocInfo.onLinkToLaneUid);
    if (NonNull_Pointer(ptr)) {
      return ptr->getMeanCurvature();
    }
  }
  return 0.0;
}

Base::txFloat Billboard_Ego::compute_curvature(const Base::Info_Lane_t& segmentId) TX_NOEXCEPT {
  if (segmentId.IsOnLane()) {
    auto ptr = HdMap::HadmapCacheConCurrent::GetTxSectionPtr(
        std::make_pair(segmentId.onLaneUid.roadId, segmentId.onLaneUid.sectionId));
    if (NonNull_Pointer(ptr)) {
      return ptr->getMeanCurvature();
    } else {
      return 0.0;
    }
  } else {
    return compute_lanelink_curvature(segmentId.onLinkFromLaneUid.roadId, segmentId.onLinkToLaneUid.roadId);
  }
}

Coord::txENU Billboard_Ego::ComputeEndPt(const Base::Info_Lane_t segmentId) TX_NOEXCEPT {
  const Base::LocInfoSet locSet = SegmentId2LocInfoSet(segmentId);
  Base::txVec3 localEnd;
  localEnd.setZero();
  Base::txInt cnt = 0;
  for (const auto& refLocInfo : locSet) {
    auto geom_ptr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(refLocInfo);
    if (NonNull_Pointer(geom_ptr)) {
      LOG(INFO) << TX_VARS(refLocInfo) << TX_VARS(Utils::ToString(geom_ptr->GetEndPt()));
      localEnd += (geom_ptr->GetEndPt());
      cnt++;
    }
  }
  if (cnt > 0) {
    localEnd /= cnt;
  }
  return Coord::txENU(localEnd);
}

Base::txBool Billboard_Ego::InitializeTrajMgr(const control_path_node_vec& ref_control_path) TX_NOEXCEPT {
  if (ref_control_path.size() >= 2) {
    auto tmp_centripetal_cat_mull_ptr = std::make_shared<CentripetalCatMull>(ref_control_path);
    const txFloat raw_segment_length = tmp_centripetal_cat_mull_ptr->GetLength();
    std::vector<controlPoint> controlPointVec;
    for (txFloat s = 0.0; s < raw_segment_length; s += FLAGS_MapLocationInterval) {
      const Base::txVec3 enu3d = tmp_centripetal_cat_mull_ptr->GetLocalPos(s);
      controlPointVec.emplace_back(controlPoint{__East__(enu3d), __North__(enu3d)});
    }
    const Base::txVec3 enu3d = tmp_centripetal_cat_mull_ptr->GetLocalPos(raw_segment_length);
    controlPointVec.emplace_back(controlPoint{__East__(enu3d), __North__(enu3d)});
    m_trajMgr = nullptr;
    const auto controlPointSize = controlPointVec.size();
    if (controlPointSize >= HdMap::txLaneInfo::CatmullRom_ControlPointSize) {
      m_trajMgr = std::make_shared<HdMap::txLaneInfo>(Base::txLaneUId(), controlPointVec);
    } else if (controlPointSize >= 2 TX_MARK("may be 3")) {
      m_trajMgr =
          std::make_shared<HdMap::txLaneShortInfo>(Base::txLaneUId(), controlPointVec.front(), controlPointVec.back());
    }
  } else if (1 == ref_control_path.size()) {
    Coord::txENU startEnu;
    startEnu.FromWGS84(ref_control_path[0].waypoint);
    Base::txVec3 _laneDir;
    Base::Info_Lane_t locInfo;
    Base::txFloat DistanceAlongCurve = 0.0;
    if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
            startEnu.ToWGS84(), locInfo, DistanceAlongCurve)) {
      LogInfo << "Get_S_Coord_By_Enu_Pt : " << locInfo;
      _laneDir = HdMap::HadmapCacheConCurrent::GetDir(locInfo, DistanceAlongCurve);
    } else {
      LogWarn << "Get_S_Coord_By_Enu_Pt failure." << startEnu;
      return false;
    }

    m_trajMgr = std::make_shared<HdMap::txLaneSinglePositionInfo>(Base::txLaneUId(0, 0, 0),
                                                                  controlPoint{startEnu.X(), startEnu.Y()}, _laneDir);
  } else {
    LOG(WARNING) << "[Error]" << TX_VARS(ref_control_path.size());
    return false;
  }
  return NonNull_Pointer(m_trajMgr);
}

Base::txBool Billboard_Ego::InitializeSlope() TX_NOEXCEPT {
  std::vector<std::tuple<Coord::txENU, Base::txFloat> > vec_pt_slope;
  for (int idx = 0; idx < m_vec_route_path_roadid.size(); ++idx) {
    const auto curRoadId = m_vec_route_path_roadid[idx];
    if (curRoadId >= 0) {
      auto road_ptr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(curRoadId);
      if (NonNull_Pointer(road_ptr)) {
        auto slope_vec = road_ptr->getSlope();
        for (const auto slopNode : slope_vec) {
          Base::txFloat lon, lat, yaw;
          road_ptr->getGeometry()->sl2xy(slopNode.m_starts, 0.0, lon, lat, yaw);
          Coord::txENU tmpEnu;
          tmpEnu.FromWGS84(lon, lat);
          vec_pt_slope.emplace_back(std::make_tuple(tmpEnu, slopNode.m_slope));
        }
      }
    } else {
      const auto fromRoadId = m_vec_route_path_roadid[idx - 1];
      const auto toRoadId = m_vec_route_path_roadid[idx + 1];
      auto linkSet = HdMap::HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(fromRoadId, toRoadId);
      for (auto link_uid : linkSet) {
        hadmap::txLaneLinkPtr linkPtr =
            HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(link_uid.onLinkFromLaneUid, link_uid.onLinkToLaneUid);
        if (NonNull_Pointer(linkPtr)) {
          auto slope_vec = linkPtr->getSlope();
          for (const auto slopNode : slope_vec) {
            Base::txFloat lon, lat, yaw;
            linkPtr->getGeometry()->sl2xy(slopNode.m_starts, 0.0, lon, lat, yaw);
            Coord::txENU tmpEnu;
            tmpEnu.FromWGS84(lon, lat);
            vec_pt_slope.emplace_back(std::make_tuple(tmpEnu, slopNode.m_slope));
          }
          break;
        }
      }
    }
  }
  m_vec_path_slope_info.clear();
  for (auto& tupleNode : vec_pt_slope) {
    Base::txFloat s, l;
    Base::Info_Lane_t locInfo;
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
        std::get<0>(tupleNode).ToWGS84(), locInfo, s);
    if (locInfo.IsOnLane()) {
      if (IsLocInfoInRoutePath(locInfo)) {
        if (locInfo == m_ego_start_laneInfo) {
          /**/
          if (s < m_ego_start_dist_in_lane) {
            continue;
          }
        } else if (locInfo == m_ego_end_laneInfo) {
          if (s >= m_ego_end_dist) {
            continue;
          }
        }
        m_trajMgr->xy2sl(std::get<0>(tupleNode).ENU2D(), s, l);
        if (s > 0) {
          if (0 == m_vec_path_slope_info.size() ||
              m_vec_path_slope_info.size() > 0 && Math::isNotEqual(s, std::get<0>(m_vec_path_slope_info.back()))) {
            m_vec_path_slope_info.emplace_back(std::make_tuple(s, std::get<1>(tupleNode)));
            LOG(INFO) << TX_VARS(s) << TX_VARS_NAME(enu, std::get<0>(tupleNode))
                      << TX_VARS_NAME(Slope, std::get<1>(tupleNode));
          }
        }
      } else {
        /*ignore not contain route path*/
      }
    } else {
      m_trajMgr->xy2sl(std::get<0>(tupleNode).ENU2D(), s, l);
      if (s > 0) {
        if (0 == m_vec_path_slope_info.size() ||
            m_vec_path_slope_info.size() > 0 && Math::isNotEqual(s, std::get<0>(m_vec_path_slope_info.back()))) {
          m_vec_path_slope_info.emplace_back(std::make_tuple(s, std::get<1>(tupleNode)));
          LOG(INFO) << TX_VARS(s) << TX_VARS_NAME(enu, std::get<0>(tupleNode))
                    << TX_VARS_NAME(Slope, std::get<1>(tupleNode));
        }
      }
    }
  }
  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)
