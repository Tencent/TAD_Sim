// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/base_struct.h"
#include "structs/hadmap_predef.h"
#include "types/map_types.h"

#include "structs/map_structs.h"
namespace hadmap {
// had map lane boundary struct
class TXSIMMAP_API txLaneBoundary {
 private:
  // lane boundary member variables
  struct BoundaryData;
  std::unique_ptr<BoundaryData> instancePtr;

 public:
  // Default constructor
  txLaneBoundary();

  // Copy constructor
  txLaneBoundary(const txLaneBoundary& laneboundary);

  // Destructor
  ~txLaneBoundary();

 public:
  // Functional operations

  // Get the tx data of the lane boundary
  tx_laneboundary_t& getTxData() const;
  // Get the boundary ID
  laneboundarypkid getId() const;

  // Get the SRS
  uint32_t getSrs() const;

  // Get the geometry of the current lane boundary
  const txCurve* getGeometry() const;

  // Get the lane mark
  LANE_MARK getLaneMark() const;

  // Get the lane width
  double getLaneWidth() const;
  double getDoubleInterval() const;
  double getDotSpace() const;
  double getDotOffset() const;
  double getDotLen() const;

 public:
  // Set the SRS
  txLaneBoundary& setSrs(uint32_t srs);

  // Set the boundary ID
  txLaneBoundary& setId(laneboundarypkid id);

  // Set the geometry
  txLaneBoundary& setGeometry(const txCurve* curvePtr);

  txLaneBoundary& setGeometry(const txCurve& curveIns);

  txLaneBoundary& setGeometry(const PointVec& points, CoordType ct);

  // Transfer
  // Only be used when geom is in ENU
  txLaneBoundary& transfer(const txPoint& oldEnuC, const txPoint& newEnuC);

  // Transfer to ENU
  txLaneBoundary& transfer2ENU(const txPoint& enuCenter);

  // Set the lane mark ( SOLID | BROKEN | CURB | ... )
  txLaneBoundary& setLaneMark(LANE_MARK mark);

  // Set the lane width
  txLaneBoundary& setLaneWidth(double w);

  txLaneBoundary& setDoubleInterval(double d);
  txLaneBoundary& setDotSpace(double d);
  txLaneBoundary& setDotOffset(double d);
  txLaneBoundary& setDotLen(double d);

  // Reverse data
  bool reverse();
};
}  // namespace hadmap
