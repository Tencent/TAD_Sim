// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "structs/hadmap_object.h"
#include <assert.h>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>
#include "common/coord_trans.h"
#include "structs/hadmap_curve.h"
#include "structs/map_structs.h"

namespace hadmap {
struct txObject::ObjData {
 public:
  tx_object_t data;

  tx_od_object_t od_data;

  txObjGeoms geoms;

  txLineCurvePtr repeat;

  std::vector<txLaneId> relIds;

  std::map<std::string, std::string> userData;

 public:
  ObjData() {}

  ~ObjData() {}
};

txObject::txObject() : instancePtr(new ObjData) {}

txObject::~txObject() {}

tx_object_t txObject::getTxData() const { return instancePtr->data; }

tx_od_object_t txObject::getOdData() const { return instancePtr->od_data; }

objectpkid txObject::getId() const { return instancePtr->data.pkid; }

OBJECT_TYPE txObject::getObjectType() const { return (OBJECT_TYPE)instancePtr->data.type; }

OBJECT_SUB_TYPE txObject::getObjectSubType() const { return OBJECT_SUB_TYPE(instancePtr->data.subtype); }

std::string txObject::getName() const { return instancePtr->data.name; }

void txObject::getReliedLaneIds(std::vector<txLaneId>& ids) const {
  ids.assign(instancePtr->relIds.begin(), instancePtr->relIds.end());
}

size_t txObject::getGeomSize() const { return instancePtr->geoms.size(); }

txObjGeomPtr txObject::getGeom(size_t index) const {
  if (index >= instancePtr->geoms.size()) throw std::runtime_error("getGeom error, index out of range");
  return instancePtr->geoms[index];
}

void txObject::transform2OdData() {
  tx_od_object_t data = instancePtr->od_data;
  data.object_pkid = this->getId();
  data.type = instancePtr->data.type;
  data.subtype = instancePtr->data.subtype;
  strcpy(data.name, instancePtr->data.name);
  // data.orientation = 1;
  // data.validlength = 0.0;
  // data.radius = 0.0;
  // this->getRPY(data.roll, data.pitch, data.hdg);
  this->getLWH(data.length, data.width, data.height);
  data.roadid = instancePtr->data.roadid;
  // if (instancePtr->geoms.size() > 0)
  // {
  //  txPoint point = this->getPos();
  //  data.s = point.x;
  //  data.t = point.y;
  // }
  std::map<std::string, std::string> userData;
  this->getUserData(userData);
  std::string strUserData;
  for (auto it : userData) {
    strUserData += (it.first + ":" + it.second + ";");
  }
  strcpy(data.userdata, strUserData.c_str());

  instancePtr->od_data = data;
}

void txObject::setTxOdData(tx_od_object_t& odData) {
  instancePtr->od_data = odData;
  instancePtr->data.type = instancePtr->od_data.type;
  instancePtr->data.subtype = instancePtr->od_data.subtype;
  this->setLWH(instancePtr->od_data.length, instancePtr->od_data.width, instancePtr->od_data.height);
  // this->setRPY(instancePtr->od_data.roll, instancePtr->od_data.pitch, instancePtr->od_data.hdg);
  if (this->getGeomSize() > 0 && this->getGeom()->getGeometry()) {
    this->setPos(this->getGeom()->getGeometry()->getPoint(0).x, this->getGeom()->getGeometry()->getPoint(0).y,
                 instancePtr->od_data.zoffset);
  }
  std::string userdata = odData.userdata;
  // set userdata
  std::string::size_type pos;
  std::vector<std::string> result;
  int size = userdata.size();
  for (int i = 0; i < size; i++) {
    pos = userdata.find(";", i);
    if (pos < size) {
      std::string s = userdata.substr(i, pos - i);
      if (!s.empty()) result.push_back(s);
      i = pos;
    }
  }
  std::map<std::string, std::string> userDataVec;
  for (auto it : result) {
    int pos = it.find(":");
    std::string key = it.substr(0, pos);
    std::string value = it.substr(pos + 1, it.size() - pos - 1);
    userDataVec[key] = value;
  }
  this->setUserData(userDataVec);
}

txLineCurvePtr txObject::getRepeat() const { return instancePtr->repeat; }

txObject& txObject::setId(objectpkid id) {
  instancePtr->data.pkid = id;
  return *this;
}

txObject& txObject::setObjectType(OBJECT_TYPE type) {
  instancePtr->data.type = type;
  return *this;
}

txObject& txObject::setName(const std::string& name) {
  strcpy(instancePtr->data.name, name.c_str());
  return *this;
}

txObject& txObject::addReliedLaneId(const txLaneId& laneId) {
  for (size_t i = 0; i < instancePtr->relIds.size(); ++i)
    if (laneId == instancePtr->relIds[i]) return *this;
  instancePtr->relIds.push_back(laneId);
  return *this;
}

txObject& txObject::setReliedLaneIds(const std::vector<txLaneId>& laneIds) {
  instancePtr->relIds.clear();
  instancePtr->relIds.assign(laneIds.begin(), laneIds.end());
  return *this;
}

txObject& txObject::addGeom(const txObjGeomPtr& geomPtr) {
  instancePtr->geoms.push_back(geomPtr);
  return *this;
}

txObject& txObject::setRepeat(txLineCurvePtr geomPtr) {
  instancePtr->repeat = geomPtr;
  return *this;
}

txObject& txObject::setST(const double s, const double t) {
  instancePtr->data.s = s;
  instancePtr->data.t = t;
  return *this;
}

txObject& txObject::setOdRoadId(const int& id) {
  instancePtr->data.roadid = id;
  instancePtr->od_data.roadid = id;
  return *this;
}

txObject& txObject::setLaneLinkid(const int& id) {
  instancePtr->od_data.lanelinkid = id;
  return *this;
}

// only be used when geom is in ENU
txObject& txObject::transfer(const txPoint& oldEnuC, const txPoint& newEnuC) {
  for (auto& geomPtr : instancePtr->geoms) geomPtr->transfer(oldEnuC, newEnuC);
  return *this;
}

// transfer to enu
txObject& txObject::transfer2ENU(const txPoint& enuCenter) {
  for (auto& geomPtr : instancePtr->geoms) geomPtr->transfer2ENU(enuCenter);
  return *this;
}

roadpkid txObject::getRoadId() const { return instancePtr->data.roadid; }

txPoint txObject::getPos() const { return txPoint(instancePtr->data.x, instancePtr->data.y, instancePtr->data.z); }

void txObject::getST(double& s, double& t) const {
  s = instancePtr->data.s;
  t = instancePtr->data.t;
}

void txObject::getLWH(double& l, double& w, double& h) const {
  l = instancePtr->data.length;
  w = instancePtr->data.width;
  h = instancePtr->data.height;
}

void txObject::getRPY(double& r, double& p, double& y) const {
  r = instancePtr->data.roll;
  p = instancePtr->data.pitch;
  y = instancePtr->data.yaw;
}

void txObject::getRawTypeString(std::string& type, std::string& subtype) const {
  type = instancePtr->data.rawtype;
  subtype = instancePtr->data.rawsubtype;
}

void txObject::getUserData(std::map<std::string, std::string>& userData) const { userData = instancePtr->userData; }

txObject& txObject::setData(tx_object_t data) {
  instancePtr->data = data;
  return *this;
}

// set pole type
int setPoleType(const std::string name, const std::string type, uint32_t& ntype, uint32_t& nsubtype) {
  if (name == "Vertical_Pole" || name == "UprightPole") {
    ntype = OBJECT_TYPE_Pole;
    nsubtype = POLE_VERTICAL;
  } else if (name == "Cross_Pole" || name == "SingleArmPole") {
    ntype = OBJECT_TYPE_Pole;
    nsubtype = POLE_CROSS;
  } else if (name == "Cantilever_Pole") {
    ntype = OBJECT_TYPE_Pole;
    nsubtype = POLE_Cantilever;
  } else if (name == "Pillar_Pole_6m") {
    ntype = OBJECT_TYPE_Pole;
    nsubtype = POLE_Pillar_6m;
  } else if (name == "Pillar_Pole_3m") {
    ntype = OBJECT_TYPE_Pole;
    nsubtype = POLE_Pillar_3m;
  } else if (type == "pole") {
    ntype = OBJECT_TYPE_Pole;
    nsubtype = POLE_VERTICAL;
  } else {
    return -1;
  }
  return 0;
}

// set trafficlight type
int setTrafficType(const std::string name, const std::string& type, const std::string subType, int st, uint32_t& ntype,
                   uint32_t& nsubtype) {
  if (type == "1000001" || (subType == "l2" || subType == "l3") ||
      (name == "verticalOmnidirectionalLight")) {  // TrafficLights
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_VERTICAL_ALLDIRECT;
  } else if ((type == "1000011" && subType == "60") || (name == "verticalStraightRoundLight")) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_VERTICAL_STRAIGHTROUND;
  } else if ((type == "1000011" && subType == "10") || (name == "verticalLeftTurnLight")) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_VERTICAL_LEFTTURN;
  } else if ((type == "1000011" && subType == "30") || (name == "verticalStraightLight")) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_VERTICAL_STRAIGHT;
  } else if ((type == "1000011" && subType == "20") || (name == "verticalRightTurnLight")) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_VERTICAL_RIGHTTURN;
  } else if ((type == "1000011" && subType == "70") || (name == "verticalUTurnLight")) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_VERTICAL_UTURN;
  } else if ((type == "1000002") || (name == "verticalPedestrianLight")) {  // 人行灯
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_HORIZON_PEDESTRIAN;
  } else if ((type == "1000003" || ((subType == "l1")) || (name == "horizontalOmnidirectionalLight"))) {  // 人行灯
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_HORIZON_ALLDIRECT;
  } else if ((type == "1000021" && subType == "60") || (name == "horizontalStraightRoundLight")) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_HORIZON_STRAIGHTROUND;
  } else if ((type == "1000021" && subType == "10") || (name == "horizontalRightTurnLight")) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_HORIZON_LEFTTURN;
  } else if ((type == "1000021" && subType == "30") || (name == "horizontalStraightRoundLight")) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_HORIZON_STRAIGHT;
  } else if ((type == "1000021" && subType == "20") || (name == "horizontalStraightRoundLight")) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_HORIZON_RIGHTTURN;
  } else if ((type == "1000021" && subType == "70") || (name == "horizontalStraightRoundLight")) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_HORIZON_UTURN;
  } else if (type == "1000013") {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_BICYCLELIGHT;
  } else if (type == "1000009") {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_TWOCOLOR;
  } else if ((type == "TrafficLights") || type == "trafficLight" || st == OBJECT_SOURCE_SIGNAL_DYNAMIC) {
    ntype = OBJECT_TYPE_TrafficLights;
    nsubtype = LIGHT_VERTICAL_ALLDIRECT;
  } else {
    return -1;
  }
  return 0;
}

// set trafficlight type
int setSignType(const std::string name, const std::string& type, const std::string subType, int st, uint32_t& ntype,
                uint32_t& nsubtype) {
  ntype = OBJECT_TYPE_TrafficSign;
  if (type == "1010103200001111" || type == "1010103200001110" || subType == "w30") {
    nsubtype = SIGN_WARNING_SLOWDOWN;
  } else if (type == "1010100211001111" || type == "1010100211001110" || subType == "w43") {
    nsubtype = SIGN_WARNING_TURNLEFT;
  } else if (type == "1010100212001111" || type == "1010100212001110" || subType == "w42") {
    nsubtype = SIGN_WARNING_TURNRIGHT;
  } else if (type == "1010100121001111" || type == "1010100121001110" || subType == "w22") {
    nsubtype = SIGN_WARNING_T_RIGHT;
  } else if (type == "1010100123001111" || type == "1010100123001110" || subType == "w20") {
    nsubtype = SIGN_WARNING_T_DOWN;
  } else if (type == "1010100111001111" || type == "1010100111001110" || subType == "w13") {
    nsubtype = SIGN_WARNING_CROSS;
  } else if (type == "1010101100001111" || type == "1010101100001110" || subType == "w55") {
    nsubtype = SIGN_WARNING_CHILD;
  } else if (type == "1010100511001111" || type == "1010100511001110" || subType == "w31") {
    nsubtype = SIGN_WARNING_UP;
  } else if (type == "1010103500001111" || type == "1010103500001110" || subType == "w32") {
    nsubtype = SIGN_WARNING_BUILDING;
  } else if (type == "1010100711001111" || type == "1010100711001110" || subType == "w18") {
    nsubtype = SIGN_WARNING_NARROW_LEFT;
  } else if (type == "1010100713001111" || type == "1010100713001110" || subType == "w8") {
    nsubtype = SIGN_WARNING_NARROW_BOTH;
  } else if (type == "1010102812001111" || type == "1010102812001110" || subType == "w40") {
    nsubtype = SIGN_WARNING_RAILWAY;
  } else if (type == "1010100122001111" || type == "1010100122001110" || subType == "w21") {
    nsubtype = SIGN_WARNING_T_LEFT;
  } else if (type == "1010101811001111" || type == "1010101811001110" || subType == "w1") {
    nsubtype = SIGN_WARNING_STEEP_LEFT;
  } else if (type == "1010101812001111" || type == "1010101812001110" || subType == "w2") {
    nsubtype = SIGN_WARNING_STEEP_RIGHT;
  } else if (type == "1010102000001111" || type == "1010102000001110" || subType == "w3") {
    nsubtype = SIGN_WARNING_VILLAGE;
  } else if (type == "1010101911001111" || type == "1010101911001110" || subType == "w4") {
    nsubtype = SIGN_WARNING_DIKE_LEFT;
  } else if (type == "1010101912001111" || type == "1010101912001110" || subType == "w5") {
    nsubtype = SIGN_WARNING_DIKE_RIGHT;
  } else if (type == "1010104012001111" || type == "1010104012001110" || subType == "w6") {
    nsubtype = SIGN_WARAING_T_CROSSED;
  } else if (type == "1010102200001111" || type == "1010102200001110" || subType == "w7") {
    nsubtype = SIGN_WARAING_FERRY;
  } else if (type == "1010101512001111" || type == "1010101512001110" || subType == "w9") {
    nsubtype = SIGN_WARAING_FALL_ROCK;
  } else if (type == "1010100311001111" || type == "1010100311001110" || subType == "w10") {
    nsubtype = SIGN_WARAING_REVERSE_CURVE_LEFT;
  } else if (type == "1010100312001111" || type == "1010100312001110" || subType == "w11") {
    nsubtype = SIGN_WARAING_REVERSE_CURVE_RIGHT;
  } else if (type == "1010102700001111" || type == "1010102700001110" || subType == "w12") {
    nsubtype = SIGN_WARAING_WATER_PAVEMENT;
  } else if (type == "1010100112001111" || type == "1010100112001110" || subType == "w14") {
    nsubtype = SIGN_WARNING_T_BOTH;
  } else if (type == "1010100131001111" || type == "1010100131001110" || subType == "w15") {
    nsubtype = SIGN_WARNING_JOIN_LEFT;
  } else if (type == "1010100132001111" || type == "1010100132001110" || subType == "w16") {
    nsubtype = SIGN_WARNING_JOIN_RIGHT;
  } else if (type == "1010100134001111" || type == "1010100134001110" || subType == "w17") {
    nsubtype = SIGN_WARNING_Y_LEFT;
  } else if (type == "1010100141001111" || type == "1010100134001110" || subType == "w23") {
    nsubtype = SIGN_WARNING_CIRCLE_CROSS;
  } else if (type == "1010100400001111" || type == "1010100400001110" || subType == "w24") {
    nsubtype = SIGN_WARNING_CURVE_AHEAD;
  } else if (type == "1010100600001111" || type == "1010100600001110" || subType == "w25") {
    nsubtype = SIGN_WARNING_LONG_DESCENT;
  } else if (type == "1010102400001111" || type == "1010102400001110" || subType == "w26") {
    nsubtype = SIGN_WARNING_ROUGH_ROAD;
  } else if (type == "1010104312001111" || type == "1010104312001110" || subType == "w27") {
    nsubtype = SIGN_WARNING_SNOW;
  } else if (type == "1010103000001111" || type == "1010103000001110" || subType == "w52") {
    nsubtype = SIGN_WARNING_DISABLE;
  } else if (type == "1010101300001111" || type == "1010101300001110" || subType == "w36") {
    nsubtype = SIGN_WARNING_ANIMALS;
  } else if (type == "1010103100001111" || type == "1010103100001110" || subType == "w34") {
    nsubtype = SIGN_WARNING_ACCIDENT;
  } else if (type == "1010103800001111" || type == "1010103800001110" || subType == "w53") {
    nsubtype = SIGN_WARNING_TIDALBU_LANE;
  } else if (type == "1010104314001111" || type == "1010104314001110" || subType == "w51") {
    nsubtype = SIGN_WARNING_BAD_WEATHER;
  } else if (type == "1010102600001111" || type == "1010102600001110" || subType == "w28") {
    nsubtype = SIGN_WARNING_LOWLYING;
  } else if (type == "1010102500001111" || type == "1010102500001110" || subType == "w29") {
    nsubtype = SIGN_WARNING_HIGHLYING;
  } else if (type == "1010100512001111" || type == "1010100512001110" || subType == "w41") {
    nsubtype = SIGN_WARNING_DOWNHILL;
  } else if (type == "1010104400001111" || type == "1010104400001110" || subType == "w67") {
    nsubtype = SIGN_WARNING_QUEUESLIKELY;
  } else if (type == "1010104011001111" || type == "1010104011001110" || subType == "w33") {
    nsubtype = SIGN_WARNING_CROSS_PLANE;
  } else if (type == "1010102100001111" || type == "1010102100001110" || subType == "w37") {
    nsubtype = SIGN_WARNING_TUNNEL;
  } else if (type == "1010103700001111" || type == "1010103700001110" || subType == "w38") {
    nsubtype = SIGN_WARNING_TUNNEL_LIGHT;
  } else if (type == "1010102300001111" || type == "1010102300001110" || subType == "w39") {
    nsubtype = SIGN_WARNING_HUMPBACK_BRIDGE;
  } else if (type == "1010100712001111" || type == "1010100712001110" || subType == "w47") {
    nsubtype = SIGN_WARNING_NARROW_RIGHT;
  } else if (type == "1010102900001111" || type == "1010102900001110" || subType == "w56") {
    nsubtype = SIGN_WARNING_NON_MOTOR;
  } else if (type == "1010101700001111" || type == "1010101700001110" || subType == "w44") {
    nsubtype = SIGN_WARNING_SLIPPERY;
  } else if (type == "1010101400001111" || type == "1010101400001110" || subType == "w45") {
    nsubtype = SIGN_WARNING_TRIFFICLIGHT;
  } else if (type == "1010103313001111" || type == "1010103313001110" || subType == "w48") {
    nsubtype = SIGN_WARNING_DETOUR_RIGHT;
  } else if (type == "1010100800001111" || type == "1010100800001110" || subType == "w49") {
    nsubtype = SIGN_WARNING_NARROW_BRIDGE;
  } else if (type == "1010103900001111" || type == "1010103900001110" || subType == "w50") {
    nsubtype = SIGN_WARNING_KEEP_DISTANCE;
  } else if (type == "1010104111001111" || subType == "w58") {
    nsubtype = SIGN_WARNING_MERGE_LEFT;
  } else if (type == "1010104111001111" || type == "1010104111001110" || subType == "w59") {
    nsubtype = SIGN_WARNING_MERGE_RIGHT;
  } else if (type == "1010101600001111" || type == "1010101600001110" || subType == "w60") {
    nsubtype = SIGN_WARNING_CROSSWIND;
  } else if (type == "1010104311001111" || type == "1010104311001110" || subType == "w61") {
    nsubtype = SIGN_WARNING_ICY_ROAD;
  } else if (type == "1010101511001111" || type == "1010101511001110" || subType == "w62") {
    nsubtype = SIGN_WARNING_ROCKFALL;
  } else if (type == "1010103400001111" || type == "1010103400001110" || subType == "w63") {
    nsubtype = SIGN_WARNING_CAUTION;
  } else if (type == "1010104313001111" || type == "1010104313001110" || subType == "w54") {
    nsubtype = SIGN_WARNING_FOGGY;
  } else if (type == "1010101200001111" || type == "1010101200001110" || subType == "w64") {
    nsubtype = SIGN_WARNING_LIVESTOCK;
  } else if (type == "1010103312001111" || type == "1010103312001110" || subType == "w65") {
    nsubtype = SIGN_WARNING_DETOUR_LEFT;
  } else if (type == "1010103311001111" || type == "1010103311001110" || subType == "w66") {
    nsubtype = SIGN_WARNING_DETOUR_BOTH;
  } else if (type == "1010100900001111" || type == "1010100900001110" || subType == "w35") {
    nsubtype = SIGN_WARNING_BOTHWAY;
  } else if (type == "1010202400001413" || type == "1010202400001410" || subType == "p14") {
    nsubtype = SIGN_BAN_STRAIGHT;
  } else if (type == "1010200600001413" || type == "1010200600001410" || subType == "p10") {
    nsubtype = SIGN_BAN_VEHICLE;
  } else if (((type == "1010203800001413" || type == "1010203800001410") && subType == "120") || subType == "pl120") {
    nsubtype = SIGN_BAN_SPPED_120;
  } else if (((type == "1010203800001413" || type == "1010203800001410") && subType == "100") || subType == "pl100") {
    nsubtype = SIGN_BAN_SPPED_100;
  } else if (((type == "1010203800001413" || type == "1010203800001410") && subType == "80") || subType == "pl80") {
    nsubtype = SIGN_BAN_SPPED_80;
  }
  if (((type == "1010203800001413" || type == "1010203800001410") && subType == "70") || subType == "pl70") {
    nsubtype = SIGN_BAN_SPPED_70;
  } else if (((type == "1010203800001413" || type == "1010203800001410") && subType == "60") || subType == "pl60") {
    nsubtype = SIGN_BAN_SPPED_60;
  } else if (((type == "1010203800001413" || type == "1010203800001410") && subType == "50") || subType == "pl50") {
    nsubtype = SIGN_BAN_SPPED_50;
  } else if (((type == "1010203800001413" || type == "1010203800001410") && subType == "40") || subType == "pl40") {
    nsubtype = SIGN_BAN_SPPED_40;
  } else if (((type == "1010203800001413" || type == "1010203800001410") && subType == "30") || subType == "pl30") {
    nsubtype = SIGN_BAN_SPPED_30;
  } else if (type == "1010200100001914" || type == "1010200100001910" || subType == "ps") {
    nsubtype = SIGN_BAN_STOP_YIELD;
  } else if (((type == "1010203500001413" || type == "1010203500001410") && subType == "5") || subType == "ph5") {
    nsubtype = SIGN_BAN_HEIGHT_5;
  } else if (((type == "1010203800001413" || type == "1010203500001410") && subType == "20") || subType == "pl20") {
    nsubtype = SIGN_BAN_SPPED_20;
  } else if (((type == "1010203800001413" || type == "1010203500001410") && subType == "5") || subType == "pl05") {
    nsubtype = SIGN_BAN_SPPED_05;
  } else if (type == "1010200500001513" || type == "1010200500001510" || subType == "pne") {
    nsubtype = SIGN_BAN_DIVERINTO;
  }
  if (type == "1010201400001413" || type == "1010201400001410" || subType == "p12") {
    nsubtype = SIGN_BAN_MOTOR_BIKE;
  } else if (((type == "1010203600001413" || type == "1010203700001410") && subType == "55") || subType == "pm55") {
    nsubtype = SIGN_BAN_WEIGHT_50;
  }
  if (((type == "1010203600001413" || type == "1010203700001410") && subType == "20") || subType == "pm20") {
    nsubtype = SIGN_BAN_WEIGHT_20;
  } else if (type == "1010203300001413" || type == "1010203300001410" || subType == "p11") {
    nsubtype = SIGN_BAN_HONKING;
  } else if (type == "1010200700001413" || type == "1010200700001410" || subType == "p26") {
    nsubtype = SIGN_BAN_TRUCK;
  } else if (((type == "1010203600001413" || type == "1010203700001410") && subType == "30") || subType == "pm30") {
    nsubtype = SIGN_BAN_WEIGHT_30;
  } else if (((type == "1010203600001413" || type == "1010203700001410") && subType == "10") || subType == "pm10") {
    nsubtype = SIGN_BAN_WEIGHT_10;
  } else if (type == "1010203111001713" || type == "1010203111001710" || subType == "pn") {
    nsubtype = SIGN_BAN_TEMP_PARKING;
  } else if (((type == "1010203700001413" || type == "1010203700001410") && subType == "14") || subType == "pa14") {
    nsubtype = SIGN_BAN_AXLE_WEIGHT_14;
  } else if (((type == "1010203700001413" || type == "1010203700001410") && subType == "13") || subType == "pa13") {
    nsubtype = SIGN_BAN_AXLE_WEIGHT_13;
  } else if (((type == "1010203700001413" || type == "1010203700001410") && subType == "40") || subType == "pm40") {
    nsubtype = SIGN_BAN_WEIGHT_40;
  } else if (type == "1010200200002012" || type == "1010200200002010" || subType == "pg") {
    nsubtype = SIGN_BAN_SLOW;
  }
  if (type == "1010202211001413" || type == "1010202211001410" || subType == "p23") {
    nsubtype = SIGN_BAN_TURN_LEFT;
  } else if (type == "1010204100001413" || type == "1010204100001410" || subType == "p27") {
    nsubtype = SIGN_BAN_DANGEROUS_GOODS;
  } else if (type == "1010201200001413" || type == "1010201200001410" || subType == "p18") {
    nsubtype = SIGN_BAN_TRACTORS;
  } else if (type == "1010201300001413" || type == "1010201300001410" || subType == "p22") {
    nsubtype = SIGN_BAN_TRICYCLE;
  } else if (type == "1010201000001413" || type == "1010201000001410" || subType == "p25") {
    nsubtype = SIGN_BAN_MINIBUS;
  } else if (type == "1010202600001413" || type == "1010202600001410" || subType == "p28") {
    nsubtype = SIGN_BAN_STRAIGHT_AND_LEFT;
  } else if (type == "1010202000001413" || type == "1010202000001410" || subType == "p15") {
    nsubtype = SIGN_BAN_VEHICLE_BY_HUMAN;
  } else if (type == "1010201800001413" || type == "1010201800001410" || subType == "p16") {
    nsubtype = SIGN_BAN_TRACYCLE01_BY_HUMAN;
  } else if (type == "1010201900001413" || type == "1010201900001410" || subType == "p17") {
    nsubtype = SIGN_BAN_TRACYCLE02_BY_HUMAN;
  } else if (type == "1010202311001413" || type == "1010202311001410" || subType == "p19") {
    nsubtype = SIGN_BAN_TURN_RIGHT;
  } else if (type == "1010202500001413" || type == "1010202500001410" || subType == "p20") {
    nsubtype = SIGN_BAN_LEFT_AND_RIGHT;
  } else if (type == "1010202700001413" || type == "1010202700001410" || subType == "p21") {
    nsubtype = SIGN_BAN_STRAIGHT_AND_RIGHT;
  } else if (type == "1010200400001213" || type == "1010200400001210" || subType == "pb") {
    nsubtype = SIGN_BAN_GO;
  } else if (type == "1010200300002113" || type == "1010200300002110" || subType == "pe") {
    nsubtype = SIGN_BAN_GIVE_WAY;
  } else if (type == "1010202312001413" || type == "1010202312001410" || subType == "p24") {
    nsubtype = SIGN_BAN_BUS_TURN_RIGHT;
  } else if (type == "1010202315001413" || type == "1010202315001410" || subType == "p31") {
    nsubtype = SIGN_BAN_TRUCK_TURN_RIGHT;
  } else if (type == "1010203000001613" || type == "1010203000001610" || subType == "p30") {
    nsubtype = SIGN_BAN_NO_OVERTAKING;
  } else if (type == "1010202212001413" || type == "1010202212001410" || subType == "p29") {
    nsubtype = SIGN_BAN_BUS_TURN_LEFT;
  } else if (type == "1010202900001413" || type == "1010202900001410" || subType == "p1") {
    nsubtype = SIGN_BAN_OVERTAKING;
  } else if (type == "1010201700001413" || type == "1010201700001410" || subType == "p2") {
    nsubtype = SIGN_BAN_ANIMALS;
  } else if (type == "1010200900001413" || type == "1010200900001410" || subType == "p3") {
    nsubtype = SIGN_BAN_BUS;
  } else if (type == "1010200800001413" || type == "1010200800001410" || subType == "p4") {
    nsubtype = SIGN_BAN_ELECTRO_TRICYCLE;
  } else if (type == "1010201600001413" || type == "1010201600001410" || subType == "p6") {
    nsubtype = SIGN_BAN_NO_MOTOR;
  } else if (type == "1010202215001413" || type == "1010202215001410" || subType == "p7") {
    nsubtype = SIGN_BAN_TRUCK_TURN_LEFT;
  } else if (type == "1010201100001413" || type == "1010201100001410" || subType == "p8") {
    nsubtype = SIGN_BAN_TRAILER;
  } else if (type == "1010202100001413" || type == "1010202100001410" || subType == "p9") {
    nsubtype = SIGN_BAN_HUMAN;
  } else if (type == "1010201500001413" || subType == "p13") {
    nsubtype = SIGN_BAN_THE_TWO_TYPES;
  } else if (((type == "1010203500001413" || type == "1010203500001410") && subType == "3.5") || subType == "ph3.5") {
    nsubtype = SIGN_BAN_HEIGHT_3_5;
  } else if (((type == "1010203400001413" || type == "1010203400001410") && subType == "3") || subType == "pw3") {
    nsubtype = SIGN_BAN_HEIGHT_3;
  } else if (((type == "1010203600001413" || type == "1010203700001410") && subType == "10") || subType == "pa10") {
    nsubtype = SIGN_BAN_AXLE_WEIGHT_10;
  } else if (type == "1010204200001413" || type == "1010204200001410" || subType == "pd") {
    nsubtype = SIGN_BAN_CUSTOMS_MARK;
  } else if (type == "1010204000001413" || type == "1010204000001410" || subType == "pc") {
    nsubtype = SIGN_BAN_STOP;
  } else if (type == "1010203200001713" || type == "1010203200001710" || subType == "pnl") {
    nsubtype = SIGN_BAN_LONG_PARKING;
  } else if (type == "1010203900001613" || type == "1010203900001610" || subType == "pr40") {
    nsubtype = SIGN_BAN_REMOVE_LIMIT_40;
  } else if (type == "1010300100002413" || type == "1010300100002410" || subType == "i13") {
    nsubtype = SIGN_INDOCATION_STRAIGHT;
  } else if (((type == "1010301500002413" || type == "1010301500002410") && subType == "60") || subType == "il60") {
    nsubtype = SIGN_INDOCATION_LOWEST_SPEED_60;
  } else if (((type == "1010301500002413" || type == "1010301500002410") && subType == "40") || subType == "il40") {
    nsubtype = SIGN_INDOCATION_LOWEST_SPEED_40;
  } else if (type == "1010300700002413" || type == "1010300700002410" || subType == "i5") {
    nsubtype = SIGN_INDOCATION_ALONG_RIGHT;
  } else if (type == "1010301800002616" || type == "1010301800002610" || subType == "ip") {
    nsubtype = SIGN_INDOCATION_PEDESTRIAN_CROSSING;
  } else if (type == "1010300300002413" || type == "1010300300002410" || subType == "i10") {
    nsubtype = SIGN_INDOCATION_TURN_RIGHT;
  } else if (type == "1010301100002416" || type == "1010301100002410" || subType == "i3") {
    nsubtype = SIGN_INDOCATION_ROUNDABOUT;
  } else if (type == "1010300200002413" || type == "1010300200002410" || subType == "i12") {
    nsubtype = SIGN_INDOCATION_TURN_LEFT;
  } else if (type == "1010300500002413" || type == "1010300500002410" || subType == "i14") {
    nsubtype = SIGN_INDOCATION_STRAIGHT_RIGHT;
  } else if (type == "1010300400002413" || type == "1010300400002410" || subType == "i15") {
    nsubtype = SIGN_INDOCATION_STRAIGHT_LEFT;
  } else if (((type == "1010301500002413" || type == "1010301500002410") && subType == "50") || subType == "il50") {
    nsubtype = SIGN_INDOCATION_LOWEST_SPEED_50;
  } else if (type == "1010301300002413" || type == "1010301300002410" || subType == "i1") {
    nsubtype = SIGN_INDOCATION_WALK;
  } else if (type == "1010302014002413" || type == "1010302014002410" || subType == "i2") {
    nsubtype = SIGN_INDOCATION_NO_MOTOR;
  } else if (type == "1010302012002413" || type == "1010302012002410" || subType == "i4") {
    nsubtype = SIGN_INDOCATION_MOTOR;
  } else if (type == "1010300800002413" || type == "1010300800002410" || subType == "i6") {
    nsubtype = SIGN_INDOCATION_ALONG_LEFT;
  } else if (type == "1010301000002413" || type == "1010301000002410" || subType == "i7") {
    nsubtype = SIGN_INDOCATION_PASS_STAIGHT_001;
  } else if (type == "1010300900002413" || type == "1010300900002410" || subType == "i8") {
    nsubtype = SIGN_INDOCATION_PASS_STAIGHT_002;
  } else if (type == "1010301400002413" || type == "1010301400002410" || subType == "i9") {
    nsubtype = SIGN_INDOCATION_WHISTLE;
  } else if (type == "1010300600002413" || type == "1010300600002410" || subType == "i11") {
    nsubtype = SIGN_INDOCATION_LEFT_AND_RIGHT;
  } else if (type == "1010302111002416" || type == "1010302111002410") {
    nsubtype = SIGN_INDOCATION_PARKING;
  } else if (type == "1010101000001111" || type == "1010101000001110") {
    nsubtype = SIGN_WARNING_PED;
  } else if (type == "1010202800001413" || type == "1010202800001410") {
    nsubtype = SIGN_BAN_UTURN;
  } else {
    return -1;
  }
  return 0;
}

// set warn type
void txObject::setObjectType(const std::string name, const std::string type, const std::string subType, int st) {
  if (0 == setPoleType(name, type, instancePtr->data.type, instancePtr->data.subtype)) {
    // return;
  } else if (0 == setTrafficType(name, type, subType, st, instancePtr->data.type, instancePtr->data.subtype)) {
    // return;
  } else if (type.size() > 4 && std::string(type.substr(0, 4)) == std::string("1010") ||
             st == OBJECT_SOURCE_SIGNAL) {  // warning sign
    setSignType(name, type, subType, st, instancePtr->data.type, instancePtr->data.subtype);
  } else if (name == "Arrow_Forward" || name == "ZS_11") {  // Arrow
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_Straight;
    this->setRawTypeString("none", "-1");
  } else if (name == "Arrow_Left_And_Forward" || name == "ZS_12") {
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_StraightLeft;
    this->setRawTypeString("none", "-1");
  } else if (name == "Arrow_Left" || name == "ZS_13") {
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_Left;
    this->setRawTypeString("none", "");
  } else if (name == "Arrow_Right" || name == "ZS_14") {
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_Right;
    this->setRawTypeString("none", "");
  } else if (name == "Arrow_Right_And_Forward" || name == "ZS_15") {
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_StraightRight;
    this->setRawTypeString("none", "");
  } else if (name == "Arrow_U_Turns" || name == "ZS_16") {
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_Uturn;
    this->setRawTypeString("none", "");
  } else if (name == "Arrow_Forward_And_U_Turns" || name == "ZS_17") {
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_StraightUturn;
    this->setRawTypeString("none", "");
  } else if (name == "Arrow_Left_And_U_Turns" || name == "ZS_18") {
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_LeftUturn;
    this->setRawTypeString("none", "");
  } else if (name == "Arrow_Left_And_Right" || name == "ZS_19") {
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_LeftRight;
    this->setRawTypeString("none", "");
  } else if (name == "Turn_And_Merge_Left" || name == "ZS_20") {
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_SkewLeft;
    this->setRawTypeString("none", "");
  } else if (name == "Turn_And_Merge_Right" || name == "ZS_21") {
    instancePtr->data.type = OBJECT_TYPE_Arrow;
    instancePtr->data.subtype = Arrow_SkewRight;
    this->setRawTypeString("none", "");
  } else if (name == "Arrow_Turn_And_Straight") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = Arrow_TurnStraight;
    this->setRawTypeString("none", "");
  } else if (name == "Stop_Line" || name == "JZ_25") {  // stop line
    instancePtr->data.type = OBJECT_TYPE_Stop;
    instancePtr->data.subtype = Stop_Line;
    this->setRawTypeString("none", "-1");
  } else if (name == "Stop_To_Give_Way" || name == "JZ_26") {
    instancePtr->data.type = OBJECT_TYPE_Stop;
    instancePtr->data.subtype = Stop_Line_GiveWay;
    this->setRawTypeString("none", "-1");
  } else if (name == "Slow_Down_To_Give_Way" || name == "JZ_27") {
    instancePtr->data.type = OBJECT_TYPE_Stop;
    instancePtr->data.subtype = Stop_Slow_Line_GiveWay;
    this->setRawTypeString("none", "");
  } else if (name == "Crosswalk_Line" || name == "ZS_04") {  // cross walk
    instancePtr->data.type = OBJECT_TYPE_CrossWalk;
    instancePtr->data.subtype = CrossWalk_001;
    this->setRawTypeString("none", "");
  } else if (name == "Turn_Left_Waiting" || name == "ZS_01") {  // RoadSign
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Turn_Left_Waiting;
    this->setRawTypeString("none", "");
  } else if (name == "Crosswalk_Warning_Line" || name == "ZS_05") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Crosswalk_Warning_Line;
    this->setRawTypeString("none", "");
  } else if (name == "White_Broken_Line_Vehicle_Distance_Confirmation" || name == "ZS_07") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_White_Broken_Line;
    this->setRawTypeString("none", "");
  } else if (name == "White_Semicircle_Line_Vehicle_Distance_Confirmation" || name == "ZS_08") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_White_Semicircle_Line;
    this->setRawTypeString("none", "");
  } else if (name == "Word_Mark_100_120" || name == "ZS_22") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_100_120;
    this->setRawTypeString("none", "");
  } else if (name == "Word_Mark_80_100" || name == "ZS_22_2") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_80_100;
    this->setRawTypeString("none", "");
  } else if (name == "Non_Motor_Vehicle" || name == "ZS_23") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Non_Motor_Vehicle;
    this->setRawTypeString("none", "");
  } else if (name == "Disabled_Parking_Space_Road_Mark" || name == "ZS_24") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Disable_Parking;
    this->setRawTypeString("none", "");
  } else if (name == "Lateral_Deceleration_Marking" || name == "JG_38") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Lateral_Dec;
    this->setRawTypeString("none", "");
  } else if (name == "Longitudinal_Deceleration_Marking" || name == "JG_39") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Longitudinal_Dec;
    this->setRawTypeString("none", "");
  } else if (name == "Circular_Center_Circle" || name == "JZ_29") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Circular_Center;
    this->setRawTypeString("none", "");
  } else if (name == "Rhombus_Center_Circle" || name == "JZ_30") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Rhombus_Center;
    this->setRawTypeString("none", "");
  } else if (name == "Mesh_Line" || name == "JZ_31") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Mesh_Line;
    this->setRawTypeString("none", "");
  } else if (name == "Bus_Only_Lane_Line" || name == "JZ_32") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_BusOnly;
    this->setRawTypeString("none", "");
  } else if (name == "Small_Cars_Lane_Line" || name == "JZ_33") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_SmallCarOnly;
    this->setRawTypeString("none", "");
  } else if (name == "Big_Cars_Lane_Line" || name == "JZ_34") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_BigCarOnly;
    this->setRawTypeString("none", "");
  } else if (name == "Non_Motor_Vehicle_Line" || name == "JZ_35") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Non_Motor_Vehicle_Line;
    this->setRawTypeString("none", "");
  } else if (name == "Turning_Forbidden" || name == "JZ_37") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Turning_Forbidden;
    this->setRawTypeString("none", "");
  } else if (name == "White_Semicircle_Line") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_White_Semicircle_Line;
    this->setRawTypeString("none", "");
  } else if (name == "Crosswalk_with_Left_and_Right_Side") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Crosswalk_with_Left_and_Right_Side;
    this->setRawTypeString("none", "");
  } else if (name == "Road_Guide_Lane_Line") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Road_Guide_Lane_Line;
    this->setRawTypeString("none", "");
  } else if (name == "Variable_Direction_Lane_Line") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Variable_Direction_Lane_Line;
    this->setRawTypeString("none", "");
  } else if (name == "Intersection_Guide_Line") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Intersection_Guide_Line;
    this->setRawTypeString("none", "");
  } else if (name == "U_Turning_Forbidden") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_U_Turning_Forbidden;
    this->setRawTypeString("none", "");
  } else if (name == "Non_Motor_Vehicle_Area") {
    instancePtr->data.type = OBJECT_TYPE_RoadSign;
    instancePtr->data.subtype = RoadSign_Non_Motor_Vehicle_Area;
    this->setRawTypeString("none", "");
  } else if (name == "D_001" || name == "Pothole") {  // road surtface
    instancePtr->data.type = OBJECT_TYPE_Surface;
    instancePtr->data.subtype = Surface_Pothole;
  } else if (name == "D_002" || name == "Patch") {
    instancePtr->data.type = OBJECT_TYPE_Surface;
    instancePtr->data.subtype = Surface_Patch;
  } else if (name == "D_003" || name == "Crack") {
    instancePtr->data.type = OBJECT_TYPE_Surface;
    instancePtr->data.subtype = Surface_Crack;
  } else if (name == "D_004" || name == "Asphalt_Line") {
    instancePtr->data.type = OBJECT_TYPE_Surface;
    instancePtr->data.subtype = Surface_Asphalt_Line;
  } else if (name == "D_005" || name == "Rut_Track") {
    instancePtr->data.type = OBJECT_TYPE_Surface;
    instancePtr->data.subtype = Surface_Rut_Track;
  } else if (name == "D_006" || name == "Stagnant_Water") {
    instancePtr->data.type = OBJECT_TYPE_Surface;
    instancePtr->data.subtype = Surface_Stagnant_Water;
  } else if (name == "J_003" || name == "Protrusion") {
    instancePtr->data.type = OBJECT_TYPE_Surface;
    instancePtr->data.subtype = Surface_Protrusion;
  } else if (name == "J_001" || name == "Well_Cover") {
    instancePtr->data.type = OBJECT_TYPE_Surface;
    instancePtr->data.subtype = Surface_Well_Cover;
  } else if (name == "J_002" || name == "Deceleration_Zone") {
    instancePtr->data.type = OBJECT_TYPE_SpeedBump;
    instancePtr->data.subtype = SpeedBump_001;
  } else if (name == "J_004" || name == "Reflective_Road_Sign") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Reflective_Road_Sign;
  } else if (name == "J_005" || name == "Parking_Hole") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Parking_Hole;
  } else if (name == "J_006" || name == "Parking_Lot") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Parking_Lot;
  } else if (name == "J_007" || name == "Ground_Lock") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Ground_Lock;
  } else if (name == "J_008" || name == "Plastic_Vehicle_Stopper") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Plastic_Vehicle_Stopper;
  } else if (name == "J_009" || name == "Parking_Limit_Position_Pole_2m") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Parking_Limit_Position_Pole_2m;
  } else if (name == "J_010" || name == "Support_Vehicle_Stopper") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Support_Vehicle_Stopper;
  } else if (name == "J_014" || name == "Charging_Station") {
    instancePtr->data.type = OBJECT_TYPE_ChargingPile;
    instancePtr->data.subtype = ChargingPile_001;
  } else if (name == "J_011" || name == "Tree") {
    instancePtr->data.type = OBJECT_TYPE_Tree;
    instancePtr->data.subtype = Tree_001;
  } else if (name == "Shrub") {
    instancePtr->data.type = OBJECT_TYPE_Tree;
    instancePtr->data.subtype = Shrub_001;
  } else if (name == "Grass") {
    instancePtr->data.type = OBJECT_TYPE_Tree;
    instancePtr->data.subtype = Grass_001;
  } else if (name == "J_012" || name == "Building") {
    instancePtr->data.type = OBJECT_TYPE_Building;
    instancePtr->data.subtype = Building_001;
  } else if (name == "J_013" || name == "BusStation") {
    instancePtr->data.type = OBJECT_TYPE_BusStation;
    instancePtr->data.subtype = BusStation_001;
  } else if (name == "Traffic_Barrier") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Traffic_Barrier;
  } else if (name == "Road_Curb") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Road_Curb;
  } else if (name == "Lamp") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Lamp;
  } else if (name == "Traffic_Cone") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Traffic_Cone;
  } else if (name == "Traffic_Horse") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Traffic_Horse;
  } else if (name == "GarbageCan") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_GarbageCan;
  } else if (name == "Obstacle") {
    instancePtr->data.type = OBJECT_TYPE_Obstacle;
    instancePtr->data.subtype = Obstacle_Obstacle;
  } else if (subType == "l1" || type == "parkingSpace" || type == "parking" || st == OBJECT_SOURCE_PARKING) {
    instancePtr->data.type = OBJECT_TYPE_ParkingSpace;
    if (name == "Parking_Space_Mark") {
      instancePtr->data.subtype = Parking_Space_001;
    } else if (name == "Parking_6m") {
      instancePtr->data.subtype = Parking_Space_003;
    } else if (name == "Parking_5m") {
      instancePtr->data.subtype = Parking_Space_004;
    } else if (name == "Parking_45deg") {
      instancePtr->data.subtype = Parking_Space_005;
    } else if (name == "Parking_60deg") {
      instancePtr->data.subtype = Parking_Space_006;
    } else if (name == "Time_Limit_Parking_Space_Mark") {
      instancePtr->data.subtype = Parking_Space_002;
    } else {
      instancePtr->data.subtype = Parking_Space_001;
    }
  }
  // other object(tadsim 1.0 is signals  but tadsim 2.0 is object)
  if (subType == "camera" || name == "Camera") {
    instancePtr->data.type = OBJECT_TYPE_V2X_Sensor;
    instancePtr->data.subtype = Sensors_Camera;
  } else if (subType == "radar" || name == "Millimeter_Wave_Radar") {
    instancePtr->data.type = OBJECT_TYPE_V2X_Sensor;
    instancePtr->data.subtype = Sensors_Radar;
  } else if (subType == "rsu" || name == "RSU") {
    instancePtr->data.type = OBJECT_TYPE_V2X_Sensor;
    instancePtr->data.subtype = Sensors_RSU;
  } else if (name == "Lidar") {
    instancePtr->data.type = OBJECT_TYPE_V2X_Sensor;
    instancePtr->data.subtype = Sensors_Lidar;
  } else if (type == "custom") {
    instancePtr->data.type = OBJECT_TYPE_Custom;
    instancePtr->data.subtype = CustomSubType;
  } else if (name == "Tunnel") {
    instancePtr->data.type = OBJECT_TYPE_Tunnel;
    instancePtr->data.subtype = Tunnel_001;
  } else if (name == "PedestrianBridge") {
    instancePtr->data.type = OBJECT_TYPE_PedestrianBridge;
    instancePtr->data.subtype = PedestrianBridge;
  }

  instancePtr->od_data.type = instancePtr->data.type;
  instancePtr->od_data.subtype = instancePtr->data.subtype;
  this->setRawTypeString(type, subType);
  strcpy(instancePtr->data.name, name.c_str());
  strcpy(instancePtr->od_data.name, name.c_str());
}

// get poleType
void getPole(uint32_t data_subtype, std::string& name, std::string& type, std::string& subtype) {
  switch (data_subtype) {
    case OtherSubtype:
      return;
    case POLE_VERTICAL:
      name = "Vertical_Pole";
      type = "pole";
      subtype = "none";
      break;
    case POLE_CROSS:
      name = "Cross_Pole";
      type = "pole";
      subtype = "none";
      break;
    case POLE_Cantilever:
      name = "Cantilever_Pole";
      type = "pole";
      subtype = "none";
      break;
    case POLE_Pillar_6m:
      name = "Pillar_Pole_6m";
      type = "pole";
      subtype = "none";
      break;
    case POLE_Pillar_3m:
      name = "Pillar_Pole_3m";
      type = "pole";
      subtype = "none";
      break;
    default:
      break;
  }
}

// get SIGN_WARNING Type
void getSign(uint32_t data_subtype, std::string& name, std::string& type, std::string& subtype) {
  name = "null";
  subtype = "-1";
  switch (data_subtype) {
    case SIGN_WARNING_SLOWDOWN:
      type = "1010103200001111";
      break;
    case SIGN_WARNING_TURNLEFT:
      type = "1010100211001111";
      break;
    case SIGN_WARNING_TURNRIGHT:
      type = "1010100212001111";
      break;
    case SIGN_WARNING_T_RIGHT:
      type = "1010100121001111";
      break;
    case SIGN_WARNING_T_DOWN:
      type = "1010100123001111";
      break;
    case SIGN_WARNING_CROSS:
      type = "1010100111101111";
      break;
    case SIGN_WARNING_CHILD:
      type = "1010101100001111";
      break;
    case SIGN_WARNING_UP:
      type = "1010100511001111";
      break;
    case SIGN_WARNING_BUILDING:
      type = "1010103500001111";
      break;
    case SIGN_WARNING_NARROW_LEFT:
      type = "1010100711001111";
      break;
    case SIGN_WARNING_NARROW_BOTH:
      type = "1010100713001111";
      break;
    case SIGN_WARNING_RAILWAY:
      type = "1010102812001111";
      break;
    case SIGN_WARNING_T_LEFT:
      type = "1010100122001111";
      break;
    case SIGN_WARNING_STEEP_LEFT:
      type = "1010101811001111";
      break;
    case SIGN_WARNING_STEEP_RIGHT:
      type = "1010101812001111";
      break;
    case SIGN_WARNING_VILLAGE:
      type = "1010102000001111";
      break;
    case SIGN_WARNING_DIKE_LEFT:
      type = "1010101911001111";
      break;
    case SIGN_WARNING_DIKE_RIGHT:
      type = "1010101912001111";
      break;
    case SIGN_WARAING_T_CROSSED:
      type = "1010104012001111";
      break;
    case SIGN_WARAING_FERRY:
      type = "1010102200001111";
      break;
    case SIGN_WARAING_FALL_ROCK:
      type = "1010101512001111";
      break;
    case SIGN_WARAING_REVERSE_CURVE_LEFT:
      type = "1010100311001111";
      break;
    case SIGN_WARAING_REVERSE_CURVE_RIGHT:
      type = "1010100312001111";
      break;
    case SIGN_WARAING_WATER_PAVEMENT:
      type = "1010102700001111";
      break;
    case SIGN_WARNING_T_BOTH:
      type = "1010100112001111";
      break;
    case SIGN_WARNING_JOIN_LEFT:
      type = "1010100131001111";
      break;
    case SIGN_WARNING_JOIN_RIGHT:
      type = "1010100132001111";
      break;
    case SIGN_WARNING_Y_LEFT:
      type = "1010100134001111";
      break;
    case SIGN_WARNING_Y_RIGHT:
      type = "1010100134001111";
      break;
    case SIGN_WARNING_CIRCLE_CROSS:
      type = "1010100141001111";
      break;
    case SIGN_WARNING_CURVE_AHEAD:
      type = "1010100400001111";
      break;
    case SIGN_WARNING_LONG_DESCENT:
      type = "1010100600001111";
      break;
    case SIGN_WARNING_ROUGH_ROAD:
      type = "1010102400001111";
      break;
    case SIGN_WARNING_SNOW:
      type = "1010104312001111";
      break;
    case SIGN_WARNING_DISABLE:
      type = "1010103000001111";
      break;
    case SIGN_WARNING_ANIMALS:
      type = "1010101300001111";
      break;
    case SIGN_WARNING_ACCIDENT:
      type = "1010103100001111";
      break;
    case SIGN_WARNING_TIDALBU_LANE:
      type = "1010103800001111";
      break;
    case SIGN_WARNING_BAD_WEATHER:
      type = "1010104314001111";
      break;
    case SIGN_WARNING_LOWLYING:
      type = "1010102600001111";
      break;
    case SIGN_WARNING_HIGHLYING:
      type = "1010102500001111";
      break;
    case SIGN_WARNING_DOWNHILL:
      type = "1010100512001111";
      break;
    case SIGN_WARNING_QUEUESLIKELY:
      type = "1010104400001111";
      break;
    case SIGN_WARNING_CROSS_PLANE:
      type = "1010104011001111";
      break;
    case SIGN_WARNING_TUNNEL:
      type = "1010102100001111";
      break;
    case SIGN_WARNING_TUNNEL_LIGHT:
      type = "1010103700001111";
      break;
    case SIGN_WARNING_HUMPBACK_BRIDGE:
      type = "1010102300001111";
      break;
    case SIGN_WARNING_NARROW_RIGHT:
      type = "1010100712001111";
      break;
    case SIGN_WARNING_NON_MOTOR:
      type = "1010102900001111";
      break;
    case SIGN_WARNING_SLIPPERY:
      type = "1010101700001111";
      break;
    case SIGN_WARNING_TRIFFICLIGHT:
      type = "1010101400001111";
      break;
    case SIGN_WARNING_DETOUR_RIGHT:
      type = "1010103313001111";
      break;
    case SIGN_WARNING_NARROW_BRIDGE:
      type = "1010100800001111";
      break;
    case SIGN_WARNING_KEEP_DISTANCE:
      type = "1010103900001111";
      break;
    case SIGN_WARNING_MERGE_LEFT:
      type = "1010104111001111";
      break;
    case SIGN_WARNING_CROSSWIND:
      type = "1010101600001111";
      break;
    case SIGN_WARNING_ICY_ROAD:
      type = "1010104311001111";
      break;
    case SIGN_WARNING_ROCKFALL:
      type = "1010101511001111";
      break;
    case SIGN_WARNING_CAUTION:
      type = "1010103400001111";
      break;
    case SIGN_WARNING_FOGGY:
      type = "1010104313001111";
      break;
    case SIGN_WARNING_LIVESTOCK:
      type = "1010101200001111";
      break;
    case SIGN_WARNING_DETOUR_LEFT:
      type = "1010103312001111";
      break;
    case SIGN_WARNING_DETOUR_BOTH:
      type = "1010103311001111";
      break;
    case SIGN_WARNING_BOTHWAY:
      type = "1010100900001111";
      break;
    case SIGN_BAN_STRAIGHT:
      type = "1010202400001413";
      break;
    case SIGN_BAN_VEHICLE:
      type = "1010200600001413";
      break;
    case SIGN_BAN_SPPED_120:
      type = "1010203800001413";
      subtype = "120";
      break;
    case SIGN_BAN_SPPED_100:
      type = "1010203800001413";
      subtype = "100";
      break;
    case SIGN_BAN_SPPED_80:
      type = "1010203800001413";
      subtype = "80";
      break;
    case SIGN_BAN_SPPED_70:
      type = "1010203800001413";
      subtype = "70";
      break;
    case SIGN_BAN_SPPED_60:
      type = "1010203800001413";
      subtype = "60";
      break;
    case SIGN_BAN_SPPED_50:
      type = "1010203800001413";
      subtype = "50";
      break;
    case SIGN_BAN_SPPED_40:
      type = "1010203800001413";
      subtype = "40";
      break;
    case SIGN_BAN_SPPED_30:
      type = "1010203800001413";
      subtype = "30";
      break;
    case SIGN_BAN_STOP_YIELD:
      type = "1010200100001914";
      break;
    case SIGN_BAN_HEIGHT_5:
      type = "1010203500001413";
      subtype = "5";
      break;
    case SIGN_BAN_SPPED_20:
      type = "1010203800001413";
      subtype = "20";
      break;
    case SIGN_BAN_SPPED_05:
      type = "1010203800001413";
      subtype = "5";
      break;
    case SIGN_BAN_DIVERINTO:
      type = "1010200500001513";
      break;
    case SIGN_BAN_MOTOR_BIKE:
      type = "1010201400001413";
      break;
    case SIGN_BAN_WEIGHT_50:
      type = "1010203600001413";
      subtype = "55";
      break;
    case SIGN_BAN_WEIGHT_20:
      type = "1010203600001413";
      subtype = "20";
      break;
    case SIGN_BAN_HONKING:
      type = "1010203300001413";
      break;
    case SIGN_BAN_TRUCK:
      type = "1010200700001413";
      break;
    case SIGN_BAN_WEIGHT_30:
      type = "1010203600001413";
      subtype = "30";
      break;
    case SIGN_BAN_WEIGHT_10:
      type = "1010203600001413";
      subtype = "10";
      break;
    case SIGN_BAN_TEMP_PARKING:
      type = "1010203111001713";
      break;
    case SIGN_BAN_AXLE_WEIGHT_14:
      type = "1010203700001413";
      subtype = "14";
      break;
    case SIGN_BAN_AXLE_WEIGHT_13:
      type = "1010203700001413";
      subtype = "13";
      break;
    case SIGN_BAN_WEIGHT_40:
      type = "1010203700001413";
      subtype = "40";
      break;
    case SIGN_BAN_SLOW:
      type = "1010200200002012";
      break;
    case SIGN_BAN_TURN_LEFT:
      type = "1010202211001413";
      break;
    case SIGN_BAN_DANGEROUS_GOODS:
      type = "1010204100001413";
      break;
    case SIGN_BAN_TRACTORS:
      type = "1010201200001413";
      break;
    case SIGN_BAN_TRICYCLE:
      type = "1010201300001413";
      break;
    case SIGN_BAN_MINIBUS:
      type = "1010201000001413";
      break;
    case SIGN_BAN_STRAIGHT_AND_LEFT:
      type = "1010202600001413";
      break;
    case SIGN_BAN_VEHICLE_BY_HUMAN:
      type = "1010202000001413";
      break;
    case SIGN_BAN_TRACYCLE01_BY_HUMAN:
      type = "1010201800001413";
      break;
    case SIGN_BAN_TRACYCLE02_BY_HUMAN:
      type = "1010201900001413";
      break;
    case SIGN_BAN_TURN_RIGHT:
      type = "1010202311001413";
      break;
    case SIGN_BAN_LEFT_AND_RIGHT:
      type = "1010202500001413";
      break;
    case SIGN_BAN_STRAIGHT_AND_RIGHT:
      type = "1010202700001413";
      break;
    case SIGN_BAN_GO:
      type = "1010200400001213";
      break;
    case SIGN_BAN_GIVE_WAY:
      type = "1010200300002113";
      break;
    case SIGN_BAN_BUS_TURN_RIGHT:
      type = "1010202312001413";
      break;
    case SIGN_BAN_TRUCK_TURN_RIGHT:
      type = "1010202315001413";
      break;
    case SIGN_BAN_NO_OVERTAKING:
      type = "1010203000001613";
      break;
    case SIGN_BAN_BUS_TURN_LEFT:
      type = "1010202212001413";
      break;
    case SIGN_BAN_OVERTAKING:
      type = "1010202900001413";
      break;
    case SIGN_BAN_ANIMALS:
      type = "1010201700001413";
      break;
    case SIGN_BAN_BUS:
      type = "1010200900001413";
      break;
    case SIGN_BAN_ELECTRO_TRICYCLE:
      type = "1010200800001413";
      break;
    case SIGN_BAN_NO_MOTOR:
      type = "1010201600001413";
      break;
    case SIGN_BAN_TRUCK_TURN_LEFT:
      type = "1010202215001413";
      break;
    case SIGN_BAN_TRAILER:
      type = "1010201100001413";
      break;
    case SIGN_BAN_HUMAN:
      type = "1010202100001413";
      break;
    case SIGN_BAN_THE_TWO_TYPES:
      type = "1010201500001413";
      break;
    case SIGN_BAN_HEIGHT_3_5:
      type = "1010203500001413";
      subtype = "3.5";
      break;
    case SIGN_BAN_HEIGHT_3:
      type = "1010203400001413";
      subtype = "3";
      break;
    case SIGN_BAN_AXLE_WEIGHT_10:
      type = "1010203600001413";
      subtype = "10";
      break;
    case SIGN_BAN_CUSTOMS_MARK:
      type = "1010204200001413";
      break;
    case SIGN_BAN_STOP:
      type = "1010204000001413";
      break;
    case SIGN_BAN_LONG_PARKING:
      type = "1010203200001713";
      break;
    case SIGN_BAN_REMOVE_LIMIT_40:
      type = "1010203900001613";
      break;
    case SIGN_INDOCATION_STRAIGHT:
      type = "1010300100002413";
      break;
    case SIGN_INDOCATION_LOWEST_SPEED_60:
      type = "1010301500002413";
      subtype = "60";
      break;
    case SIGN_INDOCATION_LOWEST_SPEED_40:
      type = "1010301500002413";
      subtype = "40";
      break;
    case SIGN_INDOCATION_ALONG_RIGHT:
      type = "1010300700002413";
      break;
    case SIGN_INDOCATION_PEDESTRIAN_CROSSING:
      type = "1010301800002616";
      break;
    case SIGN_INDOCATION_TURN_RIGHT:
      type = "1010300300002413";
      break;
    case SIGN_INDOCATION_ROUNDABOUT:
      type = "1010301100002416";
      break;
    case SIGN_INDOCATION_TURN_LEFT:
      type = "1010300200002413";
      break;
    case SIGN_INDOCATION_STRAIGHT_RIGHT:
      type = "1010300500002413";
      break;
    case SIGN_INDOCATION_STRAIGHT_LEFT:
      type = "1010300400002413";
      break;
    case SIGN_INDOCATION_LOWEST_SPEED_50:
      type = "1010301500002413";
      subtype = "50";
      break;
    case SIGN_INDOCATION_WALK:
      type = "1010301300002413";
      break;
    case SIGN_INDOCATION_NO_MOTOR:
      type = "1010302014002413";
      break;
    case SIGN_INDOCATION_MOTOR:
      type = "1010302012002413";
      break;
    case SIGN_INDOCATION_ALONG_LEFT:
      type = "1010300800002413";
      break;
    case SIGN_INDOCATION_PASS_STAIGHT_001:
      type = "1010301000002413";
      break;
    case SIGN_INDOCATION_PASS_STAIGHT_002:
      type = "1010300900002413";
      break;
    case SIGN_INDOCATION_WHISTLE:
      type = "1010301400002413";
      break;
    case SIGN_INDOCATION_LEFT_AND_RIGHT:
      type = "1010300600002413";
      break;
    case SIGN_BAN_UTURN:
      type = "1010202800001413";
      break;
    case SIGN_WARNING_PED:
      type = "1010101000001111";
      break;
    case SIGN_INDOCATION_PARKING:
      type = "1010302111002416";
      break;
    default:
      break;
  }
}

// get SIGN_WARNING Type
void getRoadSign(uint32_t data_subtype, std::string& name, std::string& type, std::string& subtype) {
  type = "null";
  subtype = "-1";
  switch (data_subtype) {
    case RoadSign_Turn_Left_Waiting:
      name = "Turn_Left_Waiting";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Crosswalk_Warning_Line:
      name = "Crosswalk_Warning_Line";
      type = "none";
      subtype = "";
      break;
    case RoadSign_White_Broken_Line:
      name = "White_Broken_Line_Vehicle_Distance_Confirmation";
      type = "none";
      subtype = "";
      break;
    case RoadSign_White_Semicircle_Line:
      name = "White_Semicircle_Line_Vehicle_Distance_Confirmation";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Disable_Parking:
      name = "Disabled_Parking_Space_Road_Mark";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Lateral_Dec:
      name = "Lateral_Deceleration_Marking";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Longitudinal_Dec:
      name = "Longitudinal_Deceleration_Marking";
      type = "none";
      subtype = "";
    case RoadSign_100_120:
      name = "Word_Mark_100_120";
      type = "none";
      subtype = "";
      break;
    case RoadSign_80_100:
      name = "Word_Mark_80_100";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Non_Motor_Vehicle:
      name = "Non_Motor_Vehicle";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Circular_Center:
      name = "Circular_Center_Circle";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Rhombus_Center:
      name = "Rhombus_Center_Circle";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Mesh_Line:
      name = "Mesh_Line";
      type = "none";
      subtype = "";
      break;
    case RoadSign_BusOnly:
      name = "Bus_Only_Lane_Line";
      type = "none";
      subtype = "";
      break;
    case RoadSign_SmallCarOnly:
      name = "Small_Cars_Lane_Line";
      type = "none";
      subtype = "";
      break;
    case RoadSign_BigCarOnly:
      name = "Big_Cars_Lane_Line";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Non_Motor_Vehicle_Line:
      name = "Non_Motor_Vehicle_Line";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Turning_Forbidden:
      name = "Turning_Forbidden";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Crosswalk_with_Left_and_Right_Side:
      name = "Crosswalk_with_Left_and_Right_Side";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Road_Guide_Lane_Line:
      name = "Road_Guide_Lane_Line";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Variable_Direction_Lane_Line:
      name = "Variable_Direction_Lane_Line";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Intersection_Guide_Line:
      name = "Intersection_Guide_Line";
      type = "none";
      subtype = "";
      break;

    case RoadSign_U_Turning_Forbidden:
      name = "U_Turning_Forbidden";
      type = "none";
      subtype = "";
      break;
    case RoadSign_Non_Motor_Vehicle_Area:
      name = "Non_Motor_Vehicle_Area";
      type = "none";
      subtype = "";
      break;
  }
}

#define nSingleObjectTypeNumMax 100
#define nSingleSignTypeNumMax 1000  // 由于SIGN 数量超过了100个，所以预留1000个位置

void txObject::getObjectFromatType(std::string& name, std::string& type, std::string& subtype) const {
  if (instancePtr->data.subtype >= POLE_VERTICAL &&
      instancePtr->data.subtype < POLE_VERTICAL + nSingleObjectTypeNumMax) {
    getPole(instancePtr->data.subtype, name, type, subtype);
  } else if (instancePtr->data.subtype >= SIGN_WARNING_SLOWDOWN &&
             instancePtr->data.subtype < SIGN_WARNING_SLOWDOWN + nSingleSignTypeNumMax) {
    getSign(instancePtr->data.subtype, name, type, subtype);
  } else if (instancePtr->data.subtype >= RoadSign_Turn_Left_Waiting &&
             instancePtr->data.subtype < RoadSign_Turn_Left_Waiting + nSingleObjectTypeNumMax) {
    getRoadSign(instancePtr->data.subtype, name, type, subtype);
  } else {
    switch (instancePtr->data.subtype) {
      case OtherSubtype:
        return;
        break;
      case LIGHT_VERTICAL_ALLDIRECT:
        name = "verticalOmnidirectionalLight";
        type = "1000001";
        subtype = "-1";
        break;
      case LIGHT_VERTICAL_STRAIGHTROUND:
        name = "verticalStraightRoundLight";
        type = "1000011";
        subtype = "60";
        break;
      case LIGHT_VERTICAL_LEFTTURN:
        name = "verticalLeftTurnLigh";
        type = "1000011";
        subtype = "10";
        break;
      case LIGHT_VERTICAL_STRAIGHT:
        name = "verticalStraightLight";
        type = "1000011";
        subtype = "30";
        break;
      case LIGHT_VERTICAL_RIGHTTURN:
        name = "verticalRightTurnLight";
        type = "1000011";
        subtype = "20";
        break;
      case LIGHT_VERTICAL_UTURN:
        name = "verticalUTurnLigh";
        type = "1000011";
        subtype = "70";
        break;
      case LIGHT_HORIZON_PEDESTRIAN:
        name = "verticalPedestrianLight";
        type = "1000002";
        subtype = "-1";
        break;
      case LIGHT_HORIZON_ALLDIRECT:
        name = "horizontalOmnidirectionalLight";
        type = "1000003";
        subtype = "-1";
        break;
      case LIGHT_HORIZON_STRAIGHTROUND:
        name = "horizontalStraightRoundLight";
        type = "1000021";
        subtype = "60";
        break;
      case LIGHT_HORIZON_LEFTTURN:
        name = "horizontalLeftTurnLight";
        type = "1000021";
        subtype = "10";
        break;
      case LIGHT_HORIZON_STRAIGHT:
        name = "horizontalStraightLight";
        type = "1000021";
        subtype = "30";
        break;
      case LIGHT_HORIZON_RIGHTTURN:
        name = "horizontalRightTurnLight";
        type = "1000021";
        subtype = "20";
        break;
      case LIGHT_HORIZON_UTURN:
        name = "horizontalUTurnLight";
        type = "1000021";
        subtype = "70";
        break;
      case LIGHT_BICYCLELIGHT:
        name = "bicycleLight";
        type = "1000013";
        subtype = "-1";
        break;
      case LIGHT_TWOCOLOR:
        name = "twoColorIndicatorLight";
        type = "1000009";
        subtype = "-1";
        break;
      case CrossWalk_001:
        name = "Crosswalk_Line";
        type = "none";
        subtype = "";
        break;
      //
      case Arrow_Straight:
        name = "Arrow_Forward";
        type = "none";
        subtype = "";
        break;
      case Arrow_StraightLeft:
        name = "Arrow_Left_And_Forward";
        type = "none";
        subtype = "";
        break;
      case Arrow_Left:
        name = "Arrow_Left";
        type = "none";
        subtype = "";
        break;
      case Arrow_Right:
        name = "Arrow_Right";
        type = "none";
        subtype = "";
        break;
      case Arrow_StraightRight:
        name = "Arrow_Right_And_Forward";
        type = "none";
        subtype = "";
        break;
      case Arrow_Uturn:
        name = "Arrow_U_Turns";
        type = "none";
        subtype = "";
        break;
      case Arrow_StraightUturn:
        name = "Arrow_Forward_And_U_Turns";
        type = "none";
        subtype = "";
        break;
      case Arrow_LeftUturn:
        name = "Arrow_Left_And_U_Turns";
        type = "none";
        subtype = "";
        break;
      case Arrow_LeftRight:
        name = "Arrow_Left_And_Right";
        type = "none";
        subtype = "";
        break;
      case Arrow_TurnStraight:
        name = "Arrow_Turn_And_Straight";
        type = "none";
        subtype = "";
        break;
      case Arrow_SkewLeft:
        name = "Turn_And_Merge_Left";
        type = "none";
        subtype = "";
        break;
      case Arrow_SkewRight:
        name = "Turn_And_Merge_Right";
        type = "none";
        subtype = "";
        break;
      //
      case Stop_Line:
        name = "Stop_Line";
        type = "none";
        subtype = "";
        break;
      case Stop_Line_GiveWay:
        name = "Stop_To_Give_Way";
        type = "none";
        subtype = "";
        break;
      case Stop_Slow_Line_GiveWay:
        name = "Slow_Down_To_Give_Way";
        type = "none";
        subtype = "";
        break;
      //
      case Parking_Space_001:
        name = "Parking_Space_Mark";
        type = "parkingSpace";
        subtype = "";
        break;
      case Parking_Space_002:
        name = "Time_Limit_Parking_Space_Mark";
        type = "parkingSpace";
        subtype = "";
        break;
      case Parking_Space_003:
        name = "Parking_6m";
        type = "parkingSpace";
        subtype = "";
        break;
      case Parking_Space_004:
        name = "Parking_5m";
        type = "parkingSpace";
        subtype = "";
        break;
      case Parking_Space_005:
        name = "Parking_45deg";
        type = "parkingSpace";
        subtype = "";
        break;
      case Parking_Space_006:
        name = "Parking_60deg";
        type = "parkingSpace";
        subtype = "";
        break;
      case Sensors_Camera:
        name = "Camera";
        type = "none";
        subtype = "";
        break;
      case Sensors_Radar:
        name = "Millimeter_Wave_Radar";
        type = "none";
        subtype = "";
        break;
      case Sensors_RSU:
        name = "RSU";
        type = "none";
        subtype = "";
        break;
      case Sensors_Lidar:
        name = "Lidar";
        type = "none";
        subtype = "";
        break;
      case Surface_Pothole:
        name = "Pothole";
        type = "none";
        subtype = "";
        break;
      case Surface_Patch:
        name = "Patch";
        type = "patch";
        subtype = "";
        break;
      case Surface_Crack:
        name = "Crack";
        type = "barrier";
        subtype = "";
        break;
      case Surface_Asphalt_Line:
        name = "Asphalt_Line";
        type = "none";
        subtype = "";
        break;
      case Surface_Rut_Track:
        name = "Rut_Track";
        type = "none";
        subtype = "";
        break;
      case Surface_Stagnant_Water:
        name = "Stagnant_Water";
        type = "none";
        subtype = "";
        break;
      case Surface_Protrusion:
        name = "Protrusion";
        type = "obstacle";
        subtype = "";
        break;
      case Surface_Well_Cover:
        name = "Well_Cover";
        type = "none";
        subtype = "";
        break;
      case SpeedBump_001:
        name = "Deceleration_Zone";
        type = "barrier";
        subtype = "";
        break;
      case Obstacle_Reflective_Road_Sign:
        name = "Reflective_Road_Sign";
        type = "barrier";
        subtype = "";
        break;
      case Obstacle_Parking_Hole:
        name = "Parking_Hole";
        type = "barrier";
        subtype = "";
        break;
      case Obstacle_Parking_Lot:
        name = "Parking_Lot";
        type = "barrier";
        subtype = "";
        break;
      case Obstacle_Ground_Lock:
        name = "Ground_Lock";
        type = "barrier";
        subtype = "";
        break;
      case Obstacle_Plastic_Vehicle_Stopper:
        name = "Plastic_Vehicle_Stopper";
        type = "barrier";
        subtype = "";
        break;
      case Obstacle_Parking_Limit_Position_Pole_2m:
        name = "Parking_Limit_Position_Pole_2m";
        type = "barrier";
        subtype = "";
        break;
      case Obstacle_Traffic_Barrier:
        name = "Traffic_Barrier";
        type = "obstacle";
        subtype = "";
        break;
      case Obstacle_Road_Curb:
        name = "Road_Curb";
        type = "barrier";
        subtype = "";
        break;
      case Obstacle_Lamp:
        name = "Lamp";
        type = "none";
        subtype = "";
        break;
      case Obstacle_Traffic_Cone:
        name = "Traffic_Cone";
        type = "obstacle";
        subtype = "";
        break;
      case Obstacle_Traffic_Horse:
        name = "Traffic_Horse";
        type = "obstacle";
        subtype = "";
        break;
      case Obstacle_GarbageCan:
        name = "GarbageCan";
        type = "obstacle";
        subtype = "";
        break;
      case Obstacle_Obstacle:
        name = "Obstacle";
        type = "obstacle";
        subtype = "";
        break;
      case Obstacle_Support_Vehicle_Stopper:
        name = "Support_Vehicle_Stopper";
        type = "barrier";
        subtype = "";
        break;
      case ChargingPile_001:
        name = "Charging_Station";
        type = "barrier";
        subtype = "";
        break;
      case Tree_001:
        name = "Tree";
        type = "tree";
        subtype = "";
        break;
      case Shrub_001:
        name = "Shrub";
        type = "vegetation";
        subtype = "";
        break;
      case Grass_001:
        name = "Grass";
        type = "vegetation";
        subtype = "";
        break;
      case Building_001:
        name = "Building";
        type = "building";
        subtype = "";
        break;
      case BusStation_001:
        name = "BusStation";
        type = "none";
        subtype = "";
        break;
      case Tunnel_001:
        name = "Tunnel";
        type = "none";
        subtype = "";
        break;
      case PedestrianBridge:
        name = "PedestrianBridge";
        type = "none";
        subtype = "";
        break;
      case CustomSubType:
        name = this->getName();
        type = "custom";
        subtype = "";
      default:
        break;
    }
  }  // others

  if (type == "null") {
    type = this->getName();
  }
}

void txObject::setPos(double x, double y, double z) {
  instancePtr->data.x = x;
  instancePtr->data.y = y;
  instancePtr->data.z = z;
}

void txObject::setLWH(double l, double w, double h) {
  instancePtr->data.length = l;
  instancePtr->data.width = w;
  instancePtr->data.height = h;
}

void txObject::setRPY(double r, double p, double y) {
  instancePtr->data.roll = r;
  instancePtr->data.pitch = p;
  instancePtr->data.yaw = y;
}

void txObject::setRawTypeString(const std::string& type, const std::string& subtype) {
  assert(type.size() < sizeof(instancePtr->data.rawtype));
  assert(subtype.size() < sizeof(instancePtr->data.rawsubtype));
  memcpy(instancePtr->data.rawtype, type.c_str(), type.size());
  memcpy(instancePtr->data.rawsubtype, subtype.c_str(), subtype.size());
}

void txObject::setUserData(const std::map<std::string, std::string>& userData) { instancePtr->userData = userData; }

const std::string& txParkingSpace::getAccess() const { return _access; }

const std::string& txParkingSpace::getRestrictions() const { return _restrictions; }

const ParkingMarking& txParkingSpace::getFrontMarking() const { return _mark[0]; }

const ParkingMarking& txParkingSpace::getRearMarking() const { return _mark[1]; }

const ParkingMarking& txParkingSpace::getLeftMarking() const { return _mark[2]; }

const ParkingMarking& txParkingSpace::getRightMarking() const { return _mark[3]; }

std::string& txParkingSpace::getAccess() { return _access; }

std::string& txParkingSpace::getRestrictions() { return _restrictions; }

ParkingMarking& txParkingSpace::getFrontMarking() { return _mark[0]; }

ParkingMarking& txParkingSpace::getRearMarking() { return _mark[1]; }

ParkingMarking& txParkingSpace::getLeftMarking() { return _mark[2]; }

ParkingMarking& txParkingSpace::getRightMarking() { return _mark[3]; }
}  // namespace hadmap
