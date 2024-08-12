// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the class Waypoint.

#include "hadmap_waypoint.h"
#include <iostream>
#include "hadmap_map.h"
#include "utils.h"

namespace hadmap {
Waypoint::Waypoint() : map(nullptr), position(Position()), is_junction(false) {}

Waypoint::Waypoint(Position k_position) : map(nullptr), position(k_position), is_junction(false) {}

Waypoint::Waypoint(std::shared_ptr<Map> k_map, Position k_position)
    : map(k_map), position(k_position), is_junction(false) {
  position = k_map->ConvertPosition(k_position);
}

Waypoint::Waypoint(std::shared_ptr<Map> k_map, Position k_position, bool k_is_junction)
    : map(k_map), position(k_position), is_junction(k_is_junction) {
  position = k_map->ConvertPosition(k_position);
}

Waypoint::Waypoint(const Waypoint& other) : map(other.map), position(other.position), is_junction(other.is_junction) {}

Waypoint& Waypoint::operator=(const Waypoint& k) {
  if (this != &k) {
    map = k.map;
    position = k.position;
    is_junction = k.is_junction;
  }
  return *this;
}

bool Waypoint::operator==(const Waypoint& other) const {
  return map == other.map && position == other.position && is_junction == other.is_junction;
}

bool Waypoint::operator!() const { return !map && !is_junction && position == Position(); }

const Waypoint Waypoint::operator+(const Waypoint& waypoint) const {
  Waypoint ret = *this;

  ret.position = this->position + waypoint.position;
  ret.is_junction = waypoint.is_junction;

  return ret;
}

const Waypoint Waypoint::operator-(const Waypoint& waypoint) const {
  Waypoint ret = *this;

  ret.position = this->position - waypoint.position;
  ret.is_junction = waypoint.is_junction;

  return ret;
}

const Waypoint Waypoint::operator*(const double d) const {
  Waypoint ret = *this;

  ret.position = this->position * d;

  return ret;
}

const Waypoint Waypoint::operator/(const double d) const {
  if (AlmostEqual(d)) {
    return *this;
  }

  Waypoint ret = *this;
  ret.position = this->position / d;
  return ret;
}

Waypoint& Waypoint::Update(uint64_t road_id, int64_t lane_id, double offset, double s, double x, double y, double z,
                           double h, double p, double r) {
  // 更新 lane 坐标系下信息
  position.lane.road_id += road_id;
  position.lane.lane_id += lane_id;
  position.lane.offset += offset;
  position.lane.s += s;
  position.lane.orientation.h += h;
  position.lane.orientation.p += p;
  position.lane.orientation.r += r;
  // 更新 world 坐标系下的位置信息
  position.world.x += x;
  position.world.y += y;
  position.world.z += z;
  position.world.h += h;
  position.world.p += p;
  position.world.r += r;
  // 返回修改后的 Waypoint 对象
  return *this;
}

Waypoint Waypoint::Next(double distance, std::string direction) const {
  if (!map) {
    return *this;
  }
  return Waypoint(map, Position(map->Next(position.world, distance, direction)));
}

}  // namespace hadmap
