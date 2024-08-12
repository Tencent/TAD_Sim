// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_scene_loader.h"
#include <regex>
#include "nlohmann/json.hpp"
#include "tx_string_utils.h"
#include "tx_timer_on_cpu.h"
TX_NAMESPACE_OPEN(Base)

Base::txBool ISceneLoader::sBeSelfDefinedSeed = false;
Base::txInt ISceneLoader::snSelfDefinedSeed = 55;

Base::txInt ISceneLoader::GetSelfDefineRandomSeed() TX_NOEXCEPT {
  const auto now_count = Base::TimingCPU::GetCurrentTimeCount();
  return (now_count % 100);
}

ISceneLoader::VEHICLE_BEHAVIOR ISceneLoader::IVehiclesViewer::behaviorEnum() const TX_NOEXCEPT {
  const txString strBehavior = behavior();

  if (_NonEmpty_(FLAGS_VehicleBehaviorMapping) && (strBehavior == FLAGS_VehicleBehaviorMapping)) {
    return _plus_(VEHICLE_BEHAVIOR::eInjectMode);
  }

  // 根据behavior进行匹配，返回对应枚举
  if (strBehavior == txString("Planning")) {
    return _plus_(VEHICLE_BEHAVIOR::ePlanning);
  } else if (strBehavior == txString("UserDefine")) {
    return _plus_(VEHICLE_BEHAVIOR::eUserDefined);
  } else if (strBehavior == txString("TrafficVehicle")) {
    return _plus_(VEHICLE_BEHAVIOR::eTadAI);
  } else if (strBehavior == txString("TrafficVehicleArterial")) {
    return _plus_(VEHICLE_BEHAVIOR::eTadAI_Arterial);
  } else if (strBehavior == txString("TrajectoryFollow")) {
    return _plus_(VEHICLE_BEHAVIOR::eTrajectoryFollow);
  } else if (strBehavior == txString("RelativeTrajectoryFollow")) {
    return _plus_(VEHICLE_BEHAVIOR::eRelativeTrajectoryFollow);
  } else {
    LOG(FATAL) << ", Unsupport Behavior :" << strBehavior;
    return _plus_(VEHICLE_BEHAVIOR::eUserDefined);
  }
}

ISceneLoader::EventActionType ISceneLoader::IViewer::EvaluateActionType(const txString &strProfile) TX_NOEXCEPT {
  LOG(WARNING) << TX_VARS(strProfile) << TX_VARS_NAME(ttc, FLAGS_EventTypeTTC)
               << TX_VARS_NAME(ego, FLAGS_EventTypeEgoDistance);
  // 判断strProfile是否包含TTC或EgoDistance事件类型
  if (Utils::IsStringContain(strProfile, FLAGS_EventTypeTTC TX_MARK("ttc")) ||
      Utils::IsStringContain(strProfile, FLAGS_EventTypeEgoDistance TX_MARK("egodistance"))) {
    TX_MARK("ttc/egodistance xxxx");
    // 判断strProfile是否包含欧几里得距离或车道投影类型
    if (Utils::IsStringContain(strProfile, FLAGS_DistProjectionTypeEuclid TX_MARK("euclideandistance")) ||
        Utils::IsStringContain(strProfile, FLAGS_DistProjectionTypeLane TX_MARK("laneprojection"))) {
      TX_MARK("ttc/egodistance euclideandistance/laneprojection xxxx");
      // 判断strProfile是否包含"["和"]"
      if (Utils::IsStringContain(strProfile, "[") && Utils::IsStringContain(strProfile, "]")) {
        // 如果strProfile包含"["和"]"，返回TTC_EgoDist_With_SpecialProjection_With_TriggerIndex事件类型
        return _plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex);
      } else {
        // 如果strProfile不包含"["和"]"，返回TTC_EgoDist_With_SpecialProjection事件类型
        return _plus_(EventActionType::TTC_EgoDist_With_SpecialProjection);
      }
    } else {
      return _plus_(EventActionType::TTC_EgoDist_With_DefaultProjection);
    }
  } else {
    return _plus_(EventActionType::TIME_TRIGGER);
  }
}

ISceneLoader::DistanceProjectionType ISceneLoader::IViewer::EvaluateProjectionType(const txString &strProfile)
    TX_NOEXCEPT {
  if (Utils::IsStringContain(strProfile, FLAGS_DistProjectionTypeEuclid TX_MARK("euclideandistance"))) {
    // 如果strProfile包含欧几里得距离类型，返回Euclidean距离投影类型
    return _plus_(DistanceProjectionType::Euclidean);
  } else if (Utils::IsStringContain(strProfile, FLAGS_DistProjectionTypeLane TX_MARK("laneprojection"))) {
    // 如果strProfile包含车道投影类型，返回Lane距离投影类型
    return _plus_(DistanceProjectionType::Lane);
  } else {
    // 如果strProfile不包含欧几里得距离或车道投影类型，返回默认的Lane距离投影类型
    return _plus_(DistanceProjectionType::Lane);
    TX_MARK("default value.");
  }
}

hadmap::txPoint ISceneLoader::IRouteViewer::startGPS() const TX_NOEXCEPT {
  return hadmap::txPoint(startLon(), startLat(), FLAGS_default_altitude);
}

hadmap::txPoint ISceneLoader::IRouteViewer::endGPS() const TX_NOEXCEPT {
  return hadmap::txPoint(endLon(), endLat(), FLAGS_default_altitude);
}

std::vector<hadmap::txPoint> ISceneLoader::IRouteViewer::midGPS() const TX_NOEXCEPT {
  std::vector<hadmap::txPoint> retMids;
  // 获取轨迹的中间点
  const auto &ref_mid_points = midPoints();
  for (const auto &refPair : ref_mid_points) {
    // 重新构造添加并返回
    retMids.emplace_back(hadmap::txPoint(std::get<0>(refPair), std::get<1>(refPair), FLAGS_default_altitude));
  }
  return retMids;
}

void ISceneLoader::SetEgoData(const sim_msg::Location &_egoInfo) TX_NOEXCEPT { m_EgoData = _egoInfo; }

#if __TX_Mark__("IConditionViewer")

Base::txString ISceneLoader::ISceneEventViewer::IConditionViewer::combine_type_id(
    const std::tuple<ElementType, txSysId> &type_id) TX_NOEXCEPT {
  std::ostringstream oss;
  // ego类型时前缀
  if (_plus_(ElementType::TAD_Ego) == std::get<0>(type_id)) {
    oss << "E_";
  } else if (_plus_(ElementType::TAD_Vehicle) == std::get<0>(type_id)) {  // vehicle类型前缀
    oss << "V_";
  } else if (_plus_(ElementType::TAD_Pedestrian) == std::get<0>(type_id)) {  // 行人前缀
    oss << "P_";
  } else if (_plus_(ElementType::TAD_Immovability) == std::get<0>(type_id)) {  // 障碍物前缀
    oss << "O_";
  } else {
    oss << "U_";
  }
  oss << std::get<1>(type_id);
  return oss.str();
}

std::tuple<ISceneLoader::ISceneEventViewer::IConditionViewer::ElementType, Base::txSysId>
ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(const txString &refStr) TX_NOEXCEPT {
  if (_NonEmpty_(refStr)) {
    const char firstCh = std::toupper(refStr[0]);
    switch (firstCh) {
#  if USE_EgoGroup
      case 'E': {
        // split ego_1 for type and id . If str do not have _ , return FLAGS_Default_EgoVehicle_Id.
        size_t pos = refStr.find('_');
        if (pos != std::string::npos) {
          return std::make_tuple(_plus_(ElementType::TAD_Ego),
                                 (std::atol(refStr.substr(pos + 1).c_str()) + FLAGS_Default_EgoIdPrefix));
        } else {
          return std::make_tuple(_plus_(ElementType::TAD_Ego), FLAGS_Default_EgoVehicle_Id);
        }
      }
#  else  /*USE_EgoGroup*/
      case 'E': {
        return std::make_tuple(_plus_(ElementType::TAD_Ego), FLAGS_Default_EgoVehicle_Id);
      }
#  endif /*USE_EgoGroup*/
      case 'V': {
        // 切分vehicle
        const Base::txString strId = refStr.substr(2, refStr.size() - 2);
        return std::make_tuple(_plus_(ElementType::TAD_Vehicle), std::atol(strId.c_str()));
      }
      case 'P': {
        // 切分pedestrian
        const Base::txString strId = refStr.substr(2, refStr.size() - 2);
        return std::make_tuple(_plus_(ElementType::TAD_Pedestrian), std::atol(strId.c_str()));
      }
      case 'O': {
        const Base::txString strId = refStr.substr(2, refStr.size() - 2);
        return std::make_tuple(_plus_(ElementType::TAD_Immovability), std::atol(strId.c_str()));
      }
      default:
        break;
    }
  }

  return std::make_tuple(_plus_(ElementType::Unknown), FLAGS_Topological_Root_Id);
}
#endif /*__TX_Mark__("IConditionViewer")*/

#if __TX_Mark__("IReachAbsPositionConditionViewer")
Base::txString ISceneLoader::ISceneEventViewer::IReachAbsPositionConditionViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(position_type, __enum2lpsz__(ConditionPositionType, position_type()))
      << TX_VARS_NAME(point_gps, Utils::ToString(point_gps())) << TX_VARS_NAME(radius, radius())
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(trigger_count, trigger_count())
      << TX_VARS_NAME(target_type, __enum2lpsz__(ElementType, target_type())) << TX_VARS_NAME(target_id, target_id())
      << "}" << std::endl;
  return oss.str();
}

Base::txBool ISceneLoader::ISceneEventViewer::IReachAbsPositionConditionViewer::initialize(const kvMap &ref_kv_map)
    TX_NOEXCEPT {
  // 判断ref_kv_map中是否包含所需的键值对
  if (_Contain_(ref_kv_map, "position_type") && _Contain_(ref_kv_map, "lon") && _Contain_(ref_kv_map, "lat") &&
      _Contain_(ref_kv_map, "alt") && _Contain_(ref_kv_map, "radius") && _Contain_(ref_kv_map, "target_element") &&
      _Contain_(ref_kv_map, "condition_boundary") && _Contain_(ref_kv_map, "trigger_count")) {
    try {
      // 从ref_kv_map中获取并设置条件的位置类型、经纬度、高度、半径、边界类型和触发次数
      m_position_type = __lpsz2enum__(ConditionPositionType, ref_kv_map.at("position_type").c_str());
      const txFloat _lon = std::atof(ref_kv_map.at("lon").c_str());
      const txFloat _lat = std::atof(ref_kv_map.at("lat").c_str());
      const txFloat _alt = std::atof(ref_kv_map.at("alt").c_str());
      m_position = hadmap::txPoint(_lon, _lat, _alt);
      m_radius = std::atof(ref_kv_map.at("radius").c_str());
      mBoundaryType = __lpsz2enum__(ConditionBoundaryType, ref_kv_map.at("condition_boundary").c_str());
      m_trigger_count = std::atol(ref_kv_map.at("trigger_count").c_str());
      // 从ref_kv_map中获取并设置目标元素的类型和ID
      std::tie(m_target_element_type, m_target_element_id) = splite_type_id(ref_kv_map.at("target_element"));
      // 判断目标元素类型是否为Unknown
      if (_plus_(ElementType::Unknown) == m_target_element_type) {
        mIsValid = false;
      } else {
        mIsValid = true;
      }
    } catch (...) {
      mIsValid = false;
    }
  } else {
    // 如果ref_kv_map中不包含所需的键值对，将mIsValid设置为false
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "position_type")) << TX_COND(_Contain_(ref_kv_map, "lon"))
                 << TX_COND(_Contain_(ref_kv_map, "lat")) << TX_COND(_Contain_(ref_kv_map, "alt"))
                 << TX_COND(_Contain_(ref_kv_map, "radius")) << TX_COND(_Contain_(ref_kv_map, "target_element"))
                 << TX_COND(_Contain_(ref_kv_map, "condition_boundary"))
                 << TX_COND(_Contain_(ref_kv_map, "trigger_count"));
  }
  return IsValid();
}

#endif /*__TX_Mark__("IReachAbsPositionConditionViewer")*/

#if __TX_Mark__("IReachAbsLaneConditionViewer")

Base::txString ISceneLoader::ISceneEventViewer::IReachAbsLaneConditionViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(position_type, __enum2lpsz__(ConditionPositionType, position_type()))
      << TX_VARS_NAME(road_id, road_id()) << TX_VARS_NAME(lane_id, lane_id())
      << TX_VARS_NAME(lateral_offset, lateral_offset()) << TX_VARS_NAME(longitudinal_offset, longitudinal_offset())
      << TX_VARS_NAME(tolerance, tolerance())
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(trigger_count, trigger_count())
      << TX_VARS_NAME(target_type, __enum2lpsz__(ElementType, target_type())) << TX_VARS_NAME(target_id, target_id())
      << "}" << std::endl;
  return oss.str();
}

Base::txBool ISceneLoader::ISceneEventViewer::IReachAbsLaneConditionViewer::initialize(const kvMap &ref_kv_map)
    TX_NOEXCEPT {
  // 判断ref_kv_map中是否包含所需的键值对
  if (_Contain_(ref_kv_map, "position_type") && _Contain_(ref_kv_map, "road_id") && _Contain_(ref_kv_map, "lane_id") &&
      _Contain_(ref_kv_map, "target_element") && _Contain_(ref_kv_map, "lateral_offset") &&
      _Contain_(ref_kv_map, "longitudinal_offset") && _Contain_(ref_kv_map, "condition_boundary") &&
      _Contain_(ref_kv_map, "trigger_count") && _Contain_(ref_kv_map, "tolerance")) {
    // 解析位置类型 roadid laneid
    m_position_type = __lpsz2enum__(ConditionLaneType, ref_kv_map.at("position_type").c_str());
    m_road_id = std::atol(ref_kv_map.at("road_id").c_str());
    m_lane_id = std::atol(ref_kv_map.at("lane_id").c_str());

    // 解析横向偏移 纵向偏移
    m_lateral_offset = std::atof(ref_kv_map.at("lateral_offset").c_str());
    m_longitudinal_offset = std::atof(ref_kv_map.at("longitudinal_offset").c_str());
    m_tolerance = std::atof(ref_kv_map.at("tolerance").c_str());

    mBoundaryType = __lpsz2enum__(ConditionBoundaryType, ref_kv_map.at("condition_boundary").c_str());
    m_trigger_count = std::atol(ref_kv_map.at("trigger_count").c_str());

    std::tie(m_target_element_type, m_target_element_id) = splite_type_id(ref_kv_map.at("target_element"));
    // 判断目标元素类型是否为Unknown
    if (_plus_(ElementType::Unknown) == m_target_element_type) {
      mIsValid = false;
    } else {
      mIsValid = true;
    }
  } else {
    // 如果ref_kv_map中不包含所需的键值对，将mIsValid设置为false
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "position_type")) << TX_COND(_Contain_(ref_kv_map, "road_id"))
                 << TX_COND(_Contain_(ref_kv_map, "lane_id")) << TX_COND(_Contain_(ref_kv_map, "target_element"))
                 << TX_COND(_Contain_(ref_kv_map, "lateral_offset"))
                 << TX_COND(_Contain_(ref_kv_map, "longitudinal_offset"))
                 << TX_COND(_Contain_(ref_kv_map, "condition_boundary"))
                 << TX_COND(_Contain_(ref_kv_map, "trigger_count")) << TX_COND(_Contain_(ref_kv_map, "tolerance"));
  }
  return IsValid();
}

#endif /*__TX_Mark__("IReachAbsLaneConditionViewer")*/

#if __TX_Mark__("IVelocityTriggerConditionViewer")

Base::txBool ISceneLoader::ISceneEventViewer::IVelocityTriggerConditionViewer::initialize(const kvMap &ref_kv_map)
    TX_NOEXCEPT {
  // 检查kv中是否存在必要字段
  if (_Contain_(ref_kv_map, "speed_type") && _Contain_(ref_kv_map, "equation_op") &&
      _Contain_(ref_kv_map, "speed_threshold") && _Contain_(ref_kv_map, "speed_unit") &&
      _Contain_(ref_kv_map, "target_element") && _Contain_(ref_kv_map, "distance_type") &&
      _Contain_(ref_kv_map, "condition_boundary") && _Contain_(ref_kv_map, "trigger_count") &&
      _Contain_(ref_kv_map, "source_element")) {
    try {
      // 为struct中属性按照kv对应赋值
      mSpeedType = __lpsz2enum__(ConditionSpeedType, ref_kv_map.at("speed_type").c_str());
      mEquOp = __lpsz2enum__(ConditionEquationOp, ref_kv_map.at("equation_op").c_str());
      mDistType = __lpsz2enum__(ConditionDistanceType, ref_kv_map.at("distance_type").c_str());
      mBoundaryType = __lpsz2enum__(ConditionBoundaryType, ref_kv_map.at("condition_boundary").c_str());
      m_speed_threshold = std::atof(ref_kv_map.at("speed_threshold").c_str());
      m_trigger_count = std::atol(ref_kv_map.at("trigger_count").c_str());

      std::tie(m_target_element_type, m_target_element_id) = splite_type_id(ref_kv_map.at("target_element"));
      std::tie(m_source_element_type, m_source_element_id) = splite_type_id(ref_kv_map.at("source_element"));
      // 判断目标或源元素类型是否为Unknown
      if (_plus_(ElementType::Unknown) == m_target_element_type ||
          _plus_(ElementType::Unknown) == m_source_element_type) {
        mIsValid = false;
      } else {
        mIsValid = true;
      }
    } catch (...) {
      mIsValid = false;
    }
  } else {
    // 如果ref_kv_map中不包含所需的键值对，将mIsValid设置为false
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "speed_type")) << TX_COND(_Contain_(ref_kv_map, "equation_op"))
                 << TX_COND(_Contain_(ref_kv_map, "speed_threshold")) << TX_COND(_Contain_(ref_kv_map, "speed_unit"))
                 << TX_COND(_Contain_(ref_kv_map, "target_element")) << TX_COND(_Contain_(ref_kv_map, "distance_type"))
                 << TX_COND(_Contain_(ref_kv_map, "condition_boundary"))
                 << TX_COND(_Contain_(ref_kv_map, "trigger_count")) << TX_COND(_Contain_(ref_kv_map, "source_element"));
  }
  return IsValid();
}

Base::txString ISceneLoader::ISceneEventViewer::IVelocityTriggerConditionViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  // 结构化输出condition中的信息
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(speed_type, __enum2lpsz__(ConditionSpeedType, speed_type()))
      << TX_VARS_NAME(equation_op, __enum2lpsz__(ConditionEquationOp, op()))
      << TX_VARS_NAME(distanceType, __enum2lpsz__(ConditionDistanceType, distanceType()))
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(speed_threshold, speed_threshold()) << TX_VARS_NAME(trigger_count, trigger_count())

      << TX_VARS_NAME(target_type, __enum2lpsz__(ElementType, target_type())) << TX_VARS_NAME(target_id, target_id())

      << TX_VARS_NAME(source_type, __enum2lpsz__(ElementType, source_type())) << TX_VARS_NAME(source_id, source_id())

      << "}" << std::endl;
  return oss.str();
}

#endif /*__TX_Mark__("IDistanceTriggerViewer")*/

#if __TX_Mark__("ITimeTriggerViewer")
Base::txString ISceneLoader::ISceneEventViewer::ITimeTriggerViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(trigger_time, trigger_time()) << "}" << std::endl;
  return oss.str();
}

Base::txBool ISceneLoader::ISceneEventViewer::ITimeTriggerViewer::initialize(const kvMap &ref_kv_map) TX_NOEXCEPT {
  // 检查kv中是否存在必要字段
  if (_Contain_(ref_kv_map, "trigger_time")) {
    // 将kv中字段提取并赋值
    m_trigger_time = std::atof(ref_kv_map.at("trigger_time").c_str());
    if (_Contain_(ref_kv_map, "equation_op")) {
      LOG(WARNING) << TX_VARS_NAME(time_equation_op, ref_kv_map.at("equation_op"));
      mEquOp = __lpsz2enum__(ConditionEquationOp, ref_kv_map.at("equation_op").c_str());
    } else {
      LOG(WARNING) << "do not contain equation_op, default eq";
    }
    // 有效位
    mIsValid = true;
  } else {
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "trigger_time"));
  }
  return IsValid();
}
#endif /*__TX_Mark__("ITimeTriggerViewer")*/

#if __TX_Mark__("ITTCTriggerViewer")

Base::txString ISceneLoader::ISceneEventViewer::ITTCTriggerViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(ttc_threshold, ttc_threshold())
      << TX_VARS_NAME(equation_op, __enum2lpsz__(ConditionEquationOp, op()))
      << TX_VARS_NAME(target_type, __enum2lpsz__(ElementType, target_type())) << TX_VARS_NAME(target_id, target_id())
      << TX_VARS_NAME(distanceType, __enum2lpsz__(ConditionDistanceType, distanceType()))
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(trigger_count, trigger_count()) << "}" << std::endl;
  return oss.str();
}

Base::txBool ISceneLoader::ISceneEventViewer::ITTCTriggerViewer::initialize(const kvMap &ref_kv_map) TX_NOEXCEPT {
  // 从kv中解析字段，判断包含字段
  if (_Contain_(ref_kv_map, "ttc_threshold") && _Contain_(ref_kv_map, "equation_op") &&
      _Contain_(ref_kv_map, "target_element") && _Contain_(ref_kv_map, "distance_type") &&
      _Contain_(ref_kv_map, "condition_boundary") && _Contain_(ref_kv_map, "trigger_count")) {
    try {
      // 将kv中字段提取并赋值
      m_ttc_threshold = std::atof(ref_kv_map.at("ttc_threshold").c_str());
      mEquOp = __lpsz2enum__(ConditionEquationOp, ref_kv_map.at("equation_op").c_str());
      mDistType = __lpsz2enum__(ConditionDistanceType, ref_kv_map.at("distance_type").c_str());
      mBoundaryType = __lpsz2enum__(ConditionBoundaryType, ref_kv_map.at("condition_boundary").c_str());
      m_trigger_count = std::atol(ref_kv_map.at("trigger_count").c_str());
      std::tie(m_target_element_type, m_target_element_id) = splite_type_id(ref_kv_map.at("target_element"));
      // 判断目标元素类型是否为Unknown
      if (_plus_(ElementType::Unknown) == m_target_element_type) {
        mIsValid = false;
      } else {
        mIsValid = true;
      }
    } catch (...) {
      mIsValid = false;
    }
  } else {
    // 如果ref_kv_map中不包含所需的键值对，将mIsValid设置为false
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "ttc_threshold")) << TX_COND(_Contain_(ref_kv_map, "equation_op"))
                 << TX_COND(_Contain_(ref_kv_map, "target_element")) << TX_COND(_Contain_(ref_kv_map, "distance_type"))
                 << TX_COND(_Contain_(ref_kv_map, "condition_boundary"))
                 << TX_COND(_Contain_(ref_kv_map, "trigger_count"));
  }
  return IsValid();
}

#endif /*__TX_Mark__("ITTCTriggerViewer")*/

#if __TX_Mark__("IDistanceTriggerViewer")
Base::txString ISceneLoader::ISceneEventViewer::IDistanceTriggerViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(distance_threshold, distance_threshold())
      << TX_VARS_NAME(equation_op, __enum2lpsz__(ConditionEquationOp, op()))
      << TX_VARS_NAME(target_type, __enum2lpsz__(ElementType, target_type())) << TX_VARS_NAME(target_id, target_id())
      << TX_VARS_NAME(distanceType, __enum2lpsz__(ConditionDistanceType, distanceType()))
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(trigger_count, trigger_count()) << "}" << std::endl;
  return oss.str();
}

Base::txBool ISceneLoader::ISceneEventViewer::IDistanceTriggerViewer::initialize(const kvMap &ref_kv_map) TX_NOEXCEPT {
  // 判断ref_kv_map中是否包含所需的键值对
  if (_Contain_(ref_kv_map, "distance_threshold") && _Contain_(ref_kv_map, "equation_op") &&
      _Contain_(ref_kv_map, "target_element") && _Contain_(ref_kv_map, "distance_type") &&
      _Contain_(ref_kv_map, "condition_boundary") && _Contain_(ref_kv_map, "trigger_count")) {
    try {
      // 从kv中提取并赋值
      m_distance_threshold = std::atof(ref_kv_map.at("distance_threshold").c_str());
      mEquOp = __lpsz2enum__(ConditionEquationOp, ref_kv_map.at("equation_op").c_str());
      mDistType = __lpsz2enum__(ConditionDistanceType, ref_kv_map.at("distance_type").c_str());
      mBoundaryType = __lpsz2enum__(ConditionBoundaryType, ref_kv_map.at("condition_boundary").c_str());
      m_trigger_count = std::atol(ref_kv_map.at("trigger_count").c_str());
      std::tie(m_target_element_type, m_target_element_id) = splite_type_id(ref_kv_map.at("target_element"));
      if (_plus_(ElementType::Unknown) == m_target_element_type) {
        mIsValid = false;
      } else {
        mIsValid = true;
      }

      if (_Contain_(ref_kv_map, "source_element")) {
        std::tie(m_source_element_type, m_source_element_id) = splite_type_id(ref_kv_map.at("source_element"));
      }
    } catch (...) {
      mIsValid = false;
    }
  } else {
    // 如果ref_kv_map中不包含所需的键值对，将mIsValid设置为false
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "distance_threshold"))
                 << TX_COND(_Contain_(ref_kv_map, "equation_op")) << TX_COND(_Contain_(ref_kv_map, "target_element"))
                 << TX_COND(_Contain_(ref_kv_map, "distance_type"))
                 << TX_COND(_Contain_(ref_kv_map, "condition_boundary"))
                 << TX_COND(_Contain_(ref_kv_map, "trigger_count"));
  }
  return IsValid();
}

#endif /*__TX_Mark__("IDistanceTriggerViewer")*/

#if __TX_Mark__("IEgoAttachLaneUidCustomViewer")

// 初始化IEgoAttachLaneUidCustomViewer对象
Base::txBool ISceneLoader::ISceneEventViewer::IEgoAttachLaneUidCustomViewer::initialize(const kvMap &ref_kv_map)
    TX_NOEXCEPT {
  // 判断ref_kv_map中是否包含所需的键值对
  if (_Contain_(ref_kv_map, "road_id") && _Contain_(ref_kv_map, "section_id") && _Contain_(ref_kv_map, "lane_id")) {
    // 从ref_kv_map中获取并设置车道的道路ID、区段ID和车道ID
    mLaneUid.roadId = std::atol(ref_kv_map.at("road_id").c_str());
    mLaneUid.sectionId = std::atol(ref_kv_map.at("section_id").c_str());
    mLaneUid.laneId = std::atol(ref_kv_map.at("lane_id").c_str());
    mIsValid = true;
  } else {
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "road_id")) << TX_COND(_Contain_(ref_kv_map, "section_id"))
                 << TX_COND(_Contain_(ref_kv_map, "lane_id"));
  }
  return IsValid();
}

Base::txString ISceneLoader::ISceneEventViewer::IEgoAttachLaneUidCustomViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(LaneUid, Utils::ToString(mLaneUid)) << "}" << std::endl;
  return oss.str();
}

#endif /*__TX_Mark__("IEgoAttachLaneUidCustomViewer")*/

// 初始化IEndConditionViewer对象
Base::txBool ISceneLoader::ISceneEventViewer::IEndConditionViewer::initialize(const kvMapVec &ref_kv_map_vec)
    TX_NOEXCEPT {
  mEndConditionVec.clear();
  mIsValid = true;
  // 遍历ref_kv_map_vec中的每个kvMap
  for (const auto &ref_kv_map : ref_kv_map_vec) {
    // 判断ref_kv_map中是否包含所需的键值对
    if (_Contain_(ref_kv_map, "type") && _Contain_(ref_kv_map, "value")) {
      try {
        // 从ref_kv_map中获取并设置结束条件的类型和值
        EndConditionKeyType key = __lpsz2enum__(EndConditionKeyType, ref_kv_map.at("type").c_str());
        txFloat value = std::atof(ref_kv_map.at("value").c_str());
        mEndConditionVec.emplace_back(std::make_tuple(key, value));
      } catch (...) {
        // 如果在设置过程中发生异常，输出警告日志并将mIsValid设置为false
        LOG(WARNING) << "conver error. " << TX_VARS_NAME(type, ref_kv_map.at("type"))
                     << TX_VARS_NAME(value, ref_kv_map.at("value"));
        mIsValid = false;
        return IsValid();
      }
    } else {
      // 如果ref_kv_map中不包含所需的键值对，输出警告日志并将mIsValid设置为false
      LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "type")) << TX_COND(_Contain_(ref_kv_map, "type"));
      mIsValid = false;
      return IsValid();
    }
  }
  return _NonEmpty_(mEndConditionVec);
}

Base::txString ISceneLoader::ISceneEventViewer::IEndConditionViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "endCondition {" << std::endl;
  for (const auto &refend : endConditionList()) {
    oss << TX_VARS_NAME(endConditionType, __enum2lpsz__(EndConditionKeyType, std::get<0>(refend)))
        << TX_VARS_NAME(endConditionValue, (std::get<1>(refend))) << std::endl;
  }
  oss << "}" << std::endl;
  return oss.str();
}

// 将SceneEventActionType转换为对应的单位字符串
Base::txString ISceneLoader::ISceneEventViewer::IActionViewer::actionType2Unit(const SceneEventActionType &_type)
    TX_NOEXCEPT {
  if (_plus_(SceneEventActionType::acc) == _type) {
    // 如果_type为acc，返回"m/s2 acc"
    return txString("m/s2 acc");
  } else if (_plus_(SceneEventActionType::merge) == _type) {
    return txString("s merge");
  } else if (_plus_(SceneEventActionType::velocity) == _type) {
    // 如果_type为velocity，返回"m/s velocity"
    return txString("m/s velocity");
  } else {
    return txString("unknown");
  }
}

Base::txBool ISceneLoader::ISceneEventViewer::IActionViewer::initialize(const kvMapVec &ref_kv_map_vec) TX_NOEXCEPT {
  static std::map<txString, txString> map_conversion = {
      {"left_in_lane", "eLeftInLane"},   {"left", "eLeft"},         {"right", "eRight"},
      {"right_in_lane", "eRightInLane"}, {"straight", "eStraight"}, {"none", "eStraight"}};
  mEventActionInfoVec.clear();
  mIsValid = true;
  // 遍历ref_kv_map_vec中的每个kvMap
  for (const auto ref_kv_map : ref_kv_map_vec) {
    // 如果当前kv中包含type
    if (_Contain_(ref_kv_map, "type")) {
      // 从ref_kv_map中获取并设置事件动作的类型
      SceneEventActionType actionType = __lpsz2enum__(SceneEventActionType, ref_kv_map.at("type").c_str());
      Base::txInt actionId = -1;
      // 如果ref_kv_map中包含"actionid"键且其值非空，从ref_kv_map中获取并设置事件动作的ID
      if (_Contain_(ref_kv_map, "actionid") && _NonEmpty_(ref_kv_map.at("actionid"))) {
        actionId = std::atoi(ref_kv_map.at("actionid").c_str());
      }
      /*acc = 0, merge = 1, velocity = 2*/
      // 如果事件动作类型为acc且ref_kv_map中包含"value"键
      if (_plus_(SceneEventActionType::acc) == actionType && _Contain_(ref_kv_map, "value")) {
        const txFloat acc = std::atof(ref_kv_map.at("value").c_str());
        // 将事件动作信息添加到mEventActionInfoVec中
        mEventActionInfoVec.emplace_back(make_EventActionInfo(
            actionId, actionType, _plus_(VehicleMoveLaneState::eStraight), acc, 0.0, _plus_(DrivingStatus::normal)));
      } else if (_plus_(SceneEventActionType::merge) == actionType && _Contain_(ref_kv_map, "value") &&
                 _Contain_(ref_kv_map, "subtype")) {  // 如果事件动作类型为merge且ref_kv_map中包含"value"和"subtype"键
        // 从ref_kv_map中获取并设置车道变换类型
        VehicleMoveLaneState moveType =
            __lpsz2enum__(VehicleMoveLaneState, map_conversion[ref_kv_map.at("subtype")].c_str());
        const txFloat time = std::atof(ref_kv_map.at("value").c_str());
        txFloat laneoffset = 0.0;
        // 如果ref_kv_map中包含"laneoffset"键
        if (_Contain_(ref_kv_map, "laneoffset")) {
          laneoffset = std::atof(ref_kv_map.at("laneoffset").c_str());
        } else if (_Contain_(ref_kv_map, "offset")) {  // 如果ref_kv_map中包含"offset"键
          laneoffset = std::atof(ref_kv_map.at("offset").c_str());
        } else {
          LOG(WARNING) << "merge action do not have laneoffset or offset";
          return false;
        }
        mEventActionInfoVec.emplace_back(
            make_EventActionInfo(actionId, actionType, moveType, time, laneoffset, _plus_(DrivingStatus::normal)));
      } else if (_plus_(SceneEventActionType::velocity) == actionType &&
                 _Contain_(ref_kv_map, "value")) {  // 如果事件动作类型为velocity且ref_kv_map中包含"value"键
        const txFloat velocity = std::atof(ref_kv_map.at("value").c_str());
        mEventActionInfoVec.emplace_back(make_EventActionInfo(actionId, actionType,
                                                              _plus_(VehicleMoveLaneState::eStraight), velocity, 0.0,
                                                              _plus_(DrivingStatus::normal)));
      } else if (
          _plus_(SceneEventActionType::crash_stop_renew) == actionType && _Contain_(ref_kv_map, "heading") &&
          _Contain_(ref_kv_map, "velocity") &&
          _Contain_(
              ref_kv_map,
              "status")) {  // 如果事件动作类型为crash_stop_renew且ref_kv_map中包含"heading"、"velocity"和"status"键
        // 从ref_kv_map中获取并设置航向角
        const txFloat heading = std::atof(ref_kv_map.at("heading").c_str());
        const txFloat velocity = std::atof(ref_kv_map.at("velocity").c_str());
        // 从ref_kv_map中获取并设置驾驶状态
        const DrivingStatus status = __lpsz2enum__(DrivingStatus, ref_kv_map.at("status").c_str());
        mEventActionInfoVec.emplace_back(make_EventActionInfo(
            actionId, actionType, _plus_(VehicleMoveLaneState::eStraight), heading, velocity, status));
      } else if (_plus_(SceneEventActionType::lateralDistance) == actionType &&
                 _Contain_(ref_kv_map, "target_element") && _Contain_(ref_kv_map, "distance") &&
                 _Contain_(ref_kv_map, "freespace") && _Contain_(ref_kv_map, "continuous") &&
                 _Contain_(ref_kv_map, "maxAcc") && _Contain_(ref_kv_map, "maxDec") &&
                 _Contain_(ref_kv_map, "maxSpeed")) {  // 如果事件动作类型为lateralDistance且ref_kv_map中包含目标key
        // 设置距离
        const txFloat distance = std::atof(ref_kv_map.at("distance").c_str());
        // 设置最大加速度
        const txFloat maxAcc = std::atof(ref_kv_map.at("maxAcc").c_str());
        // 设置最大减速度
        const txFloat maxDec = std::atof(ref_kv_map.at("maxDec").c_str());
        // 设置最大速度
        const txFloat maxSpeed = std::atof(ref_kv_map.at("maxSpeed").c_str());
        const txBool freespace = (txString("true") == ref_kv_map.at("freespace")) ? (true) : (false);
        const txBool continuous = (txString("true") == ref_kv_map.at("continuous")) ? (true) : (false);

        ElementType target_element_type = _plus_(ElementType::Unknown);
        txSysId target_element_id = 0;
        const txString target_element = ref_kv_map.at("target_element");
        std::tie(target_element_type, target_element_id) =
            ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(ref_kv_map.at("target_element"));
        // 如果目标元素类型为Unknown，设置mIsValid为false
        if (_plus_(ElementType::Unknown) == target_element_type) {
          mIsValid = false;
        } else {
          // 将事件动作信息添加到mEventActionInfoVec中
          mEventActionInfoVec.emplace_back(
              make_EventActionInfo_lateralDistance(actionId, actionType, distance, maxAcc, maxDec, maxSpeed, freespace,
                                                   continuous, target_element_type, target_element_id, target_element));
          mIsValid = true;
        }
        return IsValid();
      } else {
        std::ostringstream oss;
        for (const auto &kv : ref_kv_map) {
          oss << "{" << TX_VARS_NAME(key, kv.first) << TX_VARS_NAME(value, kv.second) << "},";
        }
        LOG(WARNING) << "un support action type : " << ref_kv_map.at("type") << ", " << oss.str();
      }
    } else {
      LOG(WARNING) << "do not have type";
      mIsValid = false;
      return IsValid();
    }
  }
  return _NonEmpty_(mEventActionInfoVec);
}

Base::txString ISceneLoader::ISceneEventViewer::IActionViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "action {" << std::endl;
  // 遍历动作列表并输出
  for (const auto &refAct : actionList()) {
    oss << TX_VARS_NAME(actionType, __enum2lpsz__(SceneEventActionType, refAct.action_type()))
        << TX_VARS_NAME(actionSubType, __enum2lpsz__(VehicleMoveLaneState, refAct.merge_type()))
        << TX_VARS_NAME(actionValue, refAct.value())
        << TX_VARS_NAME(Value_Unit, (actionType2Unit(refAct.action_type()))) << std::endl;
  }
  oss << "}" << std::endl;
  return oss.str();
}

Base::txString ISceneLoader::ISceneEventViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_VARS(id()) << std::endl;
  // 遍历条件列表并输出
  for (const auto condition_ptr : conditionVec()) {
    oss << TX_VARS_NAME(condition, ((condition_ptr) ? (condition_ptr->Str()) : ("null"))) << std::endl;
  }
  oss << TX_VARS_NAME(endCondition, ((endCondition()) ? (endCondition()->Str()) : ("null"))) << std::endl;
  oss << TX_VARS_NAME(action, ((action()) ? (action()->Str()) : ("null"))) << std::endl;
  return oss.str();
}

ISceneLoader::ISceneEventViewer::kvMap ISceneLoader::ISceneEventViewer::generateKVMap(txString _str /*, -> :*/)
    TX_NOEXCEPT {
  ISceneLoader::ISceneEventViewer::kvMap retKVMap;
  // 初始化一个字符串向量，用于存储分割后的字符串
  std::vector<Base::txString> results;
  Utils::trim(_str);
  // 使用逗号分割输入字符串_str，并将结果存储在results集合中
  boost::algorithm::split(results, _str, boost::is_any_of(","));
  // 遍历results向量中的每个字符串
  for (Base::txString kv : results) {
    Utils::trim(kv);
    std::vector<Base::txString> k_v;
    // 使用冒号分割字符串kv，并将结果存储在k_v向量中
    boost::algorithm::split(k_v, kv, boost::is_any_of(":"));
    // 如果k_v向量的大小为2，表示字符串kv中包含一个键值对
    if (2 == k_v.size()) {
      txString str_key = k_v[0];
      txString str_value = k_v[1];
      // 去除键和值的前后空格
      Utils::trim(str_key);
      Utils::trim(str_value);
      retKVMap[str_key] = str_value;
    }
  }
  return retKVMap;
}

std::vector<ISceneLoader::ISceneEventViewer::kvMap> ISceneLoader::ISceneEventViewer::generateKVMapVec(
    txString _str /* ; -> , -> : */) TX_NOEXCEPT {
  std::vector<ISceneLoader::ISceneEventViewer::kvMap> retKVMapVec;
  // 去除输入字符串_str的前后空格
  Utils::trim(_str);
  std::vector<Base::txString> results;
  // 使用分号分割输入字符串_str，并将结果存储在results向量中
  boost::algorithm::split(results, _str, boost::is_any_of(";"));
  // 遍历results向量中的每个字符串
  for (Base::txString kv_set : results) {
    // 调用generateKVMap函数，将字符串kv_set转换为键值对映射表
    ISceneLoader::ISceneEventViewer::kvMap cur_kv = generateKVMap(kv_set);
    // 如果生成的键值对映射表非空，将其添加到retKVMapVec中
    if (_NonEmpty_(cur_kv)) {
      retKVMapVec.emplace_back(cur_kv);
    } else {
      // 如果生成的键值对映射表为空，输出警告日志
      LOG(WARNING) << "kv map generate error. " << kv_set;
    }
  }
  return retKVMapVec;
}

TX_NAMESPACE_OPEN(Cfg)

struct SceneEvent1200_Action {
  /*type:velocity,value:2,subtype:none,offset:0.0,multi:{}*/
  Base::txString actionid = "-1";
  Base::txString type;
  Base::txString value;
  Base::txString subtype;
  Base::txString offset;

#if __TX_Mark__("lateralDistance")
  Base::txString target_element;
  Base::txString distance;
  Base::txString freespace;
  Base::txString continuous;
  Base::txString maxAcc;
  Base::txString maxDec;
  Base::txString maxSpeed;
#endif /*__TX_Mark__("lateralDistance")*/
  ISceneLoader::ISceneEventViewer::kvMap getKVMap() const TX_NOEXCEPT {
    // 初始化一个空的键值对映射表
    ISceneLoader::ISceneEventViewer::kvMap ret_v;
    ret_v["actionid"] = actionid;
    ret_v["type"] = type;
    ret_v["value"] = value;
    ret_v["subtype"] = subtype;
    ret_v["offset"] = offset;

    // 如果事件动作类型为lateralDistance，将额外的属性添加到键值对映射表中
#if __TX_Mark__("lateralDistance")
    ret_v["target_element"] = target_element;
    ret_v["distance"] = distance;
    ret_v["freespace"] = freespace;
    ret_v["continuous"] = continuous;
    ret_v["maxAcc"] = maxAcc;
    ret_v["maxDec"] = maxDec;
    ret_v["maxSpeed"] = maxSpeed;
#endif /*__TX_Mark__("lateralDistance")*/
    return ret_v;
  }

  Base::txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{ " << TX_VARS(actionid) << TX_VARS(type) << TX_VARS(value) << TX_VARS(subtype) << TX_VARS(offset)
        << "lateralDistance = {" << TX_VARS(target_element) << TX_VARS(distance) << TX_VARS(freespace)
        << TX_VARS(continuous) << TX_VARS(maxAcc) << TX_VARS(maxDec) << TX_VARS(maxSpeed) << "}"
        << " },";
    return oss.str();
  }
  friend std::ostream &operator<<(std::ostream &os, const SceneEvent1200_Action v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
};

struct SceneEvent1200_Action_Array {
  std::vector<SceneEvent1200_Action> content;
  Base::txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    // 遍历输出node内容
    for (const auto &refNode : content) {
      oss << refNode.Str();
    }
    return oss.str();
  }
  friend std::ostream &operator<<(std::ostream &os, const SceneEvent1200_Action_Array v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SceneEvent1200_Action, actionid, type, value, subtype, offset,

                                                target_element, distance, freespace, continuous, maxAcc, maxDec,
                                                maxSpeed);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SceneEvent1200_Action_Array, content);

TX_NAMESPACE_CLOSE(Cfg)

std::vector<ISceneLoader::ISceneEventViewer::kvMap> ISceneLoader::ISceneEventViewer::generateKVMapVec_Json(
    txString _str /* [{type:velocity,value:2},{}] */) TX_NOEXCEPT {
  std::vector<ISceneLoader::ISceneEventViewer::kvMap> retKVMapVec;
  Utils::trim(_str);
  // 使用正则表达式替换字符串_str中的单词为带双引号的单词
  std::regex ex(R"([\w\-.]+)");
  _str = regex_replace(_str, ex, "\"$&\"");
  // 将字符串_str添加到一个新的JSON字符串中
  txString json_content = R"({"content":)" + _str + "}";
  LOG(INFO) << TX_VARS_NAME(ActionJsonContent, json_content);
  // 解析JSON字符串
  nlohmann::json re_parse_json = nlohmann::json::parse(json_content);

  // 初始化一个SceneEvent1200_Action_Array对象
  Cfg::SceneEvent1200_Action_Array action_node_array;
  Cfg::from_json(re_parse_json, action_node_array);

  // 输出SceneEvent1200_Action_Array对象的信息
  LOG(INFO) << action_node_array;

  // 遍历SceneEvent1200_Action_Array对象中的每个元素
  for (const auto &actionNode : action_node_array.content) {
    // 获取事件动作信息的键值对映射表
    ISceneLoader::ISceneEventViewer::kvMap cur_kv = actionNode.getKVMap();
    retKVMapVec.emplace_back(cur_kv);
  }
  return retKVMapVec;
}

#if __SecenEventVersion_1_2_0_0__

Base::txBool ISceneLoader::ISceneEventViewer::initialize(const txSysId _evId, const kvMap &_condition,
                                                         const kvMapVec &_endcondition,
                                                         const kvMapVec &_action) TX_NOEXCEPT {
  EventParam_t params;
  params.conditionKVMap = _condition;
  params.endConditionKVMapVec = _endcondition;
  params.actionKVMapVec = _action;
  return initialize(_evId, params);
}

Base::txBool ISceneLoader::ISceneEventViewer::initialize(const txSysId _evId,
                                                         const EventParam_t &_ev_param) TX_NOEXCEPT {
  if (_ev_param.IsValid()) {
    m_event_id = _evId;

    // 初始化事件条件、结束条件和动作
    if (CallSucc(initialize_condition(_ev_param.conditionKVMap)) &&
        CallSucc(initialize_end_condition(_ev_param.endConditionKVMapVec)) &&
        CallSucc(initialize_action(_ev_param.actionKVMapVec))) {
      // 如果初始化成功，输出日志
      LOG(INFO) << "Event " << _evId << " create success.";
    } else {
      LOG(WARNING) << "Event " << _evId << " create failure.";
    }
  } else {
    // 如果事件参数无效，输出警告日志
    LOG(WARNING) << "Event " << _evId << " create failure. parameter is invalid.";
  }
  return IsInited();
}

Base::txBool ISceneLoader::ISceneEventViewer::initialize(const txSysId _evId,
                                                         const EventGroupParam_t &_ev_group_param) TX_NOEXCEPT {
  // 如果事件参数有效
  if (_ev_group_param.IsValid()) {
    m_event_id = _evId;
    // 遍历事件组条件，并逐个初始化
    for (Base::txInt idx = 0; idx < _ev_group_param.conditionNumber; ++idx) {
      // 如果初始化条件失败，返回false
      if (CallFail(initialize_condition(_ev_group_param.groupConditionVec.at(idx)))) {
        return false;
      }
    }
    // 初始化事件结束条件和动作
    if (CallSucc(initialize_end_condition(_ev_group_param.endConditionKVMapVec)) &&
        CallSucc(initialize_action(_ev_group_param.actionKVMapVec))) {
      // 如果初始化成功，输出日志
      LOG(INFO) << "Event " << _evId << " create success. condition group.";
    } else {
      LOG(WARNING) << "Event " << _evId << " create failure. condition group.";
    }
    return IsInited();
  } else {
    LOG(WARNING) << "Event " << _evId << " create failure. condition group, parameter is invalid. ";
    return false;
  }
}

ISceneLoader::ISceneEventViewer::IConditionViewer::SceneEventType ISceneLoader::ISceneEventViewer::conditionType() const
    TX_NOEXCEPT {
  // 如果事件条件非空
  if (_NonEmpty_(mCondPtrVec)) {
    // 如果事件版本为V_1_2_0_0
    if (eSceneEventVersion::V_1_2_0_0 == version()) {
      return _plus_(IConditionViewer::SceneEventType::conditionGroup);
    } else {
      // 如果事件条件指针非空，返回条件类型，否则返回none类型
      return ((NonNull_Pointer(mCondPtrVec.front())) ? (mCondPtrVec.front()->conditionType())
                                                     : (_plus_(IConditionViewer::SceneEventType::none)));
    }
  } else {
    return _plus_(IConditionViewer::SceneEventType::none);
  }
}

Base::txBool ISceneLoader::ISceneEventViewer::initialize_condition(const kvMap &_condition) TX_NOEXCEPT {
  // 如果条件中包含类型信息
  if (_Contain_(_condition, "type")) {
#  define _CreateCondition_(__type__)                           \
    mCondPtrVec.emplace_back(std::make_shared<__type__>());     \
    if (CallFail(mCondPtrVec.back()->initialize(_condition))) { \
      mCondPtrVec.erase(mCondPtrVec.end() - 1);                 \
    }

    // 获取条件类型
    IConditionViewer::SceneEventType conditionType = __lpsz2enum__(SceneEventType, _condition.at("type").c_str());

    // 根据条件类型创建相应的条件对象
    if (_plus_(IConditionViewer::SceneEventType::velocity_trigger) == conditionType) {
      _CreateCondition_(IVelocityTriggerConditionViewer);
    } else if (_plus_(IConditionViewer::SceneEventType::reach_abs_position) == conditionType) {
      _CreateCondition_(IReachAbsPositionConditionViewer);
    } else if (_plus_(IConditionViewer::SceneEventType::reach_abs_lane) == conditionType) {
      _CreateCondition_(IReachAbsLaneConditionViewer);
    } else if (_plus_(IConditionViewer::SceneEventType::time_trigger) == conditionType) {
      _CreateCondition_(ITimeTriggerViewer);
    } else if (_plus_(IConditionViewer::SceneEventType::ttc_trigger) == conditionType) {
      _CreateCondition_(ITTCTriggerViewer);
    } else if (_plus_(IConditionViewer::SceneEventType::distance_trigger) == conditionType) {
      _CreateCondition_(IDistanceTriggerViewer);
    } else if (_plus_(IConditionViewer::SceneEventType::ego_attach_laneid_custom) == conditionType) {
      _CreateCondition_(IEgoAttachLaneUidCustomViewer);
    } else if (_plus_(IConditionViewer::SceneEventType::timeheadway_trigger) == conditionType) {
      _CreateCondition_(ITimeHeadwayTriggerViewer);
    } else if (_plus_(IConditionViewer::SceneEventType::element_state) == conditionType) {
      _CreateCondition_(IElementStateTriggerViewer);
    } else {
      LOG(WARNING) << "un support condition : " << _condition.at("type");
      clear();
      return false;
    }
#  undef _CreateCondition_
    return true;
  } else {
    return false;
  }
}

Base::txBool ISceneLoader::ISceneEventViewer::initialize_end_condition(const kvMapVec &_endcondition) TX_NOEXCEPT {
  mEndConditionPtr = std::make_shared<IEndConditionViewer>();
  // 如果事件结束条件初始化失败，清空结束条件指针并返回false
  if (CallFail(mEndConditionPtr->initialize(_endcondition))) {
    mEndConditionPtr = nullptr;
    return false;
  } else {
    return true;
  }
}

Base::txBool ISceneLoader::ISceneEventViewer::initialize_action(const kvMapVec &_action) TX_NOEXCEPT {
  mActionPtr = std::make_shared<IActionViewer>();
  // 如果事件动作初始化失败，清空动作指针并返回false
  if (CallFail(mActionPtr->initialize(_action))) {
    mActionPtr = nullptr;
    return false;
  } else {
    return true;
  }
}

#endif /*__SecenEventVersion_1_2_0_0__*/

#if __TX_Mark__("vehicle_measurements")
Base::txBool ISceneLoader::GetVehicleGeometory(const Base::txSysId egoId, const Base::Enums::EgoSubType egoType,
                                               sim_msg::VehicleGeometory &refVehGeom) const TX_NOEXCEPT {
  // 如果车辆几何信息映射中包含指定的egoId
  if (_Contain_(_id2VehGeomMap, egoId)) {
    const auto &refVehicleGeometoryList = _id2VehGeomMap.at(egoId);
    // 如果ego类型为领导车辆
    if (_plus_(Base::Enums::EgoSubType::eLeader) == egoType) {
      refVehGeom.CopyFrom(refVehicleGeometoryList.front());
      return true;
    } else if (_plus_(Base::Enums::EgoSubType::eFollower) == egoType) {  // 如果ego类型为跟随车辆
      // 如果跟随车辆的车队中有车辆
      if (refVehicleGeometoryList.trailer_size() > 0) {
        // 将跟随车辆的几何信息复制到refVehGeom
        refVehGeom.CopyFrom(refVehicleGeometoryList.trailer(0));
        return true;
      } else {
        LOG(WARNING) << "do not have trailer" << TX_VARS(egoId);
        return false;
      }
    } else {
      LOG(WARNING) << "unknow ego type " << TX_VARS(egoType._to_string());
      return false;
    }
  } else {
    LOG(WARNING) << "do not have ego geom info. " << TX_VARS(egoId);
    return false;
  }
}

void ISceneLoader::SetVehicleGeometory(const Base::txSysId egoId,
                                       const sim_msg::VehicleGeometoryList &refVehGeomList) TX_NOEXCEPT {
  _id2VehGeomMap[egoId].CopyFrom(refVehGeomList);
}

#endif /*__TX_Mark__("vehicle_measurements")*/

#if __TX_Mark__("ITimeHeadwayTriggerViewer")

Base::txBool ISceneLoader::ISceneEventViewer::ITimeHeadwayTriggerViewer::initialize(const kvMap &ref_kv_map)
    TX_NOEXCEPT {
  // 如果条件中包含所需的键值对
  if (_Contain_(ref_kv_map, "value") && _Contain_(ref_kv_map, "equation_op") && _Contain_(ref_kv_map, "freespace") &&
      _Contain_(ref_kv_map, "alongroute") && _Contain_(ref_kv_map, "condition_boundary") &&
      _Contain_(ref_kv_map, "target_element") && _Contain_(ref_kv_map, "trigger_count")) {
    try {
      // 获取时间头距阈值
      m_time_head_way_threshold = std::atof(ref_kv_map.at("value").c_str());
      mEquOp = __lpsz2enum__(ConditionEquationOp, ref_kv_map.at("equation_op").c_str());
      m_freespace = (Base::txString("true") == ref_kv_map.at("freespace")) ? (true) : (false);
      m_alongroute = (Base::txString("true") == ref_kv_map.at("alongroute")) ? (true) : (false);
      // 获取条件边界类型
      mBoundaryType = __lpsz2enum__(ConditionBoundaryType, ref_kv_map.at("condition_boundary").c_str());
      m_trigger_count = std::atol(ref_kv_map.at("trigger_count").c_str());

      // 获取目标元素类型和ID
      std::tie(m_target_element_type, m_target_element_id) = splite_type_id(ref_kv_map.at("target_element"));
      if (_plus_(ElementType::Unknown) == m_target_element_type) {
        mIsValid = false;
      } else {
        mIsValid = true;
      }

      // 如果条件中包含源元素信息，获取源元素类型和ID
      if (_Contain_(ref_kv_map, "source_element")) {
        std::tie(m_source_element_type, m_source_element_id) = splite_type_id(ref_kv_map.at("source_element"));
      } else {
        TX_MARK("sim event");
      }
    } catch (...) {
      mIsValid = false;
    }
  } else {
    // 如果条件中不包含所需的键值对，设置条件无效
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "value")) << TX_COND(_Contain_(ref_kv_map, "equation_op"))
                 << TX_COND(_Contain_(ref_kv_map, "freespace")) << TX_COND(_Contain_(ref_kv_map, "alongroute"))
                 << TX_COND(_Contain_(ref_kv_map, "target_element"))
                 << TX_COND(_Contain_(ref_kv_map, "condition_boundary"))
                 << TX_COND(_Contain_(ref_kv_map, "trigger_count")) << TX_COND(_Contain_(ref_kv_map, "source_element"));
  }
  return IsValid();
}

Base::txString ISceneLoader::ISceneEventViewer::ITimeHeadwayTriggerViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(time_head_way_threshold, time_head_way_threshold())
      << TX_VARS_NAME(equation_op, __enum2lpsz__(ConditionEquationOp, op())) << TX_COND_NAME(freespace, freespace())
      << TX_COND_NAME(alongroute, alongroute())
      << TX_VARS_NAME(boundaryType, __enum2lpsz__(ConditionBoundaryType, boundaryType()))
      << TX_VARS_NAME(trigger_count, trigger_count())

      << TX_VARS_NAME(target_type, __enum2lpsz__(ElementType, target_type())) << TX_VARS_NAME(target_id, target_id())

      << TX_VARS_NAME(source_type, __enum2lpsz__(ElementType, source_type())) << TX_VARS_NAME(source_id, source_id())

      << "}" << std::endl;
  return oss.str();
}
#endif /*__TX_Mark__("ITimeHeadwayTriggerViewer")*/

#if __TX_Mark__("IElementStateTriggerViewer")
Base::txString ISceneLoader::ISceneEventViewer::IElementStateTriggerViewer::Str() const TX_NOEXCEPT {
  /*
   virtual txSysId dst_event_id() const TX_NOEXCEPT { return m_dst_event_id; }
          virtual txInt dst_event_action_id() const TX_NOEXCEPT { return m_dst_event_action_id; }
          virtual SceneEventElementStatusType state() const TX_NOEXCEPT { return mState; }
  */
  std::ostringstream oss;
  oss << "conditon {" << TX_VARS_NAME(type, __enum2lpsz__(SceneEventType, conditionType()))
      << TX_VARS_NAME(dst_event_id, dst_event_id()) << TX_VARS_NAME(dst_event_action_id, dst_event_action_id())
      << TX_VARS_NAME(state, __enum2lpsz__(SceneEventElementStatusType, state())) << "}" << std::endl;
  return oss.str();
}

Base::txBool ISceneLoader::ISceneEventViewer::IElementStateTriggerViewer::initialize(const kvMap &ref_kv_map)
    TX_NOEXCEPT {
  // 如果条件中包含所需的键值对
  if (_Contain_(ref_kv_map, "element_type") && _Contain_(ref_kv_map, "element_ref") && _Contain_(ref_kv_map, "state")) {
    try {
      // 获取元素状态
      mState = __lpsz2enum__(SceneEventElementStatusType, ref_kv_map.at("state").c_str());
      // 获取目标事件ID和动作ID
      m_dst_event_id_action_id = ref_kv_map.at("element_ref");
      auto str_kv = Utils::SpliteStringVector(m_dst_event_id_action_id, "_");

      // 如果目标事件ID和动作ID格式正确
      if (2 == str_kv.size()) {
        m_dst_event_id = std::atoi(str_kv[0].c_str());
        m_dst_event_action_id = std::atoi(str_kv[1].c_str());
        mIsValid = true;
      } else {
        // 如果目标事件ID和动作ID格式错误，设置条件无效
        mIsValid = false;
      }
    } catch (...) {
      mIsValid = false;
    }
  } else {
    // 如果条件中不包含所需的键值对，设置条件无效
    mIsValid = false;
    LOG(WARNING) << TX_COND(_Contain_(ref_kv_map, "element_type")) << TX_COND(_Contain_(ref_kv_map, "element_ref"))
                 << TX_COND(_Contain_(ref_kv_map, "state"));
  }
  return IsValid();
}

#endif /*__TX_Mark__("IElementStateTriggerViewer")*/

#if USE_EgoGroup
void ISceneLoader::SetVehicleGeometory(const sim_msg::Scene &refSceneDesc) TX_NOEXCEPT {
  if (refSceneDesc.egos().size() > 0) {
    for (auto &refEgo : refSceneDesc.egos()) {
      Base::Enums::EgoSubType egoSubType;
      if (refEgo.physicles().size() == 1) {
        egoSubType = Base::Enums::EgoSubType::eLeader;
        _group2VehGeomMap[__strcat__(refEgo.group(), egoSubType)].CopyFrom(refEgo.physicles()[0].geometory());
      } else if (refEgo.physicles().size() == 2) {
        _group2VehGeomMap[__strcat__(refEgo.group(), egoSubType)].CopyFrom(refEgo.physicles()[0].geometory());
        egoSubType = Base::Enums::EgoSubType::eFollower;
        _group2VehGeomMap[__strcat__(refEgo.group(), egoSubType)].CopyFrom(refEgo.physicles()[1].geometory());
      } else {
        LOG(WARNING) << "helper.scene_pb()  get egos geometory error." << TX_VARS(refEgo.id())
                     << TX_VARS(refEgo.group()) << TX_VARS(refEgo.physicles().size());
      }
    }
  } else {
    LOG(WARNING) << "helper.scene_pb() get egos geometory error.";
  }
}

Base::txBool ISceneLoader::GetVehicleGeometory(const Base::txString egoGroup, const Base::Enums::EgoSubType egoType,
                                               sim_msg::VehicleGeometory &refVehGeom) const TX_NOEXCEPT {
  const Base::txString groupFill = __strcat__(egoGroup, egoType);
  if (_Contain_(_group2VehGeomMap, groupFill)) {
    refVehGeom.CopyFrom(_group2VehGeomMap.at(groupFill));
    return true;
  } else {
    LOG(WARNING) << "do not have ego geom info. " << TX_VARS(groupFill);
    return false;
  }
}

#endif /*USE_EgoGroup*/
TX_NAMESPACE_CLOSE(Base)
