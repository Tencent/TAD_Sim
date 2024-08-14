// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/base_struct.h"
#include "structs/hadmap_junction.h"
#include "structs/hadmap_predef.h"

#include <memory>

namespace hadmap {
class txMapInterface;
using txMapInterfacePtr = std::shared_ptr<txMapInterface>;

class TXSIMMAP_API txMapInterface {
 public:
  struct TXSIMMAP_API txProjectionInterface {
    // wgs84 -> enu
    virtual void transform(double& x, double& y) const = 0;

    // enu -> wgs84
    virtual void untransform(double& x, double& y) const = 0;
  };

  struct TXSIMMAP_API txProjection : public txProjectionInterface {
    friend class txMap;

   private:
    pkid_t pId;
    txPoint originLL;

    //
    double dx;
    double dy;

   public:
    txProjection() : pId(0), dx(0.0), dy(0.0) {}
    // transfer point lonlat to enu
    void lonlat2enu(const txPoint& lonlat, txPoint& enu) const;

    // transfer point enu to lonlat
    void enu2lonlat(const txPoint& enu, txPoint& lonlat) const;

    // transfer lonlat to enu
    void transform(double& x, double& y) const;

    // transfer enu to lonlat
    void untransform(double& x, double& y) const;

    // get X
    double dX() const;

    // get Y
    double dY() const;

    pkid_t pID() const;
  };

  using txProjectionConstPtr = std::shared_ptr<const txProjection>;

  using txProjectionInterfaceConstPtr = std::shared_ptr<const txProjectionInterface>;

 public:
  txMapInterface() {}

  virtual ~txMapInterface() {}

  txMapInterface(const txMapInterface&) = delete;

 public:
  // only have query interface

  // get geom coord type of cur map
  virtual CoordType getGeomCoordType() = 0;

  // get projection info
  // if coordtype is not enu, then return null
  // this function should be called after frame loc update
  virtual txProjectionConstPtr getProjection() = 0;

  // get road info by id
  // if roadId is invalid or specified road not in cur map, then return null
  virtual txRoadPtr getRoadById(const roadpkid& roadId) = 0;

  // get lane info by id
  // if laneId is invalid or specified lane not in cur map, then return null
  virtual txLanePtr getLaneById(const txLaneId& laneId) = 0;

  // get lanes with specified area
  // pos coord is the same as map geom coord
  // radius -> meter
  virtual txLanes getLanes(const txPoint& pos, const double& radius) = 0;

  // nearest lane by distance
  // pos coord is same as map geom coord
  // default search radius is 10m
  virtual txLanePtr nearestLane(const txPoint& pos, double radius) = 0;

  // nearest lane by distance and yaw
  // pos coord is same as map geom coord
  // yaw -> [ -PI, PI ), east -> 0, north -> PI/2
  // default search radius is 10m
  virtual txLanePtr nearestLane(const txPoint& pos, double radius, const double& yaw) = 0;

  // next lanes on map
  virtual txLanes getNextLanes(const txLanePtr& lanePtr) = 0;

  // pre lanes on map
  virtual txLanes getPreLanes(const txLanePtr& lanePtr) = 0;

  // left lane on map
  // get lanePtr in the same section
  // if cur lane is most left lane, then return NULL
  virtual txLanePtr getLeftLane(const txLanePtr& lanePtr) = 0;

  // right lane on map
  // get lanePtr in the same section
  // if cur lane is most right lane, then return NULL
  virtual txLanePtr getRightLane(const txLanePtr& lanePtr) = 0;

  // get lane boundaries
  // from left to right, on cur road with specified range
  // pos coord is same as map geom coord
  // yaw -> [ -PI, PI ), east -> 0, north -> PI/2
  // range.frist <----- pos ------> range.second
  // range.frint ( -N, 0.0 ]
  // range.second [ 0.0, N )
  virtual txLaneBoundaries getLaneBoundaries(const txPoint& pos, const double& yaw,
                                             const std::pair<double, double>& range) = 0;

  // get objects by envelope
  // if objTypes is empty, then get objects with any type
  virtual txObjects getObjects(const PointVec& envelope, const std::vector<OBJECT_TYPE>& objTypes) = 0;

  // get objects by envelope and yaw
  // if objTypes is empty, then get objects with any type
  // yaw -> [ -PI, PI ), east -> 0, north -> PI/2
  // yaw only used from road sign | traffic sign | lane boundary
  virtual txObjects getObjects(const PointVec& envelope, const double& yaw,
                               const std::vector<OBJECT_TYPE>& objTypes) = 0;

  // get objects by pos, filter by yaw and range
  // if objTypes is empty, then get objects with any type
  // yaw -> [ -PI, PI ), east -> 0, north -> PI/2
  // yaw only used from road sign | traffic sign | lane boundary
  // range.frist <----- pos ------> range.second
  // range.frint ( -N, 0.0 ]
  // range.second [ 0.0, N )
  virtual txObjects getObjects(const txPoint& pos, const double& yaw, const std::pair<double, double>& range,
                               const std::vector<OBJECT_TYPE>& objTypes) = 0;

  // get object by id
  virtual txObjectPtr getObject(const objectpkid& objId) = 0;

  // get area object which the specified pos within
  // area type [ DrivingArea, CrossWalk ]
  virtual txObjectPtr getObject(const txPoint& pos, const OBJECT_TYPE& obj_type) = 0;

  // get objects by type & relids
  virtual txObjects getObjects(const std::vector<OBJECT_TYPE>& objTypes, const std::vector<txLaneId>& relIds) = 0;

  // get junction by id
  virtual txJunctionPtr getJunction(const junctionpkid& jId) = 0;

  virtual txJunctions getJunctions() = 0;

  // junction road priority compare
  // if curPtr and cmpPtr are not in the same junction, return PRIORITY_NONE
  // if curPtr and cmpPtr are not affect each other, return PRIORITY_NONE
  // if curPtr and cmpPtr are with same priority, return PRIORITY_SAME
  // if curPtr has higher priority than cmpPtr, return PRIORITY_HIGH
  // if cmmPtr has higher priority than curPtr, return PRIORITY_LOW
  virtual PRIORITY_TYPE priorityCmp(const txJunctionRoadPtr& curPtr, const txJunctionRoadPtr& cmpPtr) = 0;
};
}  // namespace hadmap
