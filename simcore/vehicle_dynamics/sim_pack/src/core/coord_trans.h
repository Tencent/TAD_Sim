// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_SIM_PACK_CORE_COORD_TRANS_H_
#define VEHICLEDYNAMICS_SIM_PACK_CORE_COORD_TRANS_H_

namespace coord_trans_api {
// lon lat ele -> global x y z
void lonlat2global(double& x, double& y, double& z);

// global x y z -> lon lat ele
void global2lonlat(double& x, double& y, double& z);

// global x y z -> local x y z ( seu )
void global2local(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// local x y z ( seu )-> global x y z
void local2global(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// lon lat ele -> local x y z
void lonlat2local(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// local x y z -> lon lat ele
void local2lonlat(double& lon, double& lat, double& ele, double refLon, double refLat, double refAlt);

// global x y z -> enu x y z
void global2enu(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// enu x y z -> global x y z
void enu2global(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// lon lat ele -> enu x y z
void lonlat2enu(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// enu x y z -> lon lat ele
void enu2lonlat(double& x, double& y, double& z, double refLon, double refLat, double refAlt);

// seu x y z -> enu x y z
void seu2enu(double& enuX, double& enuY, double& enuZ, double seuX, double seuY, double seuZ);

// enu x y z -> seu x y z
void enu2seu(double& seuX, double& seuY, double& seuZ, double enuX, double enuY, double enuZ);

// lon lat -> mercator
void lonlat2mercator(double& x, double& y);

// mercator -> lon lat
void mercator2lonlat(double& x, double& y);
}  // namespace coord_trans_api

#endif  // VEHICLEDYNAMICS_SIM_PACK_CORE_COORD_TRANS_H_
