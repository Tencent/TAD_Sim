/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  SimMapsdk
 * @author   longerding@tencent
 * Modify history:
 ******************************************************************************/
#pragma once

#include <memory>
#include <string>

#include "structs/hadmap_header.h"
#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_road.h"

namespace hadmap {
class DataParse;
using DataParsePtr = std::shared_ptr<DataParse>;
using AreaPointVec = std::vector<std::tuple<hadmap::txPoint, hadmap::txPoint>>;
class TXSIMMAP_API DataParse {
 public:
  enum MapDataType {
    OPENDRIVE = 1,
  };

 public:
  explicit DataParse(MapDataType type) : dataType(type), parseFlag(false) {}

  virtual ~DataParse() {}

 public:
  MapDataType getDataType() { return dataType; }

  // parse had map data from opendrive / nds / json
  // parse success, return true
  virtual bool parse(const std::string& dataPath) = 0;

  // parse had map data from opendrive
  // parse success, return true
  virtual bool parseArea(const std::string& dataPath, AreaPointVec ptr) = 0;

  // get had map data after parsing complete
  // parse failed, return false
  virtual bool getData(txRoads& roads, txLaneLinks& links, txObjects& objs, txJunctions& juncs, txOdHeaderPtr& header,
                       txPoint& refPoint, bool doubleRoad) = 0;

 protected:
  MapDataType dataType;
  bool parseFlag;
};
}  // namespace hadmap
