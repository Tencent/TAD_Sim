/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "aabb.h"
#include "common/engine/constant.h"
#include "common/log/system_logger.h"

#ifndef __HADMAP_SERVER__
#  include "../engine.h"
#  include "../renderer/material.h"
#  include "../renderer/material_manager.h"
#  include "../renderer/renderer.h"
#  include "../renderer/vao/vao_manager.h"
#  include "../renderer/vao/vertex_array_object.h"
#endif
CAABB::CAABB() {
  m_pvao = nullptr;
  m_pmat = nullptr;

  Reset();
}

CAABB::CAABB(const CPoint3D& min, const CPoint3D& max) {
  m_pvao = nullptr;
  m_pmat = nullptr;

  Set(min, max);
}
CAABB::~CAABB() {
#ifndef __HADMAP_SERVER__
  if (m_pvao) {
    IVAOManager* vaomgr = Engine::Instance().GetRender()->GetVaoManager();
    if (!vaomgr) {
      assert(false);
      SYSTEM_LOGGER_ERROR("vaomanager not created!");
      return;
    }

    vaomgr->DestoryVAO(m_pvao);
    m_pvao = nullptr;
  }
#endif
}

void CAABB::Reset() {
  m_min = CPoint3D(INFINITY, INFINITY, INFINITY);
  m_max = CPoint3D(-INFINITY, -INFINITY, -INFINITY);
}

void CAABB::Set(const CPoint3D& min, const CPoint3D& max) {
  m_min = min;
  m_max = max;
}
void CAABB::Expand(float fX, float fY, float fZ) {
  if (m_max.X() < fX) m_max.X() = fX;
  if (m_max.Y() < fY) m_max.Y() = fY;
  if (m_max.Z() < fZ) m_max.Z() = fZ;

  if (m_min.X() > fX) m_min.X() = fX;
  if (m_min.Y() > fY) m_min.Y() = fY;
  if (m_min.Z() > fZ) m_min.Z() = fZ;
}
void CAABB::Expand(const CVector3& point) {
  if (m_max.X() < point.X()) m_max.X() = point.X();
  if (m_max.Y() < point.Y()) m_max.Y() = point.Y();
  if (m_max.Z() < point.Z()) m_max.Z() = point.Z();

  if (m_min.X() > point.X()) m_min.X() = point.X();
  if (m_min.Y() > point.Y()) m_min.Y() = point.Y();
  if (m_min.Z() > point.Z()) m_min.Z() = point.Z();
}

void CAABB::Expand(const CPoint3D& point) {
  if (m_max.X() < point.X()) m_max.X() = point.X();
  if (m_max.Y() < point.Y()) m_max.Y() = point.Y();
  if (m_max.Z() < point.Z()) m_max.Z() = point.Z();

  if (m_min.X() > point.X()) m_min.X() = point.X();
  if (m_min.Y() > point.Y()) m_min.Y() = point.Y();
  if (m_min.Z() > point.Z()) m_min.Z() = point.Z();
}

void CAABB::Expand(const CPoint3D* pPoints, int nNum) {
  if (!pPoints || nNum < 1) {
    return;
  }

  for (int i = 0; i < nNum; ++i) {
    Expand(pPoints[i]);
  }
}

int CAABB::Intersect(CAABB& otherAABB) {
  if (m_max.X() < otherAABB.m_min.X() && m_max.Y() < otherAABB.m_min.Y() && m_max.Z() < otherAABB.m_min.Z()) {
    return AABB_Outside;
  }

  if (m_min.X() > otherAABB.m_max.X() && m_min.Y() > otherAABB.m_max.Y() && m_min.Z() > otherAABB.m_min.Z()) {
    return AABB_Outside;
  }

  return AABB_Intersect;
}

CPoint3D CAABB::Center() { return m_max * 0.5 + m_min * 0.5; }

#ifndef __HADMAP_SERVER__

void CAABB::PrepareDraw() {
  IVAOManager* vaomgr = Engine::Instance().GetRender()->GetVaoManager();
  if (!vaomgr) {
    assert(false);
    SYSTEM_LOGGER_ERROR("vaomanager not created!");
    return;
  }

  if (m_pvao) {
    vaomgr->DestoryVAO(m_pvao);
    m_pvao = nullptr;
  }

  m_pvao = vaomgr->CreateVAO();

  float* pBuffer = DrawData();

  m_pvao->SetData(pBuffer);
  m_pvao->SetDataSize(72 * sizeof(float));
  m_pvao->SetElementCount(24);
  m_pvao->SetDataType(VET_float3);
  m_pvao->SetDataSemantic(VES_position);
  m_pvao->PrepareData();

  m_pmat = Engine::Instance().GetRender()->GetMaterialManager()->GetDefaultMaterial();
}

float* CAABB::DrawData() {
  float data[72] = {
      m_min.X(), m_min.Y(), m_min.Z(),                                   // line 0
      m_max.X(), m_min.Y(), m_min.Z(), m_max.X(), m_min.Y(), m_min.Z(),  // line 1
      m_max.X(), m_min.Y(), m_max.Z(), m_max.X(), m_min.Y(), m_max.Z(),  // line 2
      m_min.X(), m_min.Y(), m_max.Z(), m_min.X(), m_min.Y(), m_max.Z(),  // line 3
      m_min.X(), m_min.Y(), m_min.Z(), m_min.X(), m_max.Y(), m_min.Z(),  // line 4
      m_max.X(), m_max.Y(), m_min.Z(), m_max.X(), m_max.Y(), m_min.Z(),  // line 5
      m_max.X(), m_max.Y(), m_max.Z(), m_max.X(), m_max.Y(), m_max.Z(),  // line 6
      m_min.X(), m_max.Y(), m_max.Z(), m_min.X(), m_max.Y(), m_max.Z(),  // line 7
      m_min.X(), m_max.Y(), m_min.Z(), m_min.X(), m_min.Y(), m_min.Z(),  // line 8
      m_min.X(), m_max.Y(), m_min.Z(), m_max.X(), m_min.Y(), m_min.Z(),  // line 9
      m_max.X(), m_max.Y(), m_min.Z(), m_max.X(), m_min.Y(), m_max.Z(),  // line 10
      m_max.X(), m_max.Y(), m_max.Z(), m_min.X(), m_min.Y(), m_max.Z(),  // line 11
      m_min.X(), m_max.Y(), m_max.Z(),
  };
  float* pBuffer = new float[72];
  for (int i = 0; i < 72; ++i) {
    pBuffer[i] = data[i];
  }

  return pBuffer;
}

void CAABB::Draw() {
  if (!m_pmat || !m_pvao) {
    return;
  }

  m_pmat->Apply(true);
  m_pvao->Bind();
  m_pvao->Draw();
  m_pvao->Unbind();
  m_pmat->Apply(false);
}
#endif
