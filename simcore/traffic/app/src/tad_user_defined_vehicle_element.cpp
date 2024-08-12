// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_user_defined_vehicle_element.h"
#include "tad_acc_event.h"
#include "tad_scene_event_1_0_0_0.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "tad_vehicle_merge_event.h"
#include "tad_velocity_event.h"
#include "tx_centripetal_cat_mull.h"
#include "tx_event.h"
#include "tx_frame_utils.h"
#include "tx_hadmap_utils.h"
#include "tx_spatial_query.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"
#include "tx_trajectory_sampling_node.h"
#define VehicleTrajInfo LOG_IF(INFO, FLAGS_LogLevel_Vehicle_Traj)
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_UserDefine)
#define KineticsInfo LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
#define LogWarn LOG(WARNING)
#define LateralActionInfo LOG(INFO) << "[LateralAction]" << TX_VARS(Id())

TX_NAMESPACE_OPEN(TrafficFlow)

TAD_UserDefined_VehicleElement::~TAD_UserDefined_VehicleElement() {
  /*LOG(WARNING) << "[~]TAD_UserDefined_VehicleElement" << TX_VARS(Id());*/
}

Base::txFloat TAD_UserDefined_VehicleElement::Get_MoveTo_Sideway_Angle(Base::txFloat const exper_time) {
  Base::txFloat const t = std::max(exper_time, 0.0);
  // 计算侧向速度
  Base::txFloat lateral_velocity = t * 1.037 - (0.2304444) * std::pow(t, 2);

  Base::txFloat curLaneWidth = FLAGS_default_lane_width;
  /*if (tracker->getCurrentLane()) {
      auto curLaneId = tracker->getCurrentLane()->getTxLaneId();
      curLaneWidth = MapManager::getInstance().GetLaneWidth(curLaneId);
  }*/

  return lateral_velocity * (curLaneWidth) / FLAGS_default_lane_width;
}

Base::txFloat TAD_UserDefined_VehicleElement::MoveBack_Sideway_Angle(const Base::txFloat time) {
  // 将输入的时间限制在0以上
  Base::txFloat t = (time > 0) ? time : 0.0;

  // 初始化侧向速度
  Base::txFloat V_Y = 0.0;
  Base::txFloat a, b;
  // if (ZeroValueCheck(LaneChanging_Duration- Average_LaneChanging_Duration))
  /*b = 1.74 + 0.5*(3 - LaneAborting_Duration);*/
  b = 3.11 + 1.5 * (2.2 - m_LaneAbortingTimeMgr.Duration());
  // else if (ZeroValueCheck(LaneChanging_Duration - Average_LaneChanging_Duration_Short))
  //  b = 3.57 + 1.75*(2.2 - LaneAborting_Duration);
  a = b / m_LaneAbortingTimeMgr.Duration();
  V_Y = t * b - a * t * t;
  return V_Y;
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::Initialize_Kinetics(
    Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr) TX_NOEXCEPT {
  // 如果车辆属性指针非空
  if (NonNull_Pointer(_elemAttrViewPtr)) {
    /*mKinetics.m_velocity = (g_TT_V) ? (*g_TT_V) : (_elemAttrViewPtr->start_v());*/
    // 设置车辆初始速度
    mKinetics.m_velocity = _elemAttrViewPtr->start_v();
    /*mKinetics.m_velocity = _elemAttrViewPtr->start_v();*/
    mKinetics.LastVelocity() = mKinetics.m_velocity;
    // 设置车辆最大速度
    mKinetics.m_velocity_max = _elemAttrViewPtr->max_v();
    mKinetics.velocity_desired = mKinetics.m_velocity_max;
    mKinetics.raw_velocity_max = mKinetics.m_velocity_max;
    return true;
  } else {
    LogWarn << "Initialize_Kinetics Error. " << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr));
    return false;
  }
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::CheckStart(Base::TimeParamManager const &timeMgr)
    TX_NOEXCEPT {
  // 如果车辆尚未开始移动
  if (CallFail(IsStart())) {
    // 如果相对时间大于0.0且绝对时间大于等于车辆的开始时间
    if (/*timeMgr.RelativeTime() > 0.0 &&*/
        timeMgr.AbsTime() >= mLifeCycle.StartTime()) {
      // 设置车辆开始移动
      mLifeCycle.SetStart();
      SM::txUserDefinedState::SetStart();
      SM::txUserDefinedState::SetLaneKeep();
      OnStart(timeMgr);
    }
  }
  return IsStart();
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::Initialize(
    ISceneLoader::IViewerPtr _absAttrView, ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 检查指针的有效性
  if (NonNull_Pointer(_absAttrView) && NonNull_Pointer(_sceneLoader) && CallSucc(_absAttrView->IsInited())) {
    Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr =
        std::dynamic_pointer_cast<Base::ISceneLoader::IVehiclesViewer>(_absAttrView);
    // 视图指针不为空且行为支持
    if (NonNull_Pointer(_elemAttrViewPtr) && CallSucc(IsSupportBehavior(_elemAttrViewPtr->behaviorEnum()))) {
      // 获取路由视图
      Base::ISceneLoader::IRouteViewerPtr _elemRouteViewPtr = _sceneLoader->GetRouteData(_elemAttrViewPtr->routeID());
      LogInfo << "Vehicle Attr :" << (_elemAttrViewPtr->Str());
      // 判断路由视图指针不为空
      if (NonNull_Pointer(_elemRouteViewPtr)) {
        LogInfo << "Vehicle Id : " << (_elemAttrViewPtr->id()) << ", Route : " << (_elemRouteViewPtr->Str());
        ClearSceneEvent();
        // 如果存在场景事件
        if (_sceneLoader->HasSceneEvent()) {
          m_use_scene_event_1_0_0_0 = true;
          _elemAccViewPtr = nullptr;
          _elemAccEventViewPtr = nullptr;
          _elemMergeViewPtr = nullptr;
          _elemMergeEventViewPtr = nullptr;
          _elemVelocityViewPtr = nullptr;
          _elemVelocityEventViewPtr = nullptr;

          // 将所有场景事件提取为map
          std::vector<ISceneEventViewerPtr> all_scene_event = _sceneLoader->GetSceneEventVec();
          std::map<txSysId, ISceneEventViewerPtr> map_id2ptr;
          for (auto ptr : all_scene_event) {
            if (NonNull_Pointer(ptr)) {
              map_id2ptr[ptr->id()] = ptr;
            }
          }

          // 在map中找到当前的元素涉及的事件指针并记录
          const auto event_id_vec = _elemAttrViewPtr->eventId();
          for (const auto evId : event_id_vec) {
            if (_Contain_(map_id2ptr, evId)) {
              m_vec_scene_event.emplace_back(map_id2ptr.at(evId));
            } else {
              LogWarn << "can not find scene event " << TX_VARS(evId);
            }
          }
        } else {
          m_use_scene_event_1_0_0_0 = false;
          TX_MARK("old version, deprecated");
          // 不存在场景事件则初始化默认事件视图
          _elemAccViewPtr = _sceneLoader->GetAccsData(_elemAttrViewPtr->accID());
          _elemAccEventViewPtr = _sceneLoader->GetAccsEventData(_elemAttrViewPtr->accID());
          _elemMergeViewPtr = _sceneLoader->GetMergesData(_elemAttrViewPtr->mergeID());
          _elemMergeEventViewPtr = _sceneLoader->GetMergesEventData(_elemAttrViewPtr->mergeID());
          _elemVelocityViewPtr = _sceneLoader->GetVelocityData(_elemAttrViewPtr->id());
          _elemVelocityEventViewPtr = _sceneLoader->GetVelocityEventData(_elemAttrViewPtr->id());
        }
        m_vehicle_type = __lpsz2enum__(
            VEHICLE_TYPE,
            (_elemAttrViewPtr->vehicleType()).c_str()); /*Utils::String2ElementType((_elemAttrViewPtr->vehicleType()),
                                                           _plus_(VEHICLE_TYPE::Sedan)); */
#if USE_CustomModelImport
        m_vehicle_type_str = _elemAttrViewPtr->vehicleType().c_str();
        m_vehicle_type_id = Base::CatalogCache::VEHICLE_TYPE(m_vehicle_type_str);
#endif /*#USE_CustomModelImport*/
        if (CallSucc(IsOnlyTrajectoryFollow(m_vehicle_type))) {
          LogWarn << TX_VARS(Id()) << TX_VARS_NAME(type, (_elemAttrViewPtr->vehicleType()))
                  << " IsOnlyTrajectoryFollow";
          return false;
        }
        const auto refRoutePtr = (_elemRouteViewPtr);
        const auto refVehiclePtr = (_elemAttrViewPtr);
        mIdentity.Id() = refVehiclePtr->id();
        mIdentity.SysId() = CreateSysId(mIdentity.Id());
#if 1
        // 根据catalog存在与否设置几何信息
#  if USE_CustomModelImport
        if (CallFail(InitVehicleCatalog(m_vehicle_type_str))) {
#  else  /*#USE_CustomModelImport*/
        if (CallFail(InitCatalog(m_vehicle_type))) {
#  endif /*#USE_CustomModelImport*/
          // 使用默认的值
          mGeometryData.Length() = refVehiclePtr->length();
          mGeometryData.Width() = refVehiclePtr->width();
          mGeometryData.Height() = refVehiclePtr->height();
          LogWarn << "[Catalog_query][failure] " << TX_VARS(Id())
                  << TX_VARS_NAME(VehicleType, (_elemAttrViewPtr->vehicleType())) << TX_VARS(GetLength())
                  << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
        } else {
          // 使用catalog中的值
          mGeometryData.Length() = Catalog_Length();
          mGeometryData.Width() = Catalog_Width();
          mGeometryData.Height() = Catalog_Height();
          LOG_IF(INFO, FLAGS_LogLevel_Catalog) << "[Catalog_query][success] " << TX_VARS(Id())
                                               << TX_VARS_NAME(VehicleType, (_elemAttrViewPtr->vehicleType()))
                                               << TX_VARS(GetLength()) << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
        }
#endif
        // 初始化车辆状态
        SM::txUserDefinedState::Initialize(mIdentity.Id());
        // 初始化随机数生成器
        mPRandom.Initialize(mIdentity.Id(), _sceneLoader->GetRandomSeed());
        // 设置车辆的开始时间
        mLifeCycle.StartTime() = refVehiclePtr->start_t();

        // 设置几何体类型为车辆
        mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;

        // 设置车辆的激进程度
        m_aggress = refVehiclePtr->aggress();
        TX_MARK("For lanekeep");

        // 初始化车辆动力学参数
        if (CallFail(Initialize_Kinetics(_elemAttrViewPtr TX_MARK("visualizer")))) {
          LogWarn << "initialize kinetics error.";
          return false;
        }

        // 设置车辆的跟随目标
        if (FLAGS_Topological_Root_Id != refVehiclePtr->follow()) {
          m_optional_followId = refVehiclePtr->follow();
        } else {
          m_optional_followId = boost::none;
        }

        // 计算车辆的起始车道ID
        const Base::txLaneID startLaneId = refVehiclePtr->laneID();
        // 设置车辆的下一个车道索引
        mLocation.NextLaneIndex() = TrafficFlow::RoutePathManager::ComputeGoalLaneIndex(startLaneId);
        // 设置车辆的车道偏移量
        mLocation.LaneOffset() = refVehiclePtr->l_offset();

        // 判断车辆是否位于车道上
        const txBool bLocateOnLaneLink = (0 == startLaneId);
        LOG(INFO) << "[vehicle_init_location] " << TX_VARS_NAME(VehicleId, mIdentity.Id())
                  << (bLocateOnLaneLink ? " [LaneLink]" : " [Lane]");
        if (CallFail(bLocateOnLaneLink)) {
          TX_MARK("on lane");
          txFloat distanceCurveOnInit_Lane = 0.0;
          txFloat distancePedal_lane = 0.0;
          // 获取车道指针
          hadmap::txLanePtr initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetLaneForInit(
              refRoutePtr->startLon(), refRoutePtr->startLat(), distanceCurveOnInit_Lane, distancePedal_lane);

          // 检查车道指针是否为空
          if (NonNull_Pointer(initLane_roadId_sectionId)) {
            LogInfo << TX_VARS_NAME(initLaneUid, Utils::ToString(initLane_roadId_sectionId->getTxLaneId()));

            // 获取当前车道id
            Base::txLaneUId _laneUid = initLane_roadId_sectionId->getTxLaneId();
            _laneUid.laneId = startLaneId;
            // 根据车道id获取车道指针
            hadmap::txLanePtr initLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(_laneUid);
            if (Null_Pointer(initLane)) {
              LogWarn << ", GetLane Failure." << TX_VARS_NAME(_laneUid, Utils::ToString(_laneUid));
              return false;
            }

            initLane_roadId_sectionId = initLane;
            // 如果获取车辆初始位置所在的车道上的位置成功
            if (HdMap::HadmapCacheConCurrent::Get_LAL_Lane_By_S(
                    initLane->getTxLaneId(), GenerateST(refVehiclePtr->start_s(), distanceCurveOnInit_Lane),
                    mLocation.PosOnLaneCenterLinePos())) {
              // 设置车辆的位置为车道上的位置
              mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
              // 设置车辆在车道上的距离
              mLocation.DistanceAlongCurve() = GenerateST(refVehiclePtr->start_s(), distanceCurveOnInit_Lane);
              // 获取车辆所在车道的方向
              mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneDir(initLane, mLocation.DistanceAlongCurve());
              /*mLocation.rot_for_display() = Utils::DisplayerGetLaneAngleFromVector(mLocation.vLaneDir());*/
              // 计算车辆的偏移量
              RawVehicleCoord() =
                  ComputeLaneOffset(mLocation.PosOnLaneCenterLinePos(), mLocation.vLaneDir(), mLocation.LaneOffset());
              // 同步车辆位置
              SyncPosition(0.0);
              // 初始化车辆追踪器
              mLocation.InitTracker(mIdentity.Id());
              // 重定位追踪器
              RelocateTracker(initLane, 0.0);
              // 计算车辆的多边形
              mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
              FillingSpatialQuery();
              // 保存稳定状态
              SaveStableState();

              // 初始化路径
              mRoute.Initialize(mLocation.PosOnLaneCenterLinePos().WGS84(), refRoutePtr->midGPS(),
                                refRoutePtr->endGPS());
              // 获取路径信息数组
              roadInfoArray = mRoute.roadInfoArray();
              // 重置路径信息数组
              roadInfoArray.Reset();
              LOG(INFO) << "reset roadInfoArray.Relax() = " << roadInfoArray.Relax();
              LogInfo << TX_VARS(Id()) << TX_VARS(mLocation.PosOnLaneCenterLinePos())
                      << TX_VARS(mLocation.PosWithLateralWithoutOffset()) << TX_VARS(VehicleCoordStr());
              return true;
            } else {
              LogWarn << ", Get_LAL_Lane_By_S Failure.";
              return false;
            }
          } else {
            LogWarn << ", Get Init Lane Error.";
            return false;
          }
        } else if (CallSucc(bLocateOnLaneLink)) {
          // 处于车道连接上
          TX_MARK("on link");
          Base::txFloat distancePedal_lanelink = 0.0;
          Base::txFloat distanceCurveOnInit_LaneLink = 0.0;
          // 获取车辆初始位置所在的车道连接
          hadmap::txLaneLinkPtr initLaneLinkptr = HdMap::HadmapCacheConCurrent::GetLaneLinkForInit(
              refRoutePtr->startLon(), refRoutePtr->startLat(), distanceCurveOnInit_LaneLink, distancePedal_lanelink);
          // 如果获取到了车辆初始位置所在的车道连接
          if (NonNull_Pointer(initLaneLinkptr)) {
            // 获取车辆初始位置所在的车道连接上的位置
            if (HdMap::HadmapCacheConCurrent::Get_LAL_LaneLink_By_S(
                    Base::Info_Lane_t(initLaneLinkptr->getId(), initLaneLinkptr->fromTxLaneId(),
                                      initLaneLinkptr->toTxLaneId()),
                    distanceCurveOnInit_LaneLink, mLocation.PosOnLaneCenterLinePos())) {
              // 初始化location信息
              mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
              mLocation.DistanceAlongCurve() = distanceCurveOnInit_LaneLink;
              mLocation.vLaneDir() =
                  HdMap::HadmapCacheConCurrent::GetLaneLinkDir(initLaneLinkptr, mLocation.DistanceAlongCurve());
              /*mLocation.rot_for_display() = Utils::DisplayerGetLaneAngleFromVector(mLocation.vLaneDir());*/
              RawVehicleCoord() =
                  ComputeLaneOffset(mLocation.PosOnLaneCenterLinePos(), mLocation.vLaneDir(), mLocation.LaneOffset());
              // 同步车辆位置
              SyncPosition(0.0);
              // 初始化车辆追踪器
              mLocation.InitTracker(mIdentity.Id());
              // 重新定位车辆追踪器
              RelocateTracker(initLaneLinkptr, 0.0);
              // 计算车辆的多边形
              mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
              // 填充空间查询
              FillingSpatialQuery();
              // 保存稳定状态
              SaveStableState();
              // 初始化路径
              mRoute.Initialize(mLocation.PosOnLaneCenterLinePos().WGS84(), refRoutePtr->midGPS(),
                                refRoutePtr->endGPS());
              // 获取路径信息数组
              roadInfoArray = mRoute.roadInfoArray();
              roadInfoArray.Reset();
              LOG(INFO) << "reset roadInfoArray.Relax() = " << roadInfoArray.Relax();
              LogInfo << TX_VARS(mLocation.PosOnLaneCenterLinePos()) << TX_VARS(mLocation.PosWithLateralWithoutOffset())
                      << TX_VARS(VehicleCoordStr());
              return true;
            } else {
              LogWarn << "GetLonLatFromSTonLinkWithoutOffset Failure.";
              return false;
            }
          } else {
            LogWarn << ", Get Init LaneLink Error.";
            return false;
          }
        } else {
          LogWarn << "start lane id error. " << TX_VARS(startLaneId);
          return false;
        }
      } else {
        LogWarn << "Can not find Route Info.";
        return false;
      }
    } else {
      LogWarn << "Param Cast Error." << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr))
              << TX_COND_NAME(IsSupportBehavior, IsSupportBehavior(_elemAttrViewPtr->behaviorEnum()));
      return false;
    }
  } else {
    LogWarn << "Param Error." << TX_COND_NAME(_absAttrView, NonNull_Pointer(_absAttrView))
            << TX_COND_NAME(_sceneLoader, NonNull_Pointer(_sceneLoader))
            << TX_COND_NAME(_absAttrView_IsInited, CallSucc(_absAttrView->IsInited()));
    return false;
  }
}

void TAD_UserDefined_VehicleElement::CheckWayPoint() TX_NOEXCEPT {
  // 调用roadInfoArray的CheckWayPoint方法，传入车辆位置信息和车道位置信息
  roadInfoArray.CheckWayPoint(mLocation.PosOnLaneCenterLinePos(), mLocation.LaneLocInfo());
}

hadmap::txLaneLinkPtr TAD_UserDefined_VehicleElement::SearchNextLinkByRouteInfo(
    const hadmap::txLaneLinks &nextLaneLinks, const Base::txLaneID toLaneId_neg,
    const Base::txLaneLinkID wishLaneLinkId) TX_NOEXCEPT {
  // 遍历下一个车道连接数组
  for (auto linker : (nextLaneLinks)) {
    // 如果找到了符合条件的车道连接，返回该车道连接
    if (wishLaneLinkId == (linker->getId())) {
      return linker;
    }
  }
  // 遍历下一个车道连接数组
  for (auto linker : (nextLaneLinks)) {
    // 获取路径信息数组的松弛值
    Base::txDiff_t const nRelaxSize = roadInfoArray.Relax();
    // 遍历路径信息数组
    for (Base::txDiff_t i = 0; i < nRelaxSize && i < 10; ++i) {
      // 如果找到了符合条件的车道连接，返回该车道连接
      if (roadInfoArray.NextRoadId(i) == linker->toRoadId() && toLaneId_neg == linker->toLaneId()) {
        roadInfoArray.Next(i);
        return linker;
        break;
        TX_MARK("should jump out of the outer layers of this layer");
      }
    }
  }
  // 如果没有找到符合条件的车道连接，从下一个车道连接数组中随机选择一个车道连接
  return RandomNextLink(nextLaneLinks);
  TX_MARK("nextLaneLinks.front();");
}

hadmap::txLaneLinkPtr TAD_UserDefined_VehicleElement::SearchNextLinkByRouteInfo(
    const hadmap::txLaneLinks &nextLaneLinks, const Base::txLaneID toLaneId_neg) TX_NOEXCEPT {
  hadmap::txLaneLinkPtr nextLaneLink(nullptr);
  // 遍历下一个车道连接数组
  for (auto linker : (nextLaneLinks)) {
    // 获取路径信息数组的松弛值
    Base::txDiff_t const nRelaxSize = roadInfoArray.Relax();
    // 遍历路径信息数组
    for (Base::txDiff_t i = 0; i < nRelaxSize && i < 10; ++i) {
      // 如果找到了符合条件的车道连接，将该车道连接赋值给nextLaneLink，并更新路径信息数组，然后返回nextLaneLink
      if (roadInfoArray.NextRoadId(i) == linker->toRoadId() && toLaneId_neg == linker->toLaneId()) {
        nextLaneLink = linker;
        roadInfoArray.Next(i);
        return nextLaneLink;
        break;
        TX_MARK("should jump out of the outer layers of this layer");
      }
    }
  }
  return RandomNextLink(nextLaneLinks);
  TX_MARK("nextLaneLinks.front();");
}

const hadmap::txLanePtr SearchLeastLaneChangeWithGoalLane(
    const hadmap::txLanes &laneVec, const Base::txLaneID goalLaneIndex TX_MARK("(laneId + 1) * -1")) {
  hadmap::txLanePtr retPtr = laneVec.front();
  Base::txLaneID idxDiff = FLAGS_MAX_LANE_COUNT;
  // 遍历车道数组
  for (const auto &lanePtr : (laneVec)) {
    // 计算当前车道与目标车道的车道索引差值
    const auto curDiff =
        std::abs(RoutePathManager::ComputeGoalLaneIndex(lanePtr->getTxLaneId().laneId) - goalLaneIndex);
    // 如果当前车道与目标车道的车道索引差值小于等于已知的最小车道索引差值，更新最小车道索引差值和返回的车道指针
    if (curDiff <= idxDiff) {
      TX_MARK("Tend to change lanes to the right");
      idxDiff = curDiff;
      retPtr = lanePtr;
    }
  }
  return retPtr;
}

Base::txBool TAD_UserDefined_VehicleElement::MoveStraightOnS(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  // 计算车辆在当前时间段内的移动距离
  const txFloat moving_distance = Compute_Displacement(timeMgr);
  // 获取车辆当前所在的车道信息
  const auto curLaneInfo = mLocation.LaneLocInfo();

  // 获取车辆当前所在的曲线的长度
  const txFloat curveLength = mLocation.tracker()->getCurrentCurve()->getLength();
  // 如果车辆当前所在的曲线上的距离加上移动距离小于曲线的长度，更新车辆当前所在的曲线上的距离
  if ((mLocation.DistanceAlongCurve() + moving_distance) < curveLength) {
    mLocation.DistanceAlongCurve() += moving_distance;
  } else {
    // 计算车辆在下一个车道上的距离
    const txFloat offset = (mLocation.DistanceAlongCurve() + moving_distance) - curveLength;
    // 如果车辆在车道连接上
    if (mLocation.IsOnLaneLink()) {
      // 获取下一个车道的指针
      hadmap::txLanePtr nextLane =
          HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.tracker()->getCurrentLaneLink()->toTxLaneId());
      // 如果成功将车辆重定位到下一个车道，更新车辆在下一个车道上的距离
      if (RelocateTracker(nextLane, 0.0)) {
        mLocation.DistanceAlongCurve() = offset;
      } else {
        // 如果重定位失败，停止车辆
        StopVehicle(TODO("state machine"));
        LogWarn << "mLocation.tracker()->relocate error. StopVehicle";
        return false;
      }
    } else {
      if (mLocation.tracker()->isReachedRoadLastSection()) {
        // 获取下一个车道连接数组
        const hadmap::txLaneLinks nextLaneLinks =
            HdMap::HadmapCacheConCurrent::GetLaneNextLinks(mLocation.LaneLocInfo().onLaneUid);
        // 如果下一个车道连接数组非空
        if (_NonEmpty_(nextLaneLinks)) {
          hadmap::txLaneLinkPtr nextLaneLink = nullptr;
          // 如果路径信息数组中有下一个路点
          if (roadInfoArray.HasNextWayPoint()) {
            const auto nextWayPointArea = roadInfoArray.GetNextWayPoint();
            // 如果下一个路点是车道连接区域
            if (nextWayPointArea.IsLaneLinkArea()) {
              const Base::txLaneLinkID nextLaneLinkId = nextWayPointArea.GetLaneLinkId();
              const Base::txLaneUId fromUid = nextWayPointArea.GetLaneLinkFromUid();
              const Base::txLaneUId toUid = nextWayPointArea.GetLaneLinkToUid();
              hadmap::txLaneLinkPtr nextLaneLinkNeed = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(fromUid, toUid);
              const Base::txLaneID toLaneId_neg = nextLaneLinkNeed->toLaneId() * -1;
              // 根据路径信息搜索下一个车道连接
              nextLaneLink = SearchNextLinkByRouteInfo(nextLaneLinks, toLaneId_neg, nextLaneLinkId);
            } else {
              // 如果下一个路点不是车道连接区域，从下一个车道连接数组中随机选择一个车道连接
              nextLaneLink = RandomNextLink(nextLaneLinks);
              TX_MARK("nextLaneLinks.front();");
            }
          } else {
            // 如果路径信息数组中没有下一个路点，根据路径信息搜索下一个车道连接
            nextLaneLink = SearchNextLinkByRouteInfo(nextLaneLinks, mLocation.NextLaneIndex());
          }
          // 如果找到了下一个车道连接，并且下一个车道连接的几何体非空
          if (NonNull_Pointer(nextLaneLink) && NonNull_Pointer(nextLaneLink->getGeometry())) {
            // 如果新车道的长度大于偏移量，更新车辆在新车道上的距离
            if ((nextLaneLink->getGeometry()->getLength()) > offset) {
              TX_MARK("The new lane is longer than offset and can carry the overflow from this move");
              mLocation.DistanceAlongCurve() = offset;
              RelocateTracker(nextLaneLink, 0.0);
              mLocation.NextLaneIndex() = nextLaneLink->toLaneId();
              // GetNextCrossRoadFromLaneLink(m_nextCrossRoadInfo, nextLaneLink->fromRoadId(),
              // nextLaneLink->toRoadId());
            } else {
              // 如果新车道的长度小于等于偏移量，获取下一个车道指针
              hadmap::txLanePtr nextLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(nextLaneLink->toTxLaneId());
              // 如果成功获取下一个车道指针
              if (NonNull_Pointer(nextLane)) {
                // 将车辆重定位到下一个车道
                RelocateTracker(nextLane, 0.0);
                const Base::txFloat newOffset =
                    offset -
                    (NonNull_Pointer(nextLaneLink->getGeometry()) ? (nextLaneLink->getGeometry()->getLength()) : (0.0));
                // 如果新的偏移量大于0，更新车辆在新车道上的距离
                if (newOffset > 0.0) {
                  mLocation.DistanceAlongCurve() = newOffset;
                } else {
                  // 如果新的偏移量小于等于0，停止车辆
                  mLocation.DistanceAlongCurve() = 0.0;
                  LogWarn << "Compute lanelink offset error.";
                  StopVehicle(); /*Kill();*/
                  return false;
                }
              } else {
                // 如果获取下一个车道指针失败，停止车辆
                LogWarn << "GetTxLanePtr nullptr,"
                        << TX_VARS_NAME(toLaneUid, Utils::ToString(nextLaneLink->toTxLaneId()));
                StopVehicle(); /*Kill();*/
                return false;
              }
            }
          } else {
            // 如果没有找到下一个车道连接，停止车辆
            LogWarn << "Cannot find nextLaneLink with roadInfoArray.NextRoadId.";
            StopVehicle(); /*Kill();*/
            return false;
          }
        } else {
          LogWarn << "LaneLink Size is Zero.";
          StopVehicle();
          return false;
        }
      } else {
        TX_MARK("本次到达的不是Road的最后一个section，在路内");
        hadmap::txLanes nextLanes =
            HdMap::HadmapCacheConCurrent::GetNextLanesByLaneUid(mLocation.tracker()->getCurrentLane());

        if (_NonEmpty_(nextLanes)) {
#if __TX_Mark__("SIM-3776")
          // 根据目标车道搜索最少变道的车道
          const hadmap::txLanePtr nextLane =
              SearchLeastLaneChangeWithGoalLane(nextLanes, roadInfoArray.CurGoalLaneIndex());
#else  /*__TX_Mark__("SIM-3776")*/
          const hadmap::txLanePtr nextLane = nextLanes->front();
          TODO("FLAGS_LogLevel_Routing may be multi lanes");
#endif /*__TX_Mark__("SIM-3776")*/
          if (NonNull_Pointer(nextLane)) {
            // 将车辆重定位到下一个车道
            RelocateTracker(nextLane, 0.0);
            // 更新车辆在新车道上的距离
            mLocation.DistanceAlongCurve() = offset;
            // UpdateNextCrossRoadPathPlaneDirection(m_laneInfo.onLaneUid.roadId, m_laneInfo.onLaneUid.sectionId);//
          } else {
            LogWarn << " not empty vector 'nextLanes' first element ptr is null.";
            StopVehicle(); /*Kill();*/
            return false;
          }
        } else {
          // 如果下一个车道数组为空，停止车辆
          LogWarn << " LaneLink Size is Zero. "
                  << TX_VARS_NAME(cur_lane_uid, Utils::ToString(mLocation.tracker()->getCurrentLane()->getTxLaneId()));
          StopVehicle(); /*Kill();*/
          return false;
        }
      }
    }
  }
  mLocation.UpdateNewMovePointWithoutOffset();
  return true;
}

TAD_UserDefined_VehicleElement::txFloat TAD_UserDefined_VehicleElement::Compute_LateralDisplacement(
    Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  Base::txFloat cur_step_lateral_displacement = 0.0;
  // 如果车辆的纵向速度大于0
  if (GetVelocity() > 0.0) {
    cur_step_lateral_displacement = (m_LaneChangeTimeMgr.Duration() > 0.0)
                                        ? (mKinetics.m_LateralVelocity * timeMgr.RelativeTime())
                                        : (-1.0 * mKinetics.m_LateralVelocity * timeMgr.RelativeTime());
  } else {
    // 如果车辆的纵向速度为0，横向位移为0
    TX_MARK("lateral displacement is zero when longitudinal velocity is zero");
    cur_step_lateral_displacement = 0.0;
    mKinetics.m_LateralVelocity = 0.0;
  }
  // 更新车辆的横向位移
  mKinetics.m_LateralDisplacement += cur_step_lateral_displacement;
  return mKinetics.m_LateralDisplacement;
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::UpdateSwitchLaneRunning(
    Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  // 如果车辆处于变道开始状态
  if (IsInTurn_Start()) {
    // 如果变道过程时间小于等于变道总时间的60%
    if (m_LaneChangeTimeMgr.Procedure() <=
        m_LaneChangeTimeMgr.Duration() *
            (Utils::VehicleBehaviorCfg::
                 ChangeTopologyThreshold()) /*1229HD LaneChanging_Procedure <= LaneChanging_Duration * 0.6*/) {
      // 将车辆状态从变道开始改为变道进行中
      ChaneFromStartToIng();
    }
  } else if (IsInTurn_Ing()) {  // 如果车辆处于变道进行中状态
    // 如果变道过程时间小于等于0
    if (m_LaneChangeTimeMgr.Procedure() <= 0.0) {
      // 如果车辆在车道内部变道
      if (IsInTurn_LeftRight_InLane()) {
        // 重置变道过程时间、总时间和变道状态
        m_LaneChangeTimeMgr.SetProcedure(0.0);
        m_LaneChangeTimeMgr.SetDuration(0.0);
        m_LaneChangeTimeMgr.SetChangeInLaneAction(false);
        /*mLocation.PosWithLateralWithoutOffset() = mLocation.vPos();*/
        /*m_OnLanePos = m_elemRoadInfo.Pos;*/
        mKinetics.m_LateralDisplacement = 0.0;
        // 将车辆状态从变道进行中改为变道完成
        ChaneFromIngToFinish();
        // 设置车辆保持车道
        SetLaneKeep();
        bNeedResetOffset = true;
      } else {
        // 重置变道过程时间、总时间
        m_LaneChangeTimeMgr.SetProcedure(0.0);
        m_LaneChangeTimeMgr.SetDuration(0.0);
        /*mLocation.OnLanePos() = mLocation.PosWithoutOffset();*/
        /*mLocation.CurLaneIndex() = mLocation.NextLaneIndex();*/
        // 重置车辆的横向位移
        mKinetics.m_LateralDisplacement = 0.0;
        // 将车辆状态从变道进行中改为变道完成
        ChaneFromIngToFinish();
        // 设置车辆保持车道
        SetLaneKeep();
        // 获取初始车道和车道链接
        hadmap::txLanePtr initLane = nullptr;
        hadmap::txLaneLinkPtr initLaneLink = nullptr;
        // 获取车辆的S坐标和距离
        if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
                mLocation.PosWithLateralWithoutOffset(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve())) {
          // 如果车辆在车道链接上
          if (mLocation.IsOnLaneLink()) {
            // 获取车道链接指针
            initLaneLink = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(mLocation.LaneLocInfo().onLinkFromLaneUid,
                                                                          mLocation.LaneLocInfo().onLinkToLaneUid);
            if (NonNull_Pointer(initLaneLink)) {
              // 将车辆重定位到车道链接
              RelocateTracker(initLaneLink, 0.0);
            }
          } else {
            initLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
            // 如果车道指针非空
            if (NonNull_Pointer(initLane)) {
              // 将车辆重定位到车道
              RelocateTracker(initLane, 0.0);
            }
          }
        }
      }

#if 1
      using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
      /*for lane change finish*/
      UpdateHashedLaneInfo(mLocation);
      const HashedLaneInfo &selfStableHashedLaneInfo = StableHashedLaneInfo();
      UpdateHashedRoadInfoCache(selfStableHashedLaneInfo);
#endif
      return true;
    }
  } else if (IsInTurn_Abort()) {  // 如果车辆处于变道取消状态
    /*changeLaneState = CHANGE_LANE_STATE_CANCEL_ING;*/
  }

  return false;
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::LaneChangePostOp(
    Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  Base::txFloat fAngle = 0.0;

  mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
  // 如果处于换道中
  if (IsInLaneChange(/*CHANGE_LANE_STATE::CHANGE_LANE_STATE_START <= changeLaneState*/)) {
    // 计算横向偏移距离
    Compute_LateralDisplacement(timeMgr);
    // 处于左转且存在左侧车道
    if (CallSucc(IsInTurnLeft()) && NonNull_Pointer(mLocation.tracker()->getLeftLane())) {
      // 设置下条车道索引
      mLocation.NextLaneIndex() = RoutePathManager::ComputeGoalLaneIndex(mLocation.tracker()->getLeftLane()->getId());
    } else if (CallSucc(IsInTurnRight()) &&
               NonNull_Pointer(mLocation.tracker()->getRightLane())) {  // 处于右转且存在右侧车道
      // 设置下条车道索引
      mLocation.NextLaneIndex() = RoutePathManager::ComputeGoalLaneIndex(mLocation.tracker()->getRightLane()->getId());
    } else {
      TX_MARK("moveState is VEHICLE_MOVE_LANE_STRAIGHT, maintain nextLaneIndex.");
    }

    Base::txVec3 tmpLaneDir = mLocation.vLaneDir();

    // 计算目标方向向量
    const Base::txVec3 vTargetDir =
        Utils::VetRotVecByDegree(tmpLaneDir,
                                 ((IsInTurnRight() || IsInTurnRight_InLane()) ? Unit::txDegree::MakeDegree(RightOnENU)
                                                                              : Unit::txDegree::MakeDegree(LeftOnENU)),
                                 Utils::Axis_Enu_Up());

    // 根据目标方向向量和横向位移量更新车辆位置
    mLocation.PosWithLateralWithoutOffset().TranslateLocalPos(__East__(vTargetDir) * GetLateralDisplacement(),
                                                              __North__(vTargetDir) * GetLateralDisplacement());
    const txFloat maxV_V = Get_MoveTo_Sideway_Angle(2.25);
    // 如果最大侧向速度不为零
    if (!Math::isZero(maxV_V)) {
#if 1
      // 根据车道变换时间管理器的持续时间和经验值计算角度
      if (m_LaneChangeTimeMgr.Duration() > 0.0) {
        const txFloat t = ((m_LaneChangeTimeMgr.Experience() / m_LaneChangeTimeMgr.Duration()) *
                           m_LaneChangeTimeMgr.Average_LaneChanging_Duration());
        const txFloat tmpV_V = Get_MoveTo_Sideway_Angle(t);
        fAngle = tmpV_V * 15.0 / maxV_V;
      } else if (m_LaneAbortingTimeMgr.Duration() > 0.0) {  // 根据车道放弃时间管理器的持续时间和经验值计算角度
        /*const txFloat t = m_LaneAbortingTimeMgr.Experience();*/
        const txFloat t = ((m_LaneChangeTimeMgr.Experience() / m_LaneChangeTimeMgr.Duration()) *
                           m_LaneChangeTimeMgr.Average_LaneChanging_Duration());
        const txFloat tmpV_V = MoveBack_Sideway_Angle(t);
        fAngle = tmpV_V * 15.0 / maxV_V;
      } else {  // 如果没有车道变换和放弃时间管理器，角度为零
        fAngle = 0.0;
      }

      // 根据车辆是否在车道内或车道外的左右转弯，以及是否放弃转弯，调整角度
      if (IsInTurn_LeftRight_InLane()) {
        fAngle *= (IsInTurnLeft_InLane()) ? (1.0) : (-1.0);
        fAngle *= (IsInTurn_Abort()) ? (-1.0) : (1.0);
        fAngle *= (GetVelocity() > 0.0) ? (1.0) : (0.0);
      } else if (IsInTurn_LeftRight()) {
        fAngle *= (IsInTurnLeft()) ? (1.0) : (-1.0);
        fAngle *= (IsInTurn_Abort()) ? (-1.0) : (1.0);
        fAngle *= (GetVelocity() > 0.0) ? (1.0) : (0.0);
      } else {
        LogWarn << "un impossible condition.";
      }
#endif
    }
    // 如果车辆正在执行车道变换，清除合并操作并重置车道保持计数器
    if (UpdateSwitchLaneRunning(timeMgr)) {
      ClearMergeAction(timeMgr);
      mLaneKeepMgr.ResetCounter(Utils::VehicleBehaviorCfg::LaneKeep(), GetAggress());
    }
  } else if (IsInLateralAction()) {  // 如果车辆正在执行侧向动作
    // 计算当前侧向位移量
    const txFloat cur_step_lateral_displacement = m_LateralActionMgrPtr->last_v() * timeMgr.RelativeTime();
    // 更新车辆的侧向位移量
    mKinetics.m_LateralDisplacement += cur_step_lateral_displacement;
    // 计算目标方向向量
    const Base::txVec3 vTargetDir =
        Utils::VetRotVecByDegree(mLocation.vLaneDir(), Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());
    // 记录日志信息
    LateralActionInfo << TX_VARS(cur_step_lateral_displacement) << TX_VARS(mKinetics.m_LateralDisplacement)
                      << TX_VARS(Utils::ToString(vTargetDir));
    // 根据目标方向向量和侧向位移量更新车辆位置
    mLocation.PosWithLateralWithoutOffset().TranslateLocalPos(__East__(vTargetDir) * GetLateralDisplacement(),
                                                              __North__(vTargetDir) * GetLateralDisplacement());

    // 如果侧向目标距离接近零且侧向动作不是连续的，清除侧向距离动作并重置车道保持计数器
    if (CallSucc(Math::isZero(std::fabs(m_LateralActionMgrPtr->sign_target_dist()), 0.1)) &&
        CallFail(m_LateralActionMgrPtr->continuous())) {
      ClearLateralDistanceAction(timeMgr);
      mLaneKeepMgr.ResetCounter(Utils::VehicleBehaviorCfg::LaneKeep(), GetAggress());
    }
  } else {
    SetLaneKeep();
    mKinetics.m_LateralDisplacement = 0.0;
    mKinetics.m_LateralVelocity = 0.0;
  }

  // 更新车辆角度
  mLocation.fAngle().FromDegree(fAngle);
  // 计算车辆在车道上的偏移量
  RawVehicleCoord() =
      ComputeLaneOffset(mLocation.PosWithLateralWithoutOffset(), mLocation.vLaneDir(), mLocation.LaneOffset());
  // 同步车辆位置
  SyncPosition(timeMgr.PassTime());
  // 计算车辆在车道上的偏移量
  Compute_CenterLine_Offset();
  // 如果需要重置偏移量
  if (bNeedResetOffset) {
    // 将需要重置偏移量的标志设置为false
    bNeedResetOffset = false;
    txFloat s = 0.0, l = 0.0, yaw = 0.0;
    // 获取当前曲线指针
    const hadmap::txLineCurve *curCurvePtr = mLocation.tracker()->getCurrentCurve();
    // 如果曲线指针非空
    if (NonNull_Pointer(curCurvePtr)) {
      // 如果曲线坐标类型为WGS84
      if (hadmap::COORD_WGS84 == (curCurvePtr->getCoordType())) {
        curCurvePtr->xy2sl(RawVehicleCoord().Lon(), RawVehicleCoord().Lat(), s, l, yaw);
        // 更新车道偏移量
        mLocation.LaneOffset() = l;
        LogInfo << "Reset offset " << TX_VARS_NAME(new_lane_offset, mLocation.LaneOffset());
      }
    }
  }

  if (mLocation.IsOnLane()) {
    CheckDeadLine();
  }
  return true;
}

Base::txBool TAD_UserDefined_VehicleElement::CheckDeadLine() TX_NOEXCEPT {
  // 如果车辆到达截止区域
  if (mRoute.ArriveAtDeadlineArea(mLocation.LaneLocInfo().onLaneUid, GeomCenter().ToENU())) {
    // 停止车辆
    StopVehicle();
  }
  return IsAlive();
}

TAD_UserDefined_VehicleElement::txFloat TAD_UserDefined_VehicleElement::MoveTo_Sideway(
    const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  // 获取车道变换时间管理器的经验
  const txFloat experience = m_LaneChangeTimeMgr.Experience();
  const txFloat lateral_velocity = Get_MoveTo_Sideway(experience);
  // 更新车道变换时间管理器的时间
  m_LaneChangeTimeMgr.Step(timeMgr.RelativeTime());
  return lateral_velocity;
}

TAD_UserDefined_VehicleElement::txFloat TAD_UserDefined_VehicleElement::Get_MoveTo_Sideway(const txFloat exper_time)
    TX_NOEXCEPT {
  if (m_LaneChangeTimeMgr.Procedure() > 0.0) {
    txFloat s = 0.0, l = 0.0, yaw = 0.0;
    // 如果车辆正在左转
    if (IsInTurnLeft()) {
      // 如果左侧车道指针非空
      if (NonNull_Pointer(mLocation.tracker()->getLeftLane())) {
        // 获取目标车道曲线指针
        const hadmap::txLineCurve *targetLaneCurve =
            dynamic_cast<const hadmap::txLineCurve *>(mLocation.tracker()->getLeftLane()->getGeometry());
        // 如果目标车道曲线指针非空
        if (NonNull_Pointer(targetLaneCurve)) {
          // 如果目标车道曲线坐标类型为WGS84
          if (hadmap::COORD_WGS84 == (targetLaneCurve->getCoordType())) {
            targetLaneCurve->xy2sl(mLocation.PosWithLateralWithoutOffset().Lon(),
                                   mLocation.PosWithLateralWithoutOffset().Lat(), s, l, yaw);
          } else if (hadmap::COORD_ENU == (targetLaneCurve->getCoordType())) {  // 如果目标车道曲线坐标类型为ENU
            // 获取车辆位置的ENU坐标
            const auto &enu = mLocation.PosWithLateralWithoutOffset().ToENU();
            targetLaneCurve->xy2sl(enu.X(), enu.Y(), s, l, yaw);
          }
        }
      }
    } else if (IsInTurnRight()) {  // 如果车辆正在右转
      // 如果右侧车道指针非空
      if (NonNull_Pointer(mLocation.tracker()->getRightLane())) {
        // 获取目标车道曲线指针
        const hadmap::txLineCurve *targetLaneCurve =
            dynamic_cast<const hadmap::txLineCurve *>(mLocation.tracker()->getRightLane()->getGeometry());
        // 如果目标车道曲线指针非空
        if (NonNull_Pointer(targetLaneCurve)) {
          // 如果目标车道曲线坐标类型为WGS84
          if (hadmap::COORD_WGS84 == (targetLaneCurve->getCoordType())) {
            targetLaneCurve->xy2sl(mLocation.PosWithLateralWithoutOffset().Lon(),
                                   mLocation.PosWithLateralWithoutOffset().Lat(), s, l, yaw);
          } else if (hadmap::COORD_ENU == (targetLaneCurve->getCoordType())) {  // 如果目标车道曲线坐标类型为ENU
            const auto &enu = mLocation.PosWithLateralWithoutOffset().ToENU();
            targetLaneCurve->xy2sl(enu.X(), enu.Y(), s, l, yaw);
          } else {
            // txMsg("hadmap::COORD_ENU hadmap::COORD_WGS84");
          }
        } else {
          // txMsg("(targetLaneCurve)");
        }
      } else {
        // txMsg("(tracker->getRightLane())");
      }
    } else if (IsInTurnLeft_InLane()) {  // 左换道
      l = m_LaneChangeTimeMgr.GetChangeInLaneOffset();
      /*LOG(WARNING) << TX_VARS(l)
          << TX_VARS_NAME(Duration, m_LaneChangeTimeMgr.Duration())
          << TX_VARS_NAME(l_Duration, std::fabs(l) / m_LaneChangeTimeMgr.Duration());*/
      return std::fabs(l) / m_LaneChangeTimeMgr.Duration();
    } else if (IsInTurnRight_InLane()) {  // 右换道
      l = m_LaneChangeTimeMgr.GetChangeInLaneOffset();
      /*LOG(WARNING) << TX_VARS(l)
          << TX_VARS_NAME(Duration, m_LaneChangeTimeMgr.Duration())
          << TX_VARS_NAME(l_Duration, std::fabs(l) / m_LaneChangeTimeMgr.Duration());*/
      return std::fabs(l) / m_LaneChangeTimeMgr.Duration();
    }

    // LOG(WARNING) << TX_VARS(l) << TX_VARS(m_LaneChangeTimeMgr.Procedure());
    l = std::fabs(l);
    return l / m_LaneChangeTimeMgr.Procedure();
  } else {
    return 0.0;
  }
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::Update(TimeParamManager const &timeMgr)
    TX_NOEXCEPT {
  // 先检查元素活跃
  if (CallSucc(IsAlive()) && CallFail(IsStop())) {
    mKinetics.m_lateral_acceleration = 0.0;
    CheckWayPoint();
    // 更新下次的方向
    UpdateNextCrossDirection();
    TX_MARK("update next cross direction info.");
    /*Step 1 Update Kinetics*/
    if (GetVelocity() <= 0.0) {
      mKinetics.m_velocity = 0.0;
      // 计算距离
      Compute_Displacement(timeMgr);
      RawVehicleCoord() =
          ComputeLaneOffset(mLocation.PosWithLateralWithoutOffset(), mLocation.vLaneDir(), mLocation.LaneOffset());
      // 位置同步
      SyncPosition(timeMgr.PassTime());
      mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
      return true;
    } else {
      if (MoveStraightOnS(timeMgr)) {
        // 判断车辆是否处于变道状态
        if (IsInLaneChange()) {
          txFloat last_lateral_velocity = mKinetics.m_LateralVelocity;
          mKinetics.m_LateralVelocity = MoveTo_Sideway(timeMgr);
          // 计算车辆的侧向加速度
          mKinetics.m_lateral_acceleration =
              (mKinetics.m_LateralVelocity - last_lateral_velocity) / timeMgr.RelativeTime();
          LOG(INFO) << TX_VARS(mKinetics.m_LateralVelocity) << TX_VARS(last_lateral_velocity)
                    << TX_VARS(mKinetics.m_lateral_acceleration);
        }
        // 限制车辆的速度
        mKinetics.LimitVelocity();

        // 判断车辆是否需要进行侧向距离的操作
        if (IsInLateralAction()) {
          UpdateLateralDistanceAction(timeMgr);
        }
        // 计算车辆的实际加速度
        if (mKinetics.m_acceleration > 0) {
          ComputeRealAcc(timeMgr);
        }
        TX_MARK("move to Component::Location, pImpl->NormalTargetRotateAngle();");
        // 执行交叉线变更后的操作
        LaneChangePostOp(timeMgr);
        mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
        return true;
      } else {
        StopVehicle();
        LogWarn << TX_VARS_NAME(vehicle_id, Id()) << " MoveStraightOnS failure. stop!";
        return false;
      }
    }
  } else {
    return (IsStop());
  }
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::Release() TX_NOEXCEPT {
  mLifeCycle.SetEnd();
  return true;
}

sim_msg::Car *TAD_UserDefined_VehicleElement::FillingElement(txFloat const timeStamp,
                                                             sim_msg::Car *pSimVehicle) TX_NOEXCEPT {
  pSimVehicle = ParentClass::FillingElement(timeStamp, pSimVehicle);
#if USE_VehicleKinectInfo
  if (NonNull_Pointer(pSimVehicle)) {
    // 如果车辆有跟随目标
    if (HasFollow()) {
      // 设置车辆的速度为mKinetics.m_show_velocity
      pSimVehicle->set_v(mKinetics.m_show_velocity);
      pSimVehicle->set_show_abs_velocity(mKinetics.m_show_velocity);
    } else {
      pSimVehicle->set_show_abs_velocity(GetVelocity());
    }
    // 设置车辆属性
    pSimVehicle->set_show_abs_acc(GetAcc());
    pSimVehicle->set_show_relative_velocity(show_relative_velocity_vertical());
    pSimVehicle->set_show_relative_velocity_horizontal(show_relative_velocity_horizontal());
    pSimVehicle->set_show_relative_dist_vertical(show_relative_dist_vertical());
    pSimVehicle->set_show_relative_dist_horizontal(show_relative_dist_horizontal());
    LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
        << TX_VARS(Id()) << TX_VARS(show_abs_acc()) << TX_VARS(show_abs_velocity())
        << TX_VARS(show_relative_velocity_horizontal()) << TX_VARS(show_relative_velocity_vertical())
        << TX_VARS(show_relative_dist_horizontal()) << TX_VARS(show_relative_dist_vertical());
  }
#endif /*USE_VehicleKinectInfo*/
  // 返回pSimVehicle指针
  return pSimVehicle;
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::FillingElement(
    Base::TimeParamManager const &timeMgr, sim_msg::Traffic &trafficData) TX_NOEXCEPT {
  return ParentClass::FillingElement(timeMgr, trafficData);
}

void TAD_UserDefined_VehicleElement::SetKinetics(Base::TimeParamManager const &timeMgr,
                                                 const KineticsInfo_t &info) TX_NOEXCEPT {
  /*mKinetics.m_velocity_last = mKinetics.m_velocity;*/
  // 根据info设置kinetics
  mKinetics.m_velocity = info.m_compute_velocity;
  mKinetics.m_show_velocity = info.m_show_velocity;
  // mKinetics.m_acceleration = 0.0; // info.m_acc;
  ComputeRealAcc(timeMgr);
  /*LOG(WARNING) << TX_VARS_NAME(user_define_id, Id()) << TX_VARS_NAME(m_compute_velocity, mKinetics.m_velocity)
      << TX_VARS_NAME(m_show_velocity, mKinetics.m_show_velocity);*/
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::Update_Kinetics(
    Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  // 如果车辆存活且未停止
  if (IsAlive() && !IsStop()) {
    /*mKinetics.m_velocity_last = mKinetics.m_velocity;*/
    // 更新车辆速度，速度等于当前速度加上加速度乘以相对时间
    mKinetics.m_velocity = mKinetics.m_velocity + mKinetics.m_acceleration * timeMgr.RelativeTime();
    mKinetics.LimitVelocity();
  } else {
    // 如果车辆未存活或已停止，将速度设置为0
    mKinetics.m_velocity = 0.0;
  }
  return true;
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::Pre_Update(
    const TimeParamManager &timeMgr, std::map<Elem_Id_Type, KineticsInfo_t> &map_elemId2Kinetics) TX_NOEXCEPT {
  ParentClass::Pre_Update(timeMgr, map_elemId2Kinetics);
  // 如果车辆有跟随目标
  if (CallSucc(HasFollow())) {
    // 如果车辆存活且未停止
    if (IsAlive() && CallFail(IsStop())) {
      // 获取跟随目标的元素ID
      const Base::txInt followElemId = GetFollow();
      // 如果在map_elemId2Kinetics中找到了跟随目标的元素ID
      if (map_elemId2Kinetics.end() != map_elemId2Kinetics.find(followElemId)) {
        // 获取跟随目标的运动学信息
        const auto &kineticsInfo = map_elemId2Kinetics.at(followElemId);
        // 设置车辆的运动学信息为跟随目标的运动学信息
        SetKinetics(timeMgr, kineticsInfo);
      } else {
        LogWarn << TX_VARS_NAME(Id, Id()) << " can not find follow info. " << TX_VARS_NAME(followElemId, followElemId);
      }
    }
  } else {
    // 更新车辆运动学信息
    Update_Kinetics(timeMgr);
    // 如果车辆有加速度结束条件
    if (CallSucc(HasAccEndCondition())) {
      // 获取加速度结束条件
      const AccEndConditionManager accEndCond = GetAccEndCondition();
      // 如果加速度结束条件类型为时间
      if ((_plus_(Base::ISceneLoader::acc_invalid_type::eTime)) == accEndCond.m_endCondition.m_type) {
        // 计算剩余时间
        const Base::txFloat remainTime = accEndCond.m_remainingTime - timeMgr.RelativeTime();
        if (remainTime > 0.0) {
          SetAccEndConditionRemainTime(remainTime);
        } else {
          // 如果剩余时间小于等于0，将加速度设置为0，清除加速度结束条件
          mKinetics.m_acceleration = 0.0;
          ClearAccEndCondition(timeMgr);
        }
      } else if ((_plus_(Base::ISceneLoader::acc_invalid_type::eVelocity)) ==
                 accEndCond.m_endCondition.m_type) {  // 如果加速度结束条件类型为速度
        LogInfo << TX_VARS(accEndCond.m_happenVelocity) << TX_VARS(accEndCond.m_targetVelocity)
                << TX_VARS(GetVelocity());
        LogInfo << TX_VARS((accEndCond.m_happenVelocity <= accEndCond.m_targetVelocity))
                << TX_VARS((GetVelocity() >= accEndCond.m_targetVelocity));

        // 如果车辆速度大于等于目标速度，将加速度设置为0，将车辆速度设置为目标速度，清除加速度结束条件
        if ((accEndCond.m_happenVelocity <= accEndCond.m_targetVelocity) &&
            (GetVelocity() >= accEndCond.m_targetVelocity)) {
          mKinetics.m_acceleration = 0.0;
          mKinetics.m_velocity = accEndCond.m_targetVelocity;
          ClearAccEndCondition(timeMgr);
        } else if ((accEndCond.m_happenVelocity >=
                    accEndCond.m_targetVelocity) &&  // 如果车辆速度小于等于目标速度，将加速度设置为0，
                                                     // // 将车辆速度设置为目标速度，清除加速度结束条件
                   (GetVelocity() <= accEndCond.m_targetVelocity)) {
          mKinetics.m_acceleration = 0.0;
          mKinetics.m_velocity = accEndCond.m_targetVelocity;
          ClearAccEndCondition(timeMgr);
        }
      }
    }
  }
  return true;
}

Base::txString TAD_UserDefined_VehicleElement::Str() const TX_NOEXCEPT { return ""; }

TAD_UserDefined_VehicleElement::EventHandlerType TAD_UserDefined_VehicleElement::GetEventHandlerType() const
    TX_NOEXCEPT {
  return EventHandlerType::hVehicle;
}

Base::txString TAD_UserDefined_VehicleElement::HandlerDesc() const TX_NOEXCEPT {
  return Base::txString("TAD_UserDefined_VehicleElement::HandlerDesc");
}

Base::txFloat TAD_UserDefined_VehicleElement::ComputeMinLaneChangeTime(const Base::txFloat laneChangeTime) TX_NOEXCEPT {
  Base::txFloat retTime = 0.0;
  // 如果车辆速度不为0
  if (Math::isNotZero(GetVelocity())) {
    TX_MARK("The road is too short to change lanes. JIRA-284");
    // 初始化距离到道路末尾的距离为100
    Base::txFloat distToRoadEnd = 100;
    // 如果车辆已到达道路最后一段且道路附近有交叉口
    if (mLocation.tracker()->isReachedRoadLastSection() &&
        HdMap::HadmapCacheConCurrent::IsRoadNearJunction(mLocation.LaneLocInfo().onLaneUid)) {
      // 获取道路末尾的坐标
      const Coord::txWGS84 curRoadEndPt =
          HdMap::HadmapCacheConCurrent::GetRoadEnd(mLocation.LaneLocInfo().onLaneUid.roadId);
      // 计算距离道路末尾的距离
      distToRoadEnd =
          Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(mLocation.PosOnLaneCenterLinePos(), curRoadEndPt) - 10.0;
    }

    // 如果距离道路末尾的距离小于0
    if (distToRoadEnd < 0.0) {
      return -1.0;
    }
    retTime = distToRoadEnd / GetVelocity();
    // 如果车辆到达道路末尾所需的时间小于给定的车道变更时间
    if (retTime < laneChangeTime) {
      // 如果车辆到达道路末尾所需的时间大于2秒
      if (retTime > 2.0) {
        return retTime;
      } else {
        // 返回-1表示无法进行车道变更
        return -1.0;
      }
    } else {
      // 返回给定的车道变更时间
      return laneChangeTime;
    }
  } else {
    return laneChangeTime;
  }
}

Base::txBool TAD_UserDefined_VehicleElement::ChangeInLane(const Base::Enums::VehicleMoveLaneState state,
                                                          Base::txFloat changeTime,
                                                          const Base::txFloat offset) TX_NOEXCEPT {
  using VehicleMoveLaneState = Base::Enums::VehicleMoveLaneState;
  // 如果车辆存活且车辆状态为左转或右转
  if (IsAlive() &&
      ((_plus_(VehicleMoveLaneState::eLeftInLane)) == state || (_plus_(VehicleMoveLaneState::eRightInLane)) == state)) {
    // 如果车辆正在进行车道变更
    if (IsInLaneChange()) {
      return false;
    } else if (changeTime <= 0.0) {  // 如果车道变更时间小于等于0
      return false;
    } else if (offset <= 0.0) {
      return false;
    }

    // 设置车道变更时间
    m_LaneChangeTimeMgr.SetDuration(changeTime);
    // 设置车道变更过程时间
    m_LaneChangeTimeMgr.SetProcedure(m_LaneChangeTimeMgr.Duration());
    // 设置车道变更最小取消时间
    m_LaneChangeTimeMgr.SetMinCancelDuration(m_LaneChangeTimeMgr.Duration());
    TX_MARK("do not Lane Aborting");
    m_LaneChangeTimeMgr.SetChangeInLaneAction(true);
    TX_MARK("Change in Lane.");
    m_LaneChangeTimeMgr.SetChangeInLaneOffset(offset);

    // 如果车辆状态为左转
    if ((_plus_(VehicleMoveLaneState::eLeftInLane)) == state) {
      SetTurnLeft_InLane_Start();
    } else if ((_plus_(VehicleMoveLaneState::eRightInLane)) == state) {
      SetTurnRight_InLane_Start();
    }
    return true;
  } else {
    LogWarn << "TAD_UserDefined_VehicleElement::ChangeInLane state error.";
    return false;
  }
}

Base::txBool TAD_UserDefined_VehicleElement::SwitchLane(const Base::Enums::VehicleMoveLaneState state,
                                                        Base::txBool bCheckGoalLane, Base::txBool bForce,
                                                        Base::txFloat laneChangeTime) TX_NOEXCEPT {
  txAssert(IsAlive());
  // 如果车辆状态为直行
  if ((_plus_(VehicleMoveLaneState::eStraight)) == state) {
    LOG_IF(INFO, FLAGS_LogLevel_LaneChange)
        << TX_VARS(Id()) << " set state is VEHICLE_MOVE_LANE_STRAIGHT. do not need switch lane.";
    return false;
  }
  // 如果车辆正在进行车道变更
  if (IsInLaneChange()) {
    LOG_IF(INFO, FLAGS_LogLevel_LaneChange) << "VEHICLE_MOVE_LANE_STRAIGHT != moveState, " << TX_VARS(State());
    return false;
  }

#if 1
  // 如果车辆在车道连接线上
  if (mLocation.IsOnLaneLink()) {
    LOG_IF(INFO, FLAGS_LogLevel_LaneChange) << TX_VARS(Id()) << " in lane link, cannot lane change.";
    return false;
  }
#  if 0 /*TADSIM-84*/
  if (mLocation.tracker()->isReachedRoadLastSection()) {
    const Base::txFloat distanceToRoadEnd =
        ((mLocation.tracker()->getCurrentLane()->getGeometry()->getLength()) - mLocation.DistanceAlongCurve());
    if (distanceToRoadEnd < FLAGS_RefuseLaneChangeDistance) {
      LOG_IF(INFO, FLAGS_LogLevel_LaneChange) << TX_VARS(Id()) << "near to junction , cannot lane change."
                                              << TX_VARS(distanceToRoadEnd) << TX_VARS(FLAGS_RefuseLaneChangeDistance);
      return false;
    } else {
      LOG_IF(INFO, FLAGS_LogLevel_LaneChange) << TX_VARS(distanceToRoadEnd);
    }
  }
#  endif
#endif

  // 如果车辆在车道连接线上
  if ((_plus_(VehicleMoveLaneState::eLeft)) == state) {
    TX_MARK("Left lane change");
    // 如果车辆已经在最左侧车道
    if (mLocation.IsOnLeftestLane()) {
      TX_MARK("Already the innermost lane");
      LOG_IF(INFO, FLAGS_LogLevel_LaneChange)
          << " IsOnLeftestLane : " << TX_VARS(Id()) << TX_VARS(mLocation.LaneLocInfo());
      return false;
    } else if (mLocation.tracker()->IsLeftBoundarySolid()) {  // 如果车辆左侧边界为实心
      LOG_IF(INFO, FLAGS_LogLevel_LaneChange)
          << " IsLeftBoundarySolid : " << TX_VARS(Id()) << TX_VARS(mLocation.LaneLocInfo());
      return false;
    }
  } else if ((_plus_(VehicleMoveLaneState::eRight)) == state) {  // 如果车辆状态为右转
    TX_MARK("Right lane change");
    // 如果车辆已经在最右侧车道
    if (mLocation.IsOnRightestLane()) {
      TX_MARK("It's on the last lane of the road, or the end of the road");
      LOG_IF(INFO, FLAGS_LogLevel_LaneChange)
          << " IsOnRightestLane : " << TX_VARS(Id()) << TX_VARS(mLocation.LaneLocInfo());
      return false;
    } else if (mLocation.tracker()->IsRightBoundarySolid()) {  // 如果车辆右侧边界为实心
      LOG_IF(INFO, FLAGS_LogLevel_LaneChange)
          << " IsRightBoundarySolid : " << TX_VARS(Id()) << TX_VARS(mLocation.LaneLocInfo());
      return false;
    }
  }

  // 计算最小车道变更时间
  laneChangeTime = ComputeMinLaneChangeTime(laneChangeTime);
#if 1
  // 如果车道变更时间小于等于0
  if (laneChangeTime <= 0.0) {
    LogWarn << TX_VARS(laneChangeTime) << " invalid.";
    return false;
  }
#endif
  // 设置车道变更时间管理器的持续时间
  m_LaneChangeTimeMgr.SetDuration(laneChangeTime);
  m_LaneChangeTimeMgr.SetProcedure(m_LaneChangeTimeMgr.Duration());
  // 设置车道变更时间管理器的最小取消持续时间
  m_LaneChangeTimeMgr.SetMinCancelDuration((Utils::VehicleBehaviorCfg::NoCheckLCAbortThreshold()) *
                                           m_LaneChangeTimeMgr.Duration());  // 1229 HD

  // 如果车辆状态为左转
  if ((_plus_(VehicleMoveLaneState::eLeft)) == state) {
    // 设置左转开始
    SetTurnLeft_Start();
  } else if ((_plus_(VehicleMoveLaneState::eRight)) == state) {  // 如果车辆状态为右转
    SetTurnRight_Start();
  }
  return true;
}

Base::txBool TAD_UserDefined_VehicleElement::HandlerEvent(Base::TimeParamManager const &timeMgr,
                                                          ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT {
  if (IsAlive()) {
    using IActionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IActionViewerPtr;
    using IEndConditionViewerPtr = Base::ISceneLoader::ISceneEventViewer::IEndConditionViewerPtr;
    using EventActionInfoVec = Base::ISceneLoader::ISceneEventViewer::IActionViewer::EventActionInfoVec;
    using EndConditionVec = Base::ISceneLoader::ISceneEventViewer::IEndConditionViewer::EndConditionVec;
    using SceneEventActionType = Base::Enums::SceneEventActionType;
    using VehicleMoveLaneState = Base::Enums::VehicleMoveLaneState;
    using EndConditionKeyType = Base::Enums::EndConditionKeyType;
    using acc_invalid_type = Base::Enums::acc_invalid_type;

    const Base::txSysId evId = _eventPtr->id();
    const Base::txString elemType_elemId = TAD_SceneEvent_1_0_0_0::Make_ElemType_ElemId(Type(), Id());
    // 获取action和endcondition条件视图
    IActionViewerPtr actionPtr = _eventPtr->action();
    IEndConditionViewerPtr endCondPtr = _eventPtr->endCondition();
    // 判断action和endcondition视图指针不为空
    if (NonNull_Pointer(actionPtr) && NonNull_Pointer(endCondPtr)) {
      const EventActionInfoVec &actionVec = actionPtr->actionList();
      const EndConditionVec &endCondVec = endCondPtr->endConditionList();
      // 判断action和endcondtion的数量相同
      if (actionVec.size() == endCondVec.size()) {
        // 遍历action视图
        for (size_t idx = 0; idx < actionVec.size(); ++idx) {
          const EventActionInfoVec::value_type &refAct = actionVec[idx];
          const EndConditionVec::value_type &refEndCond = endCondVec[idx];
          const Base::txInt actionId = refAct.action_id();
          const Base::txString evId_actionId = TAD_SceneEvent_1_0_0_0::Make_evId_actId(evId, actionId);
#if __TX_Mark__("element_state")
          // 更新场景事件状态
          TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(
              evId_actionId,
              TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
                  elemType_elemId, _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::pendingState),
                  timeMgr.PassTime()));
          const SceneEventStateInfo sceneEventStateInfo_running = TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
              elemType_elemId, _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::runningState),
              timeMgr.PassTime());
#endif /*__TX_Mark__("element_state")*/
          /*std::tuple<SceneEventActionType, VehicleMoveLaneState, Base::txFloat>;*/
          /*std::tuple<EndConditionKeyType, Base::txFloat>;*/
          // action类型是acc时
          if (_plus_(SceneEventActionType::acc) == refAct.action_type()) {
            mKinetics.m_acceleration = refAct.value();

            // endcondition类型是time时
            if ((_plus_(EndConditionKeyType::time)) == std::get<0>(refEndCond)) {
              // 设置对应的acc结束条件管理器
              AccEndConditionManager accMgr;
              accMgr.m_endCondition.m_IsValid = true;
              accMgr.m_endCondition.m_type = _plus_(acc_invalid_type::eTime);
              accMgr.m_endCondition.m_endCondition = std::get<1>(refEndCond);
              accMgr.m_remainingTime = std::get<1>(refEndCond);
              accMgr.m_happenVelocity = GetVelocity();
              accMgr.m_targetVelocity = 0.0;
              m_optional_AccEndCondition = accMgr;
#if __TX_Mark__("element_state")
              // 更新场景事件状态
              TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
              if (NonNull_Pointer(m_AccActionManagerPtr)) {
                ClearAccAction(timeMgr); /*clear last acc ation*/
              }
              m_AccActionManagerPtr = std::make_shared<ActionManager>();
              // accAction管理器初始化
              m_AccActionManagerPtr->Initialize(evId_actionId, elemType_elemId);
#endif /*__TX_Mark__("element_state")*/
            } else if ((_plus_(EndConditionKeyType::velocity)) == std::get<0>(refEndCond)) {  // 结束条件类型时速度时
              // 设置对应的结束条件管理器
              AccEndConditionManager accMgr;
              accMgr.m_endCondition.m_IsValid = true;
              accMgr.m_endCondition.m_type = _plus_(acc_invalid_type::eVelocity);
              accMgr.m_endCondition.m_endCondition = std::get<1>(refEndCond);

              accMgr.m_remainingTime = 0.0;
              accMgr.m_happenVelocity = GetVelocity();
              accMgr.m_targetVelocity = std::get<1>(refEndCond);
              m_optional_AccEndCondition = accMgr;
              LogInfo << TX_VARS(accMgr.m_endCondition.m_type) << TX_VARS(accMgr.m_happenVelocity)
                      << TX_VARS(accMgr.m_targetVelocity) << TX_VARS(m_optional_AccEndCondition.has_value());

#if __TX_Mark__("element_state")
              // 更新场景事件状态
              TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
              if (NonNull_Pointer(m_AccActionManagerPtr)) {
                ClearAccAction(timeMgr); /*clear last acc ation*/
              }
              m_AccActionManagerPtr = std::make_shared<ActionManager>();
              m_AccActionManagerPtr->Initialize(evId_actionId, elemType_elemId);
#endif /*__TX_Mark__("element_state")*/
            }
            LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
                << "[event_handler_vehicle_acc_event], id = " << Id() << ", Set Acc_event = " << (GetAcc());
          } else if (_plus_(SceneEventActionType::velocity) == refAct.action_type()) {  // 如果事件行为类型时速度
            mKinetics.m_velocity = refAct.value();
            LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
                << "[event_handler_vehicle_velocity], id = " << Id() << ", Set Velocity = " << (GetVelocity());
#if __TX_Mark__("element_state")
            // 更新场景事件状态
            TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(
                evId_actionId,
                TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
                    elemType_elemId, _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::completeState),
                    timeMgr.PassTime()));
#endif /*__TX_Mark__("element_state")*/
          } else if (_plus_(SceneEventActionType::crash_stop_renew) ==
                     refAct.action_type()) {  // 场景事件行为类型是crashstoprenew时
            // 设置车辆的角度和速度
            mLocation.fAngle().FromDegree(refAct.value());
            mKinetics.m_velocity = refAct.lane_offset();
            // 设置车辆的行驶状态
            SetDrivingStatus(refAct.driving_status());
            LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
                << "[event_handler_crash_stop], id = " << Id() << TX_VARS(mLocation.fAngle()) << TX_VARS(GetVelocity())
                << TX_VARS(StrDrivingStatus());
            // 如果车辆的行驶状态为停车碰撞
            if (_plus_(DrivingStatus::stop_crash) == Driving_Status()) {
              // 定义影响车道的变量
              HdMap::HadmapCacheConCurrent::evId2RefuseChangeLaneSet::mapped_type influnce_lane;
              // 将特定的车道添加到影响车道列表中
              influnce_lane.emplace_back(
                  std::make_tuple(Base::txLaneUId(10002319, 0, -1), 0.0, FLAGS_event_99930_99931_influnce_distance));
              influnce_lane.emplace_back(
                  std::make_tuple(Base::txLaneUId(10002319, 0, -2), 0.0, FLAGS_event_99930_99931_influnce_distance));
              influnce_lane.emplace_back(
                  std::make_tuple(Base::txLaneUId(10002319, 0, -3), 0.0, FLAGS_event_99930_99931_influnce_distance));
              HdMap::HadmapCacheConCurrent::AddSpecialRefuseChangeLane(999, influnce_lane,
                                                                       HdMap::HadmapCacheConCurrent::SwitchDir::eRight);
            } else if (_plus_(DrivingStatus::normal) == Driving_Status()) {  // 如果车辆的行驶状态为正常
              HdMap::HadmapCacheConCurrent::RemoveSpecialRefuseChangeLane(999);
            }
          } else if (_plus_(SceneEventActionType::merge) == refAct.action_type()) {  // 场景事件行为类型时merge时
            // 处于车道保持
            if (CallSucc(IsLaneKeep())) {
              Base::txBool const bCheckGoalLane = true;
              // 如果合并类型是左转或右转
              if ((_plus_(VehicleMoveLaneState::eLeft) == refAct.merge_type()) ||
                  (_plus_(VehicleMoveLaneState::eRight) == refAct.merge_type())) {
                // 根据合并类型进行道路切换
                if (SwitchLane(refAct.merge_type(), bCheckGoalLane, true, refAct.value())) {
                  LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
                      << "[event_handler_vehicle_merge], id = " << Id()
                      << TX_VARS_NAME(Direction, (__enum2lpsz__(VehicleMoveLaneState, refAct.merge_type())))
                      << TX_VARS_NAME(Duration, (refAct.value())) << ", Run  Success.";
#if __TX_Mark__("element_state")
                  // 更新场景事件状态
                  TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
                  if (NonNull_Pointer(m_MergeActionManagerPtr)) {
                    ClearMergeAction(timeMgr); /*clear last acc ation*/
                    LOG(WARNING) << "another merge action is running.";
                    /*un-impossible*/
                  }
                  // 初始化合并行为管理器
                  m_MergeActionManagerPtr = std::make_shared<ActionManager>();
                  m_MergeActionManagerPtr->Initialize(evId_actionId, elemType_elemId);
#endif /*__TX_Mark__("element_state")*/
                  /*return true;*/ TX_MARK("MS_100375");
                } else {
                  LOG(INFO) << "[event_handler_vehicle_merge], id = " << Id()
                            << TX_VARS_NAME(Direction, (__enum2lpsz__(VehicleMoveLaneState, refAct.merge_type())))
                            << TX_VARS_NAME(Duration, (refAct.value())) << ", Run  Failure.";
                  /*return false;*/ TX_MARK("MS_100375");
                }
              } else if ((_plus_(VehicleMoveLaneState::eLeftInLane) == refAct.merge_type()) ||
                         (_plus_(VehicleMoveLaneState::eRightInLane) ==
                          refAct.merge_type())) {  // 合并类型时左换道或右换道时
                // 进行换道成功
                if (ChangeInLane(refAct.merge_type(), refAct.value(), refAct.lane_offset())) {
                  LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
                      << "[event_handler_vehicle_merge], id = " << Id()
                      << TX_VARS_NAME(Direction, (__enum2lpsz__(VehicleMoveLaneState, refAct.merge_type())))
                      << TX_VARS_NAME(Duration, (refAct.value())) << TX_VARS_NAME(Offset, (refAct.lane_offset()))
                      << ", Run  Success.";
#if __TX_Mark__("element_state")
                  // 更新场景事件状态
                  TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
                  if (NonNull_Pointer(m_MergeActionManagerPtr)) {
                    ClearMergeAction(timeMgr); /*clear last acc ation*/
                    LOG(WARNING) << "another merge action is running.";
                    /*un-impossible*/
                  }
                  // 初始化合并行为管理
                  m_MergeActionManagerPtr = std::make_shared<ActionManager>();
                  m_MergeActionManagerPtr->Initialize(evId_actionId, elemType_elemId);
#endif /*__TX_Mark__("element_state")*/
                  /*return true;*/ TX_MARK("MS_100375");
                } else {
                  LOG(INFO) << "[event_handler_vehicle_merge], id = " << Id()
                            << TX_VARS_NAME(Direction, (__enum2lpsz__(VehicleMoveLaneState, refAct.merge_type())))
                            << TX_VARS_NAME(Duration, (refAct.value())) << TX_VARS_NAME(Offset, (refAct.lane_offset()))
                            << ", Run  Failure.";
                  /*return false;*/ TX_MARK("MS_100375");
                }
              }
              /*return false;*/ TX_MARK("MS_100375");
            } else {
              LOG(WARNING) << "[event_handler_vehicle_merge]" << TX_VARS(Id()) << TX_VARS(timeMgr.PassTime())
                           << TX_VARS_NAME(State, State()._to_string()) << " can not execute merge event."
                           << TX_VARS(evId) << TX_VARS(actionId);
              /*return false;*/ TX_MARK("MS_100375");
            }
          } else if (_plus_(SceneEventActionType::lateralDistance) ==
                     refAct.action_type()) {  // 场景事件行为类型是侧向距离时
            // 如果车道保持情况
            if (CallSucc(IsLaneKeep())) {
              m_LateralActionMgrPtr = nullptr;
              // 判断目标元素指针不为空
              if (NonNull_Pointer(refAct.target_element_ptr())) {
                // 创建一个新的LateralDistanceActionManager对象
                m_LateralActionMgrPtr = std::make_shared<LateralDistanceActionManager>();
                // 如果LateralDistanceActionManager对象不为空且初始化成功
                if (NonNull_Pointer(m_LateralActionMgrPtr) &&
                    CallSucc(m_LateralActionMgrPtr->Initialize(evId_actionId, elemType_elemId, refAct.target_element(),
                                                               refAct.target_element_ptr(), refAct.distance(),
                                                               refAct.maxAcc(), refAct.maxDec(), refAct.maxSpeed(),
                                                               refAct.freespace(), refAct.continuous()))) {
                  // 设置车辆的横向行动
                  SetLateralAction();
                  // 更新场景事件状态
                  TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
                  LateralActionInfo << "[event_handler_vehicle_lateralDistance]" << TX_VARS(timeMgr.PassTime())
                                    << TX_VARS_NAME(State, State()._to_string()) << " create success." << TX_VARS(evId)
                                    << TX_VARS(actionId);
                  /*return true;*/ TX_MARK("MS_100375");
                } else {
                  m_LateralActionMgrPtr = nullptr;
                  LateralActionInfo << "[event_handler_vehicle_lateralDistance]" << TX_VARS(timeMgr.PassTime())
                                    << TX_VARS_NAME(State, State()._to_string())
                                    << " can not execute lateralDistance event." << TX_VARS(evId) << TX_VARS(actionId)
                                    << " LateralDistanceActionManager init failure."
                                    << TX_VARS(refAct.target_element());
                  /*return false;*/ TX_MARK("MS_100375");
                }
              } else {
                LateralActionInfo << "[event_handler_vehicle_lateralDistance]" << TX_VARS(timeMgr.PassTime())
                                  << TX_VARS_NAME(State, State()._to_string())
                                  << " can not execute lateralDistance event." << TX_VARS(evId) << TX_VARS(actionId)
                                  << " do not find target element." << TX_VARS(refAct.target_element());
                /*return false;*/ TX_MARK("MS_100375");
              }
            } else {
              LateralActionInfo << "[event_handler_vehicle_lateralDistance]" << TX_VARS(timeMgr.PassTime())
                                << TX_VARS_NAME(State, State()._to_string())
                                << " can not execute lateralDistance event." << TX_VARS(evId) << TX_VARS(actionId);
              /*return false;*/ TX_MARK("MS_100375");
            }
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
}

// 事件处理器的实现
Base::txBool TAD_UserDefined_VehicleElement::HandlerEvent(IEvent &_event) TX_NOEXCEPT {
  if (IsAlive()) {
    // eAcc事件发生时
    if (IEvent::EventType::eAcc == _event.GetEventType()) {
      HandlerEAcc(_event);
      return true;
    } else if (IEvent::EventType::eAccEvent == _event.GetEventType()) {  // eAccEvent事件发生时
      HandlerEAccEvent(_event);
      return true;
    } else if (IEvent::EventType::eVelocity == _event.GetEventType()) {  // 事件类型是eVelocity
      TAD_VelocityEvent &refAccEvent = dynamic_cast<TAD_VelocityEvent &>(_event);
      mKinetics.m_velocity = refAccEvent.GetVelocityValue();
      LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
          << "[event_handler_vehicle_velocity], id = " << Id() << ", Set Velocity = " << (GetVelocity());
      return true;
    } else if (IEvent::EventType::eVelocityEvent == _event.GetEventType()) {  // 事件类型是eVelocityEvent
      TAD_Velocity_EventEvent &refAccEventEvent = dynamic_cast<TAD_Velocity_EventEvent &>(_event);
      mKinetics.m_velocity = refAccEventEvent.GetVelocityValue();
      LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
          << "[event_handler_vehicle_velocity_event], id = " << Id() << ", Set Velocity_event = " << (GetVelocity());
      return true;
    } else if (IEvent::EventType::eMerge == _event.GetEventType()) {  // 事件类型是eMerge
      HandlerEMerge(_event);
      return true;
    } else if (IEvent::EventType::eMergeEvent == _event.GetEventType()) {  // 事件类型时mergeEvent时
      HandlerEMergeEvent(_event);
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void TAD_UserDefined_VehicleElement::HandlerEAcc(IEvent &_event) TX_NOEXCEPT {
  TAD_AccEvent &refAccEvent = dynamic_cast<TAD_AccEvent &>(_event);
  mKinetics.m_acceleration = refAccEvent.GetAccValue();
  const Base::ISceneLoader::EventEndCondition_t endCond = refAccEvent.GetEndCondition();
  if (endCond.m_IsValid) {
    // 结束条件类型是eTime
    if ((_plus_(Base::ISceneLoader::acc_invalid_type::eTime)) == endCond.m_type) {
      AccEndConditionManager accMgr;
      accMgr.m_endCondition = endCond;
      accMgr.m_remainingTime = endCond.m_endCondition;
      accMgr.m_happenVelocity = GetVelocity();
      accMgr.m_targetVelocity = 0.0;
      m_optional_AccEndCondition = accMgr;
    } else if ((_plus_(Base::ISceneLoader::acc_invalid_type::eVelocity)) ==
               endCond.m_type) {  // 结束条件类型是eVelocity
      AccEndConditionManager accMgr;
      accMgr.m_endCondition = endCond;
      accMgr.m_remainingTime = 0.0;
      accMgr.m_happenVelocity = GetVelocity();
      accMgr.m_targetVelocity = endCond.m_endCondition;
      m_optional_AccEndCondition = accMgr;
    } else {
      m_optional_AccEndCondition = boost::none;
    }
  } else {
    m_optional_AccEndCondition = boost::none;
  }
  LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
      << "[event_handler_vehicle_acc], id = " << Id() << ", Set Acc = " << (GetAcc());
}

void TAD_UserDefined_VehicleElement::HandlerEAccEvent(IEvent &_event) TX_NOEXCEPT {
  TAD_Acc_EventEvent &refAccEventEvent = dynamic_cast<TAD_Acc_EventEvent &>(_event);
  mKinetics.m_acceleration = refAccEventEvent.GetAccValue();
  const Base::ISceneLoader::EventEndCondition_t endCond = refAccEventEvent.GetEndCondition();
  if (endCond.m_IsValid) {
    // 加速事件结束条件类型为时间
    if ((_plus_(Base::ISceneLoader::acc_invalid_type::eTime)) == endCond.m_type) {
      AccEndConditionManager accMgr;
      accMgr.m_endCondition = endCond;
      accMgr.m_remainingTime = endCond.m_endCondition;
      accMgr.m_happenVelocity = GetVelocity();
      accMgr.m_targetVelocity = 0.0;
      m_optional_AccEndCondition = accMgr;
    } else if ((_plus_(Base::ISceneLoader::acc_invalid_type::eVelocity)) ==
               endCond.m_type) {  // 加速事件结束条件类型为速度
      AccEndConditionManager accMgr;
      accMgr.m_endCondition = endCond;
      accMgr.m_remainingTime = 0.0;
      accMgr.m_happenVelocity = GetVelocity();
      accMgr.m_targetVelocity = endCond.m_endCondition;
      m_optional_AccEndCondition = accMgr;
    } else {
      m_optional_AccEndCondition = boost::none;
    }
  } else {
    m_optional_AccEndCondition = boost::none;
  }
  LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
      << "[event_handler_vehicle_acc_event], id = " << Id() << ", Set Acc_event = " << (GetAcc());
}

void TAD_UserDefined_VehicleElement::HandlerEMerge(IEvent &_event) TX_NOEXCEPT {
  TAD_VehicleMergeEvent &refMergeEvent = dynamic_cast<TAD_VehicleMergeEvent &>(_event);
  Base::txBool const bCheckGoalLane = true;
  // 合并事件的方向值时左转或右转时
  if ((_plus_(Base::Enums::VehicleMoveLaneState::eRight)) == refMergeEvent.GetDirectionValue() ||
      (_plus_(Base::Enums::VehicleMoveLaneState::eLeft)) == refMergeEvent.GetDirectionValue()) {
    // 切换车道成功
    if (SwitchLane(refMergeEvent.GetDirectionValue(), bCheckGoalLane, true, refMergeEvent.GetCurDuration())) {
      // 记录日志
      LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
          << "[event_handler_vehicle_merge], id = " << Id()
          << TX_VARS_NAME(Direction, (refMergeEvent.GetDirectionValue()))
          << TX_VARS_NAME(Duration, (refMergeEvent.GetCurDuration()))
          << TX_VARS_NAME(Offset, (refMergeEvent.GetCurOffset())) << ", Run  Success.";
    } else {
      LOG(INFO) << "[event_handler_vehicle_merge], id = " << Id()
                << TX_VARS_NAME(Direction, (refMergeEvent.GetDirectionValue()))
                << TX_VARS_NAME(Duration, (refMergeEvent.GetCurDuration()))
                << TX_VARS_NAME(Offset, (refMergeEvent.GetCurOffset())) << ", Run  Failure.";
    }
  } else if ((_plus_(Base::Enums::VehicleMoveLaneState::eRightInLane)) == refMergeEvent.GetDirectionValue() ||
             (_plus_(Base::Enums::VehicleMoveLaneState::eLeftInLane)) ==
                 refMergeEvent.GetDirectionValue()) {  // 合并事件的方向值时左换道或右换道时
    // 换道
    if (ChangeInLane(refMergeEvent.GetDirectionValue(), refMergeEvent.GetCurDuration(), refMergeEvent.GetCurOffset())) {
      // 记录日志
      LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
          << "[event_handler_vehicle_merge], id = " << Id()
          << TX_VARS_NAME(Direction, (refMergeEvent.GetDirectionValue()))
          << TX_VARS_NAME(Duration, (refMergeEvent.GetCurDuration()))
          << TX_VARS_NAME(Offset, (refMergeEvent.GetCurOffset())) << ", Run  Success.";
    } else {
      LOG(INFO) << "[event_handler_vehicle_merge], id = " << Id()
                << TX_VARS_NAME(Direction, (refMergeEvent.GetDirectionValue()))
                << TX_VARS_NAME(Duration, (refMergeEvent.GetCurDuration()))
                << TX_VARS_NAME(Offset, (refMergeEvent.GetCurOffset())) << ", Run  Failure.";
    }
  }
}

void TAD_UserDefined_VehicleElement::HandlerEMergeEvent(IEvent &_event) TX_NOEXCEPT {
  TAD_VehicleMergeEventEvent &refMergeEvent = dynamic_cast<TAD_VehicleMergeEventEvent &>(_event);
  Base::txBool const bCheckGoalLane = true;
  // 合并事件的方向值时左转或右转时
  if ((_plus_(Base::Enums::VehicleMoveLaneState::eRight)) == refMergeEvent.GetDirectionValue() ||
      (_plus_(Base::Enums::VehicleMoveLaneState::eLeft)) == refMergeEvent.GetDirectionValue()) {
    // 切换车道成功
    if (SwitchLane(refMergeEvent.GetDirectionValue(), bCheckGoalLane, true, refMergeEvent.GetCurDuration())) {
      LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
          << "[event_handler_vehicle_merge], id = " << Id()
          << TX_VARS_NAME(Direction, (refMergeEvent.GetDirectionValue()))
          << TX_VARS_NAME(Duration, (refMergeEvent.GetCurDuration()))
          << TX_VARS_NAME(Offset, (refMergeEvent.GetCurOffset())) << ", Run  Success.";
    } else {
      LOG(INFO) << "[event_handler_vehicle_merge], id = " << Id()
                << TX_VARS_NAME(Direction, (refMergeEvent.GetDirectionValue()))
                << TX_VARS_NAME(Duration, (refMergeEvent.GetCurDuration()))
                << TX_VARS_NAME(Offset, (refMergeEvent.GetCurOffset())) << ", Run  Failure.";
    }
  } else if ((_plus_(Base::Enums::VehicleMoveLaneState::eRightInLane)) == refMergeEvent.GetDirectionValue() ||
             (_plus_(Base::Enums::VehicleMoveLaneState::eLeftInLane)) ==
                 refMergeEvent.GetDirectionValue()) {  // 合并事件的方向值时左换道或右换道时
    // 换道
    if (ChangeInLane(refMergeEvent.GetDirectionValue(), refMergeEvent.GetCurDuration(), refMergeEvent.GetCurOffset())) {
      // 记录事件信息
      LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
          << "[event_handler_vehicle_merge], id = " << Id()
          << TX_VARS_NAME(Direction, (refMergeEvent.GetDirectionValue()))
          << TX_VARS_NAME(Duration, (refMergeEvent.GetCurDuration()))
          << TX_VARS_NAME(Offset, (refMergeEvent.GetCurOffset())) << ", Run  Success.";
    } else {
      LOG(INFO) << "[event_handler_vehicle_merge], id = " << Id()
                << TX_VARS_NAME(Direction, (refMergeEvent.GetDirectionValue()))
                << TX_VARS_NAME(Duration, (refMergeEvent.GetCurDuration()))
                << TX_VARS_NAME(Offset, (refMergeEvent.GetCurOffset())) << ", Run  Failure.";
    }
  }
}

TAD_UserDefined_VehicleElement::EventHitUtilInfo_t TAD_UserDefined_VehicleElement::GetEventHitUtilInfo() const
    TX_NOEXCEPT {
  // 定义事件命中实用信息变量
  EventHitUtilInfo_t retV;
  retV._ElemId = Id();
  Coord::txWGS84 geom_center_wgs84 = GeomCenter();
  retV._ElementGeomCenter = geom_center_wgs84.ToENU();
  retV._Velocity = mKinetics.m_velocity;
  retV._vecPolygon = mGeometryData.Polygon();
  return retV;
}

Base::txSysId TAD_UserDefined_VehicleElement::HanderId() const TX_NOEXCEPT { return Id(); }

Base::txSysId TAD_UserDefined_VehicleElement::HanderSysId() const TX_NOEXCEPT { return SysId(); }

TAD_UserDefined_VehicleElement::VehicleMoveLaneState TAD_UserDefined_VehicleElement::GetSwitchLaneState() const
    TX_NOEXCEPT {
  // 如果处于车道改变状态
  if (IsInLaneChange()) {
    // 是否左右转
    if (IsInTurnLeft()) {
      return VehicleMoveLaneState::eLeft;
    } else if (IsInTurnRight()) {
      return VehicleMoveLaneState::eRight;
    } else if (IsInTurnLeft_InLane()) {  // 是否左右换道
      return VehicleMoveLaneState::eLeftInLane;
    } else if (IsInTurnRight_InLane()) {
      return VehicleMoveLaneState::eRightInLane;
    } else {
      LOG(FATAL) << "State machine error.";
      return VehicleMoveLaneState::eStraight;
    }
  } else {
    return VehicleMoveLaneState::eStraight;
  }
}

#if USE_VehicleKinectInfo

TAD_UserDefined_VehicleElement::KineticsUtilInfo_t TAD_UserDefined_VehicleElement::GetKineticsUtilInfo() const
    TX_NOEXCEPT {
  return KineticsUtilInfo_t();
}

Base::txBool TAD_UserDefined_VehicleElement::ComputeKinetics(Base::TimeParamManager const &timeMgr,
                                                             const KineticsUtilInfo_t &_objInfo) TX_NOEXCEPT {
  m_show_abs_velocity = GetVelocity();
  m_show_abs_acc = GetAcc();
  // 如果相对时间大于0且对象信息有效
  if (timeMgr.RelativeTime() > 0.0 && CallSucc(_objInfo.m_isValid) /*&&
                                      CallFail(_objInfo._vecPolygon.empty()) &&
                                      CallFail(GetPolygon().empty())*/
  ) {
    const Base::txVec2 _adc_center = _objInfo._ElementGeomCenter.ENU2D();
    /*const Base::txVec2 _last_adc_center = _objInfo._LastElementPos.GetLocalPos_2D();*/
    const Base::txVec2 _obs_center = GeomCenter().ToENU().ENU2D();
    /*const Base::txVec2 _last_obs_center = pImpl->m_elemRoadInfo.lastPos.GetLocalPos_2D();*/
    const Base::txMat2 &tranMatInv = _objInfo._TransMatInv;

    const Base::txVec2 local_adc_center(0.0, 0.0);
    /*const Base::txVec2 local_last_adc_center = (_last_adc_center - _adc_center).transpose() * tranMatInv;*/
    const Base::txVec2 &local_obs_center = (_obs_center - _adc_center).transpose() * tranMatInv;
    /*const Base::txVec2 local_last_obs_center = (_last_obs_center - _adc_center).transpose() * tranMatInv;*/

    // 设置显示的相对水平距离和垂直距离
    m_show_relative_dist_horizontal = local_obs_center.x() /** -1.0*/;
    m_show_relative_dist_vertical = local_obs_center.y();
    // 设置显示的绝对海拔高度和相对海拔高度
    m_show_abs_dist_elevation = Altitude();
    m_show_relative_dist_elevation = show_abs_dist_elevation() - _objInfo._ElementGeomCenter.GetWGS84().Alt();

    // const Base::txVec2 local_adc_velocity = (local_adc_center - local_last_adc_center) / timeMgr.relativeTime;
    // 计算相对速度
    Base::txVec2 local_relative_velocity;
    if (Math::isNotZero(m_local_last_obs_center.x()) && Math::isNotZero(m_local_last_obs_center.y())) {
      local_relative_velocity = (local_obs_center - m_local_last_obs_center) / timeMgr.RelativeTime();
    } else {
      local_relative_velocity.setZero();
    }

    // const Base::txVec2 local_relative_velocity = (local_obs_velocity - local_adc_velocity);

    // 设置显示的相对水平速度和垂直速度
    m_show_relative_velocity_horizontal = local_relative_velocity.x() * -1.0;
    m_show_relative_velocity_vertical = local_relative_velocity.y();

    m_local_last_obs_center = local_obs_center;
  } else {
    m_show_relative_dist_horizontal = 0.0;
    m_show_relative_dist_vertical = 0.0;
    m_show_relative_velocity_horizontal = 0.0;
    m_show_relative_velocity_vertical = 0.0;
  }
  return true;
}

#endif /*USE_VehicleKinectInfo*/

Geometry::Topology::txEdgeVec TAD_UserDefined_VehicleElement::GetTopologyRelationShip() const TX_NOEXCEPT {
  // 定义拓扑边的向量
  Geometry::Topology::txEdgeVec edges;
  edges.push_back(Geometry::Topology::txEdge(SysRootId(), SysId()));
  Base::txSysId followSysId;
  // 如果车辆行为为用户自定义，且存在跟随对象，且获取跟随对象的系统ID成功
  if (_plus_(VEHICLE_BEHAVIOR::eUserDefined) == VehicleBehavior() && HasFollow() &&
      GetSysIdByElemId((GetFollow()), followSysId)) {
    // 将跟随对象的系统ID和当前系统ID作为一条拓扑边添加到向量中
    edges.push_back(Geometry::Topology::txEdge(followSysId, SysId()));
  }
  txAssert(0 < edges.size());
  return edges;
}

void TAD_UserDefined_VehicleElement::UpdateNextCrossDirection() TX_NOEXCEPT {
  if (mLocation.IsOnLane()) {
    m_op_self_next_driving_direction.clear_value();
    // 更新最后一个在车道上的信息
    JunctionInfo_SetLastOnLaneInfo(GeomCenter().ToENU(), GetLaneDir());
  } else {
    // 如果没有下一个路口方向
    if (CallFail(m_op_self_next_driving_direction.has_value())) {
      // 获取起始车道和目标车道
      hadmap::txLanePtr pFromLane =
          HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLinkFromLaneUid);
      hadmap::txLanePtr pToLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLinkToLaneUid);

      // 获取起始车道和目标车道的方向
      const Base::txVec3 fromLaneDir =
          HdMap::HadmapCacheConCurrent::GetLaneDir(pFromLane, pFromLane->getLength() - 1.0);
      const Base::txVec3 toLaneDir = HdMap::HadmapCacheConCurrent::GetLaneDir(pToLane, 1.0);

      m_op_self_next_driving_direction = Base::GetSelfDrivingDirection(fromLaneDir, toLaneDir);
    }
  }
}

void TAD_UserDefined_VehicleElement::ResetLaneKeep() TX_NOEXCEPT {
  mLaneKeepMgr.ResetCounter(Utils::VehicleBehaviorCfg::LaneKeep(), m_aggress);
}

TAD_UserDefined_VehicleElement::txBool TAD_UserDefined_VehicleElement::FillingTrajectory(
    Base::TimeParamManager const &timeMgr, sim_msg::Trajectory &refTraj) TX_NOEXCEPT {
  refTraj.Clear();
  // 保存稳定状态
  SaveStableState();
  // 如果车辆存活
  if (IsAlive()) {
    using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
#if 1
    /*for lane change finish*/
    // 更新车辆位置的哈希信息
    UpdateHashedLaneInfo(mLocation);
    const HashedLaneInfo &selfStableHashedLaneInfo = StableHashedLaneInfo();
    // 更新哈希道路信息缓存
    UpdateHashedRoadInfoCache(selfStableHashedLaneInfo);
#endif
    // 获取起始哈希节点指针
    HashedLaneInfoOrthogonalListPtr start_hashed_node_ptr = Weak2SharedPtr(std::get<1>(m_HashedRoadInfoCache));

    // 如果车辆在左转
    if (IsInTurnLeft() || IsInTurnLeft_InLane()) {
      const HashedLaneInfo leftHashedLaneInfo = mLocation.GetLeftHashedLaneInfo(Id());
      start_hashed_node_ptr = HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(leftHashedLaneInfo);
    } else if (IsInTurnRight() || IsInTurn_LeftRight_InLane()) {  // 如果车辆在右转
      const HashedLaneInfo rightHashedLaneInfo = mLocation.GetRightHashedLaneInfo(Id());
      start_hashed_node_ptr = HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(rightHashedLaneInfo);
    }

    // 如果起始哈希节点指针不为空
    if (NonNull_Pointer(start_hashed_node_ptr)) {
      // 计算车辆在车道上的距离
      const Base::txFloat reverse_base = DistanceAlongCurve() - start_hashed_node_ptr->GetSelfHashedInfo().StartScope();
      Base::txFloat cur_velocity = __HYPOT__(GetVelocity(), GetLateralVelocity());
      // 计算轨迹长度
      const Base::txFloat traj_length = cur_velocity * FLAGS_vehicle_trajectory_duration_s + 10.0;
      // 计算实际搜索距离
      const Base::txFloat real_search_dist = reverse_base + traj_length;

      std::list<HashedLaneInfoOrthogonalListPtr> search_result_by_level;
      static const Base::txBool bCheckReachable = true;
      static const Base::txBool bDoNotCheckReachable = false;
      // 如果搜索结果列表为空
      if (CallFail(SearchOrthogonalListFrontByLevel(mRoute.IsValid(), start_hashed_node_ptr, real_search_dist, 0,
                                                    search_result_by_level))) {
        search_result_by_level.clear();
        // 重新搜索结果列表，不检查可达性
        SearchOrthogonalListFrontByLevel(bDoNotCheckReachable, start_hashed_node_ptr, real_search_dist, 0,
                                         search_result_by_level);
        VehicleTrajInfo << "[trajectory][using_random_select]";
      } else {
        VehicleTrajInfo << "[trajectory][using_check_reachable]";
      }

      VehicleTrajInfo << "[trajectory]" << TX_VARS(reverse_base) << TX_VARS(traj_length) << TX_VARS(real_search_dist)
                      << TX_VARS(search_result_by_level.size());
      // 如果启用轨迹日志
      if (FLAGS_LogLevel_Vehicle_Traj) {
        Base::txInt idx = 0;
        // 遍历搜索结果列表
        for (const auto &ref : search_result_by_level) {
          LOG(INFO) << "[trajectory]" << TX_VARS(idx) << (ref->GetSelfHashedInfo());
          ++idx;
        }
      }

      // 初始化采样轨迹点列表
      std::vector<Base::TrajectorySamplingNode> vec_sampling_traj_pts;
      // vec_sampling_traj_pts.emplace_back(
      //     TrajectorySamplingNode(GetCurrentLaneInfo() /*current lane info*/, DistanceAlongCurve()));
      vec_sampling_traj_pts.emplace_back(
          Base::TrajectorySamplingNode(start_hashed_node_ptr->GetSelfHashedInfo().LaneInfo() /*current lane info*/,
                                       DistanceAlongCurve(), LaneOffset()));

      // 如果车辆正在变道
      if (CallSucc(IsInLaneChange())) {
        // 遍历搜索结果列表
        for (const auto &refTrajOrthogonalListPtr : search_result_by_level) {
          const auto &cur_level_HashNodeInfo = refTrajOrthogonalListPtr->GetSelfHashedInfo();
          // 将当前车道信息和距离车道起点的距离添加到采样轨迹点列表中
          vec_sampling_traj_pts.emplace_back(
              Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                           cur_level_HashNodeInfo.EndScope(), LaneOffset()));
        }
      } else {
        const auto &start_lane_info = start_hashed_node_ptr->GetSelfHashedInfo().LaneInfo();

        // 遍历搜索结果列表
        for (const auto &refTrajOrthogonalListPtr : search_result_by_level) {
          const auto &cur_level_HashNodeInfo = refTrajOrthogonalListPtr->GetSelfHashedInfo();

          // 如果当前车道信息与搜索结果的车道信息相同
          if (CallSucc(start_lane_info == cur_level_HashNodeInfo.LaneInfo())) {
            // 如果车辆在当前车道的范围内
            if (DistanceAlongCurve() < cur_level_HashNodeInfo.EndScope() &&
                cur_level_HashNodeInfo.StartScope() < DistanceAlongCurve()) {
              // 计算实际距离
              const Base::txFloat real_length = cur_level_HashNodeInfo.EndScope() - DistanceAlongCurve();

              // 将当前车道信息和距离车道起点的距离添加到采样轨迹点列表中
              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               DistanceAlongCurve() + 0.3 * real_length, LaneOffset()));

              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               DistanceAlongCurve() + 0.6 * real_length, LaneOffset()));

              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               cur_level_HashNodeInfo.EndScope(), LaneOffset()));
            } else {
              // 将当前车道信息和距离车道起点的距离添加到采样轨迹点列表中
              vec_sampling_traj_pts.emplace_back(
                  Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                               cur_level_HashNodeInfo.EndScope(), LaneOffset()));
            }
          } else {
            const Base::txFloat real_length = cur_level_HashNodeInfo.RealLength();
            // 将当前车道信息和距离车道起点的距离添加到采样轨迹点列表中
            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                             cur_level_HashNodeInfo.StartScope() + 0.35 * real_length, LaneOffset()));

            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                             cur_level_HashNodeInfo.StartScope() + 0.7 * real_length, LaneOffset()));

            vec_sampling_traj_pts.emplace_back(
                Base::TrajectorySamplingNode(cur_level_HashNodeInfo.LaneInfo() /*current lane info*/,
                                             cur_level_HashNodeInfo.EndScope(), LaneOffset()));
          }
        }
      }

      // 遍历采样轨迹点列表
      for (auto itr = std::begin(vec_sampling_traj_pts); itr != vec_sampling_traj_pts.end();) {
        // 如果当前采样轨迹点有效
        if (itr->ComputeSamplingPoint()) {
          ++itr;
        } else {
          // 从采样轨迹点列表中删除当前采样轨迹点
          itr = vec_sampling_traj_pts.erase(itr);
        }
      }
#if 1
      // if (IsOnLane() /*&& CallSucc(IsInLaneChange())*/)
      {
        for (auto itr = (std::begin(vec_sampling_traj_pts) + 1); itr != vec_sampling_traj_pts.end();) {
          const Base::txFloat dist_with_first_pt =
              Coord::txENU::EuclideanDistanceLocal_2D(vec_sampling_traj_pts[0].SamplingPt(), itr->SamplingPt());

          if (dist_with_first_pt < 5.0) {
            itr = vec_sampling_traj_pts.erase(itr);
          } else {
            break;
          }
        }
      }
#endif
      if (FLAGS_LogLevel_Vehicle_Traj) {
        for (const auto &ref : vec_sampling_traj_pts) {
          LOG(INFO) << "[trajectory]" << TX_VARS_NAME(SamplingNode, ref.Str());
        }
      }
      const Base::txFloat ego_alt = Altitude();
      // 如果采样轨迹点列表中的采样轨迹点数量大于1
      if (vec_sampling_traj_pts.size() > 1) {
        // 定义一个控制路径节点向量
        TrafficFlow::CentripetalCatMull::control_path_node_vec ref_control_path;
        // 添加第一个控制路径节点
        {
          /*first node real position*/
          Base::ISceneLoader::IRouteViewer::control_path_node path_node;
          path_node.waypoint = GeomCenter().WGS84();
          ref_control_path.emplace_back(path_node);
        }
        // 遍历采样轨迹点列表
        for (Base::txSize idx = 1; idx < vec_sampling_traj_pts.size(); ++idx) {
          Base::ISceneLoader::IRouteViewer::control_path_node path_node;
          path_node.waypoint = vec_sampling_traj_pts[idx].SamplingPt().ToWGS84().WGS84();
          ref_control_path.emplace_back(path_node);
        }
        // 创建一个基于圆滑Catmull-Rom曲线的车道几何对象
        auto laneGeomPtr = std::make_shared<TrafficFlow::CentripetalCatMull>(ref_control_path);
        if (NonNull_Pointer(laneGeomPtr)) {
          const Base::txFloat traj_real_length = laneGeomPtr->GetLength();
          Coord::txWGS84 end_wgs84;
          end_wgs84.FromENU(laneGeomPtr->GetEndPt());
#if 1
          refTraj.set_a(GetAcc());
          VehicleTrajInfo << TX_VARS(GetVelocity()) << TX_VARS(GetAcc());
          Base::txFloat cur_velocity = __HYPOT__(GetVelocity(), GetLateralVelocity());
          const Base::txFloat cur_acc = GetAcc();
          Base::txFloat cur_s = 0.0;
          // 遍历车辆轨迹的每个时间点
          for (Base::txFloat start_s = 0.0; start_s <= FLAGS_vehicle_trajectory_duration_s;
               start_s += timeMgr.RelativeTime()) {
            // 如果当前车辆轨迹点的s值小于车辆轨迹的实际长度且小于车辆轨迹的最大长度
            if (cur_s < traj_real_length && cur_s < (cur_velocity * FLAGS_vehicle_trajectory_duration_s)) {
              // 创建一个新的轨迹点并将其添加到轨迹中
              sim_msg::TrajectoryPoint *new_traj_point_ptr = refTraj.add_point();
              new_traj_point_ptr->set_z(ego_alt);
              new_traj_point_ptr->set_t(start_s + timeMgr.AbsTime());
              new_traj_point_ptr->set_v(cur_velocity);
              new_traj_point_ptr->set_a(GetAcc());
              new_traj_point_ptr->set_s(cur_s);

              /*1. update velocity*/
              cur_velocity += cur_acc * timeMgr.RelativeTime();
              if (cur_velocity >= mKinetics.m_velocity_max) {
                cur_velocity = mKinetics.m_velocity_max;
                // cur_acc = 0.0;
              }
              if (cur_velocity < 0.0) {
                cur_velocity = 0.0;
                // cur_acc = 0.0;
              }

              // 计算当前轨迹点的位置
              Coord::txWGS84 cur_pt_wgs84;
              cur_pt_wgs84.FromENU(laneGeomPtr->GetLocalPos(new_traj_point_ptr->s()));
              new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
              new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
              const Base::txFloat _headingRadian =
                  Utils::GetLaneAngleFromVectorOnENU(laneGeomPtr->GetLaneDir(new_traj_point_ptr->s())).GetRadian();
              new_traj_point_ptr->set_theta(_headingRadian);

              /*2. update s*/
              cur_s += cur_velocity * timeMgr.RelativeTime();
            } else {
              break;
            }
          }
          VehicleTrajInfo << TX_VARS(cur_s) << TX_VARS(cur_velocity) << TX_VARS(cur_acc)
                          << TX_VARS(timeMgr.RelativeTime());
#else
          for (Base::txSize idx = 0; idx < vec_ms_time_traj_point.size(); ++idx) {
            auto &ref_traj_sample = vec_ms_time_traj_point[idx];
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
        // 如果采样轨迹点列表中只有一个采样轨迹点
      } else if (1 == vec_sampling_traj_pts.size()) {
        // 创建一个新的轨迹点并将其添加到轨迹中
        sim_msg::TrajectoryPoint *new_traj_point_ptr = refTraj.add_point();
        new_traj_point_ptr->set_z(ego_alt);
        new_traj_point_ptr->set_t(timeMgr.AbsTime());
        new_traj_point_ptr->set_v(cur_velocity);
        new_traj_point_ptr->set_a(0.0);
        new_traj_point_ptr->set_s(0.0);

        // 获取采样轨迹点的位置
        Coord::txWGS84 cur_pt_wgs84 = vec_sampling_traj_pts.front().SamplingPt().ToWGS84();
        new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
        new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
        const Base::txFloat _headingRadian = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetRadian();
        new_traj_point_ptr->set_theta(_headingRadian);

        // 创建一个新的轨迹点并将其添加到轨迹中，将其设置为与第一个轨迹点相同
        sim_msg::TrajectoryPoint *new_clone_traj_point_ptr = refTraj.add_point();
        new_clone_traj_point_ptr->CopyFrom(*new_traj_point_ptr);
      } else {
#if 0
        const auto& refPos = GetPosition();
        for (Base::txSize idx = 0; idx < vec_ms_time_traj_point.size(); ++idx) {
          auto point_ptr = std::get<1>(vec_ms_time_traj_point[idx]);
          point_ptr->set_a(0.0);
          point_ptr->set_v(0.0);
          point_ptr->set_s(0.0);
          point_ptr->set_x(refPos.Lon());
          point_ptr->set_y(refPos.Lat());
          const Base::txFloat _headingRadian = GetHeadingWithAngle().GetRadian();
          point_ptr->set_theta(_headingRadian);
        }
#endif
      }
    }
    return true;
  } else {
    return false;
  }
}

TAD_UserDefined_VehicleElement::FrenetProjInfo TAD_UserDefined_VehicleElement::ComputeRoadProjectDistance(
    const Coord::txENU &target_element_geom_center) const TX_NOEXCEPT {
  FrenetProjInfo res;
  // 如果车辆的跟踪器和车辆的跟踪器的道路几何信息有效
  if (NonNull_Pointer(mLocation.tracker()) && NonNull_Pointer(mLocation.tracker()->getRoadGeomInfo())) {
    Base::txVec2 sourceST, targetST;
    mLocation.tracker()->getRoadGeomInfo()->xy2sl(StableGeomCenter().ENU2D(), sourceST.x(), sourceST.y());
    // 将目标元素的几何中心转换为Frenet坐标
    mLocation.tracker()->getRoadGeomInfo()->xy2sl(target_element_geom_center.ENU2D(), targetST.x(), targetST.y());

    Base::txFloat signDistance = targetST.x() - sourceST.x();
    res.mIsValid = true;
    // 计算目标元素与车辆的距离
    res.mProjDistance = std::fabs(signDistance);
    // 根据目标元素与车辆的Frenet坐标的x值之差判断目标元素与车辆的空间关系
    if (signDistance > 0.0) {
      res.mTarget2Source = _plus_(Base::Enums::Element_Spatial_Relationship::eFront);
    } else if (signDistance < 0.0) {
      res.mTarget2Source = _plus_(Base::Enums::Element_Spatial_Relationship::eRear);
    } else {
      res.mTarget2Source = _plus_(Base::Enums::Element_Spatial_Relationship::eOverlap);
    }
  }
  return res;
}

void TAD_UserDefined_VehicleElement::UpdateLateralDistanceAction(TimeParamManager const &timeMgr) TX_NOEXCEPT {
  // 如果车辆的纵向行为管理器有效且目标元素有效
  if (NonNull_Pointer(m_LateralActionMgrPtr) && CallSucc(m_LateralActionMgrPtr->IsValid()) &&
      NonNull_Pointer(m_LateralActionMgrPtr->target_element_ptr())) {
    txFloat current_dist = 0.0;
    if (CallSucc(m_LateralActionMgrPtr->freespace())) {
      /*bounding box distance*/
      const auto target_polygon = m_LateralActionMgrPtr->target_element_ptr()->Polygon();

      // 获取目标元素的几何中心
      const Base::txVec2 _adc_center = m_LateralActionMgrPtr->target_element_ptr()->StableGeomCenter().ENU2D();
      // 获取目标元素的逆变换矩阵
      const Base::txMat2 &tranMatInv = m_LateralActionMgrPtr->target_element_ptr()->TransMatInv();
      const Base::txVec2 local_adc_center(0.0, 0.0);
      // 初始化最大和最小x值
      Base::txFloat max_x = -FLT_MAX, min_x = FLT_MAX;
      // 遍历目标多边形的顶点
      for (const auto _obs_center : target_polygon) {
        // 将目标多边形的顶点从全局坐标系转换到局部坐标系
        const Base::txVec2 &local_obs_center = (_obs_center - _adc_center).transpose() * tranMatInv;
        if (local_obs_center.x() > max_x) {
          max_x = local_obs_center.x();
        }

        if (local_obs_center.x() < min_x) {
          min_x = local_obs_center.x();
        }
      }

      // 获取车辆的多边形
      const auto src_polygon = Polygon();
      // 初始化一个用于存储车辆多边形顶点在局部坐标系下的x值的向量
      std::vector<Base::txFloat> proj_x_axle;
      proj_x_axle.reserve(src_polygon.size());
      // 遍历车辆多边形的顶点
      for (const auto _obs_center : src_polygon) {
        // 将车辆多边形的顶点从全局坐标系转换到局部坐标系
        const Base::txVec2 &local_obs_center = (_obs_center - _adc_center).transpose() * tranMatInv;
        proj_x_axle.emplace_back(local_obs_center.x());
      }
      // 初始化一个用于存储车辆多边形顶点在局部坐标系下的x值与最大和最小x值之差的向量
      std::vector<Base::txFloat> proj_x_sign_dist;
      proj_x_sign_dist.reserve(proj_x_axle.size());

      // 初始化标志变量，用于判断车辆多边形顶点在局部坐标系下的x值是否为0、正数或负数
      Base::txBool hasZero = false, hasPositive = false, hadNegative = false;
      // 遍历proj_x_axle向量中的x值
      for (const Base::txFloat x_dist : proj_x_axle) {
        // 判断x值与最大和最小x值之差的关系，并更新标志变量
        if (x_dist > max_x) {
          hasPositive = true;
          proj_x_sign_dist.emplace_back(x_dist - max_x);
        } else if (x_dist >= min_x) {
          hasZero = true;
          proj_x_sign_dist.emplace_back(0.0);
        } else {
          hadNegative = true;
          proj_x_sign_dist.emplace_back(x_dist - min_x);
        }
      }
      current_dist = 0.0;
      // 如果车辆多边形顶点在局部坐标系下的x值中有0，则当前距离为0
      if (hasZero) {
        current_dist = 0.0;
      } else {
        // 判断x值与最大和最小x值之差的关系，并更新标志变量
        if (hadNegative && hasPositive) {
          /*overlap*/
          current_dist = 0.0;
        } else {
          /*only all positive, or, only all negative*/
          // 如果车辆多边形顶点在局部坐标系下的x值都是正数，且没有0和负数
          if (CallSucc(hasPositive) && CallFail(hasZero) && CallFail(hadNegative)) {
            current_dist = FLT_MAX;
            for (const auto dist : proj_x_sign_dist) {
              // 找到最小的正数x值
              if (dist >= 0.0 && dist < current_dist) {
                current_dist = dist;
              }
            }
          } else if (CallSucc(hadNegative) && CallFail(hasZero) &&
                     CallFail(hasPositive)) {  // 如果车辆多边形顶点在局部坐标系下的x值都是负数，且没有0和正数
            current_dist = FLT_MAX;
            for (const auto dist : proj_x_sign_dist) {
              if (dist <= 0.0 && std::fabs(dist) < current_dist) {
                current_dist = std::fabs(dist);
              }
            }
            // 将负数x值取反，得到正数距离
            current_dist *= -1.0;
          } else {
            LOG(WARNING) << "[Lateral_Action] Logic failure.";
          }
        }
      }
      LateralActionInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS(min_x) << TX_VARS(max_x) << TX_COND(hasPositive)
                        << TX_COND(hasZero) << TX_COND(hadNegative) << TX_VARS(current_dist);
    } else {
      /*geom center distance*/
      const Base::txVec2 _adc_center = m_LateralActionMgrPtr->target_element_ptr()->StableGeomCenter().ENU2D();
      const Base::txVec2 _obs_center = StableGeomCenter().ENU2D();
      const Base::txMat2 &tranMatInv = m_LateralActionMgrPtr->target_element_ptr()->TransMatInv();
      // 将负数x值取反，得到正数距离
      const Base::txVec2 local_adc_center(0.0, 0.0);
      // 将车辆的几何中心从全局坐标系转换到局部坐标系
      const Base::txVec2 &local_obs_center = (_obs_center - _adc_center).transpose() * tranMatInv;
      current_dist = local_obs_center.x();
      LateralActionInfo << TX_VARS(local_obs_center.x()) << TX_VARS(local_obs_center.y())
                        << TX_VARS_NAME(t00, tranMatInv.coeff(0, 0)) << TX_VARS_NAME(t01, tranMatInv.coeff(0, 1))
                        << TX_VARS_NAME(t10, tranMatInv.coeff(1, 0)) << TX_VARS_NAME(t11, tranMatInv.coeff(1, 1))
                        << TX_VARS(_obs_center.x()) << TX_VARS(_obs_center.y()) << TX_VARS(_adc_center.x())
                        << TX_VARS(_adc_center.y());
    }
    // 获取上一次的横向速度
    const txFloat last_lateral_velocity = m_LateralActionMgrPtr->last_v();
    // 计算目标距离与当前距离的差值
    const txFloat sign_target_dist = (m_LateralActionMgrPtr->distance() - current_dist);
    // 设置目标距离与当前距离的差值
    m_LateralActionMgrPtr->set_sign_target_dist(sign_target_dist);
    LateralActionInfo << TX_VARS(sign_target_dist) << TX_VARS(current_dist)
                      << TX_VARS(m_LateralActionMgrPtr->distance());
#if 0
        if (CallSucc(Math::isZero(std::fabs(sign_target_dist), 0.1)) &&
            CallFail(m_LateralActionMgrPtr->continuous())) {
            ClearLateralDistanceAction(timeMgr);
            return;
        }
#endif
    const txFloat timeStep_s = timeMgr.RelativeTime();
    const txFloat sign_target_velocity = sign_target_dist / timeStep_s;
    const txFloat sign_target_acc = (sign_target_velocity - last_lateral_velocity) / timeStep_s;
    const txFloat constraint_acc =
        Math::ScopeContraint(-1.0 * m_LateralActionMgrPtr->maxDec(), m_LateralActionMgrPtr->maxAcc(), sign_target_acc);
    const txFloat constraint_v =
        Math::ScopeContraint(-1.0 * m_LateralActionMgrPtr->maxSpeed(), m_LateralActionMgrPtr->maxSpeed(),
                             last_lateral_velocity + constraint_acc * timeStep_s);
    // 更新上一次的横向速度
    m_LateralActionMgrPtr->last_v() = constraint_v;

    LateralActionInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS(timeStep_s) << TX_VARS(sign_target_dist)
                      << TX_VARS(sign_target_velocity) << TX_VARS(last_lateral_velocity) << TX_VARS(sign_target_acc)
                      << TX_VARS(constraint_acc) << TX_VARS(constraint_v);
  } else {
    LateralActionInfo << TX_VARS(timeMgr.PassTime()) << " m_LateralActionMgrPtr invalid.";
  }
}

void TAD_UserDefined_VehicleElement::ClearLateralDistanceAction(TimeParamManager const &timeMgr) TX_NOEXCEPT {
  // 如果m_LateralActionMgrPtr不为空，说明车辆正在执行横向距离动作
  if (NonNull_Pointer(m_LateralActionMgrPtr)) {
    TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(
        m_LateralActionMgrPtr->evId_actionId(),
        TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
            m_LateralActionMgrPtr->self_elemType_elemId(),
            _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::completeState), timeMgr.PassTime()));
    LateralActionInfo << " lateral action hit the target. stop action." << (m_LateralActionMgrPtr->Str());
    // 清除横向距离动作
    m_LateralActionMgrPtr = nullptr;
    SetLaneKeep();
#if 1
    bNeedResetOffset = true;
    mKinetics.m_LateralDisplacement = 0.0;
#  if 1
    // 更新车道信息
    using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
    /*for lane change finish*/
    UpdateHashedLaneInfo(mLocation);
    const HashedLaneInfo &selfStableHashedLaneInfo = StableHashedLaneInfo();
    // 更新hash道路信息缓存
    UpdateHashedRoadInfoCache(selfStableHashedLaneInfo);
#  endif
#endif
  }
}

void TAD_UserDefined_VehicleElement::ClearMergeAction(TimeParamManager const &timeMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(m_MergeActionManagerPtr)) {
    // 更新场景事件状态
    TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(
        m_MergeActionManagerPtr->evId_actionId(),
        TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
            m_MergeActionManagerPtr->self_elemType_elemId(),
            _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::completeState), timeMgr.PassTime()));
    m_MergeActionManagerPtr = nullptr;
    LateralActionInfo << " merge action hit the target. stop action.";
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)
#undef LogInfo
#undef LogWarn
#undef LateralActionInfo
