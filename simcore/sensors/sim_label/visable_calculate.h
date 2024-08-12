// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
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
  polygon bounds0;
  mpolygon bounds;
  std::set<std::pair<int, int>> range;

  void update_range();
  bool isCover(const VisableBuffer &rhs) const;
};

bool ReVisable(std::vector<VisableBuffer> &vbs);
