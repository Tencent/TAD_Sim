/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/SensorV2.h"

#include <algorithm>
#include <iostream>

#include "common/utils/misc.h"

void SensorGroup::parseXML(TiXmlElement* ele) {
  this->m_name = ele->Attribute("name");
  TiXmlElement* sensorEle = ele->FirstChildElement("Sensor");
  while (sensorEle) {
    Sensor tmp;
    tmp.idx = sensorEle->Attribute("idx");
    tmp.ID = sensorEle->Attribute("ID");
    tmp.InstallSlot = sensorEle->Attribute("InstallSlot");
    tmp.Device = sensorEle->Attribute("Device");
    tmp.LocationX = sensorEle->Attribute("LocationX");
    tmp.LocationY = sensorEle->Attribute("LocationY");
    tmp.LocationZ = sensorEle->Attribute("LocationZ");
    tmp.RotationX = sensorEle->Attribute("RotationX");
    tmp.RotationY = sensorEle->Attribute("RotationY");
    tmp.RotationZ = sensorEle->Attribute("RotationZ");
    this->m_SensorVec.push_back(tmp);
    sensorEle = sensorEle->NextSiblingElement("Sensor");
  }
}

void SensorGroup::parseJson(Json::Value& json) {
  this->m_name = json.get("groupName", "").asString();
  for (auto it : json["group"]) {
    Sensor tmp;
    tmp.idx = formatValue((it)["idx"]);
    tmp.Device = formatValue((it)["Device"]);
    tmp.ID = formatValue((it)["ID"]);
    tmp.InstallSlot = formatValue((it)["InstallSlot"]);
    tmp.LocationX = formatValue((it)["LocationX"]);
    tmp.LocationY = formatValue((it)["LocationY"]);
    tmp.LocationZ = formatValue((it)["LocationZ"]);
    tmp.RotationX = formatValue((it)["RotationX"]);
    tmp.RotationY = formatValue((it)["RotationY"]);
    tmp.RotationZ = formatValue((it)["RotationZ"]);
    this->m_SensorVec.push_back(tmp);
  }
}

void SensorGroup::toJson(Json::Value& value) {
  for (auto& sensor : m_SensorVec) {
    Json::Value group;
    group["Device"] = sensor.Device;
    group["ID"] = sensor.ID;
    group["InstallSlot"] = sensor.InstallSlot;
    group["LocationX"] = sensor.LocationX;
    group["LocationY"] = sensor.LocationY;
    group["LocationZ"] = sensor.LocationZ;
    group["RotationX"] = sensor.RotationX;
    group["RotationY"] = sensor.RotationY;
    group["RotationZ"] = sensor.RotationZ;
    group["idx"] = std::atoi(sensor.idx.c_str());
    value["group"].append(group);
  }
  value["groupName"] = m_name;
}

TiXmlElement* SensorGroup::toSaveXml() {
  TiXmlElement* SensorGroupEle = new TiXmlElement("SensorGroup");
  SensorGroupEle->SetAttribute("name", this->m_name);
  for (auto it : this->m_SensorVec) {
    TiXmlElement* sensorEle = new TiXmlElement("Sensor");
    sensorEle->SetAttribute("idx", it.idx);
    sensorEle->SetAttribute("ID", it.ID);
    sensorEle->SetAttribute("InstallSlot", it.InstallSlot);
    sensorEle->SetAttribute("Device", it.Device);
    sensorEle->SetAttribute("LocationX", it.LocationX);
    sensorEle->SetAttribute("LocationY", it.LocationY);
    sensorEle->SetAttribute("LocationZ", it.LocationZ);
    sensorEle->SetAttribute("RotationX", it.RotationX);
    sensorEle->SetAttribute("RotationY", it.RotationY);
    sensorEle->SetAttribute("RotationZ", it.RotationZ);
    SensorGroupEle->LinkEndChild(sensorEle);
  }
  return SensorGroupEle;
}

void SensorV2::parseXML(const char* filename) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(filename);
  if (!bRet) return;
  parseXML(doc);
}

void SensorV2::parseXML(const TiXmlDocument& doc) {
  TiXmlElement* xmlRoot = const_cast<TiXmlElement*>(doc.RootElement());
  if (!xmlRoot) return;
  TiXmlElement* elemCatalog = xmlRoot->FirstChildElement("Catalog");
  if (!elemCatalog) return;
  while (elemCatalog) {
    std::string catalogName = elemCatalog->Attribute("name");
    if (catalogName == "SensorGroup") {
      TiXmlElement* SensorGroupEle = elemCatalog->FirstChildElement("SensorGroup");
      while (SensorGroupEle) {
        SensorGroup group;
        group.parseXML(SensorGroupEle);
        SensorGroupEle = SensorGroupEle->NextSiblingElement("SensorGroup");
        this->SensorGroupCatalog.push_back(group);
      }
    } else if (catalogName == "SensorDefine") {
      SensorDefine define;
      define.parseXML(elemCatalog);
      this->SensorDefineCatalog = define;
    }
    elemCatalog = elemCatalog->NextSiblingElement("Catalog");
  }
}

void SensorV2::toJson(Json::Value& value) {
  for (size_t i = 0; i < this->SensorDefineCatalog.m_SensorBasePtrVec.size(); i++) {
    SensorBaseV2 base = this->SensorDefineCatalog.m_SensorBasePtrVec.at(i);
    Json::Value sensorJson;
    sensorJson["idx"] = std::atoi(base.key2Value["idx"].c_str());
    sensorJson["type"] = base.type;
    sensorJson["name"] = base.key2Value["name"];
    Json::Value paramJson;
    for (auto& kv : base.key2Value) {
      if (kv.first == "idx" || kv.first == "name") {
        continue;
      }
      if (std::find(base.keys.begin(), base.keys.end(), kv.first) != base.keys.end()) {
        continue;
      }
      paramJson[kv.first] = kv.second;
    }
    sensorJson["param"] = paramJson;
    value["sensors"].append(sensorJson);
  }
}

void SensorV2::toGroupJson(std::string groupname, Json::Value& value) {
  for (int i = 0; i < this->SensorGroupCatalog.size(); i++) {
    if (groupname == this->SensorGroupCatalog.at(i).m_name) {
      for (auto it : this->SensorGroupCatalog.at(i).m_SensorVec) {
        Json::Value sensor;
        sensor["idx"] = atoi(it.idx.c_str());
        sensor["ID"] = it.ID;
        sensor["InstallSlot"] = it.InstallSlot;
        sensor["Device"] = it.Device;
        sensor["LocationX"] = it.LocationX;
        sensor["LocationY"] = it.LocationY;
        sensor["LocationZ"] = it.LocationZ;
        sensor["RotationX"] = it.RotationX;
        sensor["RotationY"] = it.RotationY;
        sensor["RotationZ"] = it.RotationZ;
        value["group"].append(sensor);
      }
    }
  }
}

bool SensorV2::saveXmlDefine(const char* filename) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(filename);
  if (!bRet) return false;
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return false;
  TiXmlElement* elemCatalog = xmlRoot->FirstChildElement("Catalog");
  if (!elemCatalog) return false;
  while (elemCatalog) {
    std::string catalogName = elemCatalog->Attribute("name");
    if (catalogName == "SensorDefine") {
      SensorDefine define = this->SensorDefineCatalog;
      TiXmlElement* ele = define.saveXml();
      xmlRoot->RemoveChild(elemCatalog);
      xmlRoot->LinkEndChild(ele);
      doc.SaveFile(filename);
      return true;
    }
    elemCatalog = elemCatalog->NextSiblingElement("Catalog");
  }
  return false;
}

bool SensorV2::addSensorDefine(const char* filename, SensorBaseV2 baseinfo) {
  TiXmlDocument doc;
  if (!doc.LoadFile(filename)) {
    doc.Parse(kSensorCatalogTpl);
  }
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return false;
  TiXmlElement* elemCatalog = xmlRoot->FirstChildElement("Catalog");
  if (!elemCatalog) return false;
  while (elemCatalog) {
    std::string catalogName = elemCatalog->Attribute("name");
    if (catalogName == "SensorDefine") {
      TiXmlElement* ele = baseinfo.toSaveXml();
      if (!ele) {
        return false;
      }
      elemCatalog->LinkEndChild(ele);
      doc.SaveFile(filename);
      return true;
    }
    elemCatalog = elemCatalog->NextSiblingElement("Catalog");
  }
  return false;
}

bool SensorV2::updateSensorDefine(const char* filename, SensorBaseV2 baseinfo) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(filename);
  if (!bRet) return false;
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return false;
  TiXmlElement* elemCatalog = xmlRoot->FirstChildElement("Catalog");
  if (!elemCatalog) return false;
  while (elemCatalog) {
    std::string catalogName = elemCatalog->Attribute("name");
    if (catalogName == "SensorDefine") {
      TiXmlElement* sensorEle = elemCatalog->FirstChildElement();
      while (sensorEle) {
        if (sensorEle->Attribute("idx") == baseinfo.key2Value["idx"]) {
          TiXmlElement* ele = baseinfo.toSaveXml();
          elemCatalog->InsertAfterChild(sensorEle, *ele);
          elemCatalog->RemoveChild(sensorEle);
          doc.SaveFile(filename);
          return true;
        }
        sensorEle = sensorEle->NextSiblingElement();
      }
    }
    elemCatalog = elemCatalog->NextSiblingElement("Catalog");
  }
  return false;
}

bool SensorV2::deleteSensorDefine(const char* filename, std::string idx) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(filename);
  if (!bRet) return false;
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return false;
  TiXmlElement* elemCatalog = xmlRoot->FirstChildElement("Catalog");
  if (!elemCatalog) return false;
  while (elemCatalog) {
    std::string catalogName = elemCatalog->Attribute("name");
    if (catalogName == "SensorDefine") {
      TiXmlElement* sensorEle = elemCatalog->FirstChildElement();
      while (sensorEle) {
        if (sensorEle->Attribute("idx") == idx) {
          elemCatalog->RemoveChild(sensorEle);
          doc.SaveFile(filename);
          return true;
        }
        sensorEle = sensorEle->NextSiblingElement();
      }
    }
    elemCatalog = elemCatalog->NextSiblingElement("Catalog");
  }
  return false;
}

bool SensorV2::addGroupSensor(SensorGroup group, std::string filename) {
  TiXmlDocument doc;
  if (!doc.LoadFile(filename.c_str())) {
    doc.Parse(kSensorCatalogTpl);
  }
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return false;
  TiXmlElement* elemCatalog = xmlRoot->FirstChildElement("Catalog");
  if (!elemCatalog) return false;
  while (elemCatalog) {
    std::string catalogName = elemCatalog->Attribute("name");
    if (catalogName == "SensorGroup") {
      TiXmlElement* SensorGroupEle = elemCatalog->FirstChildElement("SensorGroup");
      while (SensorGroupEle) {
        if (SensorGroupEle->Attribute("name") == group.m_name) {
          TiXmlElement* ele = group.toSaveXml();
          elemCatalog->InsertAfterChild(SensorGroupEle, *ele);
          elemCatalog->RemoveChild(SensorGroupEle);
          doc.SaveFile(filename);
          return true;
        }
        SensorGroupEle = SensorGroupEle->NextSiblingElement("SensorGroup");
      }
      TiXmlElement* ele = group.toSaveXml();
      elemCatalog->LinkEndChild(ele);
      doc.SaveFile(filename);
      return true;
    }
    elemCatalog = elemCatalog->NextSiblingElement("Catalog");
  }
  return false;
}

bool SensorV2::deleteGroupSensor(std::string groupName, std::string filename) {
  TiXmlDocument doc;
  if (!doc.LoadFile(filename.c_str())) {
    doc.Parse(kSensorCatalogTpl);
  }
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return false;
  TiXmlElement* elemCatalog = xmlRoot->FirstChildElement("Catalog");
  if (!elemCatalog) return false;
  while (elemCatalog) {
    std::string catalogName = elemCatalog->Attribute("name");
    if (catalogName == "SensorGroup") {
      TiXmlElement* SensorGroupEle = elemCatalog->FirstChildElement("SensorGroup");
      while (SensorGroupEle) {
        if (SensorGroupEle->Attribute("name") == groupName) {
          elemCatalog->RemoveChild(SensorGroupEle);
          doc.SaveFile(filename);
          return true;
        }
        SensorGroupEle = SensorGroupEle->NextSiblingElement("SensorGroup");
      }
    }
    elemCatalog = elemCatalog->NextSiblingElement("Catalog");
  }
  return false;
}

const SensorGroupVec& SensorV2::getSensorGroup() { return SensorGroupCatalog; }

SensorDefine& SensorV2::getSensorDefineCatalog() { return SensorDefineCatalog; }

void SensorDefine::parseXML(TiXmlElement* ele) {
  TiXmlElement* eleSensorEle = ele->FirstChildElement();
  while (eleSensorEle) {
    std::string name = eleSensorEle->Value();
    std::vector<std::string> tmp;
    SensorBaseV2 sensorData;
    sensorData.type = name;
    for (TiXmlAttribute* attr = eleSensorEle->FirstAttribute(); attr; attr = attr->Next()) {
      std::string key = attr->Name();
      const char* val = attr->Value();
      if (!val) {
        continue;
      }
      sensorData.key2Value[key] = (key == "name" && strlen(val) == 0) ? name : std::string(val);
    }
    this->m_SensorBasePtrVec.push_back(sensorData);
    eleSensorEle = eleSensorEle->NextSiblingElement();
  }
}

TiXmlElement* SensorDefine::saveXml() {
  TiXmlElement* ele = new TiXmlElement("Catalog");
  ele->SetAttribute("name", "SensorDefine");
  for (auto& base : this->m_SensorBasePtrVec) {
    TiXmlElement* one = base.toSaveXml();
    if (!one) {
      ele->LinkEndChild(one);
    }
  }
  return ele;
}

TiXmlElement* SensorBaseV2::toSaveXml() {
  if (type.empty()) {
    return nullptr;
  }
  TiXmlElement* root = new TiXmlElement(this->type.c_str());
  for (auto& kv : this->key2Value) {
    root->SetAttribute(kv.first.c_str(), kv.second.c_str());
  }
  return root;
}

void SensorBaseV2::fromJson(const Json::Value& json) {
  // { "idx":0, "name":"", "type":"", "param":{} }
  type = formatValue(json.get("type", ""));
  key2Value["idx"] = formatValue(json.get("idx", 0));
  key2Value["name"] = formatValue(json.get("name", ""));
  Json::Value param = json.get("param", Json::Value(Json::objectValue));

  for (Json::Value::const_iterator iter = param.begin(); iter != param.end(); ++iter) {
    key2Value[iter.key().asString()] = formatValue(*iter);
  }
}
