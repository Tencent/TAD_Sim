// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

#include "common/macros.h"
#include "structs/hadmap_predef.h"
#include "transmission/CJsonObject.hpp"
#include "types/map_types.h"

namespace hadmap {
struct TXSIMMAP_API PostParam {
  enum ParamType { ROAD_ID = 1, SECT_ID = 2, LANE_ID = 3, BOUN_ID = 4, OBJ_TYPE = 5, POS = 6 };

 protected:
  ParamType type;

 public:
  explicit PostParam(ParamType t) : type(t) {}

  ParamType getType() const { return type; }

  std::string typeStr() const {
    switch (type) {
      case ROAD_ID:
        return std::string("roadid");
        break;
      case SECT_ID:
        return std::string("sectid");
        break;
      case LANE_ID:
        return std::string("laneid");
        break;
      case BOUN_ID:
        return std::string("boundaryid");
        break;
      case OBJ_TYPE:
        return std::string("objtype");
        break;
      case POS:
        return std::string("pos");
        break;
      default:
        return std::string("");
        break;
    }
  }

  virtual void addToJson(neb::CJsonObject& jAry) const = 0;
};

using PostParamPtr = std::shared_ptr<PostParam>;

struct TXSIMMAP_API RoadIdParam : public PostParam {
 private:
  roadpkid roadId;

 public:
  explicit RoadIdParam(const roadpkid& rid) : PostParam(ROAD_ID), roadId(rid) {}

  void addToJson(neb::CJsonObject& jAry) const { jAry.Add(static_cast<uint64>(roadId)); }
};

struct TXSIMMAP_API LaneIdParam : public PostParam {
 private:
  txLaneId laneId;

 public:
  explicit LaneIdParam(const txLaneId& id) : PostParam(LANE_ID), laneId(id) {}

  void addToJson(neb::CJsonObject& jAry) const {
    neb::CJsonObject jObj;
    jObj.Add(static_cast<uint64>(laneId.roadId));
    jObj.Add(static_cast<uint64>(laneId.sectionId));
    jObj.Add(static_cast<int64>(laneId.laneId));

    jAry.Add(jObj);
  }
};

struct TXSIMMAP_API BoundaryIdParam : public PostParam {
 private:
  laneboundarypkid boundaryId;

 public:
  explicit BoundaryIdParam(const laneboundarypkid& id) : PostParam(BOUN_ID), boundaryId(id) {}

  void addToJson(neb::CJsonObject& jAry) const { jAry.Add(static_cast<uint64>(boundaryId)); }
};

struct TXSIMMAP_API ObjTypeParam : public PostParam {
 private:
  OBJECT_TYPE objType;

 public:
  explicit ObjTypeParam(const OBJECT_TYPE& t) : PostParam(OBJ_TYPE), objType(t) {}

  void addToJson(neb::CJsonObject& jAry) const { jAry.Add(static_cast<uint32>(objType)); }
};

struct TXSIMMAP_API PosParam : public PostParam {
 private:
  double lon;
  double lat;

 public:
  PosParam(const double& x, const double& y) : PostParam(POS), lon(x), lat(y) {}

  void addToJson(neb::CJsonObject& jAry) const {
    neb::CJsonObject jObj;
    jObj.Add("lon", lon);
    jObj.Add("lat", lat);
    // jObj.Add( lon );
    // jObj.Add( lat );

    jAry.Add(jObj);
  }
};

TXSIMMAP_API void generatePostJson(const std::vector<PostParamPtr>& params, std::string& jsonStr);
}  // namespace hadmap
