// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "common/map_util.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <limits>
#include <vector>
#include "common/coord_trans.h"

#include <iomanip>
#include <iostream>

using namespace hadmap;
using namespace coord_trans_api;

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

namespace map_util {
// Calculate the Euclidean distance between two points in 3D space
double directlyDis(double a[], double b[], size_t len) {
  double seqDis = 0.0;
  for (size_t i = 0; i < len; ++i) seqDis += std::pow(a[i] - b[i], 2);
  return std::sqrt(seqDis);
}

// Calculate the squared Euclidean distance between two points in 3D space
double directlySeqDis(double a[], double b[], size_t len) {
  double seqDis = 0.0;
  for (size_t i = 0; i < len; ++i) seqDis += std::pow(a[i] - b[i], 2);
  return seqDis;
}

// Calculate the Euclidean distance between two points in 2D space
double directlyDis(const Point2d& a, const Point2d& b) {
  double seqDis = std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2);
  return std::sqrt(seqDis);
}

// Calculate the squared Euclidean distance between two points in 2D space
double directlySeqDis(const Point2d& a, const Point2d& b) {
  double seqDis = std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2);
  return seqDis;
}

// Calculate the distance between two points in 3D space, considering the WGS84 coordinate system
double distanceBetweenPoints(const Point3d& p0, const Point3d& p1, const bool& WGS84) {
  double x1, y1, z1, x2, y2, z2;
  x1 = p0.x;
  y1 = p0.y;
  z1 = p0.z;
  x2 = p1.x;
  y2 = p1.y;
  z2 = p1.z;
  if (WGS84) {
    lonlat2global(x1, y1, z1);
    lonlat2global(x2, y2, z2);
  }
  return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
}

// Normalize a vector in 3D space
void normalize(double v[], size_t len) {
  double* b = new double[len];
  memset(b, 0, sizeof(double) * len);
  double length = directlyDis(v, b, len);
  delete[] b;
  for (size_t i = 0; i < len; ++i) v[i] /= length;
}

// Normalize a vector in 2D space
void normalize(Point2d& n) {
  double v[] = {n.x, n.y};
  normalize(v, 2);
  n.x = v[0];
  n.y = v[1];
}

// Calculate the direction vector of a point on a curve
void dirVector(const hadmap::Points2d& points, size_t index, hadmap::Point2d& dirVec) {
  if (index > points.size()) return;
  if (index == 0) {
    dirVec.x = -(points[index + 1].y - points[index].y);
    dirVec.y = points[index + 1].x - points[index].x;
  } else if (index == points.size() - 1) {
    dirVec.x = -(points[index].y - points[index - 1].y);
    dirVec.y = points[index].x - points[index - 1].x;
  } else {
    double fN[] = {points[index - 1].x - points[index].x, points[index - 1].y - points[index].y};
    double bN[] = {points[index + 1].x - points[index].x, points[index + 1].y - points[index].y};
    normalize(fN, 2);
    normalize(bN, 2);
    dirVec.x = fN[0] + bN[0];
    dirVec.y = fN[1] + bN[1];
  }
}

// Find the nearest point on a curve to a given point in 3D space
bool nearestPoint(const Points3d& curve, const Point3d& loc, bool WGS84, size_t& nearestIndex, double& distance) {
  size_t curvePointSize = curve.size();
  distance = std::numeric_limits<double>::max();
  nearestIndex = 0;

  double oriLoc[] = {loc.x, loc.y, loc.z};
  if (WGS84) lonlat2global(oriLoc[0], oriLoc[1], oriLoc[2]);

  for (size_t i = 0; i < curvePointSize; ++i) {
    double a[] = {curve[i].x, curve[i].y, curve[i].z};
    if (WGS84) lonlat2global(a[0], a[1], a[2]);
    double curDis = directlySeqDis(a, oriLoc);
    if (curDis < distance) {
      distance = curDis;
      nearestIndex = i;
    }
  }
  distance = std::sqrt(distance);
  return true;
}

// Find the nearest point on a curve to a given point in 2D space
bool nearestPoint2d(const Points2d& curve, const Point2d& loc, bool WGS84, size_t& nearestIndex, double& distance) {
  size_t curvePointSize = curve.size();
  distance = std::numeric_limits<double>::max();
  nearestIndex = 0;

  double oriLoc[] = {loc.x, loc.y, 0.0};
  if (WGS84) lonlat2global(oriLoc[0], oriLoc[1], oriLoc[2]);

  for (size_t i = 0; i < curvePointSize; ++i) {
    double a[] = {curve[i].x, curve[i].y, 0.0};
    if (WGS84) lonlat2global(a[0], a[1], a[2]);
    double curDis = directlySeqDis(a, oriLoc);
    if (curDis < distance) {
      distance = curDis;
      nearestIndex = i;
    }
  }
  distance = std::sqrt(distance);
  return true;
}

// Calculate the intersection point of two line segments in 2D space
bool intersectPointDP(const hadmap::Point2d& firstP0, const hadmap::Point2d& firstP1, const hadmap::Point2d& secondP0,
                      const hadmap::Point2d& secondP1, hadmap::Point2d& intersectP) {
  double a0 = firstP0.y - firstP1.y;
  double b0 = firstP1.x - firstP0.x;
  double c0 = firstP0.x * firstP1.y - firstP1.x * firstP0.y;

  double a1 = secondP0.y - secondP1.y;
  double b1 = secondP1.x - secondP0.x;
  double c1 = secondP0.x * secondP1.y - secondP1.x * secondP0.y;

  double D = a0 * b1 - a1 * b0;
  if (fabs(D) < 1e-5) {
    return false;
  } else {
    intersectP.x = (b0 * c1 - b1 * c0) / D;
    intersectP.y = (a1 * c0 - a0 * c1) / D;
    return true;
  }
}

// Calculate the intersection point of two line segments in 2D space, using direction vectors
bool intersectPointNP(const hadmap::Point2d& firstP, const hadmap::Point2d& firstN, const hadmap::Point2d& secondP,
                      const hadmap::Point2d& secondN, hadmap::Point2d& intersectP) {
  return intersectPointDP(firstP, Point2d(firstP.x + firstN.x, firstP.y + firstN.y), secondP,
                          Point2d(secondP.x + secondN.x, secondP.y + secondN.y), intersectP);
}

// Calculate the distance from a point to a line segment in 3D space
double calcPoint2LinesegDis(const Point3d& point, const Point3d& segBegin, const Point3d& segEnd, const bool& WGS84) {
  Point3d projP;
  double t = projectPoint2Line(point, segBegin, segEnd, WGS84, projP);
  t = t > 1.0 ? 1.0 : (t < 0.0 ? 0.0 : t);
  double a[] = {point.x, point.y, point.z};
  double b[] = {projP.x, projP.y, projP.z};
  if (WGS84) {
    lonlat2global(a[0], a[1], a[2]);
    lonlat2global(b[0], b[1], b[2]);
  }
  return directlyDis(a, b);
}

double calcPoint2PolylineDis(const Point3d& point, const Points3d& polyline, const bool& WGS84, const bool& countZ) {
  size_t size = polyline.size();
  double dis = DBL_MAX;
  for (size_t i = 0; i < size - 1; ++i) {
    double curDis = DBL_MAX;
    if (countZ) {
      curDis = calcPoint2LinesegDis(point, polyline[i], polyline[i + 1], WGS84);
    } else {
      Point3d pNZ(point.x, point.y, 0.0);
      Point3d polyI(polyline[i].x, polyline[i].y, 0.0);
      Point3d polyI1(polyline[i + 1].x, polyline[i + 1].y, 0.0);
      curDis = calcPoint2LinesegDis(pNZ, polyI, polyI1, WGS84);
    }
    if (curDis < dis) dis = curDis;
  }
  return dis;
}

double calcLaneWidth(const Points3d& leftBoundaryGeom, const Points3d& rightBoundaryGeom, const bool& WGS84) {
  Points3d leftGeomGlobal, rightGeomGlobal;
  for (size_t i = 0; i < leftBoundaryGeom.size(); ++i) {
    double loc[3] = {leftBoundaryGeom[i].x, leftBoundaryGeom[i].y, leftBoundaryGeom[i].z};
    if (WGS84) lonlat2global(loc[0], loc[1], loc[2]);
    leftGeomGlobal.push_back(Point3d(loc[0], loc[1], loc[2]));
  }
  for (size_t i = 0; i < rightBoundaryGeom.size(); ++i) {
    double loc[3] = {rightBoundaryGeom[i].x, rightBoundaryGeom[i].y, rightBoundaryGeom[i].z};
    if (WGS84) lonlat2global(loc[0], loc[1], loc[2]);
    rightGeomGlobal.push_back(Point3d(loc[0], loc[1], loc[2]));
  }
  std::vector<double> dis;
  for (size_t i = 0; i < leftGeomGlobal.size(); ++i) {
    dis.push_back(calcPoint2PolylineDis(leftGeomGlobal[i], rightGeomGlobal, false, true));
  }

  std::sort(dis.begin(), dis.end());
  double r = 0.0;
  size_t c = 0;
  double m = dis[dis.size() / 2];
  for (size_t i = 0; i < dis.size(); ++i) {
    if (dis[i] - m < 0.5 || m - dis[i] < 0.5) {
      r += dis[i];
      c += 1;
    }
  }
  if (c == 0) {
    return 0.0;
  } else {
    return r / c;
  }
}

double projectPoint2Line(const Point3d& p, const Point3d& start, const Point3d& end, const bool& WGS84,
                         Point3d& projectP) {
  double x1, y1, z1, x2, y2, z2, x3, y3, z3;
  x1 = start.x;
  y1 = start.y;
  z1 = start.z;
  x2 = end.x;
  y2 = end.y;
  z2 = end.z;
  x3 = p.x;
  y3 = p.y;
  z3 = p.z;
  if (WGS84) {
    lonlat2global(x1, y1, z1);
    lonlat2global(x2, y2, z2);
    lonlat2global(x3, y3, z3);
  }

  double seX, seY, seZ, seL2;
  seX = x2 - x1;
  seY = y2 - y1;
  seZ = z2 - z1;
  seL2 = seX * seX + seY * seY + seZ * seZ;
  double u, ru;
  ru = 0;
  if (seL2 < 1e-7) {
    u = 0;
  } else {
    u = ((x3 - x1) * seX + (y3 - y1) * seY + (z3 - z1) * seZ) / seL2;
    ru = u;
    if (u > 1.0) {
      u = 1.0;
    } else if (u < 0.0) {
      u = 0.0;
    }
  }
  projectP.x = x1 + u * seX;
  projectP.y = y1 + u * seY;
  projectP.z = z1 + u * seZ;
  if (WGS84) global2lonlat(projectP.x, projectP.y, projectP.z);
  return ru;
}

double projectPoint2Line(const hadmap::Point2d& p, const Point2d& start, const hadmap::Point2d& end, const bool& WGS84,
                         Point2d& projectP) {
  Point3d p3d(p.x, p.y, 0.0);
  Point3d start3d(start.x, start.y, 0.0);
  Point3d end3d(end.x, end.y, 0.0);
  Point3d projectp3d;
  double u = projectPoint2Line(p3d, start3d, end3d, WGS84, projectp3d);
  projectP.x = projectp3d.x;
  projectP.y = projectp3d.y;
  return u;
}

static bool cmp(const hadmap::tx_lane_t& l0, const hadmap::tx_lane_t& l1) {
  if (l0.road_pkid == l1.road_pkid) {
    if (l0.section_id == l1.section_id) {
      return l0.lane_id > l1.lane_id;
    } else {
      return l0.section_id < l1.section_id;
    }
  } else {
    return l0.road_pkid < l1.road_pkid;
  }
}

void sortLaneData(hadmap::tx_lane_vec& lanes) { std::sort(lanes.begin(), lanes.end(), cmp); }

bool pointOnLine(const Point2d& begin, const Point2d& end, const Point2d& intersectP) {
  double dx = std::abs(begin.x - end.x) * 1e-4;
  double dy = std::abs(begin.y - end.y) * 1e-4;
  if ((intersectP.x > fmin(begin.x, end.x) - dx && intersectP.x < fmax(begin.x, end.x) + dx) ||
      (intersectP.y > fmin(begin.y, end.y) - dy && intersectP.y < fmax(begin.y, end.y) + dy)) {
    return true;
  } else {
    return false;
  }
}

int pointSide(const Point2d& begin, const Point2d& end, const Point2d& intersectP) {
  Point2d dir(end.x - begin.x, end.y - begin.y);
  Point2d pV(intersectP.x - begin.x, intersectP.y - begin.y);
  double z = pV.x * dir.y - pV.y * dir.x;
  if (z > 0) {
    return -1;
  } else {
    return 1;
  }
}

double dir2yaw(const Point2d& dir, bool ENU) {
  double yaw = atan2(dir.y, dir.x);
  if (!ENU) {
    yaw += M_PI * 0.5;
  }
  yaw *= 180.0 / M_PI;
  if (yaw < 0) {
    yaw += 360;
  }
  return yaw;
}

void yaw2dir(double yaw, bool ENU, Point2d& dir) {
  if (!ENU) {
    yaw -= 90;
  }
  yaw *= M_PI / 180;
  dir.x = cos(yaw);
  dir.y = sin(yaw);
}

double angle(const Point2d& v1, const Point2d& v2) {
  Point2d nv1 = v1;
  Point2d nv2 = v2;
  normalize(nv1);
  normalize(nv2);

  double arccos = nv1.x * nv2.x + nv1.y * nv2.y;
  return acos(arccos) * 180.0 / M_PI;
}

double angle2radian(double a) {
  if (a > 180.0) a -= 360;
  return a * M_PI / 180.0;
}

double radian2angle(double r) {
  r *= 180.0 / M_PI;
  if (r < 0) r += 360;
  return r;
}

DIR_TYPE turnDir(const double& entranceYaw, const double& exitYaw) {
  double yawOffset = fmod(exitYaw - entranceYaw + 720.0, 360.0);
  if (yawOffset > 180.0) {
    yawOffset -= 360;
  }
  if (fabs(yawOffset) < 30.0) {
    return DIR_Straight;
  } else if (fabs(yawOffset) > 135.0) {
    return DIR_Uturn;
  } else if (yawOffset > 0) {
    return DIR_Left;
  } else {
    return DIR_Right;
  }
}

bool posInPolygon(const Point2d& pos, const Points2d& polygon, const bool& wgs84) {
  double total = 0.0;
  if (wgs84) {
    for (size_t i = 0; i < polygon.size(); ++i) {
      size_t j = i + 1 == polygon.size() ? 0 : i + 1;
      Point2d a(polygon[i].x, polygon[i].y);
      Point2d b(polygon[j].x, polygon[j].y);
      double z = 0.0;
      lonlat2enu(a.x, a.y, z, pos.x, pos.y, 0.0);
      lonlat2enu(b.x, b.y, z, pos.x, pos.y, 0.0);
      total += angle(a, b);
    }
  } else {
    for (size_t i = 0; i < polygon.size(); ++i) {
      size_t j = i + 1 == polygon.size() ? 0 : i + 1;
      Point2d a(polygon[i].x - pos.x, polygon[i].y - pos.y);
      Point2d b(polygon[j].x - pos.x, polygon[j].y - pos.y);
      total += angle(a, b);
    }
  }
  return fabs(total - 360.0) < 1.0;
}
int pointToRectDistance(const Point2d& v1, const Point2d& leftBottom, const Point2d& rightTop) {
  if (v1.x >= leftBottom.x && v1.y >= leftBottom.y && v1.x <= rightTop.x && v1.y <= rightTop.y) {
    return 0;
  } else if (v1.x < leftBottom.x) {
    if (v1.y > rightTop.y) {
      return directlyDis(v1, Point2d(leftBottom.x, rightTop.y));
    } else if (v1.y < leftBottom.y) {
      return directlyDis(v1, leftBottom);
    } else {
      return leftBottom.x - v1.x;
    }
  } else if (v1.x > rightTop.x) {
    if (v1.y > rightTop.y) {
      return directlyDis(v1, rightTop);
    } else if (v1.y < leftBottom.y) {
      return directlyDis(v1, Point2d(rightTop.x, leftBottom.y));
    } else {
      return v1.x - leftBottom.x;
    }
  } else if (v1.y > rightTop.y) {
    return v1.y - rightTop.y;
  } else if (v1.y < leftBottom.y) {
    return leftBottom.y - v1.y;
  }
}

int isPointInRect(const hadmap::Point2d& v1, const hadmap::Point2d& leftBottom, const hadmap::Point2d& rightTop) {
  if (v1.x >= leftBottom.x && v1.y >= leftBottom.y && v1.x <= rightTop.x && v1.y <= rightTop.y) {
    return 1;
  }
  return 0;
}
bool IsInRect(const hadmap::Point2d& point, const hadmap::Point2d& A, const hadmap::Point2d& B,
              const hadmap::Point2d& C, const hadmap::Point2d& D) {
  // 四边形内的点都在顺时针（逆时针）向量的同一边，即夹角小于90，向量积同向
  float x = point.x;
  float y = point.y;

  float a = (B.x - A.x) * (y - A.y) - (B.y - A.y) * (x - A.x);
  float b = (C.x - B.x) * (y - B.y) - (C.y - B.y) * (x - B.x);
  float c = (D.x - C.x) * (y - C.y) - (D.y - C.y) * (x - C.x);
  float d = (A.x - D.x) * (y - D.y) - (A.y - D.y) * (x - D.x);
  if ((a > 0 && b > 0 && c > 0 && d > 0) || (a < 0 && b < 0 && c < 0 && d < 0)) {
    return true;
  }
  return false;
}

double calcCircumcircleRadius(hadmap::Point2d A, hadmap::Point2d B, hadmap::Point2d C, const bool& WGS84) {
  constexpr double eps = 1E-6;

  if (WGS84) {
    double z = 0.0;
    coord_trans_api::lonlat2enu(B.x, B.y, z, A.x, A.y, z);
    coord_trans_api::lonlat2enu(C.x, C.y, z, A.x, A.y, z);
    A.x = 0.0;
    A.y = 0.0;
  }

  double yaw_ab = dir2yaw(hadmap::Point2d(B.x, B.y), true);
  double yaw_ac = dir2yaw(hadmap::Point2d(C.x, C.y), true);
  double radian_a = std::fabs(angle2radian(yaw_ab) - angle2radian(yaw_ac));

  if (std::fabs(radian_a) < eps) {
    return INFINITY;
  }

  if (radian_a > M_PI) {
    radian_a = 2 * M_PI - radian_a;
  }
  return directlyDis(B, C) / (2 * std::sin(radian_a / 2) + eps);
}
}  // namespace map_util
