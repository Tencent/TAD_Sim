// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tadsim_loop.h"
#include "logsim_sim_template.h"
#include "tx_header.h"
#include "tx_logger.h"
#include "tx_tadsim_flags.h"
#include "worldsim_sim_template.h"
#if Use_ManualVehicle
#  include "tad_manual_vehicle_template.h"
#endif /*Use_ManualVehicle*/
#include "log2world_template.h"
#include "tx_timer_on_cpu.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_TADSim_Loop)
#define LogWarn LOG(WARNING)
#define UnifiedTraffic 1

TX_NAMESPACE_OPEN(TrafficFlow)

void ParseInitParameter(tx_sim::InitHelper& helper) TX_NOEXCEPT {
  if (Utils::Str2Type(helper.GetParameter("EnableTrafficVisionFilter"), FLAGS_EnableTrafficVisionFilter)) {
    LogInfo << "[Input_Parameter] " << TX_FLAGS(EnableTrafficVisionFilter);
  }

  if (Utils::Str2Type(helper.GetParameter("TrafficVisionFilterRadius"), FLAGS_TrafficVisionFilterRadius)) {
    LogInfo << "[Input_Parameter] " << TX_FLAGS(TrafficVisionFilterRadius);
  }

  if (Utils::Str2Type(helper.GetParameter("TrafficVisionFilterAltitudeDiff"), FLAGS_TrafficVisionFilterAltitudeDiff)) {
    LogInfo << "[Input_Parameter] " << TX_FLAGS(TrafficVisionFilterAltitudeDiff);
  }

  LogInfo << "[Input_Parameter][inject_vehicle_behavior_db] start.";
  FLAGS_inject_vehicle_behavior_db = helper.GetParameter(FLAGS_params_vehicle_behavior_db);
  LogInfo << "[Input_Parameter][inject_vehicle_behavior_db] end." << TX_VARS(FLAGS_inject_vehicle_behavior_db.size())
          << TX_VARS(FLAGS_inject_vehicle_behavior_db);
}

void Communicate(tx_sim::InitHelper& helper) TX_NOEXCEPT {
  // eWorldSim
  {
    ParseInitParameter(helper);
    const Base::txString value = helper.GetParameter("max_step_count");
    if (!value.empty()) {
      FLAGS_max_step_count = std::stoi(value);
      LogWarn << "Init with parameter max_step_count = " << FLAGS_max_step_count << std::endl;
    }

    helper.Subscribe(FLAGS_Location_TopicName /*LOCATION*/);
#if USE_EgoGroup
    helper.Subscribe(FLAGS_EgoUnion_TopicName /*EgoUnion/LOCATION*/);
    helper.Subscribe(FLAGS_EgoUnion_Trailer_TopicName /*EgoUnion/LOCATION_TRAILER*/);
    helper.Subscribe(FLAGS_EgoUnion_Specified_TopicName /*.hightlight_group*/);
#endif
    // publish our topics with messages we produced in Step callback.
    helper.Publish(FLAGS_Traffic_TopicName /*TRAFFIC*/);
#if Use_TrailerEgo
    helper.Subscribe(FLAGS_TrailerEgo_TopicName /*LOCATION_TRAILER*/);
#endif /*Use_TrailerEgo*/
  }

  // eL2wDesktop
  helper.Subscribe(FLAGS_L2W_Switch_TopicName /*.log2world_trigger*/);
  helper.Publish(FLAGS_L2W_RawLocation_TopicName /*LOCATION_REPLAY*/);
  helper.Publish(FLAGS_L2W_RawTrajectory_TopicName /*TRAJECTORY_REPLAY*/);
  helper.Publish(FLAGS_L2W_RawTraffic_TopicName /*TRAFFIC_REPLAY*/);

  // eManualVehicle
  helper.Subscribe(FLAGS_ManualVehicleTopicName /*TX_Manual_Vehicle*/);
}

void TADSimLoop::Init(tx_sim::InitHelper& helper) {
  LOG(INFO) << TX_VARS_NAME(sim_mode, helper.GetParameter("sim_mode"))
            << TX_VARS_NAME(_sim_mode, helper.GetParameter("_sim_mode"));

#if UnifiedTraffic
  LogInfo << "call " << __func__;
  Communicate(helper);

  m_Param_SimMode = helper.GetParameter("sim_mode");
  if ("logsim" == m_Param_SimMode) {
    m_SimMode = SimulationMode::eLogSim;
  } else if ("worldsim" == m_Param_SimMode) {
    m_SimMode = SimulationMode::eWorldSim;

    if (FLAGS_ManualVehicle) {
      m_SimMode = SimulationMode::eManualVehicle;
    }
  } else if ("xosc_replay" == m_Param_SimMode) {
    m_SimMode = SimulationMode::eXoscReplay;
  } else if ("manual_vehicle" == m_Param_SimMode) {
    m_SimMode = SimulationMode::eManualVehicle;
  } else if ("l2w_desktop" == m_Param_SimMode) {
    m_SimMode = SimulationMode::eL2wDesktop;
  }

#else
  m_SimMode = SimulationMode::eWorldSim;
  // l2w仿真模式
  if ("l2w" == helper.GetParameter("_sim_mode")) {
    m_SimMode = SimulationMode::eL2wDesktop;
  } else if ("logsim" == helper.GetParameter("_sim_mode")) {
    // logsim仿真模式
    m_SimMode = SimulationMode::eLogSim;
  } else {
    if ("logsim" == helper.GetParameter("sim_mode")) {
      m_SimMode = SimulationMode::eLogSim;
    } else if ("worldsim" == helper.GetParameter("sim_mode")) {
      // worldsim仿真模式
      m_SimMode = SimulationMode::eWorldSim;
      if (FLAGS_ManualVehicle) {
        m_SimMode = SimulationMode::eManualVehicle;
      }
    } else if ("xosc_replay" == helper.GetParameter("sim_mode")) {
      // xosc回放
      m_SimMode = SimulationMode::eXoscReplay;
    } else if ("manual_vehicle" == helper.GetParameter("sim_mode")) {
      m_SimMode = SimulationMode::eManualVehicle;
    } else if ("l2w_desktop" == helper.GetParameter("sim_mode")) {
      m_SimMode = SimulationMode::eL2wDesktop;
    }
  }

  m_SimPtr = nullptr;
  // 根据仿真模式选择创建具体实例
  switch (m_SimMode) {
    case SimulationMode::eLogSim: {
      m_SimPtr = std::make_shared<InterpolatePlayerModule>();
      break;
    }
    case SimulationMode::eWorldSim: {
      m_SimPtr = std::make_shared<WorldsimSimLoop>();
      break;
    }
    case SimulationMode::eManualVehicle: {
      m_SimPtr = std::make_shared<TAD_ManualVehicleSimLoop>();
      break;
    }
    case SimulationMode::eL2wDesktop: {
      m_SimPtr = std::make_shared<Log2WorldSimLoop>();
      break;
    }
  }

  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Init(helper);
  } else {
    LOG(ERROR) << "UnSupport Simulation Mode.";
  }
#endif

  // 若参数random_seed为true
  if (_NonEmpty_(helper.GetParameter("random_seed")) &&
      boost::equal(std::string("true"), helper.GetParameter("random_seed"))) {
    const auto now_count = Base::TimingCPU::GetCurrentTimeCount();
    const Base::txInt _seed = now_count % 100;
    // 设置随机种子
    Base::ISceneLoader::SetSelfDefineRandomSeed(true, _seed);
    LOG(INFO) << "[Seed]" << TX_VARS(now_count) << TX_VARS(_seed);
  } else {
    Base::ISceneLoader::SetSelfDefineRandomSeed(false, 0);
    LOG(INFO) << "[Seed] using seed from scene cfg." << TX_VARS(helper.GetParameter("random_seed"));
  }

#if __education_version__
  // 从helper中获取名为"rear_comfort"的参数值，并将其赋值给param_rear_comfort
  const std::string param_rear_comfort = helper.GetParameter("rear_comfort");
  if (_NonEmpty_(param_rear_comfort) && std::string("false") == param_rear_comfort) {
    FLAGS_EnableRearComfortGap = false;
  }
  LOG(INFO) << "[Rear_Comfort_Gap]" << TX_VARS(FLAGS_EnableRearComfortGap) << TX_VARS(param_rear_comfort)
            << TX_COND(_NonEmpty_(param_rear_comfort)) << TX_COND(std::string("false") == param_rear_comfort);
#endif /*__education_version__*/

#if __TX_Mark__("YD")
  const std::string strVehicleOutputTrajectory = helper.GetParameter("VehicleOutputTrajectory");
  if (_NonEmpty_(strVehicleOutputTrajectory) && std::string("true") == strVehicleOutputTrajectory) {
    FLAGS_VehicleOutputTrajectory = true;
  }
  LOG(INFO) << "[VehicleOutputTrajectory]" << TX_COND(FLAGS_VehicleOutputTrajectory)
            << TX_VARS(strVehicleOutputTrajectory);
#endif /*__TX_Mark__("YD")*/
}

void TADSimLoop::Reset(tx_sim::ResetHelper& helper) {
#if UnifiedTraffic
  m_SimPtr = nullptr;

  Base::txString settingStr = helper.setting_pb();
  sim_msg::Setting _setting;
  _setting.ParseFromArray(settingStr.c_str(), settingStr.size());
  LOG(INFO) << TX_VARS(_setting.sim_mode().DebugString());

  if (m_Param_SimMode.empty() &&
      (sim_msg::Setting_SimMode_SimModeType_UNKNOWN != _setting.sim_mode().sim_mode_type())) {
    if (sim_msg::Setting_SimMode_SimModeType_LOG2WORLD == _setting.sim_mode().sim_mode_type()) {
      m_SimMode = SimulationMode::eL2wDesktop;
      LOG(INFO) << TX_VARS_NAME(sim_mode, "l2w");
    } else if (sim_msg::Setting_SimMode_SimModeType_LOGSIM == _setting.sim_mode().sim_mode_type()) {
      m_SimMode = SimulationMode::eLogSim;
      LOG(INFO) << TX_VARS_NAME(sim_mode, "logsim");
    } else if (sim_msg::Setting_SimMode_SimModeType_MANUAL_VEHICLE == _setting.sim_mode().sim_mode_type()) {
      m_SimMode = SimulationMode::eManualVehicle;
      LOG(INFO) << TX_VARS_NAME(sim_mode, "manual_vehicle");
    } else {
      m_SimMode = SimulationMode::eWorldSim;
      LOG(INFO) << TX_VARS_NAME(sim_mode, "worldsim");

      if (FLAGS_ManualVehicle) {
        m_SimMode = SimulationMode::eManualVehicle;
        LOG(INFO) << TX_VARS_NAME(sim_mode, "manual_vehicle");
      }
    }
  }

  switch (m_SimMode) {
    case SimulationMode::eLogSim: {
      m_SimPtr = std::make_shared<InterpolatePlayerModule>();
      break;
    }
    case SimulationMode::eWorldSim: {
      m_SimPtr = std::make_shared<WorldsimSimLoop>();
      break;
    }
    case SimulationMode::eManualVehicle: {
      m_SimPtr = std::make_shared<TAD_ManualVehicleSimLoop>();
      break;
    }
    case SimulationMode::eL2wDesktop: {
      m_SimPtr = std::make_shared<Log2WorldSimLoop>();
      break;
    }
  }
#endif
  // 重置仿真
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Reset(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

void TADSimLoop::Step(tx_sim::StepHelper& helper) {
  // 如果m_SimPtr非空
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Step(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

void TADSimLoop::Stop(tx_sim::StopHelper& helper) {
  // 如果m_SimPtr非空
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Stop(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

TXSIM_MODULE(TrafficFlow::TADSimLoop)
