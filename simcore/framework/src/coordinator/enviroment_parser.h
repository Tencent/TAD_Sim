// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "environment.pb.h"

namespace tx_sim {
namespace coordinator {
//! @brief 类名：CEnviromentParser
//! @details 类功能：解析环境配置文件，提取环境信息
class CEnviromentParser {
 public:
  //! @brief 构造函数
  //! @details 初始化CEnviromentParser对象，设置环境配置文件路径
  //! @param[in] envPath 环境配置文件路径
  CEnviromentParser(std::string envPath);

  //! @brief 析构函数
  //! @details 释放CEnviromentParser对象占用的资源
  ~CEnviromentParser();

  //! @brief 函数名：BuildEnviroment
  //! @details 函数功能：解析环境配置文件，提取环境信息
  //! @param[in] mapEnv 指向一个整数到EnvironmentalConditions的映射的指针，用于存储解析出的环境信息
  //! @return 布尔值，表示解析是否成功
  bool BuildEnviroment(google::protobuf::Map<int64_t, sim_msg::EnvironmentalConditions>* mapEnv);

 private:
  //! @brief 成员变量：m_envPath
  //! @details 存储环境配置文件路径
  std::string m_envPath;
};

}  // namespace coordinator
}  // namespace tx_sim
