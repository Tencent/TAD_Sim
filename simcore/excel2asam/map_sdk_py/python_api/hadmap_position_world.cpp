// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the struct PositionWorld.

#include "hadmap_position_world.h"
#include "utils.h"

namespace hadmap {
PositionWorld::PositionWorld() : x(0.0), y(0.0), z(0.0), h(0.0), p(0.0), r(0.0) {}

PositionWorld::PositionWorld(double kx, double ky, double kz) : x(kx), y(ky), z(kz), h(0.0), p(0.0), r(0.0) {}
PositionWorld::PositionWorld(double kx, double ky, double kz, double kh, double kp, double kr)
    : x(kx), y(ky), z(kz), h(kh), p(kh), r(kr) {}

PositionWorld::PositionWorld(const PositionWorld& k) : x(k.x), y(k.y), z(k.z), h(k.h), p(k.h), r(k.r) {}

PositionWorld& PositionWorld::operator=(const PositionWorld& k) {
  if (this != &k) {
    x = k.x;
    y = k.y;
    z = k.z;
    h = k.h;
    p = k.p;
    r = k.r;
  }
  return *this;
}

bool PositionWorld::operator==(const PositionWorld& other) const {
  return AlmostEqual(x, other.x) && AlmostEqual(y, other.y) && AlmostEqual(z, other.z) && AlmostEqual(h, other.h) &&
         AlmostEqual(p, other.p) && AlmostEqual(r, other.r);
}

bool PositionWorld::operator!() const {
  return AlmostEqual(x) && AlmostEqual(y) && AlmostEqual(z) && AlmostEqual(h) && AlmostEqual(p) && AlmostEqual(r);
}

const PositionWorld PositionWorld::operator+(const PositionWorld& point) const {
  PositionWorld ret = *this;
  ret.x += point.x;
  ret.y += point.y;
  ret.z += point.z;
  ret.h += point.h;
  ret.p += point.p;
  ret.r += point.r;
  return ret;
}

const PositionWorld PositionWorld::operator-(const PositionWorld& point) const {
  PositionWorld ret = *this;
  ret.x -= point.x;
  ret.y -= point.y;
  ret.z -= point.z;
  ret.h -= point.h;
  ret.p -= point.p;
  ret.r -= point.r;
  return ret;
}

const PositionWorld PositionWorld::operator*(const double d) const {
  PositionWorld ret = *this;
  ret.x *= d;
  ret.y *= d;
  ret.z *= d;
  ret.h *= d;
  ret.p *= d;
  ret.r *= d;
  return ret;
}

const PositionWorld PositionWorld::operator/(const double d) const {
  if (AlmostEqual(d)) {
    return *this;
  }

  PositionWorld ret = *this;
  ret.x /= d;
  ret.y /= d;
  ret.z /= d;
  ret.h /= d;
  ret.p /= d;
  ret.r /= d;
  return ret;
}

}  // namespace hadmap
