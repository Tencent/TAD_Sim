// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_planning_scene_event_loader.h"
#include <boost/container/small_vector.hpp>
#include "tad_event_dispatcher_1_0_0_0.h"
#include "tx_collision_detection2d.h"
#include "tx_event_utils.h"
#include "tx_math.h"
#include "tx_planning_vehicle_element.h"
#define SceneLoaderInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_SceneEvent)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(SceneLoader)

hadmap::txPoint PlanningSceneEvent_SceneLoader::DummyDriverViewer::startGPS() const TX_NOEXCEPT {
  hadmap::txPoint retV;
  const auto waypoints = m_initial.Waypoints();

  if (_NonEmpty_(waypoints) && CallSucc(waypoints.front().GetPosition().gps(retV))) {
    return retV;
  }

  LogWarn << "startGPS is null.";
  return hadmap::txPoint();
}

hadmap::txPoint PlanningSceneEvent_SceneLoader::DummyDriverViewer::endGPS() const TX_NOEXCEPT {
  hadmap::txPoint retV;
  const auto waypoints = m_initial.Waypoints();
  if ((waypoints.size() > 1) && CallSucc(waypoints.back().GetPosition().gps(retV))) {
    return retV;
  }

  LogWarn << "endGPS is null.";
  return hadmap::txPoint();
}

std::vector<hadmap::txPoint> PlanningSceneEvent_SceneLoader::DummyDriverViewer::midGPS() const TX_NOEXCEPT {
  std::vector<hadmap::txPoint> retV;
  auto waypoints = m_initial.Waypoints();
  if (waypoints.size() > 2) {
    waypoints.erase(waypoints.begin());
    waypoints.erase(waypoints.end() - 1);
    for (const auto& mid : waypoints) {
      hadmap::txPoint pt;
      if (CallSucc(mid.GetPosition().gps(pt))) {
        retV.emplace_back(std::move(pt));
      } else {
        LogWarn << "mid is error.";
      }
    }
    return retV;
  }
  LogWarn << "midGPS is null.";
  return retV;
}

Base::Enums::VEHICLE_BEHAVIOR PlanningSceneEvent_SceneLoader::Str2Behavior(const Base::txString strEgoType)
    TX_NOEXCEPT {
  return _plus_(Base::Enums::VEHICLE_BEHAVIOR::eTadAI);
}

Base::Enums::ConditionBoundaryType PlanningSceneEvent_SceneLoader::Enum2BoundaryEdge(
    const sim_msg::ConditionEdge eEdgeType) TX_NOEXCEPT {
  switch (eEdgeType) {
    case sim_msg::CONDITION_EDGE_BOTH: {
      return _plus_(Base::Enums::ConditionBoundaryType::both);
    }
    case sim_msg::CONDITION_EDGE_RISING: {
      return _plus_(Base::Enums::ConditionBoundaryType::rising);
    }
    case sim_msg::CONDITION_EDGE_FALLING: {
      return _plus_(Base::Enums::ConditionBoundaryType::falling);
    }
    default:
      break;
  }
  return _plus_(Base::Enums::ConditionBoundaryType::none);
}

Base::Enums::DistanceProjectionType PlanningSceneEvent_SceneLoader::Enum2DistanceType(
    const sim_msg::DistanceType eDistType) TX_NOEXCEPT {
  switch (eDistType) {
    case sim_msg::DISTANCE_TYPE_EUCLIDEAN: {
      return _plus_(Base::Enums::DistanceProjectionType::Euclidean);
    }
    case sim_msg::DISTANCE_TYPE_LANEPROJECTION: {
      return _plus_(Base::Enums::DistanceProjectionType::Lane);
    }
    default:
      break;
  }
  return _plus_(Base::Enums::DistanceProjectionType::Euclidean);
}

Base::Enums::ConditionEquationOp PlanningSceneEvent_SceneLoader::Enum2OpType(const sim_msg::Rule eRule) TX_NOEXCEPT {
  switch (eRule) {
    case sim_msg::RULE_EQUAL_TO: {
      return _plus_(Base::Enums::ConditionEquationOp::eq);
    }
    case sim_msg::RULE_GREATER_THEN: {
      return _plus_(Base::Enums::ConditionEquationOp::gt);
    }
    case sim_msg::RULE_GREATER_THEN_EQUAL: {
      return _plus_(Base::Enums::ConditionEquationOp::gte);
    }
    case sim_msg::RULE_LESS_THEN: {
      return _plus_(Base::Enums::ConditionEquationOp::lt);
    }
    case sim_msg::RULE_LESS_THEN_EQUAL: {
      return _plus_(Base::Enums::ConditionEquationOp::lte);
    }
    default:
      break;
  }
  return _plus_(Base::Enums::ConditionEquationOp::eq);
}

Base::Enums::ConditionDirDimension PlanningSceneEvent_SceneLoader::Enum2DirDimension(const sim_msg::DirDimension dd)
    TX_NOEXCEPT {
  switch (dd) {
    case sim_msg::DIR_DIMENSION_LATERAL: {
      return _plus_(Base::Enums::ConditionDirDimension::eLATERAL);
    }
    case sim_msg::DIR_DIMENSION_LONGITUDINAL: {
      return _plus_(Base::Enums::ConditionDirDimension::eLONGITUDINAL);
    }
    case sim_msg::DIR_DIMENSION_VERTICAL: {
      return _plus_(Base::Enums::ConditionDirDimension::eVERTICAL);
    }
    default:
      break;
  }
  return _plus_(Base::Enums::ConditionDirDimension::eLONGITUDINAL);
}

void PlanningSceneEvent_SceneLoader::Position::Clear() TX_NOEXCEPT {
  position_type = PositionType::none;
  position_data = NonePosition();
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::Initialize(const sim_msg::Position& refPosition) TX_NOEXCEPT {
  Clear();
  if (refPosition.has_world()) {
    position_type = PositionType::world;
    position_data = refPosition.world();
    return true;
  } else if (refPosition.has_relative_world()) {
    position_type = PositionType::relative_world;
    position_data = refPosition.relative_world();
    return true;
  } else if (refPosition.has_relative_object()) {
    position_type = PositionType::relative_object;
    position_data = refPosition.relative_object();
    return true;
  } else if (refPosition.has_road()) {
    position_type = PositionType::road;
    position_data = refPosition.road();
    return true;
  } else if (refPosition.has_relative_road()) {
    position_data = refPosition.relative_road();
    position_type = PositionType::relative_road;
    return true;
  } else if (refPosition.has_lane()) {
    position_data = refPosition.lane();
    position_type = PositionType::lane;
    return true;
  } else if (refPosition.has_relative_lane()) {
    position_data = refPosition.relative_lane();
    position_type = PositionType::relative_lane;
    return true;
  }
  return false;
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::gps(hadmap::txPoint& pos) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::world))) {
    IPositionWorld::value_type s = boost::get<IPositionWorld::value_type>(position_data);
    pos = hadmap::txPoint(s.x(), s.y(), s.z());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::world))) {
    IPositionWorld::value_type s = boost::get<IPositionWorld::value_type>(position_data);
    rpy = Base::txVec3(Math::Degrees2Radians(s.r(/*unit : degree*/)), Math::Degrees2Radians(s.p(/*unit : degree*/)),
                       Math::Degrees2Radians(s.h(/*unit : degree*/)));
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_world_type_sysId(
    std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_world))) {
    IPositionRelativeWorld::value_type s = boost::get<IPositionRelativeWorld::value_type>(position_data);
    std::tie(std::get<0>(type_sysId), std::get<1>(type_sysId)) =
        Base::ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(s.entity_ref());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_world_step(Base::txVec3& step) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_world))) {
    IPositionRelativeWorld::value_type s = boost::get<IPositionRelativeWorld::value_type>(position_data);
    step = Base::txVec3(s.dx(), s.dy(), s.dz());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_world_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_world))) {
    IPositionRelativeWorld::value_type s = boost::get<IPositionRelativeWorld::value_type>(position_data);
    rpy = Base::txVec3(Math::Degrees2Radians(s.orientation().r(/*unit : degree*/)), s.orientation().p(),
                       s.orientation().h());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_object_type_sysId(
    std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_object))) {
    IPositionRelativeObject::value_type s = boost::get<IPositionRelativeObject::value_type>(position_data);
    std::tie(std::get<0>(type_sysId), std::get<1>(type_sysId)) =
        Base::ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(s.entity_ref());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_object_step(Base::txVec3& step) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_object))) {
    IPositionRelativeObject::value_type s = boost::get<IPositionRelativeObject::value_type>(position_data);
    step = Base::txVec3(s.dx(), s.dy(), s.dz());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_object_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_object))) {
    IPositionRelativeObject::value_type s = boost::get<IPositionRelativeObject::value_type>(position_data);
    rpy = Base::txVec3(Math::Degrees2Radians(s.orientation().r(/*unit : degree*/)), s.orientation().p(),
                       s.orientation().h());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::road_id(uint64_t& rid) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::road))) {
    IPositionRoad::value_type s = boost::get<IPositionRoad::value_type>(position_data);
    rid = s.road_id();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::road_st(Base::txVec2& _st) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::road))) {
    IPositionRoad::value_type s = boost::get<IPositionRoad::value_type>(position_data);
    _st = Base::txVec2(s.s(), s.t());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::road_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::road))) {
    IPositionRoad::value_type s = boost::get<IPositionRoad::value_type>(position_data);
    rpy = Base::txVec3(Math::Degrees2Radians(s.orientation().r(/*unit : degree*/)), s.orientation().p(),
                       s.orientation().h());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_road_type_sysId(
    std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_road))) {
    IPositionRelativeRoad::value_type s = boost::get<IPositionRelativeRoad::value_type>(position_data);
    std::tie(std::get<0>(type_sysId), std::get<1>(type_sysId)) =
        Base::ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(s.entity_ref());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_road_st(Base::txVec2& _st) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_road))) {
    IPositionRelativeRoad::value_type s = boost::get<IPositionRelativeRoad::value_type>(position_data);
    _st = Base::txVec2(s.ds(), s.dt());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_road_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_road))) {
    IPositionRelativeRoad::value_type s = boost::get<IPositionRelativeRoad::value_type>(position_data);
    rpy = Base::txVec3(Math::Degrees2Radians(s.orientation().r(/*unit : degree*/)), s.orientation().p(),
                       s.orientation().h());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::lane_road_id(uint64_t& rid) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::lane))) {
    IPositionLane::value_type s = boost::get<IPositionLane::value_type>(position_data);
    rid = s.road_id();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::lane_lane_id(int64_t& lid) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::lane))) {
    IPositionLane::value_type s = boost::get<IPositionLane::value_type>(position_data);
    lid = s.lane_id();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::lane_st(Base::txVec2& _st) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::lane))) {
    IPositionLane::value_type s = boost::get<IPositionLane::value_type>(position_data);
    _st = Base::txVec2(s.s(), s.offset());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::lane_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::lane))) {
    IPositionLane::value_type s = boost::get<IPositionLane::value_type>(position_data);
    rpy = Base::txVec3(Math::Degrees2Radians(s.orientation().r(/*unit : degree*/)), s.orientation().p(),
                       s.orientation().h());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_lane_type_sysId(
    std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_lane))) {
    IPositionRelativeLane::value_type s = boost::get<IPositionRelativeLane::value_type>(position_data);
    std::tie(std::get<0>(type_sysId), std::get<1>(type_sysId)) =
        Base::ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(s.entity_ref());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_lane_step(int32_t& _s) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_lane))) {
    IPositionRelativeLane::value_type s = boost::get<IPositionRelativeLane::value_type>(position_data);
    _s = s.dlane();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_lane_st(Base::txVec2& _st) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_lane))) {
    IPositionRelativeLane::value_type s = boost::get<IPositionRelativeLane::value_type>(position_data);
    _st = Base::txVec2(s.ds(), s.offset());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Position::relative_lane_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT {
  if (CallSucc(IsValid(PositionType::relative_lane))) {
    IPositionRelativeLane::value_type s = boost::get<IPositionRelativeLane::value_type>(position_data);
    rpy = Base::txVec3(Math::Degrees2Radians(s.orientation().r(/*unit : degree*/)), s.orientation().p(),
                       s.orientation().h());
    return true;
  } else {
    return false;
  }
}

void PlanningSceneEvent_SceneLoader::Condition::Clear() TX_NOEXCEPT {
  condition_type = ConditionType::none;
  condition_data = NoneCondition();
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::Initialize(const sim_msg::Condition& refCondition) TX_NOEXCEPT {
  Clear();
  m_condition_boundary = Enum2BoundaryEdge(refCondition.edge());
  m_delay = refCondition.delay();
  m_trigger_count = refCondition.count();

  if (refCondition.has_ttc()) {
    condition_type = ConditionType::ttc;
    condition_data = refCondition.ttc();
    return true;
  } else if (refCondition.has_speed()) {
    condition_type = ConditionType::speed;
    condition_data = refCondition.speed();
    return true;
  } else if (refCondition.has_relative_speed()) {
    condition_type = ConditionType::relative_speed;
    condition_data = refCondition.relative_speed();
    return true;
  } else if (refCondition.has_reach_position()) {
    condition_type = ConditionType::reach_position;
    condition_data = refCondition.reach_position();
    return true;
  } else if (refCondition.has_relative_distance()) {
    condition_type = ConditionType::distance;
    condition_data = refCondition.relative_distance();
    return true;
  } else if (refCondition.has_ego_attach_laneid_custom()) {
    condition_type = ConditionType::ego_attach_laneid_custom;
    condition_data = refCondition.ego_attach_laneid_custom();
    return true;
  } else if (refCondition.has_simulation_time()) {
    condition_type = ConditionType::simulation_time;
    condition_data = refCondition.simulation_time();
    return true;
  } else if (refCondition.has_time_headway()) {
    condition_type = ConditionType::time_headway;
    condition_data = refCondition.time_headway();
  } else if (refCondition.has_storyboard_element_state()) {
    condition_type = ConditionType::storyboard_element_state;
    condition_data = refCondition.storyboard_element_state();
  } else {
    condition_type = ConditionType::none;
    condition_data = NoneCondition();
    return false;
  }
  return false;
}

Base::ISceneLoader::ISceneEventViewer::kvMap PlanningSceneEvent_SceneLoader::Condition::KeyValueMap() const
    TX_NOEXCEPT {
  using ConditionDirDimension = Base::Enums::ConditionDirDimension;
  using ConditionBoundaryType = Base::Enums::ConditionBoundaryType;
  using DistanceProjectionType = Base::Enums::DistanceProjectionType;
  using ConditionType = Condition::ConditionType;
  using ConditionEquationOp = Base::Enums::ConditionEquationOp;

  Base::ISceneLoader::ISceneEventViewer::kvMap _condition;
  _condition["condition_boundary"] = edge()._to_string();
  _condition["delay_threshold"] = std::to_string(delay());
  _condition["trigger_count"] = std::to_string(count());
  if (IsValid(ConditionType::ttc)) {
    txFloat ttc_threshold;
    ConditionEquationOp equation_op;
    std::tuple<Base::Enums::ElementType, Base::txSysId> target_element;
    DistanceProjectionType distance_type;
    if (CallSucc(TimeToCollision_value(ttc_threshold)) && CallSucc(TimeToCollision_rule(equation_op)) &&
        CallSucc(TimeToCollision_type_sysId(target_element)) &&
        CallSucc(TimeToCollision_distance_type(distance_type))) {
      _condition["type"] = _plus_(Base::Enums::SceneEventType::dummy_ttc_trigger)._to_string();
      _condition["ttc_threshold"] = std::to_string(ttc_threshold);
      _condition["equation_op"] = equation_op._to_string();
      _condition["target_element"] =
          Base::ISceneLoader::ISceneEventViewer::IConditionViewer::combine_type_id(target_element);
      _condition["distance_type"] = distance_type._to_string();
    } else {
      LogWarn << "condition ttc_trigger parse error.";
      _condition.clear();
    }
  } else if (IsValid(ConditionType::time_headway)) {
    txFloat value;
    ConditionEquationOp equation_op;
    std::tuple<Base::Enums::ElementType, Base::txSysId> target_element;
    txBool freespace, alongroute;
    if (CallSucc(TimeHeadWay_value(value)) && CallSucc(TimeHeadWay_rule(equation_op)) &&
        CallSucc(TimeHeadWay_type_sysId(target_element)) && CallSucc(TimeHeadWay_freespace(freespace)) &&
        CallSucc(TimeHeadWay_along_route(alongroute))) {
      _condition["type"] = _plus_(Base::Enums::SceneEventType::dummy_time_head_way)._to_string();
      _condition["value"] = std::to_string(value);
      _condition["equation_op"] = equation_op._to_string();
      _condition["freespace"] = bool2lpsz(freespace);
      _condition["alongroute"] = bool2lpsz(alongroute);
      _condition["target_element"] =
          Base::ISceneLoader::ISceneEventViewer::IConditionViewer::combine_type_id(target_element);
    } else {
      LogWarn << "condition time_headway parse error.";
      _condition.clear();
    }
  } else if (IsValid(ConditionType::storyboard_element_state)) {
    TrafficFlow::TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType _state;
    std::tuple<Base::txSysId, Base::txInt> _evId_actId;
    if (CallSucc(ElementState_evId_actionId(_evId_actId)) && CallSucc(ElementState_state(_state))) {
      _condition["type"] = _plus_(Base::Enums::SceneEventType::dummy_element_state)._to_string();
      _condition["element_type"] = "action";
      _condition["element_ref"] =
          TrafficFlow::TAD_SceneEvent_1_0_0_0::Make_evId_actId(std::get<0>(_evId_actId), std::get<1>(_evId_actId));
      _condition["state"] = _state._to_string();
    } else {
      LogWarn << "condition storyboard_element_state parse error.";
      _condition.clear();
    }
  } else if (IsValid(ConditionType::speed)) {
    ConditionDirDimension direction_type;
    ConditionEquationOp equation_op;
    txFloat speed_threshold;
    if (CallSucc(Speed_direction(direction_type)) && CallSucc(Speed_rule(equation_op)) &&
        CallSucc(Speed_value_m_s(speed_threshold))) {
      _condition["type"] = _plus_(Base::Enums::SceneEventType::dummy_speed)._to_string();
      _condition["direction_type"] = direction_type._to_string();
      _condition["equation_op"] = equation_op._to_string();
      _condition["speed_threshold"] = std::to_string(speed_threshold);
    } else {
      LogWarn << "condition speed parse error.";
      _condition.clear();
    }
  } else if (IsValid(ConditionType::relative_speed)) {
    ConditionDirDimension direction_type;
    ConditionEquationOp equation_op;
    txFloat speed_threshold;
    std::tuple<Base::Enums::ElementType, Base::txSysId> target_element;
    if (CallSucc(RelativeSpeed_direction(direction_type)) && CallSucc(RelativeSpeed_rule(equation_op)) &&
        CallSucc(RelativeSpeed_value_m_s(speed_threshold)) && CallSucc(RelativeSpeed_type_sysId(target_element))) {
      _condition["type"] = _plus_(Base::Enums::SceneEventType::dummy_relative_speed)._to_string();
      _condition["direction_type"] = direction_type._to_string();
      _condition["equation_op"] = equation_op._to_string();
      _condition["speed_threshold"] = std::to_string(speed_threshold);
      _condition["target_element"] =
          Base::ISceneLoader::ISceneEventViewer::IConditionViewer::combine_type_id(target_element);
    } else {
      LogWarn << "condition relative_speed parse error.";
      _condition.clear();
    }
  } else if (IsValid(ConditionType::reach_position)) {
    txFloat radius;
    Position pos;
    hadmap::txPoint _gps;
    if (CallSucc(ReachPosition_radius_m(radius)) && CallSucc(ReachPosition_position(pos)) &&
        CallSucc(pos.IsValid(Position::PositionType::world)) && CallSucc(pos.gps(_gps))) {
      _condition["type"] = _plus_(Base::Enums::SceneEventType::dummy_reach_position)._to_string();
      _condition["radius"] = std::to_string(radius);
      _condition["gps"] = Utils::ToString(_gps);
    } else {
      LogWarn << "condition reach_position parse error.";
      _condition.clear();
    }

  } else if (IsValid(ConditionType::distance)) {
    txFloat distance_threshold;
    DistanceProjectionType distance_type;
    ConditionEquationOp equation_op;
    std::tuple<Base::Enums::ElementType, Base::txSysId> target_element;
    if (CallSucc(RelativeDistance_distance_type(distance_type)) &&
        CallSucc(RelativeDistance_value_m(distance_threshold)) && CallSucc(RelativeDistance_rule(equation_op)) &&
        CallSucc(RelativeDistance_type_sysId(target_element))) {
      _condition["type"] = _plus_(Base::Enums::SceneEventType::dummy_distance_trigger)._to_string();
      _condition["distance_threshold"] = std::to_string(distance_threshold);
      _condition["distance_type"] = distance_type._to_string();
      _condition["equation_op"] = equation_op._to_string();
      _condition["target_element"] =
          Base::ISceneLoader::ISceneEventViewer::IConditionViewer::combine_type_id(target_element);
    } else {
      LogWarn << "condition distance parse error.";
      _condition.clear();
    }
  } else if (IsValid(ConditionType::ego_attach_laneid_custom)) {
    Base::txLaneUId laneUid;
    if (CallSucc(EgoAttachLaneidCustom_road_id(laneUid))) {
      _condition["type"] = _plus_(Base::Enums::SceneEventType::dummy_ego_attach_laneid_custom)._to_string();
      _condition["road_id"] = std::to_string(laneUid.roadId);
      _condition["section_id"] = std::to_string(laneUid.sectionId);
      _condition["lane_id"] = std::to_string(laneUid.laneId);
    } else {
      LogWarn << "condition ego_attach_laneid_custom parse error.";
      _condition.clear();
    }
  } else if (IsValid(ConditionType::simulation_time)) {
    txFloat time_in_second;
    ConditionEquationOp rule;
    if (CallSucc(SimulationTime_value_s(time_in_second)) && CallSucc(SimulationTime_rule(rule))) {
      _condition["type"] = _plus_(Base::Enums::SceneEventType::dummy_time_trigger)._to_string();
      _condition["trigger_time"] = std::to_string(time_in_second);
      _condition["equation_op"] = rule._to_string();
    } else {
      LogWarn << "condition simulation_time parse error.";
      _condition.clear();
    }
  } else {
    LogWarn << "unsupport trigger condition type.";
    _condition.clear();
  }

  SceneLoaderInfo << "******** condition ********";
  for (const auto& pair : _condition) {
    SceneLoaderInfo << TX_VARS_NAME(Key, pair.first) << TX_VARS_NAME(Value, pair.second);
  }
  SceneLoaderInfo << "***************************";
  return _condition;
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::TimeToCollision_type_sysId(
    std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::ttc))) {
    IConditionTimeToCollision::value_type s = boost::get<IConditionTimeToCollision::value_type>(condition_data);
    std::tie(std::get<0>(type_sysId), std::get<1>(type_sysId)) =
        Base::ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(s.entity_ref());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::TimeToCollision_distance_type(
    Base::Enums::DistanceProjectionType& _type) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::ttc))) {
    IConditionTimeToCollision::value_type s = boost::get<IConditionTimeToCollision::value_type>(condition_data);
    _type = Enum2DistanceType(s.distance_type());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::TimeToCollision_rule(
    Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::ttc))) {
    IConditionTimeToCollision::value_type s = boost::get<IConditionTimeToCollision::value_type>(condition_data);
    _rule = Enum2OpType(s.rule());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::TimeToCollision_value(Base::txFloat& _v) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::ttc))) {
    IConditionTimeToCollision::value_type s = boost::get<IConditionTimeToCollision::value_type>(condition_data);
    _v = s.value();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::Speed_direction(Base::Enums::ConditionDirDimension& _dir) const
    TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::speed))) {
    IConditionSpeed::value_type s = boost::get<IConditionSpeed::value_type>(condition_data);
    _dir = Enum2DirDimension(s.direction());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::Speed_rule(Base::Enums::ConditionEquationOp& _rule) const
    TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::speed))) {
    IConditionSpeed::value_type s = boost::get<IConditionSpeed::value_type>(condition_data);
    _rule = Enum2OpType(s.rule());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::Speed_value_m_s(Base::txFloat& _v) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::speed))) {
    IConditionSpeed::value_type s = boost::get<IConditionSpeed::value_type>(condition_data);
    _v = s.value();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::RelativeSpeed_type_sysId(
    std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::relative_speed))) {
    IConditionRelativeSpeed::value_type s = boost::get<IConditionRelativeSpeed::value_type>(condition_data);
    std::tie(std::get<0>(type_sysId), std::get<1>(type_sysId)) =
        Base::ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(s.entity_ref());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::RelativeSpeed_direction(
    Base::Enums::ConditionDirDimension& _dir) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::relative_speed))) {
    IConditionRelativeSpeed::value_type s = boost::get<IConditionRelativeSpeed::value_type>(condition_data);
    _dir = Enum2DirDimension(s.direction());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::RelativeSpeed_rule(
    Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::relative_speed))) {
    IConditionRelativeSpeed::value_type s = boost::get<IConditionRelativeSpeed::value_type>(condition_data);
    _rule = Enum2OpType(s.rule());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::RelativeDistance_type_sysId(
    std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::distance))) {
    IConditionRelativeDistance::value_type s = boost::get<IConditionRelativeDistance::value_type>(condition_data);
    std::tie(std::get<0>(type_sysId), std::get<1>(type_sysId)) =
        Base::ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(s.entity_ref());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::RelativeDistance_distance_type(
    Base::Enums::DistanceProjectionType& _type) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::distance))) {
    IConditionRelativeDistance::value_type s = boost::get<IConditionRelativeDistance::value_type>(condition_data);
    _type = Enum2DistanceType(s.distance_type());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::RelativeDistance_rule(
    Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::distance))) {
    IConditionRelativeDistance::value_type s = boost::get<IConditionRelativeDistance::value_type>(condition_data);
    _rule = Enum2OpType(s.rule());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::RelativeDistance_value_m(Base::txFloat& _v) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::distance))) {
    IConditionRelativeDistance::value_type s = boost::get<IConditionRelativeDistance::value_type>(condition_data);
    _v = s.value();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::EgoAttachLaneidCustom_road_id(Base::txLaneUId& _laneUid) const
    TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::ego_attach_laneid_custom))) {
    IConditionEgoAttachLaneidCustom::value_type s =
        boost::get<IConditionEgoAttachLaneidCustom::value_type>(condition_data);
    _laneUid = Base::txLaneUId(s.road_id(), s.section_id(), s.lane_id());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::RelativeSpeed_value_m_s(Base::txFloat& _v) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::relative_speed))) {
    IConditionRelativeSpeed::value_type s = boost::get<IConditionRelativeSpeed::value_type>(condition_data);
    _v = s.value();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::ReachPosition_radius_m(Base::txFloat& _r) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::reach_position))) {
    IConditionReachPosition::value_type s = boost::get<IConditionReachPosition::value_type>(condition_data);
    _r = s.radius();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::ReachPosition_position(Position& _pos) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::reach_position))) {
    IConditionReachPosition::value_type s = boost::get<IConditionReachPosition::value_type>(condition_data);
    _pos.Initialize(s.position());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::SimulationTime_rule(
    Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::simulation_time))) {
    IConditionSimulationTime::value_type s = boost::get<IConditionSimulationTime::value_type>(condition_data);
    _rule = Enum2OpType(s.rule());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::SimulationTime_value_s(Base::txFloat& _v) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::simulation_time))) {
    IConditionSimulationTime::value_type s = boost::get<IConditionSimulationTime::value_type>(condition_data);
    _v = s.value();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::TimeHeadWay_type_sysId(
    std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::time_headway))) {
    IConditionTimeHeadWay::value_type s = boost::get<IConditionTimeHeadWay::value_type>(condition_data);
    std::tie(std::get<0>(type_sysId), std::get<1>(type_sysId)) =
        Base::ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(s.entity_ref());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::TimeHeadWay_rule(Base::Enums::ConditionEquationOp& _rule) const
    TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::time_headway))) {
    IConditionTimeHeadWay::value_type s = boost::get<IConditionTimeHeadWay::value_type>(condition_data);
    _rule = Enum2OpType(s.rule());
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::TimeHeadWay_value(Base::txFloat& _v) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::time_headway))) {
    IConditionTimeHeadWay::value_type s = boost::get<IConditionTimeHeadWay::value_type>(condition_data);
    _v = s.value();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::TimeHeadWay_freespace(Base::txBool& _freespace) const
    TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::time_headway))) {
    IConditionTimeHeadWay::value_type s = boost::get<IConditionTimeHeadWay::value_type>(condition_data);
    _freespace = s.freespace();
    return true;
  } else {
    return false;
  }
}
Base::txBool PlanningSceneEvent_SceneLoader::Condition::TimeHeadWay_along_route(Base::txBool& _along_route) const
    TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::time_headway))) {
    IConditionTimeHeadWay::value_type s = boost::get<IConditionTimeHeadWay::value_type>(condition_data);
    _along_route = s.along_route();
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Condition::ElementState_evId_actionId(
    std::tuple<Base::txSysId, Base::txInt>& evId_actId) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::storyboard_element_state))) {
    IConditionStoryboardElementState::value_type s =
        boost::get<IConditionStoryboardElementState::value_type>(condition_data);
    auto str_kv = Utils::SpliteStringVector(s.storyboard_ref(), "_");

    if (2 == str_kv.size()) {
      std::get<0>(evId_actId) = std::atoi(str_kv[0].c_str());
      std::get<1>(evId_actId) = std::atoi(str_kv[1].c_str());
      return true;
    } else {
      LOG(WARNING) << "storyboard_ref parse failure. " << TX_VARS(s.storyboard_ref());
      return false;
    }
  } else {
    return false;
  }
}
Base::txBool PlanningSceneEvent_SceneLoader::Condition::ElementState_state(
    TrafficFlow::TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType& _rule) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ConditionType::storyboard_element_state))) {
    IConditionStoryboardElementState::value_type s =
        boost::get<IConditionStoryboardElementState::value_type>(condition_data);

    auto _pb_state = s.state();
    if (sim_msg::StoryboardElementState::STORYBOARD_ELEMENT_STATE_COMPLETE_STATE == _pb_state) {
      _rule = TrafficFlow::TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::completeState;
      return true;
    } else if (sim_msg::StoryboardElementState::STORYBOARD_ELEMENT_STATE_RUNNING_STATE == _pb_state) {
      _rule = TrafficFlow::TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::runningState;
      return true;
    } else {
      LOG(WARNING) << "ElementState_state parse failure. unsupport state. " << TX_VARS(_pb_state);
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::Trigger::Initialize(const sim_msg::Trigger& refTrigger) TX_NOEXCEPT {
  m_name = refTrigger.name();
  m_vec_condition.clear();
  for (const auto& refSingleCond : refTrigger.conditions()) {
    Condition cur_cond;
    if (cur_cond.Initialize(refSingleCond)) {
      m_vec_condition.emplace_back(cur_cond);
    }
  }
  return _NonEmpty_(m_vec_condition);
}

Base::txBool PlanningSceneEvent_SceneLoader::action_ego::Initialize(const sim_msg::Action& refActionEgo) TX_NOEXCEPT {
  Clear();
  if (refActionEgo.has_activate()) {
    action_type = ActionType::activate;
    action_data = refActionEgo.activate();
    return true;
  } else if (refActionEgo.has_assign()) {
    action_type = ActionType::assign;
    action_data = refActionEgo.assign();
    return true;
  } else if (refActionEgo.has_override()) {
    action_type = ActionType::override;
    action_data = refActionEgo.override();
    return true;
  } else if (refActionEgo.has_command()) {
    action_type = ActionType::command;
    action_data = refActionEgo.command();
    return true;
  } else if (refActionEgo.has_status()) {
    action_type = ActionType::status;
    action_data = refActionEgo.status();
    return true;
  }
  return false;
}

Base::txString PlanningSceneEvent_SceneLoader::action_ego::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  switch (action_type) {
    case ActionType::activate: {
      Activate _a;
      activate(_a);
      oss << "ActionType::activate, " << _a.Str();
      break;
    }
    case ActionType::assign: {
      Assign _a;
      assign(_a);
      oss << "ActionType::assign, " << _a.Str();
      break;
    }
    case ActionType::override: {
      Override _a;
      override_(_a);
      oss << "ActionType::override, " << _a.Str();
      break;
    }
    case ActionType::command: {
      Command _a;
      command(_a);
      oss << "ActionType::command, " << _a.Str();
      break;
    }
    case ActionType::status: {
      Status _a;
      status(_a);
      oss << "ActionType::status, " << _a.Str();
      break;
    }
    default:
      oss << "ActionType::None";
      break;
  }
  return oss.str();
}

void PlanningSceneEvent_SceneLoader::action_ego::Clear() TX_NOEXCEPT {
  action_type = ActionType::none;
  action_data = NoneAction();
}

Base::txBool PlanningSceneEvent_SceneLoader::action_ego::activate(Activate& _a) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ActionType::activate))) {
    Activate::value_type s = boost::get<Activate::value_type>(action_data);
    return _a.Initialize(s);
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::action_ego::assign(Assign& _a) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ActionType::assign))) {
    Assign::value_type s = boost::get<Assign::value_type>(action_data);
    return _a.Initialize(s);
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::action_ego::override_(Override& _a) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ActionType::override))) {
    Override::value_type s = boost::get<Override::value_type>(action_data);
    return _a.Initialize(s);
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::action_ego::command(Command& _a) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ActionType::command))) {
    Command::value_type s = boost::get<Command::value_type>(action_data);
    return _a.Initialize(s);
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::action_ego::status(Status& _a) const TX_NOEXCEPT {
  if (CallSucc(IsValid(ActionType::status))) {
    Status::value_type s = boost::get<Status::value_type>(action_data);
    return _a.Initialize(s);
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::event_ego::Initialize(const sim_msg::Event& _eventEgo) TX_NOEXCEPT {
  m_id = _eventEgo.id();
  m_name = _eventEgo.name();
  m_info = _eventEgo.info();
  m_trigger.Initialize(_eventEgo.trigger());
  m_action.clear();
  Base::txInt actId = 0;
  for (const auto& refActionEgo : _eventEgo.actions()) {
    action_ego cur_action;
    cur_action.Initialize(refActionEgo);
    cur_action.set_action_id(actId);
    m_action.emplace_back(std::move(cur_action));
    actId++;
  }
  return true;
}

Base::txBool PlanningSceneEvent_SceneLoader::DummyDriverViewer::Initialize(const sim_msg::Ego& refEgo) TX_NOEXCEPT {
  m_inited = true;
  m_dummyId = refEgo.id();
  m_name = refEgo.name();
  m_group_name = refEgo.group();
  // type current delete
  m_behavior = Str2Behavior("");
  for (const auto& refPhy : refEgo.physicles()) {
    physicle_ego new_phy;
    new_phy.Initialize(refPhy);
    m_physicles.emplace_back(std::move(new_phy));
  }
  if (_Empty_(m_physicles)) {
    LOG(WARNING) << "physicles empty.";
    return false;
  }
  m_initial.Initialize(refEgo.initial());
  m_dynamic.clear();
  m_scene_event_viewer_ptr_vec.clear();
  for (const auto& ref : refEgo.dynamic().events()) {
    event_ego _e;
    _e.Initialize(ref);
    m_dynamic.emplace_back(_e);

    IDummySceneEventViewerPtr scene_event_viewer_ptr = std::make_shared<IDummySceneEventViewer>();
    kvMapVec conditionMapVec;
    LOG(WARNING) << TX_VARS(_e.trigger().conditions().size());
    for (const auto& refCond : _e.trigger().conditions()) {
      conditionMapVec.emplace_back(refCond.KeyValueMap());
    }
    if (NonNull_Pointer(scene_event_viewer_ptr) &&
        CallSucc(scene_event_viewer_ptr->initialize(
            m_dummyId, conditionMapVec /*_e.trigger().condition().KeyValueMap()*/, _e.actions()))) {
      scene_event_viewer_ptr->set_version(eSceneEventVersion::V_1_2_0_0);
      m_scene_event_viewer_ptr_vec.emplace_back(std::move(scene_event_viewer_ptr));
    }
  }
  m_sensor_group.Initialize(refEgo.sensor_group());

  return IsInited();
}

Base::txString PlanningSceneEvent_SceneLoader::DummyDriverViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "DummyDriverViewer {" << TX_VARS(id()) << TX_VARS(Name()) << TX_VARS(Type())
      << TX_VARS_NAME(initEgo, initEgo().Str()) << TX_VARS_NAME(physicle_ego, phyEgoLeader().Str())
      << TX_VARS_NAME(phyEgoSize, phyEgoSize());
  for (const auto& ref : m_scene_event_viewer_ptr_vec) {
    std::ostringstream oss_cond;
    for (const auto& refCond : ref->conditionVec()) {
      oss_cond << (refCond->Str()) << ",";
    }
    oss << TX_VARS_NAME(Condition, oss_cond.str());
    oss << TX_VARS_NAME(actions_size, ref->actions().size());
    for (const auto& refAction : ref->actions()) {
      oss << TX_VARS_NAME(action, refAction.Str());
    }
  }
  oss << "}";
  return oss.str();
}

PlanningSceneEvent_SceneLoader::DummyDriverViewerPtr PlanningSceneEvent_SceneLoader::GetDummyDriverViewer(
    const Base::txSysId _id) const TX_NOEXCEPT {
  for (const DummyDriverViewerPtr ptr : m_dummy_viewer_vec) {
    if (NonNull_Pointer(ptr) && CallSucc(_id == ptr->id())) {
      return ptr;
    }
  }
  return nullptr;
}

Base::txBool PlanningSceneEvent_SceneLoader::LoadSceneEvent(sim_msg::Scene& ref_scene_event) TX_NOEXCEPT {
  ClearSceneEvent();
  m_driver_scene_event.CopyFrom(ref_scene_event);
  if ("1.1.0.0" == m_driver_scene_event.setting().version() || "1.0.0" == m_driver_scene_event.setting().version() ||
      "1.2.0.0" == m_driver_scene_event.setting().version()) {
    const Base::txString version = m_driver_scene_event.setting().version();
    if (boost::equal(txString("1.0.0.0"), version) || boost::equal(txString("1.0.0"), version)) {
      SetSceneEventVersion(ParentClass::eSceneEventVersion::V_1_0_0_0);
    } else if (boost::equal(txString("1.1.0.0"), version)) {
      SetSceneEventVersion(ParentClass::eSceneEventVersion::V_1_1_0_0);
    } else if (boost::equal(txString("1.2.0.0"), version)) {
      SetSceneEventVersion(ParentClass::eSceneEventVersion::V_1_2_0_0);
    }

    if (CallSucc(ParseSceneEvent())) {
      for (DummyDriverViewerPtr ptr : m_dummy_viewer_vec) {
        if (NonNull_Pointer(ptr)) {
          SceneLoaderInfo << (ptr->Str());
        }
      }
      SceneLoaderInfo << "parse scene event success.";
      return true;
    } else {
      LogWarn << "parse scene event failure.";
      return false;
    }
  } else {
    LogWarn << "un support scene event version " << m_driver_scene_event.setting().version();
    return false;
  }
}

#if __TX_Mark__("ConditionSpeed")
Base::txString PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummySpeedTriggerConditionViewer::Str() const
    TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(direction_type, __enum2lpsz__(ConditionDirDimension, direction()))
      << TX_VARS_NAME(speed_type, __enum2lpsz__(ConditionSpeedType, speed_type()))
      << TX_VARS_NAME(equation_op, __enum2lpsz__(ConditionEquationOp, op()))
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(speed_threshold, speed_threshold()) << TX_VARS_NAME(delay_threshold, delay_threshold())
      << TX_VARS_NAME(trigger_count, trigger_count()) << "}" << std::endl;
  return oss.str();
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummySpeedTriggerConditionViewer::initialize(
    const kvMap& ref_kv_map) TX_NOEXCEPT {
  if (_Contain_(ref_kv_map, "direction_type") && _Contain_(ref_kv_map, "equation_op") &&
      _Contain_(ref_kv_map, "speed_threshold") && _Contain_(ref_kv_map, "condition_boundary") &&
      _Contain_(ref_kv_map, "delay_threshold") && _Contain_(ref_kv_map, "trigger_count")) {
    try {
      mEquOp = __lpsz2enum__(ConditionEquationOp, ref_kv_map.at("equation_op").c_str());
      mDirection = __lpsz2enum__(ConditionDirDimension, ref_kv_map.at("direction_type").c_str());
      mBoundaryType = __lpsz2enum__(ConditionBoundaryType, ref_kv_map.at("condition_boundary").c_str());
      m_speed_threshold = std::atof(ref_kv_map.at("speed_threshold").c_str());
      m_delay_threshold = std::atof(ref_kv_map.at("delay_threshold").c_str());
      m_trigger_count = std::atol(ref_kv_map.at("trigger_count").c_str());
      mIsValid = true;
    } catch (...) {
      mIsValid = false;
    }
  } else {
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "direction_type")) << TX_COND(_Contain_(ref_kv_map, "equation_op"))
                 << TX_COND(_Contain_(ref_kv_map, "speed_threshold")) << TX_COND(_Contain_(ref_kv_map, "trigger_count"))
                 << TX_COND(_Contain_(ref_kv_map, "delay_threshold"))
                 << TX_COND(_Contain_(ref_kv_map, "condition_boundary"));
  }
  return IsValid();
}
#endif /*__TX_Mark__("ConditionSpeed")*/

#if __TX_Mark__("ConditionRelativeSpeed")
Base::txString PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyRelativeSpeedTriggerConditionViewer::Str()
    const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(direction_type, __enum2lpsz__(ConditionDirDimension, direction()))
      << TX_VARS_NAME(speed_type, __enum2lpsz__(ConditionSpeedType, speed_type()))
      << TX_VARS_NAME(equation_op, __enum2lpsz__(ConditionEquationOp, op()))
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(speed_threshold, speed_threshold())
      << TX_VARS_NAME(target_type, __enum2lpsz__(ElementType, target_type())) << TX_VARS_NAME(target_id, target_id())
      << TX_VARS_NAME(trigger_count, trigger_count()) << TX_VARS_NAME(delay_threshold, delay_threshold()) << "}"
      << std::endl;
  return oss.str();
}

Base::txBool
PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyRelativeSpeedTriggerConditionViewer::initialize(
    const kvMap& ref_kv_map) TX_NOEXCEPT {
  if (_Contain_(ref_kv_map, "direction_type") && _Contain_(ref_kv_map, "equation_op") &&
      _Contain_(ref_kv_map, "speed_threshold") && _Contain_(ref_kv_map, "target_element") &&
      _Contain_(ref_kv_map, "delay_threshold") && _Contain_(ref_kv_map, "condition_boundary") &&
      _Contain_(ref_kv_map, "trigger_count")) {
    try {
      mEquOp = __lpsz2enum__(ConditionEquationOp, ref_kv_map.at("equation_op").c_str());
      mDirection = __lpsz2enum__(ConditionDirDimension, ref_kv_map.at("direction_type").c_str());
      mBoundaryType = __lpsz2enum__(ConditionBoundaryType, ref_kv_map.at("condition_boundary").c_str());
      m_speed_threshold = std::atof(ref_kv_map.at("speed_threshold").c_str());
      m_trigger_count = std::atol(ref_kv_map.at("trigger_count").c_str());
      m_delay_threshold = std::atof(ref_kv_map.at("delay_threshold").c_str());
      std::tie(m_target_element_type, m_target_element_id) = splite_type_id(ref_kv_map.at("target_element"));
      if (_plus_(ElementType::Unknown) == m_target_element_type) {
        mIsValid = false;
      } else {
        mIsValid = true;
      }
    } catch (...) {
      mIsValid = false;
    }
  } else {
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "direction_type")) << TX_COND(_Contain_(ref_kv_map, "equation_op"))
                 << TX_COND(_Contain_(ref_kv_map, "speed_threshold"))
                 << TX_COND(_Contain_(ref_kv_map, "target_element")) << TX_COND(_Contain_(ref_kv_map, "trigger_count"))
                 << TX_COND(_Contain_(ref_kv_map, "delay_threshold"))
                 << TX_COND(_Contain_(ref_kv_map, "condition_boundary"));
  }
  return IsValid();
}

#endif /*__TX_Mark__("ConditionRelativeSpeed")*/

#if __TX_Mark__("ConditionReachPosition")
Base::txString PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyReachAbsPositionConditionViewer::Str()
    const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(trigger_count, trigger_count()) << TX_VARS_NAME(delay_threshold, delay_threshold())
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(point_gps, Utils::ToString(point_gps())) << TX_VARS_NAME(radius, radius()) << "}" << std::endl;
  return oss.str();
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyReachAbsPositionConditionViewer::initialize(
    const kvMap& ref_kv_map) TX_NOEXCEPT {
  if (_Contain_(ref_kv_map, "radius") && _Contain_(ref_kv_map, "gps") && _Contain_(ref_kv_map, "condition_boundary") &&
      _Contain_(ref_kv_map, "delay_threshold") && _Contain_(ref_kv_map, "trigger_count")) {
    try {
      Utils::Str2GPS(ref_kv_map.at("gps"), m_position);
      m_radius = std::atof(ref_kv_map.at("radius").c_str());
      mBoundaryType = __lpsz2enum__(ConditionBoundaryType, ref_kv_map.at("condition_boundary").c_str());
      m_delay_threshold = std::atof(ref_kv_map.at("delay_threshold").c_str());
      m_trigger_count = std::atol(ref_kv_map.at("trigger_count").c_str());
      mIsValid = true;
    } catch (...) {
      mIsValid = false;
    }
  } else {
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "radius")) << TX_COND(_Contain_(ref_kv_map, "gps"))
                 << TX_COND(_Contain_(ref_kv_map, "condition_boundary"))
                 << TX_COND(_Contain_(ref_kv_map, "delay_threshold"))
                 << TX_COND(_Contain_(ref_kv_map, "trigger_count"));
  }
  return IsValid();
}

#endif /*__TX_Mark__("ConditionReachPosition")*/

#if __TX_Mark__("ConditionTimeToCollision")
Base::txString PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTTCTriggerViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(ttc_threshold, ttc_threshold())
      << TX_VARS_NAME(equation_op, __enum2lpsz__(ConditionEquationOp, op()))
      << TX_VARS_NAME(target_type, __enum2lpsz__(ElementType, target_type())) << TX_VARS_NAME(target_id, target_id())
      << TX_VARS_NAME(distanceType, __enum2lpsz__(ConditionDistanceType, distanceType()))
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(trigger_count, trigger_count()) << TX_VARS_NAME(delay_threshold, delay_threshold()) << "}"
      << std::endl;
  return oss.str();
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTTCTriggerViewer::initialize(
    const kvMap& ref_kv_map) TX_NOEXCEPT {
  if (CallSucc(ParentClass::initialize(ref_kv_map)) && _Contain_(ref_kv_map, "delay_threshold")) {
    try {
      m_delay_threshold = std::atof(ref_kv_map.at("delay_threshold").c_str());
      mIsValid = true;
    } catch (...) {
      mIsValid = false;
    }
  } else {
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "delay_threshold"));
  }
  return IsValid();
}

#endif /*__TX_Mark__("ConditionTimeToCollision")*/

#if __TX_Mark__("ConditionRelativeDistance")
Base::txString PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyDistanceTriggerViewer::Str() const
    TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(distance_threshold, distance_threshold())
      << TX_VARS_NAME(equation_op, __enum2lpsz__(ConditionEquationOp, op()))
      << TX_VARS_NAME(target_type, __enum2lpsz__(ElementType, target_type())) << TX_VARS_NAME(target_id, target_id())
      << TX_VARS_NAME(distanceType, __enum2lpsz__(ConditionDistanceType, distanceType()))
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(trigger_count, trigger_count()) << TX_VARS_NAME(delay_threshold, delay_threshold()) << "}"
      << std::endl;
  return oss.str();
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyDistanceTriggerViewer::initialize(
    const kvMap& ref_kv_map) TX_NOEXCEPT {
  if (CallSucc(ParentClass::initialize(ref_kv_map)) && _Contain_(ref_kv_map, "delay_threshold")) {
    try {
      m_delay_threshold = std::atof(ref_kv_map.at("delay_threshold").c_str());
      mIsValid = true;
    } catch (...) {
      mIsValid = false;
    }
  } else {
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "delay_threshold"));
  }
  return IsValid();
}

#endif /*__TX_Mark__("ConditionRelativeDistance")*/
Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::initialize(
    const Base::txSysId _evId, const kvMapVec& _condition_vec, const std::vector<action_ego> _action_vec) TX_NOEXCEPT {
  m_event_id = _evId;
#define _CreateCondition_(__type__)                           \
  mCondPtrVec.emplace_back(std::make_shared<__type__>());     \
  if (CallFail(mCondPtrVec.back()->initialize(_condition))) { \
    mCondPtrVec.erase(mCondPtrVec.end() - 1);                 \
  }

  for (const kvMap& _condition : _condition_vec) {
    if (_Contain_(_condition, "type")) {
      IConditionViewer::SceneEventType conditionType = __lpsz2enum__(SceneEventType, _condition.at("type").c_str());
      if (_plus_(IConditionViewer::SceneEventType::dummy_speed) == conditionType) {
        _CreateCondition_(IDummySpeedTriggerConditionViewer);
      } else if (_plus_(IConditionViewer::SceneEventType::dummy_relative_speed) == conditionType) {
        _CreateCondition_(IDummyRelativeSpeedTriggerConditionViewer);
      } else if (_plus_(IConditionViewer::SceneEventType::dummy_reach_position) == conditionType) {
        _CreateCondition_(IDummyReachAbsPositionConditionViewer);
      } else if (_plus_(IConditionViewer::SceneEventType::dummy_ttc_trigger) == conditionType) {
        _CreateCondition_(IDummyTTCTriggerViewer);
      } else if (_plus_(IConditionViewer::SceneEventType::dummy_distance_trigger) == conditionType) {
        _CreateCondition_(IDummyDistanceTriggerViewer);
      } else if (_plus_(IConditionViewer::SceneEventType::dummy_ego_attach_laneid_custom) == conditionType) {
        _CreateCondition_(IDummyEgoAttachLaneUidCustomViewer);
      } else if (_plus_(IConditionViewer::SceneEventType::dummy_time_trigger) == conditionType) {
        _CreateCondition_(IDummyTimeTriggerViewer);
      } else if (_plus_(IConditionViewer::SceneEventType::dummy_time_head_way) == conditionType) {
        _CreateCondition_(IDummyTimeHeadwayTriggerViewer);
      } else if (_plus_(IConditionViewer::SceneEventType::dummy_element_state) == conditionType) {
        _CreateCondition_(IDummyElementStateTriggerViewer);
      } else {
        LOG(WARNING) << "un support condition : " << _condition.at("type");
        clear();
        return false;
      }
    }
  }

  if (_NonEmpty_(mCondPtrVec)) {
    m_action_vec = _action_vec;
  }
  return IsInited();
#undef _CreateCondition_
}

Base::txBool PlanningSceneEvent_SceneLoader::ParseSceneEvent() TX_NOEXCEPT {
  using ConditionDirDimension = Base::Enums::ConditionDirDimension;
  using ConditionBoundaryType = Base::Enums::ConditionBoundaryType;
  using DistanceProjectionType = Base::Enums::DistanceProjectionType;
  using ConditionType = Condition::ConditionType;
  using ConditionEquationOp = Base::Enums::ConditionEquationOp;

  m_dummy_viewer_vec.clear();
  for (const auto& refEgo : m_driver_scene_event.egos()) {
    DummyDriverViewerPtr cur_ego_viewer_ptr = std::make_shared<DummyDriverViewer>();
    if (NonNull_Pointer(cur_ego_viewer_ptr) && CallSucc(cur_ego_viewer_ptr->Initialize(refEgo))) {
      if (cur_ego_viewer_ptr->Group() == m_groupname) {
        LOG(WARNING) << refEgo.DebugString();
      }
      LogWarn << TX_VARS_NAME(dummy_viewer, (cur_ego_viewer_ptr->Str()));
      m_dummy_viewer_vec.emplace_back(std::move(cur_ego_viewer_ptr));
    }
  }
  return _NonEmpty_(m_dummy_viewer_vec);
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummySpeedTriggerConditionViewer::TriggerEvent(
    const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(curElemPtr)) {
    const Base::txFloat left_v = curElemPtr->GetRawVelocity();
    const Base::txFloat right_v = speed_threshold();
    if (CallSucc(Utils::EquationOperator(op(), left_v, right_v))) {
      boost::optional<std::list<Base::txFloat> > lastValueList = lastValue();
      if (CallSucc(Utils::BoundaryOperator(boundaryType(), lastValueList, left_v, right_v))) {
        currentTriggerCount()++;
        if (currentTriggerCount() < trigger_count()) {
          clearLastValue();
          setLastValue(left_v);
          return false;
        } else {
          return true;
        }
      } else {
        setLastValue(left_v);
        return false;
      }
    } else {
      setLastValue(left_v);
      return false;
    }
  } else {
    return false;
  }
  return false;
}

Base::txBool
PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyRelativeSpeedTriggerConditionViewer::TriggerEvent(
    const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr _souceElemPtr,
    Base::IElementManagerPtr /*_elemMgrPtr*/) TX_NOEXCEPT {
  TrafficFlow::SimPlanningVehicleElementPtr planning_ptr =
      std::dynamic_pointer_cast<TrafficFlow::SimPlanningVehicleElement>(_souceElemPtr);
  if (Null_Pointer(planning_ptr)) {
    LogWarn << "planning_ptr is nullptr.";
    return false;
  }
  TrafficFlow::SimPlanningVehicleElement::SceneEventTriggerInfoPtr targetElemPtr =
      planning_ptr->QuerySceneEventTriggerInfoFromEnvData(target_type(), target_id());
  /*Base::ITrafficElementPtr targetElemPtr = Utils::QueryTrafficElementPtr(target_type(), target_id(), _elemMgrPtr);*/
  if (NonNull_Pointer(planning_ptr) && NonNull_Pointer(targetElemPtr)) {
    Base::txFloat left_v = 0.0;
    if (_plus_(ConditionDirDimension::eLONGITUDINAL) == direction()) {
      left_v = planning_ptr->GetRawVelocity() - targetElemPtr->GetRawVelocity();
    } else if (_plus_(ConditionDirDimension::eLATERAL) == direction()) {
      left_v = planning_ptr->GetLateralVelocity() - targetElemPtr->GetLateralVelocity();
    } else {
      left_v = 0.0;
    }

    const Base::txFloat right_v = speed_threshold();
    if (CallSucc(Utils::EquationOperator(op(), left_v, right_v))) {
      boost::optional<std::list<Base::txFloat> > lastValueList = lastValue();
      if (CallSucc(Utils::BoundaryOperator(boundaryType(), lastValueList, left_v, right_v))) {
        currentTriggerCount()++;
        if (currentTriggerCount() < trigger_count()) {
          clearLastValue();
          setLastValue(left_v);
          return false;
        } else {
          return true;
        }
      } else {
        setLastValue(left_v);
        return false;
      }
    } else {
      setLastValue(left_v);
      return false;
    }
  } else {
    return false;
  }
  return false;
}

Base::txBool
PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyReachAbsPositionConditionViewer::TriggerEvent(
    const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(curElemPtr)) {
    Coord::txWGS84 targetPt;
    targetPt.FromWGS84(point_gps());
    Coord::txWGS84 centerPt(curElemPtr->GetLocation());
    const Base::txFloat left_v = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(targetPt, centerPt);
    const Base::txFloat right_v = radius();
    LOG(WARNING) << TX_VARS(left_v) << TX_VARS(right_v);
    boost::optional<std::list<Base::txFloat> > lastValueList = lastValue();
    if (CallSucc(Utils::EquationOperator(_plus_(ConditionEquationOp::eq), left_v, right_v))) {
      if (CallSucc(Utils::BoundaryOperator(boundaryType(), lastValueList, left_v, right_v))) {
        currentTriggerCount()++;
        if (currentTriggerCount() < trigger_count()) {
          clearLastValue();
          setLastValue(left_v);
          return false;
        } else {
          return true;
        }
      } else {
        setLastValue(left_v);
        return false;
      }

    } else {
      setLastValue(left_v);
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTTCTriggerViewer::TriggerEvent(
    const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr _curElemPtr,
    Base::IElementManagerPtr /*_elemMgrPtr*/) TX_NOEXCEPT {
  TrafficFlow::SimPlanningVehicleElementPtr planning_ptr =
      std::dynamic_pointer_cast<TrafficFlow::SimPlanningVehicleElement>(_curElemPtr);
  if (Null_Pointer(planning_ptr)) {
    LogWarn << "planning_ptr is nullptr.";
    return false;
  }
  TrafficFlow::SimPlanningVehicleElement::SceneEventTriggerInfoPtr targetElemPtr =
      planning_ptr->QuerySceneEventTriggerInfoFromEnvData(target_type(), target_id());
  /*Base::ITrafficElementPtr targetElemPtr = Utils::QueryTrafficElementPtr(target_type(), target_id(), _elemMgrPtr);*/
  if (NonNull_Pointer(planning_ptr) && NonNull_Pointer(targetElemPtr)) {
    const DistanceProjectionType projType = _plus_(DistanceProjectionType::Euclidean);
    const Base::txFloat dist = planning_ptr->Compute_TTC_Distance(targetElemPtr, projType);
    boost::optional<std::list<Base::txFloat> > lastValueList = lastValue();
    const Base::txFloat left_v = dist;
    const Base::txFloat right_v = ttc_threshold();
    LOG(WARNING) << TX_VARS(_curElemPtr->Id()) << TX_VARS(target_id()) << TX_VARS(left_v) << TX_VARS(right_v);
    if (CallSucc(Utils::EquationOperator(op(), left_v, right_v))) {
      if (CallSucc(Utils::BoundaryOperator(boundaryType(), lastValueList, left_v, right_v))) {
        currentTriggerCount()++;
        if (currentTriggerCount() < trigger_count()) {
          clearLastValue();
          setLastValue(left_v);
          return false;
        } else {
          return true;
        }
      } else {
        setLastValue(left_v);
        return false;
      }
    } else {
      setLastValue(left_v);
      return false;
    }
  } else {
    LOG(WARNING) << TX_COND(NonNull_Pointer(planning_ptr)) << TX_COND(NonNull_Pointer(targetElemPtr));
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyDistanceTriggerViewer::TriggerEvent(
    const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr _curElemPtr,
    Base::IElementManagerPtr /*_elemMgrPtr*/) TX_NOEXCEPT {
  TrafficFlow::SimPlanningVehicleElementPtr planning_ptr =
      std::dynamic_pointer_cast<TrafficFlow::SimPlanningVehicleElement>(_curElemPtr);
  if (NonNull_Pointer(planning_ptr)) {
    TrafficFlow::SimPlanningVehicleElement::SceneEventTriggerInfoPtr targetElemPtr =
        planning_ptr->QuerySceneEventTriggerInfoFromEnvData(target_type(), target_id());
    /*Base::ITrafficElementPtr targetElemPtr = Utils::QueryTrafficElementPtr(target_type(), target_id(), _elemMgrPtr);*/
    // Base::ITrafficElementPtr souceElemPtr = _curElemPtr/*_elemMgrPtr->GetEgoArray().front()*/;
    if (NonNull_Pointer(targetElemPtr) && NonNull_Pointer(_curElemPtr)) {
      const DistanceProjectionType projType = _plus_(DistanceProjectionType::Euclidean);
      const Base::txFloat dist = planning_ptr->Compute_EGO_Distance(targetElemPtr, projType);
      boost::optional<std::list<Base::txFloat> > lastValueList = lastValue();
      const Base::txFloat left_v = dist;
      const Base::txFloat right_v = distance_threshold();
      if (CallSucc(Utils::EquationOperator(op(), left_v, right_v))) {
        if (CallSucc(Utils::BoundaryOperator(boundaryType(), lastValueList, left_v, right_v))) {
          currentTriggerCount()++;
          if (currentTriggerCount() < trigger_count()) {
            clearLastValue();
            setLastValue(left_v);
            return false;
          } else {
            return true;
          }
        } else {
          setLastValue(left_v);
          return false;
        }
      } else {
        setLastValue(left_v);
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyEgoAttachLaneUidCustomViewer::TriggerEvent(
    const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(curElemPtr)) {
    const auto laneInfo = curElemPtr->GetCurrentLaneInfo();
    const auto rightInfo = laneUid();
    if (CallFail(laneInfo.isOnLaneLink) && laneInfo.onLaneUid.roadId == rightInfo.roadId &&
        laneInfo.onLaneUid.sectionId == rightInfo.sectionId && laneInfo.onLaneUid.laneId == rightInfo.laneId) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTimeTriggerViewer::TriggerEvent(
    const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr, Base::IElementManagerPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(curElemPtr)) {
    const Base::txFloat passTime = timeMgr.PassTime();
    const Base::txBool op_result = Utils::EquationOperator(op(), passTime, trigger_time());
    LOG(INFO) << "[time_trigger][dummy] " << TX_COND(op_result) << TX_VARS_NAME(op, op()._to_string())
              << TX_VARS(passTime) << TX_VARS_NAME(trigger_time, trigger_time());
    if (CallSucc(op_result)) { /*if (passTime >= trigger_time())*/
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyTimeHeadwayTriggerViewer::TriggerEvent(
    const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr,
    Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT {
  TrafficFlow::SimPlanningVehicleElementPtr planning_ptr =
      std::dynamic_pointer_cast<TrafficFlow::SimPlanningVehicleElement>(curElemPtr);
  if (Null_Pointer(planning_ptr)) {
    LogWarn << "planning_ptr is nullptr.";
    return false;
  }
  TrafficFlow::SimPlanningVehicleElement::SceneEventTriggerInfoPtr targetElemPtr =
      planning_ptr->QuerySceneEventTriggerInfoFromEnvData(target_type(), target_id());
  if (NonNull_Pointer(planning_ptr) && NonNull_Pointer(targetElemPtr)) {
    const auto sign = planning_ptr->Compute_EGO_Distance(targetElemPtr, DistanceProjectionType::Lane);
    const Base::txFloat srcVelocity = planning_ptr->StableVelocity();
    if (sign <= 0.0 && Math::isNotZero(srcVelocity) && srcVelocity > 0.0) {
      Base::txFloat dist = 0.0;
      /*souceElem behind of targetElem*/

      const Base::txFloat targetLength = targetElemPtr->GetLength();
      const Base::txFloat sourceLength = planning_ptr->GetLength();
      if (CallSucc(alongroute())) {
        /*lane proj*/
        if (CallSucc(freespace())) {
          /*bounding box*/
          dist = std::fabs(sign);
        } else {
          /*geom center*/
          dist = std::fabs(sign) - 0.5 * (targetLength + sourceLength);
          dist = (dist > 0.0) ? (dist) : (0.0);
        }
      } else {
        /*abs dist*/
        if (CallSucc(freespace())) {
          /*bounding box*/
          Base::txVec2 nouse_intersectPt1, nouse_intersectPt2;
          dist = Geometry::CD2D::smallest_dist_between_ego_and_obsVehicle(
              targetElemPtr->GetPolygon(), planning_ptr->GetPolygon(), nouse_intersectPt1, nouse_intersectPt2);

        } else {
          /*geom center*/
          dist = Coord::txENU::EuclideanDistanceLocal_2D(targetElemPtr->StableGeomCenter(),
                                                         planning_ptr->StableGeomCenter());
        }
      }
      if (dist < 0.0) {
        LogWarn << "[Error] dist < 0.0 ";
      }
      const Base::txFloat left_v = dist / srcVelocity;
      const Base::txFloat right_v = time_head_way_threshold();
      LOG(WARNING) << TX_VARS(planning_ptr->Id()) << TX_VARS(left_v) << TX_VARS(right_v);
      boost::optional<std::list<Base::txFloat> > lastValueList = lastValue();
      if (CallSucc(Utils::EquationOperator(op(), left_v, right_v))) {
        if (CallSucc(Utils::BoundaryOperator(boundaryType(), lastValueList, left_v, right_v))) {
          currentTriggerCount()++;
          if (currentTriggerCount() < trigger_count()) {
            clearLastValue();
            setLastValue(left_v);
            return false;
          } else {
            return true;
          }
        } else {
          setLastValue(left_v);
          return false;
        }
      } else {
        setLastValue(left_v);
        return false;
      }
    } else {
      return false;
    }
  } else {
    LOG(WARNING) << TX_COND(NonNull_Pointer(planning_ptr)) << TX_COND(NonNull_Pointer(targetElemPtr));
    return false;
  }
}

Base::txBool PlanningSceneEvent_SceneLoader::IDummySceneEventViewer::IDummyElementStateTriggerViewer::TriggerEvent(
    const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
#if 0
  irtual txString eventId_actionId() const TX_NOEXCEPT { return m_dst_event_id_action_id; }
  virtual txSysId dst_event_id() const TX_NOEXCEPT { return m_dst_event_id; }
  virtual txInt dst_event_action_id() const TX_NOEXCEPT { return m_dst_event_action_id; }
  virtual SceneEventElementStatusType state() const TX_NOEXCEPT { return mState; }
#endif
  const auto e_a = eventId_actionId();
  const auto target_state = state();
  TrafficFlow::TAD_SceneEvent_1_0_0_0::SceneEventStateInfo queryRes;
  if (CallSucc(TrafficFlow::TAD_SceneEvent_1_0_0_0::QuerySceneEventStates(e_a, queryRes))) {
    if (target_state == queryRes.state) {
      LOG(INFO) << "query success. state match." << TX_VARS(e_a) << TX_VARS_NAME(state, target_state._to_string());
      return true;
    } else {
      LOG(INFO) << "query success. but state not match" << TX_VARS(e_a)
                << TX_VARS_NAME(need_state, target_state._to_string())
                << TX_VARS_NAME(cur_state, queryRes.state._to_string());
      return false;
    }
  } else {
    LOG(INFO) << "query failure. " << TX_VARS(e_a);
    return false;
  }
}

Base::txString PlanningSceneEvent_SceneLoader::GetSimSimulationTraffic() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->traffic_file_path) : (""));
}

Base::txString PlanningSceneEvent_SceneLoader::GetSimSimulationMapFile() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.mapfile) : (""));
}

Base::txBool PlanningSceneEvent_SceneLoader::GetSimSimulationPlannerRouteStart(
    std::tuple<Base::txFloat, Base::txFloat>& res) const TX_NOEXCEPT {
  Base::txString strStart = (m_DataSource_Scene->_planner.route.start);
  boost::container::small_vector<Base::txString, 2> results;
  boost::algorithm::split(results, strStart, boost::is_any_of(","));
  try {
    res = std::make_tuple<Base::txFloat, Base::txFloat>(std::stod(results[0]), std::stod(results[1]));
  } catch (const std::invalid_argument&) {
    LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                 << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
    return false;
  } catch (const std::out_of_range&) {
    LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                 << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
    return false;
  } catch (...) {
    LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                 << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
    return false;
  }
  return true;
}

Base::txFloat PlanningSceneEvent_SceneLoader::GetSimSimulationMapfileLongitude() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.lon) : (0.0));
}

Base::txFloat PlanningSceneEvent_SceneLoader::GetSimSimulationMapfileLatitude() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.lat) : (0.0));
}

Base::txFloat PlanningSceneEvent_SceneLoader::GetSimSimulationMapfileAltitude() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.alt) : (FLAGS_default_altitude));
}

Base::txBool PlanningSceneEvent_SceneLoader::GetMapManagerInitParams(
    HdMap::HadmapCacheConCurrent::InitParams_t& refParams) TX_NOEXCEPT {
  refParams.strTrafficFilePath = GetSimSimulationTraffic(); /*simulation.traffic,*/
  refParams.strHdMapFilePath = GetSimSimulationMapFile();   /*simData.scene.c_str(),*/

  SceneLoaderInfo << "Map File From .sim : " << refParams.strHdMapFilePath;
  using namespace Utils;
  using namespace boost::filesystem;
  FilePath path_CurrentPath = FilePath(_source).parent_path();
  path_CurrentPath.append(refParams.strHdMapFilePath);
  refParams.strHdMapFilePath = path_CurrentPath.string();
  SceneLoaderInfo << "Map File Modify .sim : " << refParams.strHdMapFilePath;

  __Lon__(refParams.SceneOriginGPS) = GetSimSimulationMapfileLongitude();
  __Lat__(refParams.SceneOriginGPS) = GetSimSimulationMapfileLatitude();
  __Alt__(refParams.SceneOriginGPS) = GetSimSimulationMapfileAltitude();

  return true;
}

Base::txBool PlanningSceneEvent_SceneLoader::HasEgoId(const Base::txSysId egoId) const TX_NOEXCEPT {
  for (const auto& refViewerPtr : m_dummy_viewer_vec) {
    if (NonNull_Pointer(refViewerPtr) && egoId == (refViewerPtr->id())) {
      return true;
    }
  }
  return false;
}

#if __TX_Mark__("Str()")
Base::txString PlanningSceneEvent_SceneLoader::initial_ego::Str() const TX_NOEXCEPT {
  /*
  * txFloat Speed() const TX_NOEXCEPT { return m_initial.speed(); }
          Assign assign() const TX_NOEXCEPT { Assign _assign; _assign.Initialize(m_initial.assign()); return
  std::move(_assign); } Activate activate() const TX_NOEXCEPT { Activate _activate;
  _activate.Initialize(m_initial.activate()); return std::move(_activate); } WaypointVec Waypoints() const TX_NOEXCEPT
          {
                  WaypointVec retV;
                  for (const auto& refwp : m_initial.waypoints())
                  {
                          Waypoint wp; wp.Initialize(refwp); retV.emplace_back(std::move(wp));
                  }
                  return retV;
          }
          txBool trajectory_enabled() const TX_NOEXCEPT { return m_initial.trajectory_enabled(); }
  */
  std::ostringstream oss, oss_waypoints;
  oss << "initial_ego: {" << TX_VARS_NAME(speed, Speed()) << TX_COND(trajectory_enabled())
      << TX_VARS_NAME(Assign, assign().Str()) << TX_VARS_NAME(Activate, activate().Str());

  const auto& way_pts = Waypoints();
  oss_waypoints << "{" << TX_VARS_NAME(pt_size, way_pts.size());
  for (const auto& refwp : way_pts) {
    oss_waypoints << refwp.Str();
  }
  oss_waypoints << "}";
  oss << TX_VARS_NAME(waypoints, oss_waypoints.str());
  oss << "}";
  return oss.str();
}

Base::Enums::ControlPathGear GearSts2Enums(const sim_msg::GearSts& gear) TX_NOEXCEPT {
  if (sim_msg::GearSts::GEAR_STS_R == gear) {
    return _plus_(Base::Enums::ControlPathGear::reverse);
  } else {
    return _plus_(Base::Enums::ControlPathGear::drive);
  }
}

Base::txString PlanningSceneEvent_SceneLoader::Waypoint::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  hadmap::txPoint gps;
  GetPosition().gps(gps);
  oss << "{" << Utils::ToString(gps) << TX_VARS(Speed_m_s()) << TX_VARS_NAME(Gear, GearSts2Enums(Gear())._to_string())
      << "},";
  return oss.str();
}

Base::txString PlanningSceneEvent_SceneLoader::physicle_ego::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_VARS_NAME(ego_physicle_common, Common().Str())
      << TX_VARS_NAME(performance, Performance().Str())
      // << TX_VARS_NAME(FrontAxle, FrontAxle().Str()) << TX_VARS_NAME(RearAxle, RearAxle().Str())
      // << TX_VARS_NAME(Offset_X, Offset_X()) << TX_VARS_NAME(Offset_Z, Offset_Z())
      << "}";
  return oss.str();
}

Base::txString PlanningSceneEvent_SceneLoader::bounding_box::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_VARS_NAME(Center, Utils::ToString(Center())) << TX_VARS_NAME(Length, Length())
      << TX_VARS_NAME(Width, Width()) << TX_VARS_NAME(Higth, Higth()) << "}";
  return oss.str();
}

Base::txString PlanningSceneEvent_SceneLoader::performance::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_VARS_NAME(max_speed_m_s, max_speed_m_s()) << TX_VARS_NAME(max_accel_m_ss, max_accel_m_ss())
      << TX_VARS_NAME(max_decel_m_ss, max_decel_m_ss()) << "}";
  return oss.str();
}

Base::txString PlanningSceneEvent_SceneLoader::ego_physicle_common::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_VARS_NAME(BoundingBox, BoundingBox().Str()) << TX_VARS_NAME(Color, Color())
      << TX_VARS_NAME(Mass, Mass())
      //<< TX_VARS_NAME(GeometryReference, GeometryReference()) << TX_VARS_NAME(ReferencePoint, ReferencePoint())
      << "}";
  return oss.str();
}
//
// Base::txString PlanningSceneEvent_SceneLoader::alxe::Str() const TX_NOEXCEPT
// {
//  std::ostringstream oss;
//  oss << "{" << TX_VARS_NAME(MaxSteering, MaxSteering()) << TX_VARS_NAME(TrackWidth_m, TrackWidth_m())
//    << TX_VARS_NAME(WheelDiameter_m, WheelDiameter_m()) << TX_VARS_NAME(PositionX, PositionX()) <<
// TX_VARS_NAME(PositionZ, PositionZ()) << "}";   return oss.str();
// }

#endif /*__TX_Mark__("Str()")*/
TX_NAMESPACE_CLOSE(SceneLoader)
