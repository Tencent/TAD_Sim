// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/base_struct.h"
#include "structs/hadmap_lane.h"
#include "structs/hadmap_predef.h"

#include <memory>
#include <string>

namespace hadmap {
class TXSIMMAP_API txSection : public std::enable_shared_from_this<txSection> {
 private:
  // section member
  struct SectionData;
  std::unique_ptr<SectionData> instancePtr;

 public:
  txSection();

  txSection(const txSection &section);

  ~txSection();

 public:
  // get section info

  // get section id
  sectionpkid getId() const;

  // get road id
  roadpkid getRoadId() const;

  // get length
  // ave length of each lane data
  double getLength() const;

  // get slope
  double getMeanSlope() const;

  // get meanCurvature
  double getMeanCurvature() const;

  // get lane from lane id
  txLanePtr &getLane(int8_t laneid) const;

  // get lanes of cur section
  txLanes &getLanes() const;

  // get right lanes
  std::vector<hadmap::txLaneId> getRightLanes() const;

  // get left lanes
  std::vector<hadmap::txLaneId> getLeftLanes() const;

  // get start percent
  double getStartPercentOfRoad() const;

  // get end percent
  double getEndPercentOfRoad() const;

  // get boundaries of cur section
  // from left to right
  txLaneBoundaries getBoundaries() const;

  // get boundaries of cur section by specified range
  // from left to right
  // boundary geom may be cut by range
  // start < 0 : from first point to end
  // end < 0 | end > length : from start to last point
  txLaneBoundaries getBoundariesByRange(const double &start, const double &end) const;

  // get distance to junction
  double getDistanceToJunction() const;

  // get specified lane
  txLanePtr get(lanepkid laneId) const;

  txLanePtr get(const std::string &name) const;

  // get road ptr
  txRoadPtr getRoad() const;

 public:
  // set section info

  // set road id
  txSection &setRoadId(roadpkid id);

  // set section id
  txSection &setId(sectionpkid id);

  // set lanes
  txSection &setLanes(const txLanes &lanes);

  // add lane
  // name and id of current lane should be different from other lanes in current section
  txSection &add(const txLanePtr &lanePtr);

  // remove lane
  txSection &removeLane(lanepkid laneId);

  // set distance to junction
  txSection &setDistanceToJunction(double d);

  // set mean slope
  txSection &setMeanSlope(double d);

  // set mean curvature
  txSection &setMeanCurvature(double d);

  // set road ptr
  txSection &setRoad(const txRoadPtr roadPtr);

  // reverse data
  // exchange lane order
  // reverse lane boundary data
  bool reverse();

  // exchange lane boundary index
  bool exchangeBoundaryIndex(bool curLane, bool oppositeLane);

  // bind section ptr to lanes
  txSection &bindSectionPtr();
};
}  // namespace hadmap
