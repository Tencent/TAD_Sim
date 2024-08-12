// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_event_utils.h"
#include "tx_collision_detection2d.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(Utils)

DistanceProjectionType DistType2ProjType(const ConditionDistanceType _dist_type,
                                         Base::ISceneLoaderPtr scene_loader_ptr) TX_NOEXCEPT {
  if (NonNull_Pointer(scene_loader_ptr)) {
    // 如果_dist_type等于Base::Enums::ConditionDistanceType::laneprojection
    if (_plus_(Base::Enums::ConditionDistanceType::laneprojection) == _dist_type) {
      // 返回DistanceProjectionType::Lane
      return _plus_(DistanceProjectionType::Lane);
    } else {
      return _plus_(DistanceProjectionType::Euclidean);
    }
#if 0
    if (Base::ISceneLoader::eSceneEventVersion::V_1_0_0_0 == scene_loader_ptr->SceneEventVersion()) {
      if (_plus_(Base::Enums::ConditionDistanceType::laneprojection) == _dist_type) {
        return _plus_(DistanceProjectionType::Lane);
      } else {
        return _plus_(DistanceProjectionType::Euclidean);
      }
    } else if (Base::ISceneLoader::eSceneEventVersion::V_1_1_0_0 == scene_loader_ptr->SceneEventVersion()) {
      if (_plus_(Base::Enums::ConditionDistanceType::laneprojection) == _dist_type) {
        return _plus_(DistanceProjectionType::Lane);
      } else {
        return _plus_(DistanceProjectionType::Euclidean);
      }
    } else {
      return _plus_(DistanceProjectionType::Euclidean);
    }
#endif
  } else {
    return _plus_(DistanceProjectionType::Euclidean);
  }
#if 0
  /*Euclidean, Lane, Frenet*/
  /*euclideandistance = 0, laneprojection = 1, lanecoordinate*/
  if (_plus_(Base::Enums::ConditionDistanceType::lanecoordinate) == _dist_type) {
    return _plus_(DistanceProjectionType::Frenet);
  } else if (_plus_(Base::Enums::ConditionDistanceType::laneprojection) == _dist_type) {
    return _plus_(DistanceProjectionType::Lane);
  } else {
    return _plus_(DistanceProjectionType::Euclidean);
  }
#endif
}

Base::txBool IsValidProj(Base::ITrafficElementPtr src_ptr, Base::ITrafficElementPtr target_ptr,
                         const DistanceProjectionType projType) TX_NOEXCEPT {
  // 如果src_ptr和target_ptr都不为空
  if (NonNull_Pointer(src_ptr) && NonNull_Pointer(target_ptr)) {
    // 如果projType等于DistanceProjectionType::Frenet
    if (_plus_(DistanceProjectionType::Frenet) == projType) {
#if 1
      // 获取src_ptr的当前道路信息
      const Base::Info_Road_t& src_roadInfo = src_ptr->GetCurrentRoadInfo();
      // 获取target_ptr的当前道路信息
      const Base::Info_Road_t& target_roadInfo = target_ptr->GetCurrentRoadInfo();
      return (src_roadInfo == target_roadInfo);
#else
      const Base::Info_Lane_t& src_laneInfo = src_ptr->GetCurrentLaneInfo();
      const Base::Info_Lane_t& target_laneInfo = target_ptr->GetCurrentLaneInfo();
      if (src_laneInfo.IsOnLane() == target_laneInfo.IsOnLane()) {
        if (CallSucc(src_laneInfo.IsOnLane())) {
          /*src & target on road*/
          return (src_laneInfo.onLaneUid.roadId == target_laneInfo.onLaneUid.roadId);
        } else {
          /*src & target on link*/
          return (src_laneInfo.onLinkFromLaneUid.roadId == target_laneInfo.onLinkFromLaneUid.roadId) &&
                 (src_laneInfo.onLinkToLaneUid.roadId == target_laneInfo.onLinkToLaneUid.roadId);
        }
      } else {
        /*source & target do not on same road or link*/
        return false;
      }
#endif
    } else {
      /*euclideandistance or laneprojection */
      return true;
    }
  } else {
    /*data in valid*/
    return false;
  }
}

Base::ITrafficElementPtr QueryTrafficElementPtr(const ElementType _type, const Base::txSysId _elemId,
                                                Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT {
  // 获取指定类型的交通元素指针列表
  std::vector<Base::ITrafficElementPtr>& refTypeTrafficPtrVec = _elemMgrPtr->SearchElementByType(_type);
  // 遍历交通元素指针列表
  for (auto elemPtr : refTypeTrafficPtrVec) {
    // 如果找到与指定ID匹配的交通元素，返回该元素的指针
    if (_elemId == (elemPtr->Id())) {
      return elemPtr;
    }
  }
  return nullptr;
}

Base::txBool EquationOperator(const ConditionEquationOp _op, const Base::txFloat left_v,
                              const Base::txFloat right_v) TX_NOEXCEPT {
  // == 运算符处理
  if (_plus_(ConditionEquationOp::eq) == _op) {
    LogInfo << TX_COND(Math::isZero(left_v - right_v, 0.1));
    return (Math::isZero(left_v - right_v, 0.1));
  } else if (_plus_(ConditionEquationOp::gt) == _op) {  // > 运算符处理
    LogInfo << TX_COND((left_v > right_v));
    return (left_v > right_v);
  } else if (_plus_(ConditionEquationOp::gte) == _op) {  // >= 运算符处理
    LogInfo << TX_COND((left_v >= right_v));
    return (left_v >= right_v);
  } else if (_plus_(ConditionEquationOp::lt) == _op) {  // < 运算符处理
    LogInfo << TX_COND((left_v < right_v));
    return (left_v < right_v);
  } else if (_plus_(ConditionEquationOp::lte) == _op) {  // <= 运算符处理
    LogInfo << TX_COND((left_v <= right_v));
    return (left_v <= right_v);
  } else {
    return false;
  }
}

Base::txBool BoundaryOperator(const ConditionBoundaryType _boundary,
                              const boost::optional<std::list<Base::txFloat> > opLastValue, const Base::txFloat value,
                              const Base::txFloat _threshold) TX_NOEXCEPT {
  // 如果上一次的值存在
  if (opLastValue) {
    const std::list<Base::txFloat> lastValue = *opLastValue;
    // 如果边界条件为无
    if (_plus_(ConditionBoundaryType::none) == _boundary) {
      return true;
    } else if (_plus_(ConditionBoundaryType::rising) == _boundary) {  // 如果边界条件为上升
      LogInfo << TX_VARS(lastValue.back()) << TX_VARS(_threshold) << TX_VARS(value)
              << TX_COND((lastValue.back() < _threshold)) << TX_COND((_threshold < value));
      // 比较上一次的值是否小于阈值，以及阈值是否小于当前值，如果满足条件则返回true，否则返回false
      return (lastValue.back() < _threshold) && (_threshold < value);
    } else if (_plus_(ConditionBoundaryType::falling) == _boundary) {  // 如果边界条件为下降
      LogInfo << TX_VARS(lastValue.back()) << TX_VARS(_threshold) << TX_VARS(value)
              << TX_COND((lastValue.back() > _threshold)) << TX_COND((_threshold > value));
      // 比较上一次的值是否大于阈值，以及阈值是否大于当前值，如果满足条件则返回true，否则返回false
      return (lastValue.back() > _threshold) && (_threshold > value);
    } else if (_plus_(ConditionBoundaryType::both) == _boundary) {  // 如果边界条件为上升和下降
      LogInfo << TX_VARS(lastValue.back()) << TX_VARS(_threshold) << TX_VARS(value)
              << TX_COND((lastValue.back() < _threshold)) << TX_COND((_threshold < value))
              << TX_COND((lastValue.back() > _threshold)) << TX_COND((_threshold > value));
      // 比较上一次的值是否小于阈值，以及阈值是否小于当前值，或者上一次的值是否大于阈值，以及阈值是否大于当前值，如果满足条件则返回true，否则返回false
      return ((lastValue.back() < _threshold) && (_threshold < value)) ||
             ((lastValue.back() > _threshold) && (_threshold > value));
    } else {
      return false;
    }
  } else {
    if (_plus_(ConditionBoundaryType::none) == _boundary) {
      return true;
    } else {
      return false;
    }
  }
}

Base::txFloat Compute_EGO_Distance(Base::ITrafficElementPtr target_ptr, Base::ITrafficElementPtr source_ptr,
                                   const DistanceProjectionType projType) TX_NOEXCEPT {
  using namespace Geometry::CD2D;
  // 获取目标元素和原始元素的投影信息
  const auto targetpolygon = target_ptr->GetPolygon();
  const auto sourcepolygon = source_ptr->GetPolygon();
#if 0
  std::ostringstream oss_target, oss_source;
  for (const auto pt : targetpolygon) {
    // Coord::txWGS84 cur_pt; cur_pt.FromENU(pt.x(), pt.y());
    oss_target << Utils::ToString(pt);
  }
  for (const auto pt : sourcepolygon) {
    oss_source << Utils::ToString(pt);
  }
  LOG(INFO) << "[DEBUG]" << TX_VARS_NAME(target, oss_target.str()) << TX_VARS_NAME(source, oss_source.str());
#endif
  // 如果目标物体和源物体的多边形相交，返回0.0
  if (GJK::HitPolygons(target_ptr->GetPolygon(), source_ptr->GetPolygon())) {
    return 0.0;
  } else {  // 如果目标物体和源物体的多边形不相交
    // 如果距离投影类型为车道
    if ((_plus_(DistanceProjectionType::Lane)) == projType) {
      // 计算目标物体和源物体之间的距离投影
      TX_MARK("dist > 0 means obs is front of ego, dist < 0.0 means obs is behind of ego");
      return target_ptr->ComputeProjectDistance(target_ptr->GetPolygon(), source_ptr->GetPolygon());
    } else if ((_plus_(DistanceProjectionType::Frenet)) == projType) {  // 如果距离投影类型为弗雷涅特
      // 计算源物体在目标物体上的投影距离
      const Base::ILocalCoord::FrenetProjInfo projInfo =
          source_ptr->ComputeRoadProjectDistance(target_ptr->StableGeomCenter());
      if (projInfo.mIsValid) {
        return projInfo.mProjDistance;
      } else {
        return FLT_MAX;
      }
    } else {  // 如果距离投影类型不是车道或弗雷涅特，计算目标物体和源物体之间的最小距离
      Base::txVec2 nouse_intersectPt1, nouse_intersectPt2;
      return smallest_dist_between_ego_and_obsVehicle(target_ptr->GetPolygon(), source_ptr->GetPolygon(),
                                                      nouse_intersectPt1, nouse_intersectPt2);
    }
  }
}

Base::txFloat Compute_TTC_Distance(Base::ITrafficElementPtr target_ptr, Base::ITrafficElementPtr source_ptr,
                                   const DistanceProjectionType projType) TX_NOEXCEPT {
  TX_MARK("dist > 0 means obs is front of ego, dist < 0.0 means obs is behind of ego");
  Base::txFloat ttc = FLT_MAX;
  Base::txFloat distance = 0.0, sign = 0.0;
  // 如果距离投影类型为弗雷涅特
  if (_plus_(DistanceProjectionType::Frenet) == projType) {
    // 计算源物体在目标物体上的投影距离
    const Base::ILocalCoord::FrenetProjInfo projInfo =
        source_ptr->ComputeRoadProjectDistance(target_ptr->StableGeomCenter());
    // 如果投影距离有效
    if (CallSucc(projInfo.mIsValid)) {
      distance = projInfo.mProjDistance;
      // 设置符号为目标物体与源物体的空间关系
      sign = __enum2int__(Base::Enums::Element_Spatial_Relationship, projInfo.mTarget2Source);
    } else {  // 如果投影距离无效
      LogWarn << "ComputeRoadProjectDistance failure. " << TX_VARS(source_ptr->Id()) << TX_VARS(target_ptr->Id());
      return ttc;
    }
  } else {  // 如果距离投影类型不是弗雷涅特
    // 计算目标物体和源物体之间的距离
    distance = Compute_EGO_Distance(target_ptr, source_ptr, projType);
    sign = Compute_EGO_Distance(target_ptr, source_ptr, DistanceProjectionType::Lane);
  }

  const Base::txFloat egoVelocity = target_ptr->GetRawVelocity();
  const Base::txFloat vehicleVelocity = source_ptr->GetRawVelocity();
  LOG(INFO) << TX_VARS_NAME(srcId, source_ptr->Id()) << TX_VARS_NAME(targetId, target_ptr->Id())
            << TX_VARS_NAME(relationship, ((sign > 0.0) ? ("src front of target")
                                                        : ((sign < 0.0) ? ("src behind of target") : ("overlap"))))
            << TX_VARS_NAME(dist, std::fabs(distance)) << TX_VARS_NAME(src_v, vehicleVelocity)
            << TX_VARS_NAME(target_v, egoVelocity);

  if (sign > 0.0) {
    LogInfo << "[TTC_Distance] on lane projection, vehicle is front of ego." << _StreamPrecision_ << TX_VARS(sign);
    TX_MARK("on laneprojection, vehicle is front of ego.");
    // 如果目标物体和源物体的速度差不为0
    if (!Math::isZero(egoVelocity - vehicleVelocity, 0.1)) {
      // 如果目标物体的速度大于源物体的速度
      if (egoVelocity > vehicleVelocity) {
        ttc = std::fabs(distance) / (egoVelocity - vehicleVelocity);
      } else {  // 如果目标物体的速度小于等于源物体的速度
        TX_MARK("impossible collision");
      }
    } else {  // 如果目标物体和源物体的速度差为0
      TX_MARK("impossible collision");
    }
  } else if (sign < 0.0) {
    LogInfo << "[TTC_Distance] on lane projection, vehicle is behind of ego." << _StreamPrecision_ << TX_VARS(sign);
    TX_MARK("on laneprojection, vehicle is behand of ego.");
    // 如果目标物体和源物体的速度差不为0
    if (!Math::isZero(egoVelocity - vehicleVelocity, 0.1)) {
      // 如果目标物体的速度大于源物体的速度
      if (vehicleVelocity > egoVelocity) {
        ttc = std::fabs(distance) / (vehicleVelocity - egoVelocity);
      } else {  // 如果目标物体的速度小于等于源物体的速度
        TX_MARK("impossible collision");
      }
    } else {  // 如果目标物体和源物体的速度差为0
      TX_MARK("impossible collision");
    }
  } else {  // 如果符号等于0.0
    TX_MARK("has been collision.");
    LogInfo << "[TTC_Distance] vehicle and ego have collided." << _StreamPrecision_ << TX_VARS(sign);
    // 设置TTC值为0.0
    ttc = 0.0;
  }

  LogInfo << "[TTC_Distance] " << TX_VARS(distance) << TX_VARS(egoVelocity - vehicleVelocity) << TX_VARS(egoVelocity)
          << TX_VARS(vehicleVelocity) << TX_VARS(ttc);
  return std::fabs(ttc);
}

TX_NAMESPACE_CLOSE(Utils)

#undef LogInfo
#undef LogWarn
