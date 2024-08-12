// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "sensor_catalog_parser.h"
#include <regex>
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "glog/logging.h"
#include "json/json.h"
#include "xml_util.h"


using namespace tx_sim::utils;
namespace fs = boost::filesystem;

namespace tx_sim {
namespace coordinator {

//! @brief 构建传感器目录
//! @return 构建成功返回true，否则返回false
bool CSensorCatalogParser::Build() {
  if (!fs::is_regular_file(m_sensorCatalogPath)) return true;
  m_sensorCatalogRoot = GetXMLRoot(m_sensorCatalogPath, m_doc);
  if (!m_sensorCatalogRoot) return false;
  BuildSensorMap();
  return true;
}
//! @brief 函数名：GetSensorType
//! @details 函数功能：
//!
//! @param[in,out]
//!
//! @return
sim_msg::SensorType GetSensorType(std::string sType) {
  sim_msg::SensorType sensorType = sim_msg::SensorType::SENSOR_TYPE_USER_DEFINE;
  // sensortype camera
  if (sType == "Camera") sensorType = sim_msg::SensorType::SENSOR_TYPE_CAMERA;
  // sensortype radar
  else if (sType == "Radar")
    sensorType = sim_msg::SensorType::SENSOR_TYPE_RADAR;
  // sensortype TraditionalLidar
  else if (sType == "TraditionalLidar")
    sensorType = sim_msg::SensorType::SENSOR_TYPE_TRADITIONAL_LIDAR;
  // sensor type truth
  else if (sType == "Truth")
    sensorType = sim_msg::SensorType::SENSOR_TYPE_TRUTH;
  // sensor type IMU
  else if (sType == "IMU")
    sensorType = sim_msg::SensorType::SENSOR_TYPE_IMU;
  // sensor type GPS
  else if (sType == "GPS")
    sensorType = sim_msg::SensorType::SENSOR_TYPE_GPS;
  // sensor type fisheye
  else if (sType == "Fisheye")
    sensorType = sim_msg::SensorType::SENSOR_TYPE_FISHEYE;
  // sensor type Semantic
  else if (sType == "Semantic")
    sensorType = sim_msg::SensorType::SENSOR_TYPE_SEMANTIC;
  // sensor type Depth
  else if (sType == "Depth")
    sensorType = sim_msg::SensorType::SENSOR_TYPE_DEPTH;
  // sensor type Ultrasonic
  else if (sType == "Ultrasonic")
    sensorType = sim_msg::SensorType::SENSOR_TYPE_ULTRASONIC;
  // sensor type OBU
  else if (sType == "OBU")
    sensorType = sim_msg::SensorType::SENSOR_TYPE_OBU;
  return sensorType;
}

//! @brief 构建传感器组
//! @param[in] pSensorGroup 指向传感器组对象的指针
//! @param[in] sGroupID 传感器组的ID
void CSensorCatalogParser::BuildSensorGroup(sim_msg::SensorGroup* pSensorGroup, std::string sGroupID) {
  const tinyxml2::XMLElement* pSensorGroupNode = m_mapSensorGroup[sGroupID];
  if (!pSensorGroupNode) {
    LOG(ERROR) << "can not find sensor group ID" + sGroupID;
    return;
  }
  const tinyxml2::XMLElement* pSensorGroupItemNode = pSensorGroupNode->FirstChildElement();
  while (pSensorGroupItemNode) {
    pSensorGroup->set_name(sGroupID);
    std::string sIDx = pSensorGroupItemNode->Attribute("idx");
    std::string sID = pSensorGroupItemNode->Attribute("ID");
    const tinyxml2::XMLElement* pSensorDefineItem = m_mapSensorDefine[sIDx];
    if (!pSensorDefineItem) {
      pSensorGroupItemNode = pSensorGroupItemNode->NextSiblingElement();
      continue;
    }
    std::string sSensorName = pSensorDefineItem->Name();
    sim_msg::SensorType sensorType = GetSensorType(sSensorName);
    sim_msg::Sensor* pSensor = pSensorGroup->add_sensors();
    pSensor->set_type(sensorType);
    sim_msg::Sensor_Extrinsic* pSensor_Extrinsic = pSensor->mutable_extrinsic();
    pSensor_Extrinsic->set_id(std::stoll(sID));
    pSensor_Extrinsic->set_installslot(pSensorGroupItemNode->Attribute("InstallSlot"));
    pSensor_Extrinsic->set_device(pSensorGroupItemNode->Attribute("Device"));
    pSensor_Extrinsic->set_locationx(std::stod(pSensorGroupItemNode->Attribute("LocationX")));
    pSensor_Extrinsic->set_locationy(std::stod(pSensorGroupItemNode->Attribute("LocationY")));
    pSensor_Extrinsic->set_locationz(std::stod(pSensorGroupItemNode->Attribute("LocationZ")));
    pSensor_Extrinsic->set_rotationx(std::stod(pSensorGroupItemNode->Attribute("RotationX")));
    pSensor_Extrinsic->set_rotationy(std::stod(pSensorGroupItemNode->Attribute("RotationY")));
    pSensor_Extrinsic->set_rotationz(std::stod(pSensorGroupItemNode->Attribute("RotationZ")));

    sim_msg::Sensor_Intrinsic* pSensor_Intrinsic = pSensor->mutable_intrinsic();
    auto mapParams = pSensor_Intrinsic->mutable_params();
    const XMLAttribute* pAttributeItem = pSensorDefineItem->FirstAttribute();
    while (pAttributeItem) {
      std::string sAttribute = pAttributeItem->Name();
      std::string sValue = pAttributeItem->Value();
      (*mapParams)[sAttribute] = sValue;
      pAttributeItem = pAttributeItem->Next();
    }
    pSensorGroupItemNode = pSensorGroupItemNode->NextSiblingElement();
  }
}

//! @brief 构建传感器映射
//! @details 构建传感器组和传感器定义的映射
void CSensorCatalogParser::BuildSensorMap() {
  const XMLElement* pNodeCata = m_sensorCatalogRoot->FirstChildElement();
  while (pNodeCata) {
    std::string sName = GetNodeAttributeWithDefault(pNodeCata, "name", "");
    if (sName == "SensorDefine") {
      const XMLElement* pNodeCatalogSensorDefineItem = pNodeCata->FirstChildElement();
      while (pNodeCatalogSensorDefineItem) {
        std::string sID = GetNodeAttributeWithDefault(pNodeCatalogSensorDefineItem, "idx", "");
        m_mapSensorDefine[sID] = pNodeCatalogSensorDefineItem;
        pNodeCatalogSensorDefineItem = pNodeCatalogSensorDefineItem->NextSiblingElement();
      }
    } else if (sName == "SensorGroup") {
      const XMLElement* pNodeCatalogSensorGroupItem = pNodeCata->FirstChildElement();
      while (pNodeCatalogSensorGroupItem) {
        std::string sID = GetNodeAttributeWithDefault(pNodeCatalogSensorGroupItem, "name", "");
        m_mapSensorGroup[sID] = pNodeCatalogSensorGroupItem;
        pNodeCatalogSensorGroupItem = pNodeCatalogSensorGroupItem->NextSiblingElement();
      }
    }
    pNodeCata = pNodeCata->NextSiblingElement();
  }
}

}  // namespace coordinator
}  // namespace tx_sim
