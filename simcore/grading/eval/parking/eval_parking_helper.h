// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "eval/eval_base.h"
#include "parking_state.pb.h"

namespace eval {
extern const double eps;
extern const double PI;

/**
 * @brief calculate the Euclidean distance between two points. If this distance is less than some small value eps, the
 * two points are considered to be the same.
 */
bool isSamePoint(const sim_msg::ParkingPoint *pt1, const sim_msg::ParkingPoint *pt2);

/**
 * @brief Compare each point (pt1 and pt2) with each of the corners of the parking space p by using isSamePoint(). If
 * both pt1 and pt2 match any of the corners, then returns true, indicating that both points are corners of the same
 * parking space.
 */
bool isSameParking(const sim_msg::ParkingPoint *pt1, const sim_msg::ParkingPoint *pt2, const sim_msg::Parking *p);

/**
 * @brief calculates the perpendicular distance from pointP to the line defined by two points lt (left top) and lb (left
 * bottom) of a space parking.
 */
double GetPointToPakingLineDist2D(Eigen::Vector3d pointP, sim_msg::Parking parking);

/**
 * @brief Calculate the shortest distance from a point to a line segment in 2D.
 */
double GetPointToSegDist(const Eigen::Vector3d &p1, const Eigen::Vector3d &p2, const Eigen::Vector3d &q);

/**
 * @brief Calculate the shortest distance from a point to a Polygon in 2D.
 */
double GetPointToPolygonDist2D(const Eigen::Vector3d &q, const RectCorners &ego_corners);

/**
 * @brief the angle between two vectors
 * @param pt1 vector 1.
 * @param pt2 vector 2.
 * @param method: 0 is the angle, 1 is the radian.
 * @return The angle is between 0 and pi.
 */
double GetAngleDiff(Eigen::Vector3d pt1, Eigen::Vector3d pt2, int method = 0);

/**
 * @brief Check if the corner point is in the parking space.
 */
bool InParkingSpace(Eigen::Vector3d corner, sim_msg::Parking parking);

/**
 * @brief Check the lateral yaw between vehicle and parking space line.
 */
double GetParkingSpaceLineLateralYaw();

/**
 * @brief transform the parking from WGS84 to ENU.
 *
 * @param map_manager
 * @param parking parking which needs to be converted.
 */
void ParkingWGS84ToENU(const MapManagerPtr &map_manager, sim_msg::Parking &parking);
}  // namespace eval
