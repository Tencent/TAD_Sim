// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_catalog_cache.h"
#include "tad_catalog.h"
#include "tx_path_utils.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Catalogs)
TX_NAMESPACE_OPEN(Base)

CatalogCache::id2VehicleType CatalogCache::s_id2VehicleType;
CatalogCache::id2PedestrianType CatalogCache::s_id2PedestrianType;
CatalogCache::id2ObstacleType CatalogCache::s_id2ObstacleType;

#if USE_CustomModelImport
CatalogCache::vehicleTypeEnums CatalogCache::s_vehicleTypeEnums;
CatalogCache::pedestrianTypeEnums CatalogCache::s_pedestrianTypeEnums;
CatalogCache::obstacleTypeEnums CatalogCache::s_obstacleTypeEnums;

CatalogCache::str2VehicleType CatalogCache::s_str2VehicleType;
CatalogCache::str2PedestrianType CatalogCache::s_str2PedestrianType;
CatalogCache::str2ObstacleType CatalogCache::s_str2ObstacleType;

std::map<std::string, std::string> CatalogCache::s_categoryMap = {{"ego", "Ego"},
                                                                  {"car", "Car"},
                                                                  {"pedestrian", "DynamicObstacle"},
                                                                  {"moto", "DynamicObstacle"},
                                                                  {"bike", "DynamicObstacle"},
                                                                  {"animal", "DynamicObstacle"},
                                                                  {"machine", "DynamicObstacle"},
                                                                  {"obstacle", "StaticObstacle"}};

txString CatalogCache::GetMajorCategory(const Base::txString& minorCategory) TX_NOEXCEPT {
  auto it = s_categoryMap.find(minorCategory);
  if (it != s_categoryMap.end()) {
    return it->second;
  } else {
    return "Unknown";  // 返回"Unknown"表示输入的小类在映射中不存在
  }
}
#endif /*USE_CustomModelImport*/

txInt GenerateIdByName(const txString& _type) TX_NOEXCEPT {
  const std::string prefix = "user_";
  int modelId = -1;
  if (_type.substr(0, prefix.size()) == prefix) {
    size_t pos = _type.rfind('_');
    if (pos != std::string::npos) {
      std::string hashStr = _type.substr(pos + 1);
      try {
        int hash = std::stoi(hashStr);
        modelId = (hash >> 16) + 100000;
      } catch (...) {
        LOG(WARNING) << "Generate model id failed. " << TX_VARS(_type) << TX_VARS(hashStr);
      }
      return modelId;
    }
  }
  return modelId;
}

txBool CatalogCache::Initialize(const txString& std_vehicle_catalog_path, const txString& std_pedestrian_catalog_path,
                                const txString& std_obstacle_catalog_path) TX_NOEXCEPT {
  LogInfo << TX_VARS(std_vehicle_catalog_path) << TX_VARS(std_pedestrian_catalog_path)
          << TX_VARS(std_obstacle_catalog_path);
  Release();
  using namespace Utils;
  using namespace boost::filesystem;
  // 对应的catalog路径是否存在
  if (CallSucc(exists(FilePath(std_vehicle_catalog_path))) && CallSucc(exists(FilePath(std_pedestrian_catalog_path))) &&
      CallSucc(exists(FilePath(std_obstacle_catalog_path)))) {
    // 加载vehicle catalog
    auto veh_catalog_ptr = SceneLoader::Type_Catalog::load_vehicle_catalog(std_vehicle_catalog_path);
    // 加载pedestrian catalog
    auto ped_catalog_ptr = SceneLoader::Type_Catalog::load_pedestrian_catalog(std_pedestrian_catalog_path);
    // 加载obstacle catalog
    auto obs_catalog_ptr = SceneLoader::Type_Catalog::load_obstacle_catalog(std_obstacle_catalog_path);

#if USE_CustomModelImport
    ImportCustomModel(veh_catalog_ptr, ped_catalog_ptr, obs_catalog_ptr);
#else  /*USE_CustomModelImport*/
    if (NonNull_Pointer(veh_catalog_ptr)) {
      for (auto refInfo : veh_catalog_ptr->catalog.vehicles) {
        const auto type = __lpsz2enum__(VEHICLE_TYPE, refInfo.name.c_str());
        // 初始化插入
        if (_plus_(Base::Enums::VEHICLE_TYPE::undefined) != type) {
          refInfo.bbx.Init();
          s_id2VehicleType.insert(std::make_pair(__enum2int__(VEHICLE_TYPE, type), refInfo));
        } else {
          // 将refInfo.name转换为PEDESTRIAN_TYPE枚举类型
          const auto ped_type = __lpsz2enum__(PEDESTRIAN_TYPE, refInfo.name.c_str());
          // 创建一个Catalog_Pedestrian对象
          Catalog_Pedestrian ped;
          // 将refInfo.bbx赋值给ped.bbx
          ped.bbx = refInfo.bbx;
          // 初始化ped.bbx
          ped.bbx.Init();
          s_id2PedestrianType.insert(std::make_pair(__enum2int__(PEDESTRIAN_TYPE, ped_type), ped));
        }
      }
    }

    if (NonNull_Pointer(ped_catalog_ptr)) {
      // 遍历pedestrian
      for (auto refInfo : ped_catalog_ptr->catalog.pedestrians) {
        // 将refInfo.name转换为PEDESTRIAN_TYPE枚举类型
        const auto type = __lpsz2enum__(PEDESTRIAN_TYPE, refInfo.name.c_str());
        // 初始化refInfo的bbx
        refInfo.bbx.Init();
        // 将type和refInfo插入到s_id2PedestrianType中
        s_id2PedestrianType.insert(std::make_pair(__enum2int__(PEDESTRIAN_TYPE, type), refInfo));
      }
    }

    if (NonNull_Pointer(obs_catalog_ptr)) {
      // 遍历obstacle
      for (auto refInfo : obs_catalog_ptr->catalog.obstacles) {
        const auto type = __lpsz2enum__(STATIC_ELEMENT_TYPE, refInfo.name.c_str());
        refInfo.bbx.Init();
        s_id2ObstacleType.insert(std::make_pair(__enum2int__(STATIC_ELEMENT_TYPE, type), refInfo));
      }
    }
#endif /*USE_CustomModelImport*/
    return true;
  } else {
    return false;
  }
}

txBool CatalogCache::Initialize(const txString& std_vehicle_catalog_path, const txString& std_pedestrian_catalog_path,
                                const txString& std_obstacle_catalog_path, const txString& sys_vehicle_catalog_path,
                                const txString& sys_pedestrian_catalog_path,
                                const txString& sys_obstacle_catalog_path) TX_NOEXCEPT {
  LogInfo << TX_VARS(std_vehicle_catalog_path) << TX_VARS(std_pedestrian_catalog_path)
          << TX_VARS(std_obstacle_catalog_path);
  Release();
  using namespace Utils;
  using namespace boost::filesystem;
  if (CallSucc(exists(FilePath(std_vehicle_catalog_path))) && CallSucc(exists(FilePath(std_pedestrian_catalog_path))) &&
      CallSucc(exists(FilePath(std_obstacle_catalog_path))) && CallSucc(exists(FilePath(sys_vehicle_catalog_path))) &&
      CallSucc(exists(FilePath(sys_pedestrian_catalog_path))) &&
      CallSucc(exists(FilePath(sys_obstacle_catalog_path)))) {
    auto veh_sys_catalog_ptr = SceneLoader::Type_Catalog::load_vehicle_catalog(sys_vehicle_catalog_path);
    auto ped_sys_catalog_ptr = SceneLoader::Type_Catalog::load_pedestrian_catalog(sys_pedestrian_catalog_path);
    auto obs_sys_catalog_ptr = SceneLoader::Type_Catalog::load_obstacle_catalog(sys_obstacle_catalog_path);

    auto veh_catalog_ptr = SceneLoader::Type_Catalog::load_vehicle_catalog(std_vehicle_catalog_path);
    auto ped_catalog_ptr = SceneLoader::Type_Catalog::load_pedestrian_catalog(std_pedestrian_catalog_path);
    auto obs_catalog_ptr = SceneLoader::Type_Catalog::load_obstacle_catalog(std_obstacle_catalog_path);

    if (NonNull_Pointer(veh_sys_catalog_ptr)) {
      for (auto refInfo : veh_sys_catalog_ptr->catalog.vehicles) {
        Base::txInt modelId = -1;
        Base::txString category = "";
        // Update the enumeration of vehicle types
        for (auto refProperty : refInfo.properties) {
          if ("model_id" == refProperty.name) {
            modelId = std::stod(refProperty.value);
          } else if ("interfaceCategory" == refProperty.name) {
            category = refProperty.value;
          }
        }

        if ("Car" == GetMajorCategory(category)) {
          s_vehicleTypeEnums.insert(std::make_pair(refInfo.name.c_str(), modelId));
          refInfo.bbx.Init();
          s_str2VehicleType.insert(std::make_pair(refInfo.name.c_str(), refInfo));
        } else if ("DynamicObstacle" == GetMajorCategory(category)) {
          s_pedestrianTypeEnums.insert(std::make_pair(refInfo.name.c_str(), modelId));
          Catalog_Pedestrian ped;
          ped.bbx = refInfo.bbx;
          ped.bbx.Init();
          s_str2PedestrianType.insert(std::make_pair(refInfo.name.c_str(), ped));
        } else if ("StaticObstacle" == GetMajorCategory(category)) {
          s_obstacleTypeEnums.insert(std::make_pair(refInfo.name.c_str(), modelId));
          Catalog_MiscObject obs;
          obs.bbx = refInfo.bbx;
          obs.bbx.Init();
          s_str2ObstacleType.insert(std::make_pair(refInfo.name.c_str(), obs));
        }
      }
    }

    if (NonNull_Pointer(veh_catalog_ptr)) {
      for (auto refInfo : veh_catalog_ptr->catalog.vehicles) {
        Base::txInt modelId = -1;
        Base::txString category = "";
        // Update the enumeration of vehicle types
        for (auto refProperty : refInfo.properties) {
          if ("model_id" == refProperty.name) {
            modelId = std::stod(refProperty.value);
          } else if ("interfaceCategory" == refProperty.name) {
            category = refProperty.value;
          }
        }

        if ("Car" == GetMajorCategory(category)) {
          s_vehicleTypeEnums.insert(std::make_pair(refInfo.name.c_str(), modelId));
          refInfo.bbx.Init();
          s_str2VehicleType.insert(std::make_pair(refInfo.name.c_str(), refInfo));
        } else if ("DynamicObstacle" == GetMajorCategory(category)) {
          s_pedestrianTypeEnums.insert(std::make_pair(refInfo.name.c_str(), modelId));
          Catalog_Pedestrian ped;
          ped.bbx = refInfo.bbx;
          ped.bbx.Init();
          s_str2PedestrianType.insert(std::make_pair(refInfo.name.c_str(), ped));
        } else if ("StaticObstacle" == GetMajorCategory(category)) {
          s_obstacleTypeEnums.insert(std::make_pair(refInfo.name.c_str(), modelId));
          Catalog_MiscObject obs;
          obs.bbx = refInfo.bbx;
          obs.bbx.Init();
          s_str2ObstacleType.insert(std::make_pair(refInfo.name.c_str(), obs));
        }
      }
    }

    if (NonNull_Pointer(ped_sys_catalog_ptr)) {
      for (auto refInfo : ped_sys_catalog_ptr->catalog.pedestrians) {
        // Update the enumeration of vehicle types
        for (auto refProperty : refInfo.properties) {
          if ("model_id" == refProperty.name) {
            s_pedestrianTypeEnums.insert(std::make_pair(refInfo.name.c_str(), std::stod(refProperty.value)));
            break;
          }
        }
        refInfo.bbx.Init();
        s_str2PedestrianType.insert(std::make_pair(refInfo.name.c_str(), refInfo));
      }
    }

    if (NonNull_Pointer(ped_catalog_ptr)) {
      for (auto refInfo : ped_catalog_ptr->catalog.pedestrians) {
        // Update the enumeration of vehicle types
        for (auto refProperty : refInfo.properties) {
          if ("model_id" == refProperty.name) {
            s_pedestrianTypeEnums.insert(std::make_pair(refInfo.name.c_str(), std::stod(refProperty.value)));
            break;
          }
        }
        refInfo.bbx.Init();
        s_str2PedestrianType.insert(std::make_pair(refInfo.name.c_str(), refInfo));
      }
    }

    if (NonNull_Pointer(obs_sys_catalog_ptr)) {
      for (auto refInfo : obs_sys_catalog_ptr->catalog.obstacles) {
        // Update the enumeration of vehicle types
        for (auto refProperty : refInfo.properties) {
          if ("model_id" == refProperty.name) {
            s_obstacleTypeEnums.insert(std::make_pair(refInfo.name.c_str(), std::stod(refProperty.value)));
            break;
          }
        }
        refInfo.bbx.Init();
        s_str2ObstacleType.insert(std::make_pair(refInfo.name.c_str(), refInfo));
      }
    }

    if (NonNull_Pointer(obs_catalog_ptr)) {
      for (auto refInfo : obs_catalog_ptr->catalog.obstacles) {
        // Update the enumeration of vehicle types
        for (auto refProperty : refInfo.properties) {
          if ("model_id" == refProperty.name) {
            s_obstacleTypeEnums.insert(std::make_pair(refInfo.name.c_str(), std::stod(refProperty.value)));
            break;
          }
        }
        refInfo.bbx.Init();
        s_str2ObstacleType.insert(std::make_pair(refInfo.name.c_str(), refInfo));
      }
    }
    return true;
  } else {
    return false;
  }
}

void CatalogCache::Release() TX_NOEXCEPT {
  s_id2VehicleType.clear();
  s_id2PedestrianType.clear();
  s_id2ObstacleType.clear();

#if USE_CustomModelImport
  s_vehicleTypeEnums.clear();
  s_pedestrianTypeEnums.clear();
  s_obstacleTypeEnums.clear();

  s_str2VehicleType.clear();
  s_str2PedestrianType.clear();
  s_str2ObstacleType.clear();
#endif /*USE_CustomModelImport*/
}

void CatalogCache::ShowCacheInfo() TX_NOEXCEPT {
#if USE_CustomModelImport
  LogInfo << "[CatalogCache::ShowCacheInfo]" << TX_VARS(s_vehicleTypeEnums.size())
          << TX_VARS(s_pedestrianTypeEnums.size()) << TX_VARS(s_obstacleTypeEnums.size())
          << TX_VARS(s_str2VehicleType.size()) << TX_VARS(s_str2PedestrianType.size())
          << TX_VARS(s_str2ObstacleType.size());

  for (const auto& refVeh : s_vehicleTypeEnums) {
    LogInfo << TX_VARS_NAME(Name, refVeh.first) << TX_VARS_NAME(Idx, refVeh.second);
  }
  for (const auto& refPed : s_pedestrianTypeEnums) {
    LogInfo << TX_VARS_NAME(Name, refPed.first) << TX_VARS_NAME(Idx, refPed.second);
  }
  for (const auto& refObs : s_obstacleTypeEnums) {
    LogInfo << TX_VARS_NAME(Name, refObs.first) << TX_VARS_NAME(Idx, refObs.second);
  }
  for (const auto& refVeh : s_str2VehicleType) {
    LogInfo << TX_VARS_NAME(Name, refVeh.first) << TX_VARS_NAME(Content, refVeh.second);
  }
  for (const auto& refPed : s_str2PedestrianType) {
    LogInfo << TX_VARS_NAME(Name, refPed.first) << TX_VARS_NAME(Content, refPed.second);
  }
  for (const auto& refObs : s_str2ObstacleType) {
    LogInfo << TX_VARS_NAME(Name, refObs.first) << TX_VARS_NAME(Content, refObs.second);
  }
#else
  LogInfo << "[CatalogCache::ShowCacheInfo]" << TX_VARS(s_id2VehicleType.size()) << TX_VARS(s_id2PedestrianType.size())
          << TX_VARS(s_id2ObstacleType.size());
  for (const auto& refVeh : s_id2VehicleType) {
    const auto tmp = __int2enum__(VEHICLE_TYPE, refVeh.first);
    LogInfo << TX_VARS_NAME(Idx, refVeh.first)
            << TX_VARS_NAME(VEHICLE_TYPE, __enum2lpsz__(Base::Enums::VEHICLE_TYPE, tmp))
            << TX_VARS_NAME(Content, refVeh.second);
  }
  // 遍历展示ped信息
  for (const auto& refPed : s_id2PedestrianType) {
    const auto tmp = __int2enum__(PEDESTRIAN_TYPE, refPed.first);
    LogInfo << TX_VARS_NAME(Idx, refPed.first)
            << TX_VARS_NAME(PEDESTRIAN_TYPE, __enum2lpsz__(Base::Enums::PEDESTRIAN_TYPE, tmp))
            << TX_VARS_NAME(Content, refPed.second);
  }
  // 遍历展示obs信息
  for (const auto& refObs : s_id2ObstacleType) {
    const auto tmp = __int2enum__(STATIC_ELEMENT_TYPE, refObs.first);
    LogInfo << TX_VARS_NAME(Idx, refObs.first)
            << TX_VARS_NAME(OBSTACLE_TYPE, __enum2lpsz__(Base::Enums::STATIC_ELEMENT_TYPE, tmp))
            << TX_VARS_NAME(Content, refObs.second);
  }
#endif /*USE_CustomModelImport*/
}

txBool CatalogCache::Query_Vehicle_Catalog(const Base::Enums::VEHICLE_TYPE& _type) TX_NOEXCEPT {
  id2VehicleType::const_accessor ca;
  // 查询到返回true
  if (CallSucc(s_id2VehicleType.find(ca, __enum2int__(VEHICLE_TYPE, _type)))) {
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

txBool CatalogCache::Query_Pedestrian_Catalog(const Base::Enums::PEDESTRIAN_TYPE& _type) TX_NOEXCEPT {
  id2PedestrianType::const_accessor ca;
  if (CallSucc(s_id2PedestrianType.find(ca, __enum2int__(PEDESTRIAN_TYPE, _type)))) {
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

txBool CatalogCache::Query_Obstacle_Catalog(const Base::Enums::STATIC_ELEMENT_TYPE& _type) TX_NOEXCEPT {
  id2ObstacleType::const_accessor ca;
  if (CallSucc(s_id2ObstacleType.find(ca, __enum2int__(STATIC_ELEMENT_TYPE, _type)))) {
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

txBool CatalogCache::Query_Vehicle_Catalog(const Base::Enums::VEHICLE_TYPE& _type,
                                           Catalog_Vehicle& ref_veh_catalog) TX_NOEXCEPT {
  id2VehicleType::const_accessor ca;
  if (CallSucc(s_id2VehicleType.find(ca, __enum2int__(VEHICLE_TYPE, _type)))) {
    // 查询到则赋值到输出
    ref_veh_catalog = (*ca).second;
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

txBool CatalogCache::Query_Pedestrian_Catalog(const Base::Enums::PEDESTRIAN_TYPE& _type,
                                              Catalog_Pedestrian& ref_ped_catalog) TX_NOEXCEPT {
  id2PedestrianType::const_accessor ca;
  // 如果在s_id2PedestrianType中查找到了与_type对应的键值对
  if (CallSucc(s_id2PedestrianType.find(ca, __enum2int__(PEDESTRIAN_TYPE, _type)))) {
    // 查询到则赋值到输出
    ref_ped_catalog = (*ca).second;
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

txBool CatalogCache::Query_Obstacle_Catalog(const Base::Enums::STATIC_ELEMENT_TYPE& _type,
                                            Catalog_MiscObject& ref_obs_catalog) TX_NOEXCEPT {
  id2ObstacleType::const_accessor ca;
  // 如果在s_id2ObstacleType中查找到了与_type对应的键值对
  if (CallSucc(s_id2ObstacleType.find(ca, __enum2int__(STATIC_ELEMENT_TYPE, _type)))) {
    // 查询到则赋值到输出
    ref_obs_catalog = (*ca).second;
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

#if USE_CustomModelImport
txInt CatalogCache::VEHICLE_TYPE(const Base::txString& _type) TX_NOEXCEPT {
  vehicleTypeEnums::const_accessor ca;
  if (CallSucc(s_vehicleTypeEnums.find(ca, _type))) {
    txInt modelId = (*ca).second;
    ca.release();
    return modelId;
  }
  ca.release();
  return GenerateIdByName(_type);
}

txInt CatalogCache::PEDESTRIAN_TYPE(const Base::txString& _type) TX_NOEXCEPT {
  pedestrianTypeEnums::const_accessor ca;
  if (CallSucc(s_pedestrianTypeEnums.find(ca, _type))) {
    txInt modelId = (*ca).second;
    ca.release();
    return modelId;
  }
  ca.release();
  return GenerateIdByName(_type);
}

txInt CatalogCache::OBSTACLE_TYPE(const Base::txString& _type) TX_NOEXCEPT {
  obstacleTypeEnums::const_accessor ca;
  if (CallSucc(s_obstacleTypeEnums.find(ca, _type))) {
    txInt modelId = (*ca).second;
    ca.release();
    return modelId;
  }
  ca.release();
  return GenerateIdByName(_type);
}

txBool CatalogCache::Query_Vehicle_Catalog(const Base::txString& _type) TX_NOEXCEPT {
  str2VehicleType::const_accessor ca;
  if (CallSucc(s_str2VehicleType.find(ca, _type))) {
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

txBool CatalogCache::Query_Pedestrian_Catalog(const Base::txString& _type) TX_NOEXCEPT {
  str2PedestrianType::const_accessor ca;
  if (CallSucc(s_str2PedestrianType.find(ca, _type))) {
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

txBool CatalogCache::Query_Obstacle_Catalog(const Base::txString& _type) TX_NOEXCEPT {
  str2ObstacleType::const_accessor ca;
  if (CallSucc(s_str2ObstacleType.find(ca, _type))) {
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

txBool CatalogCache::Query_Vehicle_Catalog(const Base::txString& _type, Catalog_Vehicle& ref_veh_catalog) TX_NOEXCEPT {
  str2VehicleType::const_accessor ca;
  if (CallSucc(s_str2VehicleType.find(ca, _type))) {
    ref_veh_catalog = (*ca).second;
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

txBool CatalogCache::Query_Pedestrian_Catalog(const Base::txString& _type,
                                              Catalog_Pedestrian& ref_ped_catalog) TX_NOEXCEPT {
  str2PedestrianType::const_accessor ca;
  if (CallSucc(s_str2PedestrianType.find(ca, _type))) {
    ref_ped_catalog = (*ca).second;
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

txBool CatalogCache::Query_Obstacle_Catalog(const Base::txString& _type,
                                            Catalog_MiscObject& ref_obs_catalog) TX_NOEXCEPT {
  str2ObstacleType::const_accessor ca;
  if (CallSucc(s_str2ObstacleType.find(ca, _type))) {
    ref_obs_catalog = (*ca).second;
    ca.release();
    return true;
  }
  ca.release();
  return false;
}

void CatalogCache::ImportCustomModel(SceneLoader::Type_Catalog::VehicleCatalogPtr veh_catalog_ptr,
                                     SceneLoader::Type_Catalog::PedestrianCatalogPtr ped_catalog_ptr,
                                     SceneLoader::Type_Catalog::MiscObjectCatalogPtr obs_catalog_ptr) TX_NOEXCEPT {
  if (NonNull_Pointer(veh_catalog_ptr)) {
    for (auto refInfo : veh_catalog_ptr->catalog.vehicles) {
      // Update the enumeration of vehicle types
      for (auto refProperty : refInfo.properties) {
        if ("model_id" == refProperty.name) {
          s_vehicleTypeEnums.insert(std::make_pair(refInfo.name.c_str(), std::stod(refProperty.value)));
          break;
        }
      }

      refInfo.bbx.Init();
      s_str2VehicleType.insert(std::make_pair(refInfo.name.c_str(), refInfo));
    }
  }

  if (NonNull_Pointer(ped_catalog_ptr)) {
    for (auto refInfo : ped_catalog_ptr->catalog.pedestrians) {
      // Update the enumeration of vehicle types
      for (auto refProperty : refInfo.properties) {
        if ("model_id" == refProperty.name) {
          s_pedestrianTypeEnums.insert(std::make_pair(refInfo.name.c_str(), std::stod(refProperty.value)));
          break;
        }
      }
      refInfo.bbx.Init();
      s_str2PedestrianType.insert(std::make_pair(refInfo.name.c_str(), refInfo));
    }
  }

  if (NonNull_Pointer(obs_catalog_ptr)) {
    for (auto refInfo : obs_catalog_ptr->catalog.obstacles) {
      // Update the enumeration of vehicle types
      for (auto refProperty : refInfo.properties) {
        if ("model_id" == refProperty.name) {
          s_obstacleTypeEnums.insert(std::make_pair(refInfo.name.c_str(), std::stod(refProperty.value)));
          break;
        }
      }
      refInfo.bbx.Init();
      s_str2ObstacleType.insert(std::make_pair(refInfo.name.c_str(), refInfo));
    }
  }
}
#endif /*USE_CustomModelImport*/
TX_NAMESPACE_CLOSE(Base)
