// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <map>
#include <string>
#include <vector>

#include "json/json.h"

#include "utils/constant.h"

namespace tx_sim {
namespace coordinator {

//! @brief JsonConfig 结构体
//! @details JsonConfig 是一个用于处理 JSON 配置的结构体，提供了将配置编码为字符串或 Json::Value 对象，以及从字符串或
//! Json::Value 对象解码配置的方法。
struct JsonConfig {
  //!
  //! @brief 函数名：EncodeToStr
  //! @details 函数功能：将配置编码为字符串
  //!
  //! @return 编码后的字符串
  //!
  virtual std::string EncodeToStr() const;

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将配置编码为 Json::Value 对象
  //!
  //! @param[out] root 编码后的 Json::Value 对象
  //!
  virtual void Encode(Json::Value& root) const = 0;

  //!
  //! @brief 函数名：DecodeFromStr
  //! @details 函数功能：从字符串解码配置
  //!
  //! @param[in] json_str 需要解码的 JSON 字符串
  //!
  virtual void DecodeFromStr(const std::string& json_str);

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码配置
  //!
  //! @param[in] root 需要解码的 Json::Value 对象
  //!
  virtual void Decode(const Json::Value& root) = 0;
};

//!
//! @brief 结构体名：ConnectionConfig
//! @details 结构体功能：存储连接配置信息
//!
struct ConnectionConfig {
  //!
  //! @brief 成员变量：cmd_timeout
  //! @details 命令超时时间，单位为毫秒
  //!
  //! @note in milli-seconds.
  uint32_t cmd_timeout{0};

  //!
  //! @brief 成员变量：step_timeout
  //! @details 步进命令的超时时间，单位为毫秒，0表示无超时
  //!
  //! @note in milli-seconds, specially for step command. 0 means no timeout.
  uint32_t step_timeout{0};

  //!
  //! @brief 重载运算符：==
  //! @details 用于比较两个ConnectionConfig对象是否相等
  //!
  bool operator==(const ConnectionConfig& rhs) const;

  //!
  //! @brief 重载运算符：!=
  //! @details 用于比较两个ConnectionConfig对象是否不相等
  //!
  bool operator!=(const ConnectionConfig& rhs) const { return !(*this == rhs); }
};

//! @brief ModuleConfig 结构体
//! @details ModuleConfig 是一个用于处理模块配置的结构体，继承自 JsonConfig 结构体，提供了将配置编码为字符串或
//! Json::Value 对象，以及从字符串或 Json::Value 对象解码配置的方法。
struct ModuleConfig : public JsonConfig {
  std::string name;
  // unique in one setup.
  std::string module_scheme_name;
  std::string module_group_name;
  std::string module_scheme_category;
  uint32_t execute_period{0};
  // in milli-seconds.
  uint32_t response_time{0};
  // in milli-seconds.
  int32_t m_nPriority = 50;
  std::string m_accessMode;
  std::map<std::string, std::string> init_args;
  ConnectionConfig conn_args;
  bool auto_launch{false};
  std::vector<std::string> dep_paths;
  std::string so_path;
  std::string bin_path;
  std::vector<std::string> bin_args;
  tx_sim::impl::ModuleCategory category{tx_sim::impl::kModuleCategoryUnknown};
  int shadow_id{0};
  // default 0 means normal mode, >0 specifies the ID in shadow mode.

  //!
  //! @brief 构造函数
  //!
  ModuleConfig() {}

  //!
  //! @brief 构造函数
  //!
  //! @param[in] n 模块名称
  //!
  ModuleConfig(const std::string& n) : name(n) {}

  //!
  //! @brief 构造函数
  //!
  //! @param[in] n 模块名称
  //! @param[in] exe_perd 执行周期（毫秒）
  //! @param[in] resp_time 响应时间（毫秒），默认为 0
  //!
  ModuleConfig(const std::string& n, uint32_t exe_perd, uint32_t resp_time = 0)
      : name(n), execute_period(exe_perd), response_time(resp_time) {}

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将配置编码为 Json::Value 对象
  //!
  //! @param[out] root 编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码配置
  //!
  //! @param[in] root 需要解码的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;

  //!
  //! @brief 函数名：operator==
  //! @details 函数功能：比较两个 ModuleConfig 对象是否相等
  //!
  //! @param[in] rhs 需要比较的 ModuleConfig 对象
  //!
  //! @return 两个对象是否相等
  //!
  bool operator==(const ModuleConfig& rhs) const;

  //!
  //! @brief 函数名：operator!=
  //! @details 函数功能：比较两个 ModuleConfig 对象是否不相等
  //!
  //! @param[in] rhs 需要比较的 ModuleConfig 对象
  //!
  //! @return 两个对象是否不相等
  //!
  bool operator!=(const ModuleConfig& rhs) const { return !(*this == rhs); }

  //!
  //! @brief 函数名：EqualsToImported
  //! @details 函数功能：比较两个 ModuleConfig 对象是否相等（导入的配置）
  //!
  //! @param[in] imported 需要比较的 ModuleConfig 对象
  //!
  //! @return 两个对象是否相等
  //!
  bool EqualsToImported(const ModuleConfig& imported) const;
};

struct ModuleScheme : public JsonConfig {
  int64_t id;  // all system default scheme's id should be <= 0, all user defined scheme's id should be > 0.
  std::string name;
  std::vector<std::tuple<std::string, std::string>> modules;
  std::string category;

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 ModuleScheme 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 ModuleScheme 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;
};

struct ModuleMultiEgoScheme : public JsonConfig {
  int64_t active_scene_id;
  std::map<std::string, int64_t> active_multiego;
  void Encode(Json::Value& root) const override;
  void Decode(const Json::Value& root) override;
};

struct ModuleSchemeList : public JsonConfig {
  int64_t active_id;
  int64_t active_scene_id;
  std::map<std::string, int64_t> active_multiego;
  std::vector<ModuleScheme> schemes;

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 ModuleSchemeList 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 ModuleSchemeList 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;
};

struct Log2WorldTopicConfig : public JsonConfig {
  std::map<std::string, tx_sim::impl::Log2WorldTopicMode> logsim_topic_configs;
  std::map<std::string, tx_sim::impl::Log2WorldTopicMode> worldsim_topic_configs;

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 Log2WorldTopicConfig 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 Log2WorldTopicConfig 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;
};

struct Log2WorldPlayConfig : public JsonConfig {
  bool ego_switch = true;
  bool traffic_switch = true;
  int32_t switch_time_ms = 0;
  tx_sim::impl::Log2WorldSwitchType switch_type{tx_sim::impl::kLog2WorldSwitchScene};

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 Log2WorldPlayConfig 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 Log2WorldPlayConfig 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;

  //!
  //! @brief 函数名：operator==
  //! @details 函数功能：比较两个 Log2WorldPlayConfig 对象是否相等
  //!
  //! @param[in] rhs 需要比较的 Log2WorldPlayConfig 对象
  //!
  //! @return 两个对象是否相等
  //!
  bool operator==(const Log2WorldPlayConfig& rhs) const;

  //!
  //! @brief 函数名：operator!=
  //! @details 函数功能：比较两个 Log2WorldPlayConfig 对象是否不相等
  //!
  //! @param[in] rhs 需要比较的 Log2WorldPlayConfig 对象
  //!
  //! @return 两个对象是否不相等
  //!
  bool operator!=(const Log2WorldPlayConfig& rhs) const { return !(*this == rhs); }
};

struct LogsimEvent : public JsonConfig {
  int64_t timestamp_ms = 0;  // epoch time
  int16_t type = 0;

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 LogsimEvent 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 LogsimEvent 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;
};

struct LogsimEventInfo : public JsonConfig {
  int64_t start_timestamp_ms = 0;  // epoch time
  uint64_t total_duration_ms = 0;
  std::vector<LogsimEvent> events;

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 LogsimEventInfo 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 LogsimEventInfo 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;
};

struct CoordinatorConfig : public JsonConfig {
  std::vector<ModuleConfig> module_configs;
  std::vector<ModuleConfig> default_launch_module_configs;

  uint16_t control_rate{tx_sim::impl::kDefaultControlRate};  // the play rate in percent, 100 for normal speed, 0 for
                                                             // maximum, any >0 value for specified rate percent.
  uint32_t scenario_time_limit{tx_sim::impl::kDefaultScenarioTimeLimit};  // in seconds, 0 for unlimit.
  tx_sim::impl::CoordinationMode coord_mode{tx_sim::impl::kDefaultCoordMode};
  bool auto_reset{tx_sim::impl::kDefaultPlayerAutoReset};
  bool auto_stop{tx_sim::impl::kDefaultPlayerAutoStop};
  bool adding_initial_location_msg{tx_sim::impl::kDefaultAddingInitialLocationMsg};
  bool log_perf{tx_sim::impl::kDefaultPerfLoggingEnabled};
  Log2WorldTopicConfig log2world_config;
  std::string custom_grading_feedback_process;
  std::string m_module_log_directory;
  bool override_user_log{tx_sim::impl::kDefaultOverrideUserLog};
  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 CoordinatorConfig 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 CoordinatorConfig 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;

  //!
  //! @brief 函数名：operator==
  //! @details 函数功能：比较两个 CoordinatorConfig 对象是否相等
  //!
  //! @param[in] rhs 需要比较的 CoordinatorConfig 对象
  //!
  //! @return 两个对象是否相等
  //!
  bool operator==(const CoordinatorConfig& rhs) const;

  bool operator!=(const CoordinatorConfig& rhs) const { return !(*this == rhs); }
};

struct GradingKpi : public JsonConfig {
  int64_t id;
  std::string name;
  std::string category;
  std::string parameters;  // in json format, key-value pairs.
  std::string pass_condition;
  std::string finish_condition;

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 GradingKpi 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 GradingKpi 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;
};

struct GradingKpiList : public JsonConfig {
  int64_t group_id;
  std::vector<GradingKpi> kpis;

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 GradingKpiList 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 GradingKpiList 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;
};

struct GradingKpiGroup : public JsonConfig {
  int64_t id;  // -1 contains all default grading KPIs, 0 is the default KPI group(subset of id -1), >=1 user added.
  std::string name;
  std::string file_path;
  std::vector<GradingKpi> kpis;

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 GradingKpiGroup 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 GradingKpiGroup 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;
};

struct GradingKpiGroupList : public JsonConfig {
  int64_t active_id;
  std::vector<GradingKpiGroup> groups;

  //!
  //! @brief 函数名：Encode
  //! @details 函数功能：将 GradingKpiGroupList 对象编码为 Json::Value 对象
  //!
  //! @param[out] root 存储编码后的 Json::Value 对象
  //!
  void Encode(Json::Value& root) const override;

  //!
  //! @brief 函数名：Decode
  //! @details 函数功能：从 Json::Value 对象解码为 GradingKpiGroupList 对象
  //!
  //! @param[in] root 存储编码后的 Json::Value 对象
  //!
  void Decode(const Json::Value& root) override;
};

class ConfigLoader {
 public:
  virtual void LoadCoordinatorConfig(CoordinatorConfig& config) = 0;
  virtual bool LoadModuleConfig(ModuleConfig& config) = 0;
  virtual bool LoadModuleConfigByName(std::string schemeName, std::vector<ModuleConfig>& module_cfgs) = 0;
};

}  // namespace coordinator
}  // namespace tx_sim
