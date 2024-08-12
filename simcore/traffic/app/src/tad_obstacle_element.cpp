// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_obstacle_element.h"
#include "tx_frame_utils.h"
#include "tx_protobuf_utils.h"
#include "tx_scene_loader.h"
#include "tx_units.h"
#if USE_RTree
#  include "tx_obb.h"
#  include "tx_spatial_query.h"
#endif
#include "HdMap/tx_hashed_road.h"
#include "tx_geometry.h"
#include "tx_hadmap_utils.h"
#include "tx_tadsim_flags.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Obstacle) << _StreamPrecision_
#define LogWarn LOG(WARNING) << _StreamPrecision_
TX_NAMESPACE_OPEN(TrafficFlow)

TAD_ObstacleElement::~TAD_ObstacleElement() { /*LOG(WARNING) << "[~]TAD_ObstacleElement";*/
}

// 设置障碍物元素的旋转角度
void TAD_ObstacleElement::SetRot(Base::ISceneLoader::IObstacleViewerPtr _view_ptr) TX_NOEXCEPT {
  // 如果观察者指针有效
  if (NonNull_Pointer(_view_ptr)) {
    // 如果起始角度小于0
    if ((_view_ptr->start_angle()) < 0.0) {
      // 使用显示方向设置旋转角度
      m_rot.FromDegree(mLocation.rot_for_display().GetDegree() + _view_ptr->direction());
      LogInfo << "[using_direction] " << TX_VARS(_view_ptr->start_angle());
    } else {
      // 使用起始角度设置旋转角度
      m_rot.FromDegree(_view_ptr->start_angle());
      LogInfo << "[using_start_angle] " << TX_VARS(_view_ptr->start_angle());
    }
  }
}

Base::txBool TAD_ObstacleElement::Initialize(Base::ISceneLoader::IViewerPtr _absAttrView,
                                             Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 判断输入的参数指针有效
  if (NonNull_Pointer(_absAttrView) && NonNull_Pointer(_sceneLoader) && CallSucc(_absAttrView->IsInited())) {
    // 设置元素来源
    m_vehicle_come_from = Base::IVehicleElement::VehicleSource::scene_cfg;
    _elemAttrViewPtr = std::dynamic_pointer_cast<Base::ISceneLoader::IObstacleViewer>(_absAttrView);

    // 元素属性视图不为空
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      // 处理元素的路线数据
      _elemRouteViewPtr = _sceneLoader->GetRouteData(_elemAttrViewPtr->routeID());
      if (Null_Pointer(_elemRouteViewPtr)) {
        LogWarn << "Obstacle Route error.";
        return false;
      }
      auto refObstaclePtr = _elemAttrViewPtr;
      auto refRoutePtr = _elemRouteViewPtr;
      if (true /* Math::isNotZero(refRoutePtr->startLon()) && Math::isNotZero(refRoutePtr->startLat())*/) {
        // 初始化障碍物元素的起始点
        RawVehicleCoord().FromWGS84(refRoutePtr->startLon(), refRoutePtr->startLat(), FLAGS_default_altitude);
        hadmap::txLanePtr initLane_roadId_sectionId = HdMap::HadmapCacheConCurrent::GetLaneForInit(
            RawVehicleCoord().Lon(), RawVehicleCoord().Lat(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset());

        // 设置元素的身份属性
        mObstacleType = __lpsz2enum__(STATIC_ELEMENT_TYPE, (refObstaclePtr->type()).c_str());
#if USE_CustomModelImport
        mObstacleTypeStr = refObstaclePtr->type().c_str();
        mObstacleTypeId = Base::CatalogCache::OBSTACLE_TYPE(mObstacleTypeStr);
#endif /*#USE_CustomModelImport*/
        mIdentity.Id() = refObstaclePtr->id();
        mIdentity.SysId() = CreateSysId(mIdentity.Id());
#if 1
#  if USE_CustomModelImport
        if (CallFail(InitObstacleCatalog(mObstacleTypeStr))) {
#  else  /*#USE_CustomModelImport*/
        if (CallFail(InitCatalog(mObstacleType))) {
#  endif /*#USE_CustomModelImport*/
          // 初始化catalog失败情况
          mGeometryData.Length() = refObstaclePtr->length();
          mGeometryData.Width() = refObstaclePtr->width();
          mGeometryData.Height() = refObstaclePtr->height();
          LogWarn << "[Catalog_query][failure][obs] " << TX_VARS(Id())
                  << TX_VARS_NAME(ObstacleType, (refObstaclePtr->type())) << TX_VARS(GetLength()) << TX_VARS(GetWidth())
                  << TX_VARS(GetHeigth());
        } else {
          // 使用catalog初始化
          mGeometryData.Length() = Catalog_Length();
          mGeometryData.Width() = Catalog_Width();
          mGeometryData.Height() = Catalog_Height();
          LOG_IF(INFO, FLAGS_LogLevel_Catalog) << "[Catalog_query][success][obs] " << TX_VARS(Id())
                                               << TX_VARS_NAME(ObstacleType, (refObstaclePtr->type()))
                                               << TX_VARS(GetLength()) << TX_VARS(GetWidth()) << TX_VARS(GetHeigth());
        }
#endif

        // 初始化道路，路段相关
        if (NonNull_Pointer(initLane_roadId_sectionId)) {
          LogInfo << "initLaneUid = " << Utils::ToString(initLane_roadId_sectionId->getTxLaneId())
                  << TX_VARS(mLocation.DistanceAlongCurve()) << TX_VARS(mLocation.LaneOffset());

          // 获取目标车道uid
          const Base::txLaneUId destLaneUid(
              (initLane_roadId_sectionId->getRoadId()), (initLane_roadId_sectionId->getSectionId()),
              ((refObstaclePtr->laneID() < 0) ? (refObstaclePtr->laneID())
                                              : (initLane_roadId_sectionId->getTxLaneId().laneId)));
          hadmap::txLanePtr initLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(destLaneUid);
          // 初始化车道有效
          if (NonNull_Pointer(initLane)) {
            // 初始化障碍物元素在车道上具体位置
            if (HdMap::HadmapCacheConCurrent::GetLonLatFromSTonLaneWithOffset(
                    initLane->getTxLaneId(), GenerateST(refObstaclePtr->start_s(), mLocation.DistanceAlongCurve()),
                    refObstaclePtr->l_offset(), RawVehicleCoord())) {
              // 设置障碍物沿着车道的距离
              mLocation.DistanceAlongCurve(/*bug ID101582635*/) = refObstaclePtr->start_s();
              mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eObstacle;
              // 重定位tracker
              mLocation.InitTracker(mIdentity.Id());
              RelocateTracker(initLane, 0.0);

              // 设置元素位置的朝向角度
              mLocation.vLaneDir() =
                  HdMap::HadmapCacheConCurrent::GetLaneDir(initLane, GenerateST(refObstaclePtr->start_s(), 0.0));
              SetRot(_elemAttrViewPtr);
              SyncPosition(0.0);
              const Base::txVec3 frontAxis3d(mLocation.vLaneDir().x(), mLocation.vLaneDir().y(), 0.0);
              // 计算元素几何信息
              mGeometryData.ComputePolygon(GeomCenter().ToENU(), frontAxis3d);
              SubdivideGeom();
              FillingSpatialQuery();
              // 注册位置信息
              RegisterInfoOnInit(mLocation);
              LogInfo << "Obstacle Init Success. " << Str() << TX_VARS(Id()) << TX_VARS(SysId());
              // LOG(WARNING) << TX_VARS(Id()) << TX_VARS(VehicleCoordStr());
              return true;
            } else {
              LOG(WARNING) << "GetLonLatFromSTonLane Failure.";
              return false;
            }
          } else {
            LOG(WARNING) << "GetLane Failure." << Utils::ToString(destLaneUid);
            return false;
          }
        } else {  // 道路-路段无效
          hadmap::txLaneLinkPtr initLaneLinkptr = HdMap::HadmapCacheConCurrent::GetLaneLinkForInit(
              RawVehicleCoord().Lon(), RawVehicleCoord().Lat(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset());
          // 当前在lanelink中
          if (NonNull_Pointer(initLaneLinkptr)) {
            // 可以获取元素在lanlink上位置
            if (HdMap::HadmapCacheConCurrent::Get_LAL_LaneLink_By_S(
                    Base::Info_Lane_t(initLaneLinkptr->getId(), initLaneLinkptr->fromTxLaneId(),
                                      initLaneLinkptr->toTxLaneId()),
                    refObstaclePtr->start_s(), RawVehicleCoord())) {
              // 设置元素location属性
              mLocation.DistanceAlongCurve(/*bug ID101582635*/) = refObstaclePtr->start_s();
              mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eObstacle;
              // 初始化tracker
              mLocation.InitTracker(mIdentity.Id());
              RelocateTracker(initLaneLinkptr, 0.0);

              // 初始化元素朝向角度等
              mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneLinkDir(
                  initLaneLinkptr, GenerateST(refObstaclePtr->start_s(), 0.0));
              SetRot(refObstaclePtr);
              SyncPosition(0.0);
              const Base::txVec3 frontAxis3d(mLocation.vLaneDir().x(), mLocation.vLaneDir().y(), 0.0);
              // 计算元素几何信息
              mGeometryData.ComputePolygon(GeomCenter().ToENU(), frontAxis3d);
              SubdivideGeom();
              // 更新空间查询
              FillingSpatialQuery();
              // 注册位置信息
              RegisterInfoOnInit(mLocation);
              LogInfo << "Obstacle Init Success. " << Str() << TX_VARS(Id()) << TX_VARS(SysId());
              LOG(WARNING) << TX_VARS(Id()) << TX_VARS(VehicleCoordStr());
              return true;
            } else {
              LOG(WARNING) << "GetLonLatFromSTonLane Failure.";
              return false;
            }
          } else {
            LOG(WARNING) << "GetLane Failure." << RawVehicleCoord();
            return false;
          }
          LOG(WARNING) << "GetLane Failure." << RawVehicleCoord();
          return false;
        }
      } else {
        LogWarn << "Lon Lat Error. lon = " << (refRoutePtr->startLon()) << ", lat = " << (refRoutePtr->startLat());
        return false;
      }
    } else {
      LogWarn << "Param Cast Error." << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr))
              << TX_COND_NAME(_elemRouteViewPtr, NonNull_Pointer(_elemRouteViewPtr));
      return false;
    }
  } else {
    LogWarn << "Param Error." << TX_COND_NAME(_absAttrView, NonNull_Pointer(_absAttrView))
            << TX_COND_NAME(_sceneLoader, NonNull_Pointer(_sceneLoader))
            << TX_COND_NAME(_absAttrView_IsInited, CallSucc(_absAttrView->IsInited()));
    return false;
  }
}

void TAD_ObstacleElement::UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT {
#if __ClearHashLanePerStep__
  // 获取当前的哈说lane信息
  m_curHashedLaneInfo = _location.GetHashedLaneInfo(Id());
  // 更新最新信息
  HdMap::HashedRoadCacheConCurrent::RegisterVehicle(m_curHashedLaneInfo, GetVehicleElementPtr());
#endif /*__ClearHashLanePerStep__*/
}

Base::txBool TAD_ObstacleElement::FillingSpatialQuery() TX_NOEXCEPT {
  // 元素活跃
  if (IsAlive()) {
    // 判断多边数量
    const auto& refPolygon = mGeometryData.Polygon();
    if (Geometry::OBB2D::PolygonSize == refPolygon.size()) {
      // 更新hashlaneinfo
      UpdateHashedLaneInfo(mLocation);
#if 1
      // 插入多边形顶点道空间查询树
      Geometry::SpatialQuery::RTree2D::InsertBulk(m_vec_subdivide_vertex, GetBasePtr());
#else
      /*LOG(INFO) << "[obs] " << mGeometryData.Str();*/
      Geometry::SpatialQuery::RTree2D::getInstance().Insert(refPolygon[0], refPolygon[1], refPolygon[2], refPolygon[3],
                                                            GeomCenter().ToENU().ENU2D(), GetBasePtr());
#endif
      return true;
    } else {
      LOG(WARNING) << "m_vecPolygon.size = " << refPolygon.size();
      return false;
    }
  } else {
    return false;
  }
}

TAD_ObstacleElement::KineticsInfo_t TAD_ObstacleElement::GetKineticsInfo(Base::TimeParamManager const& timeMgr) const
    TX_NOEXCEPT {
  // 创建KineticsInfo_t实例返回
  KineticsInfo_t retInfo;
  retInfo.m_elemId = Id();
  retInfo.m_compute_velocity = GetVelocity();
  retInfo.m_acc = GetAcc();
  retInfo.m_show_velocity = GetShowVelocity();
  return retInfo;
}

Base::txBool TAD_ObstacleElement::Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT { return true; }

Base::txBool TAD_ObstacleElement::Release() TX_NOEXCEPT { return true; }

Base::txBool TAD_ObstacleElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                                 sim_msg::Traffic& trafficData) TX_NOEXCEPT {
  // 元素活跃
  if (IsAlive()) {
#if 1
    sim_msg::StaticObstacle* tmpObstacleData = trafficData.mutable_staticobstacles(IndexInPb());
#else
    sim_msg::StaticObstacle* tmpObstacleData = nullptr;
    {
#  if USE_TBB
      tbb::mutex::scoped_lock lock(s_tbbMutex_outputTraffic);
#  endif
      tmpObstacleData = trafficData.add_staticobstacles();
    }
#endif
    // 设置需要输出的各个属性值
    tmpObstacleData->set_t(timeMgr.TimeStamp());
    tmpObstacleData->set_id(Id());
#if USE_CustomModelImport
    tmpObstacleData->set_type(mObstacleTypeId);
#else  /*#USE_CustomModelImport*/
    tmpObstacleData->set_type(__enum2int__(STATIC_ELEMENT_TYPE, mObstacleType));
#endif /*#USE_CustomModelImport*/
    const hadmap::txPoint geom_center_gps = RawVehicleCoord().WGS84();
    tmpObstacleData->set_x(__Lon__(geom_center_gps));
    tmpObstacleData->set_y(__Lat__(geom_center_gps));
    tmpObstacleData->set_z(Altitude() /*__Alt__(geom_center_gps)*/);
    tmpObstacleData->set_length(GetLength());
    tmpObstacleData->set_width(GetWidth());
    tmpObstacleData->set_height(GetHeigth());
    tmpObstacleData->set_heading(m_rot.GetRadian());

    /*FillingSpatialQuery();*/
  }
  return true;
}

Base::txBool TAD_ObstacleElement::IsTransparentObject() const TX_NOEXCEPT {
#if USE_CustomModelImport
  return IsTransparentObject(mObstacleTypeStr);
#else  /*#USE_CustomModelImport*/
  return IsTransparentObject(mObstacleType);
#endif /*#USE_CustomModelImport*/
}

Base::txBool TAD_ObstacleElement::IsTransparentObject(const Base::Enums::STATIC_ELEMENT_TYPE& _obs_type) TX_NOEXCEPT {
  return (_plus_(Base::Enums::STATIC_ELEMENT_TYPE::Port_Crane_001) == _obs_type) ||
         (_plus_(Base::Enums::STATIC_ELEMENT_TYPE::Port_Crane_002) == _obs_type);
}

Base::txBool TAD_ObstacleElement::IsTransparentObject(const Base::txString& _obs_type) TX_NOEXCEPT {
  return ("Port_Crane_001" == _obs_type) || ("Port_Crane_002" == _obs_type);
}

void TAD_ObstacleElement::SubdivideGeom() TX_NOEXCEPT {
  m_vec_subdivide_vertex.clear();
  const auto& refPolygon = mGeometryData.Polygon();
  if (Geometry::OBB2D::PolygonSize == refPolygon.size()) {
    // 如果障碍物元素是透明对象
    if (CallSucc(IsTransparentObject())) {
      TX_MARK("Not involved in environmental perception");
    } else if (GetLength() <= FLAGS_TAD_Obs_Subdivide_Threshold || GetWidth() <= FLAGS_TAD_Obs_Subdivide_Threshold) {
      // 如果障碍物元素的长度或宽度小于等于阈值
      // 将多边形的四个顶点和几何中心添加到子分割顶点向量中
      m_vec_subdivide_vertex.emplace_back(refPolygon[0]);
      m_vec_subdivide_vertex.emplace_back(refPolygon[1]);
      m_vec_subdivide_vertex.emplace_back(refPolygon[2]);
      m_vec_subdivide_vertex.emplace_back(refPolygon[3]);
      m_vec_subdivide_vertex.emplace_back(GeomCenter().ToENU().ENU2D());
    } else {
      // 计算多边形的四个顶点在地图坐标系下的坐标
      const Base::txVec2 p0 = Geometry::RigidTransformation(Coord::txENU(Utils::Vec2_Vec3(refPolygon[0])), m_rot,
                                                            Base::txVec3(0.0, 0.0, 0.0))
                                  .ENU2D(); /*(-1, -1)*/
      const Base::txVec2 p1 = Geometry::RigidTransformation(Coord::txENU(Utils::Vec2_Vec3(refPolygon[1])), m_rot,
                                                            Base::txVec3(0.0, 0.0, 0.0))
                                  .ENU2D(); /*( 1, -1)*/
      const Base::txVec2 p2 = Geometry::RigidTransformation(Coord::txENU(Utils::Vec2_Vec3(refPolygon[2])), m_rot,
                                                            Base::txVec3(0.0, 0.0, 0.0))
                                  .ENU2D(); /*( 1,  1)*/
      const Base::txVec2 p3 = Geometry::RigidTransformation(Coord::txENU(Utils::Vec2_Vec3(refPolygon[3])), m_rot,
                                                            Base::txVec3(0.0, 0.0, 0.0))
                                  .ENU2D(); /*(-1,  1)*/

      // 计算u轴和v轴的向量
      Base::txVec2 u_axis = p1 - p0;
      Base::txVec2 v_axis = p3 - p0;
      // 计算u轴和v轴的长度
      Base::txVec2 u_dir = u_axis;
      u_dir.normalize();
      Base::txVec2 v_dir = v_axis;
      v_dir.normalize();
      const Base::txFloat u_len = u_axis.norm(); /*same as length*/
      const Base::txFloat v_len = v_axis.norm(); /*same as width*/
                                                 // 初始化u轴和v轴的距离
      Base::txFloat u_dist = 0.0;
      Base::txFloat v_dist = 0.0;
      // 遍历u轴和v轴的距离，每次增加2.0
      while (u_dist <= u_len) {
        while (v_dist <= v_len) {
          // 计算新的采样点坐标
          const Base::txVec2 new_sample_pt = u_dist * u_dir + v_dist * v_dir;
          // 将新的采样点添加到子分割顶点向量中
          m_vec_subdivide_vertex.emplace_back(new_sample_pt);
          v_dist += 2.0;
        }
        u_dist += 2.0;
      }
    }
  }
}

#if __TX_Mark__("TAD_VertexElement")
#  define VertexInfo LOG_IF(INFO, FLAGS_LogLevel_DebugVertex)

Base::txBool TAD_VertexElement::Initialize(const Base::txSysId _id, const Base::txFloat _lon,
                                           const Base::txFloat _lat) TX_NOEXCEPT {
  if (true /*Math::isNotZero(_lon) && Math::isNotZero(_lat)*/) {
    // 初始化元素的几何信息
    m_vehicle_come_from = Base::IVehicleElement::VehicleSource::scene_cfg;
    RawVehicleCoord().FromWGS84(_lon, _lat, FLAGS_default_altitude);
    mObstacleType = _plus_(Base::Enums::STATIC_ELEMENT_TYPE::Box);
    mObstacleTypeStr = "Box";
    mObstacleTypeId = 1;

    mIdentity.Id() = _id;

    // 初始化长宽高默认为1.0
    mGeometryData.Length() = 1.0;
    mGeometryData.Width() = 1.0;
    mGeometryData.Height() = 1.0;
    VertexInfo << "[Vertex] " << TX_VARS(Id()) << TX_VARS_NAME(loc, RawVehicleCoord().StrWGS84());
    mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eObstacle;
    return true;
  } else {
    LogWarn << "TAD_VertexElement::Initialize failure." << TX_VARS(_id) << TX_VARS(_lon) << TX_VARS(_lat);
    return false;
  }
}

Base::txBool TAD_VertexElement::Initialize(Base::ISceneLoader::IViewerPtr _absAttrView,
                                           Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 参数检查
  if (NonNull_Pointer(_absAttrView) && NonNull_Pointer(_sceneLoader) && CallSucc(_absAttrView->IsInited())) {
    m_vehicle_come_from = Base::IVehicleElement::VehicleSource::scene_cfg;
    _elemAttrViewPtr = std::dynamic_pointer_cast<Base::ISceneLoader::IObstacleViewer>(_absAttrView);

    // 元素属性视图指针不为空
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      Base::txFloat lon = 0.0;
      Base::txFloat lat = 0.0;
      const auto rId = _elemAttrViewPtr->routeID();
      if (rId > 0.0) {
        // 获取元素路线视图
        _elemRouteViewPtr = _sceneLoader->GetRouteData(rId);
        if (Null_Pointer(_elemRouteViewPtr)) {
          LogWarn << "Obstacle Route error." << TX_VARS(rId);
          return false;
        } else {
          // 位置初始化
          lon = _elemRouteViewPtr->startLon();
          lat = _elemRouteViewPtr->startLat();
        }
      } else {
        auto location_ptr = _sceneLoader->GetLocationData(rId * -1);
        if (Null_Pointer(location_ptr)) {
          LogWarn << "Obstacle Location error." << TX_VARS_NAME(lId, rId * -1);
          return false;
        } else {
          // 位置初始化
          lon = __Lon__(location_ptr->PosGPS());
          lat = __Lat__(location_ptr->PosGPS());
        }
      }
      auto refObstaclePtr = _elemAttrViewPtr;
      if (true /*Math::isNotZero(lon) && Math::isNotZero(lat)*/) {
        // 初始化起点
        RawVehicleCoord().FromWGS84(lon, lat, FLAGS_default_altitude);
        mObstacleType = _plus_(Base::Enums::STATIC_ELEMENT_TYPE::Box);
        mObstacleTypeStr = "Box";
        mObstacleTypeId = 1;
        mIdentity.Id() = refObstaclePtr->id();

        // 初始化几何信息
        mGeometryData.Length() = refObstaclePtr->length();
        mGeometryData.Width() = refObstaclePtr->width();
        mGeometryData.Height() = refObstaclePtr->height();
        VertexInfo << "[Vertex] " << TX_VARS(Id()) << TX_VARS_NAME(loc, RawVehicleCoord().StrWGS84());
        mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eObstacle;
        return true;
      } else {
        LogWarn << "Lon Lat Error. lon = " << (lon) << ", lat = " << (lat);
        return false;
      }
    } else {
      LogWarn << "Param Cast Error." << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr))
              << TX_COND_NAME(_elemRouteViewPtr, NonNull_Pointer(_elemRouteViewPtr));
      return false;
    }
  } else {
    LogWarn << "Param Error." << TX_COND_NAME(_absAttrView, NonNull_Pointer(_absAttrView))
            << TX_COND_NAME(_sceneLoader, NonNull_Pointer(_sceneLoader))
            << TX_COND_NAME(_absAttrView_IsInited, CallSucc(_absAttrView->IsInited()));
    return false;
  }
}

Base::txBool TAD_VertexElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                               sim_msg::Traffic& trafficData) TX_NOEXCEPT {
  // 检查元素活跃
  if (IsAlive()) {
    sim_msg::StaticObstacle* tmpObstacleData = nullptr;
    {
#  if USE_TBB
      tbb::mutex::scoped_lock lock(s_tbbMutex_outputTraffic);
#  endif
      tmpObstacleData = trafficData.add_staticobstacles();
    }
    // 向Traffic数据中添加需要输出的数据
    tmpObstacleData->set_t(timeMgr.TimeStamp());
    tmpObstacleData->set_id(Id());
#  if USE_CustomModelImport
    tmpObstacleData->set_type(mObstacleTypeId);
#  else  /*#USE_CustomModelImport*/
    tmpObstacleData->set_type(__enum2int__(STATIC_ELEMENT_TYPE, mObstacleType));
#  endif /*#USE_CustomModelImport*/
    const hadmap::txPoint geom_center_gps = RawVehicleCoord().WGS84();
    tmpObstacleData->set_x(__Lon__(geom_center_gps));
    tmpObstacleData->set_y(__Lat__(geom_center_gps));
    tmpObstacleData->set_z(Altitude() /*__Alt__(geom_center_gps)*/);
    tmpObstacleData->set_length(GetLength());
    tmpObstacleData->set_width(GetWidth());
    tmpObstacleData->set_height(GetHeigth());
    tmpObstacleData->set_heading(0.0);

    /*FillingSpatialQuery();*/
  }
  return true;
}
#  undef VertexInfo
#endif /*TAD_VertexElement*/

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
