// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_road.h"
#include "structs/hadmap_curve.h"

#include "common/map_util.h"
#include "common/mapdata_trans.h"

namespace hadmap {
struct txRoad::RoadData {
 public:
  // real road data
  tx_road_t data;

  // road geometry
  txLineCurve* geom;

  // sections
  txSections sections;

 public:
  RoadData() : geom(NULL) {}

  ~RoadData() {
    if (geom != NULL) delete geom;
  }
};

txRoad::txRoad() : instancePtr(new RoadData) {}

txRoad::txRoad(roadpkid id, const std::string& name) : instancePtr(new RoadData) {
  instancePtr->data.pkid = id;
  memcpy(instancePtr->data.name, name.c_str(), name.length() + 1);
}

txRoad::txRoad(const txRoad& road) : instancePtr(new RoadData) {
  instancePtr->data = road.instancePtr->data;
  instancePtr->geom = new txLineCurve;
  instancePtr->geom->clone(road.getGeometry());

  // section init
  txSections& sections = road.instancePtr->sections;
  for (size_t i = 0; i < sections.size(); ++i) {
    instancePtr->sections.push_back(txSectionPtr(new txSection(*sections[i])));
    instancePtr->sections.back()->setRoadId(instancePtr->data.pkid);
    instancePtr->sections.back()->bindSectionPtr();
    // instancePtr->sections.back()->setRoad(
    //  std::dynamic_pointer_cast< txRoad >( shared_from_this() ) );
  }
}

txRoad::~txRoad() {}

// get info

bool txRoad::isJunction() const { return false; }

std::string txRoad::getUniqueId() const {
  char index[128];
  sprintf(index, "road_%s", std::to_string(instancePtr->data.pkid).c_str());
  return std::string(index);
}
tx_road_t& txRoad::getTxData() const { return instancePtr->data; }
LINK_TYPE txRoad::getLinkType() const { return LINK_TYPE_ROAD; }

roadpkid txRoad::getId() const { return instancePtr->data.pkid; }

taskpkid txRoad::getTaskId() const { return instancePtr->data.task_pkid; }

std::string txRoad::getName() const { return instancePtr->data.name; }

double txRoad::getLength() const {
  if (instancePtr->geom != NULL) {
    return instancePtr->geom->getLength();
  } else {
    double len = 0.0;
    for (auto itr = instancePtr->sections.begin(); itr != instancePtr->sections.end(); ++itr)
      len += (*itr)->getLength();
    return len;
  }
}

const txCurve* txRoad::getGeometry() const { return instancePtr->geom; }

double txRoad::getGround() const { return instancePtr->data.ground; }

uint32_t txRoad::getSpeedLimit() const { return instancePtr->data.speed_limit; }

ROAD_TYPE txRoad::getRoadType() const { return (ROAD_TYPE)instancePtr->data.road_type; }

uint8_t txRoad::getDirection() const { return instancePtr->data.direction; }

uint8_t txRoad::getToll() const { return instancePtr->data.toll; }

uint8_t txRoad::getFuncClass() const { return instancePtr->data.func_class; }

bool txRoad::isUrban() const { return instancePtr->data.urban; }

bool txRoad::isPaved() const { return instancePtr->data.paved; }

uint8_t txRoad::getTransType() const { return instancePtr->data.transition_type; }

bool txRoad::hasLaneMarking() const { return instancePtr->data.lane_mark; }

ROAD_MATERIAL txRoad::getMaterial() const { return (ROAD_MATERIAL)instancePtr->data.material; }

txSections& txRoad::getSections() const { return instancePtr->sections; }

txLanePtr txRoad::getLane(sectionpkid sectionId, lanepkid laneId) const {
  txLanePtr lanePtr;
  for (size_t i = 0; i < instancePtr->sections.size(); ++i) {
    if (instancePtr->sections[i]->getId() == sectionId) {
      lanePtr = instancePtr->sections[i]->get(laneId);
      break;
    }
  }
  return lanePtr;
}

txCurvatureVec txRoad::getCurvature() const { return instancePtr->data.curvaturesss; }

txSlopeVec txRoad::getSlope() const { return instancePtr->data.slopevec; }

txRoadLink txRoad::getRoadPreLink() const { return instancePtr->data.preLink; }

txRoadLink txRoad::getRoadToLink() const { return instancePtr->data.endLink; }

txControlPoint txRoad::getControlPoint() const { return instancePtr->data.controlPoint; }

txOpenCrgVec txRoad::getCrgVec() const { return instancePtr->data.opencrg; }
txControlPoint txRoad::getElePoint() const { return instancePtr->data.eleControlPoint; }
bool txRoad::isBidirectional() const {
  for (const auto& section_ptr : instancePtr->sections) {
    const txLanes& lanes = section_ptr->getLanes();
    if (!lanes.empty() && lanes.front()->getId() > 0) {
      return true;
    }
  }
  return false;
}

// set info

txRoad& txRoad::setId(roadpkid id) {
  instancePtr->data.pkid = id;
  for (size_t i = 0; i < instancePtr->sections.size(); ++i) {
    instancePtr->sections[i]->setRoadId(id);
  }
  return *this;
}

txRoad& txRoad::setName(const std::string& name) {
  strcpy(instancePtr->data.name, name.c_str());
  return *this;
}

txRoad& txRoad::setGeometry(const txCurve* curvePtr) {
  if (instancePtr->geom == NULL) instancePtr->geom = new txLineCurve;
  instancePtr->geom->clone(curvePtr);
  // mapdata_trans::LineCurveToPoints3d( instancePtr->geom,
  //  instancePtr->data.geom );
  return *this;
}

txRoad& txRoad::setGeometry(const txCurve& curveIns) {
  if (instancePtr->geom == NULL) instancePtr->geom = new txLineCurve;
  instancePtr->geom->clone(curveIns);
  // mapdata_trans::LineCurveToPoints3d( instancePtr->geom,
  //  instancePtr->data.geom );
  return *this;
}

txRoad& txRoad::setGeometry(const PointVec& points, CoordType ct) {
  if (instancePtr->geom == NULL) instancePtr->geom = new txLineCurve(ct);
  if (ct != instancePtr->geom->getCoordType()) instancePtr->geom->setCoordType(ct);
  instancePtr->geom->setCoords(points);

  instancePtr->data.geom.resize(points.size());
  for (size_t i = 0; i < points.size(); ++i) {
    instancePtr->data.geom[i].x = points[i].x;
    instancePtr->data.geom[i].y = points[i].y;
    instancePtr->data.geom[i].z = points[i].z;
  }
  // mapdata_trans::LineCurveToPoints3d( instancePtr->geom,
  //  instancePtr->data.geom );
  return *this;
}

txRoad& txRoad::transfer(const txPoint& oldEnuC, const txPoint& newEnuC) {
  if (instancePtr->geom == NULL || instancePtr->geom->empty()) {
    return *this;
  } else {
    instancePtr->geom->transfer(oldEnuC, newEnuC);
  }
  return *this;
}

txRoad& txRoad::transfer2ENU(const txPoint& enuCenter) {
  if (instancePtr->geom == NULL || instancePtr->geom->empty()) {
    return *this;
  } else {
    instancePtr->geom->transfer2ENU(enuCenter);
  }
  return *this;
}

txRoad& txRoad::setSpeedLimit(uint32_t speed) {
  instancePtr->data.speed_limit = speed;
  return *this;
}

txRoad& txRoad::setGround(double g) {
  instancePtr->data.ground = g;
  return *this;
}

txRoad& txRoad::setRoadType(ROAD_TYPE type) {
  instancePtr->data.road_type = (uint32_t)type;
  return *this;
}

txRoad& txRoad::setDirection(uint8_t direction) {
  instancePtr->data.direction = direction;
  return *this;
}

txRoad& txRoad::setToll(uint8_t toll) {
  instancePtr->data.toll = toll;
  return *this;
}

txRoad& txRoad::setFuncClass(uint8_t funcClass) {
  instancePtr->data.func_class = funcClass;
  return *this;
}

txRoad& txRoad::setUrban(bool flag) {
  instancePtr->data.urban = flag;
  return *this;
}

txRoad& txRoad::setPaved(bool flag) {
  instancePtr->data.paved = flag;
  return *this;
}

txRoad& txRoad::setTransType(uint8_t transType) {
  instancePtr->data.transition_type = transType;
  return *this;
}

txRoad& txRoad::setLaneMarking(bool flag) {
  instancePtr->data.lane_mark = flag;
  return *this;
}

txRoad& txRoad::setMaterial(ROAD_MATERIAL material) {
  instancePtr->data.material = (uint32_t)material;
  return *this;
}

txRoad& txRoad::addSection(const txSectionPtr& sectionPtr) {
  for (size_t i = 0; i < instancePtr->sections.size(); ++i) {
    if (instancePtr->sections[i]->getId() == sectionPtr->getId()) return *this;
  }
  instancePtr->sections.push_back(sectionPtr);
  instancePtr->sections.back()->setRoadId(instancePtr->data.pkid);
  instancePtr->sections.back()->setRoad(std::dynamic_pointer_cast<txRoad>(shared_from_this()));
  return *this;
}

txRoad& txRoad::setSections(const txSections& sections) {
  if (sections.size() == 0) {
    return *this;
  } else {
    instancePtr->sections.clear();
    for (size_t i = 0; i < sections.size(); ++i) {
      txSectionPtr curSecPtr(new txSection(*sections[i]));
      curSecPtr->bindSectionPtr();
      addSection(curSecPtr);
    }
    return *this;
  }
}

txRoad& txRoad::setTaskId(taskpkid taskId) {
  instancePtr->data.task_pkid = taskId;
  return *this;
}

txRoad& txRoad::setCurvature(txCurvatureVec vec) {
  instancePtr->data.curvaturesss = vec;
  return *this;
}
txRoad& txRoad::setSlope(txSlopeVec vec) {
  instancePtr->data.slopevec = vec;
  return *this;
}
txRoad& txRoad::setElePoint(txControlPoint ele) {
  instancePtr->data.eleControlPoint = ele;
  return *this;
}
txRoad& txRoad::setPreLink(txRoadLink& link) {
  instancePtr->data.preLink = link;
  return *this;
}
txRoad& txRoad::setToLink(txRoadLink& link) {
  instancePtr->data.endLink = link;
  return *this;
}
txRoad& txRoad::setControlPoint(txControlPoint& point) {
  instancePtr->data.controlPoint = point;
  return *this;
}
txRoad& txRoad::setCrgs(txOpenCrgVec crgs) {
  instancePtr->data.opencrg = crgs;
  return *this;
}
bool txRoad::reverse() {
  /*
  if ( instancePtr->data.geom.size() > 1 )
  {
          size_t geomSize = instancePtr->data.geom.size();
          for ( size_t i = 0, j = geomSize - 1; i < j; ++ i, -- j )
                  std::swap( instancePtr->data.geom[i], \
                          instancePtr->data.geom[j] );
          mapdata_trans::Points3dToLineCurve( instancePtr->data.geom, \
                  instancePtr->geom );
  }
  */
  instancePtr->geom->reverse();
  if (instancePtr->sections.size() == 0) return true;
  for (size_t i = 0, j = instancePtr->sections.size() - 1; i <= j; ++i, --j) {
    if (i == j) {
      instancePtr->sections[i]->reverse();
      break;
    } else {
      sectionpkid frontId = instancePtr->sections[i]->getId();
      sectionpkid backId = instancePtr->sections[j]->getId();
      instancePtr->sections[i]->reverse();
      instancePtr->sections[j]->reverse();
      instancePtr->sections[i]->setId(backId);
      instancePtr->sections[j]->setId(frontId);
      std::swap(instancePtr->sections[i], instancePtr->sections[j]);
    }
  }
  return true;
}

bool txRoad::exchangeBoundaryIndex(bool curLane, bool oppositeLane) {
  for (size_t i = 0; i < instancePtr->sections.size(); ++i)
    instancePtr->sections[i]->exchangeBoundaryIndex(curLane, oppositeLane);
  return true;
}

txRoad& txRoad::bindRoadPtr() {
  for (size_t i = 0; i < instancePtr->sections.size(); ++i) {
    instancePtr->sections[i]->setRoad(std::dynamic_pointer_cast<txRoad>(shared_from_this()));
  }
  return *this;
}

bool txRoad::buildInnerTopo() {
  if (instancePtr->sections.size() == 0) {
    return false;
  } else {
    if (instancePtr->sections.size() == 1) return true;
    for (size_t i = 0; i < instancePtr->sections.size() - 1; ++i) {
      txLanes& frontLanes = instancePtr->sections[i]->getLanes();
      txLanes& backLanes = instancePtr->sections[i + 1]->getLanes();
      for (size_t j = 0; j < frontLanes.size(); ++j) frontLanes[j]->clearNext();
      for (size_t j = 0; j < backLanes.size(); ++j) backLanes[j]->clearPrev();
      for (size_t j = 0; j < frontLanes.size(); ++j) {
        if (frontLanes[j]->getGeometry() && frontLanes[j]->getGeometry()->empty() == false) {
          txPoint end = frontLanes[j]->getGeometry()->getEnd();
          for (size_t k = 0; k < backLanes.size(); ++k) {
            if (backLanes[k]->getGeometry() && backLanes[k]->getGeometry()->empty() == false) {
              txPoint begin = backLanes[k]->getGeometry()->getStart();
              double dis =
                  map_util::distanceBetweenPoints(Point3d(end.x, end.y, end.z), Point3d(begin.x, begin.y, begin.z),
                                                  frontLanes[j]->getGeometry()->getCoordType() == COORD_WGS84);
              if (dis < 0.15) {
                frontLanes[j]->addNext(backLanes[k]->getUniqueId());
                backLanes[k]->addPrev(frontLanes[j]->getUniqueId());
              }
            }
          }
        }
      }
    }
    return true;
  }
}

bool txRoad::buildOuterTopo(const txRoadPtr& nextRoadPtr) {
  if (nextRoadPtr == NULL || nextRoadPtr->getSections().size() == 0) {
    return false;
  } else {
    txLanes& frontLanes = instancePtr->sections.back()->getLanes();
    txLanes& backLanes = nextRoadPtr->instancePtr->sections.front()->getLanes();

    for (size_t j = 0; j < frontLanes.size(); ++j) {
      if (frontLanes[j]->getGeometry() && frontLanes[j]->getGeometry()->empty() == false) {
        txPoint end = frontLanes[j]->getGeometry()->getEnd();
        for (size_t k = 0; k < backLanes.size(); ++k) {
          if (backLanes[k]->getGeometry() && backLanes[k]->getGeometry()->empty() == false) {
            txPoint begin = backLanes[k]->getGeometry()->getStart();
            double dis =
                map_util::distanceBetweenPoints(Point3d(end.x, end.y, end.z), Point3d(begin.x, begin.y, begin.z),
                                                frontLanes[j]->getGeometry()->getCoordType() == COORD_WGS84);
            if (dis < 0.15) {
              frontLanes[j]->addNext(backLanes[k]->getUniqueId());
              backLanes[k]->addPrev(frontLanes[j]->getUniqueId());
            }
          }
        }
      }
    }
    return true;
  }
}
}  // namespace hadmap
