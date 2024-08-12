// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/tx_lane_geom_info.h"
#include "location.pb.h"
#include "routeplan/traj_node.h"
#include "traffic.pb.h"
#include "traffic_billboard.pb.h"
#include "tx_centripetal_cat_mull.h"
#include "tx_header.h"
#include "tx_locate_info.h"
#include "tx_sim_point.h"
#include "tx_trajectory_follower.h"
#include "tx_billboard_lane_info_aver_speed.h"
#include "tx_billboard_flags.h"
#include "tx_billboard_hwy_entry.h"
#include "txsim_messenger.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class Billboard_Ego {
 public:
  using txFloat = Base::txFloat;
  using SegmentKinectParam = Base::SegmentKinectParam;
  using SegmentKinectParamList = Base::SegmentKinectParamList;
  using controlPoint = HdMap::txLaneInfoInterface::controlPoint;
  using controlPointVec = HdMap::txLaneInfoInterface::controlPointVec;
  using control_path_node = Base::ISceneLoader::IRouteViewer::control_path_node;
  using control_path_node_vec = CentripetalCatMull::control_path_node_vec;
  using ControlPathGear = Base::SegmentKinectParam::ControlPathGear;
  using RoadNetworkStatisticsDict =
      tbb::concurrent_hash_map<Base::Info_Lane_t, LaneInfoAverSpeed /*sim_msg::BillboardLaneInfo*/,
                               Utils::Info_Lane_t_HashCompare>;

 public:
  Billboard_Ego() TX_DEFAULT;
  virtual ~Billboard_Ego() TX_DEFAULT;
  virtual Base::txBool Initialize(const std::vector<tx_sim::Vector3d>& egp_path) TX_NOEXCEPT;
  virtual Base::txBool Initialize_hwy(const std::vector<Base::Info_Lane_t>& entry_lanes_uid,
                                      const Base::txFloat _search_dist) TX_NOEXCEPT;
  virtual Base::txBool Initialize_pile(const std::vector<Base::Info_Lane_t>& pile_lanes_uid,
                                       const Base::txFloat _search_dist) TX_NOEXCEPT;
  virtual Base::txBool Update(const sim_msg::Location& _loc,
                              const RoadNetworkStatisticsDict& _road_network_info) TX_NOEXCEPT;
  virtual Base::txBool UpdateTrafficLights(const sim_msg::Traffic& _traffic_pb) TX_NOEXCEPT;
  virtual void Clear() TX_NOEXCEPT;
  virtual sim_msg::EgoNavigationInfo pb(const RoadNetworkStatisticsDict& _road_network_info) TX_NOEXCEPT;

 public:
  virtual Base::txBool LocationEgo() TX_NOEXCEPT;
  virtual Base::txBool IsEgoOnRouting() const TX_NOEXCEPT;
  /*virtual Base::txInt ComputeSectionCountToDestination(const RoadNetworkStatisticsDict& _road_network_info)
   * TX_NOEXCEPT;*/
  virtual Base::txInt ComputeSectionInfoToDestination(const RoadNetworkStatisticsDict& _road_network_info) TX_NOEXCEPT;

  virtual Base::txFloat ComputeDistanceToDestination() const TX_NOEXCEPT;
  /*virtual Base::txBool ComputeAheadStaticMapInfo() TX_NOEXCEPT;*/
  /*virtual Base::txBool ComputeAheadRoadConditions() TX_NOEXCEPT;
  virtual Base::txBool UpdateAheadAverageSpeed(const std::list< Base::Info_Lane_t >& _ahead_section) TX_NOEXCEPT;*/
  Base::txInt SectionCountToDestination() const TX_NOEXCEPT { return m_nav_info.sec_cnt_to_end(); }
  Base::txFloat DistanceToDestination() const TX_NOEXCEPT { return m_nav_info.dist_to_end(); }
  // virtual Base::txBool AbsorbCandidateLane( const Coord::txENU _pos, Base::Info_Lane_t& result_uid, Base::txFloat&
  // result_s) const TX_NOEXCEPT;

 public:
  const sim_msg::Location& ego_location_pb() const TX_NOEXCEPT { return m_in_Location; }
  const Base::txBool IsValid() const TX_NOEXCEPT { return m_isValid && NonNull_Pointer(m_trajMgr); }
  const Base::txVec3 Velocity3d() const TX_NOEXCEPT {
    return Base::txVec3(ego_location_pb().velocity().x(), ego_location_pb().velocity().y(),
                        ego_location_pb().velocity().z());
  }
  const Base::txFloat Speed() const TX_NOEXCEPT { return Velocity3d().norm(); }
  const Base::Info_Lane_t& LaneLocInfo() const TX_NOEXCEPT { return m_in_ego_laneInfo; }
  const Base::txString Str() const TX_NOEXCEPT;
  const Base::txBool ego_on_lane() const TX_NOEXCEPT { return LaneLocInfo().IsOnLane(); }
  const Base::txBool ego_on_lanelink() const TX_NOEXCEPT { return LaneLocInfo().IsOnLaneLink(); }
  static inline Base::txInt ResetCnt(const Base::txInt inCnt) TX_NOEXCEPT { return (inCnt < 0) ? (0) : (inCnt); }
  static inline Base::txFloat ResetDist(const Base::txFloat inDist) TX_NOEXCEPT {
    return (inDist < 0.) ? (0.) : (inDist);
  }
  static inline Base::txFloat GetSectionStartLength(const Base::txRoadID _rid, const Base::txInt _sid) TX_NOEXCEPT;
  static inline sim_msg::BillboardSegmentId LaneLocInfo2SegmentId(const Base::Info_Lane_t _lane_loc_info) TX_NOEXCEPT;
  static inline sim_msg::BillboardLaneIdentity LaneLocInfo2BillboardLaneIdentity(const Base::Info_Lane_t _lane_loc_info)
      TX_NOEXCEPT;
  static inline sim_msg::BillboardLaneUId LaneUid2BillboardLaneUId(const Base::txLaneUId _laneUid) TX_NOEXCEPT;
  static inline std::tuple<Base::LocInfoSet, sim_msg::BillboardSectionInfo> ComputeSingleSegmentInfo(
      const Base::Info_Lane_t& _loc_info, const RoadNetworkStatisticsDict& _road_network_info) TX_NOEXCEPT;
  static sim_msg::BillboardLaneStatisticsInfo ComputeSegmentStatisticsInfo(
      const RoadNetworkStatisticsDict& _road_network_info, const Base::LocInfoSet _locSet) TX_NOEXCEPT;

 public:
  struct AltitudeNode {
    Base::txFloat m_starts = 0.0;
    Base::txFloat m_length = 0.0;
    Base::txFloat m_altitude = 0.0;
  };

 protected:
  Base::txFloat max_dist_thresold() const TX_NOEXCEPT { return 2.0 * FLAGS_default_lane_width; }
  virtual void make_hadmap_static_info(hadmap::txRoadPtr _rid_ptr) TX_NOEXCEPT;
  virtual void make_hadmap_static_info(hadmap::txLaneLinkPtr _link_ptr) TX_NOEXCEPT;
  virtual void make_slope(const sim_msg::BillboardRoadId& id, const hadmap::txSlopeVec& slopeVec) TX_NOEXCEPT;
  virtual void make_curvature(const sim_msg::BillboardRoadId& id,
                              const hadmap::txCurvatureVec& curvatureVec) TX_NOEXCEPT;
  virtual void make_speed_limit(const sim_msg::BillboardRoadId& id, const Base::txFloat _curveLen,
                                const Base::txFloat _speedLimit) TX_NOEXCEPT;
  virtual void make_altitude(const sim_msg::BillboardRoadId& id,
                             const std::vector<AltitudeNode>& altitudeVec) TX_NOEXCEPT;
  Base::txFloat AltitudeStep() const TX_NOEXCEPT { return FLAGS_AltitudeStep; }
  virtual std::vector<Billboard_Ego::AltitudeNode> GetAltitudeNodeVec(hadmap::txLaneLinkPtr _link_ptr) TX_NOEXCEPT;
  virtual std::vector<Billboard_Ego::AltitudeNode> GetAltitudeNodeVec(hadmap::txRoadPtr _rid_ptr) TX_NOEXCEPT;
  Base::txBool IsEqual(const sim_msg::BillboardRoadId& pb_id, const Base::Info_Lane_t& _uid) const TX_NOEXCEPT;
  Base::txBool compute_slope() const TX_NOEXCEPT { return FLAGS_compute_slope; }
  Base::txBool compute_altitude() const TX_NOEXCEPT { return FLAGS_compute_altitude; }
  Base::txBool compute_curvature() const TX_NOEXCEPT { return FLAGS_compute_curvature; }
  Base::txBool compute_speed_limit() const TX_NOEXCEPT { return FLAGS_compute_speed_limit; }
  /*virtual Base::txFloat compute_pre_route_distance(const Base::Info_Lane_t end_loc_info, Base::Info_Lane_t&
   * absorb_loc_info) const TX_NOEXCEPT;*/
  virtual Base::txFloat compute_lanelink_length(const Base::txRoadID from_rid,
                                                const Base::txRoadID to_rid) const TX_NOEXCEPT;
  virtual Base::txFloat compute_lanelink_slope(const Base::txRoadID from_rid,
                                               const Base::txRoadID to_rid) const TX_NOEXCEPT;
  virtual Base::txFloat compute_lanelink_curvature(const Base::txRoadID from_rid,
                                                   const Base::txRoadID to_rid) const TX_NOEXCEPT;
  Base::txBool RoadIdInRoute(const Base::txRoadID cur_rid) const TX_NOEXCEPT;
  Base::LocInfoSet SegmentId2LocInfoSet(const Base::Info_Lane_t& segmentId) const TX_NOEXCEPT;
  Base::txFloat compute_average_vehicle_speed_kmh(const RoadNetworkStatisticsDict& _road_network_info,
                                                  const Base::Info_Lane_t& segmentId) TX_NOEXCEPT;
  Base::txFloat compute_slope(const Base::Info_Lane_t& segmentId) TX_NOEXCEPT;
  Base::txFloat compute_altitude_m(const Base::Info_Lane_t& segmentId) TX_NOEXCEPT;
  Base::txFloat compute_speed_limit_kmh(const Base::Info_Lane_t& segmentId) TX_NOEXCEPT;
  Base::txFloat compute_curvature(const Base::Info_Lane_t& segmentId) TX_NOEXCEPT;

 protected:
  sim_msg::Location m_in_Location;
  Coord::txWGS84 m_in_ego_pos;
  Base::Info_Lane_t m_in_ego_laneInfo;
  Base::txFloat m_in_ego_dist = 0.0;
  Base::txBool m_isValid = false;
  std::vector<Coord::txWGS84> m_way_points;
  std::vector<int64_t> m_vec_route_path_roadid; /*recommendRoadInfoArray*/

  /*using segment_length_info = std::tuple<Base::Info_Lane_t, Base::txFloat, Base::txFloat>;
  std::vector< segment_length_info > recommendRoadInfoArray_segment_length;*/

  Coord::txWGS84 m_ego_end;
  Base::Info_Lane_t m_ego_end_laneInfo;
  Base::txFloat m_ego_end_dist = 0.0;

  Coord::txWGS84 m_ego_start;
  Base::Info_Lane_t m_ego_start_laneInfo;
  Base::txFloat m_ego_start_dist_in_lane = 0.0;
  Base::txFloat m_ego_start_dist_in_road = 0.0;

  class NavigationInfo {
   public:
    void Clear() TX_NOEXCEPT {
      m_inited = false;
      m_computed_ego_pos.FromWGS84(0.0, 0.0, 0.0);
    }
    Base::txBool NeedReCompute(Coord::txWGS84 new_ego_pos) const TX_NOEXCEPT {
      if (m_inited) {
        const Base::txFloat dist = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(new_ego_pos, m_computed_ego_pos);
        if (dist < FLAGS_NavigationUpdateThreshold) {
          return false;
        } else {
          return true;
        }
      } else {
        return true;
      }
    }

    void Update(const Coord::txWGS84 newPt, const Base::txFloat _sec_cnt, const Base::txFloat _d2e,
                const Base::txFloat _d2h, const Base::txFloat _d2p) TX_NOEXCEPT {
      m_inited = true;
      m_computed_ego_pos = newPt;
      m_sec_cnt_to_end = _sec_cnt;
      m_dist_to_end = _d2e;
      m_dist_to_hwy_entrance = _d2h;
      m_dist_to_pile = _d2p;
    }

    Base::txInt sec_cnt_to_end() const TX_NOEXCEPT { return m_sec_cnt_to_end; }
    Base::txFloat dist_to_end() const TX_NOEXCEPT { return m_dist_to_end; }
    Base::txFloat dist_to_hwy_entrance() const TX_NOEXCEPT { return m_dist_to_hwy_entrance; }
    Base::txFloat dist_to_pile() const TX_NOEXCEPT { return m_dist_to_pile; }

   protected:
    Base::txBool m_inited = false;
    Coord::txWGS84 m_computed_ego_pos;
    Base::txInt m_sec_cnt_to_end = 0;
    Base::txFloat m_dist_to_end = 0.0;
    Base::txFloat m_dist_to_hwy_entrance = 0.0;
    Base::txFloat m_dist_to_pile = 0.0;
  };
  NavigationInfo m_nav_info;
  Billboard_Hwy m_hwy;
  Billboard_Hwy m_pile;
  /*std::list< std::tuple< Base::LocInfoSet, sim_msg::BillboardSectionInfo> > m_ahead_section_lst;*/

  Base::txBool m_destination_is_charging_pile = false;
  std::list<sim_msg::BillboardRoadInfo> m_ahead_slope;
  std::list<sim_msg::BillboardRoadInfo> m_ahead_altitude;
  std::list<sim_msg::BillboardRoadInfo> m_ahead_curvature;
  std::list<sim_msg::BillboardRoadInfo> m_ahead_speed_limit;
  Base::txFloat m_slope;
  Base::txFloat m_altitude;
  Base::txFloat m_curvature;
  Base::txFloat m_speed_limit;
  Base::txFloat m_aver_speed;
  /*std::vector<  HdMap::txLaneInfoInterfacePtr  > m_vec_candidate_lane;*/

  std::vector<sim_msg::BillboardSTSegmentInfo> m_vec_ahead_st_segment_info;

 public:
  Base::txBool InitializeTrajMgr(const control_path_node_vec& ref_control_path) TX_NOEXCEPT;
  Base::txBool InitializeSlope() TX_NOEXCEPT;
  Coord::txENU ComputeEndPt(const Base::Info_Lane_t segmentId) TX_NOEXCEPT;
  Base::txFloat _s() const TX_NOEXCEPT { return m_st_on_route_path.x(); }
  Base::txFloat& _s() TX_NOEXCEPT { return m_st_on_route_path.x(); }

  Base::txFloat _t() const TX_NOEXCEPT { return m_st_on_route_path.y(); }
  Base::txFloat& _t() TX_NOEXCEPT { return m_st_on_route_path.y(); }

 protected:
  HdMap::txLaneInfoInterfacePtr m_trajMgr = nullptr;
  // std::vector< std::tuple<Base::txFloat/*end_s*/, Base::Info_Lane_t > > m_vec_path_aver_speed_info;
  std::vector<std::tuple<Base::txFloat /*s*/, Base::txFloat /*slope*/> > m_vec_path_slope_info;
  std::vector<sim_msg::TrafficLigthInfo> m_vec_path_trafficlight_info;
  struct ego_path_locInfo {
    Coord::txENU enu;
    Base::Info_Lane_t locinfo;
    Base::txFloat s_on_path;
    Base::txFloat s_on_laneInfo;
  };
  std::vector<Base::Info_Lane_t> m_vec_path_locInfo;
  Base::txVec2 m_st_on_route_path;

  struct SegmentLocInfoOnRoutePath {
    enum class SegmentLocInfoType : Base::txInt { eSection = 0, eLinkSet = 1 };
    SegmentLocInfoType m_type;
    Base::Info_Lane_t m_sectionUid_or_LinkId;
    Base::LocInfoSet m_locInfoSet;
    Coord::txENU m_segment_endPt;
    Base::txFloat m_segment_endPt_s_on_route_path;
    Base::txFloat m_segment_endPt_t_on_route_path;
    Base::txString Str() const TX_NOEXCEPT {
      std::ostringstream oss;
      if (SegmentLocInfoType::eSection == m_type) {
        oss << "[OnSection: " << m_sectionUid_or_LinkId << "]";
      } else {
        oss << "[OnLink: " << m_sectionUid_or_LinkId << "]";
      }
      oss << "[relation]";
      for (const auto info : m_locInfoSet) {
        oss << info;
      }
      oss << TX_VARS(m_segment_endPt) << TX_VARS_NAME(S, m_segment_endPt_s_on_route_path)
          << TX_VARS_NAME(T, m_segment_endPt_t_on_route_path);
      return oss.str();
    }
  };
  std::vector<SegmentLocInfoOnRoutePath> m_vec_segment_endPt_on_route_path;
  Base::txBool ComputeSegmentInfo(const Base::txSectionUId& _secUid, Base::LocInfoSet& locInfoSet,
                                  Coord::txENU& segmentEndPt) TX_NOEXCEPT;
  Base::txBool ComputeSegmentInfo(const Base::Info_Lane_t& _linkUid, Base::LocInfoSet& locInfoSet,
                                  Coord::txENU& segmentEndPt) TX_NOEXCEPT;
  std::unordered_set<Base::Info_Lane_t, Utils::Info_Lane_t_HashCompare> m_hash_set_locInfo_on_route_path;
  Base::txBool IsLocInfoInRoutePath(const Base::Info_Lane_t& _locInfo) const TX_NOEXCEPT;
};

TX_NAMESPACE_CLOSE(TrafficFlow)
