// Copyright 2024 Tencent Inc. All rights reserved.
//
// \brief "云端虚拟城市仿真调度引擎服务"
//
#include <atomic>
#include <iomanip>
#include <random>
#include <sstream>
#include <thread>

#include "common/coord_trans.h"
#include "boost/algorithm/string/join.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/range/adaptor/transformed.hpp"
#include "glog/logging.h"
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"

#include "grading.pb.h"
#include "location.pb.h"
#include "sim_cloud_city_service.grpc.pb.h"
#include "traffic.pb.h"

#include "cloud_topic_writer.h"
#include "coordinator/config_mgr.h"
#include "coordinator/coordinator.h"
#include "coordinator/scenario_parser.h"
#include "coordinator/xml_util.h"
#include "txsim_messenger.h"
#include "utils/proc_service.h"
#include "utils/time.h"


// macro definition of the module name
#define TXSIM_CLOUD_CITY_MODULE_NAME_GRADING  "Grading"
#define TXSIM_CLOUD_CITY_MODULE_NAME_PLANNING "Planning"
#define TXSIM_CLOUD_CITY_DEFAULT_MAP          "yby.sqlite"

// macro definition of the config key name
#define TXSIM_CLOUD_CITY_CONFIG_KEY_LON              "lon"
#define TXSIM_CLOUD_CITY_CONFIG_KEY_LAT              "lat"
#define TXSIM_CLOUD_CITY_CONFIG_KEY_CITY_INFO        "cityInfo."
#define TXSIM_CLOUD_CITY_CONFIG_KEY_ORIGIN_POINT     TXSIM_CLOUD_CITY_CONFIG_KEY_CITY_INFO "originPoint."
#define TXSIM_CLOUD_CITY_CONFIG_KEY_ORIGIN_X         TXSIM_CLOUD_CITY_CONFIG_KEY_ORIGIN_POINT \ 
                                                     TXSIM_CLOUD_CITY_CONFIG_KEY_LON
#define TXSIM_CLOUD_CITY_CONFIG_KEY_ORIGIN_Y         TXSIM_CLOUD_CITY_CONFIG_KEY_ORIGIN_POINT \
                                                     TXSIM_CLOUD_CITY_CONFIG_KEY_LAT
#define TXSIM_CLOUD_CITY_CONFIG_KEY_END_POINTS       TXSIM_CLOUD_CITY_CONFIG_KEY_CITY_INFO "endPoints"
#define TXSIM_CLOUD_CITY_CONFIG_KEY_HADMAP           TXSIM_CLOUD_CITY_CONFIG_KEY_CITY_INFO "hadmap"
#define TXSIM_CLOUD_CITY_CONFIG_KEY_START_POINT      TXSIM_CLOUD_CITY_CONFIG_KEY_CITY_INFO "startLocation."
#define TXSIM_CLOUD_CITY_CONFIG_KEY_START_LON        TXSIM_CLOUD_CITY_CONFIG_KEY_START_POINT \ 
                                                     TXSIM_CLOUD_CITY_CONFIG_KEY_LON
#define TXSIM_CLOUD_CITY_CONFIG_KEY_START_LAT        TXSIM_CLOUD_CITY_CONFIG_KEY_START_POINT \
                                                     TXSIM_CLOUD_CITY_CONFIG_KEY_LAT
#define TXSIM_CLOUD_CITY_CONFIG_KEY_START_VELOCITY   TXSIM_CLOUD_CITY_CONFIG_KEY_START_POINT "v"
#define TXSIM_CLOUD_CITY_CONFIG_KEY_START_THETA      TXSIM_CLOUD_CITY_CONFIG_KEY_START_POINT "theta"
#define TXSIM_CLOUD_CITY_CONFIG_KEY_MAX_VELOCITY     TXSIM_CLOUD_CITY_CONFIG_KEY_CITY_INFO "maxVelocity"
#define TXSIM_CLOUD_CITY_CONFIG_KEY_PERCEPTION_RANGE TXSIM_CLOUD_CITY_CONFIG_KEY_CITY_INFO "maxPerceptionRange"
#define TXSIM_CLOUD_CITY_CONFIG_KEY_JOB_ID           TXSIM_CLOUD_CITY_CONFIG_KEY_CITY_INFO "jobId"
#define TXSIM_CLOUD_CITY_CONFIG_KEY_EGO_ID           TXSIM_CLOUD_CITY_CONFIG_KEY_CITY_INFO "egoId"

// macro definition of the setting
#define TXSIM_CLOUD_CITY_RESET_MAX_TRY_TIMES 5


// some namespace alias for convenience
namespace prog_ops = boost::program_options;
using namespace tx_sim::coordinator;
using namespace tx_sim::impl;
using namespace tx_sim::utils;
using namespace sim_msg;


// static variables
static std::random_device g_rd;
static std::mt19937 g_gen(g_rd());


/******************************************************************************/
/*************************** helper functions *********************************/
/******************************************************************************/

/**
 * @brief transform a protobuf message to a zmq message
 * @param pb_msg - input protobuf message
 * @return zmq message
 */
static zmq::message_t Protobuf2ZmqMsg(const google::protobuf::Message& pb_msg) {
  size_t byte_size = pb_msg.ByteSizeLong();
  uint8_t* byte_array = new uint8_t[byte_size];
  pb_msg.SerializeToArray(byte_array, byte_size);
  return zmq::message_t(byte_array, byte_size, [](void* data, void* hint) { delete[] static_cast<uint8_t*>(data); });
}


/******************************************************************************/
/************************* Sim Cloud City Service *****************************/
/******************************************************************************/

/**
 * @brief "云端虚拟城市仿真调度引擎服务端", inherits from grpc SimDriverService::Service,
 * calling SimInit() and SimStep().
 */
class SimCloudCityService final : public SimDriverService::Service {
 public:
  //! @brief 构造函数：SimCloudCityService
  //! @details 构造函数用途：初始化SimCloudCityService对象
  //!
  //! @param[in] root_path 根路径
  SimCloudCityService(const std::string& root_path);
  //! @brief 析构函数：SimCloudCityService
  //! @details 析构函数用途：销毁SimCloudCityService对象
  ~SimCloudCityService();
  //! @brief 成员函数：SetupSimEnv
  //! @details 成员函数用途：设置模拟环境
  void SetupSimEnv();
  //! @brief 成员函数：SimInit
  //! @details 成员函数用途：初始化模拟
  //!
  //! @param[in] ctx gRPC服务器上下文
  //! @param[in] req 初始化请求
  //! @param[out] resp 初始化响应
  //!
  //! @return gRPC状态
  grpc::Status SimInit(grpc::ServerContext* ctx, const InitRequest* req, InitResponse* resp) override;
  //! @brief 成员函数：SimStep
  //! @details 成员函数用途：执行模拟步骤
  //!
  //! @param[in] ctx gRPC服务器上下文
  //! @param[in] input 步骤输入
  //! @param[out] output 步骤输出
  //!
  //! @return gRPC状态
  grpc::Status SimStep(grpc::ServerContext* ctx, const StepInput* input, StepOutput* output) override;

 private:
  //! @brief 成员函数：GetSetupConfigurations
  //! @details 成员函数用途：从文件中读取配置信息
  //!
  //! @param[in] file_path 配置文件路径
  void GetSetupConfigurations(const std::string& file_path);
  //! @brief 成员函数：CheckResetStatus
  //! @details 成员函数用途：检查重置状态
  //!
  //! @param[out] output_status 输出的错误状态
  //!
  //! @return 布尔值，表示是否需要重置
  bool CheckResetStatus(ErrorStatus& output_status);
  //! @brief 成员函数：OutputResetingMessages
  //! @details 成员函数用途：输出重置消息
  //!
  //! @param[out] output 输出的步骤输出
  //! @param[in] input_traffic 输入的交通信息
  //! @param[in] frame_id 帧ID
  //! @param[in] begin_frame_id 开始帧ID
  void OutputResetingMessages(StepOutput* output, const sim_msg::Traffic& input_traffic, uint64_t frame_id,
                              uint64_t begin_frame_id);
  //! @brief 成员函数：GetTrafficPerceptionMsg
  //! @details 成员函数用途：获取交通感知消息
  //!
  //! @param[in] traffic 输入的交通信息
  //! @param[in] adjust_time 调整时间
  //!
  //! @return zmq消息，包含交通感知信息
  zmq::message_t GetTrafficPerceptionMsg(const Traffic& traffic, uint64_t adjust_time);
  //! @brief 成员函数：UpdateEgoLocation
  //! @details 成员函数用途：更新自车位置
  //!
  //! @param[in] ego_location 输入的自车位置
  void UpdateEgoLocation(const Location& ego_location);
  //! @brief 成员函数：UpdateOutputLocationFromLastOne
  //! @details 成员函数用途：根据上一个位置更新输出位置
  //!
  //! @param[in,out] loc 输入/输出的位置
  void UpdateOutputLocationFromLastOne(Location* loc);
  //! @brief 成员函数：CheckStepStatus
  //! @details 成员函数用途：检查步骤状态
  //!
  //! @param[in] step_ec 步骤错误代码
  //! @param[out] output_status 输出的错误状态
  //! @param[in] frame_id 帧ID
  void CheckStepStatus(CmdErrorCode step_ec, ErrorStatus& output_status, uint64_t frame_id);
  //! @brief 成员函数：ResetEgoRoute
  //! @details 成员函数用途：重置自车路线
  //!
  //! @param[in,out] reset_params 重置参数
  void ResetEgoRoute(ModuleResetRequest& reset_params);
  //! @brief 成员函数：ResetEgoCar
  //! @details 成员函数用途：重置自车
  //!
  //! @param[in] frame_id 帧ID
  void ResetEgoCar(uint64_t frame_id);
  //! @brief 成员函数：WriteGradingResult
  //! @details 成员函数用途：写入评分结果
  //!
  //! @param[in] status 命令状态
  //! @param[in] frame_id 帧ID
  void WriteGradingResult(const CommandStatus& status, uint64_t frame_id);
  //! @brief 成员函数：PrintStepTimeCost
  //! @details 成员函数用途：打印步骤时间消耗
  //!
  //! @param[in] start_time 开始时间
  //! @param[in] us1 时间消耗1
  //! @param[in] sy1 时间消耗2
  void PrintStepTimeCost(int64_t start_time, uint64_t us1, uint64_t sy1);

  //! @brief 成员变量：ego_id_
  //! @details 成员变量用途：存储自车ID
  int64_t ego_id_;

  //! @brief 成员变量：job_id_
  //! @details 成员变量用途：存储任务ID
  int64_t job_id_;

  //! @brief 成员变量：max_ego_perception_distance_
  //! @details 成员变量用途：存储自车感知距离的最大值
  double max_ego_perception_distance_ = 0;

  //! @brief 成员变量：last_end_idx_
  //! @details 成员变量用途：存储上一个结束索引
  int32_t last_end_idx_ = -1;

  //! @brief 成员变量：no_location_count_
  //! @details 成员变量用途：存储没有位置的计数
  int32_t no_location_count_ = 0;

  //! @brief 成员变量：m_begin_frame_id
  //! @details 成员变量用途：存储开始帧ID
  uint64_t m_begin_frame_id = 0;

  //! @brief 成员变量：last_step_failed_
  //! @details 成员变量用途：存储上一步是否失败的标志
  std::atomic<bool> last_step_failed_{false};

  //! @brief 成员变量：rpc_mtx_
  //! @details 成员变量用途：用于同步RPC操作的互斥量
  std::mutex rpc_mtx_;

  //! @brief 成员变量：init_cv_
  //! @details 成员变量用途：用于同步初始化操作的条件变量
  std::condition_variable init_cv_;

  //! @brief 成员变量：init_result_
  //! @details 成员变量用途：存储初始化结果
  ErrorStatus init_result_;

  //! @brief 成员变量：last_reset_result_
  //! @details 成员变量用途：存储上一次重置结果
  std::atomic<CmdErrorCode> last_reset_result_{kCmdAccepted};  // accepted means the execution is not finished yet.

  //! @brief 成员变量：ego_reset_params_
  //! @details 成员变量用途：存储自车重置参数
  ModuleResetRequest ego_reset_params_;

  //! @brief 成员变量：last_location_
  //! @details 成员变量用途：存储上一个位置
  sim_msg::Location last_location_;

  //! @brief 成员变量：origin_start_location_
  //! @details 成员变量用途：存储起始位置
  sim_msg::Location origin_start_location_;

  //! @brief 成员变量：end_points_
  //! @details 成员变量用途：存储终点的集合
  std::vector<std::pair<double, double>> end_points_;

  //! @brief 成员变量：step_log_prefix_
  //! @details 成员变量用途：存储步骤日志前缀
  std::string step_log_prefix_;

  //! @brief 成员变量：reset_thread_
  //! @details 成员变量用途：存储重置线程的指针
  std::unique_ptr<std::thread> reset_thread_;

  //! @brief 成员变量：coordinator_
  //! @details 成员变量用途：存储协调器的指针
  std::unique_ptr<Coordinator> coordinator_;

  //! @brief 成员变量：topic_writer_
  //! @details 成员变量用途：存储SimCloud主题写入器的指针
  std::unique_ptr<SimCloudTopicWriter> topic_writer_;

  //! @brief 成员变量：m_lastmileage
  //! @details 成员变量用途：存储上一个里程
  double m_lastmileage = 0;

  //! @brief 成员变量：m_curmileage
  //! @details 成员变量用途：存储当前里程
  double m_curmileage = 0;

  //! @brief 成员变量：m_stoping
  //! @details 成员变量用途：存储是否停止的标志
  bool m_stoping = true;
};


/**
 * @brief constructor
 * @param[in] root_path The directory of configuration files and database
 */
SimCloudCityService::SimCloudCityService(const std::string& root_path) {
  boost::filesystem::path cfg_file = root_path, cfg_db = root_path;
  // local_service.config算法配置路径
  cfg_file /= kServiceConfigFileName;
  // 数据库路径
  cfg_db /= kServiceDatabaseName;
  std::string cfg_file_path(cfg_file.lexically_normal().string());
  if (!boost::filesystem::is_regular_file(cfg_file_path))
    throw std::runtime_error("no " + cfg_file_path + " file found in " + root_path);
  // 获取算法启动配置
  GetSetupConfigurations(cfg_file_path);

  auto cfg_mgr = std::make_shared<ConfigManager>(cfg_db.lexically_normal().string());
  // 初始化数据库
  cfg_mgr->InitConfigData(cfg_file_path, root_path);
  // 构造zmq服务,该服务算法与调度服务通信
  auto ctx = std::make_shared<zmq::context_t>();
  coordinator_.reset(new Coordinator(
      root_path, std::dynamic_pointer_cast<ConfigLoader>(cfg_mgr),
      std::make_shared<ModuleManagerImpl>(std::make_shared<ModuleRegistryImpl>(ctx),
                                          std::make_shared<ZmqConnectionFactory>(ctx),
                                          std::make_shared<ModuleProcessServiceImpl>(ctx), cfg_file_path),
      std::make_shared<ScenarioXmlParser>()));
  coordinator_->AddCloudCityDefaultStepModule();

  topic_writer_.reset(new SimCloudTopicWriter(cfg_file_path));
  topic_writer_->SetIds(job_id_, -1, ego_id_);

  init_result_.set_err_code(ERROR_NOT_INIT);
}


/**
 * @brief destructor
 */
SimCloudCityService::~SimCloudCityService() {
  // stop the reset thread
  if (reset_thread_ && reset_thread_->joinable()) reset_thread_->join();
}


/**
 * @brief Setup the simulation environment
 */
void SimCloudCityService::SetupSimEnv() {
  // first reset the ego
  ResetEgoCar(0);
  CmdErrorCode ec = last_reset_result_.load(std::memory_order_acquire);

  std::unique_lock<std::mutex> lk(rpc_mtx_);
  init_result_.set_err_code(ec == kCmdSucceed ? SUCCESS : ERROR_INIT);
  if (ec != kCmdSucceed) init_result_.set_err_msg(Enum2String(ec));
  // notify the init_cv_ in SimInit
  init_cv_.notify_all();
}


/**
 * @brief the grpc call to initialize the simulation service
 * @param[in] ctx grpc context
 * @param[in] req request message for initialization
 * @param[out] resp response message for clients for a new request
 * @return grpc status
 */
grpc::Status SimCloudCityService::SimInit(grpc::ServerContext* ctx, const InitRequest* req, InitResponse* resp) {
  std::unique_lock<std::mutex> lk(rpc_mtx_);
  // wait for init_cv_ to be notified in SetupSimEnv
  init_cv_.wait(lk, [this] { return init_result_.err_code() != ERROR_NOT_INIT; });
  resp->mutable_status()->MergeFrom(init_result_);
  return grpc::Status::OK;
}


/**
 * @brief the grpc call to step the simulation
 * @param[in] ctx grpc context
 * @param[in] input request message for step
 * @param[out] output response message for step
 * @return grpc status
 */
grpc::Status SimCloudCityService::SimStep(grpc::ServerContext* ctx, const StepInput* input, StepOutput* output) {
  // start time record
  int64_t step_start_time = tx_sim::utils::GetCpuTimeInMillis();
  uint64_t us1 = 0, sy1 = 0;
  std::tie(us1, sy1) = tx_sim::utils::GetProcessCpuTime();
  std::unique_lock<std::mutex> lck(rpc_mtx_, std::defer_lock);
  if (!lck.try_lock()) {
    LOG(WARNING) << "step_frame_" << input->frame_id() << " concurrent rpc call.";
    PrintStepTimeCost(step_start_time, us1, sy1);
    return grpc::Status(grpc::StatusCode::CANCELLED, "concurrent rpc call.");
  }

  uint64_t frame_id = input->frame_id();
  step_log_prefix_ = "step_frame_" + std::to_string(frame_id);
  if (input->log_level() >= 0) FLAGS_v = input->log_level();
  VLOG(1) << step_log_prefix_ << " start executing ...";

  output->set_uuid(ego_id_);
  // confirm reset status is ready
  if (!CheckResetStatus(*output->mutable_status())) {
    if (output->status().err_code() == MODULE_STOP)
      OutputResetingMessages(output, input->traffic(), frame_id, m_begin_frame_id);
    PrintStepTimeCost(step_start_time, us1, sy1);
    return grpc::Status::OK;
  }
  // check last step execution result
  if (last_step_failed_.load(std::memory_order_acquire)) {
    LOG(ERROR) << step_log_prefix_ << " last step execution failed.";
    output->mutable_status()->set_err_code(ERROR_STEP);
    PrintStepTimeCost(step_start_time, us1, sy1);
    return grpc::Status::OK;
  }
  // if m_stoping, reset the m_begin_frame_id to input frame id
  if (m_stoping) {
    m_begin_frame_id = frame_id;
    m_stoping = false;
  }
  // assembling command info ...
  CommandInfo cmd_info(kCmdStep);
  std::string log_level_msg_str = std::to_string(input->log_level());
  cmd_info.input_msgs[tx_sim::topic::kLogLevel] = zmq::message_t(log_level_msg_str.data(), log_level_msg_str.size());
  // handle traffic message from input
  cmd_info.input_msgs[tx_sim::topic::kTraffic] =
      GetTrafficPerceptionMsg(input->traffic(), coordinator_->current_simulation_time());
  cmd_info.status_cb = [this, input, output, frame_id](const CommandStatus& status) {
    if (status.ec == kCmdAccepted) return;
    VLOG(1) << std::setprecision(3) << step_log_prefix_ << " step_time_cost: " << status.total_time_cost << "/"
            << status.total_cpu_time_us << "/" << status.total_cpu_time_sy << "ms (" << status.GetTimeCostStr() << ")";
    bool found_location_msg = false;
    std::ostringstream ss;
    for (const auto& kv : status.step_message.messages) {
      std::string sTopic = kv.first.sTopic;
      if (sTopic == tx_sim::topic::kLocation) {
        found_location_msg = true;
        Location& loc = *output->mutable_ego_location();
        loc.ParseFromArray(kv.second.data(), kv.second.size());
        UpdateEgoLocation(loc);
      } else if (sTopic == tx_sim::topic::kGrading) {
        Grading& gading = *output->mutable_grading();
        gading.ParseFromArray(kv.second.data(), kv.second.size());
        m_curmileage = m_lastmileage + gading.mileage();
        gading.set_mileage(m_curmileage);
      } else if (sTopic == tx_sim::topic::kTrajectory) {
        Trajectory& trajectory = *output->mutable_planning_line();
        trajectory.ParseFromArray(kv.second.data(), kv.second.size());
        // note: sample trajectory point for reduce front-end pressure
        SampleTrajectory(&trajectory, 3, 5);
      } else if (sTopic == "LOCATION_TRAILER") {
        Location& trailer = *output->add_trailer();
        trailer.ParseFromArray(kv.second.data(), kv.second.size());
      }
      ss << "(" << sTopic << ": " << kv.second.size() << " bytes), ";
    }

    VLOG(1) << step_log_prefix_ << " written kafka with messages: [" << ss.str() << "]";
    topic_writer_->WritePb(status.step_message, frame_id, 0, true, m_lastmileage, m_begin_frame_id);

    // Dealing with no location message
    if (found_location_msg) {
      no_location_count_ = 0;
    } else {
      ++no_location_count_;
      LOG(WARNING) << step_log_prefix_ << " no " << tx_sim::topic::kLocation
                   << " message found! count:" << no_location_count_;
      UpdateOutputLocationFromLastOne(output->mutable_ego_location());
    }
  };
  // assembling command info done. executing it ...
  CmdErrorCode ec = coordinator_->Execute(cmd_info);
  CheckStepStatus(ec, *output->mutable_status(), frame_id);
  output->set_begin_frame_id(m_begin_frame_id);

  VLOG(1) << step_log_prefix_ << " execution finished.";
  PrintStepTimeCost(step_start_time, us1, sy1);
  return grpc::Status::OK;
}


/**
 * @brief get the setup configurations from a json config file
 * @param file_path the path to the config file to be read
 */
void SimCloudCityService::GetSetupConfigurations(const std::string& file_path) {
  boost::property_tree::ptree pt;
  boost::property_tree::json_parser::read_json(file_path, pt);

  // read the ego start location in.
  auto* start_loc = last_location_.mutable_position();
  start_loc->set_x(pt.get<double>(TXSIM_CLOUD_CITY_CONFIG_KEY_START_LON));
  start_loc->set_y(pt.get<double>(TXSIM_CLOUD_CITY_CONFIG_KEY_START_LAT));
  start_loc->set_z(0);
  last_location_.mutable_rpy()->set_z(pt.get<double>(TXSIM_CLOUD_CITY_CONFIG_KEY_START_THETA));
  auto* last_v = last_location_.mutable_velocity();
  double start_v = pt.get<double>(TXSIM_CLOUD_CITY_CONFIG_KEY_START_VELOCITY);
  last_v->set_x(start_v * std::cos(last_location_.rpy().z()));
  last_v->set_y(start_v * std::sin(last_location_.rpy().z()));
  origin_start_location_ = last_location_;
  if (ego_reset_params_.ego_infos.size() == 0) { ego_reset_params_.ego_infos.emplace_back(); }
  ModuleEgoInfo& ego_info = ego_reset_params_.ego_infos.back();
  // read other reset parameters in.
  ego_reset_params_.ego_infos.back().ego_speed_limit = pt.get<double>(TXSIM_CLOUD_CITY_CONFIG_KEY_MAX_VELOCITY);
  ego_reset_params_.map_local_origin[0] = pt.get<double>(TXSIM_CLOUD_CITY_CONFIG_KEY_ORIGIN_X);
  ego_reset_params_.map_local_origin[1] = pt.get<double>(TXSIM_CLOUD_CITY_CONFIG_KEY_ORIGIN_Y);
  ego_reset_params_.map_local_origin[2] = 0;
  for (auto& item : pt.get_child(TXSIM_CLOUD_CITY_CONFIG_KEY_END_POINTS)) {
    end_points_.emplace_back(item.second.get<double>(TXSIM_CLOUD_CITY_CONFIG_KEY_LON),
                             item.second.get<double>(TXSIM_CLOUD_CITY_CONFIG_KEY_LAT));
  }
  if (end_points_.empty()) throw std::invalid_argument("end point list is empty in config file.");
  std::string hadmap_path = pt.get<std::string>(TXSIM_CLOUD_CITY_CONFIG_KEY_HADMAP);
  ego_reset_params_.map_path = hadmap_path.empty() ? TXSIM_CLOUD_CITY_DEFAULT_MAP : hadmap_path;
  LOG(INFO) << "using hadmap: " << ego_reset_params_.map_path;

  // read cloud city configurations for the ego car.
  max_ego_perception_distance_ = pt.get<double>(TXSIM_CLOUD_CITY_CONFIG_KEY_PERCEPTION_RANGE);
  LOG(INFO) << "max perception range of the ego car: " << max_ego_perception_distance_;
  job_id_ = pt.get<int64_t>(TXSIM_CLOUD_CITY_CONFIG_KEY_JOB_ID);
  ego_id_ = pt.get<int64_t>(TXSIM_CLOUD_CITY_CONFIG_KEY_EGO_ID);
  LOG(INFO) << "job id: " << job_id_ << ", ego id: " << ego_id_;
  // cloud city scenario_path is empty,add egoid here
  ego_info.ego_vehicle_measurement.emplace_back();
  ego_info.ego_vehicle_measurement.back().first = ego_id_;
  LOG(INFO) << "ego_reset_params_.ego_vehicle_measurement size: " << ego_info.ego_vehicle_measurement.size();
  try {
    tinyxml2::XMLDocument doc;
    std::string egoConfigPath = "/home/sim_root/sim/data/Catalogs/Vehicles/default.xosc";
    const tinyxml2::XMLElement* catalog_root = tx_sim::utils::GetXMLRoot(egoConfigPath, doc);
    tx_sim::utils::ParseEgoMeasurement(egoConfigPath, kDefaultCatalogName.c_str(), kDefaultCatalogEntryName.c_str(),
                                       "suv", std::get<1>(ego_info.ego_vehicle_measurement.back()));
  } catch (const std::exception& e) {
    LOG(WARNING) << "cloud city service trying parsing the measurement info of the ego vehicle error: " << e.what();
  }
  LOG(INFO) << "ego_reset_params_.ego_vehicle_measurement size: " << ego_info.ego_vehicle_measurement.size();
  LOG(INFO) << "m_scene_pb size: " << ego_reset_params_.m_scene_pb.size();
  tx_sim::utils::ParseSimCloudCityVehicle(ego_reset_params_.m_scene_pb, "/home/sim_root/sim/sys/",
                                          "/home/sim_root/sim/data/");
  LOG(INFO) << "m_scene_pb size: " << ego_reset_params_.m_scene_pb.size();
}


/**
 * @brief check the reset status of the last reset.
 * @param[out] output_status the output status, whose err_code is set according to the last reset status.
 * @return true if the last reset is kCmdSucceed.
 */
bool SimCloudCityService::CheckResetStatus(ErrorStatus& output_status) {
  CmdErrorCode reset_ec = last_reset_result_.load(std::memory_order_acquire);
  switch (reset_ec) {
    case kCmdSucceed: return true;
    case kCmdAccepted: {
      VLOG(1) << step_log_prefix_ << " reseting ...";
      output_status.set_err_code(MODULE_STOP);
      return false;
    }
    default: {
      LOG(ERROR) << step_log_prefix_ << " last reset execution failed: " << Enum2String(reset_ec);
      output_status.set_err_code(ERROR_RESET);
      return false;
    }
  }
}


/**
 * @brief output the reseting messages.
 * @param[in] output the output message.
 * @param[in] input_traffic the input traffic.
 * @param[in] frame_id the frame id.
 * @param[in] begin_frame_id the begin frame id.
 */
void SimCloudCityService::OutputResetingMessages(StepOutput* output, const sim_msg::Traffic& input_traffic,
                                                 uint64_t frame_id, uint64_t begin_frame_id) {
  StepMessage step_msg;
  zmq::message_t filtered_tfc, loc;
  std::ostringstream ss;
  // fill location msg.
  UpdateOutputLocationFromLastOne(output->mutable_ego_location());
  size_t loc_msg_size = output->ego_location().ByteSizeLong();
  loc.rebuild(loc_msg_size);
  output->ego_location().SerializeToArray(loc.data(), loc.size());

  output->set_begin_frame_id(begin_frame_id);
  // step_msg.messages.emplace_back(tx_sim::topic::kLocation, std::move(loc));
  // ss << "(" << tx_sim::topic::kLocation << ": " << loc_msg_size << " bytes) ";
  // can't file start location to kafka write when reset,because this start location may cut in recode.
  //  fill traffic msg.
  filtered_tfc = GetTrafficPerceptionMsg(input_traffic, coordinator_->current_simulation_time());
  step_msg.messages.emplace_back(tx_sim::topic::kTraffic, std::move(filtered_tfc));
  ss << "(" << tx_sim::topic::kTraffic << ": " << filtered_tfc.size() << " bytes)";
  VLOG(1) << step_log_prefix_ << " written kafka with messages: [" << ss.str() << "]";
  topic_writer_->WritePb(step_msg, frame_id, 0, true, m_lastmileage, m_begin_frame_id);
}


/**
 * @brief get the traffic perception message.
 * @param[in] traffic the original traffic data.
 * @param[in] adjust_time the time to be adjusted.
 * @return the zmq message of filtered traffic.
 */
zmq::message_t SimCloudCityService::GetTrafficPerceptionMsg(const Traffic& traffic, uint64_t adjust_time) {
  const auto& origin = ego_reset_params_.map_local_origin;
  double ep[3] = {last_location_.position().x(), last_location_.position().y(), 0}, tp[3];
  VLOG(3) << step_log_prefix_ << " ego position(lonlat): (" << ep[0] << ", " << ep[1] << ").";
  coord_trans_api::lonlat2enu(ep[0], ep[1], ep[2], origin[0], origin[1], 0);
  VLOG(3) << step_log_prefix_ << " ego position(enu): (" << ep[0] << ", " << ep[1] << ").";
  Traffic ret_traffic;

// for convenience later, we use a macro definition to filter the traffic by field name.
#define TXSIM_FUNC_FILTER_TRAFFIC(field_name)                                                            \
  do {                                                                                                   \
    bool first_item_logged = false;                                                                      \
    std::stringstream ss;                                                                                \
    for (const auto& item : output_traffic.field_name()) {                                               \
      tp[0] = item.x(), tp[1] = item.y(), tp[2] = 0;                                                     \
      if (!first_item_logged) ss << "first item position(lonlat): (" << tp[0] << ", " << tp[1] << "), "; \
      coord_trans_api::lonlat2enu(tp[0], tp[1], tp[2], origin[0], origin[1], 0);                         \
      if (!first_item_logged) ss << "position(enu): (" << tp[0] << ", " << tp[1] << "), ";               \
      double dis = std::sqrt(std::pow(tp[0] - ep[0], 2) + std::pow(tp[1] - ep[1], 2));                   \
      if (dis <= max_ego_perception_distance_) ret_traffic.add_##field_name()->CopyFrom(item);           \
      if (!first_item_logged) {                                                                          \
        VLOG(3) << step_log_prefix_ << " " << ss.str() << " distance to ego car: " << dis << ".";        \
        first_item_logged = true;                                                                        \
      }                                                                                                  \
    }                                                                                                    \
  } while (0);

  sim_msg::Traffic output_traffic;
  // copy
  for (const auto& car : traffic.cars()) {
    if (car.id() == ego_id_) {
      VLOG(2) << step_log_prefix_ << " found identical traffic ego car: " << car.id() << " (" << car.x() << ", "
              << car.y() << ").";
      continue;
    }
    output_traffic.add_cars()->CopyFrom(car);
  }
  output_traffic.mutable_staticobstacles()->CopyFrom(traffic.staticobstacles());
  output_traffic.mutable_dynamicobstacles()->CopyFrom(traffic.dynamicobstacles());
  output_traffic.mutable_trafficlights()->CopyFrom(traffic.trafficlights());
  // filter and process
  TXSIM_FUNC_FILTER_TRAFFIC(cars);
  TXSIM_FUNC_FILTER_TRAFFIC(staticobstacles);
  TXSIM_FUNC_FILTER_TRAFFIC(dynamicobstacles);
  TXSIM_FUNC_FILTER_TRAFFIC(trafficlights);
  for (auto& item : *ret_traffic.mutable_cars()) item.set_t(adjust_time);
  for (auto& item : *ret_traffic.mutable_staticobstacles()) item.set_t(adjust_time);
  for (auto& item : *ret_traffic.mutable_dynamicobstacles()) item.set_t(adjust_time);

  VLOG(2) << step_log_prefix_ << " received traffic: " << traffic.cars_size() << " cars, "
          << traffic.staticobstacles_size() << " static obstacles, " << traffic.dynamicobstacles_size()
          << " dynamic obstacles and " << traffic.trafficlights_size()
          << " traffic lights. ===>>> after filtered: " << ret_traffic.cars_size() << " cars, "
          << ret_traffic.staticobstacles_size() << " static obstacles, " << ret_traffic.dynamicobstacles_size()
          << " dynamic obstacles and " << ret_traffic.trafficlights_size() << " traffic lights.";

  return Protobuf2ZmqMsg(ret_traffic);
}


/**
 * @brief update the last_location_ with the latest location.
 * @param ego_location The current location of the ego car.
 */
void SimCloudCityService::UpdateEgoLocation(const Location& ego_location) {
  last_location_.CopyFrom(ego_location);
}


/**
 * @brief update the location with the last_location_.
 * @param loc The location to be updated.
 */
void SimCloudCityService::UpdateOutputLocationFromLastOne(Location* loc) {
  loc->CopyFrom(last_location_);
}


/**
 * @brief check the step status
 * @param step_ec The step error code
 * @param output_status The output ErrorStatus status
 * @param frame_id The frame id
 */
void SimCloudCityService::CheckStepStatus(CmdErrorCode step_ec, ErrorStatus& output_status, uint64_t frame_id) {
  if (no_location_count_ >= 50) {
    if (step_ec == kCmdSucceed) step_ec = kCmdScenarioStopped;
    LOG(WARNING) << step_log_prefix_ << " there's no location message received in 200 continuous step frames.";
    no_location_count_ = 0;
  }
  switch (step_ec) {
    case kCmdSucceed: {
      output_status.set_err_code(SUCCESS);
      return;
    }
    case kCmdScenarioStopped: {
      last_reset_result_.store(kCmdAccepted, std::memory_order_release);
      output_status.set_err_code(MODULE_STOP);
      if (reset_thread_ && reset_thread_->joinable()) reset_thread_->join();
      last_location_.clear_velocity();  // ego car stopped(v = 0) when scenario stopped.
      m_stoping = true;
      m_lastmileage = m_curmileage;
      VLOG(0) << step_log_prefix_ << " scenario stopped. reseting ..., current frame_id: " << frame_id
              << " begin frame_id: " << m_begin_frame_id << " mileage: " << m_curmileage;
      reset_thread_.reset(new std::thread(&SimCloudCityService::ResetEgoCar, this, frame_id));
      return;
    }
    default: {
      LOG(ERROR) << step_log_prefix_ << " step execution failed: " << Enum2String(step_ec);
      output_status.set_err_code(ERROR_STEP);
      last_step_failed_.store(true, std::memory_order_release);
    }
  }
}


/**
 * @brief reset the ego route.
 * @param reset_params The module reset request params.
 */
void SimCloudCityService::ResetEgoRoute(ModuleResetRequest& reset_params) {
  reset_params.ego_infos[0].ego_path.clear();
  // set ego start location with the last_location_//wrong!!!!!!
  last_location_ = origin_start_location_;  // begin from startlocation;
  reset_params.ego_infos[0].ego_path.emplace_back(last_location_.position().x(), last_location_.position().y(),
                                                  last_location_.position().z());
  reset_params.ego_infos[0].ego_initial_velocity =
      std::sqrt(std::pow(last_location_.velocity().x(), 2) + std::pow(last_location_.velocity().y(), 2) +
                std::pow(last_location_.velocity().z(), 2));

  reset_params.ego_infos[0].ego_initial_theta = last_location_.rpy().z();
  // randomly picking an ego end point.
  std::uniform_int_distribution<> dis(0, end_points_.size() - 1);
  int rd_idx = dis(g_gen);
  if (rd_idx == last_end_idx_) rd_idx = (rd_idx + 1) % end_points_.size();
  last_end_idx_ = rd_idx;
  reset_params.ego_infos[0].ego_path.emplace_back(end_points_[rd_idx].first, end_points_[rd_idx].second, 0);
}


/**
 * @brief Reset the ego cat with frame_id. First kCmdPause, then kCmdSetup.
 * @param frame_id frame id to be reset.
 */
void SimCloudCityService::ResetEgoCar(uint64_t frame_id) {
  // pause the scenario.
  if (frame_id != 0) {
    VLOG(1) << "stop the ego car ...";
    CommandInfo stop_cmd(kCmdPause);  // there is no kCmdStop
    stop_cmd.status_cb = [this, frame_id](const CommandStatus& status) {
      if (status.ec == kCmdAccepted) return;
      for (const auto& mst : status.module_status)
        if (mst.ec != kModuleOK)
          LOG(ERROR) << "module " << mst.name << " error: (" << Enum2String(mst.ec) << ") " << mst.msg;
      // WriteGradingResult(status, frame_id);
    };
    CmdErrorCode stop_ec = coordinator_->Execute(stop_cmd);
    if (stop_ec != kCmdSucceed) {
      LOG(ERROR) << "stop ego car failed: " << Enum2String(stop_ec);
      last_reset_result_.store(stop_ec, std::memory_order_release);
      return;
    }
  }
  VLOG(1) << "ego car stopped.";

  // Setup the scenario.
  CommandInfo cmd_info(kCmdSetup);
  cmd_info.status_cb = [](const CommandStatus& status) {
    for (const auto& ist : status.init_status)
      VLOG(1) << "module " << ist.name << " init status: " << Enum2String(ist.state);
    for (const auto& mst : status.module_status)
      if (mst.ec != kModuleOK)
        LOG(ERROR) << "module " << mst.name << " error: (" << Enum2String(mst.ec) << ") " << mst.msg;
  };
  cmd_info.max_setup_monitoring_count = 600;  // approximately 10 min.
  cmd_info.extra_init_params.emplace_back(TXSIM_CLOUD_CITY_CONFIG_KEY_JOB_ID, std::to_string(job_id_));
  cmd_info.extra_init_params.emplace_back(TXSIM_CLOUD_CITY_CONFIG_KEY_EGO_ID, std::to_string(ego_id_));
  cmd_info.reset_params = ego_reset_params_;
  const auto& r = cmd_info.reset_params;

  CmdErrorCode ec = kCmdAccepted;
  for (size_t i = 0; i < TXSIM_CLOUD_CITY_RESET_MAX_TRY_TIMES; ++i) {
    ResetEgoRoute(cmd_info.reset_params);
    const auto& start_pos = r.ego_infos[0].ego_path[0];
    const auto& end_pos = r.ego_infos[0].ego_path[1];
    VLOG(0) << "[" << i << "] reseting ego car from (" << std::get<0>(start_pos) << ", " << std::get<1>(start_pos)
            << ") with v=" << r.ego_infos[0].ego_initial_velocity << " theta=" << r.ego_infos[0].ego_initial_theta
            << " to (" << std::get<0>(end_pos) << ", " << std::get<1>(end_pos) << ") ...";
    ec = coordinator_->Execute(cmd_info);
    if (ec == kCmdSucceed) {
      VLOG(0) << "ego car reset succeed.";
      break;
    } else {
      LOG(ERROR) << "ego car reset failed: " << Enum2String(ec)
                 << (i < TXSIM_CLOUD_CITY_RESET_MAX_TRY_TIMES - 1 ? ", try again ..." : "");
    }
  }
  m_begin_frame_id = frame_id;
  last_reset_result_.store(ec, std::memory_order_release);
}


/**
 * @brief Write grading result from status to kGrading topic.
 * @param status input status include grading feedback.
 * @param frame_id frame id.
 */
void SimCloudCityService::WriteGradingResult(const CommandStatus& status, uint64_t frame_id) {
  bool found_grading_feedback = false;
  for (const ModuleCmdStatus& m : status.module_status) {
    if (m.name == TXSIM_CLOUD_CITY_MODULE_NAME_GRADING && !m.feedback.empty()) {
      // topic_writer_->Write(m.feedback, frame_id, 0);////grading alway in trafficframe
      found_grading_feedback = true;
      break;
    } else if (m.name == TXSIM_CLOUD_CITY_MODULE_NAME_PLANNING && !ModuleResponseSucceed(m.ec)) {
      if (m.ec == kModuleTimeout || m.ec == kModuleError) {
        LOG(WARNING) << step_log_prefix_ << " sending grading algo kpi since planning error ...";
        Grading kpi_msg;
        kpi_msg.mutable_algorithm()->set_state(m.ec == kModuleTimeout ? GRADING_ALGORITHM_TIMEOUT
                                                                      : GRADING_ALGORITHM_THROWEXCEPTION);
        StepMessage m;
        m.messages.emplace_back(tx_sim::topic::kGrading, Protobuf2ZmqMsg(kpi_msg));
      }
    }
  }

  if (!found_grading_feedback)
    LOG(ERROR) << step_log_prefix_ << "no " << TXSIM_CLOUD_CITY_MODULE_NAME_GRADING << " feedback found.";
}


/**
 * @brief a function to print step time cost.
 * @param start_time starting timestamp of this step.
 * @param us1 user time of this step.
 * @param sy1 system time of this step.
 */
void SimCloudCityService::PrintStepTimeCost(int64_t start_time, uint64_t us1, uint64_t sy1) {
  int64_t spent_time = tx_sim::utils::GetCpuTimeInMillis() - start_time;
  uint64_t us2 = 0, sy2 = 0;
  std::tie(us2, sy2) = tx_sim::utils::GetProcessCpuTime();
  double cpu_us = (double)(us2 - us1) / 1000, cpu_sy = (double)(sy2 - sy1) / 1000;
  VLOG(1) << std::setprecision(3) << step_log_prefix_ << " total_step_time_cost: " << spent_time << "/" << cpu_us << "/"
          << cpu_sy << "ms.";
  LOG_IF(WARNING, spent_time > 100) << step_log_prefix_ << " total_step_time_cost_too_long: " << spent_time << "/"
                                    << cpu_us << "/" << cpu_sy << "ms.";
}

/******************************************************************************/
/*************************** main entry point *********************************/
/******************************************************************************/

/**
 * @brief parse command line arguments.
 * @param ac number of arguments.
 * @param av array of argument values.
 * @param root root directory of the cloud service.
 * @param endpoint the specified ip address or host name + port which the cloud service should listen on.
 * @return true if succeed, false otherwise.
 */
static bool InstallProgramOptions(int ac, char** av, std::string& root, std::string& endpoint) {
  prog_ops::options_description op_desc("Options");
  // clang-format off
  op_desc.add_options()
    ("help,h", "display help info")
    ("root,r", prog_ops::value<std::string>(&root)->default_value("/home/sim"),
      "the root directory of the cloud service in where contains configurations and data.")
    ("endpoint,p", prog_ops::value<std::string>(&endpoint)->default_value("0.0.0.0:8401"),
      "if specified, the cloud service will listening on the specified address. format: address:port")
    ("log-dir,l", prog_ops::value<std::string>(&FLAGS_log_dir)->default_value("/tmp"),
      "the directory in where to write the log files of the cloud service.")
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
  std::string app_root, endpoint;
  // parse command line arguments.
  if (!InstallProgramOptions(argc, argv, app_root, endpoint)) return 0;

  // set glog options.
  FLAGS_logbuflevel = -1;     // no buffer
  FLAGS_max_log_size = 10;    // maximum 10 mb log file size
  FLAGS_stderrthreshold = 4;  // no stderr output
  FLAGS_logbufsecs = 0;
  // google::EnableLogCleaner(3);  // auto-removes 3 days earlier log files
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  std::unique_ptr<grpc::Server> server = nullptr;
  std::unique_ptr<SimCloudCityService> service = nullptr;
  try {
    // sim service construct
    service.reset(new SimCloudCityService(app_root));
    // grpc server construct and start
    grpc::ServerBuilder builder;
    builder.AddListeningPort(endpoint, grpc::InsecureServerCredentials());
    builder.RegisterService(service.get());
    server = std::move(builder.BuildAndStart());
    // sim service setup
    service->SetupSimEnv();
  } catch (const std::exception& e) {
    LOG(ERROR) << "cloud city service initiation failed: " << e.what();
    return 1;
  }

  if (server) {
    LOG(INFO) << "sim cloud city service serving on " << endpoint << "...";
    server->Wait();  // block until exit.
  } else {
    LOG(ERROR) << "sim cloud city grpc server build/start failed.";
    return 1;
  }
  return 0;
}