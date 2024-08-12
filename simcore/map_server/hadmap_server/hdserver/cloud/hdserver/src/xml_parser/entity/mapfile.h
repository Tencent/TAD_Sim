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
#include "common/engine/math/vec3.h"
#include "common/map_data/map_data_predef.h"

/*
class CRoadData;
typedef std::vector<CRoadData*>			RoadVec;
typedef std::map<LaneID, CRoadData*>	RoadMap;
class CLaneData;
typedef std::vector<CLaneData*>			LaneVec;
typedef std::map<LaneID, CLaneData*>	LaneMap;
class CLaneBoundaryData;
typedef std::vector<CLaneBoundaryData*>			LaneBoundaryVec;
typedef std::map<LaneID, CLaneBoundaryData*>	LaneBoundaryMap;
class CLaneLink;
typedef std::vector<CLaneLink*> LaneLinkVec;
typedef std::set<LaneID, CLaneLink*>	LaneLinkSet;
class CMapObject;
typedef std::vector<CMapObject*>			MapObjectVec;
typedef std::map<objectpkid, CMapObject*>		MapObjectMap;
*/

class CAABB;

class CMapFile {
 public:
  int ParseMapOld(const char* strMapFile);

  int ParseMap(const char* strMapFile);

  int ParseMapV2(const char* strMapFile, bool bReadHadmapConfig = true);

  void ClearAll();

 protected:
  CVector3 computeRefP(const CAABB& aabb);

  void ClearRoads();
  void ClearLanes();
  void ClearLaneBoundaries();
  void ClearLaneLinks();
  void ClearMapObjects();
  void CoordConvertV2(CAABB& aabb, CPoint3D& refPoint);
  void CalculateBox(CAABB& aabb);

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

  float m_center[3];
  CPoint3D m_refPt;
  float m_envelop_min[3];
  float m_envelop_max[3];

  RoadVec m_roads;
  LaneVec m_lanes;
  LaneBoundaryVec m_laneboundaries;
  LaneLinkVec m_lanelinks;
  MapObjectVec m_mapobjects;
};
