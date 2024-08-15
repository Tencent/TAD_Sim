// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/base_struct.h"
#include "structs/hadmap_predef.h"

#include "structs/map_structs.h"
namespace hadmap {
class TXSIMMAP_API txLaneLink : public txLinkObject {
 private:
  struct LaneLinkData;
  std::unique_ptr<LaneLinkData> instancePtr;

 public:
  txLaneLink();

  txLaneLink(const txLaneLink& link);

  ~txLaneLink();

 public:
  //
  tx_lanelink_t getTxData() const;

  // get info
  lanelinkpkid getId() const;

  // get link type ( LINK_TYPE_LANELINK )
  LINK_TYPE getLinkType() const;

  // get from road id
  roadpkid fromRoadId() const;

  // get from section id
  sectionpkid fromSectionId() const;

  // get from lane id
  lanepkid fromLaneId() const;

  // get from txLaneId
  txLaneId fromTxLaneId() const;

  // get to road id
  roadpkid toRoadId() const;

  // get to section id
  sectionpkid toSectionId() const;

  // get to lane id
  lanepkid toLaneId() const;

  // get to txLaneId
  txLaneId toTxLaneId() const;

  // get origin geometry
  const txCurve* getGeometry() const;

  // get origin roadid
  const roadpkid getOdrRoadId() const;

  // get unique id
  std::string getUniqueId() const;

  // get junction id
  junctionpkid getJunctionId() const;

  // get controlPoint
  txControlPoint getControlPoint() const;

  // get eleControlPoint
  txControlPoint getEleControlPoint() const;

  // get curvature
  txCurvatureVec getCurvature() const;

  // get slope
  txSlopeVec getSlope() const;

  // get mean slope
  double getMeanSlope() const;

  // get pre contact
  txContactType getPreContact();

  // get suc contact
  txContactType getSuccContact();

  // get mean curvature
  double getMeanCurvature() const;

  // get left boundaries
  const txLaneBoundaries& getLeftBoundaries() const;

  // get right boundaries
  const txLaneBoundaries& getRightBoundaries() const;

  double getDeltaDierction() const;

  // unit: m
  double getRadius();
  // unit: km/h
  double getSpeedLimit();

 public:
  bool operator==(const txLaneLink& link) const;

  std::string linkIndex() const;

 public:
  // set info
  txLaneLink& setId(lanelinkpkid id);

  // set from road id
  txLaneLink& setFromRoadId(roadpkid roadId);

  // set from section id
  txLaneLink& setFromSectionId(sectionpkid sectionId);

  // set from lane id
  txLaneLink& setFromLaneId(lanepkid laneId);

  // set to road id
  txLaneLink& setToRoadId(roadpkid roadId);

  // set to section id
  txLaneLink& setToSectionId(sectionpkid sectionId);

  // set to lane id
  txLaneLink& setToLaneId(lanepkid laneId);

  // set orignal roadId
  txLaneLink& setOdrRoadId(roadpkid roadId);

  // set control point
  txLaneLink& setControlPoint(txControlPoint control);

  // set control point
  txLaneLink& setEleControlPoint(txControlPoint control);

  // set origin geometry
  txLaneLink& setGeometry(const txCurve* curvePtr);

  txLaneLink& setGeometry(const txCurve& curveIns);

  txLaneLink& setGeometry(const PointVec& points, CoordType ct);

  // Set the curvature
  txLaneLink& setCurvature(const txCurvatureVec& curvatureVec);

  // Set the slope
  txLaneLink& setSlope(const txSlopeVec& slope);

  // Set the pre contact
  txLaneLink& setPreContact(const txContactType& preContact);

  // Set the succ contact
  txLaneLink& setSuccContact(const txContactType& succContact);

  // Set the junction ID
  txLaneLink& setJunctionId(junctionpkid jid);

  // Set the left boundaries
  txLaneLink& setLeftBoundaries(const txLaneBoundaries& bds);

  // Set the right boundaries
  txLaneLink& setRightBoundaries(const txLaneBoundaries& bds);
};
}  // namespace hadmap
