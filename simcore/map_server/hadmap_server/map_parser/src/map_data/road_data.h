/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include "common/engine/math/vec3.h"

#include <hadmap.h>
#include "../engine/entity/entity.h"
#include "common/map_data/map_data_predef.h"

class CSectionData;

class CRoadData : public CEntity {
 public:
  typedef std::vector<CSectionData*> Sections;

  CRoadData();

  void Reset();

  void FromTXRoad(hadmap::txRoad& road);

  void Lonlat2Local(double refLon, double refLat, double refAlt);
  void Lonlat2ENU(double refLon, double refLat, double refAlt);

  PointsVec& Data() { return m_points; }
  Sections& Secs() { return m_sections; }

  roadpkid Id() { return m_id; }
  taskpkid TaskId() { return m_taskid; }
  std::string Name() { return m_name; }
  double Length() { return m_dLength; }
  double Ground() { return m_dGround; }
  HADMAP_ROAD_TYPE Type() { return m_type; }
  uint8_t Direction() { return m_direction; }
  uint8_t Toll() { return m_toll; }
  uint8_t Function() { return m_funcClass; }
  HADMAP_ROAD_MATERIAL Material() { return m_material; }
  uint64_t Attribute() { return m_attribute; }
  uint8_t Trans() { return m_transType; }
  bool isUrban() { return m_urban; }
  bool isPaved() { return m_paved; }
  bool isBiDirection() { return m_bidirection; }

 protected:
  roadpkid m_id;
  taskpkid m_taskid;
  std::string m_name;
  double m_dLength;
  double m_dGround;
  HADMAP_ROAD_TYPE m_type;
  uint8_t m_direction;
  uint8_t m_toll;
  uint8_t m_funcClass;
  bool m_urban;
  bool m_paved;
  bool m_bidirection;
  uint8_t m_transType;
  HADMAP_ROAD_MATERIAL m_material;
  uint64_t m_attribute;

  Sections m_sections;
  PointsVec m_points;
};
