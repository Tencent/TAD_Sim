/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/
#pragma once

#include "common/engine/common.h"
#include "common/engine/math/vec3.h"

class CVertexArrayObject;
class CMaterial;

class CAABB {
 public:
  CAABB();
  CAABB(const CVector3& min, const CVector3& max);
  ~CAABB();

  void Reset();
  void Set(const CVector3& min, const CVector3& max);
  void Expand(float fX, float fY, float fZ);
  void Expand(const CVector3& point);
  void Expand(const CPoint3D& point);
  void Expand(const CVector3* pPoints, int nNum);
  int Intersect(CAABB& otherAABB);

  void setMin(const CVector3& m) { m_min = m; }

  void setMax(const CVector3& m) { m_max = m; }
  const CVector3& Min() const { return m_min; }
  const CVector3& Max() const { return m_max; }
  CVector3 Center();

#ifndef __HADMAP_SERVER__
  void PrepareDraw();
  void Draw();

 protected:
  float* DrawData();
#endif

 protected:
  CVertexArrayObject* m_pvao;
  CMaterial* m_pmat;
  CVector3 m_min;
  CVector3 m_max;
};
