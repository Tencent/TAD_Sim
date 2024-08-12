// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_curve.h"
#include "common/coord_trans.h"
#include "common/map_util.h"

#include <float.h>
#include <math.h>
#include <algorithm>
#include <stdexcept>
#include <string>

namespace hadmap {
#define SL_BUFFER 2.0

struct txLineCurve::CurveData {
 public:
  // geometry of cur curve
  PointVec geom;
  // distance between cur point to begin
  //  0, 0 - 1, 0 - 2, 0 - 3, ..., 0 - n-1
  std::vector<double> disFromBegin;

  // yaw
  std::vector<double> pointsYaw;

  // point normal
  // point to right side
  std::vector<Point2d> pointsNormal;

  // sl trans info
  // intersect point | normal
  // if record intersect point, attention the coord, + center
  // z == 0 -> normal, normal point to right
  // z > 0 -> left point
  // z < 0 -> right point
  // only record info when coord type is not WGS84
  // [ [0, 1], [1, 2], .... ,[ n-1, n ] ]
  std::vector<Point3d> slRefInfo;

  // center point
  txPoint center;

 public:
  CurveData() : center(0.0, 0.0, 0.0) {}

  ~CurveData() {}

 public:
  // set coords
  void setCoords(const PointVec& points);

  // get length
  double getLength() const;

  // get size
  size_t size() const;

  // get specified point
  txPoint getPoint(double distance, CoordType ct) const;

  int getPointIndex(double distance) const;

  txPoint getPoint(size_t index) const;

  double yaw(size_t index) const;

  void getPoints(PointVec& points) const;

  // get geometry
  void getGeometry(PointVec& g) const;

  // get distance from beginning by specified point
  double getPassedDistance(const txPoint& specifiedPoint, txPoint& pedalPoint, const CoordType& ct = COORD_ENU) const;

  // calc the dis between begin to end
  void calcLastLength(CoordType ct);

  // calc length for geom
  // resize disFromBegin first
  void calcLength(CoordType ct);

  // calc yaw
  void calcYaw(CoordType ct);

  // calc normal
  void calcNormal(CoordType ct);

  // calc sl reference info
  void calcSLInfo(CoordType ct);

  // reset geom data
  void resetGeom(const PointVec& geom, CoordType ct);
};

void txLineCurve::CurveData::resetGeom(const PointVec& geom, CoordType ct) {
  setCoords(geom);
  calcLength(ct);
  calcYaw(ct);
  calcNormal(ct);
  calcSLInfo(ct);
}

void txLineCurve::CurveData::calcSLInfo(CoordType ct) {
  //  if ( ct == COORD_WGS84 ){
  //    return; }
  if (geom.size() <= 1) {
    return;
  } else {
    if (pointsNormal.empty()) calcNormal(ct);
    slRefInfo.resize(pointsNormal.size() - 1);
    for (size_t i = 0; i < slRefInfo.size(); ++i) {
      Point2d begin, end, intersectP;
      if (ct == COORD_WGS84) {
        txPoint pos(geom[i + 1].x + center.x, geom[i + 1].y + center.y, geom[i + 1].z + center.z);
        coord_trans_api::lonlat2enu(pos.x, pos.y, pos.z, geom[i].x + center.x, geom[i].y + center.y,
                                    geom[i].z + center.z);
        begin.x = 0.0;
        begin.y = 0.0;
        end.x = pos.x;
        end.y = pos.y;
      } else {
        begin.x = 0.0;
        begin.y = 0.0;
        end.x = geom[i + 1].x - geom[i].x;
        end.y = geom[i + 1].y - geom[i].y;
      }
      if (map_util::intersectPointNP(begin, pointsNormal[i], end, pointsNormal[i + 1], intersectP)) {
        int f = map_util::pointSide(begin, end, intersectP);

        /*if ( ct == COORD_WGS84 )
        {
                double z = 0.0;
                coord_trans_api::enu2lonlat( intersectP.x, intersectP.y, z, \
                        geom[i].x + center.x, geom[i].y + center.y, geom[i].z + center.z );
                coord_trans_api::lonlat2enu( intersectP.x, intersectP.y, z, \
                        center.x, center.y, center.z );
        }*/

        slRefInfo[i].x = intersectP.x;
        slRefInfo[i].y = intersectP.y;
        slRefInfo[i].z = f * 1.0;
      } else {
        slRefInfo[i].x = pointsNormal[i].x;
        slRefInfo[i].y = pointsNormal[i].y;
        slRefInfo[i].z = 0.0;
      }
    }
  }
}

void txLineCurve::CurveData::calcNormal(CoordType ct) {
  if (geom.size() <= 1) {
    return;
  } else {
    if (pointsYaw.empty()) calcYaw(ct);
    pointsNormal.resize(pointsYaw.size());
    for (size_t i = 0; i < pointsYaw.size(); ++i) {
      double normalYaw = pointsYaw[i] + 90;
      // map_util::yaw2dir( normalYaw, ct == COORD_ENU, pointsNormal[i] );
      map_util::yaw2dir(normalYaw, true, pointsNormal[i]);
    }
  }
}

void txLineCurve::CurveData::calcYaw(CoordType ct) {
  if (geom.size() <= 1) {
    return;
  } else {
    pointsYaw.resize(geom.size());
    for (size_t i = 0; i < geom.size(); ++i) {
      bool enu = ct != COORD_XYZ;
      Point2d dir;
      if (i == 0 || i == geom.size() - 1) {
        size_t bi = i == 0 ? i : i - 1;
        size_t ei = i == 0 ? i + 1 : i;
        Point2d b(geom[bi].x + center.x, geom[bi].y + center.y);
        Point2d e(geom[ei].x + center.x, geom[ei].y + center.y);
        if (ct == COORD_WGS84) {
          double z = 0.0;
          coord_trans_api::lonlat2enu(e.x, e.y, z, b.x, b.y, z);
          b.x = 0;
          b.y = 0;
        }
        dir.x = e.x - b.x;
        dir.y = e.y - b.y;

      } else {
        Point2d a(geom[i - 1].x + center.x, geom[i - 1].y + center.y);
        Point2d b(geom[i].x + center.x, geom[i].y + center.y);
        Point2d c(geom[i + 1].x + center.x, geom[i + 1].y + center.y);
        if (ct == COORD_WGS84) {
          double z = 0.0;
          coord_trans_api::lonlat2enu(a.x, a.y, z, b.x, b.y, z);
          coord_trans_api::lonlat2enu(c.x, c.y, z, b.x, b.y, z);
          b.x = 0;
          b.y = 0;
        }
        Point2d dirba(a.x - b.x, a.y - b.y);
        Point2d dirbc(c.x - b.x, c.y - b.y);
        map_util::normalize(dirba);
        map_util::normalize(dirbc);
        dir.x = dirbc.x - dirba.x;
        dir.y = dirbc.y - dirba.y;
      }
      pointsYaw[i] = map_util::dir2yaw(dir, enu);
    }
  }
}

void txLineCurve::CurveData::calcLastLength(CoordType ct) {
  if (geom.size() == 0) {
    return;
  } else if (geom.size() == 1) {
    disFromBegin.resize(1, 0.0);
  } else {
    size_t i = disFromBegin.size() - 1;
    double lastCoord[3] = {geom[i].x + center.x, geom[i].y + center.y, geom[i].z + center.z};
    if (COORD_WGS84 == ct) coord_trans_api::lonlat2global(lastCoord[0], lastCoord[1], lastCoord[2]);
    for (i += 1; i < geom.size(); ++i) {
      double curCoord[3] = {geom[i].x + center.x, geom[i].y + center.y, geom[i].z + center.z};
      if (COORD_WGS84 == ct) coord_trans_api::lonlat2global(curCoord[0], curCoord[1], curCoord[2]);
      disFromBegin.push_back(disFromBegin.back() + map_util::directlyDis(lastCoord, curCoord));
      memcpy(lastCoord, curCoord, sizeof(double) * 3);
    }
  }
}

void txLineCurve::CurveData::calcLength(CoordType ct) {
  if (geom.size() <= 1) {
    return;
  } else {
    disFromBegin.resize(geom.size(), 0.0);
    double lastCoord[3] = {geom[0].x + center.x, geom[0].y + center.y, geom[0].z + center.z};
    if (COORD_WGS84 == ct) coord_trans_api::lonlat2global(lastCoord[0], lastCoord[1], lastCoord[2]);
    for (size_t i = 1; i < geom.size(); ++i) {
      double curCoord[3] = {geom[i].x + center.x, geom[i].y + center.y, geom[i].z + center.z};
      if (COORD_WGS84 == ct) coord_trans_api::lonlat2global(curCoord[0], curCoord[1], curCoord[2]);
      disFromBegin[i] = disFromBegin[i - 1] + map_util::directlyDis(lastCoord, curCoord);
      memcpy(lastCoord, curCoord, sizeof(double) * 3);
    }
  }
}

void txLineCurve::CurveData::setCoords(const PointVec& points) {
  if (points.size() < 1) {
    geom.clear();
  } else {
    geom.clear();
    center = points.front();
    for (size_t i = 0; i < points.size(); ++i) {
      txPoint p;
      p.x = points[i].x - center.x;
      p.y = points[i].y - center.y;
      p.z = points[i].z - center.z;
      p.w = points[i].w;
      p.h = points[i].h;

      if (i > 0 && std::abs(p.x - geom.back().x) < 1e-9 && std::abs(p.y - geom.back().y) < 1e-9 &&
          std::abs(p.z - geom.back().z) < 1e-9) {
        continue;
      }
      geom.push_back(p);
    }
  }
}

double txLineCurve::CurveData::getLength() const {
  if (geom.size() <= 1) {
    return 0.0;
  } else {
    if (disFromBegin.empty()) {
      return 0.0;
    } else {
      return disFromBegin.back();
    }
  }
}

size_t txLineCurve::CurveData::size() const { return geom.size(); }

int txLineCurve::CurveData::getPointIndex(double distance) const {
  if (geom.empty()) {
    return -1;
  } else {
    for (size_t i = 1; i < geom.size(); ++i) {
      if (distance < disFromBegin[i]) {
        if (disFromBegin[i] - distance > distance - disFromBegin[i - 1]) {
          return static_cast<int>(i) - 1;
        } else {
          return static_cast<int>(i);
        }
      }
    }
    return static_cast<int>(geom.size()) - 1;
  }
}

txPoint txLineCurve::CurveData::getPoint(double distance, CoordType ct) const {
  if (geom.empty()) return txPoint();

  if (distance <= 0) {
    txPoint p(geom.front());
    p.x += center.x;
    p.y += center.y;
    p.z += center.z;
    return p;
  } else if (distance >= disFromBegin.back()) {
    txPoint p(geom.back());
    p.x += center.x;
    p.y += center.y;
    p.z += center.z;
    return p;
  } else {
    size_t index = 1;
    for (; index < disFromBegin.size(); ++index) {
      if (distance < disFromBegin[index]) break;
    }
    if (index >= geom.size()) return txPoint();
    distance -= disFromBegin[index - 1];

    txPoint curPoint(geom[index - 1]);
    curPoint.x += center.x;
    curPoint.y += center.y;
    curPoint.z += center.z;
    if (COORD_WGS84 == ct) coord_trans_api::lonlat2global(curPoint.x, curPoint.y, curPoint.z);
    txPoint nextPoint(geom[index]);
    nextPoint.x += center.x;
    nextPoint.y += center.y;
    nextPoint.z += center.z;
    if (COORD_WGS84 == ct) coord_trans_api::lonlat2global(nextPoint.x, nextPoint.y, nextPoint.z);

    double dir[] = {nextPoint.x - curPoint.x, nextPoint.y - curPoint.y, nextPoint.z - curPoint.z};
    map_util::normalize(dir);
    curPoint.x += dir[0] * distance;
    curPoint.y += dir[1] * distance;
    curPoint.z += dir[2] * distance;

    if (COORD_WGS84 == ct) coord_trans_api::global2lonlat(curPoint.x, curPoint.y, curPoint.z);
    return curPoint;
  }
}

txPoint txLineCurve::CurveData::getPoint(size_t index) const {
  if (geom.empty()) throw std::length_error("txLineCurve::getPoint geom is empty");

  if (index >= geom.size()) {
    txPoint p(geom.back());
    p.x += center.x;
    p.y += center.y;
    p.z += center.z;
    return p;
  } else {
    txPoint p(geom[index]);
    p.x += center.x;
    p.y += center.y;
    p.z += center.z;
    return p;
  }
}

double txLineCurve::CurveData::yaw(size_t index) const {
  if (geom.empty()) throw std::length_error("txLineCurve::yaw geom is empty");
  if (pointsYaw.empty()) throw std::length_error("txLineCurve::yaw yaw is empty");

  if (index >= geom.size()) index = geom.size() - 1;

  return pointsYaw[index];
}

void txLineCurve::CurveData::getPoints(PointVec& points) const {
  points.assign(geom.begin(), geom.end());
  for (size_t i = 0; i < points.size(); ++i) {
    points[i].x += center.x;
    points[i].y += center.y;
    points[i].z += center.z;
  }
}

void txLineCurve::CurveData::getGeometry(PointVec& g) const {
  g.assign(geom.begin(), geom.end());
  for (size_t i = 0; i < g.size(); ++i) {
    g[i].x += center.x;
    g[i].y += center.y;
    g[i].z += center.z;
  }
}

double txLineCurve::CurveData::getPassedDistance(const txPoint& specifiedPoint, txPoint& pedalPoint,
                                                 const CoordType& ct) const {
  if (geom.empty()) throw std::runtime_error("getPassedDistacne Error, geom emtpy");
  double minDis = DBL_MAX;
  double u;
  size_t minIndex = 0;
  Point3d specP(specifiedPoint.x, specifiedPoint.y, specifiedPoint.z);
  Point3d pedalP;
  for (size_t i = 1; i < geom.size(); ++i) {
    double tmpU = map_util::projectPoint2Line(
        specP, Point3d(geom[i - 1].x + center.x, geom[i - 1].y + center.y, geom[i - 1].z + center.z),
        Point3d(geom[i].x + center.x, geom[i].y + center.y, geom[i].z + center.z), COORD_WGS84 == ct, pedalP);
    double curDis = map_util::distanceBetweenPoints(specP, pedalP, COORD_WGS84 == ct);
    if (curDis < minDis) {
      minDis = curDis;
      minIndex = i;
      u = tmpU;
      pedalPoint.x = pedalP.x;
      pedalPoint.y = pedalP.y;
      pedalPoint.z = pedalP.z;
    }
  }

  if (minIndex > 0) {
    return disFromBegin[minIndex - 1] + u * (disFromBegin[minIndex] - disFromBegin[minIndex - 1]);
  } else {
    throw std::runtime_error("getPassedDistance Error");
  }
}

//
// txLineCurve
//
txLineCurve::txLineCurve(CoordType ct) : txCurve(ct), instancePtr(new CurveData) {}

txLineCurve::~txLineCurve() {}

bool txLineCurve::empty() const {
  if (instancePtr->geom.empty()) {
    return true;
  } else {
    return false;
  }
}

double txLineCurve::getLength() const { return instancePtr->getLength(); }

txPoint txLineCurve::getPoint(double distance) const { return instancePtr->getPoint(distance, coordType); }

int txLineCurve::getPointIndex(double distance) const { return instancePtr->getPointIndex(distance); }

txPoint txLineCurve::getStart() const { return instancePtr->getPoint(size_t(0)); }

txPoint txLineCurve::getEnd() const { return instancePtr->getPoint(instancePtr->size()); }

void txLineCurve::sampleV2(double angleLimit, double angleLimitLen, PointVec& points) const {
  points.clear();
  if (instancePtr->geom.empty()) return;

  angleLimit = angleLimit * 3.1415926 / 180;
  PointVec originP(instancePtr->geom.begin(), instancePtr->geom.end());
  for (size_t i = 0; i < originP.size(); ++i) {
    originP[i].x += instancePtr->center.x;
    originP[i].y += instancePtr->center.y;
    originP[i].z += instancePtr->center.z;
  }
  if (COORD_WGS84 == coordType) {
    for (size_t i = 0; i < originP.size(); ++i)
      coord_trans_api::lonlat2global(originP[i].x, originP[i].y, originP[i].z);
  }
  points.push_back(originP.front());

  size_t isize = originP.size();
  if (isize > 2) {
    for (size_t i = 0; i < isize - 1;) {
      txPoint Direction;
      Direction.x = originP[i + 1].x - originP[i].x;
      Direction.y = originP[i + 1].y - originP[i].y;
      Direction.z = originP[i + 1].z - originP[i].z;
      double dLen = std::sqrt(std::pow(Direction.x, 2) + std::pow(Direction.y, 2) + std::pow(Direction.z, 2));
      // double dLen = instancePtr->disFromBegin[i + 1] - instancePtr->disFromBegin[i];
      if (dLen == 0) continue;
      size_t j = i + 2;
      for (; j < isize; j++) {
        txPoint NextDirection;
        NextDirection.x = originP[j].x - originP[i].x;
        NextDirection.y = originP[j].y - originP[i].y;
        NextDirection.z = originP[j].z - originP[i].z;
        double dNextLen =
            std::sqrt(std::pow(NextDirection.x, 2) + std::pow(NextDirection.y, 2) + std::pow(NextDirection.z, 2));
        if (dNextLen == 0) continue;
        double dotMul = Direction.x * NextDirection.x + Direction.y * NextDirection.y + Direction.z * NextDirection.z;
        double angle = fabs(std::acos(dotMul / (dLen * dNextLen)));
        if (angle >= angleLimit || dNextLen * angle >= angleLimit * angleLimitLen) {
          if (angle >= 2 * angleLimit || dNextLen > angleLimitLen * 3) {
            points.push_back(originP[j - 1]);
          }

          if (j != isize - 1) {
            points.push_back(originP[j]);
            break;
          }
        }
      }
      i = j;
    }
  }

  points.push_back(originP.back());
  if (COORD_WGS84 == coordType) {
    for (size_t i = 0; i < points.size(); ++i) coord_trans_api::global2lonlat(points[i].x, points[i].y, points[i].z);
  }
}

void txLineCurve::sample(double interval, PointVec& points) const {
  if (interval < 1e-3) return;
  points.clear();
  if (instancePtr->geom.empty()) return;
  size_t newPointNum = size_t(instancePtr->getLength() / interval + 0.5) + 1;
  if (newPointNum < 2) newPointNum = 2;
  PointVec originP(instancePtr->geom.begin(), instancePtr->geom.end());
  for (size_t i = 0; i < originP.size(); ++i) {
    originP[i].x += instancePtr->center.x;
    originP[i].y += instancePtr->center.y;
    originP[i].z += instancePtr->center.z;
  }
  if (COORD_WGS84 == coordType) {
    for (size_t i = 0; i < originP.size(); ++i)
      coord_trans_api::lonlat2global(originP[i].x, originP[i].y, originP[i].z);
  }

  points.push_back(originP.front());

  interval = instancePtr->getLength() / (newPointNum - 1);
  for (size_t i = 1, disIndex = 0; i < newPointNum - 1; ++i) {
    double curDis = interval * i;

    if (fabs(curDis - instancePtr->disFromBegin[disIndex]) < 1e-5) {
      points.push_back(originP[disIndex]);
      disIndex += 1;
    } else if (curDis < instancePtr->disFromBegin[disIndex]) {
      curDis -= instancePtr->disFromBegin[disIndex - 1];
      double dir[] = {originP[disIndex].x - originP[disIndex - 1].x, originP[disIndex].y - originP[disIndex - 1].y,
                      originP[disIndex].z - originP[disIndex - 1].z};
      map_util::normalize(dir);
      dir[0] *= curDis;
      dir[1] *= curDis;
      dir[2] *= curDis;

      txPoint newP(originP[disIndex - 1].x + dir[0], originP[disIndex - 1].y + dir[1],
                   originP[disIndex - 1].z + dir[2]);

      points.push_back(newP);
    } else {
      i -= 1;
      disIndex += 1;
    }
  }

  points.push_back(originP.back());

  if (COORD_WGS84 == coordType) {
    for (size_t i = 0; i < points.size(); ++i) coord_trans_api::global2lonlat(points[i].x, points[i].y, points[i].z);
  }
}

void txLineCurve::sample(double interval, const double start_s, const double end_s, PointVec& points) const {
  if (interval < 1e-2) return;
  points.clear();
  if (instancePtr->geom.empty()) return;
  size_t newPointNum = size_t((end_s - start_s) / interval + 0.5) + 1;
  if (newPointNum < 2) newPointNum = 2;
  PointVec originP(instancePtr->geom.begin(), instancePtr->geom.end());
  for (size_t i = 0; i < originP.size(); ++i) {
    originP[i].x += instancePtr->center.x;
    originP[i].y += instancePtr->center.y;
    originP[i].z += instancePtr->center.z;
  }
  if (COORD_WGS84 == coordType) {
    for (size_t i = 0; i < originP.size(); ++i)
      coord_trans_api::lonlat2global(originP[i].x, originP[i].y, originP[i].z);
  }

  points.push_back(originP.front());

  interval = instancePtr->getLength() / (newPointNum - 1);
  for (size_t i = 1, disIndex = 0; i < newPointNum - 1; ++i) {
    double curDis = interval * i;

    if (fabs(curDis - instancePtr->disFromBegin[disIndex]) < 1e-5) {
      points.push_back(originP[disIndex]);
      disIndex += 1;
    } else if (curDis < instancePtr->disFromBegin[disIndex]) {
      curDis -= instancePtr->disFromBegin[disIndex - 1];
      double dir[] = {originP[disIndex].x - originP[disIndex - 1].x, originP[disIndex].y - originP[disIndex - 1].y,
                      originP[disIndex].z - originP[disIndex - 1].z};
      map_util::normalize(dir);
      dir[0] *= curDis;
      dir[1] *= curDis;
      dir[2] *= curDis;

      txPoint newP(originP[disIndex - 1].x + dir[0], originP[disIndex - 1].y + dir[1],
                   originP[disIndex - 1].z + dir[2]);

      points.push_back(newP);
    } else {
      i -= 1;
      disIndex += 1;
    }
  }

  points.push_back(originP.back());

  if (COORD_WGS84 == coordType) {
    for (size_t i = 0; i < points.size(); ++i) coord_trans_api::global2lonlat(points[i].x, points[i].y, points[i].z);
  }
}

double txLineCurve::getPassedDistance(const txPoint& specifiedPoint, txPoint& pedalPoint, const CoordType& ct) const {
  if (ct != coordType) {
    txPoint sp(specifiedPoint);
    if (COORD_WGS84 == ct) {
      coord_trans_api::lonlat2global(sp.x, sp.y, sp.z);
      double dis = instancePtr->getPassedDistance(sp, pedalPoint, coordType);
      coord_trans_api::global2lonlat(pedalPoint.x, pedalPoint.y, pedalPoint.z);
      return dis;
    } else if (COORD_XYZ == ct || COORD_ENU == ct) {
      coord_trans_api::global2lonlat(sp.x, sp.y, sp.z);
      double dis = instancePtr->getPassedDistance(sp, pedalPoint, coordType);
      coord_trans_api::lonlat2global(pedalPoint.x, pedalPoint.y, pedalPoint.z);
      return dis;
    } else {
      return 0.0;
    }
  } else {
    return instancePtr->getPassedDistance(specifiedPoint, pedalPoint, ct);
  }
}

size_t txLineCurve::size() const { return instancePtr->size(); }

txPoint txLineCurve::getPoint(size_t index) const { return instancePtr->getPoint(index); }

double txLineCurve::yaw(size_t index) const { return instancePtr->yaw(index); }

void txLineCurve::getPoints(PointVec& points) const { instancePtr->getPoints(points); }

void txLineCurve::addPoint(const txPoint& point, bool update) {
  if (instancePtr->geom.empty()) instancePtr->center = point;
  txPoint curP(point);
  curP.x -= instancePtr->center.x;
  curP.y -= instancePtr->center.y;
  curP.z -= instancePtr->center.z;
  instancePtr->geom.push_back(curP);
  instancePtr->calcLastLength(coordType);
  if (update) {
    instancePtr->calcYaw(coordType);
    instancePtr->calcNormal(coordType);
    instancePtr->calcSLInfo(coordType);
  }
}

void txLineCurve::setCoords(const PointVec& points) { instancePtr->resetGeom(points, coordType); }

void txLineCurve::clone(const txCurve& curveIns) {
  try {
    const txLineCurve& samplingCurveIns = dynamic_cast<const txLineCurve&>(curveIns);
    this->coordType = samplingCurveIns.getCoordType();
    PointVec tmpGeom;
    samplingCurveIns.instancePtr->getGeometry(tmpGeom);
    instancePtr->resetGeom(tmpGeom, this->coordType);
  } catch (const std::bad_cast&) {
  }
}

void txLineCurve::clone(const txCurve* curvePtr) {
  const txLineCurve* samplingCurvePtr = dynamic_cast<const txLineCurve*>(curvePtr);
  if (samplingCurvePtr != NULL) {
    this->coordType = samplingCurvePtr->getCoordType();
    PointVec tmpGeom;
    samplingCurvePtr->instancePtr->getGeometry(tmpGeom);
    instancePtr->resetGeom(tmpGeom, this->coordType);
  }
}

void txLineCurve::clone(const txLineCurve* lineCurvePtr) {
  if (lineCurvePtr != NULL) {
    this->coordType = lineCurvePtr->getCoordType();
    *instancePtr = *lineCurvePtr->instancePtr;
  }
}

void txLineCurve::splice(const txCurve* curvePtr) {
  const txLineCurve* samplingCurvePtr = dynamic_cast<const txLineCurve*>(curvePtr);
  if (samplingCurvePtr != NULL) {
    if (this->coordType != samplingCurvePtr->getCoordType()) {
      throw std::logic_error("txLineCurve::splice Coord Type Error");
    }
    PointVec tmpGeom;
    samplingCurvePtr->instancePtr->getGeometry(tmpGeom);
    PointVec newGeom(instancePtr->geom.begin(), instancePtr->geom.end());
    for (size_t i = 0; i < newGeom.size(); ++i) {
      newGeom[i].x += instancePtr->center.x;
      newGeom[i].y += instancePtr->center.y;
      newGeom[i].z += instancePtr->center.z;
    }
    newGeom.insert(newGeom.end(), tmpGeom.begin(), tmpGeom.end());
    instancePtr->resetGeom(newGeom, this->coordType);
  }
}

void txLineCurve::splice(const txCurve& curveIns) {
  try {
    const txLineCurve& samplingCurveIns = dynamic_cast<const txLineCurve&>(curveIns);
    if (this->coordType != samplingCurveIns.getCoordType()) {
      throw std::logic_error("txLineCurve::splice Coord Type Error");
    }
    PointVec tmpGeom;
    samplingCurveIns.instancePtr->getGeometry(tmpGeom);
    PointVec newGeom(instancePtr->geom.begin(), instancePtr->geom.end());
    for (size_t i = 0; i < newGeom.size(); ++i) {
      newGeom[i].x += instancePtr->center.x;
      newGeom[i].y += instancePtr->center.y;
      newGeom[i].z += instancePtr->center.z;
    }
    newGeom.insert(newGeom.end(), tmpGeom.begin(), tmpGeom.end());
    instancePtr->resetGeom(newGeom, this->coordType);
  } catch (const std::bad_cast&) {
  }
}

void txLineCurve::reverse() {
  if (instancePtr->geom.size() < 2) return;
  for (size_t i = 0; i < instancePtr->geom.size(); ++i) {
    instancePtr->geom[i].x += instancePtr->center.x;
    instancePtr->geom[i].y += instancePtr->center.y;
    instancePtr->geom[i].z += instancePtr->center.z;
  }
  instancePtr->center = instancePtr->geom.back();
  for (size_t i = 0, j = instancePtr->geom.size() - 1; i < j; ++i, --j)
    std::swap(instancePtr->geom[i], instancePtr->geom[j]);
  for (size_t i = 0; i < instancePtr->geom.size(); ++i) {
    instancePtr->geom[i].x -= instancePtr->center.x;
    instancePtr->geom[i].y -= instancePtr->center.y;
    instancePtr->geom[i].z -= instancePtr->center.z;
  }
  instancePtr->calcYaw(this->coordType);
  instancePtr->calcLength(this->coordType);
  instancePtr->calcNormal(this->coordType);
}

bool txLineCurve::transfer(const txPoint& oldEnuC, const txPoint& newEnuC) {
  if (coordType == COORD_WGS84 || coordType == COORD_XYZ) {
    return false;
  } else {
    /*
    instancePtr->center = instancePtr->oriBegin;
    coord_trans_api::lonlat2enu( instancePtr->center.x, instancePtr->center.y, instancePtr->center.z, \
            newEnuC.x, newEnuC.y, newEnuC.z );
    */
    coord_trans_api::enu2lonlat(instancePtr->center.x, instancePtr->center.y, instancePtr->center.z, oldEnuC.x,
                                oldEnuC.y, oldEnuC.z);
    coord_trans_api::lonlat2enu(instancePtr->center.x, instancePtr->center.y, instancePtr->center.z, newEnuC.x,
                                newEnuC.y, newEnuC.z);
    return true;
  }
}

bool txLineCurve::transfer2ENU(const txPoint& enuCenter) {
  if (coordType != COORD_WGS84) {
    return false;
  } else {
    coordType = COORD_ENU;
    txPoint newCenter(instancePtr->center);
    coord_trans_api::lonlat2enu(newCenter.x, newCenter.y, newCenter.z, enuCenter.x, enuCenter.y, enuCenter.z);
    for (size_t i = 0; i < instancePtr->geom.size(); ++i) {
      txPoint curP(instancePtr->geom[i]);
      curP.x += instancePtr->center.x;
      curP.y += instancePtr->center.y;
      curP.z += instancePtr->center.z;
      coord_trans_api::lonlat2enu(curP.x, curP.y, curP.z, enuCenter.x, enuCenter.y, enuCenter.z);
      curP.x -= newCenter.x;
      curP.y -= newCenter.y;
      curP.z -= newCenter.z;
      instancePtr->geom[i] = curP;
    }
    instancePtr->center = newCenter;

    instancePtr->calcNormal(COORD_ENU);
    instancePtr->calcSLInfo(COORD_ENU);
    return true;
  }
}

bool txLineCurve::xy2sl(const double& x, const double& y, double& s, double& l, double& yaw) const {
  if (instancePtr->geom.empty()) {
    return false;
  }

  Points2d localPoints(instancePtr->geom.size() + 2);
  Point2d localXY(x, y);
  for (size_t i = 1, j = 0; j < instancePtr->geom.size(); ++i, ++j) {
    localPoints[i].x = instancePtr->geom[j].x + instancePtr->center.x;
    localPoints[i].y = instancePtr->geom[j].y + instancePtr->center.y;
  }
  if (this->coordType == COORD_WGS84) {
    localXY.x = 0;
    localXY.y = 0;
    double z = 0.0;
    for (size_t i = 1; i < localPoints.size() - 1; ++i)
      coord_trans_api::lonlat2enu(localPoints[i].x, localPoints[i].y, z, x, y, z);
    // instancePtr->center.x, instancePtr->center.y, instancePtr->center.z );
  }

  // generate buffer coord
  Point2d frontDir(localPoints[1].x - localPoints[2].x, localPoints[1].y - localPoints[2].y);
  map_util::normalize(frontDir);
  localPoints[0].x = localPoints[1].x + 2 * SL_BUFFER * frontDir.x;
  localPoints[0].y = localPoints[1].y + 2 * SL_BUFFER * frontDir.y;

  Point2d backDir(localPoints[localPoints.size() - 2].x - localPoints[localPoints.size() - 3].x,
                  localPoints[localPoints.size() - 2].y - localPoints[localPoints.size() - 3].y);
  map_util::normalize(backDir);
  localPoints.back().x = localPoints[localPoints.size() - 2].x + 2 * SL_BUFFER * backDir.x;
  localPoints.back().y = localPoints[localPoints.size() - 2].y + 2 * SL_BUFFER * backDir.y;

  size_t nIndex;
  double nDis;
  if (map_util::nearestPoint2d(localPoints, localXY, false, nIndex, nDis)) {
    if (nIndex == 0 || nIndex == localPoints.size() - 1) return false;

    std::vector<size_t> index;
    if (nIndex > 0) {
      index.push_back(nIndex - 1);
      index.push_back(nIndex);
      if (nIndex < localPoints.size() - 1) index.push_back(nIndex + 1);
    } else {
      index.push_back(nIndex);
      index.push_back(nIndex + 1);
    }

    Points2d& normal = instancePtr->pointsNormal;

    for (size_t i = 0; i < index.size() - 1; ++i) {
      Point2d tagP;
      Point2d intersectP;
      Point2d& curP = localPoints[index[i]];
      Point2d& nexP = localPoints[index[i + 1]];
      if (index[i] == 0) {
        tagP.x = localXY.x + instancePtr->pointsNormal[0].x;
        tagP.y = localXY.y + instancePtr->pointsNormal[0].y;
      } else if (index[i] == instancePtr->geom.size()) {
        tagP.x = localXY.x + instancePtr->pointsNormal.back().x;
        tagP.y = localXY.y + instancePtr->pointsNormal.back().y;
      } else {
        Point3d& refInfo = instancePtr->slRefInfo[index[i] - 1];
        if (fabs(refInfo.z) > 1e-5) {
          Point2d& preP = localPoints[index[i]];
          tagP.x = refInfo.x + preP.x;
          tagP.y = refInfo.y + preP.y;
        } else {
          tagP.x = refInfo.x + localXY.x;
          tagP.y = refInfo.y + localXY.y;
        }
      }

      if (map_util::intersectPointDP(curP, nexP, localXY, tagP, intersectP)) {
        if (map_util::pointOnLine(curP, nexP, intersectP)) {
          if (index[i] == 0) {
            s = -map_util::directlyDis(nexP, intersectP);
          } else {
            s = instancePtr->disFromBegin[index[i] - 1] + map_util::directlyDis(curP, intersectP);
          }
          l = map_util::directlyDis(localXY, intersectP) * map_util::pointSide(curP, nexP, localXY);
          yaw = map_util::dir2yaw(Point2d(nexP.x - curP.x, nexP.y - curP.y),
                                  (coordType == COORD_ENU || coordType == COORD_WGS84));
          return true;
        }
      }
      /*
      Point2d fNormal, sNormal, intersectP;
      map_util::dirVector( localPoints, index[i], fNormal );
      map_util::dirVector( localPoints, index[i+1], sNormal );
      Point2d& curP = localPoints[index[i]];
      Point2d& nexP = localPoints[index[i+1]];
      if ( map_util::intersectPointNP( curP, fNormal, \
                      nexP, sNormal, intersectP ) )
      {
              Point2d realIntersectP;
              if ( map_util::intersectPointDP( curP, nexP, \
                              localXY, intersectP, realIntersectP ) )
              {
                      if ( map_util::pointOnLine( curP, nexP, \
                                      realIntersectP ) )
                      {
                              if ( index[i] == 0 ){
                                      s = - map_util::directlyDis( nexP, realIntersectP );}
                              else
                                      s = instancePtr->disFromBegin[ index[i] - 1 ] + \
                                                      map_util::directlyDis( curP, realIntersectP );
                              l = map_util::directlyDis( realIntersectP, localXY ) * \
                                              map_util::pointSide( curP, nexP, localXY );
                              yaw = map_util::dir2yaw( Point2d( nexP.x - curP.x, nexP.y - curP.y ), \
                                              coordType == COORD_ENU );
                              return true;
                      }
              }
      }
      else if ( map_util::intersectPointDP( curP, nexP, \
                              localXY, Point2d( localXY.x + fNormal.x, localXY.y + fNormal.y ), intersectP ) )
      {
              if ( map_util::pointOnLine( curP, nexP, intersectP ) )
              {
                      if ( index[i] == 0 ){
                              s = - map_util::directlyDis( nexP, intersectP );}
                      else
                              s = instancePtr->disFromBegin[ index[i] - 1 ] + \
                                              map_util::directlyDis( curP, intersectP );
                      l = map_util::directlyDis( intersectP, localXY ) * \
                                      map_util::pointSide( curP, nexP, localXY );
                      yaw = map_util::dir2yaw( Point2d( nexP.x - curP.x, nexP.y - curP.y ), \
                                      coordType == COORD_ENU );
                      return true;

              }
      }
      */
    }

    return false;
  } else {
    return false;
  }
}

// TODO(undefined): need to be fixed when coordtype == COORD_WGS84
bool txLineCurve::sl2xy(const double& s, const double& l, double& x, double& y, double& yaw) const {
  std::vector<double>& disFromBegin = instancePtr->disFromBegin;
  if (s < 0.0 || s > disFromBegin.back() + SL_BUFFER) return false;

  size_t tag = 0;
  if (s > disFromBegin[1]) {
    auto itr = std::lower_bound(disFromBegin.begin(), disFromBegin.end(), s);
    tag = itr - disFromBegin.begin() - 1;
  }
  /*
  size_t i = 0, j = disFromBegin.size() - 1;
  size_t tag = j + 1;
  while ( i < j )
  {
          size_t k = ( i + j ) >> 1;
          if ( s < instancePtr->disFromBegin[k] ){
                  j = k;}
          else if ( s > instancePtr->disFromBegin[k] ){
                  i = k;}
          else
          {
                  tag = k;
                  break;
          }
          if ( i + 1 == j ){
                  break;}
  }
  if ( tag  == instancePtr->disFromBegin.size() ){
          tag = i;}
  */

  if (this->coordType == COORD_WGS84) {
    Points2d localPoints(instancePtr->geom.size());
    for (size_t i = 0; i < instancePtr->geom.size(); ++i) {
      localPoints[i].x = instancePtr->geom[i].x + instancePtr->center.x;
      localPoints[i].y = instancePtr->geom[i].y + instancePtr->center.y;
    }
    double ref_x = instancePtr->geom[tag].x + instancePtr->center.x;
    double ref_y = instancePtr->geom[tag].y + instancePtr->center.y;
    double z = 0.0;
    for (size_t i = 0; i < localPoints.size(); ++i)
      coord_trans_api::lonlat2enu(localPoints[i].x, localPoints[i].y, z, ref_x, ref_y, 0);

    size_t front = tag == localPoints.size() - 1 ? tag - 1 : tag;
    size_t back = front + 1;
    Point2d dir(localPoints[back].x - localPoints[front].x, localPoints[back].y - localPoints[front].y);
    // map_util::normalize( dir );
    double offset = (s - disFromBegin[front]) / (disFromBegin[back] - disFromBegin[front]);
    Point2d tagP(localPoints[front].x + offset * dir.x, localPoints[front].y + offset * dir.y);

    yaw = instancePtr->pointsYaw[tag];
    Point2d curN;
    if (tag == localPoints.size() - 1) {
      curN.x = instancePtr->pointsNormal[tag].x;
      curN.y = instancePtr->pointsNormal[tag].y;
      if (l > 0) {
        curN.x *= -1;
        curN.y *= -1;
      }
    } else {
      Point2d& beginN = instancePtr->pointsNormal[tag];
      Point2d& endN = instancePtr->pointsNormal[tag + 1];
      Point2d intersectP;
      Point2d begin, end;
      begin.x = localPoints[tag].x;
      begin.y = localPoints[tag].y;
      end.x = localPoints[tag + 1].x;
      end.y = localPoints[tag + 1].y;

      if (!map_util::intersectPointNP(begin, beginN, end, endN, intersectP)) {
        intersectP.x = tagP.x + beginN.x;
        intersectP.y = tagP.y + beginN.y;
      }
      if (l * map_util::pointSide(begin, end, intersectP) > 0) {
        curN.x = intersectP.x - tagP.x;
        curN.y = intersectP.y - tagP.y;
      } else {
        curN.x = tagP.x - intersectP.x;
        curN.y = tagP.y - intersectP.y;
      }
    }

    map_util::normalize(curN);
    x = tagP.x + fabs(l) * curN.x;
    y = tagP.y + fabs(l) * curN.y;
    z = 0.0;
    coord_trans_api::enu2lonlat(x, y, z, ref_x, ref_y, 0);

    return true;

  } else {
    PointVec& geom = instancePtr->geom;
    size_t front = tag == geom.size() - 1 ? tag - 1 : tag;
    size_t back = front + 1;
    Point2d dir(geom[back].x - geom[front].x, geom[back].y - geom[front].y);
    // map_util::normalize( dir );
    double offset = (s - disFromBegin[front]) / (disFromBegin[back] - disFromBegin[front]);
    Point2d tagP(geom[front].x + offset * dir.x, geom[front].y + offset * dir.y);

    // yaw = map_util::dir2yaw( Point2d( instancePtr->geom[tag+1].x - instancePtr->geom[tag].x, \
      //      instancePtr->geom[tag+1].y - instancePtr->geom[tag].y ), coordType == COORD_ENU );
    yaw = instancePtr->pointsYaw[tag];

    Point3d refInfo;
    if (tag < geom.size() - 1) {
      refInfo = instancePtr->slRefInfo[tag];
      // refInfo.x += geom[tag].x;
      // refInfo.y += geom[tag].y;
    } else {
      refInfo.x = instancePtr->pointsNormal[tag].x;
      refInfo.y = instancePtr->pointsNormal[tag].y;
      refInfo.z = 0.0;
    }

    if (fabs(refInfo.z) > 1e-5) {
      Point2d curN(refInfo.x - tagP.x, refInfo.y - tagP.y);
      map_util::normalize(curN);

      x = tagP.x + l * refInfo.z * curN.x;
      y = tagP.y + l * refInfo.z * curN.y;
    } else {
      x = tagP.x + l * refInfo.x;
      y = tagP.y + l * refInfo.y;
    }

    /*
    Point2d& beginN = instancePtr->pointsNormal[tag];
    Point2d& endN = instancePtr->pointsNormal[tag+1];

    Point2d intersectP;
    Point2d begin, end;
    begin.x = geom[tag].x;
    begin.y = geom[tag].y;
    end.x = geom[tag+1].x;
    end.y = geom[tag+1].y;

    if ( s < instancePtr->disFromBegin.back() )
    {
            if ( !map_util::intersectPointNP( begin, beginN, \
                            end, endN, intersectP ) )
            {
                    intersectP.x = tagP.x + beginN.x;
                    intersectP.y = tagP.y + beginN.y;
            }
    }
    else
    {
            intersectP.x = tagP.x + endN.x;
            intersectP.y = tagP.y + endN.y;
    }
    Point2d curN;
    if ( l * map_util::pointSide( begin, end, intersectP ) > 0 )
    {
            curN.x = intersectP.x - tagP.x;
            curN.y = intersectP.y - tagP.y;
    }
    else
    {
            curN.x = tagP.x - intersectP.x;
            curN.y = tagP.y - intersectP.y;
    }
    map_util::normalize( curN );
    x = tagP.x + fabs(l) * curN.x;
    y = tagP.y + fabs(l) * curN.y;
    */

    x += instancePtr->center.x;
    y += instancePtr->center.y;

    return true;
  }
}

double txLineCurve::getYaw(const double& s) const {
  double ds = std::min(std::max(s, 0.1), instancePtr->disFromBegin.back() - 0.1);
  size_t tag = 0;
  for (size_t i = 0; i < instancePtr->disFromBegin.size() - 1; ++i) {
    if (instancePtr->disFromBegin[i] < ds && ds < instancePtr->disFromBegin[i + 1]) {
      tag = i;
      break;
    }
  }
  return instancePtr->pointsYaw[tag];
  /*
      if ( s < 0.0 ){
              return -1.0;}
      size_t tag = instancePtr->disFromBegin.size() - 1;
      for ( size_t i = 0; i < instancePtr->disFromBegin.size() - 1; ++ i )
      {
              if ( instancePtr->disFromBegin[i] < s && \
                              s < instancePtr->disFromBegin[i+1] )
              {
                      tag = i;
                      break;
              }
      }
      if ( tag == instancePtr->disFromBegin.size() - 1 ){
              return -1.0;}

      Points2d localPoints( instancePtr->geom.size() );
      localPoints[0].x = 0.0;
      localPoints[0].y = 0.0;
      for ( size_t i = 1; i < instancePtr->geom.size(); ++ i )
      {
              localPoints[i].x = instancePtr->geom[i].x + instancePtr->center.x;
              localPoints[i].y = instancePtr->geom[i].y + instancePtr->center.y;
      }
      if ( this->coordType == COORD_WGS84 )
      {
              double z = 0.0;
              for ( size_t i = 1; i < localPoints.size(); ++ i )
                      coord_trans_api::lonlat2local( localPoints[i].x, localPoints[i].y, z, \
                              instancePtr->center.x, instancePtr->center.y, instancePtr->center.z );
      }

      Point2d dir( localPoints[tag+1].x - localPoints[tag].x, localPoints[tag+1].y - localPoints[tag].y );
      map_util::normalize( dir );

      return  map_util::dir2yaw( Point2d( localPoints[tag+1].x - localPoints[tag].x, \
                              localPoints[tag+1].y - localPoints[tag].y ), coordType == COORD_ENU );
      */
}

bool txLineCurve::cut(const double& begin, const double& end, PointVec& points) const {
  if (begin > -1e-7 && end > -1e-7 && begin > end) return false;
  if (begin > instancePtr->disFromBegin.back()) return false;

  const double CREATE_INTERVAL_T = 0.2;
  size_t start_index = 0, end_index = instancePtr->geom.size() - 1;
  txPoint* start_p = NULL;
  txPoint* end_p = NULL;

  if (begin < 1e-1) {
    start_index = 0;
  } else {
    for (size_t i = 1; i < instancePtr->disFromBegin.size(); ++i) {
      if (begin <= instancePtr->disFromBegin[i]) {
        start_index = i;
        break;
      }
    }
    if (fabs(begin - instancePtr->disFromBegin[start_index]) < CREATE_INTERVAL_T) {
      start_p = NULL;
    } else if (fabs(begin - instancePtr->disFromBegin[start_index - 1]) < CREATE_INTERVAL_T) {
      start_index -= 1;
    } else {
      double offset = begin - instancePtr->disFromBegin[start_index - 1];
      start_p = new txPoint(genIntervalP(start_index - 1, start_index, offset));
    }
  }

  if (end < 1e-1 || end >= instancePtr->disFromBegin.back()) {
    end_index = instancePtr->geom.size() - 1;
  } else {
    for (size_t i = 1; i < instancePtr->disFromBegin.size(); ++i) {
      if (end < instancePtr->disFromBegin[i]) {
        end_index = i - 1;
        break;
      }
    }
    if (fabs(end - instancePtr->disFromBegin[end_index]) < CREATE_INTERVAL_T) {
      end_p = NULL;
    } else if (fabs(end - instancePtr->disFromBegin[end_index + 1]) < CREATE_INTERVAL_T) {
      end_index += 1;
    } else {
      double offset = end - instancePtr->disFromBegin[end_index];
      end_p = new txPoint(genIntervalP(end_index, end_index + 1, offset));
    }
  }

  points.clear();
  if (end_index - start_index + 1 == instancePtr->geom.size()) {
    points.assign(instancePtr->geom.begin(), instancePtr->geom.end());
  } else {
    if (start_p != NULL) {
      points.push_back(txPoint(*start_p));
      delete start_p;
    }
    points.insert(points.end(), instancePtr->geom.begin() + start_index, instancePtr->geom.begin() + end_index + 1);
    if (end_p != NULL) {
      points.push_back(txPoint(*end_p));
      delete end_p;
    }
  }

  for (auto& p : points) {
    p.x += instancePtr->center.x;
    p.y += instancePtr->center.y;
    p.z += instancePtr->center.z;
  }
  return true;
}

txPoint txLineCurve::genIntervalP(const size_t& start, const size_t& end, const double& offset) const {
  if (start + 1 != end) return txPoint(-1.0, -1.0, -1.0);
  double yaw = instancePtr->pointsYaw[start];
  Point2d dir;
  map_util::yaw2dir(yaw, true, dir);
  txPoint tagP(instancePtr->geom[start]);
  if (this->coordType == COORD_WGS84) tagP = txPoint(0.0, 0.0, 0.0);
  tagP.x += dir.x * offset;
  tagP.y += dir.y * offset;
  if (this->coordType == COORD_WGS84) {
    txPoint c(instancePtr->geom[start]);
    c.x += instancePtr->center.x;
    c.y += instancePtr->center.y;
    c.z += instancePtr->center.z;
    coord_trans_api::enu2lonlat(tagP.x, tagP.y, tagP.z, c.x, c.y, c.z);
    tagP.x -= instancePtr->center.x;
    tagP.y -= instancePtr->center.y;
    tagP.z -= instancePtr->center.z;
  }
  return tagP;
}
}  // namespace hadmap
