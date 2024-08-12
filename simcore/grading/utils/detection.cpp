// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "utils/detection.h"
#include <manager/actor_manager.h>
#include <algorithm>

namespace eval {
void CollisionDetection::CalAABB(const RectCorners &corners, CPosition &bl, CPosition &tr) {
  double x_s = 1e12, y_s = 1e12, x_b = -1e12, y_b = -1e12;

  for (auto r = 0; r < corners.size(); r++) {
    x_s = corners[r][0] < x_s ? corners[r][0] : x_s;
    y_s = corners[r][1] < y_s ? corners[r][1] : y_s;
    x_b = corners[r][0] > x_b ? corners[r][0] : x_b;
    y_b = corners[r][1] > y_b ? corners[r][1] : y_b;
  }

  bl.SetValues(x_s, y_s, 0.0);
  tr.SetValues(x_b, y_b, 0.0);
}

void CollisionDetection::CalCenterPt(const RectCorners &corners, CPosition &center) {
  double sum_x = 0.0, sum_y = 0.0;
  for (size_t i = 0; i < corners.size(); ++i) {
    sum_x += corners[i][0];
    sum_y += corners[i][1];
  }
  double center_x = sum_x / corners.size();
  double center_y = sum_y / corners.size();
  center.SetValues(center_x, center_y, 0.0);
}

bool CollisionDetection::IsInsideAABB(const CPosition &pt, const CPosition &bl, const CPosition &tr) {
  return pt.GetX() >= bl.GetX() && pt.GetY() >= bl.GetY() && pt.GetX() <= tr.GetX() && pt.GetY() <= tr.GetY();
}

bool CollisionDetection::IsInsideRect(const CPosition &pt, const RectCorners &corners_enu) {
  bool inside = false;
  // Get the Y coordinate of the point we're checking
  auto cross_y = pt.GetY();

  // Iterate over each corner of the polygon
  for (auto i = 0; i < corners_enu.size(); ++i) {
    int next_i = (i + 1) % corners_enu.size();

    CPosition pt_i(corners_enu[i][0], corners_enu[i][1], corners_enu[i][2]);
    CPosition pt_next_i(corners_enu[next_i][0], corners_enu[next_i][1], corners_enu[next_i][2]);

    double y_min = std::min(pt_i.GetY(), pt_next_i.GetY());
    double y_max = std::max(pt_i.GetY(), pt_next_i.GetY());

    // Check if the point's Y coordinate is between the min and max Y values of the edge
    if (y_min < cross_y && cross_y < y_max) {
      // Calculate the change in X and Y along the edge
      double delta_x = pt_next_i.GetX() - pt_i.GetX();
      double delta_y = pt_next_i.GetY() - pt_i.GetY();
      // Calculate the X coordinate on the edge that intersects with the point's Y coordinate
      double cross_x = (cross_y - pt_i.GetY()) * delta_x / delta_y + pt_i.GetX();

      // If the point's X coordinate is less than the calculated intersection X coordinate,
      // the point is crossing the edge, so we toggle the 'inside' flag
      if (pt.GetX() < cross_x) inside = !inside;
    }
  }

  return inside;
}

bool CollisionDetection::IsCollision(CStaticActor *actor1, CStaticActor *actor2) {
  if (actor1 != nullptr && actor2 != nullptr) {
    // get actor location
    const CLocation &loc1 = actor1->GetLocation();
    const CLocation &loc2 = actor2->GetLocation();

    // calculate absolute delta distance, see if these 2 actors may overlaps
    auto abs_dist = CEvalMath::AbsoluteDistance2D(loc1.GetPosition(), loc2.GetPosition());
    auto max_lenght_dist = actor1->GetBaseLength() + actor2->GetBaseLength();

    // if may overlap
    if (abs_dist <= max_lenght_dist * 0.5) {
      RectCorners &&corners1_enu = actor1->TransCorners2BaseCoord();
      RectCorners &&corners2_enu = actor2->TransCorners2BaseCoord();

      EVector3d bl_1, tr_1, center_pt_1;
      EVector3d bl_2, tr_2, center_pt_2;
      EVector3d pt;

      CalAABB(corners2_enu, bl_2, tr_2);
      for (auto e_pt : corners1_enu) {
        pt.SetValues(e_pt[0], e_pt[1], e_pt[2]);
        if (IsInsideAABB(pt, bl_2, tr_2) && IsInsideRect(pt, corners2_enu)) return true;
      }

      CalAABB(corners1_enu, bl_1, tr_1);
      for (auto e_pt : corners2_enu) {
        pt.SetValues(e_pt[0], e_pt[1], e_pt[2]);
        if (IsInsideAABB(pt, bl_1, tr_1) && IsInsideRect(pt, corners1_enu)) return true;
      }

      CalCenterPt(corners1_enu, center_pt_1);
      if (IsInsideAABB(center_pt_1, bl_2, tr_2) && IsInsideRect(center_pt_1, corners2_enu)) {
        VLOG_0 << "warning: center_pt_1 is inside actor 2 but the vertexs are not.\n";
        return true;
      }

      CalCenterPt(corners2_enu, center_pt_2);
      if (IsInsideAABB(center_pt_2, bl_1, tr_1) && IsInsideRect(center_pt_2, corners1_enu)) {
        VLOG_0 << "warning: center_pt_2 is inside actor 1 but the vertexs are not.\n";
        return true;
      }
    }
  }
  return false;
}
}  // namespace eval
