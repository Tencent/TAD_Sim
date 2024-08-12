// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <atomic>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <utility>

#include "json/writer.h"

#include "log2world_trigger.pb.h"

#include "config.h"
#include "module_mgr.h"
#include "scenario_parser.h"
#include "shmem_pool.h"
#include "status.h"
#include "status_writer.h"
#include "stepper.h"
#include "utils/constant.h"
#include "utils/defs.h"

namespace tx_sim {
namespace coordinator {

enum StopRecordLevel : int16_t {
  kStopRecordNone = 0,
  kStopRecordFeedbacks = 1,
  kStopRecordAll = 2,
};

enum ServiceType { kServiceUnknow, kServiceLocal, kServiceCloud, kServiceCloudCity };

typedef std::vector<std::pair<std::string, std::string>> InitParamList;
typedef std::unordered_map<EgoTopic, zmq::message_t, EgoTopicHash, EgoTopicEqual> TopicMessageMap;
typedef std::unordered_map<std::string, std::unordered_map<std::string, zmq::message_t>> TopicUnionMessageMap;

struct CommandInfo {
  tx_sim::impl::Command cmd;
  // 命令
  CommandStatusCb status_cb;
  // 命令状态回调函数
  TopicWriterCb writetopic_cb;
  // 主题写入回调函数
  InitParamList extra_init_params;
  // 额外的初始化参数
  tx_sim::impl::ModuleResetRequest reset_params;
  // 模块重置请求参数
  TopicMessageMap input_msgs;
  // 输入消息映射
  size_t max_setup_monitoring_count{0};  // 0 for forever(default).
  // 最大设置监控次数，
  // 0 表示无限次（默认）
  Log2WorldPlayConfig l2w_config;
  // 日志到世界播放配置
  LogsimEventInfo logsim_events;
  // 日志模拟事件信息
  bool pause_on_start{true};
  std::string highlight_group;
  // 是否在开始时暂停

  //!
  //! @brief 构造函数，用于创建 CommandInfo 对象
  //!
  //! @param[in] c 命令
  //!
  CommandInfo(tx_sim::impl::Command c) : cmd(c) {}

  //!
  //! @brief 拷贝构造函数，用于创建 CommandInfo 对象的副本
  //!
  //! @param[in] ci 要拷贝的 CommandInfo 对象
  //!
  CommandInfo(const CommandInfo& ci);

  //!
  //! @brief 函数名：IsCancelCmd
  //! @details 函数功能：检查命令是否为取消命令
  //!
  //! @return 如果命令为 kCmdPause 或 kCmdUnSetup，则返回 true，否则返回 false
  //!
  bool IsCancelCmd() const { return cmd == tx_sim::impl::kCmdPause || cmd == tx_sim::impl::kCmdUnSetup; }
};

class Coordinator final {
 public:
  // 构造函数，用于创建 Coordinator 对象
  // 参数：
  //   root_path: 根路径
  //   config_loader: 配置加载器
  //   module_mgr: 模块管理器
  //   scenario_parser: 场景解析器
  Coordinator(const std::string& root_path, std::shared_ptr<ConfigLoader> config_loader,
              std::shared_ptr<ModuleManager> module_mgr,
              std::shared_ptr<IScenarioParser> scenario_parser);  // for dependency injection.

  // 析构函数
  ~Coordinator();

  // 禁用拷贝和移动构造函数
  TXSIM_DISABLE_COPY_MOVE(Coordinator)

  // 执行命令
  // 参数：
  //   cmd_info: 命令信息
  //   async: 是否异步执行，默认为 false
  // 返回值：命令执行结果
  tx_sim::impl::CmdErrorCode Execute(const CommandInfo& cmd_info, bool async = false);

  // 线程安全的 getter 函数
  bool is_cmd_executing() { return is_cmd_executing_.load(std::memory_order_acquire); }

  // 线程安全的 getter 函数
  const std::string current_scenario() {
    std::lock_guard<std::mutex> lk(current_scenario_mtx_);
    return current_scenario_;
  }

  // 非线程安全的 getter 函数
  uint64_t current_simulation_time() { return sim_time_; }

  // 非线程安全的 getter 函数
  int32_t log2world_switch_time() { return log2world_switch_point_; }

  // 非线程安全的 getter 函数
  int32_t kafka_log2world_switch_time() { return m_kafka_switch_point; }

  // 非线程安全的 getter 函数
  std::string getSwitchType() { return m_switchType; }

  // 判断是否为评分模块
  // 参数：
  //   name: 模块名称
  // 返回值：如果是评分模块，则返回 true，否则返回 false
  bool IsGradingModule(const std::string& name) { return grading_modules_.find(name) != grading_modules_.end(); }

  // 添加本地服务默认设置模块
  // 参数：
  //   vecModuleConfig: 模块配置列表
  //! @brief 函数名：AddLocalServiceDefaultSetupModule
  //! @details 函数功能：添加本地服务默认设置模块
  //!
  //! @param[in] vecModuleConfig 一个包含模块配置信息的ModuleConfig对象向量
  //!
  //! @note 该函数根据输入的模块配置信息向量，添加本地服务默认设置模块。
  void AddLocalServiceDefaultSetupModule(const std::vector<ModuleConfig>& vecModuleConfig);

  //! @brief 函数名：AddCloudServiceDefaultStepModule
  //! @details 函数功能：添加云服务默认步骤模块
  //!
  //! @note 该函数添加云服务默认步骤模块。
  void AddCloudServiceDefaultStepModule();

  //! @brief 函数名：AddCloudCityDefaultStepModule
  //! @details 函数功能：添加云城市默认步骤模块
  //!
  //! @note 该函数添加云城市默认步骤模块。
  void AddCloudCityDefaultStepModule();

 private:
  enum { kLogsimPhraseIdx = 0, kWorldsimPhraseIdx, kMaxSimPhraseIdx };

  struct SimTopic {
    std::string name;          // the original registered name.
    std::string sim_name;      // name used for actual simulation.
    std::string display_name;  // name for displaying needs.
  };

  struct ModulePlayContext;

  struct TopicMeta {
    SimTopic topic;
    void (Coordinator::*message_loading_handler)(const SimTopic&, ModulePlayContext&, tx_sim::impl::ModuleStepRequest&);
    void (Coordinator::*message_unloading_handler)(const SimTopic&, ModulePlayContext&,
                                                   tx_sim::impl::ModuleStepResponse&, TopicMessageList&);

    //!
    //! @brief 构造函数，用于创建 TopicMeta 对象
    //!
    //! @param[in] t 主题名称
    //! @param[in] lh 消息加载处理函数
    //! @param[in] ulh 消息卸载处理函数
    //!
    TopicMeta(const std::string& t, decltype(TopicMeta::message_loading_handler) lh,
              decltype(TopicMeta::message_unloading_handler) ulh)
        : message_loading_handler(lh), message_unloading_handler(ulh) {
      topic.sim_name = topic.name = t;
    }
  };

  struct ModulePlayContext {
    ModuleConfig config;
    std::shared_ptr<ModuleConnection> conn;
    std::vector<TopicMeta> pub_topics[2];  // [logsim_topics, worldsim_topics]
    std::vector<TopicMeta> sub_topics;
    std::unordered_set<std::string> pub_topics_broadcast;
    std::unordered_set<std::string> pub_shmems_broadcast;
    size_t current_sim_phrase = kWorldsimPhraseIdx;
    bool has_outstanding_step_request{false};
    bool last_cmd_timed_out{false};
    int shadow_id = 0;
    struct {
      uint64_t sent_bytes = 0;
      uint64_t recv_bytes = 0;
      uint64_t cpu_user = 0;
      uint64_t cpu_sys = 0;
      uint64_t elapsed = 0;
      size_t step_count = 0;
      uint64_t max_elapsed = 0;
    } perf_stats;
  };
  typedef std::unordered_map<EgoScheme, std::vector<ModulePlayContext>, EgoSchemeHash, EgoSchemeEqual> EgoSchemeMap;

  // 检查命令状态
  // 参数：
  //   cmd: 命令
  // 返回值：命令执行结果
  tx_sim::impl::CmdErrorCode CheckCmdState(tx_sim::impl::Command cmd);

  // 判断是否为日志模拟场景
  // 返回值：如果是日志模拟场景，则返回 true，否则返回 false
  bool IsLogsimScenario() { return logsim_events_.start_timestamp_ms != 0; }

  // 解析log2world参数
  // 参数：
  //   cmd_info: 命令信息
  void ParseLog2WorldParams(const CommandInfo& cmd_info);

  // 取消设置场景
  // 参数：
  //   status: 命令状态
  //   cb: 命令状态回调函数
  //   record_level: 记录级别
  void UnSetupScenario(CommandStatus& status, const CommandStatusCb& cb, StopRecordLevel record_level);

  // 设置场景
  // 参数：
  //   status: 命令状态
  //   cmd_info: 命令信息
  void SetupScenario(CommandStatus& status, const CommandInfo& cmd_info);

  // 应用配置
  // 参数：
  //   extra_init_params: 额外的初始化参数
  //   status: 命令状态
  void ApplyConfiguration(const InitParamList& extra_init_params, CommandStatus& status);

  // 监控模块设置
  // 参数：
  //   status: 命令状态
  //   max_count: 最大设置次数
  //   cb: 命令状态回调函数
  void MonitorModuleSetup(CommandStatus& status, size_t max_count, const CommandStatusCb& cb);

  // 应用模块主题
  // 参数：
  //   module_name: 模块名称
  //   topic_info: 主题信息
  void ApplyModuleTopics(const std::string& module_name, const TopicPubSubInfo& topic_info);

  // 检查是否为日志播放模块
  // 参数：
  //   module_name: 模块名称
  //   pubs: 发布主题列表
  void CheckIsLogplayModule(const std::string& module_name, const std::vector<std::string>& pubs);

  // 按方案拆分模块
  // 参数：
  //   vecSchemems: 方案模块列表
  void SplitMoudleByScheme(EgoSchemeMap& vecSchemems);

  // 配置模拟主题
  // 参数：
  //   status: 命令状态
  // 返回值：如果成功，则返回 true，否则返回 false
  bool ConfiguringSimTopics(CommandStatus& status);
  bool CheckConflictTopic(const std::vector<ModulePlayContext>& vecModules, size_t idx, CommandStatus& status);
  // 应用模块日志到世界主题
  // 参数：
  //   tm: 主题元数据
  //   topic_config: 主题配置
  //   is_logplay_module: 是否为日志播放模块
  void ApplyModuleLog2WorldTopic(TopicMeta& tm,
                                 const decltype(Log2WorldTopicConfig::worldsim_topic_configs)& topic_config,
                                 bool is_logplay_module);

  // 调整影子模块订阅主题
  void AdjustShadowModuleSubTopics();

  // 打印主题配置
  void PrintTopicConfigs();

  // 重置
  // 参数：
  //   status: 命令状态
  //   cmd_info: 命令信息
  //   req: 模块重置请求
  void Reset(CommandStatus& status, const CommandInfo& cmd_info, tx_sim::impl::ModuleResetRequest& req);

  // 步进
  // 参数：
  //   status: 命令状态
  //   cmd_info: 命令信息
  // 返回值：步进结果
  uint32_t Step(CommandStatus& status, const CommandInfo& cmd_info);

  // 停止
  // 参数：
  //   status: 命令状态
  //   cb: 命令状态回调函数
  //   record_level: 记录级别
  void Stop(CommandStatus& status, const CommandStatusCb& cb, StopRecordLevel record_level);

  // 运行
  // 参数：
  //   status: 命令状态
  //   cb: 命令状态回调函数
  void Run(CommandStatus& status, const CommandStatusCb& cb);

  // 清除场景状态
  void ClearScenarioStatus();

  //! @brief 函数名：PollOutstandingRequests
  //! @details 函数功能：轮询未完成的请求
  //!
  //! @param[in,out] status 一个包含命令状态信息的CommandStatus对象
  //! @param[in,out] stop_modules 一个包含停止模块名称的字符串集合
  //! @param[in] cb 一个命令状态回调函数
  //!
  //! @note 该函数轮询未完成的请求，并根据命令状态回调函数更新命令状态和停止模块集合。
  void PollOutstandingRequests(CommandStatus& status, std::unordered_set<std::string>& stop_modules,
                               const CommandStatusCb& cb);

  //! @brief 函数名：LoadModuleResetRequest
  //! @details 函数功能：加载模块重置请求
  //!
  //! @param[in,out] req 一个包含模块重置请求信息的ModuleResetRequest对象
  //! @param[in] params 一个包含模块重置请求参数的ModuleResetRequest对象
  //!
  //! @note 该函数根据输入的模块重置请求参数，加载一个新的模块重置请求。
  tx_sim::impl::CmdErrorCode LoadModuleResetRequest(tx_sim::impl::ModuleResetRequest& req, const CommandInfo& cmd_info);

  //! @brief 函数名：UnloadModuleResetResult
  //! @details 函数功能：卸载模块重置结果
  //!
  //! @param[in,out] resp 一个包含模块重置响应信息的ModuleResetResponse对象
  //! @param[in] name 模块名称
  //! @param[in,out] status 一个包含命令状态信息的CommandStatus对象
  //!
  //! @note 该函数卸载模块重置结果，并根据模块名称和命令状态更新命令状态。
  void UnloadModuleResetResult(tx_sim::impl::ModuleResetResponse& resp, const std::string& name, CommandStatus& status);

  //! @brief 函数名：LoadModuleStepRequest
  //! @details 函数功能：加载模块步进请求
  //!
  //! @param[in,out] req 一个包含模块步进请求信息的ModuleStepRequest对象
  //! @param[in] name 模块名称
  //!
  //! @note 该函数根据输入的模块名称，加载一个新的模块步进请求。
  void LoadModuleStepRequest(tx_sim::impl::ModuleStepRequest& req, const std::string& name);

  //! @brief 函数名：UnloadModuleStepResult
  //! @details 函数功能：卸载模块步进结果
  //!
  //! @param[in,out] resp 一个包含模块步进响应信息的ModuleStepResponse对象
  //! @param[in] name 模块名称
  //! @param[in,out] status 一个包含命令状态信息的CommandStatus对象
  //!
  //! @note 该函数卸载模块步进结果，并根据模块名称和命令状态更新命令状态。
  void UnloadModuleStepResult(tx_sim::impl::ModuleStepResponse& resp, const std::string& name, CommandStatus& status);

  //! @brief 函数名：UnloadModuleStopResult
  //! @details 函数功能：卸载模块停止结果
  //!
  //! @param[in,out] resp 一个包含模块停止响应信息的ModuleStopResponse对象
  //! @param[in] name 模块名称
  //! @param[in,out] status 一个包含命令状态信息的CommandStatus对象
  //! @param[in] record_level 记录级别
  //!
  //! @note 该函数卸载模块停止结果，并根据模块名称、命令状态和记录级别更新命令状态。
  void UnloadModuleStopResult(tx_sim::impl::ModuleStopResponse& resp, const std::string& name, CommandStatus& status,
                              StopRecordLevel record_level);
  void AggregateModuleStepMessages(tx_sim::impl::ModuleStepResponse& resp, const std::string& name,
                                   CommandStatus& status);
  void FilterCommandStatus(CommandStatus& status, const std::string& filter_group);
  //! @brief 函数名：HandleMessageLoading
  //! @details 函数功能：处理消息加载
  //!
  //! @param[in] topic 模拟主题
  //! @param[in,out] module 模块播放上下文
  //! @param[in,out] req 一个包含模块步进请求信息的ModuleStepRequest对象
  //!
  //! @note 该函数根据模拟主题和模块播放上下文，处理消息加载。
  void HandleMessageLoading(const SimTopic& topic, ModulePlayContext& module, tx_sim::impl::ModuleStepRequest& req);

  //! @brief 函数名：HandleMessageUnloading
  //! @details 函数功能：处理消息卸载
  //!
  //! @param[in] topic 模拟主题
  //! @param[in,out] module 模块播放上下文
  //! @param[in,out] resp 一个包含模块步进响应信息的ModuleStepResponse对象
  //! @param[in,out] status_msgs 一个包含状态消息的TopicMessageList对象
  //!
  //! @note 该函数根据模拟主题、模块播放上下文、模块步进响应和状态消息列表，处理消息卸载。
  void HandleMessageUnloading(const SimTopic& topic, ModulePlayContext& module, tx_sim::impl::ModuleStepResponse& resp,
                              TopicMessageList& status_msgs);

  //! @brief 函数名：HandleSubShmemLoading
  //! @details 函数功能：处理子共享内存加载
  //!
  //! @param[in] name 模拟主题名称
  //! @param[in,out] module 模块播放上下文
  //! @param[in,out] req 一个包含模块步进请求信息的ModuleStepRequest对象
  //!
  //! @note 该函数根据模拟主题名称和模块播放上下文，处理子共享内存加载。
  void HandleSubShmemLoading(const SimTopic& name, ModulePlayContext& module, tx_sim::impl::ModuleStepRequest& req);

  //! @brief 函数名：HandleSubShmemUnloading
  //! @details 函数功能：处理子共享内存卸载
  //!
  //! @param[in] name 模拟主题名称
  //! @param[in,out] module 模块播放上下文
  //! @param[in,out] resp 一个包含模块步进响应信息的ModuleStepResponse对象
  //! @param[in,out] status_msgs 一个包含状态消息的TopicMessageList对象
  //!
  //! @note 该函数根据模拟主题名称、模块播放上下文、模块步进响应和状态消息列表，处理子共享内存卸载。
  void HandleSubShmemUnloading(const SimTopic& name, ModulePlayContext& module, tx_sim::impl::ModuleStepResponse& resp,
                               TopicMessageList& status_msgs);

  //! @brief 函数名：HandlePubShmemLoading
  //! @details 函数功能：处理发布共享内存加载
  //!
  //! @param[in] name 模拟主题名称
  //! @param[in,out] module 模块播放上下文
  //! @param[in,out] req 一个包含模块步进请求信息的ModuleStepRequest对象
  //!
  //! @note 该函数根据模拟主题名称和模块播放上下文，处理发布共享内存加载。
  void HandlePubShmemLoading(const SimTopic& name, ModulePlayContext& module, tx_sim::impl::ModuleStepRequest& req);

  //! @brief 函数名：HandlePubShmemUnloading
  //! @details 函数功能：处理发布共享内存卸载
  //!
  //! @param[in] name 模拟主题名称
  //! @param[in,out] module 模块播放上下文
  //! @param[in,out] resp 一个包含模块步进响应信息的ModuleStepResponse对象
  //! @param[in,out] status_msgs 一个包含状态消息的TopicMessageList对象
  //!
  //! @note 该函数根据模拟主题名称、模块播放上下文、模块步进响应和状态消息列表，处理发布共享内存卸载。
  void HandlePubShmemUnloading(const SimTopic& name, ModulePlayContext& module, tx_sim::impl::ModuleStepResponse& resp,
                               TopicMessageList& status_msgs);

  //! @brief 函数名：ReceiveModuleReply
  //! @details 函数功能：接收模块回复
  //!
  //! @param[in] name 模块名称
  //! @param[in,out] m 模块播放上下文
  //! @param[in,out] reply 一个包含模块响应信息的ModuleResponse对象
  //! @param[in] cb 一个命令状态回调函数
  //!
  //! @note 该函数根据模块名称、模块播放上下文、模块响应和命令状态回调函数，接收模块回复。
  void ReceiveModuleReply(const std::string& name, ModulePlayContext& m, tx_sim::impl::ModuleResponse& reply,
                          const CommandStatusCb& cb);

  //! @brief 函数名：CheckScenarioStarted
  //! @details 函数功能：检查场景是否已启动
  //!
  //! @param[in] last_module_triggered 最后一个触发的模块
  //!
  //! @note 该函数根据最后一个触发的模块，检查场景是否已启动。
  void CheckScenarioStarted(bool last_module_triggered);

  //! @brief 函数名：CheckRunStatus
  //! @details 函数功能：检查运行状态
  //!
  //! @param[in,out] sttus 一个包含命令状态信息的CommandStatus对象
  //! @param[in] cb 一个命令状态回调函数
  //! @param[in] start_time 开始时间
  //!
  //! @note 该函数根据命令状态、命令状态回调函数和开始时间，检查运行状态。
  bool CheckRunStatus(CommandStatus& sttus, const CommandStatusCb& cb, int64_t start_time);

  //! @brief 函数名：ControlStepTime
  //! @details 函数功能：控制步进时间
  //!
  //! @param[in] time_to_next_step 下一步的时间
  //! @param[in] step_start_time 步骤开始时间
  //! @param[in] sim_start_time 模拟开始时间
  //! @param[in] tmpSimTime 临时模拟时间
  //!
  //! @note 该函数根据下一步的时间、步骤开始时间、模拟开始时间和临时模拟时间，控制步进时间。
  void ControlStepTime(uint32_t time_to_next_step, int64_t step_start_time, int64_t sim_start_time, int64_t tmpSimTime);

  //! @brief 函数名：CallbackRoutine
  //! @details 函数功能：回调例程
  //!
  //! @param[in] cb 一个命令状态回调函数
  //! @param[in] st 一个包含命令状态信息的CommandStatus对象
  //!
  //! @note 该函数根据命令状态回调函数和命令状态，执行回调例程。
  void CallbackRoutine(const CommandStatusCb& cb, const CommandStatus& st);

  //! @brief 函数名：EnablePerfLogging
  //! @details 函数功能：启用性能日志记录
  //!
  //! @note 该函数启用性能日志记录。
  void EnablePerfLogging();

  //! @brief 函数名：LoggingPerfStats
  //! @details 函数功能：记录性能统计信息
  //!
  //! @note 该函数记录性能统计信息。
  void LoggingPerfStats();

  //! @brief 函数名：ProcessGradingFeedback
  //! @details 函数功能：处理评分反馈
  //!
  //! @param[in] feedback 反馈信息
  //!
  //! @note 该函数根据反馈信息，处理评分反馈。
  void ProcessGradingFeedback(const std::string& feedback);

  //! @brief 函数名：OutputModuleStepStats
  //! @details 函数功能：输出模块步进统计信息
  //!
  //! @param[in] status 一个包含命令状态信息的CommandStatus对象
  //!
  //! @note 该函数根据命令状态，输出模块步进统计信息。
  const std::string OutputModuleStepStats(const CommandStatus& status);

  //! @brief 函数名：AddSystemTime
  //! @details 函数功能：添加系统时间
  //!
  //! @param[in] req 一个包含模块步进请求信息的ModuleStepRequest对象
  //!
  //! @note 该函数根据模块步进请求信息，添加系统时间。
  void AddSystemTime(tx_sim::impl::ModuleStepRequest& req);

  //! @brief 函数名：AppendLaunchInCurrentConfig
  //! @details 函数功能：将启动配置追加到当前配置
  //!
  //! @param[in] req 一个包含模块重置请求信息的ModuleResetRequest对象
  //!
  //! @note 该函数根据模块重置请求信息，将启动配置追加到当前配置
  void AppendLaunchInCurrentConfig(const tx_sim::impl::ModuleResetRequest* req);

  //! @brief 函数名：GetCurrentControllerName
  //! @details 函数功能：获取当前控制器名称
  //!
  //! @param[in] sTopicName 一个字符串，表示当前控制器的名称
  //! @param[in] data 一个指向常量的指针，表示输入数据的起始地址
  //! @param[in] size 一个整数，表示输入数据的大小
  //!
  //! @note 该函数根据输入的控制器名称和数据，获取当前控制器的名称，用于后续的控制器操作和状态查询。
  void GetCurrentControllerName(std::string sTopicName, const void* data, int size);

  //! @brief 成员变量：m_gradingPerfStats
  //! @details 成员变量用途：存储评分性能统计信息
  //!
  //! @note 该成员变量是一个无序映射，键为字符串，值为tx_sim::impl::GradingPerfStats类型，用于存储评分性能统计信息。
  std::unordered_map<std::string, tx_sim::impl::GradingPerfStats> m_gradingPerfStats;
  //! @brief 静态常量：kKeepAliveStatus
  //! @details 静态常量用途：表示保持活动状态的命令状态
  //!
  //! @note 该静态常量表示保持活动状态的命令状态，用于后续的命令执行和状态查询。
  static const CommandStatus kKeepAliveStatus;

  //! @brief 静态常量：kKeepAliveInterval, kMaxModuleForcingTimeout
  //! @details 静态常量用途：表示保持活动间隔和最大模块强制超时时间
  //!
  //! @note 该静态常量表示保持活动间隔和最大模块强制超时时间，用于后续的命令执行和状态查询。
  static const uint32_t kKeepAliveInterval = 1000, kMaxModuleForcingTimeout = 5000;  // ms

  //! @brief 静态常量：kPerfLogPath
  //! @details 静态常量用途：表示性能日志路径
  //!
  //! @note 该静态常量表示性能日志路径，用于后续的日志记录和查询。
  static const std::string kPerfLogPath;

  //! @brief 成员变量：root_path_
  //! @details 成员变量用途：表示根路径
  //!
  //! @note 该成员变量表示根路径，用于后续的文件操作和路径查询。
  const std::string root_path_;

  //! @brief 成员变量：cmd_mtx_
  //! @details 成员变量用途：表示命令互斥锁
  //!
  //! @note 该成员变量表示命令互斥锁，用于后续的命令执行和状态查询。
  std::mutex cmd_mtx_;
  std::string highlight_group_;
  //! @brief 成员变量：is_cmd_executing_, un_setup_, pause_
  //! @details 成员变量用途：表示命令执行状态、模块设置状态和暂停状态
  //!
  //! @note 该成员变量表示命令执行状态、模块设置状态和暂停状态，用于后续的命令执行和状态查询。
  std::atomic_bool is_cmd_executing_{false}, un_setup_{false}, pause_{false};

  //! @brief 成员变量：scenario_set_, module_reset_
  //! @details 成员变量用途：表示场景设置状态和模块重置状态
  //!
  //! @note 该成员变量表示场景设置状态和模块重置状态，用于后续的命令执行和状态查询。
  bool scenario_set_{false}, module_reset_{false};

  //! @brief 成员变量：logsim_events_
  //! @details 成员变量用途：表示日志模拟事件信息
  //!
  //! @note 该成员变量表示日志模拟事件信息，用于后续的日志记录和查询。
  LogsimEventInfo logsim_events_;

  //! @brief 成员变量：logplay_module_
  //! @details 成员变量用途：表示日志播放模块
  //!
  //! @note 该成员变量表示日志播放模块，用于后续的日志播放和查询。
  std::string logplay_module_;

  //! @brief 成员变量：play_cmd_thread_
  //! @details 成员变量用途：表示播放命令线程
  //!
  //! @note 该成员变量表示播放命令线程，用于后续的命令执行和状态查询。
  std::unique_ptr<std::thread> play_cmd_thread_;

  //! @brief 成员变量：cancel_cmd_thread_
  //! @details 成员变量用途：表示取消命令线程
  //!
  //! @note 该成员变量表示取消命令线程，用于后续的命令执行和状态查询。
  std::unique_ptr<std::thread> cancel_cmd_thread_;

  //! @brief 成员变量：sim_time_, max_sim_time_
  //! @details 成员变量用途：表示模拟时间和最大模拟时间
  //!
  //! @note 该成员变量表示模拟时间和最大模拟时间，用于后续的模拟时间查询和更新。
  uint64_t sim_time_ = 0, max_sim_time_ = 0;  // in milli-seconds

  //! @brief 成员变量：scenario_started_
  //! @details 成员变量用途：表示场景是否已启动
  //!
  //! @note 该成员变量表示场景是否已启动，用于后续的场景启动和状态查询。
  bool scenario_started_{true};

  // in milli-seconds, the time point after when the scenario switches from logsim to worldsim mode. clearly 0 means
  // there's no logsim, and std::numeric_limits<int32_t>::max() means always using logsim.
  //! @brief 成员变量：log2world_switch_point_
  //! @details 成员变量用途：表示日志到世界切换点
  //!
  //! @note 该成员变量表示日志到世界切换点，用于后续的日志切换和查询。
  int32_t log2world_switch_point_ = 0;

  //! @brief 成员变量：m_kafka_switch_point
  //! @details 成员变量用途：表示Kafka切换点
  //!
  //! @note 该成员变量表示Kafka切换点，用于后续的Kafka切换和查询。
  int32_t m_kafka_switch_point = 0;

  //! @brief 成员变量：m_switchType
  //! @details 成员变量用途：表示切换类型
  //!
  //! @note 该成员变量表示切换类型，用于后续的切换和查询。
  std::string m_switchType;

  //! @brief 成员变量：l2w_trigger_msg_
  //! @details 成员变量用途：表示日志到世界触发消息
  //!
  //! @note 该成员变量表示日志到世界触发消息，用于后续的日志切换和查询。
  zmq::message_t l2w_trigger_msg_;
  zmq::message_t m_highlight_group_msg;
  //! @brief 成员变量：location_topics_
  //! @details 成员变量用途：表示所有映射的位置主题
  //!
  //! @note 该成员变量表示所有映射的位置主题，用于后续的位置主题查询和更新。
  std::set<std::string> location_topics_;  // all mapped location topic names(shadow mode etc.)
  //! @brief 成员变量：messages_
  //! @details 成员变量用途：表示主题消息映射
  //!
  //! @note 该成员变量表示主题消息映射，用于后续的主题消息查询和更新。
  TopicMessageMap messages_;
  TopicUnionMessageMap union_messages_;
  //! @brief 成员变量：current_config_
  //! @details 成员变量用途：表示当前协调器配置
  //!
  //! @note 该成员变量表示当前协调器配置，用于后续的配置查询和更新。
  CoordinatorConfig current_config_;

  //! @brief 成员变量：modules_
  //! @details 成员变量用途：表示模块播放上下文映射
  //!
  //! @note 该成员变量表示模块播放上下文映射，用于后续的模块播放上下文查询和更新。
  std::unordered_map<std::string, ModulePlayContext> modules_;

  //! @brief 成员变量：m_multimap_topicname_moduleconnection
  //! @details 成员变量用途：表示主题名到模块连接的多重映射
  //!
  //! @note 该成员变量表示主题名到模块连接的多重映射，用于后续的主题名和模块连接查询和更新。
  std::unordered_multimap<std::string, std::shared_ptr<ModuleConnection>> m_multimap_topicname_moduleconnection;

  //! @brief 成员变量：stepper_
  //! @details 成员变量用途：表示模拟步进器
  //!
  //! @note 该成员变量表示模拟步进器，用于后续的模拟步进和查询。
  std::unique_ptr<SimStepper> stepper_;

  //! @brief 成员变量：shmem_pool_
  //! @details 成员变量用途：表示共享内存池
  //!
  //! @note 该成员变量表示共享内存池，用于后续的共享内存查询和更新。
  std::unique_ptr<ShmemPool> shmem_pool_;

  //! @brief 成员变量：config_loader_
  //! @details 成员变量用途：表示配置加载器
  //!
  //! @note 该成员变量表示配置加载器，用于后续的配置加载和查询。
  std::shared_ptr<ConfigLoader> config_loader_;

  //! @brief 成员变量：module_mgr_
  //! @details 成员变量用途：表示模块管理器
  //!
  //! @note 该成员变量表示模块管理器，用于后续的模块管理和查询。
  std::shared_ptr<ModuleManager> module_mgr_;

  //! @brief 成员变量：scenario_parser_
  //! @details 成员变量用途：表示场景解析器
  //!
  //! @note 该成员变量表示场景解析器，用于后续的场景解析和查询。
  std::shared_ptr<IScenarioParser> scenario_parser_;

  //! @brief 成员变量：perf_log_stream_
  //! @details 成员变量用途：表示性能日志流
  //!
  //! @note 该成员变量表示性能日志流，用于后续的性能日志记录和查询。
  std::ofstream perf_log_stream_;

  //! @brief 成员变量：perf_writer_
  //! @details 成员变量用途：表示性能日志写入器
  //!
  //! @note 该成员变量表示性能日志写入器，用于后续的性能日志记录和查询。
  std::unique_ptr<Json::StreamWriter> perf_writer_;

  //! @brief 成员变量：grading_feedback_process_thread_
  //! @details 成员变量用途：表示评分反馈处理线程
  //!
  //! @note 该成员变量表示评分反馈处理线程，用于后续的评分反馈处理和查询。
  std::unique_ptr<std::thread> grading_feedback_process_thread_;

  //! @brief 成员变量：grading_modules_
  //! @details 成员变量用途：表示评分模块集合
  //!
  //! @note 该成员变量表示评分模块集合，用于后续的评分模块查询和更新。
  std::set<std::string> grading_modules_;

  //! @brief 成员变量：current_scenario_
  //! @details 成员变量用途：表示当前场景
  //!
  //! @note 该成员变量表示当前场景，用于后续的场景查询和更新。
  std::string current_scenario_;

  //! @brief 成员变量：current_scenario_mtx_
  //! @details 成员变量用途：表示当前场景互斥锁
  //!
  //! @note 该成员变量表示当前场景互斥锁，用于后续的场景查询和更新。
  std::mutex current_scenario_mtx_;

  //! @brief 成员变量：last_l2w_play_config_
  //! @details 成员变量用途：表示最后一次日志到世界播放配置
  //!
  //! @note 该成员变量表示最后一次日志到世界播放配置，用于后续的日志到世界播放配置查询和更新。
  std::unique_ptr<Log2WorldPlayConfig> last_l2w_play_config_;

  //! @brief 成员变量：m_vecAutoLaunchModule
  //! @details 成员变量用途：表示自动启动的模块配置列表
  //!
  //! @note 该成员变量表示自动启动的模块配置列表，用于后续的模块启动和查询。
  std::vector<ModuleConfig> m_vecAutoLaunchModule;

  //! @brief 成员变量：m_subTopicMap
  //! @details 成员变量用途：表示订阅主题映射
  //!
  //! @note 该成员变量表示订阅主题映射，用于后续的订阅主题查询和更新。
  std::unordered_map<std::string, std::set<std::string>> m_subTopicMap;
  std::unordered_set<std::string> m_unionTopic;
  //! @brief 成员变量：m_serviceType
  //! @details 成员变量用途：表示服务类型
  //!
  //! @note 该成员变量表示服务类型，用于后续的服务类型查询和更新。
  ServiceType m_serviceType;
  std::unordered_map<std::string, std::string> m_sCurrentSchemeMap;
  std::vector<std::string> m_vecGroupName;
  std::atomic_bool m_bTerminateSetup{false};
};

}  // namespace coordinator
}  // namespace tx_sim
