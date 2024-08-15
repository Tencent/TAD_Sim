// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "common/macros.h"

namespace coord_trans_api {
// lon lat ele -> global x y z
TXSIMMAP_API void lonlat2global(double& x, double& y, double& z);

// global x y z -> lon lat ele
TXSIMMAP_API void global2lonlat(double& x, double& y, double& z);

// global x y z -> local x y z ( seu )
TXSIMMAP_API void global2local(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// local x y z ( seu )-> global x y z
TXSIMMAP_API void local2global(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// lon lat ele -> local x y z
TXSIMMAP_API void lonlat2local(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// local x y z -> lon lat ele
TXSIMMAP_API void local2lonlat(double& lon, double& lat, double& ele, double refLon, double refLat, double refAlt);

// global x y z -> enu x y z
TXSIMMAP_API void global2enu(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// enu x y z -> global x y z
TXSIMMAP_API void enu2global(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// lon lat ele -> enu x y z
TXSIMMAP_API void lonlat2enu(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// enu x y z -> lon lat ele
TXSIMMAP_API void enu2lonlat(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// seu x y z -> enu x y z
TXSIMMAP_API void seu2enu(double& enuX, double& enuY, double& enuZ, double seuX, double seuY, double seuZ);

// enu x y z -> seu x y z
TXSIMMAP_API void enu2seu(double& seuX, double& seuY, double& seuZ, double enuX, double enuY, double enuZ);

// lon lat -> mercator
TXSIMMAP_API void lonlat2mercator(double& x, double& y);

// mercator -> lon lat
TXSIMMAP_API void mercator2lonlat(double& x, double& y);
}  // namespace coord_trans_api
