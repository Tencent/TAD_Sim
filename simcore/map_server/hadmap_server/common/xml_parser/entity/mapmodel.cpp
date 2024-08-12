/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "mapmodel.h"

#include <tinyxml.h>
#include <boost/filesystem.hpp>
#include "common/log/system_logger.h"
int CMapModel::getModelListJson(const char* modelPath, const char* modelDir, Json::Value& json) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(modelPath);
  if (!bRet) {
    doc.Parse(kDefaultModelTpl);
  }
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return -1;
  TiXmlElement* pModel = xmlRoot->FirstChildElement("MapModels")->FirstChildElement("MapModel");
  while (pModel) {
    Json::Value jModel;
    jModel["variable"] = pModel->Attribute("name");
    // BoundingBox
    TiXmlElement* pBoundingBox = pModel->FirstChildElement("BoundingBox");
    if (!pBoundingBox) return -2;
    Json::Value jBoundingBox;
    // BoundingBox - Center
    TiXmlElement* pCenter = pBoundingBox->FirstChildElement("Center");
    if (!pCenter) return -2;
    Json::Value jCenter;
    jCenter["x"] = std::atof(pCenter->Attribute("x"));
    jCenter["y"] = std::atof(pCenter->Attribute("y"));
    jCenter["z"] = std::atof(pCenter->Attribute("z"));
    jBoundingBox["center"] = jCenter;

    // BoundingBox - Dimensions
    TiXmlElement* pDimensions = pBoundingBox->FirstChildElement("Dimensions");
    if (!pDimensions) return -3;
    Json::Value jDimensions;
    jDimensions["height"] = std::atof(pDimensions->Attribute("height"));
    jDimensions["length"] = std::atof(pDimensions->Attribute("length"));
    jDimensions["width"] = std::atof(pDimensions->Attribute("width"));
    jBoundingBox["dimensions"] = jDimensions;
    jModel["boundingBox"] = jBoundingBox;

    // Properties - Property
    TiXmlElement* pProperty = pModel->FirstChildElement("Properties")->FirstChildElement("Property");
    std::map<std::string, std::string> propertyName2Value;
    while (pProperty) {
      const std::string strName = pProperty->Attribute("name");
      const std::string strValue = pProperty->Attribute("value");
      propertyName2Value[strName] = strValue;
      pProperty = pProperty->NextSiblingElement("Property");
    }
    if (propertyName2Value.find("zhname") != propertyName2Value.end()) {
      jModel["name"] = propertyName2Value.find("zhname")->second;
    }
    if (propertyName2Value.find("rawFileName") != propertyName2Value.end()) {
      jModel["rawFileName"] = propertyName2Value.find("rawFileName")->second;
    }
    if (propertyName2Value.find("model3d") != propertyName2Value.end()) {
      boost::filesystem::path pathModel3d = modelDir;
      pathModel3d = pathModel3d.parent_path();
      pathModel3d /= propertyName2Value.find("model3d")->second;
      jModel["model3d"] = boost::filesystem::system_complete(pathModel3d).string();
    }
    json.append(jModel);
    pModel = pModel->NextSiblingElement();
  }
  return 0;
}

int CMapModel::addModelFromJson(const char* modelPath, const Json::Value& json, int flagupdate) {
  boost::filesystem::path path_mapModel_dir = modelPath;
  if (!boost::filesystem::exists(path_mapModel_dir.parent_path())) {
    boost::filesystem::create_directories(path_mapModel_dir.parent_path());
  }
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(modelPath);
  if (!bRet) {
    doc.Parse(kDefaultModelTpl);
  }
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return -1;
  TiXmlElement* pModels = xmlRoot->FirstChildElement("MapModels");
  if (!pModels) return -2;
  TiXmlElement* pModel = NULL;
  bool flag_isExist = false;
  // check id add or update
  TiXmlElement* pTmp = xmlRoot->FirstChildElement("MapModels")->FirstChildElement("MapModel");
  while (pTmp) {
    std::string strName = pTmp->Attribute("name");
    if (strName == std::string(json.get("variable", "").asString().c_str())) {
      pModel = pTmp;
      flag_isExist = true;
      pModel->Clear();
      break;
    }
    pTmp = pTmp->NextSiblingElement();
  }
  if (!pModel) {
    pModel = new TiXmlElement("MapModel");
  }
  // create model node
  pModel->SetAttribute("name", json.get("variable", "").asString().c_str());
  // create boundingbox node
  TiXmlElement* pBoundingBox = new TiXmlElement("BoundingBox");
  TiXmlElement* pCenter = new TiXmlElement("Center");
  pCenter->SetDoubleAttribute("x", json["boundingBox"]["center"].get("x", 0).asDouble());
  pCenter->SetDoubleAttribute("y", json["boundingBox"]["center"].get("y", 0).asDouble());
  pCenter->SetDoubleAttribute("z", json["boundingBox"]["center"].get("z", 0).asDouble());
  pBoundingBox->InsertEndChild(*pCenter);
  TiXmlElement* pDimensions = new TiXmlElement("Dimensions");
  pDimensions->SetDoubleAttribute("height", json["boundingBox"]["dimensions"].get("height", 0).asDouble());
  pDimensions->SetDoubleAttribute("length", json["boundingBox"]["dimensions"].get("length", 0).asDouble());
  pDimensions->SetDoubleAttribute("width", json["boundingBox"]["dimensions"].get("width", 0).asDouble());
  pBoundingBox->InsertEndChild(*pDimensions);
  pModel->InsertEndChild(*pBoundingBox);
  // create Properties
  TiXmlElement* pProperties = new TiXmlElement("Properties");
  TiXmlElement* pPropertyModel3d = new TiXmlElement("Property");
  pPropertyModel3d->SetAttribute("name", "model3d");
  std::string strModel3d = json.get("model3d", "").asString().c_str();
  pPropertyModel3d->SetAttribute("value", strModel3d);
  pProperties->InsertEndChild(*pPropertyModel3d);
  TiXmlElement* pPropertyName = new TiXmlElement("Property");
  pPropertyName->SetAttribute("name", "zhname");
  pPropertyName->SetAttribute("value", json.get("name", "").asString().c_str());
  pProperties->InsertEndChild(*pPropertyName);
  TiXmlElement* pPropertyRawFileName = new TiXmlElement("Property");
  pPropertyRawFileName->SetAttribute("name", "rawFileName");
  pPropertyRawFileName->SetAttribute("value", json.get("rawFileName", "").asString().c_str());
  pProperties->InsertEndChild(*pPropertyRawFileName);

  pModel->InsertEndChild(*pProperties);
  if (!flag_isExist) {
    pModels->InsertEndChild(*pModel);
  }
  // save model file
  doc.SaveFile(modelPath);
  return 0;
}

int CMapModel::delModelFromName(const char* modelPath, const std::string variablename) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(modelPath);
  if (!bRet) return -1;
  TiXmlElement* xmlRoot = doc.RootElement();
  if (!xmlRoot) return -2;
  TiXmlElement* pModel = xmlRoot->FirstChildElement("MapModels")->FirstChildElement("MapModel");
  while (pModel) {
    std::string strName = pModel->Attribute("name");
    if (strName == variablename) {
      TiXmlElement* pProperty = pModel->FirstChildElement("Properties")->FirstChildElement("Property");
      std::map<std::string, std::string> propertyName2Value;
      while (pProperty) {
        const std::string strName = pProperty->Attribute("name");
        const std::string strValue = pProperty->Attribute("value");
        propertyName2Value[strName] = strValue;
        pProperty = pProperty->NextSiblingElement("Property");
      }
      if (propertyName2Value.find("model3d") != propertyName2Value.end()) {
        boost::filesystem::path pathModel3d = modelPath;
        pathModel3d = pathModel3d.parent_path();
        pathModel3d /= propertyName2Value.find("model3d")->second;
        boost::filesystem::remove_all(pathModel3d.parent_path());
      }
      xmlRoot->FirstChildElement("MapModels")->RemoveChild(pModel);
      doc.SaveFile(modelPath);
      return 0;
    }
    pModel = pModel->NextSiblingElement();
  }
  return -3;
}

int CMapModel::getModelPathFromName(const char* modelPath, const std::string variablename) { return 0; }
