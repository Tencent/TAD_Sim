/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once
#include <memory>
#include <string>
#include <vector>

#include <json/value.h>
#include <tinyxml.h>

#include "SensorV2.h"
#include "common/log/system_logger.h"

#define CATALOG_TYPE_EGO ("Ego")
#define CATALOG_TYPE_VEHICLE ("Vehicle")
#define CATALOG_TYPE_PEDESTRIAN ("Pedestrian")
#define CATALOG_TYPE_MISCOBJECT ("MiscObject")
#define DATA_CATALOG_TO_SYS ("../../../sys/scenario/Catalogs/")

enum VehicleType {
  VEHICLE_CAR,
  VEHICLE_TRUCK,
  VEHICLE_ALL,
};

class CBoundingBox {
 public:
  CBoundingBox() {}
  void toJson(Json::Value& jsonValue);
  void parseXML(TiXmlElement* elemPlanner);
  int parseFromJson(const Json::Value& json);
  static TiXmlElement* toTixmlFromJson(const Json::Value& json);

 public:
  std::string m_strX = "0.0";
  std::string m_strY = "0.0";
  std::string m_strZ = "0.0";
  std::string m_strWidth = "0.0";
  std::string m_strLength = "0.0";
  std::string m_strHeight = "0.0";
};
class CPerformance {
 public:
  CPerformance() {}
  void toJson(Json::Value& jsonValue);
  void parseXML(TiXmlElement* elemPlanner);
  static TiXmlElement* toTixmlFromJson(const Json::Value& json);

  std::string getMaxSpeed();
  std::string getMaxAcceleration();
  std::string getMaxDeceleration();

 private:
  std::string m_strMaxSpeed;
  std::string m_strMaxAcceleration;
  std::string m_strMaxDeceleration;
};
class CFrontAxle {
 public:
  CFrontAxle() {}
  void toJson(Json::Value& jsonValue);
  void parseXML(TiXmlElement* elemPlanner);
  static TiXmlElement* toTixmlFromJson(const Json::Value& json);

  TiXmlElement* saveToXml();

 public:
  std::string m_strMaxSteering;
  std::string m_strWheelDiameter;
  std::string m_strTrackWidth;
  std::string m_strPositionX;
  std::string m_strPositionZ;
};
class CRearAxle {
 public:
  CRearAxle() {}
  void toJson(Json::Value& jsonValue);
  void parseXML(TiXmlElement* elemPlanner);
  static TiXmlElement* toTixmlFromJson(const Json::Value& json);
  TiXmlElement* saveToXml();

 public:
  std::string m_strMaxSteering;
  std::string m_strWheelDiameter;
  std::string m_strTrackWidth;
  std::string m_strPositionX;
  std::string m_strPositionZ;
};
class CProperties {
 public:
  CProperties() {}
  void toJson(Json::Value& jsonValue);
  //
  static std::string findValueFromName(TiXmlElement* pPropertys, std::string strName);

 public:
  std::map<std::string, std::string> m_strName2Value;
};

class CTrailer {
 public:
  CTrailer() {}
  void toJson(Json::Value& jsonValue);
  void parseXML(TiXmlElement* elemPlanner);
  const std::string Name() { return m_strName; }

 public:
  std::string m_strName;
  std::string m_strTrailingUnit;
  std::string m_strOffsetX;
  std::string m_strOffsetZ;
};

// catalog
class CCatalogModel {
 public:
  virtual void parseXML(TiXmlElement* ptrEle) = 0;
  virtual void toJson(Json::Value& json) = 0;
  std::string& getName() { return m_strName; }
  CPerformance& getPeformance() { return m_objPer; }
  CBoundingBox& getBoundingBox() { return m_objBox; }

 protected:
  std::string m_strName;
  std::string m_strModel3d;
  CPerformance m_objPer;
  CRearAxle m_objRear;
  CFrontAxle m_objFront;
  CBoundingBox m_objBox;
};
// ------------------------------------      defaut.xosc          ------------------------------------
class CDriveCatalog {
 public:
  CDriveCatalog() {}
  void toJson(Json::Value& jsonValue);
  int fromJson(const std::string& jsonstr);
  int fromJson(const Json::Value& root);

  void parseXML(TiXmlElement* elemPlanner);
  void setTrailerName2CVehicle(std::string name, CDriveCatalog catalog);
  TiXmlElement* SaveToXml();
  std::vector<CTrailer> getVecTrailer();
  //
  std::string m_strName;
  std::string m_strModel3d;
  std::string m_strVehicleCategory;
  std::map<std::string, std::string> m_property;
  std::string m_strTractorName;
  VehicleType m_strType;
  // Properties
  std::string m_controlerName;
  std::string m_SensorGroup;
  std::string m_weight;
  std::string m_strFlagPreset;
  std::string m_strAliasName = "";
  std::string m_dynamicId;  /// Dynamic Id
  std::string m_strModelId = "";
  std::string m_strZhName = "";
  std::string m_strEnName = "";
  std::string m_strWeight;
  // SensorGroup
  SensorGroup m_group;

 public:
  std::string m_strOrigin;
  CBoundingBox m_objBox;
  CPerformance m_objPer;
  CRearAxle m_objRear;
  CFrontAxle m_objFront;
  std::vector<CTrailer> m_vecTrailer;
  std::vector<CDriveCatalog> m_TrailerName2CVehicle;
};
// ------------------------------------      pedestian            ------------------------------------
class CSinglePedestrian : public CCatalogModel {
 public:
  virtual void parseXML(TiXmlElement* ptrEle);
  virtual void toJson(Json::Value& json);

 private:
  std::string m_strCategory;
  std::string m_strMass;

  std::string m_modelId;
  std::string m_interfaceCategory = "";
  std::string m_zhName = "";
  std::string m_enName = "";
};
using CCatalogModelPtr = std::shared_ptr<CCatalogModel>;

// ------------------------------------      vehicle              ------------------------------------
class CSingleVechile : public CCatalogModel {
 public:
  virtual void parseXML(TiXmlElement* ptrEle);
  virtual void toJson(Json::Value& json);

 private:
  VehicleType m_strType;
  // Property
  std::string m_modelId;
  std::string m_interfaceCategory = "";
  std::string m_zhName = "";
  std::string m_enName = "";
  std::string m_strVehicleCategory;
};
// ------------------------------------      miscobject           ------------------------------------
class CSingleMisObject : public CCatalogModel {
 public:
  virtual void parseXML(TiXmlElement* ptrEle);
  virtual void toJson(Json::Value& json);

 public:
  std::string m_strCategory;
  std::string m_strMass;
  // property
  std::string m_modelId;
  std::string m_interfaceCategory = "";
  std::string m_zhName = "";
  std::string m_enName = "";
};
