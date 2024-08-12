// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "log2world_template.h"
#include "tad_scene_loader_factory.h"
#include "log2world_trigger.pb.h"
#include "tx_header.h"
#include "tx_spatial_query.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_L2W)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)

void Log2WorldSimLoop::Init(tx_sim::InitHelper& helper) TX_NOEXCEPT {
  ParentClass::Init(helper);
  helper.Subscribe(FLAGS_L2W_Switch_TopicName);
  helper.Publish(FLAGS_L2W_RawLocation_TopicName /*tx_sim::topic::kLocation*/);
  helper.Publish(FLAGS_L2W_RawTrajectory_TopicName /*tx_sim::topic::kTrajectory*/);
  helper.Publish(FLAGS_L2W_RawTraffic_TopicName);
}

void Log2WorldSimLoop::CreateSystem() TX_NOEXCEPT {
  m_TrafficElementSystem_Ptr = m_l2w_system_ptr = std::make_shared<Log2WorldSystem>();
  if (NonNull_Pointer(TrafficSystemPtr()) && NonNull_Pointer(Log2WorldTrafficSystemPtr())) {
    LogInfo << "Create Log2WorldSystem Success.";
  } else {
    LogWarn << "Create Log2WorldSystem Failure.";
  }
}

void Log2WorldSimLoop::ResetVars() TX_NOEXCEPT {
  m_l2w_status = L2W_Status::eLogsim;
  ParentClass::ResetVars();
}

Log2WorldSimLoop::txBool Log2WorldSimLoop::Debug_L2W_Switch_SpecialTime(const Base::TimeParamManager& timeMgr) const
    TX_NOEXCEPT {
  if ((m_l2w_status == L2W_Status::eLogsim) && (FLAGS_L2W_Switch_SpecialTime >= 0) &&
      (FLAGS_L2W_Switch_SpecialTime <= timeMgr.PassTime())) {
    return true;
  } else {
    return false;
  }
}

Base::ISceneLoaderPtr Log2WorldSimLoop::CreateSceneLoader(const Base::txString strSceneType) const TX_NOEXCEPT {
  Scene::TAD_SceneLoaderFactory loaderFactory(strSceneType);
  return loaderFactory.CreateSceneLoader();
}

Log2WorldSimLoop::txBool Log2WorldSimLoop::NeedTriggerWorldSim(tx_sim::StepHelper& helper,
                                                               const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  if (IsSpecialTimeSwitch()) {
    /*LOG(INFO) << "do Debug_L2W_Switch_SpecialTime";*/
    return Debug_L2W_Switch_SpecialTime(timeMgr);
  } else {
    Base::txString payload_l2w_switch;
    helper.GetSubscribedMessage(FLAGS_L2W_Switch_TopicName, payload_l2w_switch);
    if (CallFail(payload_l2w_switch.empty())) {
      sim_msg::Log2worldTrigger trigger;
      trigger.ParseFromString(payload_l2w_switch);

      if (sim_msg::Log2worldTriggerType::LOG2WORLD_TRIGGER_NONE != trigger.type()) {
        LOG(WARNING) << TX_VARS_NAME(from_cmd, m_l2w_trigger_cmd) << TX_VARS_NAME(to_cmd, trigger.type())
                     << TX_VARS_NAME(at, timeMgr);
        m_l2w_trigger_cmd = trigger.type();
      }

      if (sim_msg::Log2worldTriggerType::LOG2WORLD_TRIGGER_TRAFFIC == trigger.type() ||
          sim_msg::Log2worldTriggerType::LOG2WORLD_TRIGGER_BOTH == trigger.type()) {
        LOG(INFO) << "do l2w trigger. " << TX_VARS(trigger.type()) << TX_VARS(trigger.trigger_time());
        return true;
      } else {
        return false;
      }
    } else {
      LogInfo << "payload_l2w_switch.empty() = true";
      return false;
    }
  }
}

void Log2WorldSimLoop::PreSimulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  if (IsLogSim() && NeedTriggerWorldSim(helper, timeMgr)) {
    m_l2w_status = eWorldsim;
    LOG(WARNING) << "**** switch logsim to worldsim, at " << timeMgr.PassTime();
    /*Log2WorldTrafficSystemPtr()->EnableDynamicChangeScene();*/
    Log2WorldTrafficSystemPtr()->SwitchSim(timeMgr);
  }
}

void Log2WorldSimLoop::PostSimulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(Log2WorldTrafficSystemPtr()) &&
      NonNull_Pointer(Log2WorldTrafficSystemPtr()->SimrecSceneLoader())) {
    txBool needStop = false;
    const txFloat abs_time_ms = Utils::SecondToMillisecond(timeMgr.AbsTime());
    const txFloat scene_start_time_ms = Log2WorldTrafficSystemPtr()->SimrecSceneLoader()->SimrecStartTime();
    const txFloat traffic_end_time_ms =
        Log2WorldTrafficSystemPtr()->SimrecSceneLoader()->SimrecTrafficEndTime() - scene_start_time_ms;
    const txFloat ego_end_time_ms =
        Log2WorldTrafficSystemPtr()->SimrecSceneLoader()->SimrecEgoEndTime() - scene_start_time_ms;
    if (sim_msg::Log2worldTriggerType::LOG2WORLD_TRIGGER_NONE == m_l2w_trigger_cmd ||
        sim_msg::Log2worldTriggerType::LOG2WORLD_TRIGGER_TRAFFIC == m_l2w_trigger_cmd) {
      if (abs_time_ms >= ego_end_time_ms) {
        needStop = true;
      }
    } else if (sim_msg::Log2worldTriggerType::LOG2WORLD_TRIGGER_EGO == m_l2w_trigger_cmd) {
      if (abs_time_ms >= traffic_end_time_ms) {
        needStop = true;
      }
    } else if (sim_msg::Log2worldTriggerType::LOG2WORLD_TRIGGER_BOTH == m_l2w_trigger_cmd) {
    } else {
      LogWarn << "unknown cmd type :" << TX_VARS(m_l2w_trigger_cmd);
      needStop = true;
    }

    if (CallSucc(needStop)) {
      LogWarn << "StopScenario " << TX_VARS(m_l2w_trigger_cmd) << TX_VARS(abs_time_ms) << TX_VARS(ego_end_time_ms)
              << TX_VARS(traffic_end_time_ms);
      helper.StopScenario("location play finished");
    }
  }
}

void Log2WorldSimLoop::ReceiveEgoInfo(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  const auto sEgoType = TrafficSystemPtr()->GetEgoType();
  if (IsLogSim()) {
    Log2WorldTrafficSystemPtr()->UpdatePlanningCarData(timeMgr, Base::Enums::EgoSubType::eLeader);
    if (_plus_(Base::ISceneLoader::EgoType::eTruck) == sEgoType) {
      Log2WorldTrafficSystemPtr()->UpdatePlanningCarData(timeMgr, Base::Enums::EgoSubType::eFollower);
    }
  } else if (IsWorldSim()) {
    ParentClass::ReceiveEgoInfo(helper, timeMgr);
  }
}

void Log2WorldSimLoop::SimulationTraffic(tx_sim::StepHelper& helper,
                                         const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  const Base::txBool res = TrafficSystemPtr()->Update(timeMgr);
  m_outputTraffic.Clear();
  TrafficSystemPtr()->FillingTrafficData(timeMgr, m_outputTraffic);
  TrafficSystemPtr()->FillingSpatialQuery();
  PublishMessage(helper, tx_sim::topic::kTraffic, m_outputTraffic);
#if 1
  std::ostringstream oss;
  if (FLAGS_LogLevel_L2W && m_outputTraffic.cars_size() > 0) {
    oss << "{" << TX_VARS_NAME(car_lon, m_outputTraffic.cars(0).x())
        << TX_VARS_NAME(car_lat, m_outputTraffic.cars(0).y()) << "}";
  }
  LogInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS_NAME(TopicName, tx_sim::topic::kTraffic)
          << TX_VARS_NAME(sample, oss.str()) << TX_VARS_NAME(send_note, m_outputTraffic.DebugString());
  {
    Log2WorldTrafficSystemPtr()->FillingTrafficDataShadow(timeMgr, m_output_ShadowTraffic);
    PublishMessage(helper, FLAGS_L2W_RawTraffic_TopicName, m_output_ShadowTraffic);
    oss.str("");
    if (FLAGS_LogLevel_L2W && m_output_ShadowTraffic.cars_size() > 0) {
      oss << "{" << TX_VARS_NAME(car_lon, m_output_ShadowTraffic.cars(0).x())
          << TX_VARS_NAME(car_lat, m_output_ShadowTraffic.cars(0).y()) << "}";
    }
    LogInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS_NAME(TopicName, FLAGS_L2W_RawTraffic_TopicName)
            << TX_VARS_NAME(sample, oss.str()) << TX_VARS_NAME(send_note, m_output_ShadowTraffic.DebugString());
  }

  {
    sim_msg::Location cur_ego_location;
    if (NonNull_Pointer(Log2WorldTrafficSystemPtr()) &&
        NonNull_Pointer(Log2WorldTrafficSystemPtr()->SimrecSceneLoader()) &&
        CallSucc(Log2WorldTrafficSystemPtr()->SimrecSceneLoader()->InterpEgoLocation(timeMgr, cur_ego_location))) {
      cur_ego_location.set_t(Utils::MillisecondToSecond(cur_ego_location.t()));
      const Base::txFloat ego_t_s = cur_ego_location.t();
      const txFloat scene_start_time_ms = Log2WorldTrafficSystemPtr()->SimrecSceneLoader()->SimrecStartTime();
      cur_ego_location.set_t(ego_t_s + Utils::MillisecondToSecond(scene_start_time_ms));
      // LogWarn << TX_VARS(cur_ego_location.t());
      PublishMessage(helper, FLAGS_L2W_RawLocation_TopicName, cur_ego_location);
      LogInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS_NAME(TopicName, FLAGS_L2W_RawLocation_TopicName)
              << TX_VARS_NAME(Lon, cur_ego_location.position().x())
              << TX_VARS_NAME(Lat, cur_ego_location.position().y())
              << TX_VARS_NAME(send_note, cur_ego_location.DebugString());
      sim_msg::Trajectory cur_traj =
          Log2WorldTrafficSystemPtr()->SimrecSceneLoader()->GetTrajectory(cur_ego_location.t());
      PublishMessage(helper, FLAGS_L2W_RawTrajectory_TopicName, cur_traj);
      LogInfo << TX_VARS(timeMgr.PassTime()) << TX_VARS_NAME(TopicName, FLAGS_L2W_RawTrajectory_TopicName)
              << TX_VARS_NAME(send_note, cur_traj.DebugString());
    } else {
      LogInfo << TX_VARS(timeMgr.PassTime()) << " log data play finish.";
    }
  }

#endif
  m_lastStepTime = timeMgr.TimeStamp();
  TrafficSystemPtr()->CheckSimulationResult(m_outputTraffic);
}

Base::txBool Log2WorldSimLoop::IsSimModeValid(const Base::txString strScenePath) const TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  std::string strExt = FilePath(strScenePath).extension().string();
  if (boost::iequals(strExt, FLAGS_L2W_Extension)) {
    return true;
  } else {
    LOG(WARNING) << "[Error] SimMode is logsim, " << TX_VARS(strExt) << TX_VARS(FLAGS_L2W_Extension);
    return false;
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)
