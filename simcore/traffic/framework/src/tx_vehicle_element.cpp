// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_vehicle_element.h"
#include "tx_obb.h"
#include "tx_protobuf_utils.h"
#include "tx_spatial_query.h"
#if USE_HashedRoadNetwork
#  include "HdMap/tx_hashed_road.h"
#endif /*USE_HashedRoadNetwork*/
#include "tx_collision_detection2d.h"
#include "tx_geometry_element_line.h"
#define FCW_LogInfo LOG_IF(INFO, FLAGS_LogLevel_FCW)
#define FCW_LogWarn LOG_IF(WARNING, FLAGS_LogLevel_FCW)
TX_NAMESPACE_OPEN(Base)

txBool IVehicleElement::CheckStart(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  // 判断当前元素生命周期是否开始
  if (!IsStart()) {
    // 如果当前时间大于等于车辆生命周期的开始时间
    if (timeMgr.AbsTime() >= mLifeCycle.StartTime()) {
      // 将车辆的生命周期标记为已开始
      mLifeCycle.SetStart();
    }
  }
  return IsStart();
}

txBool IVehicleElement::CheckEnd(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT { return IsEnd(); }

txBool IVehicleElement::Kill() TX_NOEXCEPT {
  StopVehicle();
  return mLifeCycle.Kill();
}

void IVehicleElement::StopVehicle() TX_NOEXCEPT {
  // 设置车辆速度、最大速度和加速度为0
  mKinetics.LastVelocity() = mKinetics.m_velocity = mKinetics.m_velocity_max = 0.0;
  mKinetics.m_acceleration = 0.0;
  mKinetics.m_displacement = 0.0;

  // 根据当前车辆所在的道路车道状态进行相应操作
  switch (GetSwitchLaneState()) {
    case VehicleMoveLaneState::eLeft:
    case VehicleMoveLaneState::eLeftInLane: {
      // 将车辆从左侧道道入口方向驶入
      mLocation.LaneOffset() = mKinetics.m_LateralDisplacement;
      break;
    }
    case VehicleMoveLaneState::eRight:
    case VehicleMoveLaneState::eRightInLane: {
      // 将车辆从右侧道道入口方向驶入
      mLocation.LaneOffset() = (-1.0) * mKinetics.m_LateralDisplacement;
      break;
    }
  }
  // 将车辆生命周期状态标记为停止
  mLifeCycle.SetStop();
  if (FLAGS_Kill_After_Stop || VehicleSource::veh_input == Vehicle_ComeFrom()) {
    mLifeCycle.SetEnd();
  }

  OnEnd();
}

void IVehicleElement::SaveStableState() TX_NOEXCEPT {
  // 保存车辆的稳定状态
  mLocation.LastGeomCenter() = mLocation.GeomCenter().ToENU();
  mLocation.LastRearAxleCenter() = mLocation.RearAxleCenter().ToENU();
  mLocation.LastOnLanePos() = mLocation.PosOnLaneCenterLinePos();
  mLocation.LastLaneDir() = mLocation.vLaneDir();
  mLocation.LastLaneInfo() = mLocation.LaneLocInfo();
  mLocation.LastHeading() = mLocation.heading();
  mKinetics.LastVelocity() = GetVelocity();
  mKinetics.LastAcceleration() = GetAcc();
  mLocation.LastDistanceAlongCurve() = DistanceAlongCurve();
  mLocation.LastInvertDistanceAlongCurve() = InvertDistanceAlongCurve();
  mLastDrivingStatus = mDrivingStatus;
}

void IVehicleElement::ComputeRealAcc(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  mKinetics.m_acceleration = (GetVelocity() - mKinetics.LastVelocity()) / (timeMgr.RelativeTime());
}

sim_msg::Car *IVehicleElement::FillingElement(txFloat const timeStamp, sim_msg::Car *pSimVehicle) TX_NOEXCEPT {
  if (NonNull_Pointer(pSimVehicle)) {
    // 设置车辆当前的状态
    pSimVehicle->set_t(timeStamp);
    pSimVehicle->set_id(mIdentity.Id());
    const hadmap::txPoint geom_center_gps = RawVehicleCoord().WGS84();
    pSimVehicle->set_x(__Lon__(geom_center_gps));
    pSimVehicle->set_y(__Lat__(geom_center_gps));
    pSimVehicle->set_z(Altitude() /*__Alt__(geom_center_gps)*/);
    pSimVehicle->set_loffset(mLocation.CenterLineOffset());

    /*LOG(WARNING) << TX_VARS(Id()) << TX_VARS_NAME(State, GetSwitchLaneState()._to_string())
        << TX_VARS(mLocation.LaneOffset()) << " #### " << TX_VARS(mLocation.CenterLineOffset())
        << TX_VARS(mKinetics.m_LateralVelocity);*/
    // 设置车辆当前的速度
    pSimVehicle->set_v(GetVelocity());
    pSimVehicle->set_vl(mKinetics.m_LateralVelocity);
    // 设置车辆的尺寸
    pSimVehicle->set_length(GetLength());
    pSimVehicle->set_width(GetWidth());
    pSimVehicle->set_height(GetHeigth());
    const Base::Info_Lane_t &curLaneInfo = mLocation.LaneLocInfo();
    // 当在lanlink上时
    if (CallSucc(curLaneInfo.isOnLaneLink)) {
      // 设置车辆当前所在的道路ID
      pSimVehicle->set_tx_road_id(curLaneInfo.onLinkFromLaneUid.roadId);
      // 设置车辆当前所在的道路节点ID
      pSimVehicle->set_tx_section_id(curLaneInfo.onLinkFromLaneUid.sectionId);
      // 设置车辆当前所在的车道ID
      pSimVehicle->set_tx_lane_id(curLaneInfo.onLinkFromLaneUid.laneId);
      // 设置车辆当前所在的道路链接ID
      pSimVehicle->set_tx_lanelink_id(curLaneInfo.onLinkId_without_equal);
      pSimVehicle->set_to_tx_road_id(curLaneInfo.onLinkToLaneUid.roadId);
      pSimVehicle->set_to_tx_section_id(curLaneInfo.onLinkToLaneUid.sectionId);
      pSimVehicle->set_to_tx_lane_id(curLaneInfo.onLinkToLaneUid.laneId);
    } else {
      pSimVehicle->set_tx_road_id(curLaneInfo.onLaneUid.roadId);
      pSimVehicle->set_tx_section_id(curLaneInfo.onLaneUid.sectionId);
      pSimVehicle->set_tx_lane_id(curLaneInfo.onLaneUid.laneId);
      pSimVehicle->set_tx_lanelink_id(LINK_PKID_INVALID);
      pSimVehicle->set_to_tx_road_id(curLaneInfo.onLaneUid.roadId);
      pSimVehicle->set_to_tx_section_id(curLaneInfo.onLaneUid.sectionId);
      pSimVehicle->set_to_tx_lane_id(curLaneInfo.onLaneUid.laneId);
    }
    const Base::txFloat _headingRadian = GetHeadingWithAngle().GetRadian();
    pSimVehicle->set_heading(_headingRadian);
    pSimVehicle->set_theta(_headingRadian);
#if USE_CustomModelImport
    pSimVehicle->set_type(VehicleTypeId());
#else  /*#USE_CustomModelImport*/
    pSimVehicle->set_type(__enum2int__(VEHICLE_TYPE, VehicleType()));
#endif /*#USE_CustomModelImport*/
    pSimVehicle->set_acc(GetAcc());
    pSimVehicle->set_acc_l(GetLateralAcc());
    pSimVehicle->set_angle_along_centerline_degree(mLocation.fAngle().GetDegree());

    pSimVehicle->mutable_fcw()->CopyFrom(mFCW);
    if (FLAGS_Filling_ExternInfo) {
      // 计算车辆外部信息
      ComputeExternInfo(pSimVehicle->mutable_extra_info());
      // 记录并输出外部信息
      if (FLAGS_LogLevel_ExternInfo && pSimVehicle->extra_info().front_veh()) {
        LOG(INFO) << "[front_veh]" << TX_VARS(timeStamp) << TX_VARS(Id())
                  << TX_VARS_NAME(dist2ego, pSimVehicle->extra_info().dist_to_ego_m())
                  << TX_VARS_NAME(veh_speed, pSimVehicle->extra_info().speed_ms());
      } else if (FLAGS_LogLevel_ExternInfo && pSimVehicle->extra_info().rear_veh()) {
        LOG(INFO) << "[rear_veh]" << TX_VARS(timeStamp) << TX_VARS(Id())
                  << TX_VARS_NAME(dist2ego, pSimVehicle->extra_info().dist_to_ego_m())
                  << TX_VARS_NAME(veh_speed, pSimVehicle->extra_info().speed_ms());
      }
    }
  }
  return pSimVehicle;
}

void IVehicleElement::ResetFCW() TX_NOEXCEPT {
  mFCW.clear_state();
  mFCW.clear_euclidean_distance();
  mFCW.clear_lane_proj_distance();
  mFCW.clear_ego_v();
  mFCW.clear_vehicle_v();
  mFCW.set_ttc_euclidean_distance(-1.0);
  mFCW.set_ttc_lane_proj_distance(-1.0);
  mFCW.set_thw_euclidean_distance(-1.0);
  mFCW.set_thw_lane_proj_distance(-1.0);
  mFCW.set_state(sim_msg::No_Relationship);
}

txBool IVehicleElement::FillingElement(Base::TimeParamManager const &timeMgr,
                                       sim_msg::Traffic &trafficData) TX_NOEXCEPT {
  if ((IsAlive())) {
#if 1
    sim_msg::Car *pSimVehicle = trafficData.mutable_cars(IndexInPb());
#else
    sim_msg::Car *pSimVehicle = nullptr;
    {
      tbb::mutex::scoped_lock lock(s_tbbMutex_outputTraffic);
      // LOG(INFO) << TX_VARS(Id()) << TX_COND(IsAlive()) << " trafficData.add_cars ...";
      pSimVehicle = trafficData.add_cars();
      // LOG(INFO) << TX_VARS(Id()) << TX_COND(IsAlive()) << " trafficData.add_cars finish.";
    }
#endif
    pSimVehicle = FillingElement(timeMgr.TimeStamp(), pSimVehicle);
    if (NeedOutputTrajectory()) {
#if 1
      Geometry::Element::txPolylinePtr polylinePtr =
          std::dynamic_pointer_cast<Geometry::Element::txPolyline>(StableTrajectory());
      // 如果polylinePtr指针非空
      if (NonNull_Pointer(polylinePtr)) {
        sim_msg::Trajectory &refTraj = *(pSimVehicle->mutable_planning_line());
        // 清空refTraj
        refTraj.Clear();
        const Base::txSize nPtSize = polylinePtr->SamplingPtSize();
        Base::txFloat cur_s = 0.0;
        // 遍历polylinePtr的采样点
        for (Base::txSize idx = 0; idx < nPtSize; ++idx) {
          // 为refTraj添加一个TrajectoryPoint指针
          sim_msg::TrajectoryPoint *new_traj_point_ptr = refTraj.add_point();
          new_traj_point_ptr->set_z(0.0);
          new_traj_point_ptr->set_t(idx + timeMgr.AbsTime());
          new_traj_point_ptr->set_v(0.0);
          new_traj_point_ptr->set_a(0.0);
          new_traj_point_ptr->set_s(cur_s);

          // 获取polylinePtr的采样点
          const Base::txVec3 &refEnu = polylinePtr->SamplingPt(idx);
          Coord::txWGS84 cur_pt_wgs84;
          // 将refEnu的ENU坐标转换为WGS84坐标
          cur_pt_wgs84.FromENU(refEnu);
          new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
          new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
          new_traj_point_ptr->set_theta(0.0);
        }
      }
#else
      FillingTrajectory(timeMgr, *(pSimVehicle->mutable_planning_line()));
#endif
    }
    // LOG(INFO) << TX_VARS(Id()) << TX_COND(IsAlive()) << " FillingElement finish." <<
    // Utils::ProtobufDebugJson(pSimVehicle);
    /*FillingSpatialQuery();*/
#if __TX_Mark__("simulation consistency")
    SaveStableState();
#endif /*__TX_Mark__("simulation consistency")*/
  } else {
    mGeometryData.SetInValid();
  }
  return true;
}

txBool IVehicleElement::FillingSpatialQuery() TX_NOEXCEPT {
  if (IsAlive() && mGeometryData.IsValid() && CallFail(IsTransparentObject())) {
#if USE_HashedRoadNetwork
    // 更新hash车道信息
    UpdateHashedLaneInfo(mLocation);

#  if _EraseRtree2d_
    const auto &refPolygon = mGeometryData.Polygon();
    Geometry::SpatialQuery::RTree2D::InsertBulk(refPolygon[0], refPolygon[1], refPolygon[2], refPolygon[3],
                                                GeomCenter().ToENU().ENU2D(), GetBasePtr());
#  endif

#else
    const auto &refPolygon = mGeometryData.Polygon();
    Geometry::SpatialQuery::RTree2D::getInstance().Insert(refPolygon[0], refPolygon[1], refPolygon[2], refPolygon[3],
                                                          GetPosition().ToENU().ENU2D(), GetBasePtr());
#endif /*USE_HashedRoadNetwork*/
#if 0
        std::ostringstream oss;
        for (const auto& pt : refPolygon) {
            Coord::txWGS84 tmp_pt; tmp_pt.FromENU(pt[0], pt[1]);
            oss << Utils::ToString(tmp_pt.WGS84());
        }
        LOG(WARNING) << TX_VARS(Id()) << " : " << oss.str();
#endif
    return true;
  } else {
    // LOG(WARNING) << TX_VARS(Id()) << TX_COND(IsAlive()) << TX_COND(mGeometryData.IsValid());
    return false;
  }
}

IVehicleElement::KineticsInfo_t IVehicleElement::GetKineticsInfo(Base::TimeParamManager const &timeMgr) const
    TX_NOEXCEPT {
  KineticsInfo_t retInfo;
  retInfo.m_elemId = Id();
  retInfo.m_compute_velocity = GetVelocity();
  retInfo.m_acc = GetAcc();
  retInfo.m_show_velocity = GetShowVelocity();
  /*LOG(WARNING) << TX_VARS_NAME(m_elemId, retInfo.m_elemId) << TX_VARS_NAME(m_compute_velocity,
     retInfo.m_compute_velocity)
      << TX_VARS_NAME(m_acc, retInfo.m_acc) << TX_VARS_NAME(m_show_velocity, retInfo.m_show_velocity);*/
  return retInfo;
}

Base::txBool IVehicleElement::GetPredictionPos(const Base::txFloat distance, Coord::txENU &prePoint) const TX_NOEXCEPT {
  txAssert(_plus_(ElementType::TAD_Vehicle) == Type());
  // 获取车辆的位置追踪器
  const auto mtracker = mLocation.tracker();
  // 如果位置追踪器和当前曲线有效
  if (NonNull_Pointer(mtracker) && NonNull_Pointer(mtracker->getCurrentCurve())) {
    // 获取车辆在指定距离前的点
    const hadmap::txPoint preTxPoint = mtracker->getCurrentCurve()->getPoint(distance + mLocation.DistanceAlongCurve());
    // 将获取到的点转换为ENU坐标系并赋值给prePoint
    prePoint.FromWGS84(preTxPoint);
    return true;
  } else {
    return false;
  }
}

hadmap::txLaneLinkPtr IVehicleElement::RandomNextLink(const hadmap::txLaneLinks &nextLaneLinks) TX_NOEXCEPT {
  // 如果nextLaneLinks非空
  if (_NonEmpty_(nextLaneLinks)) {
    const auto nRnd = mPRandom.GetRandomInt();
    // 根据随机数选择下一个车道连接并返回
    return nextLaneLinks.at(Math::RangeInt32(nRnd, nextLaneLinks.size()));
  } else {
    return nullptr;
  }
}

hadmap::txLanePtr IVehicleElement::RandomNextLane(const hadmap::txLanes &nextLanes) TX_NOEXCEPT {
  // 如果存在下条车道集合
  if (_NonEmpty_(nextLanes)) {
    const auto nRnd = mPRandom.GetRandomInt();
    // 根据随机数选择下一个车道并返回
    return nextLanes.at(Math::RangeInt32(nRnd, nextLanes.size()));
  } else {
    return nullptr;
  }
}

Base::txBool IVehicleElement::IsTurnRightAtNextIntersection() const TX_NOEXCEPT {
  // 如果在车道上
  if (mLocation.IsOnLane()) {
    return false;
  } else {
    // 如果m_op_self_next_driving_direction有值
    if (m_op_self_next_driving_direction.has_value()) {
      using SelfDrivingDirection = Base::Enums::VehicleInJunction_SelfDrivingDirection;
      // 判断是否右转并返回结果
      return ((_plus_(SelfDrivingDirection::eRightSide)) == (*m_op_self_next_driving_direction));
    } else {
      return false;
    }
  }
}

Base::txBool IVehicleElement::IsTurnLeftAtNextIntersection() const TX_NOEXCEPT {
  // 在车道上返回false
  if (mLocation.IsOnLane()) {
    return false;
  } else {
    if (m_op_self_next_driving_direction.has_value()) {
      using SelfDrivingDirection = Base::Enums::VehicleInJunction_SelfDrivingDirection;
      // 判断是否左转并返回结果
      return ((_plus_(SelfDrivingDirection::eLeftSide)) == (*m_op_self_next_driving_direction));
    } else {
      return false;
    }
  }
}

Base::txBool IVehicleElement::IsGoStraightAtNextIntersection() const TX_NOEXCEPT {
  // 在车道上返回false
  if (mLocation.IsOnLane()) {
    return true;
  } else {
    // 存在下次的行驶方向
    if (m_op_self_next_driving_direction.has_value()) {
      using SelfDrivingDirection = Base::Enums::VehicleInJunction_SelfDrivingDirection;
      // 判断是否直行并返回结果
      return ((_plus_(SelfDrivingDirection::eStraight)) == (*m_op_self_next_driving_direction));
    } else {
      return true;
    }
  }
}

Base::txBool IVehicleElement::IsUTurnAtNextIntersection() const TX_NOEXCEPT {
  // 在车道上返回false
  if (mLocation.IsOnLane()) {
    return false;
  } else {
    // 存在下次的行驶方向
    if (m_op_self_next_driving_direction.has_value()) {
      using SelfDrivingDirection = Base::Enums::VehicleInJunction_SelfDrivingDirection;
      // 判断是否掉头并返回结果
      return ((_plus_(SelfDrivingDirection::eUTurnSide)) == (*m_op_self_next_driving_direction));
    } else {
      return false;
    }
  }
}

#if USE_HashedRoadNetwork

void IVehicleElement::RegisterInfoOnInit(const Component::Location &_location) TX_NOEXCEPT {
  // 获取当前车道信息的哈希值
  m_curHashedLaneInfo = _location.GetHashedLaneInfo(Id());
  // 在当前车道信息的哈希值中注册车辆
  HdMap::HashedRoadCacheConCurrent::RegisterVehicle(m_curHashedLaneInfo, GetVehicleElementPtr());
}

void IVehicleElement::UpdateHashedLaneInfo(const Component::Location &_location) TX_NOEXCEPT {
#  if __ClearHashLanePerStep__
  m_curHashedLaneInfo = _location.GetHashedLaneInfo(Id());
  // 在当前车道信息的哈希值中注册车辆
  HdMap::HashedRoadCacheConCurrent::RegisterVehicle(m_curHashedLaneInfo, GetVehicleElementPtr());
#  else  /*__ClearHashLanePerStep__*/
  const HashedLaneInfo newHashedLaneInfo = _location.GetHashedLaneInfo(Id());
  if (CallFail(newHashedLaneInfo == m_curHashedLaneInfo)) {
    HdMap::HashedRoadCacheConCurrent::UnRegisterVehicle(m_curHashedLaneInfo, SysId());
    HdMap::HashedRoadCacheConCurrent::RegisterVehicle(newHashedLaneInfo, GetVehicleElementPtr());
  }
  m_curHashedLaneInfo = newHashedLaneInfo;
#  endif /*__ClearHashLanePerStep__*/
}

void IVehicleElement::UnRegisterInfoOnKill() TX_NOEXCEPT {
#  if __ClearHashLanePerStep__
#  else  /*__ClearHashLanePerStep__*/
  HdMap::HashedRoadCacheConCurrent::UnRegisterVehicle(m_curHashedLaneInfo, SysId());
#  endif /*__ClearHashLanePerStep__*/
}

#endif /*USE_HashedRoadNetwork*/

void IVehicleElement::OnStart(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
#if USE_HashedRoadNetwork
  // 在初始化时注册信息
  RegisterInfoOnInit(mLocation);
  mGeometryData.SetInValid(true);
#endif /*USE_HashedRoadNetwork*/
}

void IVehicleElement::OnEnd() TX_NOEXCEPT {
#if USE_HashedRoadNetwork
  UnRegisterInfoOnKill();
#endif /*USE_HashedRoadNetwork*/
}

void IVehicleElement::SetVelocityDesired(const txFloat _new_velocity_desired) TX_NOEXCEPT {
  // LOG(WARNING) << TX_VARS(Id()) << TX_VARS(_new_velocity_desired);
  mKinetics.velocity_desired = mKinetics.m_velocity_max = _new_velocity_desired;
}

void IVehicleElement::ResetVelocityDesired() TX_NOEXCEPT {
  // LOG(WARNING) << TX_VARS(Id()) << "ResetVelocityDesired";
  mKinetics.velocity_desired = mKinetics.m_velocity_max = mKinetics.raw_velocity_max;
}

Base::txFloat IVehicleElement::GeomCurveLength() const TX_NOEXCEPT {
  // 如果mLocation.tracker()非空且mLocation.tracker()->getCurrentCurve()非空
  if (NonNull_Pointer(mLocation.tracker()) && NonNull_Pointer(mLocation.tracker()->getCurrentCurve())) {
    // 返回当前曲线的长度
    return mLocation.tracker()->getCurrentCurve()->getLength();
  } else {
    return 0.0;
  }
}

Base::txFloat IVehicleElement::Compute_CenterLine_Offset() TX_NOEXCEPT {
  TX_MARK("left : +; right : -");
  // 将车辆的中心线偏移量设置为车道偏移量
  mLocation.CenterLineOffset() = mLocation.LaneOffset();

  // 根据车辆的切换车道状态进行处理
  switch (GetSwitchLaneState()) {
    case _plus_(VehicleMoveLaneState::eLeft):
    case _plus_(VehicleMoveLaneState::eLeftInLane): {
      // 如果车辆在左侧切换车道，将车辆的中心线偏移量增加横向位移量
      mLocation.CenterLineOffset() += mKinetics.m_LateralDisplacement;
      break;
    }
    case _plus_(VehicleMoveLaneState::eRight):
    case _plus_(VehicleMoveLaneState::eRightInLane): {
      // 如果车辆在右侧切换车道，将车辆的中心线偏移量减少横向位移量
      mLocation.CenterLineOffset() -= mKinetics.m_LateralDisplacement;
      break;
    }
  }

  return mLocation.CenterLineOffset();
}

void IVehicleElement::Compute_FCW(Base::ITrafficElementPtr target_ptr) TX_NOEXCEPT {
  ResetFCW();
  // 目标指针不为空且当前元素存活
  if (NonNull_Pointer(target_ptr) && IsAlive()) {
    const Base::txVec3 ego_lane_dir = target_ptr->GetLaneDir();
    const Base::txVec3 self_lane_dir = StableLaneDir();
    // 如果车辆的车道方向与自身车道方向同步
    if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(ego_lane_dir), Utils::Vec3_Vec2(self_lane_dir)))) {
      const auto ego_polygon = target_ptr->GetPolygon();
      const auto self_polygon = GetPolygon();

      // 如果包围盒未重合
      if (CallFail(Geometry::CD2D::GJK::HitPolygons(ego_polygon, self_polygon))) {
        TX_MARK("dist > 0 means obs is front of ego, dist < 0.0 means obs is behind of ego");
        const txFloat sign = target_ptr->ComputeProjectDistance(ego_polygon, self_polygon);
        // 计算障碍物与车辆在车道上的投影距离的绝对值
        const txFloat abs_lane_proj_dist = std::fabs(sign);

        // 用于存储障碍物与车辆之间的最近点
        Base::txVec2 nouse_intersectPt1, nouse_intersectPt2;
        // 计算障碍物与车辆之间的欧氏距离
        const txFloat euclid_dist = Geometry::CD2D::smallest_dist_between_ego_and_obsVehicle(
            ego_polygon, self_polygon, nouse_intersectPt1, nouse_intersectPt2);

        const Base::txFloat egoVelocity = target_ptr->GetRawVelocity();
        const Base::txFloat vehicleVelocity = GetRawVelocity();

        // 设置障碍物与车辆在车道上的投影距离
        mFCW.set_lane_proj_distance(abs_lane_proj_dist);
        mFCW.set_euclidean_distance(euclid_dist);
        mFCW.set_ego_v(egoVelocity);
        // 设置车辆的速度
        mFCW.set_vehicle_v(vehicleVelocity);

        // 设置车辆的速度
        if (sign > 0.0) {
          // 设置障碍物与车辆的关系为车辆在障碍物后面
          mFCW.set_state(sim_msg::Ego_In_Back);
          FCW_LogInfo << "[FCW] vehicle is front of ego." << TX_VARS(Id()) << _StreamPrecision_ << TX_VARS(sign);
          TX_MARK("on laneprojection, vehicle is front of ego.");
          // 如果车辆与障碍物的速度差不为0
          if (!Math::isZero(egoVelocity - vehicleVelocity, 0.1)) {
            // 如果车辆速度大于障碍物速度
            if (egoVelocity > vehicleVelocity) {
              // 计算车辆与障碍物在车道上的投影距离的时间到达
              mFCW.set_ttc_lane_proj_distance(abs_lane_proj_dist / (egoVelocity - vehicleVelocity));
              // 计算车辆与障碍物之间的欧氏距离的时间到达
              mFCW.set_ttc_euclidean_distance(euclid_dist / (egoVelocity - vehicleVelocity));
            } else {
              TX_MARK("impossible collision");
            }
          } else {
            TX_MARK("impossible collision");
          }
          // 如果车辆速度不为0
          if (Math::isNotZero(egoVelocity)) {
            // 计算车辆与障碍物在车道上的投影距离的距离到达
            mFCW.set_thw_lane_proj_distance(abs_lane_proj_dist / egoVelocity);
            mFCW.set_thw_euclidean_distance(euclid_dist / egoVelocity);
          }
        } else if (sign < 0.0) {  // 如果障碍物在车辆后方
          mFCW.set_state(sim_msg::Ego_In_Front);
          FCW_LogInfo << "[FCW] vehicle is behind of ego." << TX_VARS(Id()) << _StreamPrecision_ << TX_VARS(sign);
          TX_MARK("on laneprojection, vehicle is behand of ego.");
          // 如果车辆与障碍物的速度差不为0
          if (!Math::isZero(egoVelocity - vehicleVelocity, 0.1)) {
            // 如果障碍物速度大于车辆速度
            if (vehicleVelocity > egoVelocity) {
              // 计算车辆与障碍物在车道上的投影距离的时间到达
              mFCW.set_ttc_lane_proj_distance(abs_lane_proj_dist / (vehicleVelocity - egoVelocity));
              mFCW.set_ttc_euclidean_distance(euclid_dist / (vehicleVelocity - egoVelocity));
            } else {
              TX_MARK("impossible collision");
            }
          } else {
            TX_MARK("impossible collision");
          }
          // 如果障碍物速度不为0
          if (Math::isNotZero(vehicleVelocity)) {
            // 计算车辆与障碍物在车道上的投影距离的距离到达
            mFCW.set_thw_lane_proj_distance(abs_lane_proj_dist / vehicleVelocity);
            mFCW.set_thw_euclidean_distance(euclid_dist / vehicleVelocity);
          }
        } else {
          TX_MARK("has been collision.");
          FCW_LogInfo << "[FCW] vehicle and ego have collided." << TX_VARS(Id()) << _StreamPrecision_ << TX_VARS(sign);
          mFCW.set_state(sim_msg::Ego_Vehicle_Overlap);
        }
      } else {
        TX_MARK("collision");
      }
    }
  } else {
    FCW_LogWarn << TX_VARS(Id()) << " param failure.";
  }
  FCW_LogInfo << TX_VARS(Id()) << mFCW.ShortDebugString();
}

#if __TX_Mark__("parallel simulation crash")
void IVehicleElement::SetCurDistanceToTargetSignLight(Base::txFloat dist) TX_NOEXCEPT {
  {
    TX_MARK("all vehicle for debug, need update per-vehicle");
    tbb::mutex::scoped_lock lock(m_tbbMutex);
    mDistanceToTargetSignLight = dist;
  }
}

void IVehicleElement::SetCurTargetSignLightPtr(Base::ISignalLightElementPtr tlPtr) TX_NOEXCEPT {
  {
    tbb::mutex::scoped_lock lock(m_tbbMutex);
    m_weak_TargetSignLightPtr = tlPtr;
  }
}

void IVehicleElement::SetCurTargetSignLight(Base::ISignalLightElementPtr tlPtr, Base::txFloat dist) TX_NOEXCEPT {
  {
    tbb::mutex::scoped_lock lock(m_tbbMutex);
    // 如果不遵守规则，直接返回
    if (!mRuleCompliance) {
      return;
    }
    // 如果当前目标信号灯有效且新的信号灯有效
    if (!m_weak_TargetSignLightPtr.expired() && NonNull_Pointer(tlPtr)) {
      // 如果车辆ID为偶数且新信号灯ID大于当前信号灯ID，或者车辆ID为奇数且新信号灯ID小于当前信号灯ID，直接返回
      if ((Id() % 2 == 0 && tlPtr->Id() > m_weak_TargetSignLightPtr.lock()->Id()) ||
          (Id() % 2 == 1 && tlPtr->Id() < m_weak_TargetSignLightPtr.lock()->Id())) {
        return;
      }
    }
    m_weak_TargetSignLightPtr = tlPtr;
    mDistanceToTargetSignLight = dist;
  }
}

#endif /*__TX_Mark__("parallel simulation crash")*/

void IVehicleElement::SetDistanceToPedestrian(Base::txFloat dist) TX_NOEXCEPT {
  tbb::mutex::scoped_lock lock(m_tbbMutex);
  mDistanceToPedestrian = dist;
}

void IVehicleElement::SetCurFrontPedestrianPtr(Base::IPedestrianElementPtr pedePtr) TX_NOEXCEPT {
  tbb::mutex::scoped_lock lock(m_tbbMutex);
  m_weak_PedestrianPtr = pedePtr;
}
#if __TX_Mark__("VehicleCoord")
Base::txBool IVehicleElement::SyncPosition(const txFloat _passTime) TX_NOEXCEPT {
  const Base::txVec2 dir2d(GetHeadingWithAngle().Cos(), GetHeadingWithAngle().Sin());
  if (CallSucc(RawVehicleCoord_Is_RearAxleCenter())) {
#  if 0
    TX_MARK("given rear_axle_center, compute geom_center");
    mLocation.RearAxleCenter() = RawVehicleCoord();
    const txFloat halfLen = 0.5 * GetLength();
    const txFloat rear_axle_to_rear = VehicleGeometory().vehicle_geometory().rear_axle_to_rear();
    const txFloat step = halfLen - rear_axle_to_rear;
    const Base::txVec2 step_vec = dir2d * step;
    mLocation.GeomCenter() = mLocation.RearAxleCenter();
    mLocation.GeomCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
    LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
        << "[SyncPosition][rear_axle]" << TX_VARS(_passTime) << TX_VARS(Id()) << TX_VARS(halfLen)
        << TX_VARS(rear_axle_to_rear) << TX_VARS(step) << TX_VARS(RawVehicleCoord());
#  else
    // 获取车辆相对于车尾轴心的前进方向
    const Base::txVec2 step_vec = dir2d * Catalog_Rear2Center_North();
    // 将车辆当前的坐标设置为车尾轴心位置
    mLocation.RearAxleCenter() = RawVehicleCoord();
    // 将车辆的经纬度坐标设置为车尾轴心位置
    mLocation.GeomCenter() = mLocation.RearAxleCenter();
    // 计算车辆的经纬度坐标，并将其向前进的方向移动一定距离
    mLocation.GeomCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
    LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
        << "[SyncPosition][rear_axle]" << TX_VARS(_passTime) << TX_VARS(Id())
        << TX_VARS_NAME(step, Catalog_Rear2Center_North()) << TX_VARS(RawVehicleCoord());
#  endif
  } else if (CallSucc(RawVehicleCoord_Is_GeomCenter())) {
#  if 0
    TX_MARK("given geom_center, compute rear_axle_center");
    mLocation.GeomCenter() = RawVehicleCoord();
    const txFloat halfLen = 0.5 * GetLength();
    const txFloat rear_axle_to_rear = VehicleGeometory().vehicle_geometory().rear_axle_to_rear();
    const txFloat step = halfLen - rear_axle_to_rear;
    const Base::txVec2 step_vec = dir2d * step * -1.0;
    mLocation.RearAxleCenter() = mLocation.GeomCenter();
    mLocation.RearAxleCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
    LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
        << "[SyncPosition][geom_center]" << TX_VARS(_passTime) << TX_VARS(Id()) << TX_VARS(halfLen)
        << TX_VARS(rear_axle_to_rear) << TX_VARS(step * -1.0) << TX_VARS(RawVehicleCoord());
#  else
    // 设置位置信息
    mLocation.GeomCenter() = RawVehicleCoord();
    const Base::txVec2 step_vec = dir2d * Catalog_Rear2Center_North() * -1.0;
    mLocation.RearAxleCenter() = mLocation.GeomCenter();
    mLocation.RearAxleCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
    LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
        << "[SyncPosition][geom_center]" << TX_VARS(_passTime) << TX_VARS(Id())
        << TX_VARS_NAME(step, (Catalog_Rear2Center_North() * -1.0)) << TX_VARS(RawVehicleCoord());
#  endif
  } else {
    mLocation.RearAxleCenter() = RawVehicleCoord();
    mLocation.GeomCenter() = RawVehicleCoord();
    LOG(WARNING) << "unsupport Vehicle Coord Type." << TX_VARS(RawVehicleCoord());
    return false;
  }
  return true;
}
#endif /*VehicleCoord*/

sim_msg::Vec3 IVehicleElement::local_cartesian_point() const TX_NOEXCEPT {
  sim_msg::Vec3 res;
  res.set_x(local_cartesian_point_x());
  res.set_y(local_cartesian_point_y());
  res.set_z(local_cartesian_point_z());
  return res;
}

void IVehicleElement::ComputeExternInfo(sim_msg::Extra_Info *_extern_info) const TX_NOEXCEPT {
  if (NonNull_Pointer(_extern_info)) {
    // 将车辆的局部欧几里得坐标系下的点信息存储到 _extern_info 结构体中
    _extern_info->mutable_local_cartesian_point()->CopyFrom(local_cartesian_point());
    _extern_info->set_speed_ms(GetVelocity());
    // 获取车辆与其他车辆的空间关系
    switch (m_ego_relationship) {
      case Base::IVehicleElement::spatial_relationship_with_ego::ego_front_veh:
        // 当前车辆在前方车辆之前
        _extern_info->set_front_veh(true);
        _extern_info->set_rear_veh(false);
        break;
      case Base::IVehicleElement::spatial_relationship_with_ego::ego_rear_veh:
        // 当前车辆在后方车辆之后
        _extern_info->set_front_veh(false);
        _extern_info->set_rear_veh(true);
        break;
      default:
        // 当前车辆与其他车辆的空间关系不明确
        _extern_info->set_front_veh(false);
        _extern_info->set_rear_veh(false);
        break;
    }
    _extern_info->set_dist_to_ego_m(m_ego_dist);
  }
}

Base::txBool IVehicleElement::IsTransparentObject(const Base::Enums::VEHICLE_TYPE &_vehicle_type) TX_NOEXCEPT {
  return (_plus_(Base::Enums::VEHICLE_TYPE::Port_Crane_001) == _vehicle_type) ||
         (_plus_(Base::Enums::VEHICLE_TYPE::Port_Crane_002) == _vehicle_type) ||
         (_plus_(Base::Enums::VEHICLE_TYPE::Port_Crane_002_0_0) == _vehicle_type) ||
         (_plus_(Base::Enums::VEHICLE_TYPE::Port_Crane_002_0_2) == _vehicle_type) ||
         (_plus_(Base::Enums::VEHICLE_TYPE::Port_Crane_002_0_5) == _vehicle_type) ||
         (_plus_(Base::Enums::VEHICLE_TYPE::Port_Crane_002_1_0) == _vehicle_type) ||
         (_plus_(Base::Enums::VEHICLE_TYPE::Port_Crane_002_5_0) == _vehicle_type) ||
         (_plus_(Base::Enums::VEHICLE_TYPE::Port_Crane_003) == _vehicle_type) ||
         (_plus_(Base::Enums::VEHICLE_TYPE::Port_Crane_004) == _vehicle_type);
}

Base::txBool IVehicleElement::IsTransparentObject(const Base::txString &_vehicle_type) TX_NOEXCEPT {
  return ("Port_Crane_001" == _vehicle_type) || ("Port_Crane_002" == _vehicle_type) ||
         ("Port_Crane_002_0_0" == _vehicle_type) || ("Port_Crane_002_0_2" == _vehicle_type) ||
         ("Port_Crane_002_0_5" == _vehicle_type) || ("Port_Crane_002_1_0" == _vehicle_type) ||
         ("Port_Crane_002_5_0" == _vehicle_type) || ("Port_Crane_003" == _vehicle_type) ||
         ("Port_Crane_004" == _vehicle_type);
}

Base::txBool IVehicleElement::FillingTrajectory(Base::TimeParamManager const &timeMgr,
                                                sim_msg::Trajectory &refTraj) TX_NOEXCEPT {
  // 获取当前车辆的海拔高度
  const Base::txFloat ego_alt = Altitude();
  refTraj.clear_point();
  // 创建一个新的轨迹点，并设置其海拔高度、时间、速度、加速度和距离
  sim_msg::TrajectoryPoint *new_traj_point_ptr = refTraj.add_point();
  new_traj_point_ptr->set_z(ego_alt);
  new_traj_point_ptr->set_t(timeMgr.AbsTime());
  new_traj_point_ptr->set_v(0.0);
  new_traj_point_ptr->set_a(0.0);
  new_traj_point_ptr->set_s(0.0);

  // 获取当前车辆的地理坐标，并设置轨迹点的X和Y坐标
  Coord::txWGS84 cur_pt_wgs84 = RawVehicleCoord();
  new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
  new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
  // 获取当前车辆的纬度向量，并计算轨迹点的朝向角（以弧度为单位）
  const Base::txFloat _headingRadian = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetRadian();
  new_traj_point_ptr->set_theta(_headingRadian);

  // 创建一个新的轨迹点，并将其与原轨迹点相同，但设置为新的轨迹点
  sim_msg::TrajectoryPoint *new_clone_traj_point_ptr = refTraj.add_point();
  new_clone_traj_point_ptr->CopyFrom(*new_traj_point_ptr);
  LOG(WARNING) << "default trajectory." << TX_VARS(timeMgr.PassTime()) << cur_pt_wgs84.StrWGS84();
  return true;
}

Base::txBool IVehicleElement::IsNearLink(const txFloat dist) const TX_NOEXCEPT {
  // 获取车辆当前的车道信息
  const auto &locInfo = GetCurrentLaneInfo();
  // 获取车辆的几何中心点
  const Coord::txWGS84 elemPt = GeomCenter();
  // 如果车辆在车道上
  if (CallSucc(locInfo.IsOnLane())) {
    /*lane*/
    const Coord::txWGS84 road_end_pt = HdMap::HadmapCacheConCurrent::GetRoadEnd(locInfo.onLaneUid.roadId);
    // 计算车辆与车道终点的欧几里得距离
    const Base::txFloat dist2roadend = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(elemPt, road_end_pt);
    // 如果车辆与车道终点的距离小于指定距离，返回true，否则返回false
    return (dist2roadend < dist);
  } else {
    /*link*/
    const Coord::txWGS84 road_end_pt = HdMap::HadmapCacheConCurrent::GetRoadEnd(locInfo.onLinkFromLaneUid.roadId);
    // 计算车辆与连接线终点的欧几里得距离
    const Base::txFloat dist2roadend = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(elemPt, road_end_pt);
    return (dist2roadend < dist);
  }
}

Base::txBool IVehicleElement::IsOnJunction() TX_NOEXCEPT {
  /*ego_near_junction_dist*/
  // 如果车辆附近的距离小于FLAGS_elem_near_junction_dist
  if (IsNearLink(FLAGS_elem_near_junction_dist)) {
    // 获取车辆当前的车道信息
    const auto cur_locInfo = GetCurrentLaneInfo();
    // 定义一个hadmap::txLaneLinks类型的变量res_links
    hadmap::txLaneLinks res_links;
    // 定义一个Base::txRoadID类型的变量roadId，初始化为-1
    Base::txRoadID roadId = -1;
    // 如果车辆在车道上
    if (cur_locInfo.IsOnLane()) {
      roadId = cur_locInfo.onLaneUid.roadId;
    } else {
      roadId = cur_locInfo.onLinkFromLaneUid.roadId;
    }
    // 获取roadId对应的连接线信息
    hadmap::getLaneLinks(txMapHdr, roadId, ROAD_PKID_INVALID, res_links);
    // 定义一个std::set<hadmap::roadpkid>类型的变量to_road_set
    std::set<hadmap::roadpkid> to_road_set;
    // 遍历res_links中的连接线信息
    for (auto linkptr : res_links) {
      // 如果连接线信息有效
      if (NonNull_Pointer(linkptr)) {
        // 将连接线的toTxLaneId().roadId插入到to_road_set中
        to_road_set.insert(linkptr->toTxLaneId().roadId);
      }
    }
    // LogWarn << "[Sketch]" << TX_VARS(to_road_set.size());
    return (to_road_set.size() > 1);
  } else {
    // LogWarn << "[Sketch][ignore] IsNearLink ";
    return false;
  }
}

#if __TX_Mark__("SceneSketch")
void ISceneSketchElement::UpdateSketchSurroundingVehicle(const TimeParamManager &timeMgr) TX_NOEXCEPT {
  LOG(FATAL) << "ISceneSketchElement::UpdateSketchSurroundingVehicle";
}

#endif /*SceneSketch*/
TX_NAMESPACE_CLOSE(Base)

#undef FCW_LogInfo
#undef FCW_LogWarn
