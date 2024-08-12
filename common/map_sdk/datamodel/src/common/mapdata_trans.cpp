// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "common/mapdata_trans.h"
#include "types/map_types.h"

#include <exception>

using namespace hadmap;

namespace mapdata_trans {
// Function to convert Points3d to LineCurve
void Points3dToLineCurve(const Points3d& geom, txLineCurve* curvePtr) {
  if (geom.size() == 1) throw std::runtime_error("Geom Size Error, Size 1");
  if (curvePtr == NULL) throw std::runtime_error("Line curve not initialize");

  PointVec geomPoints;
  for (size_t i = 0; i < geom.size(); ++i) geomPoints.push_back(txPoint(geom[i].x, geom[i].y, geom[i].z));

  curvePtr->setCoords(geomPoints);
}

// Function to convert Points5d to LineCurve
void Points5dToLineCurve(const Points5d& geom, txLineCurve* curvePtr) {
  if (geom.size() == 1) {
    // throw std::runtime_error("Geom Size Error, Size 1");
    if (curvePtr == NULL) {
      throw std::runtime_error("Line curve not initialize");
    }
  }

  PointVec geomPoints;
  for (size_t i = 0; i < geom.size(); ++i) {
    geomPoints.push_back(txPoint(geom[i].x, geom[i].y, geom[i].z));
  }

  curvePtr->setCoords(geomPoints);
}

// Function to convert LineCurve to Points3d
void LineCurveToPoints3d(const txLineCurve* curvePtr, Points3d& geom) {
  if (curvePtr == NULL) throw std::runtime_error("Line Curve Pointer Empty");

  geom.clear();
  if (curvePtr->empty()) {
    geom.clear();
    return;
  }

  for (size_t i = 0; i < curvePtr->size(); ++i) {
    txPoint p = curvePtr->getPoint(i);
    geom.push_back(Point3d(p.x, p.y, p.z));
  }
}

// Function to convert LineCurve to Points5d
void LineCurveToPoints5d(const txLineCurve* curvePtr, Points5d& geom) {
  if (curvePtr == NULL) throw std::runtime_error("Line Curve Pointer Empty");

  geom.clear();
  if (curvePtr->empty()) {
    geom.clear();
    return;
  }

  for (size_t i = 0; i < curvePtr->size(); ++i) {
    txPoint p = curvePtr->getPoint(i);
    geom.push_back(Point5d(p.x, p.y, p.z, 0.0, 0.0));
  }
}

// Function to convert tx_lane_t to txLanePtr
void TxLaneToHadLane(const tx_lane_t& lane, txLanePtr& lanePtr) {
  if (!lanePtr) lanePtr.reset(new txLane);

  (*lanePtr)
      .setRoadId(lane.road_pkid)
      .setSectionId(lane.section_id)
      .setId(lane.lane_id)
      .setName(lane.name)
      .setLeftBoundaryId(lane.left_boundary)
      .setRightBoundaryId(lane.right_boundary)
      .setLaneType(LANE_TYPE(lane.lane_type))
      .setLaneArrow(LANE_ARROW(lane.lane_arrow))
      .setSpeedLimit(lane.speed_limit);

  PointVec points(lane.geom.size());
  for (size_t i = 0; i < lane.geom.size(); ++i) {
    points[i].x = lane.geom[i].x;
    points[i].y = lane.geom[i].y;
    points[i].z = lane.geom[i].z;
  }
  (*lanePtr).setGeometry(points, COORD_WGS84);
}

// Function to convert txLanePtr to tx_lane_t
void HadLaneToTxLane(const txLanePtr lanePtr, tx_lane_t& lane) {
  if (lanePtr == NULL) return;

  lane.road_pkid = lanePtr->getRoadId();
  lane.section_id = lanePtr->getSectionId();
  lane.lane_id = lanePtr->getId();
  lane.left_boundary = lanePtr->getLeftBoundaryId();
  lane.right_boundary = lanePtr->getRightBoundaryId();
  lane.lane_type = (uint32_t)lanePtr->getLaneType();
  lane.lane_arrow = (uint32_t)lanePtr->getLaneArrow();
  lane.speed_limit = lanePtr->getSpeedLimit();

  if (lanePtr->getName().length() < 32) strncpy(lane.name, lanePtr->getName().c_str(), lanePtr->getName().length());

  if (lanePtr->getGeometry() == NULL || lanePtr->getGeometry()->empty()) {
    return;
  } else {
    const txLineCurve* _curve_ptr = dynamic_cast<const txLineCurve*>(lanePtr->getGeometry());
    LineCurveToPoints3d(_curve_ptr, lane.geom);
  }
}

// Function to convert tx_laneboundary_t to txLaneBoundaryPtr
void TxBoundaryToHadBoundary(const tx_laneboundary_t& boundary, txLaneBoundaryPtr& boundaryPtr) {
  if (!boundaryPtr) boundaryPtr.reset(new txLaneBoundary);

  (*boundaryPtr).setId(boundary.pkid).setSrs(boundary.srs).setLaneMark(LANE_MARK(boundary.lane_mark));

  PointVec points(boundary.geom.size());
  for (size_t i = 0; i < boundary.geom.size(); ++i) {
    points[i].x = boundary.geom[i].x;
    points[i].y = boundary.geom[i].y;
    points[i].z = boundary.geom[i].z;
  }
  (*boundaryPtr).setGeometry(points, COORD_WGS84);
}

// Function to convert txLaneBoundaryPtr to tx_laneboundary_t
void HadBoundaryToTxBoundary(const txLaneBoundaryPtr boundaryPtr, tx_laneboundary_t& boundary) {
  if (boundaryPtr == NULL) return;

  boundary.pkid = boundaryPtr->getId();
  boundary.lane_mark = (uint32_t)boundaryPtr->getLaneMark();

  if (boundaryPtr->getGeometry() == NULL || boundaryPtr->getGeometry()->empty()) {
    return;
  } else {
    const txLineCurve* _curve_ptr = dynamic_cast<const txLineCurve*>(boundaryPtr->getGeometry());
    LineCurveToPoints3d(_curve_ptr, boundary.geom);
  }
}

// Function to convert tx_road_t to txRoadPtr
void TxRoadToHadRoad(const tx_road_t& road, txRoadPtr& roadPtr) {
  if (!roadPtr) roadPtr.reset(new txRoad);

  (*roadPtr)
      .setId(road.pkid)
      .setTaskId(road.task_pkid)
      .setName(road.name)
      .setGround(road.ground)
      .setRoadType(ROAD_TYPE(road.road_type))
      .setDirection(road.direction)
      .setToll(road.toll)
      .setFuncClass(road.func_class)
      .setUrban(road.urban)
      .setPaved(road.paved)
      .setTransType(road.transition_type)
      .setLaneMarking(road.lane_mark)
      .setMaterial(ROAD_MATERIAL(road.material));

  txLineCurve curve(COORD_WGS84);
  Points3dToLineCurve(road.geom, &curve);
  (*roadPtr).setGeometry(curve);
}

// Function to convert txRoadPtr to tx_road_t
void HadRoadToTxRoad(const txRoadPtr roadPtr, tx_road_t& road) {
  if (roadPtr == NULL) return;

  road.pkid = roadPtr->getId();
  road.task_pkid = roadPtr->getTaskId();
  road.ground = roadPtr->getGround();
  road.road_type = (uint32_t)roadPtr->getRoadType();
  road.direction = roadPtr->getDirection();
  road.toll = roadPtr->getToll();
  road.func_class = roadPtr->getFuncClass();
  road.urban = roadPtr->isUrban();
  road.paved = roadPtr->isPaved();
  road.transition_type = roadPtr->getTransType();
  road.lane_mark = roadPtr->hasLaneMarking();
  road.material = roadPtr->getMaterial();

  if (roadPtr->getName().length() < 32) strncpy(road.name, roadPtr->getName().c_str(), roadPtr->getName().length());

  if (roadPtr->getGeometry() == NULL || roadPtr->getGeometry()->empty()) {
    return;
  } else {
    const txLineCurve* _curve_ptr = dynamic_cast<const txLineCurve*>(roadPtr->getGeometry());
    LineCurveToPoints3d(_curve_ptr, road.geom);
  }
}

// Function to convert tx_lanelink_t to txLaneLinkPtr
void TxLaneLinkToHadLaneLink(const tx_lanelink_t& lanelink, txLaneLinkPtr& lanelinkPtr) {
  if (!lanelinkPtr) lanelinkPtr.reset(new txLaneLink);

  (*lanelinkPtr)
      .setId(lanelink.pkid)
      .setFromRoadId(lanelink.from_road_pkid)
      .setFromSectionId(lanelink.from_section_id)
      .setFromLaneId(lanelink.from_lane_id)
      .setToRoadId(lanelink.to_road_pkid)
      .setToSectionId(lanelink.to_section_id)
      .setToLaneId(lanelink.to_lane_id)
      .setCurvature(lanelink.curvature)
      .setSlope(lanelink.slope)
      .setPreContact(lanelink.preContact)
      .setSuccContact(lanelink.succContact)
      .setJunctionId(lanelink.junction_id);

  txLineCurve curve(COORD_WGS84);
  Points3dToLineCurve(lanelink.geom, &curve);
  (*lanelinkPtr).setGeometry(curve);
}

// Function to convert txLaneLinkPtr to tx_lanelink_t
void HadLaneLinkToTxLaneLink(const txLaneLinkPtr lanelinkPtr, tx_lanelink_t& lanelink) {
  if (lanelinkPtr == NULL) return;

  lanelink.pkid = lanelinkPtr->getId();
  lanelink.from_road_pkid = lanelinkPtr->fromRoadId();
  lanelink.from_section_id = lanelinkPtr->fromSectionId();
  lanelink.from_lane_id = lanelinkPtr->fromLaneId();
  lanelink.to_road_pkid = lanelinkPtr->toRoadId();
  lanelink.to_section_id = lanelinkPtr->toSectionId();
  lanelink.to_lane_id = lanelinkPtr->toLaneId();

  if (lanelinkPtr->getGeometry() == NULL || lanelinkPtr->getGeometry()->empty()) {
    return;
  } else {
    const txLineCurve* _curve_ptr = dynamic_cast<const txLineCurve*>(lanelinkPtr->getGeometry());
    LineCurveToPoints3d(_curve_ptr, lanelink.geom);
  }
}

// Function to convert tx_object_t to txObjectPtr
void TxObjectToHadObject(const hadmap::tx_object_t& object, hadmap::txObjectPtr& objectPtr) {
  if (!objectPtr) objectPtr.reset(new txObject);

  (*objectPtr).setId(object.pkid).setObjectType(static_cast<OBJECT_TYPE>(object.type)).setName(object.name);
}

// Function to convert txObjectPtr to tx_object_t
void HadObjectToTxObject(const hadmap::txObjectPtr objectPtr, hadmap::tx_object_t& object) {
  if (objectPtr == NULL) return;

  object.pkid = objectPtr->getId();
  object.type = static_cast<uint32_t>(objectPtr->getObjectType());

  if (objectPtr->getName().length() < 32) {
    strncpy(object.name, objectPtr->getName().c_str(), objectPtr->getName().length());
  }
}

// Function to convert tx_object_geom_t to txObjGeomPtr
void TxObjGeomToHadObjGeom(const hadmap::tx_object_geom_t& geom, hadmap::txObjGeomPtr& geomPtr) {
  if (!geomPtr) geomPtr.reset(new txObjectGeom);

  (*geomPtr)
      .setId(geom.pkid)
      .setType(static_cast<OBJECT_GEOMETRY_TYPE>(geom.type))
      .setStyle(static_cast<OBJECT_STYLE>(geom.style))
      .setColor(geom.color)
      .setGeomText(geom.text);

  txLineCurve curve(COORD_WGS84);
  Points5dToLineCurve(geom.geom, &curve);
  (*geomPtr).setGeometry(curve);
}
}  // namespace mapdata_trans
