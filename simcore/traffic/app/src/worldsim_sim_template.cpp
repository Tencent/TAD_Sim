// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "worldsim_sim_template.h"
#include <chrono>
#include <thread>
#include "HdMap/tx_hashed_lane_info.h"
#include "HdMap/tx_hashed_road.h"
#include "SceneLoader/tx_tadloader.h"
#include "tad_event_dispatcher_1_0_0_0.h"
#include "tad_stand_alone_traffic_element_system.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "tx_application.h"
#include "tx_class_counter.h"
#include "tx_event_dispatcher.h"
#include "tx_scene_loader_factory.h"
#include "tx_spatial_query.h"
#include "tx_sys_info.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"
#include "tx_timer_on_cpu.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_TADSim_Loop)
#define SceneLoaderLogInfo LOG_IF(INFO, FLAGS_LogLevel_SceneLoader)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)

// 根据参数名称解析参数
void WorldsimSimLoop::ParseInitParameter(tx_sim::InitHelper& helper) TX_NOEXCEPT {
  if (Utils::Str2Type(helper.GetParameter("EnableTrafficVisionFilter"), FLAGS_EnableTrafficVisionFilter)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(EnableTrafficVisionFilter);
  }

  if (Utils::Str2Type(helper.GetParameter("TrafficVisionFilterRadius"), FLAGS_TrafficVisionFilterRadius)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(TrafficVisionFilterRadius);
  }

  if (Utils::Str2Type(helper.GetParameter("TrafficVisionFilterAltitudeDiff"), FLAGS_TrafficVisionFilterAltitudeDiff)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(TrafficVisionFilterAltitudeDiff);
  }

  LOG(INFO) << "[Input_Parameter][inject_vehicle_behavior_db] start.";
  FLAGS_inject_vehicle_behavior_db = helper.GetParameter(FLAGS_params_vehicle_behavior_db);
  LOG(INFO) << "[Input_Parameter][inject_vehicle_behavior_db] end." << TX_VARS(FLAGS_inject_vehicle_behavior_db.size())
            << TX_VARS(FLAGS_inject_vehicle_behavior_db);
}

// WorldSim初始化函数，解析前端参数，发布/订阅topic等
void WorldsimSimLoop::Init(tx_sim::InitHelper& helper) TX_NOEXCEPT {
  LogInfo << "call " << __func__;
  // 解析参数
  ParseInitParameter(helper);
  const Base::txString value = helper.GetParameter("max_step_count");
  if (!value.empty()) {
    FLAGS_max_step_count = std::stoi(value);
    LogWarn << "Init with parameter max_step_count = " << FLAGS_max_step_count << std::endl;
  }

  helper.Subscribe(FLAGS_Location_TopicName /*tx_sim::topic::kLocation*/);
#if USE_EgoGroup
  helper.Subscribe(FLAGS_EgoUnion_TopicName /*EgoUnion/LOCATION*/);
  helper.Subscribe(FLAGS_EgoUnion_Trailer_TopicName /*EgoUnion/LOCATION_TRAILER*/);
  helper.Subscribe(FLAGS_EgoUnion_Specified_TopicName /*.hightlight_group*/);
#endif
  // publish our topics with messages we produced in Step callback.
  helper.Publish(FLAGS_Traffic_TopicName /*tx_sim::topic::kTraffic*/);
#if Use_TrailerEgo
  helper.Subscribe(FLAGS_TrailerEgo_TopicName);
#endif /*Use_TrailerEgo*/
}

// 重置场景，主要重置environment，变量等
void WorldsimSimLoop::Reset(tx_sim::ResetHelper& helper) TX_NOEXCEPT {
  m_max_step_count_ = FLAGS_max_step_count;
  m_inject_vehicle_behavior_db = FLAGS_inject_vehicle_behavior_db;
  CreateEnv(helper);
}

// step回调函数，进行每一步的仿真
void WorldsimSimLoop::Step(tx_sim::StepHelper& helper) TX_NOEXCEPT {
  const Base::TimeParamManager timeMgr = MakeTimeMgr(helper.timestamp());
  timeMgr.str();
  Simulation(helper, timeMgr);
}

void WorldsimSimLoop::Stop(tx_sim::StopHelper& helper) TX_NOEXCEPT {
  LogInfo << "call ";
  helper.set_feedback("stepCounts", std::to_string(m_step_count_));
}

// 根据场景类型，创建对应的场景加载器
Base::ISceneLoaderPtr WorldsimSimLoop::CreateSceneLoader(const Base::txString strSceneType) const TX_NOEXCEPT {
  Base::SceneLoaderFactory loaderFactory(strSceneType);
  return loaderFactory.CreateSceneLoader();
}

// 创建traffic system
void WorldsimSimLoop::CreateSystem() TX_NOEXCEPT {
  m_TrafficElementSystem_Ptr = std::make_shared<TAD_StandAlone_TrafficElementSystem>();
  if (NonNull_Pointer(m_TrafficElementSystem_Ptr)) {
    LogInfo << "Create TrafficElementSystem Success.";
  } else {
    LogWarn << "Create TrafficElementSystem Failure.";
  }
}

void WorldsimSimLoop::ResetVars() TX_NOEXCEPT {
  CreateSystem();
  ResetVarsPassive(TrafficSystemPtr());
}

// 创建环境，包括system的构建，场景和地图环境的加载
void WorldsimSimLoop::CreateEnv(tx_sim::ResetHelper& helper) TX_NOEXCEPT {
  LogInfo << "call " << __func__;
#if USE_EgoGroup
  // get sim scene from helper
  sim_msg::Scene sceneDesc;
  sceneDesc.ParseFromString(helper.scene_pb());
  // for (auto& refEgo : sceneDesc.egos()) {
  // LOG(WARNING) << "helper.scene_pb : refEgo = " << refEgo.ShortDebugString();
  // }

#else  /*USE_EgoGroup*/
  // 获取各种数据
  std::vector<std::pair<int64_t, std::string> > measurements;
  sim_msg::VehicleGeometoryList vehGeomList;
  helper.vehicle_measurements(measurements);
  // 检查传入的数据是否有效
  if (1 != measurements.size()) {
    LogWarn << "helper.vehicle_measurements error. " << TX_VARS(measurements.size());
    return;
  } else {
    vehGeomList.ParseFromString(measurements.front().second);
    LOG(INFO) << vehGeomList.ShortDebugString();
  }
#endif /*USE_EgoGroup*/

  using namespace Utils;
  using namespace boost::filesystem;
  const Base::txString strSceneType = ComputeSceneType(helper.scenario_file_path());
  // 创建场景加载器
  Base::ISceneLoaderPtr loader = CreateSceneLoader(strSceneType);

  // 初始化环境变量
  ResetVars();
  LogInfo << loader->ClassName();

  // 获取场景数据路径和地图路径
  Base::txString strSceneDataPath, strHadmapPath;
  const Base::txBool res_Sim2SceneFile =
      SceneLoader::TAD_SceneLoader::Sim2SceneFile(helper.scenario_file_path(), strSceneDataPath, strHadmapPath);

  // 检查路径和模式是否有效
  if (CallSucc(res_Sim2SceneFile) && NonNull_Pointer(loader) && CallSucc(exists(FilePath(strSceneDataPath))) &&
      CallSucc(exists(FilePath(strHadmapPath))) && CallSucc(IsSimModeValid(strSceneDataPath))) {
#if USE_EgoGroup
    loader->SetVehicleGeometory(sceneDesc);
#else  /*USE_EgoGroup*/
    // 设置车辆几何信息
    loader->SetVehicleGeometory(0, vehGeomList);
#endif /*USE_EgoGroup*/
    // 创建环境
    auto res = CreateEnvPassive(loader, helper.scenario_file_path(), strSceneDataPath, strHadmapPath);
    if (CallSucc(res)) {
      LogInfo << "CreateEnvPassive success.";
    } else {
      LogWarn << "CreateEnvPassive failure.";
    }
  } else {
    LogWarn << "file loader failure. " << TX_COND(res_Sim2SceneFile) << TX_COND(exists(FilePath(strSceneDataPath)))
            << TX_COND(exists(FilePath(strHadmapPath)));
    return;
  }
}

void WorldsimSimLoop::PreSimulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {}

void WorldsimSimLoop::ReceiveEgoInfo(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
#if USE_EgoGroup
  // update highlight ego
  Base::txString specifiedEgoGroupPayload;
  GetSubscribedMessage(helper, FLAGS_EgoUnion_Specified_TopicName, specifiedEgoGroupPayload);

  TrafficSystemPtr()->UpdatePlanningCarHighlight(timeMgr, specifiedEgoGroupPayload);

  // update egos
  const auto sEgoType = TrafficSystemPtr()->GetEgoType();
  GetSubscribedMessage(helper, FLAGS_EgoUnion_TopicName, m_payload_);
  TrafficSystemPtr()->UpdatePlanningCarData(timeMgr, Base::Enums::EgoSubType::eLeader, m_payload_);

  // update ego trailers
  GetSubscribedMessage(helper, FLAGS_EgoUnion_Trailer_TopicName, m_payload_trailer_);
  TrafficSystemPtr()->UpdatePlanningCarData(timeMgr, Base::Enums::EgoSubType::eFollower, m_payload_trailer_);

  m_payload_.clear();
  m_payload_trailer_.clear();

#else  /*USE_EgoGroup*/
  const auto sEgoType = TrafficSystemPtr()->GetEgoType();
  // 获取ego位置信息，更新traffic的planning data
  GetSubscribedMessage(helper, tx_sim::topic::kLocation, m_payload_);
  TrafficSystemPtr()->UpdatePlanningCarData(timeMgr, Base::Enums::EgoSubType::eLeader, m_payload_);
  // ego为truck时，另外获取trailer位置信息，更新traffic的planning data
  if (_plus_(Base::ISceneLoader::EgoType::eTruck) == sEgoType) {
    GetSubscribedMessage(helper, FLAGS_TrailerEgo_TopicName, m_payload_trailer_);
    TrafficSystemPtr()->UpdatePlanningCarData(timeMgr, Base::Enums::EgoSubType::eFollower, m_payload_trailer_);
  }

  m_payload_.clear();
  m_payload_trailer_.clear();
#endif /*USE_EgoGroup*/
}

void WorldsimSimLoop::SimulationTraffic(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  m_outputTraffic.Clear();
  // 对traffic仿真调用，抽取traffic信息到m_outputTraffic
  SimulationTrafficPassive(timeMgr, &m_outputTraffic);
  // 发布traffic的topic消息
  PublishMessage(helper, FLAGS_Traffic_TopicName, m_outputTraffic);
}

void WorldsimSimLoop::Simulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  static Base::TimingCPU s_timer;
  s_timer.StartCounter();
  LOG_IF(INFO, FLAGS_LogLevel_Visualizer || FLAGS_LogLevel_Statistics_MultiThread || FLAGS_LogLevel_Statistics)
      << __func__;

  using TimeParamManager = Base::TimeParamManager;
  // 若traffic system可用 仿真当前步
  if (NonNull_Pointer(TrafficSystemPtr()) && CallSucc(TrafficSystemPtr()->IsAlive())) {
    // LOG(INFO) << timeMgr.str();
    // 分别对仿真前和仿真后，调用各个函数进行处理
    PreSimulation(helper, timeMgr);

    // 更新ego位置
    if (FLAGS_SimulateWithPnC) {
      ReceiveEgoInfo(helper, timeMgr);
    }

    // 仿真traffic
    SimulationTraffic(helper, timeMgr);

    // 仿真后置处理
    PostSimulation(helper, timeMgr);
  } else {
    LOG(WARNING) << "m_TrafficElementSystemPtr is not initialized, Send 0 elements.";
  }
  m_step_count_++;
  LOG_IF(INFO, FLAGS_LogLevel_Statistics_MultiThread || FLAGS_LogLevel_Statistics)
      << "Step end. " << TX_VARS_NAME(timer, s_timer.GetElapsedMicroseconds());
}

void WorldsimSimLoop::PostSimulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {}

Base::txBool WorldsimSimLoop::IsSimModeValid(const Base::txString strScenePath) const TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  std::string strExt = FilePath(strScenePath).extension().string();
  // 根据场景文件拓展名与传入的Worldsim_Extension进行匹配，是否一致
  if (boost::iequals(strExt, FLAGS_Worldsim_Extension)) {
    return true;
  } else {
    LOG(WARNING) << "[Error] SimMode is logsim, " << TX_VARS(strExt) << TX_VARS(FLAGS_Worldsim_Extension);
    return false;
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef SceneLoaderLogInfo
#undef LogWarn
