// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/base_struct.h"
#include "structs/hadmap_predef.h"
#include "structs/map_structs.h"

namespace hadmap {
class TXSIMMAP_API txObjectGeom {
 private:
  struct GeomData;
  std::unique_ptr<GeomData> instancePtr;

 public:
  txObjectGeom();

  ~txObjectGeom();

 public:
  // get info of cur obj geom
  tx_object_geom_t getTxData() const;

  // get geom id
  objectgeompkid getId() const;

  // get object geom type
  // point, polyline, polygon
  OBJECT_GEOMETRY_TYPE getType() const;

  // get object style, shape
  // rectangle, triangle, circle ....
  OBJECT_STYLE getStyle() const;

  // get geometry
  const txCurve* getGeometry() const;

  // get main axis
  // only used for road sign | crosswalk | traffic sign
  const txCurve* getMainAxis() const;

  // get color
  uint32_t getColor() const;

  // get geometry text
  // road sign text
  std::string getGeomText() const;

  // get geom5d
  PointVec getGeom5D() const;

 public:
  // set info of cur obj geom

  // set object id
  txObjectGeom& setId(const objectgeompkid& id);

  // set obj geom type
  txObjectGeom& setType(const OBJECT_GEOMETRY_TYPE& type);

  // set obj style
  txObjectGeom& setStyle(const OBJECT_STYLE& style);

  // set color
  txObjectGeom& setColor(const uint32_t& color);

  // set geometry
  txObjectGeom& setGeometry(const txCurve* curvePtr);

  txObjectGeom& setGeometry(const txCurve& curveIns);

  txObjectGeom& setGeometry(const PointVec& points, CoordType ct);

  // set geometry text
  txObjectGeom& setGeomText(const std::string& text);

  // only be used when geom is in ENU
  txObjectGeom& transfer(const txPoint& oldEnuC, const txPoint& newEnuC);

  // transfer to enu
  txObjectGeom& transfer2ENU(const txPoint& enuCenter);

 private:
  // generate main axis
  bool genMainAxis();

  // transfer coord, only used when geom is in enu
  bool transferMainAxis(const txPoint& old_enu_c, const txPoint& new_enu_c);

  // transfer wgs84 -> enu
  bool transferMainAxis2ENU(const txPoint& enu_center);
};
}  // namespace hadmap
