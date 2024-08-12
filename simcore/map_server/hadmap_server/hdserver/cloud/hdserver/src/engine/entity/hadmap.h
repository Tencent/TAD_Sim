/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once

#include <string>
#include "../../xml_parser/entity/query_params.h"
#include "./aabb.h"
#include "common/map_data/map_data_predef.h"
#include "common/map_data/map_query.h"
#include "common/xml_parser/entity/signlight.h"

class Ray;

class CVertexArrayObject;
class CMaterial;

class CHadmap {
 public:
  typedef std::vector<CVertexArrayObject*> VaoVec;
  CHadmap();
  ~CHadmap();

  void Clear();
  void Reset();

  void PushRoad(CRoadData* road);
  void PushRoad(RoadVec& roads);
  void PushLaneLink(CLaneLink* link);
  void PushLaneLink(LaneLinkVec& links);
  void PushObject(CMapObject* obj);
  void PushObject(MapObjectVec& objects);

  void PushTrafficLights(LogicLightVec& lights);
  void PushJunctionTraffics(std::map<int, std::map<int, int>>& junctions);
  void RearrangeData();

  LogicLightVec& GetTrafficLights();
  std::map<int, std::map<int, int>>& GetJunction2TrafficRoadMap();

  void Intersect(Ray& ray);
  int GetRoadByPointLL(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& usecID);
  int GetLaneByPointLL(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& usecID, lanepkid& laneID,
                       double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dDist, double& dWidth,
                       double& dOffset);
  int GetLaneByPointLL(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& usecID, lanepkid& laneID,
                       double& dLaneLon, double& dLaneLat, double& dLaneAlt, double& dDist, double& dWidth,
                       double& dOffset, std::string& rname);
  int GetLanelinkByPointLL(double dLon, double dLat, lanelinkpkid& lanelinkid, double& dLanelinkLon,
                           double& dLanelinkLat, double& dLanelinkAlt, double& dDist, double& dOffset);

  int LocalToLonlat(double& dLon, double& dLat, double& dAlt);
  int LonlatToLocal(double& dLon, double& dLat, double& dAlt);
  int QueryNearbyInfo(double dLon, double dLat, sTagQueryNearbyInfoResult& result);
  int QueryNearbyLane(double dLon, double dLat, roadpkid& uRoadID, sectionpkid& usecID, lanepkid& laneID);
  int QueryNextLane(roadpkid uRoadID, sectionpkid usecID, lanepkid laneID, roadpkid& nextRoadID, sectionpkid& nextSecID,
                    lanepkid& nextLaneID);
  int QueryNextLanes(const roadpkid& uRoadID, const sectionpkid& uSecID, const lanepkid& laneID, LaneIDVec& outLaneVec);

  int QueryLonLat(roadpkid uRoadID, sectionpkid usecID, lanepkid laneID, double dShift, double dOffset, double& dLon,
                  double& dLat, double& dAlt, double& dWidth);
  int QueryLonLatByPoint(double dLon, double dLat, lanepkid laneID, double dShift, double dOffset, double& dFinalLon,
                         double& dFinalLat, double& dFinalAlt, double& dWidth, double& dFinalShift, bool start = false);

  int QueryForwardPoints(sTagGetForwardPointsParam&, std::vector<std::vector<hadmap::txLaneId>>*,
                         std::vector<std::pair<double, hadmap::PointVec>>*);
  int QueryLonLatByPointOnLanelink(double dLon, double dLat, lanelinkpkid laneID, double dShift, double dOffset,
                                   double& dFinalLon, double& dFinalLat, double& dFinalAlt, double& dFinalShift);

  RoadVec& Roads() { return m_roads; }
  RoadMap& Roadsmap() { return m_roadmap; }
  MapObjectVec& Objects() { return m_objects; }
  LaneLinkVec& LaneLinks() { return m_links; }
  LaneVec& Lanes() { return m_lanes; }
  LaneBoundaryVec& LaneBoundaries() { return m_laneboundaries; }

  CMapQuery& MapQuery() { return m_map_query; }
  void ProcessMapData();

  EntityVec& Selected() { return m_entity_selected; }

  CRoadData* FindRoad(roadpkid id);
  CSectionData* FindSection(roadpkid roadId, sectionpkid secId);
  CLaneData* FindLane(roadpkid roadId, sectionpkid secId, lanepkid Laneid);
  CLaneBoundaryData* FindLaneBoundary(laneboundarypkid id);
  CLaneLink* FindLaneLink(lanelinkpkid id);
  CMapObject* FindMapObject(objectpkid id);
  CAABB& AabbXyz() { return m_aabb_xyz; }
  CAABB& AabbLl() { return m_aabb_ll; }
  CVector3& Center() { return m_center; }
  std::string Name() { return m_strFileName; }
  std::string FullPath() { return m_strFullPath; }
  std::string Etag() { return m_strEtag; }
  CPoint3D& RefPoint() { return m_ref_point_ll; }
  int64_t LastModifiedTime() { return m_llLastModifiedTime; }

  void SetFileName(std::string strName) { m_strFileName = strName; }
  void SetFullPath(std::string strFullPath);
  void SetEtag(std::string strEtag) { m_strEtag = strEtag; }
  void SetLastModifiedTime(int64_t time) { m_llLastModifiedTime = time; }
  void SetAaabbXyz(CAABB& aabb) { m_aabb_xyz = aabb; }
  void SetAabbLl(CAABB& aabb) { m_aabb_ll = aabb; }
  void SetCenter(CVector3& center) { m_center = center; }
  void SetRefPoint(CPoint3D& point);
  void SetSelected(EntityVec& entitys);

#ifndef __HADMAP_SERVER__
  void Draw();
  void PrepareData();

 protected:
  void PrepareGeomData();
  void PrepareRoadData();
  void PrepareLaneData();
  void PrepareLaneBoundaryData();
  void PrepareLaneLinkData();
  void PrepareMapObjectData();
  void PrepareMatData();
#endif

 protected:
  std::string m_strFileName;
  std::string m_strFullPath;
  std::string m_strEtag;
  int64_t m_llLastModifiedTime;
  CAABB m_aabb_ll;
  CAABB m_aabb_xyz;
  CVector3 m_center;
  CMapQuery m_map_query;
  CPoint3D m_ref_point_ll;

  RoadVec m_roads;
  SectionVec m_sections;
  LaneVec m_lanes;
  LaneBoundaryVec m_laneboundaries;
  LaneLinkVec m_links;
  MapObjectVec m_objects;

  LaneVec m_lanes_selected;
  EntityVec m_entity_selected;

  RoadMap m_roadmap;
  SectionMap m_sectionmap;
  LaneMap m_lanemap;
  LaneBoundaryMap m_laneboundarymap;
  MapObjectMap m_objectmap;
  LaneLinkIDMap m_lanelinkmap;
  LaneLinkMap m_fromlanemap;
  LaneLinkMap m_tolanemap;

  LogicLightVec m_lights;
  std::map<int, std::map<int, int>> m_junction2roadid;

  CVertexArrayObject* m_vao_road;
  CVertexArrayObject* m_vao_lane;
  CVertexArrayObject* m_vao_lane_boundary;
  CVertexArrayObject* m_vao_lane_link;
  CVertexArrayObject* m_vao_map_object;
  VaoVec m_vao_lane_meshes;

  CMaterial* m_material;
  CMaterial* m_mat_road;
  CMaterial* m_mat_lane;
  CMaterial* m_mat_boundary;
  CMaterial* m_mat_link;
  CMaterial* m_mat_object;
};
