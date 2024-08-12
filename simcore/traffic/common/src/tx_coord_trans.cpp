// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_coord_trans.h"

namespace Coord {
txFloat Trans::sLon = 0.0;
txFloat Trans::sLat = 0.0;
txFloat Trans::sAlt = 0.0;
txFloat Trans::R_global2local[9] = {0};
txFloat Trans::R_local2global[9] = {0};

void Trans::initialize(const txFloat L, const txFloat B, const txFloat H) TX_NOEXCEPT {
  zx_to_dx_matrix(L, B, H, R_local2global);
  dx_to_zx_matrix(L, B, H, R_global2local);
  sLon = L;
  sLat = B;
  sAlt = H;
  lonlat2global(sLon, sLat, sAlt);
}

#if __UseProj__
#  if __TX_Mark__("UTM2WGS84Utility")
Base::txBool UTM2WGS84Utility::Initialize(const Base::txInt utm_zone, const Base::txBool is_north) TX_NOEXCEPT {
  m_last_zone = utm_zone;
  m_last_north = is_north;
  pj_utm = pj_init_plus(MakeUTMZone(m_last_zone, m_last_north).c_str());
  pj_wgs84 = pj_init_plus(m_wgs84.c_str());
  return true;
}

hadmap::txPoint UTM2WGS84Utility::UTM2LonLat(const Base::txFloat x, const Base::txFloat y) TX_NOEXCEPT {
  hadmap::txPoint gps;
  gps.z = 0.0;
  gps.x = x;
  gps.y = y;

  const int code = pj_transform(pj_utm, pj_wgs84, 1, 1, &gps.x, &gps.y, NULL);
  if (0 != code) {
    LOG(INFO) << TX_VARS_NAME("UTM2LonLat.proj4.error", pj_strerrno(code));
  }

  gps.x /= DEG_TO_RAD;
  gps.y /= DEG_TO_RAD;
  gps.z = 0.0;
  return gps;
}

Base::txVec3 UTM2WGS84Utility::LonLat2UTM(const Base::txFloat lon, const Base::txFloat lat) TX_NOEXCEPT {
  Base::txVec3 enu;
  enu.z() = 0.0;
  enu.x() = lon * DEG_TO_RAD;
  enu.y() = lat * DEG_TO_RAD;
  const int code = pj_transform(pj_wgs84, pj_utm, 1, 1, &enu.x(), &enu.y(), NULL);
  if (0 != code) {
    LOG(INFO) << TX_VARS_NAME("LonLat2UTM.proj4.error", pj_strerrno(code));
  }
  return enu;
}
#  endif /*__TX_Mark__("UTM2WGS84Utility")*/
#endif   /*__UseProj__*/

}  // namespace Coord
