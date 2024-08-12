/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "parser_json.h"
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>
#include "../../database/entity/scenario.h"
#include "../../engine/config.h"
#include "./batch_scene_param.h"
#include "./control_track.h"
#include "./hadmap_params.h"
#include "./measurement.h"
#include "./query_params.h"
#include "./scenario_set_params.h"
#include "./scene_params.h"
#include "common/engine/constant.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/environments/environment.h"
#include "common/xml_parser/entity/traffic.h"
#include "common/xml_parser/xosc/xosc_converter.h"
#include "common/xml_parser/xosc/xosc_reader_1_0_v4.h"
#include "common/xml_parser/xosc/xosc_writer_1_0_v4.h"
#include "parser.h"
#include "sensors/sensors.h"
#include "simulation.h"

CParserJson::CParserJson() {}

CParserJson::~CParserJson() {}

std::string CParserJson::XmlToJson(const char* pPath, const char* strDir, sTagEntityScenario& scenario) {
  const char* strFileName = scenario.m_strName.c_str();
  if (!strFileName || !strDir || !pPath) {
    SYSTEM_LOGGER_ERROR("file name is null");
    return "";
  }

  boost::filesystem::path p = pPath;
  boost::filesystem::path sceneDirPath = p;
  sceneDirPath.append(strDir);
  boost::filesystem::path simFilePath = sceneDirPath;
  simFilePath.append(strFileName);
  std::string strExt = simFilePath.extension().string();

  std::string strStemName = simFilePath.stem().string();
  std::string strSimPath = "";
  if (strExt.size() == 0 || (strExt != ".sim" && strExt != ".xosc")) {
    std::string strName = strFileName;
    strName.append(".sim");
    simFilePath = sceneDirPath;
    simFilePath.append(strName);
  }

  if (!boost::filesystem::exists(simFilePath)) {
    SYSTEM_LOGGER_ERROR("Not founed, sim file: %s", simFilePath.c_str());
    return "";
  }

  CParser parser;
  sTagSimuTraffic sst;
  // save map version, for cloud
  sst.m_simulation.m_mapFile.m_strMapfile = scenario.m_strMapName;
  sst.m_simulation.m_mapFile.m_strVersion = scenario.m_strMapVersion;
  int nRes = parser.Parse(sst, simFilePath.string().c_str(), sceneDirPath.string().c_str(), false);
  if (!sst.m_traffic.isConfigTraffic) {
    sst.m_traffic.m_ActivePlan = "-1";
  }

  CSimulation& sim = sst.m_simulation;
  CTraffic& traffic = sst.m_traffic;

  if (sim.m_planner.m_route.m_strID.size() > 0) {
    CTraffic::RouteMap::iterator itr = traffic.Routes().find(sim.m_planner.m_route.m_strID);
    if (itr == traffic.Routes().end()) {
      traffic.Routes().insert(std::make_pair(sim.m_planner.m_route.m_strID, sim.m_planner.m_route));
    }
  }

  if (nRes != 0) {
    SYSTEM_LOGGER_ERROR("Parse failed, sim file: %s", simFilePath.c_str());
    return "";
  }

  // return Save(&(sst.m_simulation), &(sst.m_traffic), "test.json");
  return Save(&sst, scenario, "test.json");
}

std::string CParserJson::XoscToSim(const char* pPath, const char* strDir, sTagEntityScenario& scenario) {
  const char* strFileName = scenario.m_strName.c_str();
  if (!strFileName || !strDir || !pPath) {
    return "file name is null";
  }

  boost::filesystem::path p = pPath;
  boost::filesystem::path sceneDirPath = p;
  sceneDirPath.append(strDir);
  boost::filesystem::path simFilePath = sceneDirPath;
  simFilePath.append(strFileName);
  std::string strExt = simFilePath.extension().string();

  std::string strStemName = simFilePath.stem().string();
  std::string strSimPath = "";
  if (strExt.size() == 0 || strExt != ".xosc") {
    return "file suffix is not .xosc";
  }

  if (!boost::filesystem::exists(simFilePath)) {
    return "file not exists";
  }

  CParser parser;
  sTagSimuTraffic sst;
  // save map version, for cloud
  sst.m_simulation.m_mapFile.m_strMapfile = scenario.m_strMapName;
  sst.m_simulation.m_mapFile.m_strVersion = scenario.m_strMapVersion;
  int nRes = parser.Parse(sst, simFilePath.string().c_str(), sceneDirPath.string().c_str(), false);

  CSimulation& sim = sst.m_simulation;
  CTraffic& traffic = sst.m_traffic;

  if (sim.m_planner.m_route.m_strID.size() > 0) {
    CTraffic::RouteMap::iterator itr = traffic.Routes().find(sim.m_planner.m_route.m_strID);
    if (itr == traffic.Routes().end()) {
      traffic.Routes().insert(std::make_pair(sim.m_planner.m_route.m_strID, sim.m_planner.m_route));
    }
  }

  int nRet = SaveToSim(pPath, (strStemName + ".sim").c_str(), sst, strSimPath);
  if (nRet != 0) {
    return "save to file error";
  }
  scenario.m_strName = strStemName + ".sim";
  return "";
}

// std::string CParserJson::Save(CSimulation* pSim, CTraffic* pTraffic
// , CSensors* pSensor, const char* pFileName) {
std::string CParserJson::Save(sTagSimuTraffic* pSceneData, sTagEntityScenario& scenario, const char* pFileName) {
  Json::Value root;

  CProject* pProject = &(pSceneData->m_project);
  CSimulation* pSim = &(pSceneData->m_simulation);
  CTraffic* pTraffic = &(pSceneData->m_traffic);
  CSensors* pSensors = &(pSceneData->m_sensors);
  CEnvironment* pEnvironment = &(pSceneData->m_environment);

  if (scenario.m_nID >= 0) {
    Json::Value infos;
    infos["id"] = Json::Value::UInt64(scenario.m_nID);
    infos["name"] = scenario.m_strName;
    infos["map"] = scenario.m_strMap;
    infos["info"] = scenario.m_strInfo;
    infos["label"] = scenario.m_strLabel;
    root["infos"] = infos;
  }

  if (pSim) {
    Json::Value sim;

    // map
    if (pSim->m_mapFile.m_strLon.size() > 0 && pSim->m_mapFile.m_strLat.size() > 0) {
      Json::Value map;
      map["lon"] = pSim->m_mapFile.m_strLon;
      map["lat"] = pSim->m_mapFile.m_strLat;
      map["alt"] = pSim->m_mapFile.m_strAlt;
      map["level"] = pSim->m_mapFile.m_strUnrealLevelIndex;
      boost::filesystem::path p = pSim->m_mapFile.m_strMapfile;
      map["file"] = p.filename().string();

      sim["map"] = map;
    }

    // planner
    if (!pSim->m_planner.m_route.m_strID.empty() && !pSim->m_planner.m_strStartVelocity.empty() &&
        !pSim->m_planner.m_strTheta.empty() && !pSim->m_planner.m_strVelocityMax.empty()) {
      pSim->m_planner.ConvertToValue();
      Json::Value planner;
      planner["type"] = pSim->m_planner.m_strType;
      planner["name"] = pSim->m_planner.m_strName;
      planner["routeID"] = (Json::Value::Int64)(pSim->m_planner.m_route.m_ID);
      if (pSim->m_planner.m_strStartVelocity.empty())
        planner["start_v"] = "0.0";
      else
        planner["start_v"] = pSim->m_planner.m_strStartVelocity;
      planner["theta"] = pSim->m_planner.m_strTheta;
      planner["Velocity_Max"] = pSim->m_planner.m_strVelocityMax;
      planner["acceleration_max"] = pSim->m_planner.m_accelerationMax;
      planner["deceleration_max"] = pSim->m_planner.m_decelerationMax;
      if (pSim->m_planner.m_controlLongitudinal == "true") {
        planner["control_longitudinal"] = true;
      } else {
        planner["control_longitudinal"] = false;
      }
      if (pSim->m_planner.m_controlLateral == "true") {
        planner["control_lateral"] = true;
      } else {
        planner["control_lateral"] = false;
      }
      if (pSim->m_planner.m_trajectoryEnabled == "true") {
        planner["trajectory_enabled"] = true;
      } else {
        planner["trajectory_enabled"] = false;
      }
      if (!pSim->m_planner.m_strStartAlt.empty()) {
        planner["alt_start"] = pSim->m_planner.m_strStartAlt;
      }
      if (!pSim->m_planner.m_strEndAlt.empty()) {
        planner["alt_end"] = pSim->m_planner.m_strEndAlt;
      }
      if (pSim->m_planner.m_scenceEvents.size() > 0) {
        auto iter = pSim->m_planner.m_scenceEvents.begin();
        Json::Value jsonEvents;
        for (; iter != pSim->m_planner.m_scenceEvents.end(); iter++) {
          Json::Value jEvent = iter->second.saveJson();
          jsonEvents.append(jEvent);
        }
        planner["sceneevents"] = jsonEvents;
      }
      // input path
      Json::Value jInputPath;
      sPath& inputPath = pSim->m_planner.m_inputPath;
      for (size_t i = 0; i < inputPath.m_vPoints.size(); ++i) {
        Json::Value item;
        item["lon"] = inputPath.m_vPoints[i].m_strLon;
        item["lat"] = inputPath.m_vPoints[i].m_strLat;
        item["alt"] = inputPath.m_vPoints[i].m_strAlt;
        jInputPath.append(item);
      }
      planner["inputPath"] = jInputPath;

      // control path
      Json::Value jControlPath;
      sControlPath& controlPath = pSim->m_planner.m_controlPath;
      for (int i = 0; i < controlPath.m_vPoints.size(); ++i) {
        Json::Value item;
        item["lon"] = controlPath.m_vPoints[i].m_strLon;
        item["lat"] = controlPath.m_vPoints[i].m_strLat;
        item["alt"] = controlPath.m_vPoints[i].m_strAlt;
        item["velocity"] = controlPath.m_vPoints[i].m_strVelocity;
        item["gear"] = controlPath.m_vPoints[i].m_strGear;
        if (controlPath.m_vPoints[i].m_accs == "null") {
          item["accs"] = Json::Value();
        } else {
          item["accs"] = controlPath.m_vPoints[i].m_accs;
        }
        if (controlPath.m_vPoints[i].m_heading == "null") {
          item["heading"] = Json::Value();
        } else {
          item["heading"] = controlPath.m_vPoints[i].m_heading;
        }
        if (controlPath.m_vPoints[i].m_frontwheel == "null") {
          item["frontwheel"] = Json::Value();
        } else {
          item["frontwheel"] = controlPath.m_vPoints[i].m_frontwheel;
        }
        jControlPath.append(item);
      }
      planner["pathSampleInterval"] = pSim->m_planner.m_controlPath.m_strSampleInterval;
      planner["controlPath"] = jControlPath;
      sim["planner"] = planner;
    }

    // rosbag
    if (pSim->m_rosbag.Path() && (strlen(pSim->m_rosbag.Path()) > 0)) {
      Json::Value rosbag;
      rosbag["path"] = pSim->m_rosbag.Path();
      sim["rosbag"] = rosbag;
    }

    // l3 state machine
    if (!pSim->m_l3States.IsEmpty()) {
      Json::Value l3statemachine;
      CL3States::StateItems& states = pSim->m_l3States.States();
      for (int i = 0; i < states.size(); ++i) {
        Json::Value item;
        item["type"] = states[i].strStateType;
        item["name"] = states[i].strStateName;
        item["triggerTime"] = states[i].dTriggerTime;
        item["value"] = states[i].nValue;
        l3statemachine.append(item);
      }
      sim["l3statemachine"] = l3statemachine;
    }
    // geofence
    if (pSim->m_strGeoFence.size() > 0) {
      Json::Value geoFence = pSim->m_strGeoFence;
      sim["geofence"] = geoFence;
    }

    // generate info
    // if (pSim->m_generateInfo.m_strGenerateInfo.size() > 0)
    {
      Json::Value generateInfo;
      generateInfo["param"] = pSim->m_generateInfo.m_strGenerateInfo;
      generateInfo["origin"] = pSim->m_generateInfo.m_strOriginFile;
      sim["generateinfo"] = generateInfo;
    }

    root["sim"] = sim;
  }

  if (pTraffic) {
    Json::Value traffic;

    traffic["mode"] = pTraffic->m_strMode;

    // data
    // if (pTraffic->RandomSeed() > 0 || pTraffic->Aggress() > 0)
    {
      Json::Value datas;
      datas["randomseed"] = (Json::Value::Int64)(pTraffic->RandomSeed());
      datas["aggress"] = pTraffic->Aggress();
      traffic["data"] = datas;
    }

    // measurements
    Json::Value measurements;
    CProject::MeasurementMap& mapMeasurements = pProject->Measurements();
    CProject::MeasurementMap::iterator meaitr = mapMeasurements.begin();
    for (; meaitr != mapMeasurements.end(); ++meaitr) {
      meaitr->second.ConvertToValue();

      Json::Value measurement;
      measurement["id"] = meaitr->second.m_strID;
      Json::Value posArr;
      MeasurementNodes::iterator itr = meaitr->second.m_nodes.begin();
      for (; itr != meaitr->second.m_nodes.end(); ++itr) {
        Json::Value n;
        n["lon"] = itr->dLon;
        n["lat"] = itr->dLat;
        n["alt"] = itr->dAlt;
        posArr.append(n);
      }
      measurement["posArr"] = posArr;
      measurements.append(measurement);
    }

    if (mapMeasurements.size() > 0) {
      traffic["measurements"] = measurements;
    }

    // routes
    Json::Value routes;
    CTraffic::RouteMap mapRoutes = pTraffic->Routes();
    CTraffic::RouteMap::iterator itr = mapRoutes.begin();
    for (; itr != mapRoutes.end(); ++itr) {
      Json::Value route;

      route["id"] = itr->second.m_strID;
      route["type"] = itr->second.m_strType;

      // if (_stricmp(itr->second.m_strType.c_str(), "start_end") == 0)
      if (boost::algorithm::iequals(itr->second.m_strType, "start_end")) {
        route["start"] = itr->second.m_strStart;
        route["end"] = itr->second.m_strEnd;

        if (itr->second.m_strMid.size() > 0) {
          route["mid"] = itr->second.m_strMid;
        }
        if (itr->second.m_strMids.size() > 0) {
          route["mids"] = itr->second.m_strMids;
        }
        if (itr->second.m_strInfo.size() > 0) {
          route["info"] = itr->second.m_strInfo;
        }

      } else {
        route["roadID"] = itr->second.m_strRoadID;
        route["sectionID"] = itr->second.m_strSectionID;
      }
      itr->second.SegmentString();
      if (itr->second.m_strControlPath.size() > 0) {
        itr->second.SegmentString();
        Json::Value controlpath;
        for (int i = 0; i < itr->second.m_ControlPathVer.size(); i++) {
          Json::Value n;
          n["lon"] = itr->second.m_ControlPathVer[i].strLon;
          n["lat"] = itr->second.m_ControlPathVer[i].strLat;
          n["alt"] = itr->second.m_ControlPathVer[i].strAlt;
          n["speed_m_s"] = itr->second.m_ControlPathVer[i].strSpeed_m_s;
          n["gear"] = itr->second.m_ControlPathVer[i].strGear;

          controlpath.append(n);
        }

        route["controlPath"] = controlpath;
      }
      routes.append(route);
    }

    traffic["routes"] = routes;

    // accs
    Json::Value accs;
    CTraffic::AccelerationMap mapAccs = pTraffic->Accs();
    CTraffic::AccelerationMap::iterator maitr = mapAccs.begin();
    for (; maitr != mapAccs.end(); ++maitr) {
      // maitr->second.ConvertToValue();
      maitr->second.ClearNodes();
      maitr->second.ConvertToTimeValue();
      maitr->second.ConvertToEventValue();

      Json::Value acc;
      acc["id"] = maitr->second.m_strID;
      if (maitr->second.m_nodes.size() == 0) {
        Json::Value timenode;
        // timenode.append("");

        acc["profile"].append(timenode);
      } else {
        TimeNodes::iterator itor = maitr->second.m_nodes.begin();
        for (; itor != maitr->second.m_nodes.end(); ++itor) {
          Json::Value timenode;
          std::string strTrigType = TrigTypeValue2Str(itor->nTriggerType);
          timenode.append(strTrigType);
          timenode.append(itor->dTriggerValue);
          std::string strDistanceMode = TrigDistanceModeValue2Str(itor->nDistanceMode);
          timenode.append(strDistanceMode);
          timenode.append(itor->dAcc);
          timenode.append(itor->nCount);
          std::string strStopType = AccEndConditionTypeValue2FrontStr(itor->nStopType);
          timenode.append(strStopType);
          timenode.append(itor->dStopValue);
          acc["profile"].append(timenode);
        }
      }

      accs.append(acc);
    }

    traffic["accs"] = accs;

    // merges
    Json::Value merges;
    CTraffic::MergeMap mapMerges = pTraffic->Merges();
    CTraffic::MergeMap::iterator mmitr = mapMerges.begin();
    for (; mmitr != mapMerges.end(); ++mmitr) {
      // mmitr->second.ConvertToValue();
      mmitr->second.ClearNodes();
      mmitr->second.ConvertToTimeValue();
      mmitr->second.ConvertToEventValue();

      Json::Value merge;

      merge["id"] = mmitr->second.m_strID;
      if (mmitr->second.m_merges.size() == 0) {
        Json::Value timenode;

        merge["profile"].append(timenode);
      } else {
        MergeUnits::iterator itor = mmitr->second.m_merges.begin();
        for (; itor != mmitr->second.m_merges.end(); ++itor) {
          Json::Value timenode;
          std::string strType = TrigTypeValue2Str(itor->nTrigType);
          timenode.append(strType);
          timenode.append(itor->dTrigValue);
          std::string strDistanceMode = TrigDistanceModeValue2Str(itor->nDistanceMode);
          timenode.append(strDistanceMode);
          timenode.append(itor->nDir);
          timenode.append(itor->dDuration);
          timenode.append(itor->dOffset);
          timenode.append(itor->nCount);
          merge["profile"].append(timenode);
        }
      }

      merges.append(merge);
    }

    traffic["merges"] = merges;

    // velocities
    Json::Value velocities;
    CTraffic::VelocityMap mapVelocities = pTraffic->Velocities();
    CTraffic::VelocityMap::iterator mvelitr = mapVelocities.begin();
    for (; mvelitr != mapVelocities.end(); ++mvelitr) {
      // maitr->second.ConvertToValue();
      mvelitr->second.ClearNodes();
      mvelitr->second.ConvertToTimeValue();
      mvelitr->second.ConvertToEventValue();

      Json::Value vel;
      vel["id"] = mvelitr->second.m_strID;
      if (mvelitr->second.m_nodes.size() > 0) {
        VelocityTimeNodes::iterator itor = mvelitr->second.m_nodes.begin();
        for (; itor != mvelitr->second.m_nodes.end(); ++itor) {
          Json::Value timenode;
          std::string strTrigType = TrigTypeValue2Str(itor->nTriggerType);
          timenode.append(strTrigType);
          timenode.append(itor->dTriggerValue);
          std::string strDistanceMode = TrigDistanceModeValue2Str(itor->nDistanceMode);
          timenode.append(strDistanceMode);
          timenode.append(itor->dVelocity);
          timenode.append(itor->nCount);
          vel["profile"].append(timenode);
        }
        velocities.append(vel);
      }
    }

    if (mapVelocities.size() > 0) {
      traffic["velocities"] = velocities;
    }

    // vehicles
    Json::Value vehicles;
    CTraffic::VehicleMap mapVehicles = pTraffic->Vehicles();
    CTraffic::VehicleMap::iterator mvitr = mapVehicles.begin();
    for (; mvitr != mapVehicles.end(); ++mvitr) {
      Json::Value vehicle;

      vehicle["id"] = mvitr->second.m_strID;
      vehicle["routeID"] = mvitr->second.m_strRouteID;
      vehicle["laneID"] = mvitr->second.m_strLaneID;
      vehicle["start_s"] = mvitr->second.m_strStartShift;
      vehicle["start_t"] = mvitr->second.m_strStartTime;
      vehicle["start_v"] = mvitr->second.m_strStartVelocity;
      vehicle["l_offset"] = mvitr->second.m_strOffset;
      vehicle["length"] = mvitr->second.m_strLength;
      vehicle["width"] = mvitr->second.m_strWidth;
      vehicle["height"] = mvitr->second.m_strHeight;
      vehicle["accID"] = mvitr->second.m_strAccID;
      vehicle["mergeID"] = mvitr->second.m_strMergeID;
      vehicle["vehicleType"] = mvitr->second.m_strType;
      vehicle["max_v"] = mvitr->second.m_strMaxVelocity;
      vehicle["behavior"] = mvitr->second.m_strBehavior;
      vehicle["aggress"] = mvitr->second.m_strAggress;
      vehicle["follow"] = mvitr->second.m_strFollow;
      vehicle["eventId"] = mvitr->second.m_strEventId;
      vehicle["angle"] = mvitr->second.m_strAngle;
      vehicle["sensorGroup"] = mvitr->second.m_strSensorGroup;
      vehicle["obuStatus"] = mvitr->second.m_strObuStauts;
      vehicle["start_angle"] = mvitr->second.m_strStartAngle;

      vehicles.append(vehicle);
    }

    traffic["vehicles"] = vehicles;

    // obstacles
    Json::Value obstacles;
    CTraffic::ObstacleMap mapObtstacles = pTraffic->Obstacles();
    CTraffic::ObstacleMap::iterator moitr = mapObtstacles.begin();
    for (; moitr != mapObtstacles.end(); ++moitr) {
      Json::Value obstacle;

      obstacle["id"] = moitr->second.m_strID;
      obstacle["routeID"] = moitr->second.m_strRouteID;
      obstacle["laneID"] = moitr->second.m_strLaneID;
      obstacle["start_s"] = moitr->second.m_strStartShift;
      obstacle["l_offset"] = moitr->second.m_strOffset;
      obstacle["length"] = moitr->second.m_strLength;
      obstacle["width"] = moitr->second.m_strWidth;
      obstacle["height"] = moitr->second.m_strHeight;
      obstacle["type"] = moitr->second.m_strType;
      obstacle["direction"] = moitr->second.m_strDirection;
      obstacle["start_angle"] = moitr->second.m_strStartAngle;

      obstacles.append(obstacle);
    }

    traffic["obstacles"] = obstacles;

    /*
    // pedestrians
    Json::Value pedestrians;
    Pedestrians2Json(pedestrians, pTraffic);
    traffic["pedestrians"] = pedestrians;
    */

    // pedestrians
    Json::Value pedestrians;
    PedestriansV22Json(pedestrians, pTraffic);
    traffic["pedestriansV2"] = pedestrians;
    // activePlan
    // for new version signalight
    traffic["activePlan"] = pTraffic->m_ActivePlan;
    // signlights
    Json::Value signlights;
    CTraffic::SignLightMap mapSignights = pTraffic->Signlights();
    CTraffic::SignLightMap::iterator msitr = mapSignights.begin();
    for (; msitr != mapSignights.end(); ++msitr) {
      Json::Value signlight;

      signlight["id"] = msitr->second.m_strID;
      signlight["routeID"] = msitr->second.m_strRouteID;
      signlight["start_s"] = msitr->second.m_strStartShift;
      signlight["start_t"] = msitr->second.m_strStartTime;
      signlight["l_offset"] = msitr->second.m_strOffset;
      signlight["time_green"] = msitr->second.m_strTimeGreen;
      signlight["time_yellow"] = msitr->second.m_strTimeYellow;
      signlight["time_red"] = msitr->second.m_strTimeRed;
      signlight["direction"] = msitr->second.m_strDirection;
      signlight["phase"] = msitr->second.m_strPhase;
      signlight["lane"] = msitr->second.m_strLane;
      bool bEnabled = boost::algorithm::iequals("Activated", msitr->second.m_strStatus);
      signlight["enabled"] = bEnabled;
      // for new version signalight
      signlight["eventId"] = msitr->second.m_strEventId;
      signlight["plan"] = msitr->second.m_strPlan;
      signlight["junction"] = msitr->second.m_strJunction;
      signlight["phaseNumber"] = msitr->second.m_strPhasenumber;
      std::string signalHead = "";
      for (size_t i = 0; i < msitr->second.m_strSignalheadVec.size(); i++) {
        signalHead.append(msitr->second.m_strSignalheadVec.at(i));
        if (i != msitr->second.m_strSignalheadVec.size() - 1) {
          signalHead.append(",");
        }
      }
      signlight["signalHead"] = signalHead;

      signlights.append(signlight);
    }
    traffic["signlights"] = signlights;

    traffic["trafficFlow"] = pTraffic->TrafficFlow().JsonMeta();

    // scene events
    Json::Value sceneevents;
    CTraffic::SceneEventMap mapSceneEvents = pTraffic->SceneEvents();
    CTraffic::SceneEventMap::iterator mseitr = mapSceneEvents.begin();
    for (; mseitr != mapSceneEvents.end(); ++mseitr) {
#ifdef SCENE_EVENTS_UPGRAD
      Json::Value jEvent = mseitr->second.saveJson();
#else
      Json::Value jEvent = mseitr->second.saveJson("1.0.0.0");
#endif
      sceneevents.append(jEvent);
    }
    traffic["sceneevents"] = sceneevents;
    root["traffic"] = traffic;
  }

  if (pSensors) {
    Json::Value sensors;
    bool bIncludeSensorData = XmlToJsonSensorsInfo((*pSensors), sensors);

    if (bIncludeSensorData) {
      root["sensor"] = sensors;
    }
  }

  if (pEnvironment) {
    Json::Value environment;
    bool bIncludeEnvironmentData = XmlToJsonEnvironmentInfo((*pEnvironment), environment);

    if (bIncludeEnvironmentData) {
      root["environment"] = environment;
    }
  }

  root["grading"] = pSim->m_strGrading;

  /*
  bool bGradingExist = pSim->m_strGrading.size() > 0 ? true: false;
  if (bGradingExist)
  {
          root["grading"] = "local";
  } else {
          root["grading"] = "global";
  }
  */
  // std::string strStyledResult = root.toStyledString();
  // Json::FastWriter writer;
  // Json::StreamWriterBuilder swbuilder;
  // std::string strResult = Json::writeString(swbuilder, root);

  return ToJsonString(root);
  /*
  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream ss;
  writer->write(root, &ss);
  std::string strUnstyledResult = ss.str();

  if (0 && pFileName)
  {
          FILE* f = NULL;
#ifdef _WINDOWS
          fopen_s(&f, pFileName, "w");
#else
          f = fopen(pFileName, "w");
#endif
          if (f)
          {
                  fwrite(strUnstyledResult.c_str(), strUnstyledResult.size(),
1, f);
                  fclose(f);
          }
  }

  return strUnstyledResult;
  */
}

std::string CParserJson::ToJsonString(const Json::Value& jRoot, bool bUseUnstyled) {
  if (bUseUnstyled) {
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::stringstream ss;
    writer->write(jRoot, &ss);
    return ss.str();
  } else {
    return jRoot.toStyledString();
  }
}

int CParserJson::InsertTrafficLights(const std::string& strJson, sTagSimuTraffic& scene) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  size_t nLen = strlen(strJson.c_str());
  if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse json failed: ", strJson.c_str());
    return -1;
  }
  if (root["signlights"].empty() || root["routes"].empty()) {
    return -1;
  }
  scene.m_traffic.m_mapSignlights.clear();
  for (auto it : root["signlights"]) {
    CSignLight _light;
    _light.Reset();
    _light.m_strID = it.get("id", "0").asString();
    _light.m_strRouteID = it.get("routeID", "0").asString();
    _light.m_strStartShift = it.get("start_s", "0").asString();
    _light.m_strStartTime = it.get("start_t", "0").asString();
    _light.m_strOffset = it.get("l_offset", "0").asString();
    _light.m_strDirection = it.get("direction", "0").asString();
    _light.m_strTimeGreen = it.get("time_green", "0").asString();
    _light.m_strTimeYellow = it.get("time_yellow", "0").asString();
    _light.m_strTimeRed = it.get("time_red", "0").asString();
    _light.m_strPhase = it.get("phase", "A").asString();
    _light.m_strLane = it.get("lane", "ALL").asString();
    _light.m_strStatus = it.get("enabled", "Activated").asString();
    _light.m_strEventId = it.get("eventId", "").asString();
    _light.m_strPlan = it.get("plan", "0").asString();
    _light.m_strJunction = it.get("junction", "").asString();
    _light.m_strPhasenumber = it.get("phaseNumber", "").asString();
    std::string m_strSignal = it.get("signalHead", "").asString();
    std::vector<std::string> strVec;
    boost::algorithm::split(strVec, m_strSignal, boost::algorithm::is_any_of(","));
    for (auto it : strVec) {
      _light.m_strSignalheadVec.push_back(it);
    }
    scene.m_traffic.m_mapSignlights[_light.m_strID] = _light;
  }

  for (auto it : root["routes"]) {
    CRoute route;
    route.m_strID = it.get("id", "0").asString();
    route.m_strType = it.get("type", "start_end").asString();
    route.m_strStart = it.get("start", "0.0,0.0").asString();
    scene.m_traffic.m_mapRoutes[route.m_strID] = route;
  }

  scene.m_traffic.m_mapJunction2Road.clear();
  for (auto it : root["junctions"]) {
    std::string junctionId = it["junctionid"].asString();
    std::map<std::string, std::string> traffic2RoadId;
    for (auto itLight : it["tafficlights"]) {
      traffic2RoadId[itLight["id"].asString()] = itLight["roadid"].asString();
    }
    scene.m_traffic.m_mapJunction2Road["junctionId"] = traffic2RoadId;
  }

  scene.m_traffic.m_ActivePlan = "0";
  return 0;
}

int CParserJson::Pedestrians2Json(Json::Value& pedestrians, CTraffic* pTraffic) {
  CTraffic::PedestrianMap mapPedestrian = pTraffic->Pedestrians();
  CTraffic::PedestrianMap::iterator mpitr = mapPedestrian.begin();
  for (; mpitr != mapPedestrian.end(); ++mpitr) {
    mpitr->second.ConvertToValue();

    Json::Value pedestrian;

    pedestrian["id"] = mpitr->second.m_strID;
    pedestrian["routeID"] = mpitr->second.m_strRouteID;
    pedestrian["laneID"] = mpitr->second.m_strLaneID;
    pedestrian["start_s"] = mpitr->second.m_strStartShift;
    pedestrian["start_t"] = mpitr->second.m_strStartTime;
    pedestrian["l_offset"] = mpitr->second.m_strOffset;
    pedestrian["subType"] = mpitr->second.m_strType;
    if (mpitr->second.m_strEndTime.size() > 0) {
      pedestrian["end_t"] = mpitr->second.m_strEndTime;
    }

    for (int i = 0; i < mpitr->second.m_directions.size(); ++i) {
      std::string strDirection = "direction";
      strDirection.append(std::to_string(i + 1));
      pedestrian[strDirection] = mpitr->second.m_directions[i].m_strDir;

      std::string strDirectionTime = strDirection + "_t";
      pedestrian[strDirectionTime] = mpitr->second.m_directions[i].m_strDirDuration;

      std::string strDirectionVelocity = strDirection + "_v";
      pedestrian[strDirectionVelocity] = mpitr->second.m_directions[i].m_strDirVelocity;
    }

    if (mpitr->second.m_events.m_strProfiles.size() > 0) {
      std::string strType = TrigTypeValue2Str(mpitr->second.m_events.m_TriggerType);
      pedestrian["conditionType"] = strType;

      if (mpitr->second.m_events.m_nodes.size() > 0) {
        PedestrianEventNodes::iterator itor = mpitr->second.m_events.m_nodes.begin();
        for (; itor != mpitr->second.m_events.m_nodes.end(); ++itor) {
          Json::Value eventnode;
          eventnode["value"] = itor->dTriggerValue;
          eventnode["direction"] = itor->dDirection;
          eventnode["velocity"] = itor->dVelocity;
          eventnode["times"] = itor->nCount;
          pedestrian["conditions"].append(eventnode);
        }
      }
    }

    pedestrians.append(pedestrian);
  }
  return 0;
}

int CParserJson::PedestriansV22Json(Json::Value& pedestrians, CTraffic* pTraffic) {
  CTraffic::PedestrianV2Map mapPedestrian = pTraffic->PedestriansV2();
  CTraffic::PedestrianV2Map::iterator mpitr = mapPedestrian.begin();

  CTraffic::PedestrianVelocityMap mapVelocities = pTraffic->PedestrianVelocities();
  CTraffic::PedestrianVelocityMap::iterator vItr;
  for (; mpitr != mapPedestrian.end(); ++mpitr) {
    mpitr->second.ConvertToValue();

    Json::Value pedestrian;

    pedestrian["id"] = mpitr->second.m_strID;
    pedestrian["routeID"] = mpitr->second.m_strRouteID;
    pedestrian["laneID"] = mpitr->second.m_strLaneID;
    pedestrian["start_s"] = mpitr->second.m_strStartShift;
    pedestrian["start_t"] = mpitr->second.m_strStartTime;
    pedestrian["l_offset"] = mpitr->second.m_strOffset;
    pedestrian["start_v"] = mpitr->second.m_strStartVelocity;
    pedestrian["max_v"] = mpitr->second.m_strMaxVelocity;
    pedestrian["behavior"] = mpitr->second.m_strBehavior;
    pedestrian["subType"] = mpitr->second.m_strType;
    pedestrian["angle"] = mpitr->second.m_strAngle;
    pedestrian["start_angle"] = mpitr->second.m_strStartAngle;
    if (mpitr->second.m_strEventId.size() > 0) {
      pedestrian["eventId"] = mpitr->second.m_strEventId;
    }
    if (mpitr->second.m_strEndTime.size() > 0) {
      pedestrian["end_t"] = mpitr->second.m_strEndTime;
    }

    for (int i = 0; i < mpitr->second.m_directions.size(); ++i) {
      std::string strDirection = "direction";
      strDirection.append(std::to_string(i + 1));
      pedestrian[strDirection] = mpitr->second.m_directions[i].m_strDir;

      std::string strDirectionTime = strDirection + "_t";
      pedestrian[strDirectionTime] = mpitr->second.m_directions[i].m_strDirDuration;

      std::string strDirectionVelocity = strDirection + "_v";
      pedestrian[strDirectionVelocity] = mpitr->second.m_directions[i].m_strDirVelocity;
    }

    if (mpitr->second.m_events.m_strProfiles.size() > 0) {
      std::string strType = TrigTypeValue2Str(mpitr->second.m_events.m_TriggerType);
      pedestrian["conditionType"] = strType;

      if (mpitr->second.m_events.m_nodes.size() > 0) {
        PedestrianEventNodes::iterator itor = mpitr->second.m_events.m_nodes.begin();
        for (; itor != mpitr->second.m_events.m_nodes.end(); ++itor) {
          Json::Value eventnode;
          eventnode["value"] = itor->dTriggerValue;
          eventnode["direction"] = itor->dDirection;
          eventnode["velocity"] = itor->dVelocity;
          eventnode["times"] = itor->nCount;
          pedestrian["conditions"].append(eventnode);
        }
      }
    }

    vItr = mapVelocities.find(mpitr->second.m_strID);
    if (vItr != mapVelocities.end()) {
      vItr->second.ClearNodes();
      vItr->second.ConvertToTimeValue();
      vItr->second.ConvertToEventValue();
      if (vItr->second.m_nodes.size() > 0) {
        PedestrianVelocityNodes::iterator itor = vItr->second.m_nodes.begin();
        for (; itor != vItr->second.m_nodes.end(); ++itor) {
          Json::Value condition;
          std::string strTrigType = TrigTypeValue2Str(itor->nTriggerType);
          condition["type"] = strTrigType;
          condition["value"] = itor->dTriggerValue;
          std::string strDistanceMode = TrigDistanceModeValue2Str(itor->nDistanceMode);
          condition["mode"] = strDistanceMode;
          pedestrian["conditionsV2"].append(condition);
          pedestrian["directions"].append(itor->dDirection);
          pedestrian["velocities"].append(itor->dVelocity);
          pedestrian["counts"].append(itor->nCount);
        }
      }
    }
    pedestrians.append(pedestrian);
  }
  return 0;
}

int CParserJson::ParsePedestrians(Json::Value& jPedestrians, CTraffic& traffic) {
  if (!jPedestrians.isNull()) {
    CTraffic::PedestrianMap& pedestrians = traffic.Pedestrians();

    Json::ValueIterator pItr = jPedestrians.begin();
    for (; pItr != jPedestrians.end(); ++pItr) {
      CPedestrian pedestrian;
      pedestrian.m_strID = (*pItr)["id"].asString();
      pedestrian.m_strRouteID = (*pItr)["routeID"].asString();
      pedestrian.m_strLaneID = (*pItr)["laneID"].asString();
      pedestrian.m_strStartShift = (*pItr)["start_s"].asString();
      pedestrian.m_strStartTime = (*pItr)["start_t"].asString();
      pedestrian.m_strOffset = (*pItr)["l_offset"].asString();

      if (!(*pItr)["subType"].isNull()) {
        pedestrian.m_strType = (*pItr)["subType"].asString();
      } else {
        pedestrian.m_strType = "human";
      }

      if (!(*pItr)["end_t"].isNull()) {
        pedestrian.m_strEndTime = (*pItr)["end_t"].asString();
      }

      tagDirection dir1;
      dir1.m_strDir = (*pItr)["direction1"].asString();
      dir1.m_strDirDuration = (*pItr)["direction1_t"].asString();
      dir1.m_strDirVelocity = (*pItr)["direction1_v"].asString();
      dir1.ConvertToValue();

      pedestrian.m_directions.push_back(dir1);

      if (!(*pItr)["direction2"].isNull()) {
        tagDirection dir2;
        dir2.m_strDir = (*pItr)["direction2"].asString();
        dir2.m_strDirDuration = (*pItr)["direction2_t"].asString();
        dir2.m_strDirVelocity = (*pItr)["direction2_v"].asString();
        dir2.ConvertToValue();

        pedestrian.m_directions.push_back(dir2);
      }

      if (!(*pItr)["conditions"].isNull()) {
        pedestrian.m_events.m_strID = pedestrian.m_strID;
        pedestrian.m_events.ConvertToValue();

        if (!(*pItr)["conditionType"].isNull()) {
          std::string strTrigType = (*pItr)["conditionType"].asString();
          pedestrian.m_events.m_TriggerType = TrigTypeStr2Value(strTrigType);
        }

        Json::ValueIterator penItr = (*pItr)["conditions"].begin();
        for (; penItr != (*pItr)["conditions"].end(); ++penItr) {
          PedestrianEventNode pen;
          pen.dTriggerValue = (*penItr)["value"].asDouble();
          pen.dDirection = (*penItr)["direction"].asDouble();
          pen.dVelocity = (*penItr)["velocity"].asDouble();
          pen.nCount = (*penItr)["times"].asInt();

          pedestrian.m_events.m_nodes.push_back(pen);
        }

        pedestrian.m_events.ConvertToStr();
      }

      pedestrian.ConvertToValue();

      CTraffic::PedestrianMap::iterator itr = pedestrians.find(pedestrian.m_strID);
      if (itr == pedestrians.end()) {
        pedestrians.insert(std::make_pair(pedestrian.m_strID, pedestrian));
      } else {
        SYSTEM_LOGGER_WARN("pedestrian id repeated!");
      }
    }
  }
  return 0;
}

int CParserJson::ParsePedestriansV2(Json::Value& jPedestrians, CTraffic& traffic) {
  if (!jPedestrians.isNull()) {
    CTraffic::PedestrianV2Map& pedestrians = traffic.PedestriansV2();
    CTraffic::PedestrianVelocityMap& velocities = traffic.PedestrianVelocities();

    Json::ValueIterator pItr = jPedestrians.begin();
    for (; pItr != jPedestrians.end(); ++pItr) {
      CPedestrianV2 pedestrian;
      pedestrian.m_strID = (*pItr)["id"].asString();
      pedestrian.m_strRouteID = (*pItr)["routeID"].asString();
      pedestrian.m_strLaneID = (*pItr)["laneID"].asString();
      pedestrian.m_strStartShift = (*pItr)["start_s"].asString();
      pedestrian.m_strStartTime = (*pItr)["start_t"].asString();
      pedestrian.m_strOffset = (*pItr)["l_offset"].asString();
      pedestrian.m_strStartVelocity = (*pItr)["start_v"].asString();
      pedestrian.m_strMaxVelocity = (*pItr)["max_v"].asString();
      pedestrian.m_strBehavior = (*pItr)["behavior"].asString();
      pedestrian.m_strEventId = (*pItr).get("eventId", "").asString();
      pedestrian.m_strAngle = (*pItr).get("angle", "0").asString();
      pedestrian.m_strStartAngle = (*pItr).get("start_angle", "").asString();
      if (!(*pItr)["subType"].isNull()) {
        pedestrian.m_strType = (*pItr)["subType"].asString();
      } else {
        pedestrian.m_strType = "human";
      }

      if (!(*pItr)["conditions"].isNull()) {
        /*
        pedestrian.m_events.m_strID = pedestrian.m_strID;
        pedestrian.m_events.ConvertToValue();
        */

        CPedestrianVelocity vel;
        vel.m_strID = pedestrian.m_strID;
        vel.m_ID = atoi(vel.m_strID.c_str());

        Json::Value jConditions = (*pItr)["conditions"];
        Json::Value jVelocities = (*pItr)["velocities"];
        Json::Value jDirections = (*pItr)["directions"];
        Json::Value jCounts = (*pItr)["counts"];

        Json::ValueIterator peConItr = jConditions.begin();
        Json::ValueIterator peVItr = jVelocities.begin();
        Json::ValueIterator peDItr = jDirections.begin();
        Json::ValueIterator peCountItr = jCounts.begin();

        for (; peConItr != jConditions.end() && peVItr != jVelocities.end() && peCountItr != jCounts.end() &&
               peDItr != jDirections.end();
             ++peConItr, ++peVItr, ++peCountItr, ++peDItr) {
          PedestrianVelocityNode tn;
          std::string strTrigType = (*peConItr)["type"].asString();
          tn.nTriggerType = TrigTypeStr2Value(strTrigType);
          tn.dTriggerValue = (*peConItr)["value"].asFloat();
          std::string strTrigDistanceMode = (*peConItr)["mode"].asString();
          tn.nDistanceMode = TrigDistanceModeStr2Value(strTrigDistanceMode);
          tn.dDirection = (*peDItr).asDouble();
          tn.dVelocity = (*peVItr).asDouble();
          tn.nCount = (*peCountItr).asInt();
          vel.m_nodes.push_back(tn);
        }

        vel.ConvertToTimeStr();
        vel.ConvertToEventStr();

        CTraffic::PedestrianVelocityMap::iterator itr = velocities.find(vel.m_strID);
        if (itr == velocities.end()) {
          velocities.insert(std::make_pair(vel.m_strID, vel));
        } else {
          SYSTEM_LOGGER_WARN("pedestrian velocity id repeated!");
        }
      }

      pedestrian.ConvertToValue();
      pedestrian.ConvertToStr();

      CTraffic::PedestrianV2Map::iterator itr = pedestrians.find(pedestrian.m_strID);
      if (itr == pedestrians.end()) {
        pedestrians.insert(std::make_pair(pedestrian.m_strID, pedestrian));
      } else {
        SYSTEM_LOGGER_WARN("pedestrian id repeated!");
      }
    }
  }
  return 0;
}

int CParserJson::JsonToXml(const char* strJson, sTagSimuTraffic& scene) {
  if (!strJson) {
    SYSTEM_LOGGER_ERROR("json string is null");
    return -1;
  }

  Json::Value root;
  if (misc::ParseJson(strJson, &root, "JsonToXml") != 0) {
    return -1;
  }

  if (root.isMember("pedestrian")) {
    sTagEntityScenario scenario;
    scenario.m_strName = "PED_tpl.sim";
    scenario.m_strMapName = "PED_tpl_map.xodr";
    std::string ped_tpl = CParserJson{}.XmlToJson(CEngineConfig::Instance().ResourceDir(), "scene", scenario);
    Json::Value ped_json_root = StringToJson(ped_tpl);
    ped_json_root["traffic"]["trafficFlow"] = root["pedestrian"];
    root = ped_json_root;
    scene.m_simulation.m_strNavMeshFileName = "../hadmap/navMesh.bin";
    scene.m_simulation.m_mapFile.m_strMapfile = scenario.m_strMapName;
  }

  // traffic
  Json::Value& jInfos = root["infos"];
  if (!jInfos.isNull()) {
    scene.m_strType = jInfos["type"].asString();
  }

  // traffic
  Json::Value& jTraffic = root["traffic"];
  CTraffic& traffic = scene.m_traffic;

  if (!jTraffic.isNull()) {
    traffic.m_strMode = jTraffic.get("mode", "traffic").asString();
    // data
    Json::Value& jDatas = jTraffic["data"];
    if (!jDatas.isNull()) {
      if (!jDatas["randomseed"].isNull()) {
        // 前端 暂无界面支持设置该数据，所以在生成文件的时候直接写个默认值55
        int64_t randomSeed = jDatas["randomseed"].asInt();
        traffic.SetRandomSeed(randomSeed);
      }

      if (!jDatas["aggress"].isNull()) {
        double dAggress = jDatas["aggress"].asFloat();
        traffic.SetAggress(dAggress);
      }
    }

    // measurements
    Json::Value& jMeasurements = jTraffic["measurements"];
    if (!jMeasurements.isNull()) {
      CProject& project = scene.m_project;
      CProject::MeasurementMap& measurements = project.Measurements();

      Json::ValueIterator mItr = jMeasurements.begin();
      for (; mItr != jMeasurements.end(); ++mItr) {
        CMeasurement measurement;
        measurement.m_strID = (*mItr)["id"].asString();
        measurement.m_ID = atoi(measurement.m_strID.c_str());

        if (!(*mItr)["posArr"].isNull()) {
          Json::ValueIterator pItr = (*mItr)["posArr"].begin();
          for (; pItr != (*mItr)["posArr"].end(); ++pItr) {
            sTagMeasurementPoint mp;
            std::string strLon = (*pItr)["lon"].asString();
            std::string strLat = (*pItr)["lat"].asString();
            std::string strAlt = (*pItr)["alt"].asString();
            mp.dLon = atof(strLon.c_str());
            mp.dLat = atof(strLat.c_str());
            mp.dAlt = atof(strAlt.c_str());
            measurement.m_nodes.push_back(mp);
          }
        }

        measurement.ConvertToStr();

        CProject::MeasurementMap::iterator itr = measurements.find(measurement.m_strID);
        if (itr == measurements.end()) {
          measurements.insert(std::make_pair(measurement.m_strID, measurement));
        } else {
          SYSTEM_LOGGER_WARN("measurement id repeated!");
        }
      }
    }

    // routes
    Json::Value& jRoutes = jTraffic["routes"];
    if (!jRoutes.isNull()) {
      CTraffic::RouteMap& routes = traffic.Routes();

      Json::ValueIterator rItr = jRoutes.begin();
      for (; rItr != jRoutes.end(); ++rItr) {
        CRoute route;
        route.m_strID = (*rItr)["id"].asString();
        route.m_strType = (*rItr)["type"].asString();
        if (!(*rItr)["info"].isNull()) {
          route.m_strInfo = (*rItr)["info"].asString();
        }

        // if (_stricmp(route.m_strType.c_str(), "start_end") == 0)
        if (boost::algorithm::iequals(route.m_strType, "start_end")) {
          route.m_strStart = (*rItr)["start"].asString();
          if (!(*rItr)["end"].isNull()) {
            route.m_strEnd = (*rItr)["end"].asString();
          }

          if (!(*rItr)["mid"].isNull()) {
            route.m_strMid = (*rItr)["mid"].asString();
          }

          if (!(*rItr)["mids"].isNull()) {
            route.m_strMids = (*rItr)["mids"].asString();
          }

          if (!(*rItr)["controlPath"].isNull()) {
            Json::Value& jControlPaths = (*rItr)["controlPath"];
            Json::ValueIterator cpItr = jControlPaths.begin();
            for (; cpItr != jControlPaths.end(); ++cpItr) {
              sRouteControlPath temp;
              temp.strLon = (*cpItr)["lon"].asString();
              temp.strLat = (*cpItr)["lat"].asString();
              temp.strAlt = (*cpItr)["alt"].asString();
              temp.strSpeed_m_s = (*cpItr)["speed_m_s"].asString();
              temp.strGear = (*cpItr)["gear"].asString();
              route.m_ControlPathVer.push_back(temp);
            }
            route.MergeString();
          }

          // route.RemoveStrAlt();

        } else {
          route.m_strRoadID = (*rItr)["roadID"].asString();
          route.m_strSectionID = (*rItr)["sectionID"].asString();
        }

        route.FromStr();
        route.ConvertToValue();

        CTraffic::RouteMap::iterator itr = routes.find(route.m_strID);
        if (itr == routes.end()) {
          routes.insert(std::make_pair(route.m_strID, route));
        } else {
          SYSTEM_LOGGER_WARN("routs id repeated!");
        }
      }
    }

    // accs
    Json::Value& jAccs = jTraffic["accs"];
    if (!jAccs.isNull()) {
      CTraffic::AccelerationMap& accs = traffic.Accs();

      Json::ValueIterator aItr = jAccs.begin();
      for (; aItr != jAccs.end(); ++aItr) {
        CAcceleration acc;
        if ((*aItr)["id"].isNull()) {
          continue;
        }
        acc.m_strID = (*aItr)["id"].asString();
        acc.m_ID = atoi(acc.m_strID.c_str());
        if (!(*aItr)["profile"].isNull()) {
          Json::ValueIterator tnItr = (*aItr)["profile"].begin();
          for (; tnItr != (*aItr)["profile"].end(); ++tnItr) {
            TimeNode tn;
            if (tnItr->size() == 2) {
              tn.nTriggerType = TCT_TIME_ABSOLUTE;
              tn.dTriggerValue = (*tnItr)[0].asFloat();
              // timeAbsolute模式下 distance mode值无效
              tn.nDistanceMode = TDM_NONE;
              tn.dAcc = (*tnItr)[1].asDouble();
              tn.nCount = -1;
              tn.nStopType = AECT_NONE;
              tn.dStopValue = 0;

            } else if (tnItr->size() == 3) {
              std::string strTrigType = (*tnItr)[0].asString();
              tn.nTriggerType = TrigTypeStr2Value(strTrigType);
              tn.nDistanceMode = TDM_LANE_PROJECTION;
              tn.dTriggerValue = (*tnItr)[1].asFloat();
              tn.dAcc = (*tnItr)[2].asDouble();
              tn.nCount = -1;
              tn.nStopType = AECT_NONE;
              tn.dStopValue = 0;

            } else if (tnItr->size() == 4) {
              std::string strTrigType = (*tnItr)[0].asString();
              tn.nTriggerType = TrigTypeStr2Value(strTrigType);
              tn.dTriggerValue = (*tnItr)[1].asFloat();
              std::string strTrigDistanceMode = (*tnItr)[2].asString();
              tn.nDistanceMode = TrigDistanceModeStr2Value(strTrigDistanceMode);
              tn.dAcc = (*tnItr)[3].asDouble();
              tn.nCount = -1;
              tn.nStopType = AECT_NONE;
              tn.dStopValue = 0;

            } else if (tnItr->size() == 5) {
              std::string strTrigType = (*tnItr)[0].asString();
              tn.nTriggerType = TrigTypeStr2Value(strTrigType);
              tn.dTriggerValue = (*tnItr)[1].asFloat();
              std::string strTrigDistanceMode = (*tnItr)[2].asString();
              tn.nDistanceMode = TrigDistanceModeStr2Value(strTrigDistanceMode);
              tn.dAcc = (*tnItr)[3].asDouble();
              tn.nCount = (*tnItr)[4].asInt();
              tn.nStopType = AECT_NONE;
              tn.dStopValue = 0;
            } else if (tnItr->size() == 7) {
              std::string strTrigType = (*tnItr)[0].asString();
              tn.nTriggerType = TrigTypeStr2Value(strTrigType);
              tn.dTriggerValue = (*tnItr)[1].asFloat();
              std::string strTrigDistanceMode = (*tnItr)[2].asString();
              tn.nDistanceMode = TrigDistanceModeStr2Value(strTrigDistanceMode);
              tn.dAcc = (*tnItr)[3].asDouble();
              tn.nCount = (*tnItr)[4].asInt();
              std::string strFrontStr = (*tnItr)[5].asString();
              tn.nStopType = AccEndConditionTypeFrontStr2Value(strFrontStr.c_str());
              tn.dStopValue = (*tnItr)[6].asDouble();

            } else {
              tn.nTriggerType = TCT_TIME_ABSOLUTE;
              tn.dTriggerValue = 0.0;
              // timeAbsolute模式下 distance mode值无效
              tn.nDistanceMode = TDM_NONE;
              tn.dAcc = 0.0;
              tn.nStopType = AECT_NONE;
              tn.dStopValue = 0;
            }

            acc.m_nodes.push_back(tn);
          }
          // acc.ConvertToStr();
          acc.ConvertToTimeStr();
          acc.ConvertToEventStr();
        } else {
          acc.m_strProfilesTime = "";
          acc.ConvertToTimeValue();
          SYSTEM_LOGGER_INFO("json acc profile is null");
        }

        CTraffic::AccelerationMap::iterator itr = accs.find(acc.m_strID);
        if (itr == accs.end()) {
          accs.insert(std::make_pair(acc.m_strID, acc));
        } else {
          SYSTEM_LOGGER_WARN("accs id repeated!");
        }
      }
    }

    // merges
    Json::Value& jMerges = jTraffic["merges"];
    if (!jMerges.isNull()) {
      CTraffic::MergeMap& merges = traffic.Merges();

      Json::ValueIterator mItr = jMerges.begin();
      for (; mItr != jMerges.end(); ++mItr) {
        CMerge merge;
        if ((*mItr)["id"].isNull()) {
          continue;
        }
        merge.m_strID = (*mItr)["id"].asString();
        merge.m_ID = atoi(merge.m_strID.c_str());

        if (!(*mItr)["profile"].isNull()) {
          Json::ValueIterator muItr = (*mItr)["profile"].begin();
          for (; muItr != (*mItr)["profile"].end(); ++muItr) {
            tagMergeUnit mu;
            if (muItr->size() == 7) {
              std::string strType = (*muItr)[0].asString();
              mu.nTrigType = TrigTypeStr2Value(strType);
              mu.dTrigValue = (*muItr)[1].asDouble();
              std::string strTrigDistanceMode = (*muItr)[2].asString();
              mu.nDistanceMode = TrigDistanceModeStr2Value(strTrigDistanceMode);
              mu.nDir = (*muItr)[3].asInt();
              mu.dDuration = (*muItr)[4].asDouble();
              mu.dOffset = (*muItr)[5].asDouble();
              mu.nCount = (*muItr)[6].asInt();
            } else if (muItr->size() == 4) {
              std::string strType = (*muItr)[0].asString();
              mu.nTrigType = TrigTypeStr2Value(strType);
              mu.dTrigValue = (*muItr)[1].asDouble();
              std::string strTrigDistanceMode = (*muItr)[2].asString();
              mu.nDistanceMode = TrigDistanceModeStr2Value(strTrigDistanceMode);
              mu.nDir = (*muItr)[3].asInt();

            } else if (muItr->size() == 3) {
              std::string strType = (*muItr)[0].asString();
              mu.nTrigType = TrigTypeStr2Value(strType);
              mu.dTrigValue = (*muItr)[1].asDouble();
              mu.nDistanceMode = TDM_LANE_PROJECTION;
              mu.nDir = (*muItr)[2].asInt();
            } else if (muItr->size() == 2) {
              mu.nTrigType = TCT_TIME_ABSOLUTE;
              mu.dTrigValue = (*muItr)[0].asDouble();
              // timeAbsolute模式下 distance mode值无效
              mu.nDistanceMode = TDM_NONE;
              mu.nDir = (*muItr)[1].asInt();
            } else {
              mu.nTrigType = TCT_TIME_ABSOLUTE;
              mu.dTrigValue = 0;
              // timeAbsolute模式下 distance mode值无效
              mu.nDistanceMode = TDM_NONE;
              mu.nDir = 0;
            }

            merge.m_merges.push_back(mu);
          }
        } else {
          merge.m_strProfileTime = "";
          merge.ConvertToTimeValue();
          SYSTEM_LOGGER_INFO("json merge profile is null");
        }

        // merge.ConvertToStr();
        merge.ConvertToTimeStr();
        merge.ConvertToEventStr();
        CTraffic::MergeMap::iterator itr = merges.find(merge.m_strID);
        if (itr == merges.end()) {
          merges.insert(std::make_pair(merge.m_strID, merge));
        } else {
          SYSTEM_LOGGER_WARN("merge id repeated!");
        }
      }
    }

    // velocities
    Json::Value& jVelocities = jTraffic["velocities"];
    if (!jVelocities.isNull()) {
      CTraffic::VelocityMap& velocities = traffic.Velocities();
      Json::ValueIterator vItr = jVelocities.begin();
      for (; vItr != jVelocities.end(); ++vItr) {
        CVelocity vel;
        if ((*vItr)["id"].isNull()) {
          continue;
        }
        vel.m_strID = (*vItr)["id"].asString();
        vel.m_ID = atoi(vel.m_strID.c_str());
        if (!(*vItr)["profile"].isNull()) {
          Json::ValueIterator tnItr = (*vItr)["profile"].begin();
          for (; tnItr != (*vItr)["profile"].end(); ++tnItr) {
            VelocityTimeNode tn;
            if (tnItr->size() == 2) {
              tn.nTriggerType = TCT_TIME_ABSOLUTE;
              tn.dTriggerValue = (*tnItr)[0].asFloat();
              // timeAbsolute模式下 distance mode值无效
              tn.nDistanceMode = TDM_NONE;
              tn.dVelocity = (*tnItr)[1].asDouble();

            } else if (tnItr->size() == 3) {
              std::string strTrigType = (*tnItr)[0].asString();
              tn.nTriggerType = TrigTypeStr2Value(strTrigType);
              tn.dTriggerValue = (*tnItr)[1].asFloat();
              tn.nDistanceMode = TDM_LANE_PROJECTION;
              tn.dVelocity = (*tnItr)[2].asDouble();

            } else if (tnItr->size() == 4) {
              std::string strTrigType = (*tnItr)[0].asString();
              tn.nTriggerType = TrigTypeStr2Value(strTrigType);
              tn.dTriggerValue = (*tnItr)[1].asFloat();
              std::string strTrigDistanceMode = (*tnItr)[2].asString();
              tn.nDistanceMode = TrigDistanceModeStr2Value(strTrigDistanceMode);
              tn.dVelocity = (*tnItr)[3].asDouble();
            } else if (tnItr->size() == 5) {
              std::string strTrigType = (*tnItr)[0].asString();
              tn.nTriggerType = TrigTypeStr2Value(strTrigType);
              tn.dTriggerValue = (*tnItr)[1].asFloat();
              std::string strTrigDistanceMode = (*tnItr)[2].asString();
              tn.nDistanceMode = TrigDistanceModeStr2Value(strTrigDistanceMode);
              tn.dVelocity = (*tnItr)[3].asDouble();
              tn.nCount = (*tnItr)[4].asInt();
            }
            /*
            else
            {
                    tn.nTriggerType = TCT_TIME_ABSOLUTE;
                    tn.dTriggerValue = 0.0;
                    tn.dVelocity = 0.0;
            }
            */

            vel.m_nodes.push_back(tn);
          }
          // acc.ConvertToStr();
          vel.ConvertToTimeStr();
          vel.ConvertToEventStr();
        } else {
          vel.m_strProfilesTime = "";
          vel.ConvertToTimeValue();
          SYSTEM_LOGGER_INFO("json velocity profile is null");
        }

        CTraffic::VelocityMap::iterator itr = velocities.find(vel.m_strID);
        if (itr == velocities.end()) {
          velocities.insert(std::make_pair(vel.m_strID, vel));
        } else {
          SYSTEM_LOGGER_WARN("velocity id repeated!");
        }
      }
    }

    Json::Value& jTrafficFlow = jTraffic["trafficFlow"];
    if (!jTrafficFlow.isNull()) {
      traffic.TrafficFlow().JsonMeta() = jTrafficFlow;
    }
    // scene events
    Json::Value& jSceneEvents = jTraffic["sceneevents"];
    if (!jSceneEvents.isNull()) {
      CTraffic::SceneEventMap& SceneEvents = traffic.SceneEvents();
      try {
        Json::ValueIterator sItr = jSceneEvents.begin();
        for (; sItr != jSceneEvents.end(); ++sItr) {
          SceneEvent mScene_Event;
          mScene_Event.fromJson2Xml(*sItr);
          auto itr = SceneEvents.find(mScene_Event.m_id);
          if (itr == SceneEvents.end()) {
            SceneEvents.insert(std::make_pair(mScene_Event.m_id, mScene_Event));
          } else {
            SYSTEM_LOGGER_WARN("scene event id repeated!");
          }
        }
      } catch (const std::exception& ex) {
        SYSTEM_LOGGER_FATAL("Parse scene_event json error");
        return -1;
      }
    }

    // vehicles
    Json::Value& jVheicles = jTraffic["vehicles"];
    if (!jVheicles.isNull()) {
      CTraffic::VehicleMap& vehicles = traffic.Vehicles();

      Json::ValueIterator vItr = jVheicles.begin();
      for (; vItr != jVheicles.end(); ++vItr) {
        CVehicle vehicle;
        vehicle.m_strID = (*vItr)["id"].asString();
        vehicle.m_strRouteID = (*vItr)["routeID"].asString();
        vehicle.m_strLaneID = (*vItr)["laneID"].asString();
        vehicle.m_strStartShift = (*vItr)["start_s"].asString();
        vehicle.m_strStartTime = (*vItr)["start_t"].asString();
        vehicle.m_strStartVelocity = (*vItr)["start_v"].asString();
        vehicle.m_strMaxVelocity = (*vItr)["max_v"].asString();
        vehicle.m_strOffset = (*vItr)["l_offset"].asString();
        vehicle.m_strLength = (*vItr)["length"].asString();
        vehicle.m_strWidth = (*vItr)["width"].asString();
        vehicle.m_strHeight = (*vItr)["height"].asString();
        vehicle.m_strAccID = (*vItr)["accID"].asString();
        vehicle.m_strMergeID = (*vItr)["mergeID"].asString();
        vehicle.m_strType = (*vItr)["vehicleType"].asString();
        vehicle.m_strBehavior = (*vItr)["behavior"].asString();
        vehicle.m_strAggress = (*vItr)["aggress"].asString();
        vehicle.m_strFollow = (*vItr)["follow"].asString();
        vehicle.m_strEventId = (*vItr)["eventId"].asString();
        vehicle.m_strAngle = (*vItr)["angle"].asString();
        vehicle.m_strSensorGroup = vItr->get("sensorGroup", "").asString();
        vehicle.m_strObuStauts = vItr->get("obuStatus", "").asString();
        vehicle.m_strStartAngle = vItr->get("start_angle", "").asString();
        vehicle.ConvertToValue();
        vehicle.ConvertToStr();
        CTraffic::VehicleMap::iterator itr = vehicles.find(vehicle.m_strID);
        if (itr == vehicles.end()) {
          vehicles.insert(std::make_pair(vehicle.m_strID, vehicle));
        } else {
          SYSTEM_LOGGER_WARN("vehicle id repeated!");
        }
      }
    }

    // obstacles
    Json::Value& jObstacle = jTraffic["obstacles"];
    if (!jObstacle.isNull()) {
      CTraffic::ObstacleMap& obstacles = traffic.Obstacles();

      Json::ValueIterator oItr = jObstacle.begin();
      for (; oItr != jObstacle.end(); ++oItr) {
        CObstacle obstacle;
        obstacle.m_strID = (*oItr)["id"].asString();
        obstacle.m_strRouteID = (*oItr)["routeID"].asString();
        obstacle.m_strLaneID = (*oItr)["laneID"].asString();
        obstacle.m_strStartShift = (*oItr)["start_s"].asString();
        obstacle.m_strOffset = (*oItr)["l_offset"].asString();
        obstacle.m_strLength = (*oItr)["length"].asString();
        obstacle.m_strWidth = (*oItr)["width"].asString();
        obstacle.m_strHeight = (*oItr)["height"].asString();
        obstacle.m_strType = (*oItr)["type"].asString();
        obstacle.m_strDirection = (*oItr)["direction"].asString();
        obstacle.m_strStartAngle = (*oItr)["start_angle"].asString();
        obstacle.ConvertToValue();
        obstacle.ConvertToStr();

        CTraffic::ObstacleMap::iterator itr = obstacles.find(obstacle.m_strID);
        if (itr == obstacles.end()) {
          obstacles.insert(std::make_pair(obstacle.m_strID, obstacle));
        } else {
          SYSTEM_LOGGER_WARN("obstacle id repeated!");
        }
      }
    }

    // pedestrians
    Json::Value& jPedestrians = jTraffic["pedestrians"];
    int nRet = ParsePedestrians(jPedestrians, traffic);
    if (nRet != 0) {
      SYSTEM_LOGGER_WARN("parse pedestrina error!");
    }

    Json::Value& jPedestriansV2 = jTraffic["pedestriansV2"];
    nRet = ParsePedestriansV2(jPedestriansV2, traffic);
    if (nRet != 0) {
      SYSTEM_LOGGER_WARN("parse pedestrina v2 error!");
    }

    // activePlan
    traffic.m_ActivePlan = jTraffic.get("activePlan", "0").asString();
    // signlights
    Json::Value& jSignlights = jTraffic["signlights"];
    if (!jSignlights.isNull()) {
      CTraffic::SignLightMap& signlights = traffic.Signlights();

      Json::ValueIterator sItr = jSignlights.begin();
      for (; sItr != jSignlights.end(); ++sItr) {
        CSignLight signlight;
        signlight.m_strID = (*sItr)["id"].asString();
        signlight.m_strRouteID = (*sItr)["routeID"].asString();
        signlight.m_strStartShift = (*sItr)["start_s"].asString();
        signlight.m_strStartTime = (*sItr)["start_t"].asString();
        signlight.m_strOffset = (*sItr)["l_offset"].asString();
        signlight.m_strTimeGreen = (*sItr)["time_green"].asString();
        signlight.m_strTimeYellow = (*sItr)["time_yellow"].asString();
        signlight.m_strTimeRed = (*sItr)["time_red"].asString();
        signlight.m_strDirection = (*sItr)["direction"].asString();
        signlight.m_strPhase = (*sItr)["phase"].asString();
        signlight.m_strLane = (*sItr)["lane"].asString();
        bool bEnabled = (*sItr)["enabled"].asBool();
        signlight.m_strStatus = bEnabled ? "Activated" : "Deactivated";
        // for new version signalight
        signlight.m_strPlan = (*sItr)["plan"].asString();
        signlight.m_strJunction = (*sItr)["junction"].asString();
        signlight.m_strPhasenumber = (*sItr)["phaseNumber"].asString();
        std::string m_strSignal = (*sItr).get("signalHead", "").asString();
        std::vector<std::string> strVec;
        boost::algorithm::split(strVec, m_strSignal, boost::algorithm::is_any_of(","));
        for (auto it : strVec) {
          signlight.m_strSignalheadVec.push_back(it);
        }
        signlight.ConvertToValue();

        CTraffic::SignLightMap::iterator itr = signlights.find(signlight.m_strID);
        if (itr == signlights.end()) {
          signlights.insert(std::make_pair(signlight.m_strID, signlight));
        } else {
          SYSTEM_LOGGER_WARN("signlight id repeated!");
        }
      }
    }
  }

  CSimulation& simulation = scene.m_simulation;
  simulation.m_strInfo = jInfos.get("info", "").asString();

  // simulation
  ParseJsonToSim(root["sim"], simulation, traffic);

  CSensors& sensors = scene.m_sensors;
  Json::Value& jSensors = root["sensor"];
  // bool bIncludeSensorInfo = false;
  bool bIncludeSensorInfo = JsonToXmlSensorsInfo(jSensors, sensors);

  CEnvironment& environment = scene.m_environment;
  Json::Value& jEnvironment = root["environment"];
  // bool bIncludeSensorInfo = false;
  bool bIncludeEnvironmentInfo = JsonToXmlEnvironmentInfo(jEnvironment, environment);

  // Grading
  Json::Value& jGrading = root["grading"];
  bool bIncludeGrading = false;
  {
    std::string strGradingFile = jGrading.asString();
    if (strGradingFile.length() > 0) {
      bIncludeGrading = true;
      scene.m_simulation.SetGradingFile(strGradingFile.c_str());
    }
  }

  scene.m_bIncludeEnvironment = bIncludeEnvironmentInfo;
  scene.m_bIncludeSensors = bIncludeSensorInfo;
  scene.m_bIncludeGrading = bIncludeGrading;

  return 0;
}

int CParserJson::SaveToXOSC(const char* pPath, const char* strFileName, sTagSimuTraffic& scene,
                            std::string& strXoscPath) {
  // 保存路径计算
  boost::filesystem::path sceneDirPath = pPath;
  sceneDirPath /= "scene";
  boost::filesystem::path simFilePath = sceneDirPath;
  simFilePath /= strFileName;
  std::string strStemName = simFilePath.stem().string();

  XOSCWriter_1_0_v4 writer;
  boost::filesystem::path xoscPath = sceneDirPath;
  std::string strXoscName = strFileName;
  xoscPath /= (strXoscName);
  strXoscPath = xoscPath.string();
  std::string strExt = xoscPath.extension().string();
  if (strExt.empty() || strExt != ".xosc") {
    strXoscPath.append(".xosc");
  }
  scene.m_simulation.m_nID = scene.m_nID;
  return writer.Save2XOSC(strXoscPath.c_str(), scene.m_traffic, scene.m_simulation, scene.m_environment);
}

int CParserJson::SaveToSim(const char* pPath, const char* strFileName, sTagSimuTraffic& scene,
                           std::string& strSimPath) {
  // 保存路径计算
  boost::filesystem::path p = pPath;
  boost::filesystem::path sceneDirPath = p;
  sceneDirPath /= "scene";
  boost::filesystem::path simFilePath = sceneDirPath;
  simFilePath /= strFileName;
  std::string strExt = simFilePath.extension().string();

  std::string strStemName = simFilePath.stem().string();
  strSimPath = "";
  std::string strTrafficName = strStemName;
  if (strExt.size() == 0 || strExt != ".sim") {
    // std::string strName = strStemName + ".sim";
    std::string strName = strFileName;
    strTrafficName = strFileName;
    strName.append(".sim");
    simFilePath = sceneDirPath;
    // simFilePath.append(strName);
    simFilePath /= strName;
  }
  // traffic文件名
  std::string strProjName = strTrafficName;
  std::string strXoscName = strTrafficName;
  std::string strSensorName = strTrafficName;
  std::string strEnvironmentName = strTrafficName;
  std::string strControlPathName = strTrafficName;
  std::string strGradingName = strTrafficName;
  strTrafficName.append("_traffic.xml");

  boost::filesystem::path trafficPath = sceneDirPath;
  // trafficPath.append(strTrafficName);
  trafficPath /= (strTrafficName);
  // 保存traffic文件
  scene.m_traffic.Save(trafficPath.string().c_str());

  // 保存 sensor 文件
  if (scene.m_bIncludeSensors) {
    strSensorName.append("_sensor.xml");
    boost::filesystem::path sensorPath = sceneDirPath;
    // sensorPath.append(strSensorName);
    sensorPath /= (strSensorName);
    scene.m_sensors.Save(sensorPath.string().c_str());
    scene.m_simulation.SetSensorFile(sensorPath.filename().string().c_str());
    scene.m_project.SetSensorFile(sensorPath.filename().string().c_str());
  } else {
    scene.m_simulation.SetSensorFile("");
    scene.m_project.SetSensorFile("");
  }

  // 保存 environment 文件
  if (scene.m_bIncludeEnvironment) {
    strEnvironmentName.append("_environment.xml");
    boost::filesystem::path environmentPath = sceneDirPath;
    // environmentPath.append(strEnvironmentName);
    environmentPath /= (strEnvironmentName);
    scene.m_environment.Save(environmentPath.string().c_str());
    scene.m_simulation.SetEnvironmentFile(environmentPath.filename().string().c_str());
    scene.m_project.SetEnvironmentFile(environmentPath.filename().string().c_str());
  } else {
    scene.m_simulation.SetEnvironmentFile("");
    scene.m_project.SetEnvironmentFile("");
  }

  // 保存 control path 文件
  // if (bIncludeSensorInfo)
  if (scene.m_simulation.m_planner.m_trajectoryEnabled == "true") {
    strControlPathName.append("_controlpath.txt");
    scene.m_simulation.SetControlPathFile(strControlPathName.c_str());
    scene.m_project.SetControlPathFile(strControlPathName.c_str());
    boost::filesystem::path controlPathPath = sceneDirPath;
    // controlPathPath.append(strControlPathName);
    controlPathPath /= (strControlPathName);
    scene.m_simulation.m_controlTrack.Save(controlPathPath.string().c_str());
  }

  // 保存grading文件
  if (scene.m_bIncludeGrading) {
    scene.m_project.SetGradingFile(scene.m_simulation.m_strGrading.c_str());
  } else {
    scene.m_simulation.SetGradingFile("");
    scene.m_project.SetGradingFile("");
  }

  // 保存sim文件
  if (scene.m_simulation.m_mapFile.m_strMapfile.size() > 0) {
    boost::filesystem::path tmpPath = scene.m_simulation.m_mapFile.m_strMapfile;
    std::string strHadmapName = "../hadmap/";
    strHadmapName.append(tmpPath.filename().string());
    scene.m_simulation.m_mapFile.m_strMapfile = strHadmapName;
  }

  // traffic路径
  scene.m_simulation.SetTrafficFile(trafficPath.filename().string().c_str());

  // 保存sim文件
  scene.m_simulation.m_nID = scene.m_nID;
  scene.m_simulation.SetScenePath(simFilePath.string().c_str());
  scene.m_simulation.Save(simFilePath.string().c_str());
  strSimPath = simFilePath.string();

  // 保存project文件
  scene.m_project.SetSimulationFile(simFilePath.filename().string().c_str());
  scene.m_project.SetTrafficFile(trafficPath.filename().string().c_str());
  strProjName.append(".prj");

  boost::filesystem::path projectPath = sceneDirPath;
  projectPath /= (strProjName);
  scene.m_project.Save(projectPath.string().c_str());

  return 0;
}

int CParserJson::SaveToFile(const char* pPath, const char* strFileName, sTagSimuTraffic& scene,
                            std::string& strSimPath) {
  if (!strFileName || !pPath) {
    SYSTEM_LOGGER_ERROR("save to file param is null");
    return -1;
  }

  int nRet = 0;
  if (boost::iequals(scene.m_strType, "xosc")) {
    nRet = SaveToXOSC(pPath, strFileName, scene, strSimPath);
  } else {
    nRet = SaveToSim(pPath, strFileName, scene, strSimPath);
  }

  if (!boost::filesystem::exists(boost::filesystem::path(strSimPath))) {
    SYSTEM_LOGGER_ERROR("file not exists: %s", strSimPath.c_str());
    return -1;
  }
  return nRet;
}

int CParserJson::ParseSceneParamOneEvent(Json::Value& jOneEvent, sTagBatchSceneParam& param) {
  bool bAccEnabled = true;
  bool bTrigValueEnable = true;
  bool bMergeOffsetEnabled = true;
  bool bMergeDurationEnabled = true;
  bool bVelocityEnabled = true;

  std::string strType = "timeAbsolute";
  Json::Value jType = jOneEvent["type"];
  if (!jType.isNull()) {
    strType = jType["value"].asString();
  }
  int nTriggerType = TrigTypeStr2Value(strType);

  std::string strMode = "";
  Json::Value jMode = jOneEvent["distancemode"];
  if (!jMode.isNull()) {
    strMode = jMode["value"].asString();
  }
  int nDistanceMode = TrigDistanceModeStr2Value(strMode);

  Json::Value jTrigValue;
  if (nTriggerType == TCT_TIME_ABSOLUTE) {
    jTrigValue = jOneEvent["time"];
  } else {
    jTrigValue = jOneEvent["value"];
  }
  if (jTrigValue.isNull()) {
    bTrigValueEnable = false;
    return 0;
  }

  std::string strTrigValueStart = jTrigValue["start"].asString();
  std::string strTrigValueEnd = jTrigValue["end"].asString();
  std::string strTrigValueStep = jTrigValue["sep"].asString();
  double dValueStart = 0, dValueEnd = 0, dValueStep = 0;
  dValueStart = atof(strTrigValueStart.c_str());
  dValueEnd = atof(strTrigValueEnd.c_str());
  dValueStep = std::fabs(atof(strTrigValueStep.c_str()));

  std::string strOrder = "";
  Json::Value jOrder = jOneEvent["times"];
  if (!jOrder.isNull()) {
    strOrder = jOrder["value"].asString();
  }
  int nOrder = 0;
  if (strOrder.size() > 0) {
    nOrder = atoi(strOrder.c_str());
  }

  Json::Value jVelocity = jOneEvent["velocity"];
  std::string strTrigVelocityStart = jVelocity["start"].asString();
  std::string strTrigVelocityEnd = jVelocity["end"].asString();
  std::string strTrigVelocityStep = jVelocity["sep"].asString();
  double dVelocityStart = 0, dVelocityEnd = 0, dVelocityStep = 0;
  if (strTrigVelocityStart.size() == 0 || strTrigVelocityEnd.size() == 0 || strTrigVelocityStep.size() == 0) {
    bVelocityEnabled = false;
  } else {
    dVelocityStart = atof(strTrigVelocityStart.c_str());
    dVelocityEnd = atof(strTrigVelocityEnd.c_str());
    dVelocityStep = std::fabs(atof(strTrigVelocityStep.c_str()));
  }

  Json::Value jAcceleration = jOneEvent["acceleration"];
  std::string strAccelerationStart = jAcceleration["start"].asString();
  std::string strAccelerationEnd = jAcceleration["end"].asString();
  std::string strAccelerationStep = jAcceleration["sep"].asString();
  double dAccelerationStart = 0, dAccelerationEnd = 0, dAccelerationStep = 0, dAccelerationEndValue = 0;
  if (strAccelerationStart.size() == 0 || strAccelerationEnd.size() == 0 || strAccelerationStep.size() == 0) {
    bAccEnabled = false;
  } else {
    dAccelerationStart = atof(strAccelerationStart.c_str());
    dAccelerationEnd = atof(strAccelerationEnd.c_str());
    dAccelerationStep = std::fabs(atof(strAccelerationStep.c_str()));
  }

  std::string strAccelerationEndconditionType = "";
  Json::Value jAccelerationEndConditionType = jOneEvent["accelerationTerminationType"];
  if (!jAccelerationEndConditionType.isNull()) {
    strAccelerationEndconditionType = jAccelerationEndConditionType["value"].asString();
  }

  std::string strAccelerationEndconditionValue = "";
  Json::Value jAccelerationEndConditionValue = jOneEvent["accelerationTerminationValue"];
  if (!jAccelerationEndConditionValue.isNull()) {
    std::string strAccelerationValue = jAccelerationEndConditionValue["value"].asString();
    dAccelerationEndValue = atof(strAccelerationValue.c_str());
  }

  Json::Value jDirection = jOneEvent["direction"];
  std::string strMergeDirection = jDirection["value"].asString();
  int nMergeDir = MergeModeStr2Value(strMergeDirection);

  double dMergeOffsetStart = 0, dMergeOffsetEnd = 0, dMergeOffsetStep = 0;
  double dMergeDurationStart = 0, dMergeDurationEnd = 0, dMergeDurationStep = 0;

  Json::Value jMergeOffset = jOneEvent["offsets"];
  Json::Value jMergeDuration = jOneEvent["durations"];
  std::string strMergeOffsetStart = jMergeOffset["start"].asString();
  std::string strMergeOffsetEnd = jMergeOffset["end"].asString();
  std::string strMergeOffsetStep = jMergeOffset["sep"].asString();
  std::string strMergeDurationStart = jMergeDuration["start"].asString();
  std::string strMergeDurationEnd = jMergeDuration["end"].asString();
  std::string strMergeDurationStep = jMergeDuration["sep"].asString();

  if (strMergeOffsetStart.size() == 0 || strMergeOffsetEnd.size() == 0 || strMergeOffsetStep.size() == 0) {
    bMergeOffsetEnabled = false;
  } else {
    dMergeOffsetStart = atof(strMergeOffsetStart.c_str());
    dMergeOffsetEnd = atof(strMergeOffsetEnd.c_str());
    dMergeOffsetStep = std::fabs(atof(strMergeOffsetStep.c_str()));
  }

  if (strMergeDurationStart.size() == 0 || strMergeDurationEnd.size() == 0 || strMergeDurationStep.size() == 0) {
    bMergeDurationEnabled = false;
  } else {
    dMergeDurationStart = atof(strMergeDurationStart.c_str());
    dMergeDurationEnd = atof(strMergeDurationEnd.c_str());
    dMergeDurationStep = std::fabs(atof(strMergeDurationStep.c_str()));
  }

  if (bAccEnabled) {
    TimeParamNode tpn;
    tpn.nTriggerType = nTriggerType;
    tpn.nDistanceMode = nDistanceMode;
    tpn.nCount = nOrder;
    tpn.dTriggerValueStart = dValueStart;
    tpn.dTriggerValueEnd = dValueEnd;
    tpn.dTriggerValueSep = dValueStep;
    tpn.dAccStart = dAccelerationStart;
    tpn.dAccEnd = dAccelerationEnd;
    tpn.dAccSep = dAccelerationStep;
    tpn.nStopType = AccEndConditionTypeFrontStr2Value(strAccelerationEndconditionType);
    tpn.dStopValue = dAccelerationEndValue;
    param.m_acceleration_param.m_paramNodes.push_back(tpn);
  }

  if (bMergeOffsetEnabled || bMergeDurationEnabled) {
    tagMergeParamUnit mpu;
    mpu.nTrigType = nTriggerType;
    mpu.nDistanceMode = nDistanceMode;
    mpu.nCount = nOrder;
    mpu.nDir = nMergeDir;
    mpu.dTrigValueStart = dValueStart;
    mpu.dTrigValueEnd = dValueEnd;
    mpu.dTrigValueSep = dValueStep;
    mpu.dOffsetStart = dMergeOffsetStart;
    mpu.dOffsetEnd = dMergeOffsetEnd;
    mpu.dOffsetSep = dMergeOffsetStep;
    mpu.dDurationStart = dMergeDurationStart;
    mpu.dDurationEnd = dMergeDurationEnd;
    mpu.dDurationSep = dMergeDurationStep;
    param.m_merge_param.m_paramMerges.push_back(mpu);
  }

  if (bVelocityEnabled) {
    VelocityTimeParamNode vtpn;
    vtpn.nTriggerType = nTriggerType;
    vtpn.nDistanceMode = nDistanceMode;
    vtpn.nCount = nOrder;
    vtpn.dTriggerValueStart = dValueStart;
    vtpn.dTriggerValueEnd = dValueEnd;
    vtpn.dTriggerValueSep = dValueStep;
    vtpn.dVelocityStart = dVelocityStart;
    vtpn.dVelocityEnd = dVelocityEnd;
    vtpn.dVelocitySep = dVelocityStep;
    param.m_velocity_param.m_paramNodes.push_back(vtpn);
  }
  return 0;
}

int CParserJson::ParseSceneParamEvents(Json::Value& jEvents, sTagBatchSceneParam& param) {
  if (!jEvents.isNull()) {
    Json::Value jTimeList = jEvents["timeList"];
    Json::Value jConditionList = jEvents["conditionList"];

    Json::Value::iterator jItr = jTimeList.begin();
    for (; jItr != jTimeList.end(); ++jItr) {
      ParseSceneParamOneEvent((*jItr), param);
    }
    jItr = jConditionList.begin();
    for (; jItr != jConditionList.end(); ++jItr) {
      ParseSceneParamOneEvent((*jItr), param);
    }
  }
  return 0;
}

int CParserJson::ParseScenarioSetListParams(const char* strJson, sTagGetScenarioSetListParam& param) {
  int nLen = strlen(strJson);
  if (nLen < 1) {
    return 0;
  }

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse get scenario set list param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jParam = root;

  if (!jParam.isNull()) {
    param.m_nOffset = jParam["offset"].asInt();
    param.m_nLimit = jParam["limit"].asInt();
  }

  return 0;
}

int CParserJson::ParseScenarioSetCreateParams(const char* strJson, sTagCreateScenarioSetParam& param) {
  int nLen = strlen(strJson);

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse create scenario set param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jParam = root;

  if (!jParam.isNull()) {
    param.m_strName = jParam["name"].asString();
    param.m_strScenes = jParam["scenes"].asString();
  }

  return 0;
}

int CParserJson::ParseScenarioSetUpdateParams(const char* strJson, sTagUpdateScenarioSetParam& param) {
  int nLen = strlen(strJson);

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse update scenario set param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jParam = root;

  if (!jParam.isNull()) {
    param.m_nID = jParam["id"].asInt();
    param.m_strName = jParam["name"].asString();
    param.m_strScenes = jParam["scenes"].asString();
  }

  return 0;
}

int CParserJson::ParseScenarioSetDeleteParams(const char* strJson, sTagDeleteScenarioSetParam& param) {
  int nLen = strlen(strJson);

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse delete scenario set param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jParam = root;

  if (!jParam.isNull()) {
    param.m_nID = jParam["id"].asInt();
  }

  return 0;
}

int CParserJson::ParseScenarioGetParams(const char* strJson, sTagEntityScenario& param) {
  int nLen = strlen(strJson);

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse scenario create param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jParam = root;

  if (!jParam.isNull()) {
    if (!jParam.isMember("id")) {
      param.m_nID = -1;
    } else if (!jParam["id"].isNull()) {
      param.m_nID = jParam["id"].asInt();
    } else {
      param.m_nID = 0;
    }

    if (!jParam["name"].isNull()) {
      param.m_strName = jParam["name"].asString();
    }

    if (!jParam["bucket"].isNull()) {
      param.m_strCosBucket = jParam["bucket"].asString();
    }

    if (!jParam["key"].isNull()) {
      param.m_strCosKey = jParam["key"].asString();
    }

    if (!jParam["mapName"].isNull()) {
      param.m_strMapName = jParam["mapName"].asString();
    }

    if (!jParam["mapVersion"].isNull()) {
      param.m_strMapVersion = jParam["mapVersion"].asString();
    }

    param.m_strMapUrl = jParam.get("mapUrl", "").asString();
  }

  return 0;
}

int CParserJson::ParseScenarioCreateParams(const char* strJson, sTagEntityScenario& param) {
  int nLen = strlen(strJson);

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse scenario create param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jParam = root;

  if (!jParam.isNull()) {
    if (!jParam["id"].isNull()) {
      param.m_nID = jParam["id"].asInt();
    } else {
      param.m_nID = -1;
    }

    if (!jParam["name"].isNull()) {
      param.m_strName = jParam["name"].asString();
    }
    if (!jParam["map"].isNull()) {
      param.m_strMap = jParam["map"].asString();
    }
    if (!jParam["info"].isNull()) {
      param.m_strInfo = jParam["info"].asString();
    }
    if (!jParam["content"].isNull()) {
      param.m_strContent = jParam["content"].asString();
    }
    if (!jParam["bucket"].isNull()) {
      param.m_strCosBucket = jParam["bucket"].asString();
    }

    if (!jParam["key"].isNull()) {
      param.m_strCosKey = jParam["key"].asString();
    }

    if (!jParam["mapName"].isNull()) {
      param.m_strMapName = jParam["mapName"].asString();
    }

    if (!jParam["mapVersion"].isNull()) {
      param.m_strMapVersion = jParam["mapVersion"].asString();
    }

    boost::filesystem::path name(param.m_strName);
    std::string ext = name.extension().string();
    if (ext.empty() || (ext != ".sim" && ext != ".xosc")) {
      ext = std::string(".") + jParam.get("format", "SIM").asString();
      std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
      param.m_strName.append(ext);
    }

    param.m_strMapUrl = jParam.get("mapUrl", "").asString();
    param.m_strNavMeshUrl = jParam.get("navMeshUrl", "").asString();
  }

  return 0;
}

int CParserJson::ParseScenarioSearchParams(const char* strJson, sTagSearchScenarioParam& param) {
  int nLen = strlen(strJson);

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse scenario search param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jParam = root;

  if (!jParam.isNull()) {
    param.m_strType = jParam["type"].asString();
    param.m_strKeys = jParam["key"].asString();
  }

  return 0;
}

int CParserJson::ParseScenarioListParams(const char* strJson, sTagGetScenarioListParam& param) {
  int nLen = strlen(strJson);

  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse scenario create param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jParam = root;

  if (!jParam.isNull()) {
    if (!jParam["offset"].isNull()) {
      param.m_nOffset = jParam["offset"].asInt();
    } else {
      param.m_nOffset = -1;
    }

    if (!jParam["limit"].isNull()) {
      param.m_nLimit = jParam["limit"].asInt();
    } else {
      param.m_nLimit = -1;
    }
  }

  return 0;
}

int CParserJson::ParseScenarioInfoListParams(const char* strJson, sTagGetScenarioInfoListParam& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse scenario info list param json failed: ", strJson);
    return -1;
  }

  std::string strIDS = root["ids"].asString();
  std::vector<std::string> ids;
  boost::algorithm::split(ids, strIDS, boost::is_any_of(", "), boost::token_compress_on);
  std::copy_if(std::begin(ids), std::end(ids), std::inserter(param.m_ids, std::begin(param.m_ids)),
               [](const std::string& id) { return id.size() > 0; });
  // source
  /*
  Json::Value ids = root["ids"];

  Json::ValueIterator itr = ids.begin();
  for (; itr != ids.end(); ++itr)
  {
          std::string strID = (*itr)["id"].asString();
          param.m_ids.emplace_back(strID);
  }
  */

  return 0;
}

int CParserJson::ParseSceneParam(const char* strJson, sTagBatchSceneParam& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parser param scene json failed: ", strJson);
    return -1;
  }

  // export flag
  Json::Value& jExportFlag = root["export_flag"];
  if (!jExportFlag.isNull()) {
    param.m_export_flag = jExportFlag.asInt();
  }

  // type
  Json::Value& jType = root["type"];

  if (!jType.isNull()) {
    std::string strType = jType.asString();
    if (boost::iequals(strType, "single")) {
      param.m_type = SCENE_BATCH_GEN_TYPE_SINGLE;
    } else {
      param.m_type = SCENE_BATCH_GEN_TYPE_ALL;
    }
  }

  // prefix
  Json::Value& jPrefix = root["prefix"];

  if (!jPrefix.isNull()) {
    std::string strPrefix = jPrefix.asString();
    param.m_strPrefix = strPrefix;
  }

  // vehicle
  Json::Value& jVehicle = root["vehicle"];
  if (!jVehicle.isNull()) {
    if (param.m_type == SCENE_BATCH_GEN_TYPE_SINGLE) {
      std::string strID = jVehicle["id"].asString();
      param.m_vehicle_id = atoi(strID.c_str());
    }
    Json::Value& jPos = jVehicle["pos"];
    param.m_vehicle_pos_start = jPos["start"].asDouble();
    param.m_vehicle_pos_end = jPos["end"].asDouble();
    param.m_vehicle_pos_sep = jPos["sep"].asDouble();

    Json::Value& jOffset = jVehicle["offset"];
    param.m_vehicle_offset_start = jOffset["start"].asDouble();
    param.m_vehicle_offset_end = jOffset["end"].asDouble();
    param.m_vehicle_offset_sep = jOffset["sep"].asDouble();

    if (param.m_vehicle_pos_start > param.m_vehicle_pos_end) {
      double dTmp = param.m_vehicle_pos_start;
      param.m_vehicle_pos_start = param.m_vehicle_pos_end;
      param.m_vehicle_pos_end = dTmp;
    }
    if (param.m_vehicle_pos_sep < 0) {
      param.m_vehicle_pos_sep *= -1;
    }

    Json::Value& jVelocity = jVehicle["velocity"];
    param.m_vehicle_velocity_start = jVelocity["start"].asDouble();
    param.m_vehicle_velocity_end = jVelocity["end"].asDouble();
    param.m_vehicle_velocity_sep = jVelocity["sep"].asDouble();

    if (param.m_vehicle_velocity_start > param.m_vehicle_velocity_end) {
      double dTmp = param.m_vehicle_velocity_start;
      param.m_vehicle_velocity_start = param.m_vehicle_velocity_end;
      param.m_vehicle_velocity_end = dTmp;
    }
    if (param.m_vehicle_velocity_sep < 0) {
      param.m_vehicle_velocity_sep *= -1;
    }

    // event
    Json::Value& jTrigger = jVehicle["trigger"];
    ParseSceneParamEvents(jTrigger, param);
  }

  // ego
  Json::Value& jEgo = root["ego"];
  if (!jEgo.isNull()) {
    Json::Value& jVelocity = jEgo["velocity"];
    param.m_ego_velocity_start = jVelocity["start"].asDouble();
    param.m_ego_velocity_end = jVelocity["end"].asDouble();
    param.m_ego_velocity_sep = jVelocity["sep"].asDouble();

    if (param.m_ego_velocity_start > param.m_ego_velocity_end) {
      double dTmp = param.m_ego_velocity_start;
      param.m_ego_velocity_start = param.m_ego_velocity_end;
      param.m_ego_velocity_end = dTmp;
    }
    if (param.m_ego_velocity_sep < 0) {
      param.m_ego_velocity_sep *= -1;
    }
  }

  return 0;
}

int CParserJson::ParseScenesDeleteParams(const char* strJson, sTagScenesDeleteParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse scenes delete param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jFiles = root;

  if (!jFiles.isNull()) {
    Json::ValueIterator fItr = jFiles.begin();
    for (; fItr != jFiles.end(); ++fItr) {
      if (!(*fItr)["id"].isNull()) {
        std::string strID = (*fItr)["id"].asString();
        param.m_ids.push_back(strID);
      }

      /*
      if (!(*fItr)["name"].isNull())
      {
              std::string strFileName = (*fItr)["name"].asString();
              param.m_sceneNames.push_back(strFileName);
      }
      */
    }
  }

  return 0;
}

int CParserJson::ParseScenesDownloadParams(const char* strJson, sTagScenesDownloadParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse scenes delete param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jFiles = root["sources"];

  if (!jFiles.isNull()) {
    Json::ValueIterator fItr = jFiles.begin();
    for (; fItr != jFiles.end(); ++fItr) {
      std::string strFileName = (*fItr)["name"].asString();
      param.m_sceneNames.push_back(strFileName);
    }
  }

  // destination
  param.m_strDestDir = root["dest"].asString();

  return 0;
}

int CParserJson::ParseScenesUploadParams(const char* strJson, sTagScenesUploadParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse scenes delete param json failed: ", strJson);
    return -1;
  }

  // source
  param.m_strType = root["type"].asString();
  if (boost::iequals(param.m_strType, "dir")) {
    param.m_strSourceDir = root["dir"].asString();
  } else if (boost::iequals(param.m_strType, "files")) {
    param.m_strSourceDir = root["dir"].asString();
    // files
    Json::Value& jFiles = root["sources"];

    if (!jFiles.isNull()) {
      Json::ValueIterator fItr = jFiles.begin();
      for (; fItr != jFiles.end(); ++fItr) {
        std::string strFileName = (*fItr)["name"].asString();
        param.m_sceneNames.push_back(strFileName);
      }
    }
  } else {
    SYSTEM_LOGGER_ERROR("upload scenes param error");
    return -1;
  }

  return 0;
}

int CParserJson::ParseHadmapsDeleteParams(const char* strJson, sTagHadmapsDeleteParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse hadmaps delete param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jFiles = root;

  if (!jFiles.isNull()) {
    Json::ValueIterator fItr = jFiles.begin();
    for (; fItr != jFiles.end(); ++fItr) {
      std::string strFileName = (*fItr)["name"].asString();
      param.m_hadmapNames.push_back(strFileName);
    }
  }

  return 0;
}

int CParserJson::ParseHadmapsDownloadParams(const char* strJson, sTagHadmapsDownloadParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse hadmaps delete param json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jFiles = root["sources"];

  if (!jFiles.isNull()) {
    Json::ValueIterator fItr = jFiles.begin();
    for (; fItr != jFiles.end(); ++fItr) {
      std::string strFileName = (*fItr)["name"].asString();
      param.m_hadmapNames.push_back(strFileName);
    }
  }

  // destination
  param.m_strDestName = root["dest"].asString();

  return 0;
}

bool CParserJson::basicParser(const char* strJson, Json::Value* retJ) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    return false;
  }
  *retJ = root;
  return true;
}

int CParserJson::ParseGetForwardPointsParam(const char* strJson, sTagGetForwardPointsParam& param) {
  Json::Value root;
  if (!basicParser(strJson, &root)) {
    SYSTEM_LOGGER_ERROR("parse get forward point  param  json failed: ", strJson);
    return -1;
  }

  param.hadmapName = root["hadmapName"].asString();
  param.x = root["x"].asDouble();
  param.y = root["y"].asDouble();
  param.z = root["z"].asDouble();
  param.length = root["length"].asDouble();
  param.interval = root["interval"].asDouble();
  return 0;
}

/*int CParserJson::ParseGetLaneRelParam(const char* strJson,
                                      sTagGetLaneRelParam& param) {
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;

    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    int nLen = strlen(strJson);
    if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
        SYSTEM_LOGGER_ERROR("parse get lane rel callback param  json failed: ",
                            strJson);
        return -1;
    }

    // files
    Json::Value& jFiles = root["hadmaps"];

    if (!jFiles.isNull()) {
        Json::ValueIterator fItr = jFiles.begin();
        for (; fItr != jFiles.end(); ++fItr) {
            std::string strFileName = (*fItr)["name"].asString();
            param.m_hadmapNames.push_back(strFileName);
        }
    } else {
        SYSTEM_LOGGER_ERROR(
            "The hadmap names is empty/no-exist fro "
            "cloudHadmapUploadCallbackReq param.");
        return -1;
    }

    return 0;
}*/

// int ParseGetRoadRelParam(const char* strJson, sTagGetRoadRelParam& param)
int CParserJson::ParseGetRoadRelParam(const char* strJson, sTagGetRoadRelParam& param) {
  return ParseGetRelParam(strJson, param);
}

int CParserJson::ParseGetLaneRelParam(const char* strJson, sTagGetLaneRelParam& param) {
  return ParseGetRelParam(strJson, param);
}

int CParserJson::ParseCloudHadmapUploadCallbackParam(const char* strJson, sTagCloudHadmapUploadCallbackParam& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse cloud hadmap upload callback param  json failed: ", strJson);
    return -1;
  }

  // files
  Json::Value& jFiles = root["hadmaps"];

  if (!jFiles.isNull()) {
    Json::ValueIterator fItr = jFiles.begin();
    for (; fItr != jFiles.end(); ++fItr) {
      std::pair<std::string, std::string> cosInfo;
      cosInfo.first = (*fItr).get("cosBucket", "").asString();
      cosInfo.second = (*fItr).get("cosKey", "").asString();
      std::string strFileName = cosInfo.second.empty() ? fItr->get("name", "").asString()
                                                       : boost::filesystem::path(cosInfo.second).filename().string();
      std::string strVersion = fItr->get("version", "").asString();
      std::string strMapName = computeMapCompleteName(strFileName, strVersion);
      param.m_hadmapNames.push_back(strMapName);
      param.m_hadmapCosInfos[strMapName] = cosInfo;
    }
  } else {
    SYSTEM_LOGGER_ERROR(
        "The hadmap names is empty/no-exist fro "
        "cloudHadmapUploadCallbackReq param.");
    return -1;
  }

  return 0;
}

int CParserJson::ParseHadmapsUploadParams(const char* strJson, sTagHadmapsUploadParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse scenes delete param json failed: ", strJson);
    return -1;
  }

  // source
  param.m_strType = root["type"].asString();
  if (boost::iequals(param.m_strType, "dir")) {
    param.m_strSourceDir = root["dir"].asString();
  } else if (boost::iequals(param.m_strType, "files")) {
    param.m_strSourceDir = root["dir"].asString();
    // files
    Json::Value& jFiles = root["sources"];

    if (!jFiles.isNull()) {
      Json::ValueIterator fItr = jFiles.begin();
      for (; fItr != jFiles.end(); ++fItr) {
        std::string strFileName = (*fItr)["name"].asString();
        param.m_hadmapNames.push_back(strFileName);
      }
    }
  } else {
    SYSTEM_LOGGER_ERROR("upload hadmaps param error");
    return -1;
  }

  return 0;
}

int CParserJson::ParseQueryNearbyInfoParams(const char* strJson, sTagQueryNearbyInfoParams& param) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse query nearby info param json failed: ", strJson);
    return -1;
  }

  // source
  param.m_strHadmapName = root["mapname"].asString();
  std::string strLon = root["lon"].asString();
  std::string strLat = root["lat"].asString();

  param.m_dLon = atof(strLon.c_str());
  param.m_dLat = atof(strLat.c_str());

  return 0;
}

int CParserJson::SaveGlobalSensorConfig(const char* strJson) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse scenes delete param json failed: ", strJson);
    return -1;
  }

  CSensors sensors;
  bool bIncludeSensors = JsonToXmlSensorsInfo(root, sensors);

  if (!bIncludeSensors) {
    SYSTEM_LOGGER_ERROR("no sensor info in global sensors");
    return -1;
  }

  // 保存 sensor 文件
  boost::filesystem::path sensorsDirPath = CEngineConfig::Instance().SensorsDir();
  boost::filesystem::path GlobalSensorFilePath = sensorsDirPath;
  GlobalSensorFilePath /= "sensor_global.xml";
  sensors.Save(GlobalSensorFilePath.string().c_str());

  return 0;
}

std::string CParserJson::LoadGlobalSensorConfig() {
  // 载入sensor 文件
  boost::filesystem::path sensorsDirPath = CEngineConfig::Instance().SensorsDir();
  boost::filesystem::path GlobalSensorFilePath = sensorsDirPath;
  GlobalSensorFilePath /= "sensor_global.xml";
  CSensors sensors;
  int nRet = sensors.Parse(GlobalSensorFilePath.string().c_str());
  if (nRet != 0) {
    return "";
  }

  Json::Value root;
  bool bIncludeSensorsData = XmlToJsonSensorsInfo(sensors, root);

  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream ss;
  writer->write(root, &ss);
  std::string strUnstyledResult = ss.str();

  return strUnstyledResult;
}

int CParserJson::SaveGlobalEnvironmentConfig(const char* strJson) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;

  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  Json::Value root;
  int nLen = strlen(strJson);
  if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
    SYSTEM_LOGGER_ERROR("parse global environment param json failed: ", strJson);
    return -1;
  }

  CEnvironment environment;
  bool bIncludeEnvironment = JsonToXmlEnvironmentInfo(root, environment);

  if (!bIncludeEnvironment) {
    SYSTEM_LOGGER_ERROR("no environment info in global environment");
    return -1;
  }

  // 保存 sensor 文件
  boost::filesystem::path sensorsDirPath = CEngineConfig::Instance().SensorsDir();
  boost::filesystem::path GlobalEnvironmentFilePath = sensorsDirPath;
  GlobalEnvironmentFilePath /= "environment_global.xml";
  environment.Save(GlobalEnvironmentFilePath.string().c_str());

  return 0;
}

std::string CParserJson::LoadGlobalEnvironmentConfig() {
  // 载入sensor 文件
  boost::filesystem::path sensorsDirPath = CEngineConfig::Instance().SensorsDir();
  boost::filesystem::path GlobalEnvironmentFilePath = sensorsDirPath;
  GlobalEnvironmentFilePath /= "environment_global.xml";
  CEnvironment environment;
  int nRet = environment.Parse(GlobalEnvironmentFilePath.string().c_str());
  if (nRet != 0) {
    return "";
  }

  Json::Value root;
  bool bIncludeEnvironmentData = XmlToJsonEnvironmentInfo(environment, root);

  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream ss;
  writer->write(root, &ss);
  std::string strUnstyledResult = ss.str();

  return strUnstyledResult;
}

bool CParserJson::JsonToXmlEnvironmentInfo(Json::Value& jEnvironment, CEnvironment& environment) {
  bool bIncludeEnvironmentInfo = false;
  // environment info
  if (!jEnvironment.isNull()) {
    bIncludeEnvironmentInfo = true;
    // weathers
    Json::ValueIterator itr = jEnvironment.begin();
    for (; itr != jEnvironment.end(); ++itr) {
      CWeatherFrame frame;
      frame.m_timestamp = (*itr)["TimeStamp"].asInt();
      Json::Value jWeather = (*itr)["Weather"];
      if (!jWeather.isNull()) {
        Json::Value jVisibility = jWeather["Visibility"];
        if (!jVisibility.isNull()) {
          double dVisible = jVisibility.asDouble();
          frame.m_weather.m_strVisibility = std::to_string(dVisible);
        }

        Json::Value jRainFall = jWeather["RainFall"];
        if (!jRainFall.isNull()) {
          double dRainFall = jRainFall.asDouble();
          frame.m_weather.m_strRainFall = std::to_string(dRainFall);
        }

        Json::Value jSnowFall = jWeather["SnowFall"];
        if (!jSnowFall.isNull()) {
          double dSnowFall = jSnowFall.asDouble();
          frame.m_weather.m_strSnowFall = std::to_string(dSnowFall);
        }

        Json::Value jWindSpeed = jWeather["WindSpeed"];
        if (!jWindSpeed.isNull()) {
          double dWindSpeed = jWindSpeed.asDouble();
          frame.m_weather.m_strWindSpeed = std::to_string(dWindSpeed);
        }

        Json::Value jCloudDensity = jWeather["CloudDensity"];
        if (!jCloudDensity.isNull()) {
          double dCloudDensity = jCloudDensity.asDouble();
          frame.m_weather.m_strCloudDensity = std::to_string(dCloudDensity);
        }

        Json::Value jTemperature = jWeather["Temperature"];
        if (jTemperature.isDouble()) {
          double dTemperature = jTemperature.asDouble();
          frame.m_weather.m_strTemperature = std::to_string(dTemperature);
        }

        Json::Value jCloudState = jWeather["CloudState"];
        if (!jTemperature.isNull()) {
          frame.m_weather.m_strCloudState = jCloudState.asString();
        }
        Json::Value jPrecipitationType = jWeather["PrecipitationType"];
        if (!jPrecipitationType.isNull()) {
          frame.m_weather.m_strPrecipitationType = jPrecipitationType.asString();
        }
        Json::Value jPrecipitationIntensity = jWeather["PrecipitationIntensity"];
        if (!jPrecipitationIntensity.isNull()) {
          double dPrecipitationIntensity = jPrecipitationIntensity.asDouble();
          frame.m_weather.m_strPrecipitationIntensity = std::to_string(dPrecipitationIntensity);
        }
        Json::Value jVisualRange = jWeather["VisualRange"];
        if (!jVisualRange.isNull()) {
          double dVisualRange = jVisualRange.asDouble();
          frame.m_weather.m_strVisualRange = std::to_string(dVisualRange);
        }

        frame.m_weather.m_lTimeStamp = frame.m_timestamp;
      }

      Json::Value jDate = (*itr)["Date"];
      if (!jDate.isNull()) {
        int nYear = jDate["Year"].asInt();
        frame.m_date.m_strYear = std::to_string(nYear);
        int nMonth = jDate["Month"].asInt();
        frame.m_date.m_strMonth = std::to_string(nMonth);
        int nDay = jDate["Day"].asInt();
        frame.m_date.m_strDay = std::to_string(nDay);
        frame.m_date.m_lTimeStamp = frame.m_timestamp;
      }

      Json::Value jTime = (*itr)["Time"];
      if (!jTime.isNull()) {
        int nHour = jTime["Hour"].asInt();
        frame.m_time.m_strHour = std::to_string(nHour);
        int nMinute = jTime["Minute"].asInt();
        frame.m_time.m_strMinute = std::to_string(nMinute);
        int nSecond = jTime["Second"].asInt();
        frame.m_time.m_strSecond = std::to_string(nSecond);
        int nMilliSecond = jTime["Millisecond"].asInt();
        frame.m_time.m_strMillisecond = std::to_string(nMilliSecond);
        frame.m_time.m_lTimeStamp = frame.m_timestamp;
      }
      CEnvironment::Weathers::iterator itr = environment.WeatherData().find(frame.m_timestamp);
      if (itr == environment.WeatherData().end()) {
        environment.WeatherData().insert(std::make_pair(frame.m_timestamp, frame));
      } else {
        SYSTEM_LOGGER_WARN("environment timestamp repeated!");
      }
    }
  }

  return bIncludeEnvironmentInfo;
}

bool CParserJson::XmlToJsonEnvironmentInfo(CEnvironment& environment, Json::Value& jEnvironment) {
  bool bIncludeEnvironmentData = false;

  // environment
  CEnvironment::Weathers weathers = environment.WeatherData();
  CEnvironment::Weathers::iterator itr = weathers.begin();
  for (; itr != weathers.end(); ++itr) {
    Json::Value jTimeLine;
    jTimeLine["TimeStamp"] = (Json::Value::UInt64)(itr->second.m_timestamp);
    {
      CWeatherV2& weather = itr->second.m_weather;
      Json::Value jWeather;
      jWeather["WindSpeed"] = atof(weather.m_strWindSpeed.c_str());
      jWeather["VisualRange"] = atof(weather.m_strVisualRange.c_str());
      jWeather["CloudState"] = (weather.m_strCloudState);
      jWeather["PrecipitationType"] = (weather.m_strPrecipitationType);
      jWeather["PrecipitationIntensity"] = atof(weather.m_strPrecipitationIntensity.c_str());
      jWeather["Temperature"] = atof(weather.m_strTemperature.c_str());
      jTimeLine["Weather"] = jWeather;
      bIncludeEnvironmentData = true;
    }

    CWeatherDate& date = itr->second.m_date;
    if (date.m_strYear.size() > 0) {
      Json::Value jDate;
      jDate["Year"] = atoi(date.m_strYear.c_str());
      jDate["Month"] = atoi(date.m_strMonth.c_str());
      jDate["Day"] = atoi(date.m_strDay.c_str());
      jTimeLine["Date"] = jDate;
      bIncludeEnvironmentData = true;
    }

    CWeatherTime& time = itr->second.m_time;
    if (time.m_strHour.size() > 0) {
      Json::Value jTime;
      jTime["Hour"] = atoi(time.m_strHour.c_str());
      jTime["Minute"] = atoi(time.m_strMinute.c_str());
      jTime["Second"] = atoi(time.m_strSecond.c_str());
      jTime["MilliSecond"] = atoi(time.m_strMillisecond.c_str());
      jTimeLine["Time"] = jTime;
      bIncludeEnvironmentData = true;
    }
    jEnvironment.append(jTimeLine);
  }

  return bIncludeEnvironmentData;
}

bool CParserJson::JsonToXmlSensorsInfo(Json::Value& jSensors, CSensors& sensors) {
  bool bIncludeSensorInfo = false;
  // sensor info
  if (!jSensors.isNull()) {
    bIncludeSensorInfo = true;
    // cameras
    Json::Value& jCameras = jSensors["cameras"];
    if (!jCameras.isNull()) {
      CSensors::CameraMap& cameras = sensors.Cameras();

      Json::ValueIterator cItr = jCameras.begin();
      for (; cItr != jCameras.end(); ++cItr) {
        CSensorCamera camera;
        camera.m_strID = (*cItr)["ID"].asString();
        camera.m_bEnabled = (*cItr)["Enabled"].asBool();
        camera.m_strLocationX = (*cItr)["LocationX"].asString();
        camera.m_strLocationY = (*cItr)["LocationY"].asString();
        camera.m_strLocationZ = (*cItr)["LocationZ"].asString();
        camera.m_strRotationX = (*cItr)["RotationX"].asString();
        camera.m_strRotationY = (*cItr)["RotationY"].asString();
        camera.m_strRotationZ = (*cItr)["RotationZ"].asString();
        camera.m_strInstallSlot = (*cItr)["InstallSlot"].asString();
        camera.m_strFrequency = (*cItr)["Frequency"].asString();
        camera.m_strDevice = (*cItr)["Device"].asString();
        camera.m_nIntrinsicParamType = (*cItr)["InsideParamGroup"].asInt();
        // camera.m_strSaveData = (*cItr)["SaveData"].asBool();
        // camera.m_bSaveData = (*cItr)["SaveData"].asBool();
        camera.m_strDisplayMode = (*cItr)["DisplayMode"].asString();
        camera.m_strResHorizontal = (*cItr)["Res_Horizontal"].asString();
        camera.m_strResVertical = (*cItr)["Res_Vertical"].asString();
        camera.m_strBlurIntensity = (*cItr)["Blur_Intensity"].asString();
        camera.m_strMotionBlurAmount = (*cItr)["MotionBlur_Amount"].asString();
        camera.m_strVignetteIntensity = (*cItr)["Vignette_Intensity"].asString();
        camera.m_strNoiseIntensity = (*cItr)["Noise_Intensity"].asString();
        camera.m_strDistortionParamters = (*cItr)["Distortion_Parameters"].asString();
        camera.m_strIntrinsicMatrix = (*cItr)["Intrinsic_Matrix"].asString();
        camera.m_strFOVHorizontal = (*cItr)["FOV_Horizontal"].asString();
        camera.m_strFOVVertical = (*cItr)["FOV_Vertical"].asString();
        camera.m_strCCDWidth = (*cItr)["CCD_Width"].asString();
        camera.m_strCCDHeight = (*cItr)["CCD_Height"].asString();
        camera.m_strFocalLength = (*cItr)["Focal_Length"].asString();

        CSensors::CameraMap::iterator itr = cameras.find(camera.m_strID);
        if (itr == cameras.end()) {
          cameras.insert(std::make_pair(camera.m_strID, camera));
        } else {
          SYSTEM_LOGGER_WARN("camera id repeated!");
        }
      }
    }
    // lidars
    Json::Value& jLidars = jSensors["lidars"];
    if (!jLidars.isNull()) {
      CSensors::TraditionalLidarMap& lidars = sensors.TraditionLidars();

      Json::ValueIterator lItr = jLidars.begin();
      for (; lItr != jLidars.end(); ++lItr) {
        CSensorTraditionalLindar lidar;
        lidar.m_strID = (*lItr)["ID"].asString();
        lidar.m_bEnabled = (*lItr)["Enabled"].asBool();
        lidar.m_strLocationX = (*lItr)["LocationX"].asString();
        lidar.m_strLocationY = (*lItr)["LocationY"].asString();
        lidar.m_strLocationZ = (*lItr)["LocationZ"].asString();
        lidar.m_strRotationX = (*lItr)["RotationX"].asString();
        lidar.m_strRotationY = (*lItr)["RotationY"].asString();
        lidar.m_strRotationZ = (*lItr)["RotationZ"].asString();
        lidar.m_strInstallSlot = (*lItr)["InstallSlot"].asString();
        lidar.m_strFrequency = (*lItr)["Frequency"].asString();
        lidar.m_strDevice = (*lItr)["Device"].asString();
        // lidar.m_strDrawPoint = (*lItr)["DrawPoint"].asBool();
        // lidar.m_strDrawRay = (*lItr)["DrawRay"].asBool();
        lidar.m_bDrawPoint = (*lItr)["DrawPoint"].asBool();
        lidar.m_bDrawRay = (*lItr)["DrawRay"].asBool();
        lidar.m_strModel = (*lItr)["Model"].asString();
        lidar.m_strUChannels = (*lItr)["uChannels"].asString();
        lidar.m_strURange = (*lItr)["uRange"].asString();
        lidar.m_strUHorizontalResolution = (*lItr)["uHorizontalResolution"].asString();
        lidar.m_strUUpperFov = (*lItr)["uUpperFov"].asString();
        lidar.m_strULowerFov = (*lItr)["uLowerFov"].asString();
        // lidar.m_strUSaveData = (*lItr)["uSaveData"].asBool();
        // lidar.m_bUSaveData = (*lItr)["uSaveData"].asBool();

        CSensors::TraditionalLidarMap::iterator itr = lidars.find(lidar.m_strID);
        if (itr == lidars.end()) {
          lidars.insert(std::make_pair(lidar.m_strID, lidar));
        } else {
          SYSTEM_LOGGER_WARN("lidar id repeated!");
        }
      }
    }
    // radars
    Json::Value& jRadars = jSensors["radars"];
    if (!jRadars.isNull()) {
      CSensors::RadarMap& radars = sensors.Radars();

      Json::ValueIterator rItr = jRadars.begin();
      for (; rItr != jRadars.end(); ++rItr) {
        CSensorRadar radar;
        radar.m_strID = (*rItr)["ID"].asString();
        radar.m_bEnabled = (*rItr)["Enabled"].asBool();
        radar.m_strLocationX = (*rItr)["LocationX"].asString();
        radar.m_strLocationY = (*rItr)["LocationY"].asString();
        radar.m_strLocationZ = (*rItr)["LocationZ"].asString();
        radar.m_strRotationX = (*rItr)["RotationX"].asString();
        radar.m_strRotationY = (*rItr)["RotationY"].asString();
        radar.m_strRotationZ = (*rItr)["RotationZ"].asString();
        radar.m_strInstallSlot = (*rItr)["InstallSlot"].asString();
        radar.m_strFrequency = (*rItr)["Frequency"].asString();
        radar.m_strDevice = (*rItr)["Device"].asString();
        // radar.m_strSaveData = (*rItr)["SaveData"].asBool();
        // radar.m_bSaveData = (*rItr)["SaveData"].asBool();
        radar.m_strF0GHz = (*rItr)["F0_GHz"].asString();
        radar.m_strPtDBm = (*rItr)["Pt_dBm"].asString();
        radar.m_strGTDBi = (*rItr)["Gt_dBi"].asString();
        radar.m_strGrDBi = (*rItr)["Gr_dBi"].asString();
        radar.m_strTsK = (*rItr)["Ts_K"].asString();
        radar.m_strFnDB = (*rItr)["Fn_dB"].asString();
        radar.m_strL0DB = (*rItr)["L0_dB"].asString();
        radar.m_strSNRMinDB = (*rItr)["SNR_min_dB"].asString();
        radar.m_strRadarAngle = (*rItr)["radar_angle"].asString();
        radar.m_strRM = (*rItr)["R_m"].asString();
        radar.m_strRcs = (*rItr)["rcs"].asString();
        radar.m_strWeather = (*rItr)["weather"].asString();
        radar.m_strTag = (*rItr)["tag"].asString();
        radar.m_strAnneTag = (*rItr)["anne_tag"].asString();
        radar.m_strHWidth = (*rItr)["hwidth"].asString();
        radar.m_strVWidth = (*rItr)["vwidth"].asString();
        radar.m_strVFov = (*rItr)["vfov"].asString();
        radar.m_strHFov = (*rItr)["hfov"].asString();
        radar.m_strAntennaAnglePath1 = (*rItr)["ANTENNA_ANGLE_path1"].asString();
        radar.m_strAntennaAnglePath2 = (*rItr)["ANTENNA_ANGLE_path2"].asString();

        CSensors::RadarMap::iterator itr = radars.find(radar.m_strID);
        if (itr == radars.end()) {
          radars.insert(std::make_pair(radar.m_strID, radar));
        } else {
          SYSTEM_LOGGER_WARN("radar id repeated!");
        }
      }
    }

    // truthes
    Json::Value& jTruths = jSensors["truths"];
    if (!jTruths.isNull()) {
      CSensors::SensorTruthMap& truths = sensors.SensorTruths();

      Json::ValueIterator tItr = jTruths.begin();
      for (; tItr != jTruths.end(); ++tItr) {
        CSensorTruth truth;
        truth.m_strID = (*tItr)["ID"].asString();
        truth.m_bEnabled = (*tItr)["Enabled"].asBool();
        truth.m_strLocationX = (*tItr)["LocationX"].asString();
        truth.m_strLocationY = (*tItr)["LocationY"].asString();
        truth.m_strLocationZ = (*tItr)["LocationZ"].asString();
        truth.m_strRotationX = (*tItr)["RotationX"].asString();
        truth.m_strRotationY = (*tItr)["RotationY"].asString();
        truth.m_strRotationZ = (*tItr)["RotationZ"].asString();
        truth.m_strInstallSlot = (*tItr)["InstallSlot"].asString();
        truth.m_strDevice = (*tItr)["Device"].asString();
        // truth.m_bSaveData = (*tItr)["SaveData"].asBool();
        truth.m_strVFov = (*tItr)["vfov"].asString();
        truth.m_strHFov = (*tItr)["hfov"].asString();
        truth.m_strDRange = (*tItr)["drange"].asString();

        CSensors::SensorTruthMap::iterator itr = truths.find(truth.m_strID);
        if (itr == truths.end()) {
          truths.insert(std::make_pair(truth.m_strID, truth));
        } else {
          SYSTEM_LOGGER_WARN("truth id repeated!");
        }
      }
    }

    // imus
    Json::Value& jIMUs = jSensors["imus"];
    if (!jIMUs.isNull()) {
      CSensors::IMUMap& imus = sensors.IMUs();

      Json::ValueIterator iItr = jIMUs.begin();
      for (; iItr != jIMUs.end(); ++iItr) {
        CSensorIMU imu;
        imu.m_strID = (*iItr)["ID"].asString();
        imu.m_bEnabled = (*iItr)["Enabled"].asBool();
        imu.m_strLocationX = (*iItr)["LocationX"].asString();
        imu.m_strLocationY = (*iItr)["LocationY"].asString();
        imu.m_strLocationZ = (*iItr)["LocationZ"].asString();
        imu.m_strRotationX = (*iItr)["RotationX"].asString();
        imu.m_strRotationY = (*iItr)["RotationY"].asString();
        imu.m_strRotationZ = (*iItr)["RotationZ"].asString();
        imu.m_strInstallSlot = (*iItr)["InstallSlot"].asString();
        imu.m_strQuaternion = (*iItr)["Quaternion"].asString();

        CSensors::IMUMap::iterator itr = imus.find(imu.m_strID);
        if (itr == imus.end()) {
          imus.insert(std::make_pair(imu.m_strID, imu));
        } else {
          SYSTEM_LOGGER_WARN("imu id repeated!");
        }
      }
    }

    // gpses
    Json::Value& jGPSes = jSensors["gpses"];
    if (!jGPSes.isNull()) {
      CSensors::GPSMap& gpses = sensors.GPSs();

      Json::ValueIterator gItr = jGPSes.begin();
      for (; gItr != jGPSes.end(); ++gItr) {
        CSensorGPS gps;
        gps.m_strID = (*gItr)["ID"].asString();
        gps.m_bEnabled = (*gItr)["Enabled"].asBool();
        gps.m_strLocationX = (*gItr)["LocationX"].asString();
        gps.m_strLocationY = (*gItr)["LocationY"].asString();
        gps.m_strLocationZ = (*gItr)["LocationZ"].asString();
        gps.m_strRotationX = (*gItr)["RotationX"].asString();
        gps.m_strRotationY = (*gItr)["RotationY"].asString();
        gps.m_strRotationZ = (*gItr)["RotationZ"].asString();
        gps.m_strInstallSlot = (*gItr)["InstallSlot"].asString();
        gps.m_strQuaternion = (*gItr)["Quaternion"].asString();

        CSensors::GPSMap::iterator itr = gpses.find(gps.m_strID);
        if (itr == gpses.end()) {
          gpses.insert(std::make_pair(gps.m_strID, gps));
        } else {
          SYSTEM_LOGGER_WARN("gps id repeated!");
        }
      }
    }
  }

  return bIncludeSensorInfo;
}

bool CParserJson::XmlToJsonSensorsInfo(CSensors& sensorInfo, Json::Value& sensors) {
  CSensors* pSensors = &sensorInfo;
  bool bIncludeSensorData = false;

  if (pSensors) {
    // cameras
    Json::Value cameras;
    CSensors::CameraMap mapCameras = pSensors->Cameras();
    CSensors::CameraMap::iterator itr = mapCameras.begin();
    for (; itr != mapCameras.end(); ++itr) {
      Json::Value camera;

      camera["ID"] = itr->second.m_strID;
      camera["Enabled"] = itr->second.m_bEnabled;
      camera["LocationX"] = itr->second.m_strLocationX;
      camera["LocationY"] = itr->second.m_strLocationY;
      camera["LocationZ"] = itr->second.m_strLocationZ;
      camera["RotationX"] = itr->second.m_strRotationX;
      camera["RotationY"] = itr->second.m_strRotationY;
      camera["RotationZ"] = itr->second.m_strRotationZ;
      camera["InstallSlot"] = itr->second.m_strInstallSlot;
      camera["Frequency"] = itr->second.m_strFrequency;
      camera["Device"] = itr->second.m_strDevice;
      camera["InsideParamGroup"] = itr->second.m_nIntrinsicParamType;
      // camera["SaveData"] = itr->second.m_bSaveData;
      camera["DisplayMode"] = itr->second.m_strDisplayMode;
      camera["Res_Horizontal"] = itr->second.m_strResHorizontal;
      camera["Res_Vertical"] = itr->second.m_strResVertical;
      camera["Blur_Intensity"] = itr->second.m_strBlurIntensity;
      camera["MotionBlur_Amount"] = itr->second.m_strMotionBlurAmount;
      camera["Vignette_Intensity"] = itr->second.m_strVignetteIntensity;
      camera["Noise_Intensity"] = itr->second.m_strNoiseIntensity;
      camera["Distortion_Parameters"] = itr->second.m_strDistortionParamters;
      camera["Intrinsic_Matrix"] = itr->second.m_strIntrinsicMatrix;
      camera["FOV_Horizontal"] = itr->second.m_strFOVHorizontal;
      camera["FOV_Vertical"] = itr->second.m_strFOVVertical;
      camera["CCD_Width"] = itr->second.m_strCCDWidth;
      camera["CCD_Height"] = itr->second.m_strCCDHeight;
      camera["Focal_Length"] = itr->second.m_strFocalLength;

      cameras.append(camera);
    }

    if (mapCameras.size() > 0) {
      sensors["cameras"] = cameras;
      bIncludeSensorData = true;
    }

    // lidars
    Json::Value lidars;
    CSensors::TraditionalLidarMap mapLidars = pSensors->TraditionLidars();
    CSensors::TraditionalLidarMap::iterator lItr = mapLidars.begin();
    for (; lItr != mapLidars.end(); ++lItr) {
      Json::Value lidar;

      lidar["ID"] = lItr->second.m_strID;
      lidar["Enabled"] = lItr->second.m_bEnabled;
      lidar["LocationX"] = lItr->second.m_strLocationX;
      lidar["LocationY"] = lItr->second.m_strLocationY;
      lidar["LocationZ"] = lItr->second.m_strLocationZ;
      lidar["RotationX"] = lItr->second.m_strRotationX;
      lidar["RotationY"] = lItr->second.m_strRotationY;
      lidar["RotationZ"] = lItr->second.m_strRotationZ;
      lidar["InstallSlot"] = lItr->second.m_strInstallSlot;
      lidar["Frequency"] = lItr->second.m_strFrequency;
      lidar["Device"] = lItr->second.m_strDevice;
      lidar["DrawPoint"] = lItr->second.m_bDrawPoint;
      lidar["DrawRay"] = lItr->second.m_bDrawRay;
      lidar["Model"] = lItr->second.m_strModel;
      lidar["uChannels"] = lItr->second.m_strUChannels;
      lidar["uRange"] = lItr->second.m_strURange;
      lidar["uHorizontalResolution"] = lItr->second.m_strUHorizontalResolution;
      lidar["uUpperFov"] = lItr->second.m_strUUpperFov;
      lidar["uLowerFov"] = lItr->second.m_strULowerFov;
      // lidar["uSaveData"] = lItr->second.m_bUSaveData;

      lidars.append(lidar);
    }

    if (mapLidars.size() > 0) {
      sensors["lidars"] = lidars;
      bIncludeSensorData = true;
    }

    // radars
    Json::Value radars;
    CSensors::RadarMap mapRadars = pSensors->Radars();
    CSensors::RadarMap::iterator rItr = mapRadars.begin();
    for (; rItr != mapRadars.end(); ++rItr) {
      Json::Value radar;

      radar["ID"] = rItr->second.m_strID;
      radar["Enabled"] = rItr->second.m_bEnabled;
      radar["LocationX"] = rItr->second.m_strLocationX;
      radar["LocationY"] = rItr->second.m_strLocationY;
      radar["LocationZ"] = rItr->second.m_strLocationZ;
      radar["RotationX"] = rItr->second.m_strRotationX;
      radar["RotationY"] = rItr->second.m_strRotationY;
      radar["RotationZ"] = rItr->second.m_strRotationZ;
      radar["InstallSlot"] = rItr->second.m_strInstallSlot;
      radar["Frequency"] = rItr->second.m_strFrequency;
      radar["Device"] = rItr->second.m_strDevice;
      // radar["SaveData"] = rItr->second.m_bSaveData;
      radar["F0_GHz"] = rItr->second.m_strF0GHz;
      radar["Pt_dBm"] = rItr->second.m_strPtDBm;
      radar["Gt_dBi"] = rItr->second.m_strGTDBi;
      radar["Gr_dBi"] = rItr->second.m_strGrDBi;
      radar["Ts_K"] = rItr->second.m_strTsK;
      radar["Fn_dB"] = rItr->second.m_strFnDB;
      radar["L0_dB"] = rItr->second.m_strL0DB;
      radar["SNR_min_dB"] = rItr->second.m_strSNRMinDB;
      radar["radar_angle"] = rItr->second.m_strRadarAngle;
      radar["R_m"] = rItr->second.m_strRM;
      radar["rcs"] = rItr->second.m_strRcs;
      radar["weather"] = rItr->second.m_strWeather;
      radar["tag"] = rItr->second.m_strTag;
      radar["anne_tag"] = rItr->second.m_strAnneTag;
      radar["hwidth"] = rItr->second.m_strHWidth;
      radar["vwidth"] = rItr->second.m_strVWidth;
      radar["vfov"] = rItr->second.m_strVFov;
      radar["hfov"] = rItr->second.m_strHFov;
      radar["ANTENNA_ANGLE_path1"] = rItr->second.m_strAntennaAnglePath1;
      radar["ANTENNA_ANGLE_path2"] = rItr->second.m_strAntennaAnglePath2;

      radars.append(radar);
    }

    if (mapRadars.size() > 0) {
      sensors["radars"] = radars;
      bIncludeSensorData = true;
    }

    // truths
    Json::Value truths;
    CSensors::SensorTruthMap mapTruths = pSensors->SensorTruths();
    CSensors::SensorTruthMap::iterator tItr = mapTruths.begin();
    for (; tItr != mapTruths.end(); ++tItr) {
      Json::Value truth;

      truth["ID"] = tItr->second.m_strID;
      truth["Enabled"] = tItr->second.m_bEnabled;
      truth["LocationX"] = tItr->second.m_strLocationX;
      truth["LocationY"] = tItr->second.m_strLocationY;
      truth["LocationZ"] = tItr->second.m_strLocationZ;
      truth["RotationX"] = tItr->second.m_strRotationX;
      truth["RotationY"] = tItr->second.m_strRotationY;
      truth["RotationZ"] = tItr->second.m_strRotationZ;
      truth["InstallSlot"] = tItr->second.m_strInstallSlot;
      // truth["SaveData"] = tItr->second.m_bSaveData;
      truth["Device"] = tItr->second.m_strDevice;
      truth["vfov"] = tItr->second.m_strVFov;
      truth["hfov"] = tItr->second.m_strHFov;
      truth["drange"] = tItr->second.m_strDRange;

      truths.append(truth);
    }

    if (mapTruths.size() > 0) {
      sensors["truths"] = truths;
      bIncludeSensorData = true;
    }

    // imus
    Json::Value imus;
    CSensors::IMUMap mapIMUs = pSensors->IMUs();
    CSensors::IMUMap::iterator iItr = mapIMUs.begin();
    for (; iItr != mapIMUs.end(); ++iItr) {
      Json::Value imu;

      imu["ID"] = iItr->second.m_strID;
      imu["Enabled"] = iItr->second.m_bEnabled;
      imu["LocationX"] = iItr->second.m_strLocationX;
      imu["LocationY"] = iItr->second.m_strLocationY;
      imu["LocationZ"] = iItr->second.m_strLocationZ;
      imu["RotationX"] = iItr->second.m_strRotationX;
      imu["RotationY"] = iItr->second.m_strRotationY;
      imu["RotationZ"] = iItr->second.m_strRotationZ;
      imu["InstallSlot"] = iItr->second.m_strInstallSlot;
      imu["Quaternion"] = iItr->second.m_strQuaternion;

      imus.append(imu);
    }

    if (mapIMUs.size() > 0) {
      sensors["imus"] = imus;
      bIncludeSensorData = true;
    }

    // gpses
    Json::Value gpses;
    CSensors::GPSMap mapGPSes = pSensors->GPSs();
    CSensors::GPSMap::iterator gItr = mapGPSes.begin();
    for (; gItr != mapGPSes.end(); ++gItr) {
      Json::Value gps;

      gps["ID"] = gItr->second.m_strID;
      gps["Enabled"] = gItr->second.m_bEnabled;
      gps["LocationX"] = gItr->second.m_strLocationX;
      gps["LocationY"] = gItr->second.m_strLocationY;
      gps["LocationZ"] = gItr->second.m_strLocationZ;
      gps["RotationX"] = gItr->second.m_strRotationX;
      gps["RotationY"] = gItr->second.m_strRotationY;
      gps["RotationZ"] = gItr->second.m_strRotationZ;
      gps["InstallSlot"] = gItr->second.m_strInstallSlot;
      gps["Quaternion"] = gItr->second.m_strQuaternion;

      gpses.append(gps);
    }

    if (mapGPSes.size() > 0) {
      sensors["gpses"] = gpses;
      bIncludeSensorData = true;
    }
  }

  return bIncludeSensorData;
}
