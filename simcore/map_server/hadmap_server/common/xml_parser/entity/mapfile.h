/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "common/engine/entity/hadmap.h"
#include "common/engine/math/vec3.h"
#include "common/map_data/map_data_predef.h"
#include "common/xml_parser/entity/signlight.h"

class CAABB;

class CMapFile {
 public:
  int ParseMapOld(const char* strMapFile);

  int ParseMap(const char* strMapFile);

  int ParseMapV2(const char* strMapFile, bool bReadHadmapConfig = true);

  ~CMapFile() {  // Disconect();
  }
  void ClearAll();

  int Disconect();

  double queryHeadAngle(const double lon, const double lat);

 protected:
  void ClearRoads();
  void ClearLanes();
  void ClearLaneBoundaries();
  void ClearLaneLinks();
  void ClearMapObjects();
  void CoordConvertV2(CAABB& aabb, CPoint3D& refPoint);
  bool getRoadEndCenterPoint(hadmap::txRoadPtr roadPtr, hadmap::txPoint& endp,
                             bool fromLaneId /*true = fromLaneId > 0*/);
  void getRoadLaneLinks(const hadmap::txRoadPtr roadPtr, std::map<std::string, std::vector<int>>& lanes,
                        std::map<std::string, std::vector<int>>& lanelinks, bool fromLaneId /*true = fromLaneId > 0*/);

  void CalculateBox(CAABB& aabb);

  void CalulatePoints(hadmap::txPoint& PA, hadmap::txPoint& PB, int& num, std::vector<hadmap::txPoint>& Points);
  //  insert to three lights

 public:
  std::string m_strMapfile;
  std::string m_strVersion;

  std::string m_strUnrealLevelIndex;

  std::string m_strLon;
  std::string m_strLat;
  std::string m_strAlt;

  double m_fLon;
  double m_fLat;
  double m_fAlt;

  double m_center[3];
  CPoint3D m_refPt;
  double m_envelop_min[3];
  double m_envelop_max[3];

  RoadVec m_roads;
  LaneVec m_lanes;
  LaneBoundaryVec m_laneboundaries;
  LaneLinkVec m_lanelinks;
  MapObjectVec m_mapobjects;
  LogicLightVec m_lights;
  std::string m_version;
  std::map<int, std::map<int, int>> m_roadJunctionId;

  hadmap::txMapHandle* pMapHandler = NULL;
};
