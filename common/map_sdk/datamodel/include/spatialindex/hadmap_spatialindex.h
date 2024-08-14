// Copyright 2024 Tencent Inc. All rights reserved.
//

// This is a C++ header file that defines a class called hadmap::txSpatialIndex for handling spatial data in maps.
// This class provides methods for inserting, deleting, and searching map data.

#pragma once

#include "common/macros.h"
#include "structs/hadmap_curve.h"
#include "types/map_defs.h"

#include <memory>
#include <string>

namespace hadmap {
// Enumeration for types of spatial data
enum SpatialDataType {
  LANE_SP = 1,  // Lane data
  LINK_SP = 2,  // Link data
  OBJ_SP = 3    // Object data
};

// Struct representing a rectangular envelope (bounding box)
struct TXSIMMAP_API txEnvelope {
  double bMin[2];  // Minimum coordinates of the envelope
  double bMax[2];  // Maximum coordinates of the envelope

  txEnvelope() {}

  // Constructor with minimum and maximum coordinates
  txEnvelope(double min0, double min1, double max0, double max1) {
    bMin[0] = min0;
    bMin[1] = min1;
    bMax[0] = max0;
    bMax[1] = max1;
  }

  // Copy constructor
  txEnvelope(const txEnvelope& envelope) { memcpy(this, &envelope, sizeof(double) * 4); }

  // Assignment operator
  txEnvelope& operator=(const txEnvelope& envelope) {
    memcpy(this, &envelope, sizeof(double) * 4);
    return *this;
  }
};

// Class for handling spatial data in maps
class TXSIMMAP_API txSpatialIndex {
 private:
  struct SpatialData;
  std::unique_ptr<SpatialData> spDataPtr;

 private:
  txSpatialIndex(const txSpatialIndex& spIndex);

 public:
  txSpatialIndex();  // Default constructor

  ~txSpatialIndex();  // Destructor

 public:
  // Insert spatial data
  bool insert(SpatialDataType type, const txLineCurve* curvePtr, pkid_t curveIndex);

  // Remove spatial data
  bool remove(SpatialDataType type, pkid_t curveIndex);

  bool removeAll(SpatialDataType type);

  // Search spatial data
  bool search(SpatialDataType type, const txEnvelope& envelope, std::vector<pkid_t>& curveIndexes);

  // Search spatial data and get point index in envelope
  bool search(SpatialDataType type, const txEnvelope& envelope, std::vector<pkid_t>& curveIndexes,
              std::vector<std::pair<size_t, size_t>>& pointRanges);
};
}  // namespace hadmap
