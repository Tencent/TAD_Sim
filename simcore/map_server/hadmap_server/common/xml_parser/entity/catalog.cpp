/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/catalog.h"
#include <set>

#include "common/log/system_logger.h"
#include "common/utils/misc.h"
#include "common/utils/stringhelper.h"

TiXmlElement* CDriveCatalog::SaveToXml() {
  TiXmlElement* VehicleEle = new TiXmlElement("Vehicle");
  VehicleEle->SetAttribute("name", this->m_strName);
  VehicleEle->SetAttribute("vehicleCategory", this->m_strVehicleCategory);
  // VehicleEle->SetAttribute("model3d", this->m_strModel3d);
  // BoundingBoxEle
  TiXmlElement* BoundingBoxEle = new TiXmlElement("BoundingBox");
  TiXmlElement* CenterEle = new TiXmlElement("Center");
  CenterEle->SetAttribute("x", this->m_objBox.m_strX);
  CenterEle->SetAttribute("y", this->m_objBox.m_strY);
  CenterEle->SetAttribute("z", this->m_objBox.m_strZ);

  TiXmlElement* DimensionsEle = new TiXmlElement("Dimensions");
  DimensionsEle->SetAttribute("width", this->m_objBox.m_strWidth);
  DimensionsEle->SetAttribute("length", this->m_objBox.m_strLength);
  DimensionsEle->SetAttribute("height", this->m_objBox.m_strHeight);
  BoundingBoxEle->LinkEndChild(CenterEle);
  BoundingBoxEle->LinkEndChild(DimensionsEle);
  VehicleEle->LinkEndChild(BoundingBoxEle);
  // PerformanceEle
  TiXmlElement* PerformanceEle = new TiXmlElement("Performance");
  PerformanceEle->SetAttribute("maxSpeed", "55.56");
  PerformanceEle->SetAttribute("maxAcceleration", "12");
  PerformanceEle->SetAttribute("maxDeceleration", "10");
  VehicleEle->LinkEndChild(PerformanceEle);
  // Axles
  TiXmlElement* AxlesEle = new TiXmlElement("Axles");
  TiXmlElement* FrontAxleEle = this->m_objFront.saveToXml();
  TiXmlElement* RearAxleEle = this->m_objRear.saveToXml();
  AxlesEle->LinkEndChild(FrontAxleEle);
  AxlesEle->LinkEndChild(RearAxleEle);
  VehicleEle->LinkEndChild(AxlesEle);
  // Properties
  TiXmlElement* Properties = new TiXmlElement("Properties");
  std::map<std::string, std::string> PropertiesKeyAndValue;
  //
  if (this->m_strVehicleCategory == "car") {
    PropertiesKeyAndValue["controller"] = this->m_controlerName;
    PropertiesKeyAndValue["SensorGroup"] = this->m_group.m_name;
    PropertiesKeyAndValue["Weight"] = this->m_weight;
    PropertiesKeyAndValue["Preset"] = this->m_strFlagPreset;
    PropertiesKeyAndValue["Alias"] = this->m_strAliasName;
    PropertiesKeyAndValue["Dynamic"] = this->m_dynamicId;
    PropertiesKeyAndValue["model3d"] = this->m_strModel3d;
    PropertiesKeyAndValue["model_id"] = "-1";
    TiXmlElement* ReferencePointEle = new TiXmlElement("ReferencePoint");
    ReferencePointEle->SetAttribute("origin", "RearAxle_SnappedToGround");
    VehicleEle->LinkEndChild(ReferencePointEle);

  } else if (this->m_strVehicleCategory == "combination") {
    PropertiesKeyAndValue["controller"] = this->m_controlerName;
    PropertiesKeyAndValue["SensorGroup"] = this->m_group.m_name;
    PropertiesKeyAndValue["Preset"] = this->m_strFlagPreset;
    PropertiesKeyAndValue["Alias"] = this->m_strAliasName;
    PropertiesKeyAndValue["Dynamic"] = this->m_dynamicId;
    PropertiesKeyAndValue["model3d"] = this->m_strModel3d;
    PropertiesKeyAndValue["model_id"] = "-1";
    TiXmlElement* TractorEle = new TiXmlElement("Tractor");
    TractorEle->SetAttribute("name", this->m_strTractorName);
    VehicleEle->LinkEndChild(TractorEle);

    for (size_t i = 0; i < this->m_vecTrailer.size(); i++) {
      TiXmlElement* TrailerEle = new TiXmlElement("Trailer");
      TrailerEle->SetAttribute("name", this->m_vecTrailer.at(i).m_strName);
      TrailerEle->SetAttribute("trailingUnit", this->m_vecTrailer.at(i).m_strTrailingUnit);
      TrailerEle->SetAttribute("offsetX", this->m_vecTrailer.at(i).m_strOffsetX);
      TrailerEle->SetAttribute("offsetZ", this->m_vecTrailer.at(i).m_strOffsetZ);
      VehicleEle->LinkEndChild(TrailerEle);
    }
    TiXmlElement* ReferencePointEle = new TiXmlElement("ReferencePoint");
    ReferencePointEle->SetAttribute("origin", "RearAxle_SnappedToGround");
    VehicleEle->LinkEndChild(ReferencePointEle);
  } else if (this->m_strVehicleCategory == "truck") {
    PropertiesKeyAndValue["Weight"] = this->m_weight;
    TiXmlElement* ReferencePointEle = new TiXmlElement("ReferencePoint");
    ReferencePointEle->SetAttribute("origin", "RearAxle_SnappedToGround");
    VehicleEle->LinkEndChild(ReferencePointEle);
  } else if (this->m_strVehicleCategory == "trailer") {
    PropertiesKeyAndValue["Weight"] = this->m_weight;
    TiXmlElement* ReferencePointEle = new TiXmlElement("ReferencePoint");
    ReferencePointEle->SetAttribute("origin", "RearAxle_SnappedToGround");
    VehicleEle->LinkEndChild(ReferencePointEle);
  }

  for (auto it : PropertiesKeyAndValue) {
    TiXmlElement* Property = new TiXmlElement("Property");
    Property->SetAttribute("name", it.first);
    Property->SetAttribute("value", it.second);
    Properties->LinkEndChild(Property);
  }
  VehicleEle->LinkEndChild(Properties);
  return VehicleEle;
}

void CDriveCatalog::setTrailerName2CVehicle(std::string name, CDriveCatalog catalog) {
  this->m_TrailerName2CVehicle.push_back(catalog);
}

std::vector<CTrailer> CDriveCatalog::getVecTrailer() { return this->m_vecTrailer; }

int CCataLog::allModelToJson(const char* DataCatalogDir, Json::Value& json) {  // default xosc
  Json::Value driverJson;
  boost::filesystem::path path_data_catalog = DataCatalogDir;
  boost::filesystem::path path_sys_catalog = DataCatalogDir;
  boost::filesystem::path path_models = DataCatalogDir;
  path_sys_catalog /= DATA_CATALOG_TO_SYS;
  path_models /= ("../models");
  // ego default.xosc
  this->parseDefaultXosc(DataCatalogDir, "");
  this->egoModelToJson(driverJson);
  for (auto& it_ego : driverJson) {
    std::string strVariable = it_ego["variable"].asString();
    for (auto& it_param : it_ego["catalogParams"]) {
      boost::filesystem::path model3d_path = path_models;
      model3d_path /= it_param["model3d"].asString();
      // truck is empty
      if (!it_param["model3d"].asString().empty())
        it_param["model3d"] = boost::filesystem::system_complete(boost::filesystem::canonical(model3d_path)).string();
    }
  }
  json["driver_catalog"] = driverJson;

  // vehicle catalog xosc
  Json::Value vechileJson;
  boost::filesystem::path path_sys_vehicle_catalog = path_sys_catalog;
  path_sys_vehicle_catalog /= ("Vehicles/VehicleCatalog.xosc");
  noEgoModelToJson(boost::filesystem::canonical(path_sys_vehicle_catalog).string().c_str(), CATALOG_TYPE_VEHICLE,
                   vechileJson);
  boost::filesystem::path path_data_vehicle_catalog = DataCatalogDir;
  path_data_vehicle_catalog /= ("Vehicles/VehicleCatalog.xosc");
  noEgoModelToJson(boost::filesystem::canonical(path_data_vehicle_catalog).string().c_str(), CATALOG_TYPE_VEHICLE,
                   vechileJson);
  for (auto& it_vec : vechileJson) {
    std::string strVariable = it_vec["variable"].asString();
    for (auto& it_param : it_vec["catalogParams"]) {
      if (utils::IsStartWith(it_param["model3d"].asString().c_str(), "user_")) {
        boost::filesystem::path model3d_path = path_models;
        model3d_path /= it_param["model3d"].asString();
        it_param["model3d"] = boost::filesystem::system_complete(boost::filesystem::canonical(model3d_path)).string();
        it_param["properties"]["preset"] = false;
      }
    }
  }
  json["vehicle_catalog"] = vechileJson;

  // Pedestrian catalog xosc
  Json::Value PedestrianJson;
  boost::filesystem::path path_sys_pedestrian_catalog = path_sys_catalog;
  path_sys_pedestrian_catalog /= ("Pedestrians/PedestrianCatalog.xosc");
  noEgoModelToJson(path_sys_pedestrian_catalog.string().c_str(), CATALOG_TYPE_PEDESTRIAN, PedestrianJson);
  boost::filesystem::path path_data_pedestrian_catalog = DataCatalogDir;
  path_data_pedestrian_catalog /= ("Pedestrians/PedestrianCatalog.xosc");
  noEgoModelToJson(path_data_pedestrian_catalog.string().c_str(), CATALOG_TYPE_PEDESTRIAN, PedestrianJson);
  for (auto& it_ped : PedestrianJson) {
    std::string strVariable = it_ped["variable"].asString();
    if (utils::IsStartWith(it_ped["catalogParams"]["model3d"].asString().c_str(), "user_")) {
      boost::filesystem::path model3d_path = path_models;
      model3d_path /= it_ped["catalogParams"]["model3d"].asString();
      it_ped["catalogParams"]["model3d"] =
          boost::filesystem::system_complete(boost::filesystem::canonical(model3d_path)).string();
      it_ped["catalogParams"]["properties"]["preset"] = false;
    }
  }
  json["pedestrian_catalog"] = PedestrianJson;

  // MiscObect catalog xosc
  Json::Value MiscobjectJson;
  boost::filesystem::path path_sys_miscobject_catalog = path_sys_catalog;
  path_sys_miscobject_catalog /= ("MiscObjects/MiscObjectCatalog.xosc");
  noEgoModelToJson(path_sys_miscobject_catalog.string().c_str(), CATALOG_TYPE_MISCOBJECT, MiscobjectJson);
  boost::filesystem::path path_data_miscobject_catalog = DataCatalogDir;
  path_data_miscobject_catalog /= ("MiscObjects/MiscObjectCatalog.xosc");
  noEgoModelToJson(path_data_miscobject_catalog.string().c_str(), CATALOG_TYPE_MISCOBJECT, MiscobjectJson);
  for (auto& it_obj : MiscobjectJson) {
    std::string strVariable = it_obj["variable"].asString();
    if (utils::IsStartWith(it_obj["catalogParams"]["model3d"].asString().c_str(), "user_")) {
      boost::filesystem::path model3d_path = path_models;
      model3d_path /= it_obj["catalogParams"]["model3d"].asString();
      it_obj["catalogParams"]["model3d"] =
          boost::filesystem::system_complete(boost::filesystem::canonical(model3d_path)).string();
      it_obj["catalogParams"]["properties"]["preset"] = false;
    }
  }
  json["miscobject_catalog"] = MiscobjectJson;
  return 0;
}

int CCataLog::egoModelToJson(Json::Value& jsonValue) {
  for (size_t i = 0; i < m_vecList.size(); i++) {
    Json::Value jsonRow;
    CDriveCatalog& obj = m_vecList.at(i);
    if (obj.m_strFlagPreset == std::string("true")) {
      obj.toJson(jsonRow);
      jsonValue.append(jsonRow);
    }
  }
  for (size_t i = 0; i < m_vecList.size(); i++) {
    Json::Value jsonRow;
    CDriveCatalog& obj = m_vecList.at(i);
    if (obj.m_strFlagPreset != std::string("true")) {
      obj.toJson(jsonRow);
      jsonValue.append(jsonRow);
    }
  }
  return 0;
}

int CCataLog::noEgoModelToJson(const char* xoscpath, const std::string catalogtype, Json::Value& json) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(xoscpath);
  if (!bRet) return -1;
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return false;
  TiXmlElement* elemCatalog = xmlRoot->FirstChildElement("Catalog");
  if (!elemCatalog) return false;
  TiXmlElement* elemPedestrian = elemCatalog->FirstChildElement(catalogtype);
  while (elemPedestrian) {
    if (catalogtype == CATALOG_TYPE_VEHICLE) {
      CSingleVechile obj;
      obj.parseXML(elemPedestrian);
      Json::Value jVehicle;
      obj.toJson(jVehicle);
      json.append(jVehicle);
    } else if (catalogtype == CATALOG_TYPE_PEDESTRIAN) {
      CSinglePedestrian obj;
      obj.parseXML(elemPedestrian);
      Json::Value jPedestrian;
      obj.toJson(jPedestrian);
      json.append(jPedestrian);
    } else if (catalogtype == CATALOG_TYPE_MISCOBJECT) {
      CSingleMisObject obj;
      obj.parseXML(elemPedestrian);
      Json::Value jMiscObject;
      obj.toJson(jMiscObject);
      json.append(jMiscObject);
    }
    elemPedestrian = elemPedestrian->NextSiblingElement(catalogtype);
  }
  return 0;
}

int CCataLog::parseDefaultXosc(const char* DataCatalogDir, const char* SysCataLogDir) {
  boost::filesystem::path path_data_catalog = DataCatalogDir;
  path_data_catalog /= "Vehicles/default.xosc";
  parseDefaultXosc(boost::filesystem::canonical(path_data_catalog).string().c_str());
  if (!std::string(SysCataLogDir).empty()) {
    boost::filesystem::path path_sys_catalog = SysCataLogDir;
    path_sys_catalog /= "Vehicles/default.xosc";
    parseDefaultXosc(boost::filesystem::canonical(path_sys_catalog).string().c_str());
  }
  return 0;
}

int CCataLog::parseDefaultXosc(std::string strDefaultXoscPath) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strDefaultXoscPath.c_str());
  if (!bRet) {
    SYSTEM_LOGGER_ERROR("Load xosc file failure : %s", strDefaultXoscPath.c_str());
    return -1;
  }
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) {
    SYSTEM_LOGGER_ERROR("Xosc file is empty : %s", strDefaultXoscPath.c_str());
    return -1;
  }
  TiXmlElement* elemCatalog = xmlRoot->FirstChildElement("Catalog");
  if (!elemCatalog) {
    SYSTEM_LOGGER_ERROR("Xosc file not find 'Catalog' element  : %s", strDefaultXoscPath.c_str());
    return -1;
  }
  while (elemCatalog) {
    std::string name = elemCatalog->Attribute("name");
    if (name == "CarCatalog") {
      TiXmlElement* elemCenter = elemCatalog->FirstChildElement("Vehicle");
      while (elemCenter) {
        CDriveCatalog obj;
        obj.parseXML(elemCenter);
        obj.m_strType = VEHICLE_CAR;
        m_vecList.push_back(obj);
        elemCenter = elemCenter->NextSiblingElement("Vehicle");
      }
    } else if (name == "TruckCatalog") {
      std::map<std::string, CDriveCatalog> name2Map;
      TiXmlElement* elemCenter = elemCatalog->FirstChildElement("Vehicle");
      while (elemCenter) {
        CDriveCatalog obj;
        obj.parseXML(elemCenter);
        name2Map[obj.m_strName] = obj;
        elemCenter = elemCenter->NextSiblingElement("Vehicle");
      }
      for (auto it : name2Map) {
        if (it.second.m_strVehicleCategory == "combination") {
          auto Trailers = it.second.getVecTrailer();
          std::string TractorName = it.second.m_strTractorName;
          if (name2Map.find(TractorName) != name2Map.end()) {
            it.second.setTrailerName2CVehicle(TractorName, name2Map.find(TractorName)->second);
          }
          for (auto itTrailer : Trailers) {
            std::string name = itTrailer.Name();
            if (name2Map.find(name) != name2Map.end()) {
              it.second.setTrailerName2CVehicle(name, name2Map.find(name)->second);
            }
          }
          it.second.m_strType = VEHICLE_TRUCK;
          m_vecList.push_back(it.second);
        }
      }
    }
    elemCatalog = elemCatalog->NextSiblingElement("Catalog");
  }
  return 0;
}

bool CCataLog::getCatalog(CDriveCatalog& log, const std::string filename) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(filename.c_str());
  if (!bRet) return false;
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return false;
  TiXmlElement* CatalogEle = xmlRoot->FirstChildElement("Catalog");
  while (CatalogEle) {
    std::string catalogName = CatalogEle->Attribute("name");
    // get car catalog
    if (log.m_strVehicleCategory == "car" && catalogName == "CarCatalog") {
      TiXmlElement* VehicleEle = CatalogEle->FirstChildElement("Vehicle");
      while (VehicleEle) {
        std::string VehicleName = VehicleEle->Attribute("name");
        if (VehicleName == log.m_strName) {
          log.parseXML(VehicleEle);
          return true;
        }
        VehicleEle = VehicleEle->NextSiblingElement("Vehicle");
      }
    } else if (log.m_strVehicleCategory == "combination" && catalogName == "TruckCatalog") {  // get truck catalog
      TiXmlElement* VehicleEle = CatalogEle->FirstChildElement("Vehicle");
      while (VehicleEle) {
        std::string VehicleName = VehicleEle->Attribute("name");
        std::string VehicleCategory = VehicleEle->Attribute("vehicleCategory");
        if (VehicleName == log.m_strName || VehicleCategory == "combination") {
          log.parseXML(VehicleEle);
          return true;
        }
        VehicleEle = VehicleEle->NextSiblingElement("Vehicle");
      }
      return true;
    }
    CatalogEle = CatalogEle->NextSiblingElement("Catalog");
  }
  return false;
}

bool CCataLog::deleteCatalog(std::string catalogname, std::string filename) {
  for (auto it : m_vecList) {
    TiXmlDocument doc;
    bool bRet = doc.LoadFile(filename.c_str());
    if (!bRet) return false;
    TiXmlElement* xmlRoot = doc.RootElement();
    if (!xmlRoot) return false;
    TiXmlElement* CatalogEle = xmlRoot->FirstChildElement("Catalog");
    while (CatalogEle) {
      std::string catalogName = CatalogEle->Attribute("name");
      if (catalogName == "CarCatalog") {
        TiXmlElement* VehicleEle = CatalogEle->FirstChildElement("Vehicle");
        while (VehicleEle) {
          std::string VehicleName = VehicleEle->Attribute("name");
          if (VehicleName == catalogname) {
            CatalogEle->RemoveChild(VehicleEle);
            doc.SaveFile(filename);
            return true;
          }
          VehicleEle = VehicleEle->NextSiblingElement("Vehicle");
        }
      } else if (catalogName == "TruckCatalog") {
        if (it.m_strName == catalogname) {
          TiXmlElement* VehicleEle = CatalogEle->FirstChildElement("Vehicle");
          std::vector<std::string> vehicleNameVec;
          vehicleNameVec.push_back(catalogname);
          vehicleNameVec.push_back(it.m_strTractorName);
          for (auto iter : it.m_vecTrailer) {
            vehicleNameVec.push_back(iter.m_strName);
          }
          CDriveCatalog log;
          while (VehicleEle) {
            std::string VehicleName = VehicleEle->Attribute("name");
            for (std::string itName : vehicleNameVec) {
              if (itName == VehicleName) {
                CatalogEle->RemoveChild(VehicleEle);
                break;
              }
            }
            VehicleEle = VehicleEle->NextSiblingElement("Vehicle");
          }
          doc.SaveFile(filename);
          return true;
        }
      }
      CatalogEle = CatalogEle->NextSiblingElement("Catalog");
    }
  }
  return false;
}

std::string CCataLog::findCatalogName(std::string egoname, VehicleType type) {
  if (type == VehicleType::VEHICLE_CAR) {
    for (auto it : this->m_vecList) {
      if (it.m_strVehicleCategory == "car" && it.m_strAliasName == egoname) {
        return it.m_strName;
      }
    }
    return "";
  } else if (type == VehicleType::VEHICLE_TRUCK) {
    for (auto it : this->m_vecList) {
      if (it.m_strVehicleCategory == "combination" && it.m_strAliasName == egoname) {
        return it.m_strName;
      }
    }
    return "";
  }
  return "";
}

int CCataLog::getEgoModelFromName(const char* dataCatalogDir, std::string strName, CDriveCatalog& driveCatalog) {
  findEgoModelFromName(dataCatalogDir, strName);
  for (auto it : getDriveCatalogList()) {
    if (it.m_strName == strName) {
      driveCatalog = it;
      return 0;
    }
  }
  return -1;
}

bool CCataLog::findEgoModelFromName(const char* DataCatalogDir, std::string strName, VehicleType type) {
  m_vecList.clear();
  boost::filesystem::path path_data_catalog = DataCatalogDir;
  boost::filesystem::path path_sys_catalog = DataCatalogDir;
  path_sys_catalog /= DATA_CATALOG_TO_SYS;
  this->parseDefaultXosc(DataCatalogDir, path_sys_catalog.string().c_str());
  for (auto it : getDriveCatalogList()) {
    if (it.m_strName == strName) {
      return true;
    }
  }
  return false;
}

std::vector<CDriveCatalog> CCataLog::getDriveCatalogList() { return m_vecList; }

int CCataLog::toTixmlFromJson(const Json::Value& root, TiXmlElement*& timxml, int index) {
  timxml->SetAttribute("name", root.get("variable", "").asString());
  Json::Value first_element;
  if (root["catalogParams"].isArray()) {
    first_element = root["catalogParams"][index];
  } else {
    first_element = root["catalogParams"];
  }
  // boundingBox
  if (first_element.isMember("boundingBox")) {
    timxml->LinkEndChild((CBoundingBox::toTixmlFromJson(first_element["boundingBox"])));
  }

  // performance
  if (first_element.isMember("performance")) {
    timxml->LinkEndChild((CPerformance::toTixmlFromJson(first_element["performance"])));
  }

  // Axles
  if (first_element.isMember("axles")) {
    TiXmlElement* pAxles = new TiXmlElement("Axles");
    pAxles->LinkEndChild(CFrontAxle::toTixmlFromJson(first_element["axles"]["frontAxle"]));
    pAxles->LinkEndChild(CRearAxle::toTixmlFromJson(first_element["axles"]["rearAxle"]));
    timxml->LinkEndChild(pAxles);
  }

  // Properties
  TiXmlElement* pProperties = new TiXmlElement("Properties");
  std::map<std::string, std::string> mapPropertyName2Value;
  if (!first_element["properties"].isNull()) {
    Json::Value jProperties = first_element["properties"];
    for (auto it = jProperties.begin(); it != jProperties.end(); ++it) {
      TiXmlElement* pProperty = new TiXmlElement("Property");
      pProperty->SetAttribute("name", fromJsonName2XmlName(it.name()));
      if (it->isString()) {
        std::string str = it->asString().data();
        std::cout << str;
        pProperty->SetAttribute("value", str.c_str());
      } else if (it->isInt()) {
        pProperty->SetAttribute("value", it->asInt());
      } else if (it->isDouble()) {
        pProperty->SetDoubleAttribute("value", it->asDouble());
      } else if (it->isBool()) {
        if (it->asBool()) {
          pProperty->SetAttribute("value", "true");
        } else {
          pProperty->SetAttribute("value", "false");
        }
      } else {
        pProperty->SetAttribute("value", "");
      }
      pProperties->InsertEndChild(*pProperty);
    }
  }

  // Set model3d
  std::string model3d = first_element.get("model3d", "").asString();
  TiXmlElement* pProperty = new TiXmlElement("Property");
  pProperty->SetAttribute("name", "model3d");
  pProperty->SetAttribute("value", model3d);
  pProperties->InsertEndChild(*pProperty);
  timxml->LinkEndChild(pProperties);

  // ReferencePoint
  if (first_element.isMember("referencePoint")) {
    TiXmlElement* pReferencePoint = new TiXmlElement("ReferencePoint");
    pReferencePoint->SetAttribute("origin", first_element["referencePoint"]["origin"].asString());
    timxml->LinkEndChild(pReferencePoint);
  }

  return 0;
}

int CCataLog::addModelFromJson(const char* CatalogDir, const Json::Value& json) {
  const std::string strCatalogCategory = json.get("catalogCategory", "").asString();
  boost::filesystem::path catalogdir = CatalogDir;
  // geos
  if (strCatalogCategory == "ego") {
    catalogdir /= "Vehicles";
    if (!boost::filesystem::exists(catalogdir)) {
      boost::filesystem::create_directory(catalogdir);
    }
    catalogdir /= "default.xosc";
    addEgoModelFromJson(catalogdir.string().c_str(), json);
  } else if (strCatalogCategory == "car") {
    // vehicle
    catalogdir /= "Vehicles";
    if (!boost::filesystem::exists(catalogdir)) {
      boost::filesystem::create_directory(catalogdir);
    }
    catalogdir /= "VehicleCatalog.xosc";
    addNoEgoModelFromJson(catalogdir.string().c_str(), "Vehicle", json);
  } else if (strCatalogCategory == "pedestrian") {
    // pedestrian
    catalogdir /= "Pedestrians";
    if (!boost::filesystem::exists(catalogdir)) {
      boost::filesystem::create_directory(catalogdir);
    }
    catalogdir /= "PedestrianCatalog.xosc";
    addNoEgoModelFromJson(catalogdir.string().c_str(), "Pedestrian", json);
  } else if (strCatalogCategory == "obstacle") {
    // miscobject
    catalogdir /= "MiscObjects";
    if (!boost::filesystem::exists(catalogdir)) {
      boost::filesystem::create_directory(catalogdir);
    }
    catalogdir /= "MiscObjectCatalog.xosc";
    addNoEgoModelFromJson(catalogdir.string().c_str(), "MiscObject", json);
  } else {
    SYSTEM_LOGGER_ERROR("Not find catalogCategory : %s", strCatalogCategory.c_str());
    return -1;
  }
  return 0;
}

int CCataLog::addEgoModelFromJson(const char* defaultxosc, const Json::Value& json) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(defaultxosc);
  if (!bRet) {
    doc.Parse(kDefaultCatalogTpl);
  }
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) {
    SYSTEM_LOGGER_ERROR("Catalog file is null：%s", defaultxosc);
    return -1;
  }

  // If vehicleCategory is car
  std::string strcatalogSubCategory = json.get("catalogSubCategory", "").asString();
  if (std::string(json.get("catalogSubCategory", "").asString()) != std::string("combination")) {
    TiXmlElement* CatalogEle = xmlRoot->FirstChildElement("Catalog");
    TiXmlElement* pVehicleEle = NULL;
    while (CatalogEle) {
      std::string catalogName = CatalogEle->Attribute("name");
      if (catalogName == "CarCatalog") {
        TiXmlElement* tmpEle = CatalogEle->FirstChildElement("Vehicle");
        while (tmpEle) {
          std::string VehicleName = tmpEle->Attribute("name");
          if (VehicleName == std::string(json.get("variable", "").asString())) {
            pVehicleEle = tmpEle;
            break;
          }
          tmpEle = tmpEle->NextSiblingElement("Vehicle");
        }
        break;
      }
      CatalogEle = CatalogEle->NextSiblingElement("Catalog");
    }
    // update
    if (pVehicleEle) {
      pVehicleEle->Clear();
      toTixmlFromJson(json, pVehicleEle);
    } else {
      // create a new vehicle
      TiXmlElement* tmp = new TiXmlElement("Vehicle");
      toTixmlFromJson(json, tmp);
      tmp->SetAttribute("vehicleCategory", json.get("catalogSubCategory", "").asString());
      pVehicleEle = tmp;
      CatalogEle->InsertEndChild(*pVehicleEle);
    }
  } else {
    // If vehicleCategory is combination
    TiXmlElement* CatalogEle = xmlRoot->FirstChildElement("Catalog");
    while (CatalogEle) {
      std::string catalogName = CatalogEle->Attribute("name");
      if (catalogName == "TruckCatalog") {
        break;
      }
      CatalogEle = CatalogEle->NextSiblingElement("Catalog");
    }
    std::string combintionName = std::string(json.get("variable", "").asString());
    int nJsonParamsSize = json["catalogParams"].size();
    // find fisrt element Attribute name = strname
    auto findElement = [](TiXmlElement*& parent, std::string& strname) {
      TiXmlElement* tmpEle = NULL;
      if (parent != NULL) {
        tmpEle = parent->FirstChildElement("Vehicle");
        while (tmpEle) {
          std::string name = tmpEle->Attribute("name");
          if (strname == name) {
            return tmpEle;
          }
          tmpEle = tmpEle->NextSiblingElement("Vehicle");
        }
      }
      return tmpEle;
    };
    // serveral params to truck
    for (int i = 0; i < nJsonParamsSize; i++) {
      std::string truckName = "";
      std::string catalogSubCategory = "";
      if (i == 0) {
        truckName = combintionName;
        catalogSubCategory = "combination";
      } else if (i == 1) {
        truckName = combintionName + "_truck1";
        catalogSubCategory = "truck";
        auto vehicleEle = findElement(CatalogEle, combintionName);
        if (NULL != vehicleEle) {
          TiXmlElement* Tractor = new TiXmlElement("Tractor");
          Tractor->SetAttribute("name", truckName.c_str());
          vehicleEle->InsertEndChild(*Tractor);
        }
      } else {
        truckName = combintionName + "_trailer1";
        catalogSubCategory = "trailer";
        auto vehicleEle = findElement(CatalogEle, combintionName);
        if (NULL != vehicleEle) {
          TiXmlElement* Trailer = new TiXmlElement("Trailer");
          Trailer->SetAttribute("name", truckName.c_str());
          Trailer->SetAttribute("trailingUnit", i - 1);
          Trailer->SetDoubleAttribute("offsetX",
                                      json["catalogParams"][i]["boundingBox"].get("trailerOffsetX", 0).asDouble());
          Trailer->SetDoubleAttribute("offsetZ",
                                      json["catalogParams"][i]["boundingBox"].get("trailerOffsetZ", 0).asDouble());
          vehicleEle->InsertEndChild(*Trailer);
        }
      }
      auto pVehicleEle = findElement(CatalogEle, truckName);
      if (NULL != pVehicleEle) {  // update
        pVehicleEle->Clear();
        toTixmlFromJson(json, pVehicleEle, i);
        pVehicleEle->SetAttribute("name", truckName.c_str());
        pVehicleEle->SetAttribute("vehicleCategory", catalogSubCategory.c_str());
      } else {  // add
        TiXmlElement* tmp = new TiXmlElement("Vehicle");
        toTixmlFromJson(json, tmp, i);
        tmp->SetAttribute("name", truckName.c_str());
        tmp->SetAttribute("vehicleCategory", catalogSubCategory.c_str());
        CatalogEle->InsertEndChild(*tmp);
      }
    }
  }
  doc.SaveFile(defaultxosc);
  return 0;
}

int CCataLog::addNoEgoModelFromJson(const char* xoscpath, const std::string catalogtype, const Json::Value& json) {
  // parse xml and find catalog element
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(xoscpath);
  if (!bRet) {
    doc.Parse(kCatalogTpl);
  }
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return -1;
  TiXmlElement* pCatalogEle = xmlRoot->FirstChildElement("Catalog");
  if (!pCatalogEle) {
    pCatalogEle = new TiXmlElement("Catalog");
    pCatalogEle->SetAttribute("name", catalogtype + ("Catalog"));
    xmlRoot->LinkEndChild(pCatalogEle);
  }
  // update the cataloh if name = variable ; or add
  TiXmlElement* pEleTmp = pCatalogEle->FirstChildElement(catalogtype);
  TiXmlElement* pFindEle = NULL;
  // update the vehicle if vehicle name = variable ; or add the pedestrian
  while (pEleTmp) {
    if (pEleTmp->Attribute("name")) {
      if (std::string(json.get("variable", "").asString()) == std::string(pEleTmp->Attribute("name"))) {
        pFindEle = pEleTmp;
        break;
      }
    }
    pEleTmp = pEleTmp->NextSiblingElement(catalogtype);
  }
  // catalogs update vehicle
  if (pFindEle) {
    pFindEle->Clear();
    toTixmlFromJson(json, pFindEle);
  } else {
    // catalogs add vehicle
    TiXmlElement* tmp = new TiXmlElement(catalogtype);
    pFindEle = tmp;
    toTixmlFromJson(json, tmp);
    pCatalogEle->LinkEndChild(tmp);
  }
  if (catalogtype == CATALOG_TYPE_VEHICLE) {
    pFindEle->SetAttribute("vehicleCategory", json.get("catalogSubCategory", "").asString());
  } else if (catalogtype == CATALOG_TYPE_PEDESTRIAN) {
    pFindEle->SetAttribute("pedestrianCategory", json.get("catalogSubCategory", "").asString());
    pFindEle->SetAttribute("model", "NONE");
  } else if (catalogtype == CATALOG_TYPE_MISCOBJECT) {
    pFindEle->SetAttribute("miscObjectCategory", json.get("catalogSubCategory", "").asString());
    pFindEle->SetAttribute("mass", 1.0);
  }

  // save file
  doc.SaveFile(xoscpath);
  return 0;
}

int CCataLog::delModelFromJson(const char* CatalogDir, const Json::Value& json) {
  const std::string strCatalogCategory = json.get("catalogCategory", "").asString();
  boost::filesystem::path catalogdir = CatalogDir;
  const std::string strVariable = json.get("variable", "").asString();
  if (strCatalogCategory == "ego") {  // ego
    catalogdir /= "Vehicles/default.xosc";
    return delEgoModelFromJson(catalogdir.string().c_str(), strVariable.c_str());
  } else if (strCatalogCategory == "car") {  // vehicle
    catalogdir /= "Vehicles/VehicleCatalog.xosc";
    return delNoEgoModelFromJson(catalogdir.string().c_str(), CATALOG_TYPE_VEHICLE, strVariable.c_str());
  } else if (strCatalogCategory == "pedestrian") {  // pedestrian
    catalogdir /= "Pedestrians/PedestrianCatalog.xosc";
    return delNoEgoModelFromJson(catalogdir.string().c_str(), CATALOG_TYPE_PEDESTRIAN, strVariable.c_str());
  } else if (strCatalogCategory == "obstacle") {  // miscobject
    catalogdir /= "MiscObjects/MiscObjectCatalog.xosc";
    return delNoEgoModelFromJson(catalogdir.string().c_str(), CATALOG_TYPE_MISCOBJECT, strVariable.c_str());
  } else {
    SYSTEM_LOGGER_ERROR("Not find catalogCategory is %s", strCatalogCategory.c_str());
    return -1;
  }
  return 0;
}

int CCataLog::delEgoModelFromJson(const char* defaultxosc, const char* strName) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(defaultxosc);
  if (!bRet) {
    doc.Parse(kDefaultCatalogTpl);
  }
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return -1;
  // first find delete ele;
  const std::string egoName = strName;
  TiXmlElement* needDeleteEle = NULL;
  std::map<std::string, int> model3dToUserCountMap;
  TiXmlElement* CatalogEle = xmlRoot->FirstChildElement("Catalog");
  while (CatalogEle) {
    std::string catalogName = CatalogEle->Attribute("name");
    if (catalogName == std::string("CarCatalog")) {
      TiXmlElement* tmpEle = CatalogEle->FirstChildElement("Vehicle");
      while (tmpEle) {
        std::string VehicleName = tmpEle->Attribute("name");
        TiXmlElement* pPropertys = tmpEle->FirstChildElement("Properties");

        std::string model3d = CProperties::findValueFromName(pPropertys, "model3d");
        // use to check to delete models
        if (model3dToUserCountMap.find(model3d) != model3dToUserCountMap.end()) {
          model3dToUserCountMap.find(model3d)->second++;
        } else {
          model3dToUserCountMap[model3d] = 1;
        }
        SYSTEM_LOGGER_WARN("VehicleName =  %s, strName = %s", VehicleName.c_str(), egoName.c_str());
        if (VehicleName == std::string(egoName)) {
          if (CProperties::findValueFromName(pPropertys, "Preset") == "true") {
            return -3;
          }
          needDeleteEle = tmpEle;
        }
        tmpEle = tmpEle->NextSiblingElement("Vehicle");
      }
      break;
    }
    CatalogEle = CatalogEle->NextSiblingElement("Catalog");
  }
  if (needDeleteEle) {
    TiXmlElement* pPropertys = needDeleteEle->FirstChildElement("Properties");
    if (model3dToUserCountMap[CProperties::findValueFromName(pPropertys, "model3d")] == 1)
      delModelDirFromModel3d(defaultxosc, pPropertys);
    CatalogEle->RemoveChild(needDeleteEle);
    doc.SaveFile();
    return 0;
  }
  SYSTEM_LOGGER_WARN("Default xosc not find variable is %s", egoName);
  return -2;
}

int CCataLog::delNoEgoModelFromJson(const char* xoscpath, std::string catalogtype, const char* strName) {
  // parse xml and find catalog element
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(xoscpath);
  if (!bRet) {
    doc.Parse(kCatalogTpl);
  }
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return -1;
  TiXmlElement* pCatalogEle = xmlRoot->FirstChildElement("Catalog");
  if (!pCatalogEle) {
    pCatalogEle = new TiXmlElement("Catalog");
    pCatalogEle->SetAttribute("name", catalogtype.append("Catalog"));
  }
  // find name = strName
  TiXmlElement* pEleTmp = pCatalogEle->FirstChildElement(catalogtype);

  // update the vehicle if vehicle name = variable ; or add the pedestrian
  while (pEleTmp) {
    if (pEleTmp->Attribute("name")) {
      if (std::string(strName) == std::string(pEleTmp->Attribute("name"))) {
        TiXmlElement* pPropertys = pEleTmp->FirstChildElement("Properties");
        TiXmlElement* pProperty = pPropertys->FirstChildElement("Property");
        while (pProperty) {
          const std::string strName = pProperty->Attribute("name");
          const std::string strValue = pProperty->Attribute("value");
          if (strName == "Preset" && strValue == "true") {
            return -3;
          }
          pProperty = pProperty->NextSiblingElement("Property");
        }
        delModelDirFromModel3d(xoscpath, pPropertys);
        pCatalogEle->RemoveChild(pEleTmp);
        doc.SaveFile();
        return 0;
      }
    }
    pEleTmp = pEleTmp->NextSiblingElement(catalogtype);
  }
  SYSTEM_LOGGER_WARN("%s, not find variable is %s", xoscpath, strName);
  return -1;
}

int CCataLog::delModelDirFromModel3d(const char* defaultxosc, TiXmlElement* timxml) {
  std::map<std::string, std::string> propertyName2Value;
  TiXmlElement* pProperty = timxml->FirstChildElement("Property");
  while (pProperty) {
    const std::string strName = pProperty->Attribute("name");
    const std::string strValue = pProperty->Attribute("value");
    propertyName2Value[strName] = strValue;
    pProperty = pProperty->NextSiblingElement("Property");
  }
  if (propertyName2Value.find("model3d") != propertyName2Value.end()) {
    boost::filesystem::path pathModel3d = defaultxosc;
    pathModel3d = pathModel3d.parent_path().parent_path().parent_path();
    pathModel3d /= "models";
    pathModel3d /= propertyName2Value.find("model3d")->second;
    std::string strP = pathModel3d.parent_path().string();
    boost::filesystem::remove_all(pathModel3d.parent_path());
  }
  return 0;
}

int CCataLog::getCatalogName(const char* DataCatalogDir, const std::string catalogtype, const std::string strName,
                             CCatalogModelPtr& model) {
  boost::filesystem::path path_data_catalog = DataCatalogDir;
  boost::filesystem::path path_sys_catalog = DataCatalogDir;
  path_sys_catalog /= DATA_CATALOG_TO_SYS;
  // parse sys catalog
  if (getCatalogFromDir(DataCatalogDir, catalogtype, strName, model) == 0) {
    return 0;
  }

  return getCatalogFromDir(path_sys_catalog.string().c_str(), catalogtype, strName, model);
}

int CCataLog::getCatalogFromDir(const char* dir, const std::string catalogtype, const std::string strName,
                                CCatalogModelPtr& model) {
  boost::filesystem::path dir_path = dir;
  // parse data catalog
  std::string path = "";
  const std::string strCatalogCategory = catalogtype;
  if (strCatalogCategory == CATALOG_TYPE_VEHICLE) {
    // vehicle
    dir_path /= "Vehicles/VehicleCatalog.xosc";
    path = dir_path.string();
    model.reset(new CSingleVechile);
  } else if (strCatalogCategory == CATALOG_TYPE_PEDESTRIAN) {
    // pedestrian
    dir_path /= "Pedestrians/PedestrianCatalog.xosc";
    path = dir_path.string();
    model.reset(new CSinglePedestrian);
  } else if (strCatalogCategory == CATALOG_TYPE_MISCOBJECT) {  // miscobject
    dir_path /= "MiscObjects/MiscObjectCatalog.xosc";
    path = dir_path.string();
    model.reset(new CSingleMisObject);
  } else {
    SYSTEM_LOGGER_ERROR("Not find catalogCategory is %s", strCatalogCategory.c_str());
    return -1;
  }
  if (!path.empty()) {
    TiXmlDocument doc;
    bool bRet = doc.LoadFile(path);
    if (!bRet) return -1;
    TiXmlElement* xmlRoot = doc.RootElement();
    if (!xmlRoot) return false;
    TiXmlElement* elemCatalog = xmlRoot->FirstChildElement("Catalog");
    if (!elemCatalog) return false;
    TiXmlElement* elemPedestrian = elemCatalog->FirstChildElement(catalogtype);
    while (elemPedestrian) {
      if (std::string(elemPedestrian->Attribute("name")) == strName) {
        if (model != NULL) {
          model->parseXML(elemPedestrian);
        }
        return 0;
      }
      elemPedestrian = elemPedestrian->NextSiblingElement(catalogtype);
    }
  }
  return -1;
}

std::string CCataLog::fromJsonName2XmlName(std::string str) {
  if (str == "dynamic")
    return "Dynamic";
  else if (str == "sensorGroup")
    return "SensorGroup";
  else if (str == "weight")
    return "Weight";
  else if (str == "preset")
    return "Preset";
  else if (str == "modelId")
    return "model_id";
  else if (str == "alias")
    return "Alias";
  else
    return str;
}
