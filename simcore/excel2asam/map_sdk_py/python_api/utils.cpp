// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines some utils.

#include "utils.h"

const double EPSILON = 1e-2;

bool AlmostEqual(double a) { return std::fabs(a) < EPSILON; }

bool AlmostEqual(double a, double b) { return std::fabs(a - b) < EPSILON; }

std::string CalDirection(double delta) {
  double thr1 = 45.0;
  double thr2 = 150.0;

  std::string phase = "";

  if (std::abs(delta) < thr1) {
    // Straight
    phase = "T";
  } else if (delta >= thr1 && delta < thr2) {
    // Left
    phase = "L";
  } else if (delta >= thr2 && delta <= 180) {
    // LeftTurn
    phase = "L0";
  } else if (delta > (-1 * thr2) && delta <= (-1 * thr1)) {
    // Right
    phase = "R";
  } else {
    // RightTurn
    phase = "R0";
  }

  return phase;
}

double Round(double value, int precision) {
  double multiplier = std::pow(10, precision);
  return std::round(value * multiplier) / multiplier;
}
