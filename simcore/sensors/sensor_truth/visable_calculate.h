/**
 * @file VisableCalculate.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <boost/geometry.hpp>
#include <set>
#include <vector>

typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> point;
typedef boost::geometry::model::multi_point<point> points;
typedef boost::geometry::model::polygon<point> polygon;
typedef boost::geometry::model::multi_polygon<polygon> mpolygon;

struct VisableBuffer {
  // for out
  int tag = 0;
  const void *rawPtr;
  double distance = 0;  // distance from center
  double area0 = 0;
  double area = 0;

  bool init(const points &pts);

  // for ReVisable
  mpolygon bounds;
  std::set<std::pair<int, int>> range;

  void update_range();
  bool isCover(const VisableBuffer &rhs) const;
};

bool ReVisable(std::vector<VisableBuffer> &vbs);
