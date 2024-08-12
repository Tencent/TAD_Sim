/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include "common/engine/math/vec3.h"

class CAABB;

class Ray {
 public:
  Ray();
  Ray(CVector3& origon, CVector3& dir);
  CVector3& Origon() { return m_origon; }
  CVector3& Dir() { return m_dir; }

  bool Intersect(CAABB& aabb);

 protected:
  CVector3 m_origon;
  CVector3 m_dir;
};
