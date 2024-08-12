// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_stand_alone_traffic_element_system.h"
#include "tad_assembler_context.h"
#include "tad_ego_vehicle_element.h"
#include "tad_event_dispatcher_1_0_0_0.h"
#include "tad_event_trigger_param.h"
#include "tad_linear_element_manager.h"
#include "tad_map_obstacle_element.h"
#include "tad_obstacle_element.h"
#include "tad_relative_trajectory_follow_element.h"
#include "tad_vehicle_kinetics_info.h"
#include "tbb/concurrent_unordered_set.h"
#include "tx_env_perception.h"
#include "tx_event_dispatcher.h"
#include "tx_pedestrian_element.h"
#include "tx_scene_sketch.h"
#include "tx_spatial_query.h"
#include "tx_tadsim_flags.h"
#include "tx_timer_on_cpu.h"

#define SketchInfo LOG_IF(INFO, FLAGS_LogLevel_Sketch) << "[Sketch]"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_TADSim_Loop)
#define LogPerfStatistic LOG_IF(INFO, FLAGS_LogLevel_Statistics_MultiThread)
#define LogL2WEgoInfo LOG_IF(INFO, FLAGS_LogLevel_L2W_EgoInfo)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool TAD_StandAlone_TrafficElementSystem::IsSupportSceneType(
    const Base::ISceneLoader::ESceneType _sceneType) const TX_NOEXCEPT {
  // 如果场景类型为eTAD、eOSC或eSimrec
  if (_plus_(Base::ISceneLoader::ESceneType::eTAD) == (_sceneType) ||
      _plus_(Base::ISceneLoader::ESceneType::eOSC) == (_sceneType) ||
      _plus_(Base::ISceneLoader::ESceneType::eSimrec) == (_sceneType)) {
    // 返回true，表示支持该场景类型
    return true;
  } else {
    return false;
  }
}

// 刷新场景事件
void TAD_StandAlone_TrafficElementSystem::FlushSceneEvents(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  // 如果时间经过大于0
  if (timeMgr.PassTime() > 0.0) {
    // 获取自车数组
    auto refEgoVec = ElemMgr()->GetEgoArray();
    for (auto curEgoPtr : refEgoVec) {
      /*TrafficFlow::TAD_EgoVehicleElementPtr curEgoPtr =
          std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(refEgo);*/
      // 如果当前自车指针不为空且有效
      if (NonNull_Pointer(curEgoPtr) && curEgoPtr->IsValid()) {
        // 输出日志信息
        LOG_IF(INFO, FLAGS_LogLevel_EventTrigger) << "[FlushSceneEvents] at " << timeMgr.str();
        // 创建一个事件触发参数对象
        TrafficFlow::EventTriggerParam_t _currentEventParam(timeMgr, curEgoPtr);
        // 刷新所有事件
        TrafficFlow::txEventDispatcher::getInstance().FlushAllEvents(_currentEventParam);
        TX_MARK(" return;   multi-ego event trigger.");
      }
    }

    // 刷新所有事件
    TrafficFlow::txEventDispatcher_v_1_0_0_0::getInstance().FlushAllEvents(timeMgr, ElemMgr(), SceneLoader());
  }
}

// 注册规划车辆
Base::txBool TAD_StandAlone_TrafficElementSystem::RegisterPlanningCar() TX_NOEXCEPT {
  LogInfo << "Call RegisterPlanningCar";

  // 如果场景加载器、元素管理器和组装器上下文指针不为空
  if (NonNull_Pointer(SceneLoader()) && NonNull_Pointer(ElemMgr()) && NonNull_Pointer(AssemblerCtx())) {
#if USE_EgoGroup

    if (IsSupportSceneType(SceneLoader()->GetSceneType()) && NonNull_Pointer(AssemblerCtx())) {
      return AssemblerCtx()->GenerateEgo(ElemMgr(), SceneLoader());
    } else {
      LogWarn << "RegisterPlanningCar failed." << TX_VARS(SceneLoader()->GetSceneType())
              << TX_VARS(NonNull_Pointer(AssemblerCtx()));
      return false;
    }

#else  /*USE_EgoGroup*/
    // 如果自车数组为空
    if (CallSucc(ElemMgr()->GetEgoArray().empty())) {
      TX_MARK("Ego has not been created.");

      // 如果支持当前场景类型
      if (IsSupportSceneType(SceneLoader()->GetSceneType())) {
        sim_msg::Location pbEgoInfo;
        // 获取路由信息
        SceneLoader()->GetRoutingInfo(pbEgoInfo);
        // 设置自车数据
        SceneLoader()->SetEgoData(pbEgoInfo);
      } else {
        // 如果不支持当前场景类型，输出警告信息
        LogWarn << "Un support Scene Type (eTAD, eOSC)";
        return false;
      }

      // 如果组装器上下文指针不为空且生成自车成功
      if (NonNull_Pointer(AssemblerCtx()) && CallSucc(AssemblerCtx()->GenerateEgo(ElemMgr(), SceneLoader()))) {
        // 输出日志信息
        LogInfo << "GenerateEgo Success.";
      } else {
        LOG(WARNING) << "GenerateEgo Failure.";
        return false;
      }
      // 获取自车数组
      const auto &refEgoArray = ElemMgr()->GetEgoArray();
      // 检查自车类型
      return CheckEgoType(SceneLoader()->GetEgoType(), refEgoArray.size());
    } else {
      return CheckEgoType(GetEgoType(), ElemMgr()->GetEgoCount());
    }
#endif /*USE_EgoGroup*/
  } else {
    LogWarn << "NonNull_Pointer(m_SceneDataSource) && NonNull_Pointer(m_ElementMgr) error.";
    return false;
  }
}

#if USE_EgoGroup
Base::txBool TAD_StandAlone_TrafficElementSystem::UpdatePlanningCarHighlight(
    Base::TimeParamManager const &timeMgr, const Base::txString &highlightStr) TX_NOEXCEPT {
  sim_msg::HighlightGroup highlightGroup;
  highlightGroup.ParseFromString(highlightStr);
  if (CallFail(highlightGroup.groupname().empty())) {
    FLAGS_EgoUnion_Specified_EgoGroup = highlightGroup.groupname();
    return true;
  }
  return false;
}
#endif

Base::txBool TAD_StandAlone_TrafficElementSystem::UpdatePlanningCarData(Base::TimeParamManager const &timeMgr,
                                                                        const Base::Enums::EgoSubType _egoSubType,
                                                                        const Base::txString &egoInfoStr) TX_NOEXCEPT {
  if (_NonEmpty_(egoInfoStr)) {
#if USE_EgoGroup
    sim_msg::Union egoUnion;
    egoUnion.ParseFromString(egoInfoStr);
    // LOG(INFO) << TX_VARS(egoUnion.DebugString());
    for (auto &egoMsgPair : egoUnion.messages()) {
      auto baseEgoPtr = ElemMgr()->GetEgoPtrByGroup(egoMsgPair.groupname(), _egoSubType);
      TrafficFlow::TAD_EgoVehicleElementPtr curEgoPtr =
          std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(baseEgoPtr);
      if (NonNull_Pointer(curEgoPtr)) {
        curEgoPtr->SetProtoBufferStr(egoMsgPair.content());
        curEgoPtr->SetValid(true);
      }
    }
    return true;
#else  /*USE_EgoGroup*/
    // 解析位置信息
    m_pb_location.ParseFromString(egoInfoStr);
    LogL2WEgoInfo << timeMgr.str() << TX_VARS_NAME(recive_ego_loc, m_pb_location.ShortDebugString());

    // 获取ego的集合
    auto retEgoVec = (ElemMgr()->GetEgoArray());

    for (auto refEgo : retEgoVec) {
      // 如果支持当前场景
      if (IsSupportSceneType(SceneLoader()->GetSceneType())) {
        TrafficFlow::TAD_EgoVehicleElementPtr curEgoPtr =
            std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(refEgo);
        // 根据ego或trailer类型
        if (NonNull_Pointer(curEgoPtr) && _egoSubType == (curEgoPtr->GetSubType())) {
          // 设置位置信息
          curEgoPtr->SetProtoBufferStr(egoInfoStr);
          curEgoPtr->SetValid(true);
          return true;
        }
      }
    }
#endif /*USE_EgoGroup*/
  }
  return false;
}

// 创建组装器上下文
void TAD_StandAlone_TrafficElementSystem::CreateAssemblerCtx() TX_NOEXCEPT {
  m_AssemblerCtx = std::make_shared<Scene::TAD_AssemblerContext>();
  if (NonNull_Pointer(m_AssemblerCtx)) {
    // 初始化组装器上下文，传入场景加载器
    m_AssemblerCtx->Init(SceneLoader());
  }
}

// 创建元素管理器
void TAD_StandAlone_TrafficElementSystem::CreateElemMgr() TX_NOEXCEPT {
  // 创建一个TAD_LinearElementManager对象并将其赋值给m_ElementMgr
  m_ElementMgr = std::make_shared<TrafficFlow::TAD_LinearElementManager>();
}

Base::txBool TAD_StandAlone_TrafficElementSystem::Initialize(Base::ISceneLoaderPtr loader) TX_NOEXCEPT {
  // 如果loader非空
  if (NonNull_Pointer(loader)) {
    // 将loader赋值给m_SceneDataSource
    m_SceneDataSource = loader;
    m_EgoType = SceneLoader()->GetEgoType();
    // 创建元素管理器
    CreateElemMgr();
    // 创建装配器
    CreateAssemblerCtx();

    // 检查相关指针不为空，场景元素生成成功
    if (NonNull_Pointer(AssemblerCtx()) && NonNull_Pointer(ElemMgr()) && CallSucc(SceneLoader()->LoadObjects()) &&
        CallSucc(AssemblerCtx()->GenerateScene(m_ElementMgr, loader))) {
      // 调用SceneLoader()->PrintStream函数
      SceneLoader()->PrintStream(LOG(INFO));
      // 如果FLAGS_SimulateWithPnC为真
      if (FLAGS_SimulateWithPnC) {
        TX_MARK("Ego must be created before topological sort");
        // 调用RegisterPlanningCar函数
        RegisterPlanningCar();
      }

      // 如果ElemMgr()->Initialize(loader)成功
      if (CallSucc(ElemMgr()->Initialize(loader))) {
        LogInfo << "GenerateScene Success.";
        m_isAlive = true;
      } else {
        // 输出警告信息
        LogWarn << "GenerateScene Failure.";
        m_isAlive = false;
      }
    } else {
      // 输出警告信息
      LogWarn << "GenerateScene Failure.";
      m_isAlive = false;
    }
  } else {
    LogWarn << "GenerateScene Failure. Data is Null.";
    m_isAlive = false;
  }
  return IsAlive();
}

// 仿真整体更新
Base::txBool TAD_StandAlone_TrafficElementSystem::Update(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  Base::txBool ret_v = true;
  if (CallFail(IsAlive())) {
    LogWarn << "IsAlive() = false.";
    return false;
  }
  // 首先更新场景中元素动态生成
  if (CallFail(AssemblerCtx()->DynamicGenerateScene(timeMgr, ElemMgr(), SceneLoader()))) {
    LOG(WARNING) << "DynamicGenerateScene failure.";
    return false;
  }
  // 更新signal
  if (CallFail(Update_Signal(timeMgr))) {
    ret_v = false;
    LogWarn << "Update_Signal run failure.";
  }

  // 更新生命周期
  if (CallFail(Update_CheckLifeCycle(timeMgr))) {
    ret_v = false;
    LogWarn << "Update_CheckLifeCycle run failure.";
  }

#if __Consistency__
  {
    auto &refVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
    for (auto &refFlowElem : refVehicleArray) {
      if (NonNull_Pointer(refFlowElem) && CallSucc(refFlowElem->IsAlive())) {
        refFlowElem->ResetConsistency();
      }
    }
  }
#endif /*__Consistency__*/
  ExecuteEnvPerception(timeMgr);
  if (timeMgr.RelativeTime() > 0.0) {
    // 前置更新操作
    if (CallFail(Update_Vehicle_PreSimulation(timeMgr))) {
      ret_v = false;
      LogWarn << "Update_Vehicle_PreSimulation run failure.";
    }

    // vehicle仿真更新
    if (CallFail(Update_Vehicle_Simulation(timeMgr))) {
      ret_v = false;
      LogWarn << "Update_Vehicle_Simulation run failure.";
    }

    // 更新后置操作
    if (CallFail(Update_Vehicle_PostSimulation(timeMgr))) {
      ret_v = false;
      LogWarn << "Update_Vehicle_PostSimulation run failure.";
    }

    // 更新动力学信息
    if (CallFail(Update_ComputeKineticsInfo(timeMgr))) {
      ret_v = false;
      LogWarn << "Update_ComputeKineticsInfo run failure.";
    }

    // 更新额外信息 主车前后车辆信息
    if (CallFail(Update_CarExternalInfo(timeMgr))) {
      ret_v = false;
      LogWarn << "Update_CarExternalInfo run failure.";
    }

    // 更新vehilce的fcw
    if (CallFail(Update_FCW(timeMgr))) {
      ret_v = false;
      LogWarn << "Update_FCW run failure.";
    }

    // 更新pedestrian
    if (CallFail(Update_Pedestrian(timeMgr))) {
      ret_v = false;
      LogWarn << "Update_Pedestrian run failure.";
    }

    // 更新静态障碍物
    if (CallFail(Update_Obstacle(timeMgr))) {
      ret_v = false;
      LogWarn << "Update_Obstacle run failure.";
    }

    // 刷新场景事件
    FlushSceneEvents(timeMgr);
  }
  if (CallFail(AssemblerCtx()->DynamicChangeScene(timeMgr, ElemMgr(), SceneLoader()))) {
    LOG(WARNING) << "DynamicChangeScene failure.";
    return false;
  }
  ElemMgr()->ResortKillElement();
  return ret_v;
}

Base::txBool TAD_StandAlone_TrafficElementSystem::Update_Signal(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  static Base::TimingCPU s_timer;
  s_timer.StartCounter();
  // 获取所有的signal元素
  auto &refAllSignal = ElemMgr()->GetAllSignal();
#if USE_TBB /* One lane has multiple lights  */
  // 并行循环更新
  tbb::parallel_for(static_cast<std::size_t>(0), refAllSignal.size(), [&](const std::size_t idx) {
    if (NonNull_Pointer(refAllSignal[idx])) {
      // 更新信号灯
      refAllSignal[idx]->Update(timeMgr);
    }
  }); /*lamda function*/
      /* parallel_for */
#else
  for (auto &signalPtr : refAllSignal) {
    if (NonNull_Pointer(signalPtr)) {
      signalPtr->Update(timeMgr);
    }
  }
#endif
  LogPerfStatistic << "Update_Signal :" << TX_VARS_NAME(Time_Consuming, s_timer.GetElapsedMicroseconds());
  return true;
}

Base::txBool TAD_StandAlone_TrafficElementSystem::Update_CheckLifeCycle(const Base::TimeParamManager &timeMgr)
    TX_NOEXCEPT {
  static Base::TimingCPU s_timer;
  s_timer.StartCounter();
  // 获取所有的元素
  auto &refAllElement = ElemMgr()->GetAllElement();
  tbb::parallel_for(static_cast<std::size_t>(0), refAllElement.size(), [&](const std::size_t idx) {
    auto &elemPtr = refAllElement[idx];
    if (NonNull_Pointer(elemPtr)) {
      // 检查更新生命周期
      elemPtr->CheckStart(timeMgr);
      // 检查更新生命周期结束状态
      elemPtr->CheckEnd(timeMgr);
    }
  }); /*lamda function*/
  /* parallel_for */

  // 获取relativeobs
  auto &refAllRelativeObs = ElemMgr()->GetRelativeObsArray();
  for (auto &elemPtr : refAllRelativeObs) {
    if (NonNull_Pointer(elemPtr)) {
      // 检查开始和结束
      elemPtr->CheckStart(timeMgr);
      elemPtr->CheckEnd(timeMgr);
    }
  }
  LogPerfStatistic << "Update_CheckLifeCycle :" << TX_VARS_NAME(Time_Consuming, s_timer.GetElapsedMicroseconds());
  return true;
}

Base::txBool TAD_StandAlone_TrafficElementSystem::Update_Vehicle_PreSimulation(const Base::TimeParamManager &timeMgr)
    TX_NOEXCEPT {
  std::map<Base::txSysId, Base::ITrafficElement::KineticsInfo_t> map_elemId2Kinetics;
  // 首先获取ego
  auto refEgoArray = ElemMgr()->GetEgoArray();
  // 遍历更新ego
  for (auto refEgoPtr : refEgoArray) {
    if (NonNull_Pointer(refEgoPtr) && CallSucc(refEgoPtr->IsAlive())) {
      // 预更新元素
      refEgoPtr->Pre_Update(timeMgr, map_elemId2Kinetics);
      map_elemId2Kinetics[refEgoPtr->Id()] = refEgoPtr->GetKineticsInfo(timeMgr);
    }
  }
  // 获取所有的vehicle类型元素
  auto &refVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
  // 遍历更新veh
  for (auto &refFlowElem : refVehicleArray) {
    if (NonNull_Pointer(refFlowElem) && CallSucc(refFlowElem->IsAlive())) {
      // 预更新元素
      if (CallFail(refFlowElem->Pre_Update(timeMgr, map_elemId2Kinetics))) {
        LogWarn << "Elem Id = " << (refFlowElem->Id()) << " Simulate Failure. Time : " << timeMgr.str();
      }
      map_elemId2Kinetics[refFlowElem->Id()] = refFlowElem->GetKineticsInfo(timeMgr);
    }
  }
  return true;
}

Base::txBool TAD_StandAlone_TrafficElementSystem::Update_Vehicle_PostSimulation(const Base::TimeParamManager &timeMgr)
    TX_NOEXCEPT {
  // 获取所有ego元素
  auto refEgoArray = ElemMgr()->GetEgoArray();
  for (auto egoPtr : refEgoArray) {
    if (NonNull_Pointer(egoPtr)) {
      // 更新后的处理
      egoPtr->Post_Update(timeMgr);
    }
  }

  // 根据类型获取vehicle元素
  auto &refVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
  tbb::parallel_for(static_cast<std::size_t>(0), refVehicleArray.size(), [&](const std::size_t idx) {
    auto &elemPtr = refVehicleArray[idx];
    if (NonNull_Pointer(elemPtr)) {
      // 更新后的处理
      elemPtr->Post_Update(timeMgr);
    }
  }); /*lamda function*/
      /* parallel_for */
#if __Consistency__
  std::vector<Base::ITrafficElementPtr> sort_array = refVehicleArray;
  std::sort(sort_array.begin(), sort_array.end(),
            [](const Base::ITrafficElementPtr &lhs, const Base::ITrafficElementPtr &rhs) {
              return (lhs->Id()) < (rhs->Id());
            });

  std::ostringstream oss;
  oss << FLAGS_ConsistencyRecPath + "/frame_" << timeMgr.PassTime() << ".txt";
  std::ofstream output(oss.str());
  for (auto &ptr : sort_array) {
    output << (ptr->ConsistencySnapshot()) << std::endl;
  }
  output.close();
#endif /*__Consistency__*/
  return true;
}

// 更新车辆模拟的函数
Base::txBool TAD_StandAlone_TrafficElementSystem::Update_Vehicle_Simulation(const Base::TimeParamManager &timeMgr)
    TX_NOEXCEPT {
  // 定义一个计时器
  static Base::TimingCPU s_timer;
  s_timer.StartCounter();
  auto refEgoArray = ElemMgr()->GetEgoArray();
  // 遍历Ego数组
  for (auto egoPtr : refEgoArray) {
    // 如果egoPtr非空
    if (NonNull_Pointer(egoPtr)) {
      // 更新egoPtr的状态
      egoPtr->Update(timeMgr);
    }
  }

  // 获取车辆数组
  auto &refVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
#if USE_TBB
  // 使用并行for循环遍历车辆数组
  tbb::parallel_for(static_cast<std::size_t>(0), refVehicleArray.size(), [&](const std::size_t idx) {
    auto &elemPtr = refVehicleArray[idx];
    // 如果elemPtr非空
    if (NonNull_Pointer(elemPtr)) {
      elemPtr->Update(timeMgr);
    }
  }); /*lamda function*/
      /* parallel_for */
#else
  // 如果不使用TBB，使用普通for循环遍历车辆数组
  for (auto &elemPtr : refVehicleArray) {
    if (NonNull_Pointer(elemPtr)) {
      elemPtr->Update(timeMgr);
    }
  }
#endif
  // 输出性能统计信息
  LogPerfStatistic << "Update_Vehicle_Simulation :" << TX_VARS_NAME(Time_Consuming, s_timer.GetElapsedMicroseconds());
  return true;
}

Base::txBool TAD_StandAlone_TrafficElementSystem::Update_ComputeKineticsInfo(const Base::TimeParamManager &timeMgr)
    TX_NOEXCEPT {
  static Base::TimingCPU s_timer;
  s_timer.StartCounter();
  TrafficFlow::IKineticsHandler::KineticsUtilInfo_t egoKinecticsInfo;
#if USE_EgoGroup
  auto baseEgoPtr = ElemMgr()->GetEgoPtrByGroup(FLAGS_EgoUnion_Specified_EgoGroup,
                                                _plus_(TrafficFlow::TAD_EgoVehicleElement::EgoSubType::eLeader));
  TrafficFlow::TAD_EgoVehicleElementPtr curEgoPtr =
      std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(baseEgoPtr);
  if (NonNull_Pointer(curEgoPtr)) {
    egoKinecticsInfo = curEgoPtr->GetKineticsUtilInfo();
  }
#else  /*USE_EgoGroup*/
  auto retEgoVec = (ElemMgr()->GetEgoArray());
  // 遍历retEgoVec中的每个元素
  for (auto refEgo : retEgoVec) {
    // 将refEgo转换为TrafficFlow::TAD_EgoVehicleElementPtr类型
    TrafficFlow::TAD_EgoVehicleElementPtr curEgoPtr =
        std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(refEgo);
    // 如果curEgoPtr非空且其子类型为eLeader
    if (NonNull_Pointer(curEgoPtr) &&
        _plus_(TrafficFlow::TAD_EgoVehicleElement::EgoSubType::eLeader) == (curEgoPtr->GetSubType())) {
      // 获取curEgoPtr的运动学信息
      egoKinecticsInfo = curEgoPtr->GetKineticsUtilInfo();
      break;
    }
  }
#endif /*USE_EgoGroup*/

  // 如果egoKinecticsInfo有效
  if (CallSucc(egoKinecticsInfo.m_isValid)) {
    // 获取所有车辆元素的数组
    auto &refAllVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
    // 使用并行for循环遍历所有车辆元素
    tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray.size(), [&](const std::size_t idx) {
      // 将refAllVehicleArray[idx]转换为TrafficFlow::IKineticsHandlerPtr类型
      TrafficFlow::IKineticsHandlerPtr kineticsPtr =
          std::dynamic_pointer_cast<TrafficFlow::IKineticsHandler>(refAllVehicleArray[idx]);
      if (NonNull_Pointer(kineticsPtr)) {
        // 计算车辆的运动学信息
        kineticsPtr->ComputeKinetics(timeMgr, egoKinecticsInfo);
      }
    }); /*lamda function*/

    LogPerfStatistic << "ComputeVehicleKineticsInfo Vehilce :"
                     << TX_VARS_NAME(Time_Consuming, s_timer.GetElapsedMicroseconds());

    // 获取所有行人元素的数组
    auto &refAllPedestrianArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Pedestrian);
    // 遍历所有行人元素
    for (auto refPedestrianPtr : refAllPedestrianArray) {
      // 将refPedestrianPtr转换为TrafficFlow::IKineticsHandlerPtr类型
      TrafficFlow::IKineticsHandlerPtr kineticsPtr =
          std::dynamic_pointer_cast<TrafficFlow::IKineticsHandler>(refPedestrianPtr);
      if (NonNull_Pointer(kineticsPtr)) {
        // 计算行人的运动学信息
        kineticsPtr->ComputeKinetics(timeMgr, egoKinecticsInfo);
      }
    }

    LogPerfStatistic << "ComputeVehicleKineticsInfo Pedestrian :"
                     << TX_VARS_NAME(Time_Consuming, s_timer.GetElapsedMicroseconds());
  }
  return true;
}

// 更新所有行人元素的状态
Base::txBool TAD_StandAlone_TrafficElementSystem::Update_Pedestrian(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  // 获取所有行人元素的数组
  auto &refAllPedestrianArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Pedestrian);
  // 判断是否使用TBB库进行并行计算
#if USE_TBB
  // 使用TBB库的并行for循环遍历所有行人元素
  tbb::parallel_for(static_cast<std::size_t>(0), refAllPedestrianArray.size(), [&](const std::size_t idx) {
    auto &elemPtr = refAllPedestrianArray[idx];
    if (NonNull_Pointer(elemPtr)) {
      // 更新行人元素的状态
      elemPtr->Update(timeMgr);
    }
  }); /*lamda function*/
  /* parallel_for */
#else
  // 遍历所有行人元素
  for (auto &elemPtr : refAllPedestrianArray) {
    if (NonNull_Pointer(elemPtr)) {
      // 更新行人元素的状态
      elemPtr->Update(timeMgr);
    }
  }
#endif
  return true;
}

Base::txBool TAD_StandAlone_TrafficElementSystem::Update_Obstacle(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  // 获取所有障碍物元素的数组
  auto &refAllObstacleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Immovability);
  // 遍历所有障碍物元素
  for (auto refObstaclePtr : refAllObstacleArray) {
    if (NonNull_Pointer(refObstaclePtr) && CallSucc(refObstaclePtr->IsAlive())) {
      // 更新障碍物元素的状态
      refObstaclePtr->Update(timeMgr);
    }
  }

  // 更新相对轨迹跟随障碍物元素的状态
  {
    // 获取所有相对轨迹跟随障碍物元素的数组
    auto &refAllRelativeObs = ElemMgr()->GetRelativeObsArray();
    // 遍历所有相对轨迹跟随障碍物元素
    for (auto &elemPtr : refAllRelativeObs) {
      // 如果elemPtr非空
      if (NonNull_Pointer(elemPtr)) {
        // 将elemPtr转换为TrafficFlow::TAD_RelativeTrajectoryFollowElementPtr类型
        TrafficFlow::TAD_RelativeTrajectoryFollowElementPtr relative_obs_ptr =
            std::dynamic_pointer_cast<TrafficFlow::TAD_RelativeTrajectoryFollowElement>(elemPtr);
        // 如果relative_obs_ptr非空
        if (NonNull_Pointer(relative_obs_ptr)) {
          // 如果relative_obs_ptr是相对轨迹跟随障碍物
          if (CallSucc(relative_obs_ptr->IsRelativeObs())) {
            // 获取相对轨迹跟随障碍物的参考元素类型和ID
            Base::Enums::ElementType reference_elemType;
            Base::txSysId reference_elemId;
            std::tie(reference_elemType, reference_elemId) = relative_obs_ptr->RelativeReferenceElementInfo();
            // 获取所有参考元素的数组
            auto elemVec = ElemMgr()->SearchElementByType(reference_elemType);
            // 遍历所有参考元素
            for (auto reference_elem_ptr : elemVec) {
              if (NonNull_Pointer(reference_elem_ptr) && reference_elemId == reference_elem_ptr->Id()) {
                // 更新相对轨迹跟随障碍物元素的状态
                relative_obs_ptr->Update(timeMgr, reference_elem_ptr);
              }
            }
          } else if (CallSucc(relative_obs_ptr->IsAbsoluteObs())) {
            // 更新绝对轨迹跟随障碍物元素的状态
            relative_obs_ptr->Update(timeMgr, nullptr);
          }
        }
      }
    }
  }
  return true;
}

void TAD_StandAlone_TrafficElementSystem::ExecuteEnvPerception(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {
  // 获取所有车辆元素的数组
  auto &refAllVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
#if USE_TBB
  // 使用TBB库的并行for循环遍历所有车辆元素
  tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray.size(), [&](const std::size_t idx) {
    // 将refAllVehicleArray[idx]转换为Base::IEnvPerceptionPtr类型
    Base::IEnvPerceptionPtr env_perception_interface_ptr =
        std::dynamic_pointer_cast<Base::IEnvPerception>(refAllVehicleArray[idx]);
    // 如果env_perception_interface_ptr非空
    if (NonNull_Pointer(env_perception_interface_ptr)) {
      env_perception_interface_ptr->ExecuteEnvPerception(timeMgr);
    }
  }); /*lamda function*/
      /* parallel_for */

#else
  // 遍历所有车辆元素
  for (auto &elemPtr : refAllVehicleArray) {
    Base::IEnvPerceptionPtr env_perception_interface_ptr = std::dynamic_pointer_cast<Base::IEnvPerception>(elemPtr);
    if (NonNull_Pointer(env_perception_interface_ptr)) {
      env_perception_interface_ptr->ExecuteEnvPerception(timeMgr);
    }
  }
#endif
}

Base::txBool TAD_StandAlone_TrafficElementSystem::HasEgo() const TX_NOEXCEPT {
  // 如果ElemMgr()非空且自车元素数量大于0
  if (NonNull_Pointer(ElemMgr()) && ElemMgr()->GetEgoCount() > 0) {
    // 返回true表示存在自车元素
    return true;
  } else {
    return false;
  }
}

Base::txBool TAD_StandAlone_TrafficElementSystem::Update_CarExternalInfo(const Base::TimeParamManager &timeMgr)
    TX_NOEXCEPT {
  // 如果存在自车元素
  if (CallSucc(HasEgo())) {
    // 获取所有车辆元素的数组
    auto &refAllVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
    // 使用TBB库的并行for循环遍历所有车辆元素
    tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray.size(), [&](const std::size_t idx) {
      // 将refAllVehicleArray[idx]转换为Base::IVehicleElementPtr类型
      Base::IVehicleElementPtr veh_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(refAllVehicleArray[idx]);
      if (NonNull_Pointer(veh_ptr)) {
        // 清除车辆元素的外部信息
        veh_ptr->ClearExternInfo();
      }
    }); /*lamda function*/
        /* parallel_for */

    // 获取自车元素的指针
    Base::ITrafficElementPtr ego_ptr = ElemMgr()->GetEgoArray().front();
    // 将ego_ptr转换为TrafficFlow::TAD_EgoVehicleElementPtr类型
    TrafficFlow::TAD_EgoVehicleElementPtr ego_class_ptr =
        std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(ego_ptr);
    if (NonNull_Pointer(ego_class_ptr)) {
      std::tuple<Base::txSysId, Base::txFloat> front_info, rear_info;
      // 搜寻前后车辆
      ego_class_ptr->Search_Front_Rear_Vehicle_HLane(timeMgr, front_info, rear_info);
      // 处理前车车辆
      if (std::get<0>(front_info) >= 0) {
        // 获取前车车辆的指针
        Base::ITrafficElementPtr front_veh_base_ptr = ElemMgr()->GetVehiclePtrById(std::get<0>(front_info));
        Base::IVehicleElementPtr front_veh_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(front_veh_base_ptr);
        // 如果front_veh_ptr非空
        if (NonNull_Pointer(front_veh_ptr)) {
          front_veh_ptr->set_front_vehicle(std::get<1>(front_info));
        }
      }
      // 处理后车车辆
      if (std::get<0>(rear_info) >= 0) {
        Base::ITrafficElementPtr rear_veh_base_ptr = ElemMgr()->GetVehiclePtrById(std::get<0>(rear_info));
        Base::IVehicleElementPtr rear_veh_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(rear_veh_base_ptr);
        // 如果rear_veh_ptr非空
        if (NonNull_Pointer(rear_veh_ptr)) {
          rear_veh_ptr->set_rear_vehicle(std::get<1>(rear_info));
        }
      }
    }
  }
  return true;
}

Base::txBool TAD_StandAlone_TrafficElementSystem::Update_FCW(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  // 定义碰撞预警半径的平方
  static const Base::txFloat TAD_FCW_Radius_Square = FLAGS_TAD_FCW_Radius * FLAGS_TAD_FCW_Radius;
  // 如果存在自车元素
  if (CallSucc(HasEgo())) {
    // 获取自车元素的指针
    Base::ITrafficElementPtr ego_ptr = ElemMgr()->GetEgoArray().front();
    auto ego_txwgs84 = ego_ptr->GetLocation();
    const Base::txVec2 ego_pos2d = ego_txwgs84.ToENU().ENU2D();
    // 获取所有车辆元素的数组
    auto &refAllVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
    tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray.size(), [&](const std::size_t idx) {
      // 将refAllVehicleArray[idx]转换为Base::IVehicleElementPtr类型
      Base::IVehicleElementPtr veh_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(refAllVehicleArray[idx]);
      if (NonNull_Pointer(veh_ptr)) {
        // 获取车辆元素的位置
        const Base::txVec2 car_pos2d = veh_ptr->StableGeomCenter().ENU2D();
        // 如果车辆元素与自车元素的距离小于碰撞预警半径
        if ((car_pos2d - ego_pos2d).squaredNorm() < TAD_FCW_Radius_Square) {
          // 计算车辆元素的碰撞预警信息
          veh_ptr->Compute_FCW(ego_ptr);
        }
      }
    }); /*lamda function*/
        /* parallel_for */
  }
  return true;
}

Base::txBool TAD_StandAlone_TrafficElementSystem::FillingTrafficData(Base::TimeParamManager const &timeMgr,
                                                                     sim_msg::Traffic &outTraffic) TX_NOEXCEPT {
  // 提取交通流信息到pb
  if (CallSucc(EnableTrafficVisionFilter()) && CallSucc(HasEgo())) {
    TrafficFlow::TAD_EgoVehicleElementPtr leader_ego_ptr = nullptr;
    // 主车获取
    auto refEgoArray = ElemMgr()->GetEgoArray();
    for (auto refEgoPtr : refEgoArray) {
      TrafficFlow::TAD_EgoVehicleElementPtr curEgoPtr =
          std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(refEgoPtr);
      // 如果curEgoPtr非空且curEgoPtr的子类型为领导者
      if (NonNull_Pointer(curEgoPtr) && (_plus_(Base::Enums::EgoSubType::eLeader)) == (curEgoPtr->GetSubType())) {
        // 将curEgoPtr赋值给leader_ego_ptr
        leader_ego_ptr = curEgoPtr;
      }
    }
    if (NonNull_Pointer(leader_ego_ptr)) {
      const Base::txFloat ego_alt = leader_ego_ptr->Altitude();
      LogInfo << "[ego_altitude_filter]" << TX_VARS(timeMgr) << TX_VARS(ego_alt);
      sim_msg::Traffic all_region_traffic;
      ParentClass::FillingTrafficData(timeMgr, all_region_traffic);
      tbb::concurrent_unordered_set<Base::txSysId> con_set_vehicle_id;
      const Base::txFloat FilterRadius = TrafficVisionFilterRadius();
      const Base::txFloat FilterAltitudeDiff = TrafficVisionFilterAltitudeDiff();
      const Coord::txENU &ego_enu_pt = leader_ego_ptr->StableGeomCenter();

      outTraffic.Clear();
      // 提取vehicle信息
      {
        // 获取所有车辆元素的数组
        auto refAllVehicleArray = ElemMgr()->GetAllVehiclePtr();
        // 使用TBB库的并行for循环遍历所有车辆元素
        tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray.size(), [&](const std::size_t idx) {
          if (NonNull_Pointer(refAllVehicleArray[idx])) {
            const Base::txFloat veh_alt = refAllVehicleArray[idx]->Altitude();
            const Coord::txENU &veh_enu_pt = refAllVehicleArray[idx]->StableGeomCenter();
            // 计算车辆元素与自车元素的距离
            const Base::txFloat dist = Coord::txENU::EuclideanDistanceLocal_2D(ego_enu_pt, veh_enu_pt);
            // 如果车辆元素与自车元素的距离小于过滤半径且高度差小于过滤高度差
            if ((dist < FilterRadius) && (fabs(veh_alt - ego_alt) < FilterAltitudeDiff)) {
              // 将车辆元素的ID插入到con_set_vehicle_id集合中
              con_set_vehicle_id.insert(refAllVehicleArray[idx]->Id());
            }
          }
        }); /*lamda function*/
            /* parallel_for */

        // 遍历所有车辆元素，输出车辆元素的ID、高度差和高度
        for (auto veh_ptr : refAllVehicleArray) {
          LogInfo << TX_VARS(veh_ptr->Id()) << TX_VARS(fabs((veh_ptr->Altitude()) - ego_alt))
                  << TX_VARS((veh_ptr->Altitude()));
        }
        // 遍历all_region_traffic中的所有车辆
        for (const auto &refCar : all_region_traffic.cars()) {
          // 如果车辆的ID在con_set_vehicle_id集合中
          if (CallSucc(con_set_vehicle_id.count(refCar.id()) > 0)) {
            outTraffic.add_cars()->CopyFrom(refCar);
          }
        }
      }
      // 提取pedstrains信息
      {
        std::set<Base::txSysId> set_ped_id;
        // 获取所有行人元素的数组
        const auto refAllPedestrianArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Pedestrian);
        // 遍历所有行人元素
        for (auto refPede : refAllPedestrianArray) {
          // 获取行人元素的高度
          const Base::txFloat ped_alt = refPede->Altitude();
          const Coord::txENU &ped_enu_pt = refPede->StableGeomCenter();
          // 计算行人元素与自车元素的距离
          const Base::txFloat dist = Coord::txENU::EuclideanDistanceLocal_2D(ego_enu_pt, ped_enu_pt);
          // 如果行人元素与自车元素的距离小于过滤半径且高度差小于过滤高度差
          if ((dist < FilterRadius) && (fabs(ped_alt - ego_alt) < FilterAltitudeDiff)) {
            // 将行人元素的ID插入到set_ped_id集合中
            set_ped_id.insert(refPede->Id() * -1);
          }
        }

        // 遍历all_region_traffic中的所有行人
        for (const auto &refPed : all_region_traffic.dynamicobstacles()) {
          if (CallSucc(set_ped_id.count(refPed.id()) > 0)) {
            // 将行人信息添加到outTraffic中
            outTraffic.add_dynamicobstacles()->CopyFrom(refPed);
          }
        }
      }

      // 提取obstacle信息
      {
        std::set<Base::txSysId> set_obs_id;
        const auto refAllObstacleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Immovability);
        // 遍历所有静态障碍物元素
        for (auto refObs : refAllObstacleArray) {
          // 获取静态障碍物元素的高度
          const Base::txFloat obs_alt = refObs->Altitude();
          // 获取静态障碍物元素的位置
          const Coord::txENU &obs_enu_pt = refObs->StableGeomCenter();
          // 计算静态障碍物元素与自车元素的距离
          const Base::txFloat dist = Coord::txENU::EuclideanDistanceLocal_2D(ego_enu_pt, obs_enu_pt);
          // 如果静态障碍物元素与自车元素的距离小于过滤半径且高度差小于过滤高度差
          if ((dist < FilterRadius) && (fabs(obs_alt - ego_alt) < FilterAltitudeDiff)) {
            // 将静态障碍物元素的ID插入到set_obs_id集合中
            set_obs_id.insert(refObs->Id());
          }
        }
        // 遍历all_region_traffic中的所有静态障碍物
        for (const auto &refObs : all_region_traffic.staticobstacles()) {
          if (CallSucc(set_obs_id.count(refObs.id()) > 0)) {
            // 将静态障碍物信息添加到outTraffic中
            outTraffic.add_staticobstacles()->CopyFrom(refObs);
          }
        }
      }

      {
        for (const auto &refSignal : all_region_traffic.trafficlights()) {
          // 将信号灯信息添加到outTraffic中
          outTraffic.add_trafficlights()->CopyFrom(refSignal);
        }
      }
      return true;
    } else {
      LogWarn << "system have ego, but do not have leader.ego.";
      return ParentClass::FillingTrafficData(timeMgr, outTraffic);
    }
  } else {
    return ParentClass::FillingTrafficData(timeMgr, outTraffic);
  }
}

#if __TX_Mark__("SceneSketch")

Base::txString TAD_StandAlone_TrafficElementSystem::SketchNode::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  // 将SketchNode对象的类型和源元素ID添加到字符串流中
  oss << "{" << TX_VARS_NAME(type, __enum2lpsz__(SketchEnumType, type))
      << TX_VARS_NAME(srcId, __SafeElemId__(source_ptr));
  // 遍历SketchNode对象的参与者列表
  for (auto ptr : participants) {
    // 将参与者元素ID添加到字符串流中
    oss << TX_VARS_NAME(Onlooker, __SafeElemId__(ptr));
  }
  oss << "}";
  return oss.str();
}

// 定义一个函数，用于初始化Sketch类型数组
void TAD_StandAlone_TrafficElementSystem::InitSketch() TX_NOEXCEPT {
  // 遍历Sketch类型数组的所有元素
  for (Base::txSize idx = 0; idx < SketchTypeSize(); ++idx) {
    // 将数组中的元素设置为对应的Sketch类型
    m_array_enum_type[idx] = __idx2enum__(SketchEnumType, idx);
  }
}

void TAD_StandAlone_TrafficElementSystem::ReleaseSketch() TX_NOEXCEPT { m_KeyFrameVec.clear(); }

Base::txSize TAD_StandAlone_TrafficElementSystem::UpdateSketch(const Base::TimeParamManager &timeMgr,
                                                               const sim_msg::Traffic &refTraffic) TX_NOEXCEPT {
  // 定义一个指针，用于存储自车元素
  Base::IVehicleElementPtr ego_elem_ptr = nullptr;
  if (NonNull_Pointer(ElemMgr()) && _NonEmpty_(ElemMgr()->GetEgoArray())) {
    // 获取自车元素
    ego_elem_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(ElemMgr()->GetEgoArray().front());
    // 如果自车元素非空
    if (NonNull_Pointer(ego_elem_ptr)) {
      // 更新自车元素的周围车辆信息
      ego_elem_ptr->UpdateSketchSurroundingVehicle(timeMgr);
      m_KeyFrameVec.emplace_back(SketchNodeCollection());
      // 获取最后一个SketchNodeCollection对象的引用
      SketchNodeCollection &cur_frame = m_KeyFrameVec.back();
      cur_frame.passTime = timeMgr.PassTime();
      // 将自车元素的位置信息复制到当前帧的snapshot_ego中
      cur_frame.snapshot_ego.CopyFrom(m_pb_location);
      cur_frame.snapshot_traffic.CopyFrom(refTraffic);
      // 清空当前帧的conVec_sketch_nodes列表
      cur_frame.conVec_sketch_nodes.clear();
      // 使用并行for循环遍历Sketch类型数组
      tbb::parallel_for(static_cast<std::size_t>(1), SketchTypeSize(), [&](const std::size_t idx) {
        // 检查当前Sketch类型，并将结果添加到当前帧的conVec_sketch_nodes列表中
        CheckSketch(m_array_enum_type[idx], cur_frame, ElemMgr(), ego_elem_ptr);
      }); /*lamda function*/
          /* parallel_for */

      return cur_frame.conVec_sketch_nodes.size();
    }
  }
  return 0;
}

TAD_StandAlone_TrafficElementSystem::SketchEnumType TAD_StandAlone_TrafficElementSystem::SketchVoting() const
    TX_NOEXCEPT {
  std::array<Base::txInt, Base::Enums::szSketchEnumType> voting_bucket;
  // 初始化投票数组，将所有元素设置为0
  for (auto &refVoting : voting_bucket) {
    refVoting = 0;
  }

  // 遍历KeyFrameVec中的所有帧
  for (const auto &refFrame : m_KeyFrameVec) {
    // 遍历每帧中的所有Sketch类型
    for (const auto &refType : refFrame.conVec_sketch_nodes) {
      // 将Sketch类型转换为整数，并在投票数组中对应的位置上加1
      const auto typeInt = __enum2int__(Base::Enums::SketchEnumType, refType.type);
      voting_bucket.at(typeInt)++;
    }
  }
  // 如果启用了Sketch日志记录
  if (FLAGS_LogLevel_Sketch) {
    std::ostringstream oss;
    // 遍历投票数组，将每种Sketch类型的投票数添加到字符串流中
    for (int idx = 0; idx < voting_bucket.size(); ++idx) {
      oss << (__int2enum__(SketchEnumType, idx))._to_string() << TX_VARS_NAME(vote, voting_bucket[idx]) << std::endl;
    }
    // 将字符串流转换为字符串并输出到日志中
    LOG(INFO) << "#######################\n" << oss.str() << "\n############################";
  }
  auto itr = std::max_element(std::begin(voting_bucket), std::end(voting_bucket));
  // 如果找到了最大值
  if (std::end(voting_bucket) != itr) {
    LOG(INFO) << TX_VARS_NAME(SketchEnumType_Integer, (itr - std::begin(voting_bucket)));
    return __int2enum__(SketchEnumType, (itr - std::begin(voting_bucket)));
  } else {
    // 如果没有找到最大值，调用父类的SketchVoting函数并返回结果
    return ParentClass::SketchVoting();
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch(const SketchEnumType _type, SketchNodeCollection &refCollection,
                                                      Base::IElementManagerPtr _elemMgr,
                                                      Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT{
#  define CaseCheckSketch(_type_id_)                                  \
    case _type_id_: {                                                 \
      CheckSketch_##_type_id_(refCollection, _elemMgr, ego_elem_ptr); \
      break;                                                          \
    }
    // if (20230703 == version)
    {const Base::txInt typeId = _type._to_integral();
// 使用switch语句根据Sketch类型调用相应的检查函数
switch (typeId) {
  CaseCheckSketch(1) CaseCheckSketch(2) CaseCheckSketch(3) CaseCheckSketch(4) CaseCheckSketch(5) CaseCheckSketch(6)
      CaseCheckSketch(7) CaseCheckSketch(8) CaseCheckSketch(9) CaseCheckSketch(10) CaseCheckSketch(11)
          CaseCheckSketch(12) CaseCheckSketch(13) CaseCheckSketch(14) CaseCheckSketch(15) CaseCheckSketch(16)
              CaseCheckSketch(17) CaseCheckSketch(18) CaseCheckSketch(19) CaseCheckSketch(20) CaseCheckSketch(21)
                  CaseCheckSketch(22) CaseCheckSketch(23) default : {
    LogWarn << "unsupport scene enum type:" << _type._to_string();
    break;
  }
}
}
#  undef CaseCheckSketch
}

Base::IVehicleElementPtr TAD_StandAlone_TrafficElementSystem::QueryVehiclePtr(Base::IElementManagerPtr _elemMgr,
                                                                              const Base::txSysId vehId) TX_NOEXCEPT {
  // 定义一个指针，用于存储车辆元素
  Base::IVehicleElementPtr veh_ptr = nullptr;
  if (NonNull_Pointer(_elemMgr)) {
    // 根据车辆ID查询车辆元素
    auto elem_ptr = _elemMgr->GetVehiclePtrById(vehId);
    // 将查询到的车辆元素转换为IVehicleElement类型的指针
    veh_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(elem_ptr);
  }
  return veh_ptr;
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_1(SketchNodeCollection &refCollection,
                                                        Base::IElementManagerPtr _elemMgr,
                                                        Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*weather_rain*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::weather_rain);
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_2(SketchNodeCollection &refCollection,
                                                        Base::IElementManagerPtr _elemMgr,
                                                        Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*roadsurface_water*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::roadsurface_water);
  auto objs = _elemMgr->GetAllObstaclePtr();
  // 遍历所有障碍物元素
  for (auto objPtr : objs) {
    // 将障碍物元素转换为TAD_MapObstacleElement类型的指针
    auto map_obj_elem_ptr = std::dynamic_pointer_cast<TrafficFlow::TAD_MapObstacleElement>(objPtr);
    // 如果自车元素和障碍物元素非空，且障碍物元素的名称为"Stagnant_Water"
    if (NonNull_Pointer(ego_elem_ptr) && NonNull_Pointer(map_obj_elem_ptr) &&
        Base::txString("Stagnant_Water") == (map_obj_elem_ptr->GetObjectName())) {
      // 计算自车元素和障碍物元素之间的距离
      const Base::txFloat dist = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(ego_elem_ptr->GetLocation(),
                                                                                    map_obj_elem_ptr->GetLocation());
      // 如果距离小于FLAGS_roadsurface_water_radius
      if (dist < FLAGS_roadsurface_water_radius) {
        SketchNode tmpNode;
        tmpNode.type = cur_enum_type;
        tmpNode.source_ptr = objPtr;
        // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
        refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
        SketchInfo << "[roadsurface_water][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime) << TX_VARS(dist);
      } else {
        LogWarn << "[roadsurface_water][fail]" << TX_VARS_NAME(PassTime, refCollection.passTime) << TX_VARS(dist);
      }
    }
  }
}
void TAD_StandAlone_TrafficElementSystem::CheckSketch_3(SketchNodeCollection &refCollection,
                                                        Base::IElementManagerPtr _elemMgr,
                                                        Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*traffic_signs*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::traffic_signs);
  const Base::txVec2 ego_pos_2d = ego_elem_ptr->GetLocation().ToENU().ENU2D();
  // 定义一个hadmap::txObjects对象，用于存储查询结果
  hadmap::txObjects res;
  // 调用RTree2D_Signal的FindSignInCircle函数，查询范围内的交通标志
  if (CallSucc(
          Geometry::SpatialQuery::RTree2D_Signal::getInstance().FindSignInCircle(ego_pos_2d, FLAGS_sign_radius, res))) {
    SketchNode tmpNode;
    tmpNode.type = cur_enum_type;
    // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
    refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
    SketchInfo << "[traffic_signs][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime);
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_4(SketchNodeCollection &refCollection,
                                                        Base::IElementManagerPtr _elemMgr,
                                                        Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*surrounding_cutin*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::surrounding_cutin);
  // 获取自车元素左前方的距离
  const Base::Info_NearestObject &refEgoSurroundingInfo = ego_elem_ptr->SketchSurroundingInfo();
  {
    const Base::txFloat left_front_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nLeftFront);
    Base::SimulationConsistencyPtr left_front_elem_ptr =
        refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nLeftFront);
    // 如果左前方的距离小于FLAGS_DETECT_OBJECT_DISTANCE且左前方的元素指针非空
    if ((left_front_dist < FLAGS_DETECT_OBJECT_DISTANCE) && NonNull_Pointer(left_front_elem_ptr)) {
      // 获取左前方元素的ID
      const Base::txSysId elemId = left_front_elem_ptr->ConsistencyId();
      Base::IVehicleElementPtr vehiclePtr = QueryVehiclePtr(_elemMgr, elemId);

      // 如果车辆元素指针非空且车辆元素正在右转
      if (NonNull_Pointer(vehiclePtr) && vehiclePtr->IsTurnRight()) {
        SketchNode tmpNode;
        tmpNode.type = cur_enum_type;
        tmpNode.source_ptr = vehiclePtr;
        // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
        refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
        SketchInfo << "[surrounding_cutin][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                   << TX_VARS_NAME(LeftFront, elemId);
      }
    }
  }
  {
    const Base::txFloat right_front_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nRightFront);
    Base::SimulationConsistencyPtr right_front_elem_ptr =
        refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nRightFront);
    // 如果右前方的距离小于FLAGS_DETECT_OBJECT_DISTANCE且右前方的元素指针非空
    if ((right_front_dist < FLAGS_DETECT_OBJECT_DISTANCE) && NonNull_Pointer(right_front_elem_ptr)) {
      const Base::txSysId elemId = right_front_elem_ptr->ConsistencyId();
      Base::IVehicleElementPtr vehiclePtr = QueryVehiclePtr(_elemMgr, elemId);
      // 如果车辆元素指针非空且车辆元素正在左转
      if (NonNull_Pointer(vehiclePtr) && vehiclePtr->IsTurnLeft()) {
        SketchNode tmpNode;
        tmpNode.type = cur_enum_type;
        tmpNode.source_ptr = vehiclePtr;
        // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
        refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
        SketchInfo << "[surrounding_cutin][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                   << TX_VARS_NAME(RightFront, elemId);
      }
    }
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_5(SketchNodeCollection &refCollection,
                                                        Base::IElementManagerPtr _elemMgr,
                                                        Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*ego_cutin*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::ego_cutin);
  const Base::txBool isEgoLaneChange = ego_elem_ptr->IsLaneChange();
  // 如果自车元素正在变道
  if (CallSucc(isEgoLaneChange)) {
    // 获取自车元素的周围信息
    const Base::Info_NearestObject &refEgoSurroundingInfo = ego_elem_ptr->SketchSurroundingInfo();
    // 获取自车元素右后方的距离
    const Base::txFloat right_back_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nRightBack);
    // 获取自车元素左后方的距离
    const Base::txFloat left_back_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nLeftBack);
    // 获取自车元素右前方的距离
    const Base::txFloat right_front_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nRightFront);
    // 获取自车元素左前方的距离
    const Base::txFloat left_front_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nLeftFront);
    // 判断自车元素是否正在左转
    const Base::txBool isTurnLeft = ego_elem_ptr->IsTurnLeft();
    // 判断自车元素是否正在右转
    const Base::txBool isTurnRight = ego_elem_ptr->IsTurnRight();
    // 如果自车元素正在右转且右后方或右前方的距离小于FLAGS_DETECT_OBJECT_DISTANCE，或者自车元素正在左转且左后方或左前方的距离小于FLAGS_DETECT_OBJECT_DISTANCE
    if ((CallSucc(isTurnRight) &&
         (right_back_dist < FLAGS_DETECT_OBJECT_DISTANCE || right_front_dist < FLAGS_DETECT_OBJECT_DISTANCE)) ||
        (CallSucc(isTurnLeft) &&
         (left_back_dist < FLAGS_DETECT_OBJECT_DISTANCE || left_front_dist < FLAGS_DETECT_OBJECT_DISTANCE))) {
      // 创建一个SketchNode对象
      SketchNode tmpNode;
      // 设置SketchNode的类型为ego_cutin
      tmpNode.type = cur_enum_type;
      // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
      refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
      SketchInfo << "[ego_cutin][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime) << TX_COND(isTurnRight)
                 << TX_COND(isTurnLeft) << TX_VARS(right_back_dist) << TX_VARS(left_back_dist)
                 << TX_VARS(right_front_dist) << TX_VARS(left_front_dist);
    } else {
      // 如果不满足条件，输出失败信息
      SketchInfo << "[ego_cutin][failure]" << TX_VARS_NAME(PassTime, refCollection.passTime) << TX_COND(isTurnRight)
                 << TX_COND(isTurnLeft) << TX_VARS(right_back_dist) << TX_VARS(left_back_dist)
                 << TX_VARS(right_front_dist) << TX_VARS(left_front_dist);
    }
  } else {
    SketchInfo << "[ego_cutin][failure] ego straight." << TX_VARS_NAME(PassTime, refCollection.passTime);
  }
}
void TAD_StandAlone_TrafficElementSystem::CheckSketch_6(SketchNodeCollection &refCollection,
                                                        Base::IElementManagerPtr _elemMgr,
                                                        Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*surrounding_cutout*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::surrounding_cutout);
  // 获取自车元素的周围信息
  const Base::Info_NearestObject &refEgoSurroundingInfo = ego_elem_ptr->SketchSurroundingInfo();
  // 获取自车元素前方的元素指针
  auto front_elem_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nFront);
  // 获取自车元素后方的元素指针
  auto rear_elem_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nBack);
  // 如果前方元素指针非空
  if (NonNull_Pointer(front_elem_ptr)) {
    auto veh_ptr = QueryVehiclePtr(_elemMgr, front_elem_ptr->ConsistencyId());
    // 如果车辆元素指针非空且车辆元素正在变道
    if (NonNull_Pointer(veh_ptr) && CallSucc(veh_ptr->IsLaneChange())) {
      SketchNode tmpNode;
      // 设置SketchNode的类型为surrounding_cutout
      tmpNode.type = cur_enum_type;
      tmpNode.source_ptr = veh_ptr;
      // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
      refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
      SketchInfo << "[surrounding_cutout][succ][front]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_VARS_NAME(Id, veh_ptr->Id());
    }
  }
  // 如果后方元素指针非空
  if (NonNull_Pointer(rear_elem_ptr)) {
    auto veh_ptr = QueryVehiclePtr(_elemMgr, rear_elem_ptr->ConsistencyId());
    // 如果车辆元素指针非空且车辆元素正在变道
    if (NonNull_Pointer(veh_ptr) && CallSucc(veh_ptr->IsLaneChange())) {
      SketchNode tmpNode;
      tmpNode.type = cur_enum_type;
      tmpNode.source_ptr = veh_ptr;
      // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
      refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
      SketchInfo << "[surrounding_cutout][succ][rear]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_VARS_NAME(Id, veh_ptr->Id());
    }
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_7(SketchNodeCollection &refCollection,
                                                        Base::IElementManagerPtr _elemMgr,
                                                        Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*surrounding_speedup*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::surrounding_speedup);
  const Base::Info_NearestObject &refEgoSurroundingInfo = ego_elem_ptr->SketchSurroundingInfo();
  // 获取自车元素前后方元素指针
  auto front_elem_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nFront);
  auto rear_elem_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nBack);
  // 如果前方元素指针非空
  if (NonNull_Pointer(front_elem_ptr)) {
    auto veh_ptr = QueryVehiclePtr(_elemMgr, front_elem_ptr->ConsistencyId());
    // 如果车辆元素指针非空且车辆元素的加速度大于0
    if (NonNull_Pointer(veh_ptr) && CallSucc(veh_ptr->GetAcc() > 0.0)) {
      SketchNode tmpNode;
      // 设置SketchNode的类型为surrounding_speedup
      tmpNode.type = cur_enum_type;
      tmpNode.source_ptr = veh_ptr;
      // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
      refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
      SketchInfo << "[surrounding_speedup][succ][front]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_VARS_NAME(Id, veh_ptr->Id()) << TX_VARS_NAME(acc, veh_ptr->GetAcc());
    }
  }
  // 如果后方元素指针非空
  if (NonNull_Pointer(rear_elem_ptr)) {
    auto veh_ptr = QueryVehiclePtr(_elemMgr, rear_elem_ptr->ConsistencyId());
    // 如果车辆元素指针非空且车辆元素的加速度大于0
    if (NonNull_Pointer(veh_ptr) && CallSucc(veh_ptr->GetAcc() > 0.0)) {
      SketchNode tmpNode;
      // 设置SketchNode的类型为surrounding_speedup
      tmpNode.type = cur_enum_type;
      tmpNode.source_ptr = veh_ptr;
      // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
      refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
      SketchInfo << "[surrounding_speedup][succ][rear]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_VARS_NAME(Id, veh_ptr->Id()) << TX_VARS_NAME(acc, veh_ptr->GetAcc());
    }
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_8(SketchNodeCollection &refCollection,
                                                        Base::IElementManagerPtr _elemMgr,
                                                        Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*surrounding_speedreduction*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::surrounding_speedreduction);
  const Base::Info_NearestObject &refEgoSurroundingInfo = ego_elem_ptr->SketchSurroundingInfo();
  // 获取自车元素前后方元素指针
  auto front_elem_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nFront);
  auto rear_elem_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nBack);
  // 前方元素指针不为空
  if (NonNull_Pointer(front_elem_ptr)) {
    auto veh_ptr = QueryVehiclePtr(_elemMgr, front_elem_ptr->ConsistencyId());
    // 前车元素减速
    if (NonNull_Pointer(veh_ptr) && CallSucc(veh_ptr->GetAcc() < 0.0)) {
      SketchNode tmpNode;
      tmpNode.type = cur_enum_type;
      tmpNode.source_ptr = veh_ptr;
      refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
      SketchInfo << "[surrounding_speedreduction][succ][front]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_VARS_NAME(Id, veh_ptr->Id()) << TX_VARS_NAME(acc, veh_ptr->GetAcc());
    }
  }
  // 后车元素指针不为空
  if (NonNull_Pointer(rear_elem_ptr)) {
    auto veh_ptr = QueryVehiclePtr(_elemMgr, rear_elem_ptr->ConsistencyId());
    // 后车减速
    if (NonNull_Pointer(veh_ptr) && CallSucc(veh_ptr->GetAcc() < 0.0)) {
      SketchNode tmpNode;
      // 当前类型设置
      tmpNode.type = cur_enum_type;
      tmpNode.source_ptr = veh_ptr;
      // 添加
      refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
      SketchInfo << "[surrounding_speedreduction][succ][rear]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_VARS_NAME(Id, veh_ptr->Id()) << TX_VARS_NAME(acc, veh_ptr->GetAcc());
    }
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_9(SketchNodeCollection &refCollection,
                                                        Base::IElementManagerPtr _elemMgr,
                                                        Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*surrounding_stop*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::surrounding_stop);
  const Base::Info_NearestObject &refEgoSurroundingInfo = ego_elem_ptr->SketchSurroundingInfo();
  // 获取前面元素指针并检查
  auto front_elem_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nFront);
  if (NonNull_Pointer(front_elem_ptr)) {
    auto veh_ptr = QueryVehiclePtr(_elemMgr, front_elem_ptr->ConsistencyId());
    // 前面车辆不为空且速度停止
    if (NonNull_Pointer(veh_ptr) && CallSucc(Math::isZero(veh_ptr->GetVelocity()))) {
      SketchNode tmpNode;
      // 设置skech类型
      tmpNode.type = cur_enum_type;
      tmpNode.source_ptr = veh_ptr;
      refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
      SketchInfo << "[surrounding_stop][succ][front]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_VARS_NAME(Id, veh_ptr->Id()) << TX_VARS_NAME(acc, veh_ptr->GetAcc());
    }
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_10(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*surrounding_cross*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::surrounding_cross);
  // 获取自车元素的周围元素指针列表
  const std::vector<Base::ITrafficElementPtr> &circleElemVec = ego_elem_ptr->GetSketchCircleElementPtrVec();

  // 获取自车元素的2D坐标 车道方向
  const Base::txVec2 vPos2d = ego_elem_ptr->StableGeomCenter().ENU2D();
  const Base::txVec3 &egoDir = ego_elem_ptr->StableLaneDir();
  // 遍历周围元素指针列表
  for (auto elemPtr : circleElemVec) {
    // 如果元素指针非空且元素类型为TAD_Vehicle
    if (NonNull_Pointer(elemPtr) && (_plus_(Base::Enums::ElementType::TAD_Vehicle) == elemPtr->Type())) {
      const Base::txVec2 nearestElementCenter2d = elemPtr->StableGeomCenter().ENU2D();
      const Base::txVec2 local_coord_other =
          ego_elem_ptr->RelativePosition(vPos2d, nearestElementCenter2d, ego_elem_ptr->TransMatInv());
      // 如果元素在自车元素前方
      if (local_coord_other.y() > 0.0) {
        /*front*/
        const Base::txVec3 &elemDir = elemPtr->GetLaneDir();
        // 如果自车元素和元素的车道方向不同步
        if (CallFail(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(elemDir)))) {
          SketchNode tmpNode;
          // 设置SketchNode的类型为surrounding_cross
          tmpNode.type = cur_enum_type;
          tmpNode.source_ptr = elemPtr;
          // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
          refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
          SketchInfo << "[surrounding_cross][succ][front]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                     << TX_VARS_NAME(Id, elemPtr->Id())
                     << TX_VARS_NAME(elemDir, Utils::ToString(elemPtr->GetLaneDir()));
        }
      }
    }
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_11(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*ego_3_2_lanechange*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::ego_3_2_lanechange);
  const Base::txBool isEgoLaneChange = ego_elem_ptr->IsLaneChange();
  // 如果自车元素正在变道
  if (CallSucc(isEgoLaneChange)) {
    // 获取自车元素当前所在的车道信息
    const auto &cur_loc_info = ego_elem_ptr->GetCurrentLaneInfo();
    // 如果自车元素在车道上
    if (cur_loc_info.IsOnLane()) {
      // 获取自车元素所在车道的指针
      auto lane_ptr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(cur_loc_info.onLaneUid);
      // 获取自车元素所在道路的车道段数量
      const Base::txSize nSecSize = HdMap::HadmapCacheConCurrent::GetSectionCountOnRoad(cur_loc_info.onLaneUid.roadId);
      // 获取自车元素所在车道段的Uid
      const Base::txSectionUId curSecUid = Utils::LaneUid2SectionUid(cur_loc_info.onLaneUid);
      const Base::txSectionUId nextSecUid =
          Utils::MakeSectionUid(cur_loc_info.onLaneUid.roadId, Utils::NextSectionId(cur_loc_info.onLaneUid.sectionId));

      // 如果自车元素所在车道非空，且自车元素未到达道路的最后一个车道段，且自车元素距离车道段末尾的距离小于FLAGS_ego_3_2_lanechange_dist，且当前车道段的车道数量大于下一个车道段的车道数量
      if (NonNull_Pointer(lane_ptr) && CallFail(Utils::IsReachedRoadLastSection(lane_ptr)) &&
          ((lane_ptr->getLength() - ego_elem_ptr->DistanceAlongCurve()) < FLAGS_ego_3_2_lanechange_dist) &&
          (HdMap::HadmapCacheConCurrent::GetLaneCountOnSection(curSecUid) >
           HdMap::HadmapCacheConCurrent::GetLaneCountOnSection(nextSecUid))) {
        SketchNode tmpNode;
        tmpNode.type = cur_enum_type;
        // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
        refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
        SketchInfo << "[ego_3_2_lanechange][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                   << TX_VARS(cur_loc_info);
      } else {
        SketchInfo << "[ego_3_2_lanechange] . " << TX_VARS_NAME(PassTime, refCollection.passTime)
                   << TX_VARS(cur_loc_info);
      }
    } else {
      SketchInfo << "[ego_3_2_lanechange] ego on link. " << TX_VARS_NAME(PassTime, refCollection.passTime);
    }
  } else {
    SketchInfo << "[ego_3_2_lanechange] ego straight. " << TX_VARS_NAME(PassTime, refCollection.passTime);
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_12(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*ego_front_rear_lanechange*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::ego_front_rear_lanechange);
  // 获取自车元素是否正在变道的信息
  const Base::txBool isEgoLaneChange = ego_elem_ptr->IsLaneChange();
  // 获取自车元素周围的信息
  const Base::Info_NearestObject &refEgoSurroundingInfo = ego_elem_ptr->SketchSurroundingInfo();
  // 获取自车元素前方的距离
  const Base::txFloat front_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nFront);
  // 如果自车元素正在变道且前方距离小于FLAGS_DETECT_OBJECT_DISTANCE
  if (CallSucc(isEgoLaneChange) && (front_dist < FLAGS_DETECT_OBJECT_DISTANCE)) {
    const Base::txFloat right_back_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nRightBack);
    const Base::txFloat left_back_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nLeftBack);
    const Base::txBool isTurnLeft = ego_elem_ptr->IsTurnLeft();
    const Base::txBool isTurnRight = ego_elem_ptr->IsTurnRight();

    // 如果自车元素正在向左转且左后方距离小于FLAGS_DETECT_OBJECT_DISTANCE，或者自车元素正在向右转且右后方距离小于FLAGS_DETECT_OBJECT_DISTANCE
    if ((isTurnLeft && left_back_dist < FLAGS_DETECT_OBJECT_DISTANCE) ||
        (isTurnRight && right_back_dist < FLAGS_DETECT_OBJECT_DISTANCE)) {
      SketchNode tmpNode;
      tmpNode.type = cur_enum_type;
      // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
      refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
      SketchInfo << "[ego_front_rear_lanechange][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_COND(isTurnLeft) << TX_COND(isTurnRight) << TX_VARS(right_back_dist) << TX_VARS(left_back_dist);
    } else {
      SketchInfo << "[ego_front_rear_lanechange] . " << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_COND(isTurnLeft) << TX_COND(isTurnRight) << TX_VARS(right_back_dist) << TX_VARS(left_back_dist);
    }
  } else {
    SketchInfo << "[ego_front_rear_lanechange] ego straight. " << TX_VARS_NAME(PassTime, refCollection.passTime);
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_13(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*ego_front_front_lanechange*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::ego_front_front_lanechange);
  // 获取自车元素是否正在变道的信息
  const Base::txBool isEgoLaneChange = ego_elem_ptr->IsLaneChange();
  // 获取自车元素周围的信息
  const Base::Info_NearestObject &refEgoSurroundingInfo = ego_elem_ptr->SketchSurroundingInfo();
  // 获取自车元素前方的距离
  const Base::txFloat front_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nFront);
  // 如果自车元素正在变道且前方距离小于FLAGS_DETECT_OBJECT_DISTANCE
  if (CallSucc(isEgoLaneChange) && (front_dist < FLAGS_DETECT_OBJECT_DISTANCE)) {
    // 获取自车元素左前方和右前方距离
    const Base::txFloat right_front_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nRightFront);
    const Base::txFloat left_front_dist = refEgoSurroundingInfo.dist(Base::Info_NearestObject::nLeftFront);
    // 获取自车元素是否左转或右转
    const Base::txBool isTurnLeft = ego_elem_ptr->IsTurnLeft();
    const Base::txBool isTurnRight = ego_elem_ptr->IsTurnRight();

    // 如果自车元素正在向左转或右转且距离小于FLAGS_DETECT_OBJECT_DISTANCE
    if ((isTurnLeft && left_front_dist < FLAGS_DETECT_OBJECT_DISTANCE) ||
        (isTurnRight && right_front_dist < FLAGS_DETECT_OBJECT_DISTANCE)) {
      SketchNode tmpNode;
      tmpNode.type = cur_enum_type;
      // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
      refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
      SketchInfo << "[ego_front_rear_lanechange][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_COND(isTurnLeft) << TX_COND(isTurnRight) << TX_VARS(left_front_dist)
                 << TX_VARS(right_front_dist);
    } else {
      SketchInfo << "[ego_front_front_lanechange] . " << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_COND(isTurnLeft) << TX_COND(isTurnRight) << TX_VARS(right_front_dist)
                 << TX_VARS(left_front_dist);
    }
  } else {
    SketchInfo << "[ego_front_front_lanechange] ego straight. " << TX_VARS_NAME(PassTime, refCollection.passTime);
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_14(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*surrounding_front_rear_frontlanechange*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::surrounding_front_rear_frontlanechange);
  const Base::Info_NearestObject &refEgoSurroundingInfo = ego_elem_ptr->SketchSurroundingInfo();
  auto front_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nFront);
  if (Null_Pointer(front_ptr)) {
    {
      // 获取自车元素左前方和左后方的元素指针
      auto left_front_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nLeftFront);
      auto left_back_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nLeftBack);
      // 如果左前方和左后方的元素指针非空
      if (NonNull_Pointer(left_front_ptr) && NonNull_Pointer(left_back_ptr)) {
        auto left_front_vehicle_ptr = QueryVehiclePtr(_elemMgr, left_front_ptr->ConsistencyId());
        // 如果左前方元素的车辆指针非空且左前方元素正在向右转
        if (NonNull_Pointer(left_front_vehicle_ptr) && CallSucc(left_front_vehicle_ptr->IsTurnRight())) {
          SketchNode tmpNode;
          // 设置SketchNode的类型为surrounding_front_rear_frontlanechange
          tmpNode.type = cur_enum_type;
          // 设置SketchNode的源指针为左前方元素的车辆指针
          tmpNode.source_ptr = left_front_vehicle_ptr;
          // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
          refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
          SketchInfo << "[surrounding_front_rear_frontlanechange][succ]"
                     << TX_VARS_NAME(PassTime, refCollection.passTime)
                     << TX_VARS_NAME(left_front_id, left_front_ptr->ConsistencyId())
                     << TX_VARS_NAME(left_back_id, left_back_ptr->ConsistencyId());
        } else {
          SketchInfo << "[surrounding_front_rear_frontlanechange][fail]"
                     << TX_VARS_NAME(PassTime, refCollection.passTime)
                     << TX_COND(NonNull_Pointer(left_front_vehicle_ptr));
        }
      } else {
        SketchInfo << "[surrounding_front_rear_frontlanechange][fail]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                   << TX_COND(NonNull_Pointer(left_front_ptr)) << TX_COND(NonNull_Pointer(left_back_ptr));
      }
    }
    {
      auto right_front_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nRightFront);
      auto right_back_ptr = refEgoSurroundingInfo.elemPtr(Base::Info_NearestObject::nRightBack);
      // 如果右前方和右后方的元素指针非空
      if (NonNull_Pointer(right_front_ptr) && NonNull_Pointer(right_back_ptr)) {
        auto right_front_vehicle_ptr = QueryVehiclePtr(_elemMgr, right_front_ptr->ConsistencyId());
        // 如果右前方元素的车辆指针非空且右前方元素正在向左转
        if (NonNull_Pointer(right_front_vehicle_ptr) && CallSucc(right_front_vehicle_ptr->IsTurnLeft())) {
          SketchNode tmpNode;
          tmpNode.type = cur_enum_type;
          // 设置SketchNode的源指针为右前方元素的车辆指针
          tmpNode.source_ptr = right_front_vehicle_ptr;
          // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
          refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
          SketchInfo << "[surrounding_front_rear_frontlanechange][succ]"
                     << TX_VARS_NAME(PassTime, refCollection.passTime)
                     << TX_VARS_NAME(right_front_id, right_front_ptr->ConsistencyId())
                     << TX_VARS_NAME(right_back_id, right_back_ptr->ConsistencyId());
        } else {
          SketchInfo << "[surrounding_front_rear_frontlanechange][fail]"
                     << TX_VARS_NAME(PassTime, refCollection.passTime)
                     << TX_COND(NonNull_Pointer(right_front_vehicle_ptr));
        }
      } else {
        SketchInfo << "[surrounding_front_rear_frontlanechange][fail]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                   << TX_COND(NonNull_Pointer(right_front_ptr)) << TX_COND(NonNull_Pointer(right_back_ptr));
      }
    }
  } else {
    SketchInfo << "[surrounding_front_rear_frontlanechange] ego front had vehicle. "
               << TX_VARS_NAME(PassTime, refCollection.passTime) << TX_VARS_NAME(frontId, front_ptr->ConsistencyId());
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_15(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*ped_front_cross*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::ped_front_cross);
  CheckSketch_Cross(refCollection, _elemMgr, ego_elem_ptr, Base::Enums::DynamicObstacle::IsHuman, cur_enum_type);
#  if 0
  const std::vector<Base::ITrafficElementPtr> &circleElemVec = ego_elem_ptr->GetCircleElementPtrVec();

  const Base::txVec2 vPos2d = ego_elem_ptr->StableGeomCenter().ENU2D();
  const Base::txVec3 &egoDir = ego_elem_ptr->StableLaneDir();

  for (auto elemPtr : circleElemVec) {
    if (NonNull_Pointer(elemPtr) && (_plus_(Base::Enums::ElementType::TAD_Pedestrian) == elemPtr->Type())) {
      Base::IPedestrianElementPtr ped_elem_ptr = std::dynamic_pointer_cast<Base::IPedestrianElement>(elemPtr);
      if (NonNull_Pointer(ped_elem_ptr) && Base::Enums::DynamicObstacle::IsHuman(ped_elem_ptr->PedestrianType())) {
        SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << " [ped_front_cross] check "
                   << TX_VARS_NAME(pedId, ped_elem_ptr->Id())
                   << TX_VARS_NAME(ped_type, ped_elem_ptr->PedestrianType()._to_string());
        const Base::txVec2 nearestElementCenter2d = elemPtr->StableGeomCenter().ENU2D();
        const Base::txVec2 local_coord_other =
            ego_elem_ptr->RelativePosition(vPos2d, nearestElementCenter2d, ego_elem_ptr->TransMatInv());
        if (local_coord_other.y() > 0.0) {
          /*front*/
          const Base::txVec3 &elemDir = elemPtr->GetLaneDir();
          if (CallFail(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(elemDir)))) {
            SketchNode tmpNode;
            tmpNode.type = cur_enum_type;
            tmpNode.source_ptr = elemPtr;
            refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
            SketchInfo << "[ped_front_cross][succ][front]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                       << TX_VARS_NAME(Id, elemPtr->Id())
                       << TX_VARS_NAME(elemDir, Utils::ToString(elemPtr->GetLaneDir()));
          }
        }
      }
    }
  }
#  endif
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_16(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*bicycle_front_cross*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::bicycle_front_cross);
  CheckSketch_Cross(refCollection, _elemMgr, ego_elem_ptr, Base::Enums::DynamicObstacle::IsBike, cur_enum_type);
#  if 0
    const std::vector< Base::ITrafficElementPtr >& circleElemVec = ego_elem_ptr->GetCircleElementPtrVec();

    const Base::txVec2 vPos2d = ego_elem_ptr->StableGeomCenter().ENU2D();
    const Base::txVec3& egoDir = ego_elem_ptr->StableLaneDir();

    for (auto elemPtr : circleElemVec) {
        if (NonNull_Pointer(elemPtr) && (_plus_(Base::Enums::ElementType::TAD_Pedestrian) == elemPtr->Type())) {
            Base::IPedestrianElementPtr ped_elem_ptr = std::dynamic_pointer_cast<Base::IPedestrianElement>(elemPtr);
            if (NonNull_Pointer(ped_elem_ptr) && Base::Enums::DynamicObstacle::IsBike(ped_elem_ptr->PedestrianType())) {
                SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << " [bicycle_front_cross] check "
                    << TX_VARS_NAME(pedId, ped_elem_ptr->Id())
                    << TX_VARS_NAME(ped_type, ped_elem_ptr->PedestrianType()._to_string());
                const Base::txVec2 nearestElementCenter2d = elemPtr->StableGeomCenter().ENU2D();
                const Base::txVec2 local_coord_other = ego_elem_ptr->RelativePosition(
                                                          vPos2d, nearestElementCenter2d, ego_elem_ptr->TransMatInv());
                if (local_coord_other.y() > 0.0) {
                    /*front*/
                    const Base::txVec3& elemDir = elemPtr->GetLaneDir();
                    if (CallFail(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(elemDir)))) {
                        SketchNode tmpNode;
                        tmpNode.type = cur_enum_type;
                        tmpNode.source_ptr = elemPtr;
                        refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
                        SketchInfo << "[bicycle_front_cross][succ][front]"
                                   << TX_VARS_NAME(PassTime, refCollection.passTime)
                                   << TX_VARS_NAME(Id, elemPtr->Id())
                                   << TX_VARS_NAME(elemDir, Utils::ToString(elemPtr->GetLaneDir()));
                    }
                }
            }
        }
    }
#  endif
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_Cross(SketchNodeCollection &refCollection,
                                                            Base::IElementManagerPtr _elemMgr,
                                                            Base::IVehicleElementPtr ego_elem_ptr,
                                                            PedCatalogFunc call_func,
                                                            const SketchEnumType cur_enum_type) TX_NOEXCEPT {
  const Base::txString SketchEnumTypeName = cur_enum_type._to_string();
  const std::vector<Base::ITrafficElementPtr> &circleElemVec = ego_elem_ptr->GetSketchCircleElementPtrVec();
  // 获取自车元素的2D坐标
  const Base::txVec2 vPos2d = ego_elem_ptr->GetLocation().ToENU().ENU2D();
  // 获取自车元素的稳定车道方向
  const Base::txVec3 &egoDir = ego_elem_ptr->StableLaneDir();

  // 遍历自车元素周围的元素列表
  for (auto elemPtr : circleElemVec) {
    // 如果元素指针非空且元素类型为行人
    if (NonNull_Pointer(elemPtr) && (_plus_(Base::Enums::ElementType::TAD_Pedestrian) == elemPtr->Type())) {
      Base::IPedestrianElementPtr ped_elem_ptr = std::dynamic_pointer_cast<Base::IPedestrianElement>(elemPtr);
      // 如果行人元素指针非空且行人类型满足条件
      if (NonNull_Pointer(ped_elem_ptr) && call_func(ped_elem_ptr->PedestrianType())) {
        SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << " [" << SketchEnumTypeName << "] check "
                   << TX_VARS_NAME(pedId, ped_elem_ptr->Id())
                   << TX_VARS_NAME(ped_type, ped_elem_ptr->PedestrianType()._to_string());
        const Base::txVec2 nearestElementCenter2d = elemPtr->StableGeomCenter().ENU2D();
        const Base::txVec2 local_coord_other =
            ego_elem_ptr->RelativePosition(vPos2d, nearestElementCenter2d, ego_elem_ptr->TransMatInv());
        // 如果元素在自车元素前方
        if (local_coord_other.y() > 0.0) {
          /*front*/
          const Base::txVec3 &elemDir = elemPtr->GetLaneDir();
          // 如果自车元素和元素的车道方向不同步
          if (CallFail(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(elemDir)))) {
            SketchNode tmpNode;
            tmpNode.type = cur_enum_type;
            // 设置SketchNode的源指针为元素指针
            tmpNode.source_ptr = elemPtr;
            refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
            SketchInfo << "[" << SketchEnumTypeName << "][succ][front]"
                       << TX_VARS_NAME(PassTime, refCollection.passTime) << TX_VARS_NAME(Id, elemPtr->Id())
                       << TX_VARS_NAME(elemDir, Utils::ToString(elemPtr->GetLaneDir()));
          }
        }
      }
    }
  }
}

std::vector<Base::ITrafficElementPtr> TAD_StandAlone_TrafficElementSystem::CheckSketch_Obs(
    SketchNodeCollection &refCollection, Base::IElementManagerPtr _elemMgr,
    Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  const Base::txBool isEgoLaneChange = ego_elem_ptr->IsLaneChange();
  // 获取自车当前所在车道的信息
  const Base::Info_Lane_t ego_locInfo = ego_elem_ptr->GetCurrentLaneInfo();
  std::vector<Base::ITrafficElementPtr> res_elem_array;
  // 如果自车正在变道且自车在车道上
  if (CallSucc(isEgoLaneChange) && CallSucc(ego_locInfo.IsOnLane())) {
    // 获取自车周围的元素列表
    const std::vector<Base::ITrafficElementPtr> &circleElemVec = ego_elem_ptr->GetSketchCircleElementPtrVec();
    // 获取自车元素的2D坐标
    const Base::txVec2 vPos2d = ego_elem_ptr->StableGeomCenter().ENU2D();
    const Base::txVec3 &egoDir = ego_elem_ptr->StableLaneDir();

    // 遍历自车周围的元素列表
    for (auto elemPtr : circleElemVec) {
      TrafficFlow::TAD_ObstacleElementPtr obs_elem_ptr =
          std::dynamic_pointer_cast<TrafficFlow::TAD_ObstacleElement>(elemPtr);
      // 如果元素指针非空且元素类型为障碍物
      if (NonNull_Pointer(elemPtr) && NonNull_Pointer(obs_elem_ptr) &&
          Base::Enums::StaticObstacle::IsObstacle(obs_elem_ptr->ObstacleType())) {
        SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << " [obs_front_egolanechange] check "
                   << TX_VARS_NAME(obsId, elemPtr->Id())
                   << TX_VARS_NAME(obs_type, obs_elem_ptr->ObstacleType()._to_string());
        const Base::txVec2 nearestElementCenter2d = elemPtr->StableGeomCenter().ENU2D();
        // 计算元素相对于自车元素的局部坐标
        const Base::txVec2 local_coord_other =
            ego_elem_ptr->RelativePosition(vPos2d, nearestElementCenter2d, ego_elem_ptr->TransMatInv());
        // 获取元素的车道信息
        const Base::Info_Lane_t obs_locInfo = elemPtr->GetCurrentLaneInfo();
        // 如果元素在自车元素前方且自车和元素在同一条道路上
        if ((local_coord_other.y() > 0.0) && (ego_locInfo.onLaneUid.roadId == obs_locInfo.onLaneUid.roadId)) {
          /*front*/
          res_elem_array.push_back(elemPtr);
          SketchInfo << "[obs_front_egolanechange][succ][front]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                     << TX_VARS_NAME(Id, elemPtr->Id()) << TX_VARS_NAME(elemDirLocInfo, obs_locInfo);
        }
      }
    }
  } else {
    SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << "[obs_front_egolanechange][fail]"
               << TX_COND(isEgoLaneChange);
  }
  return std::move(res_elem_array);
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_17(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*obs_front_egolanechange*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::obs_front_egolanechange);
  auto res_elem_array = CheckSketch_Obs(refCollection, _elemMgr, ego_elem_ptr);
  // 如果检测到的元素数量为1
  if (1 == res_elem_array.size()) {
    SketchNode tmpNode;
    // 设置SketchNode的类型为当前Sketch类型
    tmpNode.type = cur_enum_type;
    tmpNode.source_ptr = res_elem_array.front();
    // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
    refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
    SketchInfo << "[obs_front_egolanechange][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime)
               << TX_VARS_NAME(Id, tmpNode.source_ptr->Id());
  } else {
    SketchInfo << "[obs_front_egolanechange][fail]" << TX_VARS_NAME(PassTime, refCollection.passTime)
               << TX_VARS(res_elem_array.size());
  }
#  if 0
  const Base::txBool isEgoLaneChange = ego_elem_ptr->IsLaneChange();
  const Base::Info_Lane_t ego_locInfo = ego_elem_ptr->GetCurrentLaneInfo();
  std::vector<SketchNode> res_SketchNode_Array;
  if (CallSucc(isEgoLaneChange) && CallSucc(ego_locInfo.IsOnLane())) {
    const std::vector<Base::ITrafficElementPtr> &circleElemVec = ego_elem_ptr->GetCircleElementPtrVec();
    const Base::txVec2 vPos2d = ego_elem_ptr->StableGeomCenter().ENU2D();
    const Base::txVec3 &egoDir = ego_elem_ptr->StableLaneDir();

    for (auto elemPtr : circleElemVec) {
      TrafficFlow::TAD_ObstacleElementPtr obs_elem_ptr =
          std::dynamic_pointer_cast<TrafficFlow::TAD_ObstacleElement>(elemPtr);
      if (NonNull_Pointer(elemPtr) && NonNull_Pointer(obs_elem_ptr) &&
          Base::Enums::StaticObstacle::IsObstacle(obs_elem_ptr->ObstacleType())) {
        SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << " [obs_front_egolanechange] check "
                   << TX_VARS_NAME(obsId, elemPtr->Id())
                   << TX_VARS_NAME(obs_type, obs_elem_ptr->ObstacleType()._to_string());
        const Base::txVec2 nearestElementCenter2d = elemPtr->StableGeomCenter().ENU2D();
        const Base::txVec2 local_coord_other =
            ego_elem_ptr->RelativePosition(vPos2d, nearestElementCenter2d, ego_elem_ptr->TransMatInv());
        const Base::Info_Lane_t obs_locInfo = elemPtr->GetCurrentLaneInfo();
        if ((local_coord_other.y() > 0.0) && (ego_locInfo.onLaneUid.roadId == obs_locInfo.onLaneUid.roadId)) {
          /*front*/
          SketchNode tmpNode;
          tmpNode.type = cur_enum_type;
          tmpNode.source_ptr = elemPtr;
          res_SketchNode_Array.emplace_back(tmpNode);
          SketchInfo << "[obs_front_egolanechange][succ][front]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                     << TX_VARS_NAME(Id, elemPtr->Id()) << TX_VARS_NAME(elemDirLocInfo, obs_locInfo);
        }
      }
    }

    if (1 == res_SketchNode_Array.size()) {
      refCollection.conVec_sketch_nodes.emplace_back(res_SketchNode_Array.front());
      SketchInfo << "[obs_front_egolanechange][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_VARS_NAME(Id, res_SketchNode_Array.front().source_ptr->Id());
    } else {
      SketchInfo << "[obs_front_egolanechange][fail]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                 << TX_VARS(res_SketchNode_Array.size());
    }
  } else {
    SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << "[obs_front_egolanechange][fail]"
               << TX_COND(isEgoLaneChange);
  }
#  endif
}

/*
BETTER_ENUM(SketchEnumType, txInt, default = 0, weather_rain = 1, roadsurface_water = 2, traffic_signs = 3,
    surrounding_cutin = 4, ego_cutin = 5, surrounding_cutout = 6, surrounding_speedup = 7, surrounding_speedreduction =
8, surrounding_stop = 9, surrounding_cross = 10, ego_3_2_lanechange = 11,ego_front_rear_lanechange = 12,
ego_front_front_lanechange = 13, surrounding_front_rear_frontlanechange = 14, ped_front_cross = 15, bicycle_front_cross
= 16, obs_front_egolanechange = 17, multi_obs_front_egolanechange = 18, junction_opposite_turnleft = 19,
junction_opposite_turnright = 20, junction_opposite_straight = 21, junction_opposite_turnU = 22, ego_parking = 23)
*/
void TAD_StandAlone_TrafficElementSystem::CheckSketch_18(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*multi_obs_front_egolanechange*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::multi_obs_front_egolanechange);
  auto res_elem_array = CheckSketch_Obs(refCollection, _elemMgr, ego_elem_ptr);
  // 如果检测到的元素数量大于1
  if (1 < res_elem_array.size()) {
    SketchNode tmpNode;
    tmpNode.type = cur_enum_type;
    // 设置SketchNode的源指针为检测到的元素指针
    tmpNode.source_ptr = res_elem_array.front();
    // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
    refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
    SketchInfo << "[multi_obs_front_egolanechange][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime)
               << TX_VARS_NAME(Id, tmpNode.source_ptr->Id());
  } else {
    SketchInfo << "[multi_obs_front_egolanechange][fail]" << TX_VARS_NAME(PassTime, refCollection.passTime)
               << TX_VARS(res_elem_array.size());
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_19(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*junction_opposite_turnleft*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::junction_opposite_turnleft);
  // 如果自车元素在交叉口上
  if (CallSucc(ego_elem_ptr->IsOnJunction())) {
    const std::vector<Base::ITrafficElementPtr> &circleElemVec = ego_elem_ptr->GetSketchCircleElementPtrVec();
    // 获取自车元素的2D坐标
    const Base::txVec2 vPos2d = ego_elem_ptr->StableGeomCenter().ENU2D();
    // 获取自车元素的稳定车道方向
    const Base::txVec3 &egoDir = ego_elem_ptr->StableLaneDir();
    // 遍历自车周围的元素列表
    for (auto elemPtr : circleElemVec) {
      // 将元素指针转换为车辆元素指针
      Base::IVehicleElementPtr veh_elem_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(elemPtr);
      if (NonNull_Pointer(veh_elem_ptr)) {
        SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << " [junction_opposite_turnleft] check "
                   << TX_VARS_NAME(vehId, veh_elem_ptr->Id())
                   << TX_VARS_NAME(veh_type, veh_elem_ptr->VehicleType()._to_string());
        // 获取元素的2D中心坐标
        const Base::txVec2 nearestElementCenter2d = elemPtr->StableGeomCenter().ENU2D();
        // 计算元素相对于自车元素的局部坐标
        const Base::txVec2 local_coord_other =
            ego_elem_ptr->RelativePosition(vPos2d, nearestElementCenter2d, ego_elem_ptr->TransMatInv());
        // 如果元素在自车元素前方且在车道链接上
        if (local_coord_other.y() > 0.0 && CallSucc(veh_elem_ptr->IsOnLaneLink())) {
          /*front*/
          // 获取元素的车道信息
          const Base::Info_Lane_t veh_elem_uid = veh_elem_ptr->GetCurrentLaneInfo();
          // 获取元素的起始车道UID
          const Base::txLaneUId veh_elem_from_uid = veh_elem_uid.onLinkFromLaneUid;
          // 获取起始车道的长度
          const Base::txFloat from_lane_length = HdMap::HadmapCacheConCurrent::GetLaneLength(veh_elem_from_uid);
          // 获取起始车道的方向
          const Base::txVec3 from_lane_dir =
              HdMap::HadmapCacheConCurrent::GetLaneDir(veh_elem_from_uid, from_lane_length - 0.1);
          // 获取起始车道的反方向
          const Base::txVec3 reverse_from_lane_dir = from_lane_dir * -1.0;
          // 如果自车元素的方向与起始车道的反方向同步
          if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(reverse_from_lane_dir)))) {
            /*from reverse lane*/
            auto linkptr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(veh_elem_uid.onLinkFromLaneUid,
                                                                          veh_elem_uid.onLinkToLaneUid);
            // 获取目标车道的方向
            const Base::txVec3 to_lane_dir =
                HdMap::HadmapCacheConCurrent::GetLaneDir(veh_elem_uid.onLinkToLaneUid, 0.1);

            // 如果车道链接指针非空且车道链接的几何体非空且自车元素的方向与目标车道的方向不同步
            if (NonNull_Pointer(linkptr) && NonNull_Pointer(linkptr->getGeometry()) &&
                /*not turn U*/
                CallFail(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(to_lane_dir)))) {
              // 获取起始车道的起始和结束GPS坐标
              Coord::txWGS84 start_gps, end_gps;
              start_gps.FromWGS84(linkptr->getGeometry()->getStart());
              end_gps.FromWGS84(linkptr->getGeometry()->getEnd());

              // 计算起始车道和结束车道相对于自车元素的局部坐标
              const Base::txVec2 local_coord_start =
                  ego_elem_ptr->RelativePosition(vPos2d, start_gps.ToENU().ENU2D(), ego_elem_ptr->TransMatInv());
              const Base::txVec2 local_coord_end =
                  ego_elem_ptr->RelativePosition(vPos2d, end_gps.ToENU().ENU2D(), ego_elem_ptr->TransMatInv());

              // 如果起始车道在自车元素前方且结束车道在自车元素后方
              if (local_coord_start.x() < 0 && local_coord_end.x() > 0) {
                SketchNode tmpNode;
                tmpNode.type = cur_enum_type;
                // 设置SketchNode的源指针为检测到的元素指针
                tmpNode.source_ptr = elemPtr;
                refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
                SketchInfo << "[junction_opposite_turnleft][succ][front]"
                           << TX_VARS_NAME(PassTime, refCollection.passTime) << TX_VARS_NAME(Id, elemPtr->Id())
                           << TX_VARS_NAME(elemDir, Utils::ToString(elemPtr->GetLaneDir()))
                           << TX_VARS_NAME(elemLocInfo, veh_elem_uid) << TX_VARS(local_coord_start.x())
                           << TX_VARS(local_coord_start.y());
              }
            }
          }
        }
      }
    }
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_20(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*junction_opposite_turnright*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::junction_opposite_turnright);
  // 如果自车元素在交叉口上
  if (CallSucc(ego_elem_ptr->IsOnJunction())) {
    // 获取自车周围的元素列表
    const std::vector<Base::ITrafficElementPtr> &circleElemVec = ego_elem_ptr->GetSketchCircleElementPtrVec();
    // 获取自车元素的2D坐标
    const Base::txVec2 vPos2d = ego_elem_ptr->StableGeomCenter().ENU2D();
    // 获取自车元素的稳定车道方向
    const Base::txVec3 &egoDir = ego_elem_ptr->StableLaneDir();
    // 遍历自车周围的元素列表
    for (auto elemPtr : circleElemVec) {
      // 将元素指针转换为车辆元素指针
      Base::IVehicleElementPtr veh_elem_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(elemPtr);
      // 如果元素指针非空
      if (NonNull_Pointer(veh_elem_ptr)) {
        SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << " [junction_opposite_turnright] check "
                   << TX_VARS_NAME(vehId, veh_elem_ptr->Id())
                   << TX_VARS_NAME(veh_type, veh_elem_ptr->VehicleType()._to_string());
        // 获取元素的2D中心坐标
        const Base::txVec2 nearestElementCenter2d = elemPtr->StableGeomCenter().ENU2D();
        // 计算元素相对于自车元素的局部坐标
        const Base::txVec2 local_coord_other =
            ego_elem_ptr->RelativePosition(vPos2d, nearestElementCenter2d, ego_elem_ptr->TransMatInv());
        // 如果元素在自车元素前方且在车道链接上
        if (local_coord_other.y() > 0.0 && CallSucc(veh_elem_ptr->IsOnLaneLink())) {
          /*front*/
          // 获取元素的车道信息
          const Base::Info_Lane_t veh_elem_uid = veh_elem_ptr->GetCurrentLaneInfo();
          // 获取元素的起始车道UID
          const Base::txLaneUId veh_elem_from_uid = veh_elem_uid.onLinkFromLaneUid;
          // 获取起始车道的长度
          const Base::txFloat from_lane_length = HdMap::HadmapCacheConCurrent::GetLaneLength(veh_elem_from_uid);
          // 获取起始车道的方向
          const Base::txVec3 from_lane_dir =
              HdMap::HadmapCacheConCurrent::GetLaneDir(veh_elem_from_uid, from_lane_length - 0.1);
          const Base::txVec3 reverse_from_lane_dir = from_lane_dir * -1.0;
          // 如果自车元素的方向与起始车道的反方向同步
          if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(reverse_from_lane_dir)))) {
            /*from reverse lane*/
            auto linkptr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(veh_elem_uid.onLinkFromLaneUid,
                                                                          veh_elem_uid.onLinkToLaneUid);
            // 获取目标车道的方向
            const Base::txVec3 to_lane_dir =
                HdMap::HadmapCacheConCurrent::GetLaneDir(veh_elem_uid.onLinkToLaneUid, 0.1);

            // 如果车道链接指针非空且车道链接的几何体非空且自车元素的方向与目标车道的方向不同步
            if (NonNull_Pointer(linkptr) && NonNull_Pointer(linkptr->getGeometry()) &&
                /*not turn U*/
                CallFail(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(to_lane_dir)))) {
              // 获取起始车道和结束车道相对于自车元素的局部坐标
              Coord::txWGS84 start_gps, end_gps;
              start_gps.FromWGS84(linkptr->getGeometry()->getStart());
              end_gps.FromWGS84(linkptr->getGeometry()->getEnd());

              const Base::txVec2 local_coord_start =
                  ego_elem_ptr->RelativePosition(vPos2d, start_gps.ToENU().ENU2D(), ego_elem_ptr->TransMatInv());
              const Base::txVec2 local_coord_end =
                  ego_elem_ptr->RelativePosition(vPos2d, end_gps.ToENU().ENU2D(), ego_elem_ptr->TransMatInv());

              // 如果起始车道在自车元素前方且结束车道在自车元素后方
              if (local_coord_start.x() < 0 && local_coord_end.x() < 0) {
                SketchNode tmpNode;
                tmpNode.type = cur_enum_type;
                tmpNode.source_ptr = elemPtr;
                // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
                refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
                SketchInfo << "[junction_opposite_turnright][succ][front]"
                           << TX_VARS_NAME(PassTime, refCollection.passTime) << TX_VARS_NAME(Id, elemPtr->Id())
                           << TX_VARS_NAME(elemDir, Utils::ToString(elemPtr->GetLaneDir()))
                           << TX_VARS_NAME(elemLocInfo, veh_elem_uid) << TX_VARS(local_coord_start.x())
                           << TX_VARS(local_coord_start.y());
              }
            }
          }
        }
      }
    }
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_21(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*junction_opposite_straight*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::junction_opposite_straight);
  // 如果自车元素在交叉口上
  if (CallSucc(ego_elem_ptr->IsOnJunction())) {
    // 获取自车周围的元素列表
    const std::vector<Base::ITrafficElementPtr> &circleElemVec = ego_elem_ptr->GetSketchCircleElementPtrVec();
    // 获取自车元素的2D坐标
    const Base::txVec2 vPos2d = ego_elem_ptr->StableGeomCenter().ENU2D();
    // 获取自车元素的稳定车道方向
    const Base::txVec3 &egoDir = ego_elem_ptr->StableLaneDir();
    // 遍历自车周围的元素列表
    for (auto elemPtr : circleElemVec) {
      // 将元素指针转换为车辆元素指针
      Base::IVehicleElementPtr veh_elem_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(elemPtr);
      // 如果元素指针非空
      if (NonNull_Pointer(veh_elem_ptr)) {
        SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << " [junction_opposite_straight] check "
                   << TX_VARS_NAME(vehId, veh_elem_ptr->Id())
                   << TX_VARS_NAME(veh_type, veh_elem_ptr->VehicleType()._to_string());
        // 获取元素的2D中心坐标
        const Base::txVec2 nearestElementCenter2d = elemPtr->StableGeomCenter().ENU2D();
        // 计算元素相对于自车元素的局部坐标
        const Base::txVec2 local_coord_other =
            ego_elem_ptr->RelativePosition(vPos2d, nearestElementCenter2d, ego_elem_ptr->TransMatInv());
        // 如果元素在自车元素前方且在车道链接上
        if (local_coord_other.y() > 0.0 && CallSucc(veh_elem_ptr->IsOnLaneLink())) {
          /*front*/
          // 获取元素的车道信息
          const Base::Info_Lane_t veh_elem_uid = veh_elem_ptr->GetCurrentLaneInfo();
          // 获取元素的起始车道UID
          const Base::txLaneUId veh_elem_from_uid = veh_elem_uid.onLinkFromLaneUid;
          // 获取起始车道的长度
          const Base::txFloat from_lane_length = HdMap::HadmapCacheConCurrent::GetLaneLength(veh_elem_from_uid);
          // 获取起始车道的方向
          const Base::txVec3 from_lane_dir =
              HdMap::HadmapCacheConCurrent::GetLaneDir(veh_elem_from_uid, from_lane_length - 0.1);
          // 获取起始车道的反方向
          const Base::txVec3 reverse_from_lane_dir = from_lane_dir * -1.0;

          const Base::txVec3 reverse_to_lane_dir =
              (-1.0) * HdMap::HadmapCacheConCurrent::GetLaneDir(veh_elem_uid.onLinkToLaneUid, 0.1);
          // 如果自车元素的方向与起始车道的反方向同步且与目标车道的反方向同步
          if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(reverse_from_lane_dir))) &&
              CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(reverse_to_lane_dir)))) {
            SketchNode tmpNode;
            tmpNode.type = cur_enum_type;
            tmpNode.source_ptr = elemPtr;
            // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
            refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
            SketchInfo << "[junction_opposite_turnright][succ][front]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                       << TX_VARS_NAME(Id, elemPtr->Id())
                       << TX_VARS_NAME(elemDir, Utils::ToString(elemPtr->GetLaneDir()))
                       << TX_VARS_NAME(elemLocInfo, veh_elem_uid);
          }
        }
      }
    }
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_22(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*junction_opposite_turnU*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::junction_opposite_turnU);
  // 如果自车元素在交叉口上
  if (CallSucc(ego_elem_ptr->IsOnJunction())) {
    // 获取自车周围的元素列表
    const std::vector<Base::ITrafficElementPtr> &circleElemVec = ego_elem_ptr->GetSketchCircleElementPtrVec();
    // 获取自车元素的2D坐标
    const Base::txVec2 vPos2d = ego_elem_ptr->StableGeomCenter().ENU2D();
    // 获取自车元素的稳定车道方向
    const Base::txVec3 &egoDir = ego_elem_ptr->StableLaneDir();
    // 遍历自车周围的元素列表
    for (auto elemPtr : circleElemVec) {
      Base::IVehicleElementPtr veh_elem_ptr = std::dynamic_pointer_cast<Base::IVehicleElement>(elemPtr);
      // 如果元素指针非空
      if (NonNull_Pointer(veh_elem_ptr)) {
        SketchInfo << TX_VARS_NAME(PassTime, refCollection.passTime) << " [junction_opposite_turnU] check "
                   << TX_VARS_NAME(vehId, veh_elem_ptr->Id())
                   << TX_VARS_NAME(veh_type, veh_elem_ptr->VehicleType()._to_string());
        const Base::txVec2 nearestElementCenter2d = elemPtr->StableGeomCenter().ENU2D();
        // 计算元素相对于自车元素的局部坐标
        const Base::txVec2 local_coord_other =
            ego_elem_ptr->RelativePosition(vPos2d, nearestElementCenter2d, ego_elem_ptr->TransMatInv());
        // 如果元素在自车元素前方且在车道链接上
        if (local_coord_other.y() > 0.0 && CallSucc(veh_elem_ptr->IsOnLaneLink())) {
          /*front*/
          // 获取元素的车道信息
          const Base::Info_Lane_t veh_elem_uid = veh_elem_ptr->GetCurrentLaneInfo();
          // 获取元素的起始车道UID
          const Base::txLaneUId veh_elem_from_uid = veh_elem_uid.onLinkFromLaneUid;
          // 获取起始车道的长度
          const Base::txFloat from_lane_length = HdMap::HadmapCacheConCurrent::GetLaneLength(veh_elem_from_uid);
          // 获取起始车道的方向
          const Base::txVec3 from_lane_dir =
              HdMap::HadmapCacheConCurrent::GetLaneDir(veh_elem_from_uid, from_lane_length - 0.1);
          const Base::txVec3 reverse_from_lane_dir = from_lane_dir * -1.0;
          // 如果自车元素的方向与起始车道的反方向同步
          if (CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(reverse_from_lane_dir)))) {
            /*from reverse lane*/
            auto linkptr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(veh_elem_uid.onLinkFromLaneUid,
                                                                          veh_elem_uid.onLinkToLaneUid);
            const Base::txVec3 to_lane_dir =
                HdMap::HadmapCacheConCurrent::GetLaneDir(veh_elem_uid.onLinkToLaneUid, 0.1);

            // 如果车道链接指针非空且车道链接的几何体非空且自车元素的方向与目标车道的方向同步
            if (NonNull_Pointer(linkptr) && NonNull_Pointer(linkptr->getGeometry()) &&
                /*not turn U*/
                CallSucc(Math::IsSynchronicity(Utils::Vec3_Vec2(egoDir), Utils::Vec3_Vec2(to_lane_dir)))) {
              SketchNode tmpNode;
              tmpNode.type = cur_enum_type;
              // 设置SketchNode的源指针为检测到的元素指针
              tmpNode.source_ptr = elemPtr;
              refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
              SketchInfo << "[junction_opposite_turnU][succ][front]" << TX_VARS_NAME(PassTime, refCollection.passTime)
                         << TX_VARS_NAME(Id, elemPtr->Id())
                         << TX_VARS_NAME(elemDir, Utils::ToString(elemPtr->GetLaneDir()))
                         << TX_VARS_NAME(elemLocInfo, veh_elem_uid);
            }
          }
        }
      }
    }
  }
}

void TAD_StandAlone_TrafficElementSystem::CheckSketch_23(SketchNodeCollection &refCollection,
                                                         Base::IElementManagerPtr _elemMgr,
                                                         Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT {
  /*ego_parking*/
  static const SketchEnumType cur_enum_type = _plus_(SketchEnumType::ego_parking);
  // 获取自车元素的2D坐标
  const Base::txVec2 ego_pos_2d = ego_elem_ptr->GetLocation().ToENU().ENU2D();
  hadmap::txObjects res;
  // 查询自车周围10米范围内的停车区域
  if (CallSucc(Geometry::SpatialQuery::RTree2D_Signal::getInstance().FindParkInCircle(ego_pos_2d, 10.0, res))) {
    SketchNode tmpNode;
    tmpNode.type = cur_enum_type;
    // 将SketchNode添加到refCollection的conVec_sketch_nodes列表中
    refCollection.conVec_sketch_nodes.emplace_back(tmpNode);
    SketchInfo << "[ego_parking][succ]" << TX_VARS_NAME(PassTime, refCollection.passTime);
  }
}

#endif /*SceneSketch*/

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
#undef SketchInfo
#undef LogPerfStatistic
