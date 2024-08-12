// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <cmath>
#include "structs/base_struct.h"
#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif
namespace hadmap {
namespace Util {
// Function to calculate the heading angle between two points
double getHdgFrmPoints(const txPoint& p1, const txPoint& p2) {
  double deg = std::atan2((p2.y - p1.y), (p2.x - p1.x));
  deg = deg * 180.0 / M_PI;
  return deg;
}

// Function to calculate the distance between two points
double getDistance(const txPoint& p1, const txPoint& p2) {
  double fDistance = std::sqrt((p2.y - p1.y) * (p2.y - p1.y) + (p2.x - p1.x) * (p2.x - p1.x));
  return fDistance;
}

// Function to fit the center of a circle using least squares method
void FitCenterByLeastSquares(std::vector<std::pair<double, double>> mapPoint, double& radius) {
  if (mapPoint.size() < 3) {
    radius = 0.0;
    return;
  }
  double radiusVecSum = 0.0;

  for (int i = 0; i < mapPoint.size() - 2; i++) {
    struct point {
      double x;
      double y;
    };
    point P1{mapPoint.at(i).first, mapPoint.at(i).second};
    point P2{mapPoint.at(i + 1).first, mapPoint.at(i + 1).second};
    point P3{mapPoint.at(i + 2).first, mapPoint.at(i + 2).second};
    if (P1.x == P2.x == P3.x) {
      radius = 0;
    } else {
      double dis1, dis2, dis3;
      double cosA, sinA, dis;
      dis1 = sqrt((P1.x - P2.x) * (P1.x - P2.x) + (P1.y - P2.y) * (P1.y - P2.y));
      dis2 = sqrt((P1.x - P3.x) * (P1.x - P3.x) + (P1.y - P3.y) * (P1.y - P3.y));
      dis3 = sqrt((P2.x - P3.x) * (P2.x - P3.x) + (P2.y - P3.y) * (P2.y - P3.y));
      dis = dis1 * dis1 + dis3 * dis3 - dis2 * dis2;

      if (dis1 == 0 || dis2 == 0 || dis3 == 0 || dis == 0) {
        radius = 0;
      } else {
        cosA = dis / (2 * dis1 * dis3);
        if (std::abs(1 - cosA * cosA) < 1e-6 || std::abs(dis2) < 1e-6) {
          radius = 0;
        } else {
          sinA = sqrt(1 - cosA * cosA);
          radius = 0.5 * dis2 / sinA;
          radius = 1 / radius;  //
        }
      }
    }
    radiusVecSum += radius;
  }
  radius = radiusVecSum / static_cast<double>(mapPoint.size() - 2);
}

// Function to split a string using a specified delimiter
void stringSplit(const std::string& str, const std::string& splits, std::vector<std::string>& res) {
  if (str == "") return;
  std::string strs = str + splits;
  size_t pos = strs.find(splits);
  int step = splits.size();
  while (pos != strs.npos) {
    std::string temp = strs.substr(0, pos);
    res.push_back(temp);
    strs = strs.substr(pos + step, strs.size());
    pos = strs.find(splits);
  }
}
}  // namespace Util
}  // namespace hadmap
