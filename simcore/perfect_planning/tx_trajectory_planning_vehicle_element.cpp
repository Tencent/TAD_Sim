// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_trajectory_planning_vehicle_element.h"
#include "tx_time_utils.h"
#include "tx_trajectory_sampling_node.h"

#define LoopInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_Traj_ClosedLoop)
#define TrajInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_Traj)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool PlanningVechicleElement::Update(TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (CallSucc(IsAlive()) && CallFail(IsStop())) {
    UpdateActivateMode(pb2enum(m_pb_vehicle_interaction.activate()));

    LoopInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS(Id()) << TX_VARS_NAME(AutoMode, AutoMode()._to_string());
    if (FLAGS_LogLevel_Ego_EnvPerception) {
      LOG(INFO) << "[Surrounding]" << timeMgr << TX_VARS(Id()) << m_EgoNearestObject.Str() << TX_VARS(StableLaneInfo())
                << TX_VARS(mLocation.LaneOffset());
      LOG(INFO) << "[Surrounding_follow]" << timeMgr << TX_VARS(Id())
                << TX_VARS_NAME(front_element_id,
                                ((NonNull_Pointer(FollowFrontPtr())) ? (FollowFrontPtr()->ConsistencyId()) : (-1)))
                << TX_VARS_NAME(front_element_dist, FollowFrontDistance()) << TX_VARS(StableLaneInfo());
    }

    if (CallFail(Need_Autopilot_Activate())) {
      SetPlanningVelocity(0.0);
      mKinetics.m_acceleration = 0.0;
      mKinetics.m_LateralVelocity = 0.0;
      return true;
    }
    CheckWayPoint();
    UpdateNextCrossDirection();
    if (CallSucc(Need_Longitudinal_Activate())) {
      mKinetics.m_acceleration = DrivingFollow(timeMgr);
      LoopInfo << TX_VARS(timeMgr.PassTime())
               << TX_VARS_NAME(front_element_id,
                               ((NonNull_Pointer(FollowFrontPtr())) ? (FollowFrontPtr()->ConsistencyId()) : (-1)))
               << TX_VARS_NAME(front_element_dist, FollowFrontDistance()) << TX_VARS(mKinetics.m_acceleration);

      // if (CallFail(CheckPedestrianWalking(timeMgr.RelativeTime(), mKinetics.m_LateralVelocity)))
      CheckPedestrianWalking(timeMgr.RelativeTime(), mKinetics.m_LateralVelocity);
      if (NonNull_Pointer(env_signal_ptr())) {
        const txFloat signal_dist = env_signal_dist();
        const auto signal_color = env_signal_ptr()->GetCurrentSignLightColorType();
        const txSysId signalId = env_signal_ptr()->Id();
        LoopInfo << TX_VARS(Id()) << "[signal] " << TX_VARS(signalId) << TX_VARS(signal_dist)
                 << TX_VARS_NAME(Color, signal_color._to_string());
        if (signal_dist < FollowFrontDistance() || signal_dist < GetLength()) {
          mKinetics.m_acceleration = CheckSignLight(signalId, signal_dist, signal_color);
          /*LOG(INFO) << "[CheckSignLight]" << TX_VARS(timeMgr.PassTime()) << TX_VARS(mKinetics.m_acceleration)
              << TX_VARS(signal_dist) << TX_VARS(FollowFrontDistance()) << TX_VARS(GetLength());*/
        }
      }
      /*LOG(WARNING) << timeMgr.str() << TX_VARS(mKinetics.m_acceleration) << TX_VARS(StableVelocity());*/
      // mKinetics.m_velocity = StableVelocity() + mKinetics.m_acceleration * timeMgr.RelativeTime();

      LoopInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS(mKinetics.m_acceleration) << TX_VARS(mKinetics.m_velocity);
      mKinetics.LastAcceleration();
      SetPlanningVelocity(mKinetics.m_velocity + mKinetics.m_acceleration * timeMgr.RelativeTime());
    } else {
      mKinetics.m_acceleration = 0.0;
      SetPlanningVelocity(0.0);
    }

    if (CallSucc(Need_Lateral_Activate())) {
      mKinetics.m_LateralVelocity = DrivingChangeLane(timeMgr);
      LoopInfo << TX_VARS(Id()) << TX_VARS(mKinetics.m_LateralVelocity);
    } else {
      mKinetics.m_LateralVelocity = 0.0;
    }

    {
      /*LOG(WARNING) << timeMgr.str() << TX_VARS(mKinetics.m_velocity);*/
      mKinetics.LimitVelocity();
      /*LOG(WARNING) << timeMgr.str() << TX_VARS(mKinetics.m_velocity) << " from_LimitVelocity";*/
      if (MoveStraightOnS(timeMgr)) {
        mKinetics.LimitVelocity();
        TX_MARK("move to Component::Location, pImpl->NormalTargetRotateAngle();");
        LaneChangePostOp(timeMgr);
        mGeometryData.ComputePolygon(StableGeomCenter(), mLocation.vLaneDir());
        return true;
      } else {
        StopVehicle();
        LogWarn << TX_VARS_NAME(vehicle_id, Id()) << " MoveStraightOnS failure. stop!";
        return false;
      }
    }
    return true;
  } else {
    LogWarn << "vehicle is not update. " << TX_VARS(Id()) << TX_COND_NAME(IsAlive, IsAlive())
            << TX_COND_NAME(IsStop, IsStop());
    return (IsStop());
  }
}

Base::txBool PlanningVechicleElement::FillingTrajectory(Base::TimeParamManager const& timeMgr,
                                                        sim_msg::Trajectory& refTraj) TX_NOEXCEPT {
  refTraj.Clear();
  SaveStableState();
  if (IsAlive()) {
    /*for lane change finish*/
    UpdateHashedLaneInfo(mLocation);
    const HashedLaneInfo& selfStableHashedLaneInfo = StableHashedLaneInfo();
    UpdateHashedRoadInfoCache(selfStableHashedLaneInfo);
    HashedLaneInfoOrthogonalListPtr start_hashed_node_ptr = Weak2SharedPtr(std::get<1>(m_HashedRoadInfoCache));

    if (IsInTurnLeft()) {
      const HashedLaneInfo leftHashedLaneInfo = mLocation.GetLeftHashedLaneInfo(Id());
      start_hashed_node_ptr = HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(leftHashedLaneInfo);
    } else if (IsInTurnRight()) {
      const HashedLaneInfo rightHashedLaneInfo = mLocation.GetRightHashedLaneInfo(Id());
      start_hashed_node_ptr = HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(rightHashedLaneInfo);
    }

    if (NonNull_Pointer(start_hashed_node_ptr)) {
      const Base::txFloat reverse_base = DistanceAlongCurve() - start_hashed_node_ptr->GetSelfHashedInfo().StartScope();
      Base::txFloat cur_velocity = __HYPOT__(GetVelocity(), GetLateralVelocity());
      const Base::txFloat traj_length = cur_velocity * FLAGS_vehicle_trajectory_duration_s + 10.0;
      const Base::txFloat real_search_dist = reverse_base + traj_length;

      std::list<HashedLaneInfoOrthogonalListPtr> search_result_by_level;
      static const Base::txBool bCheckReachable = true;
      static const Base::txBool bDoNotCheckReachable = false;
      if (CallFail(SearchOrthogonalListFrontByLevel(bCheckReachable, start_hashed_node_ptr, real_search_dist, 0,
                                                    search_result_by_level))) {
        search_result_by_level.clear();
        SearchOrthogonalListFrontByLevel(bDoNotCheckReachable, start_hashed_node_ptr, real_search_dist, 0,
                                         search_result_by_level);
        TrajInfo << "[trajectory][using_random_select]";
      } else {
        TrajInfo << "[trajectory][using_check_reachable]";
      }

      TrajInfo << "[trajectory]" << TX_VARS(reverse_base) << TX_VARS(traj_length) << TX_VARS(real_search_dist)
               << TX_VARS(search_result_by_level.size());
      if (FLAGS_LogLevel_Ego_Traj) {
        Base::txInt idx = 0;
        for (const auto& ref : search_result_by_level) {
          LOG(INFO) << "[trajectory]" << TX_VARS(idx) << (ref->GetSelfHashedInfo());
          ++idx;
        }
      }

      std::vector<Base::TrajectorySamplingNode> vec_sampling_traj_pts;
      //  vec_sampling_traj_pts.emplace_back(
      //    Base::TrajectorySamplingNode(start_hashed_node_ptr->GetSelfHashedInfo().LaneInfo() /*current
      // lane info*/, DistanceAlongCurve(), LaneOffset())
      //  );
      /*20230408*/ LoopInfo << TX_VARS_NAME(LOC, start_hashed_node_ptr->GetSelfHashedInfo().LaneInfo())
                            << TX_VARS(DistanceAlongCurve()) << TX_VARS(LaneOffset());
      if (CallSucc(IsInLaneChange())) {
        for (const auto& refTrajOrthogonalListPtr : search_result_by_level) {
          const auto& cur_level_HashNodeInfo = refTrajOrthogonalListPtr->GetSelfHashedInfo();
          vec_sampling_traj_pts.emplace_back(
              Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                           cur_level_HashNodeInfo.EndScope(), LaneOffset()));
        }
      } else {
        const auto& start_lane_info = start_hashed_node_ptr->GetSelfHashedInfo().LaneInfo();

        for (const auto& refTrajOrthogonalListPtr : search_result_by_level) {
          const auto& cur_level_HashNodeInfo = refTrajOrthogonalListPtr->GetSelfHashedInfo();

          if (CallSucc(start_lane_info == cur_level_HashNodeInfo.LaneInfo())) {
            if (DistanceAlongCurve() < cur_level_HashNodeInfo.EndScope() &&
                cur_level_HashNodeInfo.StartScope() < DistanceAlongCurve()) {
              const Base::txFloat real_length = cur_level_HashNodeInfo.EndScope() - DistanceAlongCurve();

              vec_sampling_traj_pts.emplace_back(Base::TrajectorySamplingNode(
                  cur_level_HashNodeInfo.LaneInfo(), DistanceAlongCurve(), LaneOffset(), true));

              vec_sampling_traj_pts.emplace_back(Base::TrajectorySamplingNode(
                  cur_level_HashNodeInfo.LaneInfo(), DistanceAlongCurve() + 0.3 * real_length, LaneOffset()));

              vec_sampling_traj_pts.emplace_back(Base::TrajectorySamplingNode(
                  cur_level_HashNodeInfo.LaneInfo(), DistanceAlongCurve() + 0.6 * real_length, LaneOffset()));

              /*vec_sampling_traj_pts.emplace_back(
                      Base::TrajectorySamplingNode(
                              cur_level_HashNodeInfo.LaneInfo(), cur_level_HashNodeInfo.EndScope(), LaneOffset())
              );*/
            } else {
              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               cur_level_HashNodeInfo.EndScope(), LaneOffset()));
            }
          } else {
            const Base::txFloat real_length = cur_level_HashNodeInfo.RealLength();
            vec_sampling_traj_pts.emplace_back(Base::TrajectorySamplingNode(
                cur_level_HashNodeInfo.LaneInfo(), cur_level_HashNodeInfo.StartScope(), LaneOffset(), true));
            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo(),
                                             cur_level_HashNodeInfo.StartScope() + 0.35 * real_length, LaneOffset()));

            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo(),
                                             cur_level_HashNodeInfo.StartScope() + 0.7 * real_length, LaneOffset()));

            /*vec_sampling_traj_pts.emplace_back(
                    Base::TrajectorySamplingNode(
                            cur_level_HashNodeInfo.LaneInfo(), cur_level_HashNodeInfo.EndScope(), LaneOffset())
            );*/
          }
        }
      }

      for (auto itr = std::begin(vec_sampling_traj_pts); itr != vec_sampling_traj_pts.end();) {
        if (itr->ComputeSamplingPoint()) {
          ++itr;
        } else {
          itr = vec_sampling_traj_pts.erase(itr);
        }
      }

#if _LocationCloseLoop_

      // if (IsOnLane() /*&& CallSucc(IsInLaneChange())*/)
      {
        for (auto itr = (std::begin(vec_sampling_traj_pts) + 1); itr != vec_sampling_traj_pts.end();) {
          const Base::txFloat dist_with_first_pt =
              Coord::txENU::EuclideanDistanceLocal_2D(vec_sampling_traj_pts[0].SamplingPt(), itr->SamplingPt());

          if (dist_with_first_pt < 5.0 && CallFail((*itr).FirstSegmentPt())) {
            itr = vec_sampling_traj_pts.erase(itr);
          } else {
            break;
          }
        }
      }
#endif
      if (FLAGS_LogLevel_Ego_Traj) {
        for (const auto& ref : vec_sampling_traj_pts) {
          LOG(INFO) << "[trajectory]" << TX_VARS_NAME(SamplingNode, ref.Str());
        }
      }

      /*20230408*/ /*LOG(INFO) << TX_VARS(vec_sampling_traj_pts.size());*/
      const Base::txFloat ego_alt = Altitude();
      if (vec_sampling_traj_pts.size() > 1) {
        TrafficFlow::CentripetalCatMull::control_path_node_vec ref_control_path;
#if _LocationCloseLoop_
        for (Base::txSize idx = 0; idx < vec_sampling_traj_pts.size(); ++idx) {
          Base::ISceneLoader::IRouteViewer::control_path_node path_node;
          path_node.waypoint = vec_sampling_traj_pts[idx].SamplingPt().ToWGS84().WGS84();
          path_node.locInfo = vec_sampling_traj_pts[idx].LaneInfo();
          path_node.dist_on_s = vec_sampling_traj_pts[idx].S();
          ref_control_path.emplace_back(path_node);
        }
        LOG(INFO) << "[trajectory]#######";
        for (int i = 0; i < 3 && i < vec_sampling_traj_pts.size(); ++i) {
          LOG(INFO) << "[trajectory]" << TX_VARS(i) << TX_VARS_NAME(locInfo, vec_sampling_traj_pts[i].LaneInfo())
                    << TX_VARS_NAME(S, vec_sampling_traj_pts[i].S());
        }
        LOG(INFO) << "[trajectory]*******";

#else  /*_LocationCloseLoop_*/
        {
          /*first node real position*/
          Base::ISceneLoader::IRouteViewer::control_path_node path_node;
          path_node.waypoint = RawVehicleCoord().WGS84();
          ref_control_path.emplace_back(path_node);
        }
        for (Base::txSize idx = 1; idx < vec_sampling_traj_pts.size(); ++idx) {
          Base::ISceneLoader::IRouteViewer::control_path_node path_node;
          path_node.waypoint = vec_sampling_traj_pts[idx].SamplingPt().ToWGS84().WGS84();
          ref_control_path.emplace_back(path_node);
        }
#endif /*_LocationCloseLoop_*/
        m_trajectory_ptr = std::make_shared<TrafficFlow::CentripetalCatMull>(ref_control_path);
        auto laneGeomPtr = m_trajectory_ptr;

        if (NonNull_Pointer(laneGeomPtr)) {
          const Base::txFloat traj_real_length = laneGeomPtr->GetLength();
          Coord::txWGS84 end_wgs84;
          end_wgs84.FromENU(laneGeomPtr->GetEndPt());
#if 1
          const Base::txFloat _zeroRadian =
              Utils::GetLaneAngleFromVectorOnENU(laneGeomPtr->GetLaneDir(0.0)).GetRadian();
          Unit::CircleRadian circRad(_zeroRadian - Unit::CircleRadian::Half_PI(),
                                     _zeroRadian + Unit::CircleRadian::Half_PI(), _zeroRadian);
          refTraj.set_a(GetAcc());
          TrajInfo << TX_VARS(GetVelocity()) << TX_VARS(GetAcc());
          Base::txFloat cur_velocity = __HYPOT__(GetVelocity(), GetLateralVelocity());
          Base::txFloat cur_acc = 0.0;  // GetAcc();
          Base::txFloat cur_s = 0.0;
          const Base::txFloat timeStep = 0.02;  // timeMgr.RelativeTime();
          Coord::txWGS84 lastSampltPt;
          sim_msg::TrajectoryPoint last_traj_point;
          Base::txFloat traj_base_time_s;
          std::ostringstream oss;
          oss << "[t_dist]";
          if (timeMgr.AbsTime() <= 0.1 && m_dynamic_location.t() > 15) {
            traj_base_time_s = Utils::MillisecondToSecond(m_dynamic_location.t());
          } else {
            traj_base_time_s = m_dynamic_location.t();
          }
          LOG(INFO) << TX_VARS(timeMgr.AbsTime()) << TX_VARS(m_dynamic_location.t()) << TX_VARS(traj_base_time_s);

          for (Base::txFloat start_s = 0.0; start_s <= FLAGS_ego_trajectory_duration_s; start_s += timeStep) {
            if (cur_s < traj_real_length && cur_s < (cur_velocity * FLAGS_ego_trajectory_duration_s)) {
              sim_msg::TrajectoryPoint* new_traj_point_ptr = refTraj.add_point();
              new_traj_point_ptr->set_z(ego_alt);
              new_traj_point_ptr->set_t(start_s + traj_base_time_s /*timeMgr.AbsTime()*/);
              new_traj_point_ptr->set_v(cur_velocity);
              new_traj_point_ptr->set_a(GetAcc());
              new_traj_point_ptr->set_s(cur_s);

              /*1. update velocity*/
              cur_velocity += cur_acc * timeStep;
              if (cur_velocity >= mKinetics.m_velocity_max) {
                cur_velocity = mKinetics.m_velocity_max;
                cur_acc = 0.0;
              }
              if (cur_velocity < 0.0) {
                cur_velocity = 0.0;
                cur_acc = 0.0;
              }

              Coord::txWGS84 cur_pt_wgs84;
              if (start_s > 0.0) {
                cur_pt_wgs84.FromENU(laneGeomPtr->GetLocalPos(new_traj_point_ptr->s()));
              } else {
                cur_pt_wgs84 = RawVehicleCoord();
              }
              new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
              new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());

              if (start_s > 0.0) {
                const int lastPtIdx = refTraj.point_size() - 2;
                const Base::txFloat real_dist =
                    Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(lastSampltPt, cur_pt_wgs84);
                const Base::txFloat delta_t = real_dist / cur_velocity;
                new_traj_point_ptr->set_t(last_traj_point.t() + delta_t);
                oss << TX_VARS_NAME(t, new_traj_point_ptr->t()) << TX_VARS(real_dist);
              }

              const Base::txFloat _headingRadian =
                  Utils::GetLaneAngleFromVectorOnENU(laneGeomPtr->GetLaneDir(new_traj_point_ptr->s())).GetRadian();
              const Base::txFloat wrap_headingRadian = circRad.Wrap(_headingRadian);
              // LogWarn << TX_VARS_NAME(from, _headingRadian) << TX_VARS_NAME(wrap, wrap_headingRadian);
              new_traj_point_ptr->set_theta(wrap_headingRadian);

              /*2. update s*/
              cur_s += cur_velocity * timeStep;

              /*3. save last info*/
              lastSampltPt = cur_pt_wgs84;
              last_traj_point.CopyFrom(*new_traj_point_ptr);
            } else {
              break;
            }
          }
          TrajInfo << TX_VARS(cur_s) << TX_VARS(cur_velocity) << TX_VARS(cur_acc) << TX_VARS(timeStep)
                   << TX_VARS(timeMgr.RelativeTime());
          LOG(INFO) << oss.str();
          if (refTraj.point_size() < 2) {
            refTraj.clear_point();
            sim_msg::TrajectoryPoint* new_traj_point_ptr = refTraj.add_point();
            new_traj_point_ptr->set_z(ego_alt);
            new_traj_point_ptr->set_t(timeMgr.AbsTime());
            new_traj_point_ptr->set_v(cur_velocity);
            new_traj_point_ptr->set_a(0.0);
            new_traj_point_ptr->set_s(0.0);

            Coord::txWGS84 cur_pt_wgs84 = RawVehicleCoord();
            new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
            new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
            const Base::txFloat _headingRadian = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetRadian();
            new_traj_point_ptr->set_theta(_headingRadian);

            sim_msg::TrajectoryPoint* new_clone_traj_point_ptr = refTraj.add_point();
            new_clone_traj_point_ptr->CopyFrom(*new_traj_point_ptr);
          }
#else
          for (Base::txSize idx = 0; idx < vec_ms_time_traj_point.size(); ++idx) {
            auto& ref_traj_sample = vec_ms_time_traj_point[idx];
            auto point_ptr = std::get<1>(ref_traj_sample);
            if ((point_ptr->s()) < traj_real_length) {
              Coord::txWGS84 cur_pt_wgs84;
              cur_pt_wgs84.FromENU(laneGeomPtr->GetLocalPos(point_ptr->s()));
              point_ptr->set_x(cur_pt_wgs84.Lon());
              point_ptr->set_y(cur_pt_wgs84.Lat());

              const Base::txFloat _headingRadian =
                  Utils::GetLaneAngleFromVectorOnENU(laneGeomPtr->GetLaneDir(point_ptr->s())).GetRadian();
              point_ptr->set_theta(_headingRadian);
            } else {
              point_ptr->set_a(0.0);
              point_ptr->set_v(0.0);
              point_ptr->set_s(traj_real_length);
              point_ptr->set_x(end_wgs84.Lon());
              point_ptr->set_y(end_wgs84.Lat());

              const Base::txFloat _headingRadian =
                  Utils::GetLaneAngleFromVectorOnENU(laneGeomPtr->GetLaneDir(traj_real_length)).GetRadian();
              point_ptr->set_theta(_headingRadian);
              // LogWarn << TX_VARS(_headingRadian);
            }
          }
#endif
        } else {
          LOG(FATAL) << "laneGeomPtr is nullptr.";
          return false;
        }
      } else if (1 == vec_sampling_traj_pts.size()) {
        sim_msg::TrajectoryPoint* new_traj_point_ptr = refTraj.add_point();
        new_traj_point_ptr->set_z(ego_alt);
        new_traj_point_ptr->set_t(timeMgr.AbsTime());
        new_traj_point_ptr->set_v(cur_velocity);
        new_traj_point_ptr->set_a(0.0);
        new_traj_point_ptr->set_s(0.0);

        Coord::txWGS84 cur_pt_wgs84 = vec_sampling_traj_pts.front().SamplingPt().ToWGS84();
        new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
        new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
        const Base::txFloat _headingRadian = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetRadian();
        new_traj_point_ptr->set_theta(_headingRadian);

        sim_msg::TrajectoryPoint* new_clone_traj_point_ptr = refTraj.add_point();
        new_clone_traj_point_ptr->CopyFrom(*new_traj_point_ptr);
      } else {
#if 1
        sim_msg::TrajectoryPoint* new_traj_point_ptr = refTraj.add_point();
        new_traj_point_ptr->set_z(ego_alt);
        new_traj_point_ptr->set_t(timeMgr.AbsTime());
        new_traj_point_ptr->set_v(cur_velocity);
        new_traj_point_ptr->set_a(0.0);
        new_traj_point_ptr->set_s(0.0);

        Coord::txWGS84 cur_pt_wgs84 = RawVehicleCoord();
        new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
        new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
        const Base::txFloat _headingRadian = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetRadian();
        new_traj_point_ptr->set_theta(_headingRadian);

        sim_msg::TrajectoryPoint* new_clone_traj_point_ptr = refTraj.add_point();
        new_clone_traj_point_ptr->CopyFrom(*new_traj_point_ptr);
#endif
      }
    }
    /*20230408*/ /*LOG(WARNING) << TX_VARS(timeMgr.PassTime()) << TX_VARS(refTraj.point_size());*/
    /*std::ostringstream oss; oss << timeMgr.AbsTime() << std::endl;
    for (const auto& refNode : refTraj.point())
    {
            oss << _StreamPrecision_ << TX_VARS_NAME(x, refNode.x()) << TX_VARS_NAME(y,refNode.y()) << TX_VARS_NAME(v,
    refNode.v()) << TX_VARS_NAME(t, refNode.t())<< std::endl;
    }
    LogWarn << oss.str();*/
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningVechicleElement::UpdateLocation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  RawVehicleCoord().FromWGS84(m_dynamic_location.position().x(), m_dynamic_location.position().y(),
                              m_dynamic_location.position().z());
  SyncPosition(timeMgr.PassTime());
  const Base::txVec2 vPos_Enu2D = RawVehicleCoord().ToENU().ENU2D();
  const Base::txVec2 last_EgoMassCenter = StableRawVehicleCoord().ENU2D();
#if 1
  const Base::txFloat o_dist = (vPos_Enu2D - last_EgoMassCenter).norm();
  LOG(INFO) << TX_VARS(timeMgr.PassTime()) << TX_VARS(o_dist);
  if (NonNull_Pointer(m_trajectory_ptr) && timeMgr.PassTime() > 0.0) {
    if (m_trajectory_ptr->GetLength() > 0.0) {
      Base::txFloat s = 0.0, l = 0.0;
      m_trajectory_ptr->xy2sl(RawVehicleCoord().ToENU().ENU2D(), s, l);
      const Base::txInt idx = m_trajectory_ptr->LocateIndexByDistance(s);
      Base::Info_Lane_t locInfo;
      Base::txFloat segment_s;
      if (m_trajectory_ptr->GetLocInfo(idx, locInfo, segment_s)) {
        /*20230408*/
        auto geom_info_ptr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(locInfo);
        if (NonNull_Pointer(geom_info_ptr)) {
          mLocation.LaneOffset();
          if (geom_info_ptr->xy2sl(RawVehicleCoord().ToENU().ENU2D(), mLocation.DistanceAlongCurve(), l)) {
            if (locInfo.IsOnLane()) {
              hadmap::txLanePtr nextLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(locInfo.onLaneUid);
              RelocateTracker(nextLanePtr, timeMgr.TimeStamp());
            } else {
              hadmap::txLaneLinkPtr nextLinkPtr =
                  HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(locInfo.onLinkFromLaneUid, locInfo.onLinkToLaneUid);
              if (NonNull_Pointer(nextLinkPtr)) {
                RelocateTracker(nextLinkPtr, timeMgr.TimeStamp());
              } else {
                LogWarn << "nextLinkPtr is nullptr.";
              }
            }
            mLocation.PosOnLaneCenterLinePos() = HdMap::HadmapCacheConCurrent::GetPos(locInfo, DistanceAlongCurve());
            mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetDir(locInfo, DistanceAlongCurve());
            /*20230408*/ LoopInfo << TX_VARS(locInfo) << TX_VARS_NAME(CenterPos, mLocation.PosOnLaneCenterLinePos())
                                  << TX_VARS(DistanceAlongCurve()) << TX_VARS(LaneOffset());
          } else {
            StopVehicle();
            LogWarn << "geom_info_ptr->xy2sl false." << TX_VARS(RawVehicleCoord());
          }
        } else {
          StopVehicle();
          LogWarn << "GetGeomInfoById failure." << TX_VARS(locInfo);
        }
      } else {
        StopVehicle();
        LogWarn << "m_trajectory_ptr->GetLocInfo failure." << TX_VARS(idx);
      }
      /*20230408*/ LoopInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS(s) << TX_VARS(l) << TX_VARS(idx);
    }
  } else {
    LogWarn << "m_trajectory_ptr is nullptr.";
  }
#endif

  Base::txVec2 yawDir(std::cos(m_dynamic_location.rpy().z()), std::sin(m_dynamic_location.rpy().z()));
  ComputeProjectionMat(vPos_Enu2D, (vPos_Enu2D + 2.0 * yawDir));
  mLocation.vLaneDir() = Base::txVec3(__East__(yawDir), __North__(yawDir), 0.0);

  mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());

  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)
