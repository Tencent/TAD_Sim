// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_obb.h"
#include "tx_sim_point.h"
#include "tx_string_utils.h"
TX_NAMESPACE_OPEN(Geometry)

OBB2D::OBB2D(const Base::txVec2& _c, const Base::txVec2& _FrontAxis, const Base::txVec2& _RightAxis,
             const Base::txFloat _FrontHalfSize, const Base::txFloat _RightHalfSize) {
  center = _c;
  Axis[0] = _FrontAxis;
  Axis[1] = _RightAxis;
  halfSize[0] = _FrontHalfSize;
  halfSize[1] = _RightHalfSize;
}

OBB2D::PolygonArray OBB2D::GetPolygon() const TX_NOEXCEPT {
  PolygonArray retVec;
  retVec[0] = (center + Axis[0] * halfSize[0] * (-1.0) + Axis[1] * halfSize[1] * (-1.0)); /*(-1, -1)*/
  retVec[1] = (center + Axis[0] * halfSize[0] * (1.0) + Axis[1] * halfSize[1] * (-1.0));  /*( 1, -1)*/
  retVec[2] = (center + Axis[0] * halfSize[0] * (1.0) + Axis[1] * halfSize[1] * (1.0));   /*( 1,  1)*/
  retVec[3] = (center + Axis[0] * halfSize[0] * (-1.0) + Axis[1] * halfSize[1] * (1.0));  /*(-1,  1)*/
  return retVec;
}

Base::txString OBB2D::Str() const TX_NOEXCEPT {
  std::stringstream ss;
  Coord::txWGS84 wgs84;
  const PolygonArray& refPolygon = GetPolygon();
  for (const auto& refPt : refPolygon) {
    wgs84.FromENU(__East__(refPt), __North__(refPt));
    ss << wgs84.StrWGS84();
  }
  return ss.str();
}

TX_NAMESPACE_CLOSE(Geometry)
