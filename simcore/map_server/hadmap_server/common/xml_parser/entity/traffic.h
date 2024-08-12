/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <map>
#include <string>
#include <vector>

#include "common/xml_parser/entity/merge.h"
#include "common/xml_parser/entity/obstacle.h"
#include "common/xml_parser/entity/pedestrian.h"
#include "common/xml_parser/entity/pedestrianV2.h"
#include "common/xml_parser/entity/route.h"
#include "common/xml_parser/entity/scene_event.h"
#include "common/xml_parser/entity/signlight.h"
#include "common/xml_parser/entity/traffic_flow.h"
#include "common/xml_parser/entity/vehicle.h"
#include "common/xml_parser/entity/velocity.h"
#include "xml_parser/entity/acceleration.h"

#ifndef __HADMAP_SERVER__
#  include "engine/scene/scene.h"
#endif

class TiXmlElement;
class CTrafficParam;

class CTraffic {
 public:
  typedef std::map<std::string, CRoute> RouteMap;
  typedef std::map<std::string, CAcceleration> AccelerationMap;
  typedef std::map<std::string, CVelocity> VelocityMap;
  typedef std::map<std::string, CPedestrianVelocity> PedestrianVelocityMap;
  typedef std::map<std::string, CMerge> MergeMap;
  typedef std::map<std::string, CVehicle> VehicleMap;
  typedef std::map<std::string, CObstacle> ObstacleMap;
  typedef std::map<std::string, CPedestrian> PedestrianMap;
  typedef std::map<std::string, CPedestrianV2> PedestrianV2Map;
  typedef std::map<std::string, CSignLight> SignLightMap;
  typedef std::map<std::string, SceneEvent> SceneEventMap;

 public:
  CTraffic();
  explicit CTraffic(CTrafficParam& tp);
  virtual int Parse(const char* strFile);
  virtual int Save(const char* strFile);

 public:
  // for scene

#ifndef __HADMAP_SERVER__
  int SetRoutes(CScene::Routels& routes);
  int SetAccs(CScene::Accls& accs);
  int SetMerges(CScene::Mergels& merges);
  // int SetVelocities(CScene::& velocities);
  int SetVehicles(CScene::Vehiclels& vehicles);
  int SetObstacles(CScene::Obstaclels& obstacles);
  int SetPedestrians(CScene::Pedestrianls& pedestrians);
  int SetSignlights(CScene::Signlightls& signlights);
#endif
  int TrafficFileType() { return m_nTrafficFileType; }
  void SetTrafficFileType(int nType) { m_nTrafficFileType = nType; }

  int SetRandomSeed(int64_t nRandomSeed);
  int SetAggress(double dAggress);

  RouteMap& Routes() { return m_mapRoutes; }
  AccelerationMap& Accs() { return m_mapAccs; }
  MergeMap& Merges() { return m_mapMerges; }
  VelocityMap& Velocities() { return m_mapVelocities; }
  PedestrianVelocityMap& PedestrianVelocities() { return m_mapPedestrianVelocities; }
  VehicleMap& Vehicles() { return m_mapVehicles; }
  ObstacleMap& Obstacles() { return m_mapObstacles; }
  SceneEventMap& SceneEvents() { return m_mapSceneEvents; }
  PedestrianMap& Pedestrians() { return m_mapPedestrians; }
  PedestrianV2Map& PedestriansV2() { return m_mapPedestriansV2; }
  SignLightMap& Signlights() { return m_mapSignlights; }
  int64_t RandomSeed() { return m_RandomSeed; }
  double Aggress() { return m_dAggress; }
  int FileType() { return m_nTrafficFileType; }

  AccelerationMap* AccsMap() { return &m_mapAccs; }
  MergeMap* MergesMap() { return &m_mapMerges; }
  VelocityMap* VelocitiesMap() { return &m_mapVelocities; }
  VehicleMap* VehiclesMap() { return &m_mapVehicles; }

  int GetDefaultAccID();
  std::string GetDefaultAccStrID();
  virtual int GetOneAccID();
  std::string GetOneAccStrID();
  int GetDefaultMergeID();
  std::string GetDefaultMergeStrID();
  virtual int GetOneMergeID();
  std::string GetOneMergeStrID();

  CTrafficFlow& TrafficFlow() { return m_trafficFlow; }

 public:
  int ParseData(TiXmlElement* RoutesElem, int64_t& nRandomSeed, double& dAggress);

  int ParseRoutes(TiXmlElement* RoutesElem, std::map<std::string, CRoute>& mapRoutes);
  static int ParseOneRoute(TiXmlElement* RoutElem, CRoute& r);

  int ParseAccs(TiXmlElement* AccsElem, std::map<std::string, CAcceleration>& mapAccs);
  int ParseOneAcc(TiXmlElement* AccElem, CAcceleration& a);

  int ParseAccsEvent(TiXmlElement* AccsEventElem, std::map<std::string, CAcceleration>& mapAccs);
  int ParseOneAccEvent(TiXmlElement* AccElem, CAcceleration& a);

  int ParseMerges(TiXmlElement* MergesElem, std::map<std::string, CMerge>& mapMerges);
  int ParseOneMerge(TiXmlElement* MergeElem, CMerge& m);

  int ParseSceneEvents(TiXmlElement* SceneEventsElem, std::map<std::string, SceneEvent>& mapSceneEvent);
  int ParseOneSceneEvent(TiXmlElement* SceneEventElem, SceneEvent& se, std::string version);

  int ParseMergesEvent(TiXmlElement* MergesEventElem, std::map<std::string, CMerge>& mapMerges);
  int ParseOneMergeEvent(TiXmlElement* MergeElem, CMerge& m);

  int ParseVelocities(TiXmlElement* VelocitiesElem, std::map<std::string, CVelocity>& mapVelocities);
  int ParseOneVelocity(TiXmlElement* VelocityElem, CVelocity& v);

  int ParseVelocitiesEvent(TiXmlElement* VelocitiesEventElem, std::map<std::string, CVelocity>& mapVelocities);
  int ParseOneVelocityEvent(TiXmlElement* VelocityElem, CVelocity& v);

  int ParseVehicles(TiXmlElement* VehiclesElem, std::map<std::string, CVehicle>& mapVehicles);
  virtual int ParseOneVehicle(TiXmlElement* VehcileElem, CVehicle& v);

  int ParseObstacles(TiXmlElement* ObstaclesElem, std::map<std::string, CObstacle>& mapObstacles);
  virtual int ParseOneObstacle(TiXmlElement* ObstacleElem, CObstacle& o);

  int ParsePedestrians(TiXmlElement* PedestriansElem, PedestrianMap& mapPedestrians);
  virtual int ParseOnePedestrian(TiXmlElement* PedestrianElem, CPedestrian& p);

  int ParsePedestriansEvent(TiXmlElement* PedestriansEventElem, PedestrianMap& mapPedestrians);
  virtual int ParseOnePedestrianEvent(TiXmlElement* PedestrianEventElem, CPedestrianEvent& p);

  int ParsePedestriansV2(TiXmlElement* PedestriansElem, PedestrianV2Map& mapPedestrians);
  virtual int ParseOnePedestrianV2(TiXmlElement* PedestrianElem, CPedestrianV2& p);

  int ParsePedestrianVelocities(TiXmlElement* VelocitiesElem,
                                std::map<std::string, CPedestrianVelocity>& mapVelocities);
  int ParseOnePedestrianTimeVelocity(TiXmlElement* VelocitiesElem, CPedestrianVelocity& v);
  int ParseOnePedestrianEventVelocity(TiXmlElement* VelocitiesElem, CPedestrianVelocity& v);

  int ParseSignlights(TiXmlElement* SignlightsElem, SignLightMap& mapSignlight);
  virtual int ParseOneSignlight(TiXmlElement* SignlightElem, CSignLight& l);

  int SaveData(TiXmlElement* DataElem, int64_t nRandomSeed, double dAggress);

  int SaveRoutes(TiXmlElement* RoutesElem, std::map<std::string, CRoute>& mapRoutes);
  static int SaveOneRoute(TiXmlElement* RoutElem, CRoute& r);

  int SaveAccs(TiXmlElement* AccsElem, std::map<std::string, CAcceleration>& mapAccs);
  int SaveOneAcc(TiXmlElement* AccElem, CAcceleration& a);

  int SaveAccsEvent(TiXmlElement* AccsEventElem, std::map<std::string, CAcceleration>& mapAccs);
  int SaveOneAccEvent(TiXmlElement* AccEventElem, CAcceleration& a);

  int SaveMerges(TiXmlElement* MergesElem, std::map<std::string, CMerge>& mapMerges);
  int SaveOneMerge(TiXmlElement* MergeElem, CMerge& m);

  int SaveMergesEvent(TiXmlElement* MergesEventElem, std::map<std::string, CMerge>& mapMerges);
  int SaveOneMergeEvent(TiXmlElement* MergeEventElem, CMerge& m);

  int SaveVelocities(TiXmlElement* VelocitiesElem, std::map<std::string, CVelocity>& mapVelocities);
  int SaveOneVelocity(TiXmlElement* VelocityElem, CVelocity& v);

  int SaveVelocitiesEvent(TiXmlElement* VelocitiesEventElem, std::map<std::string, CVelocity>& mapVelocities);
  int SaveOneVelocityEvent(TiXmlElement* VelocityEventElem, CVelocity& v);

  int SaveVehicles(TiXmlElement* VehiclesElem, std::map<std::string, CVehicle>& mapVehicles);
  int SaveOneVehicle(TiXmlElement* VehcileElem, CVehicle& v);

  int SaveObstacles(TiXmlElement* ObstaclesElem, std::map<std::string, CObstacle>& mapObstacles);
  int SaveOneObstacle(TiXmlElement* ObstacleElem, CObstacle& o);

  int SavePedestrians(TiXmlElement* PedestriansElem, PedestrianMap& mapPedestrians);
  int SaveOnePedestrian(TiXmlElement* PedestrianElem, CPedestrian& p);

  int SavePedestriansEvent(TiXmlElement* PedestrianEventsElem, PedestrianMap& mapPedestrians);
  int SaveOnePedestrianEvent(TiXmlElement* PedestrianEventElem, CPedestrian& p);

  int SavePedestriansV2(TiXmlElement* PedestriansElem, PedestrianV2Map& mapPedestrians);
  int SaveOnePedestrianV2(TiXmlElement* PedestriansElem, CPedestrianV2& p);

  int SavePedestrianVelocities(TiXmlElement* VelocitiesElem, std::map<std::string, CPedestrianVelocity>& mapVelocities);
  int SaveOnePedestrianVelocity(TiXmlElement* VelocityElem, CPedestrianVelocity& v);

  int SaveSignlights(TiXmlElement* SignlightsElem, SignLightMap& mapSignlight);
  int SaveOneSignlight(TiXmlElement* SignlightElem, CSignLight& l);

  int SaveSceneEvents(TiXmlElement* SceneEventsElem, SceneEventMap& mapSceneEvent);
  int SaveOneSceneEvent(TiXmlElement* SceneEventElem, SceneEvent& se);

  int ParseTrafficFlow(TiXmlElement* TrafficFlowElem, CTrafficFlow&);
  int SaveTrafficFlow(TiXmlElement* TrafficFlowElem, CTrafficFlow&);

  RouteMap m_mapRoutes;
  AccelerationMap m_mapAccs;
  MergeMap m_mapMerges;
  VelocityMap m_mapVelocities;
  PedestrianVelocityMap m_mapPedestrianVelocities;
  VehicleMap m_mapVehicles;
  ObstacleMap m_mapObstacles;
  PedestrianMap m_mapPedestrians;
  PedestrianV2Map m_mapPedestriansV2;
  SignLightMap m_mapSignlights;

  SceneEventMap m_mapSceneEvents;
  CTrafficFlow m_trafficFlow;
  // 1. traffic 代表交通车仿真
  // 2. crowd 代表只有AI行人仿真
  // 3. mixed 代表混合仿真
  std::string m_strMode = "traffic";
  //
  bool isConfigTraffic = false;
  std::string m_ActivePlan;
  std::map<std::string, std::map<std::string, std::string>> m_mapJunction2Road;

 protected:
  int64_t m_RandomSeed;
  double m_dAggress;
  int m_nTrafficFileType;
};
