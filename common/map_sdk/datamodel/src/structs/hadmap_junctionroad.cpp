// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_junctionroad.h"
#include "common/map_util.h"

namespace hadmap {
bool txJunctionRoad::isJunction() const { return true; }

bool txJunctionRoad::hasTrafficLights() const { return hasLights; }

bool txJunctionRoad::hasStopLine() const { return hasStop; }

DIR_TYPE txJunctionRoad::turnDirection() const { return turnDir; }

std::string txJunctionRoad::getUniqueId() const {
  char index[128];
  sprintf(index, "junc_%s", std::to_string(this->getId()).c_str());
  return std::string(index);
}

std::vector<hadmap::txObjectPtr> txJunctionRoad::getSpecObjs(hadmap::OBJECT_TYPE objType) {
  if (objMap.find(objType) != objMap.end()) {
    return objMap[objType];
  } else {
    return std::vector<hadmap::txObjectPtr>();
  }
}

junctionpkid txJunctionRoad::getJuncId() const { return jId; }

roadpkid txJunctionRoad::fromRoadId() const { return fromRId; }

roadpkid txJunctionRoad::toRoadId() const { return toRId; }

txJunctionRoad& txJunctionRoad::setJuncId(junctionpkid id) {
  jId = id;
  return *this;
}

txJunctionRoad& txJunctionRoad::setFromRoadId(roadpkid id) {
  fromRId = id;
  return *this;
}

txJunctionRoad& txJunctionRoad::setToRoadId(roadpkid id) {
  toRId = id;
  return *this;
}

txJunctionRoad& txJunctionRoad::setTurnDir(DIR_TYPE dir) {
  turnDir = dir;
  return *this;
}

txJunctionRoad& txJunctionRoad::setLightFlag(bool flag) {
  hasLights = flag;
  return *this;
}

txJunctionRoad& txJunctionRoad::setStopLineFlag(bool flag) {
  hasStop = flag;
  return *this;
}

txJunctionRoad& txJunctionRoad::setObj(OBJECT_TYPE type, txObjectPtr objPtr) {
  if (objMap.find(type) == objMap.end()) objMap.insert(std::make_pair(type, txObjects()));
  objMap[type].push_back(objPtr);
  return *this;
}

txJunctionRoad& txJunctionRoad::setObjs(OBJECT_TYPE type, txObjects& objs) {
  if (objMap.find(type) == objMap.end()) {
    objMap.insert(std::make_pair(type, objs));
  } else {
    objMap[type].insert(objMap[type].end(), objs.begin(), objs.end());
  }
  return *this;
}
}  // namespace hadmap
