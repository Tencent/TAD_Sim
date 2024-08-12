// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_objgeom.h"
#include "common/mapdata_trans.h"
#include "structs/hadmap_curve.h"
#include "structs/map_structs.h"

namespace hadmap {
struct txObjectGeom::GeomData {
 public:
  tx_object_geom_t data;

  txLineCurve* geomPtr;

  txLineCurve* axisPtr;

 public:
  GeomData() : geomPtr(NULL), axisPtr(NULL) {}

  ~GeomData() {
    if (NULL != geomPtr) delete geomPtr;
    if (NULL != axisPtr) delete axisPtr;
  }
};

txObjectGeom::txObjectGeom() : instancePtr(new GeomData) {}

txObjectGeom::~txObjectGeom() {}

tx_object_geom_t txObjectGeom::getTxData() const { return instancePtr->data; }

objectgeompkid txObjectGeom::getId() const { return instancePtr->data.pkid; }

OBJECT_GEOMETRY_TYPE txObjectGeom::getType() const { return (OBJECT_GEOMETRY_TYPE)instancePtr->data.type; }

OBJECT_STYLE txObjectGeom::getStyle() const { return (OBJECT_STYLE)instancePtr->data.style; }

const txCurve* txObjectGeom::getGeometry() const { return instancePtr->geomPtr; }

const txCurve* txObjectGeom::getMainAxis() const { return instancePtr->axisPtr; }

uint32_t txObjectGeom::getColor() const { return instancePtr->data.color; }

std::string txObjectGeom::getGeomText() const { return instancePtr->data.text; }

PointVec txObjectGeom::getGeom5D() const {
  PointVec ret;
  for (auto it : instancePtr->data.geom) {
    ret.push_back(txPoint(it.x, it.y, it.z));
  }
  return ret;
}

txObjectGeom& txObjectGeom::setId(const objectgeompkid& id) {
  instancePtr->data.pkid = id;
  return *this;
}

txObjectGeom& txObjectGeom::setType(const OBJECT_GEOMETRY_TYPE& type) {
  instancePtr->data.type = type;
  return *this;
}

txObjectGeom& txObjectGeom::setStyle(const OBJECT_STYLE& style) {
  instancePtr->data.style = style;
  return *this;
}

txObjectGeom& txObjectGeom::setColor(const uint32_t& color) {
  instancePtr->data.color = color;
  return *this;
}

txObjectGeom& txObjectGeom::setGeomText(const std::string& text) {
  strcpy(instancePtr->data.text, text.c_str());
  return *this;
}

txObjectGeom& txObjectGeom::setGeometry(const txCurve* curvePtr) {
  if (instancePtr->geomPtr == NULL) instancePtr->geomPtr = new txLineCurve;
  instancePtr->geomPtr->clone(curvePtr);
  mapdata_trans::LineCurveToPoints5d(instancePtr->geomPtr, instancePtr->data.geom);
  genMainAxis();
  return *this;
}

txObjectGeom& txObjectGeom::setGeometry(const txCurve& curveIns) {
  if (instancePtr->geomPtr == NULL) instancePtr->geomPtr = new txLineCurve;
  instancePtr->geomPtr->clone(curveIns);
  mapdata_trans::LineCurveToPoints5d(instancePtr->geomPtr, instancePtr->data.geom);
  genMainAxis();
  return *this;
}

txObjectGeom& txObjectGeom::setGeometry(const PointVec& points, CoordType ct) {
  if (instancePtr->geomPtr == NULL) instancePtr->geomPtr = new txLineCurve(ct);
  instancePtr->geomPtr->setCoordType(ct);
  instancePtr->geomPtr->setCoords(points);
  mapdata_trans::LineCurveToPoints5d(instancePtr->geomPtr, instancePtr->data.geom);
  genMainAxis();
  return *this;
}

txObjectGeom& txObjectGeom::transfer(const txPoint& oldEnuC, const txPoint& newEnuC) {
  if (NULL == instancePtr->geomPtr || instancePtr->geomPtr->empty()) {
    return *this;
  } else {
    instancePtr->geomPtr->transfer(oldEnuC, newEnuC);
  }
  transferMainAxis(oldEnuC, newEnuC);
  return *this;
}

// transfer to enu
txObjectGeom& txObjectGeom::transfer2ENU(const txPoint& enuCenter) {
  if (NULL == instancePtr->geomPtr || instancePtr->geomPtr->empty()) {
    return *this;
  } else {
    /*
    roadpkid rid = instancePtr->data.road_pkid;
    sectionpkid sid = instancePtr->data.section_id;
    lanepkid lid = instancePtr->data.lane_id;
    */
    instancePtr->geomPtr->transfer2ENU(enuCenter);
  }
  transferMainAxis2ENU(enuCenter);
  return *this;
}

bool txObjectGeom::genMainAxis() {
  if (instancePtr->data.type != OBJECT_GEOMETRY_TYPE_Polygon || instancePtr->geomPtr == NULL ||
      instancePtr->geomPtr->empty()) {
    return false;
  } else {
    PointVec points;
    instancePtr->geomPtr->getPoints(points);
    if (points.size() < 2) return false;
    if (instancePtr->axisPtr != NULL) delete instancePtr->axisPtr;
    instancePtr->axisPtr = new txLineCurve(instancePtr->geomPtr->getCoordType());
    PointVec axis_points(points.begin(), points.begin() + 2);
    instancePtr->axisPtr->setCoords(axis_points);
    return true;
  }
}

bool txObjectGeom::transferMainAxis(const txPoint& old_enu_c, const txPoint& new_enu_c) {
  if (NULL == instancePtr->axisPtr || instancePtr->axisPtr->empty()) {
    return false;
  } else {
    return instancePtr->axisPtr->transfer(old_enu_c, new_enu_c);
  }
}

bool txObjectGeom::transferMainAxis2ENU(const txPoint& enu_center) {
  if (NULL == instancePtr->axisPtr || instancePtr->axisPtr->empty()) {
    return false;
  } else {
    return instancePtr->axisPtr->transfer2ENU(enu_center);
  }
}
}  // namespace hadmap
