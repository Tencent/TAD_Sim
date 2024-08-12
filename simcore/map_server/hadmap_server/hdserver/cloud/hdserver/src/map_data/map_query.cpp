/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "map_query.h"
#include <common/coord_trans.h>
#include <hadmap.h>
#include <mapengine/hadmap_codes.h>
#include <mapengine/hadmap_engine.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/utils/misc.h"
#include "lane_data.h"

static bool useLocalENU = true;
static bool useRanges = false;

CMapQuery::CMapQuery() {
  m_strMapFile = "";
  m_initialized = false;
  m_pMapHandler = nullptr;
  m_dRefLon = 0;
  m_dRefLat = 0;
  m_dRefAlt = 0;
  m_mapType = hadmap::SQLITE;
}

CMapQuery::~CMapQuery() { Clear(); }

void CMapQuery::Clear() {
  hadmap::txLaneMap::iterator itr = m_mapLanes.begin();
  for (; itr != m_mapLanes.end(); ++itr) {
  }
  m_mapLanes.clear();
}

void CMapQuery::SetMapPath(const char* strPath) {
  m_strMapFile = strPath;

  boost::filesystem::path pathMapFile = strPath;
  std::string strExt = pathMapFile.extension().string();
  if (kHadmapTypeDict.count(strExt) <= 0) {
    SYSTEM_LOGGER_ERROR("unknown map type: %s!", strPath);
    return;
  }
  m_mapType = kHadmapTypeDict.at(strExt);
}

int CMapQuery::Init() {
  int nRet = checkEmptyFile(m_strMapFile);
  if (nRet != HSEC_OK) {
    return nRet;
  }

  nRet = hadmap::hadmapConnect(m_strMapFile.c_str(), static_cast<hadmap::MAP_DATA_TYPE>(m_mapType), &m_pMapHandler);

  if (nRet != TX_HADMAP_HANDLE_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s open failed!", m_strMapFile.c_str());
    return -1;
  }

  m_initialized = true;
  return 0;
}

void CMapQuery::SetRefPoint(double x, double y, double z) {
  m_dRefLon = x;
  m_dRefLat = y;
  m_dRefAlt = z;
}

int CMapQuery::Deinit() {
  int nRet = hadmap::hadmapClose(&m_pMapHandler);
  if (nRet != TX_HADMAP_HANDLE_OK) {
    SYSTEM_LOGGER_ERROR("hadmap file %s close failed!", m_strMapFile.c_str());
    return -1;
  }
  m_initialized = false;
  return 0;
}

int CMapQuery::QueryLaneLink(double dLon, double dLat, lanelinkpkid& lanelinkid) {
  hadmap::txLaneLinkPtr lanelinkPtr;
  int nRet = GetNearbyLaneLink(dLon, dLat, lanelinkPtr);
  if (nRet != 0) {
    return -1;
  }

  if (lanelinkPtr != nullptr) {
    lanelinkid = lanelinkPtr->getId();
    return 0;
  }

  return 1;
}

int CMapQuery::QueryLaneLink(double dLon, double dLat, lanelinkpkid& lanelinkid, double& dLanelinkLon,
                             double& dLanelinkLat, double& dLanelinkAlt, double& dDist, double& dOffset) {
  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  hadmap::txPoint txPtPaddle;
  hadmap::txLaneLinkPtr lanelinkPtr;
  dLanelinkLon = -999;
  dLanelinkLat = -999;
  dLanelinkAlt = 0;
  dDist = 0.0;
  dOffset = 0;

  int nRet = GetNearbyLaneLink(dLon, dLat, lanelinkPtr);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("hadmap::getLaneLink(): txPt: (%f, %f) error!", dLon, dLat);
    return -1;
  }

  if (lanelinkPtr != nullptr) {
    lanelinkid = lanelinkPtr->getId();

    SYSTEM_LOGGER_INFO("hadmap::getLaneLink(): txPt: (%f, %f) return lanelink %u!", dLon, dLat, lanelinkid);

    hadmap::txPoint txPtPaddle;
    dDist = GetPassedDistance(lanelinkPtr->getGeometry(), txPt, txPtPaddle);
    dLanelinkLon = txPtPaddle.x;
    dLanelinkLat = txPtPaddle.y;
    dLanelinkAlt = txPtPaddle.z;

    SYSTEM_LOGGER_INFO("lanelink %u txPt: (%f, %f), txPaddle:(%f, %f)!", lanelinkid, dLon, dLat, txPtPaddle.x,
                       txPtPaddle.y);

    CPoint3D pt(txPt);
    CPoint3D ptCenter(txPtPaddle);

    LonlatToLocal(pt.X(), pt.Y(), pt.Z());
    LonlatToLocal(ptCenter.X(), ptCenter.Y(), ptCenter.Z());
    pt.Z() = 0;
    ptCenter.Z() = 0;

    dOffset = pt.distance(ptCenter);

    int nDir = JudgeSign(lanelinkPtr->getGeometry(), -1, dDist, pt, ptCenter);

    if (nDir == 0) {
      return -1;
    }
    dOffset *= nDir;
    return 0;
  }

  return 1;
}

int CMapQuery::QuerySection(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID) {
  /*hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  hadmap::txLanePtr lanePtr;

  int nRet = hadmap::getLane(m_pMapHandler, txPt, lanePtr);
  if (nRet != TX_HADMAP_DATA_OK)
  {
          SYSTEM_LOGGER_INFO("get lane error!");
          return -1;
  }*/
  hadmap::txLanePtr lanePtr;
  int nRet = GetNearbyLane(dLon, dLat, lanePtr);
  if (nRet != 0) {
    return -1;
  }

  if (lanePtr != nullptr) {
    uRoadID = lanePtr->getRoadId();
    uSecID = lanePtr->getSectionId();
    laneID = lanePtr->getId();
    return 0;
  }

  return 1;
}

int CMapQuery::GetShiftAndOffset(hadmap::txLanePtr& pLane, hadmap::txPoint& txPt, double& dDistance, double& dOffset) {
  dDistance = -999;
  dOffset = -999;
  const hadmap::txLineCurve* pLineCurve = dynamic_cast<const hadmap::txLineCurve*>(pLane->getGeometry());
  hadmap::txPoint txPtPaddle;

  if (pLineCurve) {
    dDistance = pLineCurve->getPassedDistance(txPt, txPtPaddle, hadmap::COORD_WGS84);

    CPoint3D pt(txPt);
    CPoint3D ptCenter(txPtPaddle);

    LonlatToLocal(pt.X(), pt.Y(), pt.Z());
    LonlatToLocal(ptCenter.X(), ptCenter.Y(), ptCenter.Z());
    pt.Z() = 0;
    ptCenter.Z() = 0;

    dOffset = pt.distance(ptCenter);

    return 0;
  }

  SYSTEM_LOGGER_WARN("GetShiftAndOffset error!");

  return -1;
}

double CMapQuery::GetLaneWidth(roadpkid uRoadID, sectionpkid uSecID, lanepkid laneID, double dLon, double dLat) {
  hadmap::txLanePtr lanePtr;
  hadmap::txLaneId laneId;
  laneId.roadId = uRoadID;
  laneId.sectionId = uSecID;
  laneId.laneId = laneID;

  int nRet = hadmap::getLane(m_pMapHandler, laneId, lanePtr);
  if (nRet != TX_HADMAP_DATA_OK) {
    SYSTEM_LOGGER_INFO("get lane error!");
    return -1;
  }

  if (lanePtr != nullptr) {
    hadmap::txLaneBoundaryPtr ptrLeftLB = lanePtr->getLeftBoundary();
    if (ptrLeftLB == nullptr) {
      hadmap::laneboundarypkid leftLaneBoundaryID = lanePtr->getLeftBoundaryId();
      nRet = hadmap::getBoundary(m_pMapHandler, leftLaneBoundaryID, ptrLeftLB);
      if (nRet != TX_HADMAP_DATA_OK) {
        SYSTEM_LOGGER_INFO("get lane boundary error!");
        return -1;
      }
    }
    hadmap::txPoint txPt;
    txPt.x = dLon;
    txPt.y = dLat;
    txPt.z = 0;

    hadmap::txPoint txPtLeftPaddle;
    double dLeft = GetPassedDistance(ptrLeftLB->getGeometry(), txPt, txPtLeftPaddle);

    hadmap::txLaneBoundaryPtr ptrRightLB = lanePtr->getRightBoundary();
    if (ptrRightLB == nullptr) {
      hadmap::laneboundarypkid rightLaneBoundaryID = lanePtr->getRightBoundaryId();
      nRet = hadmap::getBoundary(m_pMapHandler, rightLaneBoundaryID, ptrRightLB);
      if (nRet != TX_HADMAP_DATA_OK) {
        SYSTEM_LOGGER_INFO("get lane boundary error!");
        return -1;
      }
    }

    hadmap::txPoint txPtRightPaddle;
    double dRight = GetPassedDistance(ptrRightLB->getGeometry(), txPt, txPtRightPaddle);

    CPoint3D ptLeft(txPtLeftPaddle);
    CPoint3D ptRight(txPtRightPaddle);

    LonlatToLocal(ptLeft.X(), ptLeft.Y(), ptLeft.Z());
    LonlatToLocal(ptRight.X(), ptRight.Y(), ptRight.Z());
    ptLeft.Z() = 0;
    ptRight.Z() = 0;

    double dWidth = ptLeft.distance(ptRight);
    return dWidth;
  }

  return LANE_WIDTH;
}
int CMapQuery::GetNearbyLaneFromPoint(double dLon, double dLat, double dAlt, hadmap::txLanePtr& pLane,
                                      bool bEgnoreAlt) {
  SYSTEM_LOGGER_INFO("GetNearbyLaneFromPoint hadmap::getLane(): txPt: (%f, %f, %f)", dLon, dLat, dAlt);

  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  if (!bEgnoreAlt) txPt.z = dAlt;

  int nRet = hadmap::getLane(m_pMapHandler, txPt, pLane);
  if (nRet != TX_HADMAP_DATA_OK) {
    SYSTEM_LOGGER_INFO("GetNearbyLaneFromPoint hadmap::getLane(): txPt: (%f, %f) error", dLon, dLat);
    return -1;
  }
  roadpkid uRoadID = pLane->getRoadId();
  sectionpkid uSecID = pLane->getSectionId();
  lanepkid laneID = pLane->getId();
  SYSTEM_LOGGER_INFO(
      "GetNearbyLaneFromPoint hadmap::getLane(): txPt: (%f, %f) result "
      "txLaneId: (%u, %u, %d)",
      dLon, dLat, uRoadID, uSecID, laneID);

  hadmap::txPoint txPtEnd = pLane->getGeometry()->getEnd();

  CPoint3D pt(txPt);
  CPoint3D ptEnd(txPtEnd);

  LonlatToLocal(pt.X(), pt.Y(), pt.Z());
  LonlatToLocal(ptEnd.X(), ptEnd.Y(), ptEnd.Z());
  if (bEgnoreAlt) {
    pt.Z() = 0;
    ptEnd.Z() = 0;
  }

  double dDist = pt.distance(ptEnd);

  SYSTEM_LOGGER_INFO(
      "hadmap::getLane() the distance between the end (%f, %f) and the "
      "point(%f, %f) is %f",
      txPtEnd.x, txPtEnd.y, dLon, dLat, dDist);
  if (dDist < 0.3) {
    hadmap::txLanes nextLanes;

    hadmap::getNextLanes(m_pMapHandler, pLane, nextLanes);

    if (nextLanes.size() > 0) {
      for (int i = 0; i < nextLanes.size(); ++i) {
        if (nextLanes[i]->getLength() > 0) {
          hadmap::txPoint txPtStart = nextLanes[i]->getGeometry()->getStart();
          CPoint3D ptStart(txPtStart);
          LonlatToLocal(ptStart.X(), ptStart.Y(), ptStart.Z());
          ptStart.Z() = 0;

          double dStartDist = ptStart.distance(ptEnd);
          if (dStartDist < 0.1) {
            pLane = nextLanes[i];
            roadpkid uRoadID = pLane->getRoadId();
            sectionpkid uSecID = pLane->getSectionId();
            lanepkid laneID = pLane->getId();
            SYSTEM_LOGGER_INFO(
                "hadmap::getLane() return the previous lane ,the "
                "real next lane is : txLaneId: (%u, %u, %d)",
                uRoadID, uSecID, laneID);
            break;
          }
        }
      }
    }
  }

  return 0;
}

int CMapQuery::GetNearByLaneFromRanges(double dLon, double dLat, hadmap::txLanePtr& pLane) {
  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  // 半径范围1米
  double dRadius = 0.001;
  hadmap::txLanes lanes;

  int nRet = hadmap::getLanes(m_pMapHandler, txPt, dRadius, lanes);
  if (nRet != TX_HADMAP_DATA_OK) {
    // 没有查询到lane，再调用getLane进行一次查询，这次再没有查询到，则证明没有
    int nRet = hadmap::getLane(m_pMapHandler, txPt, pLane);
    if (nRet != TX_HADMAP_DATA_OK) {
      SYSTEM_LOGGER_INFO("get lane error!");
      return -1;
    }

    return 0;
  }

  if (lanes.size() <= 0) {
    SYSTEM_LOGGER_WARN("point in radius 1 meter no lanes");
    return -1;
  } else if (lanes.size() == 1) {
    pLane = lanes[0];
    return 0;
  } else {
    SYSTEM_LOGGER_INFO("query near by lane find more than 1 lane");
    double dDistance = 100000;
    double dOffset = 1000;
    double dLength = 100000;
    int nIdx = 0;
    for (int i = 0; i < lanes.size(); ++i) {
      hadmap::txLanePtr l = lanes[i];
      double dDist = 0;
      double dOff = 0;
      int nRet = GetShiftAndOffset(l, txPt, dDist, dOff);
      if (nRet < 0) {
        SYSTEM_LOGGER_WARN("GetNearbyLane: in lanes GetShiftAndOffset error!");
        continue;
      }

      hadmap::roadpkid r = lanes[i]->getRoadId();
      hadmap::sectionpkid s = lanes[i]->getSectionId();
      hadmap::lanepkid lid = lanes[i]->getId();

      // 近似距离比较
      double length = dDist * dDist + dOff * dOff;
      length = sqrt(length);
      if (length < dLength) {
        dLength = length;
        nIdx = i;
      }
    }

    pLane = lanes[nIdx];

    return 0;
  }

  return -1;
}

int CMapQuery::GetNearbyLane(double dLon, double dLat, hadmap::txLanePtr& pLane) {
  if (useRanges) {
    return GetNearByLaneFromRanges(dLon, dLat, pLane);
  }

  return GetNearbyLaneFromPoint(dLon, dLat, 0, pLane, true);
}

int CMapQuery::GetNearbyLaneLink(double dLon, double dLat, hadmap::txLaneLinkPtr& pLaneLink) {
  SYSTEM_LOGGER_INFO("GetNearbyLaneLinkFromPoint hadmap::getLaneLink(): txPt: (%f, %f)", dLon, dLat);

  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;

  int nRet = hadmap::getLaneLink(m_pMapHandler, txPt, pLaneLink);
  if (nRet != TX_HADMAP_DATA_OK) {
    SYSTEM_LOGGER_INFO("get lanelink error!");
    return -1;
  }

  return 0;
}

double CMapQuery::GetPassedDistance(const hadmap::txCurve* pCurve, hadmap::txPoint& input, hadmap::txPoint& paddle) {
  const hadmap::txLineCurve* pLineCurve = dynamic_cast<const hadmap::txLineCurve*>(pCurve);

  if (pLineCurve) {
    double dDist = pLineCurve->getPassedDistance(input, paddle, hadmap::COORD_WGS84);

    return dDist;
  }

  SYSTEM_LOGGER_WARN("GetPassedDistance error!");

  return -100;
}

int CMapQuery::GetPassedDistancePoint(const hadmap::txCurve* pCurve, double dDist, hadmap::txPoint& output) {
  const hadmap::txLineCurve* pLineCurve = dynamic_cast<const hadmap::txLineCurve*>(pCurve);

  if (pLineCurve) {
    output = pLineCurve->getPoint(dDist);

    return 0;
  }

  SYSTEM_LOGGER_WARN("GetPassedDistancePoint error!");

  return -1;
}

int CMapQuery::GetTagentDir(const hadmap::txCurve* pCurve, double dDist, CPoint3D& dir) {
  const hadmap::txLineCurve* pLineCurve = dynamic_cast<const hadmap::txLineCurve*>(pCurve);

  if (pLineCurve) {
    double dLen = pLineCurve->getLength();
    double dStart = 0;
    double dEnd = 0;

    if (dDist > dLen) {
      dEnd = dLen;
      dStart = dLen - 2;
    } else if (dDist < 2) {
      if (dLen < 2) {
        dStart = 0;
        dEnd = dLen;
      } else {
        dEnd = 2;
        dStart = 0;
      }

    } else {
      dEnd = dDist;
      dStart = dDist - 2;
    }

    if (dEnd > dDist) {
      dDist = dDist;
    }
    if (dStart < 0) {
      dStart = 0;
    }
    hadmap::txPoint txStart = pLineCurve->getPoint(dStart);
    hadmap::txPoint txEnd = pLineCurve->getPoint(dEnd);

    LonlatToLocal(txStart.x, txStart.y, txStart.z);
    LonlatToLocal(txEnd.x, txEnd.y, txEnd.z);

    CPoint3D pStart(txStart);
    dir = txEnd;
    dir -= pStart;

    dir.norm();

    return 0;
  }

  SYSTEM_LOGGER_WARN("GetPassedDistancePoint error!");

  return -1;
}

int CMapQuery::JudgeSign(const hadmap::txCurve* pCurve, int geomDir, double dPos, CPoint3D& pt, CPoint3D& ptCenter) {
  const hadmap::txLineCurve* pLine = dynamic_cast<const hadmap::txLineCurve*>(pCurve);
  CPoint3D ptDir;
  if (pLine->getLength() < 0.01) {
    SYSTEM_LOGGER_INFO("lane geometry length is to small, length : %f", pLine->getLength());
    return 0;
  }

  CPoint3D ptStart, ptEnd;

  if (pLine->getLength() < 1) {
    ptStart = (pLine->getStart());
    ptEnd = (pLine->getEnd());

  } else {
    if (dPos >= pLine->getLength()) {
      ptStart = (pLine->getPoint((pLine->getLength() - 1)));
      ptEnd = (pLine->getEnd());
    } else if (dPos <= 0.01) {
      ptStart = (pLine->getStart());
      ptEnd = (pLine->getPoint(1.0));
    } else {
      ptStart = (pLine->getPoint(dPos));
      ptEnd = (pLine->getPoint(dPos + 1));
    }
  }

  LonlatToLocal(ptStart.X(), ptStart.Y(), ptStart.Z());
  LonlatToLocal(ptEnd.X(), ptEnd.Y(), ptEnd.Z());
  ptStart.Z() = ptEnd.Z() = 0;

  if (geomDir < 0) {
    ptDir = ptEnd - ptStart;
  } else {
    ptDir = ptStart - ptEnd;
  }
  ptDir.norm();

  CPoint3D ptBiNorm = pt - ptCenter;
  ptBiNorm.norm();
  CPoint3D ptVertical = ptDir.cross(ptBiNorm);
  CPoint3D ptZ(0, 0, 1);

  if (ptVertical.dot(ptZ) > 0) {
    return 1;
  }

  return -1;
}

int CMapQuery::QuerySectionWGS84(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID,
                                 double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dDist, double& dWidth,
                                 double& dOffset) {
  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  hadmap::txPoint txPtPaddle;
  hadmap::txLanePtr lanePtr;
  dLaneLon = 0;
  dLaneLat = 0;
  dLaneAlt = 0;
  dDist = 0.0;
  dWidth = 0;
  dOffset = 0;

  /*int nRet = hadmap::getLane(m_pMapHandler, txPt, lanePtr);
  if (nRet != TX_HADMAP_DATA_OK)
  {
          SYSTEM_LOGGER_INFO("get lane error!");
          return -1;
  }*/

  int nRet = GetNearbyLane(dLon, dLat, lanePtr);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("QuerySectionWSG84 hadmap::getLane(): txPt: (%f, %f) error! ", dLon, dLat);
    return -1;
  }

  if (lanePtr != nullptr) {
    uRoadID = lanePtr->getRoadId();
    uSecID = lanePtr->getSectionId();
    laneID = lanePtr->getId();

    SYSTEM_LOGGER_INFO(
        "QuerySectionWSG84 hadmap::getLane(): txPt: (%f, %f) result (%u, "
        "%u, %d)! ",
        dLon, dLat, uRoadID, uSecID, laneID);

    hadmap::txLaneBoundaryPtr ptrLeftLB = lanePtr->getLeftBoundary();
    if (ptrLeftLB == nullptr) {
      hadmap::laneboundarypkid leftLaneBoundaryID = lanePtr->getLeftBoundaryId();
      nRet = hadmap::getBoundary(m_pMapHandler, leftLaneBoundaryID, ptrLeftLB);
      if (nRet != TX_HADMAP_DATA_OK) {
        SYSTEM_LOGGER_INFO("get lane boundary error!");
        return -1;
      }
    }
    hadmap::txPoint txPtLeftPaddle;
    double dLeft = GetPassedDistance(ptrLeftLB->getGeometry(), txPt, txPtLeftPaddle);

    hadmap::txLaneBoundaryPtr ptrRightLB = lanePtr->getRightBoundary();
    if (ptrRightLB == nullptr) {
      hadmap::laneboundarypkid rightLaneBoundaryID = lanePtr->getRightBoundaryId();
      nRet = hadmap::getBoundary(m_pMapHandler, rightLaneBoundaryID, ptrRightLB);
      if (nRet != TX_HADMAP_DATA_OK) {
        SYSTEM_LOGGER_INFO("get lane boundary error!");
        return -1;
      }
    }
    hadmap::txPoint txPtRightPaddle;
    double dRight = GetPassedDistance(ptrRightLB->getGeometry(), txPt, txPtRightPaddle);

    hadmap::txPoint txPtPaddle;
    dDist = GetPassedDistance(lanePtr->getGeometry(), txPt, txPtPaddle);
    dLaneLon = txPtPaddle.x;
    dLaneLat = txPtPaddle.y;
    dLaneAlt = txPtPaddle.z;

    CPoint3D pt(txPt);
    CPoint3D ptCenter(txPtPaddle);
    CPoint3D ptLeft(txPtLeftPaddle);
    CPoint3D ptRight(txPtRightPaddle);

    LonlatToLocal(pt.X(), pt.Y(), pt.Z());
    LonlatToLocal(ptCenter.X(), ptCenter.Y(), ptCenter.Z());
    LonlatToLocal(ptLeft.X(), ptLeft.Y(), ptLeft.Z());
    LonlatToLocal(ptRight.X(), ptRight.Y(), ptRight.Z());
    pt.Z() = 0;
    ptCenter.Z() = 0;
    ptLeft.Z() = 0;
    ptRight.Z() = 0;

    dOffset = pt.distance(ptCenter);
    dWidth = ptLeft.distance(ptRight);

    int nDir = JudgeSign(lanePtr->getGeometry(), laneID, dDist, pt, ptCenter);

    if (nDir == 0) {
      return -1;
    }
    dOffset *= nDir;
    // dOffset = 0;
    return 0;
  }

  return 1;
}

int CMapQuery::QuerySectionByEnu(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID,
                                 double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dDist, double& dWidth,
                                 double& dOffset) {
  int nRet = QuerySection(dLon, dLat, uRoadID, uSecID, laneID);
  if (nRet < 0) {
    return nRet;
  }

  hadmap::txLanePtr lanePtr;
  hadmap::txLaneId laneId;
  laneId.roadId = uRoadID;
  laneId.sectionId = uSecID;
  laneId.laneId = laneID;

  hadmap::txLanePtr enuLanePtr;
  hadmap::txLaneMap::iterator itr = m_mapLanes.find(laneId);
  if (itr == m_mapLanes.end()) {
    SYSTEM_LOGGER_INFO("hadmap::getLane(): txLaneId: (%u, %u, %d)", uRoadID, uSecID, laneID);

    int nRet = hadmap::getLane(m_pMapHandler, laneId, lanePtr);
    if (nRet != TX_HADMAP_DATA_OK) {
      SYSTEM_LOGGER_INFO("get lane error!");
      return -1;
    }

    const hadmap::txLineCurve* pLine = dynamic_cast<const hadmap::txLineCurve*>(lanePtr->getGeometry());
    hadmap::PointVec points;
    pLine->getPoints(points);

    hadmap::txLanePtr pLane(new hadmap::txLane());
    pLane->setRoadId(uRoadID);
    pLane->setSectionId(uSecID);
    pLane->setId(laneID);

    hadmap::txLineCurve curve(hadmap::COORD_WGS84);
    curve.setCoords(points);
    pLane->setGeometry(curve);

    hadmap::txPoint txPtRef;
    txPtRef.x = m_dRefLon;
    txPtRef.y = m_dRefLat;
    txPtRef.z = m_dRefAlt;
    pLane->transfer2ENU(txPtRef);
    enuLanePtr = pLane;
    m_mapLanes.insert(std::make_pair(laneId, pLane));
  } else {
    enuLanePtr = itr->second;
  }

  double x, y, z;
  x = dLon;
  y = dLat;
  z = 0;
  coord_trans_api::lonlat2enu(x, y, z, m_dRefLon, m_dRefLat, m_dRefAlt);

  bool bRet = enuLanePtr->xy2sl(x, y, dDist, dOffset);
  if (!bRet) {
    SYSTEM_LOGGER_ERROR("hadmap::txLane::xy2sl() : (%f, %f) error!", dLon, dLat);
    return -1;
  }

  if (dDist < 0) {
    SYSTEM_LOGGER_WARN(
        "hadmap::txLane::xy2sl() : (%f, %f)  return the dist along curve "
        "is %f < 0!",
        dLon, dLat, dDist);
    dDist = 0;
  }

  double dTmp = 0;
  bRet = enuLanePtr->sl2xy(dDist, dTmp, x, y);
  if (!bRet) {
    SYSTEM_LOGGER_ERROR("hadmap::txLane::sl2xy() : (%f, %f) error!", dDist, dTmp);
    return -1;
  }

  const hadmap::txCurve* pCurve = enuLanePtr->getGeometry();

  hadmap::txPoint pCenter = pCurve->getPoint(dDist);
  z = pCenter.z;

  coord_trans_api::enu2lonlat(x, y, z, m_dRefLon, m_dRefLat, m_dRefAlt);

  dLaneLon = x;
  dLaneLat = y;
  dLaneAlt = z;

  dWidth = GetLaneWidth(uRoadID, uSecID, laneID, dLaneLon, dLaneLat);

  return 0;
}

int CMapQuery::QuerySection(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID,
                            double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dDist, double& dWidth,
                            double& dOffset) {
  if (useLocalENU) {
    return QuerySectionByEnu(dLon, dLat, uRoadID, uSecID, laneID, dLaneLon, dLaneLat, dLaneAlt, dDist, dWidth, dOffset);
  } else {
    return QuerySectionWGS84(dLon, dLat, uRoadID, uSecID, laneID, dLaneLon, dLaneLat, dLaneAlt, dDist, dWidth, dOffset);
  }
}

int CMapQuery::QueryLonLatByEnu(roadpkid uRoadID, sectionpkid uSecID, lanepkid laneID, double dShift, double dOffset,
                                double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dWidth, double& yaw) {
  dLaneLon = 0;
  dLaneLat = 0;
  dLaneAlt = 0;
  dWidth = 0;

  hadmap::txLanePtr lanePtr;
  hadmap::txLaneId laneId;
  laneId.roadId = uRoadID;
  laneId.sectionId = uSecID;
  laneId.laneId = laneID;

  hadmap::txLanePtr enuLanePtr;
  hadmap::txLaneMap::iterator itr = m_mapLanes.find(laneId);
  if (itr == m_mapLanes.end()) {
    SYSTEM_LOGGER_INFO("hadmap::getLane(): txLaneId: (%u, %u, %d)", uRoadID, uSecID, laneID);

    int nRet = hadmap::getLane(m_pMapHandler, laneId, lanePtr);
    if (nRet != TX_HADMAP_DATA_OK) {
      SYSTEM_LOGGER_INFO("get lane error!");
      return -1;
    }

    const hadmap::txLineCurve* pLine = dynamic_cast<const hadmap::txLineCurve*>(lanePtr->getGeometry());
    hadmap::PointVec points;
    pLine->getPoints(points);

    hadmap::txLanePtr pLane(new hadmap::txLane());
    pLane->setRoadId(uRoadID);
    pLane->setSectionId(uSecID);
    pLane->setId(laneID);

    hadmap::txLineCurve curve(hadmap::COORD_WGS84);
    curve.setCoords(points);
    pLane->setGeometry(curve);

    hadmap::txPoint txPtRef;
    txPtRef.x = m_dRefLon;
    txPtRef.y = m_dRefLat;
    txPtRef.z = m_dRefAlt;
    pLane->transfer2ENU(txPtRef);
    enuLanePtr = pLane;
    m_mapLanes.insert(std::make_pair(laneId, pLane));
  } else {
    enuLanePtr = itr->second;
  }

  double x, y, z;
  x = 0;
  y = 0;
  z = 0;
  bool bRet = enuLanePtr->sl2xy(dShift, dOffset, x, y);
  yaw = enuLanePtr->getYaw(dShift);
  if (!bRet) {
    SYSTEM_LOGGER_INFO("hadmap::txLane::sl2xy() : dist&offset: (%f, %f) error!", dShift, dOffset);
    return -1;
  }

  const hadmap::txCurve* pCurve = enuLanePtr->getGeometry();

  hadmap::txPoint pCenter = pCurve->getPoint(dShift);
  z = pCenter.z;

  coord_trans_api::enu2lonlat(x, y, z, m_dRefLon, m_dRefLat, m_dRefAlt);

  dLaneLon = x;
  dLaneLat = y;
  dLaneAlt = z;

  dWidth = GetLaneWidth(uRoadID, uSecID, laneID, dLaneLon, dLaneLat);

  if (bRet) {
    return 0;
  }

  return -1;
}

int CMapQuery::QueryLonLat(roadpkid uRoadID, sectionpkid uSecID, lanepkid laneID, double dDist, double dOffset,
                           double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dWidth, double& yaw) {
  if (useLocalENU) {
    return QueryLonLatByEnu(uRoadID, uSecID, laneID, dDist, dOffset, dLaneLon, dLaneLat, dLaneAlt, dWidth, yaw);
  } else {
    return QueryLonLatWGS84(uRoadID, uSecID, laneID, dDist, dOffset, dLaneLon, dLaneLat, dLaneAlt, dWidth);
  }
}

int CMapQuery::QueryLonLatByPointByEnu(double dLon, double dLat, lanepkid laneID, double dDist, double dOffset,
                                       double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dWidth,
                                       double& dFinalDist, double& dFinalYaw, bool isStart) {
  dLaneLon = 0;
  dLaneLat = 0;
  dLaneAlt = 0;
  dWidth = 0;
  dFinalDist = 0;

  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  hadmap::txLanePtr lanePtr;

  SYSTEM_LOGGER_INFO("QueryLonLatByPointByEnu(): (%f, %f)-(%d)-(%f, %f)", dLon, dLat, laneID, dDist, dOffset);

  int nRet = GetNearbyLane(dLon, dLat, lanePtr);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("get lane error!");
    return -1;
  }

  if (lanePtr != nullptr) {
    hadmap::txLaneId laneId;
    laneId.roadId = lanePtr->getRoadId();
    laneId.sectionId = lanePtr->getSectionId();
    laneId.laneId = laneID;

    double dLen = lanePtr->getLength();
    SYSTEM_LOGGER_INFO("GetNearbyLane(): (%u, %u, %d) Lane Length is %f", laneId.roadId, laneId.sectionId,
                       lanePtr->getId(), dLen);
    if (laneID != lanePtr->getId()) {
      nRet = hadmap::getLane(m_pMapHandler, laneId, lanePtr);
      if (nRet != TX_HADMAP_DATA_OK) {
        SYSTEM_LOGGER_INFO("get lane error!");
        return -1;
      }

      dLen = lanePtr->getLength();
      SYSTEM_LOGGER_INFO(
          "laneId is not equal reget lane: (%u, %u, %d) Lane Length is "
          "%f",
          laneId.roadId, laneId.sectionId, laneId.laneId, dLen);
    }

    hadmap::txLanePtr enuLanePtr;
    hadmap::txLaneMap::iterator itr = m_mapLanes.find(laneId);
    if (itr == m_mapLanes.end()) {
      if (!lanePtr->getGeometry()) {
        lanepkid tmpLaneID = lanePtr->getId();
        SYSTEM_LOGGER_INFO("lane (%u, %u, %d) geometry is null, now return", laneId.roadId, laneId.sectionId,
                           tmpLaneID);
        return -1;
      }
      const hadmap::txLineCurve* pLine = dynamic_cast<const hadmap::txLineCurve*>(lanePtr->getGeometry());
      hadmap::PointVec points;
      pLine->getPoints(points);

      hadmap::txLanePtr pLane(new hadmap::txLane());
      pLane->setRoadId(laneId.roadId);
      pLane->setSectionId(laneId.sectionId);
      pLane->setId(laneId.laneId);

      hadmap::txLineCurve curve(hadmap::COORD_WGS84);
      curve.setCoords(points);
      pLane->setGeometry(curve);

      hadmap::txPoint txPtRef;
      txPtRef.x = m_dRefLon;
      txPtRef.y = m_dRefLat;
      txPtRef.z = m_dRefAlt;
      pLane->transfer2ENU(txPtRef);
      enuLanePtr = pLane;
      m_mapLanes.insert(std::make_pair(laneId, pLane));
    } else {
      enuLanePtr = itr->second;
    }

    double x, y, z;
    x = dLon;
    y = dLat;
    z = 0;
    coord_trans_api::lonlat2enu(x, y, z, m_dRefLon, m_dRefLat, m_dRefAlt);

    SYSTEM_LOGGER_INFO("coord_trans_api::lonlat2enu(%f, %f)-->(%f, %f)!", dLon, dLat, x, y);
    double dOriginDist = 0;
    double dOriginOffset = 0;
    bool bRet = enuLanePtr->xy2sl(x, y, dOriginDist, dOriginOffset);
    if (!bRet) {
      SYSTEM_LOGGER_INFO("hadmap::txLane::xy2sl(%f, %f) error!", x, y);
      return -1;
    }

    if (dOriginDist < 0) {
      dOriginDist = 0;
    }

    dFinalDist = dOriginDist + dDist;

    bRet = enuLanePtr->sl2xy(dFinalDist, dOffset, x, y);
    dFinalYaw = enuLanePtr->getYaw(dFinalDist);
    if (!bRet) {
      SYSTEM_LOGGER_INFO(
          "hadmap::txLane::sl2xy(%f, %f) error! raw info: lonlat(%f, "
          "%f)-(origin dist,dist,offset)(%f, %f, %f) error!",
          dFinalDist, dOffset, dLon, dLat, dOriginDist, dDist, dOffset);

      bRet = enuLanePtr->sl2xy(dLen, dOffset, x, y);
      dFinalYaw = enuLanePtr->getYaw(dFinalDist);
      if (!bRet) {
        SYSTEM_LOGGER_INFO(
            "now use curve length instead of the dist "
            "hadmap::txLane::sl2xy(%f, %f) also failed!",
            dLen, dOffset);
        return -1;
      }
      SYSTEM_LOGGER_INFO(
          "now use curve length %f instead of the dist %f as the dist "
          "hadmap::txLane::sl2xy(%f, %f) succeed!",
          dLen, dFinalDist, dLen, dOffset);
    }

    const hadmap::txCurve* pCurve = enuLanePtr->getGeometry();

    hadmap::txPoint pCenter = pCurve->getPoint(dFinalDist);
    z = pCenter.z;

    coord_trans_api::enu2lonlat(x, y, z, m_dRefLon, m_dRefLat, m_dRefAlt);

    dLaneLon = x;
    dLaneLat = y;
    dLaneAlt = z;

    dWidth = GetLaneWidth(laneId.roadId, laneId.sectionId, laneId.laneId, dLaneLon, dLaneLat);

    if (bRet) {
      SYSTEM_LOGGER_INFO(
          "QueryLonLatByPointEnu() result:(%f, %f, %f), final dist: %f, "
          "width: %f",
          dLaneLon, dLaneLat, dLaneAlt, dFinalDist, dWidth);
      return 0;
    }
  }

  SYSTEM_LOGGER_INFO(
      "QueryLonLatByPointEnu() result:(%f, %f, %f), final dist: %f, width: "
      "%f",
      dLaneLon, dLaneLat, dLaneAlt, dFinalDist, dWidth);

  return -1;
}

int CMapQuery::QueryLonLatByPoint(double dLon, double dLat, lanepkid laneID, double dDist, double dOffset,
                                  double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dWidth,
                                  double& dFinalDist, double& dFinalYaw, bool isStart /*=false*/) {
  if (useLocalENU) {
    return QueryLonLatByPointByEnu(dLon, dLat, laneID, dDist, dOffset, dLaneLon, dLaneLat, dLaneAlt, dWidth, dFinalDist,
                                   dFinalYaw);
  } else {
    return QueryLonLatByPointWGS84(dLon, dLat, laneID, dDist, dOffset, dLaneLon, dLaneLat, dLaneAlt, dWidth,
                                   dFinalDist);
  }
}

int CMapQuery::QueryLonLatWGS84(roadpkid uRoadID, sectionpkid uSecID, lanepkid laneID, double dShift, double dOffset,
                                double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dWidth) {
  dLaneLon = 0;
  dLaneLat = 0;
  dLaneAlt = 0;
  dWidth = 0;

  hadmap::txLanePtr lanePtr;
  hadmap::txLaneId laneId;
  laneId.roadId = uRoadID;
  laneId.sectionId = uSecID;
  laneId.laneId = laneID;

  SYSTEM_LOGGER_INFO("hadmap::getLane(): txLaneId: (%u, %u, %d)", uRoadID, uSecID, laneID);

  int nRet = hadmap::getLane(m_pMapHandler, laneId, lanePtr);
  if (nRet != TX_HADMAP_DATA_OK) {
    SYSTEM_LOGGER_INFO("get lane error!");
    return -1;
  }

  hadmap::txPoint txPtPaddle;
  if (lanePtr != nullptr) {
    int nRet = GetPassedDistancePoint(lanePtr->getGeometry(), dShift, txPtPaddle);
    if (nRet < 0) {
      SYSTEM_LOGGER_ERROR("GetPassedDistancePoint error!");
      return -1;
    }
    hadmap::txPoint txPtTest;
    double dTest = GetPassedDistance(lanePtr->getGeometry(), txPtPaddle, txPtTest);
    CPoint3D ptTest(txPtTest);
    LonlatToLocal(ptTest.X(), ptTest.Y(), ptTest.Z());
    CPoint3D ptTest2(ptTest);
    LocalToLonlat(ptTest.X(), ptTest.Y(), ptTest.Z());

    CPoint3D pFrontDir;
    nRet = GetTagentDir(lanePtr->getGeometry(), dShift, pFrontDir);
    if (nRet < 0) {
      SYSTEM_LOGGER_ERROR("GetTagentDir error!");
      return -1;
    }
    CPoint3D pUp(0, 0, 1);
    CPoint3D pRight = pFrontDir.cross(pUp);
    /*
    hadmap::txLaneBoundaryPtr ptrLeftLB = lanePtr->getLeftBoundary();
    if (ptrLeftLB == nullptr)
    {
            hadmap::laneboundarypkid leftLaneBoundaryID =
    lanePtr->getLeftBoundaryId();
            nRet = hadmap::getBoundary(m_pMapHandler, leftLaneBoundaryID,
    ptrLeftLB);
            if (nRet != TX_HADMAP_DATA_OK)
            {
                    SYSTEM_LOGGER_INFO("get lane boundary error!");
                    return -1;
            }
    }
    hadmap::txPoint txPtLeftPaddle;
    double dLeft = GetPassedDistance(ptrLeftLB->getGeometry(), txPtPaddle,
    txPtLeftPaddle);


    hadmap::txLaneBoundaryPtr ptrRightLB = lanePtr->getRightBoundary();
    if (ptrRightLB == nullptr)
    {
            hadmap::laneboundarypkid rightLaneBoundaryID =
    lanePtr->getRightBoundaryId();
            nRet = hadmap::getBoundary(m_pMapHandler, rightLaneBoundaryID,
    ptrRightLB);
            if (nRet != TX_HADMAP_DATA_OK)
            {
                    SYSTEM_LOGGER_INFO("get lane boundary error!");
                    return -1;
            }
    }
    hadmap::txPoint txPtRightPaddle;
    double dRight = GetPassedDistance(ptrRightLB->getGeometry(), txPtPaddle,
    txPtRightPaddle);

    CPoint3D ptCenter(txPtPaddle);
    CPoint3D ptLeft(txPtLeftPaddle);
    CPoint3D ptRight(txPtRightPaddle);

    LonlatToLocal(ptCenter.X(), ptCenter.Y(), ptCenter.Z());
    LonlatToLocal(ptLeft.X(), ptLeft.Y(), ptLeft.Z());
    LonlatToLocal(ptRight.X(), ptRight.Y(), ptRight.Z());
    ptCenter.Z() = 0;
    ptLeft.Z() = 0;
    ptRight.Z() = 0;

    dWidth = ptLeft.distance(ptRight);
    */
    CPoint3D ptCenter(txPtPaddle);
    LonlatToLocal(ptCenter.X(), ptCenter.Y(), ptCenter.Z());
    pRight.norm();
    pRight = pRight * dOffset;
    if (laneID > 0) {
      pRight = pRight * -1;
    }

    // 左边为正，右边为负，所以减去
    CPoint3D pt = ptCenter - pRight;

    LocalToLonlat(pt.X(), pt.Y(), pt.Z());
    dLaneLon = pt.X();
    dLaneLat = pt.Y();
    dLaneAlt = pt.Z();

    return 0;
  }

  return 1;
}

int CMapQuery::QueryLonLatByPointWGS84(double dLon, double dLat, lanepkid laneID, double dDist, double dOffset,
                                       double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dWidth,
                                       double& dFinalShift) {
  dLaneLon = 0;
  dLaneLat = 0;
  dLaneAlt = 0;
  dWidth = 0;
  dFinalShift = 0;

  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  hadmap::txLanePtr lanePtr;

  SYSTEM_LOGGER_INFO("hadmap::getLane(): txPoint: (%f, %f)", dLon, dLat);
  /*int nRet = hadmap::getLane(m_pMapHandler, txPt, lanePtr);
  if (nRet != TX_HADMAP_DATA_OK)
  {
          SYSTEM_LOGGER_INFO("get lane error!");
          return -1;
  }*/

  // 通过(dLon, dLat)获取所在的lane
  int nRet = GetNearbyLane(dLon, dLat, lanePtr);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("get lane error!");
    return -1;
  }

  hadmap::txLaneId laneId;
  laneId.roadId = lanePtr->getRoadId();
  laneId.sectionId = lanePtr->getSectionId();

  hadmap::txPoint txPtPaddle;
  if (lanePtr != nullptr) {
    if (laneID != lanePtr->getId()) {
      hadmap::txLaneId laneId;
      laneId.roadId = lanePtr->getRoadId();
      laneId.sectionId = lanePtr->getSectionId();
      laneId.laneId = laneID;

      nRet = hadmap::getLane(m_pMapHandler, laneId, lanePtr);
      if (nRet != TX_HADMAP_DATA_OK) {
        SYSTEM_LOGGER_INFO("get lane error!");
        return -1;
      }
    }

    // 根据(dLon, dLat)获取lane上的垂足
    double dPaddleShift = GetPassedDistance(lanePtr->getGeometry(), txPt, txPtPaddle);

    // 计算最终shift
    dFinalShift = dPaddleShift + dDist;

    // 根据纵向偏移，最终的shift获取lane上的垂足点的坐标
    hadmap::txPoint txFinalPaddle;
    nRet = GetPassedDistancePoint(lanePtr->getGeometry(), dFinalShift, txFinalPaddle);
    if (nRet < 0) {
      SYSTEM_LOGGER_ERROR("GetPassedDistancePoint error!");
      return -1;
    }

    // 获取垂足点处的切线方向 pFrontDir
    CPoint3D pFrontDir;
    nRet = GetTagentDir(lanePtr->getGeometry(), dFinalShift, pFrontDir);
    if (nRet < 0) {
      SYSTEM_LOGGER_ERROR("GetTagentDir error!");
      return -1;
    }
    // 获取垂直于切线的方向
    CPoint3D pUp(0, 0, 1);
    CPoint3D pRight = pFrontDir.cross(pUp);

    // 计算offset向量
    CPoint3D ptCenter(txFinalPaddle);
    LonlatToLocal(ptCenter.X(), ptCenter.Y(), ptCenter.Z());
    pRight.norm();
    pRight = pRight * dOffset;
    if (lanePtr->getId() > 0) {
      pRight = pRight * -1;
    }

    // 获取最终点位置，（左边为正，右边为负，所以减去）
    CPoint3D pt = ptCenter - pRight;

    LocalToLonlat(pt.X(), pt.Y(), pt.Z());
    dLaneLon = pt.X();
    dLaneLat = pt.Y();
    dLaneAlt = pt.Z();

    return 0;
  }

  return 1;
}

int CMapQuery::QueryLonLatByPointOnLanelink(double dLon, double dLat, lanelinkpkid lanelinkID, double dDist,
                                            double dOffset, double& dFinalLon, double& dFinalLat, double& dFinalAlt,
                                            double& dFinalShift) {
  dFinalLon = 0;
  dFinalLat = 0;
  dFinalAlt = 0;
  dFinalShift = 0;

  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  hadmap::txLaneLinkPtr lanelinkPtr;

  SYSTEM_LOGGER_INFO("hadmap::getLaneLink(): txPoint: (%f, %f)", dLon, dLat);

  // 通过(dLon, dLat)获取所在的lanelink
  int nRet = GetNearbyLaneLink(dLon, dLat, lanelinkPtr);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("hadmap::NearbyLaneLink(): txPt: (%f, %f) error!", dLon, dLat);
    return -1;
  }

  hadmap::txPoint txPtPaddle;
  if (lanelinkPtr != nullptr) {
    if (lanelinkID != lanelinkPtr->getId()) {
      nRet = hadmap::getLaneLink(m_pMapHandler, lanelinkID, lanelinkPtr);
      if (nRet != TX_HADMAP_DATA_OK) {
        SYSTEM_LOGGER_INFO("get lane link error!");
        return -1;
      }
    }

    // 根据(dLon, dLat)获取lane上的垂足
    // double dPaddleShift = GetPassedDistance(lanelinkPtr->getGeometry(),
    // txPt, txPtPaddle);
    double dPaddleShift = 0;

    // 计算最终shift
    dFinalShift = dPaddleShift + dDist;

    // 根据纵向偏移，最终的shift获取lane上的垂足点的坐标
    hadmap::txPoint txFinalPaddle;
    nRet = GetPassedDistancePoint(lanelinkPtr->getGeometry(), dFinalShift, txFinalPaddle);
    if (nRet < 0) {
      SYSTEM_LOGGER_ERROR("GetPassedDistancePoint error!");
      return -1;
    }

    double dLength = lanelinkPtr->getGeometry()->getLength();

    SYSTEM_LOGGER_DEBUG("lanelink %d length: %f, distance along curve %f!", lanelinkID, dLength, dFinalShift);

    // 获取垂足点处的切线方向 pFrontDir
    CPoint3D pFrontDir;
    nRet = GetTagentDir(lanelinkPtr->getGeometry(), dFinalShift, pFrontDir);
    if (nRet < 0) {
      SYSTEM_LOGGER_ERROR("QueryLonLatByPointOnLanelink GetTagentDir error!");
      return -1;
    }
    // 获取垂直于切线的方向
    CPoint3D pUp(0, 0, 1);
    CPoint3D pRight = pFrontDir.cross(pUp);

    // 计算offset向量
    CPoint3D ptCenter(txFinalPaddle);
    LonlatToLocal(ptCenter.X(), ptCenter.Y(), ptCenter.Z());
    pRight.norm();
    pRight = pRight * dOffset;

    // 获取最终点位置，（左边为正，右边为负，所以减去）
    CPoint3D pt = ptCenter - pRight;

    LocalToLonlat(pt.X(), pt.Y(), pt.Z());
    dFinalLon = pt.X();
    dFinalLat = pt.Y();
    dFinalAlt = pt.Z();

    return 0;
  }

  return 1;
}

int CMapQuery::QueryForwardPoints(sTagGetForwardPointsParam& param, std::vector<std::vector<hadmap::txLaneId>>* ids,
                                  std::vector<std::pair<double, hadmap::PointVec>>* points) {
  double dis = param.length;
  double sampleDis = param.interval;
  hadmap::txPoint p(param.x, param.y, param.z);
  int nRet = hadmap::getForwardPoints(m_pMapHandler, p, dis, *points, sampleDis, ids);

  if (nRet != TX_HADMAP_DATA_OK) {
    SYSTEM_LOGGER_INFO(
        "get forward points error! point %f, %f, %f dis is: %f, interval: "
        "%f ",
        p.x, p.y, p.z, dis, sampleDis);
    return -1;
  }

  return 0;
}

int CMapQuery::GetNextRoadLanes(const hadmap::txLanePtr& lane, hadmap::txLanes& nextLanes) {
  hadmap::txLaneLinks laneLinks;
  int nRet = hadmap::getLaneLinks(m_pMapHandler, lane->getRoadId(), ROAD_PKID_INVALID, laneLinks);
  if (nRet != TX_HADMAP_DATA_OK) {
    SYSTEM_LOGGER_INFO("get lane link error!");
    return -1;
  }

  for (const hadmap::txLaneLinkPtr& l : laneLinks) {
    hadmap::txLanePtr nextLaneCandidate;
    nRet = hadmap::getLane(m_pMapHandler, hadmap::txLaneId(l->toRoadId(), l->toSectionId(), l->toLaneId()),
                           nextLaneCandidate);
    if (nRet != TX_HADMAP_DATA_OK) {
      SYSTEM_LOGGER_INFO("get lane error!");
      continue;
    }
    if (nextLaneCandidate->getGeometry() && !nextLaneCandidate->getGeometry()->empty()) {
      nextLanes.push_back(nextLaneCandidate);
    }
  }

  return 0;
}

int CMapQuery::GetLaneCenterHeight(double dLon, double dLat, double dAlt, double& dHeight) {
  dHeight = 0;
  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  txPt.z = dAlt;
  hadmap::txPoint txPtPaddle;
  hadmap::txLanePtr lanePtr;

  SYSTEM_LOGGER_INFO("GetLaneCenterHeight: txPt: (%f, %f, %f)", dLon, dLat, dAlt);

  // int nRet = GetNearbyLane(dLon, dLat, lanePtr);
  int nRet = GetNearbyLaneFromPoint(dLon, dLat, dAlt, lanePtr);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("get nearby lane error!");
    return -1;
  }

  if (lanePtr != nullptr) {
    const hadmap::txLineCurve* pLineCurve = dynamic_cast<const hadmap::txLineCurve*>(lanePtr->getGeometry());
    hadmap::txPoint txPtPaddle;

    if (pLineCurve) {
      double dDistance = pLineCurve->getPassedDistance(txPt, txPtPaddle, hadmap::COORD_WGS84);
      dHeight = txPtPaddle.z;
      return 0;
    }

    SYSTEM_LOGGER_WARN("GetLaneCenterHeight error!");
  }

  return -1;
}

int CMapQuery::QueryNextLane(roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID, roadpkid& uNextRoadID,
                             sectionpkid& uNextSecID, lanepkid& nextLaneID) {
  hadmap::txLaneId LID;
  LID.roadId = uRoadID;
  LID.sectionId = uSecID;
  LID.laneId = laneID;

  hadmap::txLanePtr lanePtr;
  int nRet = hadmap::getLane(m_pMapHandler, LID, lanePtr);
  if (nRet != TX_HADMAP_DATA_OK || lanePtr.get() == nullptr) {
    SYSTEM_LOGGER_INFO("get lane error!");
    return -1;
  }
  hadmap::txLanes nextLanes;
  nRet = hadmap::getNextLanes(m_pMapHandler, lanePtr, nextLanes);
  if (nRet != TX_HADMAP_DATA_OK && nRet != TX_HADMAP_DATA_EMPTY) {
    SYSTEM_LOGGER_INFO("get next lane error!");
    return -1;
  }

  if (nextLanes.empty()) {
    nRet = GetNextRoadLanes(lanePtr, nextLanes);
    if (nRet != 0 || nextLanes.empty()) {
      SYSTEM_LOGGER_INFO("next lanes is empty.");
      return -1;
    }
  }

  uNextRoadID = nextLanes[0]->getRoadId();
  uNextSecID = nextLanes[0]->getSectionId();
  nextLaneID = nextLanes[0]->getId();

  return 0;
}

int CMapQuery::QueryNextLanes(const roadpkid& uRoadID, const sectionpkid& uSecID, const lanepkid& laneID,
                              LaneIDVec& outLaneVec) {
  hadmap::txLaneId LID;
  LID.roadId = uRoadID;
  LID.sectionId = uSecID;
  LID.laneId = laneID;

  hadmap::txLanePtr lanePtr;
  int nRet = hadmap::getLane(m_pMapHandler, LID, lanePtr);
  if (nRet != TX_HADMAP_DATA_OK || lanePtr.get() == nullptr) {
    SYSTEM_LOGGER_INFO("get lane error!");
    return -1;
  }
  hadmap::txLanes nextLanes;
  nRet = hadmap::getNextLanes(m_pMapHandler, lanePtr, nextLanes);
  if (nRet != TX_HADMAP_DATA_OK && nRet != TX_HADMAP_DATA_EMPTY) {
    SYSTEM_LOGGER_INFO("get next lane error!");
    return -1;
  }

  if (nextLanes.empty()) {
    nRet = GetNextRoadLanes(lanePtr, nextLanes);
    if (nRet != 0 || nextLanes.empty()) {
      SYSTEM_LOGGER_INFO("next lanes is empty.");
      return -1;
    }
  }
  for (size_t i = 0, isize = nextLanes.size(); i < isize; i++) {
    LaneID laneid;
    laneid.m_roadID = nextLanes[i]->getRoadId();
    laneid.m_laneID = nextLanes[i]->getId();
    laneid.m_sectionID = nextLanes[i]->getSectionId();
    outLaneVec.push_back(laneid);
  }

  return 0;
}

int CMapQuery::LocalToLonlat(double& x, double& y, double& z) {
  coord_trans_api::local2lonlat(x, y, z, m_dRefLon, m_dRefLat, m_dRefAlt);
  return 0;
}

int CMapQuery::LonlatToLocal(double& dLon, double& dLat, double& dAlt) {
  coord_trans_api::lonlat2local(dLon, dLat, dAlt, m_dRefLon, m_dRefLat, m_dRefAlt);
  return 0;
}

int CMapQuery::LonlatToMercator(double& dLon, double& dLat) {
  coord_trans_api::lonlat2mercator(dLon, dLat);
  return 0;
}

int CMapQuery::MercatorToLonlat(double& dX, double& dY) {
  coord_trans_api::mercator2lonlat(dX, dY);
  return 0;
}

int CMapQuery::GetLonLat(double& dLon, double& dLat, double dShift, double dOffset) {
  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  hadmap::txPoint txPtPaddle;
  hadmap::txLanePtr lanePtr;

  SYSTEM_LOGGER_INFO("hadmap::getLane(): txPt: (%f, %f)", dLon, dLat);
  /*int nRet = hadmap::getLane(m_pMapHandler, txPt, lanePtr);
  if (nRet != TX_HADMAP_DATA_OK)
  {
          SYSTEM_LOGGER_INFO("get lane error!");
          return -1;
  }*/
  int nRet = GetNearbyLane(dLon, dLat, lanePtr);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("get lane error!");
    return -1;
  }

  if (lanePtr != nullptr) {
    int nRet = GetPassedDistancePoint(lanePtr->getGeometry(), dShift, txPtPaddle);
    if (nRet < 0) {
      SYSTEM_LOGGER_ERROR("GetPassedDistancePoint error!");
      return -1;
    }

    txPt.x = txPtPaddle.x;
    txPt.y = txPtPaddle.y;

    hadmap::txLaneBoundaryPtr ptrLeftLB = lanePtr->getLeftBoundary();
    if (ptrLeftLB == nullptr) {
      hadmap::laneboundarypkid leftLaneBoundaryID = lanePtr->getLeftBoundaryId();
      nRet = hadmap::getBoundary(m_pMapHandler, leftLaneBoundaryID, ptrLeftLB);
      if (nRet != TX_HADMAP_DATA_OK) {
        SYSTEM_LOGGER_INFO("get lane boundary error!");
        return -1;
      }
    }
    hadmap::txPoint txPtLeftPaddle;
    double dLeft = GetPassedDistance(ptrLeftLB->getGeometry(), txPt, txPtLeftPaddle);

    hadmap::txLaneBoundaryPtr ptrRightLB = lanePtr->getRightBoundary();
    if (ptrRightLB == nullptr) {
      hadmap::laneboundarypkid rightLaneBoundaryID = lanePtr->getRightBoundaryId();
      nRet = hadmap::getBoundary(m_pMapHandler, rightLaneBoundaryID, ptrRightLB);
      if (nRet != TX_HADMAP_DATA_OK) {
        SYSTEM_LOGGER_INFO("get lane boundary error!");
        return -1;
      }
    }
    hadmap::txPoint txPtRightPaddle;
    double dRight = GetPassedDistance(ptrRightLB->getGeometry(), txPt, txPtRightPaddle);

    CPoint3D ptCenter(txPtPaddle);
    CPoint3D ptLeft(txPtLeftPaddle);
    CPoint3D ptRight(txPtRightPaddle);

    SetRefPoint(txPtPaddle.x, txPtPaddle.y, 0);
    LonlatToLocal(ptCenter.X(), ptCenter.Y(), ptCenter.Z());
    LonlatToLocal(ptLeft.X(), ptLeft.Y(), ptLeft.Z());
    LonlatToLocal(ptRight.X(), ptRight.Y(), ptRight.Z());
    ptCenter.Z() = 0;
    ptLeft.Z() = 0;
    ptRight.Z() = 0;

    int nDir = JudgeSign(lanePtr->getGeometry(), lanePtr->getId(), dShift, ptLeft, ptRight);
    if (nDir > 0) {
      ptLeft -= ptRight;
    } else {
      ptLeft = ptRight - ptLeft;
    }

    ptLeft.norm();
    CPoint3D vOffset = ptLeft * dOffset;
    ptCenter += vOffset;

    LocalToLonlat(ptCenter.X(), ptCenter.Y(), ptCenter.Z());

    dLon = ptCenter.X();
    dLat = ptCenter.Y();

    return 0;
  }

  return -1;
}

int CMapQuery::QueryLanePosByEnu(double dLon, double dLat, double dDist, double dOffset, double& dLaneLon,
                                 double& dLaneLat, double& dLaneAlt) {
  dLaneLon = 0;
  dLaneLat = 0;
  dLaneAlt = 0;

  hadmap::txPoint txPt;
  txPt.x = dLon;
  txPt.y = dLat;
  hadmap::txLanePtr lanePtr;

  SYSTEM_LOGGER_INFO("QueryLanePoseByEnu(): (%f, %f)--(%f, %f)", dLon, dLat, dDist, dOffset);

  int nRet = GetNearbyLane(dLon, dLat, lanePtr);
  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("get lane error!");
    return -1;
  }

  if (lanePtr != nullptr) {
    hadmap::txLaneId laneId;
    laneId.roadId = lanePtr->getRoadId();
    laneId.sectionId = lanePtr->getSectionId();
    laneId.laneId = lanePtr->getId();

    double dLen = lanePtr->getLength();
    SYSTEM_LOGGER_INFO("GetNearbyLane(): (%u, %u, %d) Lane Length is %f", laneId.roadId, laneId.sectionId,
                       lanePtr->getId(), dLen);

    hadmap::txLanePtr enuLanePtr;
    hadmap::txLaneMap::iterator itr = m_mapLanes.find(laneId);
    if (itr == m_mapLanes.end()) {
      if (!lanePtr->getGeometry()) {
        lanepkid tmpLaneID = lanePtr->getId();
        SYSTEM_LOGGER_INFO("lane (%u, %u, %d) geometry is null, now return", laneId.roadId, laneId.sectionId,
                           tmpLaneID);
        return -1;
      }
      const hadmap::txLineCurve* pLine = dynamic_cast<const hadmap::txLineCurve*>(lanePtr->getGeometry());
      hadmap::PointVec points;
      pLine->getPoints(points);

      hadmap::txLanePtr pLane(new hadmap::txLane());
      pLane->setRoadId(laneId.roadId);
      pLane->setSectionId(laneId.sectionId);
      pLane->setId(laneId.laneId);

      hadmap::txLineCurve curve(hadmap::COORD_WGS84);
      curve.setCoords(points);
      pLane->setGeometry(curve);

      hadmap::txPoint txPtRef;
      txPtRef.x = m_dRefLon;
      txPtRef.y = m_dRefLat;
      txPtRef.z = m_dRefAlt;
      pLane->transfer2ENU(txPtRef);
      enuLanePtr = pLane;
      m_mapLanes.insert(std::make_pair(laneId, pLane));
    } else {
      enuLanePtr = itr->second;
    }

    double x, y, z;
    x = 0;
    y = 0;
    z = 0;
    bool bRet = enuLanePtr->sl2xy(dDist, dOffset, x, y);
    if (!bRet) {
      SYSTEM_LOGGER_INFO("hadmap::txLane::sl2xy(%f, %f) error! raw info: lonlat(%f, %f).", dDist, dOffset, dLon, dLat);

      bRet = enuLanePtr->sl2xy(dLen, dOffset, x, y);
      if (!bRet) {
        SYSTEM_LOGGER_INFO("now use curve length instead of the dist hadmap::txLane::sl2xy(%f, %f) also failed!", dLen,
                           dOffset);
        return -1;
      }
      SYSTEM_LOGGER_INFO(
          "now use curve length %f instead of the dist %f as the dist hadmap::txLane::sl2xy(%f, %f) succeed!", dLen,
          dDist, dLen, dOffset);
    }

    const hadmap::txCurve* pCurve = enuLanePtr->getGeometry();

    hadmap::txPoint pCenter = pCurve->getPoint(dDist);
    z = pCenter.z;

    coord_trans_api::enu2lonlat(x, y, z, m_dRefLon, m_dRefLat, m_dRefAlt);

    dLaneLon = x;
    dLaneLat = y;
    dLaneAlt = z;

    if (bRet) {
      SYSTEM_LOGGER_INFO("QueryLanePosByEnu() result:(%f, %f, %f), dist: %f", dLaneLon, dLaneLat, dLaneAlt, dDist);
      return 0;
    }
  }

  SYSTEM_LOGGER_INFO("QueryLanePosByEnu() result:(%f, %f, %f), dist: %f", dLaneLon, dLaneLat, dLaneAlt, dDist);

  return -1;
}

void CMapQuery::SearchPath(const std::vector<hadmap::txPoint>& from_to, hadmap::PointVec& geoms) {
  geoms.clear();

  planner::RoadInfoArray paths;
  planner::RoutePlan route_plan(m_pMapHandler);
  route_plan.searchPath(const_cast<std::vector<hadmap::txPoint>&>(from_to), paths);
  for (size_t i = 0; i < paths.size(); i++) {
    hadmap::txSections sections;
    if (paths[i].roadId > 0) {
      hadmap::txRoadPtr road_ptr = route_plan.getRoad(paths[i]);
      if (road_ptr) {
        sections = road_ptr->getSections();
      }
    } else {
      hadmap::txJunctionRoadPtr junction_ptr = route_plan.getJunction(paths[i]);
      if (junction_ptr) {
        sections = junction_ptr->getSections();
      }
    }

    for (auto&& section : sections) {
      for (auto&& lane : section->getLanes()) {
        const hadmap::txLineCurve* line_curve = dynamic_cast<const hadmap::txLineCurve*>(lane->getGeometry());
        if (!line_curve || line_curve->empty()) {
          continue;
        }
        hadmap::PointVec points;
        line_curve->sampleV2(3, 5, points);
        geoms.insert(geoms.end(), points.begin(), points.end());
      }
    }
  }
}
