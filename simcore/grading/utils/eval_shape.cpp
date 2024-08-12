// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "utils/eval_shape.h"
#include <exception>

namespace eval {
CRect::CRect(double length, double width, double height) {
  _corners.clear();
  for (auto i = 0; i < const_corner_size; ++i) _corners.push_back(Eigen::Vector3d());
  SetValues(length, width, height);
}

void CRect::SetValues(double length, double width, double height) {
  _length = length;
  _width = width;
  _height = height;

  _middle_front = {_length / 2, 0.0, 0.0};
  _middle_rear = {-_length / 2, 0.0, 0.0};

  double l = _length / 2, w = _width / 2;

  _corners[0] = {l, w, 0.0};    // fl
  _corners[1] = {l, -w, 0.0};   // fr
  _corners[2] = {-l, -w, 0.0};  // rr
  _corners[3] = {-l, w, 0.0};   // rl
}
}  // namespace eval
