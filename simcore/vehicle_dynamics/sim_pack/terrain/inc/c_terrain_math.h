#ifndef _CTERRAIN_MATH_H_
#define _CTERRAIN_MATH_H_

// A C++ program to check if a given SGridPoint lies inside a given polygon
// Refer https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
// for explanation of functions onSegment(), orientation() and doIntersect()
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "c_terrain_common.h"

using namespace std;

// Define Infinite (Using INT_MAX caused overflow problems)
#define _SGRID_POINT2D_INITIALIZER {0.0, 0.0};

namespace terrain {
struct CGridPt2d {
  real_t x;
  real_t y;
};

typedef CGridPt2d CGridPt2dVec;
typedef std::vector<CGridPt2d> SGridPolygon;

class CTerrainMath {
 public:
  // Returns true if the SGridPoint p lies inside the polygon[] with n vertices
  static bool isInside(const SGridPolygon &polygon, const CGridPt2d &p);

  // vector from p1 to p2
  static CGridPt2dVec getVectorFromPoint(const CGridPt2d &p1, const CGridPt2d &p2);

  // get clockwise angle from vector 1 to vector 2, unit rad, range [0-2*pi]
  static real_t getClockwiseAngle(const CGridPt2dVec &v1, const CGridPt2dVec &v2);

  // distance between hadmap::txPoint
  static real_t distSurfacePoint3D(const SSurfacePt3D &pt1, const SSurfacePt3D &pt2);

  static real_t distSGridPoint2d(const CGridPt2d &p1, const CGridPt2d &p2);
  static real_t getSGrid2dVectorLen(const CGridPt2dVec &v);
  static void testInside();
};
}  // namespace terrain

#endif
