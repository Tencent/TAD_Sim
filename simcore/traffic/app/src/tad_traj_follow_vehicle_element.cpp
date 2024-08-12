// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_traj_follow_vehicle_element.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "tx_sim_point.h"
#include "tx_spatial_query.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_TrajFollow)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)

std::tuple<Base::txFloat /*acc*/, Base::txFloat /*duration*/> TAD_TrajFollow_VehicleElement::ComputeTrajKinetics(
    const txFloat init_v, const txFloat final_v, const txFloat s) const TX_NOEXCEPT {
  // 计算持续时间，公式为：2 * s / (init_v + final_v)
  const txFloat duration = (2.0 * s) / (init_v + final_v);
  // 计算加速度，公式为：(final_v - init_v) / duration
  const txFloat acc = (final_v - init_v) / (duration);
  return std::make_tuple(acc, duration);
}

Base::txBool TAD_TrajFollow_VehicleElement::InitializeTrajMgr(const control_path_node_vec& ref_control_path,
                                                              const txFloat startTime,
                                                              const txFloat startAngle) TX_NOEXCEPT {
  if (ref_control_path.size() >= 2) {
    // 创建一个CentripetalCatMull对象，用于计算轨迹的几何信息
    auto tmp_centripetal_cat_mull_ptr = std::make_shared<CentripetalCatMull>(ref_control_path);
    // 获取轨迹的长度
    const txFloat raw_segment_length = tmp_centripetal_cat_mull_ptr->GetLength();
    // 创建一个controlPoint对象的向量，用于存储轨迹上的控制点
    std::vector<controlPoint> controlPointVec;
    // 遍历轨迹上的每个点，步长为FLAGS_MapLocationInterval
    for (txFloat s = 0.0; s < raw_segment_length; s += FLAGS_MapLocationInterval) {
      const Base::txVec3 enu3d = tmp_centripetal_cat_mull_ptr->GetLocalPos(s);
      controlPointVec.emplace_back(controlPoint{__East__(enu3d), __North__(enu3d)});
    }
    // 获取轨迹的最后一个点的3D坐标
    const Base::txVec3 enu3d = tmp_centripetal_cat_mull_ptr->GetLocalPos(raw_segment_length);
    // 将最后一个点的东北坐标添加到controlPointVec向量中
    controlPointVec.emplace_back(controlPoint{__East__(enu3d), __North__(enu3d)});
    m_trajMgr = nullptr;
    // 获取controlPointVec向量的大小
    const auto controlPointSize = controlPointVec.size();
    // 如果controlPointVec向量的大小大于等于HdMap::txLaneInfo::CatmullRom_ControlPointSize，创建一个HdMap::txLaneInfo对象并将其分配给m_trajMgr
    if (controlPointSize >= HdMap::txLaneInfo::CatmullRom_ControlPointSize) {
      m_trajMgr = std::make_shared<HdMap::txLaneInfo>(Base::txLaneUId(), controlPointVec);
    } else if (controlPointSize >= 2 TX_MARK("may be 3")) {
      m_trajMgr =
          std::make_shared<HdMap::txLaneShortInfo>(Base::txLaneUId(), controlPointVec.front(), controlPointVec.back());
    }

    // 如果m_trajMgr不为空，则执行以下操作
    if (NonNull_Pointer(m_trajMgr)) {
      // 初始化last_s为0.0，last_start_time为startTime
      txFloat last_s = 0.0;
      txFloat last_start_time = startTime;
      // 遍历ref_control_path中的每个点，从第二个点开始
      for (size_t idx = 1; idx < ref_control_path.size(); ++idx) {
        // 计算当前点的s值和t值
        txFloat cur_s = 0.0, cur_t = 0.0;
        Coord::txWGS84 curWGS84(ref_control_path[idx].waypoint);
        m_trajMgr->xy2sl(curWGS84.ToENU().ENU2D(), cur_s, cur_t);
        /*const Base::txVec3 locPos = m_trajMgr->GetLocalPos(cur_s);
        Coord::txWGS84 projPt; projPt.FromENU(locPos);
        LOG(WARNING) << TX_VARS(idx) << TX_VARS(curWGS84) << TX_VARS(projPt) << TX_VARS(cur_t);*/
        /*txFloat cur_s = laneGeomPtr->ParameterAtPoint(idx);*/
        // 创建一个SegmentKinectParam对象，用于存储轨迹动力学参数
        SegmentKinectParam param;
        // 设置param的mStartTime为last_start_time，mU为ref_control_path[idx -
        // 1].speed_m_s，mV为ref_control_path[idx].speed_m_s，mGear为ref_control_path[idx - 1].gear，mS为cur_s - last_s
        param.mStartTime = last_start_time;
        param.mU = ref_control_path[idx - 1].speed_m_s;
        param.mV = ref_control_path[idx].speed_m_s;
        param.mGear = ref_control_path[idx - 1].gear;
        param.mS = cur_s - last_s;

        // 输出当前点的s值、mS值、距离值、mStartTime值和mGear值
        LogInfo << TX_VARS(cur_s) << TX_VARS(param.mS)
                << TX_VARS_NAME(dist, Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(
                                          ref_control_path[idx - 1].waypoint, ref_control_path[idx].waypoint))
                << TX_VARS(param.mStartTime) << TX_VARS_NAME(Gear, __enum2lpsz__(ControlPathGear, param.mGear));
        std::tie(param.mA, param.mT) = ComputeTrajKinetics(param.mU, param.mV, param.mS);

        last_s = cur_s;
        last_start_time = param.EndTime();
        // 将param添加到m_KinectParamList向量中
        m_KinectParamList.emplace_back(param);
      }
    } else {
      return false;
    }
  } else if (1 == ref_control_path.size()) {  // 如果ref_control_path中只有一个点，执行以下操作
    // 将ref_control_path中的第一个点的WGS84坐标转换为ENU坐标，并将结果存储在startEnu中
    Coord::txENU startEnu;
    startEnu.FromWGS84(ref_control_path[0].waypoint);
    Base::txVec3 _laneDir;
    // 创建一个Info_Lane_t对象locInfo，用于存储车道信息
    Base::Info_Lane_t locInfo;
    Base::txFloat DistanceAlongCurve = 0.0;
    Unit::txDegree startAngleDeg;
    startAngleDeg.FromDegree(startAngle);
    _laneDir = Base::txVec3(startAngleDeg.Cos(), startAngleDeg.Sin(), 0.0);
#if 0
        if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().
            Get_S_Coord_By_Enu_Pt(startEnu.ToWGS84(), locInfo, DistanceAlongCurve)) {
            LogInfo << "Get_S_Coord_By_Enu_Pt : " << locInfo;
      _laneDir = HdMap::HadmapCacheConCurrent::GetDir(locInfo, DistanceAlongCurve);
        } else {
            LogWarn << "Get_S_Coord_By_Enu_Pt failure." << startEnu;
            return false;
        }
#endif
    m_trajMgr = std::make_shared<HdMap::txLaneSinglePositionInfo>(Base::txLaneUId(0, 0, 0),
                                                                  controlPoint{startEnu.X(), startEnu.Y()}, _laneDir);

    // 创建一个SegmentKinectParam对象，用于存储轨迹动力学参数
    SegmentKinectParam param;
    // 设置属性
    param.mStartTime = startTime;
    param.mU = 0.0;
    param.mV = 0.0;
    param.mGear = ref_control_path[0].gear;
    param.mS = 0.0;
    param.mA = 0.0;
    param.mT = 0.0;
    m_KinectParamList.emplace_back(param);
  } else {
    return false;
  }

  if (FLAGS_LogLevel_TrajFollow) {
    std::ostringstream oss;
    for (const auto& ref : m_KinectParamList) {
      oss << ref << ", ";
    }
    // 输出当前对象的ID、ref_control_path的大小、m_trajMgr的长度和oss中的信息
    LogInfo << TX_VARS(Id()) << TX_VARS_NAME(control_size, ref_control_path.size())
            << TX_VARS_NAME(traj_length, m_trajMgr->GetLength()) << TX_VARS_NAME(info, oss.str());
  }
  return true;
}

Base::txBool TAD_TrajFollow_VehicleElement::CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  // 如果当前对象尚未开始，执行以下操作
  if (CallFail(IsStart())) {
    // 如果当前时间大于或等于对象的开始时间，执行以下操作
    if (timeMgr.AbsTime() >= mLifeCycle.StartTime()) {
      // 设置对象的开始状态
      mLifeCycle.SetStart();
      SM::txUserDefinedState::SetStart();
      SM::txUserDefinedState::SetLaneKeep();
      OnStart(timeMgr);
    }
  }
  return IsStart();
}

Base::txBool TAD_TrajFollow_VehicleElement::InitializePedestrian(
    Base::ISceneLoader::IPedestriansViewerPtr _ped_view_ptr, ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 如果_ped_view_ptr不为空，执行以下操作
  if (NonNull_Pointer(_ped_view_ptr)) {
    Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr = _ped_view_ptr->vehicle_view_ptr();
    // 调用Initialize函数，传入_elemAttrViewPtr和_sceneLoader参数
    return Initialize(_elemAttrViewPtr, _sceneLoader);
  } else {
    LogWarn << "_ped_view_ptr is nullptr.";
    return false;
  }
}

Base::txBool TAD_TrajFollow_VehicleElement::Initialize(ISceneLoader::IViewerPtr _absAttrView,
                                                       ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 检查参数有效性
  if (NonNull_Pointer(_absAttrView) && NonNull_Pointer(_sceneLoader) && CallSucc(_absAttrView->IsInited())) {
    // 设置对象来源
    m_vehicle_come_from = Base::IVehicleElement::VehicleSource::scene_cfg;
    Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr =
        std::dynamic_pointer_cast<Base::ISceneLoader::IVehiclesViewer>(_absAttrView);
    // 如果元素属性视图存在且支持行为
    if (NonNull_Pointer(_elemAttrViewPtr) && CallSucc(IsSupportBehavior(_elemAttrViewPtr->behaviorEnum()))) {
      // 获取开始角度
      m_start_angle_degree = _elemAttrViewPtr->start_angle();
      // 根据路由id获取路由视图
      Base::ISceneLoader::IRouteViewerPtr _elemRouteViewPtr = _sceneLoader->GetRouteData(_elemAttrViewPtr->routeID());
      LogInfo << "Vehicle Attr :" << (_elemAttrViewPtr->Str());
      // 检查视图指针
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

          // 将所有的场景事件提取为map集合
          std::vector<ISceneEventViewerPtr> all_scene_event = _sceneLoader->GetSceneEventVec();
          std::map<txSysId, ISceneEventViewerPtr> map_id2ptr;
          for (auto ptr : all_scene_event) {
            if (NonNull_Pointer(ptr)) {
              map_id2ptr[ptr->id()] = ptr;
            }
          }

          // 获取当前的元素的事件，从map集合中提取对应的事件指针
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
        }
        m_vehicle_type = __lpsz2enum__(
            VEHICLE_TYPE,
            (_elemAttrViewPtr->vehicleType())
                .c_str()); /*Utils::String2ElementType((_elemAttrViewPtr->vehicleType()), +VEHICLE_TYPE::Sedan);*/
#if USE_CustomModelImport
        m_vehicle_type_str = _elemAttrViewPtr->vehicleType().c_str();
        m_vehicle_type_id = Base::CatalogCache::VEHICLE_TYPE(m_vehicle_type_str);
#endif /*#USE_CustomModelImport*/
        const auto refRoutePtr = (_elemRouteViewPtr);
        const auto refVehiclePtr = (_elemAttrViewPtr);
        // 初始化元素的身份信息
        mIdentity.Id() = refVehiclePtr->id();
        mIdentity.SysId() = CreateSysId(mIdentity.Id());
        SM::txUserDefinedState::Initialize(mIdentity.Id());
        mPRandom.Initialize(mIdentity.Id(), _sceneLoader->GetRandomSeed());
        // 初始化元素的开时间
        mLifeCycle.StartTime() = refVehiclePtr->start_t();

#if 1
        // catalog初始化失败
#  if USE_CustomModelImport
        if (CallFail(InitVehicleCatalog(m_vehicle_type_str))) {
#  else  /*#USE_CustomModelImport*/
        if (CallFail(InitCatalog(m_vehicle_type))) {
#  endif /*#USE_CustomModelImport*/
          // 使用视图提供的几何信息
          mGeometryData.Length() = refVehiclePtr->length();
          mGeometryData.Width() = refVehiclePtr->width();
          mGeometryData.Height() = refVehiclePtr->height();
          LogWarn << "[Catalog_query][failure] " << TX_VARS(Id())
                  << TX_VARS_NAME(VehicleType, (_elemAttrViewPtr->vehicleType())) << TX_VARS(GetLength())
                  << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
        } else {
          // 否则使用catalog
          mGeometryData.Length() = Catalog_Length();
          mGeometryData.Width() = Catalog_Width();
          mGeometryData.Height() = Catalog_Height();
          LOG_IF(INFO, FLAGS_LogLevel_Catalog) << "[Catalog_query][success] " << TX_VARS(Id())
                                               << TX_VARS_NAME(VehicleType, (_elemAttrViewPtr->vehicleType()))
                                               << TX_VARS(GetLength()) << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
        }
#endif
        // 设置元素的几何类型
        mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;

        // 初始化轨迹管理器
        if (InitializeTrajMgr(_elemRouteViewPtr->control_path(), mLifeCycle.StartTime(), m_start_angle_degree)) {
          m_aggress = refVehiclePtr->aggress();
          TX_MARK("For lanekeep");
          // 使用m_aggress作为参数，重置mLaneKeepMgr的计数器，并设置计数器的最大值为5.0
          mLaneKeepMgr.ResetCounter(5.0, m_aggress);

          // 调用Initialize_Kinetics函数，传入_elemAttrViewPtr参数，并在日志中添加标记"visualizer"
          if (CallFail(Initialize_Kinetics(_elemAttrViewPtr TX_MARK("visualizer")))) {
            LogWarn << "initialize kinetics error.";
            return false;
          }

          // 获取refVehiclePtr的laneID属性，并将其分配给startLaneId
          const Base::txLaneID startLaneId = refVehiclePtr->laneID();
          // 使用startLaneId计算目标车道索引，并将其分配给mLocation的NextLaneIndex属性
          mLocation.NextLaneIndex() = TrafficFlow::RoutePathManager::ComputeGoalLaneIndex(startLaneId);
          mLocation.LaneOffset() = refVehiclePtr->l_offset();

          const txBool bLocateOnLaneLink = (0 <= startLaneId);
          LOG(INFO) << "[vehicle_init_location] " << TX_VARS_NAME(VehicleId, mIdentity.Id())
                    << (bLocateOnLaneLink ? " [LaneLink]" : " [Lane]");
          // 如果在车道上
          if (CallFail(bLocateOnLaneLink)) {
            TX_MARK("on lane");
            txFloat distanceCurveOnInit_Lane = 0.0;
            txFloat distancePedal_lane = 0.0;
            // 根据位置和角度查询车道信息
            hadmap::txLanePtr initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetLaneForInit(
                __Lon__(refRoutePtr->startGPS()), __Lat__(refRoutePtr->startGPS()), distanceCurveOnInit_Lane,
                distancePedal_lane);

            if (NonNull_Pointer(initLane_roadId_sectionId)) {
              LogInfo << TX_VARS_NAME(initLaneUid, Utils::ToString(initLane_roadId_sectionId->getTxLaneId()));

              // 获取车道id
              Base::txLaneUId _laneUid = initLane_roadId_sectionId->getTxLaneId();
              _laneUid.laneId = startLaneId;
              // 重新获取车道指针
              hadmap::txLanePtr initLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(_laneUid);
              if (Null_Pointer(initLane)) {
                LogWarn << ", GetLane Failure." << TX_VARS_NAME(_laneUid, Utils::ToString(_laneUid));
                return false;
              }

              initLane_roadId_sectionId = initLane;
              if (HdMap::HadmapCacheConCurrent::Get_LAL_Lane_By_S(
                      initLane->getTxLaneId(), GenerateST(refVehiclePtr->start_s(), distanceCurveOnInit_Lane),
                      mLocation.PosOnLaneCenterLinePos())) {
                // 设置location信息
                mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
                mLocation.DistanceAlongCurve() = GenerateST(refVehiclePtr->start_s(), distanceCurveOnInit_Lane);
#if 1
                TX_MARK("JIRA-307");
                mLocation.vLaneDir() =
                    HdMap::HadmapCacheConCurrent::GetLaneDir(initLane, mLocation.DistanceAlongCurve());
#else
                mLocation.vLaneDir() = m_trajMgr->GetLaneDir(0.0);
                Unit::txDegree retDegree;
                retDegree.FromRadian(Math::Atan2(mLocation.vLaneDir().y(), mLocation.vLaneDir().x()));
                LOG(WARNING) << retDegree;
#endif

                /*mLocation.rot_for_display() = Utils::DisplayerGetLaneAngleFromVector(mLocation.vLaneDir());*/
                RawVehicleCoord() =
                    ComputeLaneOffset(mLocation.PosOnLaneCenterLinePos(), mLocation.vLaneDir(), mLocation.LaneOffset());
                // 位置同步
                SyncPosition(0.0);
                // 初始化跟踪器并重定位
                mLocation.InitTracker(mIdentity.Id());
                RelocateTracker(initLane, 0.0);
                mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
                // 更新查询空间
                FillingSpatialQuery();
                // 保存稳定状态
                SaveStableState();
                LogInfo << TX_VARS(Id()) << TX_VARS(GetCurrentLaneInfo()) << TX_VARS(VehicleCoordStr());

                return true;
              } else {
                LogWarn << ", Get_LAL_Lane_By_S Failure.";
                return false;
              }
            } else {
              LogWarn << ", Get Init Lane Error.";
              return false;
            }
          } else if (CallSucc(bLocateOnLaneLink)) {  // 在link上
            TX_MARK("on link");
            Base::txFloat distancePedal_lanelink = 0.0;
            Base::txFloat distanceCurveOnInit_LaneLink = 0.0;
            // 根据经纬度查询车道link
            hadmap::txLaneLinkPtr initLaneLinkptr = HdMap::HadmapCacheConCurrent::GetLaneLinkForInit(
                refRoutePtr->startLon(), refRoutePtr->startLat(), distanceCurveOnInit_LaneLink, distancePedal_lanelink);
            distanceCurveOnInit_LaneLink = refVehiclePtr->start_s();
            if (NonNull_Pointer(initLaneLinkptr)) {
              // 根据s 车道信息获取车道连接信息
              if (HdMap::HadmapCacheConCurrent::Get_LAL_LaneLink_By_S(
                      Base::Info_Lane_t(initLaneLinkptr->getId(), initLaneLinkptr->fromTxLaneId(),
                                        initLaneLinkptr->toTxLaneId()),
                      distanceCurveOnInit_LaneLink, mLocation.PosOnLaneCenterLinePos())) {
                // 设置location信息
                mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
                mLocation.DistanceAlongCurve() = distanceCurveOnInit_LaneLink;
                mLocation.vLaneDir() =
                    HdMap::HadmapCacheConCurrent::GetLaneLinkDir(initLaneLinkptr, mLocation.DistanceAlongCurve());
                /*mLocation.rot_for_display() = Utils::DisplayerGetLaneAngleFromVector(mLocation.vLaneDir());*/
                RawVehicleCoord() =
                    ComputeLaneOffset(mLocation.PosOnLaneCenterLinePos(), mLocation.vLaneDir(), mLocation.LaneOffset());
                // 位置同步
                SyncPosition(0.0);
                // 初始化跟踪器并重定位
                mLocation.InitTracker(mIdentity.Id());
                RelocateTracker(initLaneLinkptr, 0.0);
                mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
                // 更新查询空间
                FillingSpatialQuery();
                SaveStableState();
                LogInfo << Str();
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
          LogWarn << "InitializeTrajMgr failure." << TX_VARS(Id());
          return false;
        }
      } else {
        LogWarn << "Can not find Route Info.";
        return false;
      }
    } else {
      LogWarn << "Param Cast Error." << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr))
              << TX_VARS(_elemAttrViewPtr->behaviorEnum())
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

Base::txBool TAD_TrajFollow_VehicleElement::Initialize_Kinetics(Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr)
    TX_NOEXCEPT {
  // 如果_elemAttrViewPtr不为空，执行以下操作
  if (NonNull_Pointer(_elemAttrViewPtr)) {
    // 初始化kinetics属性值
    mKinetics.m_velocity = (_elemAttrViewPtr->start_v());
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

void TAD_TrajFollow_VehicleElement::OnStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  mKinetics.m_velocity = m_KinectParamList.front().mU;
  mKinetics.m_acceleration = m_KinectParamList.front().mA;
  // 判断m_KinectParamList的第一个元素的mGear属性是否等于ControlPathGear::reverse
  if (_plus_(ControlPathGear::reverse) == m_KinectParamList.front().mGear) {
    mKinetics.m_gear_angle.FromDegree(180.0);
  } else {
    mKinetics.m_gear_angle.FromDegree(0.0);
  }

  // 将0.0分配给m_real_displacement属性
  m_real_displacement = 0.0;
  m_KinectParamList.erase(m_KinectParamList.begin());
}

Base::txBool TAD_TrajFollow_VehicleElement::CheckDeadLine() TX_NOEXCEPT {
  // 判断m_real_displacement与m_trajMgr->GetLength()之间的差值的绝对值是否小于DeadLineRadius()
  if (std::fabs(m_real_displacement - (m_trajMgr->GetLength())) < DeadLineRadius()) {
    // 如果满足条件，调用StopVehicle()函数停止车辆
    StopVehicle();
  }
  return IsAlive();
}

Base::txBool TAD_TrajFollow_VehicleElement::Update_Kinetics(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  // 判断车辆是否存活且未停止
  if (IsAlive() && !IsStop()) {
    // 判断m_KinectParamList是否非空，且当前时间是否在m_KinectParamList的第一个元素的起始时间和结束时间之间
    if (_NonEmpty_(m_KinectParamList) && (m_KinectParamList.front().mStartTime <= timeMgr.PassTime()) &&
        (timeMgr.PassTime() <= m_KinectParamList.front().EndTime())) {
      // 将m_KinectParamList的第一个元素的mU属性分配给mKinetics的m_velocity属性
      mKinetics.m_velocity = m_KinectParamList.front().mU;
      // 将m_KinectParamList的第一个元素的mA属性分配给mKinetics的m_acceleration属性
      mKinetics.m_acceleration = m_KinectParamList.front().mA;
      // 判断m_KinectParamList的第一个元素的mGear属性是否等于ControlPathGear::reverse
      if (_plus_(ControlPathGear::reverse) == m_KinectParamList.front().mGear) {
        // 如果等于ControlPathGear::reverse，将180.0度转换为弧度并分配给mKinetics的m_gear_angle属性
        mKinetics.m_gear_angle.FromDegree(180.0);
      } else {
        mKinetics.m_gear_angle.FromDegree(0.0);
      }
      // 从m_KinectParamList中删除第一个元素
      m_KinectParamList.erase(m_KinectParamList.begin());
    }
    // 更新mKinetics的m_velocity属性，使其等于当前速度加上加速度乘以相对时间
    mKinetics.m_velocity = mKinetics.m_velocity + mKinetics.m_acceleration * timeMgr.RelativeTime();
  } else {
    mKinetics.m_velocity = 0.0;
    mKinetics.m_acceleration = 0.0;
  }
  return true;
}

Base::txFloat TAD_TrajFollow_VehicleElement::Compute_Displacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  mKinetics.m_displacement = GetVelocity() * timeMgr.RelativeTime();
  return GetDisplacement();
}

Base::txBool TAD_TrajFollow_VehicleElement::Update(TimeParamManager const& timeMgr) TX_NOEXCEPT {
  // 将相对时间转换为毫秒并分配给m_sim_step_ms
  m_sim_step_ms = Utils::SecondToMillisecond(timeMgr.RelativeTime());
  // 判断车辆是否存活且未停止
  if (CallSucc(IsAlive()) && CallFail(IsStop())) {
    /*Step 1 Update Kinetics*/
    // 如果车辆速度小于等于0.0
    if (GetVelocity() <= 0.0) {
      // 将mKinetics的m_velocity属性设置为0.0
      mKinetics.m_velocity = 0.0;
      // 计算车辆位移
      Compute_Displacement(timeMgr);
      // 计算车辆在车道上的位置
      RawVehicleCoord() =
          ComputeLaneOffset(mLocation.PosWithLateralWithoutOffset(), mLocation.vLaneDir(), mLocation.LaneOffset());
      // 同步车辆位置
      SyncPosition(timeMgr.PassTime());
      mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
    } else {
      // 计算车辆位移
      Compute_Displacement(timeMgr);
      m_real_displacement += GetDisplacement();
      // 获取车辆在轨迹上的位置
      const Base::txVec3 enuPos = m_trajMgr->GetLocalPos(m_real_displacement);
      mLocation.PosOnLaneCenterLinePos().FromENU(enuPos);
      // 获取车辆在轨迹上的车道方向
      mLocation.vLaneDir() = m_trajMgr->GetLaneDir(m_real_displacement);
      // 判断车辆是否在变道过程中
      if (IsInLaneChange()) {
        // 计算车辆的侧向速度
        mKinetics.m_LateralVelocity = MoveTo_Sideway(timeMgr);
      }

      // 变道后的操作
      LaneChangePostOp(timeMgr);
      // 计算车辆的多边形
      mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
    }
    Base::txFloat s, l;
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
        GeomCenter(), mLocation.LaneLocInfo(), s, l);

    // veh在lanlink上
    if (mLocation.LaneLocInfo().isOnLaneLink) {
      hadmap::txLaneLinkPtr linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(
          mLocation.LaneLocInfo().onLinkFromLaneUid, mLocation.LaneLocInfo().onLinkToLaneUid);
      // 重定位车辆跟踪器
      RelocateTracker(linkPtr, timeMgr.TimeStamp());
    } else {  // veh在lane上
      hadmap::txLanePtr lanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
      RelocateTracker(lanePtr, timeMgr.TimeStamp());
    }
    return true;
  } else {
    return (IsStop());
  }
}

Base::txBool TAD_TrajFollow_VehicleElement::Release() TX_NOEXCEPT {
  mLifeCycle.SetEnd();
  return true;
}

Base::txBool TAD_TrajFollow_VehicleElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                                           sim_msg::Traffic& trafficData) TX_NOEXCEPT {
  return ParentClass::FillingElement(timeMgr, trafficData);
}

// 计算车辆的多边形
Base::txFloat TAD_TrajFollow_VehicleElement::ComputeHeading(const txFloat /*_passTime*/) TX_NOEXCEPT {
  const Base::txFloat duration_ms = Utils::SecondToMillisecond(FLAGS_TrajStartAngleDuration);
  // 判断m_start_angle_action_duration_ms是否小于duration_ms且m_start_angle_degree大于等于0.0
  if (m_start_angle_action_duration_ms < duration_ms && m_start_angle_degree >= 0.0) {
    // 如果车辆速度大于0.0
    if (StableVelocity() > 0.0) {
      m_start_angle_action_duration_ms += m_sim_step_ms;
    }

    // 计算车辆的朝向角
    const Base::txFloat _headingRadian = (GetHeadingWithAngle() + mKinetics.m_gear_angle).GetRadian();
    // 计算0度朝向角
    const Base::txFloat _zeroRadian = _headingRadian;
    // 创建一个圆形弧度对象
    Unit::CircleRadian circRad(_zeroRadian - Unit::CircleRadian::PI(), _zeroRadian + Unit::CircleRadian::PI(),
                               _zeroRadian);
    // const Base::txFloat _headingRadian =
    // Utils::DisplayerGetLaneAngleFromVector(m_vTargetDir).GetRadian(TX_MARK("SIM-4072"));
    const Base::txFloat _start_angle_Radian = Math::Degrees2Radians(m_start_angle_degree);
    // 将_start_angle_Radian包装到圆形弧度对象中
    const Base::txFloat wrap_start_angle_Radian = circRad.Wrap(_start_angle_Radian);
    const txFloat res = Math::linear_interp(0.0, duration_ms, wrap_start_angle_Radian, _headingRadian,
                                            m_start_angle_action_duration_ms);
    LogInfo << TX_VARS(Id()) << TX_VARS(m_start_angle_action_duration_ms) << TX_VARS(m_sim_step_ms)
            << TX_VARS(_headingRadian) << TX_VARS(_start_angle_Radian) << TX_VARS(wrap_start_angle_Radian)
            << TX_VARS(res);
    return res;
  } else {
    const Base::txFloat _headingRadian = (GetHeadingWithAngle() + mKinetics.m_gear_angle).GetRadian();
    return _headingRadian;
  }
}

sim_msg::Car* TAD_TrajFollow_VehicleElement::FillingElement(txFloat const timeStamp,
                                                            sim_msg::Car* pSimVehicle) TX_NOEXCEPT {
  // 调用父类的FillingElement函数
  pSimVehicle = ParentClass::FillingElement(timeStamp, pSimVehicle);
#if USE_VehicleKinectInfo
  // 判断pSimVehicle是否非空
  if (NonNull_Pointer(pSimVehicle)) {
    // 设置各属性值
    pSimVehicle->set_show_abs_acc(GetAcc());
    pSimVehicle->set_show_abs_velocity(GetVelocity());
    pSimVehicle->set_show_relative_velocity(show_relative_velocity_vertical());
    pSimVehicle->set_show_relative_velocity_horizontal(show_relative_velocity_horizontal());
    pSimVehicle->set_show_relative_dist_vertical(show_relative_dist_vertical());
    pSimVehicle->set_show_relative_dist_horizontal(show_relative_dist_horizontal());

    // const Base::txFloat _headingRadian = (GetHeadingWithAngle() + mKinetics.m_gear_angle).GetRadian();
    const Base::txFloat _headingRadian = ComputeHeading(timeStamp);
    pSimVehicle->set_heading(_headingRadian);
    pSimVehicle->set_theta(_headingRadian);

    LOG_IF(INFO, FLAGS_LogLevel_Kinetics)
        << TX_VARS(Id()) << TX_VARS(show_abs_acc()) << TX_VARS(show_abs_velocity())
        << TX_VARS(show_relative_velocity_horizontal()) << TX_VARS(show_relative_velocity_vertical())
        << TX_VARS(show_relative_dist_horizontal()) << TX_VARS(show_relative_dist_vertical());
  }
#endif /*USE_VehicleKinectInfo*/
  return pSimVehicle;
}

Base::txBool TAD_TrajFollow_VehicleElement::Pre_Update(
    const TimeParamManager& timeMgr, std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT {
  // 判断车辆是否有跟随目标
  if (CallSucc(HasFollow())) {
    // 获取跟随目标的元素ID
    const Base::txInt followElemId = GetFollow();
    // 在map_elemId2Kinetics中查找跟随目标的运动信息
    if (map_elemId2Kinetics.end() != map_elemId2Kinetics.find(followElemId)) {
      // 获取跟随目标的运动信息
      const auto& kineticsInfo = map_elemId2Kinetics.at(followElemId);
      // 设置车辆的运动信息
      SetKinetics(kineticsInfo);
    } else {
      LogWarn << TX_VARS_NAME(Id, Id()) << " can not find follow info. " << TX_VARS_NAME(followElemId, followElemId);
    }
  } else {
    // 更新车辆的运动信息
    Update_Kinetics(timeMgr);
    // 判断车辆是否有加速度结束条件
    if (CallSucc(HasAccEndCondition())) {
      const AccEndConditionManager accEndCond = GetAccEndCondition();
      // 判断加速度结束条件的类型
      if ((_plus_(Base::ISceneLoader::acc_invalid_type::eTime)) == accEndCond.m_endCondition.m_type) {
        const Base::txFloat remainTime = accEndCond.m_remainingTime - timeMgr.RelativeTime();
        if (remainTime > 0.0) {
          // 设置加速度结束条件的剩余时间
          SetAccEndConditionRemainTime(remainTime);
        } else {
          mKinetics.m_acceleration = 0.0;
          ClearAccEndCondition(timeMgr);
        }
      } else if ((_plus_(Base::ISceneLoader::acc_invalid_type::eVelocity)) == accEndCond.m_endCondition.m_type) {
        // 判断车辆的速度是否达到目标速度
        if (accEndCond.m_happenVelocity <= accEndCond.m_targetVelocity &&
            GetVelocity() >= accEndCond.m_targetVelocity) {
          // 将车辆的加速度设置为0.0
          mKinetics.m_acceleration = 0.0;
          mKinetics.m_velocity = accEndCond.m_targetVelocity;
          ClearAccEndCondition(timeMgr);
        } else if (accEndCond.m_happenVelocity >= accEndCond.m_targetVelocity &&
                   GetVelocity() <= accEndCond.m_targetVelocity) {
          // 将车辆的加速度设置为0.0
          mKinetics.m_acceleration = 0.0;
          mKinetics.m_velocity = accEndCond.m_targetVelocity;
          ClearAccEndCondition(timeMgr);
        }
      }
    }
  }
  return true;
}

Base::txString TAD_TrajFollow_VehicleElement::Str() const TX_NOEXCEPT { return "TAD_TrajFollow_VehicleElement"; }

Geometry::Topology::txEdgeVec TAD_TrajFollow_VehicleElement::GetTopologyRelationShip() const TX_NOEXCEPT {
  Geometry::Topology::txEdgeVec edges;
  edges.push_back(Geometry::Topology::txEdge(SysRootId(), SysId()));
  return edges;
}

void TAD_TrajFollow_VehicleElement::SetKinetics(const KineticsInfo_t& info) TX_NOEXCEPT {
  mKinetics.m_velocity = info.m_compute_velocity;
  mKinetics.m_show_velocity = info.m_show_velocity;
  mKinetics.m_acceleration = info.m_acc;
}

TAD_TrajFollow_VehicleElement::KineticsUtilInfo_t TAD_TrajFollow_VehicleElement::GetKineticsUtilInfo() const
    TX_NOEXCEPT {
  return KineticsUtilInfo_t();
}

Base::txBool TAD_TrajFollow_VehicleElement::ComputeKinetics(Base::TimeParamManager const& timeMgr,
                                                            const KineticsUtilInfo_t& _objInfo) TX_NOEXCEPT {
  // 设置车辆的绝对速度和加速度
  m_show_abs_velocity = GetVelocity();
  m_show_abs_acc = GetAcc();
  // 判断相对时间大于0.0且车辆信息有效
  if (timeMgr.RelativeTime() > 0.0 &&
        CallSucc(_objInfo.m_isValid) /*&&
        CallFail(_objInfo._vecPolygon.empty()) &&
        CallFail(GetPolygon().empty())*/) {
    // 获取跟随目标的中心坐标
    const Base::txVec2 _adc_center = _objInfo._ElementGeomCenter.ENU2D();
    /*const Base::txVec2 _last_adc_center = _objInfo._LastElementPos.GetLocalPos_2D();*/
    // 获取车辆的中心坐标
    const Base::txVec2 _obs_center = GeomCenter().ToENU().ENU2D();
    /*const Base::txVec2 _last_obs_center = pImpl->m_elemRoadInfo.lastPos.GetLocalPos_2D();*/
    // 获取跟随目标的变换矩阵的逆矩阵
    const Base::txMat2& tranMatInv = _objInfo._TransMatInv;

    // 计算跟随目标和车辆的局部坐标
    const Base::txVec2 local_adc_center(0.0, 0.0);
    /*const Base::txVec2 local_last_adc_center = (_last_adc_center - _adc_center).transpose() * tranMatInv;*/
    const Base::txVec2& local_obs_center = (_obs_center - _adc_center).transpose() * tranMatInv;
    /*const Base::txVec2 local_last_obs_center = (_last_obs_center - _adc_center).transpose() * tranMatInv;*/

    // 计算车辆相对于跟随目标的水平和垂直距离
    m_show_relative_dist_horizontal = local_obs_center.x() /** -1.0*/;
    m_show_relative_dist_vertical = local_obs_center.y();
    m_show_abs_dist_elevation = Altitude();
    m_show_relative_dist_elevation = show_abs_dist_elevation() - _objInfo._ElementGeomCenter.GetWGS84().Alt();
    // const Base::txVec2 local_adc_velocity = (local_adc_center - local_last_adc_center) / timeMgr.relativeTime;
    Base::txVec2 local_relative_velocity;
    // 如果车辆的上一帧局部坐标不为0
    if (Math::isNotZero(m_local_last_obs_center.x()) && Math::isNotZero(m_local_last_obs_center.y())) {
      local_relative_velocity = (local_obs_center - m_local_last_obs_center) / timeMgr.RelativeTime();
    } else {
      local_relative_velocity.setZero();
    }

    // const Base::txVec2 local_relative_velocity = (local_obs_velocity - local_adc_velocity);

    // 设置车辆相对于跟随目标的水平和垂直速度
    m_show_relative_velocity_horizontal = local_relative_velocity.x() * -1.0;
    m_show_relative_velocity_vertical = local_relative_velocity.y();

    m_local_last_obs_center = local_obs_center;
  } else {
    // 如果相对时间小于等于0.0或车辆信息无效，将车辆相对于跟随目标的水平和垂直距离、速度设置为0.0
    m_show_relative_dist_horizontal = 0.0;
    m_show_relative_dist_vertical = 0.0;
    m_show_relative_velocity_horizontal = 0.0;
    m_show_relative_velocity_vertical = 0.0;
  }
  return true;
}

Base::txBool TAD_TrajFollow_VehicleElement::HandlerEvent(Base::TimeParamManager const& timeMgr,
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
    IActionViewerPtr actionPtr = _eventPtr->action();
    IEndConditionViewerPtr endCondPtr = _eventPtr->endCondition();
    // 判断行为视图和结束条件视图有效
    if (NonNull_Pointer(actionPtr) && NonNull_Pointer(endCondPtr)) {
      const EventActionInfoVec& actionVec = actionPtr->actionList();
      const EndConditionVec& endCondVec = endCondPtr->endConditionList();
      // 判断动作集合和结束条件集合大小相同
      if (actionVec.size() == endCondVec.size()) {
        // 遍历动作
        for (size_t idx = 0; idx < actionVec.size(); ++idx) {
          const EventActionInfoVec::value_type& refAct = actionVec[idx];
          const EndConditionVec::value_type& refEndCond = endCondVec[idx];
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
          // 如果动作类型是acc
          if (_plus_(SceneEventActionType::acc) == refAct.action_type()) {
            mKinetics.m_acceleration = refAct.value();

            // 如果结束条件类型是time
            if ((_plus_(EndConditionKeyType::time)) == std::get<0>(refEndCond)) {
              // 设置acc结束条件管理器
              AccEndConditionManager accMgr;
              accMgr.m_endCondition.m_IsValid = true;
              accMgr.m_endCondition.m_type = _plus_(acc_invalid_type::eTime);
              accMgr.m_endCondition.m_endCondition = std::get<1>(refEndCond);
              accMgr.m_remainingTime = std::get<1>(refEndCond);
              accMgr.m_happenVelocity = GetVelocity();
              accMgr.m_targetVelocity = 0.0;
              m_optional_AccEndCondition = accMgr;
#if __TX_Mark__("element_state")
              TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
              if (NonNull_Pointer(m_AccActionManagerPtr)) {
                ClearAccAction(timeMgr); /*clear last acc ation*/
              }
              m_AccActionManagerPtr = std::make_shared<ActionManager>();
              m_AccActionManagerPtr->Initialize(evId_actionId, elemType_elemId);
#endif /*__TX_Mark__("element_state")*/
            } else if ((_plus_(EndConditionKeyType::velocity)) == std::get<0>(refEndCond)) {  // 如果结束条件是速度
              AccEndConditionManager accMgr;
              accMgr.m_endCondition.m_IsValid = true;
              accMgr.m_endCondition.m_type = _plus_(acc_invalid_type::eVelocity);
              accMgr.m_endCondition.m_endCondition = std::get<1>(refEndCond);

              accMgr.m_remainingTime = 0.0;
              accMgr.m_happenVelocity = GetVelocity();
              accMgr.m_targetVelocity = std::get<1>(refEndCond);
              // 设置acc结束条件管理器
              m_optional_AccEndCondition = accMgr;
#if __TX_Mark__("element_state")
              TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
              if (NonNull_Pointer(m_AccActionManagerPtr)) {
                ClearAccAction(timeMgr); /*clear last acc ation*/
              }
              // 创建并且初始化加速动作管理器
              m_AccActionManagerPtr = std::make_shared<ActionManager>();
              m_AccActionManagerPtr->Initialize(evId_actionId, elemType_elemId);
#endif /*__TX_Mark__("element_state")*/
            }
            LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
                << "[event_handler_vehicle_acc_event], id = " << Id() << ", Set Acc_event = " << (GetAcc());
          } else if (_plus_(SceneEventActionType::velocity) == refAct.action_type()) {  // 如果时间动作类型是速度
            mKinetics.m_velocity = refAct.value();
#if __TX_Mark__("element_state")
            // 更新场景事件状态
            TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(
                evId_actionId,
                TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
                    elemType_elemId, _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::completeState),
                    timeMgr.PassTime()));
#endif /*__TX_Mark__("element_state")*/
            LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
                << "[event_handler_vehicle_velocity], id = " << Id() << ", Set Velocity = " << (GetVelocity());
          } else if (_plus_(SceneEventActionType::merge) == refAct.action_type()) {  // 如果动作类型时merge
            Base::txBool const bCheckGoalLane = true;
            if ((_plus_(VehicleMoveLaneState::eLeft) == refAct.merge_type()) ||
                (_plus_(VehicleMoveLaneState::eRight) == refAct.merge_type())) {
              // 如果处于变道状态
              if (ChangeInLane(refAct.merge_type(), refAct.value(), FLAGS_default_lane_width)) {
#if __TX_Mark__("element_state")
                // 更新场景事件状态
                TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
                // 如果变道动作管理器指针非空
                if (NonNull_Pointer(m_MergeActionManagerPtr)) {
                  ClearMergeAction(timeMgr); /*clear last acc ation*/
                  LOG(WARNING) << "another merge action is running.";
                  /*un-impossible*/
                }
                // 创建加速动作管理器
                m_MergeActionManagerPtr = std::make_shared<ActionManager>();
                // 初始化加速动作管理器
                m_MergeActionManagerPtr->Initialize(evId_actionId, elemType_elemId);
#endif /*__TX_Mark__("element_state")*/
                LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
                    << "[event_handler_vehicle_merge], id = " << Id()
                    << TX_VARS_NAME(Direction, (__enum2lpsz__(VehicleMoveLaneState, refAct.merge_type())))
                    << TX_VARS_NAME(Duration, (refAct.value())) << TX_VARS_NAME(Offset, (FLAGS_default_lane_width))
                    << ", Run  Success.";
              } else {
                LOG(INFO) << "[event_handler_vehicle_merge], id = " << Id()
                          << TX_VARS_NAME(Direction, (__enum2lpsz__(VehicleMoveLaneState, refAct.merge_type())))
                          << TX_VARS_NAME(Duration, (refAct.value()))
                          << TX_VARS_NAME(Offset, (FLAGS_default_lane_width)) << ", Run  Failure.";
              }
            } else if ((_plus_(VehicleMoveLaneState::eLeftInLane) == refAct.merge_type()) ||
                       (_plus_(VehicleMoveLaneState::eRightInLane) ==
                        refAct.merge_type())) {  // 如果车辆存活且变道状态为左变道或右变道
              // 如果成功执行变道操作
              if (ChangeInLane(refAct.merge_type(), refAct.value(), refAct.lane_offset())) {
#if __TX_Mark__("element_state")
                // 更新场景事件状态
                TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_running);
                // 如果变道动作管理器指针非空
                if (NonNull_Pointer(m_MergeActionManagerPtr)) {
                  ClearMergeAction(timeMgr); /*clear last acc ation*/
                  LOG(WARNING) << "another merge action is running.";
                  /*un-impossible*/
                }
                // 创建加速动作管理器
                m_MergeActionManagerPtr = std::make_shared<ActionManager>();
                // 初始化加速动作管理器
                m_MergeActionManagerPtr->Initialize(evId_actionId, elemType_elemId);
#endif /*__TX_Mark__("element_state")*/
                LOG_IF(INFO, FLAGS_LogLevel_EventTrigger)
                    << "[event_handler_vehicle_merge], id = " << Id()
                    << TX_VARS_NAME(Direction, (__enum2lpsz__(VehicleMoveLaneState, refAct.merge_type())))
                    << TX_VARS_NAME(Duration, (refAct.value())) << TX_VARS_NAME(Offset, (refAct.lane_offset()))
                    << ", Run  Success.";
              } else {
                LOG(INFO) << "[event_handler_vehicle_merge], id = " << Id()
                          << TX_VARS_NAME(Direction, (__enum2lpsz__(VehicleMoveLaneState, refAct.merge_type())))
                          << TX_VARS_NAME(Duration, (refAct.value())) << TX_VARS_NAME(Offset, (refAct.lane_offset()))
                          << ", Run  Failure.";
              }
            }
          }
        }
        m_KinectParamList.clear();
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

Base::txBool TAD_TrajFollow_VehicleElement::ChangeInLane(const Base::Enums::VehicleMoveLaneState state,
                                                         Base::txFloat changeTime,
                                                         const Base::txFloat offset) TX_NOEXCEPT {
#define LaneChangeInfo LOG_IF(INFO, FLAGS_LogLevel_LaneChange) << "[TrajFollow][lane_change]" << TX_VARS(Id())
  using VehicleMoveLaneState = Base::Enums::VehicleMoveLaneState;
  // 如果车辆存活且变道状态为左变道或右变道
  if (IsAlive() &&
      ((_plus_(VehicleMoveLaneState::eLeftInLane)) == state || (_plus_(VehicleMoveLaneState::eLeft)) == state ||
       (_plus_(VehicleMoveLaneState::eRightInLane)) == state || (_plus_(VehicleMoveLaneState::eRight)) == state)) {
    // 如果车辆处于变道状态
    if (IsInLaneChange()) {
      LaneChangeInfo << "[failure] under lane change. " << TX_VARS(State());
      return false;
    } else if (changeTime <= 0.0) {  // 如果变道持续时间小于等于0.0
      LaneChangeInfo << "[failure] under lane change. " << TX_VARS(changeTime);
      return false;
    } else if (offset <= 0.0) {  // 如果变道偏移量小于等于0.0
      LaneChangeInfo << "[failure] under lane change. " << TX_VARS(offset);
      return false;
    }

    // 设置变道持续时间、过程时间、最小取消持续时间、变道动作和变道偏移量
    m_LaneChangeTimeMgr.SetDuration(changeTime);
    m_LaneChangeTimeMgr.SetProcedure(m_LaneChangeTimeMgr.Duration());
    m_LaneChangeTimeMgr.SetMinCancelDuration(m_LaneChangeTimeMgr.Duration());
    TX_MARK("do not Lane Aborting");
    m_LaneChangeTimeMgr.SetChangeInLaneAction(true);
    TX_MARK("Change in Lane.");
    m_LaneChangeTimeMgr.SetChangeInLaneOffset(offset);

    // 根据变道状态设置变道方向
    if ((_plus_(VehicleMoveLaneState::eLeftInLane)) == state || (_plus_(VehicleMoveLaneState::eLeft)) == state) {
      SM::txUserDefinedState::SetTurnLeft_InLane_Start();
      LaneChangeInfo << "start left lane change.";
    } else if ((_plus_(VehicleMoveLaneState::eRightInLane)) == state ||
               (_plus_(VehicleMoveLaneState::eRight)) == state) {
      SM::txUserDefinedState::SetTurnRight_InLane_Start();
      LaneChangeInfo << "start right lane change.";
    }
    return true;
  } else {
    LogWarn << "TAD_TrajFollow_VehicleElement::ChangeInLane state error. " << TX_VARS(Id()) << TX_VARS(State());
    return false;
  }
#undef LaneChangeInfo
}

Base::txFloat TAD_TrajFollow_VehicleElement::MoveTo_Sideway(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  // 获取车辆变道过程时间
  const txFloat experience = m_LaneChangeTimeMgr.Experience();
  // 计算车辆的横向速度
  const txFloat lateral_velocity = Get_MoveTo_Sideway(experience);
  // 更新车辆变道过程时间
  m_LaneChangeTimeMgr.Step(timeMgr.RelativeTime());
  return lateral_velocity;
}

Base::txFloat TAD_TrajFollow_VehicleElement::Get_MoveTo_Sideway(const txFloat exper_time) TX_NOEXCEPT {
  // 如果车辆正在进行车道变更
  if (m_LaneChangeTimeMgr.Procedure() > 0.0) {
    txFloat s = 0.0, l = 0.0, yaw = 0.0;
    // 如果车辆在左转车道
    if (IsInTurnLeft_InLane()) {
      l = m_LaneChangeTimeMgr.GetChangeInLaneOffset();
      /*LOG(WARNING) << TX_VARS(l)
              << TX_VARS_NAME(Duration, m_LaneChangeTimeMgr.Duration())
              << TX_VARS_NAME(l_Duration, std::fabs(l) / m_LaneChangeTimeMgr.Duration());*/
      return std::fabs(l) / m_LaneChangeTimeMgr.Duration();
    } else if (IsInTurnRight_InLane()) {  // 如果车辆在右转车道
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

Base::txFloat TAD_TrajFollow_VehicleElement::Compute_LateralDisplacement(Base::TimeParamManager const& timeMgr)
    TX_NOEXCEPT {
  Base::txFloat cur_step_lateral_displacement = 0.0;
  // 如果车辆的速度大于0.0
  if (GetVelocity() > 0.0) {
    // 计算车辆的横向位移
    cur_step_lateral_displacement = (m_LaneChangeTimeMgr.Duration() > 0.0)
                                        ? (mKinetics.m_LateralVelocity * timeMgr.RelativeTime())
                                        : (-1.0 * mKinetics.m_LateralVelocity * timeMgr.RelativeTime());
  } else {
    TX_MARK("lateral displacement is zero when longitudinal velocity is zero");
    cur_step_lateral_displacement = 0.0;
    mKinetics.m_LateralVelocity = 0.0;
  }
  // 更新车辆的横向位移
  mKinetics.m_LateralDisplacement += cur_step_lateral_displacement;
  return mKinetics.m_LateralDisplacement;
}

Base::txFloat TAD_TrajFollow_VehicleElement::Get_MoveTo_Sideway_Angle(Base::txFloat const exper_time) TX_NOEXCEPT {
  // V_Y=CalculateLateralSpeed();
  Base::txFloat const t = std::max(exper_time, 0.0);
  Base::txFloat lateral_velocity = t * 1.037 - (0.2304444) * std::pow(t, 2);
  Base::txFloat curLaneWidth = FLAGS_default_lane_width;
  /*if (tracker->getCurrentLane()) {
          auto curLaneId = tracker->getCurrentLane()->getTxLaneId();
          curLaneWidth = MapManager::getInstance().GetLaneWidth(curLaneId);
  }*/
  return lateral_velocity * (curLaneWidth) / FLAGS_default_lane_width;
}

Base::txBool TAD_TrajFollow_VehicleElement::LaneChangePostOp(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  Base::txFloat fAngle = 0.0;
  // 将车辆位置设置为车道中心线位置
  mLocation.PosWithLateralWithoutOffset() = mLocation.PosOnLaneCenterLinePos();
  // 如果车辆处于变道状态
  if (IsInLaneChange(/*CHANGE_LANE_STATE::CHANGE_LANE_STATE_START <= changeLaneState*/)) {
    // 计算车辆的横向位移
    Compute_LateralDisplacement(timeMgr);
    Base::txVec3 tmpLaneDir = mLocation.vLaneDir();
    // 计算车辆的目标方向
    const Base::txVec3 vTargetDir =
        Utils::VetRotVecByDegree(tmpLaneDir,
                                 ((IsInTurnRight() || IsInTurnRight_InLane()) ? Unit::txDegree::MakeDegree(RightOnENU)
                                                                              : Unit::txDegree::MakeDegree(LeftOnENU)),
                                 Utils::Axis_Enu_Up());
    // 更新车辆位置
    mLocation.PosWithLateralWithoutOffset().TranslateLocalPos(__East__(vTargetDir) * GetLateralDisplacement(),
                                                              __North__(vTargetDir) * GetLateralDisplacement());
    // 计算车辆的角度
    const txFloat maxV_V = Get_MoveTo_Sideway_Angle(2.25);
    if (!Math::isZero(maxV_V)) {
      if (m_LaneChangeTimeMgr.Duration() > 0.0) {
        const txFloat t = ((m_LaneChangeTimeMgr.Experience() / m_LaneChangeTimeMgr.Duration()) *
                           m_LaneChangeTimeMgr.Average_LaneChanging_Duration());
        const txFloat tmpV_V = Get_MoveTo_Sideway_Angle(t);
        fAngle = tmpV_V * 15.0 / maxV_V;
      } else {
        fAngle = 0.0;
      }
      // 根据车辆的变道类型调整角度
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
    }
    // 更新车辆变道状态
    if (UpdateSwitchLaneRunning(timeMgr)) {
      ClearMergeAction(timeMgr);
      mLaneKeepMgr.ResetCounter(Utils::VehicleBehaviorCfg::LaneKeep(), GetAggress());
    }
  } else {
    // 如果车辆不处于变道状态，设置保持车道状态
    SM::txUserDefinedState::SetLaneKeep();
    mKinetics.m_LateralDisplacement = 0.0;
    mKinetics.m_LateralVelocity = 0.0;
  }
  // 更新车辆角度
  mLocation.fAngle().FromDegree(fAngle);
  RawVehicleCoord() = ComputeLaneOffset(mLocation.PosWithLateralWithoutOffset(), mLocation.vLaneDir(), 0.0);
  // 位置同步
  SyncPosition(timeMgr.PassTime());
  LogInfo << TX_VARS(mLocation.PosOnLaneCenterLinePos().StrWGS84())
          << TX_VARS(mLocation.PosWithLateralWithoutOffset().StrWGS84()) << TX_VARS(GetLateralDisplacement())
          << TX_VARS_NAME(vLaneDir, Utils::ToString(mLocation.vLaneDir())) << TX_VARS(mLocation.LaneOffset())
          << TX_VARS(VehicleCoordStr());
  // 计算中心线偏移
  Compute_CenterLine_Offset();
  if (bNeedResetOffset) {
    bNeedResetOffset = false;
    txFloat s = 0.0, l = 0.0, yaw = 0.0;
    m_trajMgr->xy2sl(RawVehicleCoord().ToENU().ENU2D(), s, l);
    mLocation.LaneOffset() = l;
    LogInfo << "Reset offset " << TX_VARS_NAME(new_lane_offset, mLocation.LaneOffset());
  }
  CheckDeadLine();
  return true;
}

Base::txBool TAD_TrajFollow_VehicleElement::UpdateSwitchLaneRunning(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  // 如果车辆处于变道开始状态
  if (IsInTurn_Start()) {
    // 如果变道过程时间小于等于变道持续时间的60%
    if (m_LaneChangeTimeMgr.Procedure() <=
        m_LaneChangeTimeMgr.Duration() *
            (Utils::VehicleBehaviorCfg::
                 ChangeTopologyThreshold()) /*1229HD LaneChanging_Procedure <= LaneChanging_Duration * 0.6*/) {
      // 将车辆状态从变道开始状态更改为变道进行状态
      ChaneFromStartToIng();
    }
  } else if (IsInTurn_Ing()) {  // 如果车辆处于变道进行状态
    // 如果变道过程时间小于等于0.0
    if (m_LaneChangeTimeMgr.Procedure() <= 0.0) {
      // 如果车辆处于左右变道状态
      if (IsInTurn_LeftRight_InLane()) {
        // 重置变道过程时间、持续时间和变道动作
        m_LaneChangeTimeMgr.SetProcedure(0.0);
        m_LaneChangeTimeMgr.SetDuration(0.0);
        m_LaneChangeTimeMgr.SetChangeInLaneAction(false);
        // 重置车辆的横向位移
        mKinetics.m_LateralDisplacement = 0.0;
        // 将车辆状态从变道进行状态更改为变道完成状态，并设置保持车道状态
        SM::txUserDefinedState::ChaneFromIngToFinish();
        SM::txUserDefinedState::SetLaneKeep();
        bNeedResetOffset = true;
      } else {
        LOG(FATAL) << "un support lane change type.";
      }
      return true;
    }
  } else if (IsInTurn_Abort()) {  // 如果车辆处于变道取消状态
    /*changeLaneState = CHANGE_LANE_STATE_CANCEL_ING; */
  }
  return false;
}

Base::txBool TAD_TrajFollow_VehicleElement::FillingTrajectory(Base::TimeParamManager const& timeMgr,
                                                              sim_msg::Trajectory& refTraj) TX_NOEXCEPT {
#define VehicleTrajInfo LOG_IF(INFO, FLAGS_LogLevel_Vehicle_Traj)
  refTraj.Clear();
  // 保存稳定状态
  SaveStableState();
  // 判断车辆是否存活且相对时间大于0.0
  if (IsAlive() && timeMgr.RelativeTime() > 0.0) {
    // 设置参考轨迹的加速度
    refTraj.set_a(GetAcc());
    // 设置轨迹持续时间
    const Base::txInt n_traj_duration_s = FLAGS_vehicle_trajectory_duration_s;
    // 输出车辆速度和加速度信息
    VehicleTrajInfo << TX_VARS(GetVelocity()) << TX_VARS(GetAcc());
    // 定义一个存储时间和轨迹点的元组向量
    std::vector<std::tuple<Base::txInt /*million second*/, sim_msg::TrajectoryPoint*> > vec_ms_time_traj_point;
    // 计算车辆的速度
    Base::txFloat cur_velocity = __HYPOT__(GetVelocity(), GetLateralVelocity());
    // 获取车辆的加速度
    const Base::txFloat cur_acc = GetAcc();
    Base::txFloat cur_s = 0.0;
    const Base::txFloat traj_total_length = m_trajMgr->GetLength();
    // 获取车辆的实际位移
    const Base::txFloat traj_base = m_real_displacement;
    const Base::txFloat dist_to_traj_end = DistanceToEnd();
    const Base::txFloat ego_alt = Altitude();
    for (Base::txFloat start_s = 0.0; start_s <= FLAGS_vehicle_trajectory_duration_s;
         start_s += timeMgr.RelativeTime()) {
      // 如果车辆距离参考轨迹终点的距离小于参考轨迹的总长度
      if (cur_s < dist_to_traj_end) {
        sim_msg::TrajectoryPoint* new_traj_point_ptr = refTraj.add_point();
        // 设置轨迹点的海拔高度
        new_traj_point_ptr->set_z(ego_alt);
        new_traj_point_ptr->set_t(start_s + timeMgr.AbsTime());
        // 设置轨迹点的速度
        new_traj_point_ptr->set_v(cur_velocity);
        new_traj_point_ptr->set_a(GetAcc());
        new_traj_point_ptr->set_s(cur_s);
        /*1. update velocity*/
        // 更新车辆的速度
        cur_velocity += cur_acc * timeMgr.RelativeTime();
        if (cur_velocity >= mKinetics.m_velocity_max) {
          cur_velocity = mKinetics.m_velocity_max;
          // cur_acc = 0.0;
        }
        if (cur_velocity < 0.0) {
          cur_velocity = 0.0;
          // cur_acc = 0.0;
        }
        /*2. update s*/
        cur_s += cur_velocity * timeMgr.RelativeTime();
        const Base::txFloat cur_s_local = new_traj_point_ptr->s();
        const Base::txFloat cur_s_global = cur_s_local + m_real_displacement;
        if (cur_s_local < dist_to_traj_end) {
          // 计算车辆的坐标
          Coord::txWGS84 cur_pt_wgs84;
          cur_pt_wgs84.FromENU(m_trajMgr->GetLocalPos(cur_s_global));
          // 计算车辆的方向
          const Base::txVec3 tmpLaneDir = mLocation.vLaneDir();
          const Base::txVec3 vTargetDir =
              Utils::VetRotVecByDegree(tmpLaneDir, Unit::txDegree::MakeDegree(LeftOnENU), Utils::Axis_Enu_Up());
          cur_pt_wgs84.TranslateLocalPos(__East__(vTargetDir) * mLocation.LaneOffset(),
                                         __North__(vTargetDir) * mLocation.LaneOffset());
          // 如果车辆正在变道，更新车辆的位置
          if (CallSucc(IsInLaneChange())) {
            Base::txFloat l = m_LaneChangeTimeMgr.GetChangeInLaneOffset();
            if (m_LaneChangeTimeMgr.Duration() > 0.0) {
              l = l * ((m_LaneChangeTimeMgr.Experience() / m_LaneChangeTimeMgr.Duration()));
            }
            static const Unit::txDegree sRightDir = Unit::txDegree::MakeDegree(RightOnENU);
            static const Unit::txDegree sLeftDir = Unit::txDegree::MakeDegree(LeftOnENU);
            const Base::txVec3 vTargetDir_lanechange = Utils::VetRotVecByDegree(
                tmpLaneDir, ((IsInTurnRight() || IsInTurnRight_InLane()) ? (sRightDir) : (sLeftDir)),
                Utils::Axis_Enu_Up());
            cur_pt_wgs84.TranslateLocalPos(__East__(vTargetDir_lanechange) * l, __North__(vTargetDir_lanechange) * l);
            // LOG(INFO) << Utils::ToString(vTargetDir_lanechange);
          }
          // 设置车辆的位置
          new_traj_point_ptr->set_x(cur_pt_wgs84.Lon());
          new_traj_point_ptr->set_y(cur_pt_wgs84.Lat());
          // 计算车辆的方向
          const Base::txFloat _headingRadian =
              Utils::GetLaneAngleFromVectorOnENU(m_trajMgr->GetLaneDir(cur_s_global)).GetRadian();
          new_traj_point_ptr->set_theta(_headingRadian);
        }
      } else {
        break;
      }
    }
    return true;
  } else {
    return false;
  }
#undef VehicleTrajInfo
}

void TAD_TrajFollow_VehicleElement::ClearMergeAction(TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(m_MergeActionManagerPtr)) {
    // 更新场景事件状态
    TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(
        m_MergeActionManagerPtr->evId_actionId(),
        TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
            m_MergeActionManagerPtr->self_elemType_elemId(),
            _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::completeState), timeMgr.PassTime()));
    m_MergeActionManagerPtr = nullptr;
    LOG(INFO) << " merge action hit the target. stop action.";
  }
}
TX_NAMESPACE_CLOSE(TrafficFlow)
