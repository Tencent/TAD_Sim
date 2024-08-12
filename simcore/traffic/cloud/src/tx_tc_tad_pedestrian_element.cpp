// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_tad_pedestrian_element.h"
#include "HdMap/tx_hashed_road.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_enum_def.h"
#include "tx_sim_point.h"
#include "tx_tc_gflags.h"
#define LogWarn LOG(WARNING)
const double SilentTime = 10.0;
TX_NAMESPACE_OPEN(TrafficFlow)
Base::txBool Cloud_PedestrianElement::InitializeOnCloud(const Base::txSysId pedId, const Base::Info_Lane_t& fromLocInfo,
                                                        const Base::txFloat from_s, const Base::Info_Lane_t& toLocInfo,
                                                        const Base::txFloat to_s) TX_NOEXCEPT {
  m_pedestrianType = _plus_(PEDESTRIAN_TYPE::human);
  m_pedestrianTypeStr = "human";
  m_pedestrianTypeId = 0;

  mIdentity.Id() = pedId;
  mIdentity.SysId() = CreateSysId(mIdentity.Id());

  mGeometryData.Length() = FLAGS_Pedestrian_Length;
  mGeometryData.Width() = FLAGS_Pedestrian_Width;
  mGeometryData.Height() = FLAGS_default_height_of_obstacle_pedestrian;
  mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::ePedestrian;
  mSilentMgr.Initialize(Id(), 0.0);
  mSilentMgr.ResetCounter(SilentTime, 0.0);
  auto start_geom_ptr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(fromLocInfo);
  auto end_geom_ptr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(toLocInfo);
  if (NonNull_Pointer(start_geom_ptr) && NonNull_Pointer(end_geom_ptr)) {
    m_start_pt.FromENU(start_geom_ptr->GetLocalPos(from_s));
    auto vLaneDir = start_geom_ptr->GetLaneDir(from_s);
    m_start_pt = Coord::ComputeLaneOffset(m_start_pt, vLaneDir, -1.0 * FLAGS_default_lane_width);

    m_end_pt.FromENU(end_geom_ptr->GetLocalPos(to_s));
    m_end_pt = Coord::ComputeLaneOffset(m_end_pt, vLaneDir, 0.5 * FLAGS_default_lane_width);

    hadmap::txLanePtr initLane = HdMap::HadmapCacheConCurrent::GetLaneForInit(
        m_start_pt.Lon(), m_start_pt.Lat(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset());
    if (NonNull_Pointer(initLane)) {
      mLocation.GeomCenter() = m_start_pt;
      mLocation.InitTracker(mIdentity.Id());
      RelocateTracker(initLane, 0.0);
      mLifeCycle.StartTime() = 0.0;
      mLifeCycle.EndTime() = 999.9;

      mKinetics.m_velocity = 0.0;
      // mLifeCycle.SetStart();

      GenerateWayPoints(m_start_pt, std::vector<std::pair<Base::txFloat, Base::txFloat>>(),
                        std::make_pair(m_end_pt.Lon(), m_end_pt.Lat()));

      GenerateSignalByLaneCache(fromLocInfo, from_s);
      return true;
    } else {
      LogWarn << "GetLane Failure." << TX_VARS(m_start_pt);
      return false;
    }
  } else {
    LogWarn << "GetGeomInfoById Failure." << TX_VARS(fromLocInfo) << TX_VARS(toLocInfo);
    return false;
  }
}

Base::txBool Cloud_PedestrianElement::Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (IsAlive()) {
    Base::txFloat const distance = (GetVelocity()) * timeMgr.RelativeTime();
    Base::txInt idx = 0;
    Base::txBool isStop = false;
    mLocation.GeomCenter() = m_polyline_traj.Move(distance, idx, m_vTargetDir, isStop);
    if (isStop) {
      // mKinetics.m_velocity = 0.0;
      mLifeCycle.ResetState();
      mKinetics.m_velocity = 0.0;
      mSilentMgr.ResetCounter(SilentTime, 0.0);
      GenerateWayPoints(m_start_pt, std::vector<std::pair<Base::txFloat, Base::txFloat>>(),
                        std::make_pair(m_end_pt.Lon(), m_end_pt.Lat()));
      LOG(INFO) << "[Pedestrian stop]" << TX_VARS(Id()) << TX_VARS(mSilentMgr.Duration())
                << TX_VARS(bool2lpsz(mLifeCycle.IsAlive()));
    } else {
      PublicPedestrians(timeMgr);
    }
  }
  return true;
}

Base::txBool Cloud_PedestrianElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                                     sim_msg::Traffic& trafficData) TX_NOEXCEPT {
  if (IsAlive()) {
    sim_msg::DynamicObstacle* tmpDynamicObstacleData = nullptr;
    {
#if USE_TBB
      tbb::mutex::scoped_lock lock(s_tbbMutex_outputTraffic);
#endif
      tmpDynamicObstacleData = trafficData.add_dynamicobstacles();
    }
    tmpDynamicObstacleData->set_id(Id());
    tmpDynamicObstacleData->set_t(timeMgr.TimeStamp());
    const hadmap::txPoint geom_center_gps = GetLocation().WGS84();
    tmpDynamicObstacleData->set_x(__Lon__(geom_center_gps));
    tmpDynamicObstacleData->set_y(__Lat__(geom_center_gps));
    tmpDynamicObstacleData->set_z(__Alt__(geom_center_gps));
#if USE_CustomModelImport
    tmpDynamicObstacleData->set_type(m_pedestrianTypeId);
#else  /*#USE_CustomModelImport*/
    tmpDynamicObstacleData->set_type(__enum2int__(PEDESTRIAN_TYPE, m_pedestrianType));
#endif /*#USE_CustomModelImport*/
    const Base::txFloat _headingRadian =
        Utils::DisplayerGetLaneAngleFromVector(m_vTargetDir).GetRadian(TX_MARK("SIM-4072"));
    tmpDynamicObstacleData->set_heading(_headingRadian);
    tmpDynamicObstacleData->set_length(GetLength());
    tmpDynamicObstacleData->set_width(GetWidth());
    tmpDynamicObstacleData->set_height(GetHeigth());
    tmpDynamicObstacleData->set_v(GetVelocity());
    tmpDynamicObstacleData->set_acc(0.0);
    tmpDynamicObstacleData->set_show_abs_velocity(GetVelocity());

    mGeometryData.ComputePolygon(GetLocation().ToENU(), m_vTargetDir);
    /*FillingSpatialQuery();*/
    SaveStableState();
  }
  return true;
}

void Cloud_PedestrianElement::SaveStableState() TX_NOEXCEPT {
  mLocation.LastGeomCenter() = mLocation.GeomCenter().ToENU();
  mLocation.LastRearAxleCenter() = mLocation.RearAxleCenter().ToENU();
  mLocation.LastOnLanePos() = mLocation.PosOnLaneCenterLinePos();
  mLocation.LastLaneDir() = mLocation.vLaneDir();
  mLocation.LastLaneInfo() = mLocation.LaneLocInfo();
  mLocation.LastHeading() = mLocation.heading();
  mKinetics.LastVelocity() = GetVelocity();
  mKinetics.LastAcceleration() = GetAcc();
}

void Cloud_PedestrianElement::GenerateWayPoints(
    const Coord::txWGS84& startPos, const std::vector<std::pair<Base::txFloat, Base::txFloat>>& midPoint,
    const std::pair<Base::txFloat, Base::txFloat>& refEndPoint) TX_NOEXCEPT {
  std::vector<Coord::txWGS84> vec_waypoints;
  vec_waypoints.emplace_back(Coord::txWGS84().FromWGS84(startPos.Lon(), startPos.Lat(), FLAGS_default_altitude));

  for (const auto& refWayPoint : midPoint) {
    vec_waypoints.emplace_back(
        Coord::txWGS84().FromWGS84(refWayPoint.first, refWayPoint.second, FLAGS_default_altitude));
  }

  if (Math::isNotZero(refEndPoint.first) && Math::isNotZero(refEndPoint.second)) {
    vec_waypoints.emplace_back(
        Coord::txWGS84().FromWGS84(refEndPoint.first, refEndPoint.second, FLAGS_default_altitude));
  }

  LogWarn << "GenerateWayPoints : " << TX_VARS_NAME(PedeId, Id()) << TX_VARS_NAME(waypoint_size, vec_waypoints.size());
  for (const auto& refPt : vec_waypoints) {
    LogWarn << refPt;
  }

  m_polyline_traj.Init(vec_waypoints);
#if __TX_Mark__("SIM-4754")
  Base::txInt nonuse_idx = 0;
  Base::txBool nonuse_isStop = false;
  m_polyline_traj.Move(0.0, nonuse_idx, m_vTargetDir, nonuse_isStop);
#endif /*__TX_Mark__("SIM-4754")*/
}

void Cloud_PedestrianElement::GenerateSignalByLaneCache(const Base::Info_Lane_t from_locInfo,
                                                        const Base::txFloat from_s) TX_NOEXCEPT {
  hadmap::txLanes lanes;
  lanes = HdMap::HadmapCacheConCurrent::GetLanesUnderSection(
      std::make_pair(from_locInfo.onLaneUid.roadId, from_locInfo.onLaneUid.sectionId));

  for (auto itr = std::begin(lanes); itr != std::end(lanes);) {
    if (CallFail(Utils::IsLaneValidDriving(*itr))) {
      LOG(INFO) << TX_VARS_NAME(erase_lane, Utils::ToString((*itr)->getTxLaneId()));
      itr = lanes.erase(itr);
    } else {
      ++itr;
    }
  }
  GenerateSignalByLaneCache_Internal(lanes, from_s);
}

void Cloud_PedestrianElement::GenerateSignalByLaneCache_Internal(const hadmap::txLanes& lanes,
                                                                 const Base::txFloat from_s) TX_NOEXCEPT {
  mControlOrthogonalNodeMap.clear();
  for (const auto& curLanePtr : lanes) {
    if (NonNull_Pointer(curLanePtr)) {
      const auto curLaneUid = curLanePtr->getTxLaneId();
      const HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(curLaneUid);
      const Base::txFloat start_inv_s = from_s;
      HashedLaneInfo start_hash_node =
          Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, start_inv_s, Base::Info_Lane_t(curLaneUid));
      HashedLaneInfoOrthogonalListPtr start_Orthogonal_ptr =
          HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(start_hash_node);
      HashedLaneInfoOrthogonalList::SearchOrthogonalListPost(
          start_Orthogonal_ptr, FLAGS_pedestrian_occurrence_dist * 2.0, mControlOrthogonalNodeMap);
    }
  }
}

void Cloud_PedestrianElement::PublicPedestrians(const Base::TimeParamManager&) TX_NOEXCEPT {
  HashedLaneInfoOrthogonalList::VehicleContainer refVehicleContainer;
  for (const auto& ref_node_ptr : mControlOrthogonalNodeMap) {
    HashedLaneInfoOrthogonalList::VehicleContainer resContainer;
    ref_node_ptr.second->GetRegisterVehicles(resContainer);
    refVehicleContainer.insert(resContainer.begin(), resContainer.end());
  }
  const Base::txVec3 signalPos3d = GetLocation().ToENU().ENU();
  Base::IPedestrianElementPtr self_pedestrian_ptr = std::dynamic_pointer_cast<Base::IPedestrianElement>(GetBasePtr());
  for (auto& ref_id_elemPtr : refVehicleContainer) {
    const Base::txVec3 vehiclePos3d = ref_id_elemPtr.second->StablePositionOnLane().ToENU().ENU();
    const Base::txFloat half_veh_length = ref_id_elemPtr.second->GetLength() * 0.5;
    Base::txVec3 vDir = signalPos3d - vehiclePos3d;
    const Base::txFloat curDistance = vDir.norm();
    vDir.normalize();
    const Base::txFloat fAngle = Math::VectorRadian3D(ref_id_elemPtr.second->StableLaneDir(), vDir);
    const Base::txFloat distance_sub_half_length = curDistance * sin(Math::PI * 0.5 - fAngle) - half_veh_length * 3.0;
    TX_MARK("One signal controls multiple lanes.");
    if (distance_sub_half_length > 0.0) {
      ref_id_elemPtr.second->SetCurFrontPedestrianPtr(self_pedestrian_ptr);
      ref_id_elemPtr.second->SetDistanceToPedestrian(distance_sub_half_length);
      // LOG(WARNING) << TX_VARS_NAME(pedId, Id()) << TX_VARS_NAME(elemID, ref_id_elemPtr.second->Id()) <<
      // TX_VARS(distance_sub_half_length);
    }
  }
}

void Cloud_PedestrianElement::GetInflunceVehicles(HashedLaneInfoOrthogonalList::VehicleContainer& refVehicleContainer)
    TX_NOEXCEPT {
  for (const auto& ref_node_ptr : mControlOrthogonalNodeMap) {
    HashedLaneInfoOrthogonalList::VehicleContainer resContainer;
    ref_node_ptr.second->GetRegisterVehicles(resContainer);
    refVehicleContainer.insert(resContainer.begin(), resContainer.end());
  }
}

Base::txBool Cloud_PedestrianElement::CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (CallFail(IsStart())) {
    mSilentMgr.Decrement(timeMgr.RelativeTime());
    if (mSilentMgr.Expired()) {
      HashedLaneInfoOrthogonalList::VehicleContainer refVehicleContainer;
      GetInflunceVehicles(refVehicleContainer);

      for (const auto& refElemPtr : refVehicleContainer) {
        if (NonNull_Pointer(refElemPtr.second) &&
            _plus_(Base::Enums::ElementType::TAD_Ego) == (refElemPtr.second->Type())) {
          const Base::txFloat dist =
              Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(GetLocation(), refElemPtr.second->GetLocation());

          if (dist > FLAGS_pedestrian_occurrence_dist) {
            LOG(INFO) << "[PEDESTRIAN] START " << TX_VARS(Id()) << TX_VARS(GetLocation())
                      << TX_VARS(refElemPtr.second->GetLocation());
            mLifeCycle.SetStart();
            mKinetics.m_velocity = 0.0;
            break;
          }
        }
      }
    }
  } else if (mKinetics.m_velocity <= 0.0) {
    HashedLaneInfoOrthogonalList::VehicleContainer refVehicleContainer;
    GetInflunceVehicles(refVehicleContainer);
    for (const auto& refElemPtr : refVehicleContainer) {
      if (NonNull_Pointer(refElemPtr.second) &&
          _plus_(Base::Enums::ElementType::TAD_Ego) == (refElemPtr.second->Type())) {
        const Base::txFloat dist =
            Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(GetLocation(), refElemPtr.second->GetLocation());

        if ((dist < FLAGS_pedestrian_occurrence_dist) && (dist > (0.75 * FLAGS_pedestrian_occurrence_dist))) {
          mKinetics.m_velocity = FLAGS_pedestrian_velocity;
          break;
        }
      }
    }
  }
  return IsStart();
}

TX_NAMESPACE_CLOSE(TrafficFlow)
