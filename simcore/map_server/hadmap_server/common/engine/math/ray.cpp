/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/engine/math/ray.h"
#include "common/engine/math/utils.h"
#include "engine/entity/aabb.h"

Ray::Ray() {}

Ray::Ray(CVector3& origon, CVector3& dir) {
  m_origon = origon;
  m_dir = dir;
}

bool Ray::Intersect(CAABB& aabb) {
  CVector3 ptOnPlane;
  const auto& min = aabb.Min();
  const auto& max = aabb.Max();
  float t;

  const CVector3& orig = m_origon;
  const CVector3& dir = m_dir;

  if (!CUtils::is_zero(dir.X())) {
    if (dir.X() > 0) {
      t = (min.X() - orig.X()) / dir.X();
    } else {
      t = (max.X() - orig.X()) / dir.X();
    }

    if (t > 0.0f) {
      ptOnPlane = orig + dir * t;
      if (ptOnPlane.Y() > min.Y() && ptOnPlane.Y() < max.Y() && ptOnPlane.Z() > min.Z() && ptOnPlane.Z() < max.Z()) {
        return true;
      }
    }
  }

  if (!CUtils::is_zero(dir.Y())) {
    if (dir.Y() > 0) {
      t = (min.Y() - orig.Y()) / dir.Y();
    } else {
      t = (max.Y() - orig.Y()) / dir.Y();
    }

    if (t > 0.0f) {
      ptOnPlane = orig + dir * t;
      if (ptOnPlane.X() > min.X() && ptOnPlane.X() < max.X() && ptOnPlane.Z() > min.Z() && ptOnPlane.Z() < max.Z()) {
        return true;
      }
    }
  }

  if (!CUtils::is_zero(dir.Z())) {
    if (dir.Z() > 0) {
      t = (min.Z() - orig.Z()) / dir.Z();
    } else {
      t = (max.Z() - orig.Z()) / dir.Z();
    }

    if (t > 0.0f) {
      ptOnPlane = orig + dir * t;
      if (ptOnPlane.Y() > min.Y() && ptOnPlane.Y() < max.Y() && ptOnPlane.X() > min.X() && ptOnPlane.X() < max.X()) {
        return true;
      }
    }
  }

  return false;
}
