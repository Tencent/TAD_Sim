// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <ctime>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "config.h"
#include "connection.h"
#include "registry.h"
#include "status.h"
#include "utils/defs.h"
#include "utils/proc_service.h"

namespace tx_sim {
namespace coordinator {

//! @brief 类：ModuleManager
//! @details 类功能：声明一个ModuleManager类，用于管理模块
class ModuleManager {
 public:
  //! @brief 函数名：UpdateConfigs
  //! @details 函数功能：更新模块配置信息
  //!
  //! @param[in] configs 一个包含模块配置信息的CoordinatorConfig对象
  virtual void UpdateConfigs(const CoordinatorConfig& configs) = 0;

  //! @brief 函数名：MonitorInitWorkflow
  //! @details 函数功能：监控模块初始化工作流程
  //!
  //! @param[in] events 一个包含模块初始化状态的ModuleInitStatus对象向量
  //! @param[in] cmd_st 一个包含模块命令状态的ModuleCmdStatus对象向量
  //!
  //! @return 返回一个size_t类型的值，表示模块初始化工作流程的状态
  virtual size_t MonitorInitWorkflow(std::vector<ModuleInitStatus>& events, std::vector<ModuleCmdStatus>& cmd_st) = 0;

  //! @brief 函数名：GetConnection
  //! @details 函数功能：获取指定名称的模块连接
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //!
  //! @return 返回一个指向ModuleConnection对象的智能指针，表示模块连接
  //!
  //! @note returns a validated(heartbeat/init succeed) module connection or nullptr otherwise.
  virtual std::shared_ptr<ModuleConnection> GetConnection(const std::string& name) = 0;

  //! @brief 函数名：CheckModuleStatus
  //! @details 函数功能：检查指定名称的模块状态
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //! @param[in] exit_info 一个包含模块退出信息的字符串
  //!
  //! @return 返回一个布尔值，表示模块状态是否正常
  //!
  //! @note  returns false if the module process has exited, with a string description for exit status, or connection
  //! lost. this function should be called only after the module has registered(i.e. MonitorInitWorkflow succeed).
  virtual bool CheckModuleStatus(const std::string& name, std::string& exit_info) = 0;

  //! @brief 函数名：RemoveRegistryModule
  //! @details 函数功能：移除指定名称的模块注册信息
  //!
  //! @param[in] moduleName 一个包含模块名称的字符串
  virtual void RemoveRegistryModule(const std::string moduleName) = 0;
};

//! @brief 类：ModuleManagerImpl
//! @details 类功能：声明一个ModuleManagerImpl类，继承自ModuleManager类，用于管理模块
class ModuleManagerImpl final : public ModuleManager {
 public:
  //! @brief 构造函数：ModuleManagerImpl
  //! @details 构造函数功能：初始化ModuleManagerImpl对象
  //!
  //! @param[in] registry 一个包含模块注册信息的ModuleRegistry对象
  //! @param[in] factory 一个包含模块连接工厂信息的ModuleConnectionFactory对象
  //! @param[in] proc_serv 一个包含模块进程服务信息的ModuleProcessService对象
  //! @param[in] config_file_path 一个包含配置文件路径的字符串
  ModuleManagerImpl(std::shared_ptr<ModuleRegistry> registry, std::shared_ptr<ModuleConnectionFactory> factory,
                    std::shared_ptr<tx_sim::utils::ModuleProcessService> proc_serv,
                    const std::string& config_file_path);

  //! @brief 析构函数：ModuleManagerImpl
  //! @details 析构函数功能：销毁ModuleManagerImpl对象
  ~ModuleManagerImpl();

  //! @brief 宏：TXSIM_DISABLE_COPY_MOVE
  //! @details 宏功能：禁用ModuleManagerImpl类的拷贝和移动构造函数
  TXSIM_DISABLE_COPY_MOVE(ModuleManagerImpl)

  //! @brief 函数名：UpdateConfigs
  //! @details 函数功能：更新模块配置信息
  //!
  //! @param[in] configs 一个包含模块配置信息的CoordinatorConfig对象
  void UpdateConfigs(const CoordinatorConfig& configs) override;

  //! @brief 函数名：MonitorInitWorkflow
  //! @details 函数功能：监控模块初始化工作流程
  //!
  //! @param[in] events 一个包含模块初始化状态的ModuleInitStatus对象向量
  //! @param[in] cmd_st 一个包含模块命令状态的ModuleCmdStatus对象向量
  //!
  //! @return 返回一个size_t类型的值，表示模块初始化工作流程的状态
  size_t MonitorInitWorkflow(std::vector<ModuleInitStatus>& events, std::vector<ModuleCmdStatus>& cmd_st) override;

  //! @brief 函数名：GetConnection
  //! @details 函数功能：获取指定名称的模块连接
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //!
  //! @return 返回一个指向ModuleConnection对象的智能指针，表示模块连接
  std::shared_ptr<ModuleConnection> GetConnection(const std::string& name) override;

  //! @brief 函数名：CheckModuleStatus
  //! @details 函数功能：检查指定名称的模块状态
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //! @param[in] exit_info 一个包含模块退出信息的字符串
  //!
  //! @return 返回一个布尔值，表示模块状态是否正常
  bool CheckModuleStatus(const std::string& name, std::string& exit_info);

  //! @brief 函数名：RemoveRegistryModule
  //! @details 函数功能：移除指定名称的模块注册信息
  //!
  //! @param[in] moduleName 一个包含模块名称的字符串
  void RemoveRegistryModule(const std::string moduleName) override;

 private:
  //! @brief 结构体：ModuleStatus
  //! @details 结构体功能：声明一个ModuleStatus结构体，用于存储模块状态信息
  struct ModuleStatus {
    bool event_fired{false};
    ModuleInitStatus init_status;
    ModuleConfig config;
    std::string addr;
    std::shared_ptr<ModuleConnection> conn;
    std::string uuid;
    std::time_t last_launch_mtime{0}, last_check_mtime{0};
  };

  //! @brief 函数名：CheckModuleBinaryUpdated
  //! @details 函数功能：检查指定配置的模块二进制文件是否更新
  //!
  //! @param[in] config 一个包含模块配置信息的ModuleConfig对象
  //!
  //! @return 返回一个布尔值，表示模块二进制文件是否更新
  bool CheckModuleBinaryUpdated(const ModuleConfig& config);

  //! @brief 函数名：Terminate
  //! @details 函数功能：终止指定名称的模块
  //!
  //! @param[in] name 一个包含模块名称的字符串
  void Terminate(const std::string& name);

  //! @brief 函数名：TerminateInactiveModules
  //! @details 函数功能：终止不活跃的模块
  //!
  //! @param[in] current_active_modules 一个包含当前活跃模块名称的字符串集合
  void TerminateInactiveModules(const std::set<std::string>& current_active_modules);

  //! @brief 函数名：AddSimEnvVars
  //! @details 函数功能：添加模拟环境变量
  //!
  //! @param[in] config 一个包含模块配置信息的ModuleConfig对象
  //! @param[in] ret 一个包含字符串对的向量，用于存储模拟环境变量
  void AddSimEnvVars(const ModuleConfig& config, std::vector<tx_sim::impl::StringPair>& ret);

  //! @brief 函数名：CheckModuleReusable
  //! @details 函数功能：检查指定名称的模块是否可重用
  //!
  //! @param[in] old_status 一个包含旧模块状态的ModuleStatus对象
  //! @param[in] new_config 一个包含新模块配置信息的ModuleConfig对象
  //!
  //! @return 返回一个布尔值，表示模块是否可重用
  bool CheckModuleReusable(ModuleStatus& old_status, const ModuleConfig& new_config);

  //! @brief 函数名：AddModuleConfig
  //! @details 函数功能：添加模块配置信息
  //!
  //! @param[in] new_config 一个包含新模块配置信息的ModuleConfig对象
  //!
  //! @return 返回一个字符串，表示添加的模块配置信息
  const std::string AddModuleConfig(const ModuleConfig& new_config);

  //! @brief 函数名：LaunchModule
  //! @details 函数功能：启动指定配置的模块
  //!
  //! @param[in] config 一个包含模块配置信息的ModuleConfig对象
  //! @param[in] log_file_name 一个包含模块日志文件名的字符串
  //! @param[in] cmd_st 一个包含模块命令状态的ModuleCmdStatus对象向量
  //!
  //! @return 返回一个tx_sim::impl::ModuleInitState对象，表示模块初始化状态
  tx_sim::impl::ModuleInitState LaunchModule(const ModuleConfig& config, std::string& log_file_name,
                                             std::vector<ModuleCmdStatus>& cmd_st);

  //! @brief 函数名：TryConnectingModule
  //! @details 函数功能：尝试连接指定状态的模块
  //!
  //! @param[in] status 一个包含模块状态的ModuleStatus对象
  //! @param[in] cmd_st 一个包含模块命令状态的ModuleCmdStatus对象向量
  //!
  //! @return 返回一个tx_sim::impl::ModuleInitState对象，表示模块初始化状态
  tx_sim::impl::ModuleInitState TryConnectingModule(ModuleStatus& status, std::vector<ModuleCmdStatus>& cmd_st);

  //! @brief 函数名：GetModuleLogFilePath
  //! @details 函数功能：获取指定名称的模块日志文件路径
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //!
  //! @return 返回一个字符串，表示模块日志文件路径
  std::string GetModuleLogFilePath(const std::string& name);

  //! @brief 函数名：GetModuleLogArchivePath
  //! @details 函数功能：获取指定名称的模块日志归档路径
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //!
  //! @return 返回一个字符串，表示模块日志归档路径
  std::string GetModuleLogArchivePath(const std::string& name);

  //! @brief 函数名：ArchiveModuleLogFile
  //! @details 函数功能：归档指定名称的模块日志文件
  //!
  //! @param[in] name 一个包含模块名称的字符串
  void ArchiveModuleLogFile(const std::string& name);

  //! @brief 函数名：CheckModuleExitStatus
  //! @details 函数功能：检查指定名称的模块退出状态
  //!
  //! @param[in] exit_info 一个包含模块退出信息的字符串
  //! @param[in] status 一个包含模块状态的ModuleStatus对象
  //! @param[in] cmd_st 一个包含模块命令状态的ModuleCmdStatus对象向量
  void CheckModuleExitStatus(const std::string& exit_info, ModuleStatus& status, std::vector<ModuleCmdStatus>& cmd_st);

  //! @brief 成员变量：module_status_
  //! @details 成员变量功能：存储模块状态信息
  std::map<std::string, ModuleStatus> module_status_;

  //! @brief 成员变量：registry_
  //! @details 成员变量功能：存储模块注册信息
  std::shared_ptr<ModuleRegistry> registry_;

  //! @brief 成员变量：conn_factory_
  //! @details 成员变量功能：存储模块连接工厂信息
  std::shared_ptr<ModuleConnectionFactory> conn_factory_;

  //! @brief 成员变量：proc_service_
  //! @details 成员变量功能：存储模块进程服务信息
  std::shared_ptr<tx_sim::utils::ModuleProcessService> proc_service_;

  //! @brief 成员变量：module_launcher_path_
  //! @details 成员变量功能：存储模块启动器路径
  std::string module_launcher_path_;

  //! @brief 成员变量：module_log_directory_
  //! @details 成员变量功能：存储模块日志目录路径
  std::string module_log_directory_;

  //! @brief 成员变量：override_module_log_
  //! @details 成员变量功能：存储是否覆盖模块日志的标志
  bool override_module_log_{tx_sim::impl::kDefaultOverrideUserLog};
};

}  // namespace coordinator
}  // namespace tx_sim
