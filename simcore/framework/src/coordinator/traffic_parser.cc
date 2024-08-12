// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "traffic_parser.h"
#include <regex>
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "json/json.h"
#include "xml_util.h"

namespace fs = boost::filesystem;


using namespace tx_sim::utils;
namespace tx_sim {
namespace coordinator {

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
sim_msg::BehaviorType GetBehaviorType(std::string sType) {
  if (sType == "TrafficVehicle") return sim_msg::BehaviorType::BEHAVIOR_TYPE_TRAFFIC_VEHICLE;
  else if (sType == "TrafficVehicleArterial")
    return sim_msg::BehaviorType::BEHAVIOR_TYPE_TRAFFIC_VEHICLE_ARTERIAL;
  else if (sType == "TrajectoryFollow")
    return sim_msg::BehaviorType::BEHAVIOR_TYPE_TRAJECTORY_FOLLOW;
  else if (sType == "UserDefine")
    return sim_msg::BehaviorType::BEHAVIOR_TYPE_USER_DEFINE;
  return sim_msg::BehaviorType::BEHAVIOR_TYPE_USER_DEFINE;
}

//!
//! @brief 函数名：Build
//! @details 函数功能：构建交通场景
//!
bool CTrafficParser::Build() {
  fs::path traffic_file(m_trafficPath);
  std::string ext = traffic_file.extension().string();
  if (ext == ".xml") {
    m_trafficXMLRoot = GetXMLRoot(m_trafficPath, m_doc);
    if (!m_trafficXMLRoot) return false;
    BuildRoutesMap();
    // BuildSceneEventMap();
    BuildPedestriansEventMap();
    m_trafficType = TRAFFIC_TYPE::XML;
  } else {
    m_trafficType = TRAFFIC_TYPE::SIMREC;
  }
  return true;
}


//!
//! @brief 函数名：ParseVer
//! @details 函数功能：解析版本信息
//!
//! @param[in] pSceneEventsNode XML节点指针
//!
void CTrafficParser::ParseVer(const tinyxml2::XMLElement* pSceneEventsNode) {
  const XMLElement* pSceneEvent = pSceneEventsNode->FirstChildElement();
  std::string sEventID;
  while (pSceneEvent) {
    sEventID = pSceneEvent->Attribute("id");
    m_mapSceneEvent[sEventID] = pSceneEvent;
    pSceneEvent = pSceneEvent->NextSiblingElement();
  }
}

//!
//! @brief 函数名：BuildPedestriansEventMap
//! @details 函数功能：构建行人事件映射
//!
void CTrafficParser::BuildSceneEventMap() {
  if (!TryGetChildNode(m_trafficXMLRoot, "scene_event")) return;
  const tinyxml2::XMLElement* pSceneEventsNode = GetChildNode(m_trafficXMLRoot, "scene_event");
  std::string sSceneEventVer = pSceneEventsNode->Attribute("version");
  if (sSceneEventVer == "1.0.0.0" || sSceneEventVer == "1.1.0.0") {
    ParseVer(pSceneEventsNode);
  } else {
    throw std::invalid_argument(std::string("unknow traffic version:") + sSceneEventVer);
  }
  m_sSceneEventVersion = sSceneEventVer;
}

//!
//! @brief 函数名：BuildRoutesMap
//! @details 函数功能：构建路线映射
//!
void CTrafficParser::BuildRoutesMap() {
  const tinyxml2::XMLElement* pRoutesNode = GetChildNode(m_trafficXMLRoot, "routes");
  const XMLElement* pRouteNode = pRoutesNode->FirstChildElement();
  std::string sRouteID;
  while (pRouteNode) {
    sRouteID = pRouteNode->Attribute("id");
    m_mapRoutes[sRouteID] = pRouteNode;
    pRouteNode = pRouteNode->NextSiblingElement();
  }
}

//!
//! @brief 函数名：BuildPedestriansEventMap
//! @details 函数功能：构建行人事件映射
//!
void CTrafficParser::BuildPedestriansEventMap() {
  const tinyxml2::XMLElement* pRoutesNode = GetChildNode(m_trafficXMLRoot, "pedestrians_event");
  const XMLElement* pRouteNode = pRoutesNode->FirstChildElement();
  std::string sRouteID;
  while (pRouteNode) {
    sRouteID = pRouteNode->Attribute("id");
    m_mapPedestriansEvent[sRouteID].push_back(pRouteNode);
    pRouteNode = pRouteNode->NextSiblingElement();
  }
}

//!
//! @brief 函数名：BuildVehicle
//! @details 函数功能：构建车辆
//!
//! @param[in] scene 场景对象
//! @param[in] pSensorParser 传感器目录解析器
//! @param[in] pVehicleCatalogXOSCParser 车辆目录XOSC解析器
//!
void CTrafficParser::BuildVehicle(sim_msg::Scene& scene,
                                  std::shared_ptr<CVehicleCatalogXOSCParser> pVehicleCatalogXOSCParser,
                                  std::shared_ptr<CSensorCatalogParser> pSensorDataParser) {
  m_pVehicleCatalogXOSCParser = pVehicleCatalogXOSCParser;

  if (m_trafficType == TRAFFIC_TYPE::XML) {
    const tinyxml2::XMLElement* pVehiclesNode = GetChildNode(m_trafficXMLRoot, "vehicles");
    const XMLElement* pNode = pVehiclesNode->FirstChildElement();
    while (pNode) {
      sim_msg::Vehicle* pVerhicle = scene.add_vehicles();
      std::string sVehicleType = pNode->Attribute("vehicleType");
      pVerhicle->set_objectname(sVehicleType);
      pVehicleCatalogXOSCParser->BuildCategoryVehicle(pVerhicle, sVehicleType);
      std::string sID = pNode->Attribute("id");
      pVerhicle->set_id(std::stol(sID));

      sim_msg::PhysicleVehicle* pPhysicleVehicle = pVerhicle->mutable_physicle();
      BuildPhysicleVehicle(pPhysicleVehicle, pNode);

      sim_msg::InitialVehicle* pInitialVehicle = pVerhicle->mutable_initial();
      BuildInitialVehicle(pInitialVehicle, pNode);

      // sim_msg::DynamicVehicle* pDynamicVehicle = pVerhicle->mutable_dynamic();
      // BuildDyamicVehicle(pDynamicVehicle, pNode);
      sim_msg::SensorGroup* pSensorGroup = pVerhicle->mutable_sensor_group();
      BuildSensorGroup(pSensorGroup, pNode, pSensorDataParser);
      pNode = pNode->NextSiblingElement();
    }
  } else {
    m_pVehicleCatalogXOSCParser->BuildAllVehicles(scene);
  }
}

//!
//! @brief 函数名：BuildVru
//! @details 函数功能：构建VRU
//!
//! @param[in] scene 场景对象
//!
void CTrafficParser::BuildVru(sim_msg::Scene& scene,
                              std::shared_ptr<CVehicleCatalogXOSCParser> pCVehicleCatalogXOSCParser) {
  if (m_trafficType != TRAFFIC_TYPE::XML) return;
  const tinyxml2::XMLElement* pVehiclesNode = GetChildNode(m_trafficXMLRoot, "pedestrians");
  const XMLElement* pNode = pVehiclesNode->FirstChildElement();
  while (pNode) {
    sim_msg::VulnerableRoadUser* pVru = scene.add_vrus();
    std::string sType = pNode->Attribute("type");
    pVru->set_objectname(sType);

    pCVehicleCatalogXOSCParser->BuildCategoryVru(pVru, sType);
    // none common
    sim_msg::PhysicleVru* pPhysicleVru = pVru->mutable_physicle();
    BuildPyhsicleVru(pPhysicleVru, pNode);

    sim_msg::InitialVru* pInitialVru = pVru->mutable_initial();
    BuildInitialVru(pInitialVru, pNode);

    sim_msg::Dynamic* pDynamicVru = pVru->mutable_dynamic();
    BuildDynamicVru(pDynamicVru, pNode);

    pNode = pNode->NextSiblingElement();
  }
}

//!
//! @brief 函数名：BuildDynamicVru
//! @details 函数功能：构建动态VRU
//!
//! @param[in] pDynamicVru 动态VRU对象指针
//! @param[in] pNode XML节点指针
//!
void CTrafficParser::BuildDynamicVru(sim_msg::Dynamic* pDynamicVru, const tinyxml2::XMLElement* pNode) {
  std::string sPedestrianID = pNode->Attribute("id");
  std::vector<const tinyxml2::XMLElement*>& vecPedestriansEventNodes = m_mapPedestriansEvent[sPedestrianID];
  for (auto& vecItemNode : vecPedestriansEventNodes) {
    std::string sNodeName = vecItemNode->Name();
    std::string sItemNodeID = vecItemNode->Attribute("id");
    std::string s_time_velocity_profile = vecItemNode->Attribute("profile");
    std::vector<std::string> sVecProfileItem1;
    boost::split(sVecProfileItem1, s_time_velocity_profile, boost::is_any_of(";"));
    if (sNodeName == "time_velocity") {
      for (auto& sVecProfileValueItem : sVecProfileItem1) {
        sim_msg::Event* pEventVru = pDynamicVru->add_events();
        pEventVru->set_id(std::stoi(sItemNodeID));
        sim_msg::Trigger* pTrigger = pEventVru->mutable_trigger();
        sim_msg::Condition* pCondition = pTrigger->add_conditions();
        std::vector<std::string> sVecProfileItem2;
        boost::split(sVecProfileItem2, sVecProfileValueItem, boost::is_any_of(","));
        std::string sTriggetTime = sVecProfileItem2[0];
        std::string sDirection = sVecProfileItem2[1];
        std::string sSpeed = sVecProfileItem2[2];
        sim_msg::ConditionSimulationTime* pConditionSimulationTime = pCondition->mutable_simulation_time();
        pConditionSimulationTime->set_value(std::stod(sTriggetTime));
        sim_msg::Action* pActionVru = pEventVru->add_actions();
        pActionVru->mutable_speed()->set_value(std::stod(sSpeed));
      }
    } else if (sNodeName == "event_velocity") {
      for (auto& sVecProfileValueItem : sVecProfileItem1) {
        sim_msg::Event* pEventVru = pDynamicVru->add_events();
        pEventVru->set_id(std::stoi(sItemNodeID));
        sim_msg::Trigger* pTrigger = pEventVru->mutable_trigger();
        sim_msg::Condition* pCondition = pTrigger->add_conditions();
        std::vector<std::string> sVecProfileItem2;
        boost::split(sVecProfileItem2, sVecProfileValueItem, boost::is_any_of(", "));
        std::string sTriggerType = sVecProfileItem2[0];
        std::string sLocationType = sVecProfileItem2[1];
        std::string sTriggetValue = sVecProfileItem2[2];
        std::string sDirection = sVecProfileItem2[3];
        std::string sSpeed = sVecProfileItem2[4];
        std::string sTriggerTime = sVecProfileItem2[5];
        sTriggerTime = sTriggerTime.substr(1, sTriggerTime.length() - 2);
        sim_msg::DistanceType distanceType;
        if (sLocationType == "euclideandistance") {
          distanceType = sim_msg::DistanceType::DISTANCE_TYPE_EUCLIDEAN;
        } else if (sLocationType == "laneprojection") {
          distanceType = sim_msg::DistanceType::DISTANCE_TYPE_LANEPROJECTION;
        }

        pEventVru->add_actions()->mutable_speed()->set_value(std::stod(sSpeed));
        if (sTriggerType == "ttc") {
          pCondition->set_count(std::stod(sTriggerTime));
          sim_msg::ConditionTimeToCollision* pConditionTimeToCollision = pCondition->mutable_ttc();
          pConditionTimeToCollision->set_value(std::stod(sTriggetValue));
          pConditionTimeToCollision->set_distance_type(distanceType);
        } else if (sTriggerType == "egoDistance") {
          sim_msg::ConditionRelativeDistance* pdistance = pCondition->mutable_relative_distance();
          pdistance->set_distance_type(distanceType);
          pdistance->set_value(std::stod(sTriggetValue));
        }
      }
    }
  }
}

//!
//! @brief 函数名：BuildPyhsicleVru
//! @details 函数功能：构建物理VRU
//!
//! @param[in] pPhysicleVru 物理VRU对象指针
//! @param[in] pNode XML节点指针
//!
void CTrafficParser::BuildPyhsicleVru(sim_msg::PhysicleVru* pPhysicleVru, const tinyxml2::XMLElement* pNode) {
  std::string sMaxSpeed = pNode->Attribute("max_v");
  sim_msg::Performance* pPerformance = pPhysicleVru->mutable_performance();
  pPerformance->set_max_accel(std::stod(sMaxSpeed));

  std::string sType = pNode->Attribute("type");
  m_pVehicleCatalogXOSCParser->BuildPhysicleCommon(pPhysicleVru->mutable_common(), sType, "pedestrians");
}

//!
//! @brief 函数名：BuildInitialVru
//! @details 函数功能：构建初始VRU
//!
//! @param[in] pInitialVru 初始VRU对象指针
//! @param[in] pNode XML节点指针
//!
void CTrafficParser::BuildInitialVru(sim_msg::InitialVru* pInitialVru, const tinyxml2::XMLElement* pNode) {
  std::string sSpeed = pNode->Attribute("start_v");
  pInitialVru->mutable_common()->set_speed(std::stod(sSpeed));

  std::string sVehicleRoutID = pNode->Attribute("routeID");
  const XMLElement* pRouteNode = m_mapRoutes[sVehicleRoutID];
  if (!pRouteNode) return;
  std::string sStartPoint = pRouteNode->Attribute("start");
  sim_msg::Waypoint* pWayPoint = pInitialVru->mutable_common()->add_waypoints();
  AddWayPointByWorld(sStartPoint, pWayPoint);

  std::string sEndPoint = GetNodeAttributeWithDefault(pRouteNode, "end", "");
  if (!sEndPoint.empty()) {
    sim_msg::Waypoint* pWayPoint = pInitialVru->mutable_common()->add_waypoints();
    AddWayPointByWorld(sEndPoint, pWayPoint);
  }
}

//!
//! @brief 函数名：BuildMisc
//! @details 函数功能：构建其他元素
//!
//! @param[in] scene 场景对象
//!
void CTrafficParser::BuildMisc(sim_msg::Scene& scene,
                               std::shared_ptr<CVehicleCatalogXOSCParser> pVehicleCatalogXOSCParser) {
  if (m_trafficType != TRAFFIC_TYPE::XML) return;

  const tinyxml2::XMLElement* pObstaclesNodes = GetChildNode(m_trafficXMLRoot, "obstacles");
  const XMLElement* pNode = pObstaclesNodes->FirstChildElement();
  while (pNode) {
    std::string sID = pNode->Attribute("id");
    std::string sType = pNode->Attribute("vehicleType");
    sim_msg::MiscellaneousObject* pMisc = scene.add_miscs();
    pMisc->set_id(std::stoi(sID));
    pMisc->set_objectname(sType);
    pVehicleCatalogXOSCParser->BuildCategoryMisc(pMisc, sType);

    sim_msg::PhysicleCommon* pPhysicleMisc = pMisc->mutable_physicle();
    BuildPyhsicleMisc(pPhysicleMisc, pNode);


    sim_msg::InitialCommon* pInitialMisc = pMisc->mutable_initial();
    BuildInitialMisc(pInitialMisc, pNode);
    pNode = pNode->NextSiblingElement();
  }
}

//!
//! @brief 函数名：BuildInfrastructureGroup
//! @details 函数功能：构建基础设施组
//!
//! @param[in] scene 场景对象
//!
void CTrafficParser::BuildInfrastructureGroup(sim_msg::Scene& scene) {
  if (m_trafficType != TRAFFIC_TYPE::XML) return;

  const tinyxml2::XMLElement* pSignlightsNode = TryGetChildNode(m_trafficXMLRoot, "signlights");
  if (!pSignlightsNode) return;
  sim_msg::InfrastructureGroup* pInfrastructureGroup = scene.mutable_infrastructure_group();
  const char* pActivePlan = pSignlightsNode->Attribute("activePlan");
  if (pActivePlan && !std::string(pActivePlan).empty()) pInfrastructureGroup->set_active_plan(std::stoi(pActivePlan));
  else
    pInfrastructureGroup->set_active_plan(0);
  const XMLElement* pNode = pSignlightsNode->FirstChildElement();
  while (pNode) {
    sim_msg::Infrastructure* pInfrastructure = pInfrastructureGroup->add_infrastructure();
    pInfrastructure->set_type(sim_msg::InfrastructureType::INFRASTRUCTURE_TYPE_TRAFFIC_SIGNAL);
    const XMLAttribute* pNodeAttr = pNode->FirstAttribute();
    while (pNodeAttr) {
      const char* pName = pNodeAttr->Name();
      const char* pValue = pNodeAttr->Value();
      if (std::string(pName) == "id") {
        pInfrastructure->mutable_extrinsic()->set_id(std::stoi(pValue));
      } else if (std::string(pName) == "routID") {
        const XMLElement* pRouteNode = m_mapRoutes[pValue];
        if (!pRouteNode) continue;
        std::string sStartPoint = pRouteNode->Attribute("start");
        std::vector<std::string> point_triple;
        ParsePosStr(sStartPoint, point_triple);
        pInfrastructure->mutable_waypoint()->mutable_position()->mutable_world()->set_x(std::stod(point_triple[0]));
        pInfrastructure->mutable_waypoint()->mutable_position()->mutable_world()->set_y(std::stod(point_triple[1]));
      }
      pInfrastructure->mutable_intrinsic()->mutable_params()->insert({pName, pValue});
      pNodeAttr = pNodeAttr->Next();
    }

    pNode = pNode->NextSiblingElement();
  }
}

//!
//! @brief 函数名：BuildDynamicVru
//! @details 函数功能：构建动态VRU
//!
//! @param[in] pDynamicVru 动态VRU对象指针
//! @param[in] pNode XML节点指针
//!
void CTrafficParser::BuildPyhsicleMisc(sim_msg::PhysicleCommon* pPhysicleMisc, const tinyxml2::XMLElement* pNode) {
  std::string sVehicleType = pNode->Attribute("vehicleType");
  m_pVehicleCatalogXOSCParser->BuildPhysicleCommon(pPhysicleMisc, sVehicleType, "obstacles");
}

//!
//! @brief 函数名：BuildInitialMisc
//! @details 函数功能：构建初始其他元素
//!
//! @param[in] pInitialMisc 初始其他元素对象指针
//! @param[in] pNode XML节点指针
//!
void CTrafficParser::BuildInitialMisc(sim_msg::InitialCommon* pInitialMisc, const tinyxml2::XMLElement* pNode) {
  std::string slaneID = pNode->Attribute("laneID");
  std::string sStart_s = pNode->Attribute("start_s");
  std::string sLoffset = pNode->Attribute("l_offset");
  pInitialMisc->set_speed(std::stod(sStart_s));

  sim_msg::Waypoint* pWaypoint = pInitialMisc->add_waypoints();
  pWaypoint->set_id(0);
}

//!
//! @brief 函数名：BuildPhysicleVehicle
//! @details 函数功能：构建物理车辆
//!
//! @param[in] pPhysicleVehicle 物理车辆对象指针
//! @param[in] pNode XML节点指针
//!
void CTrafficParser::BuildPhysicleVehicle(sim_msg::PhysicleVehicle* pPhysicleVehicle,
                                          const tinyxml2::XMLElement* pNode) {
  std::string sVehicleType = pNode->Attribute("vehicleType");
  sim_msg::PhysicleCommon* pPhysicleCommon = pPhysicleVehicle->mutable_common();
  m_pVehicleCatalogXOSCParser->BuildPhysicleCommon(pPhysicleCommon, sVehicleType, "vehicle");


  sim_msg::Performance* pPerformance = pPhysicleVehicle->mutable_performance();
  std::string sMaxSpeed = pNode->Attribute("max_v");
  pPerformance->set_max_speed(std::stod(sMaxSpeed));

  sim_msg::VehicleGeometory* pPhysicleGeometory = pPhysicleVehicle->mutable_geometory();
  m_pVehicleCatalogXOSCParser->BuildPhysicleVehicleGeometory(pPhysicleGeometory, sVehicleType);
}

//!
//! @brief 函数名：BuildInitialVehicle
//! @details 函数功能：构建初始车辆
//!
//! @param[in] pInitialVehicle 初始车辆对象指针
//! @param[in] pNode XML节点指针
//!
void CTrafficParser::BuildInitialVehicle(sim_msg::InitialVehicle* pInitialVehicle, const tinyxml2::XMLElement* pNode) {
  std::string sSpeed = pNode->Attribute("start_v");
  pInitialVehicle->mutable_common()->set_speed(std::stod(sSpeed));

  std::string sVehavior = pNode->Attribute("behavior");
  sim_msg::BehaviorType behaviorType = GetBehaviorType(sVehavior);
  pInitialVehicle->set_behavior_type(behaviorType);

  std::string sVehicleRoutID = pNode->Attribute("routeID");
  const XMLElement* pRouteNode = m_mapRoutes[sVehicleRoutID];
  if (!pRouteNode) return;
  std::string sStartPoint = pRouteNode->Attribute("start");
  sim_msg::Waypoint* pWayPoint = pInitialVehicle->mutable_common()->add_waypoints();
  AddWayPointByWorld(sStartPoint, pWayPoint);
  std::string sEndPoint = GetNodeAttributeWithDefault(pRouteNode, "end", "");
  if (!sEndPoint.empty()) {
    pWayPoint = pInitialVehicle->mutable_common()->add_waypoints();
    AddWayPointByWorld(sEndPoint, pWayPoint);
  }
}

//!
//! @brief 函数名：BuildSensorGroup
//! @details 函数功能：构建传感器组
//!
//! @param[in] pSensorGroup 传感器组对象指针
//! @param[in] pVehicleNode XML节点指针
//! @param[in] pSensorParser 传感器目录解析器
//!
void CTrafficParser::BuildSensorGroup(sim_msg::SensorGroup* pSensorGroup, const tinyxml2::XMLElement* pVehicleNode,
                                      std::shared_ptr<CSensorCatalogParser> pSensorDataParser) {
  if (!pVehicleNode) return;
  std::string sSensorGroup = GetNodeAttributeWithDefault(pVehicleNode, "sensorGroup", "");
  if (sSensorGroup.empty()) return;
  pSensorDataParser->BuildSensorGroup(pSensorGroup, sSensorGroup);
}

//!
//! @brief 函数名：BuildDyamicVehicle
//! @details 函数功能：构建动态车辆
//!
//! @param[in] pDynamicVehicle 动态车辆对象指针
//! @param[in] pVehicleNode XML节点指针
//!
void CTrafficParser::BuildDyamicVehicle(sim_msg::Dynamic* pDynamicVehicle, const tinyxml2::XMLElement* pVehicleNode) {
  const char* pEventIDs = pVehicleNode->Attribute("eventId");
  if (pEventIDs == nullptr) return;
  std::vector<std::string> vecIDs;
  boost::split(vecIDs, pEventIDs, boost::is_any_of(","));
  for (auto& vecItem : vecIDs) {
    const tinyxml2::XMLElement* pSceneEventNode = m_mapSceneEvent[vecItem];
    if (!pSceneEventNode) continue;
    sim_msg::Event* pEventVehicle = pDynamicVehicle->add_events();
    std::string sID = pSceneEventNode->Attribute("id");
    pEventVehicle->set_id(std::stod(sID));
    std::string sName = pSceneEventNode->Attribute("name");
    pEventVehicle->set_name(sName);

    std::string sInfo = pSceneEventNode->Attribute("info");
    pEventVehicle->set_info(sInfo);

    sim_msg::Trigger* pTrigger = pEventVehicle->mutable_trigger();
    SplitStrTrigger(pSceneEventNode, pTrigger);


    BuildVehicleAction(pEventVehicle, pSceneEventNode);
  }
}

//!
//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[in]
//!
Json::Value ParseActionFromStrToJson(std::string sSceneEventVersion, std::string sAction) {
  std::regex vowel_re(R"([\w-.]+)");
  Json::Value jsonValue;
  Json::CharReaderBuilder builder;
  JSONCPP_STRING err;

  if ("1.0.0.0" == sSceneEventVersion) {
    std::vector<std::string> vecActionStr;
    boost::split(vecActionStr, sAction, boost::is_any_of(";"));
    for (auto vecItemStr : vecActionStr) {
      Json::Value jsonItemValue;
      std::string jsonItemStr = std::regex_replace(vecItemStr, vowel_re, "\"$&\"");
      jsonItemStr = "{" + jsonItemStr + "}";
      const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
      if (!reader->parse(jsonItemStr.c_str(), jsonItemStr.c_str() + jsonItemStr.length(), &jsonItemValue, &err)) {
        std::cout << err << std::endl;
        throw std::invalid_argument(err);
      }
      std::string sType = jsonItemValue["type"].asString();

      jsonValue.append(jsonItemValue);
    }
  } else if ("1.1.0.0" == sSceneEventVersion) {
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(sSceneEventVersion.c_str(), sSceneEventVersion.c_str() + sSceneEventVersion.length(), &jsonValue,
                       &err)) {
      std::cout << err << std::endl;
      throw std::invalid_argument(err);
    }
  } else {
    throw std::invalid_argument(std::string("unknow traffic version:") + sSceneEventVersion);
  }
  return jsonValue;
}


//!
//! @brief 函数名：BuildVehicleAction
//! @details 函数功能：构建车辆动作
//!
//! @param[in] pEventVehicle 事件车辆对象指针
//! @param[in] pNode XML节点指针
//!
void CTrafficParser::BuildVehicleAction(sim_msg::Event* pEventVehicle, const tinyxml2::XMLElement* pNode) {
  std::string stdAction = pNode->Attribute("action");
  std::string sEndConditon = pNode->Attribute("endCondition");
  if (stdAction.empty() || sEndConditon.empty()) return;
  std::regex vowel_re(R"([\w-.]+)");

  std::string jsonStr = std::regex_replace(stdAction, vowel_re, "\"$&\"");
  Json::Value jsonValue = ParseActionFromStrToJson(m_sSceneEventVersion, stdAction);

  bool isArray = jsonValue.isArray();
  int eventSize = jsonValue.size();
  std::vector<std::tuple<std::string, std::string>> vecEndCondition = ParseEndCondition(sEndConditon);

  for (int jsonIndex = 0; jsonIndex < eventSize; jsonIndex++) {
    const Json::Value& item = jsonValue[jsonIndex];
    sim_msg::Action* pActionVehicle = pEventVehicle->add_actions();

    std::string sType = item["type"].asString();
    std::string sValue = item["value"].asString();
    std::string sSubType = item["subtype"].asString();
    std::string sLaneOffSet = item["offset"].asString();
    Json::Value multiValue = item["multi"];
    if (sType == "velocity") {
      sim_msg::ActionSpeed* pActionSpeed = pActionVehicle->mutable_speed();
      pActionSpeed->set_value(std::stod(sValue));
    } else if (sType == "acc") {
      sim_msg::ActionAccel* pActionAccel = pActionVehicle->mutable_accel();
      pActionAccel->set_value(std::stod(sValue));
      std::string sEndConditionType = std::get<0>(vecEndCondition[jsonIndex]);
      sim_msg::ActionAccel::EndType endType = GetEndType(sEndConditionType);
      pActionAccel->set_end_type(endType);

      std::string sEndConditonValue = std::get<1>(vecEndCondition[jsonIndex]);
      pActionAccel->set_end_value(std::stod(sEndConditonValue));
    } else if (sType == "merge") {
      sim_msg::ActionLaneChange* pActionLaneChange = pActionVehicle->mutable_lane_change();
      pActionLaneChange->set_duration(std::stod(sValue));
      if (abs(std::stod(sLaneOffSet)) < 0.0000001) {
        if (sSubType == "left") {
          pActionLaneChange->set_dir(sim_msg::ActionLaneChange_Dir::ActionLaneChange_Dir_DIR_LEFT_IN_LANE);
        } else {
          pActionLaneChange->set_dir(sim_msg::ActionLaneChange_Dir::ActionLaneChange_Dir_DIR_RIGHT_IN_LANE);
        }
      } else {
        if (sSubType == "left")
          pActionLaneChange->set_dir(sim_msg::ActionLaneChange_Dir::ActionLaneChange_Dir_DIR_LEFT);
        else if (sSubType == "right")
          pActionLaneChange->set_dir(sim_msg::ActionLaneChange_Dir::ActionLaneChange_Dir_DIR_RIGHT);
      }

    } else if (sType == "status") {
      sim_msg::Status* pStatus = pActionVehicle->mutable_status();
      BuildStatus(pStatus, sSubType, sValue, sLaneOffSet);
    }
  }
}

}  // namespace coordinator
}  // namespace tx_sim
