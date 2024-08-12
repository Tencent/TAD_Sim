/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "map_curve.h"
#include <common/coord_trans.h>
#include "../engine/entity/aabb.h"
#include "common/log/system_logger.h"

bool bTwoPointSample = false;
void CMapCurve::FromTxCurve(PointsVec& points, const hadmap::txCurve& curve) {
  bool bEmpty = curve.empty();
  if (bTwoPointSample) {
    if (!bEmpty) {
      hadmap::txPoint st = curve.getStart();
      // CPoint3D p = curve.getStart();
      CPoint3D p(st);
      points.push_back(p);
      p = curve.getEnd();
      points.push_back(p);
    }
  } else {
    if (!bEmpty) {
      double fLen = curve.getLength();
      double fInterval = 10;
      if (fLen < 50) {
        fInterval = fLen / 10;
      } else if (fLen >= 50 && fLen < 100) {
        fInterval = fLen / 20;
      } else if (fLen >= 100 && fLen < 300) {
        fInterval = fLen / 30;
      }

      if (fLen < 0.01) {
        fInterval = 1;
      }

      hadmap::PointVec pts;
      curve.sample(fInterval, pts);
      if (pts.size() > 0) {
        hadmap::PointVec::iterator itr = pts.begin();
        CPoint3D p(*itr);
        points.push_back(p);
        itr++;
        for (; itr != pts.end(); ++itr) {
          p = (*itr);
          points.push_back(p);
          points.push_back(p);
        }
        points.pop_back();
      }
    }
  }
}

void CMapCurve::FromTxLanelinkCurve(PointsVec& points, const hadmap::txCurve& curve) {
  bool bEmpty = curve.empty();
  if (bTwoPointSample) {
    if (!bEmpty) {
      hadmap::txPoint st = curve.getStart();
      // CPoint3D p = curve.getStart();
      CPoint3D p(st);
      points.push_back(p);
      p = curve.getEnd();
      points.push_back(p);
    }
  } else {
    if (!bEmpty) {
      double fLen = curve.getLength();
      double fInterval = 2;

      hadmap::PointVec pts;
      curve.sample(fInterval, pts);
      if (pts.size() > 0) {
        hadmap::PointVec::iterator itr = pts.begin();
        CPoint3D p(*itr);
        points.push_back(p);
        itr++;
        for (; itr != pts.end(); ++itr) {
          p = (*itr);
          points.push_back(p);
          points.push_back(p);
        }
        points.pop_back();
      }
    }
  }
}

void CMapCurve::FromTxPlane(PointsVec& points, const hadmap::txLineCurve& curve) {
  bool bEmpty = curve.empty();
  if (!bEmpty) {
    hadmap::PointVec pts;
    curve.getPoints(pts);
    if (pts.size() > 0) {
      hadmap::PointVec::iterator itr = pts.begin();
      CPoint3D p(*itr);
      points.push_back(p);
      itr++;
      for (; itr != pts.end(); ++itr) {
        p = (*itr);
        points.push_back(p);
        points.push_back(p);
      }

      if (pts.size() > 1) {
        points.pop_back();
      } else if (pts.size() == 1) {
        points.push_back(points[0]);
      } else {
        SYSTEM_LOGGER_ERROR("map object position data error!");
      }

    } else {
    }
  }
}

void CMapCurve::Lonlat2Local(PointsVec& points, double refLon, double refLat, double refAlt) {
  double x = 0, y = 0, z = 0;

  PointsVec::iterator itr = points.begin();
  for (; itr != points.end(); ++itr) {
    x = itr->X();
    y = itr->Y();
    z = itr->Z();
    coord_trans_api::lonlat2local(x, y, z, refLon, refLat, refAlt);
    itr->X() = x;
    itr->Y() = y;
    itr->Z() = z;
  }
}

void CMapCurve::Lonlat2ENU(PointsVec& points, double refLon, double refLat, double refAlt) {
  double x = 0, y = 0, z = 0;

  PointsVec::iterator itr = points.begin();
  for (; itr != points.end(); ++itr) {
    x = itr->X();
    y = itr->Y();
    z = itr->Z();
    coord_trans_api::lonlat2enu(x, y, z, refLon, refLat, refAlt);
    itr->X() = x;
    itr->Y() = y;
    itr->Z() = z;
  }
}

void CMapCurve::Global2Local(PointsVec& points, double refLon, double refLat, double refAlt) {
  double x = 0, y = 0, z = 0;

  PointsVec::iterator itr = points.begin();
  for (; itr != points.end(); ++itr) {
    x = itr->X();
    y = itr->Y();
    z = itr->Z();
    coord_trans_api::global2local(x, y, z, refLon, refLat, refAlt);
    itr->X() = x;
    itr->Y() = y;
    itr->Z() = z;
  }
}

void CMapCurve::FormAabb(PointsVec& points, CAABB& aabb) {
  PointsVec::iterator itr = points.begin();
  for (; itr != points.end(); ++itr) {
    aabb.Expand((*itr));
  }
}
