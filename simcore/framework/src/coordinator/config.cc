// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "config.h"

#include "boost/filesystem.hpp"
#include "utils/constant.h"
#include "utils/json_helper.h"

//!
//! @brief 静态常量字符串，表示模块配置的键名
//!
static const std::string kJsonKeyModuleConfigs = "configs";
//!
//! @brief 静态常量字符串，表示默认启动模块的键名
//!
static const std::string kJonsKeyDefaultLaunch = "defaultLaunch";
//!
//! @brief 静态常量字符串，表示模块名称的键名
//!
static const std::string kJsonKeyModuleName = "name";
//!
//! @brief 静态常量字符串，表示模块执行周期的键名
//!
static const std::string kJsonKeyModuleExecPeriod = "execPeriod";
//!
//! @brief 静态常量字符串，表示模块响应时间的键名
//!
static const std::string kJsonKeyModuleRespTime = "responseTime";
//!
//! @brief 静态常量字符串，表示模块初始化参数的键名
//!
static const std::string kJsonKeyModuleInitArgs = "initArgs";
//!
//! @brief 静态常量字符串，表示模块命令超时时间的键名
//!
static const std::string kJsonKeyModuleCmdTimeout = "cmdTimeout";
//!
//! @brief 静态常量字符串，表示模块步骤超时时间的键名
//!
static const std::string kJsonKeyModuleStepTimeout = "stepTimeout";
//!
//! @brief 静态常量字符串，表示模块自动启动的键名
//!
static const std::string kJsonKeyModuleAutoLaunch = "autoLaunch";
//!
//! @brief 静态常量字符串，表示模块依赖路径的键名
//!
static const std::string kJsonKeyModuleDepPaths = "depPaths";
//!
//! @brief 静态常量字符串，表示模块动态库路径的键名
//!
static const std::string kJsonKeyModuleSoPath = "soPath";
//!
//! @brief 静态常量字符串，表示模块可执行文件路径的键名
//!
static const std::string kJsonKeyModuleBinPath = "binPath";
//!
//! @brief 静态常量字符串，表示模块可执行文件参数的键名
//!
static const std::string kJsonKeyModuleBinArgs = "binArgs";
//!
//! @brief 静态常量字符串，表示模块类别的键名
//!
static const std::string kJsonKeyModuleCategory = "category";
//!
//! @brief 静态常量字符串，表示模块影子ID的键名
//!
static const std::string kJsonKeyModuleShadowId = "shadowId";
//!
//! @brief 静态常量字符串，表示模块优先级的键名
//!
static const std::string kJsonKeyModulePriorityLevel = "priority";
//!
//! @brief 静态常量字符串，表示模块访问模式的键名
//!
static const std::string kJsonKeyModuleAccessMode = "accessMode";
//!
//! @brief 静态常量字符串，表示控制速率的键名
//!
static const std::string kJsonKeyControlRate = "playControlRate";
//!
//! @brief 静态常量字符串，表示场景时间限制的键名
//!
static const std::string kJsonKeyScenarioLimit = "scenarioTimeLimit";
//!
//! @brief 静态常量字符串，表示协同模式的键名
//!
static const std::string kJsonKeyCoordinateMode = "coordinationMode";
//!
//! @brief 静态常量字符串，表示自动重置的键名''
//!
static const std::string kJsonKeyPlayerAutoReset = "autoReset";
//!
//! @brief 静态常量字符串，表示自动停止的键名
//!
static const std::string kJsonKeyPlayerAutoStop = "autoStop";
//!
//! @brief 静态常量字符串，表示添加初始位置消息的键名
//!
static const std::string kJsonKeyAddingInitialLocationMsg = "addingInitialLocationMsg";
//!
//! @brief 静态常量字符串，表示性能日志记录的键名
//!
static const std::string kJsonKeyEnablePerfLogging = "perfLogging";
//!
//! @brief 静态常量字符串，表示评分反馈过程的键名
//!
static const std::string kJsonKeyGradingFeedbackProcess = "gradingFeedbackProcess";
//!
//! @brief 静态常量字符串，表示覆盖用户日志的键名
//!
static const std::string kJsonKeyOverrideUserLog = "overrideUserLog";
//!
//! @brief 静态常量字符串，表示日志转换为世界坐标的键名
//!
static const std::string kJsonKeyLog2World = "log2world";
//!
//! @brief 静态常量字符串，表示日志模拟主题配置的键名
//!
static const std::string kJsonKeyLogsimTopicConfig = "logsimConfig";
//!
//! @brief 静态常量字符串，表示世界模拟主题配置的键名
//!
static const std::string kJsonKeyWorldsimTopicConfig = "worldsimConfig";
//!
//! @brief 静态常量字符串，表示日志模拟自主切换的键名
//!
static const std::string kJsonKeyL2wEgoSwitch = "egoSwitch";
//!
//! @brief 静态常量字符串，表示日志模拟交通切换的键名
//!
static const std::string kJsonKeyL2wTrafficSwitch = "trafficSwitch";
//!
//! @brief 静态常量字符串，表示日志模拟切换时间的键名
//!
static const std::string kJsonKeyL2wSwitchTime = "switchTime";
//!
//! @brief 静态常量字符串，表示日志模拟切换类型的键名
//!
static const std::string kJsonKeyL2wSwitchType = "switchType";
//!
//! @brief 静态常量字符串，表示日志模拟事件时间戳的键名
//!
static const std::string kJsonKeyLogsimEventTimestamp = "ts";
//!
//! @brief 静态常量字符串，表示日志模拟事件类型的键名
//!
static const std::string kJsonKeyLogsimEventType = "type";
//!
//! @brief 静态常量字符串，表示日志模拟事件开始时间的键名
//!
static const std::string kJsonKeyLogsimEventInfoStartTime = "startTimestamp";
//!
//! @brief 静态常量字符串，表示日志模拟事件总持续时间的键名
//!
static const std::string kJsonKeyLogsimEventInfoTotalDuration = "duration";
//!
//! @brief 静态常量字符串，表示日志模拟事件列表的键名
//!
static const std::string kJsonKeyLogsimEventInfoEvents = "events";
//!
//! @brief 静态常量字符串，表示方案ID的键名
//!
static const std::string kJsonKeySchemeId = "id";
//!
//! @brief 静态常量字符串，表示方案名称的键名
//!
static const std::string kJsonKeySchemeName = "name";
//!
//! @brief 静态常量字符串，表示方案类别的键名
//!
static const std::string kJsonKeySchemeCategory = "category";
//!
//! @brief 静态常量字符串，表示方案模块列表的键名
//!
static const std::string kJsonKeySchemeModules = "modules";
//!
//! @brief 静态常量字符串，表示方案列表活跃ID的键名
//!
static const std::string kJsonKeySchemeListActiveId = "active";
static const std::string kJsonKeySchemeListActiveMultiEgoId = "active_multiego";
//!
//! @brief 静态常量字符串，表示方案列表的键名
//!
static const std::string kJsonKeySchemeListSchemes = "schemes";
//!
//! @brief 静态常量字符串，表示KPI ID的键名
//!
static const std::string kJsonKeyKpiId = "id";
//!
//! @brief 静态常量字符串，表示KPI名称的键名
//!
static const std::string kJsonKeyKpiName = "name";
//!
//! @brief 静态常量字符串，表示KPI类别的键名
//!
static const std::string kJsonKeyKpiCategory = "category";
//!
//! @brief 静态常量字符串，表示KPI参数的键名
//!
static const std::string kJsonKeyKpiParams = "parameters";
//!
//! @brief 静态常量字符串，表示KPI通过条件的键名
//!
static const std::string kJsonKeyKpiPassCond = "passCondition";
//!
//! @brief 静态常量字符串，表示KPI完成条件的键名
//!
static const std::string kJsonKeyKpiFinishCond = "finishCondition";
//!
//! @brief 静态常量字符串，表示KPI列表组ID的键名
//!
static const std::string kJsonKeyKpiListGroupId = "groupId";
//!
//! @brief 静态常量字符串，表示KPI列表的键名
//!
static const std::string kJsonKeyKpiListKpis = "kpis";
//!
//! @brief 静态常量字符串，表示KPI组ID的键名
//!
static const std::string kJsonKeyKpiGroupId = "id";
//!
//! @brief 静态常量字符串，表示KPI组名称的键名
//!
static const std::string kJsonKeyKpiGroupName = "name";
//!
//! @brief 静态常量字符串，表示KPI组文件路径的键名
//!
static const std::string kJsonKeyKpiGroupFilePath = "filePath";
//!
//! @brief 静态常量字符串，表示KPI组列表活跃ID的键名
//!
static const std::string kJsonKeyKpiGroupListActiveId = "activeId";
//!
//! @brief 静态常量字符串，表示KPI组列表的键名
//!
static const std::string kJsonKeyKpiGroupListGroups = "groups";

using namespace tx_sim::impl;
using namespace tx_sim::utils;

namespace tx_sim {
namespace coordinator {

//! @brief 函数名:EncodeToStr
//! @details 函数功能:
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
std::string JsonConfig::EncodeToStr() const {
  Json::Value root;
  Encode(root);
  return Json::writeString(Json::StreamWriterBuilder(), root);
}

//! @brief 函数名:DecodeFromStr
//! @details 函数功能: 把json字符串解析成JsonConfig对象
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void JsonConfig::DecodeFromStr(const std::string& json_str) {
  Json::Value root;
  ReadJsonDocFromString(root, json_str);
  Decode(root);
}

//! @brief 函数名:
//! @details 函数功能: 重载==操作符
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
bool ConnectionConfig::operator==(const ConnectionConfig& rhs) const {
  return cmd_timeout == rhs.cmd_timeout && step_timeout == rhs.step_timeout;
}

//! @brief 函数名:Encode
//! @details 函数功能: ModuleConfig编码程json格式
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ModuleConfig::Encode(Json::Value& root) const {
  root[kJsonKeyModuleName] = name;
  root[kJsonKeyModuleExecPeriod] = execute_period;
  root[kJsonKeyModuleRespTime] = response_time;
  Json::Value& init_args_node = root[kJsonKeyModuleInitArgs];
  init_args_node = Json::objectValue;  // to explicitly construct a empty {} when there's no init args.
  for (const std::pair<std::string, std::string>& kv : init_args) init_args_node[kv.first] = kv.second;
  root[kJsonKeyModuleCmdTimeout] = conn_args.cmd_timeout;
  root[kJsonKeyModuleStepTimeout] = conn_args.step_timeout;
  root[kJsonKeyModuleAutoLaunch] = auto_launch;
  Json::Value& dep_paths_node = root[kJsonKeyModuleDepPaths];
  dep_paths_node = Json::arrayValue;  // to explicitly construct a empty [] when there's no dep paths.
  for (const std::string& p : dep_paths) dep_paths_node.append(p);
  root[kJsonKeyModuleSoPath] = so_path;
  root[kJsonKeyModuleBinPath] = bin_path;
  Json::Value& bin_args_node = root[kJsonKeyModuleBinArgs];
  bin_args_node = Json::arrayValue;  // to explicitly construct a empty [] when there's no bin args.
  for (const std::string& p : bin_args) bin_args_node.append(p);
  root[kJsonKeyModuleCategory] = category;
  root[kJsonKeyModuleShadowId] = shadow_id;
  root[kJsonKeyModulePriorityLevel] = m_nPriority;
}

//! @brief 函数名:decode
//! @details 函数功能: ModuleConfig从Json格式转换成ModuleConfig结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ModuleConfig::Decode(const Json::Value& root) {
  name = root[kJsonKeyModuleName].asString();
  execute_period = root[kJsonKeyModuleExecPeriod].asUInt();
  response_time = root[kJsonKeyModuleRespTime].asUInt();
  const Json::Value& init_arg_map = root[kJsonKeyModuleInitArgs];
  std::vector<std::string> keys = init_arg_map.getMemberNames();
  for (const std::string& k : keys) init_args[k] = init_arg_map[k].asString();
  conn_args.cmd_timeout = root[kJsonKeyModuleCmdTimeout].asUInt();
  conn_args.step_timeout = root[kJsonKeyModuleStepTimeout].asUInt();
  auto_launch = root[kJsonKeyModuleAutoLaunch].asBool();
  const Json::Value& dep_path_list = root[kJsonKeyModuleDepPaths];
  for (const Json::Value& p : dep_path_list) dep_paths.push_back(p.asString());
  so_path = root[kJsonKeyModuleSoPath].asString();
  bin_path = root[kJsonKeyModuleBinPath].asString();
  const Json::Value& bin_arg_list = root[kJsonKeyModuleBinArgs];
  for (const Json::Value& p : bin_arg_list) bin_args.push_back(p.asString());
  category = Int2ModuleCategory(root.get(kJsonKeyModuleCategory, kModuleCategoryUnknown).asInt());
  shadow_id = root.get(kJsonKeyModuleShadowId, 0).asInt();
  m_nPriority = root.get(kJsonKeyModulePriorityLevel, 50).asInt();
  m_accessMode = root[kJsonKeyModuleAccessMode].asString();
}

//! @brief 函数名:
//! @details 函数功能: 重载ModuleConfig==操作符
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
bool ModuleConfig::operator==(const ModuleConfig& rhs) const {
  return name == rhs.name && execute_period == rhs.execute_period && response_time == rhs.response_time &&
         init_args == rhs.init_args && conn_args.cmd_timeout == rhs.conn_args.cmd_timeout &&
         conn_args.step_timeout == rhs.conn_args.step_timeout && auto_launch == rhs.auto_launch &&
         dep_paths == rhs.dep_paths && so_path == rhs.so_path && bin_path == rhs.bin_path && bin_args == rhs.bin_args &&
         category == rhs.category && shadow_id == rhs.shadow_id;
}

// rational here: since the imported module configuration may come from a different machine, so below configs are
// considered as "functional-careless":
// 1. all connection parameters.
// 2. all program-launching-related parameters.
// if category is different, it may be a totally different function module which accidently has the same configs.
bool ModuleConfig::EqualsToImported(const ModuleConfig& imported) const {
  return name == imported.name && execute_period == imported.execute_period &&
         response_time == imported.response_time && init_args == imported.init_args && category == imported.category &&
         shadow_id == imported.shadow_id;
}

//! @brief 函数名:encode
//! @details 函数功能: ModuleScheme结构转换成json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ModuleScheme::Encode(Json::Value& root) const {
  root[kJsonKeySchemeId] = id;
  root[kJsonKeySchemeName] = name;
  Json::Value& modules_node = root[kJsonKeySchemeModules];
  modules_node = Json::arrayValue;  // to explicitly construct a empty [] when there's no modules.
  for (const auto& m : modules) {
    std::string sName;
    std::string sValue;
    std::tie(sName, sValue) = m;
    Json::Value jsonValue;
    jsonValue["name"] = sName;
    jsonValue["priority"] = sValue;
    modules_node.append(jsonValue);
  }
  root[kJsonKeySchemeCategory] = category;
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成ModuleScheme结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ModuleScheme::Decode(const Json::Value& root) {
  id = root[kJsonKeySchemeId].asInt64();
  name = root[kJsonKeySchemeName].asString();
  const Json::Value& module_names = root[kJsonKeySchemeModules];
  for (const Json::Value& m : module_names) {
    if (m.isObject()) {
      const Json::Value jsonName = m["name"];
      const Json::Value jsonPrority = m["priority"];
      std::string n = jsonPrority.asString();
      modules.emplace_back(std::make_tuple(jsonName.asString(), jsonPrority.asString()));
    } else {
      assert(true);
      std::string s = m.asString();
      modules.emplace_back(std::make_tuple(m.asString(), kDefaultModulePriorityLevelString));
    }
  }
  category = root[kJsonKeySchemeCategory].asString();
}

void ModuleMultiEgoScheme::Encode(Json::Value& root) const {
  Json::Value& multiego_node = root[kJsonKeySchemeListActiveMultiEgoId];
  multiego_node = Json::arrayValue;
  Json::Value scene_scheme_node;
  scene_scheme_node["group_name"] = "Scene";
  scene_scheme_node["group_type"] = 0;  // 全局
  scene_scheme_node["scheme_id"] = active_scene_id;
  multiego_node.append(scene_scheme_node);
  for (const auto& s : active_multiego) {
    Json::Value ego_scheme_node;
    ego_scheme_node["group_name"] = s.first;
    ego_scheme_node["group_type"] = 1;  // 主车模块
    ego_scheme_node["scheme_id"] = s.second;
    multiego_node.append(ego_scheme_node);
  }
}

void ModuleMultiEgoScheme::Decode(const Json::Value& root) {
  const Json::Value& multiego_node = root[kJsonKeySchemeListActiveMultiEgoId];
  if (!multiego_node.isNull()) {
    for (const Json::Value& s : multiego_node) {
      const std::string group_name = s["group_name"].asString();
      int64_t scheme_id = s["scheme_id"].asInt64();
      int64_t group_type = s["group_type"].asInt64();
      if (group_type == 0) {
        active_scene_id = scheme_id;
      } else {
        active_multiego[group_name] = scheme_id;
      }
    }
  }
}

//! @brief 函数名:
//! @details 函数功能:ModuleSchemeList结构转换成json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ModuleSchemeList::Encode(Json::Value& root) const {
  root[kJsonKeySchemeListActiveId] = active_id;
  Json::Value& schemes_node = root[kJsonKeySchemeListSchemes];
  for (const ModuleScheme& s : schemes) s.Encode(schemes_node.append(Json::Value()));
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成ModuleSchemeList结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void ModuleSchemeList::Decode(const Json::Value& root) {
  active_id = root[kJsonKeySchemeListActiveId].asInt64();
  const Json::Value& schemes_node = root[kJsonKeySchemeListSchemes];
  if (schemes_node.isNull()) return;
  for (const Json::Value& s : schemes_node) {
    schemes.emplace_back();
    schemes.back().Decode(s);
  }
}

//! @brief 函数名:
//! @details 函数功能:Log2WorldPlayConfig结构转换成json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void Log2WorldTopicConfig::Encode(Json::Value& root) const {
  for (const auto& topic_mode_pair : logsim_topic_configs)
    root[kJsonKeyLogsimTopicConfig][topic_mode_pair.first] = static_cast<int16_t>(topic_mode_pair.second);
  for (const auto& topic_mode_pair : worldsim_topic_configs)
    root[kJsonKeyWorldsimTopicConfig][topic_mode_pair.first] = static_cast<int16_t>(topic_mode_pair.second);
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成Log2WorldPlayConfig结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void Log2WorldTopicConfig::Decode(const Json::Value& root) {
  const Json::Value& logsim_cfg_node = root[kJsonKeyLogsimTopicConfig];
  std::vector<std::string> topics = logsim_cfg_node.getMemberNames();
  for (const std::string& t : topics) logsim_topic_configs[t] = Int2TopicMode(logsim_cfg_node[t].asInt());
  const Json::Value& worldsim_cfg_node = root[kJsonKeyWorldsimTopicConfig];
  topics = worldsim_cfg_node.getMemberNames();
  for (const std::string& t : topics) worldsim_topic_configs[t] = Int2TopicMode(worldsim_cfg_node[t].asInt());
}

//! @brief 函数名:
//! @details 函数功能:Log2WorldPlayConfig结构转换成json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void Log2WorldPlayConfig::Encode(Json::Value& root) const {
  root[kJsonKeyL2wEgoSwitch] = ego_switch;
  root[kJsonKeyL2wTrafficSwitch] = traffic_switch;
  root[kJsonKeyL2wSwitchTime] = switch_time_ms;
  root[kJsonKeyL2wSwitchType] = static_cast<int16_t>(switch_type);
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成Log2WorldPlayConfig结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void Log2WorldPlayConfig::Decode(const Json::Value& root) {
  ego_switch = root[kJsonKeyL2wEgoSwitch].asBool();
  traffic_switch = root[kJsonKeyL2wTrafficSwitch].asBool();
  switch_time_ms = root[kJsonKeyL2wSwitchTime].asInt();
  switch_type = Int2Log2WorldSwitchType(root[kJsonKeyL2wSwitchType].asInt());
}

//! @brief 函数名:
//! @details 函数功能:重载operator==操作符
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
bool Log2WorldPlayConfig::operator==(const Log2WorldPlayConfig& rhs) const {
  return ego_switch == rhs.ego_switch && traffic_switch == rhs.traffic_switch && switch_time_ms == rhs.switch_time_ms &&
         switch_type == rhs.switch_type;
}

//! @brief 函数名:
//! @details 函数功能:logsimEvent转换成json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void LogsimEvent::Encode(Json::Value& root) const {
  root[kJsonKeyLogsimEventTimestamp] = timestamp_ms;
  root[kJsonKeyLogsimEventType] = type;
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成LogsimEvent结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void LogsimEvent::Decode(const Json::Value& root) {
  timestamp_ms = root[kJsonKeyLogsimEventTimestamp].asInt64();
  type = root[kJsonKeyLogsimEventType].asInt();
}

//! @brief 函数名:
//! @details 函数功能:LogsimEventInfo转换成json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void LogsimEventInfo::Encode(Json::Value& root) const {
  root[kJsonKeyLogsimEventInfoStartTime] = start_timestamp_ms;
  root[kJsonKeyLogsimEventInfoTotalDuration] = total_duration_ms;
  Json::Value& events_node = root[kJsonKeyLogsimEventInfoEvents];
  events_node = Json::arrayValue;
  for (const LogsimEvent& e : events) e.Encode(events_node.append(Json::Value()));
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成LogsimEventInfo结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void LogsimEventInfo::Decode(const Json::Value& root) {
  start_timestamp_ms = root[kJsonKeyLogsimEventInfoStartTime].asInt64();
  total_duration_ms = root[kJsonKeyLogsimEventInfoTotalDuration].asUInt64();
  const Json::Value& events_node = root[kJsonKeyLogsimEventInfoEvents];
  for (const Json::Value& e : events_node) {
    events.emplace_back();
    events.back().Decode(e);
  }
}

//! @brief 函数名:
//! @details 函数功能:CoordinatorConfig结构转换为json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void CoordinatorConfig::Encode(Json::Value& root) const {
  root[kJsonKeyControlRate] = control_rate;
  root[kJsonKeyScenarioLimit] = scenario_time_limit;
  root[kJsonKeyCoordinateMode] = coord_mode;
  root[kJsonKeyPlayerAutoReset] = auto_reset;
  root[kJsonKeyPlayerAutoStop] = auto_stop;
  root[kJsonKeyAddingInitialLocationMsg] = adding_initial_location_msg;
  root[kJsonKeyEnablePerfLogging] = log_perf;
  root[kJsonKeyGradingFeedbackProcess] = custom_grading_feedback_process;
  root[kJsonKeyOverrideUserLog] = override_user_log;
  log2world_config.Encode(root[kJsonKeyLog2World]);
  Json::Value& module_configs_node = root[kJsonKeyModuleConfigs];
  module_configs_node = Json::arrayValue;  // to explicitly construct a empty [] when there's no module configs.
  for (const ModuleConfig& cfg : module_configs) cfg.Encode(module_configs_node.append(Json::Value()));

  Json::Value& default_launch_module_configs_node = root[kJonsKeyDefaultLaunch];
  default_launch_module_configs_node =
      Json::arrayValue;  // to explicitly construct a empty [] when there's no module configs.
  for (const ModuleConfig& cfg : default_launch_module_configs)
    cfg.Encode(default_launch_module_configs_node.append(Json::Value()));
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成CoordinatorConfig结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void CoordinatorConfig::Decode(const Json::Value& root) {
  control_rate = root.get(kJsonKeyControlRate, kDefaultControlRate).asUInt();
  scenario_time_limit = root.get(kJsonKeyScenarioLimit, kDefaultScenarioTimeLimit).asUInt();
  coord_mode = Int2CoordMode(root.get(kJsonKeyCoordinateMode, kDefaultCoordMode).asInt());
  if (coord_mode == kCoordinateUnknown) throw std::invalid_argument("unknown coordination mode.");
  auto_reset = root.get(kJsonKeyPlayerAutoReset, kDefaultPlayerAutoReset).asBool();
  auto_stop = root.get(kJsonKeyPlayerAutoStop, kDefaultPlayerAutoStop).asBool();
  adding_initial_location_msg = root.get(kJsonKeyAddingInitialLocationMsg, kDefaultAddingInitialLocationMsg).asBool();
  log_perf = root.get(kJsonKeyEnablePerfLogging, kDefaultPerfLoggingEnabled).asBool();
  custom_grading_feedback_process = root.get(kJsonKeyGradingFeedbackProcess, "").asString();
  override_user_log = root.get(kJsonKeyOverrideUserLog, kDefaultOverrideUserLog).asBool();
  log2world_config.Decode(root[kJsonKeyLog2World]);
  const Json::Value& module_configs_node = root[kJsonKeyModuleConfigs];
  for (const Json::Value& p : module_configs_node) {
    module_configs.emplace_back();
    module_configs.back().Decode(p);
  }

  const Json::Value& default_launch_module_configs_node = root[kJonsKeyDefaultLaunch];
  for (const Json::Value& p : default_launch_module_configs_node) {
    default_launch_module_configs.emplace_back();
    default_launch_module_configs.back().Decode(p);
  }
}

//! @brief 函数名:
//! @details 函数功能:重载==操作符
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
bool CoordinatorConfig::operator==(const CoordinatorConfig& rhs) const {
  if (module_configs.size() != rhs.module_configs.size()) return false;
  for (size_t i = 0; i < module_configs.size(); ++i)
    if (module_configs[i] != rhs.module_configs[i]) return false;
  return control_rate == rhs.control_rate && scenario_time_limit == rhs.scenario_time_limit &&
         coord_mode == rhs.coord_mode && auto_reset == rhs.auto_reset && auto_stop == rhs.auto_stop &&
         adding_initial_location_msg == rhs.adding_initial_location_msg &&
         custom_grading_feedback_process == rhs.custom_grading_feedback_process &&
         override_user_log == rhs.override_user_log;
}

//! @brief 函数名:
//! @details 函数功能:GradingKpi结构转换成json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void GradingKpi::Encode(Json::Value& root) const {
  root[kJsonKeyKpiId] = id;
  root[kJsonKeyKpiName] = name;
  if (!category.empty()) ReadJsonDocFromString(root[kJsonKeyKpiCategory], category);
  if (!parameters.empty()) ReadJsonDocFromString(root[kJsonKeyKpiParams], parameters);
  if (!pass_condition.empty()) ReadJsonDocFromString(root[kJsonKeyKpiPassCond], pass_condition);
  if (!finish_condition.empty()) ReadJsonDocFromString(root[kJsonKeyKpiFinishCond], finish_condition);
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成GradingKpi结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void GradingKpi::Decode(const Json::Value& root) {
  id = root.get(kJsonKeyKpiId, 0).asInt64();
  name = root[kJsonKeyKpiName].asString();
  if (root.isMember(kJsonKeyKpiCategory)) WriteJsonDocToString(root[kJsonKeyKpiCategory], category);
  if (root.isMember(kJsonKeyKpiParams)) WriteJsonDocToString(root[kJsonKeyKpiParams], parameters);
  if (root.isMember(kJsonKeyKpiPassCond)) WriteJsonDocToString(root[kJsonKeyKpiPassCond], pass_condition);
  if (root.isMember(kJsonKeyKpiFinishCond)) WriteJsonDocToString(root[kJsonKeyKpiFinishCond], finish_condition);
}

//! @brief 函数名:
//! @details 函数功能:GradingKpiGroup结构转换成json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void GradingKpiList::Encode(Json::Value& root) const {
  root[kJsonKeyKpiListGroupId] = group_id;
  Json::Value& kpis_node = root[kJsonKeyKpiListKpis];
  kpis_node = Json::arrayValue;
  for (const GradingKpi& kpi : kpis) kpi.Encode(kpis_node.append(Json::Value()));
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成GradingKpiList结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void GradingKpiList::Decode(const Json::Value& root) {
  group_id = root[kJsonKeyKpiListGroupId].asInt64();
  const Json::Value& kpis_node = root[kJsonKeyKpiListKpis];
  for (const Json::Value& kpi : kpis_node) {
    kpis.emplace_back();
    kpis.back().Decode(kpi);
  }
}

//! @brief 函数名:
//! @details 函数功能:GradingKpiGroupList结构转换成json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void GradingKpiGroup::Encode(Json::Value& root) const {
  root[kJsonKeyKpiGroupId] = id;
  root[kJsonKeyKpiGroupName] = name;
  root[kJsonKeyKpiGroupFilePath] = file_path;
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成GradingKpiGroup结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void GradingKpiGroup::Decode(const Json::Value& root) {
  id = root[kJsonKeyKpiGroupId].asInt64();
  name = root[kJsonKeyKpiGroupName].asString();
  file_path = root.get(kJsonKeyKpiGroupFilePath, "").asString();
  const Json::Value& kpis_node = root[kJsonKeyKpiListKpis];
  if (kpis_node.isNull()) return;
  for (const Json::Value& kpi : kpis_node) {
    kpis.emplace_back();
    kpis.back().Decode(kpi);
  }
}

//! @brief 函数名:
//! @details 函数功能:GradingKpiGroupList结构转换成json结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void GradingKpiGroupList::Encode(Json::Value& root) const {
  root[kJsonKeyKpiGroupListActiveId] = active_id;
  Json::Value& groups_node = root[kJsonKeyKpiGroupListGroups];
  groups_node = Json::arrayValue;
  for (const GradingKpiGroup& g : groups) g.Encode(groups_node.append(Json::Value()));
}

//! @brief 函数名:
//! @details 函数功能:json结构转换成GradingKpiGroupList结构
//!
//! @param[in]
//! @param[in]
//! @param[out]
//!
//! @return
//!
//! @note
void GradingKpiGroupList::Decode(const Json::Value& root) {
  active_id = root[kJsonKeyKpiGroupListActiveId].asInt64();
  const Json::Value& groups_node = root[kJsonKeyKpiGroupListGroups];
  for (const Json::Value& g : groups_node) {
    groups.emplace_back();
    groups.back().Decode(g);
  }
}

}  // namespace coordinator
}  // namespace tx_sim
