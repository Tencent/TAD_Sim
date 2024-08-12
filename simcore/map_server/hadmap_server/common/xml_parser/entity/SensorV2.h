/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <json/value.h>
#include <tinyxml.h>

class SensorBaseV2 {
 public:
  TiXmlElement* toSaveXml();
  void fromJson(const Json::Value& json);
  std::string type;
  std::unordered_map<std::string, std::string> key2Value;
  std::vector<std::string> keys;
};

class Sensor {
 public:
  std::string idx;
  std::string ID = "0";
  std::string InstallSlot = "C0";
  std::string Device = ".0";
  std::string LocationX = "0";
  std::string LocationY = "0";
  std::string LocationZ = "0";
  std::string RotationX = "0";
  std::string RotationY = "0";
  std::string RotationZ = "0";
};
class SensorGroup {
 public:
  void parseXML(TiXmlElement* ele);
  void parseJson(Json::Value& json);
  void toJson(Json::Value& value);
  TiXmlElement* toSaveXml();
  std::string m_name;
  std::vector<Sensor> m_SensorVec;
};
using SensorGroupVec = std::vector<SensorGroup>;

class SensorDefine {
 public:
  void parseXML(TiXmlElement* ele);
  TiXmlElement* saveXml();
  std::string m_name;
  std::vector<SensorBaseV2> m_SensorBasePtrVec;
};
class SensorV2 {
 public:
  void parseXML(const char* filename);
  void parseXML(const TiXmlDocument& doc);
  void toJson(Json::Value& value);
  void toGroupJson(std::string groupname, Json::Value& value);
  bool saveXmlDefine(const char* filename);
  bool addSensorDefine(const char* filename, SensorBaseV2 baseinfo);
  bool updateSensorDefine(const char* filename, SensorBaseV2 baseinfo);
  bool deleteSensorDefine(const char* filename, std::string idx);
  bool addGroupSensor(SensorGroup group, std::string filename);
  bool deleteGroupSensor(std::string groupName, std::string filename);
  const SensorGroupVec& getSensorGroup();
  SensorDefine& getSensorDefineCatalog();

 private:
  SensorGroupVec SensorGroupCatalog;
  SensorDefine SensorDefineCatalog;

 private:
  static constexpr const char* kSensorCatalogTpl = R"(
<?xml version="1.0" encoding="UTF-8" ?>
<TADSim>
    <FileHeader revMajor="1" revMinor="0" date="2022-08-16T17:00:00" author="tadsim"/>
    <Catalog name="SensorGroup">
        <!-- 定义传感器配置组 -->
    </Catalog>
    <Catalog name="SensorDefine">
        <!-- 定义传感器配置 -->
    </Catalog>
</TADSim>
    )";
};
