// Copyright 2024 Tencent Inc. All rights reserved.
//
// \brief "云端场景型仿真调度引擎服务"
//
#include <stdlib.h>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "glog/logging.h"
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"
#include "openssl/md5.h"

#include "cloud_topic_writer.h"
#include "coordinator/config_mgr.h"
#include "coordinator/coordinator.h"
#include "sim_cloud_service.grpc.pb.h"
#include "utils/json_helper.h"
#include "utils/proc.h"
#include "utils/proc_service.h"
#include "utils/time.h"


namespace prog_ops = boost::program_options;
using namespace tx_sim::coordinator;
using namespace tx_sim::impl;
using namespace tx_sim::utils;


/******************************************************************************/
/*************************** helper functions *********************************/
/******************************************************************************/

/**
 * @brief translate command error code to sim_cloud::ErrorCode
 * @param ec input command error code
 * @return sim_cloud::ErrorCode
 */
static sim_cloud::ErrorCode TranslateCommandErrorCode(CmdErrorCode ec) {
  switch (ec) {
    case kCmdScenarioStopped: return sim_cloud::ScenarioFinished;
    case kCmdScenarioTimeout: return sim_cloud::ScenarioTimeout;
    case kCmdFailed: return sim_cloud::ModuleError;
    case kCmdSystemError: return sim_cloud::InternalError;
    default: {
      LOG(ERROR) << "un-expected cmd error code: " << Enum2String(ec);
      return sim_cloud::InternalError;
    }
  }
}


/**
 * @brief translate module error code to sim_cloud::ModuleStatusCode
 * @param ec input module error code
 * @return sim_cloud::ModuleStatusCode
 */
static sim_cloud::ModuleStatusCode TranslateModuleErrorCode(ModuleResponseType ec) {
  switch (ec) {
    case kModuleOK: return sim_cloud::ModuleSucceed;
    case kModuleRequireStop: return sim_cloud::ModuleRequiredStop;
    case kModuleTimeout: return sim_cloud::ModuleTimeout;
    case kModuleError: return sim_cloud::ModuleException;
    default: return sim_cloud::ModuleException;  // internal error. report from cmd error code.
  }
}


/**
 * @brief add module status to result
 * @param[in] ms input module status
 * @param[out] result output sim_cloud::RunResult
 */
static void AddModuleStatusInfo(const ModuleCmdStatus& ms, sim_cloud::RunResult* result) {
  auto* s = result->add_module_status();
  s->set_name(ms.name);
  s->set_sc(TranslateModuleErrorCode(ms.ec));
  if (ms.msg == "exit code: 201") s->set_msg("module config error: worldsim can not run traffic logsim module.");
  else
    s->set_msg(ms.msg);
}


/**
 * @brief record the module status to result by calling AddModuleStatusInfo
 * @param[in] ms input module status
 * @param[out] result output sim_cloud::RunResult
 */
static void RecordModuleStatus(const std::vector<ModuleCmdStatus>& module_status, sim_cloud::RunResult* result) {
  for (const ModuleCmdStatus& ms : module_status) {
    LOG(INFO) << "module " << ms.name << " status: (ec)" << ms.ec << " (msg)\"" << ms.msg << "\" (feedback)\""
              << ms.feedback << "\"";
    AddModuleStatusInfo(ms, result);
  }
}


/**
 * @brief get scenario labels by parsing the scenario config file
 * @param configPath input config file path
 * @return scenario labels
 */
std::string GetScenarioLabels(std::string configPath) {
  boost::property_tree::ptree pt;
  boost::property_tree::json_parser::read_json(configPath, pt);
  std::string sLabels = pt.get<std::string>("scenarioLabels", "");
  return sLabels;
}

/**
 * @brief get scenario name by parsing the scenario file. If can not find ".sim", return input filename.
 * @param configPath input config file
 * @return filename
 */
std::string GetScenarioFileName(const std::string& filename) {
  size_t index = filename.find(".sim");
  if (index != std::string::npos) { return filename.substr(0, index); }
  return filename;
}


/******************************************************************************/
/*************************** Sim Cloud Service ********************************/
/******************************************************************************/

/**
 * @brief "云端场景型仿真调度引擎服务端", inherits from grpc sim_cloud::SimCloudService::Service,
 * calling RunScenario() and CancelCurrentScenario().
 */
class SimCloudService final : public sim_cloud::SimCloudService::Service {
 public:
  //! @brief 构造函数：SimCloudService
  //! @details 构造函数用途：初始化SimCloudService对象
  //!
  //! @param[in] root_path 根路径
  //! @param[in] override_path 覆盖路径
  SimCloudService(const std::string& root_path, const std::string& override_path);

  //! @brief 析构函数：SimCloudService
  //! @details 析构函数用途：销毁SimCloudService对象
  ~SimCloudService();

  //! @brief 成员函数：RunScenario
  //! @details 成员函数用途：运行场景
  //!
  //! @param[in] context gRPC服务器上下文
  //! @param[in] args 运行参数
  //! @param[out] result 运行结果
  //!
  //! @return gRPC状态，表示操作是否成功
  grpc::Status RunScenario(grpc::ServerContext* context, const sim_cloud::RunArgs* args,
                           sim_cloud::RunResult* result) override;

  //! @brief 成员函数：CancelCurrentScenario
  //! @details 成员函数用途：取消当前场景
  //!
  //! @param[in] context gRPC服务器上下文
  //! @param[in] args 取消参数
  //! @param[out] result 取消结果
  //!
  //! @return gRPC状态，表示操作是否成功
  grpc::Status CancelCurrentScenario(grpc::ServerContext* context, const sim_cloud::CancelArgs* args,
                                     sim_cloud::CancelResult* result) override;

 private:
  //! @brief 成员函数：InitSim
  //! @details 成员函数用途：初始化模拟
  void InitSim();

  //! @brief 成员函数：SetupModules
  //! @details 成员函数用途：设置模块
  //!
  //! @param[out] result 运行结果
  //! @param[in] params 运行参数
  //!
  //! @return 布尔值，表示操作是否成功
  bool SetupModules(sim_cloud::RunResult* result, const sim_cloud::RunArgs& params);

  //! @brief 成员函数：WriteGradingResult
  //! @details 成员函数用途：写入评分结果
  //!
  //! @param[in] status 命令状态
  //! @param[in] frame_id 帧ID
  //! @param[in] sim_time 模拟时间
  void WriteGradingResult(const CommandStatus& status, uint64_t frame_id, uint64_t sim_time);

  //! @brief 成员函数：FindScenarioProcessTool
  //! @details 成员函数用途：查找场景处理工具
  //!
  //! @param[in] tool_name 工具名称
  //! @param[out] tool_path 工具路径
  void FindScenarioProcessTool(const std::string& tool_name, std::string& tool_path);

  //! @brief 成员函数：ExtraProcessScenario
  //! @details 成员函数用途：额外处理场景
  //!
  //! @param[in] tool_path 工具路径
  //! @param[in] scenario_path 场景路径
  //! @param[in] job_id 任务ID
  //! @param[in] scene_id 场景ID
  void ExtraProcessScenario(const std::string& tool_path, const std::string& scenario_path, int32_t job_id,
                            int32_t scene_id);

  //! @brief 成员函数：ReportLogsimEvents
  //! @details 成员函数用途：报告Logsim事件
  //!
  //! @param[in] report_addr 报告地址
  //! @param[in] events Logsim事件信息
  //! @param[in] job_id 任务ID
  //! @param[in] scene_id 场景ID
  void ReportLogsimEvents(const std::string& report_addr, const LogsimEventInfo& events, int32_t job_id,
                          int32_t scene_id);

  //! @brief 成员函数：ParseLog2WorldArgs
  //! @details 成员函数用途：解析Log2World参数
  //!
  //! @param[in] args JSON参数
  //! @param[out] config Log2World播放配置
  //!
  //! @return 字符串，表示解析后的参数
  std::string ParseLog2WorldArgs(const Json::Value& args, Log2WorldPlayConfig& config);

  //! @brief 成员函数：GetServiceConfigFilePath
  //! @details 成员函数用途：获取服务配置文件路径
  //!
  //! @param[in] app_root 应用根路径
  //!
  //! @return 字符串，表示服务配置文件路径
  const std::string GetServiceConfigFilePath(const std::string& app_root);

  //! @brief 成员变量：app_path_
  //! @details 成员变量用途：存储应用路径
  std::string app_path_;

  //! @brief 成员变量：override_path_
  //! @details 成员变量用途：存储覆盖路径
  std::string override_path_;

  //! @brief 成员变量：pre_process_program_
  //! @details 成员变量用途：存储预处理程序
  std::string pre_process_program_;

  //! @brief 成员变量：post_process_program_
  //! @details 成员变量用途：存储后处理程序
  std::string post_process_program_;

  //! @brief 成员变量：coordinator_
  //! @details 成员变量用途：存储协调器的指针
  std::unique_ptr<Coordinator> coordinator_;

  //! @brief 成员变量：topic_writer_
  //! @details 成员变量用途：存储SimCloud主题写入器的指针
  std::unique_ptr<SimCloudTopicWriter> topic_writer_;

  //! @brief 成员变量：snpr_
  //! @details 成员变量用途：存储场景解析器的指针
  std::shared_ptr<IScenarioParser> snpr_;

  //! @brief 成员变量：l2w_cfg_
  //! @details 成员变量用途：存储Log2World播放配置的指针
  std::unique_ptr<Log2WorldPlayConfig> l2w_cfg_;
};


/**
 * @brief constructor
 * @param[in] root_path The directory of configuration files and database
 * @param[in] override_path an optional root directory where contains files that override some part of the cloud service
 * configurations
 */
SimCloudService::SimCloudService(const std::string& root_path, const std::string& override_path)
    : app_path_(root_path), override_path_(override_path) {
  FindScenarioProcessTool("scenario_pre_process.sh", pre_process_program_);
  FindScenarioProcessTool("scenario_post_process.sh", post_process_program_);
  InitSim();
}


/**
 * @brief destructor
 */
SimCloudService::~SimCloudService() {}


/**
 * @brief the grpc call to run the simulation service
 * @param[in] context grpc context
 * @param[in] args the input sim_cloud::RunArgs arguments
 * @param[out] result the output sim_cloud::RunResult result
 * @return grpc status
 */
grpc::Status SimCloudService::RunScenario(grpc::ServerContext* context, const sim_cloud::RunArgs* args,
                                          sim_cloud::RunResult* result) {
  try {
    uint64_t frame_id = 0;
    int32_t jid = args->job_id(), sid = args->scene_id();
    ExtraProcessScenario(pre_process_program_, args->scenario_path(), jid, sid);

    // reset topic_writer_ if override_path_ is not empty
    if (!override_path_.empty()) topic_writer_.reset(new SimCloudTopicWriter(GetServiceConfigFilePath(override_path_)));

    LOG(INFO) << "job " << jid << ": running scenario [" << sid << "] " << args->scenario_path() << " ...";
    if (!SetupModules(result, *args)) return grpc::Status::OK;

    // use run command to run the simulation
    CommandInfo cmd_info(kCmdRun);
    uint64_t sim_time = 0;
    cmd_info.status_cb = [result, &frame_id, this, &sim_time](const CommandStatus& status) {
      // topic_writer_->Write(status.step_message, frame_id, status.step_message.timestamp);
      bool bret = topic_writer_->WritePb(status.step_message, frame_id, status.step_message.timestamp);
      if (bret) ++frame_id;
      if (status.ec != kCmdAccepted) {  // scenario stopped or error.
        frame_id++;
        WriteGradingResult(status, frame_id, sim_time);
        RecordModuleStatus(status.module_status, result);
      }
      if (status.step_message.timestamp > 0) sim_time = status.step_message.timestamp;
    };
    cmd_info.l2w_config = *l2w_cfg_;
    CmdErrorCode ec = coordinator_->Execute(cmd_info);
    LOG(INFO) << "job " << jid << ": running scenario [" << sid << "] finished: " << Enum2String(ec);
    result->set_ec(TranslateCommandErrorCode(ec));
    if (ec == kCmdScenarioTimeout) { topic_writer_->Write("scenario time out", frame_id, sim_time); }
    ExtraProcessScenario(post_process_program_, args->scenario_path(), jid, sid);
  } catch (const std::exception& e) {
    LOG(ERROR) << "run scenario error: " << e.what();
    auto* s = result->add_module_status();
    s->set_name("cloud_service");
    s->set_sc(sim_cloud::ModuleException);
    s->set_msg(e.what());
    result->set_ec(sim_cloud::InternalError);
  }
  return grpc::Status::OK;
}


/**
 * @brief the grpc call to stop the simulation service running
 * @param[in] context grpc context
 * @param[in] args the input sim_cloud::CancelArgs arguments
 * @param[out] result the output sim_cloud::CancelResult result
 * @return grpc status
 */
grpc::Status SimCloudService::CancelCurrentScenario(grpc::ServerContext* context, const sim_cloud::CancelArgs* args,
                                                    sim_cloud::CancelResult* result) {
  CommandInfo cmd_pause(kCmdPause), cmd_unsetup(kCmdUnSetup);
  LOG(INFO) << "pause current scenario ...";
  // 1. pause the simulation
  CmdErrorCode ec = coordinator_->Execute(cmd_pause);
  LOG(INFO) << "scenario paused: " << Enum2String(ec);
  // 2. un-setup the simulation
  ec = coordinator_->Execute(cmd_unsetup);
  LOG(INFO) << "un-setup finished: " << Enum2String(ec);
  return grpc::Status::OK;
}


/**
 * @brief initialize the simulation service
 */
void SimCloudService::InitSim() {
  boost::filesystem::path cfg_db = app_path_;
  cfg_db /= kServiceDatabaseName;
  const std::string cfg_file_path = GetServiceConfigFilePath(app_path_);

  auto cfg_mgr = std::make_shared<ConfigManager>(cfg_db.lexically_normal().string());
  cfg_mgr->InitConfigData(cfg_file_path, app_path_);
  auto ctx = std::make_shared<zmq::context_t>();
  snpr_ = std::make_shared<ScenarioXmlParser>();
  coordinator_.reset(new Coordinator(
      app_path_, std::dynamic_pointer_cast<ConfigLoader>(cfg_mgr),
      std::make_shared<ModuleManagerImpl>(std::make_shared<ModuleRegistryImpl>(ctx),
                                          std::make_shared<ZmqConnectionFactory>(ctx),
                                          std::make_shared<ModuleProcessServiceImpl>(ctx), cfg_file_path),
      snpr_));
  coordinator_->AddCloudServiceDefaultStepModule();
  // if there is an override path for the cloud configuration(currently only for the topic writer configs), initiate it
  // before run command in every RunScenario RPC call.
  if (override_path_.empty()) topic_writer_.reset(new SimCloudTopicWriter(cfg_file_path));
}

/**
 * @brief setup the simulation modules
 * @param[in] params the input sim_cloud::RunArgs arguments
 * @param[out] result the output sim_cloud::RunResult result
 * @return true if setup kCmdSucceed, false otherwise
 */
bool SimCloudService::SetupModules(sim_cloud::RunResult* result, const sim_cloud::RunArgs& params) {
  // use setup conmmand to setup the simulation
  CommandInfo cmd_info(kCmdSetup);
  cmd_info.status_cb = [result](const CommandStatus& status) {
    for (const auto& ist : status.init_status)
      LOG(INFO) << "module " << ist.name << " init status: " << Enum2String(ist.state);
    for (const auto& mst : status.module_status)
      if (mst.ec != kModuleOK) {
        LOG(ERROR) << "module " << mst.name << " error: (" << Enum2String(mst.ec) << ") " << mst.msg;
        AddModuleStatusInfo(mst, result);
      }
  };

  cmd_info.writetopic_cb = [this](const std::unordered_map<std::string, std::set<std::string>>& su) {
    Json::Value jsonArrayValue = Json::arrayValue;
    for (auto module : su) {
      for (auto topic : module.second) {
        std::time_t t_c = std::time(nullptr);
        std::ostringstream ostr;
        ostr << std::put_time(std::localtime(&t_c), "%F %T");
        std::string sTime = ostr.str();
        std::cout << sTime;
        Json::Value indexItem;
        indexItem["timestamp"] = sTime;
        indexItem["topic"] = topic;
        indexItem["moduleName"] = module.first;
        jsonArrayValue.append(indexItem);
      }
    }
    Json::Value jsonBody;
    jsonBody["data"] = jsonArrayValue;
    std::string body = Json::writeString(Json::StreamWriterBuilder(), jsonBody);
    topic_writer_->TotalTopic(body);
  };

  cmd_info.max_setup_monitoring_count = 600;  // approximately 10 min.
  std::string scenario_path = params.scenario_path();
  cmd_info.reset_params.scenario_path = snpr_->CheckIsOpenScenario(scenario_path);
  std::string sLabels = GetScenarioLabels(GetServiceConfigFilePath(app_path_));
  cmd_info.reset_params.m_labels = sLabels;
  std::string sEgoType = snpr_->GetEgoType(cmd_info.reset_params.scenario_path);
  topic_writer_->SetEgoType(sEgoType);
  // process log2world config
  l2w_cfg_.reset(new Log2WorldPlayConfig);
  Json::Value l2w_params;
  std::string sceneType;
  if (!params.log2world_params().empty()) {
    LOG(INFO) << "received a logsim(log2world) scenario request: " << params.log2world_params();
    try {
      ReadJsonDocFromString(l2w_params, params.log2world_params());
      sceneType = ParseLog2WorldArgs(l2w_params, cmd_info.l2w_config);
    } catch (const std::exception& e) {
      LOG(ERROR) << "parsing log2world args error: " << e.what();
      result->set_ec(sim_cloud::InitiationFailed);
      return false;
    }
    *l2w_cfg_ = cmd_info.l2w_config;
    try {
      snpr_->ParseLogsimEvents(cmd_info.reset_params.scenario_path, cmd_info.logsim_events);
      ReportLogsimEvents(l2w_params["reportUrl"].asString(), cmd_info.logsim_events, params.job_id(),
                         params.scene_id());
      if (cmd_info.logsim_events.start_timestamp_ms == 0 && cmd_info.l2w_config.switch_time_ms != 0)
      // event数据为空 但是设置了切换时间,我认为此时跑的是log2world但是没有录制数据.必须报错
      {
        throw std::runtime_error(std::string("ego start_timestamp_ms is empty but switch time is not empty"));
      }
      if (cmd_info.logsim_events.events.empty() && cmd_info.l2w_config.switch_type == kLog2WorldSwitchEvent) {
        // logsim event数据为空，但是选择了事件切换。
        throw std::runtime_error(std::string("logsim event is empty.can not swich by event"));
      }
    } catch (const std::exception& e) {
      LOG(ERROR) << "parsing/reporting logsim events info failed: " << e.what();
      result->set_ec(sim_cloud::InitiationFailed);
      auto* s = result->add_module_status();
      s->set_name("cloud_service");
      s->set_sc(sim_cloud::ModuleException);
      s->set_msg(e.what());
      return false;
    }
  }

  CmdErrorCode ec = coordinator_->Execute(cmd_info);
  if (ec != kCmdSucceed) {
    result->set_ec((ec == kCmdSystemError || ec == kCmdScenarioParsingError) ? sim_cloud::InternalError
                                                                             : sim_cloud::InitiationFailed);
    LOG(ERROR) << "setup module error: " << Enum2String(ec);
    if (ec == kCmdScenarioParsingError) {
      auto* s = result->add_module_status();
      s->set_name("cloud_service");
      s->set_sc(sim_cloud::ModuleException);
      std::string new_scenario_path = GetScenarioFileName(scenario_path);
      s->set_msg(new_scenario_path + " parse error.");
    } else if (ec == kCmdUnEnoughInitiatedModule) {
      auto* s = result->add_module_status();
      s->set_name("cloud_service");
      s->set_sc(sim_cloud::ModuleException);
      s->set_msg("unenough init module count");
    }
  } else {
    if (params.log2world_params().empty() || sceneType == "LOGSIM") {
      topic_writer_->SetIds(params.job_id(), params.scene_id(), -1, "SCENE");
    } else {
      int32_t switch_time = coordinator_->log2world_switch_time();
      std::string sType = coordinator_->getSwitchType();
      switch_time == std::numeric_limits<int32_t>::max() ? -1 : switch_time;
      int32_t input_switch_time = switch_time;
      if (sType == "EVENT") { input_switch_time = coordinator_->kafka_log2world_switch_time(); }
      topic_writer_->SetIds(params.job_id(), params.scene_id(), -1, "LOG", input_switch_time,
                            cmd_info.logsim_events.start_timestamp_ms, switch_time);
    }
  }
  return ec == kCmdSucceed;
}


/**
 * @brief write the grading result by calling SimCloudTopicWriter's WritePb()
 * @param[in] status the input command status
 * @param[in] frame_id the frame id of the step
 * @param[in] simtime the simulation time of the step
 */
void SimCloudService::WriteGradingResult(const CommandStatus& status, uint64_t frame_id, uint64_t sim_time) {
  for (const ModuleCmdStatus& m : status.module_status) {
    LOG(INFO) << m.name << " feedback:{" << m.feedback << "} with msg:{" << m.msg << "}";
    if (coordinator_->IsGradingModule(m.name) && !m.feedback.empty()) {
      topic_writer_->WritePb(m.feedback, frame_id, sim_time);
      return;
    }
  }
  LOG(ERROR) << "no grading module feedback found.";
}


/**
 * @brief find the process tool path
 * @param[in] tool_name the tool name
 * @param[out] tool_path the tool path
 */
void SimCloudService::FindScenarioProcessTool(const std::string& tool_name, std::string& tool_path) {
  boost::filesystem::path process_bin_path(app_path_);
  process_bin_path /= tool_name;
  if (boost::filesystem::exists(process_bin_path)) tool_path = process_bin_path.lexically_normal().string();
  else
    LOG(WARNING) << "no process tool(" << tool_name << ") found. pre/post processing for scenario is disabled.";
}


/**
 * @brief process the scenario file by tool
 * @param[in] tool_path the tool path
 * @param[in] scenario_path the scenario path
 * @param[in] job_id the job id
 * @param[in] scene_id the scene id
 */
void SimCloudService::ExtraProcessScenario(const std::string& tool_path, const std::string& scenario_path,
                                           int32_t job_id, int32_t scene_id) {
  int64_t uVMem = 0;
  int64_t nMemUse = GetMemUse(uVMem);
  LOG(INFO) << "nMemUse:" << nMemUse << ", uVMem: " << uVMem;

  if (tool_path.empty()) return;
  LOG(INFO) << "job " << job_id << " extra-processing(" << tool_path << ") scenario id: " << scene_id << " ...";
  std::ostringstream cmd;
  cmd << tool_path << " " << app_path_ << " " << FLAGS_log_dir << " " << job_id << " " << scene_id << " "
      << scenario_path;
  int r = system(cmd.str().c_str());
  LOG(INFO) << "done extra-processing scenario: " << GetProcessExitStatusDescription(r);
}


/**
 * @brief report the logsim events info report url
 * @param[in] report_addr the report address
 * @param[in] events the logsim events info
 * @param[in] job_id the job id
 * @param[in] scene_id the scene id
 */
void SimCloudService::ReportLogsimEvents(const std::string& report_addr, const LogsimEventInfo& events, int32_t job_id,
                                         int32_t scene_id) {
  LOG(INFO) << "reporting logsim events info to <" << report_addr << ">, jobid = " << job_id
            << ", sceneid = " << scene_id;
  namespace beast = boost::beast;  // from <boost/beast.hpp>
  namespace http = beast::http;    // from <boost/beast/http.hpp>
  namespace net = boost::asio;     // from <boost/asio.hpp>
  using tcp = net::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

  net::io_context ioc;

  const auto pos = report_addr.find_first_of(':');
  if (pos == std::string::npos) throw std::invalid_argument(std::string("invalid report address: ") + report_addr);
  const std::string report_ip(report_addr.substr(0, pos)), report_port(report_addr.substr(pos + 1));
  tcp::resolver resolver(ioc);
  const auto results = resolver.resolve(report_ip, report_port);

  beast::tcp_stream stream(ioc);
  stream.connect(results);

  int64_t ts = GetWallTimeInMillis();
  Json::Value doc;
  events.Encode(doc);
  doc["jobId"] = job_id;
  doc["sceneId"] = scene_id;
  doc["ts"] = ts;
  std::string sign_src_str = std::to_string(job_id) + std::to_string(scene_id) + std::to_string(ts) + "tlasmai";
  unsigned char md5_rst[MD5_DIGEST_LENGTH];
  MD5(reinterpret_cast<const unsigned char*>(sign_src_str.c_str()), sign_src_str.size(), md5_rst);
  std::ostringstream ss;
  ss << std::hex << std::setfill('0');
  for (auto c : md5_rst) ss << std::setw(2) << static_cast<int>(c);
  doc["sign"] = ss.str();
  std::string payload;
  WriteJsonDocToString(doc, payload);
  LOG(INFO) << "post /simService/third/log2world/sceneInfo body => " << payload;

  http::request<http::string_body> req{http::verb::post, "/simService/third/log2world/sceneInfo", 11};
  req.set(http::field::host, report_ip);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::content_type, "application/json");
  req.content_length(payload.size());
  req.body() = payload;

  http::write(stream, req);

  beast::flat_buffer buffer;
  http::response<http::dynamic_body> res;
  http::read(stream, buffer, res);
  if (res.result() != beast::http::status::ok)
    throw std::runtime_error(report_addr + std::string(" response status error => ") +
                             std::to_string(res.result_int()));
  const std::string res_body = beast::buffers_to_string(res.body().data());
  LOG(INFO) << "post /simService/third/log2world/sceneInfo received logsim events report response => " << res_body;
  Json::Value res_doc;
  ReadJsonDocFromString(res_doc, res_body);
  if (res_doc["code"].asInt() != 0) throw std::runtime_error(std::string("received error response => ") + res_body);
}


/**
 * @brief parse log2world args from json
 * @param[in] args the input json args
 * @param[out] config the output Log2WorldPlayConfig configuration
 * @return switch_type
 */
std::string SimCloudService::ParseLog2WorldArgs(const Json::Value& args, Log2WorldPlayConfig& config) {
  config.ego_switch = args.get("egoSwitch", false).asBool();
  config.traffic_switch = args.get("trafficSwitch", false).asBool();
  config.switch_time_ms = args.get("switchTime", 0).asInt();
  const std::string switch_type = args.get("switchType", "").asString();
  if (switch_type.empty()) return "LOGSIM";
  if (switch_type == "SCENE") config.switch_type = kLog2WorldSwitchScene;
  else if (switch_type == "EVENT")
    config.switch_type = kLog2WorldSwitchEvent;
  else
    throw std::invalid_argument(std::string("unknown log2world switch type: ") + switch_type);
  return switch_type;
}


/**
 * @brief get service config file path by the input app root path
 * @param[in] app_root the input app root path
 * @return config file path
 */
const std::string SimCloudService::GetServiceConfigFilePath(const std::string& app_root) {
  boost::filesystem::path cfg_file = app_root;
  cfg_file /= kServiceConfigFileName;
  std::string cfg_file_path(cfg_file.lexically_normal().string());
  if (!boost::filesystem::is_regular_file(cfg_file_path))
    throw std::runtime_error("no " + cfg_file_path + " file found in " + app_path_);
  return cfg_file_path;
}


/******************************************************************************/
/*************************** main entry point *********************************/
/******************************************************************************/

/**
 * @brief parse command line arguments.
 * @param ac number of arguments.
 * @param av array of argument values.
 * @param endpoint the specified ip address or host name + port which the cloud service should listen on.
 * @param override_dir an optional root directory where contains files that override some part of the cloud service
 * configurations.
 * @return true if succeed, false otherwise.
 */
static bool InstallProgramOptions(int ac, char** av, std::string& root, std::string& endpoint,
                                  std::string& override_dir) {
  prog_ops::options_description op_desc("Options");
  // clang-format off
  op_desc.add_options()
    ("help,h", "display help info")
    ("root,r", prog_ops::value<std::string>(&root)->default_value("/home/sim"),
      "the root directory of the cloud service in where contains configurations and data.")
    ("override-root,o", prog_ops::value<std::string>(&override_dir)->default_value(""),
      "an optional root directory where contains files that override some part of the cloud service configurations.")
    ("endpoint,p", prog_ops::value<std::string>(&endpoint)->default_value("0.0.0.0:8401"),
      "if specified, the cloud service will listening on the specified address. format: address:port")
    ("logdir,l", prog_ops::value<std::string>(&FLAGS_log_dir)->default_value(""),
      "the directory in where to write the log files of the cloud service. default will write to the /tmp")
    ("log-level,v", prog_ops::value<int32_t>(&FLAGS_v)->default_value(0),
      "the logging level would be applied, only logging the ones whose level is less than or equal to it.");
  // clang-format on
  prog_ops::parse_command_line(ac, av, op_desc);
  prog_ops::variables_map vm;
  prog_ops::store(prog_ops::parse_command_line(ac, av, op_desc), vm);
  prog_ops::notify(vm);
  if (vm.count("help")) {
    std::cout << op_desc << std::endl;
    return false;
  }
  return true;
}


/**
 * @brief main function of the cloud service.
 * @param argc number of arguments.
 * @param argv array of argument values.
 * @return 0 if succeed, 1 otherwise.
 */
int main(int argc, char* argv[]) {
  std::string app_root, endpoint, cloud_override_dir;
  // parse command line arguments.
  if (!InstallProgramOptions(argc, argv, app_root, endpoint, cloud_override_dir)) return 0;

  // set glog options.
  FLAGS_logbuflevel = -1;  // no buffe
  if (!FLAGS_log_dir.empty()) google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  std::unique_ptr<grpc::Server> server = nullptr;
  std::unique_ptr<SimCloudService> service = nullptr;
  try {
    // sim service construct
    service.reset(new SimCloudService(app_root, cloud_override_dir));
    // grpc server construct and start
    grpc::ServerBuilder builder;
    builder.AddListeningPort(endpoint, grpc::InsecureServerCredentials());
    builder.RegisterService(service.get());
    server = std::move(builder.BuildAndStart());
  } catch (const std::exception& e) {
    LOG(ERROR) << "cloud service initiation failed: " << e.what();
    return 1;
  }

  if (server) {
    LOG(INFO) << "sim cloud service serving on " << endpoint << "...";
    server->Wait();  // block until exit.
  } else {
    LOG(ERROR) << "sim cloud grpc server build/start failed.";
    return 1;
  }
  return 0;
}