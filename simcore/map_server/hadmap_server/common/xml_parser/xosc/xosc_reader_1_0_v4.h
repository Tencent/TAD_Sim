/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include "openscenario.hpp"
#include "common/xml_parser/entity/confload.h"
#include "common/xml_parser/entity/planner.h"
#include "common/xml_parser/entity/traffic.h"
#include "common/xml_parser/xosc/xosc_base_1_0.h"

using std::map;
using std::string;
using std::vector;

struct sTagSimuTraffic;

class XOSCReader_1_0_v4 : public XOSCBase_1_0 {
 public:
  XOSCReader_1_0_v4();
  ~XOSCReader_1_0_v4();
  // parse xosc
  int ParseXOSC(const char* strFilename, sTagSimuTraffic& infos);
  // copy xosc to new filepath
  int CopyXOSC(const char* srcFileName, const char* dstFileName);
  // get xosc fileNames
  int getXoscFilenames(const std::string& dir, std::vector<std::string>& filenames);
  //  parse ManeuverCatalog
  int ParseManeuversCatalogXosc(const std::vector<std::string> maneuverName, std::vector<osc::Maneuver>& maneuver);
  //  parse EnvironmentCatalog
  int ParseEnvironmentCatalogXosc(const std::string environmentName, osc::Environment& environment);
  //  parse RouteCatalog
  int ParseRouteCatalogXosc(const std::string environmentName, osc::Route& route);
  //  parse TrajectoryCatalog
  int ParseTrajectoryCataloggXosc(const std::string environmentName, osc::Trajectory& tra);

 protected:
  osc::OpenScenario pScene;

  void Reset();

  void LoadConf(const char* fileName);

  bool CheckXosc(const char* xsdPath, const char* xoscPath);

  std::string MatchName(std::string type, std::string name);

  //  get catalog reference
  void GetCatalogReference(osc::ScenarioObject& reference, sTagSimuTraffic& infos);

  int ParseStartTrigger(osc::Trigger& trigger, string& strEntityRef, int& nType, string& strTrigerValue,
                        string& strDistanceType, string& strAlongRoute);

  int ParsePrivateAction(osc::PrivateAction& pa, sTagSimuTraffic& infos, string& strEntityRef, string& strvelocity,
                         string& strAccValue, bool& isOver_V, string& strAccOverValue, string& strMergeValue,
                         string& strMergeOffset, string& strMergeOverTime, string& strOrientation,
                         SceneEvent& singleSceneEvent, std::string actionName, bool isInit = true);
  int ParseStartTrigger(osc::Trigger& trigger, int& nType, string& strTrigerValue, string& strDistanceType,
                        string& strAlongRoute, SceneEvent& singleSceneEvent, string& ManeuverName);
  // check trigger type
  int CheckTriggerType(SceneEvent& singleSceneEvent, std::string entryname);

  int ParseAction(std::string entryName, osc::Action& st, SceneEvent& singleSceneEvent, sTagSimuTraffic& infos);

  void GetCommandValue(std::string strText, std::string& strSubType, std::string& strValue, std::string& strOffset);

  //
  int ParseParameterDeclaration(osc::ParameterDeclarations& pas);
  int ParseRoadNetWork(osc::RoadNetwork& rn, sTagSimuTraffic& infos);
  int ParseRoadNetWorkCloud(osc::RoadNetwork& rn, const char* strMapename, sTagSimuTraffic& infos);
  // int ParseCatalogLocations(osc::CatalogLocations& cls, sTagSimuTraffic& infos);
  int ParseEntities(osc::Entities& entities, sTagSimuTraffic& infos);
  int ParseStoryboard(osc::Storyboard& sb, sTagSimuTraffic& infos);

  //
  int ParseOneEgoInfo(osc::ScenarioObject& obj, std::string catalogDir);
  // parse header
  void ParseFileHeader(osc::FileHeader& fht, sTagSimuTraffic& infos);

  void FindParameterDeclarationsValude(string& str);

  bool MakeSure_P(string& strType);  // 判断是否为行人类型 对应sim的

  void ReadLongitudinalSpeedAbsolute(osc::PrivateAction& pa, string& strDynamicsShape, string& strValue,
                                     string& strDynamicsDimension, string& strSpeedValue);
  void ReadLongitudinalSpeedRelative(osc::PrivateAction& pa, string& strDynamicsShape, string& strValue,
                                     string& strDynamicsDimension, string& strSpeedValue);

  // parse Action
  void ParseEnvironmentAction(osc::EnvironmentAction& environment, sTagSimuTraffic& infos);

  void ReadLateralAction_LaneChangeAction(osc::PrivateAction& pa, string& strDynamicsShape, string& strValue,
                                          string& strDynamicsDimension, string& strTargetLane, string& strOffset);
  void AddIniSpeed(string& strEntityRef, sTagSimuTraffic& infos, string& strSpeedV);

  // TeleportAction->WorldPosition
  void ReadTeleportAction_WorldPosition(osc::PrivateAction& pa, double& dX, double& dY, double& dZ, double& dH);
  // TeleportAction->RelativeWorldPosition
  void ReadTeleActionRelWorldPos(osc::PrivateAction& pa, double& dX, double& dY, double& dZ, double& dH);
  // TeleportAction->LanePosition
  void ReadTeleportAction_LanePosition(osc::PrivateAction& pa, hadmap::roadpkid& iRoadid, hadmap::lanepkid& iLaneid,
                                       double& dS, double& dOffset, double& yaw);
  // TeleportAction->RelativeLanePosition
  void ReadTeleActionRelLanePos(osc::PrivateAction& pa, hadmap::roadpkid& iRoadid, hadmap::lanepkid& iLaneid,
                                double& dS,
                                double& dOffset);
  // TeleportAction->RoadPosition
  void ReadTeleportAction_RoadPosition(osc::PrivateAction& pa, hadmap::roadpkid& iRoadid, double& dS, double& dT);
  // TeleportAction->RelativeRoadPosition
  void ReadTeleActionRelRoadPos(osc::PrivateAction& pa, hadmap::roadpkid& iRoadid, double& dS, double& dT);

  void IniWordPos(string& strEntityRef, sTagSimuTraffic& infos, double& dX, double& dY, double& dZ, double& dH);

  void IniLanePos(string& strEntityRef, sTagSimuTraffic& infos, hadmap::roadpkid& iRoadid, hadmap::lanepkid& iLaneid,
                  double& dS, double& dT, double yaw = -9999.0);
  void IniRoadPos(string& strEntityRef, sTagSimuTraffic& infos, hadmap::roadpkid& iRoadid, double& dS, double& dT);

  void AddIniPos(string& strEntityRef, CRoute& TempRoute, sTagSimuTraffic& infos, sTagQueryInfoResult& result,
                 double& dX, double& dY, double& dZ, double& dH, bool isUse = false);

  void ReadRoutingAction(osc::PrivateAction& pa, RoutingActionType type, vector<sPathPoint>& Vpoints,
                         string& strOrientation);
  void AddRoute(string& strEntityRef, sTagSimuTraffic& infos, vector<sPathPoint>& Vpoints);

  double formatRad2Deg(double deg);

  double formatDeg2Rad(double deg);

  std::string formatOptRule(std::string rule);

  std::string getEntryRef(const std::string name);

  bool CompareString(std::string str1, std::string str2);

  void AddAccOrMerge(string& strEntityRef, sTagSimuTraffic& infos, string& strAccValue, bool& isOver_V,
                     string& strAccOverValue, string& strMergeValue, string& strMergeOffset, string& strMergeOverTime,
                     int& nType, string& strTrigerValue, string& strDistanceType, string& strAlongRoute);
  void AddVelocity(string& strEntityRef, sTagSimuTraffic& infos, string& strTargetValue, int& nType,
                   string& strTrigerValue, string& strDistanceType, string& strAlongRoute);

  void AddPedestrianVelocity(string& strEntityRef, sTagSimuTraffic& infos, string& strTargetValue, int& nType,
                             string& strTrigerValue, string& strDistanceType, string& strOrientation,
                             string& strAlongRoute);

  void StrName2ID(sTagSimuTraffic& infos);

  void FlushData(sTagSimuTraffic& infos);

  int m_nVehiclIDGeneartor;
  int m_nObstacleIDGenerator;
  int m_nPedestrianIDGenerator;
  int m_nRouteIDGenerator;
  int m_nAccIDGeneator;
  int m_nMergeIDGenerator;
  int m_nPedVIDGenerator;
  int m_nVIDGenerator;
  int m_nEgoGenerator = 1;
  // parameter declaration maps
  std::map<string, Type3_Value> m_ParameterDeclarations;
  CTraffic::VehicleMap m_mapVehicles;
  CTraffic::ObstacleMap m_mapObstacles;
  CTraffic::PedestrianV2Map m_mapPedestrians;
  CTraffic::RouteMap m_mapRoutes;
  CTraffic::AccelerationMap m_mapAccs;  // 弃用
  CTraffic::MergeMap m_mapMerges;       // 弃用
  CTraffic::PedestrianVelocityMap m_mapPedestrianVelocitys;
  CTraffic::VelocityMap m_mapVelocitys;  // 弃用
  CTraffic::SignLightMap m_mapTrafficSingals;
  CTraffic::SceneEventMap m_mapSceneEvents;
  std::map<std::string, CTraffic::SceneEventMap> m_mapSimPlanerEvents;  // add 2022.07

  CRoute m_tmpRoute;
  struct IniPos {
    double x, y, z, o;
    int rid, secid, lid;
    double s, t;
  };
  // 存储每个对象的初始位置，由于有的车是相对位置，需要依靠此计算
  std::map<string, IniPos> m_mapIniPos;

  std::map<string, string> m_mapXoscTrajectoryTracking;
  //
  std::map<std::string, CEgoInfo> m_egoInfos;
  // filePath
  std::string m_filePath;
  // catalog reference
  CatalogPath m_catalogPath;
  //
  bool isNeedModify = false;
  // author
  XOSC_VERSION m_version;
  bool m_isRouteAction;
  //
  std::shared_ptr<CConfLoad> m_conLoadPtr = nullptr;
  //
  std::map<std::string, std::string> m_name2standard;
  int npedestrian;
  int nmiscobject;
  int n;
};
