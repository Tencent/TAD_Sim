// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "actors/c_actor_base.h"

namespace eval {
CActorBase::CActorBase() {
  _type = Actor_Base;
  _map_mgr = CMapManager::GetInstance();
}

CActorBase CActorBase::CalXB(const CActorBase &GX, const CActorBase &GB) {
  CActorBase xb;

  Eigen::Matrix3d &&R_GX = GX.GetLocation().GetRotMatrix();

  *xb.MutableSimTime() = GX.GetSimTime();
  *xb.MutableLocation() = CEvalMath::CalLocationXB(GX.GetLocation(), GB.GetLocation());

  Eigen::Vector3d &&rel_v = R_GX.inverse() * (GX.GetSpeed().GetPoint() - GB.GetSpeed().GetPoint());
  xb.MutableSpeed()->SetValues(rel_v[0], rel_v[1], rel_v[2]);

  return xb;
}
}  // namespace eval
