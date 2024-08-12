// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_parking_helper.h"
#include <float.h>

namespace eval {
const double eps = 1e-6;
const double PI = 3.141592653;
bool isSamePoint(const sim_msg::ParkingPoint *pt1, const sim_msg::ParkingPoint *pt2) {
  double dist = sqrt(pow(pt1->x() - pt2->x(), 2) + pow(pt1->y() - pt2->y(), 2));
  return dist < eps;
}

bool isSameParking(const sim_msg::ParkingPoint *pt1, const sim_msg::ParkingPoint *pt2, const sim_msg::Parking *p) {
  return (isSamePoint(pt1, &(p->lt())) || isSamePoint(pt1, &(p->lb())) || isSamePoint(pt1, &(p->rt())) ||
          isSamePoint(pt1, &(p->rb()))) &&
         (isSamePoint(pt2, &(p->lt())) || isSamePoint(pt2, &(p->lb())) || isSamePoint(pt2, &(p->rt())) ||
          isSamePoint(pt2, &(p->rb())));
}

double GetPointToPakingLineDist2D(Eigen::Vector3d pointP, sim_msg::Parking parking) {
  double A = 0.0, B = 0.0, C = 0.0;
  A = parking.lt().y() - parking.lb().y();
  B = parking.lb().x() - parking.lt().x();
  C = parking.lt().x() * parking.lb().y() - parking.lt().y() * parking.lb().x();

  double distance = 0;
  distance = (fabs(A * pointP.x() + B * pointP.y() + C)) / (sqrt(A * A + B * B));
  return distance;
}

double GetPointToSegDist(const Eigen::Vector3d &p1, const Eigen::Vector3d &p2, const Eigen::Vector3d &q) {
  double fDot = (p2 - p1).dot(q - p1);
  if (fDot <= 0.0f) {
    return (p1 - q).norm();
  }

  double d2AB = (p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y());
  if (fDot >= d2AB) {
    return (p2 - q).norm();
  }

  double u = fDot / d2AB;
  Eigen::Vector3d p12 = {p1.x() + (p2.x() - p1.x()) * u, p1.y() + (p2.y() - p1.y()) * u, 0.0};
  return (q - p12).norm();
}

double GetPointToPolygonDist2D(const Eigen::Vector3d &q, const RectCorners &ego_corners) {
  int n = ego_corners.size();
  double dist = DBL_MAX;
  for (int i = 0; i < n; i++) {
    int j = i == (n - 1) ? 0 : (i + 1);
    dist = std::min(dist, GetPointToSegDist(ego_corners[i], ego_corners[j], q));
  }
  return dist;
}

double GetAngleDiff(Eigen::Vector3d pt1, Eigen::Vector3d pt2, int method) {
  double theta1 = atan2(pt1.y(), pt1.x());  // 返回(-pi,pi)之间的反正切弧度值.
  double theta2 = atan2(pt2.y(), pt2.x());
  double result = std::abs(theta2 - theta1) > PI ? 2 * PI - std::abs(theta2 - theta1) : std::abs(theta2 - theta1);
  if (method == 1) {
    return result;
  }
  return result * 180 / PI;
}

bool InParkingSpace(Eigen::Vector3d corner, sim_msg::Parking parking) {
  double pos_x = corner.x();
  double pos_y = corner.y();
  bool res = false;
  double vert[4][2];
  vert[0][0] = parking.lt().x();
  vert[0][1] = parking.lt().y();
  vert[1][0] = parking.lb().x();
  vert[1][1] = parking.lb().y();
  vert[2][0] = parking.rb().x();
  vert[2][1] = parking.rb().y();
  vert[3][0] = parking.rt().x();
  vert[3][1] = parking.rt().y();

  int i, j = 0;
  for (i = 0, j = 3; i < 4; j = i++) {
    if (((vert[i][1] > pos_y) != (vert[j][1] > pos_y)) &&
        (pos_x < (vert[j][0] - vert[i][0]) * (pos_y - vert[i][1]) / (vert[j][1] - vert[i][1]) + vert[i][0])) {
      res = true;
    }
  }
  return res;
}

double GetParkingSpaceLineLateralYaw() {
  double lateral_yaw = 0.0;

  // ActorMapInfoPtr map_info = m_map_agent.GetMapInfo();

  // if (map_info && map_info->m_on_road) {
  //   const eval::EvalPoints &sample_points = map_info->m_active_lane.m_ref_line.sample_points;
  //   int index = CEvalMath::GetNearestPointIndex(sample_points, GetLocation());
  //   if (0 <= index && index < sample_points.size()) {
  //     lateral_yaw = CEvalMath::GetLateralYaw(sample_points.at(index), GetLocation());
  //   } else {
  //     LOG_ERROR << "eval | ego is too farway from ref line.\n";
  //   }
  // }
  return lateral_yaw;
}

void ParkingWGS84ToENU(const MapManagerPtr &map_manager, sim_msg::Parking &parking) {
  auto PointWGS84ToENU = [&map_manager](sim_msg::ParkingPoint &point) {
    CPosition wgs84_point(point.x(), point.y(), point.z(), Coord_WGS84);
    map_manager->WGS84ToENU(wgs84_point);
    point.set_x(wgs84_point.GetX());
    point.set_y(wgs84_point.GetY());
    point.set_z(wgs84_point.GetZ());
  };
  PointWGS84ToENU(*parking.mutable_lt());
  PointWGS84ToENU(*parking.mutable_lb());
  PointWGS84ToENU(*parking.mutable_rb());
  PointWGS84ToENU(*parking.mutable_rt());
  PointWGS84ToENU(*parking.mutable_center());
}

}  // namespace eval
