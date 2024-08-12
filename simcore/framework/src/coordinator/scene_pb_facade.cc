// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "scene_pb_facade.h"
#include <cmath>
#include <fstream>
#include <regex>
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "glog/logging.h"
#include "json/json.h"

#include "enviroment_parser.h"
#include "location.pb.h"
#include "traffic.pb.h"
#include "utils/constant.h"
#include "vehicle_geometry.pb.h"
#include "xml_util.h"

using namespace tx_sim::utils;
namespace fs = boost::filesystem;


namespace tx_sim {
namespace coordinator {
//! @brief 构造函数
//! @details 初始化类的成员变量
//!
//! @param[in] simPath 场景配置文件的路径
//! @param[in] defaultPath 默认配置文件的路径
//! @param[in] vehicleCatalogPath 车辆目录配置文件的路径
//! @param[in] miscCatalogPath 杂项目录配置文件的路径
//! @param[in] pedestrianCatalogPath 行人目录配置文件的路径
CScenePbFacade::CScenePbFacade(const std::string& simPath, const std::string& defaultXOSCSysPath,
                               const std::string& vehicleCatalogSysPath, const std::string& miscCatalogSysPath,
                               const std::string& pedestrianCatalogSysPath, const std::string& defaultXOSCDataPath,
                               const std::string& vehicleCatalogDataPath, const std::string& miscCatalogDataPath,
                               const std::string& pedestrianCatalogDataPath,
                               const std::string& mapModel3dCatalogXMLDataPath)
    : m_simPath(simPath),
      m_defaultXOSCSysPath(defaultXOSCSysPath),
      m_vehicleCatalogXOSCSysPath(vehicleCatalogSysPath),
      m_miscCatalogXOSCSysPath(miscCatalogSysPath),
      m_pedestrianCatalogXOSCSysPath(pedestrianCatalogSysPath),
      m_defaultXOSCDataPath(defaultXOSCDataPath),
      m_vehicleCatalogXOSCDataPath(vehicleCatalogDataPath),
      m_miscCatalogXOSCDataPath(miscCatalogDataPath),
      m_pedestrianCatalogXOSCDataPath(pedestrianCatalogDataPath),
      m_mapModel3dCatalogXMLDataPath(mapModel3dCatalogXMLDataPath) {}
//! @brief 析构函数
//! @details 释放类的成员变量
CScenePbFacade::~CScenePbFacade() {}
//! @brief 函数名：BuildScenePb
//! @details 函数功能：根据场景配置文件构建场景
//!
//! @param[in,out] scene 一个用于存储场景信息的Scene对象
//!
//! @return 如果成功构建场景，则返回true，否则返回false
bool CScenePbFacade::BuildScenePb(sim_msg::Scene& scene) {
  m_simParser = std::unique_ptr<CSimParser>(new CSimParser(m_simPath));
  if (!m_simParser->Build()) return false;

  m_simXMLRoot = m_simParser->SimXMLRoot();

  if (fs::is_regular_file(m_defaultXOSCDataPath)) {
    m_defaultXOSCDataParser = std::unique_ptr<CDefaultXOSCParser>(new CDefaultXOSCParser(m_defaultXOSCDataPath));
    if (!m_defaultXOSCDataParser->Build()) return false;
  }

  fs::path sensorDataPathDir(m_defaultXOSCDataPath);
  fs::path sensorDataPath(sensorDataPathDir.parent_path());
  sensorDataPath /= "../Sensors/SensorCatalog.xosc";
  std::string sSensorDataPath = sensorDataPath.lexically_normal().make_preferred().string();
  scene.mutable_setting()->set_sensor_cfg_path(sSensorDataPath);
  m_sensorDataParser = std::shared_ptr<CSensorCatalogParser>(new CSensorCatalogParser(sSensorDataPath));

  if (!m_sensorDataParser->Build()) return false;

  BuildSetting(scene);
  if (isMultiEgoVersion()) BuildEgos(scene);
  else
    BuildEgo(scene);
  BuildEnviroment(scene);
  m_trafficParser = std::unique_ptr<CTrafficParser>(new CTrafficParser(m_trafficPath));
  if (!m_trafficParser->Build()) return false;
  BuildTraffic(scene);
  return true;
}

bool CScenePbFacade::BuildCloudCityScenePb(sim_msg::Scene& scene) {
  m_trafficParser = std::unique_ptr<CTrafficParser>(new CTrafficParser(m_trafficPath));
  if (!m_trafficParser->Build()) return false;
  BuildTraffic(scene);
  return true;
}

std::string GetHadmapData(const std::string& hadmapPath) {
  std::ifstream is{hadmapPath, std::ios::binary | std::ios::ate};
  if (!is.is_open()) return "";
  auto size = is.tellg();
  std::string str(size, '\0');  // construct string to stream size
  is.seekg(0);
  is.read(&str[0], size);
  return str;
}

void CScenePbFacade::BuildSetting(sim_msg::Scene& scene) {
  auto tuplePath = m_simParser->GetScenePath_HadMapPath_TrafficPath();
  sim_msg::Setting* pSetting = scene.mutable_setting();
  pSetting->set_version("1.0.0");
  pSetting->set_scenario_path(std::get<0>(tuplePath));
  std::string hadmap_path = std::get<1>(tuplePath);
  pSetting->set_hadmap_path(hadmap_path);
  m_trafficPath = std::get<2>(tuplePath);
  LOG(INFO) << "traffic path is:" << m_trafficPath;
  if (fs::is_regular_file(m_defaultXOSCDataPath)) { pSetting->set_default_xosc_path(m_defaultXOSCDataPath); }
  if (fs::is_regular_file(m_vehicleCatalogXOSCDataPath)) {
    pSetting->set_vehicle_catalog_path(m_vehicleCatalogXOSCDataPath);
  }
  if (fs::is_regular_file(m_pedestrianCatalogXOSCDataPath)) {
    pSetting->set_pedestrian_catalog_path(m_pedestrianCatalogXOSCDataPath);
  }
  if (fs::is_regular_file(m_miscCatalogXOSCDataPath)) {
    pSetting->set_miscobject_catalog_path(m_miscCatalogXOSCDataPath);
  }
  m_enviromentPath = m_simParser->GetEnviromentFullPath(m_defaultXOSCSysPath);
  pSetting->set_environment_cfg_path(m_enviromentPath);
  std::string filecontent = GetHadmapData(hadmap_path);
  pSetting->set_hadmap_data(filecontent);
}

bool CScenePbFacade::isMultiEgoVersion() {
  const XMLElement* planner = GetChildNode(m_simXMLRoot, "planner");
  std::string sim_version = GetNodeAttributeWithDefault(planner, "version", "1.0");
  return sim_version == "2.0";
}

void CScenePbFacade::BuildEgo(sim_msg::Scene& scene) {
  sim_msg::Ego* pEgo = scene.add_egos();
  const XMLElement* egoNode = GetChildNode(m_simXMLRoot, "planner");
  std::string sEgoType = egoNode ? GetNodeAttributeWithDefault(egoNode, "type", "suv") : "suv";
  std::string sEgoName = egoNode ? GetNodeAttributeWithDefault(egoNode, "name", "") : "";

  if (sEgoType.empty() || sEgoType == "suv" || sEgoType == "car") {
    sEgoType = "suv";
  } else if (sEgoType == "truck" || sEgoType == "combination") {
    sEgoType = "truck";
  }
  if (!sEgoName.empty()) { sEgoType = sEgoName; }
  pEgo->set_name(sEgoType);
  pEgo->set_objectname("Ego");
  pEgo->set_group(tx_sim::impl::kDefaultEgoGroupName);
  ////////////////////////PhysicleEgo///////////////////////////
  m_defaultXOSCDataParser->BuildPhysicleEgo(pEgo, sEgoType);
  assert(pEgo->physicles_size() > 0);
  m_simParser->BuildPhysicleEgo(pEgo->mutable_physicles()->at(0), egoNode);

  ////////////////////////initial///////////////////////////
  sim_msg::InitialEgo* pInitialEgo = pEgo->mutable_initial();
  m_simParser->BuildInitEgo(pInitialEgo, egoNode);
  auto properTuple = m_defaultXOSCDataParser->ParseCatalogVehicleProperties(sEgoType);
  sim_msg::Assign* pAssign = pInitialEgo->mutable_assign();
  pAssign->set_controller_name(std::get<0>(properTuple));
  std::string sSensorGroup = std::get<1>(properTuple);
  std::string sDynamicGroup = std::get<2>(properTuple);
  fs::path dynamicPathDir(m_defaultXOSCSysPath);
  fs::path dynamicPath(dynamicPathDir.parent_path());
  dynamicPath /= "../Dynamics/dynamic_";
  dynamicPath += sDynamicGroup;
  dynamicPath += ".json";
  scene.mutable_setting()->set_vehicle_dynamic_cfg_path(dynamicPath.lexically_normal().make_preferred().string());

  /////////////////////////dynamic/////////////////////////////
  sim_msg::Dynamic* pDynamicEgo = pEgo->mutable_dynamic();
  m_simParser->BuildDynamicEgo(pDynamicEgo, egoNode);

  //////////////////////////SensorGroup//////////////////////////////
  sim_msg::SensorGroup* pSensorGroup = pEgo->mutable_sensor_group();
  m_sensorDataParser->BuildSensorGroup(pSensorGroup, sSensorGroup);
}

void CScenePbFacade::BuildEgos(sim_msg::Scene& scene) {
  const XMLElement* planNode = GetChildNode(m_simXMLRoot, "planner");
  const XMLElement* egolistNode = TryGetChildNode(planNode, "egolist");
  for (const XMLElement* egoNode = egolistNode->FirstChildElement("egoinfo"); egoNode != nullptr;
       egoNode = egoNode->NextSiblingElement("egoinfo")) {
    sim_msg::Ego* pEgo = scene.add_egos();
    const XMLElement* egoSubNode = egoNode->FirstChildElement("ego");
    std::string sEgoType = egoSubNode ? GetNodeAttribute(egoSubNode, "type", "ego") : "";
    std::string sEgoName = egoSubNode ? GetNodeAttribute(egoSubNode, "name", "ego") : "";
    std::string sEgoGroup = egoSubNode ? GetNodeAttribute(egoSubNode, "group", "ego") : "";
    pEgo->set_group(sEgoGroup);
    pEgo->set_objectname(sEgoType);
    pEgo->set_name(sEgoName);
    if (sEgoType.empty() || sEgoType == "suv" || sEgoType == "car") {
      sEgoType = "suv";
    } else if (sEgoType == "truck" || sEgoType == "combination") {
      sEgoType = "truck";
    }
    if (!sEgoName.empty()) { sEgoType = sEgoName; }
    ////////////////////////PhysicleEgo///////////////////////////
    m_defaultXOSCDataParser->BuildPhysicleEgo(pEgo, sEgoType);

    assert(pEgo->physicles_size() > 0);
    m_simParser->BuildPhysicleEgo(pEgo->mutable_physicles()->at(0), egoNode);

    ////////////////////////initial///////////////////////////
    sim_msg::InitialEgo* pInitialEgo = pEgo->mutable_initial();
    m_simParser->BuildInitEgo(pInitialEgo, egoNode);
    std::tuple<std::string, std::string, std::string> properTuple;

    properTuple = m_defaultXOSCDataParser->ParseCatalogVehicleProperties(sEgoType);
    sim_msg::Assign* pAssign = pInitialEgo->mutable_assign();
    pAssign->set_controller_name(std::get<0>(properTuple));
    std::string sSensorGroup = std::get<1>(properTuple);
    std::string sDynamicGroup = std::get<2>(properTuple);

    fs::path dynamicPathDir(m_defaultXOSCSysPath);
    fs::path dynamicPath(dynamicPathDir.parent_path());
    dynamicPath /= "../Dynamics/dynamic_";
    dynamicPath += sDynamicGroup;
    dynamicPath += ".json";
    // need to check!
    scene.mutable_setting()->set_vehicle_dynamic_cfg_path(dynamicPath.lexically_normal().make_preferred().string());

    /////////////////////////dynamic/////////////////////////////
    sim_msg::Dynamic* pDynamicEgo = pEgo->mutable_dynamic();
    m_simParser->BuildDynamicEgo(pDynamicEgo, egoNode);
    //////////////////////////SensorGroup//////////////////////////////
    sim_msg::SensorGroup* pSensorGroup = pEgo->mutable_sensor_group();
    m_sensorDataParser->BuildSensorGroup(pSensorGroup, sSensorGroup);
  }
}

void CScenePbFacade::BuildTraffic(sim_msg::Scene& scene) {
  m_vehicleCatalogXOSCParser = std::shared_ptr<CVehicleCatalogXOSCParser>(new CVehicleCatalogXOSCParser(
      m_vehicleCatalogXOSCSysPath, m_miscCatalogXOSCSysPath, m_pedestrianCatalogXOSCSysPath,
      m_vehicleCatalogXOSCDataPath, m_miscCatalogXOSCDataPath, m_pedestrianCatalogXOSCDataPath,
      m_mapModel3dCatalogXMLDataPath));
  if (!m_vehicleCatalogXOSCParser->Build()) return;

  m_trafficParser->BuildVehicle(scene, m_vehicleCatalogXOSCParser, m_sensorDataParser);
  m_trafficParser->BuildVru(scene, m_vehicleCatalogXOSCParser);
  m_trafficParser->BuildMisc(scene, m_vehicleCatalogXOSCParser);
  m_trafficParser->BuildInfrastructureGroup(scene);
  m_vehicleCatalogXOSCParser->BuildAllMap3dModels(scene);
}

//! @brief 函数名：BuildEnviroment
//! @details 函数功能：构建环境
//!
//! @param[in,out] scene 一个用于存储场景信息的Scene对象
void CScenePbFacade::BuildEnviroment(sim_msg::Scene& scene) {
  if (m_enviromentPath.empty()) return;
  std::unique_ptr<CEnviromentParser> envParser(new CEnviromentParser(m_enviromentPath));
  envParser->BuildEnviroment(scene.mutable_environment());
}


}  // namespace coordinator
}  // namespace tx_sim
