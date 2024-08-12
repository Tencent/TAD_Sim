#include "c_terrain_math.h"

namespace terrain {
bool CTerrainMath::isInside(const SGridPolygon &polygon, const CGridPt2d &p) {
  real_t minX = polygon[0].x;
  real_t maxX = polygon[0].x;
  real_t minY = polygon[0].y;
  real_t maxY = polygon[0].y;
  for (int i = 1; i < polygon.size(); i++) {
    CGridPt2d q = polygon[i];
    minX = min(q.x, minX);
    maxX = max(q.x, maxX);
    minY = min(q.y, minY);
    maxY = max(q.y, maxY);
  }

  if (p.x < minX || p.x > maxX || p.y < minY || p.y > maxY) {
    return false;
  }

  bool inside = false;
  for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
    if ((polygon[i].y > p.y) != (polygon[j].y > p.y) &&
        p.x < (polygon[j].x - polygon[i].x) * (p.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x) {
      inside = !inside;
    }
  }

  return inside;
}

void CTerrainMath::testInside() {
  CGridPt2d p1 = {2, 2}, p2 = {4, 4}, p3 = {4, 0};
  CGridPt2d q1 = {1, 2}, q2 = {3, 1}, q3 = {3, 2}, q4 = {4, 5}, qo = {0, 0};

  SGridPolygon polygon;

  // SGridPoint2d ptFar = { INFINITE_POSITIVE,q3.y };

  polygon.push_back(p1);
  polygon.push_back(p2);
  polygon.push_back(p3);

  if (CTerrainMath::isInside(polygon, q1))
    LOG(INFO) << "vd | inside polygon.\n";
  else
    LOG(INFO) << "vd | not inside polygon.\n";

  if (CTerrainMath::isInside(polygon, q2))
    LOG(INFO) << "vd | inside polygon.\n";
  else
    LOG(INFO) << "vd | not inside polygon.\n";

  if (CTerrainMath::isInside(polygon, q3))
    LOG(INFO) << "vd | inside polygon.\n";
  else
    LOG(INFO) << "vd | not inside polygon.\n";

  if (CTerrainMath::isInside(polygon, q4))
    LOG(INFO) << "vd | inside polygon.\n";
  else
    LOG(INFO) << "vd | not inside polygon.\n";

  if (CTerrainMath::isInside(polygon, p1))
    LOG(INFO) << "vd | inside polygon.\n";
  else
    LOG(INFO) << "vd | not inside polygon.\n";

  if (CTerrainMath::isInside(polygon, qo))
    LOG(INFO) << "vd | inside polygon.\n";
  else
    LOG(INFO) << "vd | not inside polygon.\n";
}

// vector from p1 to p2
CGridPt2dVec CTerrainMath::getVectorFromPoint(const CGridPt2d &p1, const CGridPt2d &p2) {
  CGridPt2dVec vec;

  vec.x = p2.x - p1.x;
  vec.y = p2.y - p1.y;

  return vec;
}

// get clockwise angle from vector 1 to vector 2, unit rad, range [0-2*pi]
real_t CTerrainMath::getClockwiseAngle(const CGridPt2dVec &v1, const CGridPt2dVec &v2) {
  // origin point
  CGridPt2d C = _SGRID_POINT2D_INITIALIZER;

  // The atan2 functions return arctan y/x in the interval [−π , +π] radians
  real_t Dir_C_to_A = atan2(v1.y - C.y, v1.x - C.x);
  real_t Dir_C_to_B = atan2(v2.y - C.y, v2.x - C.x);
  real_t Angle_ACB = Dir_C_to_A - Dir_C_to_B;

  // Handle wrap around
  if (Angle_ACB < 0.0) {
    Angle_ACB += 2 * _M_PI_;
  }

  // Answer is in the range of [0-2*pi]
  return Angle_ACB;
}

// distance between hadmap::txPoint
real_t CTerrainMath::distSurfacePoint3D(const SSurfacePt3D &pt1, const SSurfacePt3D &pt2) {
  return sqrt(std::pow(pt1.x - pt2.x, 2.0) + std::pow(pt1.y - pt2.y, 2.0) + std::pow(pt1.z - pt2.z, 2.0));
}

real_t CTerrainMath::distSGridPoint2d(const CGridPt2d &pt1, const CGridPt2d &pt2) {
  return sqrt(std::pow(pt1.x - pt2.x, 2.0) + std::pow(pt1.y - pt2.y, 2.0));
}

real_t CTerrainMath::getSGrid2dVectorLen(const CGridPt2dVec &v) {
  CGridPt2d pt = _SGRID_POINT2D_INITIALIZER;

  return CTerrainMath::distSGridPoint2d(v, pt);
}
}  // namespace terrain
