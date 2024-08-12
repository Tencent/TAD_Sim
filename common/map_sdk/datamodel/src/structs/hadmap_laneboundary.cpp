// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_laneboundary.h"
#include "common/mapdata_trans.h"
#include "structs/hadmap_curve.h"
#include "structs/map_structs.h"

namespace hadmap {
struct txLaneBoundary::BoundaryData {
 public:
  // boundary data
  tx_laneboundary_t data;
  // geometry
  txLineCurve* geom;

 public:
  BoundaryData() : geom(NULL) {}

  ~BoundaryData() {
    if (geom != NULL) delete geom;
  }
};

txLaneBoundary::txLaneBoundary() : instancePtr(new BoundaryData) {}

txLaneBoundary::txLaneBoundary(const txLaneBoundary& laneboundary) : instancePtr(new BoundaryData) {
  instancePtr->data = laneboundary.instancePtr->data;
  instancePtr->geom = new txLineCurve;
  instancePtr->geom->clone(laneboundary.instancePtr->geom);
}

txLaneBoundary::~txLaneBoundary() {}

tx_laneboundary_t& txLaneBoundary::getTxData() const { return instancePtr->data; }

laneboundarypkid txLaneBoundary::getId() const { return instancePtr->data.pkid; }

txLaneBoundary& txLaneBoundary::setId(laneboundarypkid id) {
  instancePtr->data.pkid = id;
  return *this;
}

uint32_t txLaneBoundary::getSrs() const { return instancePtr->data.srs; }

txLaneBoundary& txLaneBoundary::setSrs(uint32_t srs) {
  instancePtr->data.srs = srs;
  return *this;
}

const txCurve* txLaneBoundary::getGeometry() const { return instancePtr->geom; }

txLaneBoundary& txLaneBoundary::setGeometry(const txCurve* curvePtr) {
  if (instancePtr->geom == NULL) instancePtr->geom = new txLineCurve;
  instancePtr->geom->clone(curvePtr);
  // mapdata_trans::LineCurveToPoints3d( instancePtr->geom, \
    //  instancePtr->data.geom );
  return *this;
}

txLaneBoundary& txLaneBoundary::setGeometry(const txCurve& curveIns) {
  if (instancePtr->geom == NULL) instancePtr->geom = new txLineCurve;
  instancePtr->geom->clone(curveIns);
  // mapdata_trans::LineCurveToPoints3d( instancePtr->geom, \
    //  instancePtr->data.geom );
  return *this;
}

txLaneBoundary& txLaneBoundary::setGeometry(const PointVec& points, CoordType ct) {
  if (instancePtr->geom == NULL) instancePtr->geom = new txLineCurve(ct);
  instancePtr->geom->setCoordType(ct);
  instancePtr->geom->setCoords(points);
  instancePtr->data.geom.resize(points.size());
  for (size_t i = 0; i < points.size(); ++i) {
    instancePtr->data.geom[i].x = points[i].x;
    instancePtr->data.geom[i].y = points[i].y;
    instancePtr->data.geom[i].z = points[i].z;
  }
  return *this;
}

txLaneBoundary& txLaneBoundary::transfer(const txPoint& oldEnuC, const txPoint& newEnuC) {
  if (instancePtr->geom == NULL || instancePtr->geom->empty()) {
    return *this;
  } else {
    instancePtr->geom->transfer(oldEnuC, newEnuC);
  }
  return *this;
}

txLaneBoundary& txLaneBoundary::transfer2ENU(const txPoint& enuCenter) {
  if (instancePtr->geom == NULL || instancePtr->geom->empty()) {
    return *this;
  } else {
    instancePtr->geom->transfer2ENU(enuCenter);
  }
  return *this;
}

txLaneBoundary& txLaneBoundary::setLaneMark(LANE_MARK mark) {
  instancePtr->data.lane_mark = uint32_t(mark);
  return *this;
}

txLaneBoundary& txLaneBoundary::setLaneWidth(double w) {
  instancePtr->data.lane_width = w;
  return *this;
}

txLaneBoundary& txLaneBoundary::setDoubleInterval(double d) {
  instancePtr->data.double_interval = d;
  return *this;
}

txLaneBoundary& txLaneBoundary::setDotSpace(double d) {
  instancePtr->data.dot_space = d;
  return *this;
}

txLaneBoundary& txLaneBoundary::setDotOffset(double d) {
  instancePtr->data.dot_offset = d;
  return *this;
}

txLaneBoundary& txLaneBoundary::setDotLen(double d) {
  instancePtr->data.dot_len = d;
  return *this;
}

LANE_MARK txLaneBoundary::getLaneMark() const { return (LANE_MARK)instancePtr->data.lane_mark; }

double txLaneBoundary::getLaneWidth() const { return instancePtr->data.lane_width; }

double txLaneBoundary::getDoubleInterval() const { return instancePtr->data.double_interval; }

double txLaneBoundary::getDotSpace() const { return instancePtr->data.dot_space; }

double txLaneBoundary::getDotOffset() const { return instancePtr->data.dot_offset; }

double txLaneBoundary::getDotLen() const { return instancePtr->data.dot_len; }

bool txLaneBoundary::reverse() {
  /*
  size_t geomSize = instancePtr->data.geom.size();
  for ( size_t i = 0, j = geomSize - 1; i < j; ++ i, --j )
          std::swap( instancePtr->data.geom[i], \
                  instancePtr->data.geom[j] );
  mapdata_trans::Points3dToLineCurve( instancePtr->data.geom, \
          instancePtr->geom );
  */
  instancePtr->geom->reverse();
  return true;
}
}  // namespace hadmap
