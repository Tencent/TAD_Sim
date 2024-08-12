/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "mapfile.h"
#include <mapengine/hadmap_codes.h>
#include <mapengine/hadmap_engine.h>
// #include <hadmap.h>
#include <common/coord_trans.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "../../engine/config.h"
#include "../../engine/entity/aabb.h"
#include "../../map_data/lane_data.h"
#include "../../map_data/lane_link.h"
#include "../../map_data/road_data.h"
#include "../../map_data/section_data.h"
#include "common/log/system_logger.h"
#include "common/map_data/map_object.h"
#include "common/utils/misc.h"
#include "common/utils/scoped_cleanup.h"

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
  int nRet = checkEmptyFile(strMapFile);
  if (nRet != HSEC_OK) {
    return nRet;
  }
  nRet = hadmap::hadmapConnect(strMapFile, hadmap::SQLITE, &pMapHandler);
  if (nRet != TX_HADMAP_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s open failed!", strMapFile);
    return -1;
  }

  float fLat = atof(m_strLat.c_str());
  float fLog = atof(m_strLon.c_str());
  float fAlt = atof(m_strAlt.c_str());

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
  int nRet = checkEmptyFile(strMapFile);
  if (nRet != HSEC_OK) {
    return nRet;
  }
  nRet = hadmap::hadmapConnect(strMapFile, hadmap::SQLITE, &pMapHandler);
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
    pLink->Lonlat2Local(m_fLon, m_fLat, m_fAlt);
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
  hadmap::txMapHandle* pMapHandler = NULL;
  SCOPED_CLEANUP({
    if (pMapHandler) hadmap::hadmapClose(&pMapHandler);
  });
  int nRet = 0;

  CAABB aabb;

  nRet = checkEmptyFile(strMapFile);
  if (nRet != HSEC_OK) {
    return nRet;
  }
  // try {

  nRet = hadmap::hadmapConnect(strMapFile, static_cast<hadmap::MAP_DATA_TYPE>(nMapType), &pMapHandler);
  if (nRet != TX_HADMAP_HANDLE_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s open failed!", strMapFile);
    return -1;
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

  hadmap::txRoads::iterator itr = roads.begin();
  int i = 0;
  for (; itr != roads.end(); ++itr) {
    CRoadData* rd = new CRoadData();
    rd->FromTXRoad(*(*itr));
    // rd->Lonlat2Local(m_fLon, m_fLat, m_fAlt);
    m_roads.push_back(rd);

    if (rd->Data().empty()) continue;

    aabb.Expand(rd->AabbLl().Min());
    aabb.Expand(rd->AabbLl().Max());
  }

  SYSTEM_LOGGER_DEBUG("hadmap::getObjects");
  // objects
  std::vector<hadmap::txLaneId> lanids;
  std::vector<hadmap::OBJECT_TYPE> objtypes;
  hadmap::txObjects objs;
  nRet = hadmap::getObjects(pMapHandler, lanids, objtypes, objs);
  if (nRet != TX_HADMAP_DATA_OK && nRet != TX_HADMAP_DATA_EMPTY) {
    SYSTEM_LOGGER_ERROR("hadmap::getObjects failed!");
    // return -1;
  }

  SYSTEM_LOGGER_DEBUG("parse object data");

  ClearMapObjects();

  hadmap::txObjects::iterator objItr = objs.begin();
  for (; objItr != objs.end(); ++objItr) {
    CMapObject* pObj = new CMapObject();
    pObj->FromTxObject(*objItr);
    // pObj->Lonlat2Local(m_fLon, m_fLat, m_fAlt);
    m_mapobjects.push_back(pObj);

    if (pObj->Data().empty()) continue;
    aabb.Expand(pObj->AabbLl().Min());
    aabb.Expand(pObj->AabbLl().Max());
  }

  SYSTEM_LOGGER_DEBUG("hadmap::getLaneLinks");
  // lanelinks
  roadpkid fromRoadId = ROAD_PKID_INVALID;
  roadpkid toRoadId = ROAD_PKID_INVALID;
  hadmap::txLaneLinks links;
  nRet = hadmap::getLaneLinks(pMapHandler, fromRoadId, toRoadId, links);
  if (nRet != TX_HADMAP_DATA_OK && nRet != TX_HADMAP_DATA_EMPTY) {
    SYSTEM_LOGGER_ERROR("hadmap::getLaneLinks failed!");
    return -1;
  }

  SYSTEM_LOGGER_DEBUG("parse lanelink data");
  ClearLaneLinks();

  hadmap::txLaneLinks::iterator linkItr = links.begin();
  for (; linkItr != links.end(); ++linkItr) {
    CLaneLink* pLink = new CLaneLink();
    pLink->FromTxLanelink(*(*linkItr));
    // pLink->Lonlat2Local(m_fLon, m_fLat, m_fAlt);
    m_lanelinks.push_back(pLink);

    if (pLink->Data().empty()) continue;
    aabb.Expand(pLink->AabbLl().Min());
    aabb.Expand(pLink->AabbLl().Max());
  }

  nRet = hadmap::hadmapClose(&pMapHandler);
  if (nRet != TX_HADMAP_HANDLE_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s close failed!", strMapFile);
    return -1;
  }
  /*
} catch (...) {
  SYSTEM_LOGGER_ERROR("parse hadmap %s error!", strMapFile);
  return -1;
}
*/

  SYSTEM_LOGGER_INFO("convert map points to local coordinate.");

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
      CVector3 center = computeRefP(aabb);
      tmp.X() = center.X();
      tmp.Y() = center.Y();
      tmp.Z() = center.Z();
      CoordConvertV2(aabb, tmp);
    }
  }

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

void CMapFile::CalculateBox(CAABB& aabb) {
  CVector3 min = aabb.Min();
  CVector3 max = aabb.Max();
  CVector3 center = computeRefP(aabb);

  m_center[0] = center.X();
  m_center[1] = center.Y();
  m_center[2] = center.Z();
  m_envelop_min[0] = min.X();
  m_envelop_min[1] = min.Y();
  m_envelop_min[2] = min.Z();
  m_envelop_max[0] = max.X();
  m_envelop_max[1] = max.Y();
  m_envelop_max[2] = max.Z();

  SYSTEM_LOGGER_INFO("map:%s center(%f, %f, %f)", m_strMapfile.c_str(), m_center[0], m_center[1], m_center[2]);
  SYSTEM_LOGGER_INFO("map:%s min(%f, %f, %f)", m_strMapfile.c_str(), m_envelop_min[0], m_envelop_min[1],
                     m_envelop_min[2]);
  SYSTEM_LOGGER_INFO("map:%s max(%f, %f, %f)", m_strMapfile.c_str(), m_envelop_max[0], m_envelop_max[1],
                     m_envelop_max[2]);
}

CVector3 CMapFile::computeRefP(const CAABB& aabb) {
  CVector3 min = aabb.Min();
  CVector3 max = aabb.Max();
  CVector3 center = min * 0.5 + max * 0.5;
  return center;
}

void CMapFile::CoordConvertV2(CAABB& aabb, CPoint3D& refPoint) {
  CalculateBox(aabb);
  m_refPt = refPoint;

  SYSTEM_LOGGER_INFO("map ref point(%f, %f, %f)", refPoint.X(), refPoint.Y(), refPoint.Z());

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
