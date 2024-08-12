// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_ego_vehicle_element.h"
#include "HdMap/tx_hashed_road.h"
#include "tx_frame_utils.h"
#include "tx_spatial_query.h"
#include "tx_tadsim_flags.h"
#include "tx_timer_on_cpu.h"
#ifdef ON_CLOUD
#  include "tx_tc_cloud_loader.h"
#endif /*ON_CLOUD*/
#include <set>
#include "mapengine/hadmap_engine.h"
#include "tx_collision_detection2d.h"
#include "types/map_defs.h"
#define LogInfo                                                                                              \
  LOG_IF(INFO, FLAGS_LogLevel_Ego || FLAGS_LogLevel_EventTrigger || FLAGS_LogLevel_Statistics_MultiThread || \
                   FLAGS_LogLevel_Kinetics)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

TAD_EgoVehicleElement::~TAD_EgoVehicleElement() { /*LOG(WARNING) << "[~]TAD_EgoVehicleElement";*/ }

Base::txBool TAD_EgoVehicleElement::Initialize_Kinetics(Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr)
    TX_NOEXCEPT {
  if (NonNull_Pointer(_elemAttrViewPtr)) {
#ifdef ON_LOCAL
    mKinetics.m_velocity = _elemAttrViewPtr->start_v();
    mKinetics.LastVelocity() = mKinetics.m_velocity;
    mKinetics.m_velocity_max = _elemAttrViewPtr->max_v();
    mKinetics.velocity_desired = mKinetics.m_velocity_max;
    mKinetics.raw_velocity_max = mKinetics.m_velocity_max;
#endif
    return true;
  } else {
    LogWarn << "Initialize_Kinetics Error. " << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr));
    return false;
  }
}

Base::txBool TAD_EgoVehicleElement::Initialize(Base::ISceneLoader::IViewerPtr _absAttrView,
                                               Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 将当前车道起始节点的顶点 ID 和顶点属性存储到 map_vtxId_2_vtxProp 中
  if (NonNull_Pointer(_absAttrView) && NonNull_Pointer(_sceneLoader) && CallSucc(_absAttrView->IsInited())) {
#if USE_EgoGroup
    if (CallFail(_sceneLoader->GetVehicleGeometory(_absAttrView->name(), GetSubType(), VehicleGeometory()))) {
#else  /*USE_EgoGroup*/
    // 调用 _sceneLoader->GetVehicleGeometory() 函数，传入参数 0（车辆 ID）、GetSubType()（车辆子类型）和
    // VehicleGeometory()（车辆几何信息） 这个函数的作用是获取指定车辆的几何信息
    if (CallFail(_sceneLoader->GetVehicleGeometory(0, GetSubType(), VehicleGeometory()))) {
#endif /*USE_EgoGroup*/
      LogWarn << "GetVehicleGeometory failure.";
      // 使用传入的参数设置vehicle的属性
      VehicleGeometory().mutable_vehicle_geometory()->set_length(FLAGS_EGO_Length);
      VehicleGeometory().mutable_vehicle_geometory()->set_width(FLAGS_EGO_Width);
      VehicleGeometory().mutable_vehicle_geometory()->set_height(FLAGS_EGO_Height);
      return false;
    } else {
      // 打印车辆几何信息
      LogInfo << VehicleGeometory().DebugString() << TX_VARS(VehicleGeometory().vehicle_geometory().length())
              << TX_VARS(VehicleGeometory().vehicle_geometory().rear_axle_to_rear());
    }
    m_vehicle_come_from = Base::IVehicleElement::VehicleSource::ego;
    _elemAttrViewPtr = std::dynamic_pointer_cast<Base::ISceneLoader::IVehiclesViewer>(_absAttrView);
    // 检查视图指针不为空
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      // 设置当前车辆属性
      mIdentity.Id() = _elemAttrViewPtr->id();
      mIdentity.SysId() = CreateSysId(mIdentity.Id());
      mIdentity.Name() = __strcat__(_absAttrView->name(), GetSubType());
      mGeometryData.Length() = VehicleGeometory().vehicle_geometory().length();
      mGeometryData.Width() = VehicleGeometory().vehicle_geometory().width();
      mGeometryData.Height() = VehicleGeometory().vehicle_geometory().height();
#if USE_SUDOKU_GRID
      Initialize_SUDOKU_GRID();
#endif /*USE_SUDOKU_GRID*/
#if USE_EgoGroup
      Base::Enums::EgoType egoType = _sceneLoader->GetEgoType(_absAttrView->name());
#else  /*USE_EgoGroup*/
      Base::Enums::EgoType egoType = _sceneLoader->GetEgoType();
#endif /*USE_EgoGroup*/
      // 设置vehicle的类型
      if ((_plus_(Base::Enums::EgoType::eVehicle)) == (egoType)) {
        mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::EgoSUV;
      } else if ((_plus_(Base::ISceneLoader::EgoType::eTruck)) == (egoType)) {
        if ((_plus_(EgoSubType::eLeader)) == GetSubType()) {
          mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::EgoTruckLeader;
        } else {
          mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::EgoTruckFollower;
        }
      } else {
        mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::EgoSUV;
        LogWarn << "Unknown Ego Type, Set Default type SUV.";
      }

#if USE_EgoGroup
      if (CallFail(Initialize_Kinetics(_elemAttrViewPtr TX_MARK("visualizer")))) {
        LogWarn << "initialize kinetics error.";
        return false;
      }

      const auto egoRouteDataPtr = _sceneLoader->GetEgoRouteData(_absAttrView->name());
#else  /*USE_EgoGroup*/
      // 获取ego的路由数据
      const auto egoRouteDataPtr = _sceneLoader->GetEgoRouteData();
#endif /*USE_EgoGroup*/
      if (NonNull_Pointer(egoRouteDataPtr)) {
        LogInfo << "[XOSC_SCENE_LOADER] " << (egoRouteDataPtr->Str());
        // 获取起点的经纬度
        const txFloat _lon = egoRouteDataPtr->startLon();
        const txFloat _lat = egoRouteDataPtr->startLat();
        if (Math::isNotZero(_lon) && Math::isNotZero(_lat)) {
          // 保证ego起点在车道内
          const txBool bLocateOnLaneLink = false;
          TX_MARK("ego must be on lane");
          RawVehicleCoord().FromWGS84(_lon, _lat);
          if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
                  RawVehicleCoord(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset())) {
            if (CallFail(mLocation.IsOnLaneLink())) {
              // 获取一开始初始化的车道指针
              hadmap::txLanePtr initLane =
                  HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
              if (NonNull_Pointer(initLane)) {
                LogInfo << TX_VARS_NAME(InitEgoVehicle_initLane_Uid, Utils::ToString(initLane->getTxLaneId()));
                // 初始化ego location的详细信息
                mLocation.PosOnLaneCenterLinePos() = HdMap::HadmapCacheConCurrent::GetLanePos(
                    initLane->getTxLaneId(), GenerateST(0.0, mLocation.DistanceAlongCurve()));
                // 获取车道方向并设置给 mLocation
                mLocation.vLaneDir() =
                    HdMap::HadmapCacheConCurrent::GetLaneDir(initLane, GenerateST(0.0, mLocation.DistanceAlongCurve()));
                mLocation.InitTracker(mIdentity.Id());
                // 将追踪器重新定位到初始车道和距离为 0 的位置
                RelocateTracker(initLane, 0.0);
                SyncPosition(0.0);
                mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
                LogInfo << "[Ego_Init]" << TX_VARS(Id()) << VehicleCoordStr()
                        << TX_VARS_NAME(laneUid, Utils::ToString(initLane->getTxLaneId()));
                // 保存稳定状态
                SaveStableState();
                return true;
              } else {
                LogWarn << ", Get Init Lane Error." << TX_VARS(_lon) << TX_VARS(_lat);
                return false;
              }
            } else {
              // 获取车道连接指针
              hadmap::txLaneLinkPtr initLaneLink = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(
                  mLocation.LaneLocInfo().onLinkFromLaneUid, mLocation.LaneLocInfo().onLinkToLaneUid);
              if (NonNull_Pointer(initLaneLink)) {
                // 设置车道中心线上的位置
                mLocation.PosOnLaneCenterLinePos() =
                    HdMap::HadmapCacheConCurrent::GetLaneLinkPos(initLaneLink, mLocation.DistanceAlongCurve());
                // 设置车辆的方向
                mLocation.vLaneDir() =
                    HdMap::HadmapCacheConCurrent::GetLaneLinkDir(initLaneLink, mLocation.DistanceAlongCurve());
                mLocation.InitTracker(mIdentity.Id());
                // 重定位车道信息
                RelocateTracker(initLaneLink, 0.0);
                SyncPosition(0.0);
                mGeometryData.ComputePolygon(GeomCenter().ToENU(), mLocation.vLaneDir());
                // LogInfo << "[Ego_Init]" << TX_VARS(Id()) << VehicleCoordStr()
                //     << TX_VARS_NAME(lanelinkUid, initLaneLink->getId());
                // 保存稳定状态
                SaveStableState();
                return true;
              } else {
                LogWarn << ", Get Init LaneLink Error." << TX_VARS(_lon) << TX_VARS(_lat);
                return false;
              }
            }
          } else {
            LogWarn << "Ego Init Pos Error." << TX_VARS(_lon) << TX_VARS(_lat);
            return false;
          }
        } else {
          LogWarn << "InitEgoVehicle, Lon & Lat Error." << TX_VARS(_lon) << TX_VARS(_lat);
          return false;
        }
      } else {
        LogWarn << "InitEgoVehicle Get Ego Route Data Failure.";
        return false;
      }
    } else {
      LogWarn << "Ego Param Cast Error." << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr));
      return false;
    }
  } else {
    LogWarn << "Ego Initialize Param Error." << TX_COND_NAME(_absAttrView, NonNull_Pointer(_absAttrView))
            << TX_COND_NAME(_sceneLoader, NonNull_Pointer(_sceneLoader))
            << TX_COND_NAME(_absAttrView_IsInited, CallSucc(_absAttrView->IsInited()));
    return false;
  }
}

Base::txFloat TAD_EgoVehicleElement::GetShowVelocity() const TX_NOEXCEPT {
  // 从 m_egoData 中获取速度信息
  Base::txVec3 velocity3d(m_egoData.velocity().x(), m_egoData.velocity().y(), m_egoData.velocity().z());
  // 计算速度的模长（即速度的大小）
  return velocity3d.norm();
}

void TAD_EgoVehicleElement::ComputeScalarVelocity(Base::TimeParamManager const& timeMgr,
                                                  const Base::txVec2& egoRearAxleCenter) TX_NOEXCEPT {
  // 获取速度和加速度
  Base::txVec2 velocity2d = Base::txVec2(m_egoData.velocity().x(), m_egoData.velocity().y());
  Base::txVec2 acce2d = Base::txVec2(m_egoData.acceleration().x(), m_egoData.acceleration().y());
  // 计算速度和加速度模长
  mKinetics.m_velocity = velocity2d.norm();
  mKinetics.m_acceleration = acce2d.norm();
  LogInfo << "[Ego][succ]" << TX_VARS(mKinetics.m_velocity) << TX_VARS(mKinetics.m_acceleration);
}

Base::txBool TAD_EgoVehicleElement::UpdateEgoVehicle(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  hadmap::txLanePtr resLane = nullptr;
  hadmap::txLaneLinkPtr resLaneLink = nullptr;
  const Base::txVec2 cur_rear_axle_center_enu2d = RawVehicleCoord().ToENU().ENU2D();
  // 计算速度和加速度
  ComputeScalarVelocity(TX_MARK("SIM-3793") timeMgr, cur_rear_axle_center_enu2d);
  // 计时器
  Base::TimingCPU s_timer;
  LogInfo << "hadmap::getLane start." << RawVehicleCoord().StrWGS84();
  s_timer.StartCounter();

  // 获取车道信息
  if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
          RawVehicleCoord(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve())) {
    LogInfo << "Get_S_Coord_By_Enu_Pt : " << mLocation.LaneLocInfo();
    // 判断位置是否在车道
    if (mLocation.IsOnLane()) {
      // 获取车道
      resLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
      if (NonNull_Pointer(resLane)) {
        LogInfo << "hadmap::getLane success.";
        // 重定位位置
        RelocateTracker(resLane, timeMgr.TimeStamp());
      } else {
        LogWarn << "Ego not on road.";
        return false;
      }
    } else {
      // 获取车道连接
      resLaneLink = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(mLocation.LaneLocInfo().onLinkFromLaneUid,
                                                                   mLocation.LaneLocInfo().onLinkToLaneUid);
      if (NonNull_Pointer(resLaneLink)) {
        LogInfo << "hadmap::getLaneLink success.";
        // 重定位位置
        RelocateTracker(resLaneLink, timeMgr.TimeStamp());
      } else {
        LogWarn << "Ego not on link.";
        return false;
      }
    }
  } else {
    LogWarn << "Get_S_Coord_By_Enu_Pt failure.";
    return false;
  }

  // 打印位置信息
  LogInfo << "hadmap::getLane end." << TX_VARS_NAME(timer, s_timer.GetElapsedMicroseconds())
          << ((mLocation.IsOnLaneLink()) ? "[onLink]" : "[onLane]") << TX_VARS(mLocation.DistanceAlongCurve());

  // 计算偏航方向
  Base::txVec2 yawDir(std::cos(m_egoData.rpy().z()), std::sin(m_egoData.rpy().z()));
  // 获取几何中心的 ENU 坐标
  const Base::txVec2 cur_geom_center_enu_2d = GeomCenter().ToENU().ENU2D();
  // 计算投影矩阵
  ComputeProjectionMat(cur_geom_center_enu_2d, (cur_geom_center_enu_2d + 2.0 * yawDir));
  // 设置车道方向
  mLocation.vLaneDir() = Base::txVec3(__East__(yawDir), __North__(yawDir), 0.0);

  // 根据车道信息计算车道方向
  if (NonNull_Pointer(resLaneLink)) {
    mLocation.LocalCoord_AxisY() =
        HdMap::HadmapCacheConCurrent::GetLaneLinkDir(resLaneLink, mLocation.DistanceAlongCurve());
    LogInfo << TX_VARS_NAME(Ego_OnLink_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  } else if (NonNull_Pointer(resLane)) {
    mLocation.LocalCoord_AxisY() = HdMap::HadmapCacheConCurrent::GetLaneDir(resLane, mLocation.DistanceAlongCurve());
    LogInfo << TX_VARS_NAME(Ego_OnLane_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  } else {
    mLocation.LocalCoord_AxisY() = Base::txVec3(__East__(yawDir), __North__(yawDir), 0.0);
    LogInfo << TX_VARS_NAME(Ego_NotOnMap_LocalCoord_AxisY, Utils::ToString(mLocation.LocalCoord_AxisY()));
  }
  mLocation.LocalCoord_AxisX() = Utils::VetRotVecByDegree(mLocation.LocalCoord_AxisY(),
                                                          Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());
  LogInfo << TX_VARS_NAME(Ego_LocalCoord_AxisX, Utils::ToString(mLocation.LocalCoord_AxisX()));
  mLocation.NextLaneIndex() = Utils::ComputeLaneIndex(mLocation.LaneLocInfo().onLaneUid.laneId);
  return true;
}

Base::txBool TAD_EgoVehicleElement::Post_Update(TimeParamManager const& timeMgr) TX_NOEXCEPT {
  // 如果当前时间大于上一次的时间，则执行 SnapshotHistoryTrajectory 函数
  if (m_egoData.t() > m_last_egoData.t()) {
    SnapshotHistoryTrajectory(timeMgr);
  }
  // 更新上一次的ego数据
  m_last_egoData.CopyFrom(m_egoData);
  return ParentClass::Post_Update(timeMgr);
}

Base::txBool TAD_EgoVehicleElement::Pre_Update(
    const TimeParamManager& timeMgr, std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT {
  LogInfo << TX_VARS(GetCurrentLaneInfo()) << TX_VARS_NAME(dir, Utils::ToString(StableLaneDir()));
  // 调用ParenClass的前置更新
  return ParentClass::Pre_Update(timeMgr, map_elemId2Kinetics);
}

Base::txBool TAD_EgoVehicleElement::Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  m_self_KineticsUtilInfo.m_isValid = false;
  // 当前元素活跃
  if (CallSucc(IsAlive())) {
    Base::TimingCPU s_timer;
    s_timer.StartCounter();

    // 解析pb数据
    m_egoData.ParseFromString(m_pb_str);
    LogInfo << _StreamPrecision_ << TX_VARS(GetSubType()) << TX_VARS(m_pb_str.size()) << TX_VARS(m_egoData.t())
            << timeMgr.str() << TX_VARS(m_egoData.position().x()) << TX_VARS(m_egoData.position().y())
            << TX_VARS(m_egoData.position().z()) << TX_VARS(m_egoData.velocity().x())
            << TX_VARS(m_egoData.velocity().y()) << TX_VARS(m_egoData.velocity().z())
            << TX_VARS_NAME(ego_heading, m_egoData.rpy().z());
    m_pb_str.clear();

    Base::txVec2 yawDir(std::cos(m_egoData.rpy().z()), std::sin(m_egoData.rpy().z()));
    mLocation.vLaneDir() = Base::txVec3(__East__(yawDir), __North__(yawDir), 0.0);

    // 根据不同的类型转换坐标
    if (IsLeader()) {
      RawVehicleCoord().FromWGS84(m_egoData.position().x(), m_egoData.position().y(), m_egoData.position().z());
    } else {
      RawVehicleCoord().FromWGS84(m_egoData.position().x(), m_egoData.position().y(), m_egoData.position().z());
    }
    // 对位置同步
    SyncPosition(timeMgr.PassTime());

    LogInfo << VehicleCoordStr();

    // 如果 UpdateEgoVehicle 函数执行失败，则设置几何数据为无效，并输出警告信息
    if (CallFail(UpdateEgoVehicle(timeMgr))) {
      mGeometryData.SetInValid();
      LogWarn << "Ego UpdateEgoVehicle failure.";
      return false;
    } else {  // 如果 UpdateEgoVehicle 函数执行成功，则更新下一个路口的方向
      UpdateNextCrossDirection();
    }
    const Coord::txENU geom_center_enu = GeomCenter().ToENU();
#if USE_Component
    // 计算车辆前轴的二维向量
    Base::txVec2 frontAxis2d = (m_p1 - m_p0);
    // 将前轴向量归一化
    frontAxis2d.normalize();
    // 使用车辆的中心点和前轴向量计算车辆的多边形几何数据
    mGeometryData.ComputePolygon(geom_center_enu, Utils::Vec2_Vec3(frontAxis2d));

    LogInfo << TX_VARS_NAME(timer, s_timer.GetElapsedMicroseconds());
    // 如果启用了 EgoTrailer 日志级别，则输出车辆的子类型、坐标、速度、方向和几何数据
    if (FLAGS_LogLevel_EgoTrailer) {
      LOG(INFO) << "[EgoTrailer]" << TX_VARS_NAME(SubType, GetSubType()) << TX_VARS(RawVehicleCoord())
                << TX_VARS(mKinetics.m_velocity) << TX_VARS_NAME(dir, Utils::ToString(frontAxis2d))
                << TX_VARS_NAME(Geometry, mGeometryData);
    }
#endif /*USE_Component*/
    UpdateHashedLaneInfo(mLocation);
#if USE_VehicleKinectInfo
    // 分别调用对应函数设置动力学信息
    m_self_KineticsUtilInfo.m_isValid = true;
    m_self_KineticsUtilInfo.vElementDir = GetLaneDir();
    m_self_KineticsUtilInfo._ElemId = Id();
    m_self_KineticsUtilInfo._SysId = SysId();
    m_self_KineticsUtilInfo._ElementGeomCenter = geom_center_enu;
    m_self_KineticsUtilInfo._vecPolygon = Polygon();
    m_self_KineticsUtilInfo._Velocity = GetVelocity();
    m_self_KineticsUtilInfo._Acc = GetAcc();
    m_self_KineticsUtilInfo._LocalCoords = mLocation.LocalCoords();
    m_self_KineticsUtilInfo._LastElementGeomCenter = StableGeomCenter();
    const Unit::txDegree ego_headingAngle = Utils::GetLaneAngleFromVectorOnENU(mLocation.LocalCoord_AxisY());
    Utils::CreateLocalCoordTranslationMat(geom_center_enu.ENU2D(), ego_headingAngle, m_self_KineticsUtilInfo._TransMat);
    m_self_KineticsUtilInfo._TransMatInv = m_self_KineticsUtilInfo._TransMat.inverse();
#endif /*USE_VehicleKinectInfo*/
  } else {
    // LOG(WARNING) << "Ego is not alive.";
  }
  return true;
}

Base::txBool TAD_EgoVehicleElement::Release() TX_NOEXCEPT {
  // 设置生命周期
  mLifeCycle.SetEnd();
  return true;
}

void TAD_EgoVehicleElement::SetProtoBufferStr(const Base::txString& strPB) TX_NOEXCEPT {
  // 将传入的 ProtoBuffer 字符串赋值给 m_pb_str
  m_pb_str = strPB;
  // 如果 m_pb_str 为空，则调用 SetEgoStart 函数
  if (CallFail(m_pb_str.empty())) {
    SetEgoStart();
  }
}

// 获取车辆的 ProtoBuffer 节点
::sim_msg::Location TAD_EgoVehicleElement::GetProtoBufferNode() const TX_NOEXCEPT {
  sim_msg::Location egoData;
  // 将 m_egoData 的内容复制到 egoData
  egoData.CopyFrom(m_egoData);
  return std::move(egoData);
  /*sim_msg::Location egoData;
  egoData.ParseFromString(m_pb_str); "m_pb_str clear in update"
  return egoData;*/
}

Base::txFloat TAD_EgoVehicleElement::ComputeSlope() const TX_NOEXCEPT {
  // 车辆存活，计算坡度
  if (CallSucc(IsAlive())) {
    hadmap::txSlopeVec slope_vec;
    const hadmap::txCurve* geom_curve_ptr = nullptr;
    // 在车道上时
    if (mLocation.IsOnLane()) {
      // 获取道路指针
      auto road_ptr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(mLocation.LaneLocInfo().onLaneUid.roadId);
      if (NonNull_Pointer(road_ptr)) {
        // 计算车辆所在车道对应的坡度
        slope_vec = road_ptr->getSlope();
        geom_curve_ptr = road_ptr->getGeometry();
      }
    } else {  // 在车道连接上时
      hadmap::txLaneLinkPtr linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(
          mLocation.LaneLocInfo().onLinkFromLaneUid, mLocation.LaneLocInfo().onLinkToLaneUid);
      if (NonNull_Pointer(linkPtr)) {
        slope_vec = linkPtr->getSlope();
        geom_curve_ptr = linkPtr->getGeometry();
      }
    }

    // 如果 geom_curve_ptr 不为空且 slope_vec 不为空
    if (NonNull_Pointer(geom_curve_ptr) && _NonEmpty_(slope_vec)) {
      Base::txFloat s, l, yaw;
      // 将车辆的 x 和 y 坐标转换为 s 和 l 坐标
      geom_curve_ptr->xy2sl(m_egoData.position().x(), m_egoData.position().y(), s, l, yaw);
      for (Base::txSize i = 0; i < slope_vec.size(); ++i) {
        // 如果 s 大于 slope_vec[i].m_starts
        if (s > slope_vec[i].m_starts) {
          LogInfo << "[slope]" << TX_VARS(i) << TX_VARS(s) << TX_VARS_NAME(starts, slope_vec[i].m_starts);
          return slope_vec[i].m_slope;
        }
      }
      return 0.0;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
  return 0;
}

Base::txBool TAD_EgoVehicleElement::FillingElement(Base::TimeParamManager const&,
                                                   sim_msg::Traffic& trafficData) TX_NOEXCEPT {
  sim_msg::EgoInfo* ego_info = nullptr;
  {
#if USE_TBB
    tbb::mutex::scoped_lock lock(s_tbbMutex_outputTraffic);
#endif
    ego_info = trafficData.add_egos();
  }

  // egoinfo不为空
  if (NonNull_Pointer(ego_info)) {
    // 设置车辆信息
    const Base::txVec3 centerLineDir3d =
        HdMap::HadmapCacheConCurrent::GetLaneDir(mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve());
    const Base::txFloat _headingDegree = Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir()).GetDegree();
    const Base::txFloat _laneDegree = Utils::GetLaneAngleFromVectorOnENU(centerLineDir3d).GetDegree();
    // 设置相关信息
    ego_info->set_angle_along_centerline_degree(Math::DegreeNormalise(_headingDegree - _laneDegree,
                                                                      FLAGS_EgoInfo_AngleCenterLine_LeftAngle,
                                                                      FLAGS_EgoInfo_AngleCenterLine_RightAngle));
    ego_info->set_slope(ComputeSlope());
    LogInfo << "[slope_angle_along_centerline_degree]"
            << TX_VARS_NAME(angle_along_centerline_degree, ego_info->angle_along_centerline_degree())
            << TX_VARS_NAME(slope, ego_info->slope()) << TX_VARS(_headingDegree) << TX_VARS(_laneDegree);
  }
//    LogWarn << "[ego_register]" << TX_VARS(m_curHashedLaneInfo) << TX_VARS(GeomCenter().ToENU().ENU2D());
#if __TX_Mark__("simulation consistency")
  // 保存状态
  SaveStableState();
#endif /*__TX_Mark__("simulation consistency")*/
  return true;
  /*return FillingSpatialQuery();*/
}

Base::txString TAD_EgoVehicleElement::KineticsDesc() const TX_NOEXCEPT {
  return Base::txString("TAD_EgoVehicleElement");
}

TAD_EgoVehicleElement::KineticsUtilInfo_t TAD_EgoVehicleElement::GetKineticsUtilInfo() const TX_NOEXCEPT {
  return m_self_KineticsUtilInfo;
}

Base::txBool TAD_EgoVehicleElement::ComputeKinetics(Base::TimeParamManager const&,
                                                    const KineticsUtilInfo_t&) TX_NOEXCEPT {
  return false;
}

TAD_EgoVehicleElement::VEHICLE_TYPE TAD_EgoVehicleElement::VehicleType() const TX_NOEXCEPT {
  // 获取ego类型
  if (_plus_(Base::Enums::EgoType::eVehicle) == GetEgoType()) {
    return VEHICLE_TYPE::SUV;
    // eTruck时
  } else if (_plus_(Base::Enums::EgoType::eTruck) == GetEgoType()) {
    // 分别判断返回
    if (_plus_(EgoSubType::eLeader) == GetSubType()) {
      return VEHICLE_TYPE::Truck;
    } else {
      return VEHICLE_TYPE::Semi_Trailer_Truck_001;
    }
  } else {
    LOG(FATAL) << "Ego Type error.";
    return VEHICLE_TYPE::SUV;
  }
}

void TAD_EgoVehicleElement::UpdateNextCrossDirection() TX_NOEXCEPT {
  // 在车道上
  if (mLocation.IsOnLane()) {
    // 清除 m_op_self_next_driving_direction 的值
    m_op_self_next_driving_direction.clear_value();
    // 设置车辆在车道上的信息
    JunctionInfo_SetLastOnLaneInfo(RawVehicleCoord().ToENU(), GetLaneDir());
  } else {
    if (CallFail(m_op_self_next_driving_direction.has_value())) {
      hadmap::txLanePtr pToLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLinkToLaneUid);
      // 获取车辆在车道上的方向
      const Base::txVec3& fromLaneDir = JunctionInfo_LastOnLaneDir();
      const Base::txVec3 toLaneDir = HdMap::HadmapCacheConCurrent::GetLaneDir(pToLane, 1.0);
      // 计算车辆的驾驶方向
      m_op_self_next_driving_direction = Base::GetSelfDrivingDirection(fromLaneDir, toLaneDir);
    }
  }
}

Base::txFloat TAD_EgoVehicleElement::GetRawVelocity() const TX_NOEXCEPT {
  return sqrt(m_egoData.velocity().x() * m_egoData.velocity().x() +
              m_egoData.velocity().y() * m_egoData.velocity().y());
}

#ifdef ON_CLOUD
Base::txBool TAD_EgoVehicleElement::InitializeOnCloud(ISceneLoader::IViewerPtr _absAttrView,
                                                      ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  if (NonNull_Pointer(_absAttrView) && NonNull_Pointer(_sceneLoader) && CallSucc(_absAttrView->IsInited())) {
    _elemAttrViewPtr = std::dynamic_pointer_cast<Base::ISceneLoader::IVehiclesViewer>(_absAttrView);
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      mIdentity.Id() = _elemAttrViewPtr->id();
      mIdentity.SysId() = CreateSysId(mIdentity.Id());

      mGeometryData.Length() = _elemAttrViewPtr->length();
      mGeometryData.Width() = _elemAttrViewPtr->width();
      mGeometryData.Height() = _elemAttrViewPtr->height();

      mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::EgoSUV;

      SceneLoader::TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewerPtr _EgoCloudAttrViewPtr =
          std::dynamic_pointer_cast<SceneLoader::TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer>(_absAttrView);
      if (NonNull_Pointer(_EgoCloudAttrViewPtr)) {
        const txFloat _lon = _EgoCloudAttrViewPtr->lon();
        const txFloat _lat = _EgoCloudAttrViewPtr->lat();
        if (Math::isNotZero(_lon) && Math::isNotZero(_lat)) {
          const txBool bLocateOnLaneLink = false;
          TX_MARK("ego must be on lane");
          RawVehicleCoord().FromWGS84(_lon, _lat);
          SyncPosition(0.0);
          if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
                  RawVehicleCoord(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset()) &&
              CallFail(mLocation.IsOnLaneLink())) {
            hadmap::txLanePtr initLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
            if (NonNull_Pointer(initLane)) {
              LogInfo << TX_VARS_NAME(InitEgoVehicle_initLane_Uid, Utils::ToString(initLane->getTxLaneId()));
              mLocation.PosOnLaneCenterLinePos() = HdMap::HadmapCacheConCurrent::GetLanePos(
                  initLane->getTxLaneId(), GenerateST(0.0, mLocation.DistanceAlongCurve()));

              LogInfo << "[Ego_Init]" << TX_VARS(RawVehicleCoord())
                      << TX_VARS_NAME(laneUid, Utils::ToString(initLane->getTxLaneId()));
              mLocation.vLaneDir() =
                  HdMap::HadmapCacheConCurrent::GetLaneDir(initLane, GenerateST(0.0, mLocation.DistanceAlongCurve()));
              mLocation.InitTracker(mIdentity.Id());
              RelocateTracker(initLane, 0.0);
              SaveStableState();
              {
                Base::txString strPB;
                _EgoCloudAttrViewPtr->egoLocation().SerializeToString(&strPB);
                SetProtoBufferStr(strPB);
                LOG(INFO) << "[EgoOnCloud][Initialize][Success]" << TX_VARS_NAME(SubType, GetSubType())
                          << VehicleCoordStr()
                          // << TX_VARS_NAME(Pos, GetPosition())
                          << TX_VARS_NAME(Id, Id()) << TX_VARS_NAME(Length, GetLength())
                          << TX_VARS_NAME(Width, GetWidth()) << TX_VARS_NAME(Heigth, GetHeigth());
              }
              return true;
            } else {
              LogWarn << ", Get Init Lane Error." << TX_VARS(_lon) << TX_VARS(_lat);
              return false;
            }
          } else {
            LogWarn << "Ego Init Pos Error." << TX_VARS(_lon) << TX_VARS(_lat);
            return false;
          }
        } else {
          LogWarn << "InitEgoVehicle, Lon & Lat Error." << TX_VARS(_lon) << TX_VARS(_lat);
          return false;
        }
      } else {
        LogWarn << "Cover TAD_Cloud_EgoVehiclesViewer Failure.";
        return false;
      }
    } else {
      LogWarn << "Ego Param Cast Error." << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr));
      return false;
    }
  } else {
    LogWarn << "Ego Initialize On Cloud Param Error." << TX_COND_NAME(_absAttrView, NonNull_Pointer(_absAttrView))
            << TX_COND_NAME(_sceneLoader, NonNull_Pointer(_sceneLoader))
            << TX_COND_NAME(_absAttrView_IsInited, CallSucc(_absAttrView->IsInited()));
    return false;
  }
  return true;
}
#endif /*ON_CLOUD*/

Base::txBool TAD_EgoVehicleElement::SyncPosition(const txFloat _passTime) TX_NOEXCEPT {
  const Base::txVec2 dir2d(GetHeadingWithAngle().Cos(), GetHeadingWithAngle().Sin());
  // 原始坐标系是后轴中心
  if (CallSucc(RawVehicleCoord_Is_RearAxleCenter())) {
    TX_MARK("given rear_axle_center, compute geom_center");
    mLocation.RearAxleCenter() = RawVehicleCoord();
    // 判断是否是leader
    if (CallSucc(IsLeader())) {
      const txFloat halfLen = 0.5 * GetLength();
      const txFloat rear_axle_to_rear = VehicleGeometory().vehicle_geometory().rear_axle_to_rear();
      const txFloat step = halfLen - rear_axle_to_rear;
      const Base::txVec2 step_vec = dir2d * step;
      // 设置位置信息
      mLocation.GeomCenter() = mLocation.RearAxleCenter();
      mLocation.GeomCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
      // 记录leader日志信息
      LOG_IF(INFO, FLAGS_LogLevel_Rear_Axle)
          << "[ego_leader][rear_axle]" << TX_VARS(halfLen) << TX_VARS(rear_axle_to_rear) << TX_VARS(step);
    } else {
      const txFloat halfLen = 0.5 * GetLength();
      const txFloat rear_axle_to_rear = VehicleGeometory().vehicle_geometory().rear_axle_to_rear();
      const txFloat step = halfLen - rear_axle_to_rear;
      const Base::txVec2 step_vec = dir2d * step;
      mLocation.GeomCenter() = mLocation.RearAxleCenter();
      mLocation.GeomCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
      // 记录follow日志信息
      LOG_IF(INFO, FLAGS_LogLevel_Rear_Axle)
          << "[ego_follow][rear_axle]" << TX_VARS(halfLen) << TX_VARS(rear_axle_to_rear) << TX_VARS(step);
    }
    // 原始坐标系是几何中心
  } else if (CallSucc(RawVehicleCoord_Is_GeomCenter())) {
    TX_MARK("given geom_center, compute rear_axle_center");
    mLocation.GeomCenter() = RawVehicleCoord();
    if (CallSucc(IsLeader())) {
      const txFloat halfLen = 0.5 * GetLength();
      const txFloat rear_axle_to_rear = VehicleGeometory().vehicle_geometory().rear_axle_to_rear();
      const txFloat step = halfLen - rear_axle_to_rear;
      const Base::txVec2 step_vec = dir2d * step * -1.0;
      // 将车辆后轴中心设置为车辆几何中心
      mLocation.RearAxleCenter() = mLocation.GeomCenter();
      mLocation.RearAxleCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
      // LOG(INFO) << "[ego_leader][rear_axle]" << TX_VARS(halfLen) << TX_VARS(rear_axle_to_rear) << TX_VARS(step *
      // -1.0);
    } else {
      // 计算车辆长度的一半
      const txFloat halfLen = 0.5 * GetLength();
      // 获取车辆后轴到车辆尾部的距离
      const txFloat rear_axle_to_rear = VehicleGeometory().vehicle_geometory().rear_axle_to_rear();
      // 计算车辆后轴到车辆中心的距离
      const txFloat step = halfLen - rear_axle_to_rear;
      // 计算车辆后轴到车辆中心的向量
      const Base::txVec2 step_vec = dir2d * step * -1.0;
      // 将车辆后轴中心设置为车辆几何中心
      mLocation.RearAxleCenter() = mLocation.GeomCenter();
      mLocation.RearAxleCenter().TranslateLocalPos(step_vec.x(), step_vec.y());
      // LOG(INFO) << "[ego_follow][rear_axle]" << TX_VARS(halfLen) << TX_VARS(rear_axle_to_rear) << TX_VARS(step *
      // -1.0);
    }
  } else {
    mLocation.RearAxleCenter() = RawVehicleCoord();
    mLocation.GeomCenter() = RawVehicleCoord();
    LogWarn << "unsupport Vehicle Coord Type.";
    return false;
  }
  return true;
}

Coord::txENU TAD_EgoVehicleElement::StableRawVehicleCoord() const TX_NOEXCEPT {
  // 如果原始车辆坐标是后轴中心
  if (CallSucc(RawVehicleCoord_Is_RearAxleCenter())) {
    // 返回稳定的后轴中心坐标
    return StableRearAxleCenter();
    // 如果原始车辆坐标是几何中心
  } else if (CallSucc(RawVehicleCoord_Is_GeomCenter())) {
    // 返回稳定的几何中心坐标
    return StableGeomCenter();
  } else {  // 如果原始车辆坐标既不是后轴中心也不是几何中心
    // 返回稳定的几何中心坐标
    return StableGeomCenter();
  }
}

void TAD_EgoVehicleElement::UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT {
  // 调用父类的 UpdateHashedLaneInfo 方法
  ParentClass::UpdateHashedLaneInfo(_location);
  // 如果启用了日志记录，输出车辆 ID 和当前哈希车道信息
  LOG_IF(INFO, FLAGS_LogLevel_IDM_Front) << "[cloud_debug][register][update]" << TX_VARS(Id())
                                         << TX_VARS(m_curHashedLaneInfo);
}

#if __FocusVehicle__
void TAD_EgoVehicleElement::ComputeFocusVehicles(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  // 定义关注车辆的过滤集合，只包含 TAD_Vehicle 类型
  static std::set<Base::Enums::ElementType> s_focus_vehicle_filter_set = {ElementType::TAD_Vehicle};
  const Base::txSysId pEgoSysId = SysId();
  // 获取车辆的几何中心坐标
  const Coord::txENU vVehiclePosEnu = StableGeomCenter();
  std::vector<Base::ITrafficElementPtr> resultElementPtrVec;
  // 在车辆几何中心周围查找关注车辆
  const Base::txBool bFind = Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInCircleByType(
      vVehiclePosEnu.ENU2D(), pEgoSysId, FLAGS_DETECT_FOCUS_OBJECT_DISTANCE, s_focus_vehicle_filter_set,
      resultElementPtrVec);
  // 遍历查找到的关注车辆
  for (auto focusElemPtr : resultElementPtrVec) {
    if (NonNull_Pointer(focusElemPtr)) {
      // 将当前车辆添加到关注车辆的关注车辆列表中
      focusElemPtr->AddFocusVehicle(pEgoSysId, GetBasePtr());
      LogInfo << "[ego_public_focus]" << TX_VARS(timeMgr.PassTime()) << TX_VARS(pEgoSysId)
              << TX_VARS_NAME(vehicleId, focusElemPtr->Id());
    }
  }
}
#endif  // __FocusVehicle__

void TAD_EgoVehicleElement::Search_Front_Rear_Vehicle_HLane(
    const Base::TimeParamManager& timeMgr, std::tuple<Base::txSysId, Base::txFloat>& res_front_info,
    std::tuple<Base::txSysId, Base::txFloat>& res_rear_info) const TX_NOEXCEPT {
  std::get<0>(res_front_info) = -1;
  std::get<1>(res_front_info) = -1;
  std::get<0>(res_rear_info) = -1;
  std::get<1>(res_rear_info) = -1;
  if (IsAlive() && timeMgr.PassTime() > 0.0) {
    using HashedLaneInfoOrthogonalList = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList;
    using HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr;
    const HashedLaneInfo& selfStableHashedLaneInfo = StableHashedLaneInfo();

    HashedLaneInfoOrthogonalListPtr center_node_ptr =
        HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(selfStableHashedLaneInfo);
    if (NonNull_Pointer(center_node_ptr)) {
#define _DistIdx_ (0)
#define _ElementIdx_ (1)
      {
        /*1. search front*/
        Base::txSurroundVehiclefo front_info;
        std::get<_DistIdx_>(front_info) = -1.0;
        HashedLaneInfoOrthogonalList::SearchNearestFrontElement(SysId(), Id(), GetLength(), selfStableHashedLaneInfo,
                                                                center_node_ptr, front_info,
                                                                FLAGS_ExternInfo_Search_Distance);

        // 如果不满足条件，设置结果后车信息的一致性 ID 和距离为 -1
        Base::SimulationConsistencyPtr front_veh_ptr = Weak2SharedPtr(std::get<_ElementIdx_>(front_info));
        // 如果前车距离小于等于 FLAGS_ExternInfo_Search_Distance 且大于 0.0，且前车指针非空
        if (std::get<_DistIdx_>(front_info) <= FLAGS_ExternInfo_Search_Distance &&
            std::get<_DistIdx_>(front_info) > 0.0 && NonNull_Pointer(front_veh_ptr)) {
          // 设置结果前车信息的一致性 ID 和距离
          std::get<0>(res_front_info) = front_veh_ptr->ConsistencyId();
          std::get<1>(res_front_info) = std::get<_DistIdx_>(front_info);
        } else {
          // 如果不满足条件，设置结果前车信息的一致性 ID 和距离为 -1
          std::get<0>(res_front_info) = -1;
          std::get<1>(res_front_info) = -1;
        }
      }
      {
        /*2. search rear*/
        Base::txSurroundVehiclefo rear_info;
        std::get<_DistIdx_>(rear_info) = -1.0;
        HashedLaneInfoOrthogonalList::SearchNearestRearElement(SysId(), Id(), GetLength(), selfStableHashedLaneInfo,
                                                               center_node_ptr, rear_info,
                                                               FLAGS_ExternInfo_Search_Distance);

        // 将后车信息的元素指针转换为共享指针
        Base::SimulationConsistencyPtr rear_veh_ptr = Weak2SharedPtr(std::get<_ElementIdx_>(rear_info));
        // 如果后车距离小于等于 FLAGS_ExternInfo_Search_Distance 且大于 0.0，且后车指针非空
        if (std::get<_DistIdx_>(rear_info) <= FLAGS_ExternInfo_Search_Distance &&
            std::get<_DistIdx_>(rear_info) > 0.0 && NonNull_Pointer(rear_veh_ptr)) {
          // 设置结果后车信息的一致性 ID 和距离
          std::get<0>(res_rear_info) = rear_veh_ptr->ConsistencyId();
          std::get<1>(res_rear_info) = std::get<_DistIdx_>(rear_info);
        } else {
          // 如果不满足条件，设置结果后车信息的一致性 ID 和距离为 -1
          std::get<0>(res_rear_info) = -1;
          std::get<1>(res_rear_info) = -1;
        }
      }
      LOG_IF(INFO, FLAGS_LogLevel_ExternInfo)
          << TX_VARS_NAME(FrontId, std::get<0>(res_front_info)) << TX_VARS_NAME(FrontV, std::get<1>(res_front_info))
          << TX_VARS_NAME(RearId, std::get<0>(res_rear_info)) << TX_VARS_NAME(RearV, std::get<1>(res_rear_info))
          << TX_VARS_NAME(Ego, selfStableHashedLaneInfo);

#undef _DistIdx_
#undef _ElementIdx_
    }
  }
}
#if __JunctionYield__
Base::txBool TAD_EgoVehicleElement::UpdateTrajectory(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  using namespace Geometry::Element;
  // 清除几何曲线
  ClearGeomCurve();
  // 如果车辆在车道连接线上
  if (IsOnLaneLink()) {
    // 获取车辆的几何中心坐标
    const Base::txVec2 vehiclecenter2d = GeomCenter().ToENU().ENU2D();
    // 计算车辆的半长车向量
    const Base::txVec2 vehicle_half_dir = Utils::Vec3_Vec2(GetLaneDir()) * (0.5 * GetLength());

    mGeomLongSkeletionLinePtr = std::make_shared<txLine>();
    mGeomLongSkeletionLinePtr->Initialize(Utils::Vec2_Vec3(vehiclecenter2d + vehicle_half_dir),
                                          Utils::Vec2_Vec3(vehiclecenter2d - vehicle_half_dir));
    // 创建输入参数向量
    std::vector<Base::txVec3> input_params;
    if (CallSucc(MakeJuntionTrajectory(timeMgr, input_params)) && input_params.size() > 1) {
      // 初始化几何轨迹
      txPolylinePtr polyline_ptr = std::make_shared<txPolyline>();
      polyline_ptr->Initialize(input_params);
      mGeomTrajPtr = polyline_ptr;
      return true;
    }
  }
  return false;
}

Base::txBool TAD_EgoVehicleElement::MakeJuntionTrajectory(Base::TimeParamManager const&,
                                                          std::vector<Base::txVec3>& samplingPtVec) TX_NOEXCEPT {
  // 初始化几何轨迹
  samplingPtVec.clear();
  // 如果车辆存活且在车道连接线上
  if (IsAlive() && IsOnLaneLink()) {
    // 获取车辆沿曲线的距离
    txFloat s = DistanceAlongCurve();
    /*s = (s < 0.0) ? (0.0) : (s);*/
    auto geom_ptr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(GetCurrentLaneInfo());
    // 如果几何形状指针非空
    if (NonNull_Pointer(geom_ptr)) {
      const txFloat curve_len = geom_ptr->GetLength();
      // 当距离小于曲线长度时，继续循环
      while (s < curve_len) {
        samplingPtVec.emplace_back(geom_ptr->GetLocalPos(s));
        s += 1.0;
      }
      return true;
    }
  }
  return false;
}
#endif /*__JunctionYield__*/

#if __TX_Mark__("evn perception sketch")

void TAD_EgoVehicleElement::Initialize_SUDOKU_GRID() TX_NOEXCEPT {
  // 初始化车辆扫描区域
  mVehicleScanRegion2D.Initialize(mGeometryData.Length(), mGeometryData.Width());
  // 初始化局部坐标系下的数独网格
  InitLocalCoord_SUDOKU_GRID(mGeometryData.Length(), mGeometryData.Width());
}

void TAD_EgoVehicleElement::Get_Region_Clockwise_Close_By_Enum(const Base::Enums::NearestElementDirection eValue,
                                                               std::vector<Base::txVec2>& retRegion) const TX_NOEXCEPT {
  mVehicleScanRegion2D.Get_Region_Clockwise_Close_By_Enum(eValue, retRegion);
}

void TAD_EgoVehicleElement::Compute_Region_Clockwise_Close(const Coord::txENU& vehPos, const Base::txVec3& laneDir,
                                                           const Unit::txDegree& headingDegree) TX_NOEXCEPT {
  // 获取车辆在车道上的角度
  const auto vehicleOnLaneDegree = Utils::GetLaneAngleFromVectorOnENU(laneDir);
  const Base::txFloat v_cos = vehicleOnLaneDegree.Cos();
  const Base::txFloat v_sin = vehicleOnLaneDegree.Sin();
  // 对车辆扫描区域进行刚体平移和旋转
  mVehicleScanRegion2D.Rigid_Translate_Rotation(vehPos, v_cos, v_sin, headingDegree.Cos(), headingDegree.Sin());
}

std::array<Base::txVec3, Base::Info_NearestObject::N_Dir> TAD_EgoVehicleElement::ComputeTargetPos(
    const Base::txVec3& vPos, const Base::txVec3& vehicleLaneDir) const TX_NOEXCEPT {
  std::array<Base::txVec3, Info_NearestObject::N_Dir> retArray;
  // 计算左右方向的单位向量
  const std::array<Base::txVec3, Base::Enums::szVehicleLaneChangeType> vLeftRightDir = {
      Utils::VetRotVecByDegree(vehicleLaneDir, Unit::txDegree::MakeDegree(LeftOnENU), Utils::Axis_Enu_Up()),
      Utils::VetRotVecByDegree(vehicleLaneDir, Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up())};

  // 计算目标临时位置
  const std::array<Base::txVec3, Base::Enums::szVehicleLaneChangeType> vTargetTempPos = {
      vPos + vLeftRightDir[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eLeft))] * 3.5,
      vPos + vLeftRightDir[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eRight))] * 3.5};

  // 返回目标位置数组
  return std::array<Base::txVec3, Info_NearestObject::N_Dir>{
      vPos,
      vPos,
      vPos,
      vTargetTempPos[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eLeft))],
      vTargetTempPos[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eLeft))],
      vPos,
      vTargetTempPos[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eRight))],
      vTargetTempPos[__enum2idx__(_plus_(Base::Enums::VehicleLaneChangeType::eRight))]};
}

void TAD_EgoVehicleElement::UpdateSketchSurroundingVehicle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
#  if USE_SUDOKU_GRID
  Compute_Region_Clockwise_Close(StableGeomCenter(), StableLaneDir(), GetHeadingWithAngle());
#  endif /*USE_SUDOKU_GRID*/

  using Info_NearestObject = Base::Info_NearestObject;
  Info_NearestObject& ret_NearestObject = SketchSurroundingInfo();
  ret_NearestObject = Base::Info_NearestObject(TX_MARK("must clear last frame result."));
  ret_NearestObject.setValid(true);

  const Base::txVec3& vehicleLaneDir = StableLaneDir();
  const Base::txSysId pVehicleSysId = SysId();
  const Base::txSysId pVehicleId = Id();
  const Base::txVec3& vPos = StableGeomCenter().ENU();
  const Base::txVec2 vPos2d = Utils::Vec3_Vec2(vPos);
  const Base::txFloat selfVehicleLength = GetLength();
  const Base::Info_Lane_t& selfLaneInfo = StableLaneInfo();

  {
    // 清空 SketchCircleElementPtrVec 容器
    GetSketchCircleElementPtrVec().clear();
    // 在圆形区域内查找指定类型的元素
    const Base::txBool bFind = Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInCircleByType(
        vPos2d, pVehicleSysId, FLAGS_DETECT_OBJECT_DISTANCE * 2.0,
        {Base::Enums::ElementType::TAD_Vehicle, Base::Enums::ElementType::TAD_Pedestrian,
         Base::Enums::ElementType::TAD_Immovability},
        GetSketchCircleElementPtrVec());
  }

  const std::array<Base::txVec3, Info_NearestObject::N_Dir>& vTargetPos = ComputeTargetPos(vPos, vehicleLaneDir);
  for (Base::txSize j_dir = 0; j_dir < Info_NearestObject::N_Dir; ++j_dir) {
    std::vector<Base::ITrafficElementPtr> resultElementPtrVec;
    // 在圆形区域内查找指定类型的元素
    resultElementPtrVec.reserve(20);
    const Base::txVec3& vTraget = vTargetPos[j_dir];
#  if USE_SUDOKU_GRID
    // 定义一个用于存储顺时针封闭区域的点的向量
    std::vector<Base::txVec2> localPts_clockwise_close;
    // 根据枚举值获取顺时针封闭区域的点
    Get_Region_Clockwise_Close_By_Enum(__idx2enum__(Base::Enums::NearestElementDirection, j_dir),
                                       localPts_clockwise_close);
    const Base::txBool bFind = Geometry::SpatialQuery::RTree2D::getInstance().FindElementsInAreaByType(
        pVehicleSysId, localPts_clockwise_close, {ElementType::TAD_Vehicle}, resultElementPtrVec);
#  endif /*USE_SUDOKU_GRID*/

    if (bFind) {
      for (auto resElemBasePtr : resultElementPtrVec) {
        Base::IVehicleElementPtr resElemPtr = std::dynamic_pointer_cast<Base::IVehicleElement>(resElemBasePtr);
        const Base::txVec3& otherVehicleLaneDir = resElemPtr->StableLaneDir();
        const Base::Info_Lane_t& otherLaneInfo = resElemPtr->StableLaneInfo();
        /*LOG_IF(WARNING, ((700070 == pVehicleId))) << TX_VARS(j_dir) <<
           Base::Enums::NearestElementDirection::_from_index(j_dir)._to_string()
                << TX_VARS_NAME(self_vehicle_id, pVehicleId)
                << TX_VARS_NAME(front_vehicle_id, resElemPtr->Id());*/
        // 如果车辆类型为 undefined
        if (CallSucc(_plus_(VEHICLE_TYPE::undefined) == resElemPtr->VehicleType())) {
          // 计算两个多边形之间的最小距离
          Base::txFloat curDistance = 0.0;
          if (Geometry::CD2D::GJK::HitPolygons(resElemPtr->GetPolygon(), GetPolygon())) {
            curDistance = 0.0;
          } else {
            Geometry::CD2D::cdVec2 nouse_intersectPt1, nouse_intersectPt2;
            curDistance = Geometry::CD2D::smallest_dist_between_ego_and_obsVehicle(
                resElemPtr->GetPolygon(), GetPolygon(), nouse_intersectPt1, nouse_intersectPt2);
          }

          // 如果当前距离小于目标方向的最近距离，则更新目标方向的最近距离和最近元素指针
          if (curDistance < ret_NearestObject.dist(j_dir)) {
            ret_NearestObject.dist(j_dir) = curDistance;
            ret_NearestObject.setElemPtr(j_dir, resElemPtr);
          }
          // 如果车辆方向与目标方向同步
        } else if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(vehicleLaneDir),
                                                  Utils::Vec3_Vec2(otherVehicleLaneDir)))) {
          // 如果目标方向为前方
          if (Info_NearestObject::nFront == j_dir) {
            // 计算当前距离
            const Base::txVec2 nearestElementCenter2d = resElemPtr->StableGeomCenter().ENU2D();
            const Base::txVec2 local_coord_other = RelativePosition(vPos2d, nearestElementCenter2d, TransMatInv());
            if (local_coord_other.y() > 0.0) {
              const Base::txFloat curGap = (resElemPtr->GetLength() + selfVehicleLength) * 0.5;
              const Base::txFloat curDistance =
                  ((Base::txVec2(vTraget.x(), vTraget.y())) - (nearestElementCenter2d)).norm() - curGap;
              /*LOG_IF(WARNING, ((1 == pVehicleId) || (17 == pVehicleId)) && 0 == j_dir)
                  << TX_VARS_NAME(self_vehicle_id, pVehicleId)
                  << TX_VARS_NAME(front_vehicle_id, resElemPtr->Id()) << TX_VARS(curDistance)
                  << TX_VARS_NAME(self_vehicle_sysid, pVehicleSysId)
                  << TX_VARS_NAME(front_vehicle_sysid, resElemPtr->SysId());*/
              // 如果当前距离小于目标方向的最近距离，则更新目标方向的最近距离和最近元素指针
              if (curDistance < ret_NearestObject.dist(j_dir)) {
                ret_NearestObject.dist(j_dir) = curDistance;
                ret_NearestObject.setElemPtr(j_dir, resElemPtr);
              }
#  if 0 /*20230516*/
            LogWarn << "[20230515]" << TX_VARS(Id()) << TX_VARS_NAME(vPos2d, Utils::ToString(vPos2d))
                    << TX_VARS(resElemPtr->Id())
                    << TX_VARS_NAME(nearestElementCenter2d, Utils::ToString(nearestElementCenter2d))
                    << TX_VARS_NAME(local_coord_other, Utils::ToString(local_coord_other));
#  endif
            }
            // 如果目标方向为后方
          } else if (Info_NearestObject::nBack == j_dir) {
            // 计算当前距离
            const Base::txVec2 nearestElementCenter2d = resElemPtr->StableGeomCenter().ENU2D();
            const Base::txVec2 local_coord_other = RelativePosition(vPos2d, nearestElementCenter2d, TransMatInv());
            if (local_coord_other.y() < 0.0) {
              const Base::txFloat curGap = (resElemPtr->GetLength() + selfVehicleLength) * 0.5;
              const Base::txFloat curDistance =
                  ((Base::txVec2(vTraget.x(), vTraget.y())) - (nearestElementCenter2d)).norm() - curGap;
              /*LOG_IF(WARNING, ((1 == pVehicleId) || (17 == pVehicleId)) && 0 == j_dir)
                  << TX_VARS_NAME(self_vehicle_id, pVehicleId)
                  << TX_VARS_NAME(front_vehicle_id, resElemPtr->Id()) << TX_VARS(curDistance)
                  << TX_VARS_NAME(self_vehicle_sysid, pVehicleSysId)
                  << TX_VARS_NAME(front_vehicle_sysid, resElemPtr->SysId());*/
              // 如果当前距离小于目标方向的最近距离，则更新目标方向的最近距离和最近元素指针
              if (curDistance < ret_NearestObject.dist(j_dir)) {
                ret_NearestObject.dist(j_dir) = curDistance;
                ret_NearestObject.setElemPtr(j_dir, resElemPtr);
              }
            }
          } else {
            const Base::txVec2 nearestElementCenter2d = resElemPtr->StableGeomCenter().ENU2D();
            const Base::txFloat curGap = (resElemPtr->GetLength() + selfVehicleLength) * 0.5;
            const Base::txFloat curDistance =
                ((Base::txVec2(vTraget.x(), vTraget.y())) - (nearestElementCenter2d)).norm() - curGap;
            /*LOG_IF(WARNING, ((1 == pVehicleId) || (17 == pVehicleId)) && 0 == j_dir)
                << TX_VARS_NAME(self_vehicle_id, pVehicleId)
                << TX_VARS_NAME(front_vehicle_id, resElemPtr->Id()) << TX_VARS(curDistance)
                << TX_VARS_NAME(self_vehicle_sysid, pVehicleSysId)
                << TX_VARS_NAME(front_vehicle_sysid, resElemPtr->SysId());*/
            // 如果当前距离小于目标方向的最近距离，则更新目标方向的最近距离和最近元素指针
            if (curDistance < ret_NearestObject.dist(j_dir)) {
              ret_NearestObject.dist(j_dir) = curDistance;
              ret_NearestObject.setElemPtr(j_dir, resElemPtr);
            }
          }
        }
      }
    } /*FindElementsInAreaByType*/
  }
}

void TAD_EgoVehicleElement::SnapshotHistoryTrajectory(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  // 初始化当前节点信息
  EgoRoadSTInfo curNode;
  curNode.passTime = timeMgr.PassTime();
  curNode.ego_pos = RawVehicleCoord();
  curNode.locInfo = mLocation.LaneLocInfo();
  curNode.velocity3d = Base::txVec3(m_egoData.velocity().x(), m_egoData.velocity().y(), m_egoData.velocity().z());
  curNode.heading.FromRadian(m_egoData.rpy().z());

  // 判断当前位置是否在车道上
  curNode.st_valid = false;
  Base::txFloat yaw = 0.0;
  if (curNode.locInfo.IsOnLane()) {
    // 获取road指针
    hadmap::txRoadPtr roadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(curNode.locInfo.onLaneUid.roadId);
    if (NonNull_Pointer(roadPtr) && NonNull_Pointer(roadPtr->getGeometry()) &&
        CallSucc(roadPtr->getGeometry()->xy2sl(curNode.ego_pos.Lon(), curNode.ego_pos.Lat(), curNode.st.x(),
                                               curNode.st.y(), yaw))) {
      curNode.st_valid = true;
    }
  } else {
    // 获取lanlink指针
    hadmap::txLaneLinkPtr linkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(curNode.locInfo.onLinkFromLaneUid,
                                                                                   curNode.locInfo.onLinkToLaneUid);
    if (NonNull_Pointer(linkPtr) && NonNull_Pointer(linkPtr->getGeometry()) &&
        CallSucc(linkPtr->getGeometry()->xy2sl(curNode.ego_pos.Lon(), curNode.ego_pos.Lat(), curNode.st.x(),
                                               curNode.st.y(), yaw))) {
      curNode.st_valid = true;
    }
  }
  // 将当前节点信息添加到历史轨迹信息中
  m_history_trajectory_info.emplace_back(curNode);
  // 检查车辆是否发生了车道变换
  CheckEgoLaneChange(timeMgr, m_history_trajectory_info);
}

void TAD_EgoVehicleElement::CheckEgoLaneChange(const Base::TimeParamManager& timeMgr,
                                               const EgoRoadSTInfoVec& traj_info) TX_NOEXCEPT {
  m_ego_LaneChangeState = _plus_(VehicleMoveLaneState::eStraight);
  // 如果历史轨迹信息非空且最后一个节点有效
  if (_NonEmpty_(traj_info) && traj_info.back().IsValid()) {
    // 初始化有效轨迹信息
    TAD_EgoVehicleElement::EgoRoadSTInfoVec valid_traj;
    valid_traj.reserve(FLAGS_ego_cutin_history_traj_size);
    for (auto itr = std::rbegin(traj_info);
         (itr != std::rend(traj_info)) && (valid_traj.size() < FLAGS_ego_cutin_history_traj_size); ++itr) {
      if ((*itr).IsValid(traj_info.back().locInfo)) {
        valid_traj.insert(valid_traj.begin(), *itr);
      } else {
        break;
      }
    }

    // 如果有效轨迹信息数量大于10
    if (valid_traj.size() > 10) {
      Base::txInt score = 0;
      // 遍历有效轨迹信息，计算车辆在车道上的位置变化，并更新车辆的车道变换得分
      for (Base::txSize idx = 1; idx < valid_traj.size(); ++idx) {
        // 计算车辆在车道上的位置变化，并根据位置变化更新车辆的车道变换得分
        const Base::txFloat diff = valid_traj[idx].st.y() - valid_traj[idx - 1].st.y();
        if (CallFail(Math::isZero(diff, 0.05))) {
          if (diff > 0.0) {
            score += 1;
          } else if (diff < 0.0) {
            score -= 1;
          }
        }
      }
      const Base::txInt abs_score = std::abs(score);
      // 如果车辆在车道上的位置变化大于一定阈值，则判断车辆是否发生了车道变换
      if (abs_score > (0.6 * valid_traj.size())) {
        // 如果位置变化为正，则车辆向右变道
        if (score > 0) {
          m_ego_LaneChangeState = _plus_(VehicleMoveLaneState::eRight);
        } else if (score < 0) {
          m_ego_LaneChangeState = _plus_(VehicleMoveLaneState::eLeft);
        }
      }
    } else {
      LogWarn << "[ego_lanechange_check][failure]" << TX_VARS_NAME(PassTime, timeMgr.PassTime())
              << TX_VARS(valid_traj.size());
    }
  } else {
    LogWarn << "[ego_lanechange_check][failure] traj_info error" << TX_VARS_NAME(PassTime, timeMgr.PassTime());
  }
}

#endif /*"evn perception sketch"*/
TX_NAMESPACE_CLOSE(TrafficFlow)
#undef LogInfo
#undef LogWarn
