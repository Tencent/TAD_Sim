// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/hadmap_object.h"
#include "structs/hadmap_road.h"
#include "structs/hadmap_section.h"
#include "types/map_types.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hadmap {
class TXSIMMAP_API txJunctionRoad : public hadmap::txRoad {
 public:
  // Check if the road is a junction road
  bool isJunction() const;

 public:
  // Default constructor
  txJunctionRoad() : hadmap::txRoad(), turnDir(DIR_None), hasLights(false), hasStop(true) {}

  // Constructor with road ID and name
  txJunctionRoad(roadpkid id, const std::string& name)
      : hadmap::txRoad(id, name), turnDir(DIR_None), hasLights(false), hasStop(true) {}

  // Constructor with a base road
  explicit txJunctionRoad(const hadmap::txRoad& road)
      : hadmap::txRoad(road), turnDir(DIR_None), hasLights(false), hasStop(true) {}

  // Destructor
  ~txJunctionRoad() {}

 public:
  // Check if the road has traffic lights
  bool hasTrafficLights() const;

  // Check if the road has a stop line
  bool hasStopLine() const;

  // Get the turn direction of the road
  DIR_TYPE turnDirection() const;

  // Get the specific objects of the given type
  txObjects getSpecObjs(hadmap::OBJECT_TYPE objType);

  // Get the unique ID of the road
  std::string getUniqueId() const;

  // Get the junction ID of the road
  junctionpkid getJuncId() const;

  // Get the from road ID of the road
  roadpkid fromRoadId() const;

  // Get the to road ID of the road
  roadpkid toRoadId() const;

 public:
  // Set the junction ID of the road
  txJunctionRoad& setJuncId(junctionpkid id);

  // Set the from road ID of the road
  txJunctionRoad& setFromRoadId(roadpkid id);

  // Set the to road ID of the road
  txJunctionRoad& setToRoadId(roadpkid id);

  // Set the turn direction of the road
  txJunctionRoad& setTurnDir(DIR_TYPE dir);

  // Set the traffic light flag of the road
  txJunctionRoad& setLightFlag(bool flag);

  // Set the stop line flag of the road
  txJunctionRoad& setStopLineFlag(bool flag);

  // Set an object of the given type to the road
  txJunctionRoad& setObj(OBJECT_TYPE type, txObjectPtr objPtr);

  // Set objects of the given type to the road
  txJunctionRoad& setObjs(OBJECT_TYPE type, txObjects& objs);

 private:
  // Map of objects with their types
  std::unordered_map<int, std::vector<hadmap::txObjectPtr> > objMap;
  // Turn direction of the road
  DIR_TYPE turnDir;
  // Flag indicating if the road has traffic lights
  bool hasLights;
  // Flag indicating if the road has a stop line
  bool hasStop;
  // Junction ID of the road
  junctionpkid jId;
  // From road ID of the road
  roadpkid fromRId;
  // To road ID of the road
  roadpkid toRId;
};

// Type definition for a shared pointer to a txJunctionRoad
typedef std::shared_ptr<txJunctionRoad> txJunctionRoadPtr;
}  // namespace hadmap
