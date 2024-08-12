// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "module_mgr.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <utility>

#include "boost/algorithm/string/join.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/filesystem.hpp"
#include "glog/logging.h"

#include "utils/id_impl.h"
#include "utils/json_helper.h"
#include "utils/os.h"
#include "utils/time.h"

#ifdef __linux__
#  define TXSIM_DEFAULT_PATH_MODULE_LAUNCHER "txsim-module-launcher"
#elif defined _WIN32
#  define TXSIM_DEFAULT_PATH_MODULE_LAUNCHER "txsim-module-launcher.exe"
#endif  // __linux__
#define TXSIM_JSON_KEY_MODULE_LAUNCHER_PATH "moduleLauncherPath"
#define TXSIM_JSON_KEY_MODULE_LOG_DIR       "moduleLogDirectory"

using namespace tx_sim::impl;
using namespace tx_sim::utils;

namespace tx_sim {
namespace coordinator {

//! @brief 构造函数：ModuleManagerImpl
//! @details 构造函数功能：初始化ModuleManagerImpl对象
//!
//! @param[in] registry 一个包含模块注册信息的ModuleRegistry对象
//! @param[in] factory 一个包含模块连接工厂信息的ModuleConnectionFactory对象
//! @param[in] proc_serv 一个包含模块进程服务信息的ModuleProcessService对象
//! @param[in] config_file_path 一个包含配置文件路径的字符串
ModuleManagerImpl::ModuleManagerImpl(std::shared_ptr<ModuleRegistry> registry,
                                     std::shared_ptr<ModuleConnectionFactory> factory,
                                     std::shared_ptr<tx_sim::utils::ModuleProcessService> proc_serv,
                                     const std::string& config_file_path)
    : registry_(registry), conn_factory_(factory), proc_service_(proc_serv) {
  if (boost::filesystem::exists(config_file_path)) {
    LOG(INFO) << "module manager reading config file " << config_file_path << " ...";
    Json::Value cfg_doc;
    ReadJsonDocFromFile(cfg_doc, config_file_path);
    module_launcher_path_ =
        cfg_doc.get(TXSIM_JSON_KEY_MODULE_LAUNCHER_PATH, TXSIM_DEFAULT_PATH_MODULE_LAUNCHER).asString();
    LOG(INFO) << "using module launcher path: " << module_launcher_path_;
    module_log_directory_ =
        cfg_doc.get(TXSIM_JSON_KEY_MODULE_LOG_DIR, "").asString();  // default in current working directory.
    LOG(INFO) << "using module log directory: " << module_log_directory_;
  } else {
    module_launcher_path_ = TXSIM_DEFAULT_PATH_MODULE_LAUNCHER;
    LOG(INFO) << "module manager using default configuration.";
  }
  // validating the path configurations.
  if (!module_log_directory_.empty()) {
    if (!boost::filesystem::is_directory(module_log_directory_) &&
        !boost::filesystem::create_directories(module_log_directory_))
      throw std::runtime_error("the module log direcotry specified cannot be created.");
  }
  if (!boost::filesystem::is_regular_file(module_launcher_path_))
    throw std::invalid_argument("invalid module launcher path specified.");
}

//! @brief 析构函数：ModuleManagerImpl
//! @details 析构函数功能：销毁ModuleManagerImpl对象
ModuleManagerImpl::~ModuleManagerImpl() {
  LOG(INFO) << "ModuleManagerImpl destructed.";
}
//! @brief 函数名：UpdateConfigs
//! @details 函数功能：更新模块配置信息
//!
//! @param[in] configs 一个包含模块配置信息的CoordinatorConfig对象
void ModuleManagerImpl::UpdateConfigs(const CoordinatorConfig& configs) {
  override_module_log_ = configs.override_user_log;
  std::set<std::string> active_modules;
  std::vector<ModuleRegisterSetup> registry_setup_list;
  for (const ModuleConfig& new_config : configs.module_configs) {
    const std::string& name = new_config.name;
    active_modules.insert(name);
    auto it = module_status_.find(name);
    if (it == module_status_.end()) {  // new module added.
      const std::string uuid = AddModuleConfig(new_config);
      registry_setup_list.emplace_back(new_config.name, uuid, new_config.conn_args.step_timeout);
      LOG(INFO) << "added new module status: " << name << "(" << uuid << ").";
      continue;
    }
    // has a previously set module configuration.
    auto&& old_status = it->second;
    if (CheckModuleReusable(old_status, new_config)) {  // current module setup could be reused.
      old_status.event_fired = false;
      old_status.config = new_config;  // update config anyway.
      LOG(INFO) << "reuse module status: " << name << "(" << old_status.uuid << ").";
      continue;
    } else {  // current module setup could not be reused. terminate the current one and update the new configuration.
      if (old_status.config.auto_launch) Terminate(name);
      registry_->RemoveRegisteredModule(old_status.config.name);  // we need to connect to a new one.
      auto t1 = old_status.last_launch_mtime,
           t2 = old_status.last_check_mtime;  // remember the mtimes of the file for now.
      module_status_.erase(it);
      const std::string uuid = AddModuleConfig(new_config);
      auto& s = module_status_[new_config.name];
      s.last_launch_mtime = t1, s.last_check_mtime = t2;  // restore the mtimes of the file.
      registry_setup_list.emplace_back(new_config.name, uuid, new_config.conn_args.step_timeout);
      LOG(INFO) << "module status updated: " << name << "(" << uuid << ").";
    }
  }
  TerminateInactiveModules(active_modules);
  registry_->SetupActiveModuleContext(registry_setup_list);
}

//! @brief 函数名：MonitorInitWorkflow
//! @details 函数功能：监控模块初始化工作流程
//!
//! @param[in] events 一个包含模块初始化状态的ModuleInitStatus对象向量
//! @param[in] cmd_st 一个包含模块命令状态的ModuleCmdStatus对象向量
//!
//! @return 返回一个size_t类型的值，表示模块初始化工作流程的状态
size_t ModuleManagerImpl::MonitorInitWorkflow(std::vector<ModuleInitStatus>& events,
                                              std::vector<ModuleCmdStatus>& cmd_st) {
  events.clear();
  cmd_st.clear();
  size_t finished_count = 0;
  for (auto&& module : module_status_) {
    auto&& status = module.second;
    switch (status.init_status.state) {
      case kModuleInitStart: {
        if (status.config.auto_launch && registry_->GetChildProcessHandle(status.config.name) == txsim_invalid_pid_t) {
          if (status.last_launch_mtime != 0 && status.last_check_mtime != 0 &&
              status.last_launch_mtime != status.last_check_mtime) {
            events.emplace_back(status.config.name, kModuleInitStart);
            events.back().binary_updated = true;
          }
          status.init_status.state = LaunchModule(status.config, status.init_status.log_file, cmd_st);
        } else {
          status.init_status.state = kModuleInitConnecting;
        }
        break;
      }
      case kModuleInitConnecting: {  // module has benn launched or it is launched manually. trying to connect it.
        std::string registered_addr, process_exit_info;
        if (registry_->GetModuleRegisterStatus(status.config.name, process_exit_info, &registered_addr)) {
          LOG(INFO) << "got registered module " << status.config.name << " address: " << registered_addr;
          if (registered_addr != status.addr) status.conn = nullptr;
          status.addr = registered_addr;
          status.init_status.state = TryConnectingModule(status, cmd_st);
        } else {
          CheckModuleExitStatus(process_exit_info, status, cmd_st);
        }
        break;
      }
      case kModuleInitConnected: {  // module registered successfully. sending Init request.
        events.push_back(status.init_status);
        LOG(INFO) << "module:" << status.config.name;
        ModuleInitRequest req;
        for (const auto& kv : status.config.init_args) {
          req.init_args[kv.first] = kv.second;
          LOG(INFO) << "init args:";
          LOG(INFO) << kv.first << ":(" << kv.second << ").";
        }
        req.group = status.config.module_group_name;
        status.init_status.state = status.conn->Send(req) ? kModuleInitReqSent : kModuleInitFailed;
        break;
      }
      case kModuleInitReqSent: {  // the Init request has sent, waiting response.
        ModuleInitResponse resp;
        if (!status.conn->Receive(resp)) {
          // try to update the module registered info.
          std::string registered_addr, process_exit_info;
          registry_->GetModuleRegisterStatus(status.config.name, process_exit_info, &registered_addr);
          if (registered_addr != status.addr) status.init_status.state = kModuleInitConnecting;
          CheckModuleExitStatus(process_exit_info, status, cmd_st);
          break;
        }
        status.init_status.state = (resp.type == kModuleOK ? kModuleInitSucceed : kModuleInitFailed);
        if (resp.type != kModuleOK) {
          cmd_st.emplace_back(status.config.name, resp.type, resp.err);
          LOG(ERROR) << "module " << status.config.name << " init failed: [" << Enum2String(resp.type) << "] "
                     << resp.err;
        }
        status.init_status.topic_info.SetTopics(resp.sub_topics, resp.pub_topics, resp.sub_shmems, resp.pub_shmems,
                                                resp.pub_topics_broadcast, resp.pub_shmems_broadcast);
      }
      case kModuleInitSucceed:  // module Init callback succeed and response returned.
      case kModuleInitFailed:   // launch failed / tcp socket creation failed / module Init callback failed.
      default: {                // workflow finished.
        if (!status.event_fired) {
          events.push_back(status.init_status);
          status.event_fired = true;
        }
        ++finished_count;
        break;
      }
    }
  }  // end for loop
  return finished_count;
}

//! @brief 函数名：GetConnection
//! @details 函数功能：获取指定名称的模块连接
//!
//! @param[in] name 一个包含模块名称的字符串
//!
//! @return 返回一个指向ModuleConnection对象的智能指针，表示模块连接
std::shared_ptr<ModuleConnection> ModuleManagerImpl::GetConnection(const std::string& name) {
  const auto it = module_status_.find(name);
  return it == module_status_.cend() ? nullptr : it->second.conn;
}

//! @brief 函数名：CheckModuleStatus
//! @details 函数功能：检查指定名称的模块状态
//!
//! @param[in] name 一个包含模块名称的字符串
//! @param[in] exit_info 一个包含模块退出信息的字符串
//!
//! @return 返回一个布尔值，表示模块状态是否正常
bool ModuleManagerImpl::CheckModuleStatus(const std::string& name, std::string& exit_info) {
  if (registry_->GetModuleRegisterStatus(name, exit_info)) return true;
  LOG(WARNING) << "module process " << name << (exit_info.empty() ? " expired." : (" exited: " + exit_info));
  return false;
}

//! @brief 函数名：CheckModuleBinaryUpdated
//! @details 函数功能：检查指定配置的模块二进制文件是否更新
//!
//! @param[in] config 一个包含模块配置信息的ModuleConfig对象
//!
//! @return 返回一个布尔值，表示模块二进制文件是否更新
bool ModuleManagerImpl::CheckModuleBinaryUpdated(const ModuleConfig& config) {
  boost::filesystem::path module_path(config.so_path.empty() ? config.bin_path : config.so_path);
  std::time_t lwt = boost::filesystem::last_write_time(module_path);
  auto it = module_status_.find(config.name);
  assert(it != module_status_.end());
  it->second.last_check_mtime = lwt;
  if (lwt != it->second.last_launch_mtime) {
    LOG(INFO) << "module " << config.name << "(" << module_path << ") file updated, modified time: last -> "
              << GetDateTimeStr(it->second.last_launch_mtime, kDateTimeLogStrFormat) << " current -> "
              << GetDateTimeStr(lwt, kDateTimeLogStrFormat);
    return true;
  }
  return false;
}

//! @brief 函数名：Terminate
//! @details 函数功能：终止指定名称的模块
//!
//! @param[in] name 一个包含模块名称的字符串
void ModuleManagerImpl::Terminate(const std::string& name) {
  txsim_pid_t pid = registry_->GetChildProcessHandle(name);
  if (pid == txsim_invalid_pid_t) {
    LOG(WARNING) << "module process " << name << " is no longer exists.";
    return;
  }
  LOG(INFO) << "terminating module " << name << "(" << pid << ") ...";
  auto ms_it = module_status_.find(name);
  assert(ms_it != module_status_.end());
  std::ostringstream log_module_ss;
  log_module_ss << "previously launched module " << name << "(" << ms_it->second.uuid << ") ";
  try {
    proc_service_->TerminateModule(pid);
    LOG(INFO) << log_module_ss.str() << "is terminated.";
  } catch (const std::exception& e) { LOG(ERROR) << log_module_ss.str() << "could not terminate: " << e.what(); }
  // ArchiveModuleLogFile(name);
}

//! @brief 函数名：RemoveRegistryModule
//! @details 函数功能：移除指定名称的模块注册信息
//!
//! @param[in] moduleName 一个包含模块名称的字符串
void ModuleManagerImpl::RemoveRegistryModule(const std::string moduleName) {
  registry_->RemoveRegisteredModule(moduleName);
}

//! @brief 函数名：TerminateInactiveModules
//! @details 函数功能：终止不活跃的模块
//!
//! @param[in] current_active_modules 一个包含当前活跃模块名称的字符串集合
void ModuleManagerImpl::TerminateInactiveModules(const std::set<std::string>& current_active_modules) {
  for (auto it = module_status_.begin(); it != module_status_.end();) {
    if (current_active_modules.find(it->first) == current_active_modules.end()) {  // inactive module. removing it.
      auto&& old_status = it->second;
      if (old_status.config.auto_launch) Terminate(old_status.config.name);
      registry_->RemoveRegisteredModule(old_status.config.name);
      it = module_status_.erase(it);
    } else {
      ++it;
    }
  }
}

//! @brief 函数名：AddSimEnvVars
//! @details 函数功能：添加模拟环境变量
//!
//! @param[in] config 一个包含模块配置信息的ModuleConfig对象
//! @param[in] ret 一个包含字符串对的向量，用于存储模拟环境变量
void ModuleManagerImpl::AddSimEnvVars(const ModuleConfig& config, std::vector<StringPair>& ret) {
  ret.emplace_back("LD_LIBRARY_PATH", boost::algorithm::join(config.dep_paths, ":"));
  auto it = module_status_.find(config.name);
  if (it == module_status_.end() || it->second.uuid.empty()) return;
  ret.emplace_back(kChildModuleEnvUuid, it->second.uuid);
}

// a reusable configuraion is satisfied when(logical AND):
// 1. init_args or auto_launch is the same as the new_config's;
// 2. if auto_launch is true, any launch args(dep_path/so_path/bin_path/bin_args) should be the same;
// 3. the previous one's ModuleStatus.init_status.state != kModuleInitFailed;
// 4. step_timeout non-0(since 0 means no heartbeat, we don't know if this module is still alive);
// 5. the current one is still registered.
bool ModuleManagerImpl::CheckModuleReusable(ModuleStatus& old_status, const ModuleConfig& new_config) {
  const ModuleConfig& old_config = old_status.config;

  if (old_config.init_args != new_config.init_args || old_config.auto_launch != new_config.auto_launch) return false;

  if (new_config.auto_launch) {  // it means the old one is also auto-launched.
    if (old_config.dep_paths != new_config.dep_paths || old_config.so_path != new_config.so_path) return false;
    if (new_config.so_path.empty() &&  // prefer to the shared library. comparing binary only when so is missing.
        (old_config.bin_path != new_config.bin_path || old_config.bin_args != new_config.bin_args))
      return false;
  }

  if (old_status.init_status.state == kModuleInitFailed) return false;

  if (old_config.conn_args.step_timeout != new_config.conn_args.step_timeout &&
      (old_config.conn_args.step_timeout == 0 || new_config.conn_args.step_timeout == 0))
    return false;

  if (new_config.auto_launch && registry_->GetChildProcessHandle(new_config.name) == txsim_invalid_pid_t) return false;
  if (new_config.auto_launch && CheckModuleBinaryUpdated(new_config)) return false;

  // module config is resuable. clear last init status.
  old_status.init_status.state = kModuleInitStart;
  old_status.init_status.topic_info.Clear();
  old_status.event_fired = false;

  // if connection config updates, reset the current connection socket.
  if (old_config.conn_args != new_config.conn_args) old_status.conn.reset();

  return true;
}

//! @brief 函数名：AddModuleConfig
//! @details 函数功能：添加模块配置信息
//!
//! @param[in] new_config 一个包含新模块配置信息的ModuleConfig对象
//!
//! @return 返回一个字符串，表示添加的模块配置信息
const std::string ModuleManagerImpl::AddModuleConfig(const ModuleConfig& new_config) {
  auto&& status = module_status_[new_config.name];
  status.config = new_config;  // copy needed.
  status.init_status.name = new_config.name;
  status.init_status.state = kModuleInitStart;
  status.uuid = new_config.auto_launch ? GenerateRandomUUID() : "";
  return status.uuid;
}

//! @brief 函数名：LaunchModule
//! @details 函数功能：启动指定配置的模块
//!
//! @param[in] config 一个包含模块配置信息的ModuleConfig对象
//! @param[in] log_file_name 一个包含模块日志文件名的字符串
//! @param[in] cmd_st 一个包含模块命令状态的ModuleCmdStatus对象向量
//!
//! @return 返回一个tx_sim::impl::ModuleInitState对象，表示模块初始化状态
ModuleInitState ModuleManagerImpl::LaunchModule(const ModuleConfig& config, std::string& log_file_name,
                                                std::vector<ModuleCmdStatus>& cmd_st) {
  txsim_pid_t pid = txsim_invalid_pid_t;
  std::string log_path_str(GetModuleLogFilePath(config.name));
  std::vector<StringPair> added_envs;
  AddSimEnvVars(config, added_envs);
  bool launching_shared_lib = !config.so_path.empty();  // prefer to using module launcher.
  const std::string local_registry_addr = registry_->GetLocalRegistryAddr();
  auto args = launching_shared_lib ? std::vector<std::string>{config.name, config.so_path, local_registry_addr}
                                   : config.bin_args;
  if (!launching_shared_lib) {
    args.push_back("--ip_addr_port");
    args.push_back(local_registry_addr);
  }
  const std::string& module_path = launching_shared_lib ? config.so_path : config.bin_path;
  const std::string& launching_path = launching_shared_lib ? module_launcher_path_ : config.bin_path;
  if (!boost::filesystem::is_regular_file(module_path)) {
    std::ostringstream err_ss;
    err_ss << "the specified path of the " << (launching_shared_lib ? "shared library" : "executable")
           << " does not exists: " << module_path;
    std::string err_str(err_ss.str());
    cmd_st.emplace_back(config.name, kModuleError, err_str);
    LOG(ERROR) << "process " << config.name << " launching failed: " << err_str;
    return kModuleInitFailed;
  }
  try {
    pid = proc_service_->CreateModule(launching_path, args, added_envs, log_path_str);
  } catch (const std::exception& e) {
    LOG(ERROR) << "launching module process " << config.name << " failed: " << e.what();
    cmd_st.emplace_back(config.name, kModuleError, std::string("process launching failed: ") + e.what());
    return kModuleInitFailed;
  }
  log_file_name = boost::filesystem::path(log_path_str).filename().string();
  LOG(INFO) << "redirecting stdout/stderr of process " << config.name << " to " << log_path_str;
  registry_->AddChildProcessHandle(config.name, pid);
  LOG(INFO) << "launching module process " << config.name << " succeed.";
  module_status_[config.name].last_launch_mtime = boost::filesystem::last_write_time(module_path);
  return kModuleInitConnecting;
}

//! @brief 函数名：TryConnectingModule
//! @details 函数功能：尝试连接指定状态的模块
//!
//! @param[in] status 一个包含模块状态的ModuleStatus对象
//! @param[in] cmd_st 一个包含模块命令状态的ModuleCmdStatus对象向量
//!
//! @return 返回一个tx_sim::impl::ModuleInitState对象，表示模块初始化状态
ModuleInitState ModuleManagerImpl::TryConnectingModule(ModuleStatus& status, std::vector<ModuleCmdStatus>& cmd_st) {
  if (status.conn && !status.conn->WaitingReply()) return kModuleInitConnected;
  try {
    status.conn = std::move(conn_factory_->CreateConnection(status.addr, status.config.conn_args));
    return kModuleInitConnected;
  } catch (const zmq::error_t& e) {
    LOG(ERROR) << "zmq socket(" << status.addr << ") creation error: [" << e.num() << "] " << e.what();
    cmd_st.emplace_back(status.config.name, kModuleError, "connection creation error. endpoint: " + status.addr);
    return kModuleInitFailed;
  }
}

//! @brief 函数名：GetModuleLogFilePath
//! @details 函数功能：获取指定名称的模块日志文件路径
//!
//! @param[in] name 一个包含模块名称的字符串
//!
//! @return 返回一个字符串，表示模块日志文件路径
std::string ModuleManagerImpl::GetModuleLogFilePath(const std::string& name) {
  boost::filesystem::path log_path = module_log_directory_;
  std::ostringstream log_file_name(name, std::ios_base::ate);
  if (!override_module_log_) log_file_name << "." << GetCurrentDateTimeStr();
  log_file_name << ".log";
  log_path /= log_file_name.str();
  return log_path.lexically_normal().string();
}

//! @brief 函数名：GetModuleLogArchivePath
//! @details 函数功能：获取指定名称的模块日志归档路径
//!
//! @param[in] name 一个包含模块名称的字符串
//!
//! @return 返回一个字符串，表示模块日志归档路径
std::string ModuleManagerImpl::GetModuleLogArchivePath(const std::string& name) {
  boost::filesystem::path log_path = module_log_directory_;
  log_path /= (name + "_" + tx_sim::utils::GetCurrentDateTimeStr() + ".log");
  return log_path.lexically_normal().string();
}

//! @brief 函数名：ArchiveModuleLogFile
//! @details 函数功能：归档指定名称的模块日志文件
//!
//! @param[in] name 一个包含模块名称的字符串
void ModuleManagerImpl::ArchiveModuleLogFile(const std::string& name) {
  std::string orig_path(GetModuleLogFilePath(name)), new_path(GetModuleLogArchivePath(name));
  if (boost::filesystem::exists(orig_path)) {
    if (!std::rename(orig_path.c_str(), new_path.c_str())) {
      LOG(INFO) << "archived module " << name << " log file to " << new_path;
    } else {
      LOG(ERROR) << "failed to archive module " << name << " log file to " << new_path;
    }
  } else {
    LOG(WARNING) << "no module log file found: " << orig_path;
  }
}

//! @brief 函数名：CheckModuleExitStatus
//! @details 函数功能：检查指定名称的模块退出状态
//!
//! @param[in] exit_info 一个包含模块退出信息的字符串
//! @param[in] status 一个包含模块状态的ModuleStatus对象
//! @param[in] cmd_st 一个包含模块命令状态的ModuleCmdStatus对象向量
void ModuleManagerImpl::CheckModuleExitStatus(const std::string& exit_info, ModuleStatus& status,
                                              std::vector<ModuleCmdStatus>& cmd_st) {
  if (!exit_info.empty()) {
    status.init_status.state = kModuleInitFailed;
    cmd_st.emplace_back(status.config.name, kModuleError, std::string("process exited: ") + exit_info);
    LOG(ERROR) << "module process " << status.config.name << " has exit after launching: " << exit_info;
  }
}

}  // namespace coordinator
}  // namespace tx_sim
