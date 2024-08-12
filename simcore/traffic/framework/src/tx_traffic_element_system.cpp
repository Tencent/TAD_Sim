// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_traffic_element_system.h"
#include "tbb/concurrent_hash_map.h"
#include "tx_assembler_context.h"
#include "tx_enum_def.h"
#include "tx_env_perception.h"
#include "tx_parallel_def.h"
#include "tx_spatial_query.h"
#include "tx_timer_on_cpu.h"
#include "tx_traffic_element_base.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_SystemLoop)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(Base)
TX_NAMESPACE_OPEN(TBB)

// 定义一个并行填充元素的类
class parallel_filling_element {
 public:
  // 重载()运算符，用于并行填充元素
  void operator()(const tbb::blocked_range<size_t>& r) const {
    // 遍历范围内的元素
    for (size_t i = r.begin(); i != r.end(); ++i) {
      auto refFlowElem = _vec_TrafficElementPtr[i];
      refFlowElem->FillingElement(timeMgr, outTraffic);
    }
  }

  parallel_filling_element(Base::TimeParamManager const& _timeMgr, sim_msg::Traffic& _outTraffic,
                           std::vector<Base::ITrafficElementPtr>& elemVec)
      : timeMgr(_timeMgr), outTraffic(_outTraffic), _vec_TrafficElementPtr(elemVec) {}
  std::vector<Base::ITrafficElementPtr>& _vec_TrafficElementPtr;
  Base::TimeParamManager const& timeMgr;
  sim_msg::Traffic& outTraffic;
};
TX_NAMESPACE_CLOSE(TBB)

TrafficElementSystem::~TrafficElementSystem() { Release(); }

void TrafficElementSystem::ClearLastStepData() TX_NOEXCEPT {
#if __ClearHashLanePerStep__
  HdMap::HashedRoadCacheConCurrent::UngisterAllElements();
#endif /*__ClearHashLanePerStep__*/
#if USE_RTree
  Geometry::SpatialQuery::ClearSpatialQueryElementInfo();
#endif
}

Base::txBool TrafficElementSystem::Release() TX_NOEXCEPT {
  // 将交通元素系统的存活状态设置为false
  m_isAlive = false;
  // 将场景数据源设置为空指针
  m_SceneDataSource = nullptr;
  // 如果交通元素管理器不为空，则释放其资源
  if (NonNull_Pointer(m_ElementMgr)) {
    m_ElementMgr->Release();
  }
  // 将交通元素管理器设置为空指针
  m_ElementMgr = nullptr;
  m_AssemblerCtx = nullptr;
  ReleaseSketch();
  return true;
}

Base::txBool TrafficElementSystem::FillingTrafficData(Base::TimeParamManager const& timeMgr,
                                                      sim_msg::Traffic& outTraffic) TX_NOEXCEPT {
  outTraffic.Clear();
  {
    const auto& refEgoArray = ElemMgr()->GetEgoArray();
    // 遍历所有自车数组
    for (auto& refEgoPtr : refEgoArray) {
      if (NonNull_Pointer(refEgoPtr)) {
        TX_MARK("need, for post simulation, set pb flag false.");
        // 填充交通元素
        refEgoPtr->FillingElement(timeMgr, outTraffic);
      }
    }
  }
#if 1
  {
    /*all vehicles*/
    // 获取所有车辆数组
    Base::ISimulator::AtomicSize atom_cars_size = {0};
    auto& refAllVehicleArray = ElemMgr()->GetAllVehiclePtr();

    // 使用TBB并行遍历所有车辆数组
    tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray.size(), [&](const std::size_t idx) {
      ISimulatorPtr simulator_interface_ptr = std::dynamic_pointer_cast<ISimulator>(refAllVehicleArray[idx]);
      // 如果ISimulator指针有效，则调用PreFillingElement方法
      if (NonNull_Pointer(simulator_interface_ptr)) {
        simulator_interface_ptr->PreFillingElement(atom_cars_size);
      }
    }); /*lamda function*/
        /* parallel_for */

    const Base::ISimulator::AtomicSizeValueType n_cars_size = atom_cars_size;
    /*LOG(WARNING) << TX_VARS(n_cars_size);*/
    // 重置预分配车辆数组
    ResetPreAllocateCar(n_cars_size);
    // 将预分配车辆数组的前n_cars_size个元素赋值给outTraffic.cars()
    *outTraffic.mutable_cars() = {m_PreAllocateCars.begin(), (m_PreAllocateCars.begin() + n_cars_size)};

    // 使用TBB并行遍历所有车辆数组
    tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray.size(), [&](const std::size_t idx) {
      ISimulatorPtr simulator_interface_ptr = std::dynamic_pointer_cast<ISimulator>(refAllVehicleArray[idx]);
      // 如果ISimulator指针有效，则调用FillingElement方法
      if (NonNull_Pointer(simulator_interface_ptr)) {
        simulator_interface_ptr->FillingElement(timeMgr, outTraffic);
      }
    }); /*lamda function*/
        /* parallel_for */
  }
  {
    /*all pedestrians*/
    auto& refAllPedestrianArray = ElemMgr()->GetAllPedestrianPtr();
    // 遍历所有行人数组
    for (auto refPedestrianPtr : refAllPedestrianArray) {
      // 如果行人指针有效且行人存活，则填充交通元素
      if (NonNull_Pointer(refPedestrianPtr) && CallSucc(refPedestrianPtr->IsAlive())) {
        refPedestrianPtr->FillingElement(timeMgr, outTraffic);
      }
    }
  }
  {
    /*all obstacles*/
    Base::ISimulator::AtomicSize atom_obstacle_size = {0};
    auto& refAllObstacleArray = ElemMgr()->GetAllObstaclePtr();

    // 使用TBB并行遍历所有障碍物数组
    tbb::parallel_for(static_cast<std::size_t>(0), refAllObstacleArray.size(), [&](const std::size_t idx) {
      ISimulatorPtr simulator_interface_ptr = std::dynamic_pointer_cast<ISimulator>(refAllObstacleArray[idx]);
      // 如果ISimulator指针有效，则调用PreFillingElement方法
      if (NonNull_Pointer(simulator_interface_ptr)) {
        simulator_interface_ptr->PreFillingElement(atom_obstacle_size);
      }
    }); /*lamda function*/
        /* parallel_for */

    // 获取障碍物数量
    const Base::ISimulator::AtomicSizeValueType n_obstacle_size = atom_obstacle_size;
    /*LOG(WARNING) << TX_VARS(n_obstacle_size);*/
    // 重置预分配障碍物数组
    ResetPreAllocateObstacle(n_obstacle_size);
    // 将预分配障碍物数组的前n_obstacle_size个元素赋值给outTraffic.staticobstacles()
    *outTraffic.mutable_staticobstacles() = {m_PreAllocateObstacles.begin(),
                                             (m_PreAllocateObstacles.begin() + n_obstacle_size)};
    // 使用TBB并行遍历所有障碍物数组
    tbb::parallel_for(static_cast<std::size_t>(0), refAllObstacleArray.size(), [&](const std::size_t idx) {
      ISimulatorPtr simulator_interface_ptr = std::dynamic_pointer_cast<ISimulator>(refAllObstacleArray[idx]);
      // 如果ISimulator指针有效，则调用FillingElement方法
      if (NonNull_Pointer(simulator_interface_ptr)) {
        simulator_interface_ptr->FillingElement(timeMgr, outTraffic);
      }
    }); /*lamda function*/
        /* parallel_for */
  }
#else
  {
    auto& refAllElementArray = ElemMgr()->GetAllElement();
    tbb::parallel_for(static_cast<std::size_t>(0), refAllElementArray.size(), [&](const std::size_t idx) {
      ISimulatorPtr simulator_interface_ptr = std::dynamic_pointer_cast<ISimulator>(refAllElementArray[idx]);
      if (NonNull_Pointer(simulator_interface_ptr)) {
        simulator_interface_ptr->FillingElement(timeMgr, outTraffic);
      }
    }); /*lamda function*/
        /* parallel_for */
  }
#endif
  {
    // AssemblerCtx()->TestSerialization(timeMgr, ElemMgr());
  }
  auto& refAllSignals = ElemMgr()->GetAllSignal();
  // 如果信号数组的大小大于10，则使用TBB并行遍历所有信号数组
  if (refAllSignals.size() > 10) {
    tbb::parallel_for(static_cast<std::size_t>(0), refAllSignals.size(), [&](const std::size_t idx) {
      // 将信号数组中的元素转换为ISimulator指针
      ISimulatorPtr simulator_interface_ptr = std::dynamic_pointer_cast<ISimulator>(refAllSignals[idx]);
      // 如果ISimulator指针有效，则调用FillingElement方法
      if (NonNull_Pointer(simulator_interface_ptr)) {
        simulator_interface_ptr->FillingElement(timeMgr, outTraffic);
      }
    }); /*lamda function*/
        /* parallel_for */
  } else {
    // 否则，遍历所有信号数组
    for (auto& refSignal : refAllSignals) {
      // 如果信号指针有效，则填充交通元素
      if (NonNull_Pointer(refSignal)) {
        refSignal->FillingElement(timeMgr, outTraffic);
      }
    }
  }

  {
    auto& refAllRelativeObs = ElemMgr()->GetRelativeObsArray();
    // 遍历所有相对障碍物数组
    for (auto& elemPtr : refAllRelativeObs) {
      // 如果相对障碍物指针有效，则填充交通元素
      if (NonNull_Pointer(elemPtr)) {
        elemPtr->FillingElement(timeMgr, outTraffic);
      }
    }
  }
  return true;
}

Base::txBool TrafficElementSystem::FillingSpatialQuery() TX_NOEXCEPT {
  ClearLastStepData();
#if USE_TBB
  const auto& vecAllTrafficElement = ElemMgr()->GetAllElement();
  const Base::txSize nSize = vecAllTrafficElement.size();
  // 定义一个静态的CPU计时器
  static Base::TimingCPU s_timer;
  s_timer.StartCounter();

  // 使用TBB并行遍历所有交通元素
  tbb::parallel_for(static_cast<std::size_t>(0), vecAllTrafficElement.size(), [&](const std::size_t idx) {
    if (NonNull_Pointer(vecAllTrafficElement[idx])) {
      vecAllTrafficElement[idx]->FillingSpatialQuery();
    }
  }); /*lamda function*/
      /* parallel_for */
  // 加载批量数据到RTree2D中
  Geometry::SpatialQuery::RTree2D::getInstance().LoadBulk();
  LOG_IF(INFO, FLAGS_LogLevel_Statistics_MultiThread)
      << "FillingSpatialQuery end." << TX_VARS_NAME(timer, s_timer.GetElapsedMicroseconds());
#endif /*USE_TBB*/
  return true;
}

Base::txBool TrafficElementSystem::CheckEgoType(const Base::ISceneLoader::EgoType _egoType,
                                                const Base::txInt _egoSize) const TX_NOEXCEPT {
  // 如果自车类型为车辆且自车大小不为1，则输出错误信息并返回false
  if (_plus_(Base::ISceneLoader::EgoType::eVehicle) == _egoType && 1 != _egoSize) {
    LOG(WARNING) << "[Error] vehicle ego size error. " << TX_VARS_NAME(EgoSize, _egoSize);
    return false;
  } else if (_plus_(Base::ISceneLoader::EgoType::eTruck) == _egoType &&
             2 != _egoSize) {  // 如果自车类型为卡车且自车大小不为2，则输出错误信息并返回false
    LOG(WARNING) << "[Error] truck ego size error. " << TX_VARS_NAME(EgoSize, _egoSize);
    return false;
  }

  // 如果自车类型和自车大小匹配，则输出成功信息并返回true
  LOG(INFO) << "[Success] vehicle ego size true. " << TX_VARS_NAME(egoType, _egoType)
            << TX_VARS_NAME(EgoSize, _egoSize);
  return true;
}

/*Base::ISceneLoader::EgoType TrafficElementSystem::GetEgoType() const TX_NOEXCEPT {
    return SceneLoader()->GetEgoType();
}*/

#if USE_DITW_Event
Base::txBool TrafficElementSystem::RegisterSimulationEvent(txInjectionEventPtr evtPtr) TX_NOEXCEPT {
  // 遍历注入事件向量
  for (auto itr = m_vec_injection_event.begin(); itr != m_vec_injection_event.end();) {
    auto eventPtr = *itr;
    // 如果事件无效，则从向量中删除该事件
    if (CallFail(eventPtr->IsValid())) {
      itr = m_vec_injection_event.erase(itr);
    } else {
      itr++;
    }
  }

  // 如果事件指针有效且事件有效，则将事件添加到注入事件向量中
  if (NonNull_Pointer(evtPtr) && evtPtr->IsValid()) {
    m_vec_injection_event.emplace_back(evtPtr);
    // 返回true表示注册成功
    return true;
  } else {
    return false;
  }
}

void TrafficElementSystem::ClearSimulationEvent(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  LOG(INFO) << "Clear Simulation Event at: " << timeMgr.PassTime();
  m_vec_injection_event.clear();
}

Base::txBool TrafficElementSystem::UpdateSimulationEvent(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  // 遍历注入事件向量
  for (auto itr = m_vec_injection_event.begin(); itr != m_vec_injection_event.end();) {
    // 获取当前事件指针
    auto eventPtr = *itr;
    // 如果事件无效，则从向量中删除该事件
    if (CallFail(eventPtr->IsValid())) {
      itr = m_vec_injection_event.erase(itr);
    } else {
      // 如果事件有效，则更新事件的仿真状态
      eventPtr->UpdateSimulationEvent(timeMgr.AbsTime(), ElemMgr()->GetAllVehiclePtr());
      itr++;
    }
  }
  return true;
}

#endif /*USE_DITW_Event*/
TX_NAMESPACE_CLOSE(Base)

#undef LogInfo
#undef LogWarn
