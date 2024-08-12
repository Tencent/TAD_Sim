// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "boost/filesystem/path.hpp"
#include "scene.pb.h"
#include "tinyxml2.h"

namespace tx_sim {
namespace coordinator {
class CSensorCatalogParser {
 public:
  // 构造函数
  //! @param[in] sPath 传感器目录配置文件的路径
  CSensorCatalogParser(std::string sPath) : m_sensorCatalogPath(sPath) {}

  // 析构函数
  ~CSensorCatalogParser() {}

  //! @brief 构建传感器目录
  //! @return 构建成功返回true，否则返回false
  bool Build();

  //! @brief 构建传感器组
  //! @param[in] pSensorGroup 指向传感器组对象的指针
  //! @param[in] sGroupID 传感器组的ID
  void BuildSensorGroup(sim_msg::SensorGroup* pSensorGroup, std::string sGroupID);

 private:
  //! @brief 构建传感器映射
  //! @details 构建传感器组和传感器定义的映射
  void BuildSensorMap();

  //! @brief 成员变量：m_sensorCatalogPath
  //! @details 成员变量功能：传感器目录配置文件的路径
  std::string m_sensorCatalogPath;

  //! @brief 成员变量：m_sensorCatalogRoot
  //! @details 成员变量功能：指向传感器目录配置文件XML根节点的指针
  const tinyxml2::XMLElement* m_sensorCatalogRoot = nullptr;

  //! @brief 成员变量：m_doc
  //! @details 成员变量功能：用于解析传感器目录配置文件的XML文档对象
  tinyxml2::XMLDocument m_doc;

  //! @brief 成员变量：m_mapSensorGroup
  //! @details 成员变量功能：存储传感器组的映射
  std::unordered_map<std::string, const tinyxml2::XMLElement*> m_mapSensorGroup;

  //! @brief 成员变量：m_mapSensorDefine
  //! @details 成员变量功能：存储传感器定义的映射
  std::unordered_map<std::string, const tinyxml2::XMLElement*> m_mapSensorDefine;
};

}  // namespace coordinator
}  // namespace tx_sim
