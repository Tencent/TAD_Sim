// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <map>
#include <string>
#include <vector>

#include "boost/filesystem/path.hpp"
#include "scene.pb.h"
#include "tinyxml2.h"

namespace tx_sim {
namespace coordinator {
class CVehicleCatalogXOSCParser {
 public:
  //!
  //! @brief 构造函数
  //! @details 接收三个字符串参数，分别表示车辆目录XOSC文件路径、其他元素目录XOSC文件路径和行人目录XOSC文件路径
  //!
  //! @param[in] vehicleCatalogXOSCPath 车辆目录XOSC文件路径
  //! @param[in] miscCatalogXOSCPath 其他元素目录XOSC文件路径
  //! @param[in] pedestrianCatalogXOSCPath 行人目录XOSC文件路径
  //!
  CVehicleCatalogXOSCParser(std::string vehicleCatalogXOSCSysPath, std::string miscCatalogXOSCSysPath,
                            std::string pedestrianCatalogXOSCSysPath, std::string vehicleCatalogXOSCDataPath,
                            std::string miscCatalogXOSCDataPath, std::string pedestrianCatalogXOSCDataPath,
                            std::string mapCatalogXMLDataPath)
      : m_vehicleCatalogXOSCSysPath(vehicleCatalogXOSCSysPath),
        m_miscCatalogXOSCSysPath(miscCatalogXOSCSysPath),
        m_pedestrianCatalogXOSCSysPath(pedestrianCatalogXOSCSysPath),
        m_vehicleCatalogXOSCDataPath(vehicleCatalogXOSCDataPath),
        m_miscCatalogXOSCDataPath(miscCatalogXOSCDataPath),
        m_pedestrianCatalogXOSCDataPath(pedestrianCatalogXOSCDataPath),
        m_mapCatalogXMLDataPath(mapCatalogXMLDataPath){};

  //!
  //! @brief 析构函数
  //! @details 无需执行任何操作
  //!
  ~CVehicleCatalogXOSCParser(){};

  //!
  //! @brief 函数名：Build
  //! @details 函数功能：构建车辆目录XOSC解析器
  //!
  bool Build();

  //!
  //! @brief 函数名：BuildPhysicleCommon
  //! @details 函数功能：构建物理通用元素
  //!
  //! @param[in] pPhysicleCommon 物理通用元素对象指针
  //! @param[in] trafficType 交通类型
  //! @param[in] nodeType 节点类型
  //!
  void BuildPhysicleCommon(sim_msg::PhysicleCommon* pPhysicleCommon, std::string trafficType, std::string nodeType);

  //!
  //! @brief 函数名：BuildPhysicleVehicleGeometory
  //! @details 函数功能：构建物理车辆几何体
  //!
  //! @param[in] pPhysicleCommon 物理车辆几何体对象指针
  //! @param[in] vehicleName 车辆名称
  //!
  void BuildPhysicleVehicleGeometory(sim_msg::VehicleGeometory* pPhysicleCommon, std::string vehicleName);

  //!
  //! @brief 函数名：BuildAllVehicles
  //! @details 函数功能：构建所有车辆
  //!
  //! @param[in] scene 场景对象
  //!
  void BuildAllVehicles(sim_msg::Scene& scene);

  //!
  //! @brief 函数名：BuildAllMiscObjects
  //! @details 函数功能：构建所有其他元素
  //!
  //! @param[in] scene 场景对象
  //!
  void BuildAllMiscObjects(sim_msg::Scene& scene);

  //!
  //! @brief 函数名：BuildAllPedestrians
  //! @details 函数功能：构建所有行人
  //!
  //! @param[in] scene 场景对象
  //!
  void BuildAllPedestrians(sim_msg::Scene& scene);
  void BuildAllMap3dModels(sim_msg::Scene& scene);
  void BuildCategoryVehicle(sim_msg::Vehicle* pVehicle, const std::string& name);
  void BuildCategoryVru(sim_msg::VulnerableRoadUser* pVru, const std::string& name);
  void BuildCategoryMisc(sim_msg::MiscellaneousObject* pMisc, const std::string& name);

 private:
  //!
  //! @brief 函数名：BuildVehicleNameMap
  //! @details 函数功能：构建车辆名称映射
  //!
  void BuildVehicleNameMap();

  //!
  //! @brief 函数名：BuildMiscObjectNameMap
  //! @details 函数功能：构建其他元素名称映射
  //!
  void BuildMiscObjectNameMap();

  //!
  //! @brief 函数名：BuildPedestrianNameMap
  //! @details 函数功能：构建行人名称映射
  //!
  void BuildPedestrianNameMap();
  void BuildModel3dMap();

  std::string m_vehicleCatalogXOSCSysPath;
  std::string m_miscCatalogXOSCSysPath;
  std::string m_pedestrianCatalogXOSCSysPath;
  std::string m_vehicleCatalogXOSCDataPath;
  std::string m_miscCatalogXOSCDataPath;
  std::string m_pedestrianCatalogXOSCDataPath;
  std::string m_mapCatalogXMLDataPath;

  const tinyxml2::XMLElement* m_vehicleCatalogXOSCSysXMLRoot = nullptr;
  const tinyxml2::XMLElement* m_miscObjectCatalogXOSCSysXMLRoot = nullptr;
  const tinyxml2::XMLElement* m_pedestrianCatalogXOSCSysXMLRoot = nullptr;
  const tinyxml2::XMLElement* m_vehicleCatalogXOSCDataXMLRoot = nullptr;
  const tinyxml2::XMLElement* m_miscObjectCatalogXOSCDataXMLRoot = nullptr;
  const tinyxml2::XMLElement* m_pedestrianCatalogXOSCDataXMLRoot = nullptr;
  const tinyxml2::XMLElement* m_mapCatalogDataXMLDataRoot = nullptr;

  tinyxml2::XMLDocument m_vehicleCatalogXOSCSysDoc;
  tinyxml2::XMLDocument m_miscObjectCatalogXOSCSysDoc;
  tinyxml2::XMLDocument m_pedestrianCatalogXOSCSysDoc;
  tinyxml2::XMLDocument m_vehicleCatalogXOSCDataDoc;
  tinyxml2::XMLDocument m_miscObjectCatalogXOSCDataDoc;
  tinyxml2::XMLDocument m_pedestrianCatalogXOSCDataDoc;
  tinyxml2::XMLDocument m_mapCatalogXMLDataDoc;

  std::unordered_map<std::string, const tinyxml2::XMLElement*> m_mapNameVehicle;

  //!
  //! @brief 成员变量：m_mapNameMisc
  //! @details 其他元素名称映射
  //!
  std::unordered_map<std::string, const tinyxml2::XMLElement*> m_mapNameMisc;

  //!
  //! @brief 成员变量：m_mapNamePedestrian
  //! @details 行人名称映射
  //!
  std::unordered_map<std::string, const tinyxml2::XMLElement*> m_mapNamePedestrian;
  std::unordered_map<std::string, const tinyxml2::XMLElement*> m_mapNameModel3d;
};
}  // namespace coordinator
}  // namespace tx_sim
