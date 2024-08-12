/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <hadmap.h>
#include <map>
#include <set>
#include "common/engine/math/vec3.h"
#include "common/map_data/map_data_predef.h"
#include "eigen3/Eigen/Dense"
#include "engine/entity/aabb.h"
#include "engine/entity/entity.h"
enum LINE_TYPE {
  E_SINGLE_LINE,
  E_DOUBLE_LINE,
};
struct CGeomData {
  LINE_TYPE sType;
  std::vector<hadmap::PointVec> sPoints;
};

class PointWH : public CPoint3D {
 public:
  double w = 0, h = 0;
};
struct CParkingSpace {
  struct ParkingMarking {
    std::string side;
    std::string type;
    double width = 0;
    int color = 0;
  };
  bool valid = false;
  std::vector<ParkingMarking> marks;
  std::string access;
  std::string restrictions;
};

struct road_setion_id {
  roadpkid m_roadID;
  sectionpkid m_sectionID;

  bool operator<(const road_setion_id& rhs) const { return m_roadID < rhs.m_roadID; }
};
struct LogicLights {
  objectpkid m_id;
  HADMAP_OBJECT_TYPE m_type;
  std::string m_name;
  std::string m_strType;
  std::string m_strSubType;
  double m_dLength;
  double m_dWidth;
  double m_dHeight;
  double m_dRoll;
  double m_dYaw;
  double m_dPitch;
  int m_nGeomCount;
  double m_dGroundHeight;

  CPoint3D m_position;  // 在未来去确定位置
  PointsVec m_points;
  int m_controlroadid;
  std::map<std::string, std::string> m_user_data;

  bool operator<(const LogicLights& rhs) const { return m_id < rhs.m_id; }
};

class CMapObject : public CEntity {
 public:
  static std::set<road_setion_id> LogicLightsRoadSet;
  static std::multimap<road_setion_id, LogicLights> LogicLightsMap;
  void Trans(LogicLights LL, hadmap::txPoint& LogicPoints);

  CMapObject();
  void FromTxObject(hadmap::txObjectPtr obj);
  void Lonlat2Local(double refLon, double refLat, double refAlt);
  void Lonlat2ENU(double refLon, double refLat, double refAlt);

  LogicLights CopyOneRefLight(CMapObject* srcobj);  // 深度拷贝,顺便修改下里面的一些值
  PointsVec& Data() { return m_points; }

  objectpkid Id() { return m_id; }
  HADMAP_OBJECT_TYPE Type() { return m_type; }
  std::string Name() { return m_name; }
  std::string StrType() { return m_strType; }
  std::string StrSubType() { return m_strSubType; }
  double Length() { return m_dLength; }
  double Width() { return m_dWidth; }
  double Height() { return m_dHeight; }
  double Roll() { return m_dRoll; }
  double Yaw() { return m_dYaw; }
  double Pitch() { return m_dPitch; }
  double GroundHeight() { return m_dGroundHeight; }
  void SetGroundHeight(double dHeight) { m_dGroundHeight = dHeight; }
  const CPoint3D& Position() const { return m_position; }
  const std::vector<PointWH>& Repeat() const { return m_repeat; }

  std::vector<LaneID>& ReliedLaneIds() { return m_relied_lane_id; }
  int GeomCount() { return m_nGeomCount; }
  bool hasParkingSpace() const { return m_parking_space.valid; }
  const CParkingSpace& ParkingSpace() const { return m_parking_space; }
  const std::map<std::string, std::string>& UserData() const { return m_user_data; }
  const std::map<int, std::vector<CPoint3D>>& Outlines() const { return m_outlines; }
  const std::string& MarkWidth() const { return m_parkingMarkWidth; }
  const std::string& MarkColor() const { return m_parkingMarkColor; }
  bool SetGeomsFromRoad(const hadmap::txLanePtr lanePtr, hadmap::txObjectPtr obj);
  CGeomData& getGeomData() { return m_geoms; }

 protected:
  objectpkid m_id;
  HADMAP_OBJECT_TYPE m_type;
  std::string m_name;
  std::string m_strType;
  std::string m_strSubType;
  double m_dLength;
  double m_dWidth;
  double m_dHeight;
  double m_dRoll;
  double m_dYaw;
  double m_dPitch;
  std::vector<LaneID> m_relied_lane_id;
  int m_nGeomCount;
  double m_dGroundHeight;
  CGeomData m_geoms;
  CPoint3D m_position;
  PointsVec m_points;
  std::vector<PointWH> m_repeat;
  CParkingSpace m_parking_space;
  std::map<std::string, std::string> m_user_data;
  std::map<int, std::vector<CPoint3D>> m_outlines;
  std::string m_parkingMarkWidth;
  std::string m_parkingMarkColor;
  std::vector<vector3<double>> CalculateCornerPoints(const double& centerX, const double& centerY,
                                                     const double& centerZ, const double& Len, const double& Wid,
                                                     const double& heading, const double& pitch, const double& roll);
};
