// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "boost/filesystem/path.hpp"
#include "scene.pb.h"
#include "sensor_catalog_parser.h"
#include "tinyxml2.h"
#include "traffic.pb.h"
#include "vehicle_catalog_xosc_parser.h"

namespace tx_sim {
namespace coordinator {

enum TRAFFIC_TYPE { XML, SIMREC };
//!
//! @brief 类名：CTrafficParser
//! @details 类功能：解析交通场景配置文件
//!
class CTrafficParser {
 public:
  //!
  //! @brief 构造函数
  //! @details 接收一个字符串参数，表示交通场景配置文件的路径
  //!
  //! @param[in] sPath 交通场景配置文件的路径
  //!
  CTrafficParser(std::string sPath) : m_trafficPath(sPath){};

  //!
  //! @brief 析构函数
  //! @details 无需执行任何操作
  //!
  ~CTrafficParser(){};

  //!
  //! @brief 函数名：Build
  //! @details 函数功能：构建交通场景
  //!
  bool Build();

  //!
  //! @brief 函数名：BuildVehicle
  //! @details 函数功能：构建车辆
  //!
  //! @param[in] scene 场景对象
  //! @param[in] pSensorParser 传感器目录解析器
  //! @param[in] pVehicleCatalogXOSCParser 车辆目录XOSC解析器
  //!
  void BuildVehicle(sim_msg::Scene& scene, std::shared_ptr<CVehicleCatalogXOSCParser> pVehicleCatalogXOSCParser,
                    std::shared_ptr<CSensorCatalogParser> pSensorDataParser);

  //!
  //! @brief 函数名：BuildVru
  //! @details 函数功能：构建VRU
  //!
  //! @param[in] scene 场景对象
  //!
  void BuildVru(sim_msg::Scene& scene, std::shared_ptr<CVehicleCatalogXOSCParser> pVehicleCatalogXOSCParser);

  //!
  //! @brief 函数名：BuildMisc
  //! @details 函数功能：构建其他元素
  //!
  //! @param[in] scene 场景对象
  //!
  void BuildMisc(sim_msg::Scene& scene, std::shared_ptr<CVehicleCatalogXOSCParser> pVehicleCatalogXOSCParser);

  //!
  //! @brief 函数名：BuildInfrastructureGroup
  //! @details 函数功能：构建基础设施组
  //!
  //! @param[in] scene 场景对象
  //!
  void BuildInfrastructureGroup(sim_msg::Scene& scene);

 private:
  //!
  //! @brief 函数名：BuildPhysicleVehicle
  //! @details 函数功能：构建物理车辆
  //!
  //! @param[in] pPhysicleVehicle 物理车辆对象指针
  //! @param[in] pNode XML节点指针
  //!
  void BuildPhysicleVehicle(sim_msg::PhysicleVehicle* pPhysicleVehicle, const tinyxml2::XMLElement* pNode);

  //!
  //! @brief 函数名：BuildInitialVehicle
  //! @details 函数功能：构建初始车辆
  //!
  //! @param[in] pInitialVehicle 初始车辆对象指针
  //! @param[in] pNode XML节点指针
  //!
  void BuildInitialVehicle(sim_msg::InitialVehicle* pInitialVehicle, const tinyxml2::XMLElement* pNode);

  //!
  //! @brief 函数名：BuildDyamicVehicle
  //! @details 函数功能：构建动态车辆
  //!
  //! @param[in] pDynamicVehicle 动态车辆对象指针
  //! @param[in] pVehicleNode XML节点指针
  //!
  void BuildDyamicVehicle(sim_msg::Dynamic* pDynamicVehicle, const tinyxml2::XMLElement* pVehicleNode);

  //!
  //! @brief 函数名：BuildSensorGroup
  //! @details 函数功能：构建传感器组
  //!
  //! @param[in] pSensorGroup 传感器组对象指针
  //! @param[in] pVehicleNode XML节点指针
  //! @param[in] pSensorParser 传感器目录解析器
  //!
  void BuildSensorGroup(sim_msg::SensorGroup* pSensorGroup, const tinyxml2::XMLElement* pVehicleNode,
                        std::shared_ptr<CSensorCatalogParser> pSensorDataParser);

  //!
  //! @brief 函数名：BuildRoutesMap
  //! @details 函数功能：构建路线映射
  //!
  void BuildRoutesMap();

  //!
  //! @brief 函数名：BuildSceneEventMap
  //! @details 函数功能：构建场景事件映射
  //!
  void BuildSceneEventMap();

  //!
  //! @brief 函数名：BuildPedestriansEventMap
  //! @details 函数功能：构建行人事件映射
  //!
  void BuildPedestriansEventMap();

  //!
  //! @brief 函数名：ParseVer
  //! @details 函数功能：解析版本信息
  //!
  //! @param[in] pSceneEventsNode XML节点指针
  //!
  void ParseVer(const tinyxml2::XMLElement* pSceneEventsNode);

  //!
  //! @brief 函数名：BuildVehicleAction
  //! @details 函数功能：构建车辆动作
  //!
  //! @param[in] pEventVehicle 事件车辆对象指针
  //! @param[in] pNode XML节点指针
  //!
  void BuildVehicleAction(sim_msg::Event* pEventVehicle, const tinyxml2::XMLElement* pNode);

  //!
  //! @brief 函数名：BuildPyhsicleVru
  //! @details 函数功能：构建物理VRU
  //!
  //! @param[in] pPhysicleVru 物理VRU对象指针
  //! @param[in] pNode XML节点指针
  //!
  void BuildPyhsicleVru(sim_msg::PhysicleVru* pPhysicleVru, const tinyxml2::XMLElement* pNode);

  //!
  //! @brief 函数名：BuildInitialVru
  //! @details 函数功能：构建初始VRU
  //!
  //! @param[in] pInitialVru 初始VRU对象指针
  //! @param[in] pNode XML节点指针
  //!
  void BuildInitialVru(sim_msg::InitialVru* pInitialVru, const tinyxml2::XMLElement* pNode);

  //!
  //! @brief 函数名：BuildDynamicVru
  //! @details 函数功能：构建动态VRU
  //!
  //! @param[in] pDynamicVru 动态VRU对象指针
  //! @param[in] pNode XML节点指针
  //!
  void BuildDynamicVru(sim_msg::Dynamic* pDynamicVru, const tinyxml2::XMLElement* pNode);

  //!
  //! @brief 函数名：BuildPyhsicleMisc
  //! @details 函数功能：构建物理其他元素
  //!
  //! @param[in] pPhysicleMisc 物理其他元素对象指针
  //! @param[in] pNode XML节点指针
  //!
  void BuildPyhsicleMisc(sim_msg::PhysicleCommon* pPhysicleMisc, const tinyxml2::XMLElement* pNode);

  //!
  //! @brief 函数名：BuildInitialMisc
  //! @details 函数功能：构建初始其他元素
  //!
  //! @param[in] pInitialMisc 初始其他元素对象指针
  //! @param[in] pNode XML节点指针
  //!
  void BuildInitialMisc(sim_msg::InitialCommon* pInitialMisc, const tinyxml2::XMLElement* pNode);

  //!
  //! @brief 成员变量：m_trafficPath
  //! @details 交通场景配置文件的路径
  //!
  std::string m_trafficPath;

  //!
  //! @brief 成员变量：m_trafficXMLRoot
  //! @details 交通场景配置文件的根节点
  //!
  const tinyxml2::XMLElement* m_trafficXMLRoot = nullptr;

  //!
  //! @brief 成员变量：m_doc
  //! @details XML文档对象
  //!
  tinyxml2::XMLDocument m_doc;

  //!
  //! @brief 成员变量：m_mapRoutes
  //! @details 路线映射
  //!
  std::map<std::string, const tinyxml2::XMLElement*> m_mapRoutes;

  //!
  //! @brief 成员变量：m_mapSceneEvent
  //! @details 场景事件映射
  //!
  std::map<std::string, const tinyxml2::XMLElement*> m_mapSceneEvent;

  //!
  //! @brief 成员变量：m_mapPedestriansEvent
  //! @details 行人事件映射
  //!
  std::map<std::string, std::vector<const tinyxml2::XMLElement*>> m_mapPedestriansEvent;

  //!
  //! @brief 成员变量：m_sSceneEventVersion
  //! @details 场景事件版本信息
  //!
  std::string m_sSceneEventVersion;

  //!
  //! @brief 成员变量：m_pVehicleCatalogXOSCParser
  //! @details 车辆目录XOSC解析器
  //!
  std::shared_ptr<CVehicleCatalogXOSCParser> m_pVehicleCatalogXOSCParser;

  //!
  //! @brief 成员变量：m_trafficType
  //! @details 交通类型，默认为XML
  //!
  TRAFFIC_TYPE m_trafficType = TRAFFIC_TYPE::XML;
};

}  // namespace coordinator
}  // namespace tx_sim
