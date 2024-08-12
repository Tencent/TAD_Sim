// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_map_obstacle_element.h"
#include <set>
#include "tx_tadsim_flags.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_MapObject) << _StreamPrecision_
#define LogWarn LOG(WARNING) << _StreamPrecision_

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool TAD_MapObstacleElement::IsSupportMapObjectType(const Base::txString strTypeName) const TX_NOEXCEPT {
  static std::set<Base::txString> s_supportTypeNameSet = {"J_001", "J_002", "J_003", "J_004", "J_005",
                                                          "J_006", "J_007", "J_008", "J_009", "J_010",
                                                          "J_011", "J_012", "J_013", "J_014", "J_015"};
  // Check if the object type is supported.
  return _Contain_(s_supportTypeNameSet, strTypeName);
}

Base::txBool TAD_MapObstacleElement::Initialize_txObject(hadmap::txObjectPtr obj_ptr) TX_NOEXCEPT {
  if (NonNull_Pointer(obj_ptr) && IsSupportMapObjectType(obj_ptr->getName())) {
    // 设置地图障碍物元素的名称
    m_obj_name = obj_ptr->getName();
    m_raw_tx_object_ptr = obj_ptr;

    // 设置车辆来源为高精度地图
    m_vehicle_come_from = Base::IVehicleElement::VehicleSource::hadmap;
    _elemAttrViewPtr = nullptr;
    _elemRouteViewPtr = nullptr;

    // 设置身份ID
    mIdentity.Id() = obj_ptr->getId();
    // 设置系统ID
    mIdentity.SysId() = CreateSysId(mIdentity.Id());
    // 获取原始位置
    const hadmap::txPoint raw_pos = obj_ptr->getPos();

    // 设置障碍物类型
    mObstacleType = __lpsz2enum__(STATIC_ELEMENT_TYPE, (obj_ptr->getName()).c_str());
#if USE_CustomModelImport
    mObstacleTypeStr = obj_ptr->getName().c_str();
    mObstacleTypeId = Base::CatalogCache::OBSTACLE_TYPE(mObstacleTypeStr);
#endif /*#USE_CustomModelImport*/
    LogInfo << TX_VARS_NAME(MapObjectId, Id()) << TX_VARS_NAME(ObjPos, Utils::ToString(raw_pos))
            << TX_VARS_NAME(MapObjectType, __enum2lpsz__(STATIC_ELEMENT_TYPE, mObstacleType));

    // 将原始位置从WGS84坐标系转换为车辆坐标系
    RawVehicleCoord().FromWGS84(raw_pos);
    obj_ptr->getLWH(mGeometryData.Length(), mGeometryData.Width(), mGeometryData.Height());
    // 获取初始车道
    hadmap::txLanePtr initLane = HdMap::HadmapCacheConCurrent::GetLaneForInit(
        RawVehicleCoord().Lon(), RawVehicleCoord().Lat(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset());
    // 如果初始车道有效
    if (NonNull_Pointer(initLane)) {
      LogInfo << "initLaneUid = " << Utils::ToString(initLane->getTxLaneId()) << TX_VARS(mLocation.DistanceAlongCurve())
              << TX_VARS(mLocation.LaneOffset());

      // 设置几何数据对象类型为障碍物
      mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eObstacle;
      mLocation.InitTracker(mIdentity.Id());
      RelocateTracker(initLane, 0.0);
      /*mObstacleType = refObstaclePtr->type();*/
      // 获取障碍物的旋转角度
      Base::txFloat r, p, y;
      obj_ptr->getRPY(r, p, y);
      mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneDir(initLane, mLocation.DistanceAlongCurve());
      m_rot.FromRadian(y);
      // 同步位置
      SyncPosition(0.0);
      // 计算多边形
      const Base::txVec3 frontAxis3d(mLocation.vLaneDir().x(), mLocation.vLaneDir().y(), 0.0);
      mGeometryData.ComputePolygon(GeomCenter().ToENU(), frontAxis3d);
      // 填充空间查询数据
      FillingSpatialQuery();
      // 注册初始化信息
      RegisterInfoOnInit(mLocation);
      LogInfo << "Obstacle Init Success. " << Str() << TX_VARS(Id()) << TX_VARS(SysId()) << TX_VARS(VehicleCoordStr());
      return true;
    } else {
      hadmap::txLaneLinkPtr initLaneLinkptr = HdMap::HadmapCacheConCurrent::GetLaneLinkForInit(
          RawVehicleCoord().Lon(), RawVehicleCoord().Lat(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset());
      if (NonNull_Pointer(initLaneLinkptr)) {
        Base::Info_Lane_t linkUid(initLaneLinkptr->getId(), initLaneLinkptr->fromTxLaneId(),
                                  initLaneLinkptr->toTxLaneId());
        LogInfo << "initLaneLinkptr = " << linkUid << TX_VARS(mLocation.DistanceAlongCurve())
                << TX_VARS(mLocation.LaneOffset());

        mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eObstacle;
        mLocation.InitTracker(mIdentity.Id());
        RelocateTracker(initLaneLinkptr, 0.0);
        /*mObstacleType = refObstaclePtr->type();*/

        mLocation.vLaneDir() =
            HdMap::HadmapCacheConCurrent::GetLaneLinkDir(initLaneLinkptr, mLocation.DistanceAlongCurve());
        // 获取障碍物的旋转角度
        Base::txFloat r, p, y;
        obj_ptr->getRPY(r, p, y);
        m_rot.FromRadian(y);
        // 同步位置
        SyncPosition(0.0);
        const Base::txVec3 frontAxis3d(mLocation.vLaneDir().x(), mLocation.vLaneDir().y(), 0.0);
        mGeometryData.ComputePolygon(GeomCenter().ToENU(), frontAxis3d);
        // 填充空间查询数据
        FillingSpatialQuery();
        // 注册初始化信息
        RegisterInfoOnInit(mLocation);
        LogInfo << "Obstacle Init Success. " << Str() << TX_VARS(Id()) << TX_VARS(SysId())
                << TX_VARS(VehicleCoordStr());
        return true;
      } else {
        LOG(WARNING) << "GetLane Failure." << RawVehicleCoord();
        return false;
      }
      LOG(WARNING) << "GetLane Failure." << RawVehicleCoord();
      return false;
    }
  } else {
    LogInfo << "Param Error." << TX_COND(NonNull_Pointer(obj_ptr));
    return false;
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)
