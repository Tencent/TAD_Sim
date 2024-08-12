// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_section.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_road.h"

#include <algorithm>
#include <exception>
#include <iostream>
#include <set>
#include <unordered_map>
#include <unordered_set>
namespace hadmap {
struct txSection::SectionData {
 public:
  // section id
  sectionpkid id;
  // lanes data
  txLanes lanes;
  // distance to junction
  double distanceToJunction;
  //
  double meanSlope;
  //
  double meanCurvature;
  // start of road
  double startPercent;
  // end of road
  double endPercent;
  // road pointer
  std::weak_ptr<txRoad> roadPtr;

 public:
  SectionData() : distanceToJunction(-1.0) {}

  ~SectionData() {}

 public:
  // get info

  // get id
  sectionpkid getId() const { return id; }

  // get road id
  roadpkid getRoadId() const {
    if (!lanes.empty()) return lanes[0]->getRoadId();
    if (roadPtr.lock()) {
      return roadPtr.lock()->getId();
    }
    return SECT_PKID_INVALID;
    throw std::runtime_error("Get Road Id Error, Lane Empty");
  }

  // get length
  // ave length of each lane geometry
  double getLength() const;

  // get lanes
  txLanes &getLanes();

  // get boundaries
  txLaneBoundaries getBoundaries();

  // get distance to junction
  double getDistanceToJunction() const;

  // get specified lane
  txLanePtr get(lanepkid laneId) const;

  txLanePtr get(const std::string &name) const;

 public:
  // set info

  // set road id
  void setRoadId(roadpkid id);

  // set id
  void setId(sectionpkid id);

  // set distance to junction
  void setDistanceToJunction(double d);
};

double txSection::SectionData::getLength() const {
  double len = 0.0;
  size_t count = 0;
  for (size_t i = 0; i < lanes.size(); ++i) {
    double dis = lanes[i]->getLength();
    if (dis > 1e-5) {
      len += dis;
      count += 1;
    }
  }
  if (count > 0) {
    return len / count;
  } else {
    return 0.0;
  }
}

txLanes &txSection::SectionData::getLanes() { return lanes; }

txLaneBoundaries txSection::SectionData::getBoundaries() {
  std::unordered_set<laneboundarypkid> ids;
  txLaneBoundaries boundaries;
  for (auto &lane_ptr : lanes) {
    if (ids.find(lane_ptr->getLeftBoundaryId()) == ids.end()) {
      ids.insert(lane_ptr->getLeftBoundaryId());
      boundaries.push_back(lane_ptr->getLeftBoundary());
    }
    if (ids.find(lane_ptr->getRightBoundaryId()) == ids.end()) {
      ids.insert(lane_ptr->getRightBoundaryId());
      boundaries.push_back(lane_ptr->getRightBoundary());
    }
  }
  return boundaries;
}

double txSection::SectionData::getDistanceToJunction() const { return distanceToJunction; }

txLanePtr txSection::SectionData::get(lanepkid laneId) const {
  for (size_t i = 0; i < lanes.size(); ++i) {
    if (lanes[i]->getId() == laneId) return lanes[i];
  }
  return txLanePtr();
}

txLanePtr txSection::SectionData::get(const std::string &name) const {
  for (size_t i = 0; i < lanes.size(); ++i) {
    if (lanes[i]->getName() == name) return lanes[i];
  }
  return txLanePtr();
}

void txSection::SectionData::setRoadId(roadpkid id) {
  for (size_t i = 0; i < lanes.size(); ++i) lanes[i]->setRoadId(id);
}

void txSection::SectionData::setId(sectionpkid id) {
  this->id = id;
  for (size_t i = 0; i < lanes.size(); ++i) {
    lanes[i]->setSectionId(id);
  }
}

void txSection::SectionData::setDistanceToJunction(double d) { this->distanceToJunction = d; }

//
// txSection
//
txSection::txSection() : instancePtr(new SectionData) {}

txSection::txSection(const txSection &section) : instancePtr(new SectionData) {
  instancePtr->id = section.instancePtr->id;
  instancePtr->distanceToJunction = section.instancePtr->distanceToJunction;
  instancePtr->meanSlope = section.instancePtr->meanSlope;
  instancePtr->meanCurvature = section.instancePtr->meanCurvature;

  txLaneBoundaries boundaries;
  std::unordered_map<laneboundarypkid, size_t> boundaryIndex;
  txLanes &lanes = section.instancePtr->lanes;
  for (size_t i = 0; i < lanes.size(); ++i) {
    txLaneBoundaryPtr leftPtr = lanes[i]->getLeftBoundary();
    if (leftPtr != NULL) {
      if (boundaryIndex.find(leftPtr->getId()) == boundaryIndex.end()) {
        boundaryIndex.insert(std::make_pair(leftPtr->getId(), boundaries.size()));
        boundaries.push_back(txLaneBoundaryPtr(new txLaneBoundary(*leftPtr)));
      }
    }
    txLaneBoundaryPtr rightPtr = lanes[i]->getRightBoundary();
    if (rightPtr != NULL) {
      if (boundaryIndex.find(rightPtr->getId()) == boundaryIndex.end()) {
        boundaryIndex.insert(std::make_pair(rightPtr->getId(), boundaries.size()));
        boundaries.push_back(txLaneBoundaryPtr(new txLaneBoundary(*rightPtr)));
      }
    }
  }

  for (size_t i = 0; i < lanes.size(); ++i) {
    txLanePtr curLanePtr(new txLane(*lanes[i]));
    if (lanes[i]->getLeftBoundary() != NULL) {
      curLanePtr->setLeftBoundary(boundaries[boundaryIndex[lanes[i]->getLeftBoundaryId()]]);
    }
    if (lanes[i]->getRightBoundary() != NULL) {
      curLanePtr->setRightBoundary(boundaries[boundaryIndex[lanes[i]->getRightBoundaryId()]]);
    }
    curLanePtr->setSectionId(instancePtr->id);
    // curLanePtr->setSection( shared_from_this() );
    instancePtr->lanes.push_back(curLanePtr);
  }
}

txSection::~txSection() {}

sectionpkid txSection::getId() const { return instancePtr->getId(); }

roadpkid txSection::getRoadId() const { return instancePtr->getRoadId(); }

double txSection::getLength() const { return instancePtr->getLength(); }

double txSection::getMeanSlope() const { return instancePtr->meanSlope; }

double txSection::getMeanCurvature() const { return instancePtr->meanCurvature; }

txLanePtr &txSection::getLane(int8_t laneid) const {
  for (auto &it : instancePtr->getLanes()) {
    if (it->getId() == laneid) {
      return it;
    }
  }
}

txLanes &txSection::getLanes() const { return instancePtr->getLanes(); }

std::vector<hadmap::txLaneId> txSection::getRightLanes() const {
  std::vector<hadmap::txLaneId> _rightLanes;
  for (auto &it : instancePtr->getLanes()) {
    if (it->getId() < 0) {
      _rightLanes.push_back(it->getTxLaneId());
    }
  }
  std::sort(_rightLanes.begin(), _rightLanes.end(), [](const hadmap::txLaneId &a, const hadmap::txLaneId &b) {
    return std::abs(a.laneId) < std::abs(b.laneId);
  });
  return _rightLanes;
}

std::vector<hadmap::txLaneId> txSection::getLeftLanes() const {
  std::vector<hadmap::txLaneId> _leftLanes;
  for (auto &it : instancePtr->getLanes()) {
    if (it->getId() > 0) {
      _leftLanes.push_back(it->getTxLaneId());
    }
  }
  std::sort(_leftLanes.begin(), _leftLanes.end(), [](const hadmap::txLaneId &a, const hadmap::txLaneId &b) {
    return std::abs(a.laneId) < std::abs(b.laneId);
  });
  return _leftLanes;
}

double txSection::getStartPercentOfRoad() const { return instancePtr->startPercent; }

double txSection::getEndPercentOfRoad() const { return instancePtr->endPercent; }

txLaneBoundaries txSection::getBoundaries() const { return instancePtr->getBoundaries(); }

txLaneBoundaries txSection::getBoundariesByRange(const double &start, const double &end) const {
  if (start > 0.0 && end > 0.0 && start > end) return txLaneBoundaries();

  txLaneBoundaries boundaries = getBoundaries();
  txLaneBoundaries cut_boundaries;
  for (auto &bound_ptr : boundaries) {
    txLaneBoundaryPtr cur_bound_ptr(new txLaneBoundary());
    cur_bound_ptr->setId(bound_ptr->getId()).setLaneMark(bound_ptr->getLaneMark());

    const txLineCurve *curve_ptr = dynamic_cast<const txLineCurve *>(bound_ptr->getGeometry());
    PointVec points;
    if (curve_ptr->cut(start, end, points) && points.size() > 1) {
      cur_bound_ptr->setGeometry(points, curve_ptr->getCoordType());
      cut_boundaries.push_back(cur_bound_ptr);
    } else {
      return txLaneBoundaries();
    }
  }
  return cut_boundaries;
}

double txSection::getDistanceToJunction() const { return instancePtr->getDistanceToJunction(); }

txLanePtr txSection::get(lanepkid laneId) const { return instancePtr->get(laneId); }

txLanePtr txSection::get(const std::string &name) const { return instancePtr->get(name); }

txRoadPtr txSection::getRoad() const { return instancePtr->roadPtr.lock(); }

txSection &txSection::setRoadId(roadpkid id) {
  instancePtr->setRoadId(id);
  return *this;
}

txSection &txSection::setId(sectionpkid id) {
  instancePtr->setId(id);
  return *this;
}

txSection &txSection::setLanes(const txLanes &lanes) {
  instancePtr->lanes = lanes;
  for (size_t i = 0; i < instancePtr->lanes.size(); ++i) {
    // set section pointer to lane
    instancePtr->lanes[i]->setSection(shared_from_this());

    instancePtr->lanes[i]->setSectionId(instancePtr->id);
  }
  return *this;
}

txSection &txSection::add(const txLanePtr &lanePtr) {
  for (size_t i = 0; i < instancePtr->lanes.size(); ++i) {
    if (instancePtr->lanes[i]->getId() == lanePtr->getId() && instancePtr->lanes[i]->getName() == lanePtr->getName()) {
      return *this;
    }
  }
  lanePtr->setSectionId(instancePtr->id);
  lanePtr->setSection(shared_from_this());
  instancePtr->lanes.push_back(lanePtr);
  return *this;
}

txSection &txSection::removeLane(lanepkid laneId) {
  for (auto itr = instancePtr->lanes.begin(); itr != instancePtr->lanes.end(); ++itr) {
    if ((*itr)->getId() == laneId) {
      instancePtr->lanes.erase(itr);
      return *this;
    }
  }
  return *this;
}

txSection &txSection::setDistanceToJunction(double d) {
  instancePtr->setDistanceToJunction(d);
  return *this;
}

txSection &txSection::setMeanSlope(double d) {
  instancePtr->meanSlope = d;
  return *this;
  // TODO(undefined): 在此处插入 return 语句
}

txSection &txSection::setMeanCurvature(double d) {
  instancePtr->meanCurvature = d;
  return *this;
  // TODO(undefined): 在此处插入 return 语句
}

txSection &txSection::setRoad(const txRoadPtr roadPtr) {
  instancePtr->roadPtr = roadPtr;
  return *this;
}

bool txSection::reverse() {
  if (instancePtr->lanes.size() == 0) return false;
  for (size_t i = 0, j = instancePtr->lanes.size() - 1; i <= j; ++i, --j) {
    if (i == j) {
      instancePtr->lanes[i]->reverse();
      break;
    } else {
      instancePtr->lanes[i]->reverse();
      instancePtr->lanes[j]->reverse();
      std::swap(instancePtr->lanes[i], instancePtr->lanes[j]);
    }
  }
  std::set<uint64_t> boundaryIds;
  for (size_t i = 0; i < instancePtr->lanes.size(); ++i) {
    if (boundaryIds.find(instancePtr->lanes[i]->getLeftBoundaryId()) == boundaryIds.end()) {
      boundaryIds.insert(instancePtr->lanes[i]->getLeftBoundaryId());
      instancePtr->lanes[i]->getLeftBoundary()->reverse();
    }
    if (boundaryIds.find(instancePtr->lanes[i]->getRightBoundaryId()) == boundaryIds.end()) {
      boundaryIds.insert(instancePtr->lanes[i]->getRightBoundaryId());
      instancePtr->lanes[i]->getRightBoundary()->reverse();
    }
  }
  return true;
}

bool txSection::exchangeBoundaryIndex(bool curLane, bool oppositeLane) {
  if (instancePtr->lanes.size() == 0) return false;
  if (curLane) {
    for (size_t i = 0; i < instancePtr->lanes.size(); ++i) {
      if (instancePtr->lanes[i]->getId() < 0) instancePtr->lanes[i]->exchangeBoundaryIndex();
    }
  }
  if (oppositeLane) {
    for (size_t i = 0; i < instancePtr->lanes.size(); ++i) {
      if (instancePtr->lanes[i]->getId() > 0) instancePtr->lanes[i]->exchangeBoundaryIndex();
    }
  }
  return true;
}

txSection &txSection::bindSectionPtr() {
  for (size_t i = 0; i < instancePtr->lanes.size(); ++i) {
    // set section pointer to lane
    instancePtr->lanes[i]->setSection(shared_from_this());
  }
  return *this;
}
}  // namespace hadmap
