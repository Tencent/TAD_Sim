/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once

#include <hadmap.h>
#include <map>
#include "../engine/entity/aabb.h"
#include "../engine/entity/entity.h"
#include "common/engine/math/vec3.h"
#include "common/map_data/map_data_predef.h"

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

class CMapObject : public CEntity {
 public:
  CMapObject();
  void FromTxObject(hadmap::txObjectPtr obj);
  void Lonlat2Local(double refLon, double refLat, double refAlt);
  void Lonlat2ENU(double refLon, double refLat, double refAlt);

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

  CPoint3D m_position;
  PointsVec m_points;
  std::vector<PointWH> m_repeat;
  CParkingSpace m_parking_space;
  std::map<std::string, std::string> m_user_data;
};
