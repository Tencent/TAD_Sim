// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "sim_parser.h"
#include <regex>
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "boost/regex.hpp"
#include "glog/logging.h"
#include "json/json.h"
#include "utils/json_helper.h"
#include "xml_util.h"


using namespace tx_sim::utils;
namespace fs = boost::filesystem;


namespace tx_sim {
namespace coordinator {

//! @brief 构建传感器目录
//! @return 构建成功返回true，否则返回false
bool CSimParser::Build() {
  m_simXMLRoot = GetXMLRoot(m_simPath, m_doc);
  if (!m_simXMLRoot) return false;
  return true;
}

//! @brief 获取Ego信息
//! @return 返回一个包含Ego信息的tuple
std::tuple<std::string, std::string> CSimParser::GetEgoInfo() {
  const XMLElement* planNode = GetChildNode(m_simXMLRoot, "planner");
  const XMLElement* egoNode = TryGetChildNode(planNode, "ego");
  std::string sEgoType = egoNode ? GetNodeAttributeWithDefault(egoNode, "type", "suv") : "suv";
  std::string sEgoName = egoNode ? GetNodeAttributeWithDefault(egoNode, "name", "") : "";
  return std::make_tuple(sEgoType, sEgoName);
}

//! @brief 获取场景路径、高精度地图路径和交通路径
//! @return 返回一个包含场景路径、高精度地图路径和交通路径的tuple
std::tuple<std::string, std::string, std::string> CSimParser::GetScenePath_HadMapPath_TrafficPath() {
  fs::path scenarioPath(m_simPath);
  fs::path scenario_dir = scenarioPath.parent_path();
  const XMLElement* map_node = GetChildNode(m_simXMLRoot, "mapfile");
  fs::path map_path(scenario_dir);
  map_path /= GetNodeText(map_node, "mapfile", true);
  std::string hadmapPath = map_path.lexically_normal().make_preferred().string();

  const XMLElement* traffic_node = GetChildNode(m_simXMLRoot, "traffic");
  fs::path traffic_path(scenario_dir);
  traffic_path /= GetNodeText(traffic_node, "traffic");
  std::string trafficPath = traffic_path.lexically_normal().make_preferred().string();
  return std::make_tuple(m_simPath, hadmapPath, trafficPath);
}

//! @brief 获取环境全路径
//! @param[in] defaultXOSCPath 默认的XOSC路径
//! @return 返回环境全路径
std::string CSimParser::GetEnviromentFullPath(std::string defaultXOSCPath) {
  std::string sEnviromentFileName;
  std::string xmlPath_env;

  fs::path fsSimFile(m_simPath);

  std::string fFileName = fsSimFile.filename().lexically_normal().string();
  std::string sSimFolder = fsSimFile.parent_path().lexically_normal().string();
  std::string sSimFolderParent = fsSimFile.parent_path().parent_path().lexically_normal().string();
  if (boost::filesystem::exists(sSimFolderParent + "/sensors/config.json")) {
    Json::Value jsonRoot;
    tx_sim::utils::ReadJsonDocFromFile(jsonRoot, sSimFolderParent + "/sensors/config.json");
    bool bEnvironmentUseGlobal = jsonRoot.get("environmentUseGlobal", false).asBool();
    if (bEnvironmentUseGlobal) {
      sEnviromentFileName = jsonRoot["environmentGlobalFilePath"].asString();
      fs::path fsDefaultXOSCPath(defaultXOSCPath);
      xmlPath_env = (fsDefaultXOSCPath.parent_path().parent_path() / "/Environments/" / sEnviromentFileName)
                        .lexically_normal()
                        .string();
    }
  }
  if (xmlPath_env.empty()) {
    fs::path scenarioPath(m_simPath);
    fs::path scenario_dir = scenarioPath.parent_path();
    if (!TryGetChildNode(m_simXMLRoot, "environment")) return "";
    const XMLElement* enviroment_node = GetChildNode(m_simXMLRoot, "environment");
    fs::path enviroment_path(scenario_dir);
    const char* pEnviromentName = enviroment_node->GetText();
    if (pEnviromentName) {
      enviroment_path /= GetNodeText(enviroment_node, "environment");
      xmlPath_env = enviroment_path.lexically_normal().make_preferred().string();
    }
  }
  return xmlPath_env;
}

//! @brief 构建物理Ego
//! @param[in] pPhysicleEgo 指向物理Ego对象的指针
//! @return 构建成功返回true，否则返回false
bool CSimParser::BuildPhysicleEgo(sim_msg::PhysicleEgo& pPhysicleEgo, const tinyxml2::XMLElement* node_plan) {
  const XMLElement* pNodeVelocity_Max = TryGetChildNode(node_plan, "Velocity_Max");
  std::string sSpeed;
  if (pNodeVelocity_Max) sSpeed = GetNodeText(pNodeVelocity_Max, "Velocity_Max");

  const XMLElement* pNodeacceleration_max = TryGetChildNode(node_plan, "acceleration_max");
  std::string sAccel = "12";
  if (pNodeacceleration_max) sAccel = GetNodeText(pNodeacceleration_max, "acceleration_max");

  const XMLElement* pNoddeceleration_max = TryGetChildNode(node_plan, "deceleration_max");
  std::string sDecel = "10";
  if (pNoddeceleration_max) sDecel = GetNodeText(pNoddeceleration_max, "deceleration_max");
  pPhysicleEgo.mutable_performance()->set_max_speed(!sSpeed.empty() ? std::stod(sSpeed) : 0.0);
  pPhysicleEgo.mutable_performance()->set_max_accel(!sAccel.empty() ? std::stod(sAccel) : 0.0);
  pPhysicleEgo.mutable_performance()->set_max_decel(!sDecel.empty() ? std::stod(sDecel) : 0.0);
  return true;
}


bool CSimParser::BuildInitEgo(sim_msg::InitialEgo* pInitialEgo, const tinyxml2::XMLElement* node_plan) {
  std::string start_v = GetNodeText(GetChildNode(node_plan, "start_v"), "start_v");
  pInitialEgo->mutable_common()->set_speed(stod(start_v));

  const XMLElement* control_path_node = node_plan->FirstChildElement("ControlPath");
  const XMLElement* input_path_node = node_plan->FirstChildElement("InputPath");
  const XMLElement* route_node = node_plan->FirstChildElement("route");

  if (control_path_node) {
    std::vector<std::string> point_triple;
    std::vector<std::string> path_points;
    boost::split(path_points, GetNodeAttribute(control_path_node, "points", "ControlPath"), boost::is_any_of(";"));
    if (path_points.empty())
      throw std::invalid_argument("invalid format of InputPath points, at least 1 point(start) should be given.");
    std::string sGear;
    for (const std::string& p : path_points) {
      boost::split(point_triple, p, boost::is_any_of(","));
      sim_msg::Waypoint* pWayPoint = pInitialEgo->mutable_common()->add_waypoints();
      sim_msg::Position* pPositon = pWayPoint->mutable_position();
      sim_msg::PositionWorld* pPositonWorld = pPositon->mutable_world();
      pPositonWorld->set_x(std::stod(point_triple[0]));
      pPositonWorld->set_y(std::stod(point_triple[1]));
      pPositonWorld->set_h(point_triple.size() >= 3 ? std::stod(point_triple[2]) : 0.0);
      pWayPoint->mutable_speed()->set_value(point_triple.size() >= 4 ? std::stod(point_triple[3]) : 0.0);

      sGear = point_triple.size() >= 5 ? point_triple[4] : "";
      if ("drive" == sGear) {
        pWayPoint->set_gear(sim_msg::GEAR_STS_D);
      } else if ("reverse" == sGear) {
        pWayPoint->set_gear(sim_msg::GEAR_STS_R);
      } else {
        pWayPoint->set_gear(sim_msg::GEAR_STS_D);
      }
    }
  } else if (input_path_node) {
    std::vector<std::string> point_triple;
    std::vector<std::string> path_points;
    boost::split(path_points, GetNodeAttribute(input_path_node, "points", "InputPath"), boost::is_any_of(";"));
    if (path_points.empty())
      throw std::invalid_argument("invalid format of InputPath points, at least 1 point(start) should be given.");
    for (const std::string& p : path_points) {
      ParsePosStr(p, point_triple);
      sim_msg::Waypoint* pWayPoint = pInitialEgo->mutable_common()->add_waypoints();
      sim_msg::Position* pPositon = pWayPoint->mutable_position();
      sim_msg::PositionWorld* pPositonWorld = pPositon->mutable_world();
      pPositonWorld->set_x(std::stod(point_triple[0]));
      pPositonWorld->set_y(std::stod(point_triple[1]));
      pPositonWorld->set_h(point_triple.size() >= 3 ? std::stod(point_triple[2]) : 0.0);
    }
  } else {
    sim_msg::Waypoint* pWayPoint = pInitialEgo->mutable_common()->add_waypoints();
    sim_msg::Position* pPositon = pWayPoint->mutable_position();
    sim_msg::PositionWorld* pPositonWorld = pPositon->mutable_world();
    const XMLElement* route_node = GetChildNode(node_plan, "route");
    const XMLElement* altitude_node = GetChildNode(node_plan, "altitude");
    std::string sAltitudeStart = altitude_node->Attribute("start");
    std::string sAltitudeEnd = altitude_node->Attribute("end");

    std::vector<std::string> point_triple;
    ParsePosStr(GetNodeAttribute(route_node, "start", "route"), point_triple);
    pPositonWorld->set_x(std::stod(point_triple[0]));
    pPositonWorld->set_y(std::stod(point_triple[1]));
    pPositonWorld->set_h(std::stod(sAltitudeStart));

    pWayPoint = pInitialEgo->mutable_common()->add_waypoints();
    pPositon = pWayPoint->mutable_position();
    pPositonWorld = pPositon->mutable_world();
    ParsePosStr(GetNodeAttribute(route_node, "end", "route"), point_triple);
    pPositonWorld->set_x(std::stod(point_triple[0]));
    pPositonWorld->set_y(std::stod(point_triple[1]));
    pPositonWorld->set_h(std::stod(sAltitudeEnd));
  }
  std::string theta = GetNodeText(GetChildNode(node_plan, "theta"), "theta");
  if (pInitialEgo->mutable_common()->waypoints_size() > 0) {
    pInitialEgo->mutable_common()->mutable_waypoints(0)->mutable_heading_angle()->set_value(stod(theta));
  }

  sim_msg::Activate* pActivate = pInitialEgo->mutable_activate();
  const XMLElement* node_control_longitudinal = TryGetChildNode(node_plan, "control_longitudinal");
  std::string scontrol_longitudinal;
  if (node_control_longitudinal) scontrol_longitudinal = GetNodeText(node_control_longitudinal, "control_longitudinal");
  std::string scontrol_lateral;
  const XMLElement* node_control_lateral = TryGetChildNode(node_plan, "control_lateral");
  if (node_control_lateral) scontrol_lateral = GetNodeText(node_control_lateral, "control_lateral");
  bool bcontrol_longitudinal = (scontrol_longitudinal == "false" ? false : true);
  bool bcontrol_lateral = (scontrol_lateral == "false" ? false : true);
  sim_msg::Activate_Automode value;
  if (!bcontrol_longitudinal && !bcontrol_lateral) value = sim_msg::Activate_Automode::Activate_Automode_AUTOMODE_OFF;
  else if (bcontrol_longitudinal && !bcontrol_lateral)
    value = sim_msg::Activate_Automode::Activate_Automode_AUTOMODE_LONGITUDINAL;
  else if (!bcontrol_longitudinal && bcontrol_lateral)
    value = sim_msg::Activate_Automode::Activate_Automode_AUTOMODE_LATERAL;
  else
    value = sim_msg::Activate_Automode::Activate_Automode_AUTOMODE_AUTOPILOT;
  pActivate->set_automode(value);

  std::string strajectory_enabled;
  const XMLElement* node_trajectory_enabled = TryGetChildNode(node_plan, "trajectory_enabled");
  if (node_trajectory_enabled) strajectory_enabled = GetNodeText(node_trajectory_enabled, "trajectory_enabled");
  pInitialEgo->set_trajectory_enabled(strajectory_enabled == "true" ? true : false);

  return true;
}

bool CSimParser::BuildDynamicEgo(sim_msg::Dynamic* pDynamicEgo, const tinyxml2::XMLElement* node_plan) {
  if (!TryGetChildNode(node_plan, "scene_event")) return true;
  const XMLElement* scene_event = GetChildNode(node_plan, "scene_event");
  std::string sVersion = scene_event->Attribute("version");
  if (sVersion == "1.2.0.0") {
    ParseSceneEvent_1_2_0_0(scene_event, pDynamicEgo);
  } else {
    ParseSceneEvent(scene_event, pDynamicEgo);
  }
  return true;
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
void ParseActivate(sim_msg::Activate* pActivate, std::string sSubType) {
  if (sSubType == "off") {
    pActivate->set_automode(sim_msg::Activate_Automode::Activate_Automode_AUTOMODE_OFF);
  } else if (sSubType == "lateral") {
    pActivate->set_automode(sim_msg::Activate_Automode::Activate_Automode_AUTOMODE_LATERAL);
  } else if (sSubType == "longitudinal") {
    pActivate->set_automode(sim_msg::Activate_Automode::Activate_Automode_AUTOMODE_LONGITUDINAL);
  } else if (sSubType == "autopilot") {
    pActivate->set_automode(sim_msg::Activate_Automode::Activate_Automode_AUTOMODE_AUTOPILOT);
  }
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
void ParseAssign(sim_msg::Assign* pAssing, std::string sValue, Json::Value& multiValue) {
  pAssing->set_controller_name(sValue);
  if (multiValue.isObject()) {
    sim_msg::Assign_Cruise* pCruise = pAssing->mutable_cruise();
    pCruise->set_resume_sw(GetOnOff(multiValue["resume_sw"].asString()));
    pCruise->set_cancel_sw(GetOnOff(multiValue["cancel_sw"].asString()));
    pCruise->set_speed_inc_sw(GetOnOff(multiValue["speed_inc_sw"].asString()));
    pCruise->set_speed_dec_sw(GetOnOff(multiValue["speed_dec_sw"].asString()));
    std::string sValue = multiValue["set_timegap"].asString();
    if (sValue == "null") sValue = "-1";
    pCruise->set_set_timegap(std::stod(sValue));
    sValue = multiValue["set_speed"].asString();
    if (sValue == "null") sValue = "-1";
    pCruise->set_set_speed(std::stod(sValue));

  } else {
    pAssing->set_none("");
  }
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
void ParseOverride(sim_msg::Override* pOverride, Json::Value& multiValue) {
  sim_msg::Override_Throttle* pThrottle = pOverride->mutable_throttle();
  pThrottle->set_action(GetOnOff(multiValue["throttle"]["activate"].asString()));
  pThrottle->set_value(std::stod(multiValue["throttle"]["value"].asString()));

  sim_msg::Override_Brake* pBrake = pOverride->mutable_brake();
  pBrake->set_action(GetOnOff(multiValue["brake"]["activate"].asString()));
  pBrake->set_value(std::stod(multiValue["throttle"]["value"].asString()));

  sim_msg::Override_SteeringWheel* pSteeringWheel = pOverride->mutable_steering_wheel();
  pSteeringWheel->set_action(GetOnOff(multiValue["steering_wheel"]["activate"].asString()));
  pSteeringWheel->set_value(std::stod(multiValue["steering_wheel"]["value"].asString()));

  sim_msg::Override_ParkingBrake* pParkingBrake = pOverride->mutable_parking_brake();
  pParkingBrake->set_action(GetOnOff(multiValue["parking_brake"]["activate"].asString()));
  sim_msg::ParkingBrakeSts parkingBrakeStatus;
  switch (std::stoi(multiValue["parking_brake"]["value"].asString())) {
    case 0: parkingBrakeStatus = sim_msg::ParkingBrakeSts::PARKING_BRAKE_STS_RELEASED; break;
    case 1: parkingBrakeStatus = sim_msg::ParkingBrakeSts::PARKING_BRAKE_STS_APPLIED; break;
    case 2: parkingBrakeStatus = sim_msg::ParkingBrakeSts::PARKING_BRAKE_STS_APPLYING; break;
    case 3:
      parkingBrakeStatus = sim_msg::ParkingBrakeSts::PARKING_BRAKE_STS_RELEASING;
      break;
    defaut:
      break;
  }
  pParkingBrake->set_value(parkingBrakeStatus);

  sim_msg::Override_Clutch* pClutch = pOverride->mutable_clutch();
  pClutch->set_action(GetOnOff(multiValue["clutch"]["activate"].asString()));
  pClutch->set_value(std::stod(multiValue["clutch"]["value"].asString()));

  sim_msg::Override_Gear* pGear = pOverride->mutable_gear();
  pGear->set_action(GetOnOff(multiValue["gear"]["activate"].asString()));

  sim_msg::GearSts gearStatus;
  int nGearStatus = std::stoi(multiValue["gear"]["value"].asString());
  switch (nGearStatus) {
    case 0: gearStatus = sim_msg::GearSts::GEAR_STS_D; break;
    case 1: gearStatus = sim_msg::GearSts::GEAR_STS_P; break;
    case 2: gearStatus = sim_msg::GearSts::GEAR_STS_R; break;
    case 3: gearStatus = sim_msg::GearSts::GEAR_STS_N; break;
    case 4: gearStatus = sim_msg::GearSts::GEAR_STS_M1; break;
    case 5: gearStatus = sim_msg::GearSts::GEAR_STS_M2; break;
    case 6: gearStatus = sim_msg::GearSts::GEAR_STS_M3; break;
    case 7: gearStatus = sim_msg::GearSts::GEAR_STS_M4; break;
    case 8: gearStatus = sim_msg::GearSts::GEAR_STS_M5; break;
    case 9: gearStatus = sim_msg::GearSts::GEAR_STS_M6; break;
    case 10: gearStatus = sim_msg::GearSts::GEAR_STS_M7; break;
    default: break;
  }
  pGear->set_value(gearStatus);
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
sim_msg::Dir getDirFromStr(std::string str) {
  sim_msg::Dir dir;
  if (str == "0") {
    dir = sim_msg::Dir::DIR_LEFT;
  } else if (str == "1") {
    dir = sim_msg::Dir::DIR_RIGHT;
  } else if (str == "2") {
    dir = sim_msg::Dir::DIR_ANY;
  }
  return dir;
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
void ParseCommand(sim_msg::Command* pCommand, std::string sSubType, std::string sValue, std::string sLaneOffSet) {
  sim_msg::OnOff onoff = GetOnOff(sValue);
  if (sSubType == "lane_change") {
    sim_msg::Command_LaneChange* pCommandLaneChange = pCommand->mutable_lane_change();
    pCommandLaneChange->set_action(onoff);
    sim_msg::Dir dir = getDirFromStr(sLaneOffSet);
    pCommandLaneChange->set_value(dir);
  } else if (sSubType == "overtaking") {
    sim_msg::Command_Overtaking* pOverTaking = pCommand->mutable_overtaking();
    pOverTaking->set_action(onoff);
    sim_msg::Dir dir = getDirFromStr(sLaneOffSet);
    pOverTaking->set_value(dir);
  } else if (sSubType == "pull_over") {
    pCommand->set_pull_over(onoff);
  } else if (sSubType == "emergency_stop") {
    pCommand->set_emergency_stop(onoff);
  } else if (sSubType == "lateral_speed_to_left") {
    pCommand->set_lateral_speed_to_left(std::stod(sLaneOffSet));
  } else if (sSubType == "lateral_speed_to_right") {
    pCommand->set_lateral_speed_to_right(std::stod(sLaneOffSet));
  } else if (sSubType == "lateral_accel_to_left") {
    pCommand->set_lateral_accel_to_left(std::stod(sLaneOffSet));
  } else if (sSubType == "lateral_accel_to_right") {
    pCommand->set_lateral_accel_to_right(std::stod(sLaneOffSet));
  } else if (sSubType == "userdefine") {
    pCommand->set_user_define(sValue);
  }
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
Json::Value EncodeActionToJson(const std::string& s) {
  Json::Value actions(Json::arrayValue);
  std::string s_without_user_define_action = s;

  boost::regex re(kUserDefinePattern);
  for (boost::sregex_iterator it(s.begin(), s.end(), re), end; it != end; ++it) {
    boost::smatch ms = *it;
    if (ms.size() != kUserDefinePatternSubMatchSize + 1) { continue; }
    Json::Value one;
    for (auto i = 1; i < ms.size(); i += 2) { one[ms[i].str()] = ms[i + 1].str(); }
    actions.append(one);
    s_without_user_define_action.replace(s_without_user_define_action.find(ms[0].str()), ms[0].str().size(), "{}");
  }

  s_without_user_define_action =
      boost::regex_replace(s_without_user_define_action, boost::regex(R"([\w\-.]+)"), R"("$&")");

  Json::Value other_actions;
  ReadJsonDocFromString(other_actions, s_without_user_define_action);
  for (auto&& e : other_actions) {
    if (!e.empty()) { actions.append(e); }
  }

  return actions;
}

//! @brief 成员函数：
//! @details 成员函数功能：
//!
//! @param[in]
//! @return
void BuildEgoAction(const std::string& str, sim_msg::Event* pEventEgo) {
  Json::Value jsonValue = EncodeActionToJson(str);
  bool isArray = jsonValue.isArray();
  uint32_t eventSize = jsonValue.size();
  Json::Reader jsonReader;
  for (const Json::Value& item : jsonValue) {
    sim_msg::Action* pActionEgo = pEventEgo->add_actions();

    std::string sType = item["type"].asString();
    std::string sValue = item["value"].asString();
    std::string sSubType = item["subtype"].asString();
    std::string sLaneOffSet = item["offset"].asString();
    Json::Value multiValue = item["multi"];
    if (sType == "activate") {
      sim_msg::Activate* pActivate = pActionEgo->mutable_activate();
      ParseActivate(pActivate, sSubType);
    } else if (sType == "override") {
      sim_msg::Override* pOverride = pActionEgo->mutable_override();
      ParseOverride(pOverride, multiValue);

    } else if (sType == "assign") {
      sim_msg::Assign* pAssing = pActionEgo->mutable_assign();
      ParseAssign(pAssing, sValue, multiValue);
    } else if (sType == "command") {
      sim_msg::Command* pCommand = pActionEgo->mutable_command();
      ParseCommand(pCommand, sSubType, sValue, sLaneOffSet);
    } else if (sType == "acc") {
    } else if (sType == "velocity") {
    } else if (sType == "merge") {
    } else if (sType == "status") {
      sim_msg::Status* pStatus = pActionEgo->mutable_status();
      BuildStatus(pStatus, sSubType, sValue, sLaneOffSet);
    } else {
      LOG(ERROR) << "BuildEgoAction unknow type:" << sType;
    }
  }
}

//! @brief 解析场景事件
//! @param[in] sceneXMLNode 指向场景XML节点的指针
//! @param[in] pDynamicEgo 指向动态Ego对象的指针
void CSimParser::ParseSceneEvent(const tinyxml2::XMLElement* sceneXMLNode, sim_msg::Dynamic* pDynamicEgo) {
  sim_msg::Event* pEventEgo = pDynamicEgo->add_events();
  const XMLElement* pNode = sceneXMLNode->FirstChildElement();
  std::string tmpStr;
  while (pNode) {
    std::string sID = pNode->Attribute("id");
    pEventEgo->set_id(std::stod(sID));
    std::string sName = pNode->Attribute("name");
    pEventEgo->set_name(sName);

    std::string sInfo = pNode->Attribute("info");
    pEventEgo->set_info(sInfo);

    sim_msg::Trigger* pTrigger = pEventEgo->mutable_trigger();
    SplitStrTrigger(pNode, pTrigger);

    tmpStr = pNode->Attribute("action");
    BuildEgoAction(tmpStr, pEventEgo);

    pNode = pNode->NextSiblingElement();
  }
}

void SplitStrTrigger_1_2_0_0(const tinyxml2::XMLElement* pNode, sim_msg::Trigger* pTrigger) {
  for (int i = 1; i < 10; i++) { SplitStrTrigger(pNode, pTrigger, i); }
}

//! @brief 解析场景事件（版本1.2.0.0）
//! @param[in] sceneXMLNode 指向场景XML节点的指针
//! @param[in] pDynamicEgo 指向动态Ego对象的指针
void CSimParser::ParseSceneEvent_1_2_0_0(const tinyxml2::XMLElement* sceneXMLNode, sim_msg::Dynamic* pDynamicEgo) {
  const XMLElement* pNode = sceneXMLNode->FirstChildElement();
  std::string tmpStr;
  while (pNode) {
    sim_msg::Event* pEventEgo = pDynamicEgo->add_events();
    std::string sID = pNode->Attribute("id");
    pEventEgo->set_id(std::stod(sID));
    std::string sName = pNode->Attribute("name");
    pEventEgo->set_name(sName);

    std::string sInfo = pNode->Attribute("info");
    pEventEgo->set_info(sInfo);

    sim_msg::Trigger* pTrigger = pEventEgo->mutable_trigger();
    SplitStrTrigger_1_2_0_0(pNode, pTrigger);

    tmpStr = pNode->Attribute("action");
    BuildEgoAction(tmpStr, pEventEgo);

    pNode = pNode->NextSiblingElement();
  }
}

//! @brief 构建设置参数声明
//! @param[in] pSetting 指向设置对象的指针
void CSimParser::BuildSettingParameterDeclarations(sim_msg::Setting* pSetting) {
  const XMLElement* parameterDeclarationsNode = TryGetChildNode(m_simXMLRoot, "ParameterDeclarations");
  if (!parameterDeclarationsNode) return;
  const XMLElement* pNode = parameterDeclarationsNode->FirstChildElement();
  while (pNode) {
    sim_msg::Setting::ParameterDeclaration* pParameterDeclaration = pSetting->add_parameter_declarations();
    const char* name = pNode->Attribute("name");
    const char* type = pNode->Attribute("parameterType");
    const char* value = pNode->Attribute("value");
    pParameterDeclaration->set_name(name);
    pParameterDeclaration->set_parameter_type(type);
    pParameterDeclaration->set_value(value);
    pNode = pNode->NextSiblingElement();
  }
}

}  // namespace coordinator
}  // namespace tx_sim
