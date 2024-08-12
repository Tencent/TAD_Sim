/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <json/json.h>
#include <tinyxml.h>

class CTrafficFlow {
 public:
  CTrafficFlow() {}
  virtual ~CTrafficFlow() {}
  Json::Value& JsonMeta() { return json_meta_; }
  std::string& FilePath() { return file_path_; }

  void ParseFromXml(TiXmlElement* xml_traffic_flow);
  void SerializeToXml(TiXmlElement* xml_traffic_flow);

 private:
  template <typename T>
  static T GetXmlAttribute(TiXmlElement* xml, const char* key, T default_value = {}) {
    if (!xml) {
      return default_value;
    }
    const char* p = xml->Attribute(key);
    if (p) {
      std::istringstream iss(p);
      iss >> default_value;
    }
    return default_value;
  }

  template <typename T>
  static void SetXmlAttribute(TiXmlElement* xml, const char* key, T value) {
    if (!xml) {
      return;
    }
    xml->SetAttribute(key, value);
  }

  static void SetXmlAttribute(TiXmlElement* xml, const char* key, double value) {
    if (xml) {
      xml->SetDoubleAttribute(key, value);
    }
  }

  // parse vehicleComposition
  void ParseVehicleComposition(TiXmlElement* xml_traffic_flow, Json::Value&);

  // parse vehComp
  void ParseVehComp(TiXmlElement* xml_veh_comp, Json::Value&);

  // parse vehicleInput and vehicleExit
  void ParseVehicleInputAndExit(TiXmlElement* xml_traffic_flow, Json::Value&);

  void ParseVehicleInputAndExitImpl(TiXmlElement*, const std::string&, const Json::Value&, Json::Value&);

  void ParseRouteGroup(TiXmlElement* xml_traffic_flow, Json::Value&);

  // parse location
  void ParseLocation(TiXmlElement* xml_locations, Json::Value&);

  void ParsePedestrianSim(TiXmlElement* xml_traffic_flow, Json::Value&);

  void SerializeVehicleComposition(const Json::Value& json_comps, TiXmlElement*);

  void SerializeVehComp(const Json::Value& json_comp, TiXmlElement*);

  void SerializeVehicleInputAndExit(const Json::Value& json_veh_points, TiXmlElement*);

  void SerializeVehicleInputAndExitImpl(const Json::Value&, const std::string&, TiXmlElement*);

  int SerializeLocation(const Json::Value&, const std::string&, TiXmlElement*);

  void SerializeRouteGroup(const Json::Value&, TiXmlElement*);

  void SerializeDefaultVehType(TiXmlElement*);

  void SerializeDefaultBehavior(TiXmlElement*);

  void SerializePedestrianSim(const Json::Value&, TiXmlElement*);

  int GetMaxLocationId(TiXmlElement*);

 private:
  Json::Value json_meta_;
  std::string file_path_;

  struct VehType {
    std::string id;
    std::string type;
    std::string length;
    std::string width;
    std::string height;
    std::string behavior;
    bool ignore = false;
    VehType(const std::string& i, const std::string& t, const std::string& l, const std::string& w,
            const std::string& h, const std::string& b, bool ignore = false)
        : id(i), type(t), length(l), width(w), height(h), behavior(b), ignore(ignore) {}
  };
  static std::map<std::string, VehType> veh_type_map_;

  static const int kVehTypeCount = 3;
  static const int kDefaultBehavior = 2;
  static const char* kVehExitString;
  static const char* kVehInputString;
  static const char* kVehRouteString;
};
