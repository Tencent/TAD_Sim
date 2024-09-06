// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <stdint.h>
#include <sstream>
#include <string>

namespace tx_sim {
namespace impl {

//! @brief 枚举：CommandType
//! @details 描述：命令类型
enum Command : int16_t {
  //!
  //! @brief 命令类型：取消设置
  //!
  kCmdUnSetup = 0,
  //!
  //! @brief 命令类型：设置
  //!
  kCmdSetup = 1,
  //!
  //! @brief 命令类型：重置
  //!
  kCmdReset = 2,
  //!
  //! @brief 命令类型：步进
  //!
  kCmdStep = 3,
  //!
  //! @brief 命令类型：运行
  //!
  kCmdRun = 4,
  //!
  //! @brief 命令类型：暂停
  //!
  kCmdPause = 5,
  //!
  //! @brief 命令类型：停止
  //!
  kCmdStop = 6,
  kCmdHighlightGroup = 7,
  //!
  //! @brief 命令类型: 停止setup
  //! 中断算法初始过程比如init和setup阶段
  //!
  kCmdTerminateSteup = 8,
  //! @brief 命令类型：最大玩家类型
  //!
  kCmdMaxPlayerType = 100,
  //!
  //! @brief 命令类型：启动模块
  //!
  kCmdLaunchModule = 101,
  //!
  //! @brief 命令类型：终止模块
  //!
  kCmdTerminateModule = 102,
  //!
  //! @brief 命令类型：最大模块管理器类型
  //!
  kCmdMaxModuleMgrType = 200,
  //!
  //! @brief 命令类型：添加模块配置
  //!
  kCmdAddModuleConfig = 201,
  //!
  //! @brief 命令类型：删除模块配置
  //!
  kCmdRemoveModuleConfig = 202,
  //!
  //! @brief 命令类型：更新模块配置
  //!
  kCmdUpdateModuleConfig = 203,
  //!
  //! @brief 命令类型：获取模块配置
  //!
  kCmdGetModuleConfig = 204,
  //!
  //! @brief 命令类型：获取所有模块配置
  //!
  kCmdGetAllModuleConfigs = 205,
  //!
  //! @brief 命令类型：添加模块方案
  //!
  kCmdAddModuleScheme = 206,
  //!
  //! @brief 命令类型：删除模块方案
  //!
  kCmdRemoveModuleScheme = 207,
  //!
  //! @brief 命令类型：更新模块方案
  //!
  kCmdUpdateModuleScheme = 208,
  //!
  //! @brief 命令类型：获取所有模块方案
  //!
  kCmdGetAllModuleScheme = 209,
  //!
  //! @brief 命令类型：设置活动模块方案
  //!
  kCmdSetActiveModuleScheme = 210,
  //!
  //! @brief 命令类型：导出模块方案
  //!
  kCmdExportModuleScheme = 211,
  //!
  //! @brief 命令类型：导入模块方案
  //!
  kCmdImportModuleScheme = 212,
  //!
  //! @brief 命令类型：设置系统配置
  //!
  kCmdSetSysConfigs = 213,
  //!
  //! @brief 命令类型：获取系统配置
  //!
  kCmdGetSysConfigs = 214,
  //!
  //! @brief 命令类型：恢复默认配置
  //!
  kCmdRestoreDefaultConfigs = 215,
  //!
  //! @brief 命令类型：更新播放列表
  //!
  kCmdUpdatePlayList = 216,
  //!
  //! @brief 命令类型：获取播放列表
  //!
  kCmdGetPlayList = 217,
  //!
  //! @brief 命令类型：从播放列表中删除场景
  //!
  kCmdRemoveScenariosFromPlayList = 218,
  //!
  //! @brief 命令类型：添加评分关键性能指标
  //!
  kCmdAddGradingKpi = 219,
  //!
  //! @brief 命令类型：删除评分关键性能指标
  //!
  kCmdRemoveGradingKpi = 220,
  //!
  //! @brief 命令类型：更新评分关键性能指标
  //!
  kCmdUpdateGradingKpi = 221,
  //!
  //! @brief 命令类型：获取评分关键性能指标组
  //!
  kCmdGetGradingKpisInGroup = 222,
  //!
  //! @brief 命令类型：添加评分关键性能指标组
  //!
  kCmdAddGradingKpiGroup = 223,
  //!
  //! @brief 命令类型：删除评分关键性能指标组
  //!
  kCmdRemoveGradingKpiGroup = 224,
  //!
  //! @brief 命令类型：更新评分关键性能指标组
  //!
  kCmdUpdateGradingKpiGroup = 225,
  //!
  //! @brief 命令类型：获取所有评分关键性能指标组
  //!
  kCmdGetAllGradingKpiGroups = 226,
  //!
  //! @brief 命令类型：设置默认评分关键性能指标组
  //!
  kCmdSetDefaultGradingKpiGroup = 227,
  //!
  //! @brief 命令类型：设置评分关键性能指标标签
  //!
  kCmdSetGradingKpiLabels = 228,
  //!
  //! @brief 命令类型：获取PB信息
  //!
  kCmdGetPbInfo = 229,
  //!
  //! @brief 命令类型：删除PB信息
  //!
  kCmdDelPbInfo = 230,
  //!
  //! @brief 命令类型：更新PB信息
  //!
  kCmdUpdatePbInfo = 231,
  //!
  //! @brief 命令类型：最大配置管理器类型
  //!
  kCmdMaxConfigMgrType = 300,
  kCmdUpdateMultiEgoScheme = 301,
  kCmdGetMultiEgoScheme = 302,
};

inline std::string Enum2String(Command c) {
  switch (c) {
    //!
    //! @brief 条件类型：启动
    //! @details 返回：返回客户端启动命令
    //!
    case kCmdSetup: return "CMD_SETUP";
    //!
    //! @brief 条件类型：停止
    //! @details 返回：返回客户端停止命令
    //!
    case kCmdUnSetup: return "CMD_UNSETUP";
    //!
    //! @brief 条件类型：重置
    //! @details 返回：返回客户端重置命令
    //!
    case kCmdReset: return "CMD_RESET";
    //!
    //! @brief 条件类型：步进
    //! @details 返回：返回客户端步进命令
    //!
    case kCmdStep: return "CMD_STEP";

    //!
    //! @brief 条件类型：运行
    //! @details 返回：返回客户端运行命令
    //!
    case kCmdRun: return "CMD_RUN";
    //!
    //! @brief 条件类型：暂停
    //! @details 返回：返回客户端暂停命令
    //!
    case kCmdPause: return "CMD_PAUSE";
    //!
    //! @brief 条件类型：停止
    //! @details 返回：返回客户端停止命令
    //!
    case kCmdStop: return "CMD_STOP";
    //!
    //! @brief 条件类型：启动模块
    //! @details 返回：返回客户端启动模块命令
    //!
    case kCmdLaunchModule: return "CMD_LAUNCH_MODULE";
    //!
    //! @brief 条件类型：终止模块
    //! @details 返回：返回客户端终止模块命令
    //!
    case kCmdTerminateModule: return "CMD_TERM_MODULE";
    //!
    //! @brief 条件类型：添加模块配置
    //! @details 返回：返回客户端添加模块配置命令
    //!
    case kCmdAddModuleConfig: return "CMD_ADD_MODULE_CONFIG";
    //!
    //! @brief 条件类型：移除模块配置
    //! @details 返回：返回客户端移除模块配置命令
    //!
    case kCmdRemoveModuleConfig: return "CMD_REMOVE_MODULE_CONFIG";
    //!
    //! @brief 条件类型：更新模块配置
    //! @details 返回：返回客户端更新模块配置命令
    //!
    case kCmdUpdateModuleConfig: return "CMD_UPDATE_MODULE_CONFIG";
    //!
    //! @brief 条件类型：获取模块配置
    //! @details 返回：返回客户端获取模块配置命令
    //!
    case kCmdGetModuleConfig: return "CMD_GET_MODULE_CONFIG";
    //!
    //! @brief 条件类型：获取所有模块配置
    //! @details 返回：返回客户端获取所有模块配置命令
    //!
    case kCmdGetAllModuleConfigs: return "CMD_GET_ALL_MODULE_CONFIGS";
    //!
    //! @brief 条件类型：添加模块方案
    //! @details 返回：返回客户端添加模块方案命令
    //!
    case kCmdAddModuleScheme: return "CMD_ADD_MODULE_SCHEME";

    //!
    //! @brief 条件类型：移除模块方案
    //! @details 返回：返回客户端移除模块方案命令
    //!
    case kCmdRemoveModuleScheme: return "CMD_REMOVE_MODULE_SCHEME";
    //!
    //! @brief 条件类型：更新模块方案
    //! @details 返回：返回客户端更新模块方案命令
    //!
    case kCmdUpdateModuleScheme: return "CMD_UPDATE_MODULE_SCHEME";
    //!
    //! @brief 条件类型：获取所有模块方案
    //! @details 返回：返回客户端获取所有模块方案命令
    //!
    case kCmdGetAllModuleScheme: return "CMD_GET_ALL_MODULE_SCHEME";
    //!
    //! @brief 条件类型：设置活动模块方案
    //! @details 返回：返回客户端设置活动模块方案命令
    //!
    case kCmdSetActiveModuleScheme: return "CMD_SET_ACTIVE_MODULE_SCHEME";
    //!
    //! @brief 条件类型：导出模块方案
    //! @details 返回：返回客户端导出模块方案命令
    //!
    case kCmdExportModuleScheme: return "CMD_EXPORT_MODULE_SCHEME";
    //!
    //! @brief 条件类型：导入模块方案
    //! @details 返回：返回客户端导入模块方案命令
    //!
    case kCmdImportModuleScheme: return "CMD_IMPORT_MODULE_SCHEME";
    //!
    //! @brief 条件类型：设置系统配置
    //! @details 返回：返回客户端设置系统配置命令
    //!
    case kCmdSetSysConfigs: return "CMD_SET_SYS_CONFIGS";
    //!
    //! @brief 条件类型：获取系统配置
    //! @details 返回：返回客户端获取系统配置命令
    //!
    case kCmdGetSysConfigs: return "CMD_GET_SYS_CONFIGS";
    //!
    //! @brief 条件类型：恢复默认配置
    //! @details 返回：返回客户端恢复默认配置命令
    //!
    case kCmdRestoreDefaultConfigs: return "CMD_RESTORE_DEFAULT_CONFIGS";
    //!
    //! @brief 条件类型：更新播放列表
    //! @details 返回：返回客户端更新播放列表命令
    //!
    case kCmdUpdatePlayList: return "CMD_UPDATE_PLAY_LIST";
    //!
    //! @brief 条件类型：获取播放列表
    //! @details 返回：返回客户端获取播放列表命令
    //!
    case kCmdGetPlayList: return "CMD_GET_PLAY_LIST";

    //!
    //! @brief 条件类型：从播放列表中移除场景
    //! @details 返回：返回客户端从播放列表中移除场景命令
    //!
    case kCmdRemoveScenariosFromPlayList: return "CMD_REMOVE_SCENARIO_FROM_PLAY_LIST";
    //!
    //! @brief 条件类型：添加评估KPI
    //! @details 返回：返回客户端添加评估KPI命令
    //!
    case kCmdAddGradingKpi: return "CMD_ADD_GRADING_KPI";
    //!
    //! @brief 条件类型：移除评估KPI
    //! @details 返回：返回客户端移除评估KPI命令
    //!
    case kCmdRemoveGradingKpi: return "CMD_REMOVE_GRADING_KPI";
    //!
    //! @brief 条件类型：更新评估KPI
    //! @details 返回：返回客户端更新评估KPI命令
    //!
    case kCmdUpdateGradingKpi: return "CMD_UPDATE_GRADING_KPI";
    //!
    //! @brief 条件类型：获取评估KPI组中的KPI
    //! @details 返回：返回客户端获取评估KPI组中的KPI命令
    //!
    case kCmdGetGradingKpisInGroup: return "CMD_GET_GRADING_KPIS_IN_GROUP";
    //!
    //! @brief 条件类型：添加评估KPI组
    //! @details 返回：返回客户端添加评估KPI组命令
    //!
    case kCmdAddGradingKpiGroup: return "CMD_ADD_GRADING_KPI_GROUP";
    //!
    //! @brief 条件类型：移除评估KPI组
    //! @details 返回：返回客户端移除评估KPI组命令
    //!
    case kCmdRemoveGradingKpiGroup: return "CMD_REMOVE_GRADING_KPI_GROUP";
    //!
    //! @brief 条件类型：更新评估KPI组
    //! @details 返回：返回客户端更新评估KPI组命令
    //!
    case kCmdUpdateGradingKpiGroup: return "CMD_UPDATE_GRADING_KPI_GROUP";
    //!
    //! @brief 条件类型：获取所有评估KPI组
    //! @details 返回：返回客户端获取所有评估KPI组命令
    //!
    case kCmdGetAllGradingKpiGroups: return "CMD_GET_ALL_GRADING_KPI_GROUPS";
    //!
    //! @brief 条件类型：设置默认评估KPI组
    //! @details 返回：返回客户端设置默认评估KPI组命令
    //!
    case kCmdSetDefaultGradingKpiGroup: return "CMD_SET_DEFAULT_GRADING_KPI_GROUP";
    case kCmdUpdateMultiEgoScheme: return "CMD_UPDATE_MULTIEGO_SCHEME";
    case kCmdGetMultiEgoScheme: return "CMD_GET_MULTIEGO_SCHEME";
    case kCmdHighlightGroup: return "CMD_HIGHLIGHT_GROUP";
    default: return std::to_string(static_cast<int16_t>(c));
  }
}

enum CmdErrorCode : int16_t {
  //!
  //! @brief 命令状态：命令已接受
  //! @details 返回：返回命令已接受的枚举值
  //!
  kCmdAccepted = 0,
  //!
  //! @brief 命令状态：命令被忽略
  //! @details 返回：返回命令被忽略的枚举值
  //!
  kCmdIgnored = 2,
  //!
  //! @brief 命令状态：命令被取消
  //! @details 返回：返回命令被取消的枚举值
  //!
  kCmdCancelled = 3,
  //!
  //! @brief 命令状态：命令被拒绝
  //! @details 返回：返回命令被拒绝的枚举值
  //!
  kCmdRejected = 10,
  //!
  //! @brief 命令状态：服务器繁忙
  //! @details 返回：返回服务器繁忙的枚举值
  //!
  kCmdServerBusy = 20,
  //!
  //! @brief 命令状态：命令成功
  //! @details 返回：返回命令成功的枚举值
  //!
  kCmdSucceed = 100,
  //!
  //! @brief 命令状态：方案已存在
  //! @details 返回：返回方案已存在的枚举值
  //!
  kCmdSchemeAlreadyExist = 200,
  //!
  //! @brief 命令状态：场景超时
  //! @details 返回：返回场景超时的枚举值
  //!
  kCmdScenarioTimeout = 500,
  //!
  //! @brief 命令状态：场景已停止
  //! @details 返回：返回场景已停止的枚举值
  //!
  kCmdScenarioStopped = 900,
  //!
  //! @brief 命令状态：无效的主题发布订阅
  //! @details 返回：返回无效的主题发布订阅的枚举值
  //!
  kCmdInvalidTopicPubSub = 1500,
  //!
  //! @brief 命令状态：命令失败
  //! @details 返回：返回命令失败的枚举值
  //!
  kCmdFailed = 3000,
  //!
  //! @brief 命令状态：场景解析错误
  //! @details 返回：返回场景解析错误的枚举值
  //!
  kCmdScenarioParsingError = 20000,
  //!
  //! @brief 命令状态：日志回放模块未找到
  //! @details 返回：返回日志回放模块未找到的枚举值
  //!
  kCmdLogPlayModuleNotFound = 20100,
  //!
  //! @brief 命令状态：日志回放与影像模块冲突
  //! @details 返回：返回日志回放与影像模块冲突的枚举值
  //!
  kCmdConfictLogPlayShadowModule = 20200,
  //!
  //! @brief 命令状态：无效的模块配置
  //! @details 返回：返回无效的模块配置的枚举值
  //!
  kCmdInvalidModuleConfig = 20300,
  //!
  //! @brief 命令状态：未足够的初始化模块
  //! @details 返回：返回未足够的初始化模块的枚举值
  //!
  kCmdUnEnoughInitiatedModule = 20400,
  //!
  //! @brief 命令状态：共享内存创建错误
  //! @details 返回：返回共享内存创建错误的枚举值
  //!
  kCmdShmemCreationError = 29000,
  //!
  //! @brief 命令状态：系统错误
  //! @details 返回：返回系统错误的枚举值
  //!
  kCmdSystemError = 30000,
};

inline std::string Enum2String(CmdErrorCode c) {
  switch (c) {
    //!
    //! @brief 条件类型：命令成功
    //! @details 返回：返回命令成功的字符串
    //!
    case kCmdSucceed: return "CMD_SUCCEED";
    //!
    //! @brief 条件类型：命令已接受
    //! @details 返回：返回命令已接受的字符串
    //!
    case kCmdAccepted: return "CMD_ACCEPTED";
    //!
    //! @brief 条件类型：命令已取消
    //! @details 返回：返回命令已取消的字符串
    //!
    case kCmdCancelled: return "CMD_CANCELLED";
    //!
    //! @brief 条件类型：命令被忽略
    //! @details 返回：返回命令被忽略的字符串
    //!
    case kCmdIgnored: return "CMD_IGNORED";
    //!
    //! @brief 条件类型：场景已停止
    //! @details 返回：返回场景已停止的字符串
    //!
    case kCmdScenarioStopped: return "CMD_SCENARIO_STOPPED";
    //!
    //! @brief 条件类型：命令被拒绝
    //! @details 返回：返回命令被拒绝的字符串
    //!
    case kCmdRejected: return "CMD_REJECTED";
    //!
    //! @brief 条件类型：命令失败
    //! @details 返回：返回命令失败的字符串
    //!
    case kCmdFailed: return "CMD_FAILED";
    //!
    //! @brief 条件类型：系统错误
    //! @details 返回：返回系统错误的字符串
    //!
    case kCmdSystemError: return "CMD_SYS_ERROR";
    //!
    //! @brief 条件类型：无效的主题发布订阅
    //! @details 返回：返回无效的主题发布订阅的字符串
    //!
    case kCmdInvalidTopicPubSub: return "CMD_INVALID_TOPIC_PUBSUB";
    //!
    //! @brief 条件类型：场景解析错误
    //! @details 返回：返回场景解析错误的字符串
    //!
    case kCmdScenarioParsingError: return "CMD_SCENARIO_PARSING_ERROR";
    //!
    //! @brief 条件类型：场景超时
    //! @details 返回：返回场景超时的字符串
    //!
    case kCmdScenarioTimeout: return "CMD_SCENARIO_TIMEOUT";
    //!
    //! @brief 条件类型：共享内存创建错误
    //! @details 返回：返回共享内存创建错误的字符串
    //!
    case kCmdShmemCreationError: return "CMD_SHMEM_CREATION_ERROR";
    //!
    //! @brief 条件类型：服务器繁忙
    //! @details 返回：返回服务器繁忙的字符串
    //!
    case kCmdServerBusy: return "CMD_SERVER_BUSY";
    //!
    //! @brief 条件类型：日志回放模块未找到
    //! @details 返回：返回日志回放模块未找到的字符串
    //!
    case kCmdLogPlayModuleNotFound: return "CMD_LOG_PLAY_MODULE_NOT_FOUND";
    //!
    //! @brief 条件类型：日志回放与影像模块冲突
    //! @details 返回：返回日志回放与影像模块冲突的字符串
    //!
    case kCmdConfictLogPlayShadowModule: return "CMD_CONFICT_LOGPLAY_SHADOW_MODULE";
    //!
    //! @brief 条件类型：无效的模块配置
    //! @details 返回：返回无效的模块配置的字符串
    //!
    case kCmdInvalidModuleConfig: return "CMD_INVALID_MODULE_CONFIG";
    //!
    //! @brief 条件类型：方案已存在
    //! @details 返回：返回方案已存在的字符串
    //!
    case kCmdSchemeAlreadyExist: return "CMD_SCHEME_ALREADY_EXIST";
    //!
    //! @brief 条件类型：未知命令错误代码
    //! @details 返回：返回未知命令错误代码的字符串
    //!
    default: return "UNKNOWN_CMD_ERR_CODE";
  }
}

enum ModuleInitState : int16_t {
  //!
  //! @brief 模块初始化状态：初始化开始
  //! @details 返回：返回模块初始化开始的枚举值
  //!
  kModuleInitStart = 0,
  //!
  //! @brief 模块初始化状态：连接中
  //! @details 返回：返回模块初始化连接中的枚举值
  //!
  kModuleInitConnecting = 2,
  //!
  //! @brief 模块初始化状态：已连接
  //! @details 返回：返回模块初始化已连接的枚举值
  //!
  kModuleInitConnected = 3,
  //!
  //! @brief 模块初始化状态：请求已发送
  //! @details 返回：返回模块初始化请求已发送的枚举值
  //!
  kModuleInitReqSent = 4,
  //!
  //! @brief 模块初始化状态：初始化成功
  //! @details 返回：返回模块初始化成功的枚举值
  //!
  kModuleInitSucceed = 5,
  //!
  //! @brief 模块初始化状态：初始化失败
  //! @details 返回：返回模块初始化失败的枚举值
  //!
  kModuleInitFailed = 6,
};

inline std::string Enum2String(ModuleInitState s) {
  switch (s) {
    //!
    //! @brief 条件类型：模块初始化开始
    //! @details 返回：返回模块初始化开始的字符串
    //!
    case kModuleInitStart: return "MODULE_INIT_START";
    //!
    //! @brief 条件类型：模块初始化连接中
    //! @details 返回：返回模块初始化连接中的字符串
    //!
    case kModuleInitConnecting: return "MODULE_INIT_CONNECTING";
    //!
    //! @brief 条件类型：模块初始化已连接
    //! @details 返回：返回模块初始化已连接的字符串
    //!
    case kModuleInitConnected: return "MODULE_INIT_CONNECTED";
    //!
    //! @brief 条件类型：模块初始化请求已发送
    //! @details 返回：返回模块初始化请求已发送的字符串
    //!
    case kModuleInitReqSent: return "MODULE_INIT_REQ_SENT";
    //!
    //! @brief 条件类型：模块初始化成功
    //! @details 返回：返回模块初始化成功的字符串
    //!
    case kModuleInitSucceed: return "MODULE_INIT_SUCCEED";
    //!
    //! @brief 条件类型：模块初始化失败
    //! @details 返回：返回模块初始化失败的字符串
    //!
    case kModuleInitFailed: return "MODULE_INIT_FAILED";
    //!
    //! @brief 条件类型：未知模块初始化类型
    //! @details 返回：返回未知模块初始化类型的字符串
    //!
    default: return "UNKNOWN_MODULE_INIT_TYPE";
  }
}

enum ModuleMessageType : int16_t {
  //!
  //! @brief 模块注册状态：注册成功
  //! @details 返回：返回模块注册成功的枚举值
  //!
  kModuleRegisterSucceed = 0,
  //!
  //! @brief 模块注册状态：注册失败
  //! @details 返回：返回模块注册失败的枚举值
  //!
  kModuleRegisterFailed = 1,
  //!
  //! @brief 模块注册状态：注册
  //! @details 返回：返回模块注册的枚举值
  //!
  kModuleRegister = 2,
  //!
  //! @brief 模块注册状态：注销
  //! @details 返回：返回模块注销的枚举值
  //!
  kModuleUnRegister = 3,
  //!
  //! @brief 模块注册状态：心跳
  //! @details 返回：返回模块心跳的枚举值
  //!
  kModuleHeartbeat = 5,
  //!
  //! @brief 模块注册状态：心跳断开中
  //! @details 返回：返回模块心跳断开中的枚举值
  //!
  kModuleHeartbeatDisconnecting = 6,
  //!
  //! @brief 模块注册状态：请求
  //! @details 返回：返回模块请求的枚举值
  //!
  kModuleRequest = 7,
  //!
  //! @brief 模块注册状态：响应
  //! @details 返回：返回模块响应的枚举值
  //!
  kModuleResponse = 8,
  //!
  //! @brief 模块注册状态：版本不兼容
  //! @details 返回：返回模块版本不兼容的枚举值
  //!
  kModuleConnVersionIncompatible = 10000,
  //!
  //! @brief 模块注册状态：消息错误类型
  //! @details 返回：返回模块消息错误类型的枚举值
  //!
  kModuleMessageErrorType = 30000,
};

inline std::string Enum2String(ModuleMessageType r) {
  switch (r) {
    //!
    //! @brief 条件类型：模块注册
    //! @details 返回：返回模块注册的字符串
    //!
    case kModuleRegister: return "MODULE_REGISTER";
    //!
    //! @brief 条件类型：模块注销
    //! @details 返回：返回模块注销的字符串
    //!
    case kModuleUnRegister: return "MODULE_UNREGISTER";
    //!
    //! @brief 条件类型：模块心跳
    //! @details 返回：返回模块心跳的字符串
    //!
    case kModuleHeartbeat: return "MODULE_HEARTBEAT";
    //!
    //! @brief 条件类型：模块响应
    //! @details 返回：返回模块响应的字符串
    //!
    case kModuleResponse: return "MODULE_RESPONSE";
    //!
    //! @brief 条件类型：未知模块消息类型
    //! @details 返回：返回未知模块消息类型的字符串
    //!
    default: return std::string("UNKNOWN_MODULE_MESSAGE_TYPE:") + std::to_string(r);
  }
}

enum ModuleRequestType : int16_t {
  //!
  //! @brief 模块控制状态：初始化
  //! @details 返回：返回模块控制初始化的枚举值
  //!
  kModuleInit = 1,
  //!
  //! @brief 模块控制状态：重置
  //! @details 返回：返回模块控制重置的枚举值
  //!
  kModuleReset = 2,
  //!
  //! @brief 模块控制状态：步进
  //! @details 返回：返回模块控制步进的枚举值
  //!
  kModuleStep = 3,
  //!
  //! @brief 模块控制状态：停止
  //! @details 返回：返回模块控制停止的枚举值
  //!
  kModuleStop = 4,
};

enum ModuleResponseType : int16_t {
  //!
  //! @brief 模块状态：正常
  //! @details 返回：返回模块状态正常的枚举值
  //!
  kModuleOK = 0,
  //!
  //! @brief 模块状态：需要停止
  //! @details 返回：返回模块状态需要停止的枚举值
  //!
  kModuleRequireStop = 1,
  //!
  //! @brief 模块状态：超时
  //! @details 返回：返回模块状态超时的枚举值
  //!
  kModuleTimeout = 2,
  //!
  //! @brief 模块状态：错误
  //! @details 返回：返回模块状态错误的枚举值
  //!
  kModuleError = 3,
  //!
  //! @brief 模块状态：版本不兼容
  //! @details 返回：返回模块状态版本不兼容的枚举值
  //!
  kModuleVersionIncompatible = 4,
  //!
  //! @brief 模块状态：系统错误
  //! @details 返回：返回模块状态系统错误的枚举值
  //!
  kModuleSystemError = 5,
  //!
  //! @brief 模块状态：需要重新初始化
  //! @details 返回：返回模块状态需要重新初始化的枚举值
  //!
  kModuleReInit = 6,
  //!
  //! @brief 模块状态：进程退出
  //! @details 返回：返回模块状态进程退出的枚举值
  //!
  kModuleProcessExit = 7,
};

inline std::string Enum2String(ModuleResponseType t) {
  switch (t) {
    //!
    //! @brief 条件类型：模块正常
    //! @details 返回：返回模块正常的字符串
    //!
    case kModuleOK: return "MODULE_OK";
    //!
    //! @brief 条件类型：模块需要停止
    //! @details 返回：返回模块需要停止的字符串
    //!
    case kModuleRequireStop: return "MODULE_REQUIRE_STOP";
    //!
    //! @brief 条件类型：模块超时
    //! @details 返回：返回模块超时的字符串
    //!
    case kModuleTimeout: return "MODULE_TIMEOUT";
    //!
    //! @brief 条件类型：模块错误
    //! @details 返回：返回模块错误的字符串
    //!
    case kModuleError: return "MODULE_ERROR";
    //!
    //! @brief 条件类型：模块版本不兼容
    //! @details 返回：返回模块版本不兼容的字符串
    //!
    case kModuleVersionIncompatible: return "MODULE_VERSION_INCOMPATIBLE";
    //!
    //! @brief 条件类型：模块系统错误
    //! @details 返回：返回模块系统错误的字符串
    //!
    case kModuleSystemError: return "MODULE_SYSTEM_ERROR";
    //!
    //! @brief 条件类型：模块需要重新初始化
    //! @details 返回：返回模块需要重新初始化的字符串
    //!
    case kModuleReInit: return "MODULE_REINIT";
    //!
    //! @brief 条件类型：模块进程退出
    //! @details 返回：返回模块进程退出的字符串
    //!
    case kModuleProcessExit: return "MODULE_PROCESS_EXIT";
    //!
    //! @brief 条件类型：未知模块响应类型
    //! @details 返回：返回未知模块响应类型的字符串
    //!
    default: return "UNKNOWN_MODULE_RESP_TYPE";
  }
}

inline bool ModuleResponseSucceed(ModuleResponseType t) {
  return t == kModuleOK || t == kModuleRequireStop;
}

enum CoordinationMode : int16_t {
  //!
  //! @brief 坐标系类型：未知
  //! @details 返回：返回坐标系类型未知的枚举值
  //!
  kCoordinateUnknown = 0,
  //!
  //! @brief 坐标系类型：同步
  //! @details 返回：返回坐标系类型同步的枚举值
  //!
  kCoordinateSync = 1,
  //!
  //! @brief 坐标系类型：异步
  //! @details 返回：返回坐标系类型异步的枚举值
  //!
  kCoordinateAsync = 2,
  //!
  //! @brief 坐标系类型：优先级
  //! @details 返回：返回坐标系类型优先级的枚举值
  //!
  kCoordinatePriority = 3,
};

inline CoordinationMode Int2CoordMode(int16_t v) {
  switch (v) {
    //!
    //! @brief 条件类型：坐标同步
    //! @details 返回：返回坐标同步的枚举值
    //!
    case 1: return kCoordinateSync;
    //!
    //! @brief 条件类型：坐标异步
    //! @details 返回：返回坐标异步的枚举值
    //!
    case 2: return kCoordinateAsync;
    //!
    //! @brief 条件类型：坐标优先
    //! @details 返回：返回坐标优先的枚举值
    //!
    case 3: return kCoordinatePriority;
    //!
    //! @brief 条件类型：未知坐标类型
    //! @details 返回：返回未知坐标类型的枚举值
    //!
    default: return kCoordinateUnknown;
  }
}

enum Log2WorldTopicMode : int16_t {
  //!
  //! @brief Log2World主题：日志记录
  //! @details 返回：返回Log2World主题日志记录的枚举值
  //!
  kLog2WorldTopicLogging = 1,
  //!
  //! @brief Log2World主题：仿真
  //! @details 返回：返回Log2World主题仿真的枚举值
  //!
  kLog2WorldTopicSim = 2,
  //!
  //! @brief Log2World主题：日志记录和仿真
  //! @details 返回：返回Log2World主题日志记录和仿真的枚举值
  //!
  kLog2WorldTopicBoth = 3,
  //!
  //! @brief Log2World主题：日志记录到仿真
  //! @details 返回：返回Log2World主题日志记录到仿真的枚举值
  //!
  kLog2WorldTopicLog2Sim = 4,
};

inline Log2WorldTopicMode Int2TopicMode(int16_t v) {
  switch (v) {
    //!
    //! @brief 条件类型：日志记录
    //! @details 返回：返回日志记录的枚举值
    //!
    case 1: return kLog2WorldTopicLogging;
    //!
    //! @brief 条件类型：仿真
    //! @details 返回：返回仿真的枚举值
    //!
    case 2: return kLog2WorldTopicSim;
    //!
    //! @brief 条件类型：日志记录和仿真
    //! @details 返回：返回日志记录和仿真的枚举值
    //!
    case 3: return kLog2WorldTopicBoth;
    //!
    //! @brief 条件类型：日志记录到仿真
    //! @details 返回：返回日志记录到仿真的枚举值
    //!
    case 4: return kLog2WorldTopicLog2Sim;
    //!
    //! @brief 条件类型：未知日志记录到仿真主题模式
    //! @details 返回：抛出异常，提示未知日志记录到仿真主题模式
    //!
    default: throw std::invalid_argument("unknown log2world topic mode: " + v);
  }
}

enum Log2WorldSwitchType : int16_t {
  //!
  //! @brief Log2World切换类型：场景
  //! @details 返回：返回Log2World切换类型场景的枚举值
  //!
  kLog2WorldSwitchScene = 1,

  //!
  //! @brief Log2World切换类型：事件
  //! @details 返回：返回Log2World切换类型事件的枚举值
  //!
  kLog2WorldSwitchEvent = 2,
  //!
  //! @brief Log2World切换类型：交通车自动切换
  //! @details 返回：返回Log2World切换类型事件的枚举值
  //!
  kLog2WorldManualVehicle = 3,
};

inline Log2WorldSwitchType Int2Log2WorldSwitchType(int16_t v) {
  switch (v) {
    //!
    //! @brief 条件类型：日志记录到仿真场景切换
    //! @details 返回：返回日志记录到仿真场景切换的枚举值
    //!
    case 1: return kLog2WorldSwitchScene;
    //!
    //! @brief 条件类型：日志记录到仿真事件切换
    //! @details 返回：返回日志记录到仿真事件切换的枚举值
    //!
    case 2: return kLog2WorldSwitchEvent;
    //!
    //! @brief 条件类型：交通车自动切换
    //! @details
    //!
    case 3: return kLog2WorldManualVehicle;
    //!
    //! @brief 条件类型：未知日志记录到仿真切换类型
    //! @details 返回：抛出异常，提示未知日志记录到仿真切换类型
    //!
    default: throw std::invalid_argument("unknown log2world switch type: " + v);
  }
}

enum ModuleCategory : int16_t {
  //!
  //! @brief 模块类别：未知
  //! @details 返回：返回模块类别未知的枚举值
  //!
  kModuleCategoryUnknown = 0,
  //!
  //! @brief 模块类别：真值
  //! @details 返回：返回模块类别真值的枚举值
  //!
  kModuleCategoryTruthValue = 1,
  //!
  //! @brief 模块类别：评分
  //! @details 返回：返回模块类别评分的枚举值
  //!
  kModuleCategoryGrading = 51,
  //!
  //! @brief 模块类别：感知
  //! @details 返回：返回模块类别感知的枚举值
  //!
  kModuleCategoryPerception = 101,
  //!
  //! @brief 模块类别：预测
  //! @details 返回：返回模块类别预测的枚举值
  //!
  kModuleCategoryPrediction = 102,
  //!
  //! @brief 模块类别：规划
  //! @details 返回：返回模块类别规划的枚举值
  //!
  kModuleCategoryPlanning = 103,
  //!
  //! @brief 模块类别：控制
  //! @details 返回：返回模块类别控制的枚举值
  //!
  kModuleCategoryControlling = 104,
  //!
  //! @brief 模块类别：车辆动力学
  //! @details 返回：返回模块类别车辆动力学的枚举值
  //!
  kModuleCategoryVehicleDynamics = 105,
  //! @brief 模块类别：车辆算法
  //! @details 返回：返回模块类别车辆算法的枚举值
  //!
  kModuleCategoryVehicleAlgorithm = 106,
  //!
  //! @brief 模块类别：L2
  //! @details 返回：返回模块类别L2的枚举值
  //!
  kModuleCategoryL2 = 107,
  //!
  //! @brief 模块类别：L3
  //! @details 返回：返回模块类别L3的枚举值
  //!
  kModuleCategoryL3 = 108,
  //!
  //! @brief 模块类别：L4
  //! @details 返回：返回模块类别L4的枚举值
  //!
  kModuleCategoryL4 = 109,
  //!
  //! @brief 模块类别：定位
  //! @details 返回：返回模块类别定位的枚举值
  //!
  kModuleCategoryLocation = 110,
  //!
  //! @brief 模块类别：驾驶员
  //! @details 返回：返回模块类别驾驶员的枚举值
  //!
  kModuleCategoryDriver = 111,
  //!
  //! @brief 模块类别：传感器
  //! @details 返回：返回模块类别传感器的枚举值
  //!
  kModuleCategorySensor = 112,
  //!
  //! @brief 模块类别：XIL
  //! @details 返回：返回模块类别XIL的枚举值
  //!
  kModuleCategoryXIL = 113,
  //!
  //! @brief 模块类别：工具
  //! @details 返回：返回模块类别工具的枚举值
  //!
  kModuleCategoryTool = 1001,
};

inline ModuleCategory Int2ModuleCategory(int16_t v) {
  switch (v) {
    //!
    //! @brief 条件类型：模块类别真值
    //! @details 返回：返回模块类别真值的枚举值
    //!
    case 1: return kModuleCategoryTruthValue;
    //!
    //! @brief 条件类型：模块类别评分
    //! @details 返回：返回模块类别评分的枚举值
    //!
    case 51: return kModuleCategoryGrading;
    //!
    //! @brief 条件类型：模块类别感知
    //! @details 返回：返回模块类别感知的枚举值
    //!
    case 101: return kModuleCategoryPerception;
    //!
    //! @brief 条件类型：模块类别预测
    //! @details 返回：返回模块类别预测的枚举值
    //!
    case 102: return kModuleCategoryPrediction;
    //!
    //! @brief 条件类型：模块类别规划
    //! @details 返回：返回模块类别规划的枚举值
    //!
    case 103: return kModuleCategoryPlanning;
    //!
    //! @brief 条件类型：模块类别控制
    //! @details 返回：返回模块类别控制的枚举值
    //!
    case 104: return kModuleCategoryControlling;
    //!
    //! @brief 条件类型：模块类别车辆动力学
    //! @details 返回：返回模块类别车辆动力学的枚举值
    //!
    case 105: return kModuleCategoryVehicleDynamics;
    //!
    //! @brief 条件类型：模块类别车辆算法
    //! @details 返回：返回模块类别车辆算法的枚举值
    //!
    case 106: return kModuleCategoryVehicleAlgorithm;
    //!
    //! @brief 条件类型：模块类别L2
    //! @details 返回：返回模块类别L2的枚举值
    //!
    case 107: return kModuleCategoryL2;
    //!
    //! @brief 条件类型：模块类别L3
    //! @details 返回：返回模块类别L3的枚举值
    //!
    case 108: return kModuleCategoryL3;
    //!
    //! @brief 条件类型：模块类别L4
    //! @details 返回：返回模块类别L4的枚举值
    //!
    case 109: return kModuleCategoryL4;
    //!
    //! @brief 条件类型：模块类别定位
    //! @details 返回：返回模块类别定位的枚举值
    //!
    case 110: return kModuleCategoryLocation;
    //!
    //! @brief 条件类型：模块类别驾驶员
    //! @details 返回：返回模块类别驾驶员的枚举值
    //!
    case 111: return kModuleCategoryDriver;
    //!
    //! @brief 条件类型：模块类别传感器
    //! @details 返回：返回模块类别传感器的枚举值
    //!
    case 112: return kModuleCategorySensor;
    //!
    //! @brief 条件类型：模块类别XIL
    //! @details 返回：返回模块类别XIL的枚举值
    //!
    case 113: return kModuleCategoryXIL;
    //!
    //! @brief 条件类型：模块类别工具
    //! @details 返回：返回模块类别工具的枚举值
    //!
    case 1001: return kModuleCategoryTool;
    //!
    //! @brief 条件类型：未知模块类别
    //! @details 返回：返回未知模块类别的枚举值
    //!
    default: return kModuleCategoryUnknown;
  }
}

inline bool NeedChangeControl(ModuleCategory category) {
  if (tx_sim::impl::kModuleCategoryVehicleAlgorithm == category || tx_sim::impl::kModuleCategoryL2 == category ||
      tx_sim::impl::kModuleCategoryL3 == category || tx_sim::impl::kModuleCategoryL4 == category ||
      tx_sim::impl::kModuleCategoryUnknown == category) {
    return true;
  }
  return false;
}

//!
//! @brief 默认命令超时时间（毫秒）
//! @details 返回：返回默认命令超时时间的枚举值
//!
static const uint32_t kDefaultCmdTimeoutMillis = 30000;
//!
//! @brief 默认步骤超时时间（毫秒）
//! @details 返回：返回默认步骤超时时间的枚举值
//!
static const uint32_t kDefaultStepTimeoutMillis = 3000;
//!
//! @brief 默认心跳超时时间（毫秒）
//! @details 返回：返回默认心跳超时时间的枚举值
//!
static const uint32_t kDefaultHeartbeatTimeoutMillis = 3500;
//!
//! @brief 默认心跳间隔时间（毫秒）
//! @details 返回：返回默认心跳间隔时间的枚举值
//!
static const uint32_t kDefaultHeartbeatIntervalMillis = 1000;
//!
//! @brief 默认控制频率
//! @details 返回：返回默认控制频率的枚举值
//!
static const uint16_t kDefaultControlRate = 100;
//!
//! @brief 默认场景时间限制
//! @details 返回：返回默认场景时间限制的枚举值
//!
static const uint32_t kDefaultScenarioTimeLimit = 0;
//! @brief 模块连接版本
//! @details 返回：返回模块连接版本的枚举值
//!
static const uint32_t kModuleConnVersion = 3000000;
//!
//! @brief 默认协调模式
//! @details 返回：返回默认协调模式的枚举值
//!
static const CoordinationMode kDefaultCoordMode = kCoordinateSync;
//!
//! @brief 默认玩家自动重置
//! @details 返回：返回默认玩家自动重置的枚举值
//!
static const bool kDefaultPlayerAutoReset = true;
//!
//! @brief 默认玩家自动停止
//! @details 返回：返回默认玩家自动停止的枚举值
//!
static const bool kDefaultPlayerAutoStop = true;
//!
//! @brief 默认添加初始位置消息
//! @details 返回：返回默认添加初始位置消息的枚举值
//!
static const bool kDefaultAddingInitialLocationMsg = true;
//!
//! @brief 默认覆盖用户日志
//! @details 返回：返回默认覆盖用户日志的枚举值
//!
static const bool kDefaultOverrideUserLog = true;
//!
//! @brief 日志主题后缀
//! @details 返回：返回日志主题后缀的枚举值
//!
static const std::string kLogTopicPostfix = "._LOGPLAY_";
//!
//! @brief 影子主题后缀
//! @details 返回：返回影子主题后缀的枚举值
//!
static const std::string kShadowTopicPostfix = "._SHADOW_";
//!
//! @brief 用户目录系统
//! @details 返回：返回用户目录系统的枚举值
//!
static const std::string kPathUserDirSys = "sys";
//!
//! @brief 用户目录数据
//! @details 返回：返回用户目录数据的枚举值
//!
static const std::string kPathUserDirData = "data";
//!
//! @brief 服务配置文件名
//! @details 返回：返回服务配置文件名的枚举值
//!
static const std::string kServiceConfigFileName = "local_service.config";
//!
//! @brief 服务数据库名
//! @details 返回：返回服务数据库名的枚举值
//!
static const std::string kServiceDatabaseName = "service.sqlite";
//!
//! @brief 默认评分KPI文件名
//! @details 返回：返回默认评分KPI文件名的枚举值
//!
static const std::string kFileNameDefaultGradingKpi = "default_grading_kpi.json";
//!
//! @brief 服务配置JSON键评分路径
//! @details 返回：返回服务配置JSON键评分路径的枚举值
//!
static const std::string kServiceConfigJsonKeyGradingPath = "gradingKpiDirectory";
//!
//! @brief 评分KPI文件前缀
//! @details 返回：返回评分KPI文件前缀的枚举值
//!
static const std::string kGradingKpiFilePrefix = "grading_";
//!
//! @brief 全局评分KPI文件名
//! @details 返回：返回全局评分KPI文件名的枚举值
//!
static const std::string kGlobalGradingKpiFileName = "grading.json";
//!
//! @brief 默认模块注册端口
//! @details 返回：返回默认模块注册端口的枚举值
//!
static const std::string kDefaultModuleRegistryPort = "21302";
//!
//! @brief 子模块环境变量UUID
//! @details 返回：返回子模块环境变量UUID的枚举值
//!
static const std::string kChildModuleEnvUuid = "TXSIM_MODULE_UUID";
//!
//! @brief 场景启动触发主题
//! @details 返回：返回场景启动触发主题的枚举值
//!
static const std::string kTopicScenarioStartTriggering = ".scenario_start";
//!
//! @brief Log2World触发主题
//! @details 返回：返回Log2World触发主题的枚举值
//!
static const std::string kTopicLog2WorldTriggering = ".log2world_trigger";
static const std::string kTopicHightLightGroup = ".hightlight_group";
//! @brief 播放配置主题
//! @details 返回：返回播放配置主题的枚举值
//!
static const std::string kTopicPlayConfig = ".play_config";
//!
//! @brief 默认性能日志记录启用
//! @details 返回：返回默认性能日志记录启用的枚举值
//!
static const bool kDefaultPerfLoggingEnabled = false;
//!
//! @brief 内部模块转发端点
//! @details 返回：返回内部模块转发端点的枚举值
//!
static const std::string kInnerModuleForwardingEndpoint = "inproc://txsim-coordinator-cmd-forwarding";
//!
//! @brief 内部模块实现命令转发端点
//! @details 返回：返回内部模块实现命令转发端点的枚举值
//!
static const std::string kInnerModuleImplCmdForwardingEndpoint = "inproc://txsim-module-impl-cmd-forwarding";
//!
//! @brief 模块初始化参数键模拟模式
//! @details 返回：返回模块初始化参数键模拟模式的枚举值
//!
static const std::string kModuleInitArgKeySimMode = "_sim_mode";
//!
//! @brief 默认模块优先级别字符串
//! @details 返回：返回默认模块优先级别字符串的枚举值
//!
static const std::string kDefaultModulePriorityLevelString = "50";
//!
//! @brief 全局方案名
//! @details 返回：返回全局方案名的枚举值
//!
static const std::string kGlobalSchemeName = "TADSIM_GLOBAL";
//!
//! @brief Pb信息文件名
//! @details 返回：返回Pb信息文件名的枚举值
//!
static const std::string kPbInfoFileName = "pb_info.json";
static const std::string kUnionFlag = "EgoUnion/";
static const std::string kDefaultEgoGroupName = "Ego_001";

inline std::string GetSMCPVersion(uint32_t ver, uint16_t& major, uint16_t& minor, uint16_t& patch) {
  patch = ver % 1000;
  ver /= 1000;
  minor = ver % 1000;
  ver /= 1000;
  major = ver;
  std::ostringstream ss;
  ss << major << "." << minor << "." << patch;
  return ss.str();
}

inline std::string GetSMCPVersionStr(uint32_t ver) {
  uint16_t major, minor, patch;
  return GetSMCPVersion(ver, major, minor, patch);
}

}  // namespace impl
}  // namespace tx_sim
