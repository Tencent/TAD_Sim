// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eigen3/Eigen/Dense"
#include "utils/eval_common.h"

namespace eval {
class CShape {
 public:
  virtual ShapeType GetShapeType() { return Shape_Base; }
};

/**
 * @brief Rectangle shape defined in eval.
 */
class CRect final : public CShape {
 private:
  const ShapeType _shape_type = Shape_RECT;
  double _length, _width, _height;
  Eigen::Vector3d _middle_front, _middle_rear;
  RectCorners _corners;

 public:
  explicit CRect(double length = ego_size::ego.length, double width = ego_size::ego.width,
                 double height = ego_size::ego.height);
  CRect(const CRect &in) { this->CopyFrom(in); }
  CRect &operator=(const CRect &in) {
    this->CopyFrom(in);
    return *this;
  }
  CRect(CRect &&in) { this->CopyFrom(in); }
  CRect &operator=(CRect &&in) {
    this->CopyFrom(in);
    return *this;
  }
  void CopyFrom(const CRect &in) {
    _length = in.GetLength();
    _width = in.GetWidth();
    _height = in.GetHeight();
    _middle_front = in.GetMidFront();
    _middle_rear = in.GetMidRear();
    _corners = in.GetRectCorners();
  }
  void SetValues(double length, double width, double height);

 public:
  inline double GetLength() const { return _length; }
  inline double GetWidth() const { return _width; }
  inline double GetHeight() const { return _height; }
  inline Eigen::Vector3d GetMidFront() const { return _middle_front; }
  inline Eigen::Vector3d GetMidRear() const { return _middle_rear; }
  // fl, fr, rr, rl
  inline RectCorners GetRectCorners() const { return _corners; }

  virtual ShapeType GetShapeType() const { return _shape_type; }
};

template <class T>
class CShapeBuilder {
 public:
  static CSharedPtr<T> Build() { return CSharedPtr<T>(new T()); }
};
}  // namespace eval
