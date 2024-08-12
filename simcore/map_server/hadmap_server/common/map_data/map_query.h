/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include "common/engine/math/vec3.h"
#include "common/map_data/map_data_predef.h"
#include "routeplan/route_plan.h"
#include "xml_parser/entity/hadmap_params.h"

namespace hadmap {
struct txMapHandle;
}

struct linkData {
  std::string lane;
  std::string phase;
  std::string tolanelinkids;
};
class CMapQuery {
 public:
  CMapQuery();
  ~CMapQuery();
  void Clear();

  void SetMapPath(const char* strPath);
  int Init();
  void SetRefPoint(double x, double y, double z);
  int Deinit();
  bool Initialized() { return m_initialized; }
  void SetInitialize(bool isornot) { m_initialized = isornot; }

  void RefPoint(double& x, double& y, double& z) {
    x = m_dRefLon;
    y = m_dRefLat;
    z = m_dRefAlt;
  }

  int QueryLaneLink(double dLon, double dLat, lanelinkpkid& lanelinkid);

  int QueryLaneLink(double dLon, double dLat, lanelinkpkid& lanelinkid, double& dLanelinkLon, double& dLanelinkLat,
                    double& dLanelinkAlt, double& dDist, double& dOffset, double& dYaw, double searchDistance = 5.0);

  int QuerySection(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID,
                   double searchDistance = 5.0);

  int QuerySection(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID, double& dLaneLon,
                   double& dLaneLat, double& dLaneAlt, double& dDist, double& dWidth, double& dOffset, double& dYaw,
                   double searchDistance = 5.0);
  int QueryLonLat(roadpkid uRoadID, sectionpkid uSecID, lanepkid laneID, double dDist, double dOffset, double& dLaneLon,
                  double& dLaneLat, double& dLaneAlt, double& dWidth, double& yaw);
  int QueryLonLatByPoint(double dLon, double dLat, lanepkid laneID, double dDist, double dOffset, double& dLaneLon,
                         double& dLaneLat, double& dLaneAlt, double& dWidth, double& dFinalDist, double& dFinalYaw,
                         bool isStart = false);
  int QuerySectionPoint(double& dLon, double& dLat, lanepkid laneID);

  int QueryLonLatByPointOnLanelink(double dLon, double dLat, lanelinkpkid laneID, double dDist, double dOffset,
                                   double& dFinalLon, double& dFinalLat, double& dFinalAlt, double& dFinalDist);
  int QueryLaneLinkDataFromLonLat(const double lon, const double lat, int& roadid, std::vector<linkData>& linkdatas);

  int QueryNextLane(roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID, roadpkid& uNextRoadID,
                    sectionpkid& uNextSecID, lanepkid& nextLaneID);
  int QueryNextLanes(const roadpkid& uRoadID, const sectionpkid& uSecID, const lanepkid& laneID, LaneIDVec& outLaneVec);
  int LocalToLonlat(double& x, double& y, double& z);

  int QueryForwardPoints(sTagGetForwardPointsParam&, std::vector<std::vector<hadmap::txLaneId>>*,
                         std::vector<std::pair<double, hadmap::PointVec>>*);
  int LonlatToLocal(double& dLon, double& dLat, double& dAlt);

  int LonlatToMercator(double& x, double& y);
  int MercatorToLonlat(double& x, double& y);

  int GetNextRoadLanes(const hadmap::txLanePtr& lane, hadmap::txLanes& nextLanes);

  int GetLaneCenterHeight(double dLon, double dLat, double dAlt, double& dHeight);

  int GetLonLat(double& dLon, double& dLat, double dShift, double dOffset);

  int QueryLanePosByEnu(double dLon, double dLat, double dShift, double dOffset, double& dLaneLon, double& dLaneLat,
                        double& dAlt);

  hadmap::txMapHandle* GettxMapHandle() { return m_pMapHandler; }
  void SettxMapHandle(hadmap::txMapHandle* Temp) { m_pMapHandler = Temp; }

  void SearchPath(const std::vector<hadmap::txPoint>& from_to, hadmap::PointVec& geoms);

  int GetTrafficLights(const std::string strId, std::string& routeId, std::string& phase, std::string& lane,
                       std::string& junction, std::string& roadid, hadmap::txPoint& endp);
  int GetTrafficRouteId(const std::string roadid);

  int GetNearbyLane(double dLon, double dLat, hadmap::txLanePtr& pLane, double searchDistance = 5.0,
                    bool isStart = false);
  int GetNearByLaneFromOldStart(double dLon, double dLat, hadmap::txLanePtr& pLane);

 protected:
  int QuerySectionByEnu(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID,
                        double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dDist, double& dWidth,
                        double& dOffset, double& dYaw, double searchDistance = 5.0);
  int QueryLonLatByEnu(roadpkid uRoadID, sectionpkid uSecID, lanepkid laneID, double dDist, double dOffset,
                       double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dWidth, double& yaw);
  int QueryLonLatByPointByEnu(double dLon, double dLat, lanepkid laneID, double dDist, double dOffset, double& dLaneLon,
                              double& dLaneLat, double& dLaneAlt, double& dWidth, double& dFinalDist, double& dFinalYaw,
                              bool isStart = false);

  int QuerySectionWGS84(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& uSecID, lanepkid& laneID,
                        double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dDist, double& dWidth,
                        double& dOffset, double searchDistance = 5.0);
  int QueryLonLatWGS84(roadpkid uRoadID, sectionpkid uSecID, lanepkid laneID, double dDist, double dOffset,
                       double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dWidth);
  int QueryLonLatByPointWGS84(double dLon, double dLat, lanepkid laneID, double dDist, double dOffset, double& dLaneLon,
                              double& dLaneLat, double& dLaneAlt, double& dWidth, double& dFinalDist);

  double GetLaneWidth(roadpkid uRoadID, sectionpkid uSecID, lanepkid laneID, double dLon, double dLat);

  int GetNearByLaneFromRanges(double dLon, double dLat, hadmap::txLanePtr& pLane, double searchDistance = 5.0);

  int GetNearbyLaneFromPoint(double dLon, double dLat, hadmap::txLanePtr& pLane, double searchDistance = 5.0,
                             bool isStart = false);

  int GetNearbyLaneFromPoint(double dLon, double dLat, double dAlt, hadmap::txLanePtr& pLane);

  int GetNearbyLaneLink(double dLon, double dLat, hadmap::txLaneLinkPtr& pLaneLink, double searchDistance = 5.0);
  int GetShiftAndOffset(hadmap::txLanePtr& pLane, hadmap::txPoint& pt, double& dDistance, double& dOffset);
  double GetPassedDistance(const hadmap::txCurve* pCurve, hadmap::txPoint& input, hadmap::txPoint& paddle);
  int GetPassedDistancePoint(const hadmap::txCurve* pCurve, double dDist, hadmap::txPoint& output);
  int GetTagentDir(const hadmap::txCurve* pCurve, double dDist, CPoint3D& dir);
  int JudgeSign(const hadmap::txCurve* pCurve, int geomDir, double dPos, CPoint3D& pt, CPoint3D& ptCenter);

  std::string m_strMapFile;
  bool m_initialized;
  int m_mapType;
  hadmap::txMapHandle* m_pMapHandler;
  double m_dRefLon;
  double m_dRefLat;
  double m_dRefAlt;

  hadmap::txLaneMap m_mapLanes;
};
