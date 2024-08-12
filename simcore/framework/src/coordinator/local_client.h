// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <functional>
#include <memory>

#include "status.h"
#include "utils/constant.h"

namespace tx_sim {
namespace cli {

//! @brief 类名：Client
//! @details 类功能：提供与后端服务器通信的接口，用于执行播放器请求和模块管理请求以及配置请求
class Client {
 public:
  //! @brief 构造函数
  //! @details 初始化Client对象，设置后端服务器地址
  //! @param[in] backend_addr 后端服务器地址
  Client(const std::string& backend_addr);

  //! @brief 析构函数
  //! @details 释放Client对象占用的资源
  virtual ~Client();
  /************************************ Player Requests ******************************************/
  //! @brief 播放器请求
  //! @{
  //! @brief 函数名：Setup
  //! @details 函数功能：设置播放器场景
  //!
  //! @param[in] scenario_path 一个包含场景文件路径的字符串
  //! @param[in] cb 一个回调函数，用于接收命令执行状态
  void Setup(const std::string& scenario_path, const tx_sim::coordinator::CommandStatusCb& cb);

  //! @brief 函数名：UnSetup
  //! @details 函数功能：取消播放器场景设置
  //!
  //! @param[in] cb 一个回调函数，用于接收命令执行状态
  void UnSetup(const tx_sim::coordinator::CommandStatusCb& cb);

  //! @brief 函数名：Step
  //! @details 函数功能：执行播放器单步操作
  //!
  //! @param[in] cb 一个回调函数，用于接收命令执行状态
  void Step(const tx_sim::coordinator::CommandStatusCb& cb);

  //! @brief 函数名：Stop
  //! @details 函数功能：停止播放器操作
  //!
  //! @param[in] cb 一个回调函数，用于接收命令执行状态
  void Stop(const tx_sim::coordinator::CommandStatusCb& cb);

  //! @brief 函数名：Run
  //! @details 函数功能：运行播放器
  //!
  //! @param[in] cb 一个回调函数，用于接收命令执行状态
  void Run(const tx_sim::coordinator::CommandStatusCb& cb);

  //! @brief 函数名：Pause
  //! @details 函数功能：暂停播放器操作
  //!
  //! @param[in] cb 一个回调函数，用于接收命令执行状态
  void Pause(const tx_sim::coordinator::CommandStatusCb& cb);
  void SetHighlightGroup(const tx_sim::coordinator::CommandStatusCb& cb, const std::string& group);
  //! @}

  /********************************** Module Management Requests **********************************/
  //! @brief 模块管理请求
  //! @{
  //! @brief 函数名：LaunchModule
  //! @details 函数功能：启动指定名称的模块
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode LaunchModule(const std::string& name);

  //! @brief 函数名：TerminateModule
  //! @details 函数功能：终止指定名称的模块
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode TerminateModule(const std::string& name);

  //! @}

  //! @brief 配置请求
  //! @{
  //! @brief 函数名：AddModuleConfig
  //! @details 函数功能：添加模块配置
  //!
  //! @param[in] config_json_str 一个包含模块配置信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode AddModuleConfig(const std::string& config_json_str);

  //! @brief 函数名：RemoveModuleConfig
  //! @details 函数功能：移除指定名称的模块配置
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode RemoveModuleConfig(const std::string& name);

  //! @brief 函数名：UpdateModuleConfig
  //! @details 函数功能：更新模块配置
  //!
  //! @param[in] config_json_str 一个包含模块配置信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode UpdateModuleConfig(const std::string& config_json_str);

  //! @brief 函数名：GetModuleConfig
  //! @details 函数功能：获取指定名称的模块配置
  //!
  //! @param[in] name 一个包含模块名称的字符串
  //! @param[out] config_json_str 一个包含模块配置信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode GetModuleConfig(const std::string& name, std::string& config_json_str);

  //! @brief 函数名：GetAllModuleConfigs
  //! @details 函数功能：获取所有模块配置
  //!
  //! @param[out] configs_json_str 一个包含所有模块配置信息的JSON字符串数组
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode GetAllModuleConfigs(std::vector<std::string>& configs_json_str);

  //! @brief 函数名：AddModuleScheme
  //! @details 函数功能：添加模块方案
  //!
  //! @param[in] scheme_json_str 一个包含模块方案信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode AddModuleScheme(const std::string& scheme_json_str);

  //! @brief 函数名：RemoveModuleScheme
  //! @details 函数功能：移除指定ID的模块方案
  //!
  //! @param[in] scheme_id 一个包含模块方案ID的整数
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode RemoveModuleScheme(int64_t scheme_id);

  //! @brief 函数名：UpdateModuleScheme
  //! @details 函数功能：更新模块方案
  //!
  //! @param[in] scheme_json_str 一个包含模块方案信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode UpdateModuleScheme(const std::string& scheme_json_str);

  //! @brief 函数名：GetAllModuleSchemes
  //! @details 函数功能：获取所有模块方案
  //!
  //! @param[out] scheme_list_json_str 一个包含所有模块方案信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode GetAllModuleSchemes(std::string& scheme_list_json_str);

  //! @brief 函数名：SetActiveModuleScheme
  //! @details 函数功能：设置当前激活的模块方案
  //!
  //! @param[in] scheme_id 一个包含模块方案ID的整数
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode SetActiveModuleScheme(int64_t scheme_id);

  //! @brief 函数名：ExportModuleScheme
  //! @details 函数功能：导出指定名称的模块方案
  //!
  //! @param[in] name 一个包含模块方案名称的字符串
  //! @param[in] out_path 一个包含导出文件路径的字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode ExportModuleScheme(const std::string& name, const std::string& out_path);

  //! @brief 函数名：ImportModuleScheme
  //! @details 函数功能：导入模块方案
  //!
  //! @param[in] in_path 一个包含导入文件路径的字符串
  //! @param[out] imported_name 一个包含导入后模块方案名称的字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode ImportModuleScheme(const std::string& in_path, std::string& imported_name);

  //! @brief 函数名：SetSysConfigs
  //! @details 函数功能：设置系统配置
  //!
  //! @param[in] config_json_str 一个包含系统配置信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode SetSysConfigs(const std::string& config_json_str);

  //! @brief 函数名：GetSysConfigs
  //! @details 函数功能：获取系统配置
  //!
  //! @param[out] config_json_str 一个包含系统配置信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode GetSysConfigs(std::string& config_json_str);

  //! @brief 函数名：RestoreDefaultConfigs
  //! @details 函数功能：恢复默认系统配置
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode RestoreDefaultConfigs();

  //! @brief 函数名：UpdatePlayList
  //! @details 函数功能：更新播放列表
  //!
  //! @param[in] play_list_json_str 一个包含播放列表信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode UpdatePlayList(const std::string& play_list_json_str);

  //! @brief 函数名：GetPlayList
  //! @details 函数功能：获取播放列表
  //!
  //! @param[out] play_list_json_str 一个包含播放列表信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode GetPlayList(std::string& play_list_json_str);

  //! @brief 函数名：RemoveScenariosFromPlayList
  //! @details 函数功能：从播放列表中移除指定场景
  //!
  //! @param[in] removing_list_json_str 一个包含要移除的场景列表信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode RemoveScenariosFromPlayList(const std::string& removing_list_json_str);

  //! @brief 函数名：AddGradingKpis
  //! @details 函数功能：添加评分关键指标
  //!
  //! @param[in] kpis_json_str 一个包含评分关键指标信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode AddGradingKpis(const std::string& kpis_json_str);

  //! @brief 函数名：RemoveGradingKpi
  //! @details 函数功能：移除指定ID的评分关键指标
  //!
  //! @param[in] kpi_id 一个包含评分关键指标ID的整数
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode RemoveGradingKpi(int64_t kpi_id);

  //! @brief 函数名：UpdateGradingKpi
  //! @details 函数功能：更新评分关键指标
  //!
  //! @param[in] kpi_json_str 一个包含评分关键指标信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode UpdateGradingKpi(const std::string& kpi_json_str);

  //! @brief 函数名：GetGradingKpisInGroup
  //! @details 函数功能：获取指定分组下的评分关键指标
  //!
  //! @param[in] group_id 一个包含评分关键指标分组ID的整数
  //! @param[out] kpis_json_str 一个包含评分关键指标信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode GetGradingKpisInGroup(int64_t group_id, std::string& kpis_json_str);

  //! @brief 函数名：AddGradingKpiGroup
  //! @details 函数功能：添加评分关键指标分组
  //!
  //! @param[in] kpi_group_json_str 一个包含评分关键指标分组信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode AddGradingKpiGroup(const std::string& kpi_group_json_str);

  //! @brief 函数名：RemoveGradingKpiGroup
  //! @details 函数功能：移除指定ID的评分关键指标分组
  //!
  //! @param[in] group_id 一个包含评分关键指标分组ID的整数
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode RemoveGradingKpiGroup(int64_t group_id);

  //! @brief 函数名：UpdateGradingKpiGroup
  //! @details 函数功能：更新评分关键指标分组
  //!
  //! @param[in] kpi_group_json_str 一个包含评分关键指标分组信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode UpdateGradingKpiGroup(const std::string& kpi_group_json_str);

  //! @brief 函数名：GetAllGradingKpiGroups
  //! @details 函数功能：获取所有评分关键指标分组
  //!
  //! @param[out] kpi_groups_json_str 一个包含所有评分关键指标分组信息的JSON字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode GetAllGradingKpiGroups(std::string& kpi_groups_json_str);

  //! @brief 函数名：SetDefaultGradingKpiGroup
  //! @details 函数功能：设置默认评分关键指标分组
  //!
  //! @param[in] group_id 一个包含评分关键指标分组ID的整数
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode SetDefaultGradingKpiGroup(int64_t group_id);

  //! @brief 函数名：SetGradingLabel
  //! @details 函数功能：设置评分标签
  //!
  //! @param[in] labels_str 一个包含评分标签信息的字符串
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode SetGradingLabel(const std::string& labels_str);
  tx_sim::impl::CmdErrorCode UpdateMultiEgoScheme(const std::string& scheme_json_str);
  tx_sim::impl::CmdErrorCode GetMultiEgoScheme(std::string& scheme_json_str);

 private:
  //! @brief 结构体：CtxImpl
  //! @details 结构体功能：声明一个CtxImpl结构体，用于存储上下文信息
  struct CtxImpl;

  //! @brief 函数名：DoPlayerCommand
  //! @details 函数功能：执行播放器命令
  //!
  //! @param[in] cmd 一个包含命令类型的Command对象
  //! @param[in] cb 一个回调函数，用于接收命令执行状态
  //! @param[in] scenario_path 一个包含场景文件路径的字符串，默认为空字符串
  void DoPlayerCommand(tx_sim::impl::Command cmd, const tx_sim::coordinator::CommandStatusCb& cb,
                       const std::string& scenario_path = "", const std::string& highlight_group = "");

  //! @brief 函数名：DoManagerCommand
  //! @details 函数功能：执行管理器命令
  //!
  //! @param[in] cmd 一个包含命令类型的Command对象
  //! @param[in] msg 一个包含命令参数的multipart_t对象
  //!
  //! @return 返回一个CmdErrorCode对象，表示命令执行状态
  tx_sim::impl::CmdErrorCode DoManagerCommand(tx_sim::impl::Command cmd, zmq::multipart_t& msg);

  //! @brief 成员变量：addr_
  //! @details 成员变量功能：存储地址信息
  const std::string addr_;

  //! @brief 成员变量：ctx_
  //! @details 成员变量功能：存储指向CtxImpl结构体的指针
  CtxImpl* ctx_ = nullptr;

  //! @brief 成员变量：buf_
  //! @details 成员变量功能：存储一个大小为10的无符号8位整数数组
  uint8_t buf_[10];
};

}  // namespace cli
}  // namespace tx_sim
