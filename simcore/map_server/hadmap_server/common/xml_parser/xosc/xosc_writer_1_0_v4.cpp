/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/xosc/xosc_writer_1_0_v4.h"
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "common/engine/math/utils.h"
#include "common/xml_parser/entity/catalog.h"
#include "common/xml_parser/entity/environments/environment.h"
#include "common/xml_parser/entity/traffic.h"
#include "common/xml_parser/entity/vehicle.h"
#include "engine/config.h"
#include "xml_parser/entity/parser.h"
#include "xml_parser/entity/simulation.h"
using namespace std::chrono;
using std::map;
using std::string;
using std::vector;
XOSCWriter_1_0_v4::XOSCWriter_1_0_v4() {}

XOSCWriter_1_0_v4::~XOSCWriter_1_0_v4() {}

using namespace osc;

int XOSCWriter_1_0_v4::Save2XOSC(const char* strFileName, sTagSimuTraffic& scene) {
  CTraffic& traffic = scene.m_traffic;  //
  CSimulation& simulation = scene.m_simulation;
  CEnvironment& environment = scene.m_environment;
  m_strCatalogDir = scene.getCatalogDir();
  if (CanSaveXOSC(simulation) != 0) {
    SYSTEM_LOGGER_ERROR("Can not save to xosc format");
    return Error_MapFormat;
  }
  int ret = 0;
  ret = InitMapQuery(simulation);
  if (ret != 0) {
    return Error_MapLost;
  }
  XODRCoordinator(simulation);

  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

  string desc = simulation.m_strInfo;
  osc::FileHeader fh;
  fh._author = "TAD Sim V2.2";
  fh._date = boost::posix_time::to_iso_extended_string(now);
  fh._description = desc;
  fh._revMajor = "1";
  fh._revMinor = "0";
  osc::OpenScenario xosc;
  xosc.sub_FileHeader = fh;

  // set parameter declarations
  SaveParameterDeclaration(xosc, simulation, traffic);

  // set catalog locations
  SaveCatalogs(xosc);

  // set road network
  SaveRoadNetwork(xosc, simulation, traffic);

  // set entities;
  SaveEntities(xosc, simulation, traffic);

  // set storyboard
  SaveStoryboard(xosc, simulation, traffic, environment);

  // flush to disk
  FlushToDisk(xosc, strFileName);

  return 0;
}

void XOSCWriter_1_0_v4::SaveOneParameterDeclaration(osc::OpenScenario& xosc, const char* strName, const char* strType,
                                                    const char* strValue) {
  osc::ParameterDeclaration parameter_declaration;
  parameter_declaration._name = strName;
  parameter_declaration._parameterType = strType;
  parameter_declaration._value = strValue;
  xosc.sub_ParameterDeclarations->sub_ParameterDeclaration.push_back(parameter_declaration);
}
void XOSCWriter_1_0_v4::SaveParameterDeclaration(osc::OpenScenario& xosc, CSimulation& simulation, CTraffic& traffic) {
  osc::ParameterDeclarations pd;
  xosc.sub_ParameterDeclarations = pd;
  // $UnrealLevelIndex
  SaveOneParameterDeclaration(xosc, "$UnrealLevelIndex", "double", simulation.m_planner.m_strUnrealLevel.c_str());
  SaveOneParameterDeclaration(xosc, "$ActivePlan", "integer", traffic.m_ActivePlan.c_str());
  for (auto it : traffic.Signlights()) {
    std::string nameStr = ("$TrafficInfo_" + it.second.m_strID);
    std::string value = "";
    if (traffic.Routes().find(it.second.m_strRouteID) != traffic.Routes().end()) {
      CRoute route = traffic.Routes().find(it.second.m_strRouteID)->second;
      route.ConvertToValue();
      value += ("route:" + std::to_string(route.m_dStartLon) + "," + std::to_string(route.m_dStartLat) + ";");
    }
    value += ("start_t:" + it.second.m_strStartTime + ";");
    value += ("routeid:" + it.second.m_strRouteID + ";");
    value += ("junctionid:" + it.second.m_strJunction + ";");
    value += ("l_offset:" + it.second.m_strOffset + ";");
    std::string phase = it.second.m_strPhase;
    for (string::size_type pos(0); pos != string::npos; pos++) {
      if ((pos = phase.find(";", pos)) != string::npos) {
        phase.replace(pos, 1, ",");
      } else {
        break;
      }
    }
    value += ("phase:" + phase + ";");
    std::string strlane = it.second.m_strLane;
    for (string::size_type pos(0); pos != string::npos; pos++) {
      if ((pos = strlane.find(";", pos)) != string::npos) {
        strlane.replace(pos, 1, ",");
      } else {
        break;
      }
    }
    value += ("lane:" + strlane + ";");
    value += ("status:" + it.second.m_strStatus + ";");
    value += ("plan:" + it.second.m_strPlan + ";");
    value += ("phaseNumber:" + it.second.m_strPhasenumber + ";");
    SaveOneParameterDeclaration(xosc, nameStr.c_str(), "string", value.c_str());
  }
}

void XOSCWriter_1_0_v4::SaveCatalogs(osc::OpenScenario& xosc) {
  osc::CatalogLocations cl;
  xosc.sub_CatalogLocations = cl;
}

void XOSCWriter_1_0_v4::SaveRoadNetwork(osc::OpenScenario& xosc, CSimulation& simulation, CTraffic& traffic) {
  osc::RoadNetwork rnt;
  xosc.sub_RoadNetwork = rnt;
  std::string strMapPath = "../hadmap/" + simulation.m_mapFile.m_strMapfile;
  SaveLogicFile(xosc, strMapPath.c_str());
  SceneTrafficSignals(xosc, traffic);
}

void XOSCWriter_1_0_v4::SaveLogicFile(osc::OpenScenario& xosc, const char* str) {
  osc::File lgf;
  lgf._filepath = str;
  xosc.sub_RoadNetwork->sub_LogicFile = lgf;
}
void XOSCWriter_1_0_v4::SceneGraphFile(osc::OpenScenario& xosc, const char* str) {
  osc::File sgf;
  sgf._filepath = str;
  xosc.sub_RoadNetwork->sub_SceneGraphFile = sgf;
}
void XOSCWriter_1_0_v4::SceneTrafficSignals(osc::OpenScenario& xosc, CTraffic& traffic) {
  TrafficSignals traffic_signals;
  for (auto itr : traffic.m_mapSignlights) {
    TrafficSignalController traffic_signal_controller;
    traffic_signal_controller._name = itr.second.m_strID;
    auto create_phase = [](std::string duration, std::string name) {
      Phase p;
      p._duration = duration;
      p._name = name;
      return p;
    };
    Phase p1 = create_phase(itr.second.m_strTimeRed, "stop");
    Phase p2 = create_phase(itr.second.m_strTimeGreen, "go");
    Phase p3 = create_phase(itr.second.m_strTimeYellow, "attention");

    auto create_TrafficSignalState = [](std::string state, std::string trafficSignalId) {
      TrafficSignalState p;
      p._state = state;
      p._trafficSignalId = trafficSignalId;
      return p;
    };
    for (int i = 0; i < itr.second.m_strSignalheadVec.size(); i++) {
      TrafficSignalState state1 = create_TrafficSignalState("true;flase;false", itr.second.m_strSignalheadVec.at(i));
      p1.sub_TrafficSignalState.push_back(state1);
      TrafficSignalState state2 = create_TrafficSignalState("false;flase;true", itr.second.m_strSignalheadVec.at(i));
      p2.sub_TrafficSignalState.push_back(state2);
      TrafficSignalState state3 = create_TrafficSignalState("false;true;false", itr.second.m_strSignalheadVec.at(i));
      p3.sub_TrafficSignalState.push_back(state3);
    }
    traffic_signal_controller.sub_Phase.push_back(p1);
    traffic_signal_controller.sub_Phase.push_back(p2);
    traffic_signal_controller.sub_Phase.push_back(p3);
    traffic_signals.sub_TrafficSignalController.push_back(traffic_signal_controller);
  }

  xosc.sub_RoadNetwork->sub_TrafficSignals = traffic_signals;
}

bool XOSCWriter_1_0_v4::SaveOneEgoEntity(osc::OpenScenario& xosc, const char* str, CEgoInfo& egoinfo) {
  // Center
  Center center = CreateCenter("0", "0", "0");
  // Dimensions
  Dimensions dimensions = CreateDimensions("1.808", "4.394", "1.352");
  // Performance
  Performance pf;
  pf._maxAcceleration = egoinfo.m_accelerationMax;
  pf._maxDeceleration = egoinfo.m_decelerationMax;
  pf._maxSpeed = egoinfo.m_strVelocityMax;
  // Axles
  auto create_Axle = [](std::string maxSteering, std::string pointx, std::string pointz, std::string trackWidth,
                        std::string wheelDiameter) {
    Axle ale;
    ale._maxSteering = maxSteering;
    ale._positionX = pointx;
    ale._positionZ = maxSteering;
    ale._trackWidth = maxSteering;
    ale._wheelDiameter = maxSteering;
    return ale;
  };
  Axle front_axle = create_Axle("0.46", "2.536", "0.31997", "1.608", "0.63994");
  Axle rear_axle = create_Axle("0", "0", "0.31997", "1.608", "0.63994");
  CCataLog tmp;
  CDriveCatalog egoInfo;
  std::map<std::string, std::string> m_strPropertys;
  // get catalog param
  if (tmp.getEgoModelFromName(m_strCatalogDir.c_str(), egoinfo.m_strName, egoInfo) == 0) {
    // catalog axles
    front_axle = create_Axle(egoInfo.m_objFront.m_strMaxSteering, egoInfo.m_objFront.m_strPositionX,
                             egoInfo.m_objFront.m_strPositionZ, egoInfo.m_objFront.m_strTrackWidth,
                             egoInfo.m_objFront.m_strWheelDiameter);
    rear_axle = create_Axle(egoInfo.m_objRear.m_strMaxSteering, egoInfo.m_objRear.m_strPositionX,
                            egoInfo.m_objRear.m_strPositionZ, egoInfo.m_objRear.m_strTrackWidth,
                            egoInfo.m_objRear.m_strWheelDiameter);
    // catalog boundingbox
    center = CreateCenter(egoInfo.m_objBox.m_strX, egoInfo.m_objBox.m_strY, egoInfo.m_objBox.m_strZ);
    dimensions =
        CreateDimensions(egoInfo.m_objBox.m_strHeight, egoInfo.m_objBox.m_strLength, egoInfo.m_objBox.m_strWidth);
    // catalog properity
    m_strPropertys = egoInfo.m_property;
  }

  BoundingBox bounding_box;
  bounding_box.sub_Center = center;
  bounding_box.sub_Dimensions = dimensions;
  // performace
  Axles axles;
  axles.sub_FrontAxle = front_axle;
  axles.sub_RearAxle = rear_axle;
  std::string behavior = "UserDefine";
  // behavior
  if (egoinfo.m_trajectoryEnabled == "true") {
    behavior = "TrajectoryFollow";
  }
  m_strPropertys["Behavior"] = behavior;
  m_strPropertys["control"] = "external";
  // TrajectoryTracking
  m_strPropertys["TrajectoryTracking"] = egoinfo.m_controlPath.getTrajectoryTracking();
  Properties pts;
  for (auto it : m_strPropertys) {
    Property pt = CreateProperty(it.first, it.second);
    pts.sub_Property.push_back(pt);
  }
  ScenarioObject scenario_object;
  scenario_object._name = str;
  // vehicle
  if (egoinfo.m_strType == "car" || egoinfo.m_strType == "") {
    scenario_object.sub_Vehicle = CreateVehicle(axles, bounding_box, pf, pts, egoinfo.m_strName, "car");
  } else {
    scenario_object.sub_Vehicle = CreateVehicle(axles, bounding_box, pf, pts, egoinfo.m_strName, egoinfo.m_strType);
  }
  xosc.sub_Entities->sub_ScenarioObject.push_back(scenario_object);
  return true;
}

void XOSCWriter_1_0_v4::SetIniInfos(ScenarioObject& object, string strLength, string strWidth, string strHeight,
                                    string strX, string strY, string strZ, string strStartV, string strMaxV,
                                    string strBehavior, string strAggress, string strFollow, string strEventId,
                                    string strTrajectoryAngle, string strTrajectoryTracking, string& strName,
                                    string& strType, bool isSetMaxV, std::string obuStatu, std::string SesonrGroup) {
  // bounding box
  Center center = CreateCenter(strX, strY, strZ);
  Dimensions dimensions = CreateDimensions(strHeight, strLength, strWidth);
  BoundingBox bounding_box;
  bounding_box.sub_Center = center;
  bounding_box.sub_Dimensions = dimensions;

  Property pt2 = CreateProperty("MaxV", strMaxV);
  Property pt3 = CreateProperty("Behavior", strBehavior);
  Property pt4 = CreateProperty("Aggress", strAggress);
  Property pt5 = CreateProperty("Follow", strFollow);
  Property pt6 = CreateProperty("EventID", strEventId);
  Property pt7 = CreateProperty("TrajectoryAngle", strTrajectoryAngle);
  Property pt8 = CreateProperty("TrajectoryTracking", strTrajectoryTracking);
  Property pt9 = CreateProperty("Angle", strTrajectoryAngle);
  Property pt10 = CreateProperty("SensorGroup", SesonrGroup);
  Property pt11 = CreateProperty("ObuStatus", obuStatu);
  Properties pts;
  if (isSetMaxV) {
    pts.sub_Property.push_back(pt2);
  }
  if (strName[0] != 'O') {
    pts.sub_Property.push_back(pt3);
  }
  // Axles
  auto create_Axle = [](std::string maxSteering, std::string pointx, std::string pointz, std::string trackWidth,
                        std::string wheelDiameter) {
    Axle ale;
    ale._maxSteering = maxSteering;
    ale._positionX = pointx;
    ale._positionZ = maxSteering;
    ale._trackWidth = maxSteering;
    ale._wheelDiameter = maxSteering;
    return ale;
  };
  if (strName[0] == 'V') {
    if (strAggress.size() > 0) pts.sub_Property.push_back(pt4);
    if (strFollow.size() > 0) pts.sub_Property.push_back(pt5);
    // if (strEventId.size() > 0) pts.sub_Property.push_back(pt6);
    if (strTrajectoryAngle.size() > 0) pts.sub_Property.push_back(pt7);
    if (strTrajectoryTracking.size() > 0) pts.sub_Property.push_back(pt8);

    if (SesonrGroup.size() > 0) pts.sub_Property.push_back(pt10);
    if (obuStatu.size() > 0) pts.sub_Property.push_back(pt11);
    // performace
    Performance pf = CreatePerformance("0", "999", strMaxV);
    // Axles
    Axle front_axle = create_Axle("0", "0", "0", "0", "0");
    Axle rear_axle = create_Axle("0", "0", "0", "0", "0");
    Axles axles;
    axles.sub_FrontAxle = front_axle;
    axles.sub_RearAxle = rear_axle;

    // vehicle
    object.sub_Vehicle = CreateVehicle(axles, bounding_box, pf, pts, strType, "car");
  }

  if (strName[0] == 'O') {
    object.sub_MiscObject = CreateMiscObject(bounding_box, pts, "1.0", "obstacle", strType);
  }

  if (strName[0] == 'P') {
    if (strTrajectoryTracking.size() > 0) pts.sub_Property.push_back(pt8);
    pts.sub_Property.push_back(pt9);
    object.sub_Pedestrian = CreatePedestrian(bounding_box, pts, "65", "model", strType, "pedestrian");
  }
}

void XOSCWriter_1_0_v4::SaveEntities(osc::OpenScenario& xosc, CSimulation& simulation, CTraffic& traffic) {
  Entities et;
  xosc.sub_Entities = et;
  // 主车
  for (auto it : simulation.m_planner.m_egos) {
    SaveOneEgoEntity(xosc, it.first.c_str(), it.second);
  }
  index = 0;
  string strindex;
  // 交通流车
  CTraffic::VehicleMap& mapVehicles = traffic.Vehicles();
  CTraffic::VehicleMap::iterator itr = mapVehicles.begin();
  for (; itr != mapVehicles.end(); ++itr) {
    std::string vehicleVarName = itr->second.m_strType;
    std::string vehicleObjectName = s_Entities_Vehicle_Name_Prefix + itr->first;  // V_1
    ScenarioObject scenario_object;
    scenario_object._name = (vehicleObjectName);
    itr->second.m_strName = vehicleObjectName;
    CTraffic::RouteMap& mapRoutes = traffic.Routes();
    CTraffic::RouteMap::iterator rItr = mapRoutes.find(std::to_string(itr->second.m_routeID));
    std::string strTrajectoryTracking;
    if (rItr != mapRoutes.end()) {
      rItr->second.SegmentString();
      rItr->second.MergeString();
      strTrajectoryTracking = rItr->second.m_strTrajectoryTracking_XOSC;
    }

    SetIniInfos(scenario_object, itr->second.m_boundingBox.m_strLength, itr->second.m_boundingBox.m_strWidth,
                itr->second.m_boundingBox.m_strHeight, "0", "0", "0", itr->second.m_strStartVelocity,
                itr->second.m_strMaxVelocity, itr->second.m_strBehavior, itr->second.m_strAggress,
                itr->second.m_strFollow, itr->second.m_strEventId, itr->second.m_strAngle, strTrajectoryTracking,
                vehicleObjectName, itr->second.m_strType, false, itr->second.m_strObuStauts,
                itr->second.m_strSensorGroup);

    xosc.sub_Entities->sub_ScenarioObject.push_back(scenario_object);

    index = stoi(itr->first);
  }
  strindex = std::to_string(++index);
  // 障碍物
  CTraffic::ObstacleMap& mapObstacles = traffic.Obstacles();
  CTraffic::ObstacleMap::iterator oItr = mapObstacles.begin();
  for (; oItr != mapObstacles.end(); ++oItr) {
    // string obstacleVarName = s_MiscObject_Name_Prefix + oItr->first;
    string obstacleVarName = oItr->second.m_strType;
    string obstacleObjectName = s_Entities_MiscObject_Name_Prefix + oItr->first;
    oItr->second.m_strName = obstacleObjectName;
    ScenarioObject scenario_object;
    scenario_object._name = obstacleObjectName;
    SetIniInfos(scenario_object, oItr->second.m_boundingBox.m_strLength, oItr->second.m_boundingBox.m_strWidth,
                oItr->second.m_boundingBox.m_strHeight, "0", "0", "0", "0", "0", "NULL", "", "", "", "", "",
                obstacleObjectName, oItr->second.m_strType);

    xosc.sub_Entities->sub_ScenarioObject.push_back(scenario_object);
  }
  // 行人 , 动物，自行车
  CTraffic::PedestrianV2Map& mapPedestrians = traffic.PedestriansV2();
  CTraffic::PedestrianV2Map::iterator pItr = mapPedestrians.begin();
  for (; pItr != mapPedestrians.end(); ++pItr) {
    if (m_set_PedestrianType.find(pItr->second.m_strType) != m_set_PedestrianType.end()) {
      // std::string vehicleVarName = s_Pedestrian_Name_Prefix + pItr->first;
      std::string vehicleVarName = pItr->second.m_strType;
      std::string vehicleObjectName = s_Entities_Pedestrian_Name_Prefix + pItr->first;  // P_1
      ScenarioObject scenario_object;
      scenario_object._name = vehicleObjectName;
      pItr->second.m_strName = vehicleObjectName;
      CTraffic::RouteMap& mapRoutes = traffic.Routes();
      CTraffic::RouteMap::iterator rItr = mapRoutes.find(std::to_string(pItr->second.m_routeID));
      std::string strTrajectoryTracking;
      if (rItr != mapRoutes.end()) {
        rItr->second.SegmentString();
        rItr->second.MergeString();
        strTrajectoryTracking = rItr->second.m_strTrajectoryTracking_XOSC;
      }

      // 默认值
      SetIniInfos(scenario_object, "1", "1", "1", "0", "0", "0", pItr->second.m_strStartVelocity,
                  pItr->second.m_strMaxVelocity, pItr->second.m_strBehavior, "", "", "", pItr->second.m_strAngle,
                  strTrajectoryTracking, vehicleObjectName, pItr->second.m_strType, true);

      xosc.sub_Entities->sub_ScenarioObject.push_back(scenario_object);
    } else {
      // std::string vehicleVarName = s_Vehicle_Name_Prefix + strindex;
      std::string vehicleVarName = pItr->second.m_strType;
      std::string vehicleObjectName = s_Entities_Vehicle_Name_Prefix + strindex;  // V_1
      ScenarioObject scenario_object;
      scenario_object._name = vehicleObjectName;
      SetIniInfos(scenario_object, "2.5", "1.5", "1.3", "0", "0", "0", pItr->second.m_strStartVelocity,
                  pItr->second.m_strMaxVelocity, pItr->second.m_strBehavior, "", "", "", "", "", vehicleObjectName,
                  pItr->second.m_strType);

      xosc.sub_Entities->sub_ScenarioObject.push_back(scenario_object);

      strindex = std::to_string(++index);
    }
  }
}

void XOSCWriter_1_0_v4::FlushToDisk(osc::OpenScenario& xosc, const char* strFilePath) {
  tinyxml2::XMLDocument save_doc;
  tinyxml2::XMLDeclaration* decl = save_doc.NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"");
  auto newScenario = save_doc.NewElement("OpenSCENARIO");
  osc::__save__OpenScenario(xosc, newScenario, save_doc);
  save_doc.InsertFirstChild(newScenario);
  save_doc.InsertFirstChild(decl);
  save_doc.SaveFile(strFilePath);
}

// init
bool XOSCWriter_1_0_v4::SaveStoryboardInitEgo(osc::OpenScenario& xosc, CSimulation& simulation) {
  for (auto it : simulation.m_planner.m_egos) {
    SaveStoryboardInitOneEgo(xosc, it.second);
  }
  return true;
}

bool XOSCWriter_1_0_v4::SaveStoryboardInitOneEgo(osc::OpenScenario& xosc, CEgoInfo& egoinfo) {
  egoinfo.m_route.FromStr();
  egoinfo.m_route.ConvertToValue();
  double dLon = egoinfo.m_route.m_dStartLon;
  double dLat = egoinfo.m_route.m_dStartLat;
  ConvertLonLat2XODR(dLon, dLat);

  // action 1 speed
  LongitudinalAction longitudinal_action = SaveLongitudinalAction("step", "time", "0", egoinfo.m_strStartVelocity);
  PrivateAction private_action1;
  private_action1.sub_LongitudinalAction = longitudinal_action;

  // action 2 position
  TeleportAction teleport_action = SaveTeleportAction(dLon, dLat, 0, atof(egoinfo.m_strTheta.c_str()));
  PrivateAction private_action2;
  private_action2.sub_TeleportAction = teleport_action;

  // action 3 routing

  egoinfo.m_inputPath.ConvertToValue();
  PrivateAction private_action3;
  if (egoinfo.m_inputPath.m_vPoints.size() > 1) {
    RoutingAction routing_action = SaveRoutingAction("EgoRoute", "false", egoinfo.m_inputPath);
    private_action3.sub_RoutingAction = routing_action;
  }

  // action 4 activate
  std::string flagLateral = egoinfo.m_controlLateral;
  std::string flagLongitudinal = egoinfo.m_controlLongitudinal;
  std::string strSubType;
  if (flagLateral == "false" && flagLongitudinal == "false") {
    strSubType = "off";
  } else if (flagLateral == "true" && flagLongitudinal == "false") {
    strSubType = "lateral";
  } else if (flagLateral == "false" && flagLongitudinal == "true") {
    strSubType = "longitudinal";
  } else {
    strSubType = "both";
  }
  ActivateControllerAction actionActive = SaveActivateAction(strSubType);
  PrivateAction private_action4;
  private_action4.sub_ActivateControllerAction = actionActive;
  // push all actions
  Private p;
  p._entityRef = (egoinfo.m_strGroupName);
  p.sub_PrivateAction.push_back(private_action1);
  p.sub_PrivateAction.push_back(private_action2);
  if (egoinfo.m_inputPath.m_vPoints.size() > 1) {
    p.sub_PrivateAction.push_back(private_action3);
  }
  p.sub_PrivateAction.push_back(private_action4);
  xosc.sub_Storyboard->sub_Init->sub_Actions->sub_Private.push_back(p);
  return true;
}

bool XOSCWriter_1_0_v4::SaveStoryboardInitVehicle(osc::OpenScenario& xosc, CTraffic& traffic) {
  index = 0;
  // -- vehicle
  CTraffic::VehicleMap& mapVehicles = traffic.Vehicles();
  CTraffic::VehicleMap::iterator itr = mapVehicles.begin();
  for (; itr != mapVehicles.end(); ++itr) {
    // LongitudinalAction
    LongitudinalAction longitudinal_action =
        SaveLongitudinalAction("step", "time", "0", itr->second.m_strStartVelocity);
    PrivateAction private_action1;
    private_action1.sub_LongitudinalAction = longitudinal_action;
    double dFinalLon = -999;
    double dFinalLat = -999;
    double dFinalYaw = -999;
    ExtractPosition(traffic, itr->second.m_strRouteID, itr->second.m_strLaneID, itr->second.m_strStartShift,
                    itr->second.m_strOffset, dFinalLon, dFinalLat, dFinalYaw, false);
    // TeleportAction
    TeleportAction teleport_action =
        SaveTeleportAction(dFinalLon, dFinalLat, 0, atof(itr->second.m_strStartAngle.c_str()) * PI / 180.0);
    PrivateAction private_action2;
    private_action2.sub_TeleportAction = teleport_action;
    std::string objectName = s_Entities_Vehicle_Name_Prefix + itr->first;
    Private p;
    p._entityRef = (objectName);
    p.sub_PrivateAction.push_back(private_action1);
    p.sub_PrivateAction.push_back(private_action2);
    xosc.sub_Storyboard->sub_Init->sub_Actions->sub_Private.push_back(p);
    index = stoi(itr->first);
  }
  return true;
}

bool XOSCWriter_1_0_v4::SaveStoryboardInitObstacle(osc::OpenScenario& xosc, CTraffic& traffic) {
  CTraffic::ObstacleMap& mapObstacles = traffic.Obstacles();
  CTraffic::ObstacleMap::iterator itr = mapObstacles.begin();

  for (; itr != mapObstacles.end(); ++itr) {
    double dFinalLon = -999;
    double dFinalLat = -999;
    double dFinalYaw = -999;
    ExtractPosition(traffic, itr->second.m_strRouteID, itr->second.m_strLaneID, itr->second.m_strStartShift,
                    itr->second.m_strOffset, dFinalLon, dFinalLat, dFinalYaw, false);
    // sim
    double rad = std::atof(itr->second.m_strStartAngle.c_str()) / 180.0 * PI;
    TeleportAction teleport_action = SaveTeleportAction(dFinalLon, dFinalLat, 0, rad);
    PrivateAction private_action;
    private_action.sub_TeleportAction = (teleport_action);

    std::string objectName = s_Entities_MiscObject_Name_Prefix + itr->first;
    Private p;
    p._entityRef = (objectName);
    p.sub_PrivateAction.push_back(private_action);
    xosc.sub_Storyboard->sub_Init->sub_Actions->sub_Private.push_back(p);
  }

  return true;
}

bool XOSCWriter_1_0_v4::SaveStoryboardInitPedestrian(osc::OpenScenario& xosc, CTraffic& traffic) {
  CTraffic::PedestrianV2Map& mapPedestrians = traffic.PedestriansV2();
  CTraffic::PedestrianV2Map::iterator itr = mapPedestrians.begin();
  for (; itr != mapPedestrians.end(); ++itr) {
    // speed
    LongitudinalAction longitudinal_action =
        SaveLongitudinalAction("step", "time", "0", itr->second.m_strStartVelocity);
    PrivateAction private_action1;
    private_action1.sub_LongitudinalAction = (longitudinal_action);

    // position
    double dFinalLon = -999;
    double dFinalLat = -999;
    double dFinalYaw = -999;
    ExtractPosition(traffic, itr->second.m_strRouteID, itr->second.m_strLaneID, itr->second.m_strStartShift,
                    itr->second.m_strOffset, dFinalLon, dFinalLat, dFinalYaw, false);
    TeleportAction teleport_action =
        SaveTeleportAction(dFinalLon, dFinalLat, 0, atof(itr->second.m_strStartAngle.c_str()) * PI / 180.0);
    PrivateAction private_action2;
    private_action2.sub_TeleportAction = (teleport_action);

    std::string objectName;
    if (m_set_PedestrianType.find(itr->second.m_strType) != m_set_PedestrianType.end()) {
      objectName = s_Entities_Pedestrian_Name_Prefix + itr->first;
    } else {
      string strindex = std::to_string(++index);
      objectName = s_Entities_Vehicle_Name_Prefix + strindex;  // V_1
    }
    Private p;
    p._entityRef = (objectName);
    p.sub_PrivateAction.push_back(private_action1);
    p.sub_PrivateAction.push_back(private_action2);
    xosc.sub_Storyboard->sub_Init->sub_Actions->sub_Private.push_back(p);
  }

  return true;
}

bool XOSCWriter_1_0_v4::SaveEnvironmentInit(osc::OpenScenario& xosc, CEnvironment& enviroment) {
  for (auto iter : enviroment.WeatherData()) {
    GlobalAction globalAction;
    EnvironmentAction environmentAcrion;
    // 时间戳
    ParameterDeclarations sparameterDeclarations;
    ParameterDeclaration sParameterDeclarationmTimeStamp =
        CreateParameterDeclaration("TimeStamp", "integer", std::to_string(iter.first));
    sparameterDeclarations.sub_ParameterDeclaration.push_back(sParameterDeclarationmTimeStamp);

    std::string environment_name = "Environment" + std::to_string(iter.first);
    std::string TimeOfDay_dateTime = iter.second.toDateTime();
    TimeOfDay timeofday;
    timeofday._animation = "false";
    timeofday._dateTime = TimeOfDay_dateTime;

    Sun sSun = CreateSun("10000", "40", "20");
    // 能见度
    std::string strfog = std::to_string(atof(iter.second.m_weather.m_strVisualRange.c_str()) * 1000.0);
    Fog sFog;
    sFog._visualRange = strfog;

    // 降雨量/降雪量
    std::string precipitationType = std::string(iter.second.m_weather.m_strPrecipitationType);
    std::string intensity = std::string(iter.second.m_weather.m_strPrecipitationIntensity);
    Precipitation sPrecipitation;
    sPrecipitation._intensity = intensity;
    sPrecipitation._precipitationType = precipitationType;
    // 云密度
    std::string cloudState = iter.second.getCloudState();
    Weather sWeather;
    sWeather.sub_Sun = sSun;
    sWeather.sub_Fog = sFog;
    sWeather.sub_Precipitation = sPrecipitation;
    sWeather._cloudState = cloudState;
    // 风速
    ParameterDeclaration sParameterDeclarationCloud =
        CreateParameterDeclaration(("wind_speed"), ("double"), (iter.second.m_weather.m_strWindSpeed));
    sparameterDeclarations.sub_ParameterDeclaration.push_back(sParameterDeclarationCloud);

    // 温度
    string strTemperature = std::to_string(atof(iter.second.m_weather.m_strTemperature.c_str()) + 273.15);
    ParameterDeclaration sParamTemperature = CreateParameterDeclaration("tmperature", "double", strTemperature);
    sparameterDeclarations.sub_ParameterDeclaration.push_back(sParamTemperature);
    // user
    ParameterDeclaration sParamUsingSunByUser =
        CreateParameterDeclaration(("UsingSunByUser"), ("osc::Boolean"), ("false"));
    sparameterDeclarations.sub_ParameterDeclaration.push_back(sParamUsingSunByUser);

    RoadCondition sRoadCondition;
    sRoadCondition._frictionScaleFactor = "1";
    Environment environments;
    environments.sub_TimeOfDay = timeofday;
    environments.sub_Weather = sWeather;
    environments.sub_RoadCondition = sRoadCondition;
    environments.sub_ParameterDeclarations = sparameterDeclarations;

    environmentAcrion.sub_Environment = (environments);
    globalAction.sub_EnvironmentAction = (environmentAcrion);

    xosc.sub_Storyboard->sub_Init->sub_Actions->sub_GlobalAction.push_back(globalAction);
  }
  return true;
}

TeleportAction XOSCWriter_1_0_v4::SaveTeleportAction(double dX, double dY, double dZ, double dH) {
  WorldPosition world_position;
  world_position._x = DoubleToString(dX);
  world_position._y = DoubleToString(dY);
  world_position._z = DoubleToString(dZ);
  world_position._h = DoubleToString(dH);
  world_position._p = "0";
  world_position._r = "0";
  Position position;
  position.sub_WorldPosition = world_position;
  TeleportAction teleport_action;
  teleport_action.sub_Position = position;
  return teleport_action;
}

LongitudinalAction XOSCWriter_1_0_v4::SaveLongitudinalAction(std::string strShape, std::string strDimension,
                                                             string strDynamicValue, string strSpeed) {
  AbsoluteTargetSpeed asolute_target_speed;
  asolute_target_speed._value = strSpeed;
  TransitionDynamics transition_dynamics;
  transition_dynamics._dynamicsDimension = strDimension;
  transition_dynamics._dynamicsShape = strShape;
  transition_dynamics._value = strDynamicValue;
  SpeedActionTarget speed_action_target;
  speed_action_target.sub_AbsoluteTargetSpeed = asolute_target_speed;
  SpeedAction speed_action;
  speed_action.sub_SpeedActionDynamics = transition_dynamics;
  speed_action.sub_SpeedActionTarget = speed_action_target;
  LongitudinalAction longitudinal_action;
  longitudinal_action.sub_SpeedAction = (speed_action);

  return longitudinal_action;
}

ActivateControllerAction XOSCWriter_1_0_v4::SaveActivateAction(std::string strSubType) {
  // ActivateControllerAction
  std::string flagLateral = "false";
  std::string flagLongitudinal = "false";
  if (strSubType == "off") {
    flagLateral = "false";
    flagLongitudinal = "false";
  } else if (strSubType == "lateral") {
    flagLateral = "true";
    flagLongitudinal = "false";
  } else if (strSubType == "longitudinal") {
    flagLateral = "false";
    flagLongitudinal = "true";
  } else {
    flagLateral = "true";
    flagLongitudinal = "true";
  }
  ActivateControllerAction activateAction;
  activateAction._lateral = (flagLateral);
  activateAction._longitudinal = (flagLongitudinal);
  return activateAction;
}

ControllerAction XOSCWriter_1_0_v4::SaveControlAction(Scene_Action action) {  //
  Properties proper;
  if (action.sActionMulti.flag && action.strType == "assign") {
    Property resume_sw = CreateProperty(("resume_sw"), action.sActionMulti.sAssign.s_resume_sw);
    Property cancel_sw = CreateProperty(("cancel_sw"), action.sActionMulti.sAssign.s_cancel_sw);
    Property speed_inc_sw = CreateProperty(("speed_inc_sw"), action.sActionMulti.sAssign.s_speed_inc_sw);
    Property speed_dec_sw = CreateProperty(("speed_dec_sw"), action.sActionMulti.sAssign.s_speed_dec_sw);
    Property set_timegap = CreateProperty(("set_timegap"), action.sActionMulti.sAssign.s_set_timegap);
    Property set_speed = CreateProperty(("set_speed"), action.sActionMulti.sAssign.s_set_speed);
    proper.sub_Property.push_back(resume_sw);
    proper.sub_Property.push_back(cancel_sw);
    proper.sub_Property.push_back(speed_inc_sw);
    proper.sub_Property.push_back(speed_dec_sw);
    proper.sub_Property.push_back(set_timegap);
    proper.sub_Property.push_back(set_speed);
  }
  Controller control;
  control.sub_Properties = proper;
  control._name = action.strValue;
  AssignControllerAction assignAction;
  assignAction.sub_Controller = control;
  // OverrideThrottleAction
  Scene_Action_Override temp = action.sActionMulti.sOveride;
  OverrideThrottleAction throttleAction;
  throttleAction._active = temp.activate("throttle");
  throttleAction._value = temp.value("throttle");
  OverrideBrakeAction BrakeAction;
  BrakeAction._active = temp.activate("brake");
  BrakeAction._value = temp.value("brake");
  OverrideClutchAction ClutchAction;
  ClutchAction._active = temp.activate("clutch");
  ClutchAction._value = temp.value("clutch");
  OverrideParkingBrakeAction ParkingBrakeAction;
  ParkingBrakeAction._active = temp.activate("parking_brake");
  ParkingBrakeAction._value = temp.value("parking_brake");
  OverrideSteeringWheelAction SteeringWheelAction;
  SteeringWheelAction._active = temp.activate("steering_wheel");
  SteeringWheelAction._value = temp.value("steering_wheel");
  OverrideGearAction GearAction;
  GearAction._active = temp.activate("gear");
  GearAction._number = temp.value("gear");
  OverrideControllerValueAction overrideAction;
  overrideAction.sub_Throttle = throttleAction;
  overrideAction.sub_Brake = BrakeAction;
  overrideAction.sub_Clutch = ClutchAction;
  overrideAction.sub_ParkingBrake = ParkingBrakeAction;
  overrideAction.sub_SteeringWheel = SteeringWheelAction;
  overrideAction.sub_Gear = GearAction;

  ControllerAction controlAction;
  controlAction.sub_AssignControllerAction = assignAction;
  controlAction.sub_OverrideControllerValueAction = overrideAction;
  return controlAction;
}

CustomCommandAction XOSCWriter_1_0_v4::SaveCustomAction(Scene_Action action) {
  std::string tmp;

  if (action.strType == "status") {
    bool flag = true;
    if (action.strSubType == "emergency_brake") {
      tmp = "EmergencyBrake";
    }
    if (action.strSubType == "vehicle_lost_control") {
      tmp = "VehicleLostControl";
    }
    if (action.strSubType == "hands_on_steeringwheel") {
      tmp = "HandsOnSteeringwheel";
    }
    if (action.strSubType == "eyes_on_road") {
      tmp = "EyesOnRoad";
    }
    if (action.strSubType == "lidar") {
      tmp = "Lidar";
    }
    if (action.strSubType == "radar") {
      tmp = "Radar";
    }
    if (action.strSubType == "ultrasonic") {
      tmp = "Ultrasonic";
    }
    if (action.strSubType == "camera") {
      tmp = "Camera";
    }
    if (action.strSubType == "gnss") {
      tmp = "Gnss";
    }
    if (action.strSubType == "imu") {
      tmp = "Imu";
    }
    if (action.strSubType == "localization") {
      tmp = "Localization";
    }
    if (action.strSubType == "beam") {
      tmp = "Beam";
    }
    if (action.strSubType == "brake_light") {
      tmp = "BrakeLight";
    }
    if (action.strSubType == "hazard_light") {
      tmp = "HazardLight";
    }
    if (action.strSubType == "left_turn_light") {
      tmp = "LeftTurnLight";
    }
    if (action.strSubType == "right_turn_light") {
      tmp = "RightTurnLight";
    }
    if (action.strSubType == "driver_seatbelt") {
      tmp = "DriverSeatbelt";
    }
    if (action.strSubType == "passenger_seatbelt") {
      tmp = "PassengerSeatbelt";
    }
    if (action.strSubType == "driver_door") {
      tmp = "DriverDoor";
    }
    if (action.strSubType == "passenger_door") {
      tmp = "PassengerDoor";
    }
    if (action.strSubType == "hood") {
      tmp = "Hood";
    }
    if (action.strSubType == "trunk") {
      tmp = "Trunk";
    }
    if (action.strSubType == "parkingbrake") {
      tmp = "ParkingBrake";
    }
    if (action.strSubType == "wiper") {
      tmp = "Wiper";
    }
    if (action.strSubType == "gear") {
      tmp = "Gear";
    }
    if (action.strSubType == "key") {
      tmp = "Key";
    }
    if (action.strSubType == "userdefine") {
      tmp = "UserDefine";
    } else {
      flag = false;
      SYSTEM_LOGGER_WARN("Save to xosc strSubType = %s is not define!", action.strSubType.c_str());
    }
    tmp.append(":value=" + action.strValue);
    string str = tmp;
    CustomCommandAction custom;
    custom._type = "Status";
    custom._text = str;
    return custom;
  }
  CustomCommandAction custom;
  custom._type = "Status";
  custom._text = "";
  return custom;
}

RoutingAction XOSCWriter_1_0_v4::SaveRoutingAction(std::string strName, std::string strClosed, sPath& path) {
  Route route;
  route._name = strName;
  route._closed = strClosed;
  int nPoint = path.m_vPoints.size();
  if (nPoint > 0) {
    for (auto p : path.m_vPoints) {
      double dLon = p.m_dLon;
      double dLat = p.m_dLat;
      ConvertLonLat2XODR(dLon, dLat);

      WorldPosition world_position_temp;
      world_position_temp._x = DoubleToString(dLon);
      world_position_temp._y = DoubleToString(dLat);
      world_position_temp._z = DoubleToString(p.m_dAlt);
      world_position_temp._h = "0";
      world_position_temp._p = "0";
      world_position_temp._r = "0";

      Position position_temp;  //
      position_temp.sub_WorldPosition = (world_position_temp);
      Waypoint waypoint;
      waypoint.sub_Position = position_temp;
      waypoint._routeStrategy = "shortest";
      route.sub_Waypoint.push_back(waypoint);
    }

  } else {
    SYSTEM_LOGGER_ERROR("routing action save error!");
  }

  AssignRouteAction assign_route_action;
  assign_route_action.sub_Route = (route);

  RoutingAction routing_action;
  routing_action.sub_AssignRouteAction = (assign_route_action);
  return routing_action;
}

void XOSCWriter_1_0_v4::SaveStoryboard(osc::OpenScenario& xosc, CSimulation& simulation, CTraffic& traffic,
                                       CEnvironment& environment) {
  InitActions initas;
  Init init;
  init.sub_Actions = initas;
  Trigger tg;
  Storyboard sb;
  sb.sub_Init = init;
  sb.sub_StopTrigger = tg;
  xosc.sub_Storyboard = sb;
  SaveEnvironmentInit(xosc, environment);
  // init
  SaveStoryboardInitEgo(xosc, simulation);
  SaveStoryboardInitVehicle(xosc, traffic);
  SaveStoryboardInitObstacle(xosc, traffic);
  SaveStoryboardInitPedestrian(xosc, traffic);
  //
  SaveStory(xosc, simulation, traffic);

  SaveStopTrigger(xosc);
}

bool XOSCWriter_1_0_v4::SaveStopTrigger(osc::OpenScenario& xosc) {
  Trigger stop_trriger;
  xosc.sub_Storyboard->sub_StopTrigger = stop_trriger;
  return true;
}

bool XOSCWriter_1_0_v4::SaveStory(osc::OpenScenario& xosc, CSimulation& siminfo, CTraffic& traffic) {
  SYSTEM_LOGGER_INFO("SaveStoryboard");
  Story story;
  story._name = ("Story");
  // start trigger
  SimulationTimeCondition simulation_time_condition;
  simulation_time_condition._rule = "greaterThan";
  simulation_time_condition._value = "0";
  ByValueCondition by_value_condition;
  by_value_condition.sub_SimulationTimeCondition = (simulation_time_condition);
  std::string conditionName = "Start";
  Condition condition;
  condition._conditionEdge = "rising";
  condition._delay = "0";
  condition._name = conditionName;
  condition.sub_ByValueCondition = (by_value_condition);
  ConditionGroup condition_group;
  condition_group.sub_Condition.push_back(condition);
  Trigger startTrigger;
  startTrigger.sub_ConditionGroup.push_back(condition_group);
  std::string actName = "Act";
  Act act;
  act.sub_StartTrigger = startTrigger;
  act._name = actName;

  ParameterDeclarations pd;
  story.sub_ParameterDeclarations = (pd);
  for (auto ego : siminfo.m_planner.m_egos) {
    for (auto it : ego.second.m_scenceEvents) {
      int index = 0;
      for (auto itCondition : it.second.ConditionVec) {
        std::string strName = "$ego_triggercount$" + it.second.strName + "$Condition_" + std::to_string(index++);
        std::string strType = "string";
        std::string strValue = itCondition.strCount;
        ParameterDeclaration parameter_declaration = CreateParameterDeclaration(strName, strType, strValue);
        story.sub_ParameterDeclarations->sub_ParameterDeclaration.push_back(parameter_declaration);
      }
    }
  }
  for (auto it : traffic.m_mapSceneEvents) {
    std::string refObject = "";
    for (auto itVechile : traffic.m_mapVehicles) {
      std::vector<std::string> eventidVec;
      if (!itVechile.second.m_strEventId.empty()) {
        boost::algorithm::split(eventidVec, itVechile.second.m_strEventId, boost::algorithm::is_any_of(","));
        if (eventidVec.size() > 0) {
          for (auto itStrId : eventidVec) {
            if (itStrId == it.second.m_id) {
              refObject = itVechile.second.m_strName;
              break;
            }
          }
        }
      }
    }
    int index = 0;
    for (auto itCondition : it.second.ConditionVec) {
      std::string strName =
          "$" + refObject + "_triggercount$" + it.second.strName + "$Condition_" + std::to_string(index++);
      std::string strType = "string";
      std::string strValue = itCondition.strCount;
      ParameterDeclaration parameter_declaration = CreateParameterDeclaration(strName, strType, strValue);
      story.sub_ParameterDeclarations->sub_ParameterDeclaration.push_back(parameter_declaration);
    }
  }

  // act.ManeuverGroup().push_back(maneuver_group);
  m_ManeuverGroupIndex = 0;
  int nTotalEgoManeuvers = SaveEgoStory(act, siminfo, traffic);
  int nTotalVehicleManeuvers = SaveVehiclesStory(act, traffic);  // ego and vehicles
  int nTotalPedestrianManeuvers = SavePedestriansStory(act, traffic);
  story.sub_Act.push_back(act);
  xosc.sub_Storyboard->sub_Story.push_back(story);
  return true;
}

int XOSCWriter_1_0_v4::SaveEgoStory(Act& act, CSimulation& simulation, CTraffic& traffic) {
  for (auto it : simulation.m_planner.m_egos) {
    CTraffic::SceneEventMap mapVehicles = it.second.m_scenceEvents;
    // Actors
    Actors actors;
    actors._selectTriggeringEntities = "false";
    EntityRef entity_ref;
    entity_ref._entityRef = it.first;
    actors.sub_EntityRef.push_back(entity_ref);
    m_ManeuverGroupIndex++;
    std::string maneuverGroupName = "ManeuverGroup" + std::to_string(m_ManeuverGroupIndex);
    ManeuverGroup maneuver_group;
    maneuver_group.sub_Actors = actors;
    maneuver_group._maximumExecutionCount = "1";
    maneuver_group._name = maneuverGroupName.c_str();
    // act
    maneuver_group.sub_Actors = actors;
    Maneuver maneuver;
    maneuver._name = ("Maneuver0");
    int nIndex = 0;
    CTraffic::SceneEventMap::iterator itr = mapVehicles.begin();
    for (; itr != mapVehicles.end(); ++itr) {
      std::string objectName = itr->first;
      Trigger start_trigger;
      Event event;
      event.sub_StartTrigger = start_trigger;
      event._name = itr->second.strName;
      event._priority = "overwrite";
      event._maximumExecutionCount = "1";
      // 开始添加event
      bool ishaveaction = false;
      std::string strObjectName;
      AddVehicleAction_V_A_M(itr->second, strObjectName, event, ishaveaction);
      if (ishaveaction) {
        AddVehicleTrigger_V_A_M(itr->second, event, mapVehicles);
        maneuver.sub_Event.push_back(event);
        nIndex++;
      }
    }
    if (nIndex > 0) {
      maneuver_group.sub_Maneuver.push_back(maneuver);
    }
    act.sub_ManeuverGroup.push_back(maneuver_group);
  }
  return 1;
}

int XOSCWriter_1_0_v4::SaveVehiclesStory(Act& act, CTraffic& traffic) {
  CTraffic::VehicleMap& mapVehicles = traffic.Vehicles();
  int nTotalManeuvers = 0;
  // vehicles
  CTraffic::VehicleMap::iterator itr = mapVehicles.begin();
  for (; itr != mapVehicles.end(); ++itr) {
    std::string objectName = s_Entities_Vehicle_Name_Prefix + itr->first;
    // act
    Actors actors;
    actors._selectTriggeringEntities = ("false");
    EntityRef entity_ref;
    entity_ref._entityRef = (objectName);
    actors.sub_EntityRef.push_back(entity_ref);

    m_ManeuverGroupIndex++;
    std::string maneuverGroupName = "ManeuverGroup" + std::to_string(m_ManeuverGroupIndex);
    ManeuverGroup maneuver_group;
    maneuver_group.sub_Actors = actors;
    maneuver_group._maximumExecutionCount = "1";
    maneuver_group._name = maneuverGroupName.c_str();
    maneuver_group.sub_Actors = actors;

    // ManeuverGroup 补全
    int nManeuvers = SaveOneVehicleActions(traffic, itr->second, maneuver_group, objectName);
    if (nManeuvers == 0) {
      continue;
    }
    nTotalManeuvers += nManeuvers;

    act.sub_ManeuverGroup.push_back(maneuver_group);
  }
  // obstacle

  return nTotalManeuvers;
}

int XOSCWriter_1_0_v4::ConvertOneVehicleRouting(CVehicle& v, CRoute& r, Maneuver& maneuver, string strObjectName,
                                                int& nEvents, int& nTimeOrder, int& nOrder) {
  r.FromStr();
  r.ConvertToValue();
  r.ExtractPath();

  sPath& routingPath = r.m_path;
  if (routingPath.m_vPoints.size() > 1) {
    std::string strOrder;
    std::string strTimeOrder;

    strOrder = boost::lexical_cast<std::string>(nOrder);
    strTimeOrder = boost::lexical_cast<std::string>(nTimeOrder);
    Route route;
    route._name = "VehicleRoute";
    route._closed = "0";

    double dLon = -999;
    double dLat = -999;
    double dAlt = -999;
    double dYaw = 0;
    for (int i = 0; i < routingPath.m_vPoints.size(); ++i) {
      if (i == 0) {
        ExtractPosition(r, v.m_strLaneID, v.m_strStartShift, v.m_strOffset, dLon, dLat, dYaw, false);
      } else {
        dLon = routingPath.m_vPoints[i].m_dLon;
        dLat = routingPath.m_vPoints[i].m_dLat;
        ConvertLonLat2XODR(dLon, dLat);
      }
      dAlt = routingPath.m_vPoints[i].m_dAlt;
      WorldPosition world_position_temp;
      world_position_temp._x = DoubleToString(dLon);
      world_position_temp._y = DoubleToString(dLat);
      world_position_temp._z = std::to_string(dAlt);
      world_position_temp._h = "0";
      world_position_temp._p = "0";
      world_position_temp._r = "0";

      Position position_temp;  //
      position_temp.sub_WorldPosition = (world_position_temp);
      Waypoint waypoint;
      waypoint.sub_Position = position_temp;
      waypoint._routeStrategy = "shortest";
      route.sub_Waypoint.push_back(waypoint);
    }

    AssignRouteAction assign_route_action;
    assign_route_action.sub_Route = (route);
    RoutingAction routing_action;
    routing_action.sub_AssignRouteAction = (assign_route_action);
    PrivateAction private_action;
    private_action.sub_RoutingAction = (routing_action);

    Action action;
    action._name = ("ActionVehicleRouting");
    action.sub_PrivateAction = (private_action);

    SimulationTimeCondition simulation_time_condition;

    simulation_time_condition._rule = "greaterThan";
    simulation_time_condition._value = "0";
    ByValueCondition by_value_condition;
    by_value_condition.sub_SimulationTimeCondition = (simulation_time_condition);
    Condition condition;
    condition._conditionEdge = "rising";
    condition._delay = "0";
    condition._name = "ConditionVehicleRouting";
    condition.sub_ByValueCondition = (by_value_condition);
    ConditionGroup condition_group;
    condition_group.sub_Condition.push_back(condition);
    Trigger start_trigger;
    start_trigger.sub_ConditionGroup.push_back(condition_group);

    std::string eventVarName = "Event";
    eventVarName += strOrder;
    Event event = CreateEvent(start_trigger, eventVarName.c_str(), "overwrite");
    event.sub_Action.push_back(action);
    maneuver.sub_Event.push_back(event);

    nEvents++;
    nTimeOrder++;
    nOrder++;
  }

  return 0;
}

int XOSCWriter_1_0_v4::SaveOneVehicleActions(CTraffic& traffic, CVehicle& v, ManeuverGroup& maneuver_group,
                                             std::string strObjectName) {
  int nTimeOrder = 0;
  int nManeuvers = 0;
  int nOrder = 0;

  Maneuver maneuver;
  maneuver._name = ("Maneuver0");

  int nRet = 0;

  // route
  CTraffic::RouteMap& mapRoutes = traffic.Routes();
  CTraffic::RouteMap::iterator rItr = mapRoutes.find(v.m_strRouteID);
  if (rItr != mapRoutes.end()) {
    nRet = ConvertOneVehicleRouting(v, rItr->second, maneuver, strObjectName, nManeuvers, nTimeOrder, nOrder);
  }

  // 两条路线吧   一条旧的数据格式   一条新的数据格式    如果有新数据格式 优先使用新的
  CTraffic::SceneEventMap& mapSceneEvents = traffic.SceneEvents();
  if (mapSceneEvents.size() > 0) {
    return SaveOneVehicleActionsNewVersion(traffic, v, maneuver_group, strObjectName, maneuver, nTimeOrder, nManeuvers);
  } else {
    return SaveOneVehicleActionsOldVersion(traffic, v, maneuver_group, strObjectName, maneuver, nTimeOrder, nManeuvers);
  }
}

void XOSCWriter_1_0_v4::JointData(CTraffic& traffic, CVehicle& v, vector<TimeTraggerData>& vecTT,
                                  vector<ConditionTraggerData>& CT1, vector<ConditionTraggerData>& CT2,
                                  vector<ConditionTraggerData>& CT3, vector<ConditionTraggerData>& CT4) {
  // acceleration
  CTraffic::AccelerationMap& mapAccs = traffic.Accs();
  CTraffic::AccelerationMap::iterator aItr = mapAccs.find(v.m_strAccID);
  string strTimeAcc = aItr->second.m_strProfilesTime;
  if (strTimeAcc.empty()) strTimeAcc = "0.0,0.0";
  string strTimeEndAcc = aItr->second.m_strEndConditionTime;
  if (strTimeEndAcc.empty()) strTimeEndAcc = "None,0.0";

  string strEventAcc = aItr->second.m_strProfilesEvent;
  if (strEventAcc.empty()) strEventAcc = "ttc 0,0";
  string strEventEndAcc = aItr->second.m_strEndConditionEvent;
  if (strEventEndAcc.empty()) strEventEndAcc = "None,0";

  // merge
  CTraffic::MergeMap& mapMerges = traffic.Merges();
  CTraffic::MergeMap::iterator mItr = mapMerges.find(v.m_strMergeID);
  string strTimeMerge = mItr->second.m_strProfileTime;
  if (strTimeMerge.empty()) strTimeMerge = "0.0,0,4.5";
  string strEventMerge = mItr->second.m_strProfileEvent;
  if (strEventMerge.empty()) strEventMerge = "ttc 0,0";
  // velocity
  CTraffic::VelocityMap& mapVelocities = traffic.Velocities();
  CTraffic::VelocityMap::iterator vItr = mapVelocities.find(v.m_strID);
  string strTimeVelocity = "";
  string strEventVelocity = "";
  if (vItr != mapVelocities.end()) {
    strTimeVelocity = vItr->second.m_strProfilesTime;
    strEventVelocity = vItr->second.m_strProfilesEvent;
  }
  // 拼接各种数据
  {
    // 切割 strTimeAcc   strTimeEndAcc  填充若干个TimeTraggerData
    map<string, string> map_timev;
    vector<string> str1;
    vector<string> str2;
    boost::algorithm::split(str1, strTimeAcc, boost::algorithm::is_any_of(";"));
    boost::algorithm::split(str2, strTimeEndAcc, boost::algorithm::is_any_of(";"));
    vector<doublestring> vec1;
    for (auto itr : str1) {
      vector<std::string> t_acc;
      boost::algorithm::split(t_acc, itr, boost::algorithm::is_any_of(","));

      string strTime = t_acc[0];
      boost::trim(strTime);
      string strAcc = t_acc[1];
      boost::trim(strAcc);

      doublestring temp(strTime, strAcc);
      vec1.push_back(temp);
    }

    while (str1.size() != str2.size()) {
      str2.push_back("None,0.0");
    }

    vector<doublestring> vec2;
    for (auto itr : str2) {
      vector<std::string> t_endacc;
      boost::algorithm::split(t_endacc, itr, boost::algorithm::is_any_of(","));

      string strEndType = t_endacc[0];
      boost::trim(strEndType);
      string strEndValue = t_endacc[1];
      boost::trim(strEndValue);

      doublestring temp(strEndType, strEndValue);
      vec2.push_back(temp);
    }

    for (int i = 0; i < vec1.size(); i++) {
      TimeTraggerData temp;
      temp.strTraggerTime = vec1[i]._str1;
      temp.strAcc = vec1[i]._str2;
      temp.strEndConditionType = vec2[i]._str1;
      temp.strEndConditionValue = vec2[i]._str2;

      vecTT.push_back(temp);
    }

    // 切割 strTimeMerge 填充到vecTT里面
    vector<string> str3;
    boost::algorithm::split(str3, strTimeMerge, boost::algorithm::is_any_of(";"));
    // vector<doublestring> vec2;
    for (auto itr : str3) {
      vector<std::string> t_merge;
      boost::algorithm::split(t_merge, itr, boost::algorithm::is_any_of(","));

      string strTime = t_merge[0];
      boost::trim(strTime);
      string strMerge = t_merge[1];
      boost::trim(strMerge);
      string strOffsetOverTime = "4.500";
      if (t_merge.size() >= 3) {
        strOffsetOverTime = t_merge[2];
        boost::trim(strOffsetOverTime);
      }

      string strOffset = "0";
      if (t_merge.size() > 3) {
        strOffset = t_merge[3];
        boost::trim(strOffset);
      }

      for (auto itr = vecTT.begin(); itr != vecTT.end(); itr++) {
        if (itr->strTraggerTime == strTime) {
          itr->strMerge = strMerge;
          itr->strOffsetOverTime = strOffsetOverTime;
          itr->strOffset = strOffset;
        }
      }
    }

    // 切割 strTimeVelocity 填充到vecTT里面
    vector<string> str4;
    boost::algorithm::split(str4, strTimeVelocity, boost::algorithm::is_any_of(";"));
    if (strTimeVelocity.size() > 0) {
      for (auto itr : str4) {
        vector<std::string> t_velocity;
        boost::algorithm::split(t_velocity, itr, boost::algorithm::is_any_of(","));

        string strTime = t_velocity[0];
        boost::trim(strTime);
        string strVelocity = t_velocity[1];
        boost::trim(strVelocity);

        for (auto itr = vecTT.begin(); itr != vecTT.end(); itr++) {
          if (itr->strTraggerTime == strTime) {
            itr->strVelocity = strVelocity;
          }
        }
      }
    }
  }

  {
    // 切割 strTimeAcc   strTimeEndAcc  填充若干个TimeTraggerData
    map<string, string> map_timev;
    vector<string> str1;
    vector<string> str2;
    if (strEventAcc.size() < 1) {
      return;
    }
    boost::algorithm::split(str1, strEventAcc, boost::algorithm::is_any_of(";"));
    boost::algorithm::split(str2, strEventEndAcc, boost::algorithm::is_any_of(";"));
    vector<doublestring> vec1;
    for (auto itr : str1) {
      vector<std::string> t_acc;
      boost::algorithm::split(t_acc, itr, boost::algorithm::is_any_of(","));

      string strCondition = t_acc[0];
      string str12 = t_acc[1];
      vector<std::string> t_acc_2;
      boost::algorithm::split(t_acc_2, str12, boost::algorithm::is_any_of(" "));
      string strAcc = t_acc_2[0];

      doublestring temp(strCondition, strAcc);
      vec1.push_back(temp);
    }

    vector<doublestring> vec2;
    for (auto itr : str2) {
      vector<std::string> t_endacc;
      boost::algorithm::split(t_endacc, itr, boost::algorithm::is_any_of(","));

      string strEndType = t_endacc[0];
      boost::trim(strEndType);
      string strEndValue = t_endacc[1];
      boost::trim(strEndValue);

      doublestring temp(strEndType, strEndValue);
      vec2.push_back(temp);
    }

    for (int i = 0; i < vec1.size(); i++) {
      vector<std::string> t_acc_;
      boost::algorithm::split(t_acc_, vec1[i]._str1, boost::algorithm::is_any_of(" "));

      ConditionTraggerData temp;
      temp.strCondition_3to1 = vec1[i]._str1;
      temp.strTraggerType = t_acc_[0];
      if (t_acc_.size() >= 3) {
        temp.strDisModle = t_acc_[1];
        temp.strTraggerValue = t_acc_[2];
      } else {
        temp.strDisModle = "laneprojection";
        temp.strTraggerValue = t_acc_[1];
      }

      temp.strAcc = vec1[i]._str2;

      temp.strEndConditionType = vec2[i]._str1;
      temp.strEndConditionValue = vec2[i]._str2;

      if (temp.strTraggerType == "egoDistance" && temp.strDisModle == "euclideandistance") {
        CT1.push_back(temp);
      }
      if (temp.strTraggerType == "egoDistance" && temp.strDisModle == "laneprojection") {
        CT2.push_back(temp);
      }
      if (temp.strTraggerType == "ttc" && temp.strDisModle == "euclideandistance") {
        CT3.push_back(temp);
      }
      if (temp.strTraggerType == "ttc" && temp.strDisModle == "laneprojection") {
        CT4.push_back(temp);
      }
    }

    // 切割 strTimeMerge 填充到mapCT里面
    vector<string> str3;
    boost::algorithm::split(str3, strEventMerge, boost::algorithm::is_any_of(";"));
    for (auto itr : str3) {
      vector<std::string> t_merge;
      boost::algorithm::split(t_merge, itr, boost::algorithm::is_any_of(","));

      string strCondition = t_merge[0];

      string strMerge = t_merge[1];
      boost::trim(strMerge);
      string strOffsetOverTime = "4.5";
      string strOffset = "0";
      if (t_merge.size() > 2) {
        string strOffsetOverTime = t_merge[2];
        boost::trim(strOffsetOverTime);

        if (t_merge[3].find("v") == string::npos) {
          vector<string> merge_offset;
          boost::algorithm::split(merge_offset, t_merge[3], boost::algorithm::is_any_of(" "));
          strOffset = merge_offset[0];
          boost::trim(strOffset);
        }
      }

      for (auto itr = CT1.begin(); itr != CT1.end(); itr++) {
        if (itr->strCondition_3to1 == strCondition) {
          itr->strMerge = strMerge;
          itr->strOffsetOverTime = strOffsetOverTime;
          itr->strOffset = strOffset;
        }
      }

      for (auto itr = CT2.begin(); itr != CT2.end(); itr++) {
        if (itr->strCondition_3to1 == strCondition) {
          itr->strMerge = strMerge;
          itr->strOffsetOverTime = strOffsetOverTime;
          itr->strOffset = strOffset;
        }
      }
      for (auto itr = CT3.begin(); itr != CT3.end(); itr++) {
        if (itr->strCondition_3to1 == strCondition) {
          itr->strMerge = strMerge;
          itr->strOffsetOverTime = strOffsetOverTime;
          itr->strOffset = strOffset;
        }
      }
      for (auto itr = CT4.begin(); itr != CT4.end(); itr++) {
        if (itr->strCondition_3to1 == strCondition) {
          itr->strMerge = strMerge;
          itr->strOffsetOverTime = strOffsetOverTime;
          itr->strOffset = strOffset;
        }
      }
    }

    // 切割 strTimeVelocity 填充到vecTT里面
    vector<string> str4;
    if (strEventVelocity.size() > 0) {
      boost::algorithm::split(str4, strEventVelocity, boost::algorithm::is_any_of(";"));
      for (auto itr : str4) {
        vector<std::string> t_velocity;
        boost::algorithm::split(t_velocity, itr, boost::algorithm::is_any_of(","));

        string strCondition = t_velocity[0];

        vector<std::string> t_velocityvalue;
        boost::algorithm::split(t_velocityvalue, t_velocity[1], boost::algorithm::is_any_of(","));

        vector<std::string> t_relvelocityvalue;
        boost::algorithm::split(t_relvelocityvalue, t_velocityvalue[0], boost::algorithm::is_any_of(" "));
        string strVelocity = t_relvelocityvalue[0];

        for (auto itr = CT1.begin(); itr != CT1.end(); itr++) {
          if (itr->strCondition_3to1 == strCondition) {
            itr->strVelocity = strVelocity;
          }
        }

        for (auto itr = CT2.begin(); itr != CT2.end(); itr++) {
          if (itr->strCondition_3to1 == strCondition) {
            itr->strVelocity = strVelocity;
          }
        }
        for (auto itr = CT3.begin(); itr != CT3.end(); itr++) {
          if (itr->strCondition_3to1 == strCondition) {
            itr->strVelocity = strVelocity;
          }
        }
        for (auto itr = CT4.begin(); itr != CT4.end(); itr++) {
          if (itr->strCondition_3to1 == strCondition) {
            itr->strVelocity = strVelocity;
          }
        }
      }
    }
  }
}

void XOSCWriter_1_0_v4::AddVehicleAction_V_A_M(SceneEvent& singleSceneEvent, string& strObjectName, Event& event,
                                               bool& ishaveaction) {
  SYSTEM_LOGGER_INFO("AddVehicleAction_V_A_M start action size=%d", singleSceneEvent.ActionVer.size());
  int nindex = 0;
  for (int i = 0; i < singleSceneEvent.ActionVer.size(); i++) {
    ishaveaction = true;
    std::string actionType = singleSceneEvent.ActionVer[i].strType;
    Scene_Action sAction = singleSceneEvent.ActionVer[i];
    string actionVarName = singleSceneEvent.strName + "_action_" + singleSceneEvent.m_id + "_" + sAction.strId;
    SYSTEM_LOGGER_INFO("AddAction_V_A_M  check true type=%s,check=%s", singleSceneEvent.ActionVer[i].strType.c_str(),
                       singleSceneEvent.ActionVer[i].strChecked.c_str());
    ishaveaction = true;
    if (singleSceneEvent.ActionVer[i].strType == "velocity") {
      AbsoluteTargetSpeed asolute_target_speed;
      asolute_target_speed._value = (singleSceneEvent.ActionVer[i].strValue);
      TransitionDynamics transition_dynamics;
      transition_dynamics._dynamicsDimension = "time";
      transition_dynamics._dynamicsShape = "step";
      transition_dynamics._value = "0";

      SpeedActionTarget speed_action_target;
      speed_action_target.sub_AbsoluteTargetSpeed = (asolute_target_speed);
      SpeedAction speed_action;
      speed_action.sub_SpeedActionDynamics = transition_dynamics;
      speed_action.sub_SpeedActionTarget = speed_action_target;

      LongitudinalAction longitudinal_action;
      longitudinal_action.sub_SpeedAction = (speed_action);

      PrivateAction private_action;
      private_action.sub_LongitudinalAction = (longitudinal_action);

      Action action;
      action._name = (actionVarName);
      action.sub_PrivateAction = (private_action);
      event.sub_Action.push_back(action);
      nindex++;
    } else if (singleSceneEvent.ActionVer[i].strType == "acc") {
      if (singleSceneEvent.EndConditionVer[i].strType == "velocity") {
        AbsoluteTargetSpeed asolute_target_speed;
        asolute_target_speed._value = (singleSceneEvent.EndConditionVer[i].strValue);
        TransitionDynamics transition_dynamics;
        transition_dynamics._dynamicsDimension = "rate";
        transition_dynamics._dynamicsShape = "linear";
        transition_dynamics._value = singleSceneEvent.ActionVer[i].strValue;
        SpeedActionTarget speed_action_target;
        speed_action_target.sub_AbsoluteTargetSpeed = (asolute_target_speed);
        SpeedAction speed_action;
        speed_action.sub_SpeedActionDynamics = transition_dynamics;
        speed_action.sub_SpeedActionTarget = speed_action_target;
        LongitudinalAction longitudinal_action;
        longitudinal_action.sub_SpeedAction = (speed_action);
        PrivateAction private_action;
        private_action.sub_LongitudinalAction = (longitudinal_action);

        Action action;
        action._name = (actionVarName);
        action.sub_PrivateAction = (private_action);
        event.sub_Action.push_back(action);
        nindex++;
      } else if (singleSceneEvent.EndConditionVer[i].strType == "time") {
        double tempv =
            stod(singleSceneEvent.ActionVer[i].strValue) * stod(singleSceneEvent.EndConditionVer[i].strValue);
        RelativeTargetSpeed relative_target_speed;
        relative_target_speed._continuous = "false";
        relative_target_speed._entityRef = strObjectName;
        relative_target_speed._speedTargetValueType = std::to_string(tempv);
        relative_target_speed._speedTargetValueType = "delta";
        TransitionDynamics transition_dynamics;
        transition_dynamics._dynamicsDimension = "rate";
        transition_dynamics._dynamicsShape = "linear";
        transition_dynamics._value = singleSceneEvent.ActionVer[i].strValue;
        SpeedActionTarget speed_action_target;
        speed_action_target.sub_RelativeTargetSpeed = (relative_target_speed);
        SpeedAction speed_action;
        speed_action.sub_SpeedActionDynamics = transition_dynamics;
        speed_action.sub_SpeedActionTarget = speed_action_target;
        LongitudinalAction longitudinal_action;
        longitudinal_action.sub_SpeedAction = (speed_action);
        PrivateAction private_action;
        private_action.sub_LongitudinalAction = (longitudinal_action);

        Action action;
        action._name = (actionVarName);
        action.sub_PrivateAction = (private_action);
        event.sub_Action.push_back(action);
        nindex++;
      } else {
        AbsoluteTargetSpeed asolute_target_speed;
        asolute_target_speed._value = ("-1.0000");
        TransitionDynamics transition_dynamics;
        transition_dynamics._dynamicsDimension = "rate";
        transition_dynamics._dynamicsShape = "linear";
        transition_dynamics._value = singleSceneEvent.ActionVer[i].strValue;
        SpeedActionTarget speed_action_target;
        speed_action_target.sub_AbsoluteTargetSpeed = (asolute_target_speed);
        SpeedAction speed_action;
        speed_action.sub_SpeedActionDynamics = transition_dynamics;
        speed_action.sub_SpeedActionTarget = speed_action_target;

        LongitudinalAction longitudinal_action;
        longitudinal_action.sub_SpeedAction = (speed_action);
        PrivateAction private_action;
        private_action.sub_LongitudinalAction = (longitudinal_action);

        Action action;
        action._name = (actionVarName);
        action.sub_PrivateAction = (private_action);
        event.sub_Action.push_back(action);
        nindex++;
      }
    } else if (singleSceneEvent.ActionVer[i].strType == "merge") {
      string vTemp = "0";
      if (singleSceneEvent.ActionVer[i].strSubType == "left") vTemp = "1";
      if (singleSceneEvent.ActionVer[i].strSubType == "right") vTemp = "-1";

      if (singleSceneEvent.ActionVer[i].strSubType == "left_in_lane" ||
          singleSceneEvent.ActionVer[i].strSubType == "right_in_lane") {
        vTemp = "0";
      }
      RelativeTargetLane relative_target_lane;
      relative_target_lane._entityRef = strObjectName;
      relative_target_lane._value = vTemp;
      LaneChangeTarget lane_change_target;
      lane_change_target.sub_RelativeTargetLane = (relative_target_lane);

      TransitionDynamics lane_change_action_dynamics;
      lane_change_action_dynamics._dynamicsDimension = "time";
      lane_change_action_dynamics._dynamicsShape = "sinusoidal";
      lane_change_action_dynamics._value = singleSceneEvent.ActionVer[i].strValue;
      LaneChangeAction lane_change_action;
      lane_change_action.sub_LaneChangeActionDynamics = lane_change_action_dynamics;
      lane_change_action.sub_LaneChangeTarget = lane_change_target;
      if (singleSceneEvent.ActionVer[i].strSubType == "right_in_lane") {
        string str = std::to_string(stod(singleSceneEvent.ActionVer[i].strLaneOffset) * (-1));
        lane_change_action._targetLaneOffset = (str);
      }
      if (singleSceneEvent.ActionVer[i].strSubType == "left_in_lane") {
        string str = std::to_string(stod(singleSceneEvent.ActionVer[i].strLaneOffset));
        lane_change_action._targetLaneOffset = (str);
      }

      LateralAction lateral_action;
      lateral_action.sub_LaneChangeAction = (lane_change_action);

      PrivateAction private_action;
      private_action.sub_LateralAction = (lateral_action);

      Action action;
      action._name = (actionVarName);
      action.sub_PrivateAction = (private_action);
      event.sub_Action.push_back(action);
      nindex++;
    } else if (actionType == "activate") {
      PrivateAction private_action1;
      ActivateControllerAction activateAction = SaveActivateAction(singleSceneEvent.ActionVer.at(i).strSubType);
      private_action1.sub_ActivateControllerAction = (activateAction);

      Action action;
      action._name = (actionVarName);
      action.sub_PrivateAction = (private_action1);
      event.sub_Action.push_back(action);
      nindex++;
    } else if (actionType == "assign" || actionType == "override") {
      PrivateAction private_action1;
      ControllerAction controlAction = SaveControlAction(singleSceneEvent.ActionVer.at(i));
      private_action1.sub_ControllerAction = (controlAction);

      Action action;
      action._name = (actionVarName);
      action.sub_PrivateAction = (private_action1);
      event.sub_Action.push_back(action);
      nindex++;
    } else if (actionType == "command") {
      std::string tmp;
      if (sAction.strSubType == "lane_change") {
        tmp.append("LaneChange:");
        tmp.append("active=" + sAction.strValue + ",");
        tmp.append("value=" + sAction.strLaneOffset);
      } else if (sAction.strSubType == "overtaking") {
        tmp.append("Overtaking:");
        tmp.append("active=" + sAction.strValue + ",");
        tmp.append("value=" + sAction.strLaneOffset);
      } else if (sAction.strSubType == "pull_over") {
        tmp.append("PullOver:");
        tmp.append("active=" + sAction.strValue);
      } else if (sAction.strSubType == "emergency_stop") {
        tmp.append("EmergencyStop:");
        tmp.append("active=" + sAction.strValue);
      } else if (sAction.strSubType == "lateral_speed_to_left") {
        tmp.append("LateralSpeedToLeft:");
        tmp.append("active=true,");
        tmp.append("value=" + sAction.strValue);
      } else if (sAction.strSubType == "lateral_speed_to_right") {
        tmp.append("LateralSpeedToRight:");
        tmp.append("active=true,");
        tmp.append("value=" + sAction.strValue);
      } else if (sAction.strSubType == "lateral_accel_to_left") {
        tmp.append("LateralAccelToLeft:");
        tmp.append("active=true,");
        tmp.append("value=" + sAction.strValue);
      } else if (sAction.strSubType == "lateral_accel_to_right") {
        tmp.append("LateralAccelToRight:");
        tmp.append("active=true,");
        tmp.append("value=" + sAction.strValue);
      } else if (sAction.strSubType == "userdefine") {
        tmp.append("UserDefine:");
        tmp.append("active=" + sAction.strValue + ",");
        tmp.append("value=" + sAction.strLaneOffset);
      } else {
        tmp = "";
      }

      string str = tmp;
      CustomCommandAction custom;
      custom._text = str;
      custom._type = "Command";
      UserDefinedAction userAction;
      userAction.sub_CustomCommandAction = custom;

      Action action;
      action._name = (actionVarName);
      action.sub_UserDefinedAction = (userAction);
      event.sub_Action.push_back(action);
      nindex++;
    } else if (actionType == "status") {
      CustomCommandAction CustomAction = SaveCustomAction(sAction);
      UserDefinedAction userAction;
      userAction.sub_CustomCommandAction = (CustomAction);
      Action action;
      action._name = (actionVarName);
      action.sub_UserDefinedAction = (userAction);
      event.sub_Action.push_back(action);
      nindex++;
    } else if (actionType == "lateralDistance") {
      LateralDistanceAction distanceAction;
      distanceAction._continuous = "false";
      distanceAction._entityRef = singleSceneEvent.ActionVer[i].sActionMulti.sLateralDistance.s_target_element;
      distanceAction._continuous = singleSceneEvent.ActionVer[i].sActionMulti.sLateralDistance.s_continuous;
      DynamicConstraints dynamic;
      if (std::atof(singleSceneEvent.ActionVer[i].sActionMulti.sLateralDistance.s_maxAcc.c_str()) >= 0) {
        dynamic._maxAcceleration = (singleSceneEvent.ActionVer[i].sActionMulti.sLateralDistance.s_maxAcc);
      }
      if (std::atof(singleSceneEvent.ActionVer[i].sActionMulti.sLateralDistance.s_maxDec.c_str()) >= 0) {
        dynamic._maxDeceleration = (singleSceneEvent.ActionVer[i].sActionMulti.sLateralDistance.s_maxDec);
      }
      if (std::atof(singleSceneEvent.ActionVer[i].sActionMulti.sLateralDistance.s_maxSpeed.c_str()) >= 0) {
        dynamic._maxSpeed = (singleSceneEvent.ActionVer[i].sActionMulti.sLateralDistance.s_maxSpeed);
      }
      distanceAction._distance = (singleSceneEvent.ActionVer[i].strValue);
      distanceAction.sub_DynamicConstraints = (dynamic);
      PrivateAction private_action;
      LateralAction lateral_action;
      lateral_action.sub_LateralDistanceAction = (distanceAction);
      private_action.sub_LateralAction = (lateral_action);

      Action action;
      action._name = (actionVarName);
      action.sub_PrivateAction = (private_action);
      event.sub_Action.push_back(action);
      nindex++;
    } else {
      SYSTEM_LOGGER_WARN("actionType = %s is not define!", actionType);
    }
  }
}

void XOSCWriter_1_0_v4::AddVehicle_V_A_M(vector<TimeTraggerData>::iterator ptr, string& actionBaseName,
                                         string& strObjectName, Event& event, int& num) {
  if (ptr->strVelocity != "null") {
    AbsoluteTargetSpeed asolute_target_speed;
    asolute_target_speed._value = ptr->strVelocity;
    TransitionDynamics transition_dynamics;
    transition_dynamics._dynamicsDimension = "time";
    transition_dynamics._dynamicsShape = "step";
    transition_dynamics._value = "0";
    SpeedActionTarget speed_action_target;
    speed_action_target.sub_AbsoluteTargetSpeed = (asolute_target_speed);
    SpeedAction speed_action;
    speed_action.sub_SpeedActionDynamics = transition_dynamics;
    speed_action.sub_SpeedActionTarget = speed_action_target;

    LongitudinalAction longitudinal_action;
    longitudinal_action.sub_SpeedAction = (speed_action);

    PrivateAction private_action;
    private_action.sub_LongitudinalAction = (longitudinal_action);

    string actionVarName = actionBaseName + std::to_string(++num);
    Action action;
    action._name = (actionVarName);
    action.sub_PrivateAction = (private_action);
    event.sub_Action.push_back(action);
  }
  if (ptr->strAcc != "null" && ptr->strAcc != "0.000") {
    if (ptr->strEndConditionType == "Velocity") {
      AbsoluteTargetSpeed asolute_target_speed;
      asolute_target_speed._value = ptr->strEndConditionValue;
      TransitionDynamics transition_dynamics;
      transition_dynamics._dynamicsDimension = "rate";
      transition_dynamics._dynamicsShape = "linear";
      transition_dynamics._value = ptr->strAcc;
      SpeedActionTarget speed_action_target;
      speed_action_target.sub_AbsoluteTargetSpeed = (asolute_target_speed);
      SpeedAction speed_action;
      speed_action.sub_SpeedActionDynamics = transition_dynamics;
      speed_action.sub_SpeedActionTarget = speed_action_target;
      LongitudinalAction longitudinal_action;
      longitudinal_action.sub_SpeedAction = (speed_action);
      PrivateAction private_action;
      private_action.sub_LongitudinalAction = (longitudinal_action);

      string actionVarName = actionBaseName + std::to_string(++num);
      Action action;
      action._name = (actionVarName);
      action.sub_PrivateAction = (private_action);
      event.sub_Action.push_back(action);
    } else if (ptr->strEndConditionType == "Time") {
      double tempv = stod(ptr->strAcc) * stod(ptr->strEndConditionValue);

      RelativeTargetSpeed relative_target_speed;
      relative_target_speed._continuous = "false";
      relative_target_speed._entityRef = strObjectName;
      relative_target_speed._speedTargetValueType = std::to_string(tempv);
      relative_target_speed._speedTargetValueType = "delta";
      TransitionDynamics transition_dynamics;
      transition_dynamics._dynamicsDimension = "rate";
      transition_dynamics._dynamicsShape = "linear";
      transition_dynamics._value = ptr->strAcc;
      SpeedActionTarget speed_action_target;
      speed_action_target.sub_RelativeTargetSpeed = (relative_target_speed);
      SpeedAction speed_action;
      speed_action.sub_SpeedActionDynamics = transition_dynamics;
      speed_action.sub_SpeedActionTarget = speed_action_target;
      LongitudinalAction longitudinal_action;
      longitudinal_action.sub_SpeedAction = (speed_action);
      PrivateAction private_action;
      private_action.sub_LongitudinalAction = (longitudinal_action);

      string actionVarName = actionBaseName + std::to_string(++num);
      Action action;
      action._name = (actionVarName);
      action.sub_PrivateAction = (private_action);
      event.sub_Action.push_back(action);
    }
  }
  if (ptr->strMerge != "null" && ptr->strMerge != "0") {
    // 变道
    string vTemp = "0";
    vTemp = ptr->strMerge;
    if (ptr->strMerge == "2" || ptr->strMerge == "-2") {
      // 道内换道  有offset
      vTemp = "0";
    }
    RelativeTargetLane relative_target_lane;
    relative_target_lane._entityRef = strObjectName;
    relative_target_lane._value = vTemp;
    LaneChangeTarget lane_change_target;
    lane_change_target.sub_RelativeTargetLane = (relative_target_lane);

    TransitionDynamics lane_change_action_dynamics;
    lane_change_action_dynamics._dynamicsDimension = "time";
    lane_change_action_dynamics._dynamicsShape = "sinusoidal";
    lane_change_action_dynamics._value = ptr->strOffsetOverTime;

    LaneChangeAction lane_change_action;
    lane_change_action.sub_LaneChangeActionDynamics = lane_change_action_dynamics;
    lane_change_action.sub_LaneChangeTarget = lane_change_target;
    if (ptr->strMerge == "-2") {
      ptr->strOffset = std::to_string(stod(ptr->strOffset) * (-1));
      lane_change_action._targetLaneOffset = (ptr->strOffset);
    }
    if (ptr->strMerge == "2") {
      lane_change_action._targetLaneOffset = (ptr->strOffset);
    }

    LateralAction lateral_action;
    lateral_action.sub_LaneChangeAction = (lane_change_action);

    PrivateAction private_action;
    private_action.sub_LateralAction = (lateral_action);

    string actionVarName = actionBaseName + std::to_string(++num);
    Action action;
    action._name = (actionVarName);
    action.sub_PrivateAction = (private_action);
    event.sub_Action.push_back(action);
  }
}

void XOSCWriter_1_0_v4::AddVehicle_V_A_M(vector<ConditionTraggerData>::iterator ptr, string& actionBaseName,
                                         string& strObjectName, Event& event, int& num) {
  // 有初始速度
  if (ptr->strVelocity != "null") {
    AbsoluteTargetSpeed asolute_target_speed;
    asolute_target_speed._value = (ptr->strVelocity);
    TransitionDynamics transition_dynamics;
    transition_dynamics._dynamicsDimension = "time";
    transition_dynamics._dynamicsShape = "step";
    transition_dynamics._value = "0";
    SpeedActionTarget speed_action_target;
    speed_action_target.sub_AbsoluteTargetSpeed = (asolute_target_speed);
    SpeedAction speed_action;
    speed_action.sub_SpeedActionDynamics = transition_dynamics;
    speed_action.sub_SpeedActionTarget = speed_action_target;

    LongitudinalAction longitudinal_action;
    longitudinal_action.sub_SpeedAction = (speed_action);

    PrivateAction private_action;
    private_action.sub_LongitudinalAction = (longitudinal_action);

    string actionVarName = actionBaseName + std::to_string(++num);
    Action action;
    action._name = (actionVarName);
    action.sub_PrivateAction = (private_action);
    event.sub_Action.push_back(action);
  }
  if (ptr->strAcc != "null" && ptr->strAcc != "0.000") {
    // 有加减速
    string obj_v = "null";
    if (ptr->strEndConditionType == "Velocity") {
      AbsoluteTargetSpeed asolute_target_speed;
      asolute_target_speed._value = (ptr->strEndConditionValue);
      TransitionDynamics transition_dynamics;
      transition_dynamics._dynamicsDimension = "rate";
      transition_dynamics._dynamicsShape = "linear";
      transition_dynamics._value = ptr->strAcc;
      SpeedActionTarget speed_action_target;
      speed_action_target.sub_AbsoluteTargetSpeed = (asolute_target_speed);
      SpeedAction speed_action;
      speed_action.sub_SpeedActionDynamics = transition_dynamics;
      speed_action.sub_SpeedActionTarget = speed_action_target;

      LongitudinalAction longitudinal_action;
      longitudinal_action.sub_SpeedAction = (speed_action);
      PrivateAction private_action;
      private_action.sub_LongitudinalAction = (longitudinal_action);

      string actionVarName = actionBaseName + std::to_string(++num);
      Action action;
      action._name = (actionVarName);
      action.sub_PrivateAction = (private_action);
      event.sub_Action.push_back(action);
    } else if (ptr->strEndConditionType == "Time") {
      double tempv = stod(ptr->strAcc) * stod(ptr->strEndConditionValue);

      RelativeTargetSpeed relative_target_speed;
      relative_target_speed._continuous = "false";
      relative_target_speed._entityRef = strObjectName;
      relative_target_speed._value = std::to_string(tempv);
      relative_target_speed._speedTargetValueType = "delta";
      TransitionDynamics transition_dynamics;
      transition_dynamics._dynamicsDimension = "rate";
      transition_dynamics._dynamicsShape = "linear";
      transition_dynamics._value = ptr->strAcc;
      SpeedActionTarget speed_action_target;
      speed_action_target.sub_RelativeTargetSpeed = (relative_target_speed);
      SpeedAction speed_action;
      speed_action.sub_SpeedActionDynamics = transition_dynamics;
      speed_action.sub_SpeedActionTarget = speed_action_target;

      LongitudinalAction longitudinal_action;
      longitudinal_action.sub_SpeedAction = (speed_action);
      PrivateAction private_action;
      private_action.sub_LongitudinalAction = (longitudinal_action);

      string actionVarName = actionBaseName + std::to_string(++num);
      Action action;
      action._name = (actionVarName);
      action.sub_PrivateAction = (private_action);
      event.sub_Action.push_back(action);
    }
  }
  if (ptr->strMerge != "null" && ptr->strMerge != "0") {
    // 变道
    string vTemp = "0";
    vTemp = ptr->strMerge;

    if (ptr->strMerge == "2" || ptr->strMerge == "-2") {
      vTemp = "0";
    }
    RelativeTargetLane relative_target_lane;
    relative_target_lane._entityRef = strObjectName;
    relative_target_lane._value = vTemp;

    LaneChangeTarget lane_change_target;
    lane_change_target.sub_RelativeTargetLane = (relative_target_lane);
    TransitionDynamics lane_change_action_dynamics;
    lane_change_action_dynamics._dynamicsDimension = "time";
    lane_change_action_dynamics._dynamicsShape = "sinusoidal";
    lane_change_action_dynamics._value = ptr->strOffsetOverTime;
    LaneChangeAction lane_change_action;
    lane_change_action.sub_LaneChangeActionDynamics = lane_change_action_dynamics;
    lane_change_action.sub_LaneChangeTarget = lane_change_target;
    if (ptr->strMerge == "-2") {
      ptr->strOffset = std::to_string(stod(ptr->strOffset) * (-1));
      lane_change_action._targetLaneOffset = (ptr->strOffset);
    }
    if (ptr->strMerge == "2") {
      lane_change_action._targetLaneOffset = (ptr->strOffset);
    }

    LateralAction lateral_action;
    lateral_action.sub_LaneChangeAction = (lane_change_action);

    PrivateAction private_action;
    private_action.sub_LateralAction = (lateral_action);

    string actionVarName = actionBaseName + std::to_string(++num);
    Action action;
    action._name = (actionVarName);
    action.sub_PrivateAction = (private_action);
    event.sub_Action.push_back(action);
  }
}

int XOSCWriter_1_0_v4::SaveOneVehicleActionsOldVersion(CTraffic& traffic, CVehicle& v, ManeuverGroup& maneuver_group,
                                                       std::string strObjectName, Maneuver& maneuver, int nTimeOrder,
                                                       int nManeuvers) {
  vector<TimeTraggerData> vecTT;

  vector<ConditionTraggerData> vecCT_ego_edu;
  vector<ConditionTraggerData> vecCT_ego_lane;
  vector<ConditionTraggerData> vecCT_ttc_edu;
  vector<ConditionTraggerData> vecCT_ttc_lane;

  JointData(traffic, v, vecTT, vecCT_ego_edu, vecCT_ego_lane, vecCT_ttc_edu, vecCT_ttc_lane);

  int nevent = 0;
  if (vecTT.size() != 0 || vecCT_ego_edu.size() != 0 || vecCT_ego_lane.size() != 0 || vecCT_ttc_edu.size() != 0 ||
      vecCT_ttc_lane.size() != 0) {
    for (auto itr = vecTT.begin(); itr != vecTT.end(); itr++) {
      SimulationTimeCondition simulation_time_condition;
      simulation_time_condition._rule = "greaterThan";
      simulation_time_condition._value = itr->strTraggerTime;

      ByValueCondition by_value_condition;
      by_value_condition.sub_SimulationTimeCondition = (simulation_time_condition);
      std::string ConditionBaseName = "Condition_";
      std::string ConditionVarName = ConditionBaseName + std::to_string(++nevent);
      Condition condition;
      condition._conditionEdge = "rising";
      condition._delay = "0";
      condition._name = ConditionVarName;
      condition.sub_ByValueCondition = (by_value_condition);
      ConditionGroup condition_group;
      condition_group.sub_Condition.push_back(condition);
      Trigger start_trigger;
      start_trigger.sub_ConditionGroup.push_back(condition_group);

      std::string eventBaseName = "Event";
      std::string eventVarName = eventBaseName + std::to_string(nevent);
      Event event = CreateEvent(start_trigger, eventVarName.c_str(), "overwrite");

      std::string actionBaseName = "Action";
      int actnum = 0;
      AddVehicle_V_A_M(itr, actionBaseName, strObjectName, event, actnum);

      if (actnum > 0) {
        maneuver.sub_Event.push_back(event);
        nManeuvers++;
      }
    }

    for (auto itr = vecCT_ego_edu.begin(); itr != vecCT_ego_edu.end(); itr++) {
      ConditionGroup condition_group;

      RelativeDistanceCondition relative_distance_condition;
      relative_distance_condition._entityRef = "Ego";
      relative_distance_condition._freespace = "true";
      relative_distance_condition._relativeDistanceType = "cartesianDistance";
      relative_distance_condition._rule = "lessThan";
      relative_distance_condition._value = itr->strTraggerValue;

      EntityCondition entity_condition;
      entity_condition.sub_RelativeDistanceCondition = (relative_distance_condition);

      EntityRef entity_ref;
      entity_ref._entityRef = strObjectName;
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);

      ByEntityCondition by_entity_condition;
      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;

      Condition condition;
      condition._conditionEdge = "rising";
      condition._delay = "0";
      condition._name = "Condition_RelativeDistance";
      condition.sub_ByEntityCondition = (by_entity_condition);

      condition_group.sub_Condition.push_back(condition);

      Trigger start_trigger;
      start_trigger.sub_ConditionGroup.push_back(condition_group);

      std::string eventBaseName = "Event";
      std::string eventVarName = eventBaseName + std::to_string(++nevent);
      Event event = CreateEvent(start_trigger, eventVarName.c_str(), "overwrite");

      std::string actionBaseName = "Action";
      int actnum = 0;
      AddVehicle_V_A_M(itr, actionBaseName, strObjectName, event, actnum);

      if (actnum > 0) {
        maneuver.sub_Event.push_back(event);
        nManeuvers++;
      }
    }

    for (auto itr = vecCT_ego_lane.begin(); itr != vecCT_ego_lane.end(); itr++) {
      ConditionGroup condition_group;

      RelativeDistanceCondition relative_distance_condition;
      relative_distance_condition._entityRef = "Ego";
      relative_distance_condition._freespace = "true";
      relative_distance_condition._relativeDistanceType = "longitudinal";
      relative_distance_condition._rule = "lessThan";
      relative_distance_condition._value = itr->strTraggerValue;

      EntityCondition entity_condition;
      entity_condition.sub_RelativeDistanceCondition = (relative_distance_condition);

      EntityRef entity_ref;
      entity_ref._entityRef = strObjectName;
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);

      ByEntityCondition by_entity_condition;
      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;

      Condition condition = CreateCondition("rising", "0", "Condition_RelativeDistance");
      condition.sub_ByEntityCondition = (by_entity_condition);

      condition_group.sub_Condition.push_back(condition);

      Trigger start_trigger;
      start_trigger.sub_ConditionGroup.push_back(condition_group);

      std::string eventBaseName = "Event";
      std::string eventVarName = eventBaseName + std::to_string(++nevent);
      Event event = CreateEvent(start_trigger, eventVarName.c_str(), "overwrite");

      std::string actionBaseName = "Action";
      int actnum = 0;
      AddVehicle_V_A_M(itr, actionBaseName, strObjectName, event, actnum);

      if (actnum > 0) {
        maneuver.sub_Event.push_back(event);
        nManeuvers++;
      }
    }

    for (auto itr = vecCT_ttc_edu.begin(); itr != vecCT_ttc_edu.end(); itr++) {
      ConditionGroup condition_group;
      TimeToCollisionConditionTarget time_to_collision_condition_target;
      EntityRef entity_ref;
      entity_ref._entityRef = "Ego";
      time_to_collision_condition_target.sub_EntityRef = entity_ref;

      TimeToCollisionCondition time_to_collision_condition;
      time_to_collision_condition.sub_TimeToCollisionConditionTarget = time_to_collision_condition_target;
      time_to_collision_condition._alongRoute = "false";
      time_to_collision_condition._freespace = "true";
      time_to_collision_condition._rule = "lessThan";
      time_to_collision_condition._value = itr->strTraggerValue;

      EntityCondition entity_condition;
      entity_condition.sub_TimeToCollisionCondition = (time_to_collision_condition);

      entity_ref._entityRef = strObjectName;
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);

      ByEntityCondition by_entity_condition;

      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;

      Condition condition;
      condition._conditionEdge = "rising";
      condition._delay = "0";
      condition._name = "Condition_TTC";
      condition.sub_ByEntityCondition = (by_entity_condition);

      condition_group.sub_Condition.push_back(condition);

      Trigger start_trigger;
      start_trigger.sub_ConditionGroup.push_back(condition_group);

      // 条件写好后  写action
      std::string eventBaseName = "Event";
      std::string eventVarName = eventBaseName + std::to_string(++nevent);
      Event event = CreateEvent(start_trigger, eventVarName.c_str(), "overwrite");

      std::string actionBaseName = "Action";
      int actnum = 0;
      AddVehicle_V_A_M(itr, actionBaseName, strObjectName, event, actnum);

      if (actnum > 0) {
        maneuver.sub_Event.push_back(event);
        nManeuvers++;
      }
    }

    for (auto itr = vecCT_ttc_lane.begin(); itr != vecCT_ttc_lane.end(); itr++) {
      ConditionGroup condition_group;
      TimeToCollisionConditionTarget time_to_collision_condition_target;
      EntityRef entity_ref;
      entity_ref._entityRef = "Ego";

      time_to_collision_condition_target.sub_EntityRef = entity_ref;

      TimeToCollisionCondition time_to_collision_condition;
      time_to_collision_condition.sub_TimeToCollisionConditionTarget = time_to_collision_condition_target;
      time_to_collision_condition._alongRoute = "true";
      time_to_collision_condition._freespace = "true";
      time_to_collision_condition._rule = "lessThan";
      time_to_collision_condition._value = itr->strTraggerValue;

      EntityCondition entity_condition;
      entity_condition.sub_TimeToCollisionCondition = (time_to_collision_condition);

      entity_ref._entityRef = strObjectName;
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);

      ByEntityCondition by_entity_condition;
      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;

      Condition condition = CreateCondition("rising", "0", "Condition_TTC");
      condition.sub_ByEntityCondition = (by_entity_condition);

      condition_group.sub_Condition.push_back(condition);

      Trigger start_trigger;
      start_trigger.sub_ConditionGroup.push_back(condition_group);

      // 条件写好后  写action
      std::string eventBaseName = "Event";
      std::string eventVarName = eventBaseName + std::to_string(++nevent);
      Event event = CreateEvent(start_trigger, eventVarName.c_str(), "overwrite");

      std::string actionBaseName = "Action";
      int actnum = 0;
      AddVehicle_V_A_M(itr, actionBaseName, strObjectName, event, actnum);

      if (actnum > 0) {
        maneuver.sub_Event.push_back(event);
        nManeuvers++;
      }
    }
  }
  if (nManeuvers > 0) {
    maneuver_group.sub_Maneuver.push_back(maneuver);
  }
  return nManeuvers;
}

#include <algorithm>
#include <vector>
int XOSCWriter_1_0_v4::SaveOneVehicleActionsNewVersion(CTraffic& traffic, CVehicle& v, ManeuverGroup& maneuver_group,
                                                       std::string strObjectName, Maneuver& maneuver, int nTimeOrder,
                                                       int nManeuvers) {
  CTraffic::SceneEventMap& mapSceneEvents = traffic.SceneEvents();

  string strEventID = v.m_strEventId;
  SYSTEM_LOGGER_INFO("SaveOneVehicleActionsNewVersion ..eventid=%s", strEventID.c_str());
  std::vector<std::string> streventids;
  boost::algorithm::split(streventids, strEventID, boost::algorithm::is_any_of(","));
  std::sort(streventids.begin(), streventids.end());
  auto index_it = std::unique(streventids.begin(), streventids.end());
  streventids.erase(index_it, streventids.end());

  for (auto itr : streventids) {
    auto p = mapSceneEvents.find(itr);
    if (p != mapSceneEvents.end()) {
      SYSTEM_LOGGER_INFO("SaveOneVehicleActionsNewVersion find id=%s", itr.c_str());
      SceneEvent& singleSceneEvent = p->second;
      Trigger start_trigger;
      Event event = CreateEvent(start_trigger, singleSceneEvent.strName, "overwrite");
      event._maximumExecutionCount = "1";
      bool ishaveaction = false;
      AddVehicleAction_V_A_M(singleSceneEvent, strObjectName, event, ishaveaction);
      if (ishaveaction) {
        AddVehicleTrigger_V_A_M(singleSceneEvent, event, mapSceneEvents);

        maneuver.sub_Event.push_back(event);
        nManeuvers++;
      }
    }
  }
  if (nManeuvers > 0) {
    maneuver_group.sub_Maneuver.push_back(maneuver);
  }

  return nManeuvers;
}

void XOSCWriter_1_0_v4::AddVehicleTrigger_V_A_M(SceneEvent& singleSceneEvent, Event& event,
                                                CTraffic::SceneEventMap& eventMap) {
  int index = 0;

  ConditionGroup condition_group;
  for (auto it : singleSceneEvent.ConditionVec) {
    std::string ConditionVarName = "Condition_" + std::to_string(index);
    // convert
    std::transform(it.strTargetElement.begin(), it.strTargetElement.end(), it.strTargetElement.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    Condition condition = CreateCondition(it.strBoundary, "0", ConditionVarName);
    if (it.strType == "time_trigger") {
      SimulationTimeCondition simulation_time_condition;
      simulation_time_condition._rule = formatRule(it.strEquationOp);
      simulation_time_condition._value = it.strTriggerTime;
      ByValueCondition by_value_condition;
      by_value_condition.sub_SimulationTimeCondition = (simulation_time_condition);
      condition.sub_ByValueCondition = (by_value_condition);
      condition_group.sub_Condition.push_back(condition);
    } else if (it.strType == "velocity_trigger") {
      if (it.strSpeedType == "absolute") {
        SpeedCondition speed_condition;
        speed_condition._rule = formatRule(it.strEquationOp);
        speed_condition._value = formatRule(it.strThreshold);
        EntityCondition entity_condition;
        entity_condition.sub_SpeedCondition = (speed_condition);
        EntityRef entity_ref;
        entity_ref._entityRef = (it.strTargetElement);
        TriggeringEntities triggering_entities;
        triggering_entities._triggeringEntitiesRule = ("any");
        triggering_entities.sub_EntityRef.push_back(entity_ref);
        ByEntityCondition by_entity_condition;
        by_entity_condition.sub_TriggeringEntities = triggering_entities;
        by_entity_condition.sub_EntityCondition = entity_condition;
        condition.sub_ByEntityCondition = (by_entity_condition);
        condition_group.sub_Condition.push_back(condition);
      } else {
        RelativeSpeedCondition relative_speed_condition;
        relative_speed_condition._entityRef = it.strTargetElement;
        relative_speed_condition._rule = formatRule(it.strEquationOp);
        relative_speed_condition._value = it.strThreshold;
        EntityCondition entity_condition;
        entity_condition.sub_RelativeSpeedCondition = (relative_speed_condition);
        EntityRef entity_ref;
        entity_ref._entityRef = (it.strTargetElement);
        TriggeringEntities triggering_entities;
        triggering_entities._triggeringEntitiesRule = ("any");
        triggering_entities.sub_EntityRef.push_back(entity_ref);
        ByEntityCondition by_entity_condition;
        by_entity_condition.sub_TriggeringEntities = triggering_entities;
        by_entity_condition.sub_EntityCondition = entity_condition;
        condition.sub_ByEntityCondition = (by_entity_condition);
        condition_group.sub_Condition.push_back(condition);
      }

    } else if (it.strType == "ttc_trigger") {
      TimeToCollisionConditionTarget time_to_collision_condition_target;
      EntityRef entity_ref;
      entity_ref._entityRef = (it.strTargetElement);
      time_to_collision_condition_target.sub_EntityRef = entity_ref;
      string stralongRoute;
      if (it.strDisType == "euclideandistance") {
        stralongRoute = "false";
      } else {
        stralongRoute = "true";
      }
      TimeToCollisionCondition time_to_collision_condition;
      time_to_collision_condition.sub_TimeToCollisionConditionTarget = time_to_collision_condition_target;
      time_to_collision_condition._alongRoute = stralongRoute;
      time_to_collision_condition._freespace = "true";
      time_to_collision_condition._rule = formatRule(it.strEquationOp);
      time_to_collision_condition._value = it.strThreshold;

      EntityCondition entity_condition;
      entity_condition.sub_TimeToCollisionCondition = (time_to_collision_condition);
      entity_ref._entityRef = it.strTargetElement;
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);
      ByEntityCondition by_entity_condition;
      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;
      condition.sub_ByEntityCondition = (by_entity_condition);
      condition_group.sub_Condition.push_back(condition);
    } else if (it.strType == "distance_trigger") {
      string strDistanceType;
      if (it.strDisType == "euclideandistance") {
        strDistanceType = "cartesianDistance";
      } else {
        strDistanceType = "longitudinal";
      }
      RelativeDistanceCondition relative_distance_condition;
      relative_distance_condition._entityRef = it.strTargetElement;
      relative_distance_condition._freespace = "true";
      relative_distance_condition._relativeDistanceType = strDistanceType;
      relative_distance_condition._rule = formatRule(it.strEquationOp);
      relative_distance_condition._value = it.strThreshold;
      EntityCondition entity_condition;
      entity_condition.sub_RelativeDistanceCondition = (relative_distance_condition);

      EntityRef entity_ref;
      entity_ref._entityRef = (it.strTargetElement);
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);

      ByEntityCondition by_entity_condition;
      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;
      condition.sub_ByEntityCondition = (by_entity_condition);
      condition_group.sub_Condition.push_back(condition);
    } else if (it.strType == "reach_abs_position") {
      WorldPosition world_position_temp;
      world_position_temp._x = it.strLon;
      world_position_temp._y = it.strLat;
      world_position_temp._z = it.strAlt;
      world_position_temp._h = "0";
      world_position_temp._p = "0";
      world_position_temp._r = it.strRadius;

      Position position_temp;  //
      position_temp.sub_WorldPosition = (world_position_temp);

      ReachPositionCondition reach_position_condition;
      reach_position_condition.sub_Position = position_temp;
      reach_position_condition._tolerance = it.strTolerance;

      EntityCondition entity_condition;
      entity_condition.sub_ReachPositionCondition = (reach_position_condition);

      EntityRef entity_ref;
      entity_ref._entityRef = (it.strTargetElement);
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);

      ByEntityCondition by_entity_condition;
      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;
      condition.sub_ByEntityCondition = (by_entity_condition);
      condition_group.sub_Condition.push_back(condition);

    } else if (it.strType == "reach_abs_lane") {
      Orientation orientation;
      orientation._type = (it.strPostionType);
      LanePosition lane_position_temp;
      lane_position_temp._laneId = it.strLaneId;
      lane_position_temp._offset = it.strRoadId;
      lane_position_temp._roadId = it.strLongitudinalOffset;
      lane_position_temp._offset = (it.strLateralOffset);
      lane_position_temp.sub_Orientation = (orientation);
      Position position_temp;
      position_temp.sub_LanePosition = (lane_position_temp);

      ReachPositionCondition reach_position_condition;
      reach_position_condition.sub_Position = position_temp;
      reach_position_condition._tolerance = it.strTolerance;

      EntityCondition entity_condition;
      entity_condition.sub_ReachPositionCondition = (reach_position_condition);

      EntityRef entity_ref;
      entity_ref._entityRef = (it.strTargetElement);
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);

      ByEntityCondition by_entity_condition;
      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;
      condition.sub_ByEntityCondition = (by_entity_condition);
      condition_group.sub_Condition.push_back(condition);

    } else if (it.strType == "ego_attach_laneid_custom") {
      string strParameterValue = it.strRoadId + "," + it.strSectionId + "," + it.strLaneId;
      ParameterCondition parameter_condition;
      parameter_condition._parameterRef = it.strVariate;
      parameter_condition._rule = formatRule(it.strEquationOp);
      parameter_condition._value = strParameterValue;
      ByValueCondition by_value_condition;
      by_value_condition.sub_ParameterCondition = (parameter_condition);
      condition.sub_ByValueCondition = (by_value_condition);
      condition_group.sub_Condition.push_back(condition);
    } else if (it.strType == "timeheadway_trigger") {
      TimeHeadwayCondition timeheadway;
      timeheadway._alongRoute = "false";
      timeheadway._entityRef = it.strTargetElement;
      timeheadway._freespace = "false";
      timeheadway._rule = formatRule(it.strEquationOp);
      timeheadway._value = it.strThreshold;

      EntityRef entity_ref;
      entity_ref._entityRef = (it.strTargetElement);
      EntityCondition entity_condition;
      entity_condition.sub_TimeHeadwayCondition = (timeheadway);
      entity_ref._entityRef = it.strTargetElement;
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);
      ByEntityCondition by_entity_condition;
      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;
      condition.sub_ByEntityCondition = (by_entity_condition);
      condition_group.sub_Condition.push_back(condition);
    } else if (it.strType == "element_state") {
      std::string strElementRef;
      int pos = it.strElementRef.find("_");
      if (pos >= it.strElementRef.size()) {
        SYSTEM_LOGGER_ERROR("element_state ElementRef is error ,ElementRef = ", it.strElementRef.c_str());
        continue;
      }
      std::string event_id = it.strElementRef.substr(0, pos);
      std::string action_id = it.strElementRef.substr(pos + 1, it.strElementRef.size() - pos - 1);
      //
      if (eventMap.find(event_id) != eventMap.end()) {
        bool flag = false;
        for (auto& itAction : eventMap.find(event_id)->second.ActionVer) {
          if (itAction.strId == action_id) {
            strElementRef = eventMap[event_id].strName + "_action_" + it.strElementRef;
            flag = true;
            break;
          }
        }
        if (flag == false) {
          SYSTEM_LOGGER_ERROR("element_state ElementRef is error ,ElementRef = ", it.strElementRef.c_str());
          continue;
        }
      } else {
        SYSTEM_LOGGER_ERROR("element_state ElementRef is error ,ElementRef = ", it.strElementRef.c_str());
        continue;
      }
      StoryboardElementStateCondition storyboard_condition;
      storyboard_condition._storyboardElementRef = strElementRef;
      storyboard_condition._state = it.strState;
      storyboard_condition._storyboardElementType = it.strElementType;

      ByValueCondition by_value_condition;
      by_value_condition.sub_StoryboardElementStateCondition = (storyboard_condition);
      condition.sub_ByValueCondition = (by_value_condition);
      condition_group.sub_Condition.push_back(condition);
    }
    index++;
  }
  event.sub_StartTrigger->sub_ConditionGroup.push_back(condition_group);
}

int XOSCWriter_1_0_v4::SavePedestriansStory(Act& act, CTraffic& traffic) {
  CTraffic::PedestrianV2Map& mapPedestrians = traffic.PedestriansV2();

  int nTotalManuvers = 0;

  string strindex = std::to_string(traffic.Vehicles().size() + 1);
  // vehicles
  CTraffic::PedestrianV2Map::iterator itr = mapPedestrians.begin();
  for (; itr != mapPedestrians.end(); ++itr) {
    std::string objectName;
    if (m_set_PedestrianType.find(itr->second.m_strType) != m_set_PedestrianType.end()) {
      objectName = s_Entities_Pedestrian_Name_Prefix + itr->first;
    } else {
      objectName = s_Entities_Vehicle_Name_Prefix + strindex;

      stoi(strindex);
      strindex = std::to_string(stoi(strindex) + 1);
    }

    Actors actors;
    actors._selectTriggeringEntities = ("false");
    EntityRef entity_ref;
    entity_ref._entityRef = (objectName);
    actors.sub_EntityRef.push_back(entity_ref);

    m_ManeuverGroupIndex++;
    std::string maneuverGroupName = "ManeuverGroup" + std::to_string(m_ManeuverGroupIndex);
    ManeuverGroup maneuver_group;
    maneuver_group.sub_Actors = actors;
    maneuver_group._maximumExecutionCount = "1";
    maneuver_group._name = maneuverGroupName.c_str();
    maneuver_group.sub_Actors = actors;
    // Maneuver
    int nManeuvers = SaveOnePedestrianActions(traffic, itr->second, maneuver_group, objectName);
    if (nManeuvers == 0) {
      continue;
    }
    nTotalManuvers += nManeuvers;
    act.sub_ManeuverGroup.push_back(maneuver_group);
  }
  return 0;
}

int XOSCWriter_1_0_v4::SaveOnePedestrianActions(CTraffic& traffic, CPedestrianV2& p, ManeuverGroup& maneuver_group,
                                                std::string strObjectName) {
  int nTimeOrder = 0;
  int nOrder = 0;
  int nManeuvers = 0;

  Maneuver maneuver;
  maneuver._name = ("Maneuver0");

  CTraffic::RouteMap& mapRoutes = traffic.Routes();
  CTraffic::RouteMap::iterator rItr = mapRoutes.find(p.m_strRouteID);
  if (rItr == mapRoutes.end()) {
    // assert(false);
    SYSTEM_LOGGER_ERROR("%s mapRoutes none", p.m_strRouteID.c_str());
    return 0;
  }

  int nRet = ConvertOnePedestrianRoutings(p, rItr->second, maneuver, strObjectName, nManeuvers, nTimeOrder, nOrder);
  //
  if (p.isDynamicObsctale()) {
    CTraffic::SceneEventMap& mapSceneEvents = traffic.SceneEvents();
    string strEventID = p.m_strEventId;
    std::vector<std::string> streventids;
    boost::algorithm::split(streventids, strEventID, boost::algorithm::is_any_of(","));
    std::sort(streventids.begin(), streventids.end());
    auto index_it = std::unique(streventids.begin(), streventids.end());
    streventids.erase(index_it, streventids.end());
    for (auto itr : streventids) {
      auto p = mapSceneEvents.find(itr);
      if (p != mapSceneEvents.end()) {
        SceneEvent& singleSceneEvent = p->second;
        Trigger start_trigger;
        Event event = CreateEvent(start_trigger, singleSceneEvent.strName, "overwrite");
        event._maximumExecutionCount = "1";
        // 开始添加event
        bool ishaveaction = false;
        AddVehicleAction_V_A_M(singleSceneEvent, strObjectName, event, ishaveaction);
        if (ishaveaction) {
          AddVehicleTrigger_V_A_M(singleSceneEvent, event, mapSceneEvents);
          maneuver.sub_Event.push_back(event);
          nManeuvers++;
        }
      }
    }
    if (nManeuvers > 0) {
      maneuver_group.sub_Maneuver.push_back(maneuver);
    }

    return nManeuvers;
  }

  // velocity
  CTraffic::PedestrianVelocityMap& mapVelocities = traffic.PedestrianVelocities();
  CTraffic::PedestrianVelocityMap::iterator vItr = mapVelocities.find(p.m_strID);
  if (vItr == mapVelocities.end()) {
    if (nManeuvers > 0) {
      maneuver_group.sub_Maneuver.push_back(maneuver);
    }

    return nManeuvers;
  }
  nTimeOrder = 0;
  nRet = ConvertOnePedestrianVelocities(p, vItr->second, maneuver, strObjectName, nManeuvers, nTimeOrder, nOrder);

  if (nManeuvers > 0) {
    maneuver_group.sub_Maneuver.push_back(maneuver);
  }

  return nManeuvers;
}

std::string XOSCWriter_1_0_v4::formatRule(const std::string str) {
  if (str == "eq")
    return "equalTo";
  else if (str == "gt" || str == "gte")
    return "greaterThan";
  else if (str == "lt" || str == "lte")
    return "lessThan";
  return "equalTo";
}

int XOSCWriter_1_0_v4::ConvertOnePedestrianRoutings(CPedestrianV2& p, CRoute& r, Maneuver& maneuver,
                                                    std::string strObjectName, int& nEvents, int& nTimeOrder,
                                                    int& nOrder) {
  r.FromStr();
  r.ConvertToValue();
  r.ExtractPath();

  sPath& routingPath = r.m_path;
  if (routingPath.m_vPoints.size() > 1) {
    std::string strOrder;
    std::string strTimeOrder;

    strOrder = boost::lexical_cast<std::string>(nOrder);
    strTimeOrder = boost::lexical_cast<std::string>(nTimeOrder);

    Polyline polyline;

    double dX, dY;
    double dLastX, dLastY;
    double dLastTime = 0;
    for (int i = 0; i < routingPath.m_vPoints.size(); ++i) {
      double dLon = -999;
      double dLat = -999;

      // first point
      if (i == 0) {
        // ConvertSimPosition2Local(routingPath.m_vPoints[i].m_dLon, routingPath.m_vPoints[i].m_dLat
        // , p.m_laneID, p.m_fStartShift, p.m_fStartOffset, dLastX, dLastY);
        ConvertSimPosition2Local(routingPath.m_vPoints[i].m_dLon, routingPath.m_vPoints[i].m_dLat, p.m_strLaneID,
                                 p.m_strStartShift, p.m_strOffset, dLastX, dLastY);
        // ExtractPosition(r, p.m_laneID, p.m_fStartShift, p.m_fStartOffset, dLon, dLat);
        double dFinalYaw = 0;
        ExtractPosition(r, p.m_strLaneID, p.m_strStartShift, p.m_strOffset, dLon, dLat, dFinalYaw, false);
      } else {
        dX = routingPath.m_vPoints[i].m_dLon;
        dY = routingPath.m_vPoints[i].m_dLat;
        ConvertLonLat2Local(dX, dY);
        double dLen = (dX - dLastX) * (dX - dLastX) + (dY - dLastY) * (dY - dLastY);
        dLen = sqrt(dLen);
        if (p.m_fStartVelocity > 0) {
          double dDeltaTime = dLen / p.m_fStartVelocity;
          dLastTime += dDeltaTime;
        }
        dLastX = dX;
        dLastY = dY;

        dLon = routingPath.m_vPoints[i].m_dLon;
        dLat = routingPath.m_vPoints[i].m_dLat;
        ConvertLonLat2XODR(dLon, dLat);
      }
      WorldPosition world_position_temp;
      world_position_temp._x = DoubleToString(dLon);
      world_position_temp._y = DoubleToString(dLat);
      world_position_temp._z = "0";
      world_position_temp._h = "0";
      world_position_temp._p = "0";
      world_position_temp._r = "0";

      Position position_temp;  //
      position_temp.sub_WorldPosition = (world_position_temp);

      Vertex vertex;
      vertex.sub_Position = position_temp;
      vertex._time = std::to_string(dLastTime);
      polyline.sub_Vertex.push_back(vertex);
    }

    Shape shape;
    shape.sub_Polyline = (polyline);

    Trajectory trajectory;
    trajectory.sub_Shape = shape;
    trajectory._closed = "false";
    trajectory._name = "traj1";

    TimeReference time_reference;
    TrajectoryFollowingMode trajectory_following_mode;
    trajectory_following_mode._followingMode = ("position");
    FollowTrajectoryAction follow_trajectory_action;
    follow_trajectory_action.sub_TimeReference = time_reference;
    follow_trajectory_action.sub_TrajectoryFollowingMode = trajectory_following_mode;
    follow_trajectory_action.sub_Trajectory = (trajectory);
    RoutingAction routing_action;
    routing_action.sub_FollowTrajectoryAction = (follow_trajectory_action);
    PrivateAction private_action;
    private_action.sub_RoutingAction = (routing_action);

    Action action;
    action._name = ("ActionPedRouting");
    action.sub_PrivateAction = (private_action);

    // start trigger
    SimulationTimeCondition simulation_time_condition;
    simulation_time_condition._rule = "greaterThan";
    simulation_time_condition._value = "0";

    ByValueCondition by_value_condition;
    by_value_condition.sub_SimulationTimeCondition = (simulation_time_condition);
    Condition condition = CreateCondition("rising", "0", "ConditionPedRouting");
    condition.sub_ByValueCondition = (by_value_condition);
    ConditionGroup condition_group;
    condition_group.sub_Condition.push_back(condition);
    Trigger start_trigger;
    start_trigger.sub_ConditionGroup.push_back(condition_group);

    std::string eventVarName = "Event";
    eventVarName += strOrder;
    Event event = CreateEvent(start_trigger, eventVarName, "overwrite");
    event.sub_Action.push_back(action);
    maneuver.sub_Event.push_back(event);

    nEvents++;
    nTimeOrder++;
    nOrder++;
  }
  return 0;
}

int XOSCWriter_1_0_v4::ConvertOnePedestrianVelocities(CPedestrianV2& p, CPedestrianVelocity& v, Maneuver& maneuver,
                                                      std::string strObjectName, int& nEvents, int& nTimeOrder,
                                                      int& nOrder) {
  // velocity
  v.m_nodes.clear();
  v.ConvertToEventValue();
  v.ConvertToTimeValue();

  std::string strOrder;
  std::string strTimeOrder;

  CPedestrianVelocity::EventNodes::iterator evItr = v.m_nodes.begin();
  for (; evItr != v.m_nodes.end(); ++evItr) {
    strOrder = boost::lexical_cast<std::string>(nOrder);
    strTimeOrder = boost::lexical_cast<std::string>(nTimeOrder);
    AbsoluteTargetSpeed asolute_target_speed;
    asolute_target_speed._value = std::to_string(evItr->dVelocity);
    TransitionDynamics transition_dynamics;
    transition_dynamics._dynamicsDimension = "time";
    transition_dynamics._dynamicsShape = "step";
    transition_dynamics._value = "0";
    SpeedActionTarget speed_action_target;
    speed_action_target.sub_AbsoluteTargetSpeed = (asolute_target_speed);
    SpeedAction speed_action;
    speed_action.sub_SpeedActionDynamics = transition_dynamics;
    speed_action.sub_SpeedActionTarget = speed_action_target;
    LongitudinalAction longitudinal_action;
    longitudinal_action.sub_SpeedAction = (speed_action);

    PrivateAction private_action1;
    private_action1.sub_LongitudinalAction = (longitudinal_action);

    std::string actionVarName = "Action";
    actionVarName += strOrder;
    Action action1;
    action1._name = (actionVarName);
    action1.sub_PrivateAction = (private_action1);

    Orientation orientation1;
    orientation1._type = "relative";
    orientation1._h = "0";
    RelativeObjectPosition relative_object_position1;
    relative_object_position1._entityRef = strObjectName;
    relative_object_position1._dx = "0";
    relative_object_position1._dy = "0";
    relative_object_position1.sub_Orientation = (orientation1);
    Position position1;
    position1.sub_RelativeObjectPosition = (relative_object_position1);
    Vertex vertex1;
    vertex1.sub_Position = position1;
    vertex1._time = "0";

    Orientation orientation2;
    orientation2._type = "relative";
    //
    double directtion = evItr->dDirection;
    orientation2._h = std::to_string(directtion * 3.141 / 180.0);

    RelativeObjectPosition relative_object_position2;
    relative_object_position2._entityRef = strObjectName;
    relative_object_position2._dx = "0";
    relative_object_position2._dy = "0";
    relative_object_position2.sub_Orientation = (orientation2);
    Position position2;
    position2.sub_RelativeObjectPosition = (relative_object_position2);
    Vertex vertex2;
    vertex2.sub_Position = position2;
    vertex2._time = "0";

    Polyline polyline;
    polyline.sub_Vertex.push_back(vertex1);
    polyline.sub_Vertex.push_back(vertex2);

    Shape shape;
    shape.sub_Polyline = (polyline);

    Trajectory trajectory;
    trajectory.sub_Shape = shape;
    trajectory._closed = "false";
    trajectory._name = "Truning";

    TimeReference time_reference;
    TrajectoryFollowingMode trajectory_following_mode;
    trajectory_following_mode._followingMode = "position";
    FollowTrajectoryAction follow_trajectory_action;
    follow_trajectory_action.sub_TimeReference = time_reference;
    follow_trajectory_action.sub_TrajectoryFollowingMode = trajectory_following_mode;
    follow_trajectory_action.sub_Trajectory = (trajectory);
    RoutingAction routing_action;
    routing_action.sub_FollowTrajectoryAction = (follow_trajectory_action);
    PrivateAction private_action2;
    private_action2.sub_RoutingAction = (routing_action);

    actionVarName = "ActionChangeDirection";
    actionVarName += strOrder;
    Action action2;
    action2._name = (actionVarName);
    action2.sub_PrivateAction = (private_action2);

    std::string ConditionVarName = "Condition_";
    ConditionGroup condition_group;
    if (std::abs(evItr->dVelocity) < 1e-3) continue;

    // start trigger
    if (evItr->nTriggerType == TCT_TIME_ABSOLUTE) {
      SimulationTimeCondition simulation_time_condition;
      simulation_time_condition._rule = "greaterThan";
      simulation_time_condition._value = std::to_string(evItr->dTriggerValue);

      ByValueCondition by_value_condition;
      by_value_condition.sub_SimulationTimeCondition = (simulation_time_condition);

      ConditionVarName += strTimeOrder;
      Condition condition = CreateCondition("rising", "0", ConditionVarName);
      condition.sub_ByValueCondition = (by_value_condition);

      condition_group.sub_Condition.push_back(condition);
    } else if (evItr->nTriggerType == TCT_TIME_RELATIVE) {
      TimeToCollisionConditionTarget time_to_collision_condition_target;
      EntityRef entity_ref;
      entity_ref._entityRef = "Ego";
      time_to_collision_condition_target.sub_EntityRef = entity_ref;

      string strBool = "true";
      if (evItr->nDistanceMode == TDM_EUCLIDEAN_DISTANCE) {
        strBool = "false";
      } else if (evItr->nDistanceMode == TDM_LANE_PROJECTION) {
        strBool = "true";
      }

      TimeToCollisionCondition time_to_collision_condition;
      time_to_collision_condition.sub_TimeToCollisionConditionTarget = time_to_collision_condition_target;
      time_to_collision_condition._alongRoute = strBool;
      time_to_collision_condition._freespace = "true";
      time_to_collision_condition._rule = "lessThan";
      time_to_collision_condition._value = std::to_string(evItr->dTriggerValue);

      EntityCondition entity_condition;
      entity_condition.sub_TimeToCollisionCondition = (time_to_collision_condition);

      entity_ref._entityRef = strObjectName;
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);

      ByEntityCondition by_entity_condition;
      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;
      ConditionVarName = "ConditionTTCLong_" + strTimeOrder;
      Condition condition = CreateCondition("rising", "0", ConditionVarName);
      condition.sub_ByEntityCondition = (by_entity_condition);

      condition_group.sub_Condition.push_back(condition);

    } else if (evItr->nTriggerType == TCT_POSITION_RELATIVE) {
      if (evItr->nDistanceMode == TDM_EUCLIDEAN_DISTANCE) {
        ConditionVarName = "ConditionRelativeDisLong_" + strTimeOrder;

      } else if (evItr->nDistanceMode == TDM_LANE_PROJECTION) {
        ConditionVarName = "ConditionRelativeDisCartesian_" + strTimeOrder;

      } else {
        SYSTEM_LOGGER_ERROR("convert merge distance mode error!");
      }

      std::string strRelativeDistanceMode = "longitudinal";
      if (evItr->nDistanceMode == TDM_EUCLIDEAN_DISTANCE) {
        strRelativeDistanceMode = "cartesianDistance";
      } else if (evItr->nDistanceMode == TDM_LANE_PROJECTION) {
        strRelativeDistanceMode = "longitudinal";
      } else {
        SYSTEM_LOGGER_ERROR("convert merge distance mode error!");
      }

      osc::RelativeDistanceCondition relative_distance_condition;
      relative_distance_condition._entityRef = "Ego";
      relative_distance_condition._freespace = "true";
      relative_distance_condition._relativeDistanceType = strRelativeDistanceMode;
      relative_distance_condition._rule = "lessThan";
      relative_distance_condition._value = std::to_string(evItr->dTriggerValue);
      osc::EntityCondition entity_condition;
      entity_condition.sub_RelativeDistanceCondition = (relative_distance_condition);

      EntityRef entity_ref;
      entity_ref._entityRef = (strObjectName);
      TriggeringEntities triggering_entities;
      triggering_entities._triggeringEntitiesRule = ("any");
      triggering_entities.sub_EntityRef.push_back(entity_ref);

      ByEntityCondition by_entity_condition;
      by_entity_condition.sub_TriggeringEntities = triggering_entities;
      by_entity_condition.sub_EntityCondition = entity_condition;
      Condition condition = CreateCondition("rising", "0", ConditionVarName);
      condition.sub_ByEntityCondition = (by_entity_condition);
      condition_group.sub_Condition.push_back(condition);
    } else {
      continue;
    }

    Trigger start_trigger;
    start_trigger.sub_ConditionGroup.push_back(condition_group);

    std::string eventVarName = "Event";
    eventVarName += strOrder;
    Event event = CreateEvent(start_trigger, eventVarName, "overwrite");
    event.sub_Action.push_back(action1);
    event.sub_Action.push_back(action2);

    maneuver.sub_Event.push_back(event);
    nEvents++;
    nTimeOrder++;
    nOrder++;
  }

  return 0;
}

// save
osc::Center XOSCWriter_1_0_v4::CreateCenter(std::string x, std::string y, std::string z) {
  osc::Center p;
  p._x = x;
  p._y = y;
  p._z = z;
  return p;
}

osc::Dimensions XOSCWriter_1_0_v4::CreateDimensions(std::string height, std::string length, std::string width) {
  osc::Dimensions d;
  d._height = height;
  d._length = length;
  d._width = width;
  return d;
}

osc::Property XOSCWriter_1_0_v4::CreateProperty(std::string name, std::string value) {
  osc::Property p;
  p._name = name;
  p._value = value;
  return p;
}

osc::Vehicle XOSCWriter_1_0_v4::CreateVehicle(osc::Axles axles, osc::BoundingBox boundingBox,
                                              osc::Performance performance, osc::Properties properties,
                                              std::string name, std::string vehicleCategory) {
  osc::Vehicle v;
  v.sub_Axles = axles;
  v.sub_BoundingBox = boundingBox;
  v.sub_Performance = performance;
  v.sub_Properties = properties;
  v._name = name;
  v._vehicleCategory = vehicleCategory;
  return v;
}

osc::Performance XOSCWriter_1_0_v4::CreatePerformance(std::string maxAcceleration, std::string maxDeceleration,
                                                      std::string maxSpeed) {
  osc::Performance p;
  p._maxAcceleration = maxAcceleration;
  p._maxDeceleration = maxDeceleration;
  p._maxSpeed = maxSpeed;
  return p;
}

osc::MiscObject XOSCWriter_1_0_v4::CreateMiscObject(osc::BoundingBox boundingBox, osc::Properties properties,
                                                    std::string mass, std::string miscObjectCategory,
                                                    std::string name) {
  osc::MiscObject m;
  m.sub_BoundingBox = boundingBox;
  m.sub_Properties = properties;
  m._mass = mass;
  m._miscObjectCategory = miscObjectCategory;
  m._name = name;
  return m;
}

osc::Pedestrian XOSCWriter_1_0_v4::CreatePedestrian(osc::BoundingBox boundingBox, osc::Properties properties,
                                                    std::string mass, std::string model, std::string name,
                                                    std::string pedestrianCategory) {
  osc::Pedestrian p;
  p.sub_BoundingBox = boundingBox;
  p.sub_Properties = properties;
  p._mass = mass;
  p._model = model;
  p._name = name;
  p._pedestrianCategory = pedestrianCategory;
  return p;
}

osc::ParameterDeclaration XOSCWriter_1_0_v4::CreateParameterDeclaration(std::string name, std::string parameterType,
                                                                        std::string value) {
  osc::ParameterDeclaration pd;
  pd._name = name;
  pd._parameterType = parameterType;
  pd._value = value;
  return pd;
}

osc::Sun XOSCWriter_1_0_v4::CreateSun(std::string azimuth, std::string elevation, std::string intensity) {
  osc::Sun s;
  s._azimuth = azimuth;
  s._elevation = elevation;
  s._intensity = intensity;
  return s;
}

osc::Event XOSCWriter_1_0_v4::CreateEvent(osc::Trigger sub_StartTrigger, std::string name, std::string priority) {
  osc::Event e;
  e.sub_StartTrigger = sub_StartTrigger;
  e._priority = priority;
  e._name = name;
  return e;
}

osc::Condition XOSCWriter_1_0_v4::CreateCondition(std::string conditionEdge, std::string delay, std::string name) {
  osc::Condition c;
  if (conditionEdge == "") conditionEdge = "none";
  c._conditionEdge = conditionEdge;
  c._delay = delay;
  c._name = name;
  return c;
}
