// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_event.h"
#include <cfloat>
#include <cmath>
#include "tx_collision_detection2d.h"
#include "tx_marco.h"
#include "tx_math.h"
#include "tx_sim_point.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

// 计算两个实体（ego 和 vehicle）之间的 TTC（Time to Collision）
// 若 ego 和 vehicle 的位置已知，并且我们知道 ego 和 vehicle 的运动速度
// 此函数根据给定的参数、projType 和其他参数计算 TTC
// 返回值为 TTC 的绝对值
Base::txFloat IEvent::Compute_TTC_Distance(const EventTriggerParam_t& egoInfo, const EventHitUtilInfo_t& elementInfo,
                                           const DistanceProjectionType projType) const TX_NOEXCEPT {
  TX_MARK("dist > 0 means obs is front of ego, dist < 0.0 means obs is behind of ego");
  // 计算 ego 和 vehicle 之间的距离，以及ego 和 vehicle 的相对前后位置
  const txFloat distance = Compute_EGO_Distance(egoInfo, elementInfo, projType);
  const txFloat sign = Compute_EGO_Distance(egoInfo, elementInfo, DistanceProjectionType::Lane);

  // 获取 ego 和 vehicle 的速度
  const txFloat egoVelocity = egoInfo._LogicEgoPtr->GetVelocity();
  const txFloat vehicleVelocity = elementInfo._Velocity;
  txFloat ttc = FLT_MAX;

  // 根据 ego 和 vehicle 之间的相对位置，计算TTC
  if (sign > 0.0) {
    LogInfo << "[TTC_Distance] on lane projection, vehicle is front of ego." << _StreamPrecision_ << TX_VARS(sign);
    TX_MARK("on laneprojection, vehicle is front of ego.");
    if (!Math::isEqual(egoVelocity, vehicleVelocity)) {
      if (egoVelocity > vehicleVelocity) {
        ttc = std::fabs(distance) / (egoVelocity - vehicleVelocity);
      } else {
        TX_MARK("impossible collision");
      }
    } else {
      TX_MARK("impossible collision");
    }
  } else if (sign < 0.0) {  // sign<0 交通车在ego前面
    LogInfo << "[TTC_Distance] on lane projection, vehicle is behand of ego." << _StreamPrecision_ << TX_VARS(sign);
    TX_MARK("on laneprojection, vehicle is behand of ego.");
    // 判断速度是否不同
    if (!Math::isEqual(egoVelocity, vehicleVelocity)) {
      // 计算ttc值
      if (vehicleVelocity > egoVelocity) {
        ttc = std::fabs(distance) / (vehicleVelocity - egoVelocity);
      } else {
        TX_MARK("impossible collision");
      }
    } else {
      TX_MARK("impossible collision");
    }
  } else {
    TX_MARK("has been collision.");
    LogInfo << "[TTC_Distance] vehicle and ego have collided." << _StreamPrecision_ << TX_VARS(sign);
    ttc = 0.0;
  }

  LogInfo << "[TTC_Distance] " << TX_VARS(distance) << TX_VARS(egoVelocity - vehicleVelocity) << TX_VARS(egoVelocity)
          << TX_VARS(vehicleVelocity) << TX_VARS(ttc);
  return std::fabs(ttc);
}

// 计算对象之间的投影距离
Base::txFloat IEvent::Compute_EGO_Distance(const EventTriggerParam_t& egoInfo, const EventHitUtilInfo_t& elementInfo,
                                           const DistanceProjectionType projType) const TX_NOEXCEPT {
  using namespace Geometry::CD2D;
  // 检查两个对象是否相交
  if (GJK::HitPolygons(egoInfo._LogicEgoPtr->GetPolygon(), elementInfo._vecPolygon)) {
    return 0.0;
  } else {
    // 若投影类型为 Lane，则返回两个对象的前后位置关系
    if ((_plus_(DistanceProjectionType::Lane)) == projType) {
      TX_MARK("dist > 0 means obs is front of ego, dist < 0.0 means obs is behind of ego");
      return egoInfo._LogicEgoPtr->ComputeProjectDistance(egoInfo._LogicEgoPtr->GetPolygon(), elementInfo._vecPolygon);
    } else {
      // 否则返回计算两个对象之间的距离
      Base::txVec2 nouse_intersectPt1, nouse_intersectPt2;
      return smallest_dist_between_ego_and_obsVehicle(egoInfo._LogicEgoPtr->GetPolygon(), elementInfo._vecPolygon,
                                                      nouse_intersectPt1, nouse_intersectPt2);
    }
  }
}

// 检查参数是否有效
Base::txBool IEvent::IsParamValid(const EventTriggerParam_t& egoParam,
                                  const EventHitUtilInfo_t& elementParam) const TX_NOEXCEPT {
  // 获取自身元素和其他元素的多边形坐标
  const auto& ego_polygon = egoParam._LogicEgoPtr->GetPolygon();
  const auto& obs_polygon = elementParam._vecPolygon;
  // 检查自身元素和其他元素的多边形坐标是否合法
  if ((ego_polygon.size() > 2) && (obs_polygon.size() > 2)) {
    // 遍历自身元素的多边形坐标，检查是否有无效的坐标值
    for (const auto& v : ego_polygon) {
      const auto subType_x = std::fpclassify(v.x());
      if (FP_INFINITE == subType_x || FP_NAN == subType_x || FP_SUBNORMAL == subType_x) {
        LogInfo << "IsParamValid false." << TX_VARS(subType_x);
        return false;
      }
      const auto subType_y = std::fpclassify(v.y());
      if (FP_INFINITE == subType_y || FP_NAN == subType_y || FP_SUBNORMAL == subType_y) {
        LogInfo << "IsParamValid false." << TX_VARS(subType_y);
        return false;
      }
    }
    return true;
  } else {
    LogInfo << "IsParamValid false." << TX_VARS(ego_polygon.size()) << TX_VARS(obs_polygon.size());
    return false;
  }
}

// TTC 事件触发计算
Base::txBool IEvent::HitTTC(const EventTriggerParam_t& egoParam, const EventHitUtilInfo_t& elementParam,
                            const Base::txFloat threshold, EventStateOptional& refState,
                            DistanceProjectionType projType, const txUInt triggerIndex) const TX_NOEXCEPT {
  if (IsParamValid(egoParam, elementParam)) {
    const Base::txFloat dist = Compute_TTC_Distance(egoParam, elementParam, projType);

    // 计算TTC，与阈值比较，记录当前触发事件的状态
    TwoWayEventState curState = TwoWayEventState::twUndefined;
    if (dist < threshold) {
      curState = TwoWayEventState::twInCondition;
    } else if (dist > threshold && dist < (FLT_MAX / 2.0)) {
      curState = TwoWayEventState::twOutCondition;
    } else {
      LogInfo << "[TTC TIME] Invalid ttc time = "
              << ((dist < (FLT_MAX / 2.0)) ? (std::to_string(dist))
                                           : ("FLT_MAX, Invalid ttc time, impossible collision."));
    }

    if (refState) {
      TX_MARK("state has been created by previous step.");
      TwoWayEventState& previousState = std::get<0>(*refState);
      auto& previousStateChangeCount = std::get<1>(*refState);
      // 当前事件和之前的事件状态有效，且不同，准备触发事件
      if (curState != previousState && (TwoWayEventState::twUndefined != curState) &&
          (TwoWayEventState::twUndefined != previousState)) {
        LogInfo << _StreamPrecision_ << " change state from :" << refState << TX_VARS_NAME(TTC_Time, dist)
                << TX_VARS_NAME(TTC_Threshold, threshold);
        // 更新前一事件触发状态，并更新触发次数
        previousState = curState;
        previousStateChangeCount++;
        LogInfo << _StreamPrecision_ << " change state to :" << refState << TX_VARS_NAME(TTC_Time, dist)
                << TX_VARS_NAME(TTC_Threshold, threshold);

        if (triggerIndex == std::get<1>(*refState)) {
          LogInfo << _StreamPrecision_ << " event trigger. " << TX_VARS_NAME(state_change_count, std::get<1>(*refState))
                  << TX_VARS_NAME(TTC_Time, dist) << TX_VARS_NAME(TTC_Threshold, threshold)
                  << TX_VARS_NAME(TTC_triggerIndex, triggerIndex);
          return true;
        } else {
          LogInfo << _StreamPrecision_ << " event donot trigger."
                  << TX_VARS_NAME(state_change_count, std::get<1>(*refState)) << TX_VARS_NAME(TTC_Time, dist)
                  << TX_VARS_NAME(TTC_Threshold, threshold) << TX_VARS_NAME(TTC_triggerIndex, triggerIndex);
          return false;
        }
      } else {
        LogInfo << _StreamPrecision_ << " un-change state : " << refState << TX_VARS(curState) << TX_VARS(previousState)
                << TX_VARS_NAME(TTC_Time, (dist < (FLT_MAX / 2.0))
                                              ? (std::to_string(dist))
                                              : ("FLT_MAX, Invalid ttc time, impossible collision."))
                << TX_VARS_NAME(TTC_Threshold, threshold);
        return false;
      }
    } else {
      TX_MARK("state do not been created.");
      if (TwoWayEventState::twUndefined != curState) {
        refState = std::make_tuple(curState, 0);
        LogInfo << _StreamPrecision_ << " Create two-way trigger condition. "
                << TX_VARS_NAME(state, std::get<0>(*refState))
                << TX_VARS_NAME(TTC_Time, (dist < (FLT_MAX / 2.0))
                                              ? (std::to_string(dist))
                                              : ("FLT_MAX, Invalid ttc time, impossible collision."))
                << TX_VARS_NAME(TTC_Threshold, threshold);
      }
      return false;
    }
  } else {
    LogWarn << "IsParamValid = false.";
    return false;
  }
}

// 距离 事件触发计算
Base::txBool IEvent::HitEgoDistance(const EventTriggerParam_t& egoParam, const EventHitUtilInfo_t& elementParam,
                                    const Base::txFloat threshold, EventStateOptional& refState,
                                    DistanceProjectionType projType, const txUInt triggerIndex) const TX_NOEXCEPT {
  if (IsParamValid(egoParam, elementParam)) {
    // 计算egoParam和elementParam之间的距离
    const Base::txFloat dist = Compute_EGO_Distance(egoParam, elementParam, projType);

    // 根据距离和阈值计算当前是否满足condition，记录状态
    TwoWayEventState curState = TwoWayEventState::twUndefined;
    if (dist < threshold) {
      // 满足condition
      curState = TwoWayEventState::twInCondition;
    } else {
      // 不满足condition
      curState = TwoWayEventState::twOutCondition;
    }

    if (refState) {
      TX_MARK("state has been created by previous step.");
      TwoWayEventState& previousState = std::get<0>(*refState);
      auto& previousStateChangeCount = std::get<1>(*refState);
      // 之前状态与当前状态都有效，且不相同时则更新状态
      if (curState != previousState && (TwoWayEventState::twUndefined != curState) &&
          (TwoWayEventState::twUndefined != previousState)) {
        LogInfo << _StreamPrecision_ << "change state from :" << refState << TX_VARS_NAME(EGO_Dist, dist)
                << TX_VARS_NAME(EGO_Threshold, threshold);
        // 更新之前状态，并累加状态变化次数
        previousState = curState;
        previousStateChangeCount++;
        LogInfo << _StreamPrecision_ << " change state to :" << refState << TX_VARS_NAME(EGO_Dist, dist)
                << TX_VARS_NAME(EGO_Threshold, threshold);

        if (triggerIndex == std::get<1>(*refState)) {
          LogInfo << _StreamPrecision_ << " event trigger. " << TX_VARS_NAME(state_change_count, std::get<1>(*refState))
                  << TX_VARS_NAME(EGO_Dist, dist) << TX_VARS_NAME(EGO_Threshold, threshold)
                  << TX_VARS_NAME(EGO_triggerIndex, triggerIndex);
          return true;
        } else {
          LogInfo << _StreamPrecision_ << " event do not trigger."
                  << TX_VARS_NAME(state_change_count, std::get<1>(*refState)) << TX_VARS_NAME(EGO_Dist, dist)
                  << TX_VARS_NAME(EGO_Threshold, threshold) << TX_VARS_NAME(EGO_triggerIndex, triggerIndex);
          return false;
        }
      } else {
        LogInfo << _StreamPrecision_ << " un-change state : " << refState << TX_VARS_NAME(EGO_Dist, dist)
                << TX_VARS_NAME(EGO_Threshold, threshold);
        return false;
      }
    } else {
      TX_MARK("state do not been created.");
      refState = std::make_tuple(curState, 0);
      LogInfo << _StreamPrecision_ << " Create two-way trigger condition. "
              << TX_VARS_NAME(state, std::get<0>(*refState)) << TX_VARS_NAME(EGO_Dist, dist)
              << TX_VARS_NAME(EGO_Threshold, threshold);
      return false;
    }
  } else {
    LogWarn << "IsParamValid = false.";
    return false;
  }
}

std::ostream& operator<<(std::ostream& os, const IEvent::TwoWayEventState& v) TX_NOEXCEPT {
  using tw = IEvent::TwoWayEventState;
  switch (v) {
    case tw::twInCondition: {
      os << "twInCondition";
      break;
    }
    case tw::twOutCondition: {
      os << "twOutCondition";
      break;
    }
    case tw::twUndefined: {
      os << "twUndefined";
      break;
    }
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const IEvent::EventStateOptional& v) TX_NOEXCEPT {
  if (v) {
    os << "EventStateOptional {" << TX_VARS_NAME(state, std::get<0>(*v))
       << TX_VARS_NAME(state_change_count, std::get<1>(*v)) << "}";
  } else {
    os << "EventStateOptional { none}";
  }

  return os;
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
