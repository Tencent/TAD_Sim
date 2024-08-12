// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "common/macros.h"
#include "structs/map_structs.h"
#include "types/map_types.h"

#include <string>

namespace map_util {
// Directly distance
// Global loc [x, y, z], [u, v, w]
TXSIMMAP_API double directlyDis(double a[], double b[], size_t len = 3);

TXSIMMAP_API double directlySeqDis(double a[], double b[], size_t len = 3);

TXSIMMAP_API double directlyDis(const hadmap::Point2d& a, const hadmap::Point2d& b);

TXSIMMAP_API double directlySeqDis(const hadmap::Point2d& a, const hadmap::Point2d& b);

// Calculate distance
TXSIMMAP_API double distanceBetweenPoints(const hadmap::Point3d& p0, const hadmap::Point3d& p1, const bool& WGS84);

// Normalize
TXSIMMAP_API void normalize(double v[], size_t len = 3);

TXSIMMAP_API void normalize(hadmap::Point2d& n);

// Specified dir vector
// xyz / enu coord
TXSIMMAP_API void dirVector(const hadmap::Points2d& points, size_t index, hadmap::Point2d& dirVec);

// Find nearest point index
// WGS84 is used to specify loc coord type
// GLOBAL | LON_LAT
TXSIMMAP_API bool nearestPoint(const hadmap::Points3d& curve, const hadmap::Point3d& loc, bool WGS84,
                               size_t& nearestIndex, double& distance);

// Find nearest point index in 2D plane
TXSIMMAP_API bool nearestPoint2d(const hadmap::Points2d& curve, const hadmap::Point2d& loc, bool WGS84,
                                 size_t& nearestIndex, double& distance);

// Find intersection point, line is represented by two points
// Only used in ENU coord
TXSIMMAP_API bool intersectPointDP(const hadmap::Point2d& firstP0, const hadmap::Point2d& firstP1,
                                   const hadmap::Point2d& secondP0, const hadmap::Point2d& secondP1,
                                   hadmap::Point2d& intersectP);

// Line is represented by point and normal
TXSIMMAP_API bool intersectPointNP(const hadmap::Point2d& firstP, const hadmap::Point2d& firstN,
                                   const hadmap::Point2d& secondP, const hadmap::Point2d& secondN,
                                   hadmap::Point2d& intersectP);

// Point to line segment distance
TXSIMMAP_API double calcPoint2LinesegDis(const hadmap::Point3d& point, const hadmap::Point3d& segBegin,
                                         const hadmap::Point3d& segEnd, const bool& WGS84);

// Point to polyline distance
TXSIMMAP_API double calcPoint2PolylineDis(const hadmap::Point3d& point, const hadmap::Points3d& polyline,
                                          const bool& WGS84, const bool& countZ = false);

// Calculate lane width
TXSIMMAP_API double calcLaneWidth(const hadmap::Points3d& leftBoundaryGeom, const hadmap::Points3d& rightBoundaryGeom,
                                  const bool& WGS84);

// Project point to line
TXSIMMAP_API double projectPoint2Line(const hadmap::Point3d& p, const hadmap::Point3d& start,
                                      const hadmap::Point3d& end, const bool& WGS84, hadmap::Point3d& projectP);

TXSIMMAP_API double projectPoint2Line(const hadmap::Point2d& p, const hadmap::Point2d& start,
                                      const hadmap::Point2d& end, const bool& WGS84, hadmap::Point2d& projectP);

// Sort lane data by ids
// Road id asc, section id asc, lane id desc
TXSIMMAP_API void sortLaneData(hadmap::tx_lane_vec& lanes);

// Intersect point on line segment
TXSIMMAP_API bool pointOnLine(const hadmap::Point2d& begin, const hadmap::Point2d& end,
                              const hadmap::Point2d& intersectP);

// Point in left / right side
// Left -> 1
// Right -> -1
TXSIMMAP_API int pointSide(const hadmap::Point2d& begin, const hadmap::Point2d& end, const hadmap::Point2d& point);

// Dir -> yaw
// Yaw -> [0, 360]
// Dir coord -> xyz | enu
TXSIMMAP_API double dir2yaw(const hadmap::Point2d& dir, bool ENU);

// Yaw -> dir
// Yaw -> [0, 360]
// Dir coord -> xyz | enu
TXSIMMAP_API void yaw2dir(double yaw, bool ENU, hadmap::Point2d& dir);

// Vector angle
// Angle -> [0, 180]
// Vector coord -> xyz | enu
TXSIMMAP_API double angle(const hadmap::Point2d& v0, const hadmap::Point2d& v1);

// Yaw exchange
// [0, 360] -> [-pi, pi]
TXSIMMAP_API double angle2radian(double a);

TXSIMMAP_API double radian2angle(double r);

// Judge dir type by yaw
TXSIMMAP_API hadmap::DIR_TYPE turnDir(const double& entranceYaw, const double& exitYaw);

// Point in polygon
TXSIMMAP_API bool posInPolygon(const hadmap::Point2d& pos, const hadmap::Points2d& polygon, const bool& wgs84);

// Point to rect min distance
TXSIMMAP_API int pointToRectDistance(const hadmap::Point2d& v1, const hadmap::Point2d& leftBottom,
                                     const hadmap::Point2d& rightTop);

// Check point if in rect
TXSIMMAP_API int isPointInRect(const hadmap::Point2d& v1, const hadmap::Point2d& leftBottom,
                               const hadmap::Point2d& rightTop);

TXSIMMAP_API bool IsInRect(const hadmap::Point2d& point, const hadmap::Point2d& A, const hadmap::Point2d& B,
                           const hadmap::Point2d& C, const hadmap::Point2d& D);

TXSIMMAP_API double calcCircumcircleRadius(hadmap::Point2d A, hadmap::Point2d B, hadmap::Point2d C, const bool& WGS84);
}  // namespace map_util
