/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/
#pragma once

#include "common/engine/math/vec3.h"

class CVertexArrayObject;
class CMaterial;

class CAABB {
 public:
  CAABB();
  CAABB(const CPoint3D& min, const CPoint3D& max);
  ~CAABB();

  void Reset();
  void Set(const CPoint3D& min, const CPoint3D& max);
  void Expand(float fX, float fY, float fZ);
  void Expand(const CVector3& point);
  void Expand(const CPoint3D& point);
  void Expand(const CPoint3D* pPoints, int nNum);
  int Intersect(CAABB& otherAABB);
  CPoint3D& Min() { return m_min; }
  CPoint3D& Max() { return m_max; }
  CPoint3D Center();

#ifndef __HADMAP_SERVER__
  void PrepareDraw();
  void Draw();

 protected:
  float* DrawData();
#endif

 protected:
  CVertexArrayObject* m_pvao;
  CMaterial* m_pmat;
  CPoint3D m_min;
  CPoint3D m_max;
};
