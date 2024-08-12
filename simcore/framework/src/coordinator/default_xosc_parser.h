// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <map>
#include <string>
#include <vector>

#include "boost/filesystem/path.hpp"
#include "scene.pb.h"
#include "tinyxml2.h"

namespace tx_sim {
namespace coordinator {
//! @brief 类名：CDefaultXOSCParser
//! @details 类功能：解析XOSC文件，提取车辆信息
class CDefaultXOSCParser {
 public:
  //! @brief 构造函数
  //! @details 初始化CDefaultXOSCParser对象，设置XOSC文件路径
  //! @param[in] sPath XOSC文件路径
  CDefaultXOSCParser(std::string sPath) : m_defaultXOSCPath(sPath){};

  //! @brief 析构函数
  //! @details 释放CDefaultXOSCParser对象占用的资源
  ~CDefaultXOSCParser(){};

  //! @brief 函数名：Build
  //! @details 函数功能：解析XOSC文件，提取车辆信息
  //! @return 布尔值，表示解析是否成功
  bool Build();

  //! @brief 函数名：GetPhysicleEgos
  //! @details 函数功能：获取指定名称的物理车辆信息
  //! @param[in] sEgoName 车辆名称
  //! @return 一个整数到XMLElement指针的映射，表示物理车辆信息
  std::map<int, const tinyxml2::XMLElement*> GetPhysicleEgos(std::string sEgoName);

  //! @brief 函数名：BuildPhysicleEgo
  //! @details 函数功能：构建指定名称的物理车辆信息
  //! @param[in] pEgo 指向Ego对象的指针
  //! @param[in] sEgoName 车辆名称
  //! @return 布尔值，表示构建是否成功
  bool BuildPhysicleEgo(sim_msg::Ego* pEgo, std::string sEgoName);

  //! @brief 函数名：ParseCatalogVehicleProperties
  //! @details 函数功能：解析车辆属性
  //! @param[in] sEgoName 车辆名称
  //! @return 一个包含控制器名称、传感器组名称和动态属性的元组
  std::tuple<std::string, std::string, std::string> ParseCatalogVehicleProperties(std::string sEgoName);

  //! @brief 函数名：BuildVehicleNameMap
  //! @details 函数功能：构建车辆名称映射
  void BuildVehicleNameMap();

 private:
  //! @brief 成员变量：m_defaultXOSCPath
  //! @details 存储XOSC文件路径
  std::string m_defaultXOSCPath;

  //! @brief 成员变量：m_defaultXOSCXMLRoot
  //! @details 存储XOSC文件的根元素
  const tinyxml2::XMLElement* m_defaultXOSCXMLRoot = nullptr;

  //! @brief 成员变量：m_doc
  //! @details 存储XOSC文件的XML文档对象
  tinyxml2::XMLDocument m_doc;

  //! @brief 成员变量：m_mapNameVehicle
  //! @details 存储车辆名称到XMLElement指针的映射
  std::unordered_map<std::string, const tinyxml2::XMLElement*> m_mapNameVehicle;

  //! @brief 成员变量：m_mapTruckOffset
  //! @details 存储卡车偏移量信息
  std::unordered_map<std::string, std::tuple<double, double>> m_mapTruckOffset;
};

}  // namespace coordinator
}  // namespace tx_sim
