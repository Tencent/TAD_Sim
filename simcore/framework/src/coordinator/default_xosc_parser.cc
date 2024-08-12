// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "default_xosc_parser.h"
#include "boost/filesystem.hpp"
#include "glog/logging.h"
#include "xml_util.h"

using namespace tinyxml2;
using namespace tx_sim::utils;
namespace fs = boost::filesystem;

namespace tx_sim {
namespace coordinator {
//! @brief 函数名：Build
//! @details 函数功能：解析XOSC文件，提取车辆信息
//! @return 布尔值，表示解析是否成功
bool CDefaultXOSCParser::Build() {
  m_defaultXOSCXMLRoot = GetXMLRoot(m_defaultXOSCPath, m_doc);
  if (!m_defaultXOSCXMLRoot) return false;
  BuildVehicleNameMap();
  return true;
}

//! @brief 函数名：GetPhysicleEgos
//! @details 函数功能：获取指定名称的物理车辆信息
//! @param[in] sEgoName 车辆名称
//! @return 一个整数到XMLElement指针的映射，表示物理车辆信息
std::map<int, const tinyxml2::XMLElement*> CDefaultXOSCParser::GetPhysicleEgos(std::string sEgoName) {
  std::map<int, const tinyxml2::XMLElement*> mapNode;
  if (m_mapNameVehicle.empty()) return mapNode;
  const XMLElement* Vehicle_node = m_mapNameVehicle[sEgoName];
  if (!Vehicle_node) {
    std::string sErrorFile = __FILE__;
    sErrorFile += ":";
    sErrorFile += std::to_string(__LINE__);
    throw std::invalid_argument(sErrorFile + " no vehicle element " + sEgoName + " found in " + m_defaultXOSCPath);
  }
  std::string sVehicleType = GetNodeAttributeWithDefault(Vehicle_node, "vehicleCategory", "");
  if (sVehicleType == "combination") {
    const XMLElement* pVehicleChild = Vehicle_node->FirstChildElement();
    while (pVehicleChild) {
      std::string sNodeName = pVehicleChild->Name();
      std::string sNodeNameValue = GetNodeAttributeWithDefault(pVehicleChild, "name", "");
      if ("Tractor" == sNodeName) {
        mapNode[0] = m_mapNameVehicle[sNodeNameValue];
      } else if ("Trailer" == sNodeName) {
        std::string sUnit = pVehicleChild->Attribute("trailingUnit");
        mapNode[std::stoi(sUnit)] = m_mapNameVehicle[sNodeNameValue];
      }
      if ("Tractor" == sNodeName || "Trailer" == sNodeName) {
        double offsetX = 0.0;
        double offsetZ = 0.0;
        const char* pOffsetX = pVehicleChild->Attribute("offsetX");
        if (pOffsetX) offsetX = std::stod(pOffsetX);
        const char* pOffsetZ = pVehicleChild->Attribute("offsetZ");
        if (pOffsetZ) offsetZ = std::stod(pOffsetZ);
        m_mapTruckOffset[sNodeNameValue] = std::make_tuple(offsetX, offsetZ);
      }

      pVehicleChild = pVehicleChild->NextSiblingElement();
    }
  } else {
    mapNode[0] = Vehicle_node;
  }
  return mapNode;
}

//! @brief 函数名：BuildPhysicleEgo
//! @details 函数功能：构建指定名称的物理车辆信息
//! @param[in] pEgo 指向Ego对象的指针
//! @param[in] sEgoName 车辆名称
//! @return 布尔值，表示构建是否成功
bool CDefaultXOSCParser::BuildPhysicleEgo(sim_msg::Ego* pEgo, std::string sEgoName) {
  std::map<int, const tinyxml2::XMLElement*> mapVehicleNodes = GetPhysicleEgos(sEgoName);
  if (mapVehicleNodes.empty()) return false;
  for (auto it = mapVehicleNodes.begin(); it != mapVehicleNodes.end(); ++it) {
    const tinyxml2::XMLElement* Vehicle_node = it->second;
    sim_msg::PhysicleEgo* pPhysicleEgo = pEgo->add_physicles();
    sim_msg::PhysicleCommon* pPhysicleCommon = pPhysicleEgo->mutable_common();
    sim_msg::BoundingBox* pBoundingBox = pPhysicleCommon->mutable_bounding_box();

    const XMLElement* BoundingBox = GetChildNode(Vehicle_node, "BoundingBox");
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

    // const XMLElement* Axles = GetChildNode(Vehicle_node, "Axles");
    // const XMLElement* FrontAxle = GetChildNode(Axles, "FrontAxle");
    // std::string maxSteering = FrontAxle->Attribute("maxSteering");
    // std::string wheelDiameter = FrontAxle->Attribute("wheelDiameter");
    // std::string trackWidth = FrontAxle->Attribute("trackWidth");
    // std::string positionX = FrontAxle->Attribute("positionX");
    // std::string positionZ = FrontAxle->Attribute("positionZ");

    // sim_msg::Alxe* pfront_axle = pPhysicleEgo->mutable_front_axle();
    // pfront_axle->set_max_steering(stod(maxSteering));
    // pfront_axle->set_position_x(stod(positionX));
    // pfront_axle->set_position_z(stod(positionZ));
    // pfront_axle->set_track_width(stod(trackWidth));
    // pfront_axle->set_wheel_diameter(stod(wheelDiameter));

    // const XMLElement* RearAxle = GetChildNode(Axles, "RearAxle");
    // maxSteering = RearAxle->Attribute("maxSteering");
    // wheelDiameter = RearAxle->Attribute("wheelDiameter");
    // trackWidth = RearAxle->Attribute("trackWidth");
    // positionX = RearAxle->Attribute("positionX");
    // positionZ = RearAxle->Attribute("positionZ");

    // sim_msg::Alxe* prear_axle = pPhysicleEgo->mutable_rear_axle();
    // prear_axle->set_max_steering(stod(maxSteering));
    // prear_axle->set_position_x(stod(positionX));
    // prear_axle->set_position_z(stod(positionZ));
    // prear_axle->set_track_width(stod(trackWidth));
    // prear_axle->set_wheel_diameter(stod(wheelDiameter));

    const XMLElement* Properties = GetChildNode(Vehicle_node, "Properties");
    const XMLElement* Property = Properties->FirstChildElement();
    while (Property) {
      std::string sName = Property->Attribute("name");
      if (sName == "model_id") {
        std::string sValue = Property->Attribute("value");
        if (!sValue.empty()) pPhysicleCommon->set_model_id(std::stoi(sValue));
      } else if (sName == "model3d") {
        std::string sValue = Property->Attribute("value");
        pPhysicleCommon->set_model_3d(sValue);
      }
      Property = Property->NextSiblingElement();
    }

    sim_msg::VehicleGeometory* pVehicleGeometory = pPhysicleEgo->mutable_geometory();
    std::string sName = it->second->Attribute("name");
    double offsetX = std::get<0>(m_mapTruckOffset[sName]);
    double offsetZ = std::get<1>(m_mapTruckOffset[sName]);
    VehicleXMLNode2GeometryMsg(GetNodeWithAttributeInList(Vehicle_node, "name", sName.c_str(), "Catalog.Vehicle"),
                               pVehicleGeometory, offsetX, offsetZ);
  }
  return true;
}

//! @brief 函数名：ParseCatalogVehicleProperties
//! @details 函数功能：解析车辆属性
//! @param[in] sEgoName 车辆名称
//! @return 一个包含控制器名称、传感器组名称和动态属性的元组
std::tuple<std::string, std::string, std::string> CDefaultXOSCParser::ParseCatalogVehicleProperties(
    std::string sEgoName) {
  std::string sControllerName;
  std::string sSensorGroup;
  std::string sDynamicGroup;
  const XMLElement* Vehicle_node = m_mapNameVehicle[sEgoName];
  if (!Vehicle_node) {
    std::string sErrorFile = __FILE__;
    sErrorFile += ":";
    sErrorFile += std::to_string(__LINE__);
    throw std::invalid_argument(sErrorFile + " no vehicle element " + sEgoName + " found in " + m_defaultXOSCPath);
  }
  const XMLElement* properties = GetChildNode(Vehicle_node, "Properties");
  const XMLElement* property = properties->FirstChildElement();
  while (property) {
    std::string name = GetNodeAttributeWithDefault(property, "name", "");
    std::string value = GetNodeAttributeWithDefault(property, "value", "");
    if (name == "controller") {
      sControllerName = value;
      LOG(INFO) << "controller:" << sControllerName;
    } else if (name == "SensorGroup") {
      sSensorGroup = value;
      LOG(INFO) << "SensorGroup:" << sSensorGroup;
    } else if (name == "Dynamic") {
      sDynamicGroup = value;
      LOG(INFO) << "Dynamic:" << sDynamicGroup;
    }
    property = property->NextSiblingElement();
  }
  return std::make_tuple(sControllerName, sSensorGroup, sDynamicGroup);
}

//! @brief 函数名：BuildVehicleNameMap
//! @details 函数功能：构建车辆名称映射
void CDefaultXOSCParser::BuildVehicleNameMap() {
  const XMLElement* catalog_node = m_defaultXOSCXMLRoot->FirstChildElement();
  const XMLElement* Vehicle_node = nullptr;
  while (catalog_node) {
    std::string sName = catalog_node->Name();
    if (sName == "Catalog") {
      Vehicle_node = catalog_node->FirstChildElement();
      while (Vehicle_node) {
        std::string sVehicleName = Vehicle_node->Attribute("name");
        m_mapNameVehicle[sVehicleName] = Vehicle_node;
        Vehicle_node = Vehicle_node->NextSiblingElement();
      }
    }
    catalog_node = catalog_node->NextSiblingElement();
  }
}
}  // namespace coordinator
}  // namespace tx_sim
