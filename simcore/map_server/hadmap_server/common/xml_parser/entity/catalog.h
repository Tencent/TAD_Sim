/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once
#include <string>
#include <vector>

#include <json/value.h>
#include <tinyxml.h>
#include "common/xml_parser/entity/SensorV2.h"
#include "common/xml_parser/entity/catalogbase.h"

class CCataLog {
 public:
  CCataLog() {}
  // parse xml to json
  int allModelToJson(const char* DataCatalogDir, Json::Value& json);
  int egoModelToJson(Json::Value& jsonValue);
  int noEgoModelToJson(const char* xoscpath, const std::string catalogtype, Json::Value& json);

  // defaut xosc
  int parseDefaultXosc(const char* DataCatalogDir, const char* SysCataLogDir);
  int parseDefaultXosc(std::string strDefaultXoscPath);

  // get driveCatalog from egoname
  static bool getCatalog(CDriveCatalog& log, const std::string filename);

  bool deleteCatalog(std::string catalogname, std::string filename);

  std::string findCatalogName(std::string aliasName, VehicleType type);

  int getEgoModelFromName(const char* dataCatalogDir, std::string strName, CDriveCatalog& driveCatalog);
  //
  bool findEgoModelFromName(const char* dataCatalogDir, std::string strName, VehicleType type = VEHICLE_ALL);
  // get drivecatalog lista
  std::vector<CDriveCatalog> getDriveCatalogList();
  // add scene model(contains ego and noego)
  static int addModelFromJson(const char* CatalogDir, const Json::Value& json);
  //
  static int addEgoModelFromJson(const char* defaultxosc, const Json::Value& json);
  //
  static int addNoEgoModelFromJson(const char* xoscpath, const std::string catalogtype, const Json::Value& json);
  // save no ego (from json to xmlele)
  static int toTixmlFromJson(const Json::Value& root, TiXmlElement*& timxml, int index = 0);
  // delete scene model (contains ego and noego)
  static int delModelFromJson(const char* CatalogDir, const Json::Value& json);
  // delete ego model
  static int delEgoModelFromJson(const char* defaultxosc, const char* strName);
  // delete no ego model
  static int delNoEgoModelFromJson(const char* xoscpath, std::string catalogtype, const char* strName);
  // delete model dir from model3d
  static int delModelDirFromModel3d(const char* defaultxosc, TiXmlElement* timxml);

  // find name from data dir and sys dir
  static int getCatalogName(const char* CatalogDataDir, const std::string catalogtype, const std::string strName,
                            CCatalogModelPtr& model);
  // find name from dir
  static int getCatalogFromDir(const char* dir, const std::string catalogtype, const std::string strName,
                               CCatalogModelPtr& model);

 private:
  std::vector<CDriveCatalog> m_vecList;
  static std::string fromJsonName2XmlName(std::string);
  static constexpr const char* kDefaultCatalogTpl = R"(
<?xml version="1.0" encoding="UTF-8" ?>
<OpenSCENARIO xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
xsi:noNamespaceSchemaLocation="../../../Schema/OpenSCENARIO.xsd">
    <FileHeader revMajor="1" revMinor="0" date="2020-02-21T10:00:00" 
    description="Example - Vehicle Catalog" author="ASAM e.V." />
    <Catalog name="CarCatalog">
    </Catalog>
    <Catalog name="TruckCatalog">
    </Catalog>
</OpenSCENARIO>
    )";
  static constexpr const char* kCatalogTpl = R"(
<?xml version="1.0" encoding="UTF-8" ?>
<OpenSCENARIO>
    <FileHeader revMajor="1" revMinor="0" date="2018-11-8T17:00:00" author="TAD Sim" />
</OpenSCENARIO>
    )";
};
