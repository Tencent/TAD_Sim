// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_vehicle_coord.h"
#include "tx_catalog_cache.h"
TX_NAMESPACE_OPEN(Base)

const Base::txBool VehicleCoord::RawVehicleCoord_Is_GeomCenter() const TX_NOEXCEPT {
  using ElementType = Base::Enums::ElementType;
  // 如果车辆坐标元素类型为TAD_Ego、TAD_Planning或TAD_DummyDriver
  if (_plus_(ElementType::TAD_Ego) == VehicleCoordElementType() ||
      _plus_(ElementType::TAD_Planning) == VehicleCoordElementType() ||
      _plus_(ElementType::TAD_DummyDriver) == VehicleCoordElementType()) {
    // 如果车辆坐标类型为RearAxle_SnappedToGround或TrailerHitch_SnappedToGround，则返回false
    if (sim_msg::VehicleGeometory::VehicleCoordType::VehicleGeometory_VehicleCoordType_RearAxle_SnappedToGround ==
            m_pb_veh_geom.coord_type() ||
        sim_msg::VehicleGeometory::VehicleCoordType::VehicleGeometory_VehicleCoordType_TrailerHitch_SnappedToGround ==
            m_pb_veh_geom.coord_type()) {
      return false;
    } else {
      return true;
    }
  } else {
    // 如果车辆坐标元素类型不是TAD_Ego、TAD_Planning或TAD_DummyDriver，则返回false
    return false; /*catalog boundingbox center*/
  }
}

const Base::txBool VehicleCoord::RawVehicleCoord_Is_RearAxleCenter() const TX_NOEXCEPT {
  return !RawVehicleCoord_Is_GeomCenter();
}

// 获取车辆坐标的字符串表示
txString VehicleCoord::VehicleCoordStr() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "[VehicleCoord]" << TX_COND_NAME(Is_GeomCenter, RawVehicleCoord_Is_GeomCenter())
      << TX_COND_NAME(Is_RearAxleCenter, RawVehicleCoord_Is_RearAxleCenter())
      << TX_VARS_NAME(ElementType, __enum2lpsz__(ElementType, VehicleCoordElementType()))
      << TX_VARS_NAME(Raw, RawVehicleCoord().StrWGS84()) << TX_VARS_NAME(GeomCenter, GeomCenter().StrWGS84())
      << TX_VARS_NAME(RearAxleCenter, RearAxleCenter().StrWGS84());
  return oss.str();
}

Base::txBool VehicleCoord::InitCatalog(const Base::Enums::VEHICLE_TYPE _vehType) TX_NOEXCEPT {
  // 查询指定类型的车辆信息
  Base::CatalogCache::Catalog_Vehicle veh_catalog;
  if (CallSucc(Base::CatalogCache::Query_Vehicle_Catalog(_vehType, veh_catalog))) {
    // 获取车辆的底部中心点坐标
    const auto& rear2center = veh_catalog.bbx.center.rear2center;
    const auto& dim = veh_catalog.bbx.dim;
    // 使用底部中心点坐标初始化车辆信息
    InitCatalog(rear2center.x(), rear2center.y(), rear2center.z());
    m_catalog_length = dim.dbLength;
    m_catalog_width = dim.dbWidth;
    m_catalog_height = dim.dbHeight;
    return true;
  } else {
    InitCatalog(0.0, 0.0, 0.0);
    return false;
  }
}

Base::txBool VehicleCoord::InitCatalog(const Base::Enums::PEDESTRIAN_TYPE _pedType) TX_NOEXCEPT {
  // 从目录中查询指定类型的行人信息
  Base::CatalogCache::Catalog_Pedestrian ped_catalog;
  if (CallSucc(Base::CatalogCache::Query_Pedestrian_Catalog(_pedType, ped_catalog))) {
    // 获取行人的底部中心点坐标
    const auto& rear2center = ped_catalog.bbx.center.rear2center;
    // 获取行人的尺寸信息
    const auto& dim = ped_catalog.bbx.dim;
    // 使用底部中心点坐标初始化行人信息
    InitCatalog(rear2center.x(), rear2center.y(), rear2center.z());
    // 设置行人的长度、宽度、高度信息
    m_catalog_length = dim.dbLength;
    m_catalog_width = dim.dbWidth;
    m_catalog_height = dim.dbHeight;
    return true;
  } else {
    InitCatalog(0.0, 0.0, 0.0);
    return false;
  }
}

Base::txBool VehicleCoord::InitCatalog(const Base::Enums::STATIC_ELEMENT_TYPE _obsType) TX_NOEXCEPT {
  // 根据给定的静态元素类型，从目录中查询对应的静态元素信息
  Base::CatalogCache::Catalog_MiscObject obs_catalog;
  if (CallSucc(Base::CatalogCache::Query_Obstacle_Catalog(_obsType, obs_catalog))) {
    // 获取静态元素的向量坐标（底部中心点）
    const auto& rear2center = obs_catalog.bbx.center.rear2center;
    // 获取静态元素的尺寸信息
    const auto& dim = obs_catalog.bbx.dim;
    // 使用获取到的向量坐标信息初始化静态元素的目录信息
    InitCatalog(rear2center.x(), rear2center.y(), rear2center.z());
    // 获取静态元素的长宽高
    m_catalog_length = dim.dbLength;
    m_catalog_width = dim.dbWidth;
    m_catalog_height = dim.dbHeight;
    return true;
  } else {
    InitCatalog(0.0, 0.0, 0.0);
    return false;
  }
}

#if USE_CustomModelImport
Base::txBool VehicleCoord::InitVehicleCatalog(const Base::txString& _vehType) TX_NOEXCEPT {
  Base::CatalogCache::Catalog_Vehicle veh_catalog;
  if (CallSucc(Base::CatalogCache::Query_Vehicle_Catalog(_vehType, veh_catalog))) {
    const auto& rear2center = veh_catalog.bbx.center.rear2center;
    const auto& dim = veh_catalog.bbx.dim;
    InitCatalog(rear2center.x(), rear2center.y(), rear2center.z());
    m_catalog_length = dim.dbLength;
    m_catalog_width = dim.dbWidth;
    m_catalog_height = dim.dbHeight;
    return true;
  } else {
    InitCatalog(0.0, 0.0, 0.0);
    return false;
  }
}

Base::txBool VehicleCoord::InitPedestrianCatalog(const Base::txString& _pedType) TX_NOEXCEPT {
  Base::CatalogCache::Catalog_Pedestrian ped_catalog;
  if (CallSucc(Base::CatalogCache::Query_Pedestrian_Catalog(_pedType, ped_catalog))) {
    const auto& rear2center = ped_catalog.bbx.center.rear2center;
    const auto& dim = ped_catalog.bbx.dim;
    InitCatalog(rear2center.x(), rear2center.y(), rear2center.z());
    m_catalog_length = dim.dbLength;
    m_catalog_width = dim.dbWidth;
    m_catalog_height = dim.dbHeight;
    return true;
  } else {
    InitCatalog(0.0, 0.0, 0.0);
    return false;
  }
}

Base::txBool VehicleCoord::InitObstacleCatalog(const Base::txString& _obsType) TX_NOEXCEPT {
  Base::CatalogCache::Catalog_MiscObject obs_catalog;
  if (CallSucc(Base::CatalogCache::Query_Obstacle_Catalog(_obsType, obs_catalog))) {
    const auto& rear2center = obs_catalog.bbx.center.rear2center;
    const auto& dim = obs_catalog.bbx.dim;
    InitCatalog(rear2center.x(), rear2center.y(), rear2center.z());
    m_catalog_length = dim.dbLength;
    m_catalog_width = dim.dbWidth;
    m_catalog_height = dim.dbHeight;
    return true;
  } else {
    InitCatalog(0.0, 0.0, 0.0);
    return false;
  }
}
#endif /*USE_CustomModelImport*/
TX_NAMESPACE_CLOSE(Base)
