/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/mapfile.h"

#include <mapengine/hadmap_codes.h>
#include <mapengine/hadmap_engine.h>
// #include <hadmap.h>
#include <common/coord_trans.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "common/log/system_logger.h"
#include "common/map_data/map_object.h"
#include "common/utils/misc.h"
#include "common/utils/scoped_cleanup.h"
#include "engine/config.h"
#include "engine/entity/aabb.h"
#include "map_data/lane_data.h"
#include "map_data/lane_link.h"
#include "map_data/road_data.h"
#include "map_data/section_data.h"

#ifndef M_PI
#  define M_PI 3.1415926
#endif  // !M_PI

void CMapFile::ClearRoads() {
  RoadVec::iterator tmpItr = m_roads.begin();

  for (; tmpItr != m_roads.end(); ++tmpItr) {
    delete (*tmpItr);
  }
  m_roads.clear();
}

void CMapFile::ClearLanes() {
  LaneVec::iterator tmpItr = m_lanes.begin();

  for (; tmpItr != m_lanes.end(); ++tmpItr) {
    delete (*tmpItr);
  }
  m_lanes.clear();
}

void CMapFile::ClearLaneBoundaries() {
  LaneBoundaryVec::iterator tmpItr = m_laneboundaries.begin();

  for (; tmpItr != m_laneboundaries.end(); ++tmpItr) {
    delete (*tmpItr);
  }
  m_laneboundaries.clear();
}

void CMapFile::ClearLaneLinks() {
  LaneLinkVec::iterator tmpItr = m_lanelinks.begin();

  for (; tmpItr != m_lanelinks.end(); ++tmpItr) {
    delete (*tmpItr);
  }

  m_lanelinks.clear();
}

void CMapFile::ClearMapObjects() {
  MapObjectVec::iterator tmpItr = m_mapobjects.begin();

  for (; tmpItr != m_mapobjects.end(); ++tmpItr) {
    delete (*tmpItr);
  }
  m_mapobjects.clear();
}

int CMapFile::ParseMapOld(const char* strMapFile) {
  if (!strMapFile) {
    return -1;
  }
  hadmap::txMapHandle* pMapHandler = NULL;
  int nRet = hadmap::hadmapConnect(strMapFile, hadmap::SQLITE, &pMapHandler, false);
  if (nRet != TX_HADMAP_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s open failed!", strMapFile);
    return -1;
  }

  float fLat = std::atof(m_strLat.c_str());
  float fLog = std::atof(m_strLon.c_str());
  float fAlt = std::atof(m_strAlt.c_str());

  hadmap::txPoint p;
  p.x = fLog;
  p.y = fLat;
  p.z = fAlt;
  hadmap::txLanePtr pLane;
  hadmap::txRoadPtr pRoad;
  hadmap::txSectionPtr pSec;

  nRet = hadmap::getLane(pMapHandler, p, pLane);
  if (nRet != TX_HADMAP_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s get lane failed!", strMapFile);
    return -1;
  }

  nRet = hadmap::getLane(pMapHandler, p, pLane);
  if (nRet != TX_HADMAP_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s get lane failed!", strMapFile);
    return -1;
  }

  /*while (pLane)
  {
          pLane = pLane->getNext();
  }*/

  nRet = hadmap::hadmapClose(&pMapHandler);
  if (nRet != TX_HADMAP_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s close failed!", strMapFile);
    return -1;
  }

  return 0;
}

int CMapFile::ParseMap(const char* strMapFile) {
  if (!strMapFile) {
    return -1;
  }
  hadmap::txMapHandle* pMapHandler = NULL;
  int nRet = hadmap::hadmapConnect(strMapFile, hadmap::SQLITE, &pMapHandler);
  if (nRet != TX_HADMAP_HANDLE_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s open failed!", strMapFile);
    return -1;
  }

  // roads
  bool bWholeData = false;
  hadmap::txRoads roads;
  nRet = hadmap::getRoads(pMapHandler, &bWholeData, roads);
  if (nRet != TX_HADMAP_DATA_OK) {
    SYSTEM_LOGGER_ERROR("hadmap::getRoads failed!");
    return -1;
  }

  ClearRoads();

  hadmap::txRoads::iterator itr = roads.begin();
  int i = 0;
  for (; itr != roads.end(); ++itr) {
    CRoadData* rd = new CRoadData();
    rd->FromTXRoad(*(*itr));
    rd->Lonlat2ENU(m_fLon, m_fLat, m_fAlt);
    m_roads.push_back(rd);
  }

  // objects
  std::vector<hadmap::txLaneId> lanids;
  std::vector<hadmap::OBJECT_TYPE> objtypes;
  hadmap::txObjects objs;
  nRet = hadmap::getObjects(pMapHandler, lanids, objtypes, objs);
  if (nRet != TX_HADMAP_DATA_OK) {
    SYSTEM_LOGGER_ERROR("hadmap::getObjects failed!");
    return -1;
  }
  hadmap::txObjects::iterator objItr = objs.begin();
  for (; objItr != objs.end(); ++objItr) {
    CMapObject* pObj = new CMapObject();
    pObj->FromTxObject(*objItr);
    pObj->Lonlat2ENU(m_fLon, m_fLat, m_fAlt);
    m_mapobjects.push_back(pObj);
  }

  // lanelinks
  roadpkid fromRoadId = 0;
  roadpkid toRoadId = 0;
  hadmap::txLaneLinks links;
  nRet = hadmap::getLaneLinks(pMapHandler, fromRoadId, toRoadId, links);
  if (nRet != TX_HADMAP_DATA_OK) {
    SYSTEM_LOGGER_ERROR("hadmap::getLaneLinks failed!");
    return -1;
  }
  hadmap::txLaneLinks::iterator linkItr = links.begin();
  for (; linkItr != links.end(); ++linkItr) {
    CLaneLink* pLink = new CLaneLink();
    pLink->FromTxLanelink(*(*linkItr));
    pLink->Lonlat2ENU(m_fLon, m_fLat, m_fAlt);
    m_lanelinks.push_back(pLink);
  }

  nRet = hadmap::hadmapClose(&pMapHandler);
  if (nRet != TX_HADMAP_HANDLE_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s close failed!", strMapFile);
    return -1;
  }

  return 0;
}

int CMapFile::ParseMapV2(const char* strMapFile, bool bReadHadmapConfig /* = true*/) {
  if (!strMapFile) {
    SYSTEM_LOGGER_ERROR("map file name is null");
    return -1;
  }
  boost::filesystem::path pathMapFile = strMapFile;
  if (!boost::filesystem::exists(pathMapFile)) {
    SYSTEM_LOGGER_ERROR("map file %s not exist!", strMapFile);
    return -1;
  }
  std::string strExt = pathMapFile.extension().string();
  if (kHadmapTypeDict.count(strExt) <= 0) {
    SYSTEM_LOGGER_ERROR("unknown map type: %s!", strMapFile);
    return -1;
  }
  int nMapType = kHadmapTypeDict.at(strExt);

  SYSTEM_LOGGER_DEBUG("hadmap::hadmapConnect for file: %s, type: %d", strMapFile, nMapType);
  // hadmap::txMapHandle* pMapHandler = NULL;
  pMapHandler = NULL;
  int nRet = 0;

  CAABB aabb;

  nRet = checkEmptyFile(strMapFile);
  if (nRet != HSEC_OK) {
    return nRet;
  }
  try {
    nRet = hadmap::hadmapConnect(strMapFile, static_cast<hadmap::MAP_DATA_TYPE>(nMapType), &pMapHandler, false);
    if (nRet != TX_HADMAP_HANDLE_OK) {
      SYSTEM_LOGGER_ERROR("hadmap file %s open failed!", strMapFile);
      return -1;
    }
    hadmap::txOdHeaderPtr headerPtr;
    nRet = hadmap::getHeader(pMapHandler, headerPtr);

    if (headerPtr != NULL) {
      if (headerPtr->getVendor() == "tadsim") {
        m_version = "tadsim v1.0";
      } else if (headerPtr->getVendor() == "tadsim v2.0") {
        m_version = "tadsim v2.0";
      } else {
        m_version = "three party";
      }
    }
    SYSTEM_LOGGER_DEBUG("hadmap::getRoads");
    // roads
    bool bWholeData = false;
    hadmap::txRoads roads;
    nRet = hadmap::getRoads(pMapHandler, &bWholeData, roads);
    if (nRet != TX_HADMAP_DATA_OK && nRet != TX_HADMAP_DATA_EMPTY) {
      SYSTEM_LOGGER_ERROR("hadmap::getRoads failed!");
      return -1;
    }
    SYSTEM_LOGGER_DEBUG("parse road data");
    ClearRoads();
    int i = 0;
    for (auto& iter : roads) {
      CRoadData* rd = new CRoadData();
      rd->FromTXRoad(*iter);
      // rd->Lonlat2ENU(m_fLon, m_fLat, m_fAlt);
      m_roads.push_back(rd);

      if (rd->Data().empty()) continue;
      aabb.Expand(rd->AabbLl().Min());
      aabb.Expand(rd->AabbLl().Max());
    }

    SYSTEM_LOGGER_DEBUG("hadmap::getObjects");
    // objects
    std::vector<hadmap::txLaneId> lanids;
    std::vector<hadmap::OBJECT_TYPE> objtypes;
    std::map<hadmap::junctionpkid, std::vector<LogicLights>> junctionId2Lights;
    std::map<hadmap::junctionpkid, std::vector<roadpkid>> junctionid2Roads;
    std::map<hadmap::roadpkid, std::vector<int>> road2lightid;
    hadmap::txObjects objs;
    nRet = hadmap::getObjects(pMapHandler, lanids, objtypes, objs);
    if (nRet != TX_HADMAP_DATA_OK && nRet != TX_HADMAP_DATA_EMPTY) {
      SYSTEM_LOGGER_ERROR("hadmap::getObjects failed!");
      // return -1;
    }

    SYSTEM_LOGGER_DEBUG("parse object data");

    ClearMapObjects();
    CMapObject::LogicLightsMap.clear();
    hadmap::txObjects::iterator objItr = objs.begin();
    for (; objItr != objs.end(); ++objItr) {
      CMapObject* pObj = new CMapObject();
      pObj->FromTxObject(*objItr);
      // get lanePtr
      hadmap::txLanePtr _cur_lane = NULL;
      hadmap::getLane(pMapHandler, (*objItr)->getPos(), _cur_lane);
      if (_cur_lane) {
        pObj->SetGeomsFromRoad(_cur_lane, *objItr);
      }
      m_mapobjects.push_back(pObj);
      if (pObj->Type() == HADMAP_OBJECT_TYPE_TrafficLights) {
        std::map<std::string, std::string> _userdata;
        (*objItr)->getUserData(_userdata);
        LogicLights _light;
        _light.m_id = pObj->Id();
        _light.m_type = pObj->Type();
        _light.m_name = pObj->Name();
        _light.m_dLength = pObj->Length();
        _light.m_dWidth = pObj->Width();
        _light.m_dHeight = pObj->Height();
        _light.m_dGroundHeight = pObj->GroundHeight();
        _light.m_dRoll = pObj->Roll();
        _light.m_dYaw = pObj->Yaw();
        _light.m_dPitch = pObj->Pitch();
        _light.m_strType = "trafficLight";
        _light.m_strSubType = pObj->StrSubType();
        _light.m_position = pObj->Position();
        _light.m_nGeomCount = pObj->GeomCount();
        _light.m_points = pObj->Data();
        _light.m_user_data = _userdata;
        if (_userdata.find("relate_control") != _userdata.end() && _userdata.find("relate_road") != _userdata.end()) {
          // relate_control
          int id = std::atoi(_userdata.find("relate_control")->second.c_str());
          int controlRoadId = 0;
          hadmap::txPoint pointa = (*objItr)->getPos();
          // coord_trans_api::mercator2lonlat(pointa.x, pointa.y);
          hadmap::txLanePtr _lane;
          int nRet = hadmap::getLane(pMapHandler, pointa, _lane, 11);  // set most distance 11
          if (nRet != TX_HADMAP_DATA_OK) {
            continue;
          }
          // relate_road
          if (_userdata.find("relate_road") != _userdata.end()) {
            std::vector<std::string> controlRoadIdVec;

            _light.m_controlroadid = std::atoi(_userdata.find("relate_road")->second.c_str());
            boost::split(controlRoadIdVec, _userdata.find("relate_road")->second.c_str(), boost::is_any_of("|"),
                         boost::token_compress_on);
            for (auto itRid : controlRoadIdVec) {
              if (road2lightid.find(std::atoi(itRid.c_str())) != road2lightid.end()) {
                road2lightid[std::atoi(itRid.c_str())].push_back(_light.m_id);
              } else {
                std::vector<int> lightsvec;
                lightsvec.push_back(_light.m_id);
                road2lightid[std::atoi(itRid.c_str())] = lightsvec;
              }
              if (junctionid2Roads.find(id) != junctionid2Roads.end()) {
                junctionid2Roads[id].push_back(std::atoi(itRid.c_str()));
              } else {
                std::vector<roadpkid> roadPkid;
                roadPkid.push_back(std::atoi(itRid.c_str()));
                junctionid2Roads[id] = (roadPkid);
              }
            }

          } else {
            continue;
          }
          if (junctionId2Lights.find(id) != junctionId2Lights.end()) {
            junctionId2Lights[id].push_back(_light);
          } else {
            std::vector<LogicLights> lightVec;
            lightVec.push_back(_light);
            junctionId2Lights[id] = lightVec;
            // continue;
          }
          if (m_roadJunctionId.find(id) != m_roadJunctionId.end()) {
            m_roadJunctionId.find(id)->second[_light.m_id] = _lane->getRoadId();
          } else {
            std::map<int, int> map;
            map[_light.m_id] = _lane->getRoadId();
            m_roadJunctionId[id] = map;
          }
        }
      }

      if (pObj->Data().empty()) continue;
    }
    int id_init = 10001;
    if (bReadHadmapConfig) {
      for (auto it : junctionId2Lights) {
        junctionpkid junctionid = it.first;
        int nRoadIds = 0;
        if (junctionid2Roads.find(junctionid) != junctionid2Roads.end()) {
          nRoadIds = junctionid2Roads.find(junctionid)->second.size();
        }
        std::set<int> controlRoadIdSize(junctionid2Roads.find(junctionid)->second.begin(),
                                        junctionid2Roads.find(junctionid)->second.end());
        // junctionid
        int index = 0;
        std::map<int, std::shared_ptr<CSignLight>> logicalid2id;
      }
    }

    // junctions
    hadmap::txJunctions junctions;
    hadmap::getJunctions(pMapHandler, junctions);

    // lanelinks
    SYSTEM_LOGGER_DEBUG("hadmap::getLaneLinks");
    roadpkid fromRoadId = ROAD_PKID_INVALID;
    roadpkid toRoadId = ROAD_PKID_INVALID;
    hadmap::txLaneLinks links;
    if (bReadHadmapConfig) {
      for (auto itjun : junctions) {
        std::vector<lanelinkpkid> lanelinkiDs;
        itjun->getLaneLink(lanelinkiDs);
        std::map<roadpkid, bool> rIdRights;
        for (auto itLaneLink : lanelinkiDs) {
          hadmap::txLaneLinkPtr lanelinkPtr = NULL;
          hadmap::getLaneLink(pMapHandler, itLaneLink, lanelinkPtr);
          lanelinkPtr->setJunctionId(itjun->getId());
          links.push_back(lanelinkPtr);
          if (lanelinkPtr != NULL) {
            rIdRights[lanelinkPtr->fromRoadId()] = static_cast<bool>(lanelinkPtr->fromLaneId() < 0);
          }
        }
        int nParse = rIdRights.size();
        int index = 0;
        for (auto itroad : rIdRights) {
          std::shared_ptr<CSignLight> _light(new CSignLight);
          _light->m_strID = std::to_string(id_init);
          _light->m_strRouteID = std::to_string(id_init);
          // three lights
          id_init += 10;
          _light->m_strStartShift = std::to_string(0);
          _light->m_strStartTime = std::to_string(index * 9 + 1);
          _light->m_strOffset = std::to_string(0);
          _light->m_strTimeGreen = std::to_string(5);
          _light->m_strTimeYellow = std::to_string(2);
          _light->m_strTimeRed = std::to_string((nParse - 1) * 9 + 2);
          _light->m_strDirection = std::to_string(180);
          _light->m_strPhase = "A";
          _light->m_strStatus = "Activated";
          _light->m_strLane = "ALL";
          _light->m_strEventId = "";
          if (road2lightid.find(itroad.first) != road2lightid.end()) {
            for (auto it : road2lightid.find(itroad.first)->second) {
              _light->m_strSignalheadVec.push_back(std::to_string(it));
            }
            _light->m_strStatus = "Activated";
          }

          _light->m_strPlan = "0";
          _light->m_strJunction = std::to_string(itjun->getId());
          _light->m_strPhasenumber = std::to_string(index + 1);
          // get roadCenterPoint
          hadmap::txRoadPtr roadPtr;
          hadmap::getRoad(pMapHandler, itroad.first, true, roadPtr);
          hadmap::txPoint txpoint;
          bool fRet = getRoadEndCenterPoint(roadPtr, txpoint, itroad.second);
          std::map<std::string, std::vector<int>> parsetolanes;
          std::map<std::string, std::vector<int>> parsetolanelinks;
          getRoadLaneLinks(roadPtr, parsetolanes, parsetolanelinks, itroad.second);
          if (!fRet) {
            continue;
          }
          hadmap::txLanePtr lanePtr;
          _light->m_frouteLon = txpoint.x;
          _light->m_frouteLat = txpoint.y;
          _light->m_frouteAlt = txpoint.z;
          _light->m_parsetolanes = parsetolanes;
          _light->m_parsetolanelinks = parsetolanelinks;
          this->m_lights.push_back(_light);
          index++;
        }
      }
    }
    SYSTEM_LOGGER_DEBUG("parse lanelink data");
    ClearLaneLinks();
    hadmap::txLaneLinks::iterator linkItr = links.begin();
    // compute hadmap center
    for (; linkItr != links.end(); ++linkItr) {
      CLaneLink* pLink = new CLaneLink();
      pLink->FromTxLanelink(*(*linkItr));
      m_lanelinks.push_back(pLink);
      if (pLink->Data().empty()) continue;
      aabb.Expand(pLink->AabbLl().Min());
      aabb.Expand(pLink->AabbLl().Max());
    }
  } catch (...) {
    SYSTEM_LOGGER_ERROR("parse hadmap %s error!", strMapFile);
    return -1;
  }

  SYSTEM_LOGGER_INFO("convert map points to local coordinate.");
#ifdef __CLOUD_HADMAP_SERVER__
  {
    boost::filesystem::path p = strMapFile;

    tagHadmapAttr* hadmapAttr = CEngineConfig::Instance().HadmapAttr(p.filename().string());

    if (hadmapAttr) {
      SYSTEM_LOGGER_INFO("using local hadmap config ref point. %s", strMapFile);
      CPoint3D tmp;
      tmp.X() = hadmapAttr->m_dRefLon;
      tmp.Y() = hadmapAttr->m_dRefLat;
      tmp.Z() = hadmapAttr->m_dRefAlt;
      CoordConvertV2(aabb, tmp);

    } else {
      SYSTEM_LOGGER_INFO("using center of map as ref point, %s", strMapFile);

      CPoint3D tmp;
      CVector3 center = aabb.Min() * 0.5 + aabb.Max() * 0.5;
      tmp.X() = center.X();
      tmp.Y() = center.Y();
      tmp.Z() = center.Z();
      CoordConvertV2(aabb, tmp);
    }
  }
#else
  if (bReadHadmapConfig) {
    boost::filesystem::path p = strMapFile;
    CPoint3D refPoint;
    tagHadmapAttr* hadmapAttr = CEngineConfig::Instance().HadmapAttr(p.filename().string());
    if (!hadmapAttr) {
      SYSTEM_LOGGER_ERROR("hadmap file not exist in config file!");
      return -1;
    }
    refPoint.X() = hadmapAttr->m_dRefLon;
    refPoint.Y() = hadmapAttr->m_dRefLat;
    refPoint.Z() = hadmapAttr->m_dRefAlt;
    CoordConvertV2(aabb, refPoint);
  } else {
    CalculateBox(aabb);
  }
#endif
  SYSTEM_LOGGER_DEBUG("parse map data finished!");

  return 0;
}

void CMapFile::ClearAll() {
  RoadVec::iterator itr = m_roads.begin();
  for (; itr != m_roads.end(); ++itr) {
    CRoadData::Sections& sec = (*itr)->Secs();
    CRoadData::Sections::iterator secItr = sec.begin();
    // section
    for (; secItr != sec.end(); ++secItr) {
      Lanes& lanes = (*secItr)->Data();
      Lanes::iterator lanItr = lanes.begin();
      // lanes
      for (; lanItr != lanes.end(); ++lanItr) {
        CLaneBoundaryData** pBoundary = (*lanItr)->Boundarys();
        // lane boundary
        for (int i = 0; i < 2; ++i) {
          if (pBoundary + i) {
            delete pBoundary[i];
          }
        }

        delete (*lanItr);
      }

      delete (*secItr);
    }

    delete (*itr);
  }

  ClearMapObjects();
  ClearLaneLinks();
}

int CMapFile::Disconect() {
  if (pMapHandler) {
    return hadmap::hadmapClose(&pMapHandler);
  }
}
void CMapFile::CalculateBox(CAABB& aabb) {
  auto min = aabb.Min();
  auto max = aabb.Max();
  auto center = min * 0.5 + max * 0.5;

  m_center[0] = center.X();
  m_center[1] = center.Y();
  m_center[2] = center.Z();
  m_envelop_min[0] = min.X();
  m_envelop_min[1] = min.Y();
  m_envelop_min[2] = min.Z();
  m_envelop_max[0] = max.X();
  m_envelop_max[1] = max.Y();
  m_envelop_max[2] = max.Z();

  SYSTEM_LOGGER_INFO("map:%s center(%.8f, %.8f, %.8f)", m_strMapfile.c_str(), m_center[0], m_center[1], m_center[2]);
  SYSTEM_LOGGER_INFO("map:%s min(%f, %f, %f)", m_strMapfile.c_str(), m_envelop_min[0], m_envelop_min[1],
                     m_envelop_min[2]);
  SYSTEM_LOGGER_INFO("map:%s max(%f, %f, %f)", m_strMapfile.c_str(), m_envelop_max[0], m_envelop_max[1],
                     m_envelop_max[2]);
}

void CMapFile::CoordConvertV2(CAABB& aabb, CPoint3D& refPoint) {
  CalculateBox(aabb);
  m_refPt = refPoint;

  SYSTEM_LOGGER_INFO("map ref point(%.8f, %.8f, %.8f)", refPoint.X(), refPoint.Y(), refPoint.Z());

  RoadVec::iterator rItr = m_roads.begin();
  for (; rItr != m_roads.end(); ++rItr) {
    (*rItr)->Lonlat2ENU(refPoint.X(), refPoint.Y(), refPoint.Z());
  }

  LaneLinkVec::iterator llItr = m_lanelinks.begin();
  for (; llItr != m_lanelinks.end(); ++llItr) {
    (*llItr)->Lonlat2ENU(refPoint.X(), refPoint.Y(), refPoint.Z());
  }

  MapObjectVec::iterator moItr = m_mapobjects.begin();
  for (; moItr != m_mapobjects.end(); ++moItr) {
    (*moItr)->Lonlat2ENU(refPoint.X(), refPoint.Y(), refPoint.Z());
  }
}

bool CMapFile::getRoadEndCenterPoint(hadmap::txRoadPtr roadPtr, hadmap::txPoint& endp, bool isRight) {
  // control_road
  if (roadPtr == NULL) return false;
  int nSections = roadPtr->getSections().size();
  if (nSections == 0) return false;
  hadmap::txSectionPtr lastSection = roadPtr->getSections().back();
  if (!lastSection) return false;

  std::vector<hadmap::txLaneId> laneids;
  if (isRight) {  // get right lanes
    laneids = lastSection->getRightLanes();
  } else {  // get left lanes
    laneids = lastSection->getLeftLanes();
  }
  if (laneids.size() < 1) {
    SYSTEM_LOGGER_ERROR("Get road end center point, right lanes size less than 1");
    return false;
  }

  hadmap::txLaneBoundaryPtr leftPtr = lastSection->getLane(laneids.front().laneId)->getLeftBoundary();
  hadmap::txLaneBoundaryPtr rightPtr = lastSection->getLane(laneids.back().laneId)->getRightBoundary();
  if (!leftPtr || !rightPtr) {
    SYSTEM_LOGGER_ERROR("Boundary is empty");
    return false;
  }

  // curve
  const hadmap::txLineCurve* pLineCurve_L = dynamic_cast<const hadmap::txLineCurve*>(leftPtr->getGeometry());
  const hadmap::txLineCurve* pLineCurve_R = dynamic_cast<const hadmap::txLineCurve*>(rightPtr->getGeometry());
  if (!pLineCurve_L || !pLineCurve_R) {
    SYSTEM_LOGGER_ERROR("LineCurve is empty");
    return false;
  }
  if (isRight) {
    hadmap::txPoint PointA = pLineCurve_L->getPoint(leftPtr->getGeometry()->getLength() - 2);
    hadmap::txPoint PointB = pLineCurve_R->getPoint(rightPtr->getGeometry()->getLength() - 2);
    endp = hadmap::txPoint((PointA.x + PointB.x) / 2.0, (PointA.y + PointB.y) / 2.0, (PointA.z + PointB.z) / 2.0);
  } else {
    hadmap::txPoint PointA = pLineCurve_L->getPoint(2.0);
    hadmap::txPoint PointB = pLineCurve_R->getPoint(2.0);
    endp = hadmap::txPoint((PointA.x + PointB.x) / 2.0, (PointA.y + PointB.y) / 2.0, (PointA.z + PointB.z) / 2.0);
  }
  return true;
}

void CMapFile::getRoadLaneLinks(const hadmap::txRoadPtr roadPtr, std::map<std::string, std::vector<int>>& parsetolanes,
                                std::map<std::string, std::vector<int>>& linkmap,
                                bool fromLaneId /*true = fromLaneId < 0*/) {
  if (!roadPtr) return;
  roadpkid fromroadid = roadPtr->getId();
  roadpkid toroadid = ROAD_PKID_INVALID;
  hadmap::txLaneLinks lanelinks;
  hadmap::getLaneLinks(pMapHandler, fromroadid, toroadid, lanelinks);
  for (auto it : lanelinks) {
    if ((fromLaneId && it->fromLaneId() > 0) || (!fromLaneId && it->fromLaneId() < 0)) {
      continue;
    }
    double startYaw = 0;
    if (it->getGeometry() && !it->getGeometry()->empty()) {
      const hadmap::txLineCurve* pLinecurve = dynamic_cast<const hadmap::txLineCurve*>(it->getGeometry());
      startYaw = pLinecurve->yaw(0);
      int nSize = pLinecurve->size();
      double endYaw = pLinecurve->yaw(pLinecurve->size() - 2);
      double yawDiff = (endYaw - startYaw);
      if (yawDiff < -180.0) {
        yawDiff += 360.0;
      }
      if (yawDiff > 180) {
        yawDiff -= 360.0;
      }
      std::string phase = "";
      if (std::abs(yawDiff) < 45.0) {
        phase = "T";
      } else if (yawDiff >= 45 && yawDiff < 150) {
        phase = "L";
      } else if (yawDiff >= 150 && yawDiff <= 180) {
        phase = "L0";
      } else if (yawDiff > (-150) && yawDiff <= (-45)) {
        phase = "R";
      } else {
        phase = "R0";
      }
      parsetolanes[phase].emplace_back(it->fromLaneId());
      linkmap[phase].emplace_back(it->getId());
    }
  }
}

double CMapFile::queryHeadAngle(const double lon, const double lat) {
  hadmap::txPoint pttmp(lon, lat, -9999);
  hadmap::txLanePtr ptrLane = NULL;
  if (!pMapHandler) {
    return 0.0;
  }
  hadmap::getLane(pMapHandler, pttmp, ptrLane);
  if (ptrLane) {
    double s = 0;
    double t = 0;
    double yaw = 0;
    ptrLane->xy2sl(lon, lat, s, t, yaw);
    return yaw;
  }
  return 0.0;
}

void CMapFile::CalulatePoints(hadmap::txPoint& PA, hadmap::txPoint& PB, int& num,
                              std::vector<hadmap::txPoint>& Points) {
  if (num < 1) {
    Points.push_back(PA);
    Points.push_back(PB);
    return;
  }

  double step_x = (PA.x - PB.x) / (num + 1);

  double step_y = (PA.y - PB.y) / (num + 1);

  double step_z = (PA.z - PB.z) / (num + 1);

  for (int i = 1; i <= num; i++) {
    hadmap::txPoint temp;
    temp.x = PB.x + step_x * i;
    temp.y = PB.y + step_y * i;
    temp.z = PB.z + step_z * i;

    Points.push_back(temp);
  }
}
