/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/map_data/map_object.h"
#include "common/coord_trans.h"
#include "common/log/system_logger.h"
#include "map_data/map_curve.h"

std::set<road_setion_id> CMapObject::LogicLightsRoadSet;
std::multimap<road_setion_id, LogicLights> CMapObject::LogicLightsMap;
// std::map<road_setion_id, LogicLights> CMapObject::LogicLightsMap;
CMapObject::CMapObject() {
  SetType(ENTITY_TYPE_MapObject);

  m_name = "";
  m_strType = "";
  m_strSubType = "";
  m_dLength = 0;
  m_dWidth = 0;
  m_dHeight = 0;
  m_dRoll = 0;
  m_dYaw = 0;
  m_dPitch = 0;
  m_dGroundHeight = 0;
}

void CMapObject::FromTxObject(hadmap::txObjectPtr obj) {
  m_id = obj->getId();
  m_type = static_cast<HADMAP_OBJECT_TYPE>(obj->getObjectType());
  m_name = obj->getName();
  obj->getLWH(m_dLength, m_dWidth, m_dHeight);
  obj->getRPY(m_dRoll, m_dPitch, m_dYaw);
  obj->getObjectFromatType(m_name, m_strType, m_strSubType);
  // obj->getRawTypeString(m_strType, m_strSubType);
  auto pos = obj->getPos();
  m_position.X() = pos.x;
  m_position.Y() = pos.y;
  m_position.Z() = pos.z;

  std::vector<hadmap::txLaneId> txlaneids;
  obj->getReliedLaneIds(txlaneids);
  for (int i = 0; i < txlaneids.size(); ++i) {
    LaneID laneid(txlaneids[i]);
    m_relied_lane_id.push_back(laneid);
  }

  int nCount = obj->getGeomSize();
  m_nGeomCount = nCount;

  for (int i = 0; i < nCount; ++i) {
    hadmap::txObjGeomPtr pGeom = obj->getGeom(i);
    const hadmap::txCurve* pCurve = pGeom->getGeometry();
    if (pCurve) {
      if (m_type == HADMAP_OBJECT_TYPE_TrafficSign || m_type == HADMAP_OBJECT_TYPE_RoadSign ||
          m_type == HADMAP_OBJECT_TYPE_CrossWalk || m_type == HADMAP_OBJECT_TYPE_Arrow ||
          m_type == HADMAP_OBJECT_TYPE_Text || m_type == HADMAP_OBJECT_TYPE_RoadSign_Text ||
          m_type == HADMAP_OBJECT_TYPE_LaneBoundary || /*m_type == HADMAP_OBJECT_TYPE_ParkingSpace ||*/
          m_type == HADMAP_OBJECT_TYPE_DrivingArea) {
        const hadmap::txLineCurve* pLineCurve = dynamic_cast<const hadmap::txLineCurve*>(pCurve);
        CMapCurve::FromTxPlane(m_points, (*pLineCurve));
      } else if (m_type == HADMAP_OBJECT_TYPE_ParkingSpace) {
      } else {
        CMapCurve::FromTxCurve(m_points, (*pCurve));
        m_points.clear();  // 目前奇奇怪怪的object暂不支持
      }

    } else {
      SYSTEM_LOGGER_WARN("map object %d curve is null", m_id);
    }
  }
  if (obj->getRepeat()) {
    hadmap::PointVec pts;
    obj->getRepeat()->getPoints(pts);
    for (const auto& p : pts) {
      PointWH pwh;
      pwh.X() = p.x;
      pwh.Y() = p.y;
      pwh.Z() = p.z;
      pwh.w = p.w;
      pwh.h = p.h;
      m_repeat.push_back(pwh);
    }
  }
  auto parking = dynamic_cast<hadmap::txParkingSpace*>(obj.get());
  if (parking) {
    m_parking_space.valid = true;
    m_parking_space.access = parking->getAccess();
    m_parking_space.restrictions = parking->getRestrictions();
    if (parking->getFrontMarking().valid) {
      CParkingSpace::ParkingMarking pm;
      pm.side = "front";
      pm.type = parking->getFrontMarking().type;
      pm.width = parking->getFrontMarking().width;
      pm.color = parking->getFrontMarking().color;
      m_parking_space.marks.push_back(pm);
    }
    if (parking->getRearMarking().valid) {
      CParkingSpace::ParkingMarking pm;
      pm.side = "rear";
      pm.type = parking->getFrontMarking().type;
      pm.width = parking->getFrontMarking().width;
      pm.color = parking->getFrontMarking().color;
      m_parking_space.marks.push_back(pm);
    }
    if (parking->getLeftMarking().valid) {
      CParkingSpace::ParkingMarking pm;
      pm.side = "left";
      pm.type = parking->getFrontMarking().type;
      pm.width = parking->getFrontMarking().width;
      pm.color = parking->getFrontMarking().color;
      m_parking_space.marks.push_back(pm);
    }
    if (parking->getRightMarking().valid) {
      CParkingSpace::ParkingMarking pm;
      pm.side = "right";
      pm.type = parking->getFrontMarking().type;
      pm.width = parking->getFrontMarking().width;
      pm.color = parking->getFrontMarking().color;
      m_parking_space.marks.push_back(pm);
    }
  }
  obj->getUserData(m_user_data);
  std::string origialName = obj->getName();
  int apos = origialName.find("-");
  if (apos < origialName.size()) {
    std::string key = origialName.substr(0, apos);
    std::string value = origialName.substr(apos + 1, origialName.size() - apos - 1);
    if (key == "pole" || key == "sensor") {
      m_user_data.insert(std::make_pair("pole_id", value));
    }
  }
  std::string formatType;
  std::string formatSubType;
  std::string formatName;
  if (obj->getGeomSize() > 0 && obj->getObjectType() == hadmap::OBJECT_TYPE_ParkingSpace) {
    this->m_outlines.clear();
    for (int i = 0; i < obj->getGeomSize(); i++) {
      const hadmap::txLineCurve* lineCurve = dynamic_cast<const hadmap::txLineCurve*>(obj->getGeom(i)->getGeometry());
      hadmap::PointVec points;
      lineCurve->getPoints(points);
      std::vector<CPoint3D> point3dVec;
      for (int pointIndex = 0; pointIndex < points.size(); pointIndex++) {
        hadmap::txPoint pa = points.at(pointIndex);
        CPoint3D pt(pa.x, pa.y, pa.z);
        point3dVec.push_back(pt);
      }
      this->m_outlines[i] = point3dVec;
    }
    this->m_parkingMarkWidth = std::to_string(obj->getOdData().markWidth);
    int color = obj->getGeom()->getColor();
    switch (color) {
      case hadmap::OBJECT_COLOR_White:
        this->m_parkingMarkColor = "white";
        break;
      case hadmap::OBJECT_COLOR_Red:
        this->m_parkingMarkColor = "red";
        break;
      case hadmap::OBJECT_COLOR_Yellow:
        this->m_parkingMarkColor = "yellow";
        break;
      case hadmap::OBJECT_COLOR_Blue:
        this->m_parkingMarkColor = "blue";
        break;
      case hadmap::OBJECT_COLOR_Green:
        this->m_parkingMarkColor = "green";
        break;
      case hadmap::OBJECT_COLOR_Black:
        this->m_parkingMarkColor = "black";
        break;
      default:
        this->m_parkingMarkColor = "none";
        break;
    }
  }

  CMapCurve::FormAabb(m_points, m_aabb_ll);
}

void CMapObject::Lonlat2Local(double refLon, double refLat, double refAlt) {
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2Local(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);

  coord_trans_api::lonlat2local(m_position.X(), m_position.Y(), m_position.Z(), refLon, refLat, refAlt);
  for (auto& p : m_repeat) {
    coord_trans_api::lonlat2local(p.X(), p.Y(), p.Z(), refLon, refLat, refAlt);
  }
}

void CMapObject::Lonlat2ENU(double refLon, double refLat, double refAlt) {
  CMapCurve::FormAabb(m_points, m_aabb_ll);
  CMapCurve::Lonlat2ENU(m_points, refLon, refLat, refAlt);
  CMapCurve::FormAabb(m_points, m_aabb_xyz);

  coord_trans_api::lonlat2enu(m_position.X(), m_position.Y(), m_position.Z(), refLon, refLat, refAlt);

  if (m_type == HADMAP_OBJECT_TYPE_ParkingSpace) {
    m_points = CalculateCornerPoints(m_position.X(), m_position.Y(), m_position.Z(), m_dLength, m_dWidth, m_dYaw,
                                     m_dPitch, m_dRoll);
  }

  for (auto& p : m_repeat) {
    coord_trans_api::lonlat2enu(p.X(), p.Y(), p.Z(), refLon, refLat, refAlt);
  }

  for (auto& p : m_outlines) {
    for (auto& pt : p.second) coord_trans_api::lonlat2enu(pt.X(), pt.Y(), pt.Z(), refLon, refLat, refAlt);
  }

  for (auto& p : m_geoms.sPoints) {
    for (auto& pt : p) coord_trans_api::lonlat2enu(pt.x, pt.y, pt.z, refLon, refLat, refAlt);
  }
}

LogicLights CMapObject::CopyOneRefLight(CMapObject* srcobj) {
  LogicLights temp;
  temp.m_id = srcobj->Id();
  temp.m_type = srcobj->Type();
  temp.m_name = srcobj->Name();
  temp.m_dLength = srcobj->Length();
  temp.m_dWidth = srcobj->Width();
  temp.m_dHeight = srcobj->Height();
  temp.m_dGroundHeight = srcobj->GroundHeight();
  temp.m_dRoll = srcobj->Roll();
  temp.m_dYaw = srcobj->Yaw();
  temp.m_dPitch = srcobj->Pitch();
  temp.m_strType = "trafficLight_reference";
  temp.m_strSubType = srcobj->StrSubType();
  temp.m_position = srcobj->Position();
  temp.m_nGeomCount = srcobj->GeomCount();
  temp.m_points = srcobj->Data();
  temp.m_user_data = srcobj->UserData();
  return temp;
  // temp.m_repeat = srcobj->Repeat();
}

void CMapObject::Trans(LogicLights LL, hadmap::txPoint& LogicPoints) {
  m_id = LL.m_id;
  m_type = LL.m_type;
  m_name = LL.m_name;

  m_dLength = LL.m_dLength;
  m_dWidth = LL.m_dWidth;
  m_dHeight = LL.m_dHeight;

  m_dGroundHeight = LL.m_dGroundHeight;

  m_dRoll = LL.m_dRoll;
  m_dYaw = LL.m_dYaw;
  m_dPitch = LL.m_dPitch;
  m_strType = "trafficLight_reference";

  m_strSubType = LL.m_strSubType;

  m_position.X() = LogicPoints.x;
  m_position.Y() = LogicPoints.y;
  m_position.Z() = LogicPoints.z;

  m_nGeomCount = LL.m_nGeomCount;
  m_points = LL.m_points;
  m_user_data = LL.m_user_data;
}

std::vector<vector3<double>> CMapObject::CalculateCornerPoints(const double& centerX, const double& centerY,
                                                               const double& centerZ, const double& Len,
                                                               const double& Wid, const double& heading,
                                                               const double& pitch, const double& roll) {
  Eigen::Quaterniond m_Matrix = Eigen::AngleAxisd(heading, Eigen::Vector3d::UnitZ()) *
                                Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                                Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());

  std::vector<vector3<double>> res;

  Eigen::Vector3d center_pos(centerX, centerY, centerZ);

  Eigen::Vector3d p1, p2, p3, p4;
  p1 = center_pos + m_Matrix * Eigen::Vector3d(-Len * 0.5, -Wid * 0.5, 0);
  vector3<double> res_p1;
  res_p1.X() = p1[0];
  res_p1.Y() = p1[1];
  res_p1.Z() = p1[2];
  res.push_back(res_p1);

  p2 = center_pos + m_Matrix * Eigen::Vector3d(-Len * 0.5, Wid * 0.5, 0);
  vector3<double> res_p2;
  res_p2.X() = p2[0];
  res_p2.Y() = p2[1];
  res_p2.Z() = p2[2];
  res.push_back(res_p2);
  res.push_back(res_p2);

  p3 = center_pos + m_Matrix * Eigen::Vector3d(Len * 0.5, Wid * 0.5, 0);
  vector3<double> res_p3;
  res_p3.X() = p3[0];
  res_p3.Y() = p3[1];
  res_p3.Z() = p3[2];
  res.push_back(res_p3);
  res.push_back(res_p3);

  p4 = center_pos + m_Matrix * Eigen::Vector3d(Len * 0.5, -Wid * 0.5, 0);
  vector3<double> res_p4;
  res_p4.X() = p4[0];
  res_p4.Y() = p4[1];
  res_p4.Z() = p4[2];
  res.push_back(res_p4);

  res.push_back(res_p4);
  res.push_back(res_p1);
  return res;
}
bool CMapObject::SetGeomsFromRoad(const hadmap::txLanePtr lanePtr, hadmap::txObjectPtr obj) {
  if (!lanePtr) return false;

  if (Name() == "Lateral_Deceleration_Marking" || Name() == "Variable_Direction_Lane_Line" ||
      Name() == "Longitudinal_Deceleration_Marking" || Name() == "Road_Guide_Lane_Line" ||
      Name() == "White_Semicircle_Line_Vehicle_Distance_Confirmation") {
    double starts = obj->getOdData().s - obj->getOdData().length / 2.0;
    double ends = obj->getOdData().s + obj->getOdData().length / 2.0;
    int index = 0;
    m_geoms.sPoints.clear();
    hadmap::PointVec _leftPoints;
    hadmap::PointVec _rightPoints;
    while (starts + index < ends) {
      // getleftboundary points
      double _s = (starts + index);
      hadmap::txLaneBoundaryPtr _leftBoundary = lanePtr->getLeftBoundary();
      if (_leftBoundary->getGeometry() != NULL) {
        _leftPoints.push_back(_leftBoundary->getGeometry()->getPoint(_s));
      }

      // get rightboundary points
      hadmap::txLaneBoundaryPtr _rightBoundary = lanePtr->getRightBoundary();
      if (_rightBoundary->getGeometry() != NULL) {
        _rightPoints.push_back(_rightBoundary->getGeometry()->getPoint(_s));
      }
      index++;
    }

    m_geoms.sType = E_DOUBLE_LINE;
    m_geoms.sPoints.push_back(_leftPoints);
    m_geoms.sPoints.push_back(_rightPoints);
  }
  if (Name() == "Intersection_Guide_Line") {
    m_geoms.sType = E_SINGLE_LINE;

    if (obj->getGeomSize() > 0) {
      hadmap::PointVec _points = obj->getGeom(0)->getGeom5D();
      m_geoms.sPoints.push_back(_points);
    }
  }
}
