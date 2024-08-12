// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/base_struct.h"
#include "structs/hadmap_section.h"

#include "structs/map_structs.h"
namespace hadmap {
class TXSIMMAP_API txRoad : public txLinkObject {
 private:
  // road member variables
  struct RoadData;
  std::unique_ptr<RoadData> instancePtr;

 public:
  txRoad();

  txRoad(roadpkid id, const std::string& name);

  txRoad(const txRoad& roadIns);

  virtual ~txRoad();

 public:
  virtual bool isJunction() const;

  virtual std::string getUniqueId() const;

 public:
  // get info
  tx_road_t& getTxData() const;

  // get link type ( LINK_TYPE_ROAD )
  LINK_TYPE getLinkType() const;

  // ger road id
  roadpkid getId() const;

  // get task id
  taskpkid getTaskId() const;

  // get road name
  std::string getName() const;

  // get length
  // sum the length of each section in this road
  double getLength() const;

  // get road geometry
  const txCurve* getGeometry() const;

  // get ground
  double getGround() const;

  // get speed limit
  uint32_t getSpeedLimit() const;

  // get road type
  ROAD_TYPE getRoadType() const;

  // get direction
  // 0 - unknown, 1 - two, 2 - same, 3 - reverse
  uint8_t getDirection() const;

  // get toll state
  // 1 - toll, 2 - free, 3 - free road section along a toll road
  uint8_t getToll() const;

  // get function class
  // 0 - high, 1 - city high, 2 - state road ...
  uint8_t getFuncClass() const;

  // is urban
  bool isUrban() const;

  // is paved
  bool isPaved() const;

  // get transtion type
  uint8_t getTransType() const;

  // has lane marking
  bool hasLaneMarking() const;

  // get material
  ROAD_MATERIAL getMaterial() const;

  // get all sections
  txSections& getSections() const;

  // get specified lane
  txLanePtr getLane(sectionpkid sectionId, lanepkid laneId) const;

  // get Curvature
  txCurvatureVec getCurvature() const;

  // get Slope
  txSlopeVec getSlope() const;

  // getRoadPreLink
  txRoadLink getRoadPreLink() const;

  // getRoadToLink
  txRoadLink getRoadToLink() const;

  // get ControlPoint
  txControlPoint getControlPoint() const;

  // get EleControlPoint
  txControlPoint getElePoint() const;

  // get Crg
  txOpenCrgVec getCrgVec() const;

  // bidirectional road flag
  bool isBidirectional() const;

  // txPoint getRefpoint() const;
 public:
  // set info

  // set road id and lane( road id )
  txRoad& setId(roadpkid id);

  // set road name
  txRoad& setName(const std::string& name);

  // set geometry of cur road
  txRoad& setGeometry(const txCurve* curvePtr);

  txRoad& setGeometry(const txCurve& curveIns);

  txRoad& setGeometry(const PointVec& points, CoordType ct);

  // transfer geom
  // only be used when geom is in ENU
  txRoad& transfer(const txPoint& oldEnuC, const txPoint& newEnuC);

  // transfer to enu
  // only be used when geom is in wgs84
  txRoad& transfer2ENU(const txPoint& enuCenter);

  // set speed limit
  txRoad& setSpeedLimit(uint32_t speed);

  // set ground
  txRoad& setGround(double g);

  // set road type
  txRoad& setRoadType(ROAD_TYPE type);

  // set direction
  txRoad& setDirection(uint8_t direction);

  // set toll
  txRoad& setToll(uint8_t toll);

  // set func class
  txRoad& setFuncClass(uint8_t funcClass);

  // set urban
  txRoad& setUrban(bool flag);

  // set paved
  txRoad& setPaved(bool flag);

  // set transtion type
  txRoad& setTransType(uint8_t transType);

  // set lane marking
  txRoad& setLaneMarking(bool flag);

  // set road material
  txRoad& setMaterial(ROAD_MATERIAL material);

  // add section
  txRoad& addSection(const txSectionPtr& sectionPtr);

  // set sections
  // replace origin sections
  txRoad& setSections(const txSections& sections);

  // set task id
  txRoad& setTaskId(taskpkid taskId);

  // set Curvature
  txRoad& setCurvature(txCurvatureVec vec);

  // set slope
  txRoad& setSlope(txSlopeVec vec);

  // set prelink
  txRoad& setPreLink(txRoadLink& link);

  // set endlink
  txRoad& setToLink(txRoadLink& link);

  // set controlPoint
  txRoad& setControlPoint(txControlPoint& point);

  // set EleControlPoint
  txRoad& setElePoint(txControlPoint ele);

  // set opencrg
  txRoad& setCrgs(txOpenCrgVec crgs);

  // reverse data
  // reverse road geometry
  // exchange section id
  bool reverse();

  // exchange boundary index
  bool exchangeBoundaryIndex(bool curLane, bool oppositeLane);

  // bind parrent road ptr to sections
  // call this function after copy constructor called
  txRoad& bindRoadPtr();

 public:
  // build inner topo relation
  bool buildInnerTopo();

  // build topo relation from current road to next road
  bool buildOuterTopo(const txRoadPtr& nextRoadPtr);
};
}  // namespace hadmap
