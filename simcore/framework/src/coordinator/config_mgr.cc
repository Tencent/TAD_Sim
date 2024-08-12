// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "config_mgr.h"

#include <limits>
#include <sstream>

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/join.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/filesystem.hpp"
#include "boost/range/adaptor/transformed.hpp"
#include "glog/logging.h"

#include "sql_defs.h"
#include "utils/constant.h"
#include "utils/json_helper.h"
#include "utils/leb128.h"

using namespace tx_sim::impl;
using namespace tx_sim::utils;

//! @brief 函数名：ArgsMap2String
//! @details 函数功能：将std::map<std::string, std::string>类型的参数转换为字符串
//!
//! @param[in] args 一个包含键值对的std::map<std::string, std::string>对象
//! @param[out] output 一个std::string对象，用于存储转换后的字符串
//!
//! @note 该函数将args中的键值对转换为一个字符串，并将结果存储在output中。
//!       键值对之间用kSeparatorInitArgKV分隔，整个字符串用kSeparatorInitArgList分隔。
static void ArgsMap2String(const std::map<std::string, std::string>& args, std::string& output) {
  std::vector<std::string> args_kv_strs;
  for (const std::pair<std::string, std::string>& kv : args) {
    args_kv_strs.emplace_back(kv.first + kSeparatorInitArgKV + kv.second);
  }
  output = boost::algorithm::join(args_kv_strs, kSeparatorInitArgList);
}

//! @brief 函数名：String2ArgsMap
//! @details 函数功能：将字符串转换为std::map<std::string, std::string>类型的参数
//!
//! @param[in] input 一个包含键值对的字符串
//! @param[out] args 一个std::map<std::string, std::string>对象，用于存储转换后的键值对
//!
//! @note 该函数将input中的键值对转换为一个std::map<std::string, std::string>对象，并将结果存储在args中。
//!       键值对之间用kSeparatorInitArgKV分隔，整个字符串用kSeparatorInitArgList分隔。
static void String2ArgsMap(const std::string& input, std::map<std::string, std::string>& args) {
  if (input.empty()) return;
  std::vector<std::string> args_list;
  boost::split(args_list, input, boost::is_any_of(kSeparatorInitArgList));
  for (const std::string& args_kv : args_list) {
    const auto idx = args_kv.find_first_of(kSeparatorInitArgKV);
    args[args_kv.substr(0, idx)] = (idx == std::string::npos ? "" : args_kv.substr(idx + 1));
  }
}

//! @brief 函数名：GetSqlStatementWithModuleConfig
//! @details 函数功能：根据模块配置生成SQL语句
//!
//! @param[in] sql 一个包含SQL语句模板的字符串
//! @param[in] config 一个包含模块配置信息的ModuleConfig对象
//! @param[in] format SQL语句的格式
//!
//! @return 返回一个包含生成的SQL语句的字符串
//!
//! @note 该函数根据模块配置生成SQL语句，并将结果存储在args中。
//!       键值对之间用kSeparatorInitArgKV分隔，整个字符串用kSeparatorInitArgList分隔。
static char* GetSqlStatementWithModuleConfig(const char* sql, const tx_sim::coordinator::ModuleConfig& config,
                                             SqlStatFormat format) {
  std::string init_args_str, deps_str, bin_args_str;
  if (!config.init_args.empty()) ArgsMap2String(config.init_args, init_args_str);
  if (!config.dep_paths.empty()) deps_str = boost::algorithm::join(config.dep_paths, kSeparatorDepPathList);
  if (!config.bin_args.empty()) bin_args_str = boost::algorithm::join(config.bin_args, kSeparatorInitArgList);
  uint32_t cmd_timeout = config.conn_args.cmd_timeout == 0 ? kDefaultCmdTimeoutMillis : config.conn_args.cmd_timeout;
  uint32_t step_timeout = config.conn_args.step_timeout;
#define VALUE_EXPR                                                                                          \
  config.execute_period, config.response_time, init_args_str.c_str(), config.auto_launch, deps_str.c_str(), \
      config.so_path.c_str(), config.bin_path.c_str(), bin_args_str.c_str(), cmd_timeout, step_timeout,     \
      config.category, config.shadow_id
#define VALUE_EXPR_WITHOUT_INIT_ARGS                                                                         \
  config.execute_period, config.response_time, config.auto_launch, deps_str.c_str(), config.so_path.c_str(), \
      config.bin_path.c_str(), bin_args_str.c_str(), cmd_timeout, step_timeout, config.category, config.shadow_id
  switch (format) {
    case kSqlStateInsert: return sqlite3_mprintf(sql, config.name.c_str(), VALUE_EXPR);
    case kSqlStateUpdate: return sqlite3_mprintf(sql, VALUE_EXPR, config.name.c_str());
    case kSqlStateUpsert: return sqlite3_mprintf(sql, config.name.c_str(), VALUE_EXPR, VALUE_EXPR_WITHOUT_INIT_ARGS);
    default: return nullptr;
  }
}

//! @brief 函数名：GetModuleConfigFromDB
//! @details 函数功能：从数据库中获取模块配置信息
//!
//! @param[out] m_config 一个包含模块配置信息的ModuleConfig对象
//! @param[in] columns 一个包含数据库中模块配置信息的字符串数组
//!
//! @note 该函数从数据库中获取模块配置信息，并将结果存储在m_config中。
static void GetModuleConfigFromDB(tx_sim::coordinator::ModuleConfig& m_config, char** columns) {
  m_config.name = columns[0];
  m_config.execute_period = columns[1] ? std::stoi(columns[1]) : 0;
  m_config.response_time = columns[2] ? std::stoi(columns[2]) : 0;
  if (columns[3]) String2ArgsMap(columns[3], m_config.init_args);
  m_config.auto_launch = (columns[4] && (std::stoi(columns[4]) == 1));
  if (columns[5] && strlen(columns[5]))
    boost::split(m_config.dep_paths, columns[5], boost::is_any_of(kSeparatorDepPathList));
  if (columns[6]) m_config.so_path = columns[6];
  if (columns[7]) m_config.bin_path = columns[7];
  if (columns[8] && strlen(columns[8]))
    boost::split(m_config.bin_args, columns[8], boost::is_any_of(kSeparatorInitArgList));
  m_config.conn_args.cmd_timeout = columns[9] ? std::stoi(columns[9]) : kDefaultCmdTimeoutMillis;
  m_config.conn_args.step_timeout = columns[10] ? std::stoi(columns[10]) : kDefaultStepTimeoutMillis;
  m_config.category = static_cast<ModuleCategory>(std::stoi(columns[11]));
  m_config.shadow_id = columns[12] ? std::stoi(columns[12]) : 0;
}

//! @brief 函数名：GetModuleNameListString
//! @details 函数功能：将模块配置列表中的模块名称转换为字符串
//!
//! @param[in] configs 一个包含模块配置信息的ModuleConfig对象列表
//!
//! @return 返回一个包含模块名称列表的字符串
//!
//! @note 该函数将模块配置列表中的模块名称转换为字符串，并将结果存储在args中。
//!       模块名称之间用逗号和空格分隔。
static std::string GetModuleNameListString(const std::vector<tx_sim::coordinator::ModuleConfig>& configs) {
  return boost::algorithm::join(
      configs | boost::adaptors::transformed([](const tx_sim::coordinator::ModuleConfig& c) { return c.name; }), ", ");
}

namespace tx_sim {
namespace coordinator {
bool isGroupModuelName(const std::string& groupname, const std::string& modulename) {
  std::vector<std::string> vecStr;
  boost::algorithm::split(vecStr, modulename, boost::is_any_of("-"));
  return (vecStr.size() > 1 && vecStr[0] == groupname);
}

std::string fetchModuelName(const std::string& groupname, const std::string& modulename) {
  std::vector<std::string> vecStr;
  boost::algorithm::split(vecStr, modulename, boost::is_any_of("-"));
  if (vecStr.size() > 1 && vecStr[0] == groupname) {
    return vecStr[1];
  } else {
    return modulename;
  }
}

std::string makeGroupModuelName(const std::string& groupname, const std::string& modulename) {
  return groupname + "-" + modulename;
}

//! @brief 构造函数：ConfigManager
//! @details 构造函数功能：初始化ConfigManager对象
//!
//! @param[in] db_path 一个包含数据库路径的字符串
//!
//! @note 该构造函数初始化ConfigManager对象，并将结果存储在args中。
ConfigManager::ConfigManager(const std::string& db_path) {
  int ec = sqlite3_open(db_path.c_str(), &db_);
  if (ec != SQLITE_OK || db_ == nullptr) {
    std::string err = sqlite3_errmsg(db_);
    if (db_) sqlite3_close_v2(db_);
    throw std::runtime_error(err);
  }
  LOG(INFO) << "ConfigManager initiated with sqlite database: " << db_path;

  module_config_get_cb_ = [](void* module_configs_ptr, int column_count, char** columns, char** column_names) -> int {
    auto* configs = static_cast<std::vector<ModuleConfig>*>(module_configs_ptr);
    configs->emplace_back();
    GetModuleConfigFromDB(configs->back(), columns);
    return 0;
  };
}

ConfigManager::~ConfigManager() {
  if (db_) sqlite3_close_v2(db_);
  LOG(INFO) << "ConfigManager destructed.";
}

//! @brief 函数名：InitConfigData
//! @details 函数功能：初始化配置数据
//!
//! @param[in] default_config_path 一个包含默认配置路径的字符串
//! @param[in] cloud_root_dir 一个包含云根目录路径的字符串
//!
//! @note 该函数初始化配置数据。
void ConfigManager::InitConfigData(const std::string& default_config_path, const std::string& cloud_root_dir) {
  ExecSqliteRoutine(TXSIM_SQL_CREATE_MODULE_CONFIG_TABLE);
  ExecSqliteRoutine(TXSIM_SQL_CREATE_MODULE_SCHEME_TABLE);
  ExecSqliteRoutine(TXSIM_SQL_CREATE_MULTIEGO_SCHEME_TABLE);
  ExecSqliteRoutine(TXSIM_SQL_CREATE_SYS_CONFIG_TABLE);
  ExecSqliteRoutine(TXSIM_SQL_CREATE_PLAY_LIST_TABLE);
  ExecSqliteRoutine(TXSIM_SQL_CREATE_GRADING_KPI_TABLE);
  ExecSqliteRoutine(TXSIM_SQL_CREATE_KPI_GROUP_TABLE);
  ExecSqliteRoutine(
      "create table if not exists config_version(version integer default 0 not null);"
      "insert or ignore into config_version (rowid, version) values (1, 7);");

  if (!boost::filesystem::exists(default_config_path)) return;

  boost::filesystem::path app_dir(default_config_path), data_dir, sys_dir;
  if (cloud_root_dir.empty()) {
    app_dir = app_dir.parent_path().parent_path();
    data_dir = app_dir / kPathUserDirData;
    sys_dir = app_dir / kPathUserDirSys;
    m_pbinfo_data_path = (data_dir / kPbInfoFileName).lexically_normal().string();
    m_pbinfo_sys_path = (sys_dir / kPbInfoFileName).lexically_normal().string();
  } else {
    data_dir = sys_dir = cloud_root_dir;
  }

  LOG(INFO) << "config manager reading default module config from " << default_config_path << " ...";
  Json::Value cfg_doc;
  ReadJsonDocFromFile(cfg_doc, default_config_path);
  default_config_.Decode(cfg_doc);
  // reading schemes node in.
  Json::Value scheme_node = cfg_doc[kJsonKeyModuleSchemes];
  if (scheme_node.isNull()) throw std::invalid_argument("no default schemes node found.");
  ModuleSchemeList scheme_list;
  scheme_list.Decode(scheme_node);
  // reading the directory path for grading xml file output.
  if (!cfg_doc.isMember(kServiceConfigJsonKeyGradingPath))
    throw std::invalid_argument("no grading KPI directory specified.");
  boost::filesystem::path p(data_dir), sp(sys_dir);
  p /= cfg_doc[kServiceConfigJsonKeyGradingPath].asString();
  sp /= cfg_doc[kServiceConfigJsonKeyGradingPath].asString();
  grading_kpi_writer_.reset(new GradingKpiXmlWriter(p.lexically_normal().string()));
  global_grading_kpi_file_path_ = (p.parent_path() / kGlobalGradingKpiFileName).lexically_normal().string();
  default_global_grading_kpi_file_path_ = (sp.parent_path() / kGlobalGradingKpiFileName).lexically_normal().string();

  UpsertDefaultModules();
  UpsertDefaultSchemes(scheme_list);
  AddDefaultSysConfigsIfNotExist();
  GetDefaultGradingKpis((sys_dir / kFileNameDefaultGradingKpi).string());
  if (p != sp) {
    // we need also copy the default grading kpi file from the user data dir into the default sys dir.
    const auto& dgp = default_kpi_group_.file_path;
    boost::filesystem::copy_file(p / dgp, sp / dgp, boost::filesystem::copy_options::overwrite_existing);
  }
  m_module_log_directory = cfg_doc.get("moduleLogDirectory", "").asString();
}

std::vector<ModuleConfig> ConfigManager::GetDefaultLaunchModuleConfigs() {
  return default_config_.default_launch_module_configs;
}

//! @brief 函数名：GetActiveModuleSchemeCategory
//! @details 函数功能：获取活动模块方案类别
//!
//! @return 返回一个包含活动模块方案类别的字符串
//!
//! @note 该函数获取活动模块方案类别，并将结果存储在args中。
std::string ConfigManager::GetActiveModuleSchemeCategory() {
  std::string active_module_catetory_str;
  ExecSqliteRoutine(
      TXSIM_SQL_GET_ACTIVE_MODULE_CATEGORY,
      [](void* module_list_str_ptr, int column_count, char** columns, char** column_names) -> int {
        if (columns[0]) {
          auto* module_list_str = static_cast<std::string*>(module_list_str_ptr);
          module_list_str->assign(columns[0]);
        }
        return 0;
      },
      &active_module_catetory_str);
  return active_module_catetory_str;
}

//! @brief 函数名：LoadCoordinatorConfig
//! @details 函数功能：加载协调器配置
//!
//! @param[in] config 一个包含协调器配置信息的CoordinatorConfig对象
//!
//! @note 该函数加载协调器配置，并将结果存储在args中。
void ConfigManager::LoadCoordinatorConfig(CoordinatorConfig& config) {
  LOG(INFO) << "loading coordinator configurations...";

  config.module_configs.clear();
  std::vector<std::tuple<std::string, std::string>> active_modules;
  std::map<std::string, ModuleScheme> multiego_modules;
  GetActiveModuleNames(active_modules);
  std::string module_scheme_category = GetActiveModuleSchemeCategory();
  GetActiveMultiEgoSchemes(multiego_modules);
  if (!multiego_modules.empty()) {
    GetSchemeMultiEgoModuleConfigs(multiego_modules, config.module_configs);

  } else {
    GetSchemeModuleConfigs(active_modules, config.module_configs);
  }
  // if (config.module_configs.size() == 0) throw std::runtime_error("no active module configuration found.");
  LOG(INFO) << "loaded " << config.module_configs.size() << " active module configurations: ["
            << GetModuleNameListString(config.module_configs) << "]";

  GetSysConfigs(config);
  WriteGlobalGradingKpiFile();

  config.log2world_config = default_config_.log2world_config;
  config.log_perf = default_config_.log_perf;

  std::unordered_map<std::string, std::string> map_active_modules;
  for (const auto& mapItem : multiego_modules) {
    std::string moduleName;
    std::string modulePriority;
    std::string sGroup = mapItem.first;
    for (const auto& vecItem : mapItem.second.modules) {
      std::tie(moduleName, modulePriority) = vecItem;
      if (sGroup == "Scene") {
        // global module
        map_active_modules[moduleName] = modulePriority;
      } else {
        map_active_modules[sGroup + "-" + moduleName] = modulePriority;
      }
    }
  }
  if (config.coord_mode == tx_sim::impl::CoordinationMode::kCoordinatePriority) {
    for (auto& item : config.module_configs) {
      std::string priorityTime = map_active_modules[item.name];
      item.m_nPriority = std::stoi(priorityTime);
    }
  }

  for (auto& item : config.module_configs) { item.module_scheme_category = module_scheme_category; }

  config.m_module_log_directory = m_module_log_directory;
}

//! @brief 函数名：LoadModuleConfig
//! @details 函数功能：加载模块配置
//!
//! @param[in] config 一个包含模块配置信息的ModuleConfig对象
//!
//! @return 返回一个布尔值，表示加载模块配置是否成功
//!
//! @note 该函数加载模块配置，并将结果存储在args中。
bool ConfigManager::LoadModuleConfig(ModuleConfig& config) {
  std::string name(config.name);
  try {
    GetModuleConfig(name, config);
  } catch (const std::exception& e) {
    LOG(ERROR) << "error when loading log play module: " << e.what();
    return false;
  }
  return !config.name.empty();
}

//! @brief 函数名：Execute
//! @details 函数功能：执行配置管理器命令
//!
//! @param[in] cmd 一个包含命令信息的Command对象
//! @param[in] input 一个包含输入信息的zmq::multipart_t对象
//! @param[out] output 一个包含输出信息的zmq::multipart_t对象
//!
//! @return 返回一个CmdErrorCode，表示执行命令是否成功
//!
//! @note 该函数执行配置管理器命令，并将结果存储在args中。
CmdErrorCode ConfigManager::Execute(Command cmd, zmq::multipart_t& input, zmq::multipart_t& output) {
  // 打印日志，显示正在执行的命令
  LOG(INFO) << "executing config manager cmd " << Enum2String(cmd);
  // 初始化错误码为成功
  CmdErrorCode ec = kCmdSucceed;
  // 根据命令类型进行处理
  switch (cmd) {
    // 添加模块配置
    case kCmdAddModuleConfig: {
      ModuleConfig c;
      c.DecodeFromStr(PopMsgStr(input));
      AddModuleConfig(c);
      break;
    }
      // 删除模块配置
    case kCmdRemoveModuleConfig: {
      RemoveModuleConfig(PopMsgStr(input));
      break;
    }
      // 更新模块配置
    case kCmdUpdateModuleConfig: {
      ModuleConfig c;
      c.DecodeFromStr(PopMsgStr(input));
      UpdateModuleConfig(c);
      break;
    }
      // 获取模块配置
    case kCmdGetModuleConfig: {
      ModuleConfig c;
      std::string name(PopMsgStr(input));
      GetModuleConfig(name, c);
      output.addstr(c.EncodeToStr());
      break;
    }
      // 获取所有模块配置
    case kCmdGetAllModuleConfigs: {
      std::vector<ModuleConfig> configs;
      GetAllModuleConfig(configs);
      uint8_t buf[10];
      AddMsgSize(buf, configs.size(), output);
      for (const ModuleConfig& c : configs) { output.addstr(c.EncodeToStr()); }
      break;
    }
      // 添加模组配置
    case kCmdAddModuleScheme: {
      ModuleScheme s;
      s.DecodeFromStr(PopMsgStr(input));
      AddModuleScheme(s);
      break;
    }
    // 删除模组配置
    case kCmdRemoveModuleScheme: {
      RemoveModuleScheme(PopMsgInt64(input));
      break;
    }
    // 更新模组配置
    case kCmdUpdateModuleScheme: {
      ModuleScheme s;
      s.DecodeFromStr(PopMsgStr(input));
      UpdateModuleScheme(s);
      break;
    }
    // 获取所有模组配置
    case kCmdGetAllModuleScheme: {
      ModuleSchemeList sl;
      GetAllModuleSchemes(sl);
      output.addstr(sl.EncodeToStr());
      break;
    }
    // 设置当前运行模组配置
    case kCmdUpdateMultiEgoScheme: {
      ModuleMultiEgoScheme sl;
      sl.DecodeFromStr(PopMsgStr(input));
      UpdateMultiEgoScheme(sl);
      break;
    }
    case kCmdGetMultiEgoScheme: {
      ModuleMultiEgoScheme sl;
      GetMultiEgoScheme(sl);
      output.addstr(sl.EncodeToStr());
      break;
    }
    case kCmdSetActiveModuleScheme: {
      SetActiveModuleScheme(PopMsgInt64(input));
      break;
    }
    // 导出模组配置
    case kCmdExportModuleScheme: {
      const std::string name(PopMsgStr(input));
      const std::string out_path(PopMsgStr(input));
      ExportModuleScheme(name, out_path);
      break;
    }
    // 导入模组配置
    case kCmdImportModuleScheme: {
      const std::string in_path(PopMsgStr(input));
      std::string imported_scheme_name;
      if (!ImportModuleScheme(in_path, imported_scheme_name)) ec = kCmdSchemeAlreadyExist;
      output.addstr("{\"importedSchemeName\":\"" + imported_scheme_name + "\"}");
      break;
    }
    // 设置仿真系统配置
    case kCmdSetSysConfigs: {
      CoordinatorConfig config;
      config.DecodeFromStr(PopMsgStr(input));
      SetSysConfigs(config);
      break;
    }
    // 获取仿真系统配置
    case kCmdGetSysConfigs: {
      CoordinatorConfig config;
      GetSysConfigs(config);
      output.addstr(config.EncodeToStr());
      break;
    }
    // 恢复默认仿真系统配置
    case kCmdRestoreDefaultConfigs: {
      RestoreDefault();
      break;
    }
    // 更新播放列表
    case kCmdUpdatePlayList: {
      const std::string play_list(PopMsgStr(input));
      UpdatePlayList(play_list);
      break;
    }
    // 获取播放列表
    case kCmdGetPlayList: {
      std::string play_list;
      GetPlayList(play_list);
      output.addstr(play_list);
      break;
    }
    // 移除场景
    case kCmdRemoveScenariosFromPlayList: {
      std::string removing_list(PopMsgStr(input));
      RemoveScenariosFromPlayList(removing_list);
      break;
    }
    // 添加评测指标
    case kCmdAddGradingKpi: {
      GradingKpiList kpis;
      kpis.DecodeFromStr(PopMsgStr(input));
      AddGradingKpis(kpis);
      break;
    }
    // 移除评测指标
    case kCmdRemoveGradingKpi: {
      RemoveGradingKpi(PopMsgInt64(input));
      break;
    }
    // 更新评测指标
    case kCmdUpdateGradingKpi: {
      GradingKpi kpi;
      kpi.DecodeFromStr(PopMsgStr(input));
      UpdateGradingKpi(kpi);
      break;
    }
    // 获取评测组内评测指标
    case kCmdGetGradingKpisInGroup: {
      GradingKpiList kpis;
      kpis.group_id = PopMsgInt64(input);
      GetGradingKpisInGroup(kpis);
      output.addstr(kpis.EncodeToStr());
      break;
    }
    // 添加评测组内评测指标
    case kCmdAddGradingKpiGroup: {
      GradingKpiGroup kpi_group;
      kpi_group.DecodeFromStr(PopMsgStr(input));
      AddGradingKpiGroup(kpi_group);
      break;
    }
    // 移除评测组内评测指标
    case kCmdRemoveGradingKpiGroup: {
      RemoveGradingKpiGroup(PopMsgInt64(input));
      break;
    }
    // 更新评测组内评测指标
    case kCmdUpdateGradingKpiGroup: {
      GradingKpiGroup kpi_group;
      kpi_group.DecodeFromStr(PopMsgStr(input));
      UpdateGradingKpiGroup(kpi_group);
      break;
    }
    // 获取所有评测组内评测指标
    case kCmdGetAllGradingKpiGroups: {
      GradingKpiGroupList groups;
      GetAllGradingKpiGroups(groups);
      output.addstr(groups.EncodeToStr());
      break;
    }
    // 设置评测组内默认评测指标
    case kCmdSetDefaultGradingKpiGroup: {
      SetDefaultGradingKpiGroup(PopMsgInt64(input));
      break;
    }
    // 设置评测指标标签
    case kCmdSetGradingKpiLabels: {
      std::string sLabels = PopMsgStr(input);
      UpdateKpiNameCategoryMapping(sLabels);
      break;
    }
    // 获取protobuf信息
    case kCmdGetPbInfo: {
      std::string sBody = PopMsgStr(input);
      std::string outputStr;
      GetPbInfo(sBody, outputStr);
      output.addstr(outputStr);
      break;
    }
    // 删除protobuf信息
    case kCmdDelPbInfo: {
      std::string sBody = PopMsgStr(input);
      return DelPbInfo(sBody);
    }
    // 更新protobuf信息
    case kCmdUpdatePbInfo: {
      std::string sBody = PopMsgStr(input);
      std::string outputStr;
      CmdErrorCode rc = UpdatePbInfo(sBody, outputStr);
      output.addstr(outputStr);
      return rc;
    }
    default: LOG(ERROR) << "unknown config command: " << cmd; ec = kCmdSystemError;
  }
  return ec;
}

//! @brief 函数名：GetPbInfo
//! @details 函数功能：获取PB信息
//!
//! @param[in] intput 一个包含输入信息的字符串
//! @param[out] output 一个包含输出信息的字符串
//!
//! @note 该函数根据输入的信息，获取PB信息，并将结果存储在output中。
void ConfigManager::GetPbInfo(const std::string& intput, std::string& output) {
  std::string sFiled;
  if (!intput.empty()) {
    std::unordered_map<std::string, std::string> jsonMap;
    Json2Map(intput, jsonMap);
    sFiled = jsonMap["field"];
  }
  if (!boost::filesystem::exists(m_pbinfo_sys_path)) return;
  Json::Value jsonPbInfo;
  ReadJsonDocFromFile(jsonPbInfo, m_pbinfo_sys_path);
  if (!jsonPbInfo.isArray()) return;

  Json::Value jsonPbInfoData;
  if (boost::filesystem::exists(m_pbinfo_data_path)) {
    ReadJsonDocFromFile(jsonPbInfoData, m_pbinfo_data_path);
    for (const auto& item : jsonPbInfoData) { jsonPbInfo.append(item); }
  }
  for (const auto item : jsonPbInfo) {
    std::string keyFiled = item.get("field", "").asString();
    if (!sFiled.empty() && keyFiled == sFiled) {
      WriteJsonDocToString(item, output);
      return;
    }
  }
  WriteJsonDocToString(jsonPbInfo, output);
}

//! @brief 函数名：DelPbInfo
//! @details 函数功能：删除PB信息
//!
//! @param[in] intput 一个包含输入信息的字符串
//!
//! @return 返回一个CmdErrorCode，表示执行命令是否成功
//!
//! @note 该函数根据输入的信息，删除PB信息。
CmdErrorCode ConfigManager::DelPbInfo(const std::string& intput) {
  std::unordered_map<std::string, std::string> jsonMap;
  Json2Map(intput, jsonMap);
  std::string sFiled = jsonMap["field"];
  if (sFiled.empty()) return kCmdFailed;
  if (!boost::filesystem::exists(m_pbinfo_data_path)) return kCmdFailed;
  Json::Value jsonPbInfo;
  ReadJsonDocFromFile(jsonPbInfo, m_pbinfo_data_path);
  if (!jsonPbInfo.isArray()) return kCmdFailed;
  for (int i = 0; i < jsonPbInfo.size(); i++) {
    Json::Value item = jsonPbInfo[i];
    std::string keyFiled = item.get("field", "").asString();
    if (keyFiled != sFiled) continue;
    bool preset = item.get("preset", false).asBool();
    if (preset) return kCmdFailed;
    Json::Value delValue;
    jsonPbInfo.removeIndex(i, &delValue);
    WriteJsonDocToFile(jsonPbInfo, m_pbinfo_data_path);
    return kCmdAccepted;
  }
  return kCmdFailed;
}

//! @brief 函数名：UpdatePbInfo
//! @details 函数功能：更新PB信息
//!
//! @param[in] intput 一个包含输入信息的字符串
//! @param[out] output 一个包含输出信息的字符串
//!
//! @return 返回一个CmdErrorCode，表示执行命令是否成功
//!
//! @note 该函数根据输入的信息，更新PB信息，并将结果存储在output中。
CmdErrorCode ConfigManager::UpdatePbInfo(const std::string& intput, std::string& output) {
  Json::Value jsonInput;
  ReadJsonDocFromString(jsonInput, intput);
  std::string sSourceFiled = jsonInput["source_field"].asString();
  std::string sFiled = jsonInput["field"].asString();
  jsonInput.removeMember("source_field");
  if (sFiled.empty()) return kCmdFailed;

  Json::Value jsonPbInfo = Json::arrayValue;
  if (boost::filesystem::exists(m_pbinfo_data_path)) { ReadJsonDocFromFile(jsonPbInfo, m_pbinfo_data_path); }

  if (!jsonPbInfo.isArray()) return kCmdFailed;
  bool nFind = false;
  if (!sSourceFiled.empty()) {
    for (int i = 0; i < jsonPbInfo.size(); i++) {
      Json::Value item = jsonPbInfo[i];
      std::string keyFiled = item.get("field", "").asString();
      if (keyFiled != sSourceFiled) continue;
      bool preset = item.get("preset", false).asBool();
      if (preset) return kCmdFailed;

      Json::Value delValue;
      jsonPbInfo[i] = jsonInput;
      nFind = true;
      break;
    }
  }

  if (!nFind) { jsonPbInfo.append(jsonInput); }

  WriteJsonDocToString(jsonInput, output);
  WriteJsonDocToFile(jsonPbInfo, m_pbinfo_data_path);
  return kCmdAccepted;
}

//! @brief 函数名：UpdateKpiNameCategoryMapping
//! @details 函数功能：更新KPI名称和类别映射
//!
//! @param[in] sLebels 一个包含KPI名称和类别映射信息的字符串
//!
//! @note 该函数根据输入的KPI名称和类别映射信息，更新内部的映射关系。
void ConfigManager::UpdateKpiNameCategoryMapping(std::string sLebels) {
  Json::Value jsonValue;
  ReadJsonDocFromString(jsonValue, sLebels);
  std::string sName = jsonValue.get("name", "").asString();
  Json::Value lebelArray = jsonValue["labels"];
  std::string sMapValue = kpi_name_category_mapping_[sName];
  if (sMapValue.empty()) return;
  Json::Value mapValue;
  ReadJsonDocFromString(mapValue, sMapValue);
  mapValue["labels"] = lebelArray;
  std::string newCateGoryString;
  WriteJsonDocToString(mapValue, newCateGoryString);
  kpi_name_category_mapping_[sName] = newCateGoryString;  // update kpi_name_category_mapping_

  for (auto& defaultGroupKpiItem : default_group_kpis_.kpis) {
    if (defaultGroupKpiItem.name != sName) continue;
    defaultGroupKpiItem.category = newCateGoryString;
    break;
  }

  for (auto& defaultKpiItem : default_kpis_.kpis) {
    if (defaultKpiItem.name != sName) continue;
    defaultKpiItem.category = newCateGoryString;
    break;
  }

  GradingKpiGroupList groups;
  GetAllGradingKpiGroups(groups);
  for (const auto& groupItem : groups.groups) {
    GradingKpiList kpis;
    kpis.group_id = groupItem.id;
    GetGradingKpisInGroup(kpis);
    grading_kpi_writer_->WriteGradingKpiConfigFile(kpis);
  }
  WriteGlobalGradingKpiFile();
  UpdateDefaultGradingKPIs(sName, newCateGoryString);
  // m_grading_labels_path
  // kpi_name_category_mapping_
}

//! @brief 函数名：UpdateDefaultGradingKPIs
//! @details 函数功能：更新默认评分KPIs
//!
//! @param[in] name 一个包含KPI名称的字符串
//! @param[in] newCateGoryString 一个包含新的KPI类别信息的字符串
//!
//! @note 该函数根据输入的KPI名称和新的KPI类别信息，更新默认评分KPIs。
void ConfigManager::UpdateDefaultGradingKPIs(std::string name, std::string newCateGoryString) {
  Json::Value root;
  ReadJsonDocFromFile(root, m_default_grading_kpi_file_path);
  std::unordered_map<std::string, size_t> kpi_idx_mapping;
  Json::Value& kpi_list_node = root["kpi"];
  for (Json::Value& kpi_node : kpi_list_node) {
    std::string sNodeName = kpi_node["name"].asString();
    if (sNodeName != name) continue;
    Json::Value newCateGoryNode;
    ReadJsonDocFromString(newCateGoryNode, newCateGoryString);
    kpi_node["category"] = newCateGoryNode;
    break;
  }
  WriteJsonDocToFile(root, m_default_grading_kpi_file_path);
  // m_default_grading_kpi_file_path
  // default_group_kpis_
}

//! @brief 函数名：AddModuleConfig
//! @details 函数功能：添加模块配置
//!
//! @param[in] config 一个包含模块配置信息的ModuleConfig对象
//!
//! @note 该函数根据输入的模块配置信息，添加一个新的模块配置。
void ConfigManager::AddModuleConfig(const ModuleConfig& config) {
  ExecSqliteRoutineWithAllocatedSqlStr(
      GetSqlStatementWithModuleConfig(TXSIM_SQL_INSERT_MODULE_CONFIG, config, kSqlStateInsert));
  LOG(INFO) << "added module " << config.name << " configuration.";
}

//! @brief 函数名：RemoveModuleConfig
//! @details 函数功能：删除模块配置
//!
//! @param[in] name 一个包含模块名称的字符串
//!
//! @note 该函数根据输入的模块名称，删除对应的模块配置。
void ConfigManager::RemoveModuleConfig(const std::string& name) {
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_DELETE_MODULE_CONFIG, name.c_str()));
  // also remove the module item in all referenced module scheme.
  // TODO(nemo): we should split the module_scheme table into 2 seperate ones.
  ModuleSchemeList scheme_list;
  GetAllModuleSchemes(scheme_list);
  for (ModuleScheme& scheme : scheme_list.schemes) {
    bool dirty = false;
    for (auto it = scheme.modules.begin(); it != scheme.modules.end(); ++it)
      if (std::get<0>(*it) == name) {
        scheme.modules.erase(it);
        dirty = true;
        break;
      }
    if (dirty) UpdateModuleScheme(scheme);
  }
  LOG(INFO) << "removed module configuration: " << name;
}

// 配置管理器的更新模块配置功能
// @param config 需要更新的模块配置对象
void ConfigManager::UpdateModuleConfig(const ModuleConfig& config) {
  // 执行SQLite数据库中的SQL语句
  // @param statement 需要执行的SQL语句
  // @param config 需要更新的模块配置对象
  // @param sqlState 需要执行的SQL操作状态，如更新、插入、删除等
  ExecSqliteRoutineWithAllocatedSqlStr(
      GetSqlStatementWithModuleConfig(TXSIM_SQL_UPDATE_MODULE_CONFIG, config, kSqlStateUpdate));

  // 记录更新模块配置后的日志信息
  // @param config 更新后的模块配置对象
  // @param logLevel 日志等级，如INFO、WARNING、ERROR等
  LOG(INFO) << "updated module configuration: " << config.name;
}

// 获取指定模块的配置信息
// @param name 模块名称
// @param config 存储模块配置信息的对象
void ConfigManager::GetModuleConfig(const std::string& name, ModuleConfig& config) {
  // 初始化模块配置对象
  config.name.clear();

  // 使用预定义的SQL语句从数据库中查询模块配置信息
  // @param sql_str 包含SQL查询语句的指针
  ExecSqliteRoutineWithAllocatedSqlStr(
      sqlite3_mprintf(TXSIM_SQL_GET_MODULE_CONFIG, name.c_str()),
      [](void* module_config_ptr, int column_count, char** columns, char** column_names) -> int {
        // 将从数据库中查询到的模块配置信息填充到模块配置对象中
        ModuleConfig* cfg = static_cast<ModuleConfig*>(module_config_ptr);
        GetModuleConfigFromDB(*cfg, columns);
        return 0;
      },
      &config);
  // 如果查询成功，则输出日志提示
  if (!config.name.empty()) { LOG(INFO) << "got module " << name << " configuration."; }
}

//! @brief 函数名：GetAllModuleConfig
//! @details 函数功能：获取所有模块配置
//!
//! @param[out] configs 一个包含模块配置的向量，用于存储获取到的所有模块配置
//!
//! @note 该函数获取所有模块配置，并将其存储在输出参数configs中。
void ConfigManager::GetAllModuleConfig(std::vector<ModuleConfig>& configs) {
  configs.clear();
  std::vector<ModuleConfig> module_cfg_list;
  ExecSqliteRoutine(TXSIM_SQL_GET_ALL_MODULE_CONFIGS, module_config_get_cb_, &module_cfg_list);
  // TODO(nemo): temporary processing: return configs list in an order that active module comes first.
  // removing this once the "module category" feature of front-end UI is ready.
  std::vector<std::tuple<std::string, std::string>> active_module_list;
  GetActiveModuleNames(active_module_list);
  std::unordered_map<std::string, const ModuleConfig*> module_cfgs;
  for (const auto& m : module_cfg_list) module_cfgs[m.name] = &m;
  for (const auto& m : active_module_list) {
    auto it = module_cfgs.find(std::get<0>(m));
    if (it != module_cfgs.end()) {
      configs.emplace_back();
      configs.back() = *it->second;
      module_cfgs.erase(it);
    }
  }
  for (const auto& module_name_cfg_pair : module_cfgs) {
    configs.emplace_back();
    configs.back() = *module_name_cfg_pair.second;
  }
  // removing above when front-end UI is ready for the "module category" feature.
  LOG(INFO) << "got " << configs.size() << " module configurations in total.";
}

/*!
 * @brief 函数名：AddModuleScheme
 * @details 函数功能：添加模块方案
 *
 * @param[in] scheme 一个包含模块方案信息的结构体
 */
void ConfigManager::AddModuleScheme(const ModuleScheme& scheme) {
  std::vector<std::tuple<std::string, std::string>> tupleModules = scheme.modules;
  std::vector<std::string> vecStr;
  for (auto& item : scheme.modules) {
    std::string str = std::get<0>(item) + ":" + std::get<1>(item);
    vecStr.emplace_back(str);
  }
  std::string module_list_str(boost::join(vecStr, kSeparatorModuleList));
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_ADD_MODULE_SCHEME, scheme.name.c_str(),
                                                       module_list_str.empty() ? nullptr : module_list_str.c_str(),
                                                       scheme.category.empty() ? "" : scheme.category.c_str()));
  LOG(INFO) << "added module scheme " << scheme.name;
}

/*!
 * @brief 函数名：RemoveModuleScheme
 * @details 函数功能：移除模块方案
 *
 * @param[in] scheme_id 要移除的模块方案的ID
 */
void ConfigManager::RemoveModuleScheme(int64_t scheme_id) {
  if (scheme_id == kDefaultActiveModuleSchemeId)
    throw std::invalid_argument("default active module scheme cannot be deleted.");

  // if the current activated scheme is being deleted, reset the active scheme id to default 0.
  int64_t active_scheme_id = GetConfigID(TXSIM_SQL_GET_ACTIVE_SCHEME_ID);
  if (scheme_id == active_scheme_id) {  // reset the active scheme to the last one which id is less than the currently
                                        // deleted one's.
    LOG(INFO) << "deleting currently activated scheme, try to reset other one ...";
    int64_t reset_id = GetLastIDLessThan(TXSIM_SQL_GET_LAST_SCHEME_ID, scheme_id);
    if (reset_id == kInvalidId) {
      LOG(WARNING) << "no scheme id is less then the currently deleted one's(" << scheme_id
                   << "). no active scheme is reset.";
    } else {
      SetActiveModuleScheme(reset_id);
      LOG(INFO) << "reset active scheme id to " << reset_id;
    }
  }

  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_REMOVE_MODULE_SCHEME, scheme_id));
  LOG(INFO) << "removed module scheme " << scheme_id;
}

//! @brief 函数名：UpdateModuleScheme
//! @details 函数功能：更新模组配置
//!
//! @param[in] scheme 需要更新的模块方案对象
//! @param[out]
//!
//! @note 该函数根据输入的模块方案对象，更新对应的模块方案。
void ConfigManager::UpdateModuleScheme(const ModuleScheme& scheme) {
  std::vector<std::tuple<std::string, std::string>> tupleModules = scheme.modules;
  std::vector<std::string> vecStr;
  for (auto& item : scheme.modules) {
    std::string str = std::get<0>(item) + ":" + std::get<1>(item);
    vecStr.emplace_back(str);
  }

  std::string module_list_str(boost::join(vecStr, kSeparatorModuleList));
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(
      TXSIM_SQL_UPDATE_MODULE_SCHEME, scheme.name.c_str(), module_list_str.empty() ? nullptr : module_list_str.c_str(),
      scheme.category.empty() ? "" : scheme.category.c_str(), scheme.id));
  LOG(INFO) << "updated module scheme " << scheme.name;
}

//! @brief 函数名：GetAllModuleSchemes
//! @details 函数功能：获取所有模组配置
//!
//! @param[in] scheme_list 存储所有模块方案的列表
//!
//! @param[out] scheme_list 存储所有模块方案的列表
//!
//! @return
//!
//! @note 该函数获取所有模块方案，并将其存储在输出参数scheme_list中。
void ConfigManager::GetAllModuleSchemes(ModuleSchemeList& scheme_list) {
  ExecSqliteRoutine(
      TXSIM_SQL_GET_ALL_MODULE_SCHEMES,
      [](void* scheme_list_ptr, int column_count, char** columns, char** column_names) -> int {
        auto& schemes = *static_cast<ModuleSchemeList*>(scheme_list_ptr);
        schemes.schemes.emplace_back();
        auto& scheme = schemes.schemes.back();
        scheme.id = std::stoi(columns[0]);
        scheme.name = columns[1];
        std::vector<std::string> tmpVecStr;
        // scheme.modules
        if (columns[2]) boost::split(tmpVecStr, columns[2], boost::is_any_of(kSeparatorModuleList));
        for (auto itemStr : tmpVecStr) {
          size_t nPos = itemStr.find(":");
          std::string name;
          std::string priority;
          if (nPos == std::string::npos) {
            name = itemStr;
            priority = kDefaultModulePriorityLevelString;
          } else {
            name = itemStr.substr(0, nPos);
            priority = itemStr.substr(nPos + 1);
          }
          scheme.modules.emplace_back(std::make_tuple(name, priority));
        }
        if (std::stoi(columns[3]) == 1) schemes.active_id = scheme.id;
        if (columns[4]) scheme.category = columns[4];
        return 0;
      },
      &scheme_list);
  /* unused code
    ExecSqliteRoutine(
        TXSIM_SQL_GET_ALL_MULTIEGO_SCHEME,
        [](void* scheme_list_ptr, int column_count, char** columns, char** column_names) -> int {
          auto& schemes = *static_cast<ModuleSchemeList*>(scheme_list_ptr);
          std::string group_name = columns[0];
          int64_t scheme_id = std::stoi(columns[1]);
          int64_t group_type = std::stoi(columns[2]);
          if (group_type == 0) {
              schemes.active_scene_id = scheme_id;
          } else {
              schemes.active_multiego[group_name] = scheme_id;
          }
          return 0;
        },
        &scheme_list);
  */

  LOG(INFO) << "got " << scheme_list.schemes.size() << " module schemes in total.";
}

void ConfigManager::UpdateMultiEgoScheme(const ModuleMultiEgoScheme& multiego_scheme) {
  sqlite3_str* sql_scheme_upsert_seq = sqlite3_str_new(db_);
  {
    char* sql_stat = sqlite3_mprintf(TXSIM_SQL_RESET_MULTIEGO_SCHEME);
    sqlite3_str_appendall(sql_scheme_upsert_seq, sql_stat);
    sqlite3_free(sql_stat);
  }

  for (const auto& s : multiego_scheme.active_multiego) {
    // if (s.id > 0) throw std::invalid_argument("the id of the system default scheme should NOT greater then 0.");
    char* sql_stat = sqlite3_mprintf(TXSIM_SQL_UPSERT_MULTIEGO_SCHEME, s.first.c_str(), s.second, 1, s.second, 1);
    sqlite3_str_appendall(sql_scheme_upsert_seq, sql_stat);
    sqlite3_free(sql_stat);
  }
  {
    char* sql_stat = sqlite3_mprintf(TXSIM_SQL_UPSERT_MULTIEGO_SCHEME, "Scene", multiego_scheme.active_scene_id, 0,
                                     multiego_scheme.active_scene_id, 0);
    sqlite3_str_appendall(sql_scheme_upsert_seq, sql_stat);
    sqlite3_free(sql_stat);
  }
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_str_finish(sql_scheme_upsert_seq));
}

void ConfigManager::GetMultiEgoScheme(ModuleMultiEgoScheme& multiego_scheme) {
  ExecSqliteRoutine(
      TXSIM_SQL_GET_ALL_MULTIEGO_SCHEME,
      [](void* schemes_ptr, int column_count, char** columns, char** column_names) -> int {
        std::string name = columns[0];
        int64_t scheme_id = std::stoi(columns[1]);
        int64_t type = std::stoi(columns[2]);
        auto* schemes = static_cast<ModuleMultiEgoScheme*>(schemes_ptr);
        if (type == 0) {
          schemes->active_scene_id = scheme_id;
        } else {
          schemes->active_multiego[name] = scheme_id;
        }
        return 0;
      },
      &multiego_scheme);
}

//! @brief 函数名：SetActiveModuleScheme
//! @details 函数功能：设置当前激活的模组配置
//!
//! @param[in] scheme_id 需要设置为激活状态的模组的ID
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::SetActiveModuleScheme(int64_t scheme_id) {
  sqlite3_str* sql = sqlite3_str_new(db_);
  sqlite3_str_appendall(sql, TXSIM_SQL_CLEAR_ALL_MODULE_SCHEME_ACTIVES);
  if (scheme_id != 99999) { sqlite3_str_appendf(sql, TXSIM_SQL_SET_ACTIVE_MODULE_SCHEME, scheme_id); }
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_str_finish(sql));
  LOG(INFO) << "active module scheme: " << scheme_id;
}

//! @brief 函数名：LoadModuleConfigByName
//! @details 函数功能：通过模块方案名称获取模块配置
//!
//! @param[in] scheme_name 模块方案名称
//! @param[out] module_config 存储模块配置的变量
//!
//! @return 成功返回true，失败返回false
//!
//! @note
bool ConfigManager::LoadModuleConfigByName(std::string schemeName, std::vector<ModuleConfig>& module_cfgs) {
  ModuleScheme scheme;
  scheme.name = schemeName;
  if (!GetModuleSchemeByName(scheme)) return false;
  GetSchemeModuleConfigs(scheme.modules, module_cfgs);
  for (auto& item : module_cfgs) {
    if (tx_sim::impl::NeedChangeControl(item.category)) { item.module_scheme_name = schemeName; }
  }
  return true;
}

//! @brief 函数名：ExportModuleScheme
//! @details 函数功能：导出模组配置
//!
//! @param[in] scheme_name 模块方案名称
//! @param[in] out_path 导出文件路径
//!
//! @return
//!
//! @note
void ConfigManager::ExportModuleScheme(const std::string& scheme_name, const std::string& out_path) {
  ModuleScheme scheme;
  scheme.name = scheme_name;
  if (!GetModuleSchemeByName(scheme)) throw std::runtime_error("no scheme found: " + scheme_name);
  std::vector<ModuleConfig> module_cfgs;
  GetSchemeModuleConfigs(scheme.modules, module_cfgs);

  std::unordered_map<std::string, std::string> map_active_modules;
  for (auto& vecItem : scheme.modules) {
    std::string moduleName;
    std::string modulePriority;
    std::tie(moduleName, modulePriority) = vecItem;
    map_active_modules[moduleName] = modulePriority;
  }
  for (auto& item : module_cfgs) {
    std::string sModuleName = item.name;
    std::string sPriority = map_active_modules[sModuleName];
    item.m_nPriority = std::stoi(sPriority);
  }

  Json::Value root;
  root[kJsonKeyExportedSchemeName] = scheme.name;
  root[kJsonKeyExportedSchemeCategory] = scheme.category;
  Json::Value& modules_node = root[kJsonKeyExportedSchemeModules];
  modules_node = Json::arrayValue;
  for (const ModuleConfig& cfg : module_cfgs) cfg.Encode(modules_node.append(Json::Value()));
  WriteJsonDocToFile(root, out_path);
  LOG(INFO) << "scheme " << scheme_name << " exported to " << out_path;
}

//! @brief 函数名：ImportModuleScheme
//! @details 函数功能：导入模组配置
//!
//! @param[in] in_path 导入文件路径
//! @param[out] imported_name 导入的模块方案名称
//!
//! @return 成功返回true,失败返回false
//!
//! @note
bool ConfigManager::ImportModuleScheme(const std::string& in_path, std::string& imported_name) {
  Json::Value doc;
  ReadJsonDocFromFile(doc, in_path);
  std::string import_scheme_name = doc[kJsonKeyExportedSchemeName].asString();
  std::string import_scheme_category = doc[kJsonKeyExportedSchemeCategory].asString();
  std::vector<ModuleConfig> import_module_cfgs;
  const Json::Value& module_cfgs_node = doc[kJsonKeyExportedSchemeModules];
  for (const Json::Value& c : module_cfgs_node) {
    import_module_cfgs.emplace_back();
    import_module_cfgs.back().Decode(c);
  }

  ModuleScheme scheme;
  // returns true if the name of the imported scheme is already exist.
  // >>> below comments is commneted out. <<<
  // // check if the imported scheme is already existed. returns true if so.
  // // on returning false, there's 2 case:
  // // 1. there exists a scheme named scheme.name but the module configurations in which is not the same, then returns
  // //    false with a renamed scheme.name;
  // // 2. there has no existing scheme named scheme.name and returns false with the current scheme.name;
  // // so based above cases we need continue checking in a loop until it returns true, or false with the same
  // scheme.name.
  auto compare_imported_scheme_func = [this](ModuleScheme& scheme, const std::vector<ModuleConfig>& imported) -> bool {
    // if (!GetModuleSchemeByName(scheme)) return false;
    // if (scheme.modules.size() == imported.size()) {
    //   std::vector<ModuleConfig> existed_module_cfgs;
    //   GetSchemeModuleConfigs(scheme.modules, existed_module_cfgs);
    //   bool is_all_equals = false;
    //   for (size_t i = 0; i < existed_module_cfgs.size(); ++i) {
    //     is_all_equals = existed_module_cfgs[i].EqualsToImported(imported[i]);
    //     if (!is_all_equals) break;
    //   }
    //   if (is_all_equals) return true;
    // }
    // scheme.name = scheme.name + "-2";
    // return false;
    return GetModuleSchemeByName(scheme);
  };

  scheme.name = import_scheme_name;
  scheme.category = import_scheme_category;
  std::string last_tried_name = scheme.name;
  static const size_t max_tries_count = 10;
  size_t tries_count = 0;
  // TODO(nemo): probably need a more UI-er way(return a code and let user type a different scheme name)?
  for (tries_count = 0; tries_count < max_tries_count; ++tries_count) {
    if (compare_imported_scheme_func(scheme, import_module_cfgs)) {
      LOG(INFO) << "imported scheme " << scheme.name << " is already existed, import cancelled.";
      imported_name = scheme.name;
      return false;  // there's nothing we need to do since a exactly same scheme configuration is already exist.
    }
    if (scheme.name == last_tried_name) break;  // there's no existing scheme named scheme.name.
    last_tried_name = scheme.name;  // current scheme is confict with the imported one, try another scheme name.
  }
  if (tries_count == max_tries_count) throw std::runtime_error("too much confict scheme exists with the imported one.");

  // now we start to import the scheme ...
  // 1. record the originally importing module names in scheme.
  scheme.modules.clear();
  for (const ModuleConfig& c : import_module_cfgs)
    scheme.modules.emplace_back(std::make_tuple(c.name, std::to_string(c.m_nPriority)));

  // 2. construct a <module_name, module_config> map since we need to check if the imported module is already exist.
  std::vector<ModuleConfig> existed_modules_list;
  GetAllModuleConfig(existed_modules_list);
  std::unordered_map<std::string, size_t> existed_module_idxs;
  for (size_t i = 0; i < existed_modules_list.size(); ++i) existed_module_idxs[existed_modules_list[i].name] = i;

  // 3. check if the importing module is already exist. if not insert it(maybe with a renamed one).
  for (size_t i = 0; i < import_module_cfgs.size(); ++i) {
    ModuleConfig& imported = import_module_cfgs[i];
    auto it = existed_module_idxs.find(imported.name);
    if (it != existed_module_idxs.end()) {  // a module with the same name as the imported one is already existed,
                                            // comparing them two to see if we need to adding it.
      const ModuleConfig& existed = existed_modules_list[it->second];
      if (existed.EqualsToImported(imported)) continue;  // the imported one is same with the existed, skip this one.
      // the existed one is different from the imported, adding it with a renamed module name.
      imported.name = imported.name + "-" + scheme.name;  // since the current scheme.name is unique, the renamed module
                                                          // name should also be unique.
      scheme.modules[i] = std::make_tuple(
          imported.name,
          kDefaultModulePriorityLevelString);  // also record the renamed module name in the imported scheme.
      // if the same scheme is imported more than once, ignored even the renamed imported module config is different
      // from the previous one.
      if (existed_module_idxs.find(imported.name) != existed_module_idxs.end()) continue;
    }
    AddModuleConfig(imported);
  }

  // 4. finally adding the scheme which references imported/existed module configurations.
  AddModuleScheme(scheme);
  std::ostringstream ss;
  for (const std::tuple<std::string, std::string>& m : scheme.modules) ss << std::get<0>(m) << ", ";
  LOG(INFO) << "scheme " << scheme.name << " imported with modules: [" << ss.str() << "]";

  imported_name = scheme.name;
  return true;
}

//! @brief 函数名：SetSysConfigs
//! @details 函数功能：设置系统配置
//!
//! @param[in] config 系统配置结构体
//!
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::SetSysConfigs(const CoordinatorConfig& config) {
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_UPDATE_SYS_CONFIG, config.control_rate,
                                                       config.scenario_time_limit, config.coord_mode, config.auto_reset,
                                                       config.adding_initial_location_msg, config.override_user_log,
                                                       config.custom_grading_feedback_process.c_str()));
  LOG(INFO) << "set system configs with control_rate = " << config.control_rate
            << " scenario_time_limit = " << config.scenario_time_limit << " coord_mode = " << config.coord_mode
            << " auto_reset = " << config.auto_reset << " initial_location = " << config.adding_initial_location_msg
            << " override_user_log = " << config.override_user_log
            << " grading_feedback_process = " << config.custom_grading_feedback_process;
}

//! @brief 函数名：GetSysConfigs
//! @details 函数功能：获取系统配置
//!
//! @param[in]
//! @param[out] config 系统配置结构体
//!
//! @return
//!
//! @note
void ConfigManager::GetSysConfigs(CoordinatorConfig& config) {
  ExecSqliteRoutine(
      TXSIM_SQL_SELECT_SYS_CONFIG,
      [](void* data_ptr, int column_count, char** columns, char** column_names) -> int {
        auto& data = *static_cast<CoordinatorConfig*>(data_ptr);
        data.control_rate = std::stoi(columns[0]);
        data.scenario_time_limit = std::stoi(columns[1]);
        data.coord_mode = Int2CoordMode(std::stoi(columns[2]));
        data.auto_reset = (std::stoi(columns[3]) == 1);
        data.adding_initial_location_msg = (std::stoi(columns[4]) == 1);
        data.override_user_log = (std::stoi(columns[5]) == 1);
        if (columns[6]) data.custom_grading_feedback_process = columns[6];
        return 0;
      },
      &config);
  config.auto_stop = default_config_.auto_stop;
  LOG(INFO) << "got system configurations (control_rate=" << config.control_rate
            << ", scenario_time_limit=" << config.scenario_time_limit << ", coord_mode=" << config.coord_mode
            << ", auto_reset=" << config.auto_reset << ", auto_stop=" << default_config_.auto_stop
            << ", adding_initial_location_msg=" << config.adding_initial_location_msg
            << ", override_user_log=" << config.override_user_log
            << ", grading_feedback_process=" << config.custom_grading_feedback_process << ")";
}

//! @brief 函数名：UpdatePlayList
//! @details 函数功能：更新播放列表
//!
//! @param[in] play_list 播放列表
//!
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::UpdatePlayList(const std::string& play_list_str) {
  played_scenario_ids_.clear();
  Json::Value play_list;
  ReadJsonDocFromString(play_list, play_list_str);
  const Json::Value& id_list = play_list[kJsonKeyScenarioList];
  for (const auto& id : id_list) played_scenario_ids_.insert(id.asInt64());
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_UPDATE_PLAY_LIST, play_list_str.c_str()));
  LOG(INFO) << "updated play list with " << played_scenario_ids_.size() << " scenarios.";
}

//! @brief 函数名：GetPlayList
//! @details 函数功能：获取播放列表
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::GetPlayList(std::string& play_list_str) {
  play_list_str.clear();
  ExecSqliteRoutine(
      TXSIM_SQL_GET_PLAY_LIST,
      [](void* data_ptr, int column_count, char** columns, char** column_names) -> int {
        std::string* ids_str = static_cast<std::string*>(data_ptr);
        ids_str->assign(columns[0]);
        return 0;
      },
      &play_list_str);
  if (played_scenario_ids_.empty() && !play_list_str.empty()) {  // for the first loading. init the id cache.
    Json::Value play_list;
    ReadJsonDocFromString(play_list, play_list_str);
    const Json::Value& id_list = play_list[kJsonKeyScenarioList];
    for (const auto& id : id_list) played_scenario_ids_.insert(id.asInt64());
    LOG(INFO) << "got " << played_scenario_ids_.size() << " play list on start-up.";
  } else {
    LOG(INFO) << "got play list.";
  }
}

//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::RemoveScenariosFromPlayList(const std::string& list_to_be_removed_str) {
  std::unordered_set<int64_t> removed_ids;
  Json::Value list_to_be_removed;
  ReadJsonDocFromString(list_to_be_removed, list_to_be_removed_str);
  const Json::Value& ids_to_be_removed = list_to_be_removed[kJsonKeyScenarioList];
  for (const auto& id : ids_to_be_removed)
    if (played_scenario_ids_.erase(id.asInt64())) removed_ids.insert(id.asInt64());
  if (removed_ids.empty()) return;

  std::string play_list_str;
  GetPlayList(play_list_str);
  Json::Value current_play_list, updated_play_list;
  ReadJsonDocFromString(current_play_list, play_list_str);
  const Json::Value& current_ids = current_play_list[kJsonKeyScenarioList];
  Json::Value& updated_ids = updated_play_list[kJsonKeyScenarioList];
  for (const auto& id : current_ids)
    if (removed_ids.find(id.asInt64()) == removed_ids.end()) updated_ids.append(id);
  WriteJsonDocToString(updated_play_list, play_list_str);
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_UPDATE_PLAY_LIST, play_list_str.c_str()));
  LOG(INFO) << "removed " << removed_ids.size() << " scenarios from play list.";
}

//! @brief 函数名：AddGradingKpis
//! @details 函数功能：添加评测指标
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::AddGradingKpis(const GradingKpiList& kpi_list) {
  if (kpi_list.group_id <= 0) throw std::invalid_argument("the group id of added grading kpis must greater than 0.");
  int64_t gid = kpi_list.group_id;
  if (kpi_list.kpis.empty()) {
    LOG(INFO) << "adding an empty kpi list of group id " << gid << ", ignored.";
    return;
  }
  sqlite3_str* sql_seq = sqlite3_str_new(db_);
  for (const GradingKpi& kpi : kpi_list.kpis) {
    char* sql_stat = sqlite3_mprintf(TXSIM_SQL_ADD_KPI, gid, kpi.name.c_str(), kpi.parameters.c_str(),
                                     kpi.pass_condition.c_str(), kpi.finish_condition.c_str(), gid);
    sqlite3_str_appendall(sql_seq, sql_stat);
    sqlite3_free(sql_stat);
  }
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_str_finish(sql_seq));
  LOG(INFO) << "added " << kpi_list.kpis.size() << " KPIs with group id: " << gid;
}

//! @brief 函数名：RemoveGradingKpi
//! @details 函数功能：移除评测指标
//!
//! @param[in] kpi_id:指标id
//!
//! @param[out]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::RemoveGradingKpi(int64_t kpi_id) {
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_REMOVE_KPI, kpi_id, kpi_id));
  LOG(INFO) << "removed grading KPI(id: " << kpi_id << ").";
}

void ConfigManager::UpdateGradingKpi(const GradingKpi& kpi) {
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_UPDATE_KPI, kpi.id, kpi.parameters.c_str(),
                                                       kpi.pass_condition.c_str(), kpi.finish_condition.c_str(),
                                                       kpi.id));
  LOG(INFO) << "updated grading KPI(id: " << kpi.id << ").";
}

//! @brief 函数名：GetGradingKpisInGroup
//! @details 函数功能：获取评测指标项
//!
//! @param[in]
//! @param[out] kpi_list:指标项
//!
//!
//! @return
//!
//! @note
void ConfigManager::GetGradingKpisInGroup(GradingKpiList& kpi_list) {
  kpi_list.kpis.clear();
  if (kpi_list.group_id == kMockGroupIdWithAllDefaultGradingKpi) {
    kpi_list = default_kpis_;
    LOG(INFO) << "got all default grading KPIs.";
    return;
  }
  if (kpi_list.group_id == kDefaultGradingKpiGroupId) {
    kpi_list = default_group_kpis_;
    LOG(INFO) << "got default grading KPI group.";
    return;
  }
  ExecSqliteRoutineWithAllocatedSqlStr(
      sqlite3_mprintf(TXSIM_SQL_GET_ALL_KPI_IN_GROUP, kpi_list.group_id),
      [](void* kpis_ptr, int column_count, char** columns, char** column_names) -> int {
        auto& kpis = *static_cast<std::vector<GradingKpi>*>(kpis_ptr);
        kpis.emplace_back();
        GradingKpi& kpi = kpis.back();
        kpi.id = std::stoll(columns[0]);
        kpi.name = columns[1];
        kpi.parameters = columns[2];
        kpi.pass_condition = columns[3];
        kpi.finish_condition = columns[4];
        return 0;
      },
      &kpi_list.kpis);
  for (GradingKpi& kpi : kpi_list.kpis) kpi.category = kpi_name_category_mapping_[kpi.name];
  LOG(INFO) << "got " << kpi_list.kpis.size() << " grading KPIs in group " << kpi_list.group_id;
}

//! @brief 函数名：AddGradingKpiGroup
//! @details 函数功能：添加指标组到数据库中
//!
//! @param[in] kpi_group:指标组结构
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::AddGradingKpiGroup(const GradingKpiGroup& kpi_group) {
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_ADD_KPI_GROUP, kpi_group.name.c_str()));
  if (!kpi_group.kpis.empty()) {
    GradingKpiList kpi_list;
    kpi_list.group_id = sqlite3_last_insert_rowid(db_);
    kpi_list.kpis = kpi_group.kpis;
    AddGradingKpis(kpi_list);
  }
  LOG(INFO) << "added KPI group: " << kpi_group.name;
}

//! @brief 函数名：RemoveGradingKpiGroup
//! @details 函数功能：移除指标组
//!
//! @param[in] group_id:指标组id
//! @param[out]
//!
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::RemoveGradingKpiGroup(int64_t group_id) {
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_REMOVE_KPI_GROUP, group_id, group_id));
  WriteGlobalGradingKpiFile();  // there's a chance that the currently applied global kpi group is removed. so we need
                                // to update the global grading.xml file to the default kpi group.
  LOG(INFO) << "removed kPI group with id " << group_id;
}

//! @brief 函数名：UpdateGradingKpiGroup
//! @details 函数功能：更新指标组
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::UpdateGradingKpiGroup(const GradingKpiGroup& kpi_group) {
  ExecSqliteRoutineWithAllocatedSqlStr(
      sqlite3_mprintf(TXSIM_SQL_UPDATE_KPI_GROUP, kpi_group.name.c_str(), kpi_group.id));
  LOG(INFO) << "updated KPI group name(" << kpi_group.name << ") with id " << kpi_group.id;
}

//! @brief 函数名：GetAllGradingKpiGroups
//! @details 函数功能：获取所有指标组数据
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::GetAllGradingKpiGroups(GradingKpiGroupList& kpi_group_list) {
  kpi_group_list.groups.clear();
  kpi_group_list.groups.push_back(default_kpi_group_);   // adding the default KPI group first.
  kpi_group_list.active_id = kDefaultGradingKpiGroupId;  // also set the active id to the default group in case there's
                                                         // no user's KPI group actived.

  ApplyGradingKpiConfig();  // make sure all dirty kpi groups have file name updated.
  ExecSqliteRoutine(
      TXSIM_SQL_GET_ALL_KPI_GROUP,
      [](void* group_list_ptr, int column_count, char** columns, char** column_names) -> int {
        auto& group_list = *static_cast<GradingKpiGroupList*>(group_list_ptr);
        group_list.groups.emplace_back();
        GradingKpiGroup& kpi_group = group_list.groups.back();
        kpi_group.id = std::stoll(columns[0]);
        kpi_group.name = columns[1];
        if (std::stoi(columns[2])) group_list.active_id = kpi_group.id;
        kpi_group.file_path = columns[3];
        return 0;
      },
      &kpi_group_list);
  LOG(INFO) << "got " << kpi_group_list.groups.size() << " KPI groups.";
}

//! @brief 函数名：SetDefaultGradingKpiGroup
//! @details 函数功能：设置默认指标组
//!
//! @param[in] group_id:指标组id
//! @param[out]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::SetDefaultGradingKpiGroup(int64_t group_id) {
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_SET_ACTIVE_KPI_GROUP, group_id));
  WriteGlobalGradingKpiFile();
  LOG(INFO) << "set active default KPI group id: " << group_id;
}

//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::ApplyGradingKpiConfig() {
  std::vector<int64_t> dirty_kpi_group_ids;
  ExecSqliteRoutine(
      TXSIM_SQL_GET_DIRTY_KPI_GROUP_IDS,
      [](void* id_list_ptr, int column_count, char** columns, char** column_names) -> int {
        auto& id_list = *static_cast<std::vector<int64_t>*>(id_list_ptr);
        id_list.push_back(std::stoi(columns[0]));
        return 0;
      },
      &dirty_kpi_group_ids);
  if (dirty_kpi_group_ids.empty()) return;

  for (int64_t id : dirty_kpi_group_ids) {
    GradingKpiList kpis;
    kpis.group_id = id;
    GetGradingKpisInGroup(kpis);
    std::string file_name = grading_kpi_writer_->WriteGradingKpiConfigFile(kpis);
    ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(TXSIM_SQL_UPDATE_KPI_FILE_NAME, file_name.c_str(), id));
  }

  ExecSqliteRoutine(TXSIM_SQL_CLEAR_DIRTY_KPI_GROUPS);
}

//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::RestoreDefault() {
  if (!default_config_.module_configs.empty()) {  // restore all system modules to default.
    sqlite3_str* sql_seq = sqlite3_str_new(db_);
    for (const ModuleConfig& c : default_config_.module_configs) {
      char* sql_stat = GetSqlStatementWithModuleConfig(TXSIM_SQL_UPDATE_MODULE_CONFIG, c, kSqlStateUpdate);
      sqlite3_str_appendall(sql_seq, sql_stat);
      sqlite3_free(sql_stat);
    }
    ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_str_finish(sql_seq));
  }
  LOG(INFO) << "restore module configs to default: [" << GetModuleNameListString(default_config_.module_configs) << "]";
  // also restore the system settings.
  SetSysConfigs(default_config_);
  // TODO(nemo): resotre the active module set?
}

//! @brief 函数名：ExecSqliteRoutine
//! @details 函数功能：sql执行语句
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::ExecSqliteRoutine(const char* sql, SqliteExecCb cb, void* cb_arg) {
  char* err = nullptr;
  int ec = sqlite3_exec(db_, sql, cb, cb_arg, &err);
  if (err != nullptr) {
    std::string err_msg(err);
    sqlite3_free(err);
    throw std::runtime_error(err_msg);
  }
}

//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::ExecSqliteRoutineWithAllocatedSqlStr(char* sql_str, SqliteExecCb cb, void* cb_arg) {
  if (sql_str == nullptr) throw std::runtime_error("no enough memory for sql printf.");
  try {
    ExecSqliteRoutine(sql_str, cb, cb_arg);
  } catch (const std::exception& e) {
    sqlite3_free(sql_str);
    throw;
  }
  sqlite3_free(sql_str);
}

//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
size_t ConfigManager::GetConfigCount(const std::string& table_name) {
  size_t cnt = 0;
  ExecSqliteRoutine((TXSIM_SQL_GET_CONFIG_COUNT + table_name).c_str(),
                    [](void* count_ptr, int column_count, char** columns, char** column_names) -> int {
                      auto* count = static_cast<size_t*>(count_ptr);
                      *count = std::stoi(columns[0]);
                      return 0;
                    },
                    &cnt);
  LOG(INFO) << "found " << cnt << " " << table_name << " configurations.";
  return cnt;
}

//! @brief 函数名：GetConfigID
//! @details 函数功能：通过sql语句获取模组配置ID
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
int64_t ConfigManager::GetConfigID(const char* sql) {
  int64_t id = kInvalidId;
  ExecSqliteRoutine(
      sql,
      [](void* id_ptr, int column_count, char** columns, char** column_names) -> int {
        auto* id_value = static_cast<int64_t*>(id_ptr);
        *id_value = std::stoi(columns[0]);
        return 0;
      },
      &id);
  return id;
}

//! @brief 函数名：AddDefaultSysConfigsIfNotExist
//! @details 函数功能：添加默认的系统配置
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
// add default system configurations if there's not.
void ConfigManager::AddDefaultSysConfigsIfNotExist() {
  if (GetConfigCount(TXSIM_SQL_TABLE_NAME_SYS_CONFIG) > 0) return;
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_mprintf(
      TXSIM_SQL_INSERT_SYS_CONFIG, default_config_.control_rate, default_config_.scenario_time_limit,
      default_config_.coord_mode, default_config_.auto_reset, default_config_.adding_initial_location_msg,
      default_config_.override_user_log, default_config_.custom_grading_feedback_process.c_str()));
}

//! @brief 函数名：UpsertDefaultModules
//! @details 函数功能：更新和插入模组模组配置
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
// update/insert default module configurations, existing init args would not be updated.
void ConfigManager::UpsertDefaultModules() {
  if (default_config_.module_configs.empty()) return;
  sqlite3_str* sql_seq = sqlite3_str_new(db_);
  for (const ModuleConfig& c : default_config_.module_configs) {
    char* sql_stat = GetSqlStatementWithModuleConfig(TXSIM_SQL_UPSERT_MODULE_CONFIG, c, kSqlStateUpsert);
    sqlite3_str_appendall(sql_seq, sql_stat);
    sqlite3_free(sql_stat);
  }
  ExecSqliteRoutineWithAllocatedSqlStr(sqlite3_str_finish(sql_seq));
}

//! @brief 函数名：UpsertDefaultSchemes
//! @details 函数功能：更新和插入模组配置
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
// update/insert default module schemes.
void ConfigManager::UpsertDefaultSchemes(const ModuleSchemeList& scheme_list) {
  if (scheme_list.schemes.empty())
    throw std::invalid_argument(
        "no default module schemes specified. there must be at least one default scheme configuration exists.");

  size_t original_scheme_count = GetConfigCount(TXSIM_SQL_TABLE_NAME_MODULE_SCHEME);
  int64_t older_active_scheme_id = 0;
  if (original_scheme_count > 0) older_active_scheme_id = GetConfigID(TXSIM_SQL_GET_ACTIVE_SCHEME_ID);

  int64_t min_default_id = 0;  // default scheme id should always less or equal to 0.
  sqlite3_str* sql_scheme_upsert_seq = sqlite3_str_new(db_);
  ModuleScheme scheme;
  scheme.name = kGlobalSchemeName;
  bool bExistGlobalScheme = GetModuleSchemeByName(scheme);
  for (const ModuleScheme& s : scheme_list.schemes) {
    // if (s.id > 0) throw std::invalid_argument("the id of the system default scheme should NOT greater then 0.");
    if (original_scheme_count > 0 && s.id > 0 && s.id != 99999) continue;
    if (99999 == s.id && bExistGlobalScheme) continue;
    if (s.id < min_default_id) min_default_id = s.id;
    std::vector<std::tuple<std::string, std::string>> tupleModules = s.modules;
    std::vector<std::string> vecStr;
    for (auto& item : s.modules) {
      std::string str = std::get<0>(item) + ":" + std::get<1>(item);
      vecStr.emplace_back(str);
    }
    std::string module_list_str(boost::join(vecStr, kSeparatorModuleList));
    char* sql_stat = sqlite3_mprintf(TXSIM_SQL_UPSERT_MODULE_SCHEME, s.id, s.name.c_str(), module_list_str.c_str(),
                                     s.category.c_str(), s.name.c_str(), module_list_str.c_str());
    sqlite3_str_appendall(sql_scheme_upsert_seq, sql_stat);
    sqlite3_free(sql_stat);
  }
  int upsert_str_size = sqlite3_str_length(sql_scheme_upsert_seq);
  char* upsert_str = sqlite3_str_finish(sql_scheme_upsert_seq);
  if (upsert_str_size > 0) ExecSqliteRoutineWithAllocatedSqlStr(upsert_str);

  // if either there's no schemes before or the older active scheme id is not a valid default id any more(id > 0 means
  // the active scheme is a user defined one, we should not modify it), reset the the default scheme id.
  if (original_scheme_count == 0 || older_active_scheme_id < min_default_id)
    SetActiveModuleScheme(scheme_list.active_id);
}
//! @brief 函数名：GetDefaultGradingKpis
//! @details 函数功能：获取默认的评测指标
//!
//! @param[in] kpi_files 评测指标文件列表
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::GetDefaultGradingKpis(const std::string& kpi_file_path) {
  if (!boost::filesystem::exists(kpi_file_path)) {
    LOG(WARNING) << kpi_file_path << " not found. no kpi configuration set.";
    return;
  }
  m_default_grading_kpi_file_path = kpi_file_path;

  LOG(INFO) << "reading default KPI configuration: " << kpi_file_path;
  Json::Value kpi_cfg;
  ReadJsonDocFromFile(kpi_cfg, kpi_file_path);
  std::unordered_map<std::string, size_t> kpi_idx_mapping;
  size_t idx = 0;
  default_kpis_.group_id = kMockGroupIdWithAllDefaultGradingKpi;
  auto& kpi_list = default_kpis_.kpis;
  Json::Value& kpi_list_node = kpi_cfg["kpi"];

  for (const Json::Value& kpi_node : kpi_list_node) {
    kpi_list.emplace_back();
    auto& kpi = kpi_list.back();
    kpi.Decode(kpi_node);
    kpi_idx_mapping[kpi.name] = idx++;
    kpi_name_category_mapping_[kpi.name] = kpi.category;
  }
  LOG(INFO) << "added " << idx << " default grading KPI in total.";
  default_group_kpis_.group_id = kDefaultGradingKpiGroupId;
  auto& group_kpi_list = default_group_kpis_.kpis;
  Json::Value& kpi_group_node = kpi_cfg["defaultGroup"];
  for (const Json::Value& kpi_name : kpi_group_node)
    group_kpi_list.push_back(kpi_list[kpi_idx_mapping[kpi_name.asString()]]);
  LOG(INFO) << "added default grading KPI group with " << group_kpi_list.size() << " KPIs.";

  default_kpi_group_.id = kDefaultGradingKpiGroupId;
  default_kpi_group_.name = "default";
  // over-write the default grading kpi file.
  default_kpi_group_.file_path = grading_kpi_writer_->WriteGradingKpiConfigFile(default_group_kpis_);
  // also over-write the global grading.xml.
  WriteGlobalGradingKpiFile();
}

//! @brief 函数名：GetActiveModuleNames
//! @details 函数功能：获取当前设置的模组名称和模组内所有模块名称
//!
//! @param[in] modules 模组名称列表
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::GetActiveModuleNames(std::vector<std::tuple<std::string, std::string>>& modules) {
  std::vector<std::string> tmpmodules;

  std::string active_module_list_str;
  ExecSqliteRoutine(
      TXSIM_SQL_GET_ACTIVE_MODULE_LIST,
      [](void* module_list_str_ptr, int column_count, char** columns, char** column_names) -> int {
        if (columns[0]) {
          auto* module_list_str = static_cast<std::string*>(module_list_str_ptr);
          module_list_str->assign(columns[0]);
        }
        return 0;
      },
      &active_module_list_str);

  if (!active_module_list_str.empty()) {
    boost::split(tmpmodules, active_module_list_str, boost::is_any_of(kSeparatorModuleList));
  }
  modules.clear();
  for (std::string item : tmpmodules) {
    size_t nPos = item.find(":");
    if (nPos == std::string::npos) {
      modules.emplace_back(item, kDefaultModulePriorityLevelString);
    } else {
      std::string name = item.substr(0, nPos);
      std::string priority = item.substr(nPos + 1);
      modules.emplace_back(std::make_tuple(name, priority));
    }
  }
}

void ConfigManager::GetActiveMultiEgoSchemes(std::map<std::string, ModuleScheme>& multiego_modules) {
  std::vector<std::tuple<std::string, int, int>> schemes;
  ExecSqliteRoutine(
      TXSIM_SQL_GET_ALL_MULTIEGO_SCHEME,
      [](void* schemes_ptr, int column_count, char** columns, char** column_names) -> int {
        std::string name = columns[0];
        int64_t scheme_id = std::stoi(columns[1]);
        int64_t type = std::stoi(columns[2]);
        auto* schemes = static_cast<std::vector<std::tuple<std::string, int, int>>*>(schemes_ptr);
        schemes->emplace_back(name, scheme_id, type);
        return 0;
      },
      &schemes);

  for (int i = 0; i < schemes.size(); ++i) {
    std::string scheme_name = std::get<0>(schemes[i]);
    int64_t scheme_id = std::get<1>(schemes[i]);
    ModuleScheme scheme;
    scheme.name = scheme_name;
    scheme.id = scheme_id;
    GetModuleSchemeById(scheme);
    multiego_modules[scheme_name] = scheme;
  }
}

//! @brief 函数名：GetSchemeModuleSqlStr
//! @details 函数功能：通过sql语句获取模组配置
//!
//! @param[in] modules 模组名称列表
//! @param[out]
//!
//! @return sql语句
//!
//! @note 返回的sql语句是字符串指针类型
// for a active module scheme, constructs following sqlite str:
// SELECT module_config_column_names FROM module_config WHERE name in (module_foo, module_bar, module_baz ...)
// ORDER BY CASE name WHEN module_foo THEN 1 WHEN module_bar THEN 2 WHEN module_baz THEN 3 ... END
char* ConfigManager::GetSchemeModuleSqlStr(const std::vector<std::tuple<std::string, std::string>>& modules) {
  sqlite3_str* sql = sqlite3_str_new(db_);
  sqlite3_str_appendall(sql, TXSIM_SQL_GET_ACTIVE_MODULES);
  sqlite3_str_appendchar(sql, 1, '(');
  for (size_t i = 0; i < modules.size(); ++i) {
    if (i > 0) sqlite3_str_appendchar(sql, 1, ',');
    std::string modulestr = std::get<0>(modules[i]);
    sqlite3_str_appendf(sql, "%Q", modulestr.c_str());
  }
  sqlite3_str_appendchar(sql, 1, ')');
  sqlite3_str_appendall(sql, " ORDER BY CASE name ");
  for (size_t i = 0; i < modules.size(); ++i) {
    std::string modulestr = std::get<0>(modules[i]);
    sqlite3_str_appendf(sql, "WHEN %Q THEN %d ", modulestr.c_str(), (i + 1));
  }
  sqlite3_str_appendall(sql, " END;");

  return sqlite3_str_finish(sql);
}

//! @brief 函数名：GetModuleSchemeByName
//! @details 函数功能：通过名称获取模组配置
//!
//! @param[in] scheme 模组配置结构，参数带模组名称
//! @param[out] scheme 模组配置结构
//!
//!
//! @return
//!
//! @note
bool ConfigManager::GetModuleSchemeByName(ModuleScheme& scheme) {
  scheme.id = kInvalidId;
  scheme.modules.clear();
  ExecSqliteRoutineWithAllocatedSqlStr(
      sqlite3_mprintf(TXSIM_SQL_GET_MODULE_SCHEME, scheme.name.c_str()),
      [](void* scheme_ptr, int column_count, char** columns, char** column_names) -> int {
        auto& s = *static_cast<ModuleScheme*>(scheme_ptr);
        std::vector<std::string> tmpVec;
        s.id = std::stoll(columns[0]);
        if (columns[1] && strlen(columns[1])) {
          boost::split(tmpVec, columns[1], boost::is_any_of(kSeparatorModuleList));
          for (std::string& item : tmpVec) {
            size_t nPos = item.find(":");
            if (nPos == std::string::npos) {
              s.modules.emplace_back(item, kDefaultModulePriorityLevelString);
            } else {
              std::string name = item.substr(0, nPos);
              std::string priority = item.substr(nPos + 1);
              s.modules.emplace_back(std::make_tuple(name, priority));
            }
          }
        }
        if (columns[2] && strlen(columns[2])) { s.category = columns[2]; }
        return 0;
      },
      &scheme);
  if (scheme.id == kInvalidId) LOG(ERROR) << "no module scheme found with name " << scheme.name;
  else
    LOG(INFO) << "got module scheme " << scheme.name << " with id " << scheme.id << " and " << scheme.modules.size()
              << " modules.";
  return scheme.id != kInvalidId;
}

bool ConfigManager::GetModuleSchemeById(ModuleScheme& scheme) {
  scheme.modules.clear();
  ExecSqliteRoutineWithAllocatedSqlStr(
      sqlite3_mprintf(TXSIM_SQL_GET_MODULE_SCHEME_BY_ID, scheme.id),
      [](void* scheme_ptr, int column_count, char** columns, char** column_names) -> int {
        auto& s = *static_cast<ModuleScheme*>(scheme_ptr);
        std::vector<std::string> tmpVec;
        s.id = std::stoll(columns[0]);
        if (columns[1] && strlen(columns[1])) {
          boost::split(tmpVec, columns[1], boost::is_any_of(kSeparatorModuleList));
          for (std::string& item : tmpVec) {
            size_t nPos = item.find(":");
            if (nPos == std::string::npos) {
              s.modules.emplace_back(item, kDefaultModulePriorityLevelString);
            } else {
              std::string name = item.substr(0, nPos);
              std::string priority = item.substr(nPos + 1);
              s.modules.emplace_back(std::make_tuple(name, priority));
            }
          }
        }
        return 0;
      },
      &scheme);
  if (scheme.id == kInvalidId) LOG(ERROR) << "no module scheme found with name " << scheme.name;
  else
    LOG(INFO) << "got module scheme " << scheme.name << " with id " << scheme.id << " and " << scheme.modules.size()
              << " modules.";
  return scheme.id != kInvalidId;
}
//! @brief 函数名：GetSchemeModuleConfigs
//! @details 函数功能：获取模组配置
//!
//! @param[in] module_names 模组名称列表
//! @param[out] configs 模组配置列表
//!
//! @return
//!
//! @note
void ConfigManager::GetSchemeModuleConfigs(const std::vector<std::tuple<std::string, std::string>>& module_names,
                                           std::vector<ModuleConfig>& configs) {
  if (module_names.empty()) return;
  ExecSqliteRoutineWithAllocatedSqlStr(GetSchemeModuleSqlStr(module_names), module_config_get_cb_, &configs);
}

void ConfigManager::GetSchemeMultiEgoModuleConfigs(const std::map<std::string, ModuleScheme>& schemes,
                                                   std::vector<ModuleConfig>& configs) {
  for (const auto& scheme : schemes) {
    if (scheme.second.modules.empty()) continue;
    std::vector<ModuleConfig> sub_configs;
    ExecSqliteRoutineWithAllocatedSqlStr(GetSchemeModuleSqlStr(scheme.second.modules), module_config_get_cb_,
                                         &sub_configs);
    if (scheme.second.name != "Scene") {
      for (int i = 0; i < sub_configs.size(); ++i) {
        sub_configs[i].module_group_name = scheme.second.name;
        sub_configs[i].name = makeGroupModuelName(scheme.second.name, sub_configs[i].name);
      }
    }
    configs.insert(configs.end(), sub_configs.begin(), sub_configs.end());
  }
}

//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
int64_t ConfigManager::GetLastIDLessThan(const char* sql, int64_t upper_id) {
  int64_t ret_id = kInvalidId;
  ExecSqliteRoutineWithAllocatedSqlStr(
      sqlite3_mprintf(sql, upper_id),
      [](void* id_ptr, int column_count, char** columns, char** column_names) -> int {
        auto& id = *static_cast<int64_t*>(id_ptr);
        id = std::stoll(columns[0]);
        return 0;
      },
      &ret_id);
  return ret_id;
}
//! @brief 函数名：WriteGlobalGradingKpiFile
//! @details 函数功能：把全局模组配置写进评测配置文件里面
//!
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ConfigManager::WriteGlobalGradingKpiFile() {
  GradingKpiGroupList kpi_groups;
  GetAllGradingKpiGroups(kpi_groups);
  std::string copied_kpi_file_path = grading_kpi_writer_->GetGradingKpiFilePath(kpi_groups.active_id);
  if (boost::filesystem::exists(copied_kpi_file_path)) {
    boost::filesystem::copy_file(copied_kpi_file_path, global_grading_kpi_file_path_,
                                 boost::filesystem::copy_options::overwrite_existing);
    if (global_grading_kpi_file_path_ != default_global_grading_kpi_file_path_)
      // also sync the global kpi file into the sys default dir ...
      boost::filesystem::copy_file(global_grading_kpi_file_path_, default_global_grading_kpi_file_path_,
                                   boost::filesystem::copy_options::overwrite_existing);
  } else {
    LOG(WARNING) << "kpi file [" << copied_kpi_file_path << "] not exists. no global grading kpi file copied.";
  }
}

}  // namespace coordinator
}  // namespace tx_sim
