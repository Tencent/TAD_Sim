// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "coordinator/local_client.h"

namespace tx_sim {
namespace cli {

// This interface is a wrapper for Client in local_client.h
class ISimClient {
 public:
  //!
  //! @brief 函数名：Setup
  //! @details 函数功能：设置模拟客户端
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  virtual void Setup(const tx_sim::coordinator::CommandStatusCb& cb) = 0;

  //!
  //! @brief 函数名：UnSetup
  //! @details 函数功能：取消设置模拟客户端
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  virtual void UnSetup(const tx_sim::coordinator::CommandStatusCb& cb) = 0;

  //!
  //! @brief 函数名：Step
  //! @details 函数功能：执行模拟客户端的一步
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  virtual void Step(const tx_sim::coordinator::CommandStatusCb& cb) = 0;

  //!
  //! @brief 函数名：Stop
  //! @details 函数功能：停止模拟客户端
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  virtual void Stop(const tx_sim::coordinator::CommandStatusCb& cb) = 0;

  //!
  //! @brief 函数名：Run
  //! @details 函数功能：运行模拟客户端
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  virtual void Run(const tx_sim::coordinator::CommandStatusCb& cb) = 0;

  //!
  //! @brief 函数名：Pause
  //! @details 函数功能：暂停模拟客户端
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  virtual void Pause(const tx_sim::coordinator::CommandStatusCb& cb) = 0;

  //!
  //! @brief 函数名：LaunchModule
  //! @details 函数功能：启动模块
  //!
  //! @param[in] name 模块名称
  //! @return 命令执行错误码
  //!
  virtual tx_sim::impl::CmdErrorCode LaunchModule(const std::string& name) = 0;

  //!
  //! @brief 函数名：TerminateModule
  //! @details 函数功能：终止模块
  //!
  //! @param[in] name 模块名称
  //! @return 命令执行错误码
  //!
  virtual tx_sim::impl::CmdErrorCode TerminateModule(const std::string& name) = 0;

  //!
  //! @brief 函数名：AddModuleConfig
  //! @details 函数功能：添加模块配置
  //!
  //! @param[in] config_json_str 模块配置 JSON 字符串
  //! @return 命令执行错误码
  //!
  virtual tx_sim::impl::CmdErrorCode AddModuleConfig(const std::string& config_json_str) = 0;

  //!
  //! @brief 函数名：RemoveModuleConfig
  //! @details 函数功能：删除模块配置
  //!
  //! @param[in] name 模块名称
  //! @return 命令执行错误码
  //!
  virtual tx_sim::impl::CmdErrorCode RemoveModuleConfig(const std::string& name) = 0;

  //!
  //! @brief 函数名：UpdateModuleConfig
  //! @details 函数功能：更新模块配置
  //!
  //! @param[in] config_json_str 模块配置 JSON 字符串
  //! @return 命令执行错误码
  //!
  virtual tx_sim::impl::CmdErrorCode UpdateModuleConfig(const std::string& config_json_str) = 0;

  //!
  //! @brief 函数名：GetModuleConfig
  //! @details 函数功能：获取模块配置
  //!
  //! @param[in] name 模块名称
  //! @param[out] config_json_str 模块配置 JSON 字符串
  //! @return 命令执行错误码
  //!
  virtual tx_sim::impl::CmdErrorCode GetModuleConfig(const std::string& name, std::string& config_json_str) = 0;

  //!
  //! @brief 函数名：GetAllModuleConfigs
  //! @details 函数功能：获取所有模块配置
  //!
  //! @param[out] configs_json_str 所有模块配置 JSON 字符串列表
  //! @return 命令执行错误码
  //!
  virtual tx_sim::impl::CmdErrorCode GetAllModuleConfigs(std::vector<std::string>& configs_json_str) = 0;

  //!
  //! @brief 函数名：SetSysConfigs
  //! @details 函数功能：设置系统配置
  //!
  //! @param[in] config_json_str 系统配置 JSON 字符串
  //! @return 命令执行错误码
  //!
  virtual tx_sim::impl::CmdErrorCode SetSysConfigs(const std::string& config_json_str) = 0;

  //!
  //! @brief 函数名：GetSysConfigs
  //! @details 函数功能：获取系统配置
  //!
  //! @param[out] config_json_str 系统配置 JSON 字符串
  //! @return 命令执行错误码
  //!
  virtual tx_sim::impl::CmdErrorCode GetSysConfigs(std::string& config_json_str) = 0;

  //!
  //! @brief 函数名：RestoreDefaultConfigs
  //! @details 函数功能：恢复默认配置
  //!
  //! @return 命令执行错误码
  //!
  virtual tx_sim::impl::CmdErrorCode RestoreDefaultConfigs() = 0;
};

class ISimClientFactory {
 public:
  virtual std::shared_ptr<ISimClient> CreateSimClient(const std::string& endpoint) = 0;
};

//!
//! @brief 类名：SimClient
//! @details 类功能：模拟客户端类
//!
class SimClient final : public ISimClient {
 public:
  //!
  //! @brief 构造函数
  //! @details 构造模拟客户端对象
  //!
  //! @param[in] endpoint 端点
  //!
  SimClient(const std::string& endpoint);

  //!
  //! @brief 析构函数
  //! @details 析构模拟客户端对象
  //!
  ~SimClient() {}

  //!
  //! @brief 函数名：Setup
  //! @details 函数功能：设置模拟客户端
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  void Setup(const tx_sim::coordinator::CommandStatusCb& cb) override;

  //!
  //! @brief 函数名：UnSetup
  //! @details 函数功能：取消设置模拟客户端
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  void UnSetup(const tx_sim::coordinator::CommandStatusCb& cb) override;

  //!
  //! @brief 函数名：Step
  //! @details 函数功能：执行模拟客户端的一步
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  void Step(const tx_sim::coordinator::CommandStatusCb& cb) override;

  //!
  //! @brief 函数名：Stop
  //! @details 函数功能：停止模拟客户端
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  void Stop(const tx_sim::coordinator::CommandStatusCb& cb) override;

  //!
  //! @brief 函数名：Run
  //! @details 函数功能：运行模拟客户端
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  void Run(const tx_sim::coordinator::CommandStatusCb& cb) override;

  //!
  //! @brief 函数名：Pause
  //! @details 函数功能：暂停模拟客户端
  //!
  //! @param[in] cb 命令状态回调函数
  //!
  void Pause(const tx_sim::coordinator::CommandStatusCb& cb) override;

  //!
  //! @brief 函数名：LaunchModule
  //! @details 函数功能：启动模块
  //!
  //! @param[in] name 模块名称
  //! @return 命令执行错误码
  //!
  tx_sim::impl::CmdErrorCode LaunchModule(const std::string& name) override;

  //!
  //! @brief 函数名：TerminateModule
  //! @details 函数功能：终止模块
  //!
  //! @param[in] name 模块名称
  //! @return 命令执行错误码
  //!
  tx_sim::impl::CmdErrorCode TerminateModule(const std::string& name) override;

  //!
  //! @brief 函数名：AddModuleConfig
  //! @details 函数功能：添加模块配置
  //!
  //! @param[in] config_json_str 模块配置 JSON 字符串
  //! @return 命令执行错误码
  //!
  tx_sim::impl::CmdErrorCode AddModuleConfig(const std::string& config_json_str) override;

  //!
  //! @brief 函数名：RemoveModuleConfig
  //! @details 函数功能：删除模块配置
  //!
  //! @param[in] name 模块名称
  //! @return 命令执行错误码
  //!
  tx_sim::impl::CmdErrorCode RemoveModuleConfig(const std::string& name) override;

  //!
  //! @brief 函数名：UpdateModuleConfig
  //! @details 函数功能：更新模块配置
  //!
  //! @param[in] config_json_str 模块配置 JSON 字符串
  //! @return 命令执行错误码
  //!
  tx_sim::impl::CmdErrorCode UpdateModuleConfig(const std::string& config_json_str) override;

  //!
  //! @brief 函数名：GetModuleConfig
  //! @details 函数功能：获取模块配置
  //!
  //! @param[in] name 模块名称
  //! @param[out] config_json_str 模块配置 JSON 字符串
  //! @return 命令执行错误码
  //!
  tx_sim::impl::CmdErrorCode GetModuleConfig(const std::string& name, std::string& config_json_str) override;

  //!
  //! @brief 函数名：GetAllModuleConfigs
  //! @details 函数功能：获取所有模块配置
  //!
  //! @param[out] configs_json_str 所有模块配置 JSON 字符串列表
  //! @return 命令执行错误码
  //!
  tx_sim::impl::CmdErrorCode GetAllModuleConfigs(std::vector<std::string>& configs_json_str) override;

  //!
  //! @brief 函数名：SetSysConfigs
  //! @details 函数功能：设置系统配置
  //!
  //! @param[in] config_json_str 系统配置 JSON 字符串
  //! @return 命令执行错误码
  //!
  tx_sim::impl::CmdErrorCode SetSysConfigs(const std::string& config_json_str) override;

  //!
  //! @brief 函数名：GetSysConfigs
  //! @details 函数功能：获取系统配置
  //!
  //! @param[out] config_json_str 系统配置 JSON 字符串
  //! @return 命令执行错误码
  //!
  tx_sim::impl::CmdErrorCode GetSysConfigs(std::string& config_json_str) override;

  //!
  //! @brief 函数名：RestoreDefaultConfigs
  //! @details 函数功能：恢复默认配置
  //!
  //! @return 命令执行错误码
  //!
  tx_sim::impl::CmdErrorCode RestoreDefaultConfigs() override;

 private:
  std::unique_ptr<Client> client_;
};

class SimClientFactory final : public ISimClientFactory {
 public:
  std::shared_ptr<ISimClient> CreateSimClient(const std::string& endpoint) override;
};

}  // namespace cli
}  // namespace tx_sim
