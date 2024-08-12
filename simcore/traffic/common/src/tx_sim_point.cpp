// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_sim_point.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
// #include "coord_trans.h"
#include "tx_coord_trans.h"
#include "tx_string_utils.h"
#include "tx_timer_on_cpu.h"

TX_NAMESPACE_OPEN(Coord)

hadmap::txPoint txReferencePoint::s_reference_wgs84;
#if __UseProj__
Coord::UTM2WGS84Utility txReferencePoint::m_proj_api;
#endif /*__UseProj__*/
void txReferencePoint::SetReferencePoint(const hadmap::txPoint& _wgs84) TX_NOEXCEPT {
  s_reference_wgs84 = _wgs84;
  Coord::Trans::initialize(Ref_Lon(), Ref_Lat(), Ref_Alt());
#if __UseProj__
  m_proj_api.Initialize(m_proj_api.Lon2Zone(Ref_Lon()), m_proj_api.IsNorthEarth(Ref_Lat()));
#endif /*__UseProj__*/
}

hadmap::txPoint txReferencePoint::enu2wgs84(const Base::txVec3& _enu3d) TX_NOEXCEPT {
#if __UseProj__
  return m_proj_api.UTM2LonLat(_enu3d.x(), _enu3d.y());
#else
  hadmap::txPoint _wgs84;
  __Lon__(_wgs84) = _enu3d.x();
  __Lat__(_wgs84) = _enu3d.y();
  __Alt__(_wgs84) = _enu3d.z();
  Coord::enu2lonlat(__Lon__(_wgs84), __Lat__(_wgs84), __Alt__(_wgs84), Ref_Lon(), Ref_Lat(), Ref_Alt());
  return _wgs84;
#endif
}

Base::txVec3 txReferencePoint::wgs842enu(const hadmap::txPoint& _wgs84) TX_NOEXCEPT {
#if __UseProj__
  return m_proj_api.LonLat2UTM(__Lon__(_wgs84), __Lat__(_wgs84));
#else
  Base::txVec3 _enu3d;
  _enu3d.x() = __Lon__(_wgs84);
  _enu3d.y() = __Lat__(_wgs84);
  _enu3d.z() = __Alt__(_wgs84) * 0;
  Coord::lonlat2enu(_enu3d.x(), _enu3d.y(), _enu3d.z(), Ref_Lon(), Ref_Lat(), Ref_Alt());
  return _enu3d;
#endif
}

txWGS84::txWGS84(txWGS84&& _other) noexcept {
  m_wgs84 = _other.m_wgs84;
  m_opEnu = _other.m_opEnu;

  _other.m_wgs84;
  _other.m_opEnu = boost::none;
}

txWGS84& txWGS84::operator=(txWGS84&& _other) noexcept {
  if (this != &_other) {
    m_wgs84 = _other.m_wgs84;
    m_opEnu = _other.m_opEnu;

    _other.m_wgs84;
    _other.m_opEnu = boost::none;
  }
  return *this;
}

txWGS84& txWGS84::operator=(const txWGS84& _other) noexcept {
  if (this != &_other) {
    m_wgs84 = _other.m_wgs84;
    m_opEnu = _other.m_opEnu;
  }
  return *this;
}

void txWGS84::TranslateLocalPos(const Base::txVec3& _step) TX_NOEXCEPT { FromENU(ToENU().ENU() + _step); }

txENU txWGS84::GetENU() const TX_NOEXCEPT {
  if (hasENU()) {
    return txENU(*m_opEnu);
  } else {
    return txENU();
  }
}

txENU txWGS84::ToENU() TX_NOEXCEPT {
  if (m_opEnu) {
    return txENU(*m_opEnu, m_wgs84);
  } else {
    m_opEnu = wgs842enu(m_wgs84);
    return txENU(*m_opEnu, m_wgs84);
  }
}

Base::txString txWGS84::StrWGS84() const TX_NOEXCEPT { return Utils::ToString(m_wgs84); }
Base::txString txWGS84::StrENU() const TX_NOEXCEPT {
  if (m_opEnu) {
    return Utils::ToString(*m_opEnu);
  } else {
    return "";
  }
}

Base::txFloat txWGS84::EuclideanDistanceByWGS84WithoutAlt(const txWGS84& _pt1, const txWGS84& _pt2) TX_NOEXCEPT {
  return EuclideanDistanceByWGS84WithoutAlt(_pt1.WGS84(), _pt2.WGS84());
}

Base::txFloat txWGS84::EuclideanDistanceByWGS84WithoutAlt(const hadmap::txPoint& _pt1,
                                                          const hadmap::txPoint& _pt2) TX_NOEXCEPT {
  namespace bg = boost::geometry;
  using point = bg::model::point<Base::txFloat, 2, bg::cs::geographic<bg::degree>>;
  using stype = boost::geometry::srs::spheroid<Base::txFloat>;
  return bg::distance(point(_pt1.x, _pt1.y), point(_pt2.x, _pt2.y), bg::strategy::distance::vincenty<stype>());
}

txWGS84 txENU::GetWGS84() const TX_NOEXCEPT {
  if (hasWGS84()) {
    return txWGS84(*m_opWGS84);
  } else {
    return txWGS84();
  }
}

txWGS84 txENU::ToWGS84() TX_NOEXCEPT {
  if (m_opWGS84) {
    return txWGS84(*m_opWGS84, m_enu);
  } else {
    m_opWGS84 = enu2wgs84(m_enu);
    return txWGS84(*m_opWGS84, m_enu);
  }
}

Base::txString txENU::StrWGS84() const TX_NOEXCEPT {
  if (m_opWGS84) {
    return Utils::ToString(*m_opWGS84);
  } else {
    return "";
  }
}
Base::txString txENU::StrENU() const TX_NOEXCEPT { return Utils::ToString(m_enu); }

Base::txFloat txENU::EuclideanDistanceLocal_3D(const txENU& v0, const txENU& v1) TX_NOEXCEPT {
  return (v0.ENU() - v1.ENU()).norm();
}

Base::txFloat txENU::EuclideanDistanceLocal_2D(const txENU& v0, const txENU& v1) TX_NOEXCEPT {
  return (Utils::Vec3_Vec2(v0.ENU()) - Utils::Vec3_Vec2(v1.ENU())).norm();
}

txENU::txENU(txENU&& _other) noexcept {
  m_enu = _other.m_enu;
  m_opWGS84 = _other.m_opWGS84;

  _other.m_enu.setZero();
  _other.m_opWGS84 = boost::none;
}

txENU& txENU::operator=(txENU&& _other) noexcept {
  if (this != &_other) {
    m_enu = _other.m_enu;
    m_opWGS84 = _other.m_opWGS84;

    _other.m_enu.setZero();
    _other.m_opWGS84 = boost::none;
  }
  return *this;
}

txENU& txENU::operator=(const txENU& _other) noexcept {
  if (this != &_other) {
    m_enu = _other.m_enu;
    m_opWGS84 = _other.m_opWGS84;
  }
  return *this;
}

TX_NAMESPACE_CLOSE(Coord)
