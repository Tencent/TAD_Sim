/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "map_object.h"
#include "common/coord_trans.h"
#include "common/log/system_logger.h"
#include "map_curve.h"

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
  obj->getRawTypeString(m_strType, m_strSubType);
  auto pos = obj->getPos();
  m_position.X() = pos.x;
  m_position.Y() = pos.y;
  m_position.Z() = pos.z;

  std::vector<hadmap::txLaneId> txlaneids;
  obj->getReliedLaneIds(txlaneids);
  for (int i = 0; i < txlaneids.size(); ++i) {
    LaneID laneid = txlaneids[i];

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
        m_points.clear();  //
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

  // 根据车位的中心、大小、旋转计算车位的四个角，这个暂时不提供
  /*
  if (m_type == HADMAP_OBJECT_TYPE_ParkingSpace) {
    m_points = CalculateCornerPoints(m_position.X(), m_position.Y(), m_position.Z(), m_dLength, m_dWidth, m_dYaw,
                                     m_dPitch, m_dRoll);
  }
  */

  for (auto& p : m_repeat) {
    coord_trans_api::lonlat2enu(p.X(), p.Y(), p.Z(), refLon, refLat, refAlt);
  }
}
