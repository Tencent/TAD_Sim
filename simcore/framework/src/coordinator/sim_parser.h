// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "boost/filesystem/path.hpp"
#include "scene.pb.h"
#include "tinyxml2.h"

namespace tx_sim {
namespace coordinator {
//! @brief 用户自定义模式字符串
//! @details 用户自定义模式字符串，用于解析用户自定义事件
static constexpr const char* kUserDefinePattern =
    R"(\{(actionid):(\d+),(type):(status|command),(value):(.*?),(subtype):(userdefine),(offset):(0.0),(multi):(\{\})\})";

//! @brief 用户自定义模式子匹配大小
//! @details 用户自定义模式子匹配大小，用于解析用户自定义事件
static constexpr int kUserDefinePatternSubMatchSize = 12;

class CSimParser {
 public:
  // 构造函数
  //! @param[in] sPath 传感器目录配置文件的路径
  CSimParser(std::string sPath) : m_simPath(sPath) {}

  // 析构函数
  ~CSimParser() {}

  //! @brief 构建传感器目录
  //! @return 构建成功返回true，否则返回false
  bool Build();

  //! @brief 获取Ego信息
  //! @return 返回一个包含Ego信息的tuple
  std::tuple<std::string, std::string> GetEgoInfo();
  const tinyxml2::XMLElement* SimXMLRoot() const { return m_simXMLRoot; }
  //! @brief 获取场景路径、高精度地图路径和交通路径
  //! @return 返回一个包含场景路径、高精度地图路径和交通路径的tuple
  std::tuple<std::string, std::string, std::string> GetScenePath_HadMapPath_TrafficPath();

  //! @brief 获取环境全路径
  //! @param[in] defaultXOSCPath 默认的XOSC路径
  //! @return 返回环境全路径
  std::string GetEnviromentFullPath(std::string defaultXOSCPath);

  //! @brief 构建初始Ego
  //! @param[in] pInitEgo 指向初始Ego对象的指针
  //! @return 构建成功返回true，否则返回false
  bool BuildInitEgo(sim_msg::InitialEgo* pInitEgo, const tinyxml2::XMLElement* node_plan);

  //! @brief 构建动态Ego
  //! @param[in] pDynamicEgo 指向动态Ego对象的指针
  //! @return 构建成功返回true，否则返回false
  bool BuildDynamicEgo(sim_msg::Dynamic* pDynamicEgo, const tinyxml2::XMLElement* node_plan);

  //! @brief 构建物理Ego
  //! @param[in] pPhysicleEgo 指向物理Ego对象的指针
  //! @return 构建成功返回true，否则返回false
  bool BuildPhysicleEgo(sim_msg::PhysicleEgo& pPhysicleEgo, const tinyxml2::XMLElement* node_plan);

  //! @brief 构建设置参数声明
  //! @param[in] pSetting 指向设置对象的指针
  void BuildSettingParameterDeclarations(sim_msg::Setting* pSetting);

 private:
  //! @brief 解析场景事件
  //! @param[in] sceneXMLNode 指向场景XML节点的指针
  //! @param[in] pDynamicEgo 指向动态Ego对象的指针
  void ParseSceneEvent(const tinyxml2::XMLElement* sceneXMLNode, sim_msg::Dynamic* pDynamicEgo);  // version 1.1.0.0

  //! @brief 解析场景事件（版本1.2.0.0）
  //! @param[in] sceneXMLNode 指向场景XML节点的指针
  //! @param[in] pDynamicEgo 指向动态Ego对象的指针
  void ParseSceneEvent_1_2_0_0(const tinyxml2::XMLElement* sceneXMLNode, sim_msg::Dynamic* pDynamicEgo);

  //! @brief 获取规则
  //! @param[in] sRule 规则字符串
  //! @return 返回一个包含规则信息的对象
  sim_msg::Rule GetRule(std::string sRule);

  //! @brief 成员变量：m_simPath
  //! @details 成员变量功能：传感器目录配置文件的路径
  std::string m_simPath;

  //! @brief 成员变量：m_simXMLRoot
  //! @details 成员变量功能：指向传感器目录配置文件XML根节点的指针
  const tinyxml2::XMLElement* m_simXMLRoot = nullptr;

  //! @brief 成员变量：m_simXMLPlanNode
  //! @details 成员变量功能：指向传感器目录配置文件XML规划节点的指针
  const tinyxml2::XMLElement* m_simXMLPlanNode = nullptr;

  //! @brief 成员变量：m_doc
  //! @details 成员变量功能：用于解析传感器目录配置文件的XML文档对象
  tinyxml2::XMLDocument m_doc;
};

}  // namespace coordinator
}  // namespace tx_sim
