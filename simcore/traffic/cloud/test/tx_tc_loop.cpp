// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_loop.h"
#include <glog/logging.h>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <memory>
#include "traffic.pb.h"
#include "tx_tc_test_gflags.h"

TXST_NAMESPACE_OPEN(Base)

class TimingCPU {
 private:
  std::chrono::time_point<std::chrono::system_clock> m_StartTime;

 public:
  TimingCPU() = default;
  ~TimingCPU() = default;
  TimingCPU(const TimingCPU&) = delete;
  TimingCPU& operator=(const TimingCPU&) = delete;

 public:
  void StartCounter() NOEXCEPT { m_StartTime = std::chrono::system_clock::now(); }
  int64_t GetElapsedMicroseconds() NOEXCEPT {
    std::chrono::time_point<std::chrono::system_clock> lastTime = m_StartTime;
    m_StartTime = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(m_StartTime - lastTime).count();
  }
  int64_t GetElapsedMilliseconds() NOEXCEPT {
    std::chrono::time_point<std::chrono::system_clock> lastTime = m_StartTime;
    m_StartTime = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_StartTime - lastTime).count();
  }

  static int64_t GetCurrentTimeCount() NOEXCEPT { return std::chrono::system_clock::now().time_since_epoch().count(); }
};  // TimingCPU class

TXST_NAMESPACE_CLOSE(Base)

TXST_NAMESPACE_OPEN(TrafficFlow)

#if __TX_Mark__("TrafficLoop4CloudDev")

void TrafficLoop4CloudDev::resetVars() NOEXCEPT {
  outputTraffic.Clear();
  m_bFirstFrame = true;
  step_count_ = 0;
  payload_.clear();
  m_lastStepTime = 0.0;
  m_passTime = 0.0;
}

TrafficLoop4CloudDev::TrafficLoop4CloudDev() {
  namespace boostfs = boost::filesystem;
  namespace boostdll = boost::dll;

  LOG(INFO) << "txSimTrafficCloud DLL/so testing ..." << std::endl;

  boostfs::path pluginPath;
  if (FLAGS_test_DLLPath.empty()) {
    pluginPath = boostfs::current_path() / boostfs::path("txMicroTrafficCloud");
    LOG(INFO) << "Load Plugin from " << pluginPath << std::endl;
  } else {
    boostfs::path spec_path = FLAGS_test_DLLPath;
    pluginPath = spec_path / boostfs::path("txMicroTrafficCloud");
    LOG(INFO) << "Load Plugin from special path : " << pluginPath << std::endl;
  }

  try {
    pluginCreator =
        boostdll::import_alias<PluginCreate>(pluginPath, "create_plugin", boostdll::load_mode::append_decorations);
    /* create the plugin */
    mPlugin = pluginCreator();
    if (mPlugin) {
      LOG(INFO) << "plugin class name : " << typeid(mPlugin).name() << std::endl;
      LOG(INFO) << "Plugin Name: " << mPlugin->module_name() << std::endl;
      LOG(INFO) << "Plugin Version = " << mPlugin->module_version() << std::endl;
    } else {
      LOG(WARNING) << "Plugin loading error. " << pluginPath;
    }
  } catch (const boost::system::system_error& err) {
    LOG(WARNING) << "Cannot load Plugin from " << pluginPath << std::endl;
    LOG(WARNING) << err.what() << std::endl;
    return;
  }
}

void TrafficLoop4CloudDev::Init(tx_sim::InitHelper& helper) {
  if (IsValid()) {
    LOG(INFO) << "call " << _FootPrint_;
    Base::txString value = helper.GetParameter("max_step_count");
    if (!value.empty()) {
      max_step_count_ = std::stoi(value);
      LOG(INFO) << _FootPrint_ << "Init with parameter max_step_count = " << max_step_count_ << std::endl;
    }

    helper.Subscribe(tx_sim::topic::kLocation);
    helper.Publish(tx_sim::topic::kTraffic);
  } else {
    LOG(WARNING) << "mPlugin is invalid.";
  }
}

void TrafficLoop4CloudDev::Reset(tx_sim::ResetHelper& helper) {
  if (IsValid()) {
    resetVars();
    m_start_location.ParseFromString(helper.ego_start_location());
    TrafficManager::InitInfo inParam;
    inParam.configfile_path = FLAGS_test_TrafficSystemOnCloudCfgPath;
    if (TrafficManager::ErrCode::trafficSucc == mPlugin->init(inParam)) {
      const auto temp_path = Base::txString("./vr/vehicle_behavior_db.cfg");
      LOG(INFO) << "TAD_Vehicle_Behavior config : " << temp_path;

      using namespace boost::filesystem;
      if (exists(boost::filesystem::path(temp_path))) {
        std::ifstream in_json(temp_path);
        in_json.rdbuf();
        std::stringstream ss;
        ss << in_json.rdbuf();
        mPlugin->set_vehicle_cfg(ss.str());
      } else {
        LOG(INFO) << "[vehicle_params][not find param file][use default value] " << temp_path;
      }
      LOG(INFO) << "mPlugin.init success.";
    } else {
      LOG(WARNING) << "mPlugin.init failure.";
    }
  } else {
    LOG(WARNING) << "mPlugin is invalid.";
  }
}

inline Base::txFloat MillisecondToSecond(Base::txFloat const _ms) { return _ms / 1000.0; }

void TrafficLoop4CloudDev::Step(tx_sim::StepHelper& helper) {
  const int ego_id = 1;
  double ego_dist = FLAGS_test_CloudEgoVisionRadius;

  if (IsValid()) {
    const auto timeStamp = helper.timestamp();

    Base::txFloat const relativeTime = MillisecondToSecond(timeStamp - m_lastStepTime);
    m_passTime += relativeTime;
    const auto absTime = MillisecondToSecond(timeStamp);
    step_count_++;

    if ((static_cast<int>(absTime)) % 2 == 0) {
      ego_dist *= 10.0;
    }

    sim_msg::Location pbRouteInfo;
    if (m_bFirstFrame) {
      pbRouteInfo.CopyFrom(m_start_location);
      payload_.clear();
      pbRouteInfo.SerializeToString(&payload_);
      helper.PublishMessage(tx_sim::topic::kLocation, payload_);
      m_bFirstFrame = false;
      if (TrafficManager::ErrCode::trafficSucc == (mPlugin->addEgo(ego_id, pbRouteInfo))) {
        if (TrafficManager::ErrCode::trafficSucc == (mPlugin->step(absTime))) {
          SendTraffic(helper, ego_id, ego_dist);
          LOG_IF(INFO, FLAGS_LogLevel_DLLTest) << "cloud system step success.";
        } else {
          LOG(WARNING) << "cloud system step failure.";
        }
      } else {
        LOG(WARNING) << "add ego failure.";
      }
    } else {
      helper.GetSubscribedMessage(tx_sim::topic::kLocation, payload_);
      pbRouteInfo.ParseFromString(payload_);
      const Base::txInt switch_step = -2;

      if (TrafficManager::ErrCode::trafficSucc == (mPlugin->updateEgoInfo(1, pbRouteInfo)) || __cloud_ditw__) {
        Base::TimingCPU timer;
        timer.StartCounter();
        if (TrafficManager::ErrCode::trafficSucc == (mPlugin->step(absTime))) {
          SendTraffic(helper, ego_id, ego_dist);
#  if __cloud_ditw__
          if (step_count_ == switch_step) {
            sim_msg::TrafficRecords status_record;
            std::ifstream in_json("./event.json");
            std::stringstream ss;
            ss << in_json.rdbuf();

            mPlugin->initialize_l2w_sim(status_record, ss.str());
          }
#  endif /*__cloud_ditw__*/
          LOG_IF(INFO, FLAGS_LogLevel_DLLTest) << "cloud system step success.";
        } else {
          LOG(WARNING) << "cloud system step failure.";
        }
        LOG(INFO) << "cost time : " << timer.GetElapsedMilliseconds();
      } else {
        LOG(WARNING) << "updateEgoInfo failure.";
      }
    }
    payload_.clear();
    m_lastStepTime = timeStamp;
  }
}

void TrafficLoop4CloudDev::Stop(tx_sim::StopHelper& helper) {
  LOG(INFO) << "call " << _FootPrint_;
  helper.set_feedback("stepCounts", std::to_string(step_count_));
}

void TrafficLoop4CloudDev::SendTraffic(tx_sim::StepHelper& helper, const int ego_id, const double ego_dist) NOEXCEPT {
  if (IsValid()) {
    outputTraffic.Clear();
    mPlugin->getTrafficInfoByEgo(ego_id, ego_dist, outputTraffic);
    payload_.clear();
    outputTraffic.SerializeToString(&payload_);
    helper.PublishMessage(tx_sim::topic::kTraffic, payload_);
  }
}

void TrafficLoop4CloudDev::SendTraffic(tx_sim::StepHelper& helper, sim_msg::MapPosition pos) NOEXCEPT {
  if (IsValid()) {
    sim_msg::MergeTrafficInfo tmpTraffic;
    mPlugin->getTrafficInfoBySurroundingMapPosition(pos, tmpTraffic);
    payload_.clear();
    tmpTraffic.traffic().SerializeToString(&payload_);
    helper.PublishMessage(tx_sim::topic::kTraffic, payload_);
  }
}

Base::txBool TrafficLoop4CloudDev::GetRoutingInfo(tx_sim::ResetHelper& helper,
                                                  sim_msg::Location& refEgoData) const NOEXCEPT {
  refEgoData.ParseFromString(helper.ego_start_location());
  return true;
  refEgoData.Clear();
  sim_msg::Vec3* curPos = refEgoData.mutable_position();
  sim_msg::Vec3* curRpy = refEgoData.mutable_rpy();
  sim_msg::Vec3* curV = refEgoData.mutable_velocity();

  const double fAngle = 0; /*sim.simulation.planner.theta*/
  const double lon = 0;    /*sim.simulation.planner.route.start*/
  const double lat = 23.31457674;

  const double velocity = 0.0; /*sim.simulation.planner.start_v*/
  curPos->set_x(lon);
  curPos->set_y(lat);
  curPos->set_z(0);
  curRpy->set_x(0);
  curRpy->set_y(0);
  curRpy->set_z(fAngle);
  curV->set_x(velocity * std::cos(fAngle));
  curV->set_y(velocity * std::sin(fAngle));
  curV->set_z(0);
  return true;
}

#endif /*__TX_Mark__("TrafficLoop4CloudDev")*/

TXST_NAMESPACE_CLOSE(TrafficFlow)

TXSIM_MODULE(TrafficFlow::TrafficLoop4CloudDev)
