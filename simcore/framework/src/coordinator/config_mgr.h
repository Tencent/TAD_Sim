// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "sqlite3.h"
#include "zmq_addon.hpp"

#include "config.h"
#include "grading_kpi_writer.h"

namespace tx_sim {
namespace coordinator {

bool isGroupModuelName(const std::string& groupname, const std::string& modulename);
std::string fetchModuelName(const std::string& groupname, const std::string& modulename);
std::string makeGroupModuelName(const std::string& groupname, const std::string& modulename);

//!
//! @brief 类名：ConfigManager
//! @details 类功能：管理配置数据
//!
class ConfigManager final : public ConfigLoader {
 public:
  //!
  //! @brief 构造函数
  //! @details 初始化配置管理器
  //!
  //! @param[in] db_path 数据库路径
  //!
  ConfigManager(const std::string& db_path);

  //!
  //! @brief 析构函数
  //! @details 释放资源
  //!
  ~ConfigManager();

  //!
  //! @brief 函数名：InitConfigData
  //! @details 函数功能：初始化配置数据
  //!
  //! @param[in] default_config_path 默认配置路径
  //! @param[in] cloud_root_dir 云根目录，默认为空
  //!
  void InitConfigData(const std::string& default_config_path, const std::string& cloud_root_dir = "");

  //!
  //! @brief 函数名：LoadCoordinatorConfig
  //! @details 函数功能：加载协调器配置
  //!
  //! @param[out] config 协调器配置对象
  //!
  void LoadCoordinatorConfig(CoordinatorConfig& config) override;

  //!
  //! @brief 函数名：LoadModuleConfig
  //! @details 函数功能：加载模块配置
  //!
  //! @param[out] config 模块配置对象
  //!
  bool LoadModuleConfig(ModuleConfig& config) override;

  //!
  //! @brief 函数名：LoadModuleConfigByName
  //! @details 函数功能：根据名称加载模块配置
  //!
  //! @param[in] schemeName 方案名称
  //! @param[out] module_cfgs 模块配置列表
  //!
  bool LoadModuleConfigByName(std::string schemeName, std::vector<ModuleConfig>& module_cfgs) override;

  //!
  //! @brief 函数名：GetDefaultLaunchModuleConfigs
  //! @details 函数功能：获取默认启动模块配置
  //!
  //! @return 默认启动模块配置列表
  //!
  std::vector<ModuleConfig> GetDefaultLaunchModuleConfigs();

  //!
  //! @brief 函数名：Execute
  //! @details 函数功能：执行命令
  //!
  //! @param[in] cmd 命令
  //! @param[in] input 输入数据
  //! @param[out] output 输出数据
  //!
  //! @return 命令执行结果
  //!
  tx_sim::impl::CmdErrorCode Execute(tx_sim::impl::Command cmd, zmq::multipart_t& input, zmq::multipart_t& output);

  //!
  //! @brief 函数名：AddModuleConfig
  //! @details 函数功能：添加模块配置
  //!
  //! @param[in] config 模块配置对象
  //!
  void AddModuleConfig(const ModuleConfig& config);

  //!
  //! @brief 函数名：RemoveModuleConfig
  //! @details 函数功能：移除模块配置
  //!
  //! @param[in] name 模块名称
  //!
  void RemoveModuleConfig(const std::string& name);

  //!
  //! @brief 函数名：UpdateModuleConfig
  //! @details 函数功能：更新模块配置
  //!
  //! @param[in] config 模块配置对象
  //!
  void UpdateModuleConfig(const ModuleConfig& config);

  //!
  //! @brief 函数名：GetModuleConfig
  //! @details 函数功能：获取模块配置
  //!
  //! @param[in] name 模块名称
  //! @param[out] config 模块配置对象
  //!
  void GetModuleConfig(const std::string& name, ModuleConfig& config);

  //!
  //! @brief 函数名：GetAllModuleConfig
  //! @details 函数功能：获取所有模块配置
  //!
  //! @param[out] configs 模块配置列表
  //!
  void GetAllModuleConfig(std::vector<ModuleConfig>& configs);

  //!
  //! @brief 函数名：AddModuleScheme
  //! @details 函数功能：添加模块方案
  //!
  //! @param[in] scheme 模块方案对象
  //!
  void AddModuleScheme(const ModuleScheme& scheme);

  //!
  //! @brief 函数名：RemoveModuleScheme
  //! @details 函数功能：移除模块方案
  //!
  //! @param[in] scheme_id 模块方案ID
  //!
  void RemoveModuleScheme(int64_t scheme_id);

  //!
  //! @brief 函数名：UpdateModuleScheme
  //! @details 函数功能：更新模块方案
  //!
  //! @param[in] scheme 模块方案对象
  //!
  void UpdateModuleScheme(const ModuleScheme& scheme);

  //!
  //! @brief 函数名：GetAllModuleSchemes
  //! @details 函数功能：获取所有模块方案
  //!
  //! @param[out] scheme_list 模块方案列表
  //!
  void GetAllModuleSchemes(ModuleSchemeList& scheme_list);
  void UpdateMultiEgoScheme(const ModuleMultiEgoScheme& multiego_scheme);
  void GetMultiEgoScheme(ModuleMultiEgoScheme& multiego_scheme);
  //!
  //! @brief 函数名：SetActiveModuleScheme
  //! @details 函数功能：设置活动模块方案
  //!
  //! @param[in] scheme_id 模块方案ID
  //!
  void SetActiveModuleScheme(int64_t scheme_id);

  //!
  //! @brief 函数名：ExportModuleScheme
  //! @details 函数功能：导出模块方案
  //!
  //! @param[in] scheme_name 模块方案名称
  //! @param[in] out_path 输出路径
  //!
  void ExportModuleScheme(const std::string& scheme_name, const std::string& out_path);

  //!
  //! @brief 函数名：ImportModuleScheme
  //! @details 函数功能：导入模块方案
  //!
  //! @param[in] in_path 输入路径
  //! @param[out] imported_name 导入的模块方案名称
  //!
  //! @return 导入是否成功
  //! return true if the import is finished sucessfully, false if an exact same scheme is already exist.
  //! the imported_name returns with the name of the imported scheme, whether it's already exist or is actually
  //! imported.
  bool ImportModuleScheme(const std::string& in_path, std::string& imported_name);

  //!
  //! @brief 函数名：SetSysConfigs
  //! @details 函数功能：设置系统配置
  //!
  //! @param[in] config 系统配置对象
  //!
  void SetSysConfigs(const CoordinatorConfig& config);

  //!
  //! @brief 函数名：GetSysConfigs
  //! @details 函数功能：获取系统配置
  //!
  //! @param[out] config 系统配置对象
  //!
  void GetSysConfigs(CoordinatorConfig& config);

  //!
  //! @brief 函数名：UpdatePlayList
  //! @details 函数功能：更新播放列表
  //!
  //! @param[in] play_list_str 播放列表字符串
  //!
  void UpdatePlayList(const std::string& play_list_str);

  //!
  //! @brief 函数名：GetPlayList
  //! @details 函数功能：获取播放列表
  //!
  //! @param[out] play_list_str 播放列表字符串
  //!
  void GetPlayList(std::string& play_list_str);

  //!
  //! @brief 函数名：RemoveScenariosFromPlayList
  //! @details 函数功能：从播放列表中移除场景
  //!
  //! @param[in] list_to_be_removed_str 要移除的场景列表字符串
  //!
  void RemoveScenariosFromPlayList(const std::string& list_to_be_removed_str);

  //!
  //! @brief 函数名：AddGradingKpis
  //! @details 函数功能：添加评分关键指标
  //!
  //! @param[in] kpi_list 评分关键指标列表
  //!
  void AddGradingKpis(const GradingKpiList& kpi_list);

  //!
  //! @brief 函数名：RemoveGradingKpi
  //! @details 函数功能：移除评分关键指标
  //!
  //! @param[in] kpi_id 评分关键指标ID
  //!
  void RemoveGradingKpi(int64_t kpi_id);

  //!
  //! @brief 函数名：UpdateGradingKpi
  //! @details 函数功能：更新评分关键指标
  //!
  //! @param[in] kpi 评分关键指标对象
  //!
  void UpdateGradingKpi(const GradingKpi& kpi);

  //!
  //! @brief 函数名：GetGradingKpisInGroup
  //! @details 函数功能：获取评分关键指标组
  //!
  //! @param[out] kpi_list 评分关键指标列表
  //!
  void GetGradingKpisInGroup(GradingKpiList& kpi_list);

  //!
  //! @brief 函数名：AddGradingKpiGroup
  //! @details 函数功能：添加评分关键指标组
  //!
  //! @param[in] kpi_group 评分关键指标组对象
  //!
  void AddGradingKpiGroup(const GradingKpiGroup& kpi_group);

  //!
  //! @brief 函数名：RemoveGradingKpiGroup
  //! @details 函数功能：移除评分关键指标组
  //!
  //! @param[in] group_id 评分关键指标组ID
  //!
  void RemoveGradingKpiGroup(int64_t group_id);

  //!
  //! @brief 函数名：UpdateGradingKpiGroup
  //! @details 函数功能：更新评分关键指标组
  //!
  //! @param[in] kpi_group 评分关键指标组对象
  //!
  void UpdateGradingKpiGroup(const GradingKpiGroup& kpi_group);

  //!
  //! @brief 函数名：GetAllGradingKpiGroups
  //! @details 函数功能：获取所有评分关键指标组
  //!
  //! @param[out] kpi_group_list 评分关键指标组列表
  //!
  void GetAllGradingKpiGroups(GradingKpiGroupList& kpi_group_list);

  //!
  //! @brief 函数名：SetDefaultGradingKpiGroup
  //! @details 函数功能：设置默认评分关键指标组
  //!
  //! @param[in] group_id 评分关键指标组ID
  //!
  void SetDefaultGradingKpiGroup(int64_t group_id);

  //!
  //! @brief 函数名：ApplyGradingKpiConfig
  //! @details 函数功能：应用评分关键指标配置
  //!
  void ApplyGradingKpiConfig();

  //!
  //! @brief 函数名：RestoreDefault
  //! @details 函数功能：恢复默认配置
  //!
  void RestoreDefault();

 private:
  typedef int (*SqliteExecCb)(void*, int, char**, char**);

  //!
  //! @brief 函数名：ExecSqliteRoutine
  //! @details 函数功能：执行SQLite常规操作
  //!
  //! @param[in] sql SQL语句
  //! @param[in] cb 回调函数，默认为空
  //! @param[in] cb_arg 回调函数参数，默认为空
  //!
  void ExecSqliteRoutine(const char* sql, SqliteExecCb cb = nullptr, void* cb_arg = nullptr);

  //!
  //! @brief 函数名：ExecSqliteRoutineWithAllocatedSqlStr
  //! @details 函数功能：执行SQLite常规操作，使用分配的SQL字符串
  //!
  //! @param[in] sql_str SQL字符串
  //! @param[in] cb 回调函数，默认为空
  //! @param[in] cb_arg 回调函数参数，默认为空
  //!
  void ExecSqliteRoutineWithAllocatedSqlStr(char* sql_str, SqliteExecCb cb = nullptr, void* cb_arg = nullptr);

  //!
  //! @brief 函数名：GetConfigCount
  //! @details 函数功能：获取配置数量
  //!
  //! @param[in] table_name 表名
  //!
  //! @return 配置数量
  //!
  size_t GetConfigCount(const std::string& table_name);

  //!
  //! @brief 函数名：GetConfigID
  //! @details 函数功能：获取配置ID
  //!
  //! @param[in] sql SQL语句
  //!
  //! @return 配置ID
  //!
  int64_t GetConfigID(const char* sql);

  //!
  //! @brief 函数名：UpsertDefaultModules
  //! @details 函数功能：插入或更新默认模块
  //!
  void UpsertDefaultModules();

  //!
  //! @brief 函数名：UpsertDefaultSchemes
  //! @details 函数功能：插入或更新默认方案
  //!
  //! @param[in] scheme_list 方案列表
  //!
  void UpsertDefaultSchemes(const ModuleSchemeList& scheme_list);

  //!
  //! @brief 函数名：AddDefaultSysConfigsIfNotExist
  //! @details 函数功能：如果不存在，则添加默认系统配置
  //!
  void AddDefaultSysConfigsIfNotExist();

  //!
  //! @brief 函数名：GetDefaultGradingKpis
  //! @details 函数功能：获取默认评分KPI
  //!
  //! @param[in] kpi_file_path KPI文件路径
  //!
  void GetDefaultGradingKpis(const std::string& kpi_file_path);

  //!
  //! @brief 函数名：GetActiveModuleNames
  //! @details 函数功能：获取活动模块名称
  //!
  //! @param[out] modules 模块名称列表
  //!
  void GetActiveModuleNames(std::vector<std::tuple<std::string, std::string>>& modules);
  void GetActiveMultiEgoSchemes(std::map<std::string, ModuleScheme>& multiego_modules);
  //!
  //! @brief 函数名：GetActiveModuleSchemeCategory
  //! @details 函数功能：获取活动模块方案类别
  //!
  //! @return 活动模块方案类别
  //!
  std::string GetActiveModuleSchemeCategory();

  //!
  //! @brief 函数名：GetSchemeModuleSqlStr
  //! @details 函数功能：获取方案模块SQL字符串
  //!
  //! @param[in] modules 模块名称列表
  //!
  //! @return 方案模块SQL字符串
  //!
  char* GetSchemeModuleSqlStr(const std::vector<std::tuple<std::string, std::string>>& modules);

  //!
  //! @brief 函数名：GetModuleSchemeByName
  //! @details 函数功能：根据名称获取模块方案
  //!
  //! @param[in,out] scheme 模块方案
  //!
  //! @return 是否成功
  //!
  bool GetModuleSchemeByName(ModuleScheme& scheme);
  bool GetModuleSchemeById(ModuleScheme& scheme);
  //!
  //! @brief 函数名：GetSchemeModuleConfigs
  //! @details 函数功能：获取方案模块配置
  //!
  //! @param[in] module_names 模块名称列表
  //! @param[out] configs 模块配置列表
  //!
  void GetSchemeModuleConfigs(const std::vector<std::tuple<std::string, std::string>>& module_names,
                              std::vector<ModuleConfig>& configs);
  void GetSchemeMultiEgoModuleConfigs(const std::map<std::string, ModuleScheme>& schemes,
                                      std::vector<ModuleConfig>& configs);
  //!
  //! @brief 函数名：GetLastIDLessThan
  //! @details 函数功能：获取小于给定ID的最后一个ID
  //!
  //! @param[in] sql SQL语句
  //! @param[in] upper_id 上限ID
  //!
  //! @return 小于给定ID的最后一个ID
  //!
  int64_t GetLastIDLessThan(const char* sql, int64_t upper_id);

  //!
  //! @brief 函数名：WriteGlobalGradingKpiFile
  //! @details 函数功能：写入全局评分KPI文件
  //!
  void WriteGlobalGradingKpiFile();

  //!
  //! @brief 函数名：UpdateKpiNameCategoryMapping
  //! @details 函数功能：更新KPI名称类别映射
  //!
  //! @param[in] sLabels 标签字符串
  //!
  void UpdateKpiNameCategoryMapping(std::string sLabels);

  //!
  //! @brief 函数名：UpdateDefaultGradingKPIs
  //! @details 函数功能：更新默认评分KPI
  //!
  //! @param[in] name KPI名称
  //! @param[in] newCateGoryString 新类别字符串
  //!
  void UpdateDefaultGradingKPIs(std::string name, std::string newCateGoryString);

  //!
  //! @brief 函数名：GetPbInfo
  //! @details 函数功能：获取PB信息
  //!
  //! @param[in] intput 输入字符串
  //! @param[out] output 输出字符串
  //!
  void GetPbInfo(const std::string& intput, std::string& output);

  //!
  //! @brief DelPbInfo
  //! @details 函数功能：删除PB信息
  //!
  //! @param[in] intput 输入字符串
  //! @param[out] output
  //!
  tx_sim::impl::CmdErrorCode DelPbInfo(const std::string& intput);

  //!
  //! @brief UpdatePbInfo
  //! @details 函数功能：更新PB信息
  //!
  //! @param[in] intput 输入json序列化字符串
  //! @param[out] output 输出jsos序列化字符串
  //!
  tx_sim::impl::CmdErrorCode UpdatePbInfo(const std::string& intput, std::string& output);

  // 已播放的场景ID集合
  std::unordered_set<int64_t> played_scenario_ids_;
  // SQLite执行回调函数
  SqliteExecCb module_config_get_cb_ = nullptr;
  // 默认协调器配置
  CoordinatorConfig default_config_;
  // 默认KPI列表
  GradingKpiList default_kpis_, default_group_kpis_;
  // KPI名称到类别的映射
  std::unordered_map<std::string, std::string> kpi_name_category_mapping_;
  // 默认KPI组
  GradingKpiGroup default_kpi_group_;
  // 评分KPI写入器的唯一指针
  std::unique_ptr<IGradingKpiWriter> grading_kpi_writer_;
  // 全局评分KPI文件路径
  std::string global_grading_kpi_file_path_, default_global_grading_kpi_file_path_;
  // 默认全局评分KPI文件路径
  std::string m_default_grading_kpi_file_path;
  // SQLite数据库指针
  sqlite3* db_;
  // 模块日志目录
  std::string m_module_log_directory;
  // 数据路径
  std::string m_pbinfo_data_path;
  // 系统路径
  std::string m_pbinfo_sys_path;
};

}  // namespace coordinator
}  // namespace tx_sim
