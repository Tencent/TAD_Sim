// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_curve.h"

#include "common/coord_trans.h"
#include "common/map_util.h"
#include "common/mapdata_trans.h"

#include <mutex>

namespace hadmap {
struct txLaneLink::LaneLinkData {
 public:
  // real lane link data
  tx_lanelink_t data;

  // origin geometry
  txLineCurve* geomPtr;

  txLaneBoundaries leftBds;
  txLaneBoundaries rightBds;
  double radius = -1.0;
  std::mutex radiusmtx;

 public:
  LaneLinkData() : geomPtr(NULL) {}

  ~LaneLinkData() {
    if (NULL != geomPtr) delete geomPtr;
  }
};

txLaneLink::txLaneLink() : instancePtr(new LaneLinkData) {}

txLaneLink::txLaneLink(const txLaneLink& link) : instancePtr(new LaneLinkData) {
  instancePtr->data = link.instancePtr->data;
  instancePtr->geomPtr = new txLineCurve;
  instancePtr->geomPtr->clone(link.instancePtr->geomPtr);
  instancePtr->leftBds = link.instancePtr->leftBds;
  instancePtr->rightBds = link.instancePtr->rightBds;
}

txLaneLink::~txLaneLink() {}

tx_lanelink_t txLaneLink::getTxData() const { return instancePtr->data; }

LINK_TYPE txLaneLink::getLinkType() const { return LINK_TYPE_LANELINK; }

lanelinkpkid txLaneLink::getId() const { return instancePtr->data.pkid; }

roadpkid txLaneLink::fromRoadId() const { return instancePtr->data.from_road_pkid; }

sectionpkid txLaneLink::fromSectionId() const { return instancePtr->data.from_section_id; }

lanepkid txLaneLink::fromLaneId() const { return instancePtr->data.from_lane_id; }

txLaneId txLaneLink::fromTxLaneId() const {
  return txLaneId(instancePtr->data.from_road_pkid, instancePtr->data.from_section_id, instancePtr->data.from_lane_id);
}

roadpkid txLaneLink::toRoadId() const { return instancePtr->data.to_road_pkid; }

sectionpkid txLaneLink::toSectionId() const { return instancePtr->data.to_section_id; }

lanepkid txLaneLink::toLaneId() const { return instancePtr->data.to_lane_id; }

txLaneId txLaneLink::toTxLaneId() const {
  return txLaneId(instancePtr->data.to_road_pkid, instancePtr->data.to_section_id, instancePtr->data.to_lane_id);
}

const roadpkid txLaneLink::getOdrRoadId() const { return instancePtr->data.odr_road_id; }

std::string txLaneLink::getUniqueId() const {
  char index[128];
  sprintf(
      index, "link_%s.%s.%s_%s.%s.%s", std::to_string(instancePtr->data.from_road_pkid).c_str(),
      std::to_string(instancePtr->data.from_section_id).c_str(), std::to_string(instancePtr->data.from_lane_id).c_str(),
      std::to_string(instancePtr->data.to_road_pkid).c_str(), std::to_string(instancePtr->data.to_section_id).c_str(),
      std::to_string(instancePtr->data.to_lane_id).c_str());
  return std::string(index);
}

junctionpkid txLaneLink::getJunctionId() const { return instancePtr->data.junction_id; }

txControlPoint txLaneLink::getControlPoint() const { return instancePtr->data.controlPoint; }

txControlPoint txLaneLink::getEleControlPoint() const { return instancePtr->data.eleControlPoint; }

txCurvatureVec txLaneLink::getCurvature() const { return instancePtr->data.curvature; }

txSlopeVec txLaneLink::getSlope() const { return instancePtr->data.slope; }

double txLaneLink::getMeanSlope() const {
  double length = 0;
  double sumSlope = 0;
  for (size_t i = 0; i < this->getSlope().size(); i++) {
    sumSlope += (this->getSlope().at(i).m_slope * this->getSlope().at(i).m_length);
    length += this->getSlope().at(i).m_length;
  }
  if (length == 0 || sumSlope / length < 1e-5) return 0;
  return sumSlope / length;
}

txContactType txLaneLink::getPreContact() { return instancePtr->data.preContact; }
txContactType txLaneLink::getSuccContact() { return instancePtr->data.succContact; }

double txLaneLink::getMeanCurvature() const {
  double length = 0;
  double sumCurvature = 0;
  for (size_t i = 0; i < this->getCurvature().size(); i++) {
    sumCurvature += (this->getCurvature().at(i).m_curvature * this->getCurvature().at(i).m_length);
    length += this->getCurvature().at(i).m_length;
  }
  if (length == 0 || sumCurvature / length < 1e-6) return 0;
  return sumCurvature / length;
}

const txLaneBoundaries& txLaneLink::getLeftBoundaries() const { return instancePtr->leftBds; }

const txLaneBoundaries& txLaneLink::getRightBoundaries() const { return instancePtr->rightBds; }

double txLaneLink::getDeltaDierction() const {
  if (instancePtr && instancePtr->geomPtr && instancePtr->geomPtr->size() > 2) {
    double yaw0 = instancePtr->geomPtr->yaw(0);
    double yaw1 = instancePtr->geomPtr->yaw(instancePtr->geomPtr->size() - 1);
    yaw1 -= yaw0;
    if (yaw1 > 180) {
      yaw1 -= 360;
    } else if (yaw1 < -180) {
      yaw1 += 360;
    }
    return yaw1;
  }
  return 0.0;
}

double txLaneLink::getRadius() {
  if (!instancePtr || !(instancePtr->geomPtr) || instancePtr->geomPtr->size() <= 2) {
    return INFINITY;
  }

  std::lock_guard<std::mutex> _(instancePtr->radiusmtx);
  if (instancePtr->radius >= 0.0) {
    return instancePtr->radius;
  }

  auto geom = instancePtr->geomPtr;
  hadmap::txPoint p1 = geom->getStart();
  hadmap::txPoint p2 = geom->getPoint(size_t(1));
  hadmap::txPoint p3 = geom->getEnd();

  hadmap::Point2d A(p1.x, p1.y), B(p2.x, p2.y), C(p3.x, p3.y);
  if (COORD_WGS84 == geom->getCoordType()) {
    double z = 0.0;
    coord_trans_api::lonlat2enu(B.x, B.y, z, A.x, A.y, z);
    coord_trans_api::lonlat2enu(C.x, C.y, z, A.x, A.y, z);
    A.x = 0.0;
    A.y = 0.0;
  }

  instancePtr->radius = map_util::calcCircumcircleRadius(A, B, C, false);
  // char log[1024] = {0};
  // snprintf(log, sizeof(log), "%s => (%.7f,%.7f)->(%.7f,%.7f)->(%.7f,%.7f), dis:%.3f, radius:%.3f",
  //     getUniqueId().c_str(),
  //     p1.x, p1.y,
  //     p2.x, p2.y,
  //     p3.x, p3.y,
  //     map_util::directlyDis(C, A), instancePtr->radius);
  // txlog::printf("%s\n", log);
  return instancePtr->radius;
}

double txLaneLink::getSpeedLimit() {
  double radius = getRadius();
  if (radius < 35) {
    return 35;
  } else if (radius < 68) {
    return 45;
  } else if (radius < 112) {
    return 57;
  } else if (radius < 192) {
    return 65;
  } else if (radius < 210) {
    return 67;
  } else if (radius < 239) {
    return 69;
  } else if (radius < 361) {
    return 83;
  } else if (radius < 464) {
    return 95;
  } else if (radius < 643) {
    return 120;
  } else if (radius < 1086) {
    return 130;
  }
  return INFINITY;
}

const txCurve* txLaneLink::getGeometry() const { return instancePtr->geomPtr; }

bool txLaneLink::operator==(const txLaneLink& link) const {
  if (instancePtr->data.from_road_pkid == link.instancePtr->data.from_road_pkid &&
      instancePtr->data.from_section_id == link.instancePtr->data.from_section_id &&
      instancePtr->data.from_lane_id == link.instancePtr->data.from_lane_id &&
      instancePtr->data.to_road_pkid == link.instancePtr->data.to_road_pkid &&
      instancePtr->data.to_section_id == link.instancePtr->data.to_section_id &&
      instancePtr->data.to_lane_id == link.instancePtr->data.to_lane_id) {
    return true;
  } else {
    return false;
  }
}

std::string txLaneLink::linkIndex() const {
  char index[128];
  sprintf(
      index, "%s.%s.%s_%s.%s.%s", std::to_string(instancePtr->data.from_road_pkid).c_str(),
      std::to_string(instancePtr->data.from_section_id).c_str(), std::to_string(instancePtr->data.from_lane_id).c_str(),
      std::to_string(instancePtr->data.to_road_pkid).c_str(), std::to_string(instancePtr->data.to_section_id).c_str(),
      std::to_string(instancePtr->data.to_lane_id).c_str());
  return std::string(index);
}
// set info

txLaneLink& txLaneLink::setId(lanelinkpkid id) {
  instancePtr->data.pkid = id;
  return *this;
}

txLaneLink& txLaneLink::setFromRoadId(roadpkid roadId) {
  instancePtr->data.from_road_pkid = roadId;
  return *this;
}

txLaneLink& txLaneLink::setFromSectionId(sectionpkid sectionId) {
  instancePtr->data.from_section_id = sectionId;
  return *this;
}

txLaneLink& txLaneLink::setFromLaneId(lanepkid laneId) {
  instancePtr->data.from_lane_id = laneId;
  return *this;
}

txLaneLink& txLaneLink::setToRoadId(roadpkid roadId) {
  instancePtr->data.to_road_pkid = roadId;
  return *this;
}

txLaneLink& txLaneLink::setToSectionId(sectionpkid sectionId) {
  instancePtr->data.to_section_id = sectionId;
  return *this;
}

txLaneLink& txLaneLink::setToLaneId(lanepkid laneId) {
  instancePtr->data.to_lane_id = laneId;
  return *this;
}

txLaneLink& txLaneLink::setOdrRoadId(roadpkid roadId) {
  instancePtr->data.odr_road_id = roadId;
  return *this;
}

txLaneLink& txLaneLink::setControlPoint(txControlPoint control) {
  instancePtr->data.controlPoint = control;
  return *this;
}

txLaneLink& txLaneLink::setEleControlPoint(txControlPoint control) {
  instancePtr->data.eleControlPoint = control;
  return *this;
}

txLaneLink& txLaneLink::setGeometry(const txCurve* curvePtr) {
  if (instancePtr->geomPtr == NULL) instancePtr->geomPtr = new txLineCurve;
  instancePtr->geomPtr->clone(curvePtr);
  mapdata_trans::LineCurveToPoints3d(instancePtr->geomPtr, instancePtr->data.geom);
  return *this;
}

txLaneLink& txLaneLink::setGeometry(const txCurve& curveIns) {
  if (instancePtr->geomPtr == NULL) instancePtr->geomPtr = new txLineCurve;
  instancePtr->geomPtr->clone(curveIns);
  mapdata_trans::LineCurveToPoints3d(instancePtr->geomPtr, instancePtr->data.geom);
  return *this;
}

txLaneLink& txLaneLink::setGeometry(const PointVec& points, CoordType ct) {
  if (points.empty()) return *this;

  if (instancePtr->geomPtr == NULL) instancePtr->geomPtr = new txLineCurve(ct);
  instancePtr->geomPtr->setCoordType(ct);
  instancePtr->geomPtr->setCoords(points);
  mapdata_trans::LineCurveToPoints3d(instancePtr->geomPtr, instancePtr->data.geom);
  return *this;
}
txLaneLink& txLaneLink::setCurvature(const txCurvatureVec& curvatureVec) {
  instancePtr->data.curvature = curvatureVec;
  return *this;
  // TODO(undefined): 在此处插入 return 语句
}
txLaneLink& txLaneLink::setSlope(const txSlopeVec& slope) {
  instancePtr->data.slope = slope;
  return *this;
  // TODO(undefined): 在此处插入 return 语句
}
txLaneLink& txLaneLink::setPreContact(const txContactType& preContact) {
  instancePtr->data.preContact = preContact;
  return *this;
  // TODO(undefined): 在此处插入 return 语句
}
txLaneLink& txLaneLink::setSuccContact(const txContactType& succContact) {
  instancePtr->data.succContact = succContact;
  return *this;
  // TODO(undefined): 在此处插入 return 语句
}
txLaneLink& txLaneLink::setJunctionId(junctionpkid jid) {
  // TODO(undefined): insert return statement here
  instancePtr->data.junction_id = jid;
  return *this;
}
txLaneLink& txLaneLink::setLeftBoundaries(const txLaneBoundaries& bds) {
  // TODO(undefined): insert return statement here
  instancePtr->leftBds = bds;
  return *this;
}
txLaneLink& txLaneLink::setRightBoundaries(const txLaneBoundaries& bds) {
  // TODO(undefined): insert return statement here
  instancePtr->rightBds = bds;
  return *this;
}
}  // namespace hadmap
