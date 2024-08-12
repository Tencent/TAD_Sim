// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_cloud_stand_alone_template.h"
#include "HdMap/tx_hashed_road.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "tx_application.h"
#include "tx_cloud_stand_alone_event_dispatcher.h"
#include "tx_path_utils.h"
#include "tx_spatial_query.h"
#include "tx_time_utils.h"
#include "tx_timer_on_cpu.h"
#include "tx_tc_cloud_loader.h"
#include "tx_tc_gflags.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Cloud)
#define SceneLoaderLogInfo LOG_IF(INFO, FLAGS_LogLevel_Cloud)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)

Base::TimeParamManager CloudStandAloneTemplate::MakeTimeMgr(const Base::txFloat time_stamp) TX_NOEXCEPT {
  const Base::txFloat relativeTimeInSec = Utils::MillisecondToSecond(time_stamp - m_lastStepTime);
  m_passTime += relativeTimeInSec;
  m_cloud_time_mgr =
      Base::TimeParamManager(Utils::MillisecondToSecond(time_stamp), relativeTimeInSec, m_passTime, time_stamp);
  // LOG(WARNING) << "[simulation_time] : " << timeMgr.str() << TX_VARS(time_stamp);
  cur_AbsTime = m_cloud_time_mgr.AbsTime();
  return GetTimeMgr();
}

void CloudStandAloneTemplate::ParseInitParameter(tx_sim::InitHelper& helper) TX_NOEXCEPT {}

void CloudStandAloneTemplate::ResetVars() TX_NOEXCEPT {
  ResetVarsPlugin();
  TrafficFlow::InitializeCloudEventDispatcher();
}

void CloudStandAloneTemplate::Init(tx_sim::InitHelper& helper) TX_NOEXCEPT {
  LogInfo << "call " << __func__;
  ParseInitParameter(helper);
  helper.Subscribe(FLAGS_Location_TopicName /*tx_sim::topic::kLocation*/);
  helper.Publish(FLAGS_Traffic_TopicName /*tx_sim::topic::kTraffic*/);
}

void CloudStandAloneTemplate::Reset(tx_sim::ResetHelper& helper) TX_NOEXCEPT { CreateEnv(helper); }

Base::txBool ParseMapInitParams(tx_sim::ResetHelper& helper,
                                HdMap::HadmapCacheConCurrent::InitParams_t& refParams) TX_NOEXCEPT {
  refParams.strTrafficFilePath = helper.scenario_file_path();
  refParams.strHdMapFilePath = helper.map_file_path();
  __Lon__(refParams.SceneOriginGPS) = helper.map_local_origin().x;
  __Lat__(refParams.SceneOriginGPS) = helper.map_local_origin().y;
  __Alt__(refParams.SceneOriginGPS) = helper.map_local_origin().z;

  {
    Base::txString app_path;
    Utils::GetAppPath("", app_path);
    const Base::txString cfg_path = app_path + "/HadmapFilter.cfg";
    refParams.strHadmapFilter = cfg_path;
  }
  LogInfo << "Map File :" << refParams.strHdMapFilePath << TX_VARS(Utils::ToString(refParams.SceneOriginGPS))
          << TX_VARS(refParams.strHadmapFilter);

  return true;
}

Base::txString Sim2Traffic(const Base::txString& sim_path) TX_NOEXCEPT {
  using namespace boost::filesystem;
  Utils::FilePath sim_abs_path = Utils::FilePath(sim_path);
  Utils::FilePath path_CurrentPath = sim_abs_path.parent_path();
  const auto sim_without_ext = sim_abs_path.filename().stem().string();
  path_CurrentPath.append(sim_without_ext + "_traffic.xml");
  LogWarn << TX_VARS(sim_path) << TX_VARS(path_CurrentPath);
  return path_CurrentPath.string();
}

void CloudStandAloneTemplate::CreateEnv(tx_sim::ResetHelper& helper) TX_NOEXCEPT {
  LogInfo << "call " << __func__;
  std::vector<std::pair<int64_t, std::string>> measurements;
  helper.vehicle_measurements(measurements);
  if (1 != measurements.size()) {
    LogWarn << "helper.vehicle_measurements error. " << TX_VARS(measurements.size());
    return;
  } else {
    vehGeomList.ParseFromString(measurements.front().second);
    LOG(INFO) << vehGeomList.ShortDebugString();
  }

  HdMap::HadmapCacheConCurrent::InitParams_t initParams;
  ParseMapInitParams(helper, initParams);
  inputeParams.CreateInfoFromSim(Sim2Traffic(initParams.strTrafficFilePath), initParams.SceneOriginGPS,
                                 initParams.strHdMapFilePath);

  sim_msg::Location loc;
  loc.ParseFromString(helper.ego_start_location());
  LOG(INFO) << TX_VARS(helper.ego_start_location().size());
  PluginTemplate::CreateEnv(FLAGS_SimulateWithPnC, loc);
}

void CloudStandAloneTemplate::Step(tx_sim::StepHelper& helper) TX_NOEXCEPT {
  static Base::TimingCPU s_timer;
  static Base::txInt sFrameCnt = 0;
  s_timer.StartCounter();
  const Base::TimeParamManager timeMgr = MakeTimeMgr(helper.timestamp());
  timeMgr.str();
  Simulation(helper, timeMgr);

  LOG_IF(INFO, (((sFrameCnt++) > FLAGS_tc_show_statictis_info_interval) ? (sFrameCnt = 0, true) : (false)))
      << "TrafficLoop::Step end. " << TX_VARS_NAME(timer, s_timer.GetElapsedMicroseconds() / 1000.0);
}

void CloudStandAloneTemplate::Simulation(tx_sim::StepHelper& helper,
                                         const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  ReceiveEgoInfo(helper, timeMgr);
  PluginTemplate::Simulation(timeMgr);
  CloudTrafficSystemPtr()->FillingTrafficData(timeMgr, m_outputTraffic);
  PublishMessage(helper, FLAGS_Traffic_TopicName, m_outputTraffic);
}

void CloudStandAloneTemplate::ReceiveEgoInfo(tx_sim::StepHelper& helper,
                                             const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  const auto sEgoType = TrafficSystemPtr()->GetEgoType();
  GetSubscribedMessage(helper, tx_sim::topic::kLocation, m_payload_);
  TrafficSystemPtr()->UpdatePlanningCarData(timeMgr, Base::Enums::EgoSubType::eLeader, m_payload_);
  if (_plus_(Base::ISceneLoader::EgoType::eTruck) == sEgoType) {
    GetSubscribedMessage(helper, FLAGS_TrailerEgo_TopicName, m_payload_trailer_);
    TrafficSystemPtr()->UpdatePlanningCarData(timeMgr, Base::Enums::EgoSubType::eFollower, m_payload_trailer_);
  }

  m_payload_.clear();
  m_payload_trailer_.clear();
}

void CloudStandAloneTemplate::Stop(tx_sim::StopHelper& helper) TX_NOEXCEPT {
  LogInfo << "call ";
  helper.set_feedback("stepCounts", std::to_string(m_step_count_));
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef SceneLoaderLogInfo
#undef LogWarn
