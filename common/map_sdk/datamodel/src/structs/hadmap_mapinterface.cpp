// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_mapinterface.h"
#include "common/coord_trans.h"

namespace hadmap {
void txMapInterface::txProjection::lonlat2enu(const txPoint& lonlat, txPoint& enu) const {
  enu.x = lonlat.x;
  enu.y = lonlat.y;
  enu.z = lonlat.z;
  coord_trans_api::lonlat2enu(enu.x, enu.y, enu.z, originLL.x, originLL.y, originLL.z);
}

void txMapInterface::txProjection::enu2lonlat(const txPoint& enu, txPoint& lonlat) const {
  lonlat.x = enu.x;
  lonlat.y = enu.y;
  lonlat.z = enu.z;
  coord_trans_api::enu2lonlat(lonlat.x, lonlat.y, lonlat.z, originLL.x, originLL.y, originLL.z);
}

void txMapInterface::txProjection::transform(double& x, double& y) const {
  txPoint ll(x, y, 0.0);
  txPoint r;
  lonlat2enu(ll, r);
  x = r.x;
  y = r.y;
}

void txMapInterface::txProjection::untransform(double& x, double& y) const {
  txPoint enu(x, y, 0.0);
  txPoint r;
  enu2lonlat(enu, r);
  x = r.x;
  y = r.y;
}

double txMapInterface::txProjection::dX() const { return dx; }

double txMapInterface::txProjection::dY() const { return dy; }

pkid_t txMapInterface::txProjection::pID() const { return pId; }
}  // namespace hadmap
