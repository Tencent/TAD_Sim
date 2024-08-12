// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_catalog.h"
#include "tbb/concurrent_hash_map.h"
#include "tx_enum_def.h"
#include "tx_header.h"

TX_NAMESPACE_OPEN(Base)

class CatalogCache {
 public:
  using Catalog_Vehicle = SceneLoader::Type_Catalog::Vehicle;
  using Catalog_Pedestrian = SceneLoader::Type_Catalog::Pedestrian;
  using Catalog_MiscObject = SceneLoader::Type_Catalog::MiscObject;

 private:
  CatalogCache() TX_DEFAULT;
  ~CatalogCache() TX_DEFAULT;
  CatalogCache(const CatalogCache&) TX_DELETE;
  CatalogCache& operator=(const CatalogCache&) TX_DELETE;

 public:
  /**
   * @brief 初始化车辆、行人和障碍物类目缓存。
   *
   * @param std_vehicle_catalog_path  车辆类目的标准路径。
   * @param std_pedestrian_catalog_path 行人类目的标准路径。
   * @param std_obstacle_catalog_path 障碍物类目的标准路径。
   * @return txBool 初始化是否成功，返回 true 表示成功，false 表示失败。
   */
  static txBool Initialize(const txString& std_vehicle_catalog_path, const txString& std_pedestrian_catalog_path,
                           const txString& std_obstacle_catalog_path) TX_NOEXCEPT;
  static txBool Initialize(const txString& std_vehicle_catalog_path, const txString& std_pedestrian_catalog_path,
                           const txString& std_obstacle_catalog_path, const txString& sys_vehicle_catalog_path,
                           const txString& sys_pedestrian_catalog_path,
                           const txString& sys_obstacle_catalog_path) TX_NOEXCEPT;

  /**
   * @brief 释放 txCatalogCache 类所占用的资源。
   *
   */
  static void Release() TX_NOEXCEPT;

  /**
   * @brief 显示缓存信息
   *
   */
  static void ShowCacheInfo() TX_NOEXCEPT;

  /**
   * @brief 按照类型查询vehicle_Catalog是否存在
   *
   * @param _type 类型输入
   * @return txBool 是否存在
   */
  static txBool Query_Vehicle_Catalog(const Base::Enums::VEHICLE_TYPE& _type) TX_NOEXCEPT;

  /**
   * @brief 按照类型查询vehicle_Catalog是否存在，并返回对象
   *
   * @param _type 类型输入
   * @param ref_veh_catalog 获取到的catalog对象
   * @return txBool 是否存在
   */
  static txBool Query_Vehicle_Catalog(const Base::Enums::VEHICLE_TYPE& _type,
                                      Catalog_Vehicle& ref_veh_catalog) TX_NOEXCEPT;

  /**
   * @brief 按照类型查询pedestrian_Catalog是否存在
   *
   * @param _type 类型输入
   * @return txBool 是否存在
   */
  static txBool Query_Pedestrian_Catalog(const Base::Enums::PEDESTRIAN_TYPE& _type) TX_NOEXCEPT;

  /**
   * @brief 按照类型查询pedestrian_Catalog是否存在，并返回
   *
   * @param _type 类型输入
   * @param[out] ref_ped_catalog 输出对象
   * @return txBool 是否存在
   */
  static txBool Query_Pedestrian_Catalog(const Base::Enums::PEDESTRIAN_TYPE& _type,
                                         Catalog_Pedestrian& ref_ped_catalog) TX_NOEXCEPT;

  /**
   * @brief 按照类型查询obstacle_Catalog是否存在
   *
   * @param _type 类型输入
   * @return txBool 是否存在
   */
  static txBool Query_Obstacle_Catalog(const Base::Enums::STATIC_ELEMENT_TYPE& _type) TX_NOEXCEPT;

  /**
   * @brief 按照类型查询obstacle_Catalog是否存在,并返回
   *
   * @param _type 类型输入
   * @param[out] ref_obs_catalog 输出对象
   * @return txBool 是否存在
   */
  static txBool Query_Obstacle_Catalog(const Base::Enums::STATIC_ELEMENT_TYPE& _type,
                                       Catalog_MiscObject& ref_obs_catalog) TX_NOEXCEPT;

 protected:
  using id2VehicleType = tbb::concurrent_hash_map<Base::Enums::VEHICLE_TYPE::_integral, Catalog_Vehicle>;
  static id2VehicleType s_id2VehicleType;

  using id2PedestrianType = tbb::concurrent_hash_map<Base::Enums::PEDESTRIAN_TYPE::_integral, Catalog_Pedestrian>;
  static id2PedestrianType s_id2PedestrianType;

  using id2ObstacleType = tbb::concurrent_hash_map<Base::Enums::STATIC_ELEMENT_TYPE::_integral, Catalog_MiscObject>;
  static id2ObstacleType s_id2ObstacleType;

#if USE_CustomModelImport

 public:
  static txInt VEHICLE_TYPE(const Base::txString& _type) TX_NOEXCEPT;
  static txInt PEDESTRIAN_TYPE(const Base::txString& _type) TX_NOEXCEPT;
  static txInt OBSTACLE_TYPE(const Base::txString& _type) TX_NOEXCEPT;

  static txBool Query_Vehicle_Catalog(const Base::txString& _type) TX_NOEXCEPT;
  static txBool Query_Vehicle_Catalog(const Base::txString& _type, Catalog_Vehicle& ref_veh_catalog) TX_NOEXCEPT;
  static txBool Query_Pedestrian_Catalog(const Base::txString& _type) TX_NOEXCEPT;
  static txBool Query_Pedestrian_Catalog(const Base::txString& _type, Catalog_Pedestrian& ref_ped_catalog) TX_NOEXCEPT;
  static txBool Query_Obstacle_Catalog(const Base::txString& _type) TX_NOEXCEPT;
  static txBool Query_Obstacle_Catalog(const Base::txString& _type, Catalog_MiscObject& ref_obs_catalog) TX_NOEXCEPT;
  static txString GetMajorCategory(const Base::txString& minorCategory) TX_NOEXCEPT;

  static void ImportCustomModel(SceneLoader::Type_Catalog::VehicleCatalogPtr veh_catalog_ptr,
                                SceneLoader::Type_Catalog::PedestrianCatalogPtr ped_catalog_ptr,
                                SceneLoader::Type_Catalog::MiscObjectCatalogPtr obs_catalog_ptr) TX_NOEXCEPT;

 protected:
  using vehicleTypeEnums = tbb::concurrent_hash_map<Base::txString, Base::txInt>;
  static vehicleTypeEnums s_vehicleTypeEnums;

  using pedestrianTypeEnums = tbb::concurrent_hash_map<Base::txString, Base::txInt>;
  static pedestrianTypeEnums s_pedestrianTypeEnums;

  using obstacleTypeEnums = tbb::concurrent_hash_map<Base::txString, Base::txInt>;
  static obstacleTypeEnums s_obstacleTypeEnums;

  using str2VehicleType = tbb::concurrent_hash_map<Base::txString, Catalog_Vehicle>;
  static str2VehicleType s_str2VehicleType;

  using str2PedestrianType = tbb::concurrent_hash_map<Base::txString, Catalog_Pedestrian>;
  static str2PedestrianType s_str2PedestrianType;

  using str2ObstacleType = tbb::concurrent_hash_map<Base::txString, Catalog_MiscObject>;
  static str2ObstacleType s_str2ObstacleType;

  static std::map<std::string, std::string> s_categoryMap;
#endif /*USE_CustomModelImport*/
};

TX_NAMESPACE_CLOSE(Base)
