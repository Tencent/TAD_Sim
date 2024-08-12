// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "sim_client.h"

namespace tx_sim {
namespace cli {
//!
//! @brief 构造函数
//! @details 构造模拟客户端对象
//!
//! @param[in] endpoint 端点
//!
SimClient::SimClient(const std::string& endpoint) {
  client_ = std::unique_ptr<Client>(new Client(endpoint));
}

//!
//! @brief 函数名：Setup
//! @details 函数功能：设置模拟客户端
//!
//! @param[in] cb 命令状态回调函数
//!
void SimClient::Setup(const tx_sim::coordinator::CommandStatusCb& cb) {
  client_->Setup("", cb);
}

//!
//! @brief 函数名：UnSetup
//! @details 函数功能：取消设置模拟客户端
//!
//! @param[in] cb 命令状态回调函数
//!
void SimClient::UnSetup(const tx_sim::coordinator::CommandStatusCb& cb) {
  client_->UnSetup(cb);
}

//!
//! @brief 函数名：Step
//! @details 函数功能：执行模拟客户端的一步
//!
//! @param[in] cb 命令状态回调函数
//!
void SimClient::Step(const tx_sim::coordinator::CommandStatusCb& cb) {
  client_->Step(cb);
}

//!
//! @brief 函数名：Stop
//! @details 函数功能：停止模拟客户端
//!
//! @param[in] cb 命令状态回调函数
//!
void SimClient::Stop(const tx_sim::coordinator::CommandStatusCb& cb) {
  client_->Stop(cb);
}

//!
//! @brief 函数名：Run
//! @details 函数功能：运行模拟客户端
//!
//! @param[in] cb 命令状态回调函数
//!
void SimClient::Run(const tx_sim::coordinator::CommandStatusCb& cb) {
  client_->Run(cb);
}

//!
//! @brief 函数名：Pause
//! @details 函数功能：暂停模拟客户端
//!
//! @param[in] cb 命令状态回调函数
//!
void SimClient::Pause(const tx_sim::coordinator::CommandStatusCb& cb) {
  client_->Pause(cb);
}

//!
//! @brief 函数名：LaunchModule
//! @details 函数功能：启动模块
//!
//! @param[in] name 模块名称
//! @return 命令执行错误码
//!
tx_sim::impl::CmdErrorCode SimClient::LaunchModule(const std::string& name) {
  return client_->LaunchModule(name);
}

//!
//! @brief 函数名：TerminateModule
//! @details 函数功能：终止模块
//!
//! @param[in] name 模块名称
//! @return 命令执行错误码
//!
tx_sim::impl::CmdErrorCode SimClient::TerminateModule(const std::string& name) {
  return client_->TerminateModule(name);
}

//!
//! @brief 函数名：AddModuleConfig
//! @details 函数功能：添加模块配置
//!
//! @param[in] config_json_str 模块配置 JSON 字符串
//! @return 命令执行错误码
//!
tx_sim::impl::CmdErrorCode SimClient::AddModuleConfig(const std::string& config_json_str) {
  return client_->AddModuleConfig(config_json_str);
}

//!
//! @brief 函数名：RemoveModuleConfig
//! @details 函数功能：删除模块配置
//!
//! @param[in] name 模块名称
//! @return 命令执行错误码
//!
tx_sim::impl::CmdErrorCode SimClient::RemoveModuleConfig(const std::string& name) {
  return client_->RemoveModuleConfig(name);
}

//!
//! @brief 函数名：UpdateModuleConfig
//! @details 函数功能：更新模块配置
//!
//! @param[in] config_json_str 模块配置 JSON 字符串
//! @return 命令执行错误码
//!
tx_sim::impl::CmdErrorCode SimClient::UpdateModuleConfig(const std::string& config_json_str) {
  return client_->UpdateModuleConfig(config_json_str);
}

//!
//! @brief 函数名：GetModuleConfig
//! @details 函数功能：获取模块配置
//!
//! @param[in] name 模块名称
//! @param[out] config_json_str 模块配置 JSON 字符串
//! @return 命令执行错误码
//!
tx_sim::impl::CmdErrorCode SimClient::GetModuleConfig(const std::string& name, std::string& config_json_str) {
  return client_->GetModuleConfig(name, config_json_str);
}

//!
//! @brief 函数名：GetAllModuleConfigs
//! @details 函数功能：获取所有模块配置
//!
//! @param[out] configs_json_str 所有模块配置 JSON 字符串列表
//! @return 命令执行错误码
//!
tx_sim::impl::CmdErrorCode SimClient::GetAllModuleConfigs(std::vector<std::string>& configs_json_str) {
  return client_->GetAllModuleConfigs(configs_json_str);
}

//!
//! @brief 函数名：SetSysConfigs
//! @details 函数功能：设置系统配置
//!
//! @param[in] config_json_str 系统配置 JSON 字符串
//! @return 命令执行错误码
//!
tx_sim::impl::CmdErrorCode SimClient::SetSysConfigs(const std::string& config_json_str) {
  return client_->SetSysConfigs(config_json_str);
}

//!
//! @brief 函数名：GetSysConfigs
//! @details 函数功能：获取系统配置
//!
//! @param[out] config_json_str 系统配置 JSON 字符串
//! @return 命令执行错误码
//!
tx_sim::impl::CmdErrorCode SimClient::GetSysConfigs(std::string& config_json_str) {
  return client_->GetSysConfigs(config_json_str);
}

//!
//! @brief 函数名：RestoreDefaultConfigs
//! @details 函数功能：恢复默认配置
//!
//! @return 命令执行错误码
//!
tx_sim::impl::CmdErrorCode SimClient::RestoreDefaultConfigs() {
  return client_->RestoreDefaultConfigs();
}

std::shared_ptr<ISimClient> SimClientFactory::CreateSimClient(const std::string& endpoint) {
  return std::make_shared<SimClient>(endpoint);
}

}  // namespace cli
}  // namespace tx_sim
