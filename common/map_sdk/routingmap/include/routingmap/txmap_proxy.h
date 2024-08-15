// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>

#include "mapengine/hadmap_engine.h"
#include "structs/hadmap_map.h"

namespace hadmap {
class txMapProxy : public txMapInterface {
 public:
  explicit txMapProxy(CoordType mapGeomCoordType);

  ~txMapProxy();

  txMapProxy(const txMapProxy&) = delete;

 public:
  // get geom coord type of cur map
  CoordType getGeomCoordType();

  // get projection info
  // if coordtype is not enu, then return null
  // this function should be called after frame loc update
  txProjectionConstPtr getProjection();

  // get road info by id
  // if roadId is invalid or specified road not in cur map, then return null
  txRoadPtr getRoadById(const roadpkid& roadId);

  // get lane info by id
  // if laneId is invalid or specified lane not in cur map, then return null
  txLanePtr getLaneById(const txLaneId& laneId);

  // get lanes with specified area
  // pos coord is the same with map geom coord
  // radius -> meter
  txLanes getLanes(const txPoint& pos, const double& radius);

  // nearest lane by distance
  // default search radius is 20m
  txLanePtr nearestLane(const txPoint& pos, double radius);

  // nearest lane by distance and yaw
  // yaw -> [ -PI, PI ), east -> 0, north -> PI/2
  // default search radius is 20m
  txLanePtr nearestLane(const txPoint& pos, double radius, const double& yaw);

  // next lanes on map
  txLanes getNextLanes(const txLanePtr& lanePtr);

  // pre lanes on map
  txLanes getPreLanes(const txLanePtr& lanePtr);

  // get left lane on map
  txLanePtr getLeftLane(const txLanePtr& lanePtr);

  // get right lane on map
  txLanePtr getRightLane(const txLanePtr& lanePtr);

  txLaneBoundaries getLaneBoundaries(const txPoint& pos, const double& yaw, const std::pair<double, double>& range);

  txObjectPtr getObject(const objectpkid& objId);

  txObjectPtr getObject(const txPoint& pos, const OBJECT_TYPE& obj_type);

  txObjects getObjects(const PointVec& envelope, const std::vector<OBJECT_TYPE>& objTypes);

  txObjects getObjects(const PointVec& envelope, const double& yaw, const std::vector<OBJECT_TYPE>& objTypes);

  txObjects getObjects(const std::vector<OBJECT_TYPE>& objTypes, const std::vector<txLaneId>& relIds);

  txObjects getObjects(const txPoint& pos, const double& yaw, const std::pair<double, double>& range,
                       const std::vector<OBJECT_TYPE>& objTypes);

  txJunctionPtr getJunction(const junctionpkid& jId);

  txJunctions getJunctions();

  PRIORITY_TYPE priorityCmp(const txJunctionRoadPtr& curPtr, const txJunctionRoadPtr& cmpPtr);

 public:
  // update center
  bool updateCenter(const txPoint& lonlatele);

 public:
  // insert data
  bool insertRoad(const txRoadPtr& roadPtr);

  bool insertRoads(const txRoads& roads);

  bool insertLaneLink(const txLaneLinkPtr& linkPtr);

  bool insertLaneLinks(const txLaneLinks& links);

  bool insertObject(const txObjectPtr& objPtr);

  bool insertObjects(const txObjects& objects);

  bool insertJunction(const txJunctionPtr& juncPtr);

  bool insertJunctions(const txJunctions& juncs);

 public:
  // remove data
  bool removeRoad(roadpkid roadId);

  bool removeObject(objectpkid objId);

  bool removeJunction(junctionpkid jId);

 private:
  std::unique_ptr<hadmap::txMap> mapPtr;
};
}  // namespace hadmap
