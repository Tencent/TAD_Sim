// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "structs/base_struct.h"
#include "structs/hadmap_lane.h"
#include "structs/hadmap_objgeom.h"
#include "structs/hadmap_predef.h"

namespace hadmap {
class TXSIMMAP_API txObject {
 private:
  // object member var
  struct ObjData;
  std::unique_ptr<ObjData> instancePtr;

 public:
  txObject();

  virtual ~txObject();

 public:
  // get info of cur object
  // get sqlite data
  tx_object_t getTxData() const;

  // get xodr data
  tx_od_object_t getOdData() const;

  // get object id
  objectpkid getId() const;

  // get object type
  OBJECT_TYPE getObjectType() const;

  // get object subtype
  OBJECT_SUB_TYPE getObjectSubType() const;

  // get object name
  std::string getName() const;

  // get relied lane id
  void getReliedLaneIds(std::vector<txLaneId>& ids) const;

  // get geom size
  size_t getGeomSize() const;

  // get specified geom data
  txObjGeomPtr getGeom(size_t index = 0) const;

  // get fomat type
  void getObjectFromatType(std::string& name, std::string& type, std::string& subtype) const;

  // transform to od data
  void transform2OdData();

  // set oddata
  void setTxOdData(tx_od_object_t& odData);

  txLineCurvePtr getRepeat() const;

  // get road id
  roadpkid getRoadId() const;

  txPoint getPos() const;
  void getST(double& s, double& t) const;
  void getLWH(double& len, double& wid, double& hei) const;
  void getRPY(double& roll, double& pitch, double& yaw) const;
  //
  void getRawTypeString(std::string& type, std::string& subtype) const;  // no use 20230406
  //
  void getUserData(std::map<std::string, std::string>& userData) const;

 public:
  // set info of cur object
  txObject& setData(tx_object_t data);

  // set object id
  txObject& setId(objectpkid id);

  // set object type
  txObject& setObjectType(OBJECT_TYPE type);

  // set object name
  txObject& setName(const std::string& name);

  // add relied lane id
  txObject& addReliedLaneId(const txLaneId& laneId);

  // set relied lane id
  txObject& setReliedLaneIds(const std::vector<txLaneId>& laneIds);

  // add geom data
  txObject& addGeom(const txObjGeomPtr& geomPtr);

  //
  txObject& setRepeat(txLineCurvePtr geomPtr);

  // add road id
  txObject& setOdRoadId(const int& id);

  txObject& setLaneLinkid(const int& id);

  // transfer
  // only be used when geom is in ENU
  txObject& transfer(const txPoint& oldEnuC, const txPoint& newEnuC);

  // transfer to enu
  txObject& transfer2ENU(const txPoint& enuCenter);

  txObject& setST(const double s, const double t);

  void setObjectType(const std::string name, const std::string type, const std::string subType, int st = 0);
  void setPos(double x, double y, double z);
  void setLWH(double len, double wid, double hei);
  void setRPY(double roll, double pitch, double yaw);
  void setRawTypeString(const std::string& type, const std::string& subtype);
  void setUserData(const std::map<std::string, std::string>& userData);
};

// ParkingMarking struct
struct ParkingMarking {
  bool valid = false;
  std::string type;
  double width = 0;
  OBJECT_COLOR color = OBJECT_COLOR_White;
};

// txParkingSpace class
class TXSIMMAP_API txParkingSpace : public txObject {
 public:
  // Get the access
  const std::string& getAccess() const;

  // Get the restrictions
  const std::string& getRestrictions() const;

  // Get the front parking marking
  const ParkingMarking& getFrontMarking() const;

  // Get the rear parking marking
  const ParkingMarking& getRearMarking() const;

  // Get the left parking marking
  const ParkingMarking& getLeftMarking() const;

  // Get the right parking marking
  const ParkingMarking& getRightMarking() const;

  // Get the access (non-const version)
  std::string& getAccess();

  // Get the restrictions (non-const version)
  std::string& getRestrictions();

  // Get the front parking marking (non-const version)
  ParkingMarking& getFrontMarking();

  // Get the rear parking marking (non-const version)
  ParkingMarking& getRearMarking();

  // Get the left parking marking (non-const version)
  ParkingMarking& getLeftMarking();

  // Get the right parking marking (non-const version)
  ParkingMarking& getRightMarking();

 private:
  ParkingMarking _mark[4];
  std::string _access;
  std::string _restrictions;
};

}  // namespace hadmap
