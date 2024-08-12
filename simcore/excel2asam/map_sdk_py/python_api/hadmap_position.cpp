// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the struct Position.

#include "hadmap_position.h"
#include "utils.h"

namespace hadmap {
Position::Position() : world(), lane() {}

Position::Position(PositionWorld k_world) : world(k_world), lane(PositionLane()) {}
Position::Position(PositionLane k_lane) : world(PositionWorld()), lane(k_lane) {}
Position::Position(PositionWorld k_world, PositionLane k_lane) : world(k_world), lane(k_lane) {}

Position::Position(const Position& other) : world(other.world), lane(other.lane) {}

Position& Position::operator=(const Position& k) {
  if (this != &k) {
    world = k.world;
    lane = k.lane;
  }
  return *this;
}

bool Position::operator==(const Position& other) const { return world == other.world && lane == other.lane; }

bool Position::operator!() const { return world == PositionWorld() && lane == PositionLane(); }

const Position Position::operator+(const Position& position) const {
  Position ret = *this;

  ret.world = this->world + position.world;
  ret.lane = this->lane + position.lane;

  return ret;
}

const Position Position::operator-(const Position& position) const {
  Position ret = *this;

  ret.world = this->world - position.world;
  ret.lane = this->lane - position.lane;

  return ret;
}

const Position Position::operator*(const double d) const {
  Position ret = *this;

  ret.world = this->world * d;
  ret.lane = this->lane * d;

  return ret;
}

const Position Position::operator/(const double d) const {
  if (AlmostEqual(d)) {
    return *this;
  }

  Position ret = *this;
  ret.world = this->world * d;
  ret.lane = this->lane * d;
  return ret;
}

}  // namespace hadmap
