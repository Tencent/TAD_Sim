/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <hadmap.h>
#include <vector>
#include "../engine/entity/aabb.h"
#include "../engine/entity/entity.h"
#include "common/engine/math/vec3.h"
#include "common/map_data/map_data_predef.h"

class CLaneBoundaryData;
class CVertexArrayObject;
const double LANE_WIDTH = 3.5;

struct tagGeneratedMesh {
  float* m_pData;         // 三角网格数据
  uint64_t m_uDataSize;   // buffer大小，单位字节
  uint64_t m_uDataCount;  // 元素个数，单位 sizeof(float)
};

class CLaneData : public CEntity {
 public:
  CLaneData();
  explicit CLaneData(const hadmap::txLanePtr& pLane);
  CLaneData& operator=(hadmap::txLanePtr& pLane);

  ~CLaneData();

  void Lonlat2Local(double refLon, double refLat, double refAlt);
  void Lonlat2ENU(double refLon, double refLat, double refAlt);
  bool IsEmpty() { return m_points.size() <= 0; }
  PointsVec& Data() { return m_points; }
  CLaneBoundaryData** Boundarys() { return m_boundarys; }

  uint32_t SpeedLimit() { return m_speedLimit; }
  laneboundarypkid LeftBoundaryId() { return m_leftBoundaryID; }
  laneboundarypkid RightBoundaryId() { return m_rightBoundaryID; }
  HADMAP_LANE_TYPE LaneType() { return m_laneType; }
  HADMAP_LANE_ARROW LaneArrow() { return m_laneArrow; }
  roadpkid RoadId() { return m_roadID; }
  sectionpkid SectionId() { return m_sectionID; }
  lanepkid LaneId() { return m_laneID; }
  double Width() { return m_laneWidth; }
  double Length() { return m_length; }
  double DistanceToEnd() { return m_distanceToEnd; }
  std::string Name() { return m_name; }
  bool Enabled() { return m_enabled; }

  void GenerateMesh();

  tagGeneratedMesh& Mesh() { return m_generatedMesh; }

  /*CAABB&		AabbLl() { return m_aabb_ll; }
  CAABB&		AabbXyz() { return m_aabb_xyz; }*/

 protected:
  void Reset();
  void CopyFrom(const hadmap::txLanePtr& pLane);

 protected:
  uint32_t m_speedLimit;
  laneboundarypkid m_leftBoundaryID;
  laneboundarypkid m_rightBoundaryID;
  HADMAP_LANE_TYPE m_laneType;
  HADMAP_LANE_ARROW m_laneArrow;
  roadpkid m_roadID;
  sectionpkid m_sectionID;
  lanepkid m_laneID;
  double m_laneWidth;
  double m_length;
  double m_distanceToEnd;
  std::string m_name;
  bool m_enabled;

  CLaneBoundaryData* m_boundarys[2];
  PointsVec m_points;
  tagGeneratedMesh m_generatedMesh;
};

typedef std::vector<CLaneData*> Lanes;
