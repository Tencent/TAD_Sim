/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "catalogbase.h"
#include <boost/algorithm/string.hpp>
#include "common/log/system_logger.h"
#include "common/utils/misc.h"
#define MAX_SPEED 55.56
#define MAX_ACC 12.0
#define MAX_DEC 10.0
// ------------------------------------       CBoundingBox          ------------------------------------
void CBoundingBox::toJson(Json::Value& jsonValue) {
  Json::Value center;
  center["x"] = std::atof(m_strX.c_str());
  center["y"] = std::atof(m_strY.c_str());
  center["z"] = std::atof(m_strZ.c_str());
  jsonValue["center"] = center;
  Json::Value dimensions;
  dimensions["width"] = std::atof(m_strWidth.c_str());
  dimensions["length"] = std::atof(m_strLength.c_str());
  dimensions["height"] = std::atof(m_strHeight.c_str());
  jsonValue["dimensions"] = dimensions;
  return;
}

void CBoundingBox::parseXML(TiXmlElement* ptrEle) {
  if (ptrEle == NULL) return;

  TiXmlElement* elemCenter = ptrEle->FirstChildElement("Center");
  if (elemCenter) {
    const char* pX = elemCenter->Attribute("x");
    if (pX) {
      m_strX = pX;
    }
    const char* pY = elemCenter->Attribute("y");
    if (pY) {
      m_strY = pY;
    }
    const char* pZ = elemCenter->Attribute("z");
    if (pZ) {
      m_strZ = pZ;
    }
  }
  elemCenter = ptrEle->FirstChildElement("Dimensions");
  if (elemCenter) {
    const char* pX = elemCenter->Attribute("width");
    if (pX) {
      m_strWidth = pX;
    }
    const char* pY = elemCenter->Attribute("length");
    if (pY) {
      m_strLength = pY;
    }
    const char* pZ = elemCenter->Attribute("height");
    if (pZ) {
      m_strHeight = pZ;
    }
  }
}

int CBoundingBox::parseFromJson(const Json::Value& json) {
  if (json.empty()) return -1;
  m_strWidth = std::to_string(json["dimensions"].get("width", 0).asDouble());
  m_strLength = std::to_string(json["dimensions"].get("length", 0).asDouble());
  m_strHeight = std::to_string(json["dimensions"].get("height", 0).asDouble());
  m_strX = std::to_string(json["center"].get("x", 0).asDouble());
  m_strY = std::to_string(json["center"].get("y", 0).asDouble());
  m_strZ = std::to_string(json["center"].get("z", 0).asDouble());
}

TiXmlElement* CBoundingBox::toTixmlFromJson(const Json::Value& json) {
  // create boundingbox node
  TiXmlElement* pBoundingBox = new TiXmlElement("BoundingBox");
  TiXmlElement* pCenter = new TiXmlElement("Center");
  pCenter->SetDoubleAttribute("x", json["center"].get("x", 0).asDouble());
  pCenter->SetDoubleAttribute("y", json["center"].get("y", 0).asDouble());
  pCenter->SetDoubleAttribute("z", json["center"].get("z", 0).asDouble());
  pBoundingBox->InsertEndChild(*pCenter);

  TiXmlElement* pDimensions = new TiXmlElement("Dimensions");
  pDimensions->SetDoubleAttribute("height", json["dimensions"].get("height", 0).asDouble());
  pDimensions->SetDoubleAttribute("length", json["dimensions"].get("length", 0).asDouble());
  pDimensions->SetDoubleAttribute("width", json["dimensions"].get("width", 0).asDouble());
  pBoundingBox->InsertEndChild(*pDimensions);

  return pBoundingBox;
}
// ------------------------------------       CPerformance          ------------------------------------
void CPerformance::toJson(Json::Value& jsonValue) {
  jsonValue["maxSpeed"] = std::atof(m_strMaxSpeed.c_str());
  jsonValue["maxAcceleration"] = std::atof(m_strMaxAcceleration.c_str());
  jsonValue["maxDeceleration"] = std::atof(m_strMaxDeceleration.c_str());
}

void CPerformance::parseXML(TiXmlElement* ptrEle) {
  if (ptrEle == NULL) return;
  const char* pX = ptrEle->Attribute("maxSpeed");
  if (pX) {
    ptrEle->RemoveAttribute("maxSpeed");
    ptrEle->SetDoubleAttribute("maxSpeed", MAX_SPEED);
    m_strMaxSpeed = std::to_string(MAX_SPEED);
  }
  pX = ptrEle->Attribute("maxAcceleration");
  if (pX) {
    ptrEle->RemoveAttribute("maxAcceleration");
    ptrEle->SetDoubleAttribute("maxAcceleration", MAX_ACC);
    m_strMaxAcceleration = std::to_string(MAX_ACC);
  }
  pX = ptrEle->Attribute("maxDeceleration");
  if (pX) {
    ptrEle->RemoveAttribute("maxDeceleration");
    ptrEle->SetDoubleAttribute("maxDeceleration", MAX_DEC);
    m_strMaxDeceleration = std::to_string(MAX_DEC);
  }
}

TiXmlElement* CPerformance::toTixmlFromJson(const Json::Value& json) {
  TiXmlElement* pPerformance = new TiXmlElement("Performance");
  pPerformance->SetDoubleAttribute("maxSpeed", json.get("maxSpeed", 0).asDouble());
  pPerformance->SetDoubleAttribute("maxAcceleration", json.get("maxAcceleration", 0).asDouble());
  pPerformance->SetDoubleAttribute("maxDeceleration", json.get("maxDeceleration", 0).asDouble());
  return pPerformance;
}

std::string CPerformance::getMaxSpeed() { return this->m_strMaxSpeed; }

std::string CPerformance::getMaxAcceleration() { return this->m_strMaxAcceleration; }

std::string CPerformance::getMaxDeceleration() { return this->m_strMaxDeceleration; }
// ------------------------------------       CFrontAxle          ------------------------------------
void CFrontAxle::toJson(Json::Value& jsonValue) {
  jsonValue["maxSteering"] = std::atof(m_strMaxSteering.c_str());
  jsonValue["wheelDiameter"] = std::atof(m_strWheelDiameter.c_str());
  jsonValue["trackWidth"] = std::atof(m_strTrackWidth.c_str());
  jsonValue["positionX"] = std::atof(m_strPositionX.c_str());
  jsonValue["positionZ"] = std::atof(m_strPositionZ.c_str());
}
void CFrontAxle::parseXML(TiXmlElement* ptrEle) {
  if (ptrEle == NULL) return;
  TiXmlElement* elemCenter = ptrEle->FirstChildElement("FrontAxle");
  if (elemCenter) {
    const char* pValue = elemCenter->Attribute("maxSteering");
    if (pValue) {
      m_strMaxSteering = pValue;
    }
    pValue = elemCenter->Attribute("wheelDiameter");
    if (pValue) {
      m_strWheelDiameter = pValue;
    }
    pValue = elemCenter->Attribute("trackWidth");
    if (pValue) {
      m_strTrackWidth = pValue;
    }
    pValue = elemCenter->Attribute("positionX");
    if (pValue) {
      m_strPositionX = pValue;
    }
    pValue = elemCenter->Attribute("positionZ");
    if (pValue) {
      m_strPositionZ = pValue;
    }
  }
}
TiXmlElement* CFrontAxle::toTixmlFromJson(const Json::Value& json) {
  TiXmlElement* FrontAxleEle = new TiXmlElement("FrontAxle");
  FrontAxleEle->SetAttribute("maxSteering", std::to_string(json.get("maxSteering", 0).asDouble()));
  FrontAxleEle->SetAttribute("wheelDiameter", std::to_string(json.get("wheelDiameter", 0).asDouble()));
  FrontAxleEle->SetAttribute("trackWidth", std::to_string(json.get("trackWidth", 0).asDouble()));
  FrontAxleEle->SetAttribute("positionX", std::to_string(json.get("positionX", 0).asDouble()));
  FrontAxleEle->SetAttribute("positionZ", std::to_string(json.get("positionZ", 0).asDouble()));
  return FrontAxleEle;
}
TiXmlElement* CFrontAxle::saveToXml() {
  TiXmlElement* FrontAxleEle = new TiXmlElement("FrontAxle");
  FrontAxleEle->SetAttribute("maxSteering", this->m_strMaxSteering);
  FrontAxleEle->SetAttribute("wheelDiameter", this->m_strWheelDiameter);
  FrontAxleEle->SetAttribute("trackWidth", this->m_strTrackWidth);
  FrontAxleEle->SetAttribute("positionX", this->m_strPositionX);
  FrontAxleEle->SetAttribute("positionZ", this->m_strPositionZ);
  return FrontAxleEle;
}

// ------------------------------------       CRearAxle          ------------------------------------
void CRearAxle::toJson(Json::Value& jsonValue) {
  jsonValue["maxSteering"] = std::atof(m_strMaxSteering.c_str());
  jsonValue["wheelDiameter"] = std::atof(m_strWheelDiameter.c_str());
  jsonValue["trackWidth"] = std::atof(m_strTrackWidth.c_str());
  jsonValue["positionX"] = std::atof(m_strPositionX.c_str());
  jsonValue["positionZ"] = std::atof(m_strPositionZ.c_str());
}
void CRearAxle::parseXML(TiXmlElement* ptrEle) {
  if (ptrEle == NULL) return;
  TiXmlElement* elemCenter = ptrEle->FirstChildElement("RearAxle");
  if (elemCenter) {
    const char* pValue = elemCenter->Attribute("maxSteering");
    if (pValue) {
      m_strMaxSteering = pValue;
    }
    pValue = elemCenter->Attribute("wheelDiameter");
    if (pValue) {
      m_strWheelDiameter = pValue;
    }
    pValue = elemCenter->Attribute("trackWidth");
    if (pValue) {
      m_strTrackWidth = pValue;
    }
    pValue = elemCenter->Attribute("positionX");
    if (pValue) {
      m_strPositionX = pValue;
    }
    pValue = elemCenter->Attribute("positionZ");
    if (pValue) {
      m_strPositionZ = pValue;
    }
  }
}
TiXmlElement* CRearAxle::toTixmlFromJson(const Json::Value& json) {
  TiXmlElement* FrontAxleEle = new TiXmlElement("RearAxle");
  FrontAxleEle->SetAttribute("maxSteering", std::to_string(json.get("maxSteering", 0).asDouble()));
  FrontAxleEle->SetAttribute("wheelDiameter", std::to_string(json.get("wheelDiameter", 0).asDouble()));
  FrontAxleEle->SetAttribute("trackWidth", std::to_string(json.get("trackWidth", 0).asDouble()));
  FrontAxleEle->SetAttribute("positionX", std::to_string(json.get("positionX", 0).asDouble()));
  FrontAxleEle->SetAttribute("positionZ", std::to_string(json.get("positionZ", 0).asDouble()));
  return FrontAxleEle;
}
TiXmlElement* CRearAxle::saveToXml() {
  TiXmlElement* FrontAxleEle = new TiXmlElement("RearAxle");
  FrontAxleEle->SetAttribute("maxSteering", this->m_strMaxSteering);
  FrontAxleEle->SetAttribute("wheelDiameter", this->m_strWheelDiameter);
  FrontAxleEle->SetAttribute("trackWidth", this->m_strTrackWidth);
  FrontAxleEle->SetAttribute("positionX", this->m_strPositionX);
  FrontAxleEle->SetAttribute("positionZ", this->m_strPositionZ);
  return FrontAxleEle;
}

// ------------------------------------      CTrailer          ------------------------------------
void CTrailer::toJson(Json::Value& jsonValue) {
  jsonValue["name"] = m_strName;
  jsonValue["trailingUnit"] = m_strTrailingUnit;
  jsonValue["offsetX"] = m_strOffsetX;
  jsonValue["offsetZ"] = m_strOffsetZ;
}
void CTrailer::parseXML(TiXmlElement* elemCenter) {
  if (elemCenter == NULL) return;
  const char* pValue = elemCenter->Attribute("name");
  if (pValue) {
    m_strName = pValue;
  }
  pValue = elemCenter->Attribute("trailingUnit");
  if (pValue) {
    m_strTrailingUnit = pValue;
  }
  pValue = elemCenter->Attribute("offsetX");
  if (pValue) {
    m_strOffsetX = pValue;
  }
  pValue = elemCenter->Attribute("offsetZ");
  if (pValue) {
    m_strOffsetZ = pValue;
  }
}
void CDriveCatalog::toJson(Json::Value& jsonValue) {
  jsonValue["variable"] = m_strName;
  jsonValue["catalogCategory"] = "ego";
  jsonValue["catalogSubCategory"] = m_strVehicleCategory;
  // car or truck other info
  {
    Json::Value param;
    param["model3d"] = m_strModel3d;
    // boundingBox
    Json::Value boudingbox;
    m_objBox.toJson(boudingbox);
    boudingbox["trailerOffsetX"] = 0;
    boudingbox["trailerOffsetZ"] = 0;
    param["boundingBox"] = boudingbox;
    // performance
    Json::Value jsonPerfor;
    m_objPer.toJson(jsonPerfor);
    param["performance"] = jsonPerfor;
    // axles
    Json::Value jsonFront, jsonRear;
    m_objFront.toJson(jsonFront);
    m_objRear.toJson(jsonRear);
    Json::Value jAxles;
    jAxles["frontAxle"] = jsonFront;
    jAxles["rearAxle"] = jsonRear;
    param["axles"] = jAxles;
    // referencePoint
    Json::Value jPoint;
    jPoint["origin"] = m_strOrigin;
    param["referencePoint"] = jPoint;
    // properties
    Json::Value jProperties;
    jProperties["controller"] = m_controlerName;
    jProperties["interfaceCategory"] = "ego";
    if (!m_strZhName.empty()) jProperties["zhName"] = m_strZhName;
    if (!m_strEnName.empty()) jProperties["enName"] = m_strEnName;
    jProperties["sensorGroup"] = std::atoi(m_SensorGroup.c_str());
    jProperties["weight"] = std::atof(m_weight.c_str());
    jProperties["dynamic"] = std::atoi(m_dynamicId.c_str());
    jProperties["preset"] = ((m_strFlagPreset == "true") ? true : false);
    jProperties["modelId"] = m_strModelId;
    jProperties["alias"] = m_strAliasName;
    param["properties"] = jProperties;
    // catalogParams
    jsonValue["catalogParams"].append(param);
  }
  // m_Trailer
  {
    for (auto tmp : this->m_TrailerName2CVehicle) {
      Json::Value param;
      param["model3d"] = tmp.m_strModel3d;
      // boundingBox
      Json::Value boudingbox;
      tmp.m_objBox.toJson(boudingbox);
      bool flag = true;
      for (int i = 0; i < this->m_vecTrailer.size(); i++) {
        if (tmp.m_strName == this->m_vecTrailer.at(i).m_strName) {
          boudingbox["trailerOffsetX"] = std::atof(this->m_vecTrailer.at(i).m_strOffsetX.c_str());
          boudingbox["trailerOffsetZ"] = std::atof(this->m_vecTrailer.at(i).m_strOffsetZ.c_str());
          flag = true;
          break;
        }
      }
      if (!flag) {
        boudingbox["trailerOffsetX"] = 0;
        boudingbox["trailerOffsetZ"] = 0;
      }
      param["boundingBox"] = boudingbox;
      // performance
      Json::Value jsonPerfor;
      tmp.m_objPer.toJson(jsonPerfor);
      param["performance"] = jsonPerfor;
      // axles
      Json::Value jsonFront, jsonRear;
      tmp.m_objFront.toJson(jsonFront);
      tmp.m_objRear.toJson(jsonRear);
      Json::Value jAxles;
      jAxles["frontAxle"] = jsonFront;
      jAxles["rearAxle"] = jsonRear;
      param["axles"] = jAxles;
      // referencePoint
      Json::Value jPoint;
      jPoint["origin"] = m_strOrigin;
      param["referencePoint"] = jPoint;
      // properties
      Json::Value jProperties;
      jProperties["weight"] = std::atof(tmp.m_weight.c_str());
      param["properties"] = jProperties;
      jsonValue["catalogParams"].append(param);
    }
  }
}

// ------------------------------------      defaut.xosc          ------------------------------------
int CDriveCatalog::fromJson(const std::string& jsonstr) {
  Json::Value root = StringToJson(jsonstr);
  Json::Value catalog = root.get("catalog", Json::Value(Json::objectValue));
  return fromJson(catalog);
}

int CDriveCatalog::fromJson(const Json::Value& root) {
  m_strName = root["vehicle_name"].asString();
  m_strAliasName = root["vehicle_alias"].asString();
  m_strFlagPreset = formatValue(root["preset"]);
  m_strVehicleCategory = root["vehicle_category"].asString();
  m_controlerName = root["vehicle_controller"].asString();
  m_dynamicId = root["dynamic_id"].asString();
  Json::Value vehicle_param = root["vehicle_param"];
  if (m_strVehicleCategory == "car") {
    for (auto it : vehicle_param) {
      m_strModel3d = it["model3d"].asString();
      // Center
      Json::Value center = it["Center"];
      m_objBox.m_strX = center["x"].asString();
      m_objBox.m_strY = center["y"].asString();
      m_objBox.m_strZ = center["z"].asString();
      Json::Value Dimensions = it["Dimensions"];
      m_objBox.m_strWidth = Dimensions["width"].asString();
      m_objBox.m_strLength = Dimensions["length"].asString();
      m_objBox.m_strHeight = Dimensions["height"].asString();
      // FrontAxle
      Json::Value FrontAxle = it["FrontAxle"];
      m_objFront.m_strMaxSteering = FrontAxle["maxSteering"].asString();
      m_objFront.m_strWheelDiameter = FrontAxle["wheelDiameter"].asString();
      m_objFront.m_strTrackWidth = FrontAxle["trackWidth"].asString();
      m_objFront.m_strPositionX = FrontAxle["positionX"].asString();
      m_objFront.m_strPositionZ = FrontAxle["positionZ"].asString();
      // RearAxle
      Json::Value RearAxle = it["RearAxle"];
      m_objRear.m_strMaxSteering = RearAxle["maxSteering"].asString();
      m_objRear.m_strWheelDiameter = RearAxle["wheelDiameter"].asString();
      m_objRear.m_strTrackWidth = RearAxle["trackWidth"].asString();
      m_objRear.m_strPositionX = RearAxle["positionX"].asString();
      m_objRear.m_strPositionZ = RearAxle["positionZ"].asString();
      // weight
      m_weight = it["weight"].asString();
    }
  } else {
    int index = 0;
    for (auto it : vehicle_param) {
      CDriveCatalog tmp;
      tmp.m_strModel3d = it["model3d"].asString();
      // Center
      Json::Value center = it["Center"];
      tmp.m_objBox.m_strX = center["x"].asString();
      tmp.m_objBox.m_strY = center["y"].asString();
      tmp.m_objBox.m_strZ = center["z"].asString();
      Json::Value Dimensions = it["Dimensions"];
      tmp.m_objBox.m_strWidth = Dimensions["width"].asString();
      tmp.m_objBox.m_strLength = Dimensions["length"].asString();
      tmp.m_objBox.m_strHeight = Dimensions["height"].asString();
      // FrontAxle
      Json::Value FrontAxle = it["FrontAxle"];
      tmp.m_objFront.m_strMaxSteering = FrontAxle["maxSteering"].asString();
      tmp.m_objFront.m_strWheelDiameter = FrontAxle["wheelDiameter"].asString();
      tmp.m_objFront.m_strTrackWidth = FrontAxle["trackWidth"].asString();
      tmp.m_objFront.m_strPositionX = FrontAxle["positionX"].asString();
      tmp.m_objFront.m_strPositionZ = FrontAxle["positionZ"].asString();
      // RearAxle
      Json::Value RearAxle = it["RearAxle"];
      tmp.m_objRear.m_strMaxSteering = RearAxle["maxSteering"].asString();
      tmp.m_objRear.m_strWheelDiameter = RearAxle["wheelDiameter"].asString();
      tmp.m_objRear.m_strTrackWidth = RearAxle["trackWidth"].asString();
      tmp.m_objRear.m_strPositionX = RearAxle["positionX"].asString();
      tmp.m_objRear.m_strPositionZ = RearAxle["positionZ"].asString();
      // weight
      tmp.m_weight = it["weight"].asString();
      if (index == 0) {
        tmp.m_strName = m_strName + "_truck1";
        tmp.m_strVehicleCategory = "truck";
        // tmp.m_oldName = oldname + "_truck1";
        m_strTractorName = tmp.m_strName;
      } else {
        tmp.m_strName = m_strName + "_trailer" + std::to_string(index);
        tmp.m_strVehicleCategory = "trailer";
        // tmp.m_oldName = oldname + "_trailer" + std::to_string(index);
        CTrailer trailer;
        trailer.m_strName = tmp.m_strName;
        trailer.m_strTrailingUnit = std::to_string(index);
        trailer.m_strOffsetX = formatValue(it["trailerOffsetX"]);
        trailer.m_strOffsetZ = formatValue(it["trailerOffsetZ"]);

        m_vecTrailer.push_back(trailer);
      }
      index++;
      m_TrailerName2CVehicle.push_back(tmp);
    }
  }
  // sensor
  SensorGroup group;
  group.m_name = root["sensor"]["groupName"].asString();
  for (auto it = root["sensor"]["group"].begin(); it != root["sensor"]["group"].end(); it++) {
    Sensor tmp;
    tmp.idx = formatValue((*it)["idx"]);
    tmp.Device = formatValue((*it)["Device"]);
    tmp.ID = formatValue((*it)["ID"]);
    tmp.InstallSlot = formatValue((*it)["InstallSlot"]);
    tmp.LocationX = formatValue((*it)["LocationX"]);
    tmp.LocationY = formatValue((*it)["LocationY"]);
    tmp.LocationZ = formatValue((*it)["LocationZ"]);
    tmp.RotationX = formatValue((*it)["RotationX"]);
    tmp.RotationY = formatValue((*it)["RotationY"]);
    tmp.RotationZ = formatValue((*it)["RotationZ"]);
    group.m_SensorVec.push_back(tmp);
  }
  m_group = group;
  return 0;
}

void CDriveCatalog::parseXML(TiXmlElement* ptrEle) {
  if (ptrEle == NULL) return;
  const char* pValue = ptrEle->Attribute("name");
  if (pValue) {
    m_strName = pValue;
  }
  pValue = ptrEle->Attribute("vehicleCategory");
  if (pValue) {
    m_strVehicleCategory = pValue;
  }
  pValue = ptrEle->Attribute("model3d");
  if (pValue) {
    m_strModel3d = pValue;
  }

  TiXmlElement* elemCenter = ptrEle->FirstChildElement("BoundingBox");
  if (elemCenter) {
    m_objBox.parseXML(elemCenter);
  }
  elemCenter = ptrEle->FirstChildElement("Performance");
  if (elemCenter) {
    m_objPer.parseXML(elemCenter);
  }
  elemCenter = ptrEle->FirstChildElement("Axles");
  if (elemCenter) {
    m_objFront.parseXML(elemCenter);
    m_objRear.parseXML(elemCenter);
  }
  elemCenter = ptrEle->FirstChildElement("ReferencePoint");
  if (elemCenter) {
    pValue = elemCenter->Attribute("origin");
    if (pValue) m_strOrigin = pValue;
  }
  elemCenter = ptrEle->FirstChildElement("Tractor");
  if (elemCenter) {
    pValue = elemCenter->Attribute("name");
    if (pValue) m_strTractorName = pValue;
  }
  elemCenter = ptrEle->FirstChildElement("Trailer");
  while (elemCenter) {
    CTrailer obj;
    obj.parseXML(elemCenter);
    m_vecTrailer.push_back(obj);
    elemCenter = elemCenter->NextSiblingElement("Trailer");
  }
  elemCenter = ptrEle->FirstChildElement("Properties");
  {
    this->m_property.clear();
    if (elemCenter) {
      TiXmlElement* eleProperty = elemCenter->FirstChildElement("Property");
      while (eleProperty) {
        this->m_property.insert(std::make_pair(eleProperty->Attribute("name"), eleProperty->Attribute("value")));
        if (std::string(eleProperty->Attribute("name")) == "controller") {
          this->m_controlerName = eleProperty->Attribute("value");
        } else if (boost::iequals(std::string(eleProperty->Attribute("name")), "SensorGroup")) {
          this->m_SensorGroup = eleProperty->Attribute("value");
        } else if (boost::iequals(std::string(eleProperty->Attribute("name")), "Weight")) {
          this->m_weight = eleProperty->Attribute("value");
        } else if (boost::iequals(std::string(eleProperty->Attribute("name")), "Preset")) {
          this->m_strFlagPreset = eleProperty->Attribute("value");
        } else if (boost::iequals(std::string(eleProperty->Attribute("name")), "Alias")) {
          this->m_strAliasName = eleProperty->Attribute("value");
        } else if (boost::iequals(std::string(eleProperty->Attribute("name")), "Dynamic")) {
          this->m_dynamicId = eleProperty->Attribute("value");
        } else if (boost::iequals(std::string(eleProperty->Attribute("name")), "model3d")) {
          this->m_strModel3d = eleProperty->Attribute("value");
        } else if (std::string(eleProperty->Attribute("name")) == "modelId") {
          this->m_strModelId = eleProperty->Attribute("value");
        } else if (std::string(eleProperty->Attribute("name")) == "enName") {
          this->m_strEnName = eleProperty->Attribute("value");
        } else if (std::string(eleProperty->Attribute("name")) == "zhName") {
          this->m_strZhName = eleProperty->Attribute("value");
        }
        eleProperty = eleProperty->NextSiblingElement("Property");
      }
    }
  }
  //
}

// ------------------------------------      pedestian            ------------------------------------
void CSinglePedestrian::parseXML(TiXmlElement* ptrEle) {
  if (ptrEle == NULL) return;
  const char* pValue = ptrEle->Attribute("name");
  if (pValue) {
    m_strName = pValue;
  }
  pValue = ptrEle->Attribute("pedestrianCategory");
  if (pValue) {
    m_strCategory = pValue;
  }
  pValue = ptrEle->Attribute("model");
  if (pValue) {
    m_strModel3d = pValue;
  }
  pValue = ptrEle->Attribute("mass");
  if (pValue) {
    m_strMass = pValue;
  }
  TiXmlElement* elemCenter = ptrEle->FirstChildElement("BoundingBox");
  if (elemCenter) {
    m_objBox.parseXML(elemCenter);
  }
  TiXmlElement* PropertiesEle = ptrEle->FirstChildElement("Properties");
  if (PropertiesEle) {
    TiXmlElement* propertyEle = PropertiesEle->FirstChildElement("Property");
    while (propertyEle) {
      std::string strproperty = propertyEle->Attribute("name");
      std::string strpropertyValue = propertyEle->Attribute("value");
      if (strproperty == "model3d") {
        this->m_strModel3d = strpropertyValue;
      }
      if (strproperty == "model_id") {
        this->m_modelId = strpropertyValue;
      }
      if (strproperty == "interfaceCategory") {
        this->m_interfaceCategory = strpropertyValue;
      }
      if (strproperty == "zhName") {
        this->m_zhName = strpropertyValue;
      }
      if (strproperty == "enName") {
        this->m_enName = strpropertyValue;
      }
      propertyEle = propertyEle->NextSiblingElement("Property");
    }
  }
}

void CSinglePedestrian::toJson(Json::Value& jsonValue) {
  jsonValue["variable"] = m_strName;
  jsonValue["catalogCategory"] = "pedestrian";
  jsonValue["catalogSubCategory"] = m_strCategory;
  Json::Value param;
  param["model3d"] = m_strModel3d;
  // boudingBox
  Json::Value jBoudingBox;
  m_objBox.toJson(jBoudingBox);
  param["boundingBox"] = jBoudingBox;
  // properties
  Json::Value jProperties;
  jProperties["modelId"] = m_modelId;
  jProperties["preset"] = true;
  if (!m_interfaceCategory.empty()) jProperties["interfaceCategory"] = m_interfaceCategory;
  if (!m_zhName.empty()) jProperties["zhName"] = m_zhName;
  if (!m_enName.empty()) jProperties["enName"] = m_zhName;
  param["properties"] = jProperties;
  jsonValue["catalogParams"] = param;
}

// ------------------------------------      vehicle              ------------------------------------
void CSingleVechile::parseXML(TiXmlElement* ptrEle) {
  if (ptrEle == NULL) return;
  const char* pValue = ptrEle->Attribute("name");
  if (pValue) {
    m_strName = pValue;
  }
  pValue = ptrEle->Attribute("vehicleCategory");
  if (pValue) {
    m_strVehicleCategory = pValue;
  }
  pValue = ptrEle->Attribute("model3d");
  if (pValue) {
    m_strModel3d = pValue;
  }
  TiXmlElement* elemCenter = ptrEle->FirstChildElement("BoundingBox");
  if (elemCenter) {
    m_objBox.parseXML(elemCenter);
  }
  elemCenter = ptrEle->FirstChildElement("Performance");
  if (elemCenter) {
    m_objPer.parseXML(elemCenter);
  }
  elemCenter = ptrEle->FirstChildElement("Axles");
  if (elemCenter) {
    m_objFront.parseXML(elemCenter);
    m_objRear.parseXML(elemCenter);
  }

  TiXmlElement* PropertiesEle = ptrEle->FirstChildElement("Properties");
  if (PropertiesEle) {
    TiXmlElement* propertyEle = PropertiesEle->FirstChildElement("Property");
    while (propertyEle) {
      std::string strproperty = propertyEle->Attribute("name");
      std::string strpropertyValue = propertyEle->Attribute("value");
      if (strproperty == "model3d") {
        this->m_strModel3d = strpropertyValue;
      }
      if (strproperty == "model_id") {
        this->m_modelId = strpropertyValue;
      }
      if (strproperty == "interfaceCategory") {
        this->m_interfaceCategory = strpropertyValue;
      }
      if (strproperty == "zhName") {
        this->m_zhName = strpropertyValue;
      }
      if (strproperty == "enName") {
        this->m_enName = strpropertyValue;
      }
      propertyEle = propertyEle->NextSiblingElement("Property");
    }
  }
}

void CSingleVechile::toJson(Json::Value& jsonValue) {
  jsonValue["variable"] = m_strName;
  jsonValue["catalogCategory"] = "car";
  jsonValue["catalogSubCategory"] = m_strVehicleCategory;
  Json::Value param;
  param["model3d"] = m_strModel3d;
  // boundingBox
  Json::Value boudingbox;
  m_objBox.toJson(boudingbox);
  boudingbox["trailerOffsetX"] = 0;
  boudingbox["trailerOffsetZ"] = 0;
  param["boundingBox"] = boudingbox;
  // performance
  Json::Value jsonPerfor;
  m_objPer.toJson(jsonPerfor);
  param["performance"] = jsonPerfor;
  // axles
  Json::Value jsonFront, jsonRear;
  m_objFront.toJson(jsonFront);
  m_objRear.toJson(jsonRear);
  Json::Value jAxles;
  jAxles["frontAxle"] = jsonFront;
  jAxles["rearAxle"] = jsonRear;
  param["axles"] = jAxles;
  // properties
  Json::Value jProperties;
  if (!m_zhName.empty()) jProperties["zhName"] = m_zhName;
  if (!m_enName.empty()) jProperties["enName"] = m_enName;
  if (!m_enName.empty()) jProperties["interfaceCategory"] = m_interfaceCategory;

  jProperties["modelId"] = m_modelId;
  jProperties["preset"] = true;
  param["properties"] = jProperties;
  // catalogParams
  jsonValue["catalogParams"].append(param);
}
// ------------------------------------      miscobject           ------------------------------------

void CSingleMisObject::parseXML(TiXmlElement* ptrEle) {
  if (ptrEle == NULL) return;
  const char* pValue = ptrEle->Attribute("name");
  if (pValue) {
    m_strName = pValue;
  }
  pValue = ptrEle->Attribute("miscObjectCategory");
  if (pValue) {
    m_strCategory = pValue;
  }
  pValue = ptrEle->Attribute("model");
  if (pValue) {
    m_strModel3d = pValue;
  }
  pValue = ptrEle->Attribute("mass");
  if (pValue) {
    m_strMass = pValue;
  }
  TiXmlElement* elemCenter = ptrEle->FirstChildElement("BoundingBox");
  if (elemCenter) {
    m_objBox.parseXML(elemCenter);
  }
  TiXmlElement* PropertiesEle = ptrEle->FirstChildElement("Properties");
  if (PropertiesEle) {
    TiXmlElement* propertyEle = PropertiesEle->FirstChildElement("Property");
    while (propertyEle) {
      std::string strproperty = propertyEle->Attribute("name");
      std::string strpropertyValue = propertyEle->Attribute("value");
      if (strproperty == "model3d") {
        this->m_strModel3d = strpropertyValue;
      }
      if (strproperty == "model_id") {
        this->m_modelId = strpropertyValue;
      }
      if (strproperty == "interfaceCategory") {
        this->m_interfaceCategory = strpropertyValue;
      }
      if (strproperty == "zhName") {
        this->m_zhName = strpropertyValue;
      }
      if (strproperty == "enName") {
        this->m_enName = strpropertyValue;
      }
      propertyEle = propertyEle->NextSiblingElement("Property");
    }
  }
}

void CSingleMisObject::toJson(Json::Value& jsonValue) {
  jsonValue["variable"] = m_strName;
  jsonValue["catalogCategory"] = "obstacle";
  jsonValue["catalogSubCategory"] = m_strCategory;
  Json::Value param;
  param["model3d"] = m_strModel3d;
  // boudingBox
  Json::Value jBoudingBox;
  m_objBox.toJson(jBoudingBox);
  param["boundingBox"] = jBoudingBox;
  // properties
  Json::Value jProperties;
  jProperties["modelId"] = m_modelId;
  jProperties["mass"] = m_strMass;
  jProperties["preset"] = true;
  if (!m_interfaceCategory.empty()) jProperties["interfaceCategory"] = m_interfaceCategory;
  if (!m_zhName.empty()) jProperties["zhName"] = m_zhName;
  if (!m_enName.empty()) jProperties["enName"] = m_zhName;
  param["properties"] = jProperties;
  jsonValue["catalogParams"] = param;
}

void CProperties::toJson(Json::Value& jsonValue) {}

std::string CProperties::findValueFromName(TiXmlElement* pPropertys, std::string name) {
  TiXmlElement* pProperty = pPropertys->FirstChildElement("Property");
  while (pProperty) {
    const std::string strName = pProperty->Attribute("name");
    const std::string strValue = pProperty->Attribute("value");
    if (strName == name) {
      return strValue;
    }
    pProperty = pProperty->NextSiblingElement("Property");
  }
  return "";
}
