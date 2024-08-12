// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "coordinator/coordinator.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "config_mgr.h"
#include "glog/logging.h"
#include "json/json.h"

#include "txsim_messenger.h"
#include "union.pb.h"
#include "utils/json_helper.h"
#include "utils/proc.h"
#include "utils/time.h"
#include "vehicle_interaction.pb.h"

using namespace std::chrono;

using namespace tx_sim::impl;
using namespace tx_sim::utils;

#define TXSIM_TADSIM_ENV_KEY "TADSIM"

namespace tx_sim {
namespace coordinator {

//!
//! @brief 拷贝构造函数，用于创建 CommandInfo 对象的副本
//!
//! @param[in] ci 要拷贝的 CommandInfo 对象
//!
CommandInfo::CommandInfo(const CommandInfo& ci) {
  cmd = ci.cmd;
  status_cb = ci.status_cb;
  writetopic_cb = ci.writetopic_cb;
  extra_init_params = ci.extra_init_params;
  reset_params = ci.reset_params;
  for (const auto& kv : ci.input_msgs) input_msgs[kv.first].copy(*const_cast<zmq::message_t*>(&kv.second));
  max_setup_monitoring_count = ci.max_setup_monitoring_count;
  l2w_config = ci.l2w_config;
  logsim_events = ci.logsim_events;
  pause_on_start = ci.pause_on_start;
  highlight_group = ci.highlight_group;
}

const CommandStatus Coordinator::kKeepAliveStatus{kCmdAccepted};

// 构造函数，用于创建 Coordinator 对象
// 参数：
//   root_path: 根路径
//   config_loader: 配置加载器
//   module_mgr: 模块管理器
//   scenario_parser: 场景解析器
Coordinator::Coordinator(const std::string& root_path, std::shared_ptr<ConfigLoader> config_loader,
                         std::shared_ptr<ModuleManager> module_mgr, std::shared_ptr<IScenarioParser> scenario_parser)
    : root_path_(root_path), config_loader_(config_loader), module_mgr_(module_mgr), scenario_parser_(scenario_parser) {
  LOG(INFO) << "coordinator constructed.";
}

// 析构函数
Coordinator::~Coordinator() {
  if (grading_feedback_process_thread_ && grading_feedback_process_thread_->joinable())
    grading_feedback_process_thread_->join();
  if (play_cmd_thread_ && play_cmd_thread_->joinable()) play_cmd_thread_->join();
  if (cancel_cmd_thread_ && cancel_cmd_thread_->joinable()) cancel_cmd_thread_->join();
  if (perf_log_stream_.is_open()) perf_log_stream_.close();
  LOG(INFO) << "coordinator destructed.";
}

// 执行命令
// 参数：
//   cmd_info: 命令信息
//   async: 是否异步执行，默认为 false
// 返回值：命令执行结果
CmdErrorCode Coordinator::Execute(const CommandInfo& cmd_info, bool async) {
  auto execute_routine = [this](const CommandInfo cmd_info) -> CmdErrorCode {
    LOG(INFO) << "===> coordinator executing command " << Enum2String(cmd_info.cmd) << " ...";

    CommandStatus status(kCmdAccepted);
    CallbackRoutine(cmd_info.status_cb, status);
    // 记录当前真实时间
    int64_t start_time = GetCpuTimeInMillis();
    // 初始化模块和命令耗时
    uint64_t us1 = 0, us2 = 0, sy1 = 0, sy2 = 0;
    std::tie(us1, sy1) = GetProcessCpuTime();

    switch (cmd_info.cmd) {
      // 暂停命令
      case kCmdPause: {
        cmd_mtx_.lock();
        is_cmd_executing_.store(true, std::memory_order_release);
        pause_.store(false, std::memory_order_release);
        status.ec = kCmdSucceed;
      } break;
      // 恢复命令
      case kCmdUnSetup: {
        cmd_mtx_.lock();
        is_cmd_executing_.store(true, std::memory_order_release);
        UnSetupScenario(status, cmd_info.status_cb, kStopRecordAll);
        un_setup_.store(false, std::memory_order_release);
      } break;
      // 场景设置命令
      case kCmdSetup: {
        SetupScenario(status, cmd_info);
      } break;
      // 单步仿真命令
      case kCmdStep: {
        // 解析logworld参数
        ParseLog2WorldParams(cmd_info);
        Step(status, cmd_info);
      } break;
      // 开始仿真命令
      case kCmdRun: {
        // 解析logworld参数
        ParseLog2WorldParams(cmd_info);
        Run(status, cmd_info.status_cb);
      } break;
      default: {
        LOG(ERROR) << "invalid coordinator command type: " << std::to_string(cmd_info.cmd);
        status.ec = kCmdSystemError;
      }
    }
    // 记录计算完成后的真实hi就
    std::tie(us2, sy2) = GetProcessCpuTime();
    status.total_cpu_time_us = (double)(us2 - us1) / 1000;
    status.total_cpu_time_sy = (double)(sy2 - sy1) / 1000;
    // 统计计算耗时
    status.total_time_cost = GetCpuTimeInMillis() - start_time;
    LOG(INFO) << "<=== coordinator executing command " << Enum2String(cmd_info.cmd)
              << " finished: " << Enum2String(status.ec) << ". >>> took " << status.total_time_cost << "ms <<<";
    // 调用回调函数
    CallbackRoutine(cmd_info.status_cb, status);
    // 释放锁
    is_cmd_executing_.store(false, std::memory_order_release);
    cmd_mtx_.unlock();
    // 返回当前仿真命令的状态
    return status.ec;
  };  // end of execute_routine lmbda func.

  CmdErrorCode ec = CheckCmdState(cmd_info.cmd);
  if (cmd_info.cmd == kCmdHighlightGroup) {
    highlight_group_ = cmd_info.highlight_group;
    LOG(INFO) << "coordinator receive highlight group: " << highlight_group_;
  }
  if (ec != kCmdAccepted) {
    CommandStatus st(ec);
    CallbackRoutine(cmd_info.status_cb, st);  // for compatibility to the old tcp client impl.
    return ec;
  }

  if (async) {
    if (cmd_info.IsCancelCmd()) {
      if (cancel_cmd_thread_ && cancel_cmd_thread_->joinable()) cancel_cmd_thread_->join();
      cancel_cmd_thread_.reset(new std::thread(execute_routine, cmd_info));
    } else {
      if (play_cmd_thread_ && play_cmd_thread_->joinable()) play_cmd_thread_->join();
      play_cmd_thread_.reset(new std::thread(execute_routine, cmd_info));
    }
    return kCmdAccepted;
  } else {
    return execute_routine(cmd_info);
  }
}

// 检查命令状态
// 参数：
//   cmd: 命令
// 返回值：命令执行结果
CmdErrorCode Coordinator::CheckCmdState(Command cmd) {
  if (cmd == kCmdPause || cmd == kCmdUnSetup) {
    bool expected = false;
    std::atomic_bool& tested = (cmd == kCmdPause ? pause_ : un_setup_);
    if (!tested.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
      LOG(WARNING) << " === cmd " << Enum2String(cmd) << " ignored since there's already an on-going one.";
      return kCmdIgnored;
    }
    return kCmdAccepted;
  }

  if (cmd == kCmdSetup && un_setup_.load(std::memory_order_acquire)) return kCmdCancelled;
  else if ((cmd == kCmdRun || cmd == kCmdStep) && pause_.load(std::memory_order_acquire))
    return kCmdCancelled;

  if (cmd == kCmdTerminateSteup) {
    m_bTerminateSetup.store(true, std::memory_order_release);
    return kCmdSucceed;
  }
  if (!cmd_mtx_.try_lock()) {
    LOG(WARNING) << "concurrent command executing request detected:" << Enum2String(cmd);
    return kCmdRejected;
  }
  is_cmd_executing_.store(true, std::memory_order_release);

  if ((cmd == kCmdStep || cmd == kCmdRun) && !scenario_set_) {
    LOG(ERROR) << "invalid cmd " << Enum2String(cmd) << " since there's no scenario set currently.";
    is_cmd_executing_.store(false, std::memory_order_release);
    cmd_mtx_.unlock();
    return kCmdRejected;
  }

  return kCmdAccepted;
}

// 解析log2world参数
// 参数：
//   cmd_info: 命令信息
void Coordinator::ParseLog2WorldParams(const CommandInfo& cmd_info) {
  if (!IsLogsimScenario()) return;

  const auto& c = cmd_info.l2w_config;
  const auto& e = logsim_events_;

  if (last_l2w_play_config_ != nullptr && c == *last_l2w_play_config_) return;
  if (last_l2w_play_config_ == nullptr) last_l2w_play_config_.reset(new Log2WorldPlayConfig);
  *last_l2w_play_config_ = c;

  int32_t switch_point = 0;
  std::ostringstream switch_type_log_substr;
  if (!c.ego_switch && !c.traffic_switch) {
    // 如果主车和交通车不切换,切换时间设置为最大值,使切换时间永远到达不了
    switch_point = std::numeric_limits<int32_t>::max();
    switch_type_log_substr << "NONE<switch disabled>";
  } else {
    if (c.switch_type == kLog2WorldSwitchScene) {
      // 如果是时间切换,切换时间为前端设置的切换时间
      switch_point = c.switch_time_ms;
      switch_type_log_substr << "SCENE<" << c.switch_time_ms << ">";
    } else {  // kLog2WorldSwitchEvent
      // 如果不是时间切换,那就是事件切换
      m_switchType = "EVENT";
      // 如果事件切换为空,仿真系统报错
      if (e.events.empty()) {  // TODO(nemo): should return an error?
        LOG(ERROR) << "ignoring EVENT type log2world since there's no logsim events.";
        return;
      }
      // 事件切换的事件为切换事件的第一个事件点减去前端设置的切换时间点
      switch_point = e.events[0].timestamp_ms - c.switch_time_ms;
      switch_type_log_substr << "EVENT<" << std::to_string(e.events[0].timestamp_ms) << ","
                             << std::to_string(c.switch_time_ms) << ">";
      // kafka切换的时间为仿真系统切换时间
      m_kafka_switch_point = c.switch_time_ms;
      ;
    }
  }

  if (switch_point < 0) {
    LOG(WARNING) << "invalid log2world switch point: " << switch_point << ". ignored.";
    return;
  } else if (switch_point < sim_time_) {
    LOG(WARNING) << "invalid log2world switch point: " << switch_point
                 << ", which is less than the current simulation time: " << sim_time_ << "ms, ignored.";
    return;
  } else if (sim_time_ > log2world_switch_point_) {
    LOG(WARNING) << "invalid log2world switch point: " << switch_point << ", logsim already switched to worldsim @ "
                 << log2world_switch_point_ << "ms, ignored.";
    return;
  }
  // 该topic数据为调度发送给仿真系统算法的topic数据,算法通过该结构获取切换状态
  sim_msg::Log2worldTrigger l2w_trigger;
  // 主车和交通车同时切换
  if (c.ego_switch && c.traffic_switch) l2w_trigger.set_type(sim_msg::LOG2WORLD_TRIGGER_BOTH);
  // 主车切换
  else if (c.ego_switch)
    l2w_trigger.set_type(sim_msg::LOG2WORLD_TRIGGER_EGO);
  // 交通车切换
  else if (c.traffic_switch)
    l2w_trigger.set_type(sim_msg::LOG2WORLD_TRIGGER_TRAFFIC);
  // 不切换
  else
    l2w_trigger.set_type(sim_msg::LOG2WORLD_TRIGGER_NONE);
  l2w_trigger.set_trigger_time(e.start_timestamp_ms + switch_point);

  LOG(INFO) << "set log2world configuration: " << sim_msg::Log2worldTriggerType_Name(l2w_trigger.type()) << " @ "
            << switch_point << "ms(switch type: " << switch_type_log_substr.str()
            << ", switch epoch time: " << l2w_trigger.trigger_time() << "ms, start epoch time: " << e.start_timestamp_ms
            << "ms).";

  l2w_trigger_msg_.rebuild(l2w_trigger.ByteSizeLong());
  l2w_trigger.SerializeToArray(l2w_trigger_msg_.data(), l2w_trigger_msg_.size());
  log2world_switch_point_ = switch_point;
}

// 取消设置场景
// 参数：
//   status: 命令状态
//   cb: 命令状态回调函数
//   record_level: 记录级别
void Coordinator::UnSetupScenario(CommandStatus& status, const CommandStatusCb& cb, StopRecordLevel record_level) {
  if (module_reset_) {  // ensuring module reset-stop semantics.
    LOG(INFO) << "auto stop modules ...";
    Stop(status, cb, record_level);
  }

  ClearScenarioStatus();

  status.ec = kCmdSucceed;
}

//! @brief 成员函数：AddLocalServiceDefaultSetupModule
//! @details 成员函数用途：向协调器添加本地服务的默认设置模块
//!
//! @param[in] vecModuleConfig 模块配置的向量
void Coordinator::AddLocalServiceDefaultSetupModule(const std::vector<ModuleConfig>& vecModuleConfig) {
  // 设置本地服务类型
  m_serviceType = kServiceLocal;
  m_vecAutoLaunchModule.clear();
  for (auto item : vecModuleConfig) { m_vecAutoLaunchModule.emplace_back(item); }
}

//! @brief 函数名：AddCloudCityDefaultStepModule
//! @details 函数功能：添加云城市默认步骤模块
//!
//! @note 该函数添加云城市默认步骤模块。
void Coordinator::AddCloudCityDefaultStepModule() {
  return;
  std::string fullpath = boost::filesystem::initial_path<boost::filesystem::path>().string();
  m_vecAutoLaunchModule.clear();
  ModuleConfig ModuleEnviroment;
  ModuleEnviroment.name = "Enviroment";
  ModuleEnviroment.execute_period = 10;
  ModuleEnviroment.response_time = 10;
  ModuleEnviroment.m_nPriority = 50;
  ModuleEnviroment.conn_args.cmd_timeout = 60000;
  ModuleEnviroment.conn_args.step_timeout = 30000;
  ModuleEnviroment.auto_launch = true;
  ModuleEnviroment.dep_paths.emplace_back("/home/sim_root/sim/deps");
  ModuleEnviroment.so_path = "/home/sim_root/sim/bin/libenvpb.so";
  ModuleEnviroment.category = kModuleCategoryTool;
  m_vecAutoLaunchModule.emplace_back(ModuleEnviroment);
}

void Coordinator::AddCloudServiceDefaultStepModule() {
  // 设置云服务类型
  m_serviceType = kServiceCloud;

  std::string fullpath = boost::filesystem::initial_path<boost::filesystem::path>().string();
  m_vecAutoLaunchModule.clear();
  /*
  ModuleConfig ModuleBillboard;
  ModuleBillboard.name = "Billboard";
  ModuleBillboard.execute_period = 1000;
  ModuleBillboard.response_time = 1000;
  ModuleBillboard.m_nPriority = 50;
  ModuleBillboard.conn_args.cmd_timeout = 60000;
  ModuleBillboard.conn_args.step_timeout = 30000;
  ModuleBillboard.auto_launch = true;

  ModuleBillboard.dep_paths.emplace_back("/home/sim/deps");
  ModuleBillboard.bin_path = "/home/sim/bin/tx_billboard";
  ;
  ModuleBillboard.bin_args.emplace_back("--billboard_app_name");
  ModuleBillboard.bin_args.emplace_back("Billboard");
  ModuleBillboard.bin_args.emplace_back("--ip_addr_port");
  ModuleBillboard.bin_args.emplace_back("");
  ModuleBillboard.category = kModuleCategoryTool;
  m_vecAutoLaunchModule.emplace_back(ModuleBillboard);
*/
  ModuleConfig ModuleEnviroment;
  ModuleEnviroment.name = "Enviroment";
  ModuleEnviroment.execute_period = 500;
  ModuleEnviroment.response_time = 500;
  ModuleEnviroment.m_nPriority = 50;
  ModuleEnviroment.conn_args.cmd_timeout = 60000;
  ModuleEnviroment.conn_args.step_timeout = 30000;
  ModuleEnviroment.auto_launch = true;
  ModuleEnviroment.dep_paths.emplace_back("/home/sim/deps");
  ModuleEnviroment.so_path = "/home/sim/bin/libenvpb.so";
  // ModuleEnviroment.init_args["xml_dir"] = "/home/user/display/download";
  ModuleEnviroment.category = kModuleCategoryTool;
  m_vecAutoLaunchModule.emplace_back(ModuleEnviroment);
}

//! @brief 函数名：AppendLaunchInCurrentConfig
//! @details 函数功能：将启动配置追加到当前配置
//!
//! @param[in] req 一个包含模块重置请求信息的ModuleResetRequest对象
//!
//! @note 该函数根据模块重置请求信息，将启动配置追加到当前配置
void Coordinator::AppendLaunchInCurrentConfig(const ModuleResetRequest* req) {
  current_config_.module_configs.insert(current_config_.module_configs.end(), m_vecAutoLaunchModule.begin(),
                                        m_vecAutoLaunchModule.end());
  std::string sInitControl;
  std::vector<std::string> vecControlName;
  if (kServiceLocal == m_serviceType || kServiceCloud == m_serviceType) {
    sim_msg::Scene simScene;
    int nDataSize = req->m_scene_pb.size();
    simScene.ParseFromArray(req->m_scene_pb.data(), req->m_scene_pb.size());
    auto egos = simScene.egos();
    int size = egos.size();
    if (0 == size) throw std::runtime_error("ego size is zero in sim_msg::scene");

    for (int i = 0; i < egos.size(); ++i) {
      std::string groupname = simScene.egos()[i].group();
      std::string sInitControl = simScene.egos()[i].initial().assign().controller_name();
      if (groupname.empty()) groupname = tx_sim::impl::kDefaultEgoGroupName;
      LOG(INFO) << "group: " << groupname << "init control name:" << sInitControl;
      m_sCurrentSchemeMap[groupname] = sInitControl;
      m_vecGroupName.emplace_back(groupname);
    }

    // simScene.egos()[0].dynamic().events()[0].
    if (!current_config_.module_configs.empty() && highlight_group_.empty()) {
      highlight_group_ = current_config_.module_configs[0].module_group_name;
    }
    for (auto& item : current_config_.module_configs) {
      // if (tx_sim::impl::NeedChangeControl(item.category))
      {
        if (!item.module_group_name.empty()) item.module_scheme_name = m_sCurrentSchemeMap[item.module_group_name];
      }
    }

    std::vector<ModuleConfig> golbal_module_cfgs;
    /*
    if (config_loader_->LoadModuleConfigByName(kGlobalSchemeName, golbal_module_cfgs)) {
      for (auto& item : golbal_module_cfgs) {
        // item.module_scheme_name = kGlobalSchemeName;
        item.module_scheme_name = "";
        current_config_.module_configs.emplace_back(item);
        LOG(INFO) << kGlobalSchemeName << " module name is " << item.name;
      }
    } else {
      LOG(INFO) << kGlobalSchemeName << " is emtpy.";
    }*/

    for (int i = 0; i < egos.size(); ++i) {
      auto events = simScene.egos()[i].dynamic().events();
      int nEventsSize = events.size();
      for (const auto& eventItem : events) {
        std::string sTriggerName = eventItem.name();
        for (const auto& action : eventItem.actions()) {
          if (!action.has_assign()) continue;
          std::string sControlName = action.assign().controller_name();
          vecControlName.emplace_back(sControlName);
          LOG(INFO) << "trigger name:" << sTriggerName << " control name:" << sControlName;
        }
      }
    }

    /*
    for (const auto& sItem : vecControlName)
    {
      std::vector<ModuleConfig> module_cfgs;
      if (!config_loader_->LoadModuleConfigByName(sItem, module_cfgs)) {
        LOG(INFO) << "trigger name:" << sItem << " module is empty";
      }
      for (auto& item : module_cfgs) {
        //if (tx_sim::impl::NeedChangeControl(item.category))
        bool bfind = false;
        for (auto& cur_config_module_item : current_config_.module_configs) {
            //both module to global.
          if (cur_config_module_item == item) {
              LOG(INFO) << item.name << " exist in " << cur_config_module_item.module_scheme_name
                        << " already exist in other scheme. change to global module";
            cur_config_module_item.module_scheme_name = "";
            bfind = true;
            break;
          }
        }
        if (!bfind) {
          item.module_scheme_name = sItem;
          current_config_.module_configs.emplace_back(item);
        }
      }
    }
    */
    // std::set<std::string> launchModuleName;
    // for (const auto& item : current_config_.module_configs) {
    //     if (launchModuleName.find(item.name) != launchModuleName.end() && launchModuleName.size() != 0) {
    //        LOG(ERROR) << item.name << " exist in " << item.module_scheme_name << " already exist in other scheme.";
    //       throw std::runtime_error(item.name + " exist in " + item.module_scheme_name +
    //                               " already exist in other scheme.");
    //     }
    //     launchModuleName.emplace(item.name);
    // }
  }
}

// 设置场景
// 参数：
//   status: 命令状态
//   cmd_info: 命令信息
void Coordinator::SetupScenario(CommandStatus& status, const CommandInfo& cmd_info) {
  UnSetupScenario(status, cmd_info.status_cb, kStopRecordNone);
  if (status.ec == kCmdSystemError) return;

  status.ec = kCmdAccepted;  // ignoring succeed/failure unless system-wide error.
  CallbackRoutine(cmd_info.status_cb, status);
  status.module_status.clear();

  // loading configs from outside source.
  try {
    config_loader_->LoadCoordinatorConfig(current_config_);
  } catch (const std::exception& e) {
    LOG(ERROR) << "coordinator loading configuration error: " << e.what();
    status.ec = kCmdSystemError;
    return;
  }
  // init reset request
  ModuleResetRequest req;
  for (const auto& item : cmd_info.reset_params.ego_infos) {
    LOG(INFO) << "group name:" << item.ego_group_name
              << "  ,ego_vehicle_measurement init size: " << item.ego_vehicle_measurement.size();
  }
  // try to parse the scenario file.
  status.ec = LoadModuleResetRequest(req, cmd_info);
  if (status.ec != kCmdAccepted) return;

  AppendLaunchInCurrentConfig(&req);  // append auto launch in launch.(include local cloud cloudcity)

  logsim_events_ = cmd_info.logsim_events;
  if (IsLogsimScenario()) log2world_switch_point_ = std::numeric_limits<int32_t>::max();  // defaults to logsim playing.
  ParseLog2WorldParams(cmd_info);
  ApplyConfiguration(cmd_info.extra_init_params, status);
  if (status.ec != kCmdAccepted) return;

  shmem_pool_.reset(new ShmemPool);
  MonitorModuleSetup(status, cmd_info.max_setup_monitoring_count, cmd_info.status_cb);
  if (status.ec != kCmdAccepted) return;

  // if (IsLogsimScenario() && logplay_module_.empty()) {
  //   PrintTopicConfigs();
  //   status.ec = kCmdLogPlayModuleNotFound;
  //   return;
  // }

  if (!ConfiguringSimTopics(status)) {  // check topic
    status.ec = kCmdInvalidTopicPubSub;
    return;
  }

  if (cmd_info.writetopic_cb)  // total topic and write to server
  {
    cmd_info.writetopic_cb(m_subTopicMap);
  }

  if (!shmem_pool_->UpdateShmemPubs()) {
    status.ec = kCmdShmemCreationError;
    return;
  }

  for (auto& m : modules_) m.second.conn = module_mgr_->GetConnection(m.first);

  Reset(status, cmd_info, req);
  if (status.ec != kCmdAccepted) return;

  if (cmd_info.pause_on_start) {
    status.ec = kCmdSucceed;
    return;
  }

  Run(status, cmd_info.status_cb);
}

// 应用配置
// 参数：
//   extra_init_params: 额外的初始化参数
//   status: 命令状态
void Coordinator::ApplyConfiguration(const InitParamList& extra_init_params, CommandStatus& status) {
  max_sim_time_ = current_config_.scenario_time_limit == 0 ? std::numeric_limits<uint64_t>::max()
                                                           : (current_config_.scenario_time_limit * 1000);

  stepper_ = std::move(CreateSimStepper(current_config_.coord_mode));

  std::string sim_mode;
  if (IsLogsimScenario())
    sim_mode = (last_l2w_play_config_->ego_switch || last_l2w_play_config_->traffic_switch) ? "l2w" : "logsim";

  std::ostringstream ss, gss;
  for (ModuleConfig& config : current_config_.module_configs) {
    if (config.category == kModuleCategoryGrading) {
      grading_modules_.insert(config.name);
      gss << config.name << ", ";
    }
    ss << config.name << "->" << config.execute_period << "(" << config.response_time << ") ";
    auto& m_ctx = modules_[config.name];  // the default empty module context added.
    m_ctx.config = config;
    m_ctx.shadow_id = config.shadow_id;
    try {
      if (tx_sim::impl::kCoordinatePriority == current_config_.coord_mode) {
        stepper_->AddModuleNodes(config.name, config.execute_period, config.m_nPriority);
      } else {
        stepper_->AddModuleNodes(config.name, config.execute_period, config.response_time);
      }
    } catch (const std::exception& e) {
      LOG(ERROR) << "applying module configuration error: " << e.what();
      status.ec = kCmdInvalidModuleConfig;
      return;
    }
    // adding extra init args if necessary.
    if (config.init_args.find(tx_sim::constant::kInitKeyModuleExecutePeriod) == config.init_args.end())
      config.init_args[tx_sim::constant::kInitKeyModuleExecutePeriod] = std::to_string(config.execute_period);
    if (!sim_mode.empty() && config.init_args.find(kModuleInitArgKeySimMode) == config.init_args.end())
      config.init_args[kModuleInitArgKeySimMode] = sim_mode;
    for (const auto& kv : extra_init_params)
      if (config.init_args.find(kv.first) == config.init_args.end()) config.init_args[kv.first] = kv.second;
  }
  LOG(INFO) << "module runtime config: [" << ss.str() << "]";
  LOG(INFO) << "grading modules: [" << gss.str() << "]";

  module_mgr_->UpdateConfigs(current_config_);

  if (current_config_.log_perf) {
    try {
      EnablePerfLogging();
    } catch (const std::exception& e) {
      LOG(ERROR) << "perf logging cannot be enabled: " << e.what();
      status.ec = kCmdFailed;
    }
  }
}

// 监控模块设置
// 参数：
//   status: 命令状态
//   max_count: 最大设置次数
//   cb: 命令状态回调函数
void Coordinator::MonitorModuleSetup(CommandStatus& status, size_t max_count, const CommandStatusCb& cb) {
  LOG(INFO) << "monitoring module init setup ...";

  size_t initiated_count = 0, finished_count = 0, monitoring_count = 0;
  uint32_t sleep_interval = 100;  // from 100ms to maximal 1s.

  while (!m_bTerminateSetup.load(std::memory_order_acquire) && !un_setup_.load(std::memory_order_acquire) &&
         finished_count < modules_.size()) {
    finished_count = module_mgr_->MonitorInitWorkflow(status.init_status, status.module_status);
    for (const ModuleInitStatus& init_st : status.init_status) {
      if (init_st.state == kModuleInitSucceed) {
        ApplyModuleTopics(init_st.name, init_st.topic_info);
        if (!init_st.topic_info.pub_shmems.empty()) shmem_pool_->AddShmemPubInfo(init_st.topic_info.pub_shmems);
        ++initiated_count;
      }
    }
    CallbackRoutine(cb, status);  // called every time to keep connection alive.
    if (max_count > 0 && ++monitoring_count >= max_count) {
      LOG(WARNING) << "setup monitoring count exceeded maximum " << max_count << " times.";
      break;
    }
    if (status.init_status.empty()) {  // no init event occurred, poll after awhile.
      SleepForMillis(sleep_interval);
      sleep_interval = sleep_interval > 1000 ? 1000 : sleep_interval + 100;
    }
  }
  status.init_status.clear();
  status.module_status.clear();
  if (initiated_count < modules_.size()) {  // some module has not successfully initiated.
    status.ec = (finished_count == modules_.size() || monitoring_count >= max_count) ? kCmdUnEnoughInitiatedModule
                                                                                     : kCmdCancelled;
  }

  LOG(INFO) << "module init setup finished: " << Enum2String(status.ec);
}

// 应用模块主题
// 参数：
//   module_name: 模块名称
//   topic_info: 主题信息
void Coordinator::ApplyModuleTopics(const std::string& module_name, const TopicPubSubInfo& topic_info) {
  auto& sub_topics = modules_[module_name].sub_topics;
  auto& pub_topics = modules_[module_name].pub_topics[kWorldsimPhraseIdx];
  modules_[module_name].pub_topics_broadcast = topic_info.pub_topics_broadcast;
  modules_[module_name].pub_shmems_broadcast = topic_info.pub_shmems_broadcast;

  for (const std::string& t : topic_info.sub_topics)
    sub_topics.emplace_back(t, &Coordinator::HandleMessageLoading, nullptr);
  for (const std::string& t : topic_info.pub_topics)
    pub_topics.emplace_back(t, nullptr, &Coordinator::HandleMessageUnloading);
  for (const std::string& t : topic_info.sub_shmems)
    sub_topics.emplace_back(t, &Coordinator::HandleSubShmemLoading, &Coordinator::HandleSubShmemUnloading);
  for (const std::pair<std::string, size_t>& t : topic_info.pub_shmems)
    pub_topics.emplace_back(t.first, &Coordinator::HandlePubShmemLoading, &Coordinator::HandlePubShmemUnloading);

  if (IsLogsimScenario()) CheckIsLogplayModule(module_name, topic_info.pub_topics);
}

// 检查是否为日志播放模块
// 参数：
//   module_name: 模块名称
//   pubs: 发布主题列表
void Coordinator::CheckIsLogplayModule(const std::string& module_name, const std::vector<std::string>& pubs) {
  bool pub_traffic = false, pub_location = false;
  for (const auto& t : pubs) {
    if (t == tx_sim::topic::kTraffic) pub_traffic = true;
    else if (t == tx_sim::topic::kLocation)
      pub_location = true;
  }
  if (pub_traffic && pub_location) {
    logplay_module_ = module_name;
    LOG(INFO) << "found log-play module: " << module_name;
  }
}

// split module
// step 1: split by groupname
// step 2: split by schemename
void Coordinator::SplitMoudleByScheme(EgoSchemeMap& EgoSchemeMap) {
  std::set<std::string> setGroupNames;
  std::vector<ModulePlayContext> globalModules;
  std::unordered_map<std::string, ModulePlayContext> egoGroupMap;
  EgoSchemeMap.clear();
  // step 1: split by groupname
  for (const auto& kv : modules_) {
    std::string sGroupName = kv.second.config.module_group_name;
    std::string sScheme = kv.second.config.module_scheme_name;
    if (sGroupName.empty()) {
      globalModules.emplace_back(kv.second);
    } else {
      EgoSchemeMap[EgoScheme(sGroupName, sScheme)].emplace_back(kv.second);
    }
  }
  for (auto& kv : EgoSchemeMap) { kv.second.insert(kv.second.end(), globalModules.begin(), globalModules.end()); }

  // for global module and group name is empty.
  if (EgoSchemeMap.empty()) { EgoSchemeMap[EgoScheme("", "")] = globalModules; }
}
bool Coordinator::CheckConflictTopic(const std::vector<ModulePlayContext>& vecModules, size_t idx,
                                     CommandStatus& status) {
  std::unordered_map<std::string, std::tuple<std::string, std::string>> pub_topics;
  for (const auto& kv : vecModules) {
    const std::vector<TopicMeta>& pubs = kv.pub_topics[idx];
    std::string sGroupName = kv.config.module_group_name;
    std::string sSchemeName = kv.config.module_scheme_name;
    for (const TopicMeta& tm : pubs) {
      if (tm.topic.sim_name.empty()) continue;
      std::string sRealTopicName = sGroupName + "-" + kv.config.module_scheme_name + "-" + tm.topic.sim_name;
      auto it = pub_topics.find(sRealTopicName);
      if (it != pub_topics.end())
      // topic conflic if current module name's scheme is equal to current module_scheme or equal to global scheme.
      {
        std::string sModuleName = std::get<0>(it->second);
        std::string sError = "conflict topic publication <" + tm.topic.sim_name + "> with module [" + sModuleName +
                             "] and [" + kv.config.name + "]" + " in scheme {" + kv.config.module_scheme_name + "}";
        status.module_status.emplace_back(sModuleName, kModuleError, sError);
        LOG(ERROR) << sError;
        return false;
      }
      pub_topics[sRealTopicName] = std::make_tuple(kv.config.name, kv.config.module_scheme_name);
    }
  }
  return true;
}

// 配置模拟主题
// 参数：
//   status: 命令状态
// 返回值：如果成功，则返回 true，否则返回 false
bool Coordinator::ConfiguringSimTopics(CommandStatus& status) {
  for (auto& kv : modules_) {
    auto& m = kv.second;

    if (m.shadow_id > 0) {  // it is a shadow module. it does not cares the log2world or scenario triggering configs.
      for (TopicMeta& t : m.pub_topics[kWorldsimPhraseIdx]) {
        t.topic.sim_name = t.topic.name + kShadowTopicPostfix + std::to_string(m.shadow_id);
        t.topic.display_name = t.topic.sim_name;
        if (t.topic.name == tx_sim::topic::kLocation) location_topics_.insert(t.topic.sim_name);
      }
      // the current_sim_phrase of the shadow module is always at kWorldsimPhraseIdx.
      continue;
    }

    for (TopicMeta& t : m.pub_topics[kWorldsimPhraseIdx]) {
      if (t.topic.name == kTopicScenarioStartTriggering) {
        LOG(INFO) << "current scenario is triggered by module " << kv.first;
        scenario_started_ = false;
        stepper_->SetTriggerModule(kv.first);
      }
      t.topic.display_name = t.topic.name;
      if (IsLogsimScenario()) {
        TopicMeta lt = t;
        bool is_logplay_module = (kv.first == logplay_module_);
        ApplyModuleLog2WorldTopic(t, current_config_.log2world_config.worldsim_topic_configs, is_logplay_module);
        ApplyModuleLog2WorldTopic(lt, current_config_.log2world_config.logsim_topic_configs, is_logplay_module);
        m.pub_topics[kLogsimPhraseIdx].push_back(lt);
        m.current_sim_phrase = kLogsimPhraseIdx;
      }
    }
  }

  AdjustShadowModuleSubTopics();
  PrintTopicConfigs();

  // for logsim and worldsim topic setup.
  for (size_t idx = 0; idx < kMaxSimPhraseIdx; ++idx) {
    EgoSchemeMap mapSchemems;
    SplitMoudleByScheme(mapSchemems);
    for (const auto& vecSchemeItem : mapSchemems) {
      if (!CheckConflictTopic(vecSchemeItem.second, idx, status)) return false;
    }
  }

  location_topics_.insert(tx_sim::topic::kLocation);
  return true;
}

// 应用模块日志到世界主题
// 参数：
//   tm: 主题元数据
//   topic_config: 主题配置
//   is_logplay_module: 是否为日志播放模块
void Coordinator::ApplyModuleLog2WorldTopic(TopicMeta& tm,
                                            const decltype(Log2WorldTopicConfig::worldsim_topic_configs)& topic_config,
                                            bool is_logplay_module) {
  auto it = topic_config.find(tm.topic.name);
  if (it == topic_config.end()) return;  // if not specified, publishes topic as normal.

  Log2WorldTopicMode topic_mode = it->second;
  if ((!last_l2w_play_config_->ego_switch && it->second == kLog2WorldTopicSim) ||
      (!last_l2w_play_config_->traffic_switch && it->second == kLog2WorldTopicLog2Sim))
    topic_mode = kLog2WorldTopicLogging;
  switch (topic_mode) {
    case kLog2WorldTopicLogging: {
      if (is_logplay_module) tm.topic.display_name = tm.topic.name + kLogTopicPostfix;
      else
        tm.topic.sim_name = tm.topic.display_name = "";
    } break;
    case kLog2WorldTopicSim: {
      if (is_logplay_module) tm.topic.sim_name = tm.topic.display_name = "";
    } break;
    case kLog2WorldTopicBoth: {
      if (is_logplay_module) {
        tm.topic.sim_name = "";
        tm.topic.display_name = tm.topic.name + kLogTopicPostfix;
      }
    } break;
    case kLog2WorldTopicLog2Sim: {
      if (!is_logplay_module) tm.topic.sim_name = tm.topic.display_name = "";
    } break;
    default: break;
  }
}

// for those shadow modules which is belonged to the same shadow ID, their subscribed topics should also be re-mapped
// to the shadow publications(if any).
void Coordinator::AdjustShadowModuleSubTopics() {
  std::map<int, std::map<std::string, std::string>> shadow_pubs;  // <shadow_id, <original_pub, mapped_sim_pub>>
  for (const auto& m : modules_) {
    if (m.second.shadow_id <= 0) continue;
    auto& pubs = shadow_pubs[m.second.shadow_id];
    const auto& topics = m.second.pub_topics[kWorldsimPhraseIdx];
    for (const auto& topic_meta : topics) pubs[topic_meta.topic.name] = topic_meta.topic.sim_name;
  }

  for (auto& m : modules_) {
    if (m.second.shadow_id <= 0) continue;
    auto& topics = m.second.sub_topics;
    for (auto& topic_meta : topics) {
      const auto pub_topics_it = shadow_pubs.find(m.second.shadow_id);
      // pub_topics_it has to be valid since itself is in shadow_pubs at least.
      const auto topic_it = pub_topics_it->second.find(topic_meta.topic.name);
      if (topic_it != pub_topics_it->second.cend()) topic_meta.topic.sim_name = topic_it->second;
    }
  }
}

// 打印主题配置
void Coordinator::PrintTopicConfigs() {
  auto print_module_topics_func = [this](size_t idx) -> std::string {
    std::ostringstream ss;
    for (const auto& kv : modules_) {
      ss << "<" << kv.first << ">: sub->[";
      std::set<std::string>& topics = m_subTopicMap[kv.first];
      const std::vector<TopicMeta>& subs = kv.second.sub_topics;
      for (const TopicMeta& tm : subs) {
        ss << tm.topic.name;
        auto headerString = boost::algorithm::find_head(tm.topic.name, kUnionFlag.length());
        if (std::string(headerString.begin(), headerString.end()) == kUnionFlag) {
          m_unionTopic.emplace(tm.topic.name.substr(kUnionFlag.length()));
        }

        topics.insert(tm.topic.name);
        if (tm.topic.sim_name != tm.topic.name) ss << "(" << tm.topic.sim_name << ")";
        ss << ", ";
      }
      ss << "] pubs->[";
      const std::vector<TopicMeta>& pubs = kv.second.pub_topics[idx];
      for (const TopicMeta& tm : pubs) {
        ss << tm.topic.name;
        if (tm.topic.sim_name != tm.topic.name || tm.topic.display_name != tm.topic.name)
          ss << "(" << tm.topic.sim_name << "," << tm.topic.display_name << ")";
        ss << ", ";
      }
      ss << "] ";
    }
    return ss.str();
  };
  m_subTopicMap.clear();
  if (IsLogsimScenario())
    LOG(INFO) << "module topic logsim phrase subscription/publication configuration: "
              << print_module_topics_func(kLogsimPhraseIdx);
  LOG(INFO) << "module topic worldsim phrase subscription/publication configuration: "
            << print_module_topics_func(kWorldsimPhraseIdx);

  auto print_unionTopic = [](std::unordered_set<std::string> m_unionTopic) {
    std::ostringstream sUnionTopic;
    sUnionTopic << "<";
    for (const auto& key : m_unionTopic) { sUnionTopic << key << ","; }
    sUnionTopic << ">";
    return sUnionTopic.str();
  };
  LOG(INFO) << kUnionFlag << " topic is " << print_unionTopic(m_unionTopic);
}

// 重置
// 参数：
//   status: 命令状态
//   cmd_info: 命令信息
//   req: 模块重置请求
void Coordinator::Reset(CommandStatus& status, const CommandInfo& cmd_info, ModuleResetRequest& req) {
  if (un_setup_.load(std::memory_order_acquire)) {
    status.ec = kCmdCancelled;
    return;
  }

  LOG(INFO) << "reseting scenario " << cmd_info.reset_params.scenario_path << " ...";

  stepper_->ResetModuleTimes();
  status.module_status.clear();
  status.init_status.clear();
  module_reset_ = true;

  if (current_config_.adding_initial_location_msg)  // adding the initial location message for planning use.
  {
    for (const auto& t : location_topics_) {
      for (auto& itemEgo : req.ego_infos) {
        std::string sGroup = itemEgo.ego_group_name;
        messages_[EgoTopic(sGroup, t)].copy(itemEgo.initial_location);
        LOG(INFO) << "adding initial location msg to topic: " << sGroup + "/" + t;
      }
    }
  }

  const auto config_json_str = current_config_.EncodeToStr();
  messages_[kTopicPlayConfig] = std::move(zmq::message_t(config_json_str.data(), config_json_str.size()));

  LOG(INFO) << "req.ego_vehicle_measurement size: " << req.ego_infos[0].ego_vehicle_measurement.size();
  // sending request async...
  for (auto& m : modules_) {
    std::string sGroup = m.second.config.module_group_name;
    req.group.clear();
    for (auto& egoInfo : req.ego_infos) {
      if (sGroup == egoInfo.ego_group_name) {
        req.group = sGroup;
        break;
      }
    }
    m.second.conn->Send(req);
  }

  // receiving response synchronously since we need waiting all modules anyway.
  for (auto& m : modules_) {
    ModuleResetResponse resp;
    ReceiveModuleReply(m.first, m.second, resp, cmd_info.status_cb);
    UnloadModuleResetResult(resp, m.first, status);
  }

  if (status.ec != kCmdAccepted) return;

  scenario_set_ = true;

  std::lock_guard<std::mutex> lk(current_scenario_mtx_);
  current_scenario_ = cmd_info.reset_params.scenario_path;
}

// 步进
// 参数：
//   status: 命令状态
//   cmd_info: 命令信息
// 返回值：步进结果
uint32_t Coordinator::Step(CommandStatus& status, const CommandInfo& cmd_info) {
  status.ec = kCmdSucceed;

  // update input messages.
  for (const auto& kv : cmd_info.input_msgs) {
    messages_[kv.first].copy(*const_cast<zmq::message_t*>(&kv.second));
    // records the input messages back to client, too.
    zmq::message_t m;
    m.copy(*const_cast<zmq::message_t*>(&kv.second));
    status.step_message.messages.emplace_back(kv.first, std::move(m));
  }

  std::vector<ModuleEvent> events;
  uint32_t time_to_next_event = stepper_->GetModuleEvent(events);

  for (const ModuleEvent& event : events) {
    ModulePlayContext& hdl = modules_[event.module_name];
    std::string sModuleScheneName = hdl.config.module_scheme_name;
    std::string sModuleGroupName = hdl.config.module_group_name;
    // std::string sCurrentScheme = sModuleGroupName.empty() ? m_sCurrentSchemeMap["Ego"] :
    // m_sCurrentSchemeMap[sModuleGroupName]; if (!sModuleScheneName.empty() && sModuleScheneName != sCurrentScheme)
    // continue;
    if (event.type == kModuleEventExecutionTriggered) {
      if (TXSIM_UNLIKELY(status.ec != kCmdSucceed))
        break;  // if last response indicates some module has failed, we should continue to receive those response
                // whose request is already sent out, but no new requests anymore.
      ModuleStepRequest req;
      LoadModuleStepRequest(req, event.module_name);
      AddSystemTime(req);
      req.group = sModuleGroupName;
      hdl.has_outstanding_step_request = hdl.conn->Send(req);  // should always return true.
    } else if (event.type == kModuleEventExecutionFinished) {
      ModuleStepResponse resp;
      ReceiveModuleReply(event.module_name, hdl, resp, cmd_info.status_cb);
      hdl.has_outstanding_step_request = false;
      UnloadModuleStepResult(resp, event.module_name, status);
      AggregateModuleStepMessages(resp, event.module_name, status);
    }
  }

  status.step_message.timestamp = sim_time_;
  if (TXSIM_LIKELY(scenario_started_)) sim_time_ += time_to_next_event;
  VLOG(1) << OutputModuleStepStats(status);

  // if any module requires to stop or error occurred, stop the current scenario.
  if (TXSIM_UNLIKELY(status.ec != kCmdSucceed && current_config_.auto_stop)) {
    CallbackRoutine(cmd_info.status_cb, kCmdAccepted);
    Stop(status, cmd_info.status_cb, status.ec == kCmdScenarioStopped ? kStopRecordAll : kStopRecordFeedbacks);
  } else if (TXSIM_UNLIKELY(!scenario_started_))
    CheckScenarioStarted(events.back().type == kModuleEventExecutionTriggered);
  return time_to_next_event;
}

//! @brief 函数名：AddSystemTime
//! @details 函数功能：添加系统时间
//!
//! @param[in] req 一个包含模块步进请求信息的ModuleStepRequest对象
//!
//! @note 该函数根据模块步进请求信息，添加系统时间。
void Coordinator::AddSystemTime(ModuleStepRequest& req) {
  system_clock::time_point time_point_now = system_clock::now();  // get current time
  system_clock::duration duration_since_epoch =
      time_point_now.time_since_epoch();  // from 1970-01-01 00:00:00 to current
  time_t microseconds_since_epoch = duration_cast<microseconds>(duration_since_epoch).count();
  req.cur_time = (microseconds_since_epoch / 1000.0 / 1000.0);
}

// 停止
// 参数：
//   status: 命令状态
//   cb: 命令状态回调函数
//   record_level: 记录级别
void Coordinator::Stop(CommandStatus& status, const CommandStatusCb& cb, StopRecordLevel record_level) {
  {
    std::lock_guard<std::mutex> lk(current_scenario_mtx_);
    current_scenario_.clear();
  }
  if (perf_log_stream_.is_open()) perf_log_stream_.close();

  std::unordered_set<std::string> stop_modules;
  PollOutstandingRequests(status, stop_modules, cb);

  if (status.ec == kCmdAccepted)  // kCmdAccepted means explicitly stop(called from Execute).
    status.ec = kCmdScenarioStopped;

  Json::Value jsonRoot;
  for (auto& item : m_gradingPerfStats) {
    item.second.per_step_time = item.second.total_step_real_time / item.second.step_count;
    Json::Value jsonValue;
    jsonValue["stepCount"] = item.second.step_count;
    jsonValue["executePeriod"] = item.second.execute_period;
    jsonValue["timeOut"] = item.second.ntime_out;
    jsonValue["totalStepRealTime"] = item.second.total_step_real_time;
    jsonValue["perStepTime"] = item.second.per_step_time;
    jsonRoot[item.first] = jsonValue;
  }
  std::string jsonStr = Json::writeString(Json::StreamWriterBuilder(), jsonRoot);
  LOG(INFO) << "module status: " << jsonStr;

  ModuleStopRequest req;
  req.m_sGradingPerfStats = jsonStr;
  for (auto& m : modules_) {  // sending request async...
    if (stop_modules.find(m.first) != stop_modules.end()) {
      if (!m.second.conn->Send(req)) stop_modules.erase(m.first);
    }
  }

  for (auto& m : modules_) {  // receiving response synchronously since we need waiting all modules anyway.
    if (stop_modules.find(m.first) != stop_modules.end()) {
      ModuleStopResponse resp;
      ReceiveModuleReply(m.first, m.second, resp, cb);
      UnloadModuleStopResult(resp, m.first, status, record_level);
      if (m.second.config.category == kModuleCategoryGrading && !status.module_status.empty() &&
          status.module_status.back().name == m.first)
        ProcessGradingFeedback(status.module_status.back().feedback);
    }
  }

  scenario_set_ = module_reset_ = false;
}

// 运行
// 参数：
//   status: 命令状态
//   cb: 命令状态回调函数
void Coordinator::Run(CommandStatus& status, const CommandStatusCb& cb) {
  int64_t t_sim_start = GetCpuTimeInMillis();
  int64_t tmpSimTime = 0;
  CommandInfo step_cmd(kCmdStep);
  step_cmd.status_cb = cb;

  if (current_config_.log_perf) LoggingPerfStats();
  while (status.ec == kCmdAccepted && !pause_.load(std::memory_order_acquire) &&
         !un_setup_.load(std::memory_order_acquire)) {
    int64_t t_step_start = GetCpuTimeInMillis();
    status.Clear();
    uint32_t time_to_next_step = Step(status, step_cmd);
    tmpSimTime += time_to_next_step;
    if (!highlight_group_.empty()) { FilterCommandStatus(status, highlight_group_); }
    if (!CheckRunStatus(status, cb, t_step_start)) break;
    ControlStepTime(time_to_next_step, t_step_start, t_sim_start, tmpSimTime);
  }
  // if neither error occurred nor scenario stopped, Execute() would send the last acknowledge msg.

  LOG(INFO) << "total sim time: " << sim_time_;
  if (status.ec == kCmdAccepted) status.Clear();  // default to kCmdSucceed.
}

// 清除场景状态
void Coordinator::ClearScenarioStatus() {
  sim_time_ = 0;
  stepper_.reset();
  messages_.clear();
  union_messages_.clear();
  grading_modules_.clear();
  modules_.clear();
  current_config_.module_configs.clear();
  scenario_started_ = true;
  shmem_pool_ = nullptr;  // removing all shared memory resources from OS.
  logplay_module_.clear();
  log2world_switch_point_ = 0;
  l2w_trigger_msg_.rebuild();
  last_l2w_play_config_.reset();
  m_sCurrentSchemeMap.clear();
  m_vecGroupName.clear();
  m_bTerminateSetup.store(false, std::memory_order_release);
}

//! @brief 函数名：PollOutstandingRequests
//! @details 函数功能：轮询未完成的请求
//!
//! @param[in,out] status 一个包含命令状态信息的CommandStatus对象
//! @param[in,out] stop_modules 一个包含停止模块名称的字符串集合
//! @param[in] cb 一个命令状态回调函数
//!
//! @note 该函数轮询未完成的请求，并根据命令状态回调函数更新命令状态和停止模块集合。
void Coordinator::PollOutstandingRequests(CommandStatus& status, std::unordered_set<std::string>& stop_modules,
                                          const CommandStatusCb& cb) {
  for (auto& m : modules_) {
    if (m.second.has_outstanding_step_request) {  // try receiving any on-going step response ...
      ModuleStepResponse resp;
      ReceiveModuleReply(m.first, m.second, resp, cb);
      m.second.has_outstanding_step_request = false;
      UnloadModuleStepResult(resp, m.first, status);
    }
  }

  for (auto& m : modules_)
    if (!m.second.last_cmd_timed_out) stop_modules.insert(m.first);
}

void AddSceneSetting(ModuleResetRequest& req, const std::string logdir, const std::vector<ModuleConfig>& module_configs,
                     std::shared_ptr<IScenarioParser> scenario_parser, const Log2WorldPlayConfig& l2w_config) {
  sim_msg::Scene scene;
  scene.ParseFromArray(static_cast<const char*>(req.m_scene_pb.data()), req.m_scene_pb.size());

  /*********begin add module log path***********************/
  scene.mutable_setting()->set_log_dir(logdir);
  /*********end add module log path***********************/

  /*********begin add module config***********************/
  for (const auto& item : module_configs) {
    sim_msg::Setting_ModuleConfig moduleConfig;
    moduleConfig.set_category(item.module_scheme_category);
    (*scene.mutable_setting()->mutable_module_cfg())[item.name] = moduleConfig;
  }
  /*********end add module config***********************/
  /*********begin add l2w config***********************/

  bool bExistSimRec = scenario_parser->ExistSimRec(req.scenario_path);
  if (!bExistSimRec)
    scene.mutable_setting()->mutable_sim_mode()->set_sim_mode_type(sim_msg::Setting_SimMode_SimModeType_WORLDSIM);
  else {
    scene.mutable_setting()->mutable_sim_mode()->set_sim_mode_type(sim_msg::Setting_SimMode_SimModeType_LOGSIM);

    if (l2w_config.ego_switch || l2w_config.traffic_switch) {
      scene.mutable_setting()->mutable_sim_mode()->set_sim_mode_type(sim_msg::Setting_SimMode_SimModeType_LOG2WORLD);
      scene.mutable_setting()->mutable_sim_mode()->set_ego_switch_time(l2w_config.switch_time_ms);
      scene.mutable_setting()->mutable_sim_mode()->set_traffic_switch_time(l2w_config.switch_time_ms);
    }

    if (l2w_config.switch_type == tx_sim::impl::kLog2WorldManualVehicle)
      scene.mutable_setting()->mutable_sim_mode()->set_sim_mode_type(
          sim_msg::Setting_SimMode_SimModeType_MANUAL_VEHICLE);
  }

  /*********end add l2w config***********************/
  req.m_setting_pb.rebuild(scene.setting().ByteSizeLong());
  scene.setting().SerializeToArray(req.m_setting_pb.data(), req.m_setting_pb.size());

  req.m_scene_pb.rebuild(scene.ByteSizeLong());
  scene.SerializeToArray(req.m_scene_pb.data(), req.m_scene_pb.size());

  std::string sDebug = scene.DebugString();
  LOG(INFO) << "scene pb------------------ begin";
  LOG(INFO) << sDebug;
  LOG(INFO) << "scene pb------------------ end";
}

//! @brief 函数名：LoadModuleResetRequest
//! @details 函数功能：加载模块重置请求
//!
//! @param[in,out] req 一个包含模块重置请求信息的ModuleResetRequest对象
//! @param[in] params 一个包含模块重置请求参数的ModuleResetRequest对象
//!
//! @note 该函数根据输入的模块重置请求参数，加载一个新的模块重置请求。
CmdErrorCode Coordinator::LoadModuleResetRequest(ModuleResetRequest& req, const CommandInfo& cmd_info) {
  req.m_labels = cmd_info.reset_params.m_labels;
  std::string logdir = current_config_.m_module_log_directory;
  if (cmd_info.reset_params.scenario_path.empty()) {
    LOG(INFO) << "scenario_path is empty";
    req = cmd_info.reset_params;
  } else {
    try {
      scenario_parser_->ParseScenario(cmd_info.reset_params.scenario_path, req);
      AddSceneSetting(req, logdir, current_config_.module_configs, scenario_parser_, cmd_info.l2w_config);
    } catch (const std::exception& e) {
      LOG(ERROR) << "scenario file parsing error: " << e.what();
      return kCmdScenarioParsingError;
    }
  }
  AssembleInitialLocation(req);
  return kCmdAccepted;
}

//! @brief 函数名：UnloadModuleResetResult
//! @details 函数功能：卸载模块重置结果
//!
//! @param[in,out] resp 一个包含模块重置响应信息的ModuleResetResponse对象
//! @param[in] name 模块名称
//! @param[in,out] status 一个包含命令状态信息的CommandStatus对象
//!
//! @note 该函数卸载模块重置结果，并根据模块名称和命令状态更新命令状态。
void Coordinator::UnloadModuleResetResult(ModuleResetResponse& resp, const std::string& name, CommandStatus& status) {
  status.module_status.emplace_back();
  ModuleCmdStatus& ms = status.module_status.back();
  ms.name = name;
  ms.ec = resp.type;
  if (ms.ec != kModuleOK) {
    LOG(ERROR) << "coordinator reset module " << ms.name << " error: [" << Enum2String(resp.type) << "] " << resp.err;
    // module internal error also is a system error.
    status.ApplyErrCode(resp.type == kModuleSystemError ? kCmdSystemError : kCmdFailed);
    ms.msg = resp.err;
  }
  LOG(INFO) << "setup finish module name: " << name;
}

//! @brief 函数名：LoadModuleStepRequest
//! @details 函数功能：加载模块步进请求
//!
//! @param[in,out] req 一个包含模块步进请求信息的ModuleStepRequest对象
//! @param[in] name 模块名称
//!
//! @note 该函数根据输入的模块名称，加载一个新的模块步进请求。
void Coordinator::LoadModuleStepRequest(ModuleStepRequest& req, const std::string& module_name) {
  req.sim_time = sim_time_;

  // pushing forward the current simulation phrase of the module if needed.
  ModulePlayContext& hdl = modules_[module_name];
  if (TXSIM_UNLIKELY(hdl.current_sim_phrase == kLogsimPhraseIdx && sim_time_ >= log2world_switch_point_)) {
    ++hdl.current_sim_phrase;
    req.messages[kTopicLog2WorldTriggering].copy(l2w_trigger_msg_);
    LOG(INFO) << "--> module " << module_name << " switched to worldsim on sim time " << sim_time_ << "ms.";
    VLOG(2) << "module " << module_name << " loading message: " << kTopicLog2WorldTriggering << "("
            << l2w_trigger_msg_.size() << " bytes)";
  }
  if (!highlight_group_.empty()) {
    sim_msg::HighlightGroup highlightGroup;
    highlightGroup.set_groupname(highlight_group_);
    m_highlight_group_msg.rebuild(highlightGroup.ByteSizeLong());
    highlightGroup.SerializeToArray(m_highlight_group_msg.data(), m_highlight_group_msg.size());
    req.messages[kTopicHightLightGroup].copy(m_highlight_group_msg);
  }
  // loading message of each topic.
  for (const TopicMeta& tm : hdl.sub_topics)
    if (tm.message_loading_handler) (this->*(tm.message_loading_handler))(tm.topic, hdl, req);
}

//! @brief 函数名：UnloadModuleStepResult
//! @details 函数功能：卸载模块步进结果
//!
//! @param[in,out] resp 一个包含模块步进响应信息的ModuleStepResponse对象
//! @param[in] name 模块名称
//! @param[in,out] status 一个包含命令状态信息的CommandStatus对象
//!
//! @note 该函数卸载模块步进结果，并根据模块名称和命令状态更新命令状态。
// module in shadow mode does not interfere with the module coordination, only records.
void Coordinator::UnloadModuleStepResult(ModuleStepResponse& resp, const std::string& module_name,
                                         CommandStatus& status) {
  status.module_status.emplace_back();
  ModuleCmdStatus& ms = status.module_status.back();
  ms.name = module_name;
  ms.ec = resp.type;
  ModulePlayContext& hdl = modules_[module_name];
  uint32_t module_execute_period = modules_[module_name].config.execute_period;
  bool in_shadow_mode = hdl.shadow_id > 0;
  if (ms.ec == kModuleOK || ms.ec == kModuleRequireStop) {
    ms.elapsed_time = resp.time_cost;
    ms.cpu_time_us = resp.cpu_time_us;
    ms.cpu_time_sy = resp.cpu_time_sy;
    // un-loading message of each topic.

    m_gradingPerfStats[module_name].execute_period = module_execute_period;
    m_gradingPerfStats[module_name].step_count++;
    m_gradingPerfStats[module_name].total_step_real_time += resp.time_cost;
    if (resp.time_cost > module_execute_period) { m_gradingPerfStats[module_name].ntime_out++; }

    const auto& current_topics = hdl.pub_topics[hdl.current_sim_phrase];
    for (const TopicMeta& tm : current_topics)
      if (tm.message_unloading_handler)
        (this->*(tm.message_unloading_handler))(tm.topic, hdl, resp, status.step_message.messages);
    if (TXSIM_UNLIKELY(current_config_.log_perf)) {
      hdl.perf_stats.cpu_sys += resp.cpu_time_sy;
      hdl.perf_stats.cpu_user += resp.cpu_time_us;
      hdl.perf_stats.elapsed += resp.time_cost;
      ++hdl.perf_stats.step_count;
      if (resp.time_cost > hdl.perf_stats.max_elapsed) hdl.perf_stats.max_elapsed = resp.time_cost;
    }
    if (ms.ec == kModuleRequireStop) {
      if (!in_shadow_mode) status.ApplyErrCode(kCmdScenarioStopped);
      ms.msg = resp.err;  // records the module stop reason.
      LOG(INFO) << (in_shadow_mode ? "shadow " : "") << "module " << module_name
                << " requires to stop the scenario. reason: " << ms.msg;
      stepper_->RemoveModule(module_name);
    }
  } else {
    LOG(ERROR) << "coordinator step " << (in_shadow_mode ? "shadow " : "") << "module " << module_name << " error: ["
               << Enum2String(resp.type) << "] " << resp.err;
    // module internal implementation error is also a system error.
    if (resp.type == kModuleSystemError) status.ApplyErrCode(kCmdSystemError);
    else if (!in_shadow_mode)
      status.ApplyErrCode(kCmdFailed);
    ms.msg = resp.err;
    stepper_->RemoveModule(module_name);
  }
}

//! @brief 函数名：UnloadModuleStopResult
//! @details 函数功能：卸载模块停止结果
//!
//! @param[in,out] resp 一个包含模块停止响应信息的ModuleStopResponse对象
//! @param[in] name 模块名称
//! @param[in,out] status 一个包含命令状态信息的CommandStatus对象
//! @param[in] record_level 记录级别
//!
//! @note 该函数卸载模块停止结果，并根据模块名称、命令状态和记录级别更新命令状态。
void Coordinator::UnloadModuleStopResult(ModuleStopResponse& resp, const std::string& name, CommandStatus& status,
                                         StopRecordLevel record_level) {
  // silence the succeed and non-feedback module status.
  if (resp.type == kModuleOK && resp.feedbacks.empty()) {
    LOG(INFO) << "module " << name << " stop succeed with no feedback.";
    return;
  }

  // record the internal system error ALWAYS. otherwise log it only.
  if (record_level == kStopRecordNone && resp.type != kModuleSystemError) {
    if (resp.type != kModuleOK)
      LOG(ERROR) << "coordinator stop module " << name << " error: [" << Enum2String(resp.type) << "] " << resp.err;
    return;
  }

  // record the module "Stop" status.
  status.module_status.emplace_back();
  ModuleCmdStatus& ms = status.module_status.back();
  ms.name = name;
  ms.ec = resp.type;
  if (ms.ec != kModuleOK) {
    LOG(ERROR) << "coordinator stop module " << ms.name << " error: [" << Enum2String(resp.type) << "] " << resp.err;
    if (record_level == kStopRecordAll || ms.ec == kModuleSystemError) {
      status.ApplyErrCode(ms.ec == kModuleSystemError ? kCmdSystemError : kCmdFailed);
      ms.msg = resp.err;
    } else {  // kStopRecordFeedbacks. pop the error status out.
      status.module_status.pop_back();
    }
  } else {
    ms.feedback = Map2Json(resp.feedbacks);
    LOG(INFO) << "module " << name << " stop succeed with feedback: " << ms.feedback;
  }
}

//! @brief 函数名：HandleMessageLoading
//! @details 函数功能：处理消息加载
//!
//! @param[in] topic 模拟主题
//! @param[in,out] module 模块播放上下文
//! @param[in,out] req 一个包含模块步进请求信息的ModuleStepRequest对象
//!
//! @note 该函数根据模拟主题和模块播放上下文，处理消息加载。
void Coordinator::HandleMessageLoading(const SimTopic& topic, ModulePlayContext& module, ModuleStepRequest& req) {
  auto it = messages_.find(EgoTopic(module.config.module_group_name, topic.sim_name));
  if (it != messages_.cend()) {
    if (topic.name == kTopicPlayConfig && sim_time_ > 0) return;
    req.messages[topic.name].copy(it->second);
    if (TXSIM_UNLIKELY(current_config_.log_perf)) module.perf_stats.sent_bytes += it->second.size();
    VLOG(2) << "module " << module.config.name << " loading message: " << topic.name << "(" << it->second.size()
            << " bytes)";
  } else {
    it = messages_.find(EgoTopic("", topic.sim_name));
    if (it != messages_.cend()) {
      req.messages[topic.name].copy(it->second);
      if (TXSIM_UNLIKELY(current_config_.log_perf)) module.perf_stats.sent_bytes += it->second.size();
      VLOG(2) << "module " << module.config.name << " loading message: " << topic.name << "(" << it->second.size()
              << " bytes)";
    }
  }

  auto uit = union_messages_.find(topic.sim_name);
  if (uit != union_messages_.cend()) {
    sim_msg::Union union_msg;
    for (auto& compose : uit->second) {
      std::string msg_payload;
      const char* msg = static_cast<const char*>(compose.second.data());
      uint32_t len = compose.second.size();
      msg_payload.assign(msg, len);

      sim_msg::MsgPair* pair = union_msg.add_messages();
      pair->set_groupname(compose.first);
      pair->set_content(msg_payload);
      VLOG(2) << "module " << module.config.name << " loading union message: " << compose.first << "("
              << compose.second.size() << " bytes)";
    }
    std::string union_payload;
    union_msg.SerializeToString(&union_payload);
    req.messages[topic.name].rebuild(union_payload.c_str(), union_payload.size());
  }
}

//! @brief 函数名：GetCurrentControllerName
//! @details 函数功能：获取当前控制器名称
//!
//! @param[in] sTopicName 一个字符串，表示当前控制器的名称
//! @param[in] data 一个指向常量的指针，表示输入数据的起始地址
//! @param[in] size 一个整数，表示输入数据的大小
//!
//! @note 该函数根据输入的控制器名称和数据，获取当前控制器的名称，用于后续的控制器操作和状态查询。
void Coordinator::GetCurrentControllerName(std::string sTopicName, const void* data, int size) {
  if (sTopicName != "VEHICLE_INTERACTION") return;
  std::string sContorlName;
  sim_msg::VehicleInteraction VehicleInteraction;
  VehicleInteraction.ParseFromArray(data, size);
  sContorlName = VehicleInteraction.assign().controller_name();
  std::string sGroupName = VehicleInteraction.groupname();
  if (sGroupName.empty()) {
    if (sContorlName != m_sCurrentSchemeMap["Ego"] && !sContorlName.empty()) {
      LOG(INFO) << "simtime:" << sim_time_ << " change controller from <" << m_sCurrentSchemeMap["Ego"] << "> to <"
                << sContorlName << ">";
      m_sCurrentSchemeMap["Ego"] = sContorlName;
    }
  } else {
    if (sContorlName != m_sCurrentSchemeMap[sGroupName] && !sContorlName.empty()) {
      LOG(INFO) << "simtime:" << sim_time_ << " change controller from <" << m_sCurrentSchemeMap[sGroupName] << "> to <"
                << sContorlName << ">";
      m_sCurrentSchemeMap[sGroupName] = sContorlName;
    }
  }
}

//! @brief 函数名：HandleMessageUnloading
//! @details 函数功能：处理消息卸载
//!
//! @param[in] topic 模拟主题
//! @param[in,out] module 模块播放上下文
//! @param[in,out] resp 一个包含模块步进响应信息的ModuleStepResponse对象
//! @param[in,out] status_msgs 一个包含状态消息的TopicMessageList对象
//!
//! @note 该函数根据模拟主题、模块播放上下文、模块步进响应和状态消息列表，处理消息卸载。
void Coordinator::HandleMessageUnloading(const SimTopic& topic, ModulePlayContext& module, ModuleStepResponse& resp,
                                         TopicMessageList& status_msgs) {
  std::string topic_name = topic.name;
  std::string topic_display_name = topic.display_name;
  std::string ego_group_name = module.config.module_group_name;
  {
    if (module.config.module_group_name.empty()) {
      // 对于全局模块发出的类似"Ego_001/LOCATION"的消息，需要解析和处理
      std::vector<std::string> topic_result;
      boost::split(topic_result, topic.name, boost::is_any_of("/"));
      if (topic_result.size() == 2) {
        topic_name = topic_result[1];
        topic_display_name = topic_result[1];
        ego_group_name = topic_result[0];
      }
    }
  }

  if (sim_time_ < log2world_switch_point_) {
    if (topic_name == tx_sim::topic::kLocation || topic_name == tx_sim::topic::kTrajectory) return;
  } else {
    if (last_l2w_play_config_ && !last_l2w_play_config_->ego_switch &&
        (topic_name == tx_sim::topic::kLocation || topic_name == tx_sim::topic::kTrajectory))
      return;
  }

  std::string sGroup;
  if (m_vecGroupName.size() == 1) sGroup = m_vecGroupName[0];

  auto it = resp.messages.find(topic.name);
  if (it != resp.messages.end()) {
    if (!topic.sim_name.empty()) {
      messages_[EgoTopic(ego_group_name, topic_name)].copy(it->second);
      if (1 == m_unionTopic.count(topic_name)) {
        union_messages_[kUnionFlag + topic_name][ego_group_name].copy(it->second);
      }

      // add mesages_
      if (sim_time_ <= log2world_switch_point_ || (last_l2w_play_config_ && !last_l2w_play_config_->ego_switch)) {
        if (topic_name == "LOCATION_REPLAY") {
          messages_[EgoTopic(sGroup, tx_sim::topic::kLocation)].copy(it->second);
        } else if (topic_name == "TRAJECTORY_REPLAY") {
          messages_[EgoTopic(sGroup, tx_sim::topic::kTrajectory)].copy(it->second);
        }
      }
      // GetCurrentControllerName(topic.name, it->second.data(), it->second.size());
      VLOG(2) << "module " << module.config.name << " unloading message: " << topic.name << "(" << it->second.size()
              << " bytes)";
    }

    // add status_msgs add LOCATION and TRAJECTORY;
    if (sim_time_ <= log2world_switch_point_ || (last_l2w_play_config_ && !last_l2w_play_config_->ego_switch)) {
      zmq::message_t t;
      if (topic_display_name == "LOCATION_REPLAY") {
        t.copy(it->second);
        status_msgs.emplace_back(EgoTopic(sGroup, tx_sim::topic::kLocation), std::move(t));
      } else if (topic_display_name == "TRAJECTORY_REPLAY") {
        t.copy(it->second);
        status_msgs.emplace_back(EgoTopic(sGroup, tx_sim::topic::kTrajectory), std::move(t));
      }
    }

    if (!topic_display_name.empty()) {
      if (topic_display_name == "LOCATION_REPLAY") {
        status_msgs.emplace_back(EgoTopic(sGroup, topic_display_name), std::move(it->second));
      } else if (topic_display_name == "TRAJECTORY_REPLAY") {
        status_msgs.emplace_back(EgoTopic(sGroup, topic_display_name), std::move(it->second));
      } else {
        status_msgs.emplace_back(EgoTopic(ego_group_name, topic_display_name), std::move(it->second));
      }
    }
    if (TXSIM_UNLIKELY(current_config_.log_perf)) module.perf_stats.recv_bytes += it->second.size();
  }
}

//! @brief 函数名：HandleSubShmemLoading
//! @details 函数功能：处理子共享内存加载
//!
//! @param[in] name 模拟主题名称
//! @param[in,out] module 模块播放上下文
//! @param[in,out] req 一个包含模块步进请求信息的ModuleStepRequest对象
//!
//! @note 该函数根据模拟主题名称和模块播放上下文，处理子共享内存加载。
void Coordinator::HandleSubShmemLoading(const SimTopic& topic, ModulePlayContext& module, ModuleStepRequest& req) {
  req.sub_topic_shmem_names[topic.name] = shmem_pool_->GetSubShmemName(topic.name, module.config.name);
}

//! @brief 函数名：HandleSubShmemUnloading
//! @details 函数功能：处理子共享内存卸载
//!
//! @param[in] name 模拟主题名称
//! @param[in,out] module 模块播放上下文
//! @param[in,out] resp 一个包含模块步进响应信息的ModuleStepResponse对象
//! @param[in,out] status_msgs 一个包含状态消息的TopicMessageList对象
//!
//! @note 该函数根据模拟主题名称、模块播放上下文、模块步进响应和状态消息列表，处理子共享内存卸载。
void Coordinator::HandleSubShmemUnloading(const SimTopic& topic, ModulePlayContext& module, ModuleStepResponse& resp,
                                          TopicMessageList& status_msgs) {
  shmem_pool_->DoneWithSubShmem(topic.name, module.config.name);
}

//! @brief 函数名：HandlePubShmemLoading
//! @details 函数功能：处理发布共享内存加载
//!
//! @param[in] name 模拟主题名称
//! @param[in,out] module 模块播放上下文
//! @param[in,out] req 一个包含模块步进请求信息的ModuleStepRequest对象
//!
//! @note 该函数根据模拟主题名称和模块播放上下文，处理发布共享内存加载。
void Coordinator::HandlePubShmemLoading(const SimTopic& topic, ModulePlayContext& module, ModuleStepRequest& req) {
  req.pub_topic_shmem_names[topic.name] = shmem_pool_->GetPubShmemName(topic.name, module.config.name);
}

//! @brief 函数名：HandlePubShmemUnloading
//! @details 函数功能：处理发布共享内存卸载
//!
//! @param[in] name 模拟主题名称
//! @param[in,out] module 模块播放上下文
//! @param[in,out] resp 一个包含模块步进响应信息的ModuleStepResponse对象
//! @param[in,out] status_msgs 一个包含状态消息的TopicMessageList对象
//!
//! @note 该函数根据模拟主题名称、模块播放上下文、模块步进响应和状态消息列表，处理发布共享内存卸载。
void Coordinator::HandlePubShmemUnloading(const SimTopic& topic, ModulePlayContext& module, ModuleStepResponse& resp,
                                          TopicMessageList& status_msgs) {
  shmem_pool_->DoneWithPubShmem(topic.name, module.config.name);
}

//! @brief 函数名：ReceiveModuleReply
//! @details 函数功能：接收模块回复
//!
//! @param[in] name 模块名称
//! @param[in,out] m 模块播放上下文
//! @param[in,out] reply 一个包含模块响应信息的ModuleResponse对象
//! @param[in] cb 一个命令状态回调函数
//!
//! @note 该函数根据模块名称、模块播放上下文、模块响应和命令状态回调函数，接收模块回复。
void Coordinator::ReceiveModuleReply(const std::string& name, ModulePlayContext& m, ModuleResponse& reply,
                                     const CommandStatusCb& cb) {
  while (!m_bTerminateSetup.load(std::memory_order_acquire) && !m.conn->Receive(reply, kKeepAliveInterval)) {
    CallbackRoutine(cb, Coordinator::kKeepAliveStatus);
    if (!module_mgr_->CheckModuleStatus(name, reply.err)) {
      reply.type = kModuleProcessExit;
      break;
    }
  }
  if (m_bTerminateSetup) reply.type = kModuleTimeout;
  m.last_cmd_timed_out = (reply.type == kModuleTimeout || reply.type == kModuleProcessExit);
  if (m.last_cmd_timed_out) { module_mgr_->RemoveRegistryModule(name); }
}

//! @brief 函数名：CheckScenarioStarted
//! @details 函数功能：检查场景是否已启动
//!
//! @param[in] last_module_triggered 最后一个触发的模块
//!
//! @note 该函数根据最后一个触发的模块，检查场景是否已启动。
void Coordinator::CheckScenarioStarted(bool last_module_triggered) {
  scenario_started_ = !messages_[kTopicScenarioStartTriggering].empty();
  if (!scenario_started_) return;

  stepper_->TriggerAllModules();
  LOG(INFO) << "scenario triggered and started.";

  // TODO(nemo): re-design the implementation to remove this hacking fix for async coordination.
  if (current_config_.coord_mode == kCoordinateAsync && last_module_triggered) {
    std::vector<ModuleEvent> events;
    uint32_t time_to_next_event = stepper_->GetModuleEvent(events);
    for (const ModuleEvent& event : events) {
      assert(event.type == kModuleEventExecutionTriggered);
      ModulePlayContext& hdl = modules_[event.module_name];
      ModuleStepRequest req;
      LoadModuleStepRequest(req, event.module_name);
      hdl.has_outstanding_step_request = hdl.conn->Send(req);  // should always return true.
    }
  }
}

//! @brief 函数名：CheckRunStatus
//! @details 函数功能：检查运行状态
//!
//! @param[in,out] sttus 一个包含命令状态信息的CommandStatus对象
//! @param[in] cb 一个命令状态回调函数
//! @param[in] start_time 开始时间
//!
//! @note 该函数根据命令状态、命令状态回调函数和开始时间，检查运行状态。
inline bool Coordinator::CheckRunStatus(CommandStatus& status, const CommandStatusCb& cb, int64_t start_time) {
  // let Execute() to call the last callback if not to continue.
  if (TXSIM_UNLIKELY(status.ec != kCmdSucceed)) return false;
  // send step status. set kCmdAccepted to indicate client that there's more data to receive.
  status.ec = kCmdAccepted;
  status.total_time_cost = GetCpuTimeInMillis() - start_time;
  CallbackRoutine(cb, status);
  if (TXSIM_UNLIKELY(current_config_.log_perf)) LoggingPerfStats();
  // check if scenario is timeout.
  if (TXSIM_UNLIKELY(sim_time_ > max_sim_time_)) {
    status.Clear();
    status.ec = kCmdScenarioTimeout;
    Stop(status, cb, kStopRecordAll);  // auto stop the scenario.
    return false;
  }
  return true;
}

//! @brief 函数名：ControlStepTime
//! @details 函数功能：控制步进时间
//!
//! @param[in] time_to_next_step 下一步的时间
//! @param[in] step_start_time 步骤开始时间
//! @param[in] sim_start_time 模拟开始时间
//! @param[in] tmpSimTime 临时模拟时间
//!
//! @note 该函数根据下一步的时间、步骤开始时间、模拟开始时间和临时模拟时间，控制步进时间。
inline void Coordinator::ControlStepTime(uint32_t time_to_next_step, int64_t step_start_time, int64_t sim_start_time,
                                         int64_t tmpSimTime) {
  auto& ctrl_rate = current_config_.control_rate;
  if (TXSIM_LIKELY(ctrl_rate == 100)) SleepUntilMillis(sim_start_time + tmpSimTime);
  else if (TXSIM_LIKELY(ctrl_rate == 0))
    return;
  else
    SleepUntilMillis(step_start_time + time_to_next_step * (ctrl_rate / 100.0f));
}

//! @brief 函数名：CallbackRoutine
//! @details 函数功能：回调例程
//!
//! @param[in] cb 一个命令状态回调函数
//! @param[in] st 一个包含命令状态信息的CommandStatus对象
//!
//! @note 该函数根据命令状态回调函数和命令状态，执行回调例程。
inline void Coordinator::CallbackRoutine(const CommandStatusCb& cb, const CommandStatus& st) {
  if (!cb) return;
  try {
    cb(st);
  } catch (const std::exception& e) { LOG(ERROR) << "coordinator SetupModules callback exception: " << e.what(); }
}

const std::string Coordinator::kPerfLogPath = "perf/core_perf.";

//! @brief 函数名：EnablePerfLogging
//! @details 函数功能：启用性能日志记录
//!
//! @note 该函数启用性能日志记录。
void Coordinator::EnablePerfLogging() {
  namespace bfs = boost::filesystem;
  bfs::path perf_log_path(root_path_), perf_dir_path;
  perf_log_path /= (kPerfLogPath + GetCurrentDateTimeStr() + ".log");
  perf_log_path = perf_log_path.lexically_normal();
  perf_dir_path = perf_log_path.parent_path();
  if (!bfs::is_directory(perf_dir_path) && !bfs::create_directories(perf_dir_path))
    throw std::runtime_error(std::string("perf directory ") + perf_dir_path.string() + " cannot be created.");
  perf_log_stream_.open(perf_log_path.string());
  if (!perf_log_stream_.is_open())
    throw std::runtime_error(std::string("perf log file ") + perf_log_path.string() + " cannot be opened.");
  LOG(INFO) << "opened perf log file: " << perf_log_path;

  Json::StreamWriterBuilder b;
  b["indentation"] = "";
  perf_writer_.reset(b.newStreamWriter());
}

//! @brief 函数名：LoggingPerfStats
//! @details 函数功能：记录性能统计信息
//!
//! @note 该函数记录性能统计信息。
void Coordinator::LoggingPerfStats() {
  static int64_t last_log_time = 0;
  int64_t t = GetWallTimeInMillis(), duration;
  if ((duration = t - last_log_time) < 1000) return;  // logging every 1 seconds.
  last_log_time = t;
  double duration_s = ((double)duration) / 1000.0;
  Json::Value r;
  r["t"] = t;
  r["st"] = sim_time_;
  Json::Value& ml = r["m"];
  ml = Json::arrayValue;
  uint64_t total_sent = 0, total_recv = 0;
  for (auto& name_ctx : modules_) {
    auto& ps = name_ctx.second.perf_stats;
    Json::Value& m = ml.append(Json::Value());
    m["n"] = name_ctx.first;
    m["sb"] = ps.sent_bytes;
    total_sent += ps.sent_bytes;
    m["rb"] = ps.recv_bytes;
    total_recv += ps.recv_bytes;
    m["us"] = ps.cpu_user;
    m["sy"] = ps.cpu_sys;
    m["mel"] = ps.max_elapsed;
    m["ael"] = ps.step_count > 0 ? (ps.elapsed / ps.step_count) : 0;
    std::fill(reinterpret_cast<uint8_t*>(&ps), reinterpret_cast<uint8_t*>(&ps) + sizeof(ps), static_cast<uint8_t>(0));
  }
  r["tp"] = total_recv / duration_s;
  r["tt"] = (total_recv + total_sent) / duration_s;
  perf_writer_->write(r, &perf_log_stream_);
  perf_log_stream_ << '\n';
}

//! @brief 函数名：ProcessGradingFeedback
//! @details 函数功能：处理评分反馈
//!
//! @param[in] feedback 反馈信息
//!
//! @note 该函数根据反馈信息，处理评分反馈。
void Coordinator::ProcessGradingFeedback(const std::string& feedback) {
  if (current_config_.custom_grading_feedback_process.empty()) return;
  if (feedback.empty()) {
    LOG(WARNING) << "grading feedback is empty. no processing @ " << current_config_.custom_grading_feedback_process;
    return;
  }

  if (grading_feedback_process_thread_ && grading_feedback_process_thread_->joinable())
    grading_feedback_process_thread_->join();

  grading_feedback_process_thread_.reset(new std::thread(
      [](const std::string process_path, const std::string grading_feedback) {
        LOG(INFO) << "--------------- processing grading feedback " << grading_feedback << " ...";
        std::ostringstream cmd;
        cmd << process_path << " '" << grading_feedback << "'";
        int r = system(cmd.str().c_str());
        LOG(INFO) << "--------------- done processing grading feedback: " << GetProcessExitStatusDescription(r);
      },
      current_config_.custom_grading_feedback_process, feedback));
}

//! @brief 函数名：OutputModuleStepStats
//! @details 函数功能：输出模块步进统计信息
//!
//! @param[in] status 一个包含命令状态信息的CommandStatus对象
//!
//! @note 该函数根据命令状态，输出模块步进统计信息。
const std::string Coordinator::OutputModuleStepStats(const CommandStatus& status) {
  std::ostringstream ss;
  ss << "module step stats @ " << status.step_message.timestamp << "ms: ";
  for (const auto& m : status.module_status) {
    ss << m.name << "->(" << m.elapsed_time << "/" << m.cpu_time_us << "/" << m.cpu_time_sy << ") ";
  }
  return ss.str();
}

void Coordinator::FilterCommandStatus(CommandStatus& status, const std::string& filter_group) {
  // LOG(INFO) << "========== FilterCommandStatus ===========" << "highlight group:" << filter_group;
  for (std::vector<ModuleCmdStatus>::iterator iter = status.module_status.begin();
       iter != status.module_status.end();) {
    const ModulePlayContext& hdl = modules_[iter->name];
    std::string group_name = hdl.config.module_group_name;
    if (isGroupModuelName(group_name, iter->name)) {
      if (group_name != filter_group) {
        iter = status.module_status.erase(iter);
        continue;
      } else {
        iter->name = fetchModuelName(group_name, iter->name);
      }
    }
    ++iter;
  }

  for (std::vector<ModuleInitStatus>::iterator iter = status.init_status.begin(); iter != status.init_status.end();) {
    const ModulePlayContext& hdl = modules_[iter->name];
    std::string group_name = hdl.config.module_group_name;
    if (isGroupModuelName(group_name, iter->name)) {
      if (group_name != filter_group) {
        iter = status.init_status.erase(iter);
        continue;
      } else {
        iter->name = fetchModuelName(group_name, iter->name);
      }
    }
    ++iter;
  }

  std::unordered_set<std::string> total_pub_topics;
  std::unordered_set<std::string> total_sub_topics;
  for (const auto& module : modules_) {
    const ModulePlayContext& hdl = module.second;
    std::string group_name = hdl.config.module_group_name;
    if (group_name == filter_group || group_name.empty()) {
      const std::vector<TopicMeta>& pub_topics = hdl.pub_topics[kWorldsimPhraseIdx];
      const std::vector<TopicMeta>& sub_topics = hdl.sub_topics;
      for (const auto& meta : pub_topics) { total_pub_topics.insert(meta.topic.sim_name); }
      for (const auto& meta : sub_topics) { total_sub_topics.insert(meta.topic.sim_name); }
    }
  }
  for (TopicMessageList::iterator iter = status.step_message.messages.begin();
       iter != status.step_message.messages.end();) {
    std::string topic_name = iter->first.sTopic;
    if (total_pub_topics.find(topic_name) != total_pub_topics.end() ||
        total_sub_topics.find(topic_name) != total_sub_topics.end()) {
      ++iter;
    } else {
      iter = status.step_message.messages.erase(iter);
      continue;
    }
  }
}

void Coordinator::AggregateModuleStepMessages(ModuleStepResponse& resp, const std::string& module_name,
                                              CommandStatus& status) {
  ModulePlayContext& hdl = modules_[module_name];
  std::string groupname = hdl.config.module_group_name;
  for (const auto& t : hdl.pub_topics_broadcast) {
    std::string union_topic_name = t;
    boost::replace_all(union_topic_name, groupname, "EgoUnion");
    const auto& it = messages_.find(t);
    if (it != messages_.end()) union_messages_[union_topic_name][groupname].copy(it->second);
  }
}

}  // namespace coordinator
}  // namespace tx_sim
