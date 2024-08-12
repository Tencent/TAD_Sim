// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <string>
#include "default_xosc_parser.h"
#include "sensor_catalog_parser.h"
#include "sim_parser.h"
#include "traffic_parser.h"

#include "scene.pb.h"
#include "tinyxml2.h"
#include "vehicle_geometry.pb.h"

using namespace tinyxml2;

namespace tx_sim {
namespace coordinator {
//! @brief 类名：CScenePbFacade
//! @details 类功能：用于处理场景配置文件的类
class CScenePbFacade {
 public:
  //! @brief 构造函数
  //! @details 初始化类的成员变量
  //!
  //! @param[in] simPath 场景配置文件的路径
  //! @param[in] defaultPath 默认配置文件的路径
  //! @param[in] vehicleCatalogPath 车辆目录配置文件的路径
  //! @param[in] miscCatalogPath 杂项目录配置文件的路径
  //! @param[in] pedestrianCatalogPath 行人目录配置文件的路径
  CScenePbFacade(const std::string& simPath, const std::string& defaultXOSCSysPath,
                 const std::string& vehicleCatalogSysPath, const std::string& miscCatalogSysPath,
                 const std::string& pedestrianCatalogSysPath, const std::string& defaultXOSCDataPath,
                 const std::string& vehicleCatalogDataPath, const std::string& miscCatalogDataPath,
                 const std::string& pedestrianCatalogDataPath, const std::string& mapModel3dCatalogXMLDataPath);

  //! @brief 析构函数
  //! @details 释放类的成员变量
  ~CScenePbFacade();

  //! @brief 函数名：BuildScenePb
  //! @details 函数功能：根据场景配置文件构建场景
  //!
  //! @param[in,out] scene 一个用于存储场景信息的Scene对象
  //!
  //! @return 如果成功构建场景，则返回true，否则返回false
  bool BuildScenePb(sim_msg::Scene& scene);

  //! @brief 函数名：BuildCloudCityScenePb
  //! @details 函数功能：根据场景配置文件构建云端城市场景
  //!
  //! @param[in,out] scene 一个用于存储场景信息的Scene对象
  //!
  //! @return 如果成功构建场景，则返回true，否则返回false
  bool BuildCloudCityScenePb(sim_msg::Scene& scene);

 private:
  //! @brief 函数名：BuildSetting
  //! @details 函数功能：构建场景设置
  //!
  //! @param[in,out] scene 一个用于存储场景信息的Scene对象
  void BuildSetting(sim_msg::Scene& scene);

  //! @brief 函数名：BuildEgo
  //! @details 函数功能：构建自车
  //!
  //! @param[in,out] scene 一个用于存储场景信息的Scene对象
  void BuildEgo(sim_msg::Scene& scene);
  void BuildEgos(sim_msg::Scene& scene);
  //! @brief 函数名：BuildTraffic
  //! @details 函数功能：构建交通
  //!
  //! @param[in,out] scene 一个用于存储场景信息的Scene对象
  void BuildTraffic(sim_msg::Scene& scene);

  //! @brief 函数名：BuildEnviroment
  //! @details 函数功能：构建环境
  //!
  //! @param[in,out] scene 一个用于存储场景信息的Scene对象
  void BuildEnviroment(sim_msg::Scene& scene);
  bool isMultiEgoVersion();

  //! @brief 成员变量：m_simParser
  //! @details 成员变量功能：用于解析场景配置文件的对象
  std::unique_ptr<CSimParser> m_simParser;

  //! @brief 成员变量：m_defaultXOSCParser
  //! @details 成员变量功能：用于解析默认配置文件的对象
  std::unique_ptr<CDefaultXOSCParser> m_defaultXOSCDataParser;

  //! @brief 成员变量：m_vehicleCatalogXOSCParser
  //! @details 成员变量功能：用于解析车辆目录配置文件的对象
  std::shared_ptr<CVehicleCatalogXOSCParser> m_vehicleCatalogXOSCParser;

  //! @brief 成员变量：m_trafficParser
  //! @details 成员变量功能：用于解析交通配置文件的对象
  std::unique_ptr<CTrafficParser> m_trafficParser;

  //! @brief 成员变量：m_sensorParser
  //! @details 成员变量功能：用于解析传感器目录配置文件的对象
  std::shared_ptr<CSensorCatalogParser> m_sensorDataParser;

  //! @brief 成员变量：m_simPath
  //! @details 成员变量功能：场景配置文件的路径
  std::string m_simPath;

  //! @brief 成员变量：m_trafficPath
  //! @details 成员变量功能：交通配置文件的路径
  std::string m_trafficPath;

  //! @brief 成员变量：m_defaultXOSCPath
  //! @details 成员变量功能：默认配置文件的路径
  std::string m_defaultXOSCSysPath;

  //! @brief m_vehicleCatalogXOSCSysPath
  //! @details 成员变量功能：车辆目录配置文件的路径
  std::string m_vehicleCatalogXOSCSysPath;

  //! @brief m_miscCatalogXOSCSysPath
  //! @details 成员变量功能：杂项目录配置文件的路径
  std::string m_miscCatalogXOSCSysPath;

  //! @brief m_pedestrianCatalogXOSCSysPath
  //! @details 成员变量功能：行人目录配置文件的路径
  std::string m_pedestrianCatalogXOSCSysPath;

  std::string m_defaultXOSCDataPath;
  std::string m_vehicleCatalogXOSCDataPath;
  std::string m_miscCatalogXOSCDataPath;
  std::string m_pedestrianCatalogXOSCDataPath;
  std::string m_mapModel3dCatalogXMLDataPath;

  //! @brief 成员变量：m_enviromentPath
  //! @details 成员变量功能：环境配置文件的路径
  std::string m_enviromentPath;

  //! @brief 成员变量：m_simXMLRoot
  //! @details 成员变量功能：指向场景配置文件XML根节点的指针
  const XMLElement* m_simXMLRoot = nullptr;

  //! @brief 成员变量：m_defaultXOSCXMLRoot
  //! @details 成员变量功能：指向默认配置文件XML根节点的指针
  XMLElement* m_defaultXOSCXMLRoot = nullptr;

  //! @brief 成员变量：m_trafficXMLRoot
  //! @details 成员变量功能：指向交通配置文件XML根节点的指针
  XMLElement* m_trafficXMLRoot = nullptr;
};

}  // namespace coordinator
}  // namespace tx_sim
