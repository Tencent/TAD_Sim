// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "mapengine/hadmap_engine.h"
#include "structs/hadmap_curve.h"
#include "utils/eval_common.h"
#include "utils/eval_math.h"

namespace eval {
enum MAPState { MAP_NOT_OK, MAP_OK };

struct MapCurveBase;
struct LaneBoundry;
struct StopLine;
struct RefLine;
struct Crosswalk;
struct EvalLane;
struct EvalLaneLink;
struct EvalRoad;

/**
 * map info
 */
struct EvalMapObject;
struct ActorMapInfo;
struct EvalMapStatActor;
using EvalMapObjectPtr = std::shared_ptr<EvalMapObject>;
using ActorMapInfoPtr = std::shared_ptr<ActorMapInfo>;
using EvalMapStatActorPtr = std::shared_ptr<EvalMapStatActor>;

/**
 * map query agent
 */
struct MapQueryTaskCfg;
struct MapQueryAgent;

// parsed map info
struct MapCurveBase {
 public:
  EvalPoints sample_points;  // sample points

  MapCurveBase() { Clear(); }

  void Clear() {
    sample_points.clear();
    sample_points.reserve(const_MSPS);
  }
};

/**
 * @brief lane boundary struct.
 */
struct LaneBoundry : public MapCurveBase {
 public:
  hadmap::laneboundarypkid boundry_pkid;  // boundry pkid
  hadmap::LANE_MARK lane_mark_type;       // lane mark type
  bool left_boundary;                     // if is left boundary

  LaneBoundry() { Clear(); }

  // copy info only from
  void CopyInfoFrom(const LaneBoundry &boundry) {
    boundry_pkid = boundry.boundry_pkid;
    lane_mark_type = boundry.lane_mark_type;
    left_boundary = boundry.left_boundary;
  }

  void Clear() {
    MapCurveBase::Clear();
    boundry_pkid = 0;
    lane_mark_type = hadmap::LANE_MARK_None;
  }
};

/**
 * @brief stop line struct.
 */
struct StopLine : public MapCurveBase {
 public:
  hadmap::objectpkid object_pkid;
  EvalPoints sample_points;

  StopLine() { Clear(); }

  // copy info only from
  void CopyInfoFrom(const StopLine &stop_line) { object_pkid = stop_line.object_pkid; }

  void Clear() {
    MapCurveBase::Clear();
    object_pkid = 0;
  }
};

struct RefLine : public MapCurveBase {};

/**
 * @brief crosswalk struct.
 */
struct Crosswalk : public MapCurveBase {
 public:
  hadmap::objectpkid object_pkid;
  EvalPoints sample_points;

  Crosswalk() { Clear(); }

  // copy info only from
  void CopyInfoFrom(const Crosswalk &crosswalk) { object_pkid = crosswalk.object_pkid; }

  void Clear() {
    MapCurveBase::Clear();
    object_pkid = 0;
  }
};

// actor lane
struct EvalLane {
 private:
  const size_t m_max_object = 20;

 public:
  LaneBoundry m_left_boundry, m_right_boundry;
  RefLine m_ref_line;

  // one hadmap lane pointer
  hadmap::txLanePtr m_lane;

  // left/right hadmap lane pointer and their boundry
  hadmap::txLanePtr m_left_lane, m_right_lane;
  LaneBoundry m_left_lane_left_boundry, m_right_lane_right_boundry;

  // next/pre hadmap lanes if lane is not related to junction, empty if not
  hadmap::txLanes m_pre_lanes, m_next_lanes;

  // hadmap lane links of lane if related to junction, empty if not
  hadmap::txLaneLinks m_next_lanelinks, m_pre_lanelinks;

  bool m_on_narrow_lane;
  bool m_on_entry_expressway;
  bool m_on_exit_expressway;
  bool m_no_center_lane;
  bool m_on_lane_acceleration;
  bool m_on_lane_deceleration;
  bool m_on_lane_shoulder;
  bool m_on_lane_emergency;
  bool m_on_connecting_ramp;
  bool m_lane_num_increasing;
  bool m_lane_num_decreasing;

  EvalLane() { Clear(); }

  void Clear() {
    m_left_boundry.Clear();
    m_right_boundry.Clear();
    m_left_boundry.left_boundary = true;
    m_right_boundry.left_boundary = false;
    m_ref_line.Clear();
    m_lane.reset();
    m_left_lane.reset();
    m_right_lane.reset();
    m_pre_lanes.clear();
    m_next_lanes.clear();
    m_next_lanelinks.clear();
    m_pre_lanelinks.clear();

    m_pre_lanes.reserve(m_max_object);
    m_next_lanes.reserve(m_max_object);
    m_next_lanelinks.reserve(m_max_object);
    m_pre_lanelinks.reserve(m_max_object);

    m_on_narrow_lane = false;
    m_on_entry_expressway = false;
    m_on_exit_expressway = false;
    m_no_center_lane = false;
    m_on_lane_acceleration = false;
    m_on_lane_deceleration = false;
    m_on_lane_shoulder = false;
    m_on_lane_emergency = false;
    m_on_connecting_ramp = false;
    m_lane_num_increasing = false;
    m_lane_num_decreasing = false;
  }
};

// actor lane link
struct EvalLaneLink {
 public:
  eval::EvalPoints m_sample_points;

  // one hadmap lane link pointer
  hadmap::txLaneLinkPtr m_lanelink;

  // pre/next hadmap lane connected by this lane link
  hadmap::txLanePtr m_pre_lane, m_next_lane;

  EvalLaneLink() { Clear(); }

  void Clear() {
    m_lanelink.reset();
    m_pre_lane.reset();
    m_next_lane.reset();
    m_sample_points.clear();
    m_sample_points.reserve(const_MSPS);
  }
};

// one specific type of objects in map related to specific lanes
struct EvalMapObject {
 private:
  const size_t m_max_object = 20;

 public:
  hadmap::OBJECT_TYPE m_object_type;
  hadmap::txObjects m_map_objects;
  std::vector<eval::EvalPoints> m_samples;

  explicit EvalMapObject(const hadmap::OBJECT_TYPE &obj_type) {
    Clear();
    m_object_type = obj_type;
  }
  EvalMapObject() { Clear(); }

  void Clear() {
    m_object_type = hadmap::OBJECT_TYPE_None;
    m_map_objects.clear();
    m_map_objects.reserve(m_max_object);
    m_samples.clear();
    m_samples.reserve(m_max_object);
  }
};

// one specific type of objects in map related to specific lanes
struct EvalMapStatActor {
 private:
  const size_t m_max_object = 20;

 public:
  hadmap::OBJECT_TYPE m_object_type;
  CActorMapObjects m_map_actors;
  std::vector<eval::EvalPoints> m_samples;

  explicit EvalMapStatActor(const hadmap::OBJECT_TYPE &obj_type) {
    Clear();
    m_object_type = obj_type;
  }
  EvalMapStatActor() { Clear(); }

  void Clear() {
    m_object_type = hadmap::OBJECT_TYPE_None;
    m_map_actors.clear();
    m_map_actors.reserve(m_max_object);
    m_samples.clear();
    m_samples.reserve(m_max_object);
  }
};

// actor road
struct EvalRoad {
 public:
  hadmap::txRoadPtr m_road;
  bool m_on_parking_area;
  bool m_on_private_road;
  bool m_on_motorway;
  bool m_on_ramp;
  bool m_on_urban;
  bool m_on_narrow_road;
  bool m_on_sharp_curve;
  bool m_on_steep_slope;
  bool m_on_bridge;
  bool m_on_tunnel;
  bool m_on_control_access;
  bool m_is_unbidirectional;
  bool m_has_parking_sign;
  bool m_has_parking_marking;
  double m_road_length;

  EvalRoad() { Clear(); }

  void Clear() {
    m_road.reset();
    m_on_narrow_road = false;
    m_on_parking_area = false;
    m_on_private_road = false;
    m_on_motorway = false;
    m_on_ramp = false;
    m_on_urban = false;
    m_on_narrow_road = false;
    m_on_sharp_curve = false;
    m_on_steep_slope = false;
    m_on_bridge = false;
    m_on_tunnel = false;
    m_on_control_access = false;
    m_is_unbidirectional = false;
    m_has_parking_sign = false;
    m_has_parking_marking = false;
    m_road_length = 0.0;
  }
};
}  // namespace eval
