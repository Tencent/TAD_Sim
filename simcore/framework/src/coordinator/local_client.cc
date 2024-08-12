// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "local_client.h"

#include "utils/leb128.h"
#include "zmq_addon.hpp"

#define SOCKET_TIMEOUT 10000

using namespace tx_sim::impl;
using namespace tx_sim::coordinator;
using namespace tx_sim::utils;

static std::unique_ptr<zmq::socket_t> CreateSocket(zmq::context_t& ctx, const std::string& addr) {
  std::unique_ptr<zmq::socket_t> s(new zmq::socket_t(ctx, ZMQ_DEALER));
  s->set(zmq::sockopt::linger, 0);
  s->set(zmq::sockopt::sndtimeo, SOCKET_TIMEOUT);
  s->set(zmq::sockopt::rcvtimeo, SOCKET_TIMEOUT);
  s->connect(addr);
  return s;
}

static void UnloadListFromMessage(std::vector<std::string>& configs, zmq::multipart_t& msg) {
  size_t cnt = PopMsgSize(msg);
  for (size_t i = 0; i < cnt; ++i) { configs.emplace_back(PopMsgStr(msg)); }
}

static void LoadListToMessage(const std::vector<std::string>& names, zmq::multipart_t& msg, uint8_t* buf) {
  AddMsgSize(buf, names.size(), msg);
  for (const std::string& n : names) { msg.addstr(n); }
}

static bool NeedCallback(Command cmd, const CommandStatus& status) {
  return cmd == kCmdSetup  // setup cmd returns a streaming status.
         || (cmd == kCmdRun &&
             !status.module_status.empty())  // run cmd returns a streaming status but empty one should be avoid.
         || status.ec != kCmdAccepted;       // other cmd is not a streaming rpc and returns one final status only.
}

namespace tx_sim {
namespace cli {

typedef std::unique_ptr<zmq::socket_t> SocketPtr;

struct Client::CtxImpl {
  zmq::context_t impl;
};

//! @brief 构造函数
//! @details 初始化Client对象，设置后端服务器地址
//! @param[in] backend_addr 后端服务器地址
Client::Client(const std::string& backend_addr) : addr_(backend_addr) {
  ctx_ = new CtxImpl;
}

//! @brief 析构函数
//! @details 释放Client对象占用的资源
Client::~Client() {
  if (ctx_) delete ctx_;
}

//! @brief 函数名：DoPlayerCommand
//! @details 函数功能：执行播放器命令
//!
//! @param[in] cmd 一个包含命令类型的Command对象
//! @param[in] cb 一个回调函数，用于接收命令执行状态
//! @param[in] scenario_path 一个包含场景文件路径的字符串，默认为空字符串
void Client::DoPlayerCommand(Command cmd, const CommandStatusCb& cb, const std::string& scenario_path,
                             const std::string& highlight_group) {
  SocketPtr sock(CreateSocket(ctx_->impl, addr_));
  zmq::pollitem_t poll_items[] = {{sock->handle(), 0, ZMQ_POLLIN, 0}};

  zmq::multipart_t msg;
  AddMsgType(buf_, cmd == kCmdStop ? kCmdUnSetup : cmd, msg);
  if (cmd == kCmdSetup) { msg.addstr(scenario_path); }
  if (cmd == kCmdHighlightGroup) { msg.addstr(highlight_group); }
  if (!msg.send(*sock)) {
    throw std::runtime_error("client could not sending request. maybe there is no peer at all.");
  }

  if (!msg.recv(*sock)) {  // expected first kCmdAccepted within appropriate timing.
    throw std::runtime_error("client could not receiving response. maybe there is no peer at all.");
  }
  CommandStatus status;
  // the first accepted message is a system confirm message and should not be delivered to user.
  // otherwise it indicates some error and should be delivered via callback.
  status.Decode(msg);
  if (status.ec != kCmdAccepted) {
    cb(status);
    return;  // command is rejected or some error occurred. no more message expected.
  }
  do {
    zmq::poll(poll_items, 1, 1000);
    if (poll_items[0].revents & ZMQ_POLLIN) {
      if (!msg.recv(*sock))
        throw std::runtime_error("no response received within " + std::to_string(SOCKET_TIMEOUT) +
                                 "ms. peer may not alive.");
      status.Decode(msg);
      if (NeedCallback(cmd, status)) {
        if (cmd == kCmdStop && status.ec == kCmdSucceed) status.ec = kCmdScenarioStopped;
        cb(status);
      }
    }
  } while (status.ec == kCmdAccepted);
}

//! @brief 函数名：DoManagerCommand
//! @details 函数功能：执行管理器命令
//!
//! @param[in] cmd 一个包含命令类型的Command对象
//! @param[in] msg 一个包含命令参数的multipart_t对象
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::DoManagerCommand(Command cmd, zmq::multipart_t& msg) {
  AddMsgType(buf_, cmd, msg, true);
  SocketPtr sock(CreateSocket(ctx_->impl, addr_));
  if (!msg.send(*sock)) {
    throw std::runtime_error("client could not sending request. maybe there is no peer at all.");
  }
  if (!msg.recv(*sock)) {  // expected one and only one error code response within appropriate timing.
    throw std::runtime_error("client could not receiving response. maybe there is no peer at all.");
  }
  return static_cast<CmdErrorCode>(PopMsgType(msg));
}

/**************************** wrapper implementation **************************/

//! @brief 播放器请求
//! @{
//! @brief 函数名：Setup
//! @details 函数功能：设置播放器场景
//!
//! @param[in] scenario_path 一个包含场景文件路径的字符串
//! @param[in] cb 一个回调函数，用于接收命令执行状态
void Client::Setup(const std::string& scenario_path, const tx_sim::coordinator::CommandStatusCb& cb) {
  DoPlayerCommand(kCmdSetup, cb, scenario_path);
}

//! @brief 函数名：UnSetup
//! @details 函数功能：取消播放器场景设置
//!
//! @param[in] cb 一个回调函数，用于接收命令执行状态
void Client::UnSetup(const tx_sim::coordinator::CommandStatusCb& cb) {
  DoPlayerCommand(kCmdUnSetup, cb);
}

//! @brief 函数名：Step
//! @details 函数功能：执行播放器单步操作
//!
//! @param[in] cb 一个回调函数，用于接收命令执行状态
void Client::Step(const tx_sim::coordinator::CommandStatusCb& cb) {
  DoPlayerCommand(kCmdStep, cb);
}

//! @brief 函数名：Stop
//! @details 函数功能：停止播放器操作
//!
//! @param[in] cb 一个回调函数，用于接收命令执行状态
void Client::Stop(const tx_sim::coordinator::CommandStatusCb& cb) {
  DoPlayerCommand(kCmdStop, cb);
}

//! @brief 函数名：Run
//! @details 函数功能：运行播放器
//!
//! @param[in] cb 一个回调函数，用于接收命令执行状态
void Client::Run(const tx_sim::coordinator::CommandStatusCb& cb) {
  DoPlayerCommand(kCmdRun, cb);
}

//! @brief 函数名：Pause
//! @details 函数功能：暂停播放器操作
//!
//! @param[in] cb 一个回调函数，用于接收命令执行状态
void Client::Pause(const tx_sim::coordinator::CommandStatusCb& cb) {
  DoPlayerCommand(kCmdPause, cb);
}

void Client::SetHighlightGroup(const tx_sim::coordinator::CommandStatusCb& cb, const std::string& group) {
  return DoPlayerCommand(kCmdHighlightGroup, cb, "", group);
}
//! @brief 模块管理请求
//! @{
//! @brief 函数名：LaunchModule
//! @details 函数功能：启动指定名称的模块
//!
//! @param[in] name 一个包含模块名称的字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::LaunchModule(const std::string& name) {
  zmq::multipart_t m;
  m.addstr(name);
  return DoManagerCommand(kCmdLaunchModule, m);
}

//! @brief 函数名：TerminateModule
//! @details 函数功能：终止指定名称的模块
//!
//! @param[in] name 一个包含模块名称的字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::TerminateModule(const std::string& name) {
  zmq::multipart_t m;
  m.addstr(name);
  return DoManagerCommand(kCmdTerminateModule, m);
}

//! @brief 配置请求
//! @{
//! @brief 函数名：AddModuleConfig
//! @details 函数功能：添加模块配置
//!
//! @param[in] config_json_str 一个包含模块配置信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::AddModuleConfig(const std::string& config_json_str) {
  zmq::multipart_t m;
  m.addstr(config_json_str);
  return DoManagerCommand(kCmdAddModuleConfig, m);
}

//! @brief 函数名：RemoveModuleConfig
//! @details 函数功能：移除指定名称的模块配置
//!
//! @param[in] name 一个包含模块名称的字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::RemoveModuleConfig(const std::string& name) {
  zmq::multipart_t m;
  m.addstr(name);
  return DoManagerCommand(kCmdRemoveModuleConfig, m);
}

//! @brief 函数名：UpdateModuleConfig
//! @details 函数功能：更新模块配置
//!
//! @param[in] config_json_str 一个包含模块配置信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::UpdateModuleConfig(const std::string& config_json_str) {
  zmq::multipart_t m;
  m.addstr(config_json_str);
  return DoManagerCommand(kCmdUpdateModuleConfig, m);
}

//! @brief 函数名：GetModuleConfig
//! @details 函数功能：获取指定名称的模块配置
//!
//! @param[in] name 一个包含模块名称的字符串
//! @param[out] config_json_str 一个包含模块配置信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::GetModuleConfig(const std::string& name, std::string& config_json_str) {
  zmq::multipart_t m;
  m.addstr(name);
  CmdErrorCode ec = DoManagerCommand(kCmdGetModuleConfig, m);
  if (ec == kCmdSucceed) config_json_str = PopMsgStr(m);
  return ec;
}

//! @brief 函数名：GetAllModuleConfigs
//! @details 函数功能：获取所有模块配置
//!
//! @param[out] configs_json_str 一个包含所有模块配置信息的JSON字符串数组
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::GetAllModuleConfigs(std::vector<std::string>& configs_json_str) {
  configs_json_str.clear();
  zmq::multipart_t m;
  CmdErrorCode ec = DoManagerCommand(kCmdGetAllModuleConfigs, m);
  if (ec == kCmdSucceed) UnloadListFromMessage(configs_json_str, m);
  return ec;
}

//! @brief 函数名：AddModuleScheme
//! @details 函数功能：添加模块方案
//!
//! @param[in] scheme_json_str 一个包含模块方案信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::AddModuleScheme(const std::string& scheme_json_str) {
  zmq::multipart_t m;
  m.addstr(scheme_json_str);
  return DoManagerCommand(kCmdAddModuleScheme, m);
}

//! @brief 函数名：RemoveModuleScheme
//! @details 函数功能：移除指定ID的模块方案
//!
//! @param[in] scheme_id 一个包含模块方案ID的整数
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::RemoveModuleScheme(int64_t scheme_id) {
  zmq::multipart_t m;
  AddMsgInt64(buf_, scheme_id, m);
  return DoManagerCommand(kCmdRemoveModuleScheme, m);
}

//! @brief 函数名：UpdateModuleScheme
//! @details 函数功能：更新模块方案
//!
//! @param[in] scheme_json_str 一个包含模块方案信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::UpdateModuleScheme(const std::string& scheme_json_str) {
  zmq::multipart_t m;
  m.addstr(scheme_json_str);
  return DoManagerCommand(kCmdUpdateModuleScheme, m);
}

//! @brief 函数名：GetAllModuleSchemes
//! @details 函数功能：获取所有模块方案
//!
//! @param[out] scheme_list_json_str 一个包含所有模块方案信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::GetAllModuleSchemes(std::string& scheme_list_json_str) {
  zmq::multipart_t m;
  CmdErrorCode ec = DoManagerCommand(kCmdGetAllModuleScheme, m);
  if (ec == kCmdSucceed) scheme_list_json_str = PopMsgStr(m);
  return ec;
}

tx_sim::impl::CmdErrorCode Client::UpdateMultiEgoScheme(const std::string& scheme_json_str) {
  zmq::multipart_t m;
  m.addstr(scheme_json_str);
  return DoManagerCommand(kCmdUpdateMultiEgoScheme, m);
}

tx_sim::impl::CmdErrorCode Client::GetMultiEgoScheme(std::string& scheme_json_str) {
  zmq::multipart_t m;
  CmdErrorCode ec = DoManagerCommand(kCmdGetMultiEgoScheme, m);
  if (ec == kCmdSucceed) scheme_json_str = PopMsgStr(m);
  return ec;
}

//! @brief 函数名：SetActiveModuleScheme
//! @details 函数功能：设置当前激活的模块方案
//!
//! @param[in] scheme_id 一个包含模块方案ID的整数
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::SetActiveModuleScheme(int64_t scheme_id) {
  zmq::multipart_t m;
  AddMsgInt64(buf_, scheme_id, m);
  return DoManagerCommand(kCmdSetActiveModuleScheme, m);
}

//! @brief 函数名：ExportModuleScheme
//! @details 函数功能：导出指定名称的模块方案
//!
//! @param[in] name 一个包含模块方案名称的字符串
//! @param[in] out_path 一个包含导出文件路径的字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::ExportModuleScheme(const std::string& name, const std::string& out_path) {
  zmq::multipart_t m;
  m.addstr(name);
  m.addstr(out_path);
  return DoManagerCommand(kCmdExportModuleScheme, m);
}

//! @brief 函数名：ImportModuleScheme
//! @details 函数功能：导入模块方案
//!
//! @param[in] in_path 一个包含导入文件路径的字符串
//! @param[out] imported_name 一个包含导入后模块方案名称的字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::ImportModuleScheme(const std::string& in_path, std::string& imported_name) {
  zmq::multipart_t m;
  m.addstr(in_path);
  CmdErrorCode ec = DoManagerCommand(kCmdImportModuleScheme, m);
  if (ec == kCmdSucceed || ec == kCmdSchemeAlreadyExist) imported_name = PopMsgStr(m);
  return ec;
}

//! @brief 函数名：SetSysConfigs
//! @details 函数功能：设置系统配置
//!
//! @param[in] config_json_str 一个包含系统配置信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::SetSysConfigs(const std::string& config_json_str) {
  zmq::multipart_t m;
  m.addstr(config_json_str);
  return DoManagerCommand(kCmdSetSysConfigs, m);
}

//! @brief 函数名：GetSysConfigs
//! @details 函数功能：获取系统配置
//!
//! @param[out] config_json_str 一个包含系统配置信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::GetSysConfigs(std::string& config_json_str) {
  zmq::multipart_t m;
  CmdErrorCode ec = DoManagerCommand(kCmdGetSysConfigs, m);
  if (ec == kCmdSucceed) config_json_str = PopMsgStr(m);
  return ec;
}

//! @brief 函数名：RestoreDefaultConfigs
//! @details 函数功能：恢复默认系统配置
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::RestoreDefaultConfigs() {
  zmq::multipart_t m;
  return DoManagerCommand(kCmdRestoreDefaultConfigs, m);
}

//! @brief 函数名：UpdatePlayList
//! @details 函数功能：更新播放列表
//!
//! @param[in] play_list_json_str 一个包含播放列表信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::UpdatePlayList(const std::string& play_list_json_str) {
  zmq::multipart_t m;
  m.addstr(play_list_json_str);
  return DoManagerCommand(kCmdUpdatePlayList, m);
}

//! @brief 函数名：GetPlayList
//! @details 函数功能：获取播放列表
//!
//! @param[out] play_list_json_str 一个包含播放列表信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::GetPlayList(std::string& play_list_json_str) {
  zmq::multipart_t m;
  CmdErrorCode ec = DoManagerCommand(kCmdGetPlayList, m);
  if (ec == kCmdSucceed) play_list_json_str = PopMsgStr(m);
  return ec;
}

//! @brief 函数名：RemoveScenariosFromPlayList
//! @details 函数功能：从播放列表中移除指定场景
//!
//! @param[in] removing_list_json_str 一个包含要移除的场景列表信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::RemoveScenariosFromPlayList(const std::string& removing_list_json_str) {
  zmq::multipart_t m;
  m.addstr(removing_list_json_str);
  return DoManagerCommand(kCmdRemoveScenariosFromPlayList, m);
}

//! @brief 函数名：AddGradingKpis
//! @details 函数功能：添加评分关键指标
//!
//! @param[in] kpis_json_str 一个包含评分关键指标信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::AddGradingKpis(const std::string& kpis_json_str) {
  zmq::multipart_t m;
  m.addstr(kpis_json_str);
  return DoManagerCommand(kCmdAddGradingKpi, m);
}

//! @brief 函数名：RemoveGradingKpi
//! @details 函数功能：移除指定ID的评分关键指标
//!
//! @param[in] kpi_id 一个包含评分关键指标ID的整数
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::RemoveGradingKpi(int64_t kpi_id) {
  zmq::multipart_t m;
  AddMsgInt64(buf_, kpi_id, m);
  return DoManagerCommand(kCmdRemoveGradingKpi, m);
}

//! @brief 函数名：UpdateGradingKpi
//! @details 函数功能：更新评分关键指标
//!
//! @param[in] kpi_json_str 一个包含评分关键指标信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::UpdateGradingKpi(const std::string& kpi_json_str) {
  zmq::multipart_t m;
  m.addstr(kpi_json_str);
  return DoManagerCommand(kCmdUpdateGradingKpi, m);
}

//! @brief 函数名：GetGradingKpisInGroup
//! @details 函数功能：获取指定分组下的评分关键指标
//!
//! @param[in] group_id 一个包含评分关键指标分组ID的整数
//! @param[out] kpis_json_str 一个包含评分关键指标信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::GetGradingKpisInGroup(int64_t group_id, std::string& kpis_json_str) {
  zmq::multipart_t m;
  AddMsgInt64(buf_, group_id, m);
  CmdErrorCode ec = DoManagerCommand(kCmdGetGradingKpisInGroup, m);
  if (ec == kCmdSucceed) kpis_json_str = PopMsgStr(m);
  return ec;
}

//! @brief 函数名：AddGradingKpiGroup
//! @details 函数功能：添加评分关键指标分组
//!
//! @param[in] kpi_group_json_str 一个包含评分关键指标分组信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::AddGradingKpiGroup(const std::string& kpi_group_json_str) {
  zmq::multipart_t m;
  m.addstr(kpi_group_json_str);
  return DoManagerCommand(kCmdAddGradingKpiGroup, m);
}

//! @brief 函数名：RemoveGradingKpiGroup
//! @details 函数功能：移除指定ID的评分关键指标分组
//!
//! @param[in] group_id 一个包含评分关键指标分组ID的整数
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::RemoveGradingKpiGroup(int64_t group_id) {
  zmq::multipart_t m;
  AddMsgInt64(buf_, group_id, m);
  return DoManagerCommand(kCmdRemoveGradingKpiGroup, m);
}

//! @brief 函数名：UpdateGradingKpiGroup
//! @details 函数功能：更新评分关键指标分组
//!
//! @param[in] kpi_group_json_str 一个包含评分关键指标分组信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::UpdateGradingKpiGroup(const std::string& kpi_group_json_str) {
  zmq::multipart_t m;
  m.addstr(kpi_group_json_str);
  return DoManagerCommand(kCmdUpdateGradingKpiGroup, m);
}

//! @brief 函数名：GetAllGradingKpiGroups
//! @details 函数功能：获取所有评分关键指标分组
//!
//! @param[out] kpi_groups_json_str 一个包含所有评分关键指标分组信息的JSON字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::GetAllGradingKpiGroups(std::string& kpi_groups_json_str) {
  zmq::multipart_t m;
  CmdErrorCode ec = DoManagerCommand(kCmdGetAllGradingKpiGroups, m);
  if (ec == kCmdSucceed) kpi_groups_json_str = PopMsgStr(m);
  return ec;
}

//! @brief 函数名：SetDefaultGradingKpiGroup
//! @details 函数功能：设置默认评分关键指标分组
//!
//! @param[in] group_id 一个包含评分关键指标分组ID的整数
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
CmdErrorCode Client::SetDefaultGradingKpiGroup(int64_t group_id) {
  zmq::multipart_t m;
  AddMsgInt64(buf_, group_id, m);
  return DoManagerCommand(kCmdSetDefaultGradingKpiGroup, m);
}

//! @brief 函数名：SetGradingLabel
//! @details 函数功能：设置评分标签
//!
//! @param[in] labels_str 一个包含评分标签信息的字符串
//!
//! @return 返回一个CmdErrorCode对象，表示命令执行状态
tx_sim::impl::CmdErrorCode Client::SetGradingLabel(const std::string& label_str) {
  zmq::multipart_t m;
  m.addstr(label_str);
  return DoManagerCommand(kCmdSetGradingKpiLabels, m);
}

}  // namespace cli
}  // namespace tx_sim
