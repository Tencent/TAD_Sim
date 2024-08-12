// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the struct PositionLane.

#include "hadmap_position_lane.h"
#include "utils.h"

namespace hadmap {
PositionLane::PositionLane() : road_id(0), lane_id(0), offset(0.0), s(0.0), orientation(Orientation()) {}

PositionLane::PositionLane(uint64_t k_road_id, int64_t k_lane_id, double k_offset, double k_s)
    : road_id(k_road_id), lane_id(k_lane_id), offset(k_offset), s(k_s), orientation(Orientation()) {}
PositionLane::PositionLane(uint64_t k_road_id, int64_t k_lane_id, double k_offset, double k_s,
                           Orientation k_orientation)
    : road_id(k_road_id), lane_id(k_lane_id), offset(k_offset), s(k_s), orientation(k_orientation) {}

PositionLane::PositionLane(const PositionLane& other)
    : road_id(other.road_id),
      lane_id(other.lane_id),
      offset(other.offset),
      s(other.s),
      orientation(other.orientation) {}

PositionLane& PositionLane::operator=(const PositionLane& k) {
  if (this != &k) {
    road_id = k.road_id;
    lane_id = k.lane_id;
    offset = k.offset;
    s = k.s;
    orientation = k.orientation;
  }
  return *this;
}

bool PositionLane::operator==(const PositionLane& other) const {
  return road_id == other.road_id && lane_id == other.lane_id && AlmostEqual(offset, other.offset) &&
         AlmostEqual(s, other.s) && orientation == other.orientation;
}

bool PositionLane::operator!() const {
  return road_id == 0 && lane_id == 0 && AlmostEqual(offset) && AlmostEqual(s) && orientation == Orientation();
}

const PositionLane PositionLane::operator+(const PositionLane& point) const {
  PositionLane ret = *this;
  ret.road_id += point.road_id;
  ret.lane_id += point.lane_id;
  ret.offset += point.offset;
  ret.s += point.s;
  // ret.orientation.h += point.orientation.h;
  // ret.orientation.p += point.orientation.p;
  // ret.orientation.r += point.orientation.r;
  ret.orientation = this->orientation + point.orientation;

  return ret;
}

const PositionLane PositionLane::operator-(const PositionLane& point) const {
  PositionLane ret = *this;
  ret.road_id -= point.road_id;
  ret.lane_id -= point.lane_id;
  ret.offset -= point.offset;
  ret.s -= point.s;
  // ret.orientation.h -= point.orientation.h
  // ret.orientation.p -= point.orientation.p;
  // ret.orientation.r -= point.orientation.r;
  ret.orientation = this->orientation - point.orientation;

  return ret;
}

const PositionLane PositionLane::operator*(const double d) const {
  PositionLane ret = *this;
  ret.road_id *= d;
  ret.lane_id *= d;
  ret.offset *= d;
  ret.s *= d;
  // ret.orientation.h *= d;
  // ret.orientation.p *= d;
  // ret.orientation.r *= d;
  ret.orientation = this->orientation * d;

  return ret;
}

const PositionLane PositionLane::operator/(const double d) const {
  if (AlmostEqual(d)) {
    return *this;
  }

  PositionLane ret = *this;
  ret.road_id /= d;
  ret.lane_id /= d;
  ret.offset /= d;
  ret.s /= d;
  // ret.orientation.h /= d;
  // ret.orientation.p /= d;
  // ret.orientation.r /= d;
  ret.orientation = this->orientation / d;
  return ret;
}

}  // namespace hadmap
