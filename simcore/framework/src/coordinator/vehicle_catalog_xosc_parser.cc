// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "vehicle_catalog_xosc_parser.h"
#include "boost/filesystem.hpp"
#include "glog/logging.h"
#include "xml_util.h"

using namespace tinyxml2;
using namespace tx_sim::utils;
namespace fs = boost::filesystem;


namespace tx_sim {
namespace coordinator {

//!
//! @brief 函数名：Build
//! @details 函数功能：构建车辆目录XOSC解析器
//!
bool CVehicleCatalogXOSCParser::Build() {
  if (fs::is_regular_file(m_vehicleCatalogXOSCSysPath)) {
    m_vehicleCatalogXOSCSysXMLRoot = GetXMLRoot(m_vehicleCatalogXOSCSysPath, m_vehicleCatalogXOSCSysDoc);
  }

  if (fs::is_regular_file(m_miscCatalogXOSCSysPath)) {
    m_miscObjectCatalogXOSCSysXMLRoot = GetXMLRoot(m_miscCatalogXOSCSysPath, m_miscObjectCatalogXOSCSysDoc);
  }

  if (fs::is_regular_file(m_pedestrianCatalogXOSCSysPath)) {
    m_pedestrianCatalogXOSCSysXMLRoot = GetXMLRoot(m_pedestrianCatalogXOSCSysPath, m_pedestrianCatalogXOSCSysDoc);
  }

  if (fs::is_regular_file(m_vehicleCatalogXOSCDataPath)) {
    m_vehicleCatalogXOSCDataXMLRoot = GetXMLRoot(m_vehicleCatalogXOSCDataPath, m_vehicleCatalogXOSCDataDoc);
  }

  if (fs::is_regular_file(m_miscCatalogXOSCDataPath)) {
    m_miscObjectCatalogXOSCDataXMLRoot = GetXMLRoot(m_miscCatalogXOSCDataPath, m_miscObjectCatalogXOSCDataDoc);
  }

  if (fs::is_regular_file(m_pedestrianCatalogXOSCDataPath)) {
    m_pedestrianCatalogXOSCDataXMLRoot = GetXMLRoot(m_pedestrianCatalogXOSCDataPath, m_pedestrianCatalogXOSCDataDoc);
  }

  if (fs::is_regular_file(m_mapCatalogXMLDataPath)) {
    m_mapCatalogDataXMLDataRoot = GetXMLRoot(m_mapCatalogXMLDataPath, m_mapCatalogXMLDataDoc);
  }

  BuildVehicleNameMap();
  BuildMiscObjectNameMap();
  BuildPedestrianNameMap();
  BuildModel3dMap();
  return true;
}

//!
//! @brief 函数名：BuildPhysicleCommon
//! @details 函数功能：构建物理通用元素
//!
//! @param[in] pPhysicleCommon 物理通用元素对象指针
//! @param[in] trafficType 交通类型
//! @param[in] nodeType 节点类型
//!
void CVehicleCatalogXOSCParser::BuildPhysicleCommon(sim_msg::PhysicleCommon* pPhysicleCommon, std::string trafficType,
                                                    std::string nodeType) {
  const tinyxml2::XMLElement* pVehicleNode = nullptr;
  if ("vehicle" == nodeType) pVehicleNode = m_mapNameVehicle[trafficType];
  else if ("obstacles" == nodeType) {
    pVehicleNode = m_mapNameMisc[trafficType];
  } else if ("pedestrians" == nodeType) {
    pVehicleNode = m_mapNamePedestrian[trafficType];
    if (!pVehicleNode) pVehicleNode = m_mapNameVehicle[trafficType];
  } else if ("mapmodels" == nodeType) {
    pVehicleNode = m_mapNameModel3d[trafficType];
  }

  if (!pVehicleNode) {
    LOG(INFO) << "can not find traffic " << trafficType;
    return;
  }
  sim_msg::BoundingBox* pBoundingBox = pPhysicleCommon->mutable_bounding_box();
  const XMLElement* BoundingBox = GetChildNode(pVehicleNode, "BoundingBox");
  const XMLElement* Center = GetChildNode(BoundingBox, "Center");
  std::string x = Center->Attribute("x");
  std::string y = Center->Attribute("y");
  std::string z = Center->Attribute("z");
  sim_msg::Vec3* pVec3 = pBoundingBox->mutable_center();
  pVec3->set_x(std::stod(x));
  pVec3->set_y(std::stod(y));
  pVec3->set_z(std::stod(z));

  const XMLElement* Dimensions = GetChildNode(BoundingBox, "Dimensions");
  std::string width = Dimensions->Attribute("width");
  std::string length = Dimensions->Attribute("length");
  std::string height = Dimensions->Attribute("height");
  pBoundingBox->set_higth(stod(height));
  pBoundingBox->set_length(stod(length));
  pBoundingBox->set_width(stod(width));

  const XMLElement* Properties = GetChildNode(pVehicleNode, "Properties");
  const XMLElement* Property = Properties->FirstChildElement();
  while (Property) {
    std::string sName = Property->Attribute("name");
    if (sName == "model_id") {
      std::string sValue = Property->Attribute("value");
      pPhysicleCommon->set_model_id(std::stoi(sValue));
    } else if (sName == "model3d") {
      std::string sValue = Property->Attribute("value");
      pPhysicleCommon->set_model_3d(sValue);
    }
    Property = Property->NextSiblingElement();
  }
}

//!
//! @brief 函数名：BuildPhysicleVehicleGeometory
//! @details 函数功能：构建物理车辆几何体
//!
//! @param[in] pPhysicleCommon 物理车辆几何体对象指针
//! @param[in] vehicleName 车辆名称
//!
void CVehicleCatalogXOSCParser::BuildPhysicleVehicleGeometory(sim_msg::VehicleGeometory* pPhysicleGeometory,
                                                              std::string vehicleName) {
  const tinyxml2::XMLElement* pVehicleNode = m_mapNameVehicle[vehicleName];
  if (!pVehicleNode) {
    LOG(ERROR) << "can not find traffic " << vehicleName;
    return;
  }
  VehicleXMLNode2GeometryMsg(GetNodeWithAttributeInList(pVehicleNode, "name", vehicleName.c_str(), "Catalog.Vehicle"),
                             pPhysicleGeometory);
}

//!
//! @brief 函数名：BuildVehicleNameMap
//! @details 函数功能：构建车辆名称映射
//!
void CVehicleCatalogXOSCParser::BuildVehicleNameMap() {
  if (m_vehicleCatalogXOSCSysXMLRoot) {
    const XMLElement* catalog_node = m_vehicleCatalogXOSCSysXMLRoot->FirstChildElement();
    const XMLElement* Vehicle_node = nullptr;
    while (catalog_node) {
      std::string sName = catalog_node->Name();
      if (sName == "Catalog") {
        Vehicle_node = catalog_node->FirstChildElement();
        while (Vehicle_node) {
          const char* pVehicleName = Vehicle_node->Attribute("name");
          m_mapNameVehicle[pVehicleName] = Vehicle_node;
          Vehicle_node = Vehicle_node->NextSiblingElement();
        }
      }
      catalog_node = catalog_node->NextSiblingElement();
    }
  }


  if (m_vehicleCatalogXOSCDataXMLRoot) {
    const XMLElement* catalog_node = m_vehicleCatalogXOSCDataXMLRoot->FirstChildElement();
    const XMLElement* Vehicle_node = nullptr;
    while (catalog_node) {
      std::string sName = catalog_node->Name();
      if (sName == "Catalog") {
        Vehicle_node = catalog_node->FirstChildElement();
        while (Vehicle_node) {
          const char* pVehicleName = Vehicle_node->Attribute("name");
          m_mapNameVehicle[pVehicleName] = Vehicle_node;
          Vehicle_node = Vehicle_node->NextSiblingElement();
        }
      }
      catalog_node = catalog_node->NextSiblingElement();
    }
  }
}

//!
//! @brief 函数名：BuildAllVehicles
//! @details 函数功能：构建所有车辆
//!
//! @param[in] scene 场景对象
//!
void CVehicleCatalogXOSCParser::BuildAllVehicles(sim_msg::Scene& scene) {
  for (const auto& item : m_mapNameVehicle) {
    std::string sName = item.first;
    LOG(INFO) << "build vehicle name:" << sName;
    sim_msg::Vehicle* pVerhicle = scene.add_vehicles();
    pVerhicle->set_objectname(sName);
    sim_msg::PhysicleVehicle* pPhysicleVehicle = pVerhicle->mutable_physicle();
    sim_msg::PhysicleCommon* pPhysicleCommon = pPhysicleVehicle->mutable_common();
    BuildPhysicleCommon(pPhysicleCommon, sName, "vehicle");
    sim_msg::VehicleGeometory* pPhysicleGeometory = pPhysicleVehicle->mutable_geometory();
    BuildPhysicleVehicleGeometory(pPhysicleGeometory, sName);
  }
}

//!
//! @brief 函数名：BuildAllMiscObjects
//! @details 函数功能：构建所有其他元素
//!
//! @param[in] scene 场景对象
//!
void CVehicleCatalogXOSCParser::BuildAllMiscObjects(sim_msg::Scene& scene) {
  for (const auto& item : m_mapNameMisc) {
    std::string sName = item.first;
    LOG(INFO) << "build misc name:" << sName;

    sim_msg::MiscellaneousObject* pMisc = scene.add_miscs();
    pMisc->set_objectname(sName);
    sim_msg::PhysicleCommon* pPhysicleMisc = pMisc->mutable_physicle();
    BuildPhysicleCommon(pPhysicleMisc, sName, "obstacles");
  }
}

//!
//! @brief 函数名：BuildAllPedestrians
//! @details 函数功能：构建所有行人
//!
//! @param[in] scene 场景对象
//!
void CVehicleCatalogXOSCParser::BuildAllPedestrians(sim_msg::Scene& scene) {
  for (const auto& item : m_mapNamePedestrian) {
    std::string sName = item.first;
    LOG(INFO) << "build Pedestrians name:" << sName;

    sim_msg::VulnerableRoadUser* pVru = scene.add_vrus();
    pVru->set_objectname(sName);
    sim_msg::PhysicleVru* pPhysicleVru = pVru->mutable_physicle();
    sim_msg::PhysicleCommon* pPhysicleCommon = pPhysicleVru->mutable_common();
    BuildPhysicleCommon(pPhysicleCommon, sName, "pedestrians");
  }
}

void CVehicleCatalogXOSCParser::BuildAllMap3dModels(sim_msg::Scene& scene) {
  for (const auto& item : m_mapNameModel3d) {
    std::string sName = item.first;
    LOG(INFO) << "build model3d name:" << sName;
    sim_msg::MapModel* pMapModel = scene.add_map_models();
    pMapModel->set_name(sName);
    BuildPhysicleCommon(pMapModel->mutable_common(), sName, "mapmodels");
  }
}


void CVehicleCatalogXOSCParser::BuildCategoryVehicle(sim_msg::Vehicle* pVehicle, const std::string& name) {
  const tinyxml2::XMLElement* pVehicleNode = m_mapNameVehicle[name];
  if (!pVehicleNode) {
    LOG(ERROR) << "can not find Vehicle " << name;
    return;
  }
  std::string sCategory = pVehicleNode->Attribute("vehicleCategory");
  if ("car" == sCategory) {
    pVehicle->set_category(sim_msg::CATEGORY_VEHICLE_CAR);
  } else if ("bus" == sCategory) {
    pVehicle->set_category(sim_msg::CATEGORY_VEHICLE_BUS);
  } else if ("truck" == sCategory) {
    pVehicle->set_category(sim_msg::CATEGORY_VEHICLE_TRUCK);
  } else if ("traller" == sCategory) {
    pVehicle->set_category(sim_msg::CATEGORY_VEHICLE_TRALLER);
  } else if ("bicycle" == sCategory) {
    pVehicle->set_category(sim_msg::CATEGORY_VEHICLE_BIKE);
  } else if ("electric_bike" == sCategory) {
    pVehicle->set_category(sim_msg::CATEGORY_VEHICLE_ELECTRIC_BIKE);
  } else if ("motorbike" == sCategory) {
    pVehicle->set_category(sim_msg::CATEGORY_VEHICLE_MOTORBIKE);
  } else if ("electric_motorbike" == sCategory) {
    pVehicle->set_category(sim_msg::CATEGORY_VEHICLE_ELECTRIC_MOTORBIKE);
  } else if ("semitrailer" == sCategory) {
    pVehicle->set_category(sim_msg::CATEGORY_VEHICLE_ELECTRIC_SEMITRAILER);
  } else if ("crane" == sCategory) {
    pVehicle->set_category(sim_msg::CATEGORY_VEHICLE_ELECTRIC_CRANE);
  }
}

void CVehicleCatalogXOSCParser::BuildCategoryVru(sim_msg::VulnerableRoadUser* pVru, const std::string& name) {
  const tinyxml2::XMLElement* pPedestrianNode = m_mapNamePedestrian[name];
  if (!pPedestrianNode) {
    LOG(ERROR) << "can not find Pedestrian " << name;
    return;
  }
  std::string sCategory = pPedestrianNode->Attribute("pedestrianCategory");
  if ("pedestrian" == sCategory) {
    pVru->set_category(sim_msg::CATEGORY_VRU_PEDESTRIAN);
  } else if ("animal" == sCategory) {
    pVru->set_category(sim_msg::CATEGORY_VRU_ANIMAL);
  } else if ("wheelchair" == sCategory) {
    pVru->set_category(sim_msg::CATEGORY_VRU_WHEELCHAIR);
  }
}

void CVehicleCatalogXOSCParser::BuildCategoryMisc(sim_msg::MiscellaneousObject* pMisc, const std::string& name) {
  const tinyxml2::XMLElement* pMiscNode = m_mapNameMisc[name];
  if (!pMiscNode) {
    LOG(ERROR) << "can not find Misc " << name;
    return;
  }
  std::string sCategory = pMiscNode->Attribute("miscObjectCategory");
  if ("barrier" == sCategory) {
    pMisc->set_category(sim_msg::CATEGORY_MISC_BARRIER);
  } else if ("building") {
    pMisc->set_category(sim_msg::CATEGORY_MISC_BUILDING);
  } else if ("obstacle" == sCategory) {
    pMisc->set_category(sim_msg::CATEGORY_MISC_OBSTACLE);
  }
}

void CVehicleCatalogXOSCParser::BuildMiscObjectNameMap() {
  if (m_miscObjectCatalogXOSCSysXMLRoot) {
    const XMLElement* catalog_node = m_miscObjectCatalogXOSCSysXMLRoot->FirstChildElement();
    const XMLElement* Vehicle_node = nullptr;
    while (catalog_node) {
      std::string sName = catalog_node->Name();
      if (sName == "Catalog") {
        Vehicle_node = catalog_node->FirstChildElement();
        while (Vehicle_node) {
          const char* pVehicleName = Vehicle_node->Attribute("name");
          m_mapNameMisc[pVehicleName] = Vehicle_node;
          Vehicle_node = Vehicle_node->NextSiblingElement();
        }
      }
      catalog_node = catalog_node->NextSiblingElement();
    }
  }

  if (m_miscObjectCatalogXOSCDataXMLRoot) {
    const XMLElement* catalog_node = m_miscObjectCatalogXOSCDataXMLRoot->FirstChildElement();
    const XMLElement* Vehicle_node = nullptr;
    while (catalog_node) {
      std::string sName = catalog_node->Name();
      if (sName == "Catalog") {
        Vehicle_node = catalog_node->FirstChildElement();
        while (Vehicle_node) {
          const char* pVehicleName = Vehicle_node->Attribute("name");
          m_mapNameMisc[pVehicleName] = Vehicle_node;
          Vehicle_node = Vehicle_node->NextSiblingElement();
        }
      }
      catalog_node = catalog_node->NextSiblingElement();
    }
  }
}

void CVehicleCatalogXOSCParser::BuildPedestrianNameMap() {
  if (m_pedestrianCatalogXOSCSysXMLRoot) {
    const XMLElement* catalog_node = m_pedestrianCatalogXOSCSysXMLRoot->FirstChildElement();
    const XMLElement* Vehicle_node = nullptr;
    while (catalog_node) {
      std::string sName = catalog_node->Name();
      if (sName == "Catalog") {
        Vehicle_node = catalog_node->FirstChildElement();
        while (Vehicle_node) {
          const char* pVehicleName = Vehicle_node->Attribute("name");
          m_mapNamePedestrian[pVehicleName] = Vehicle_node;
          Vehicle_node = Vehicle_node->NextSiblingElement();
        }
      }
      catalog_node = catalog_node->NextSiblingElement();
    }
  }

  if (m_pedestrianCatalogXOSCDataXMLRoot) {
    const XMLElement* catalog_node = m_pedestrianCatalogXOSCDataXMLRoot->FirstChildElement();
    const XMLElement* Vehicle_node = nullptr;
    while (catalog_node) {
      std::string sName = catalog_node->Name();
      if (sName == "Catalog") {
        Vehicle_node = catalog_node->FirstChildElement();
        while (Vehicle_node) {
          const char* pVehicleName = Vehicle_node->Attribute("name");
          m_mapNamePedestrian[pVehicleName] = Vehicle_node;
          Vehicle_node = Vehicle_node->NextSiblingElement();
        }
      }
      catalog_node = catalog_node->NextSiblingElement();
    }
  }
}

void CVehicleCatalogXOSCParser::BuildModel3dMap() {
  if (!m_mapCatalogDataXMLDataRoot) return;
  const XMLElement* mapmodels_node = m_mapCatalogDataXMLDataRoot->FirstChildElement();
  const XMLElement* mapModel_node = nullptr;
  while (mapmodels_node) {
    std::string sName = mapmodels_node->Name();
    if (sName == "MapModels") {
      mapModel_node = mapmodels_node->FirstChildElement();
      while (mapModel_node) {
        const char* modelName = mapModel_node->Attribute("name");
        m_mapNameModel3d[modelName] = mapModel_node;
        mapModel_node = mapModel_node->NextSiblingElement();
      }
    }
    mapmodels_node = mapmodels_node->NextSiblingElement();
  }
}
}  // namespace coordinator
}  // namespace tx_sim
