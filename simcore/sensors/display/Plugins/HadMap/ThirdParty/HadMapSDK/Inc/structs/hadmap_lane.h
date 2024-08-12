// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/base_struct.h"
#include "structs/hadmap_laneboundary.h"
#include "structs/hadmap_predef.h"

namespace hadmap {
class TXSIMMAP_API txLane : public txLinkObject {
 private:
  // lane member variables
  struct LaneData;
  std::unique_ptr<LaneData> instancePtr;

 public:
  txLane();

  txLane(const txLane& lane);

  ~txLane();

 public:
  // functional operations
  // get info of cur lane

  tx_lane_t& getTxLaneData() const;
  // get link type
  // return LINK_TYPE_LANE
  LINK_TYPE getLinkType() const;

  // get geometry
  const txCurve* getGeometry() const;

  // get geometry
  const txCurve* getDenseGeometry() const;

  // get left boundary
  txLaneBoundaryPtr getLeftBoundary() const;

  // get left boundary id
  laneboundarypkid getLeftBoundaryId() const;

  // get right boundary
  txLaneBoundaryPtr getRightBoundary() const;

  // get right boundary id
  laneboundarypkid getRightBoundaryId() const;

  // get lane type ( Driving | Biking | ... )
  LANE_TYPE getLaneType() const;

  // get lane arrow ( Straight | Left | Uturn | ... )
  LANE_ARROW getLaneArrow() const;

  // get speed limit
  uint32_t getSpeedLimit() const;

  // get lane material surface
  uint32_t getLaneSurface() const;

  // get lane material Friction
  double getLaneFriction() const;

  // get material sOffset
  double getMaterialOffset() const;

  // get road id
  roadpkid getRoadId() const;

  // get section id
  sectionpkid getSectionId() const;

  // get lane id
  lanepkid getId() const;

  // get txLaneId
  txLaneId getTxLaneId() const;

  // get distance to end
  // normal : summarize length of geom in next section or next road
  // 2 -> 1 : one is normal, one is 0
  double getDistanceToEnd() const;

  // get length of the geometry of cur lane
  double getLength() const;

  // get lane width
  double getLaneWidth() const;

  // get lane width from s
  double getLaneWidth(double s) const;

  // get name
  std::string getName() const;

  // is lane valid for driving
  bool isEnable() const;

  // get section ptr
  txSectionPtr getSection() const;

  // get unique id
  std::string getUniqueId() const;

 public:
  // sl coord

  // xy coord same with geom
  bool xy2sl(const double& x, const double& y, double& s, double& l);

  bool xy2sl(const double& x, const double& y, double& s, double& l, double& yaw);

  // sl to xy
  bool sl2xy(const double& s, const double& l, double& x, double& y);

  bool sl2xy(const double& s, const double& l, double& x, double& y, double& yaw);

  // s -> yaw
  double getYaw(const double& s);

 public:
  // functional operations
  // set info

  // set name
  txLane& setName(const std::string& name);

  // set geometry
  txLane& setGeometry(const txCurve* curvePtr);

  txLane& setGeometry(const txCurve& curveIns);

  txLane& setGeometry(const PointVec& points, CoordType ct);

  txLane& setDenseGeometry(const txCurve& curveIns);

  txLane& setDenseGeometry(const PointVec& points, CoordType ct);

  // transfer
  // only be used when geom is in ENU
  txLane& transfer(const txPoint& oldEnuC, const txPoint& newEnuC);

  // transfer to enu
  txLane& transfer2ENU(const txPoint& enuCenter);

  // set left boundary
  txLane& setLeftBoundary(const txLaneBoundaryPtr& boundaryPtr);

  // set left boundary id
  // if boundaryId != leftBoundary.getId()
  // then leftBoundary will be seted to NULL
  txLane& setLeftBoundaryId(laneboundarypkid boundaryId);

  // set right boundary
  txLane& setRightBoundary(const txLaneBoundaryPtr& boundaryPtr);

  // set right boundary id
  // if boundaryId != rightBoundary.getId()
  // then rightBoundary will be seted to NULL
  txLane& setRightBoundaryId(laneboundarypkid boundaryId);

  // set lane type
  txLane& setLaneType(LANE_TYPE type);

  // set lane arrow
  txLane& setLaneArrow(LANE_ARROW arrow);

  // set speed limit
  txLane& setSpeedLimit(uint32_t speed);

  // set lane material surface
  txLane& setLaneSurface(uint32_t surface);

  // set lane material Friction
  txLane& setLaneFriction(double friction);

  // set material sOffset
  txLane& setMaterialOffset(double soffset);

  // set road id
  txLane& setRoadId(roadpkid roadId);

  // set section id
  txLane& setSectionId(sectionpkid sectionId);

  // set lane id
  txLane& setId(lanepkid laneId);

  // set distance to end
  txLane& setDistanceToEnd(double disToEnd);

  // set lane width
  txLane& setLaneWidth(double width);

  // set lane is valid for driving
  txLane& setEnable(bool enable);

  // set section pointer
  txLane& setSection(const txSectionPtr& secPtr);

  // reverse data
  // reverse cur lane geometry data
  // change lane_id = lane_id * -1
  bool reverse();

  // exchange left & right boundary index
  bool exchangeBoundaryIndex();
};
}  // namespace hadmap
