/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/traffic.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <utility>

#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include "common/engine/constant.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/log/xml_logger.h"
#include "common/xml_parser/entity/parser_tools.h"
#include "xml_parser/entity/traffic_param.h"

const double EPSILLO = 0.000000001;
const double AGGRESS_RANGE_MIN = -0.00000000001;
const double AGGRESS_RANGE_MAX = 1.00000000001;

CTraffic::CTraffic() {
  m_RandomSeed = -1;
  m_dAggress = -1;
  m_nTrafficFileType = TRAFFIC_TYPE_XML;
}

CTraffic::CTraffic(CTrafficParam& tp) {
  m_mapRoutes = tp.m_mapRoutes;
  m_mapAccs = tp.m_mapAccs;
  m_mapMerges = tp.m_mapMerges;
  m_mapVelocities = tp.m_mapVelocities;
  m_mapPedestrianVelocities = tp.m_mapPedestrianVelocities;
  m_mapVehicles = tp.m_mapVehicles;
  m_mapObstacles = tp.m_mapObstacles;
  m_mapPedestrians = tp.m_mapPedestrians;
  m_mapPedestriansV2 = tp.m_mapPedestriansV2;
  m_mapSignlights = tp.m_mapSignlights;

  m_RandomSeed = tp.m_RandomSeed;
  m_dAggress = tp.m_dAggress;

  m_mapSceneEvents = tp.m_mapSceneEvents;
}

#ifndef __HADMAP_SERVER__
int CTraffic::SetRoutes(CScene::Routels& routes) {
  m_mapRoutes.clear();
  CScene::Routels::iterator rlItr = routes.begin();
  for (; rlItr != routes.end(); ++rlItr) {
    m_mapRoutes.insert(std::make_pair((*rlItr)->m_strID, (*(*rlItr))));
  }

  return 0;
}

int CTraffic::SetAccs(CScene::Accls& accs) {
  m_mapAccs.clear();
  CScene::Accls::iterator alItr = accs.begin();
  for (; alItr != accs.end(); ++alItr) {
    m_mapAccs.insert(std::make_pair((*alItr)->m_strID, (*(*alItr))));
  }

  return 0;
}

int CTraffic::SetMerges(CScene::Mergels& merges) {
  m_mapMerges.clear();
  CScene::Mergels::iterator mlItr = merges.begin();
  for (; mlItr != merges.end(); ++mlItr) {
    m_mapMerges.insert(std::make_pair((*mlItr)->m_strID, (*(*mlItr))));
  }

  return 0;
}

int CTraffic::SetVehicles(CScene::Vehiclels& vehicles) {
  m_mapVehicles.clear();
  CScene::Vehiclels::iterator vlItr = vehicles.begin();
  for (; vlItr != vehicles.end(); ++vlItr) {
    m_mapVehicles.insert(std::make_pair((*vlItr)->m_strID, (*(*vlItr))));
  }

  return 0;
}

int CTraffic::SetObstacles(CScene::Obstaclels& obstacles) {
  m_mapObstacles.clear();
  CScene::Obstaclels::iterator vlItr = obstacles.begin();
  for (; vlItr != obstacles.end(); ++vlItr) {
    m_mapObstacles.insert(std::make_pair((*vlItr)->m_strID, (*(*vlItr))));
  }

  return 0;
}

int CTraffic::SetPedestrians(CScene::Pedestrianls& pedestrians) {
  m_mapPedestrians.clear();
  CScene::Pedestrianls::iterator plItr = pedestrians.begin();
  for (; plItr != pedestrians.end(); ++plItr) {
    m_mapPedestrians.insert(std::make_pair((*plItr)->m_strID, (*(*plItr))));
  }

  return 0;
}

int CTraffic::SetSignlights(CScene::Signlightls& signlights) {
  m_mapSignlights.clear();
  CScene::Signlightls::iterator slItr = signlights.begin();
  for (; slItr != signlights.end(); ++slItr) {
    m_mapSignlights.insert(std::make_pair((*slItr)->m_strID, (*(*slItr))));
  }

  return 0;
}
#endif
int CTraffic::SetRandomSeed(int64_t nRandomSeed) {
  m_RandomSeed = nRandomSeed;

  return 0;
}

int CTraffic::SetAggress(double dAggress) {
  m_dAggress = dAggress;

  return 0;
}

int CTraffic::GetDefaultAccID() {
  int nDefaultAccID = -1;

  AccelerationMap::iterator itr = m_mapAccs.begin();
  nDefaultAccID = itr->second.m_ID;
  for (; itr != m_mapAccs.end(); ++itr) {
    if (itr->second.m_ID < nDefaultAccID) {
      nDefaultAccID = itr->second.m_ID;
    }
  }

  return nDefaultAccID;
}

std::string CTraffic::GetDefaultAccStrID() {
  int nDefaultAccID = GetDefaultAccID();

  return std::to_string(nDefaultAccID);
}

int CTraffic::GetOneAccID() {
  int nOneAccID = -1;

  AccelerationMap::iterator itr = m_mapAccs.begin();
  nOneAccID = itr->second.m_ID;
  for (; itr != m_mapAccs.end(); ++itr) {
    if (itr->second.m_ID > nOneAccID) {
      nOneAccID = itr->second.m_ID;
    }
  }

  nOneAccID++;

  return nOneAccID;
}

std::string CTraffic::GetOneAccStrID() {
  int nOneAccID = GetOneAccID();

  return std::to_string(nOneAccID);
}

int CTraffic::GetDefaultMergeID() {
  int nDefaultMergeID = -1;

  MergeMap::iterator itr = m_mapMerges.begin();
  nDefaultMergeID = itr->second.m_ID;
  for (; itr != m_mapMerges.end(); ++itr) {
    if (itr->second.m_ID < nDefaultMergeID) {
      nDefaultMergeID = itr->second.m_ID;
    }
  }

  return nDefaultMergeID;
}

std::string CTraffic::GetDefaultMergeStrID() {
  int nDefaultMergeID = GetDefaultMergeID();

  return std::to_string(nDefaultMergeID);
}

int CTraffic::GetOneMergeID() {
  int nOneMergeID = -1;

  MergeMap::iterator itr = m_mapMerges.begin();
  nOneMergeID = itr->second.m_ID;
  for (; itr != m_mapMerges.end(); ++itr) {
    if (itr->second.m_ID < nOneMergeID) {
      nOneMergeID = itr->second.m_ID;
    }
  }

  nOneMergeID++;

  return nOneMergeID;
}

std::string CTraffic::GetOneMergeStrID() {
  int nOneMergeID = GetOneMergeID();

  return std::to_string(nOneMergeID);
}

int CTraffic::Parse(const char* strFile) {
  if (!strFile) return -1;

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strFile);

  if (!bRet) return -1;

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  std::string strName = xmlRoot->Value();

  if (!boost::algorithm::iequals(strName, "traffic")) return -1;

  if (xmlRoot->Attribute("mode")) {
    m_strMode = xmlRoot->Attribute("mode");
  }

  TiXmlElement* elemData = xmlRoot->FirstChildElement("data");
  if (elemData) {
    int nRet = ParseData(elemData, m_RandomSeed, m_dAggress);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemRoutes = xmlRoot->FirstChildElement("routes");
  if (elemRoutes) {
    int nRet = ParseRoutes(elemRoutes, m_mapRoutes);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemAccs = xmlRoot->FirstChildElement("accs");
  if (elemAccs) {
    int nRet = ParseAccs(elemAccs, m_mapAccs);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemAccsEvent = xmlRoot->FirstChildElement("accs_event");
  if (elemAccsEvent) {
    int nRet = ParseAccsEvent(elemAccsEvent, m_mapAccs);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemMerges = xmlRoot->FirstChildElement("merges");
  if (elemMerges) {
    int nRet = ParseMerges(elemMerges, m_mapMerges);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemMergesEvent = xmlRoot->FirstChildElement("merges_event");
  if (elemMergesEvent) {
    int nRet = ParseMergesEvent(elemMergesEvent, m_mapMerges);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemVelocities = xmlRoot->FirstChildElement("velocities");
  if (elemVelocities) {
    int nRet = ParseVelocities(elemVelocities, m_mapVelocities);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemVelocitiesEvent = xmlRoot->FirstChildElement("velocities_event");
  if (elemVelocitiesEvent) {
    int nRet = ParseVelocitiesEvent(elemVelocitiesEvent, m_mapVelocities);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemVehicles = xmlRoot->FirstChildElement("vehicles");
  if (elemVehicles) {
    int nRet = ParseVehicles(elemVehicles, m_mapVehicles);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemObstacles = xmlRoot->FirstChildElement("obstacles");
  if (elemObstacles) {
    int nRet = ParseObstacles(elemObstacles, m_mapObstacles);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemPedestrians = xmlRoot->FirstChildElement("pedestrians");
  if (elemPedestrians) {
    int nRet = ParsePedestriansV2(elemPedestrians, m_mapPedestriansV2);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemPedestriansEvent = xmlRoot->FirstChildElement("pedestrians_event");
  if (elemPedestriansEvent) {
    int nRet = ParsePedestriansEvent(elemPedestriansEvent, m_mapPedestrians);
    if (nRet != 0) {
      return -1;
    }

    nRet = ParsePedestrianVelocities(elemPedestriansEvent, m_mapPedestrianVelocities);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemSignlights = xmlRoot->FirstChildElement("signlights");
  if (elemSignlights) {
    // check old scene
    if (elemSignlights->Attribute("activePlan")) {
      m_ActivePlan = elemSignlights->Attribute("activePlan");
      isConfigTraffic = true;
    }
    int nRet = ParseSignlights(elemSignlights, m_mapSignlights);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemSceneEvents = xmlRoot->FirstChildElement("scene_event");
  if (elemSceneEvents) {
    int nRet = ParseSceneEvents(elemSceneEvents, m_mapSceneEvents);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemTrafficFlow = xmlRoot->FirstChildElement("trafficFlow");
  if (elemTrafficFlow) {
    m_trafficFlow.FilePath() = strFile;
    int nRet = ParseTrafficFlow(elemTrafficFlow, m_trafficFlow);
    if (nRet != 0) {
      return -1;
    }
  }

  return 0;
}

int CTraffic::ParseData(TiXmlElement* elemData, int64_t& nRandomSeed, double& dAggress) {
  if (!elemData) return -1;

  const char* p = elemData->Attribute("randomseed");
  if (p) {
    nRandomSeed = atoi(p);
  }

  p = elemData->Attribute("aggress");
  if (p) {
    dAggress = atof(p);
  }

  return 0;
}

int CTraffic::ParseRoutes(TiXmlElement* RoutesElem, std::map<std::string, CRoute>& mapRoutes) {
  if (!RoutesElem) return -1;

  mapRoutes.clear();

  TiXmlElement* elemRoute = RoutesElem->FirstChildElement("route");
  while (elemRoute) {
    CRoute r;

    int nRet = ParseOneRoute(elemRoute, r);
    if (nRet) {
      std::cout << "route format error!" << std::endl;
      // assert(false);

      elemRoute = elemRoute->NextSiblingElement("route");
      continue;
    }

    mapRoutes.insert(std::make_pair(r.m_strID, r));

    elemRoute = elemRoute->NextSiblingElement("route");
  }

  return 0;
}

int CTraffic::ParseOneRoute(TiXmlElement* elemRoute, CRoute& r) {
  if (!elemRoute) return -1;

  const char* p = elemRoute->Attribute("id");
  if (p) r.m_strID = p;

  p = elemRoute->Attribute("type");
  if (p) r.m_strType = p;

  p = elemRoute->Attribute("start");
  if (p) {
    r.m_strStart = p;
    std::vector<std::string> strs;
    CParseTools::SplitString(r.m_strStart, strs, ",");
    assert(strs.size() == 2 || strs.size() == 3);
    if (strs.size() != 2 && strs.size() != 3) {
      SYSTEM_LOGGER_ERROR("traffic file start_end route error!");
    }

    r.m_strStartLon = strs[0];
    r.m_strStartLat = strs[1];
  }

  p = elemRoute->Attribute("end");
  if (p) {
    r.m_strEnd = p;
    if (r.m_strEnd.size() > 0) {
      std::vector<std::string> strs;
      CParseTools::SplitString(r.m_strEnd, strs, ",");
      assert(strs.size() == 2 || strs.size() == 3);
      if (strs.size() != 2 && strs.size() != 3) {
        SYSTEM_LOGGER_ERROR("traffic file start_end route error!");
      }

      r.m_strEndLon = strs[0];
      r.m_strEndLat = strs[1];
    }
  }

  /*
  p = elemRoute->Attribute("mid");
  if (p)
  {
          r.m_strMid = p;
          if (r.m_strMid.size() > 0)
          {
                  std::vector<std::string> strs;
                  CParseTools::SplitString(r.m_strMid, strs, ",");
                  assert(strs.size() == 2);
                  if (strs.size() != 2)
                  {
                          SYSTEM_LOGGER_ERROR("traffic file start_end route error!");
                  }

                  r.m_strMidLon = strs[0];
                  r.m_strMidLat = strs[1];
          }
  }
  */
  p = elemRoute->Attribute("mid");
  if (p) {
    r.m_strMids = p;
  }

  p = elemRoute->Attribute("controlPath");
  if (p) {
    r.m_strControlPath = p;
    r.SegmentString();
  }

  p = elemRoute->Attribute("roadID");
  if (p) r.m_strRoadID = p;

  p = elemRoute->Attribute("sectionID");
  if (p) r.m_strSectionID = p;

  p = elemRoute->Attribute("info");
  if (p) r.m_strInfo = p;

  return 0;
}

int CTraffic::ParseAccs(TiXmlElement* AccsElem, std::map<std::string, CAcceleration>& mapAccs) {
  if (!AccsElem) return -1;

  mapAccs.clear();

  TiXmlElement* elemAcc = AccsElem->FirstChildElement("acc");
  while (elemAcc) {
    CAcceleration a;

    int nRet = ParseOneAcc(elemAcc, a);
    if (nRet) {
      std::cout << "acc format error!" << std::endl;
      assert(false);

      elemAcc = elemAcc->NextSiblingElement("acc");
      continue;
    }

    mapAccs.insert(std::make_pair(a.m_strID, a));

    elemAcc = elemAcc->NextSiblingElement("acc");
  }

  return 0;
}

int CTraffic::ParseOneAcc(TiXmlElement* elemAcc, CAcceleration& a) {
  if (!elemAcc) return -1;

  const char* p = elemAcc->Attribute("id");
  if (p) a.m_strID = p;

  p = elemAcc->Attribute("profile");
  if (p) {
    a.m_strProfilesTime = p;
  }

  p = elemAcc->Attribute("endCondition");
  if (p) {
    a.m_strEndConditionTime = p;
  }

  return 0;
}

int CTraffic::ParseAccsEvent(TiXmlElement* AccsEventElem, std::map<std::string, CAcceleration>& mapAccs) {
  if (!AccsEventElem) return -1;

  // mapAccs.clear();

  TiXmlElement* elemAcc = AccsEventElem->FirstChildElement("acc");
  while (elemAcc) {
    CAcceleration a;

    int nRet = ParseOneAccEvent(elemAcc, a);
    if (nRet) {
      std::cout << "acc event format error!" << std::endl;
      assert(false);

      elemAcc = elemAcc->NextSiblingElement("acc");
      continue;
    }

    std::map<std::string, CAcceleration>::iterator itr = mapAccs.find(a.m_strID);
    if (itr == mapAccs.end()) {
      mapAccs.insert(std::make_pair(a.m_strID, a));
    } else {
      itr->second.m_strProfilesEvent = a.m_strProfilesEvent;
      itr->second.m_strEndConditionEvent = a.m_strEndConditionEvent;
    }

    elemAcc = elemAcc->NextSiblingElement("acc");
  }

  return 0;
}

int CTraffic::ParseOneAccEvent(TiXmlElement* elemAcc, CAcceleration& a) {
  if (!elemAcc) return -1;

  const char* p = elemAcc->Attribute("id");
  if (p) a.m_strID = p;

  p = elemAcc->Attribute("profile");
  if (p) {
    a.m_strProfilesEvent = p;
  }

  p = elemAcc->Attribute("endCondition");
  if (p) {
    a.m_strEndConditionEvent = p;
  }

  return 0;
}

int CTraffic::ParseMerges(TiXmlElement* MergesElem, std::map<std::string, CMerge>& mapMerges) {
  if (!MergesElem) return -1;

  mapMerges.clear();

  TiXmlElement* elemMerge = MergesElem->FirstChildElement("merge");
  while (elemMerge) {
    CMerge m;

    int nRet = ParseOneMerge(elemMerge, m);
    if (nRet) {
      std::cout << "merge format error!" << std::endl;
      assert(false);
    } else {
      mapMerges.insert(std::make_pair(m.m_strID, m));
    }

    elemMerge = elemMerge->NextSiblingElement("merge");
  }

  return 0;
}

int CTraffic::ParseOneMerge(TiXmlElement* elemMerge, CMerge& m) {
  if (!elemMerge) return -1;

  const char* p = elemMerge->Attribute("id");
  if (p) m.m_strID = p;

  p = elemMerge->Attribute("profile");
  if (p) m.m_strProfileTime = p;

  return 0;
}

int CTraffic::ParseMergesEvent(TiXmlElement* MergesEventElem, std::map<std::string, CMerge>& mapMerges) {
  if (!MergesEventElem) return -1;

  // mapMerges.clear();

  TiXmlElement* elemMerge = MergesEventElem->FirstChildElement("merge");
  while (elemMerge) {
    CMerge m;

    int nRet = ParseOneMergeEvent(elemMerge, m);
    if (nRet) {
      std::cout << "merge event format error!" << std::endl;
      // assert(false);
    } else {
      std::map<std::string, CMerge>::iterator itr = mapMerges.find(m.m_strID);
      if (itr == mapMerges.end()) {
        mapMerges.insert(std::make_pair(m.m_strID, m));
      } else {
        itr->second.m_strProfileEvent = m.m_strProfileEvent;
      }
    }

    elemMerge = elemMerge->NextSiblingElement("merge");
  }

  return 0;
}

int CTraffic::ParseOneMergeEvent(TiXmlElement* elemMerge, CMerge& m) {
  if (!elemMerge) return -1;

  const char* p = elemMerge->Attribute("id");
  if (p) m.m_strID = p;

  p = elemMerge->Attribute("profile");
  if (p) m.m_strProfileEvent = p;

  return 0;
}

int CTraffic::ParseSceneEvents(TiXmlElement* SceneEventsElem, std::map<std::string, SceneEvent>& mapSceneEvents) {
  if (!SceneEventsElem) return -1;

  mapSceneEvents.clear();

  TiXmlElement* elemSceneEvent = SceneEventsElem->FirstChildElement("event");
  while (elemSceneEvent) {
    SceneEvent se;
    std::string version = SceneEventsElem->Attribute("version");
    int nRet = ParseOneSceneEvent(elemSceneEvent, se, version);
    if (nRet) {
      std::cout << "scene event format error!" << std::endl;
      // assert(false);

      elemSceneEvent = elemSceneEvent->NextSiblingElement("event");
      continue;
    }

    mapSceneEvents.insert(std::make_pair(se.m_id, se));

    elemSceneEvent = elemSceneEvent->NextSiblingElement("event");
  }

  return 0;
}
int CTraffic::ParseOneSceneEvent(TiXmlElement* elemSceneEvent, SceneEvent& se, std::string version) {
  if (!elemSceneEvent) return -1;
  const char* p = elemSceneEvent->Attribute("id");
  if (p) se.m_id = p;

  if (version == "1.0.0.0" || version == "1.1.0.0") {
    SceneCondition condition;
    p = elemSceneEvent->Attribute("type");
    if (p) condition.strType = p;
    p = elemSceneEvent->Attribute("condition");
    std::string strCondition;
    if (p) strCondition = p;
    if (condition.loadCondition(strCondition)) se.ConditionVec.push_back(condition);
  } else {
    SceneCondition condition;
    // the most support num is 9
    for (int i = 1; i < 10; i++) {
      std::string _Type = std::string("type") + std::to_string(i);
      std::string _Conditon = std::string("condition") + std::to_string(i);
      if (elemSceneEvent->Attribute(_Type.c_str()) && elemSceneEvent->Attribute(_Conditon.c_str())) {
        SceneCondition condition;
        p = elemSceneEvent->Attribute(_Type.c_str());
        if (p) condition.strType = p;
        p = elemSceneEvent->Attribute(_Conditon.c_str());
        std::string strCondition;
        if (p) strCondition = p;
        if (condition.loadCondition(strCondition)) se.ConditionVec.push_back(condition);
      }
    }
  }

  p = elemSceneEvent->Attribute("name");
  if (p) se.strName = p;

  p = elemSceneEvent->Attribute("endCondition");
  if (p) se.strEndCondition = p;

  p = elemSceneEvent->Attribute("action");
  if (p) se.strAction = p;

  p = elemSceneEvent->Attribute("info");
  if (p) se.strInfo = p;
  if (version == "1.0.0.0") {
    se.SegmentString1_0();
  } else if (version == "1.1.0.0" || version == "1.2.0.0") {
    se.SegmentString();
  } else {
    SYSTEM_LOGGER_ERROR("ParseOneSceneEvent verion is error");
    return -1;
  }
  return 0;
}

int CTraffic::ParseVelocities(TiXmlElement* VelocitiesElem, std::map<std::string, CVelocity>& mapVelocities) {
  if (!VelocitiesElem) return -1;

  mapVelocities.clear();

  TiXmlElement* elemVelocity = VelocitiesElem->FirstChildElement("velocity");
  while (elemVelocity) {
    CVelocity v;

    int nRet = ParseOneVelocity(elemVelocity, v);
    if (nRet) {
      std::cout << "velocity format error!" << std::endl;
      assert(false);

      elemVelocity = elemVelocity->NextSiblingElement("velocity");
      continue;
    }

    mapVelocities.insert(std::make_pair(v.m_strID, v));

    elemVelocity = elemVelocity->NextSiblingElement("velocity");
  }

  return 0;
}

int CTraffic::ParseOneVelocity(TiXmlElement* elemVelocity, CVelocity& v) {
  if (!elemVelocity) return -1;

  const char* p = elemVelocity->Attribute("id");
  if (p) v.m_strID = p;

  p = elemVelocity->Attribute("profile");
  if (p) {
    v.m_strProfilesTime = p;
  }

  return 0;
}

int CTraffic::ParseVelocitiesEvent(TiXmlElement* VelocitiesEventElem, std::map<std::string, CVelocity>& mapVelocities) {
  if (!VelocitiesEventElem) return -1;

  // mapAccs.clear();

  TiXmlElement* elemVelocity = VelocitiesEventElem->FirstChildElement("velocity");
  while (elemVelocity) {
    CVelocity v;

    int nRet = ParseOneVelocityEvent(elemVelocity, v);
    if (nRet) {
      std::cout << "velocity event format error!" << std::endl;
      assert(false);

      elemVelocity = elemVelocity->NextSiblingElement("velocity");
      continue;
    }

    std::map<std::string, CVelocity>::iterator itr = mapVelocities.find(v.m_strID);
    if (itr == mapVelocities.end()) {
      mapVelocities.insert(std::make_pair(v.m_strID, v));
    } else {
      itr->second.m_strProfilesEvent = v.m_strProfilesEvent;
    }

    elemVelocity = elemVelocity->NextSiblingElement("velocity");
  }

  return 0;
}

int CTraffic::ParseOneVelocityEvent(TiXmlElement* elemVelocity, CVelocity& v) {
  if (!elemVelocity) return -1;

  const char* p = elemVelocity->Attribute("id");
  if (p) v.m_strID = p;

  p = elemVelocity->Attribute("profile");
  if (p) {
    v.m_strProfilesEvent = p;
  }

  return 0;
}

int CTraffic::ParseVehicles(TiXmlElement* VehiclesElem, std::map<std::string, CVehicle>& mapVehicles) {
  if (!VehiclesElem) return -1;

  mapVehicles.clear();

  TiXmlElement* elemVehicle = VehiclesElem->FirstChildElement("vehicle");
  while (elemVehicle) {
    CVehicle v;

    int nRet = ParseOneVehicle(elemVehicle, v);
    if (nRet) {
      std::cout << "vehicle format error!" << std::endl;
      assert(false);
    } else {
      v.ConvertToValue();
      mapVehicles.insert(std::make_pair(v.m_strID, v));
    }

    elemVehicle = elemVehicle->NextSiblingElement("vehicle");
  }

  return 0;
}

int CTraffic::ParseOneVehicle(TiXmlElement* elemVehicle, CVehicle& v) {
  if (!elemVehicle) return -1;

  const char* p = elemVehicle->Attribute("id");
  if (p) v.m_strID = p;

  p = elemVehicle->Attribute("routeID");
  if (p) v.m_strRouteID = p;

  p = elemVehicle->Attribute("laneID");
  if (p) v.m_strLaneID = p;

  p = elemVehicle->Attribute("start_s");
  if (p) v.m_strStartShift = p;

  p = elemVehicle->Attribute("start_t");
  if (p) v.m_strStartTime = p;

  p = elemVehicle->Attribute("start_v");
  if (p) v.m_strStartVelocity = p;

  p = elemVehicle->Attribute("max_v");
  if (p) v.m_strMaxVelocity = p;

  p = elemVehicle->Attribute("l_offset");
  if (p) v.m_strOffset = p;

  p = elemVehicle->Attribute("length");
  if (p) v.m_boundingBox.m_strLength = p;

  p = elemVehicle->Attribute("width");
  if (p) v.m_boundingBox.m_strWidth = p;

  p = elemVehicle->Attribute("height");
  if (p) v.m_boundingBox.m_strHeight = p;

  p = elemVehicle->Attribute("boundingBoxCenterX");
  if (p) v.m_boundingBox.m_strX = p;

  p = elemVehicle->Attribute("boundingBoxCenterY");
  if (p) v.m_boundingBox.m_strY = p;

  p = elemVehicle->Attribute("boundingBoxCenterZ");
  if (p) v.m_boundingBox.m_strZ = p;

  p = elemVehicle->Attribute("accID");
  if (p) v.m_strAccID = p;

  p = elemVehicle->Attribute("mergeID");
  if (p) v.m_strMergeID = p;

  p = elemVehicle->Attribute("vehicleType");
  if (p) v.m_strType = p;

  p = elemVehicle->Attribute("follow");
  if (p) v.m_strFollow = p;

  p = elemVehicle->Attribute("behavior");
  if (p) {
    v.m_strBehavior = p;
  } else {
    v.m_strBehavior = "UserDefine";
  }

  p = elemVehicle->Attribute("aggress");
  if (p) {
    v.m_strAggress = p;
  } else {
    v.m_strAggress = "-1";
  }
  p = elemVehicle->Attribute("eventId");
  if (p) v.m_strEventId = p;

  p = elemVehicle->Attribute("angle");
  if (p) v.m_strAngle = p;

  p = elemVehicle->Attribute("sensorGroup");
  if (p) v.m_strSensorGroup = p;

  p = elemVehicle->Attribute("obuStatus");
  if (p) v.m_strObuStauts = p;

  p = elemVehicle->Attribute("start_angle");
  if (p) v.m_strStartAngle = p;

  return 0;
}

int CTraffic::ParseObstacles(TiXmlElement* ObstaclesElem, std::map<std::string, CObstacle>& mapObstacles) {
  if (!ObstaclesElem) return 0;

  mapObstacles.clear();

  TiXmlElement* elemObstacle = ObstaclesElem->FirstChildElement("obstacle");
  while (elemObstacle) {
    CObstacle o;

    int nRet = ParseOneObstacle(elemObstacle, o);
    if (nRet) {
      std::cout << "obstacle format error!" << std::endl;
      assert(false);
    } else {
      mapObstacles.insert(std::make_pair(o.m_strID, o));
    }

    elemObstacle = elemObstacle->NextSiblingElement("obstacle");
  }

  return 0;
}

int CTraffic::ParseOneObstacle(TiXmlElement* ObstacleElem, CObstacle& o) {
  if (!ObstacleElem) return -1;

  const char* p = ObstacleElem->Attribute("id");
  if (p) o.m_strID = p;

  p = ObstacleElem->Attribute("routeID");
  if (p) o.m_strRouteID = p;

  p = ObstacleElem->Attribute("laneID");
  if (p) o.m_strLaneID = p;

  p = ObstacleElem->Attribute("start_s");
  if (p) o.m_strStartShift = p;

  p = ObstacleElem->Attribute("l_offset");
  if (p) o.m_strOffset = p;

  p = ObstacleElem->Attribute("length");
  if (p) o.m_boundingBox.m_strLength = p;

  p = ObstacleElem->Attribute("width");
  if (p) o.m_boundingBox.m_strWidth = p;

  p = ObstacleElem->Attribute("height");
  if (p) o.m_boundingBox.m_strHeight = p;

  p = ObstacleElem->Attribute("boundingBoxCenterX");
  if (p) o.m_boundingBox.m_strX = p;

  p = ObstacleElem->Attribute("boundingBoxCenterY");
  if (p) o.m_boundingBox.m_strY = p;

  p = ObstacleElem->Attribute("boundingBoxCenterZ");
  if (p) o.m_boundingBox.m_strZ = p;

  p = ObstacleElem->Attribute("vehicleType");
  if (p) o.m_strType = p;

  p = ObstacleElem->Attribute("direction");
  if (p) o.m_strDirection = p;

  p = ObstacleElem->Attribute("start_angle");
  if (p) o.m_strStartAngle = p;

  return 0;
}

int CTraffic::ParsePedestrians(TiXmlElement* PedestriansElem, PedestrianMap& mapPedestrians) {
  if (!PedestriansElem) return -1;

  mapPedestrians.clear();

  TiXmlElement* elemPedestrian = PedestriansElem->FirstChildElement("pedestrian");
  while (elemPedestrian) {
    CPedestrian p;

    int nRet = ParseOnePedestrian(elemPedestrian, p);
    if (nRet) {
      XML_LOGGER_ERROR("pedestrian format error!");
      assert(false);
    } else {
      mapPedestrians.insert(std::make_pair(p.m_strID, p));
    }

    elemPedestrian = elemPedestrian->NextSiblingElement("pedestrian");
  }

  return 0;
}

int CTraffic::ParseOnePedestrian(TiXmlElement* PedestrianElem, CPedestrian& pede) {
  if (!PedestrianElem) return -1;

  const char* p = PedestrianElem->Attribute("id");
  if (p) pede.m_strID = p;

  p = PedestrianElem->Attribute("routeID");
  if (p) pede.m_strRouteID = p;

  p = PedestrianElem->Attribute("laneID");
  if (p) pede.m_strLaneID = p;

  p = PedestrianElem->Attribute("start_s");
  if (p) pede.m_strStartShift = p;

  p = PedestrianElem->Attribute("start_t");
  if (p) pede.m_strStartTime = p;

  p = PedestrianElem->Attribute("end_t");
  if (p) pede.m_strEndTime = p;

  p = PedestrianElem->Attribute("l_offset");
  if (p) pede.m_strOffset = p;

  p = PedestrianElem->Attribute("type");
  if (p) {
    pede.m_strType = p;
  } else {
    pede.m_strType = "human";
  }

  for (int i = 1; i < 3; ++i) {
    tagDirection d;

    std::string strDirection = "direction";
    strDirection.append(std::to_string(i));

    p = PedestrianElem->Attribute(strDirection.c_str());
    if (p)
      d.m_strDir = p;
    else
      continue;

    std::string strDirectionTime = strDirection;
    strDirectionTime.append("_t");
    p = PedestrianElem->Attribute(strDirectionTime.c_str());
    if (p)
      d.m_strDirDuration = p;
    else
      continue;

    std::string strDirectionVelocity = strDirection + "_v";
    p = PedestrianElem->Attribute(strDirectionVelocity.c_str());
    if (p)
      d.m_strDirVelocity = p;
    else
      continue;

    pede.m_directions.push_back(d);
  }
  return 0;
}

int CTraffic::ParsePedestriansEvent(TiXmlElement* PedestriansEventElem, PedestrianMap& mapPedestrians) {
  if (!PedestriansEventElem) return -1;

  if (mapPedestrians.size() == 0) {
    return 0;
  }

  TiXmlElement* elemPedestrianEvent = PedestriansEventElem->FirstChildElement("shuttle");
  while (elemPedestrianEvent) {
    CPedestrianEvent pe;

    int nRet = ParseOnePedestrianEvent(elemPedestrianEvent, pe);
    if (nRet) {
      XML_LOGGER_ERROR("pedestrian event format error!");
      assert(false);
    }

    PedestrianMap::iterator itr = mapPedestrians.find(pe.m_strID);
    if (itr != mapPedestrians.end()) {
      itr->second.m_events = pe;
    } else {
      XML_LOGGER_ERROR("event no owning pedestrian!");
      assert(false);
    }

    elemPedestrianEvent = elemPedestrianEvent->NextSiblingElement("shuttle");
  }

  return 0;
}

int CTraffic::ParseOnePedestrianEvent(TiXmlElement* PedestrianEventElem, CPedestrianEvent& pe) {
  if (!PedestrianEventElem) return -1;

  const char* p = PedestrianEventElem->Attribute("id");
  if (p) pe.m_strID = p;

  p = PedestrianEventElem->Attribute("profile");
  if (p) pe.m_strProfiles = p;

  return 0;
}

int CTraffic::ParsePedestriansV2(TiXmlElement* PedestriansElem, PedestrianV2Map& mapPedestrians) {
  if (!PedestriansElem) return -1;

  mapPedestrians.clear();

  TiXmlElement* elemPedestrian = PedestriansElem->FirstChildElement("pedestrian");
  while (elemPedestrian) {
    CPedestrianV2 p;

    int nRet = ParseOnePedestrianV2(elemPedestrian, p);
    if (nRet) {
      XML_LOGGER_ERROR("pedestrian format error!");
      assert(false);
    } else {
      mapPedestrians.insert(std::make_pair(p.m_strID, p));
    }

    elemPedestrian = elemPedestrian->NextSiblingElement("pedestrian");
  }

  return 0;
}

int CTraffic::ParseOnePedestrianV2(TiXmlElement* PedestrianElem, CPedestrianV2& pede) {
  if (!PedestrianElem) return -1;

  ParseOnePedestrian(PedestrianElem, pede);

  const char* p = PedestrianElem->Attribute("start_v");
  if (p) pede.m_strStartVelocity = p;

  p = PedestrianElem->Attribute("max_v");
  if (p) pede.m_strMaxVelocity = p;

  p = PedestrianElem->Attribute("behavior");
  if (p) pede.m_strBehavior = p;
  p = PedestrianElem->Attribute("eventId");
  if (p) pede.m_strEventId = p;
  p = PedestrianElem->Attribute("angle");
  if (p) pede.m_strAngle = p;
  p = PedestrianElem->Attribute("start_angle");
  if (p) pede.m_strStartAngle = p;
  p = PedestrianElem->Attribute("boundingBoxCenterX");
  if (p) pede.m_boundingBox.m_strX = p;
  p = PedestrianElem->Attribute("boundingBoxCenterY");
  if (p) pede.m_boundingBox.m_strY = p;
  p = PedestrianElem->Attribute("boundingBoxCenterZ");
  if (p) pede.m_boundingBox.m_strZ = p;
  p = PedestrianElem->Attribute("width");
  if (p) pede.m_boundingBox.m_strWidth = p;
  p = PedestrianElem->Attribute("height");
  if (p) pede.m_boundingBox.m_strHeight = p;
  p = PedestrianElem->Attribute("length");
  if (p) pede.m_boundingBox.m_strLength = p;
  return 0;
}

int CTraffic::ParsePedestrianVelocities(TiXmlElement* VelocitiesElem,
                                        std::map<std::string, CPedestrianVelocity>& mapVelocities) {
  if (!VelocitiesElem) return -1;

  mapVelocities.clear();

  TiXmlElement* elemTimeVelocity = VelocitiesElem->FirstChildElement("time_velocity");
  while (elemTimeVelocity) {
    CPedestrianVelocity v;

    int nRet = ParseOnePedestrianTimeVelocity(elemTimeVelocity, v);
    if (nRet) {
      std::cout << "velocity format error!" << std::endl;
      assert(false);

      elemTimeVelocity = elemTimeVelocity->NextSiblingElement("time_velocity");
      continue;
    }

    mapVelocities.insert(std::make_pair(v.m_strID, v));

    elemTimeVelocity = elemTimeVelocity->NextSiblingElement("time_velocity");
  }

  TiXmlElement* elemEventVelocity = VelocitiesElem->FirstChildElement("event_velocity");
  while (elemEventVelocity) {
    CPedestrianVelocity v;

    int nRet = ParseOnePedestrianEventVelocity(elemEventVelocity, v);
    if (nRet) {
      std::cout << "velocity format error!" << std::endl;
      assert(false);

      elemEventVelocity = elemEventVelocity->NextSiblingElement("event_velocity");
      continue;
    }

    std::map<std::string, CPedestrianVelocity>::iterator itr = mapVelocities.find(v.m_strID);
    if (itr != mapVelocities.end()) {
      itr->second.m_strProfilesEvent = v.m_strProfilesEvent;
    } else {
      mapVelocities.insert(std::make_pair(v.m_strID, v));
    }

    elemEventVelocity = elemEventVelocity->NextSiblingElement("event_velocity");
  }
  return 0;
}

int CTraffic::ParseOnePedestrianTimeVelocity(TiXmlElement* elemVelocity, CPedestrianVelocity& v) {
  if (!elemVelocity) return -1;

  const char* p = elemVelocity->Attribute("id");
  if (p) v.m_strID = p;

  p = elemVelocity->Attribute("profile");
  if (p) {
    v.m_strProfilesTime = p;
  }

  return 0;
}

int CTraffic::ParseOnePedestrianEventVelocity(TiXmlElement* elemVelocity, CPedestrianVelocity& v) {
  if (!elemVelocity) return -1;

  const char* p = elemVelocity->Attribute("id");
  if (p) v.m_strID = p;

  p = elemVelocity->Attribute("profile");
  if (p) {
    v.m_strProfilesEvent = p;
  }

  return 0;
}

int CTraffic::ParseSignlights(TiXmlElement* SinglightsElem, SignLightMap& mapSignlight) {
  if (!SinglightsElem) return -1;

  mapSignlight.clear();
  TiXmlElement* elemSignlight = SinglightsElem->FirstChildElement("signlight");
  while (elemSignlight) {
    CSignLight l;

    int nRet = ParseOneSignlight(elemSignlight, l);
    if (nRet) {
      XML_LOGGER_ERROR("signlight format error!");
      assert(false);
    } else {
      mapSignlight.insert(std::make_pair(l.m_strID, l));
    }

    elemSignlight = elemSignlight->NextSiblingElement("signlight");
  }

  return 0;
}

int CTraffic::ParseOneSignlight(TiXmlElement* SignlightElem, CSignLight& l) {
  if (!SignlightElem) return -1;

  const char* p = SignlightElem->Attribute("id");
  if (p) l.m_strID = p;

  p = SignlightElem->Attribute("routeID");
  if (p) l.m_strRouteID = p;

  p = SignlightElem->Attribute("start_s");
  if (p) l.m_strStartShift = p;

  p = SignlightElem->Attribute("start_t");
  if (p) l.m_strStartTime = p;

  p = SignlightElem->Attribute("l_offset");
  if (p) l.m_strOffset = p;

  p = SignlightElem->Attribute("time_green");
  if (p) l.m_strTimeGreen = p;

  p = SignlightElem->Attribute("time_yellow");
  if (p) l.m_strTimeYellow = p;

  p = SignlightElem->Attribute("time_red");
  if (p) l.m_strTimeRed = p;

  p = SignlightElem->Attribute("direction");
  if (p) l.m_strDirection = p;
  p = SignlightElem->Attribute("compliance");
  if (p) l.m_strCompliance = p;

  p = SignlightElem->Attribute("status");
  if (p) l.m_strStatus = p;

  p = SignlightElem->Attribute("phase");
  if (p) l.m_strPhase = p;

  p = SignlightElem->Attribute("lane");
  if (p) l.m_strLane = p;

  p = SignlightElem->Attribute("plan");
  if (p) l.m_strPlan = p;

  p = SignlightElem->Attribute("junction");
  if (p) l.m_strJunction = p;

  p = SignlightElem->Attribute("phaseNumber");
  if (p) l.m_strPhasenumber = p;

  p = SignlightElem->Attribute("signalHead");
  if (p) {
    std::vector<std::string> strVec;
    boost::algorithm::split(strVec, p, boost::algorithm::is_any_of(","));
    for (auto it : strVec) {
      l.m_strSignalheadVec.push_back(it);
    }
  }

  return 0;
}

int CTraffic::Save(const char* strFile) {
  if (!strFile) return -1;
  TiXmlDocument doc;

  TiXmlDeclaration* dec = new TiXmlDeclaration("1.0", "utf-8", "yes");
  TiXmlElement* elemTraffic = new TiXmlElement("traffic");
  elemTraffic->SetAttribute("version", "1.1");
  elemTraffic->SetAttribute("mode", m_strMode);

  TiXmlElement* elemData = NULL;
  if (m_RandomSeed > 0 || (m_dAggress > AGGRESS_RANGE_MIN && m_dAggress < AGGRESS_RANGE_MAX)) {
    elemData = new TiXmlElement("data");
    int nRet = SaveData(elemData, m_RandomSeed, m_dAggress);
    if (nRet) {
      // assert(false);
      return -1;
    }
  }

  TiXmlElement* elemRoutes = new TiXmlElement("routes");
  int nRet = SaveRoutes(elemRoutes, m_mapRoutes);
  if (nRet) {
    // assert(false);
    return -1;
  }

  TiXmlElement* elemAccs = new TiXmlElement("accs");
  nRet = SaveAccs(elemAccs, m_mapAccs);
  if (nRet) {
    // assert(false);
    return -1;
  }

  TiXmlElement* elemAccsEvent = new TiXmlElement("accs_event");
  nRet = SaveAccsEvent(elemAccsEvent, m_mapAccs);
  if (nRet) {
    // assert(false);
    return -1;
  }

  TiXmlElement* elemMerges = new TiXmlElement("merges");
  nRet = SaveMerges(elemMerges, m_mapMerges);
  if (nRet) {
    assert(false);
    return -1;
  }

  TiXmlElement* elemMergesEvent = new TiXmlElement("merges_event");
  nRet = SaveMergesEvent(elemMergesEvent, m_mapMerges);
  if (nRet) {
    // assert(false);
    return -1;
  }

  TiXmlElement* elemVelocities = new TiXmlElement("velocities");
  nRet = SaveVelocities(elemVelocities, m_mapVelocities);
  if (nRet) {
    // assert(false);
  }

  TiXmlElement* elemVelocitiesEvent = new TiXmlElement("velocities_event");
  nRet = SaveVelocitiesEvent(elemVelocitiesEvent, m_mapVelocities);
  if (nRet) {
    // assert(false);
    return -1;
  }

  TiXmlElement* elemPedestriansEvent = new TiXmlElement("pedestrians_event");
  nRet = SavePedestriansEvent(elemPedestriansEvent, m_mapPedestrians);
  if (nRet) {
    // assert(false);
  }

  nRet = SavePedestrianVelocities(elemPedestriansEvent, m_mapPedestrianVelocities);
  if (nRet) {
    // assert(false);
  }

  TiXmlElement* elemVehicles = new TiXmlElement("vehicles");
  nRet = SaveVehicles(elemVehicles, m_mapVehicles);
  if (nRet) {
    // assert(false);
    return -1;
  }

  TiXmlElement* elemObstacles = new TiXmlElement("obstacles");
  nRet = SaveObstacles(elemObstacles, m_mapObstacles);
  if (nRet) {
    // assert(false);
  }

  TiXmlElement* elemPedestrians = new TiXmlElement("pedestrians");
  nRet = SavePedestrians(elemPedestrians, m_mapPedestrians);
  if (nRet) {
    // assert(false);
    return -1;
  }

  nRet = SavePedestriansV2(elemPedestrians, m_mapPedestriansV2);
  if (nRet) {
    // assert(false);
    return -1;
  }

  TiXmlElement* elemSignlights = new TiXmlElement("signlights");
  if (m_ActivePlan != "-1") elemSignlights->SetAttribute("activePlan", m_ActivePlan);
  nRet = SaveSignlights(elemSignlights, m_mapSignlights);
  if (nRet) {
    // assert(false);
    return -1;
  }
  std::map<std::string, CPedestrianVelocity>::iterator itr = m_mapPedestrianVelocities.begin();
  int n_eventsize = m_mapSceneEvents.size();
  int nextSceneEventId = 0;

  TiXmlElement* elemSceneEvents = new TiXmlElement("scene_event");
#ifdef SCENE_EVENTS_UPGRAD
  elemSceneEvents->SetAttribute("version", SCENE_EVENT_VERSION);  // 设定 scene_event 的版本
#else
  elemSceneEvents->SetAttribute("version", "1.0.0.0");
#endif
  nRet = SaveSceneEvents(elemSceneEvents, m_mapSceneEvents);
  if (nRet) {
    // assert(false);
    return -1;
  }

  if (elemData) {
    elemTraffic->LinkEndChild(elemData);
  }
  elemTraffic->LinkEndChild(elemRoutes);
  elemTraffic->LinkEndChild(elemAccs);
  elemTraffic->LinkEndChild(elemAccsEvent);
  elemTraffic->LinkEndChild(elemMerges);
  elemTraffic->LinkEndChild(elemMergesEvent);
  elemTraffic->LinkEndChild(elemVelocities);
  elemTraffic->LinkEndChild(elemVelocitiesEvent);
  elemTraffic->LinkEndChild(elemPedestriansEvent);
  elemTraffic->LinkEndChild(elemVehicles);
  elemTraffic->LinkEndChild(elemObstacles);
  elemTraffic->LinkEndChild(elemPedestrians);
  elemTraffic->LinkEndChild(elemSignlights);
  elemTraffic->LinkEndChild(elemSceneEvents);

  TiXmlElement* elemTrafficFlow = elemTraffic->FirstChildElement("trafficFlow");
  if (!elemTrafficFlow) {
    elemTrafficFlow = elemTraffic->InsertEndChild(TiXmlElement("trafficFlow"))->ToElement();
  }
  m_trafficFlow.FilePath() = strFile;
  SaveTrafficFlow(elemTrafficFlow, m_trafficFlow);

  doc.LinkEndChild(dec);
  doc.LinkEndChild(elemTraffic);

  doc.SaveFile(strFile);
  return 0;
}

int CTraffic::SaveData(TiXmlElement* DataElem, int64_t nRandomSeed, double dAggress) {
  if (!DataElem) return -1;
  nRandomSeed = 55;  // 前端 暂无界面支持设置该数据   所以在生成文件的时候直接写个默认值55
  if (nRandomSeed >= 0) {
    DataElem->SetAttribute("randomseed", nRandomSeed);
  }

  if (dAggress > AGGRESS_RANGE_MIN && dAggress < AGGRESS_RANGE_MAX) {
    DataElem->SetAttribute("aggress", CUtils::doubleToStringDot3(dAggress));
  }

  return 0;
}

int CTraffic::SaveRoutes(TiXmlElement* RoutesElem, std::map<std::string, CRoute>& mapRoutes) {
  if (!RoutesElem) return -1;

  std::map<std::string, CRoute>::iterator itr = mapRoutes.begin();
  for (; itr != mapRoutes.end(); ++itr) {
    TiXmlElement* elemRoute = new TiXmlElement("route");

    int nRet = SaveOneRoute(elemRoute, itr->second);
    if (nRet) {
      std::cout << "route save error!" << std::endl;
      assert(false);
    }

    RoutesElem->LinkEndChild(elemRoute);
  }

  return 0;
}

int CTraffic::SaveOneRoute(TiXmlElement* elemRoute, CRoute& r) {
  if (!elemRoute) return -1;

  elemRoute->SetAttribute("id", r.m_strID);

  elemRoute->SetAttribute("type", r.m_strType);

  // if (_stricmp(r.m_strType.c_str(), "start_end") == 0)
  if (boost::algorithm::iequals(r.m_strType, "start_end")) {
    elemRoute->SetAttribute("start", r.m_strStart);
    if (r.m_strEnd.size() > 0) {
      elemRoute->SetAttribute("end", r.m_strEnd);
    }
    if (r.m_strMids.size() > 0) {
      elemRoute->SetAttribute("mid", r.m_strMids);
    }
    if (r.m_strControlPath.size() > 0) {
      elemRoute->SetAttribute("controlPath", r.m_strControlPath);
    }
  } else {
    elemRoute->SetAttribute("roadID", r.m_strRoadID);
    elemRoute->SetAttribute("sectionID", r.m_strSectionID);
  }
  if (r.m_strInfo.size() > 0) {
    elemRoute->SetAttribute("info", r.m_strInfo);
  }
  return 0;
}

int CTraffic::SaveAccs(TiXmlElement* AccsElem, std::map<std::string, CAcceleration>& mapAccs) {
  if (!AccsElem) return -1;

  std::map<std::string, CAcceleration>::iterator itr = mapAccs.begin();
  for (; itr != mapAccs.end(); ++itr) {
    TiXmlElement* elemAcc = new TiXmlElement("acc");

    int nRet = SaveOneAcc(elemAcc, itr->second);
    if (nRet) {
      std::cout << "Acc save error!" << std::endl;
      assert(false);
    }

    AccsElem->LinkEndChild(elemAcc);
  }

  return 0;
}

int CTraffic::SaveOneAcc(TiXmlElement* elemAcc, CAcceleration& a) {
  if (!elemAcc) return -1;

  elemAcc->SetAttribute("id", a.m_strID);
  if (a.m_strProfilesTime.size() > 0) {
    elemAcc->SetAttribute("profile", a.m_strProfilesTime);
  } else {
    elemAcc->SetAttribute("profile", "0,0");
  }

  if (a.m_strEndConditionTime.size() > 0) {
    elemAcc->SetAttribute("endCondition", a.m_strEndConditionTime);
  } else {
    elemAcc->SetAttribute("endCondition", "None,0");
  }

  return 0;
}

int CTraffic::SaveAccsEvent(TiXmlElement* AccsEventElem, std::map<std::string, CAcceleration>& mapAccs) {
  if (!AccsEventElem) return -1;

  std::map<std::string, CAcceleration>::iterator itr = mapAccs.begin();
  for (; itr != mapAccs.end(); ++itr) {
    // if (itr->second.m_strProfilesEvent.size() > 0)
    {
      TiXmlElement* elemAcc = new TiXmlElement("acc");
      int nRet = SaveOneAccEvent(elemAcc, itr->second);
      if (nRet) {
        std::cout << "Acc Event save error!" << std::endl;
        assert(false);
      }

      AccsEventElem->LinkEndChild(elemAcc);
    }
  }

  return 0;
}

int CTraffic::SaveOneAccEvent(TiXmlElement* elemAccEvent, CAcceleration& a) {
  if (!elemAccEvent) return -1;

  elemAccEvent->SetAttribute("id", a.m_strID);
  if (a.m_strProfilesEvent.size() > 0) {
    elemAccEvent->SetAttribute("profile", a.m_strProfilesEvent);
  } else {
    elemAccEvent->SetAttribute("profile", "ttc 0,0");
  }

  if (a.m_strEndConditionEvent.size() > 0) {
    elemAccEvent->SetAttribute("endCondition", a.m_strEndConditionEvent);
  } else {
    elemAccEvent->SetAttribute("endCondition", "None,0");
  }
  return 0;
}

int CTraffic::SaveMerges(TiXmlElement* MergesElem, std::map<std::string, CMerge>& mapMerges) {
  if (!MergesElem) return -1;

  std::map<std::string, CMerge>::iterator itr = mapMerges.begin();
  for (; itr != mapMerges.end(); ++itr) {
    TiXmlElement* elemMerge = new TiXmlElement("merge");

    int nRet = SaveOneMerge(elemMerge, itr->second);
    if (nRet) {
      std::cout << "Merge save error!" << std::endl;
      assert(false);
    }

    MergesElem->LinkEndChild(elemMerge);
  }

  return 0;
}

int CTraffic::SaveOneMerge(TiXmlElement* MergeElem, CMerge& m) {
  if (!MergeElem) return -1;

  MergeElem->SetAttribute("id", m.m_strID);
  if (m.m_strProfileTime.size() > 0) {
    MergeElem->SetAttribute("profile", m.m_strProfileTime);
  } else {
    MergeElem->SetAttribute("profile", "0,0");
  }

  return 0;
}

int CTraffic::SaveMergesEvent(TiXmlElement* MergesEventElem, std::map<std::string, CMerge>& mapMerges) {
  if (!MergesEventElem) return -1;

  std::map<std::string, CMerge>::iterator itr = mapMerges.begin();
  for (; itr != mapMerges.end(); ++itr) {
    // if (itr->second.m_strProfileEvent.size() > 0)
    {
      TiXmlElement* elemMerge = new TiXmlElement("merge");

      int nRet = SaveOneMergeEvent(elemMerge, itr->second);
      if (nRet) {
        std::cout << "Merge event save error!" << std::endl;
        assert(false);
      }

      MergesEventElem->LinkEndChild(elemMerge);
    }
  }

  return 0;
}

int CTraffic::SaveOneMergeEvent(TiXmlElement* MergeElem, CMerge& m) {
  if (!MergeElem) return -1;

  MergeElem->SetAttribute("id", m.m_strID);
  if (m.m_strProfileEvent.size() > 0) {
    MergeElem->SetAttribute("profile", m.m_strProfileEvent);
  } else {
    MergeElem->SetAttribute("profile", "ttc 0,0");
  }

  return 0;
}

int CTraffic::SaveVelocities(TiXmlElement* VelocitiesElem, std::map<std::string, CVelocity>& mapVelocities) {
  if (!VelocitiesElem) return -1;

  std::map<std::string, CVelocity>::iterator itr = mapVelocities.begin();
  for (; itr != mapVelocities.end(); ++itr) {
    if (itr->second.m_strProfilesTime.size() > 0) {
      TiXmlElement* elemVelocity = new TiXmlElement("velocity");
      int nRet = SaveOneVelocity(elemVelocity, itr->second);
      if (nRet) {
        std::cout << "Velocity save error!" << std::endl;
        assert(false);
      }

      VelocitiesElem->LinkEndChild(elemVelocity);
    }
  }

  return 0;
}

int CTraffic::SaveOneVelocity(TiXmlElement* elemVelocity, CVelocity& v) {
  if (!elemVelocity) return -1;

  elemVelocity->SetAttribute("id", v.m_strID);
  if (v.m_strProfilesTime.size() > 0) {
    elemVelocity->SetAttribute("profile", v.m_strProfilesTime);
  }
  /*
  else {
          elemVelocity->SetAttribute("profile", "0,0");
  }
  */

  return 0;
}

int CTraffic::SaveVelocitiesEvent(TiXmlElement* VelocitiesEventElem, std::map<std::string, CVelocity>& mapVelocities) {
  if (!VelocitiesEventElem) return -1;

  std::map<std::string, CVelocity>::iterator itr = mapVelocities.begin();
  for (; itr != mapVelocities.end(); ++itr) {
    if (itr->second.m_strProfilesEvent.size() > 0) {
      TiXmlElement* elemVelocity = new TiXmlElement("velocity");
      int nRet = SaveOneVelocityEvent(elemVelocity, itr->second);
      if (nRet) {
        std::cout << "Velocity Event save error!" << std::endl;
        assert(false);
      }

      VelocitiesEventElem->LinkEndChild(elemVelocity);
    }
  }

  return 0;
}

int CTraffic::SaveOneVelocityEvent(TiXmlElement* elemVelocityEvent, CVelocity& v) {
  if (!elemVelocityEvent) return -1;

  elemVelocityEvent->SetAttribute("id", v.m_strID);
  if (v.m_strProfilesEvent.size() > 0) {
    elemVelocityEvent->SetAttribute("profile", v.m_strProfilesEvent);
  }
  /*
  else {
          elemAccEvent->SetAttribute("profile", "ttc 0,0");
  }
  */

  return 0;
}

int CTraffic::SaveVehicles(TiXmlElement* VehiclesElem, std::map<std::string, CVehicle>& mapVehicles) {
  if (!VehiclesElem) return -1;

  std::map<std::string, CVehicle>::iterator itr = mapVehicles.begin();
  for (; itr != mapVehicles.end(); ++itr) {
    TiXmlElement* elemMerge = new TiXmlElement("vehicle");

    int nRet = SaveOneVehicle(elemMerge, itr->second);
    if (nRet) {
      std::cout << "Vehicle save error!" << std::endl;
      // assert(false);
      return -1;
    }

    VehiclesElem->LinkEndChild(elemMerge);
  }

  return 0;
}

int CTraffic::SaveOneVehicle(TiXmlElement* elemVehicle, CVehicle& v) {
  if (!elemVehicle) return -1;
  v.ConvertToValue();
  elemVehicle->SetAttribute("id", v.m_strID);
  elemVehicle->SetAttribute("routeID", v.m_strRouteID);
  elemVehicle->SetAttribute("laneID", v.m_strLaneID);
  elemVehicle->SetAttribute("start_s", v.m_strStartShift);
  elemVehicle->SetAttribute("start_t", v.m_strStartTime);
  elemVehicle->SetAttribute("start_v", v.m_strStartVelocity);

  if (v.m_fMaxVelocity >= 0 && v.m_strMaxVelocity.size() > 0) {
    elemVehicle->SetAttribute("max_v", v.m_strMaxVelocity);
  }

  elemVehicle->SetAttribute("l_offset", v.m_strOffset);
  // boundingbox
  elemVehicle->SetAttribute("length", v.m_boundingBox.m_strLength);
  elemVehicle->SetAttribute("width", v.m_boundingBox.m_strWidth);
  elemVehicle->SetAttribute("height", v.m_boundingBox.m_strHeight);
  elemVehicle->SetAttribute("boundingBoxCenterX", v.m_boundingBox.m_strX);
  elemVehicle->SetAttribute("boundingBoxCenterY", v.m_boundingBox.m_strY);
  elemVehicle->SetAttribute("boundingBoxCenterZ", v.m_boundingBox.m_strZ);

  elemVehicle->SetAttribute("accID", v.m_strAccID);
  elemVehicle->SetAttribute("mergeID", v.m_strMergeID);
  elemVehicle->SetAttribute("vehicleType", v.m_strType);
  if (v.m_strFollow.size() > 0) {
    elemVehicle->SetAttribute("follow", v.m_strFollow);
  }
  if (v.m_dAggress >= 0) {
    elemVehicle->SetAttribute("aggress", v.m_strAggress);
  }

  if (v.m_strBehavior.size() >= 0) {
    elemVehicle->SetAttribute("behavior", v.m_strBehavior);
  }
  if (v.m_strEventId.size() > 0) {
    elemVehicle->SetAttribute("eventId", v.m_strEventId);
  }

  if (v.m_strAngle.size() > 0) {
    elemVehicle->SetAttribute("angle", v.m_strAngle);
  }

  if (v.m_strSensorGroup.size() > 0) {
    elemVehicle->SetAttribute("sensorGroup", v.m_strSensorGroup);
  }

  if (v.m_strObuStauts.size() > 0) {
    elemVehicle->SetAttribute("obuStatus", v.m_strObuStauts);
  }

  if (v.m_strStartAngle.size() > 0) {
    elemVehicle->SetAttribute("start_angle", v.m_strStartAngle);
  }
  return 0;
}

int CTraffic::SaveObstacles(TiXmlElement* ObstaclesElem, std::map<std::string, CObstacle>& mapObstacles) {
  if (!ObstaclesElem) return -1;

  std::map<std::string, CObstacle>::iterator itr = mapObstacles.begin();
  for (; itr != mapObstacles.end(); ++itr) {
    TiXmlElement* elemObstacle = new TiXmlElement("obstacle");

    int nRet = SaveOneObstacle(elemObstacle, itr->second);
    if (nRet) {
      std::cout << "Obstacle save error!" << std::endl;
      assert(false);
    }

    ObstaclesElem->LinkEndChild(elemObstacle);
  }

  return 0;
}

int CTraffic::SaveOneObstacle(TiXmlElement* ObstacleElem, CObstacle& o) {
  if (!ObstacleElem) return -1;

  ObstacleElem->SetAttribute("id", o.m_strID);
  ObstacleElem->SetAttribute("routeID", o.m_strRouteID);
  ObstacleElem->SetAttribute("laneID", o.m_strLaneID);
  ObstacleElem->SetAttribute("start_s", o.m_strStartShift);
  ObstacleElem->SetAttribute("l_offset", o.m_strOffset);
  ObstacleElem->SetAttribute("length", o.m_boundingBox.m_strLength);
  ObstacleElem->SetAttribute("width", o.m_boundingBox.m_strWidth);
  ObstacleElem->SetAttribute("height", o.m_boundingBox.m_strHeight);
  ObstacleElem->SetAttribute("boundingBoxCenterX", o.m_boundingBox.m_strX);
  ObstacleElem->SetAttribute("boundingBoxCenterY", o.m_boundingBox.m_strY);
  ObstacleElem->SetAttribute("boundingBoxCenterZ", o.m_boundingBox.m_strZ);
  ObstacleElem->SetAttribute("vehicleType", o.m_strType);
  ObstacleElem->SetAttribute("direction", o.m_strDirection);
  ObstacleElem->SetAttribute("start_angle", o.m_strStartAngle);

  return 0;
}

int CTraffic::SavePedestrians(TiXmlElement* PedestriansElem, PedestrianMap& mapPedestrians) {
  if (!PedestriansElem) return -1;

  PedestrianMap::iterator itr = mapPedestrians.begin();
  for (; itr != mapPedestrians.end(); ++itr) {
    TiXmlElement* elemPedestrian = new TiXmlElement("pedestrian");

    int nRet = SaveOnePedestrian(elemPedestrian, itr->second);
    if (nRet) {
      XML_LOGGER_ERROR("Pedestrian save error!");
      assert(false);
    }

    PedestriansElem->LinkEndChild(elemPedestrian);
  }

  return 0;
}

int CTraffic::SaveOnePedestrian(TiXmlElement* PedestrianElem, CPedestrian& p) {
  if (!PedestrianElem) return -1;

  PedestrianElem->SetAttribute("id", p.m_strID);
  PedestrianElem->SetAttribute("routeID", p.m_strRouteID);
  PedestrianElem->SetAttribute("laneID", p.m_strLaneID);
  PedestrianElem->SetAttribute("start_s", p.m_strStartShift);
  if (p.m_strStartTime.size() > 0) {
    PedestrianElem->SetAttribute("start_t", p.m_strStartTime);
  } else {
    PedestrianElem->SetAttribute("start_t", "0");
  }

  if (p.m_strType.size() > 0) {
    PedestrianElem->SetAttribute("type", p.m_strType);
  } else {
    PedestrianElem->SetAttribute("type", "human");
  }

  if (p.m_strEndTime.size() > 0) {
    PedestrianElem->SetAttribute("end_t", p.m_strEndTime);
  }
  PedestrianElem->SetAttribute("l_offset", p.m_strOffset);

  for (int i = 0; i < p.m_directions.size(); ++i) {
    std::string strDirection = "direction";
    strDirection.append(std::to_string(i + 1));
    PedestrianElem->SetAttribute(strDirection.c_str(), p.m_directions[i].m_strDir);

    std::string strDirectionTime = strDirection + "_t";
    PedestrianElem->SetAttribute(strDirectionTime.c_str(), p.m_directions[i].m_strDirDuration);

    std::string strDirectionVelocity = strDirection + "_v";
    PedestrianElem->SetAttribute(strDirectionVelocity.c_str(), p.m_directions[i].m_strDirVelocity);
  }

  if (true) {
    int nCount = p.m_directions.size();
    if (nCount < 3 && nCount > 0) {
      for (int i = nCount; i < 3; ++i) {
        std::string strDirection = "direction";
        strDirection.append(std::to_string(i + 1));
        PedestrianElem->SetAttribute(strDirection.c_str(), "0");

        std::string strDirectionTime = strDirection + "_t";
        PedestrianElem->SetAttribute(strDirectionTime.c_str(), "0");

        std::string strDirectionVelocity = strDirection + "_v";
        PedestrianElem->SetAttribute(strDirectionVelocity.c_str(), "0");
      }
    }
  }

  return 0;
}

int CTraffic::SavePedestriansEvent(TiXmlElement* PedestriansEventElem, PedestrianMap& mapPedestrians) {
  if (!PedestriansEventElem) return -1;

  PedestrianMap::iterator itr = mapPedestrians.begin();
  for (; itr != mapPedestrians.end(); ++itr) {
    if (itr->second.m_events.m_strProfiles.size() > 0) {
      TiXmlElement* elemPedestrianEvent = new TiXmlElement("shuttle");

      int nRet = SaveOnePedestrianEvent(elemPedestrianEvent, itr->second);
      if (nRet) {
        XML_LOGGER_ERROR("Pedestrian event save error!");
        assert(false);
      }

      PedestriansEventElem->LinkEndChild(elemPedestrianEvent);
    }
  }

  return 0;
}

int CTraffic::SaveOnePedestrianEvent(TiXmlElement* PedestrianEventElem, CPedestrian& p) {
  if (!PedestrianEventElem) return -1;

  PedestrianEventElem->SetAttribute("id", p.m_strID);
  PedestrianEventElem->SetAttribute("profile", p.m_events.m_strProfiles);

  return 0;
}

int CTraffic::SavePedestriansV2(TiXmlElement* PedestriansElem, PedestrianV2Map& mapPedestrians) {
  if (!PedestriansElem) return -1;

  PedestrianV2Map::iterator itr = mapPedestrians.begin();
  for (; itr != mapPedestrians.end(); ++itr) {
    TiXmlElement* elemPedestrian = new TiXmlElement("pedestrian");

    int nRet = SaveOnePedestrianV2(elemPedestrian, itr->second);
    if (nRet) {
      XML_LOGGER_ERROR("Pedestrian save error!");
      assert(false);
    }

    PedestriansElem->LinkEndChild(elemPedestrian);
  }

  return 0;
}

int CTraffic::SaveOnePedestrianV2(TiXmlElement* PedestrianElem, CPedestrianV2& p) {
  if (!PedestrianElem) return -1;

  SaveOnePedestrian(PedestrianElem, p);

  if (p.m_strStartVelocity.size() > 0) {
    PedestrianElem->SetAttribute("start_v", p.m_strStartVelocity);
  }

  if (p.m_strStartAngle.size() > 0) {
    PedestrianElem->SetAttribute("start_angle", p.m_strStartAngle);
  }

  if (p.m_strMaxVelocity.size() > 0) {
    PedestrianElem->SetAttribute("max_v", p.m_strMaxVelocity);
  }

  if (p.m_strBehavior.size() > 0) {
    PedestrianElem->SetAttribute("behavior", p.m_strBehavior);
  }

  if (p.m_strEventId.size() > 0) {
    PedestrianElem->SetAttribute("eventId", p.m_strEventId);
  }
  PedestrianElem->SetAttribute("width", p.m_boundingBox.m_strWidth);
  PedestrianElem->SetAttribute("height", p.m_boundingBox.m_strHeight);
  PedestrianElem->SetAttribute("length", p.m_boundingBox.m_strLength);
  PedestrianElem->SetAttribute("boundingBoxCenterX", p.m_boundingBox.m_strX);
  PedestrianElem->SetAttribute("boundingBoxCenterY", p.m_boundingBox.m_strY);
  PedestrianElem->SetAttribute("boundingBoxCenterZ", p.m_boundingBox.m_strZ);
  return 0;
}

int CTraffic::SavePedestrianVelocities(TiXmlElement* VelocitiesElem,
                                       std::map<std::string, CPedestrianVelocity>& mapVelocities) {
  if (!VelocitiesElem) return -1;

  std::map<std::string, CPedestrianVelocity>::iterator itr = mapVelocities.begin();
  for (; itr != mapVelocities.end(); ++itr) {
    if (itr->second.m_strProfilesTime.size() > 0 || itr->second.m_strProfilesEvent.size() > 0) {
      int nRet = SaveOnePedestrianVelocity(VelocitiesElem, itr->second);
      if (nRet) {
        std::cout << "Velocity save error!" << std::endl;
        assert(false);
      }
    }
  }

  return 0;
}

int CTraffic::SaveOnePedestrianVelocity(TiXmlElement* elemEvent, CPedestrianVelocity& v) {
  if (!elemEvent) return -1;

  if (v.m_strProfilesTime.size() > 0) {
    TiXmlElement* elemTimeVelocity = new TiXmlElement("time_velocity");
    elemTimeVelocity->SetAttribute("id", v.m_strID);
    elemTimeVelocity->SetAttribute("profile", v.m_strProfilesTime);
    elemEvent->LinkEndChild(elemTimeVelocity);
  }

  if (v.m_strProfilesEvent.size() > 0) {
    TiXmlElement* elemEventVelocity = new TiXmlElement("event_velocity");
    elemEventVelocity->SetAttribute("id", v.m_strID);
    elemEventVelocity->SetAttribute("profile", v.m_strProfilesEvent);
    elemEvent->LinkEndChild(elemEventVelocity);
  }

  return 0;
}
int CTraffic::SaveSignlights(TiXmlElement* SignlightsElem, SignLightMap& mapSignlight) {
  if (!SignlightsElem) return -1;

  SignLightMap::iterator itr = mapSignlight.begin();
  for (; itr != mapSignlight.end(); ++itr) {
    TiXmlElement* elemSignlight = new TiXmlElement("signlight");

    int nRet = SaveOneSignlight(elemSignlight, itr->second);
    if (nRet) {
      XML_LOGGER_ERROR("signlight save error!");
      assert(false);
    }

    SignlightsElem->LinkEndChild(elemSignlight);
  }

  return 0;
}

int CTraffic::SaveOneSignlight(TiXmlElement* SignlightElem, CSignLight& l) {
  if (!SignlightElem) return -1;

  SignlightElem->SetAttribute("id", l.m_strID);
  SignlightElem->SetAttribute("routeID", l.m_strRouteID);
  SignlightElem->SetAttribute("start_s", l.m_strStartShift);
  SignlightElem->SetAttribute("start_t", l.m_strStartTime);
  SignlightElem->SetAttribute("l_offset", l.m_strOffset);
  SignlightElem->SetAttribute("time_green", l.m_strTimeGreen);
  SignlightElem->SetAttribute("time_yellow", l.m_strTimeYellow);
  SignlightElem->SetAttribute("time_red", l.m_strTimeRed);
  SignlightElem->SetAttribute("direction", l.m_strDirection);
  SignlightElem->SetAttribute("compliance", l.m_strCompliance);
  SignlightElem->SetAttribute("lane", l.m_strLane);
  SignlightElem->SetAttribute("phase", l.m_strPhase);
  SignlightElem->SetAttribute("status", l.m_strStatus);
  SignlightElem->SetAttribute("plan", l.m_strPlan);
  SignlightElem->SetAttribute("eventId", l.m_strEventId);
  SignlightElem->SetAttribute("junction", l.m_strJunction);
  SignlightElem->SetAttribute("phaseNumber", l.m_strPhasenumber);
  std::string strSignal = "";
  for (int i = 0; i < l.m_strSignalheadVec.size(); i++) {
    strSignal.append(l.m_strSignalheadVec.at(i));
    if (i != l.m_strSignalheadVec.size() - 1) {
      strSignal.append(",");
    }
  }
  SignlightElem->SetAttribute("signalHead", strSignal);

  return 0;
}

int CTraffic::SaveSceneEvents(TiXmlElement* SceneEventsElem, SceneEventMap& mapSceneEvent) {
  if (!SceneEventsElem) return -1;

  auto itr = mapSceneEvent.begin();
  for (; itr != mapSceneEvent.end(); ++itr) {
    TiXmlElement* elemSceneEvent = new TiXmlElement("event");

    int nRet = SaveOneSceneEvent(elemSceneEvent, itr->second);
    if (nRet) {
      XML_LOGGER_ERROR("sceneEvent save error!");
      assert(false);
    }

    SceneEventsElem->LinkEndChild(elemSceneEvent);
  }

  return 0;
}

int CTraffic::SaveOneSceneEvent(TiXmlElement* SceneEventElem, SceneEvent& se) {
  if (!SceneEventElem) return -1;
  SceneEventElem->SetAttribute("id", se.m_id);
  SceneEventElem->SetAttribute("name", se.strName);
#ifdef SCENE_EVENTS_UPGRAD
  se.MergeString();
#else
  se.MergeString1_0();
#endif
  if (SCENE_EVENT_VERSION == "1.1.0.0") {
    if (se.ConditionVec.size() > 0) {
      std::string _typename = std::string("type");
      SceneEventElem->SetAttribute(_typename, se.ConditionVec.at(0).strType.c_str());
      std::string _conditonname = std::string("condition");
      SceneEventElem->SetAttribute(_conditonname, se.ConditionVec.at(0).mergeCondition().c_str());
    }
  } else {
    int index = 1;
    for (auto it : se.ConditionVec) {
      std::string _typename = std::string("type") + std::to_string(index);
      SceneEventElem->SetAttribute(_typename, it.strType.c_str());
      std::string _conditonname = std::string("condition") + std::to_string(index++);
      SceneEventElem->SetAttribute(_conditonname, it.mergeCondition().c_str());
    }
  }
  SceneEventElem->SetAttribute("conditionNumber", se.ConditionVec.size());
  SceneEventElem->SetAttribute("endCondition", se.strEndCondition);
  SceneEventElem->SetAttribute("action", se.strAction);
  SceneEventElem->SetAttribute("info", se.strInfo);
  return 0;
}

int CTraffic::ParseTrafficFlow(TiXmlElement* TrafficFlowElem, CTrafficFlow& traffic_flow) {
  if (!TrafficFlowElem || !TrafficFlowElem->Parent()) {
    return -1;
  }
  try {
    traffic_flow.ParseFromXml(TrafficFlowElem);
  } catch (const std::exception& ex) {
    XML_LOGGER_ERROR("Parse from xml/<trafficFlow> failed: %s", ex.what());
  } catch (...) {
    XML_LOGGER_ERROR("Parse from xml/<trafficFlow> failed: unknown ex");
  }
  return 0;
}

int CTraffic::SaveTrafficFlow(TiXmlElement* TrafficFlowElem, CTrafficFlow& traffic_flow) {
  if (!TrafficFlowElem || !TrafficFlowElem->Parent()) {
    return -1;
  }
  try {
    traffic_flow.SerializeToXml(TrafficFlowElem);
  } catch (const std::exception& ex) {
    XML_LOGGER_ERROR("Serialize to xml/<trafficFlow> failed: %s", ex.what());
  } catch (...) {
    XML_LOGGER_ERROR("Serialize to xml/<trafficFlow> failed: unknown ex");
  }
  return 0;
}
