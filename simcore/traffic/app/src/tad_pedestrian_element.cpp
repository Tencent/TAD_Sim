// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_pedestrian_element.h"
#include "tad_pedestrian_motion_event.h"
#include "tx_protobuf_utils.h"
#include "tx_scene_loader.h"
#include "tx_sim_point.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"
#include "tx_timer_on_cpu.h"
#if USE_RTree
#  include "tx_spatial_query.h"
#endif
#include <boost/algorithm/string.hpp> /*string splite*/
#include <sstream>
#include "tx_frame_utils.h"
#include "tx_hadmap_utils.h"
#include "tx_obb.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Pedestrian) << _StreamPrecision_
#define LogWarn LOG(WARNING) << _StreamPrecision_

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool TAD_PedestrianElement::CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (CallFail(IsStart())) {
    // 根据开始时间检查此刻元素是否需要启动
    if (mLifeCycle.StartTime() >= 0.0 && timeMgr.AbsTime() >= mLifeCycle.StartTime()) {
      mLifeCycle.SetStart();
      OnStart(timeMgr);
    }
  }
  return IsStart();
}

void TAD_PedestrianElement::GenerateWayPoints(const Coord::txWGS84& startPos,
                                              const std::vector<std::pair<Base::txFloat, Base::txFloat> >& midPoint,
                                              const std::pair<Base::txFloat, Base::txFloat>& refEndPoint) TX_NOEXCEPT {
  std::vector<Coord::txWGS84> vec_waypoints;
  // 添加起点
  vec_waypoints.emplace_back(Coord::txWGS84().FromWGS84(startPos.Lon(), startPos.Lat()));

  // 添加途径点
  for (const auto& refWayPoint : midPoint) {
    vec_waypoints.emplace_back(Coord::txWGS84().FromWGS84(refWayPoint.first, refWayPoint.second));
  }

  // 添加终点
  if (Math::isNotZero(refEndPoint.first) && Math::isNotZero(refEndPoint.second)) {
    vec_waypoints.emplace_back(Coord::txWGS84().FromWGS84(refEndPoint.first, refEndPoint.second));
  }

#if __TX_Mark__("compute altitude")
  for (auto itr = std::begin(vec_waypoints); itr != std::end(vec_waypoints);) {
    auto& refPt = *itr;
    Base::Info_Lane_t LaneLocInfo;
    Base::txFloat s = 0.0;
    // 若查询到道路位置信息
    if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(refPt, LaneLocInfo,
                                                                                                  s)) {
      if (CallSucc(LaneLocInfo.IsOnLane())) {
        // 在车道上时，计算高度
        hadmap::txLanePtr lanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(LaneLocInfo.onLaneUid);
        if (NonNull_Pointer(lanePtr) && NonNull_Pointer(lanePtr->getGeometry())) {
          refPt.Alt(TX_MARK("ID103977575")) = __Alt__(lanePtr->getGeometry()->getPoint(s));
          /*refPt.FromWGS84(lanePtr->getGeometry()->getPoint(s));*/
        } else {
          LogWarn << "pedestrian altitude error." << TX_VARS(LaneLocInfo);
        }
      } else {
        // 在车道link时，计算高度
        hadmap::txLaneLinkPtr linkPtr =
            HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(LaneLocInfo.onLinkFromLaneUid, LaneLocInfo.onLinkToLaneUid);
        if (NonNull_Pointer(linkPtr) && NonNull_Pointer(linkPtr->getGeometry())) {
          refPt.Alt(TX_MARK("ID103977575")) = __Alt__(linkPtr->getGeometry()->getPoint(s));
          /*refPt.FromWGS84(linkPtr->getGeometry()->getPoint(s));*/
        } else {
          LogWarn << "pedestrian altitude error." << TX_VARS(LaneLocInfo);
        }
      }
      ++itr;
    } else {
      itr = vec_waypoints.erase(itr);
      LogWarn << TX_VARS_NAME(PedId, Id()) << " way point absorb map error " << refPt.StrWGS84();
    }
  }
#endif /*__TX_Mark__("compute altitude")*/

  // 打印waypoint日志
  LogInfo << "GenerateWayPoints : " << TX_VARS_NAME(PedeId, Id()) << TX_VARS_NAME(waypoint_size, vec_waypoints.size());
  for (const auto& refPt : vec_waypoints) {
    LogInfo << refPt;
  }

  // 初始化m_polyline_traj
  m_polyline_traj.Init(vec_waypoints);
#if __TX_Mark__("SIM-4754")
  Base::txInt nonuse_idx = 0;
  Base::txBool nonuse_isStop = false;
  m_polyline_traj.Move(0.0, nonuse_idx, m_vTargetDir, nonuse_isStop);
#endif /*__TX_Mark__("SIM-4754")*/
}

Base::txBool TAD_PedestrianElement::Initialize_Kinetics(Base::ISceneLoader::IPedestriansViewerPtr _elemAttrViewPtr)
    TX_NOEXCEPT {
  if (NonNull_Pointer(_elemAttrViewPtr)) {
    // 初始化开始的动力学信息
    mKinetics.m_velocity = _elemAttrViewPtr->start_v();
    mKinetics.LastVelocity() = mKinetics.m_velocity;
    mKinetics.m_velocity_max = _elemAttrViewPtr->max_v();
    mKinetics.velocity_desired = mKinetics.m_velocity_max;
    mKinetics.raw_velocity_max = mKinetics.m_velocity_max;
    return true;
  } else {
    LogWarn << "Initialize_Kinetics Error. " << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr));
    return false;
  }
}

Base::txBool TAD_PedestrianElement::Initialize(Base::ISceneLoader::IViewerPtr _absAttrView,
                                               Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 判断输入的参数指针有效
  if (NonNull_Pointer(_absAttrView) && NonNull_Pointer(_sceneLoader) && CallSucc(_absAttrView->IsInited())) {
    // 元素属性视图不为空
    _elemAttrViewPtr = std::dynamic_pointer_cast<Base::ISceneLoader::IPedestriansViewer>(_absAttrView);
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      m_start_angle_degree = _elemAttrViewPtr->start_angle();
#if __TX_Mark__("scene event")
      m_use_scene_event_1_0_0_0 = true;
      // 获取场景事件集合
      std::vector<ISceneEventViewerPtr> all_scene_event = _sceneLoader->GetSceneEventVec();
      std::map<txSysId, ISceneEventViewerPtr> map_id2ptr;
      // 遍历事件集合，转为KV map
      for (auto ptr : all_scene_event) {
        if (NonNull_Pointer(ptr)) {
          map_id2ptr[ptr->id()] = ptr;
        }
      }

      // 获取当前元素的事件
      const auto event_id_vec = _elemAttrViewPtr->eventId();
      for (const auto evId : event_id_vec) {
        // 在场景事件中找到当前元素事件
        if (_Contain_(map_id2ptr, evId)) {
          // 记录涉及的场景事件指针
          m_vec_scene_event.emplace_back(map_id2ptr.at(evId));
        } else {
          LogWarn << "can not find scene event " << TX_VARS(evId);
        }
      }
#endif /*__TX_Mark__("scene event")*/
      // 元素的路线数据处理
      _elemRouteViewPtr = _sceneLoader->GetRouteData(_elemAttrViewPtr->routeID());
      if (Null_Pointer(_elemRouteViewPtr)) {
        LogWarn << "Get/Set Route Failure. Route Id = " << (_elemAttrViewPtr->routeID());
        return false;
      }
      LogInfo << "[XOSC_SCENE_LOADER] route : " << (_elemRouteViewPtr->Str());

      // 设置身份属性
      m_pedestrianType =
          __lpsz2enum__(PEDESTRIAN_TYPE,
                        _elemAttrViewPtr->type()
                            .c_str()) /*Utils::String2ElementType(_elemAttrViewPtr->type(), +PEDESTRIAN_TYPE::human)*/;
      mIdentity.Id() = _elemAttrViewPtr->id();
      mIdentity.SysId() = CreateSysId(mIdentity.Id());

      m_pedestrianType =
          __lpsz2enum__(PEDESTRIAN_TYPE,
                        _elemAttrViewPtr->type()
                            .c_str()) /*Utils::String2ElementType(_elemAttrViewPtr->type(), +PEDESTRIAN_TYPE::human)*/;

#if USE_CustomModelImport
      m_pedestrianTypeStr = _elemAttrViewPtr->type().c_str();
      if (Base::CatalogCache::Query_Pedestrian_Catalog(m_pedestrianTypeStr)) {
        m_pedestrianTypeId = Base::CatalogCache::PEDESTRIAN_TYPE(m_pedestrianTypeStr);
        InitPedestrianCatalog(m_pedestrianTypeStr);
        mGeometryData.Length() = Catalog_Length();
        mGeometryData.Width() = Catalog_Width();
        mGeometryData.Height() = Catalog_Height();
        LOG_IF(INFO, FLAGS_LogLevel_Catalog) << "[Catalog_query][success][ped] " << TX_VARS(Id())
                                             << TX_VARS_NAME(pedestrianType, (_elemAttrViewPtr->type()))
                                             << TX_VARS(GetLength()) << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
        // for moto / bike and so on
      } else if (Base::CatalogCache::Query_Vehicle_Catalog(m_pedestrianTypeStr)) {
        m_pedestrianTypeId = Base::CatalogCache::VEHICLE_TYPE(m_pedestrianTypeStr);
        InitVehicleCatalog(m_pedestrianTypeStr);
        mGeometryData.Length() = Catalog_Length();
        mGeometryData.Width() = Catalog_Width();
        mGeometryData.Height() = Catalog_Height();
        LOG_IF(INFO, FLAGS_LogLevel_Catalog) << "[Catalog_query][success][ped] " << TX_VARS(Id())
                                             << TX_VARS_NAME(pedestrianType, (_elemAttrViewPtr->type()))
                                             << TX_VARS(GetLength()) << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
      } else {
        m_pedestrianTypeId = -1;
        mGeometryData.Length() = FLAGS_Pedestrian_Length;
        mGeometryData.Width() = FLAGS_Pedestrian_Width;
        mGeometryData.Height() = FLAGS_default_height_of_obstacle_pedestrian;
        LogWarn << "[Catalog_query][failure][ped] " << TX_VARS(Id())
                << TX_VARS_NAME(pedestrianType, (_elemAttrViewPtr->type())) << TX_VARS(GetLength())
                << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
      }
#else  /*#USE_CustomModelImport*/
      if (CallFail(InitCatalog(m_pedestrianType))) {
        mGeometryData.Length() = FLAGS_Pedestrian_Length;
        mGeometryData.Width() = FLAGS_Pedestrian_Width;
        mGeometryData.Height() = FLAGS_default_height_of_obstacle_pedestrian;
        LogWarn << "[Catalog_query][failure][ped] " << TX_VARS(Id())
                << TX_VARS_NAME(pedestrianType, (_elemAttrViewPtr->type())) << TX_VARS(GetLength())
                << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
      } else {
        // 长宽高从catalog中获取
        mGeometryData.Length() = Catalog_Length();
        mGeometryData.Width() = Catalog_Width();
        mGeometryData.Height() = Catalog_Height();
        LOG_IF(INFO, FLAGS_LogLevel_Catalog) << "[Catalog_query][success][ped] " << TX_VARS(Id())
                                             << TX_VARS_NAME(pedestrianType, (_elemAttrViewPtr->type()))
                                             << TX_VARS(GetLength()) << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
      }
#endif /*#USE_CustomModelImport*/

      mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::ePedestrian;

      // 根据身份id获取关联的事件视图
      _elemPedestriansEvent_time_velocity_ViewerPtr = _sceneLoader->GetPedestriansEventData_TimeEvent(mIdentity.Id());
      _elemPedestriansEvent_event_velocity_ViewerPtr =
          _sceneLoader->GetPedestriansEventData_VelocityEvent(mIdentity.Id());
      return Initialize_Location();
    } else {
      LogWarn << "Get Route Failure. Route Id = " << (_elemAttrViewPtr->routeID());
      return false;
    }
  } else {
    LogWarn << "Param Error." << TX_COND_NAME(_absAttrView, NonNull_Pointer(_absAttrView))
            << TX_COND_NAME(_sceneLoader, NonNull_Pointer(_sceneLoader))
            << TX_COND_NAME(_absAttrView_IsInited, CallSucc(_absAttrView->IsInited()));
    return false;
  }
}

Base::txBool TAD_PedestrianElement::Initialize_Location() TX_NOEXCEPT {
  // 行人的开始车道id
  const Base::txLaneID startLaneId = _elemAttrViewPtr->laneID();
  const txBool bLocateOnLaneLink = (0 <= startLaneId);
  // 是否在lane上
  if (CallFail(bLocateOnLaneLink)) {
    TX_MARK("on lane");
    // 车道上位置初始化，从缓存中根据经纬度等获取车道指针
    hadmap::txLanePtr initLane_roadId_sectionId =
        HdMap::HadmapCacheConCurrent::GetLaneForInit(_elemRouteViewPtr->startLon(), _elemRouteViewPtr->startLat(),
                                                     mLocation.DistanceAlongCurve(), mLocation.LaneOffset());
    if (NonNull_Pointer(initLane_roadId_sectionId)) {
      Base::txLaneUId _laneUid = initLane_roadId_sectionId->getTxLaneId();
      LogInfo << "initLane : LaneUid = " << Utils::ToString(_laneUid) << TX_VARS(mLocation.DistanceAlongCurve())
              << TX_VARS(mLocation.LaneOffset());
      if ((_elemAttrViewPtr->laneID()) < 0) {
        _laneUid.laneId = _elemAttrViewPtr->laneID();
      }

      // 根据laneuid获取初始化时的车道
      hadmap::txLanePtr initLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(_laneUid);
      if (Null_Pointer(initLane)) {
        // 车道不存在
        LogWarn << "GetLane Failure." << Utils::ToString(_laneUid);
        return false;
      }
      mLocation.LaneOffset() = _elemAttrViewPtr->l_offset();
      if (HdMap::HadmapCacheConCurrent::GetLonLatFromSTonLaneWithOffset(
              initLane->getTxLaneId(), GenerateST(_elemAttrViewPtr->start_s(), mLocation.DistanceAlongCurve()),
              mLocation.LaneOffset(), RawVehicleCoord())) {
        // 初始化pedestrain的在lanlink时位置信息
        mLocation.InitTracker(mIdentity.Id());
        // 重定位跟踪器
        RelocateTracker(initLane, 0.0);
        // 设置生命周期相关信息
        mLifeCycle.StartTime() = _elemAttrViewPtr->start_t();
        mLifeCycle.EndTime() = _elemAttrViewPtr->end_t();

        LogInfo << "Pedestrian Init Success. " << Str();
        if (CallFail(_elemAttrViewPtr->hadDirection())) {
          Initialize_Kinetics(_elemAttrViewPtr);
          m_pedestrian_behavior = PedestrianBehavior::WayPoints;
          GenerateWayPoints(RawVehicleCoord(), _elemRouteViewPtr->midPoints(),
                            std::make_pair(_elemRouteViewPtr->endLon(), _elemRouteViewPtr->endLat()));
        }
        // 计算包围盒中心
        mGeometryData.ComputePolygon(GetLocation().ToENU(), mLocation.vLaneDir());
        // 位置同步
        SyncPosition(0.0);
        return true;
      } else {
        LogWarn << "GetLonLatFromSTonLane Failure.";
        return false;
      }
    } else {
      LogWarn << "GetLane Failure." << TX_VARS_NAME(route_lon, _elemRouteViewPtr->startLon())
              << TX_VARS_NAME(route_lat, _elemRouteViewPtr->startLat());
      return false;
    }
  } else {  // 不在lane上
    TX_MARK("on lanelink");
    Base::txFloat distancePedal_unuse = 0.0;
    // 获取初始时的lanlink指针
    hadmap::txLaneLinkPtr initLaneLinkptr = HdMap::HadmapCacheConCurrent::GetLaneLinkForInit(
        _elemRouteViewPtr->startLon(), _elemRouteViewPtr->startLat(), mLocation.DistanceAlongCurve(),
        distancePedal_unuse, mLocation.LaneOffset());

    if (NonNull_Pointer(initLaneLinkptr)) {
      LogWarn << "initLaneLink : linkId = " << TX_VARS_NAME(fromLaneUid, initLaneLinkptr->fromTxLaneId())
              << TX_VARS_NAME(toLaneUid, initLaneLinkptr->toTxLaneId()) << TX_VARS(mLocation.DistanceAlongCurve())
              << TX_VARS(mLocation.LaneOffset());

      // 根据经纬度从缓存中查询lanlink
      if (HdMap::HadmapCacheConCurrent::Get_LAL_LaneLink_By_S(
              Base::Info_Lane_t(initLaneLinkptr->getId(), initLaneLinkptr->fromTxLaneId(),
                                initLaneLinkptr->toTxLaneId()),
              mLocation.DistanceAlongCurve(), mLocation.PosOnLaneCenterLinePos())) {
        // 初始化pedestrain的在lanlink时位置信息
        mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
        mLocation.vLaneDir() =
            HdMap::HadmapCacheConCurrent::GetLaneLinkDir(initLaneLinkptr, mLocation.DistanceAlongCurve());
        // mLocation.vPos() = ComputeLaneOffset(mLocation.PosOnLaneCenterLinePos(), mLocation.vLaneDir(),
        // mLocation.LaneOffset());
        // 设置位置，生命周期，重定位跟踪器等信息
        RawVehicleCoord().FromWGS84(_elemRouteViewPtr->startLon(), _elemRouteViewPtr->startLat());
        mLocation.InitTracker(mIdentity.Id());
        RelocateTracker(initLaneLinkptr, 0.0);
        mLifeCycle.StartTime() = _elemAttrViewPtr->start_t();
        mLifeCycle.EndTime() = _elemAttrViewPtr->end_t();

        LogInfo << "Pedestrian Init Success. " << Str();
        // 如果获取方向失败
        if (CallFail(_elemAttrViewPtr->hadDirection())) {
          // 初始化kinetics
          Initialize_Kinetics(_elemAttrViewPtr);
          m_pedestrian_behavior = PedestrianBehavior::WayPoints;
          // 生成waypoints
          GenerateWayPoints(RawVehicleCoord(), _elemRouteViewPtr->midPoints(),
                            std::make_pair(_elemRouteViewPtr->endLon(), _elemRouteViewPtr->endLat()));
        }
        // 位置同步
        SyncPosition(0.0);
        return true;
      } else {
        LogWarn << "Get_LAL_LaneLink_By_S Failure.";
        return false;
      }
    } else {
      LogWarn << "GetTxLaneLinkPtr Failure." << TX_VARS_NAME(startLon, _elemRouteViewPtr->startLon())
              << TX_VARS_NAME(startLat, _elemRouteViewPtr->startLat());
      return false;
    }
  }
}

Base::txBool TAD_PedestrianElement::Update_Kinetics(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  // 如果元素存活且未停止
  if (IsAlive() && !IsStop()) {
    /*mKinetics.m_velocity_last = mKinetics.m_velocity;*/
    // 更新障碍物元素的速度
    mKinetics.m_velocity = mKinetics.m_velocity + mKinetics.m_acceleration * timeMgr.RelativeTime();
    // 限制障碍物元素的速度
    mKinetics.LimitVelocity();
  } else {
    mKinetics.m_velocity = 0.0;
  }
  return true;
}

Base::txBool TAD_PedestrianElement::Pre_UpdatePedestrian(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  ComputeLocalCoordTransMatInv(StableGeomCenter().ENU2D(), StableLaneDir());
  // 行人元素存活
  if (IsAlive()) {
    // 更新kinetics
    Update_Kinetics(timeMgr);
    // 如果元素具有加速度结束条件
    if (CallSucc(HasAccEndCondition())) {
      // 获取加速度结束条件
      const AccEndConditionManager accEndCond = GetAccEndCondition();
      // 如果加速度结束条件类型为时间
      if ((_plus_(Base::ISceneLoader::acc_invalid_type::eTime)) == accEndCond.m_endCondition.m_type) {
        const Base::txFloat remainTime = accEndCond.m_remainingTime - timeMgr.RelativeTime();
        // 如果剩余时间大于0
        if (remainTime > 0.0) {
          // 更新加速度结束条件的剩余时间
          SetAccEndConditionRemainTime(remainTime);
        } else {
          // 将加速度设置为0
          mKinetics.m_acceleration = 0.0;
          ClearAccEndCondition(timeMgr);
        }
        // 如果加速度结束条件类型为速度
      } else if ((_plus_(Base::ISceneLoader::acc_invalid_type::eVelocity)) == accEndCond.m_endCondition.m_type) {
        // 如果目标速度小于等于当前速度
        if (accEndCond.m_happenVelocity <= accEndCond.m_targetVelocity &&
            GetVelocity() >= accEndCond.m_targetVelocity) {
          // 将加速度设置为0
          mKinetics.m_acceleration = 0.0;
          mKinetics.m_velocity = accEndCond.m_targetVelocity;
          ClearAccEndCondition(timeMgr);
          // 如果目标速度大于等于当前速度
        } else if (accEndCond.m_happenVelocity >= accEndCond.m_targetVelocity &&
                   GetVelocity() <= accEndCond.m_targetVelocity) {
          mKinetics.m_acceleration = 0.0;
          // 将速度设置为目标速度
          mKinetics.m_velocity = accEndCond.m_targetVelocity;
          ClearAccEndCondition(timeMgr);
        }
      }
    }
    return true;
  } else {
    return IsAlive();
  }
}

Base::txBool TAD_PedestrianElement::Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  // for ComputeHeading func.
  m_sim_step_ms = Utils::SecondToMillisecond(timeMgr.RelativeTime());

  if (PedestrianBehavior::Direction == m_pedestrian_behavior) {
    // 行人活跃且速度不为0
    if (IsAlive() && Math::isNotZero(GetVelocity())) {
      Base::txFloat const distance = (GetVelocity()) * timeMgr.RelativeTime();

      // 更新并同步元素位置
      RawVehicleCoord().TranslateLocalPos(__East__(m_vTargetDir) * distance, __North__(m_vTargetDir) * distance);
      SyncPosition(timeMgr.PassTime());
      return true;
    } else {
      return true;
    }
  } else {
    // 如果行人元素的行为类型不是方向行为，则执行预更新操作
    Pre_UpdatePedestrian(timeMgr);
    Base::txFloat const distance = (GetVelocity()) * timeMgr.RelativeTime();
    Base::txInt idx = 0;
    Base::txBool isStop = false;
    // 更新同步行人位置
    RawVehicleCoord() = m_polyline_traj.Move(distance, idx, m_vTargetDir, isStop);
    SyncPosition(timeMgr.PassTime());
    // 如果元素停止，则将速度设置为0
    if (isStop) {
      mKinetics.m_velocity = 0.0;
    }
    // LOG(WARNING) << TXST_TRACE_VARIABLES_NAME(pede_pos, pImpl->m_Pos);
    return true;
  }
}

Base::txBool TAD_PedestrianElement::Release() TX_NOEXCEPT { return true; }

Base::txBool TAD_PedestrianElement::IsTransparentObject() const TX_NOEXCEPT {
#if USE_CustomModelImport
  return IsTransparentObject(m_pedestrianTypeStr);
#else  /*#USE_CustomModelImport*/
  return IsTransparentObject(m_pedestrianType);
#endif /*#USE_CustomModelImport*/
}

Base::txBool TAD_PedestrianElement::IsTransparentObject(const Base::Enums::PEDESTRIAN_TYPE& _ped_type) TX_NOEXCEPT {
  // 判断障碍物元素类型是否为港口吊车
  return (_plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_001) == _ped_type) ||
         (_plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_002) == _ped_type) ||
         (_plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_002_0_0) == _ped_type) ||
         (_plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_002_0_2) == _ped_type) ||
         (_plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_002_0_5) == _ped_type) ||
         (_plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_002_1_0) == _ped_type) ||
         (_plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_002_5_0) == _ped_type) ||
         (_plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_003) == _ped_type) ||
         (_plus_(Base::Enums::PEDESTRIAN_TYPE::Port_Crane_004) == _ped_type);
}

Base::txBool TAD_PedestrianElement::IsTransparentObject(const Base::txString& _ped_type) TX_NOEXCEPT {
  return ("Port_Crane_001" == _ped_type) || ("Port_Crane_002" == _ped_type) || ("Port_Crane_002_0_0" == _ped_type) ||
         ("Port_Crane_002_0_2" == _ped_type) || ("Port_Crane_002_0_5" == _ped_type) ||
         ("Port_Crane_002_1_0" == _ped_type) || ("Port_Crane_002_5_0" == _ped_type) ||
         ("Port_Crane_003" == _ped_type) || ("Port_Crane_004" == _ped_type);
}

Base::txBool TAD_PedestrianElement::FillingSpatialQuery() TX_NOEXCEPT {
  // 如果障碍物元素存活且不是透明对象
  if (IsAlive() && CallFail(IsTransparentObject())) {
    // 获取障碍物元素的多边形数据
    const auto& refPolygon = mGeometryData.Polygon();
    // 如果多边形数据的大小等于OBB2D的多边形大小
    if (Geometry::OBB2D::PolygonSize == refPolygon.size()) {
      // 将障碍物元素插入到空间查询中
      Geometry::SpatialQuery::RTree2D::InsertBulk(refPolygon[0], refPolygon[1], refPolygon[2], refPolygon[3],
                                                  GetBasePtr());
      return true;
    } else {
      LOG(WARNING) << "m_vecPolygon.size = " << refPolygon.size();
      return false;
    }
  } else {
    return false;
  }
}

void TAD_PedestrianElement::SaveStableState() TX_NOEXCEPT {
  // 保存障碍物元素的几何中心坐标
  mLocation.LastGeomCenter() = mLocation.GeomCenter().ToENU();
  mLocation.LastRearAxleCenter() = mLocation.RearAxleCenter().ToENU();
  // 保存障碍物元素在车道上的位置
  mLocation.LastOnLanePos() = mLocation.PosOnLaneCenterLinePos();
  mLocation.LastLaneDir() = mLocation.vLaneDir();
  mLocation.LastLaneInfo() = mLocation.LaneLocInfo();
  // 保存障碍物元素的航向角
  mLocation.LastHeading() = mLocation.heading();
  mKinetics.LastVelocity() = GetVelocity();
  mKinetics.LastAcceleration() = GetAcc();
}

Base::txFloat TAD_PedestrianElement::ComputeHeading(const txFloat _passTime_s) TX_NOEXCEPT {
  const Base::txFloat duration_ms = Utils::SecondToMillisecond(FLAGS_TrajStartAngleDuration);
  if (m_start_angle_action_duration_ms < duration_ms && m_start_angle_degree >= 0.0) {
    if (StableVelocity() > 0.0) {
      m_start_angle_action_duration_ms += m_sim_step_ms;
    }
    const Base::txFloat _headingRadian =
        Utils::DisplayerGetLaneAngleFromVector(m_vTargetDir).GetRadian(TX_MARK("SIM-4072"));
    // const Base::txFloat _zeroRadian = _headingRadian;
    // Unit::CircleRadian circRad(_zeroRadian - Unit::CircleRadian::Half_PI(), _zeroRadian +
    // Unit::CircleRadian::Half_PI(), _zeroRadian);
    Base::txFloat _start_angle_Radian = Math::Degrees2Radians(m_start_angle_degree);
    if (std::abs(_headingRadian - _start_angle_Radian) > Math::PI) {
      // LOG(WARNING) << TX_VARS(Id()) << TX_VARS(_headingRadian) << TX_VARS(_start_angle_Radian);
      if (_headingRadian > _start_angle_Radian) {
        _start_angle_Radian += (2 * Math::PI);
      } else {
        _start_angle_Radian -= (2 * Math::PI);
      }
    }
    // const Base::txFloat wrap_start_angle_Radian = circRad.Wrap(_start_angle_Radian);
    const txFloat res =
        Math::linear_interp(0.0, duration_ms, _start_angle_Radian, _headingRadian, m_start_angle_action_duration_ms);

    LogInfo << TX_VARS(Id()) << TX_VARS(m_start_angle_action_duration_ms) << TX_VARS(m_sim_step_ms)
            << TX_VARS(_headingRadian) << TX_VARS(_start_angle_Radian) << TX_VARS(res);
    return res;
  } else {
    const Base::txFloat _headingRadian =
        Utils::DisplayerGetLaneAngleFromVector(m_vTargetDir).GetRadian(TX_MARK("SIM-4072"));
    return _headingRadian;
  }
}

Base::txBool TAD_PedestrianElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                                   sim_msg::Traffic& trafficData) TX_NOEXCEPT {
  // 元素活跃
  if (IsAlive()) {
    sim_msg::DynamicObstacle* tmpDynamicObstacleData = nullptr;
    {
#if USE_TBB
      tbb::mutex::scoped_lock lock(s_tbbMutex_outputTraffic);
#endif
      tmpDynamicObstacleData = trafficData.add_dynamicobstacles();
    }
    // 设置各个属性值
    tmpDynamicObstacleData->set_id(Id() * -1);
    tmpDynamicObstacleData->set_t(timeMgr.TimeStamp());
    const hadmap::txPoint geom_center_gps = RawVehicleCoord().WGS84();
    // 经度值设置
    tmpDynamicObstacleData->set_x(__Lon__(geom_center_gps));
    // 纬度值设置
    tmpDynamicObstacleData->set_y(__Lat__(geom_center_gps));
    tmpDynamicObstacleData->set_z(Altitude() /*__Alt__(geom_center_gps)*/);
#if USE_CustomModelImport
    tmpDynamicObstacleData->set_type(m_pedestrianTypeId);
#else  /*#USE_CustomModelImport*/
    // 类型设置
    tmpDynamicObstacleData->set_type(__enum2int__(PEDESTRIAN_TYPE, m_pedestrianType));
#endif /*#USE_CustomModelImport*/
    // const Base::txFloat _headingRadian =
    // Utils::DisplayerGetLaneAngleFromVector(m_vTargetDir).GetRadian(TX_MARK("SIM-4072"));
    const Base::txFloat _headingRadian = ComputeHeading(timeMgr.PassTime());
    // 航向角设置
    tmpDynamicObstacleData->set_heading(_headingRadian);
    // 长宽高设置
    tmpDynamicObstacleData->set_length(GetLength());
    tmpDynamicObstacleData->set_width(GetWidth());
    tmpDynamicObstacleData->set_height(GetHeigth());
    // 速度加速度设置
    tmpDynamicObstacleData->set_v(GetVelocity());
    tmpDynamicObstacleData->set_acc(GetAcc());
    tmpDynamicObstacleData->set_show_abs_velocity(GetVelocity());
    tmpDynamicObstacleData->set_show_abs_acc(GetAcc());
#if USE_VehicleKinectInfo
    tmpDynamicObstacleData->set_show_relative_velocity(show_relative_velocity_vertical());
    tmpDynamicObstacleData->set_show_relative_velocity_horizontal(show_relative_velocity_horizontal());
    tmpDynamicObstacleData->set_show_relative_dist_vertical(show_relative_dist_vertical());
    tmpDynamicObstacleData->set_show_relative_dist_horizontal(show_relative_dist_horizontal());
    tmpDynamicObstacleData->set_show_relative_acc(show_relative_acc());
    tmpDynamicObstacleData->set_show_relative_acc_horizontal(show_relative_acc_horizontal());
#endif
    mGeometryData.ComputePolygon(GetLocation().ToENU(), m_vTargetDir);
    /*FillingSpatialQuery();*/
    SaveStableState();
  } else {
  }
  return true;
}

Base::txBool TAD_PedestrianElement::IsAlongTheRoad(const Base::txInt _direction) const TX_NOEXCEPT {
  // 判断障碍物元素的方向是否为0度或180度
  if ((0 == _direction) || (180 == _direction)) {
    return true;
  } else {
    return false;
  }
}

Base::txBool TAD_PedestrianElement::IsNotAlongTheRoad(const Base::txInt _direction) const TX_NOEXCEPT {
  return !IsAlongTheRoad(_direction);
}

#if __TX_Mark__("Base::IEventHandler")
TAD_PedestrianElement::EventHandlerType TAD_PedestrianElement::GetEventHandlerType() const TX_NOEXCEPT {
  return EventHandlerType::hPedestrian;
}

Base::txString TAD_PedestrianElement::HandlerDesc() const TX_NOEXCEPT {
  return Base::txString("TAD_PedestrianElement::HandlerDesc");
}

// 事件处理函数
Base::txBool TAD_PedestrianElement::HandlerEvent(IEvent& _event) TX_NOEXCEPT {
  if (IsAlive()) {
    if (IEvent::EventType::ePedestrianTimeVelocity == _event.GetEventType() ||
        IEvent::EventType::ePedestrianEventVelocity == _event.GetEventType()) {
      // 根据时间触发的时间类型
      if (IEvent::EventType::ePedestrianTimeVelocity == _event.GetEventType()) {
        TAD_PedestrianTimeVelocityEvent& refPedestrianTimeVelocityEvent =
            dynamic_cast<TAD_PedestrianTimeVelocityEvent&>(_event);
        std::tie(mKinetics.m_direction, mKinetics.m_velocity) = refPedestrianTimeVelocityEvent.GetValue();
        LOG(INFO) << "[ePedestrianTimeVelocity]" << TX_VARS_NAME(direction, mKinetics.m_direction)
                  << TX_VARS_NAME(velocity, mKinetics.m_velocity);
        // 设置行人行为
        m_pedestrian_behavior = PedestrianBehavior::Direction;
        // 根据条件触发的事件类型
      } else if (IEvent::EventType::ePedestrianEventVelocity == _event.GetEventType()) {
        TAD_PedestrianEventVelocityEvent& refPedestrianEventVelocityEvent =
            dynamic_cast<TAD_PedestrianEventVelocityEvent&>(_event);
        std::tie(mKinetics.m_direction, mKinetics.m_velocity) = refPedestrianEventVelocityEvent.GetValue();
        LOG(INFO) << "[ePedestrianEventVelocity]" << TX_VARS_NAME(direction, mKinetics.m_direction)
                  << TX_VARS_NAME(velocity, mKinetics.m_velocity);
        // 设置行人行为
        m_pedestrian_behavior = PedestrianBehavior::Direction;
      } else {
        LOG(WARNING) << "logical error.";
        return false;
      }

      // 旋转目标方向向量
      m_vTargetDir = Utils::VetRotVecByDegree(m_vTargetDir, (Unit::txDegree::MakeDegree(mKinetics.m_direction)),
                                              Utils::Axis_Enu_Up());
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

EventHitUtilInfo_t TAD_PedestrianElement::GetEventHitUtilInfo() const TX_NOEXCEPT {
  EventHitUtilInfo_t retV;
  retV._ElemId = Id();
  retV._ElementGeomCenter = GetLocation().ToENU();
  // 设置触发时的速度
  retV._Velocity = mKinetics.m_velocity;
  retV._vecPolygon = mGeometryData.Polygon();
  return retV;
}

Base::txSysId TAD_PedestrianElement::HanderId() const TX_NOEXCEPT { return Id(); }

Base::txSysId TAD_PedestrianElement::HanderSysId() const TX_NOEXCEPT { return SysId(); }
Base::ISceneLoader::IPedestriansViewerPtr TAD_PedestrianElement::getPedestriansViewPtr() const TX_NOEXCEPT {
  return _elemAttrViewPtr;
}

Base::ISceneLoader::IPedestriansEventViewerPtr TAD_PedestrianElement::getPedestriansEventViewPtr() const TX_NOEXCEPT {
  return nullptr;
}

Base::ISceneLoader::IPedestriansEventViewerPtr TAD_PedestrianElement::getPedestriansTimeVelocityViewPtr() const
    TX_NOEXCEPT {
  return _elemPedestriansEvent_time_velocity_ViewerPtr;
}

Base::ISceneLoader::IPedestriansEventViewerPtr TAD_PedestrianElement::getPedestriansEventVelocityViewPtr() const
    TX_NOEXCEPT {
  return _elemPedestriansEvent_event_velocity_ViewerPtr;
}

TAD_PedestrianElement::KineticsInfo_t TAD_PedestrianElement::GetKineticsInfo(
    Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT {
  KineticsInfo_t retInfo;
  retInfo.m_elemId = Id();
  retInfo.m_compute_velocity = GetVelocity();
  retInfo.m_acc = GetAcc();
  retInfo.m_show_velocity = GetShowVelocity();
  return retInfo;
}

#  if __TX_Mark__("Log.Debug")
Base::txString TAD_PedestrianElement::Str() const TX_NOEXCEPT { return "TAD_PedestrianElement"; }
#  endif  // #if __TX_Mark__("Log.Debug")

#endif  // __TX_Mark__("Base::IEventHandler")

#if __TX_Mark__("TAD_SceneEvent_1_0_0_0")
Base::txBool TAD_PedestrianElement::HandlerEvent(Base::TimeParamManager const& timeMgr,
                                                 ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT {
#  if 1
  // 行人元素活跃
  if (IsAlive()) {
    using IActionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IActionViewerPtr;
    using IEndConditionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IEndConditionViewerPtr;
    using EventActionInfoVec = Base::ISceneLoader::ISceneEventViewer::IActionViewer::EventActionInfoVec;
    using EndConditionVec = Base::ISceneLoader::ISceneEventViewer::IEndConditionViewer::EndConditionVec;
    using SceneEventActionType = Base::Enums::SceneEventActionType;
    using VehicleMoveLaneState = Base::Enums::VehicleMoveLaneState;
    using EndConditionKeyType = Base::Enums::EndConditionKeyType;
    using acc_invalid_type = Base::Enums::acc_invalid_type;

    // 获取事件id
    const Base::txSysId evId = _eventPtr->id();
    const Base::txString elemType_elemId = TAD_SceneEvent_1_0_0_0::Make_ElemType_ElemId(Type(), Id());

    // 获取事件行为
    IActionViewerPtr actionPtr = _eventPtr->action();
    // 获取结束条件视图指针
    IEndConditionViewerPtr endCondPtr = _eventPtr->endCondition();
    if (NonNull_Pointer(actionPtr) && NonNull_Pointer(endCondPtr)) {
      // 获取所有的事件行为和结束条件
      const EventActionInfoVec& actionVec = actionPtr->actionList();
      const EndConditionVec& endCondVec = endCondPtr->endConditionList();
      // 检查行为和结束条件数量相同
      if (actionVec.size() == endCondVec.size()) {
        // 遍历行为
        for (size_t idx = 0; idx < actionVec.size(); ++idx) {
          const EventActionInfoVec::value_type& refAct = actionVec[idx];
          const EndConditionVec::value_type& refEndCond = endCondVec[idx];

          const Base::txInt actionId = refAct.action_id();
          const Base::txString evId_actionId = TAD_SceneEvent_1_0_0_0::Make_evId_actId(evId, actionId);
#    if __TX_Mark__("element_state")
          // 更新场景事件状态
          TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(
              evId_actionId,
              TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
                  elemType_elemId, _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::pendingState),
                  timeMgr.PassTime()));
          const SceneEventStateInfo sceneEventStateInfo_running = TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
              elemType_elemId, _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::runningState),
              timeMgr.PassTime());
#    endif /*__TX_Mark__("element_state")*/
          /*std::tuple<SceneEventActionType, VehicleMoveLaneState, Base::txFloat>;*/
          /*std::tuple<EndConditionKeyType, Base::txFloat>;*/
          // 事件类型检查
          if (_plus_(SceneEventActionType::acc) == refAct.action_type()) {
            mKinetics.m_acceleration = refAct.value();

            // 时间条件
            if ((_plus_(EndConditionKeyType::time)) == std::get<0>(refEndCond)) {
              AccEndConditionManager accMgr;
              accMgr.m_endCondition.m_IsValid = true;
              accMgr.m_endCondition.m_type = _plus_(acc_invalid_type::eTime);
              accMgr.m_endCondition.m_endCondition = std::get<1>(refEndCond);
              accMgr.m_remainingTime = std::get<1>(refEndCond);
              accMgr.m_happenVelocity = GetVelocity();
              accMgr.m_targetVelocity = 0.0;
              m_optional_AccEndCondition = accMgr;
#    if __TX_Mark__("element_state")
              TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
              if (NonNull_Pointer(m_AccActionManagerPtr)) {
                ClearAccAction(timeMgr); /*clear last acc ation*/
              }
              m_AccActionManagerPtr = std::make_shared<ActionManager>();
              // acc行为管理对象初始化
              m_AccActionManagerPtr->Initialize(evId_actionId, elemType_elemId);
#    endif /*__TX_Mark__("element_state")*/
              // 速度条件
            } else if ((_plus_(EndConditionKeyType::velocity)) == std::get<0>(refEndCond)) {
              AccEndConditionManager accMgr;
              accMgr.m_endCondition.m_IsValid = true;
              accMgr.m_endCondition.m_type = _plus_(acc_invalid_type::eVelocity);
              accMgr.m_endCondition.m_endCondition = std::get<1>(refEndCond);

              accMgr.m_remainingTime = 0.0;
              accMgr.m_happenVelocity = GetVelocity();
              accMgr.m_targetVelocity = std::get<1>(refEndCond);
              m_optional_AccEndCondition = accMgr;

#    if __TX_Mark__("element_state")
              TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
              if (NonNull_Pointer(m_AccActionManagerPtr)) {
                ClearAccAction(timeMgr); /*clear last acc ation*/
              }
              m_AccActionManagerPtr = std::make_shared<ActionManager>();
              // acc 行为初始化
              m_AccActionManagerPtr->Initialize(evId_actionId, elemType_elemId);
#    endif /*__TX_Mark__("element_state")*/
            }
            LOG(INFO) << "[event_handler_vehicle_acc_event], id = " << Id() << ", Set Acc_event = " << (GetAcc());
          } else if (_plus_(SceneEventActionType::velocity) == refAct.action_type()) {  // 速度事件
            mKinetics.m_velocity = refAct.value();
            LOG(INFO) << "[event_handler_vehicle_velocity], id = " << Id() << ", Set Velocity = " << (GetVelocity());
#    if __TX_Mark__("element_state")
            // 更新事件状态
            TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(
                evId_actionId,
                TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
                    elemType_elemId, _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::completeState),
                    timeMgr.PassTime()));
#    endif /*__TX_Mark__("element_state")*/
          } else {
            LogWarn << "un support event type: " << (refAct.action_type()._to_string());
            return false;
          }
        }
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
#  else
  mLifeCycle.SetStart();
  OnStart(timeMgr);
  LOG(INFO) << "Pede " << TX_VARS(Id()) << " scene event trigger.";
  return true;
#  endif
}
#endif /*#if __TX_Mark__("TAD_SceneEvent_1_0_0_0")*/

#if __TX_Mark__("VehicleCoord")
Base::txBool TAD_PedestrianElement::SyncPosition(const txFloat _passTime) TX_NOEXCEPT {
  // 获取目标方向向量的2D分量
  const Base::txVec2 dir2d(m_vTargetDir.x(), m_vTargetDir.y());
  // 判断元素的坐标类型是否为车辆后轴中心
  if (CallSucc(RawVehicleCoord_Is_RearAxleCenter())) {
    const Base::txVec2 step_vec = dir2d * Catalog_Rear2Center_North();
    // 设置元素后轴中心
    mLocation.RearAxleCenter() = RawVehicleCoord();
    // 设置元素的几何中心
    mLocation.GeomCenter() = mLocation.RearAxleCenter();
    mLocation.GeomCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
    LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
        << "[SyncPosition][rear_axle]" << TX_VARS(_passTime) << TX_VARS(Id())
        << TX_VARS_NAME(step, Catalog_Rear2Center_North()) << TX_VARS(RawVehicleCoord());
  } else if (CallSucc(RawVehicleCoord_Is_GeomCenter())) {  // 判断元素的坐标类型是否为几何中心
    // 设置元素的几何中心
    mLocation.GeomCenter() = RawVehicleCoord();
    const Base::txVec2 step_vec = dir2d * Catalog_Rear2Center_North() * -1.0;
    // 设置元素后轴中心
    mLocation.RearAxleCenter() = mLocation.GeomCenter();
    mLocation.RearAxleCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
    LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
        << "[SyncPosition][geom_center]" << TX_VARS(_passTime) << TX_VARS(Id())
        << TX_VARS_NAME(step, (Catalog_Rear2Center_North() * -1.0)) << TX_VARS(RawVehicleCoord());
  } else {
    // 设置元素后轴中心
    mLocation.RearAxleCenter() = RawVehicleCoord();
    // 设置元素的几何中心
    mLocation.GeomCenter() = RawVehicleCoord();
    LOG(WARNING) << "unsupport Vehicle Coord Type." << TX_VARS(RawVehicleCoord());
    return false;
  }
  return true;
}
#endif /*VehicleCoord*/

TAD_PedestrianElement::FrenetProjInfo TAD_PedestrianElement::ComputeRoadProjectDistance(
    const Coord::txENU& target_element_geom_center) const TX_NOEXCEPT {
  FrenetProjInfo res;
  // 判断元素的跟踪器是否有效
  if (NonNull_Pointer(mLocation.tracker()) && NonNull_Pointer(mLocation.tracker()->getRoadGeomInfo())) {
    // 初始化源点和目标点的Frenet坐标
    Base::txVec2 sourceST, targetST;
    mLocation.tracker()->getRoadGeomInfo()->xy2sl(StableGeomCenter().ENU2D(), sourceST.x(), sourceST.y());
    // 将目标元素的几何中心坐标转换为Frenet坐标
    mLocation.tracker()->getRoadGeomInfo()->xy2sl(target_element_geom_center.ENU2D(), targetST.x(), targetST.y());

    // 计算源点和目标点的Frenet坐标的s轴距离
    Base::txFloat signDistance = targetST.x() - sourceST.x();
    res.mIsValid = true;
    // 计算源点和目标点的Frenet坐标的s轴距离的绝对值
    res.mProjDistance = std::fabs(signDistance);
    // 判断源点和目标点的Frenet坐标的s轴距离的符号
    if (signDistance > 0.0) {
      // 如果s轴距离为正，则目标元素位于当前元素的前方
      res.mTarget2Source = _plus_(Base::Enums::Element_Spatial_Relationship::eFront);
    } else if (signDistance < 0.0) {
      // 如果s轴距离为负，则目标元素位于当前元素的后方
      res.mTarget2Source = _plus_(Base::Enums::Element_Spatial_Relationship::eRear);
    } else {
      // 重叠
      res.mTarget2Source = _plus_(Base::Enums::Element_Spatial_Relationship::eOverlap);
    }
  }
  return res;
}

#if USE_VehicleKinectInfo

TAD_PedestrianElement::KineticsUtilInfo_t TAD_PedestrianElement::GetKineticsUtilInfo() const TX_NOEXCEPT {
  return m_self_KineticsUtilInfo;
}

Base::txBool TAD_PedestrianElement::ComputeKinetics(Base::TimeParamManager const& timeMgr,
                                                    const KineticsUtilInfo_t& _objInfo) TX_NOEXCEPT {
  // 获取元素的速度和加速度
  m_show_abs_velocity = GetVelocity();
  m_show_abs_acc = GetAcc();
  // 判断时间是否大于0且目标元素信息有效
  if (timeMgr.RelativeTime() > 0.0 && CallSucc(_objInfo.m_isValid)) {
    // 获取目标的几何中心坐标
    const Base::txVec2 _adc_center = _objInfo._ElementGeomCenter.ENU2D();
    /*const Base::txVec2 _last_adc_center = _objInfo._LastElementPos.GetLocalPos_2D();*/
    const Base::txVec2 _obs_center = GeomCenter().ToENU().ENU2D();
    /*const Base::txVec2 _last_obs_center = pImpl->m_elemRoadInfo.lastPos.GetLocalPos_2D();*/
    // 获取目标的变换矩阵的逆矩阵
    const Base::txMat2& tranMatInv = _objInfo._TransMatInv;

    const Base::txVec2 local_adc_center(0.0, 0.0);
    /*const Base::txVec2 local_last_adc_center = (_last_adc_center - _adc_center).transpose() * tranMatInv;*/
    // 计算元素在目标坐标系下的几何中心坐标
    const Base::txVec2& local_obs_center = (_obs_center - _adc_center).transpose() * tranMatInv;
    /*const Base::txVec2 local_last_obs_center = (_last_obs_center - _adc_center).transpose() * tranMatInv;*/

    // 计算行人元素与目标在水平和垂直方向上的相对距离
    m_show_relative_dist_horizontal = local_obs_center.x() /** -1.0*/;
    m_show_relative_dist_vertical = local_obs_center.y();
    // 计算行人元素与目标元素的海拔高度差
    m_show_abs_dist_elevation = Altitude();
    m_show_relative_dist_elevation = show_abs_dist_elevation() - _objInfo._ElementGeomCenter.GetWGS84().Alt();

    // const Base::txVec2 local_adc_velocity = (local_adc_center - local_last_adc_center) / timeMgr.relativeTime;
    // 计算行人元素在目标元素坐标系下的相对速度
    Base::txVec2 local_relative_velocity;
    if (Math::isNotZero(m_local_last_obs_center.x()) && Math::isNotZero(m_local_last_obs_center.y())) {
      local_relative_velocity = (local_obs_center - m_local_last_obs_center) / timeMgr.RelativeTime();
    } else {
      local_relative_velocity.setZero();
    }

    // const Base::txVec2 local_relative_velocity = (local_obs_velocity - local_adc_velocity);
    m_show_relative_velocity_horizontal = local_relative_velocity.x() * -1.0;
    m_show_relative_velocity_vertical = local_relative_velocity.y();

    Base::txVec2 local_relative_acc = (local_relative_velocity - m_local_last_obs_velocity) / timeMgr.RelativeTime();
    m_show_relative_acc = local_relative_acc.x();
    m_show_relative_acc_horizontal = local_relative_acc.y();

    // update cache
    m_local_last_obs_center = local_obs_center;
    m_local_last_obs_velocity = local_relative_velocity;
  } else {
    // 如果时间小于等于0或目标元素信息无效，则将相对距离、速度和加速度设置为0
    m_show_relative_dist_horizontal = 0.0;
    m_show_relative_dist_vertical = 0.0;
    m_show_relative_velocity_horizontal = 0.0;
    m_show_relative_velocity_vertical = 0.0;
    m_show_relative_acc = 0.0;
    m_show_relative_acc_horizontal = 0.0;
  }
  return true;
}
#endif /*USE_VehicleKinectInfo*/

TX_NAMESPACE_CLOSE(TrafficFlow)
