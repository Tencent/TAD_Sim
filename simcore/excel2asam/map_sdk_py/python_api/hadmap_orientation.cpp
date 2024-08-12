// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the struct Orientation.

#include "hadmap_orientation.h"
#include "utils.h"

namespace hadmap {
Orientation::Orientation() : h(0.0), p(0.0), r(0.0) {}

Orientation::Orientation(double kh, double kp, double kr) : h(kh), p(kp), r(kr) {}

Orientation::Orientation(const Orientation& other) : h(other.h), p(other.p), r(other.r) {}

Orientation& Orientation::operator=(const Orientation& other) {
  if (this != &other) {
    h = other.h;
    p = other.p;
    r = other.r;
  }
  return *this;
}

bool Orientation::operator==(const Orientation& other) const {
  return AlmostEqual(h, other.h) && AlmostEqual(p, other.p) && AlmostEqual(r, other.r);
}

bool Orientation::operator!() const { return AlmostEqual(h) && AlmostEqual(p) && AlmostEqual(r); }

const Orientation Orientation::operator+(const Orientation& point) const {
  Orientation ret = *this;
  ret.h += point.h;
  ret.p += point.p;
  ret.r += point.r;
  return ret;
}

const Orientation Orientation::operator-(const Orientation& point) const {
  Orientation ret = *this;
  ret.h -= point.h;
  ret.p -= point.p;
  ret.r -= point.r;
  return ret;
}

const Orientation Orientation::operator*(const double d) const {
  Orientation ret = *this;
  ret.h *= d;
  ret.p *= d;
  ret.r *= d;
  return ret;
}

const Orientation Orientation::operator/(const double d) const {
  if (AlmostEqual(d)) {
    return *this;
  }

  Orientation ret = *this;
  ret.h /= d;
  ret.p /= d;
  ret.r /= d;
  return ret;
}

}  // namespace hadmap
