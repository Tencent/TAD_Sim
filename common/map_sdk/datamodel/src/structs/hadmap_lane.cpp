// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_lane.h"
#include "structs/hadmap_curve.h"

#include "common/coord_trans.h"
#include "common/map_util.h"
#include "common/mapdata_trans.h"

#include <exception>

namespace hadmap {
struct txLane::LaneData {
 public:
  // real lane data
  tx_lane_t data;

  // line curve
  txLineCurve* geom;

  // left boundary ptr
  std::shared_ptr<txLaneBoundary> leftBoundaryPtr;

  // right boundary ptr
  std::shared_ptr<txLaneBoundary> rightBoundaryPtr;

  // distance to end
  double distanceToEnd;

  // average width of current lane
  double aveWidth;

  // is valid for driving
  bool valid4Driving;

  // section pointer
  std::weak_ptr<txSection> sectionPtr;

  // line curve
  txLineCurve* geom_dense;

 public:
  LaneData() : geom(nullptr), geom_dense(nullptr), distanceToEnd(-1.0), aveWidth(-1.0), valid4Driving(true) {}

  ~LaneData() {
    if (geom != nullptr) delete geom;
    if (geom_dense != nullptr) delete geom_dense;
  }

 public:
  bool xy2sl(const double& x, const double& y, double& s, double& l, double& yaw);

  bool sl2xy(const double& s, const double& l, double& x, double& y, double& yaw);

  double getYaw(const double& s);
};

bool txLane::LaneData::xy2sl(const double& x, const double& y, double& s, double& l, double& yaw) {
  if (geom == NULL || geom->empty()) return false;
  if (geom->xy2sl(x, y, s, l, yaw)) {
    if (data.lane_id > 0) {
      s = geom->getLength() - s;
      l *= -1;
      yaw = yaw - 180.0;
      if (yaw < 0) yaw += 360.0;
    }
    yaw = map_util::angle2radian(yaw);
    return true;
  } else {
    return false;
  }
}

bool txLane::LaneData::sl2xy(const double& s, const double& l, double& x, double& y, double& yaw) {
  if (geom == NULL || geom->empty()) return false;
  double ns = s;
  double nl = l;
  if (data.lane_id > 0) {
    ns = geom->getLength() - s;
    nl *= -1;
  }
  if (geom->sl2xy(ns, nl, x, y, yaw)) {
    if (data.lane_id > 0) {
      yaw = yaw - 180.0;
      if (yaw < 0) yaw += 360.0;
    }
    yaw = map_util::angle2radian(yaw);
    return true;
  } else {
    return false;
  }
}

double txLane::LaneData::getYaw(const double& s) {
  if (geom == NULL || geom->empty()) throw std::runtime_error("getYaw error, no geometry data");
  return geom->getYaw(s);
}

//
// hadmap_lane
//
txLane::txLane() : instancePtr(new LaneData) {}

txLane::txLane(const txLane& lane) : instancePtr(new LaneData) {
  instancePtr->data = lane.instancePtr->data;
  instancePtr->geom = new txLineCurve;
  instancePtr->geom->clone(lane.instancePtr->geom);
  instancePtr->distanceToEnd = lane.instancePtr->distanceToEnd;
  instancePtr->aveWidth = lane.instancePtr->aveWidth;
  instancePtr->valid4Driving = lane.instancePtr->valid4Driving;

  // leftBoundaryPtr & rightBoundaryPtr set outside
  // sectionPtr set outside
}

txLane::~txLane() {}
tx_lane_t& txLane::getTxLaneData() const { return instancePtr->data; }
// get info
LINK_TYPE txLane::getLinkType() const { return LINK_TYPE_LANE; }

const txCurve* txLane::getGeometry() const { return instancePtr->geom; }

const txCurve* txLane::getDenseGeometry() const { return instancePtr->geom_dense; }

lanepkid txLane::getId() const { return instancePtr->data.lane_id; }

txLaneBoundaryPtr txLane::getLeftBoundary() const { return instancePtr->leftBoundaryPtr; }

laneboundarypkid txLane::getLeftBoundaryId() const {
  if (!instancePtr->leftBoundaryPtr) {
    return instancePtr->data.left_boundary;
  } else {
    return instancePtr->leftBoundaryPtr->getId();
  }
}

txLaneBoundaryPtr txLane::getRightBoundary() const { return instancePtr->rightBoundaryPtr; }

laneboundarypkid txLane::getRightBoundaryId() const {
  if (!instancePtr->rightBoundaryPtr) {
    return instancePtr->data.right_boundary;
  } else {
    return instancePtr->rightBoundaryPtr->getId();
  }
}

LANE_TYPE txLane::getLaneType() const { return (LANE_TYPE)instancePtr->data.lane_type; }

LANE_ARROW txLane::getLaneArrow() const { return (LANE_ARROW)instancePtr->data.lane_arrow; }

uint32_t txLane::getSpeedLimit() const { return instancePtr->data.speed_limit; }

uint32_t txLane::getLaneSurface() const { return instancePtr->data.lane_surface; }

double txLane::getLaneFriction() const { return instancePtr->data.lane_friction; }

double txLane::getMaterialOffset() const { return instancePtr->data.lane_material_soffset; }

roadpkid txLane::getRoadId() const { return instancePtr->data.road_pkid; }

sectionpkid txLane::getSectionId() const { return instancePtr->data.section_id; }

txLaneId txLane::getTxLaneId() const {
  return txLaneId(instancePtr->data.road_pkid, instancePtr->data.section_id, instancePtr->data.lane_id);
}

double txLane::getDistanceToEnd() const { return instancePtr->distanceToEnd; }

double txLane::getLength() const {
  if (NULL == instancePtr->geom) {
    return 0.0;
  } else {
    return instancePtr->geom->getLength();
  }
}

double txLane::getLaneWidth() const { return instancePtr->aveWidth; }

double txLane::getLaneWidth(double s) const {
  auto pt = this->getGeometry()->getPoint(s);
  return pt.w;
}

std::string txLane::getName() const { return std::string(instancePtr->data.name); }

bool txLane::isEnable() const { return instancePtr->valid4Driving; }

txSectionPtr txLane::getSection() const { return instancePtr->sectionPtr.lock(); }

// set info
txLane& txLane::setName(const std::string& name) {
  memcpy(instancePtr->data.name, name.c_str(), name.length() + 1);
  return *this;
}

txLane& txLane::setGeometry(const txCurve* curvePtr) {
  if (NULL == instancePtr->geom) instancePtr->geom = new txLineCurve;
  instancePtr->geom->clone(curvePtr);
  // mapdata_trans::LineCurveToPoints3d( instancePtr->geom, \
    //  instancePtr->data.geom );
  return *this;
}

txLane& txLane::setGeometry(const txCurve& curveIns) {
  if (NULL == instancePtr->geom) instancePtr->geom = new txLineCurve;
  instancePtr->geom->clone(curveIns);
  // mapdata_trans::LineCurveToPoints3d( instancePtr->geom, \
    //  instancePtr->data.geom );
  return *this;
}

txLane& txLane::setGeometry(const PointVec& points, CoordType ct) {
  if (points.empty()) return *this;

  if (NULL == instancePtr->geom) instancePtr->geom = new txLineCurve(ct);
  if (ct != instancePtr->geom->getCoordType()) instancePtr->geom->setCoordType(ct);
  instancePtr->geom->setCoords(points);

  instancePtr->data.geom.resize(points.size());
  for (size_t i = 0; i < points.size(); ++i) {
    instancePtr->data.geom[i].x = points[i].x;
    instancePtr->data.geom[i].y = points[i].y;
    instancePtr->data.geom[i].z = points[i].z;
  }

  return *this;
}

txLane& txLane::setDenseGeometry(const txCurve& curveIns) {
  if (nullptr == instancePtr->geom_dense) instancePtr->geom_dense = new txLineCurve;
  instancePtr->geom_dense->clone(curveIns);
  // mapdata_trans::LineCurveToPoints3d( instancePtr->geom_dense, \
    //  instancePtr->data.geom_dense );
  return *this;
}

txLane& txLane::setDenseGeometry(const PointVec& points, CoordType ct) {
  if (points.empty()) return *this;

  if (NULL == instancePtr->geom_dense) instancePtr->geom_dense = new txLineCurve(ct);
  if (ct != instancePtr->geom_dense->getCoordType()) instancePtr->geom_dense->setCoordType(ct);
  instancePtr->geom_dense->setCoords(points);

  return *this;
}
txLane& txLane::transfer(const txPoint& oldEnuC, const txPoint& newEnuC) {
  if (NULL == instancePtr->geom || instancePtr->geom->empty()) {
    return *this;
  } else {
    instancePtr->geom->transfer(oldEnuC, newEnuC);
  }
  return *this;
}

txLane& txLane::transfer2ENU(const txPoint& enuCenter) {
  if (NULL == instancePtr->geom || instancePtr->geom->empty()) {
    return *this;
  } else {
    /*
    roadpkid rid = instancePtr->data.road_pkid;
    sectionpkid sid = instancePtr->data.section_id;
    lanepkid lid = instancePtr->data.lane_id;
    */
    instancePtr->geom->transfer2ENU(enuCenter);
  }
  return *this;
}

txLane& txLane::setLeftBoundary(const txLaneBoundaryPtr& boundaryPtr) {
  instancePtr->leftBoundaryPtr = boundaryPtr;
  if (NULL == boundaryPtr) {
    instancePtr->data.left_boundary = 0;
  } else {
    instancePtr->data.left_boundary = boundaryPtr->getId();
  }
  return *this;
}

txLane& txLane::setLeftBoundaryId(laneboundarypkid boundaryId) {
  instancePtr->data.left_boundary = boundaryId;
  if (instancePtr->leftBoundaryPtr && instancePtr->leftBoundaryPtr->getId() != boundaryId) {
    instancePtr->leftBoundaryPtr.reset();
  }
  return *this;
}

txLane& txLane::setRightBoundary(const txLaneBoundaryPtr& boundaryPtr) {
  instancePtr->rightBoundaryPtr = boundaryPtr;
  if (NULL == boundaryPtr) {
    instancePtr->data.right_boundary = 0;
  } else {
    instancePtr->data.right_boundary = boundaryPtr->getId();
  }
  return *this;
}

txLane& txLane::setRightBoundaryId(laneboundarypkid boundaryId) {
  instancePtr->data.right_boundary = boundaryId;
  if (instancePtr->rightBoundaryPtr && instancePtr->rightBoundaryPtr->getId() != boundaryId) {
    instancePtr->rightBoundaryPtr.reset();
  }
  return *this;
}

txLane& txLane::setLaneType(LANE_TYPE type) {
  instancePtr->data.lane_type = uint32_t(type);
  return *this;
}

txLane& txLane::setLaneArrow(LANE_ARROW arrow) {
  instancePtr->data.lane_arrow = uint32_t(arrow);
  return *this;
}

txLane& txLane::setSpeedLimit(uint32_t speed) {
  instancePtr->data.speed_limit = speed;
  return *this;
}

txLane& txLane::setLaneSurface(uint32_t surface) {
  instancePtr->data.lane_surface = surface;
  return *this;
}

txLane& txLane::setLaneFriction(double friction) {
  instancePtr->data.lane_friction = friction;
  return *this;
}

txLane& txLane::setMaterialOffset(double soffset) {
  instancePtr->data.lane_material_soffset = soffset;
  return *this;
}

txLane& txLane::setRoadId(roadpkid roadId) {
  instancePtr->data.road_pkid = roadId;
  return *this;
}

txLane& txLane::setId(lanepkid laneId) {
  instancePtr->data.lane_id = laneId;
  return *this;
}

txLane& txLane::setSectionId(sectionpkid sectionId) {
  instancePtr->data.section_id = sectionId;
  return *this;
}

txLane& txLane::setDistanceToEnd(double disToEnd) {
  instancePtr->distanceToEnd = disToEnd;
  return *this;
}

txLane& txLane::setLaneWidth(double width) {
  instancePtr->aveWidth = width;
  return *this;
}

txLane& txLane::setEnable(bool enable) {
  instancePtr->valid4Driving = enable;
  return *this;
}

txLane& txLane::setSection(const txSectionPtr& secPtr) {
  instancePtr->sectionPtr = secPtr;
  return *this;
}

bool txLane::reverse() {
  // reset lane id
  instancePtr->data.lane_id *= -1;

  // reverse geometry data
  /*
  if ( instancePtr->data.geom.size() > 0 )
  {
          size_t geomSize = instancePtr->data.geom.size();
          for ( size_t i = 0, j = geomSize - 1; i < j; ++ i, -- j )
                  std::swap( instancePtr->data.geom[i], \
                          instancePtr->data.geom[j] );
          mapdata_trans::Points3dToLineCurve( instancePtr->data.geom, \
                  instancePtr->geom );
  }
  */
  if (instancePtr->geom != NULL) instancePtr->geom->reverse();
  return true;
}

bool txLane::exchangeBoundaryIndex() {
  std::swap(instancePtr->data.left_boundary, instancePtr->data.right_boundary);
  std::swap(instancePtr->leftBoundaryPtr, instancePtr->rightBoundaryPtr);
  return true;
}

std::string txLane::getUniqueId() const {
  char index[128];
  sprintf(index, "lane_%s.%s.%s", std::to_string(instancePtr->data.road_pkid).c_str(),
          std::to_string(instancePtr->data.section_id).c_str(), std::to_string(instancePtr->data.lane_id).c_str());
  return std::string(index);
}

bool txLane::xy2sl(const double& x, const double& y, double& s, double& l) {
  double yaw;
  return instancePtr->xy2sl(x, y, s, l, yaw);
}

bool txLane::sl2xy(const double& s, const double& l, double& x, double& y) {
  double yaw;
  return instancePtr->sl2xy(s, l, x, y, yaw);
}

bool txLane::xy2sl(const double& x, const double& y, double& s, double& l, double& yaw) {
  return instancePtr->xy2sl(x, y, s, l, yaw);
}

bool txLane::sl2xy(const double& s, const double& l, double& x, double& y, double& yaw) {
  return instancePtr->sl2xy(s, l, x, y, yaw);
}

double txLane::getYaw(const double& s) { return instancePtr->getYaw(s); }
}  // namespace hadmap
