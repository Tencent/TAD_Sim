// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_traffic_system.h"
#include "HdMap/tx_hashed_road.h"
#include "SceneLoader/tx_assembler_factory.h"
#include "tad_ego_vehicle_element.h"
#include "tx_cloud_stand_alone_event_dispatcher.h"
#include "tx_env_perception.h"
#include "tx_hadmap_utils.h"
#include "tx_linear_element_manager.h"
#include "tx_protobuf_utils.h"
#include "tx_spatial_query.h"
#include "tx_timer_on_cpu.h"
#include "tx_tc_cloud_element_manager.h"
#include "tx_tc_cloud_event_json_parser.h"
#include "tx_tc_cloud_loader.h"
#include "tx_tc_ditw_tad_vehicle_ai_element.h"
#include "tx_tc_gflags.h"
#include "tx_tc_tad_vehicle_ai_element.h"
#define LogPerfStatistic LOG_IF(INFO, FLAGS_LogLevel_Statistics_MultiThread)
#define CloudDitwInfo LOG_IF(INFO, FLAGS_LogLevel_CloudDitw) << "[cloud_ditw_debug]"
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

CloudTrafficElementSystem::CloudTrafficElementSystem() {}

Base::txBool CloudTrafficElementSystem::IsSupportSceneType(const Base::ISceneLoader::ESceneType _sceneType) const
    TX_NOEXCEPT {
  if (+Base::ISceneLoader::ESceneType::eCloud == (_sceneType)) {
    return true;
  } else {
    return false;
  }
}

void CloudTrafficElementSystem::ExecuteEnvPerception(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
#if __FocusVehicle__
  auto refEgoArray = ElemMgr()->GetEgoArray();
  for (auto egoPtr : refEgoArray) {
    if (NonNull_Pointer(egoPtr)) {
      egoPtr->ComputeFocusVehicles(timeMgr);
    }
  }
#endif /*__FocusVehicle__*/

  auto& refAllVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
  tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray.size(), [&](const std::size_t idx) {
    Base::IEnvPerceptionPtr env_perception_interface_ptr =
        std::dynamic_pointer_cast<Base::IEnvPerception>(refAllVehicleArray[idx]);
    if (NonNull_Pointer(env_perception_interface_ptr)) {
      env_perception_interface_ptr->ExecuteEnvPerception(timeMgr);
    }
  }); /*lamda function*/
      /* parallel_for */
}

void CloudTrafficElementSystem::CreateAssemblerCtx() TX_NOEXCEPT {
  m_Cloud_AssemblerCtx = std::make_shared<Scene::TAD_Cloud_AssemblerContext>();
  m_AssemblerCtx = m_Cloud_AssemblerCtx;
}

void CloudTrafficElementSystem::CreateElemMgr() TX_NOEXCEPT {
  m_Cloud_ElementMgr = std::make_shared<TrafficFlow::CloudElementManager>();
  m_ElementMgr = m_Cloud_ElementMgr;
}

Base::txBool CloudTrafficElementSystem::RegisterPlanningCar() TX_NOEXCEPT {
  LogWarn << "Call RegisterPlanningCar";

  if (NonNull_Pointer(SceneLoader()) && NonNull_Pointer(ElemMgr()) && NonNull_Pointer(AssemblerCtx())) {
    if (CallSucc(ElemMgr()->GetEgoArray().empty())) {
      TX_MARK("Ego has not been created.");

      if (IsSupportSceneType(SceneLoader()->GetSceneType())) {
        /*sim_msg::Location pbEgoInfo;
        SceneLoader()->GetRoutingInfo(pbEgoInfo);
        SceneLoader()->SetEgoData(pbEgoInfo);*/
      } else {
        LogWarn << "Un support Scene Type (eTAD, eOSC)";
        return false;
      }

      if (NonNull_Pointer(AssemblerCtx()) && CallSucc(AssemblerCtx()->GenerateEgo(ElemMgr(), SceneLoader()))) {
        LogWarn << "GenerateEgo Success.";
      } else {
        LOG(WARNING) << "GenerateEgo Failure.";
        return false;
      }
      const auto& refEgoArray = ElemMgr()->GetEgoArray();
      return CheckEgoType(SceneLoader()->GetEgoType(), refEgoArray.size());
    } else {
      return CheckEgoType(GetEgoType(), ElemMgr()->GetEgoCount());
    }
  } else {
    LogWarn << "NonNull_Pointer(m_SceneDataSource) && NonNull_Pointer(m_ElementMgr) error.";
    return false;
  }
}

Base::txBool CloudTrafficElementSystem::Initialize(Base::ISceneLoaderPtr loader,
                                                   const Base::map_range_t valid_map_range) TX_NOEXCEPT {
  PrintFlagsValuesTrafficCloud();
  if (NonNull_Pointer(loader)) {
    m_SceneDataSource = loader;
    m_EgoType = SceneLoader()->GetEgoType();
    CreateElemMgr();
    CreateAssemblerCtx();

    if (NonNull_Pointer(CloudAssemblerCtx()) && NonNull_Pointer(ElemMgr()) &&
        CallSucc(CloudAssemblerCtx()->Init(loader, valid_map_range)) &&
        CallSucc(CloudAssemblerCtx()->GenerateScene(m_ElementMgr, loader))) {
      m_ElementMgr->Initialize(loader);
      EnableVehicleInput(FLAGS_VehicleInputEnable);
      if (CallSucc(RegEgo())) {
        LOG(INFO) << "call RegisterPlanningCar";
        RegisterPlanningCar();
      } else {
        LOG(INFO) << "do not RegisterPlanningCar";
      }
      LOG(INFO) << "GenerateScene Success." << TX_COND(EnableVehicleInput());
      m_isAlive = true;
      return true;
    } else {
      LOG(WARNING) << " GenerateScene Failure.";
      return false;
    }
  } else {
    LOG(WARNING) << " Data is Null.";
    return false;
  }
}

Base::txBool CloudTrafficElementSystem::Update_Pedestrian(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  auto& refAllPedestrianArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Pedestrian);
  for (auto refPedestrianPtr : refAllPedestrianArray) {
    if (NonNull_Pointer(refPedestrianPtr) && CallSucc(refPedestrianPtr->IsAlive())) {
      refPedestrianPtr->Update(timeMgr);
    }
  }
  return true;
}

Base::txBool CloudTrafficElementSystem::Update_Obstacle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  auto& refAllObstacleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Immovability);
  for (auto refObstaclePtr : refAllObstacleArray) {
    if (NonNull_Pointer(refObstaclePtr) && CallSucc(refObstaclePtr->IsAlive())) {
      refObstaclePtr->Update(timeMgr);
    }
  }
  return true;
}

Base::txBool CloudTrafficElementSystem::Update_Vehicle_PostSimulation(const Base::TimeParamManager& timeMgr)
    TX_NOEXCEPT {
  auto& refVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
  tbb::parallel_for(static_cast<std::size_t>(0), refVehicleArray.size(), [&](const std::size_t idx) {
    auto& elemPtr = refVehicleArray[idx];
    if (NonNull_Pointer(elemPtr) && CallSucc(elemPtr->IsAlive())) {
      elemPtr->Post_Update(timeMgr);
    }
  }); /*lamda function*/
      /* parallel_for */
  return true;
}

void CloudTrafficElementSystem::FlushSceneEvents(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (timeMgr.PassTime() > 0.0) {
    TrafficFlow::CloudEventDispatcher::getInstance().FlushAllEvents(timeMgr, ElemMgr());
  }
}

Base::txBool CloudTrafficElementSystem::Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (IsAlive() && NonNull_Pointer(CloudAssemblerCtx())) {
    if (CallSucc(EnableVehicleInput()) &&
        CallFail(CloudAssemblerCtx()->DynamicGenerateScene(timeMgr, ElemMgr(), SceneLoader()))) {
      LOG(WARNING) << "DynamicGenerateScene failure.";
      return false;
    }

    if (CallFail(Update_Signal(timeMgr))) {
      LogWarn << "Update_Signal run failure.";
      return false;
    }

    if (CallFail(Update_CheckLifeCycle(timeMgr))) {
      LogWarn << "Update_CheckLifeCycle run failure.";
      return false;
    }

    FlushSceneEvents(timeMgr);

    ExecuteEnvPerception(timeMgr);

    if (CallFail(Update_Pedestrian(timeMgr))) {
      LogWarn << "Update_Pedestrian run failure.";
      return false;
    }
    if (event_type::eFog == m_event_type) {
      Fog();
    } else if (event_type::eCrash == m_event_type) {
      Crash();
    }

    InjectTrafficEventHandler(timeMgr, ElemMgr());

    if (CallFail(Update_Vehicle_Simulation(timeMgr))) {
      LogWarn << "Update_Vehicle_Simulation run failure.";
      return false;
    }

    if (CallFail(Update_Vehicle_PostSimulation(timeMgr))) {
      TX_MARK("reset signal, pedestrian");
      LogWarn << "Update_Vehicle_PostSimulation run failure.";
      return false;
    }

    InjectTrafficEventHandlerPost(timeMgr, ElemMgr());

    /*if (CallFail(Update_Obstacle(timeMgr))) {
            LogWarn << "Update_Obstacle run failure.";
            return false;
    }*/

    if (CallFail(CloudAssemblerCtx()->DynamicChangeScene(timeMgr, ElemMgr(), SceneLoader()))) {
      LOG(WARNING) << "DynamicChangeScene failure.";
      return false;
    }

    {
      // m_AssemblerCtx->TestSerialization(timeMgr, ElemMgr());
    }
    return true;
  } else {
    LogWarn << "(IsAlive() && NonNull_Pointer(m_AssemblerCtx)) failure.";
    return false;
  }
}

Base::txBool SafeInputRegion(const Base::txVec2& inputPt, const Base::txVec2& nearestPt,
                             const Base::txFloat radius = FLAGS_tc_input_safe_region_radius) TX_NOEXCEPT {
  if ((inputPt - nearestPt).norm() > radius) {
    return true;
  } else {
    return false;
  }
}

Base::txBool CloudTrafficElementSystem::RegisterEgoOnCloud(const Base::txSysId ego_id, ::sim_msg::Location egoLoc,
                                                           const Base::txBool bForce) TX_NOEXCEPT {
  if (NonNull_Pointer(SceneLoader()) && NonNull_Pointer(CloudAssemblerCtx()) && NonNull_Pointer(ElemMgr())) {
    const auto& retEgoVec = (ElemMgr()->GetEgoArray());

    if (CallSucc(ElemMgr()->HasEgoId(ego_id))) {
      LOG_IF(WARNING, FLAGS_LogLevel_Cloud) << "ego id has been registered. " << TX_VARS(ego_id);
      return false;
    }
    /*for (const auto& egoPtr : retEgoVec) {
        if (ego_id == egoPtr->Id()) {
            LOG_IF(WARNING, FLAGS_LogLevel_Cloud) << "ego id has been registered. " << TX_VARS(ego_id);
            return false;
        }
    }*/
#if 1

    Coord::txWGS84 vPos(hadmap::txPoint(egoLoc.position().x(), egoLoc.position().y(), egoLoc.position().z()));
    const Base::txVec2 startPtLoc = vPos.GetENU().ENU2D();
    LOG(WARNING) << TX_VARS_NAME(new_ego_pt, vPos);

    Base::Info_Lane_t ego_locInfo;
    Base::txFloat _s = 0.0;
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(vPos, ego_locInfo, _s);
    auto geomptr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(ego_locInfo);
    auto egoHashedLaneInfo = Geometry::SpatialQuery::GenerateHashedLaneInfo(geomptr, _s, ego_locInfo);
    auto center_node_ptr = HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(egoHashedLaneInfo);
    Base::txSurroundVehiclefo ego_front_elem;
    Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::SearchNearestFrontElement(
        0, 0, FLAGS_EGO_Length, egoHashedLaneInfo, center_node_ptr, ego_front_elem, FLAGS_DETECT_OBJECT_DISTANCE);
    Base::SimulationConsistencyPtr nearestElementPtr = Weak2SharedPtr(std::get<_ElementIdx_>(ego_front_elem));
    if (bForce || Null_Pointer(nearestElementPtr) ||
        (NonNull_Pointer(nearestElementPtr) &&
         SafeInputRegion(startPtLoc, nearestElementPtr->StableGeomCenter().ENU2D(),
                         FLAGS_tc_ego_input_safe_region_radius))) {
      if (FLAGS_LogLevel_Cloud_EgoInput && NonNull_Pointer(nearestElementPtr)) {
        LOG(INFO) << TX_VARS_NAME(startPtLoc, vPos)
                  << TX_VARS_NAME(nearestElementLoc, nearestElementPtr->StableGeomCenter())
                  << TX_VARS_NAME(nearestElementId, nearestElementPtr->ConsistencyId()) << TX_VARS(ego_id)
                  << TX_VARS_NAME(dist, (startPtLoc - nearestElementPtr->StableGeomCenter().ENU2D()).norm());
      }

      if (+Base::ISceneLoader::ESceneType::eCloud == (SceneLoader()->GetSceneType())) {
        SceneLoader::TAD_Cloud_SceneLoaderPtr realSceneLoaderPtr =
            std::dynamic_pointer_cast<SceneLoader::TAD_Cloud_SceneLoader>(SceneLoader());

        egoLoc.set_t(ego_id);
        realSceneLoaderPtr->SetEgoData(egoLoc);

        if (CloudAssemblerCtx()->GenerateEgo(ElemMgr(), SceneLoader())) {
          ElemMgr()->GenerateAllTrafficElement();
          LOG_IF(INFO, FLAGS_LogLevel_Cloud) << "GenerateEgo Success." << TX_VARS(ego_id);
          return true;
        } else {
          LOG(WARNING) << "GenerateEgo Failure." << TX_VARS(ego_id);
          return false;
        }
      } else {
        LOG(WARNING) << "GenerateEgo Failure. Scene Type Error.";
        return false;
      }
    } else {
      LOG(WARNING) << "GenerateEgo Failure. SafeInputRegion failure." << TX_VARS(ego_id) << TX_VARS(vPos);
      return false;
    }
#endif
  } else {
    return false;
  }
}

Base::txBool CloudTrafficElementSystem::EraseEgoById(const Base::txSysId ego_id) TX_NOEXCEPT {
  if (NonNull_Pointer(SceneLoader()) && NonNull_Pointer(CloudAssemblerCtx())) {
    const auto& retEgoVec = (ElemMgr()->GetEgoArray());

    for (const auto& egoPtr : retEgoVec) {
      if (NonNull_Pointer(egoPtr) && (ego_id == egoPtr->Id())) {
        ElemMgr()->EarseEgo(ego_id);
        ElemMgr()->GenerateAllTrafficElement();
        LOG(INFO) << "ego id has been deleted. " << TX_VARS(ego_id);
        return true;
      }
    }
    LOG(WARNING) << "erase ego failure, egoid not find." << TX_VARS(ego_id);
    return false;
  } else {
    return false;
  }
}

Base::txBool CloudTrafficElementSystem::UpdateEgoOnCloud(const Base::txSysId ego_id,
                                                         const sim_msg::Location& info) TX_NOEXCEPT {
  if (NonNull_Pointer(SceneLoader()) && NonNull_Pointer(CloudAssemblerCtx())) {
    const auto& retEgoVec = (ElemMgr()->GetEgoArray());

    for (const auto& egoPtr : retEgoVec) {
      if (NonNull_Pointer(egoPtr) && (ego_id == egoPtr->Id())) {
        TrafficFlow::TAD_EgoVehicleElementPtr realEgoPtr =
            std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(egoPtr);
        if (NonNull_Pointer(realEgoPtr)) {
          static Base::txString strPB;
          strPB.clear();
          info.SerializeToString(&strPB);
          realEgoPtr->SetProtoBufferStr(strPB);
          LOG_IF(INFO, FLAGS_LogLevel_Cloud) << "ego update success. " << TX_VARS(ego_id);
          return true;
        }
      }
    }

    LOG(WARNING) << "ego id id not in current system. " << TX_VARS(ego_id);
    return false;
  } else {
    LOG(WARNING) << "ego update failure, system is invalid. " << TX_VARS(ego_id);
    return false;
  }
}

Base::txBool CloudTrafficElementSystem::QueryTrafficDataByCircle(const Base::txSysId ego_id, const Base::txFloat radius,
                                                                 std::vector<Base::ITrafficElementPtr>& queryResultVec)
    TX_NOEXCEPT {
  queryResultVec.clear();
  return false;
}

Base::txBool CloudTrafficElementSystem::SendSpecialTrafficData(
    Base::TimeParamManager const& timeMgr, std::vector<Base::ITrafficElementPtr>& vec_send_element_ptr,
    sim_msg::Traffic& outTraffic) TX_NOEXCEPT {
#if USE_TBB
  const int nSize = vec_send_element_ptr.size();
  static Base::TimingCPU s_timer;
  s_timer.StartCounter();

  // tbb::parallel_for(blocked_range<size_t>(0, nSize), TBB::parallel_filling_element(timeStamp, outTraffic,
  // vec_send_element_ptr));
  tbb::parallel_for(static_cast<std::size_t>(0), vec_send_element_ptr.size(), [&](const std::size_t idx) {
    Base::ISimulatorPtr simulator_interface_ptr =
        std::dynamic_pointer_cast<Base::ISimulator>(vec_send_element_ptr[idx]);
    if (NonNull_Pointer(simulator_interface_ptr)) {
      simulator_interface_ptr->FillingElement(timeMgr, outTraffic);
    }
  }); /*lamda function*/
      /* parallel_for */

  LOG_IF(INFO, FLAGS_LogLevel_Statistics_MultiThread)
      << "SendSpecialTrafficData end." << TX_VARS_NAME(timer, s_timer.GetElapsedMicroseconds());
#endif /*USE_TBB*/
  return true;
}

Base::txBool CloudTrafficElementSystem::Update_Signal(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  static Base::TimingCPU s_timer;
  s_timer.StartCounter();
  auto& refAllSignal = ElemMgr()->GetAllSignal();
  tbb::parallel_for(static_cast<std::size_t>(0), refAllSignal.size(), [&](const std::size_t idx) {
    if (NonNull_Pointer(refAllSignal[idx])) {
      refAllSignal[idx]->Update(timeMgr);
    }
  }); /*lamda function*/
      /* parallel_for */
  LogPerfStatistic << "Update_Signal :" << TX_VARS_NAME(Time_Consuming, s_timer.GetElapsedMicroseconds());
  return true;
}

Base::txBool CloudTrafficElementSystem::Update_CheckLifeCycle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  static Base::TimingCPU s_timer;
  s_timer.StartCounter();
  {
    auto& refAllElement = ElemMgr()->GetAllElement();
    tbb::parallel_for(static_cast<std::size_t>(0), refAllElement.size(), [&](const std::size_t idx) {
      auto& elemPtr = refAllElement[idx];
      if (NonNull_Pointer(elemPtr)) {
        elemPtr->CheckStart(timeMgr);
        elemPtr->CheckEnd(timeMgr);
      }
    }); /*lamda function*/
        /* parallel_for */
  }
#if 0
  {
    auto& refAllEdgeVehicle = CloudElemMgr()->GetAllEdgeVehiclePtr();
    tbb::parallel_for(static_cast<std::size_t>(0), refAllEdgeVehicle.size(), [&](const std::size_t idx) {
      auto& elemPtr = refAllEdgeVehicle[idx];
      if (NonNull_Pointer(elemPtr)) {
        elemPtr->CheckStart(timeMgr);
        // elemPtr->CheckEnd(timeMgr);
      }
    }); /*lamda function*/
        /* parallel_for */
  }
#endif
  LogPerfStatistic << "Update_CheckLifeCycle :" << TX_VARS_NAME(Time_Consuming, s_timer.GetElapsedMicroseconds());
  return true;
}

Base::txBool CloudTrafficElementSystem::Update_Vehicle_Simulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  static Base::TimingCPU s_timer;
  s_timer.StartCounter();
  auto refEgoArray = ElemMgr()->GetEgoArray();
  if (refEgoArray.size() > 10) {
    tbb::parallel_for(static_cast<std::size_t>(0), refEgoArray.size(), [&](const std::size_t idx) {
      auto elemPtr = refEgoArray[idx];
      if (NonNull_Pointer(elemPtr)) {
        elemPtr->Update(timeMgr);
      }
    }); /*lamda function*/
        /* parallel_for */
  } else {
    for (auto egoPtr : refEgoArray) {
      if (NonNull_Pointer(egoPtr)) {
        egoPtr->Update(timeMgr);
      }
    }
  }

  auto& refVehicleArray = ElemMgr()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
  tbb::parallel_for(static_cast<std::size_t>(0), refVehicleArray.size(), [&](const std::size_t idx) {
    /*LOG(INFO) << "[cloud_concurrency_size] " << TX_VARS_NAME(thread_id,
     * tbb::task_arena::current_thread_index());*/
    auto& elemPtr = refVehicleArray[idx];
    if (NonNull_Pointer(elemPtr)) {
      elemPtr->Update(timeMgr);
    }
  }); /*lamda function*/
      /* parallel_for */

  LogPerfStatistic << "Update_CheckLifeCycle :" << TX_VARS_NAME(Time_Consuming, s_timer.GetElapsedMicroseconds());
  return true;
}

Base::txBool CloudTrafficElementSystem::ReSetInputAgent(const std::vector<Base::map_range_t>& vec_valid_sim_range)
    TX_NOEXCEPT {
  if (NonNull_Pointer(CloudAssemblerCtx())) {
    return CloudAssemblerCtx()->ReSetInputAgent(vec_valid_sim_range);
  } else {
    return false;
  }
}

Base::txBool CloudTrafficElementSystem::FillingSimStatus(Base::TimeParamManager const& timeMgr,
                                                         its::txSimStatus& outSceneStatue) TX_NOEXCEPT {
  outSceneStatue.Clear();
  outSceneStatue.set_time(timeMgr.TimeStamp());
  {
    auto refAllElementArray = ElemMgr()->GetAllVehiclePtr();
    Base::ISimulator::AtomicSize atom_cars_size = {0};

    tbb::parallel_for(static_cast<std::size_t>(0), refAllElementArray.size(), [&](const std::size_t idx) {
      if (NonNull_Pointer(refAllElementArray[idx])) {
        refAllElementArray[idx]->PreFillingElement(atom_cars_size);
      }
    }); /*lamda function*/
        /* parallel_for */

    const Base::ISimulator::AtomicSizeValueType n_cars_size = atom_cars_size;
    // LOG(WARNING) << TX_VARS(n_cars_size);
    ResetPreAllocateTxVehicle(n_cars_size);

    *outSceneStatue.mutable_vehicle() = {m_PreAllocateTxVehicles.begin(),
                                         (m_PreAllocateTxVehicles.begin() + n_cars_size)};

    tbb::parallel_for(static_cast<std::size_t>(0), refAllElementArray.size(), [&](const std::size_t idx) {
      TAD_Cloud_AI_VehicleElementPtr cloud_ai_veh_ptr =
          std::dynamic_pointer_cast<TAD_Cloud_AI_VehicleElement>(refAllElementArray[idx]);
      if (NonNull_Pointer(cloud_ai_veh_ptr)) {
        cloud_ai_veh_ptr->FillingSimStatus(timeMgr, outSceneStatue);
      }
    }); /*lamda function*/
        /* parallel_for */
  }

  if (FLAGS_LogLevel_Traffic_PB_Output) {
    tbb::concurrent_vector<Base::txString> conVecLogStr;
    tbb::parallel_for_each(outSceneStatue.vehicle().begin(), outSceneStatue.vehicle().end(), [&](auto& element) {
      conVecLogStr.emplace_back(Utils::ProtobufDebugLogString(std::ref(element)));
    }); /*lamda function*/
        /* parallel_for */

    for (Base::txInt idx = 0; idx < conVecLogStr.size(); ++idx) {
      LOG(INFO) << "[Traffic_PB_Output] " << TX_VARS(timeMgr.TimeStamp()) << conVecLogStr[idx];
    }
  }
  return true;
}

Base::txBool CloudTrafficElementSystem::FillingSpatialQuery() TX_NOEXCEPT {
  ParentClass::FillingSpatialQuery(/*ClearLastStepData()*/);
#if USE_TBB
  const auto& vecAllEdgeTrafficElement = CloudElemMgr()->GetAllEdgeVehiclePtr();
  const Base::txSize nSize = vecAllEdgeTrafficElement.size();

  tbb::parallel_for(static_cast<std::size_t>(0), vecAllEdgeTrafficElement.size(), [&](const std::size_t idx) {
    if (NonNull_Pointer(vecAllEdgeTrafficElement[idx])) {
      vecAllEdgeTrafficElement[idx]->FillingSpatialQuery();
    }
  });  /*lamda function*/
       /* parallel_for */
#endif /*USE_TBB*/

  return true;
}

Base::txBool CloudTrafficElementSystem::UpdateDITWData(Base::TimeParamManager const& timeMgr,
                                                       const PerceptionMessageData& _input_frame) TX_NOEXCEPT {
  auto allVehicles = ElemMgr()->GetAllVehiclePtr();
  const auto& const_ref_m_ditw_3rd_traffic = _input_frame;
  tbb::concurrent_unordered_set<Base::txString> conExistingVehIdSet;

  tbb::parallel_for(static_cast<std::size_t>(0), allVehicles.size(), [&](const std::size_t idx) {
    TAD_Cloud_AI_VehicleElementPtr cloud_veh_ptr =
        std::dynamic_pointer_cast<TAD_Cloud_AI_VehicleElement>(allVehicles[idx]);

    if (NonNull_Pointer(cloud_veh_ptr) &&
        (Base::IVehicleElement::VehicleSource::_3rd == cloud_veh_ptr->Vehicle_ComeFrom())) {
      const Base::txSysId _3rdVehId = cloud_veh_ptr->Id();
      const Base::txString _3rdVehIdStr = std::to_string(_3rdVehId);
      for (const auto& refCar /*ObjectInfo*/ : const_ref_m_ditw_3rd_traffic.datalist()) {
        if (_3rdVehIdStr == refCar.id()) {
          TrafficFlow::DITW_TAD_Cloud_AI_VehicleElementPtr ditw_veh_ptr =
              std::dynamic_pointer_cast<TrafficFlow::DITW_TAD_Cloud_AI_VehicleElement>(cloud_veh_ptr);
          if (NonNull_Pointer(ditw_veh_ptr)) {
            ditw_veh_ptr->Injection3rdData(refCar);
            conExistingVehIdSet.insert(_3rdVehIdStr);
          }
          break;
        }
      }
    }
  }); /*lamda function*/
      /* parallel_for */

  for (const auto& refCar /*ObjectInfo*/ : const_ref_m_ditw_3rd_traffic.datalist()) {
    if (conExistingVehIdSet.end() == conExistingVehIdSet.find(refCar.id())) {
      CloudAssemblerCtx()->Generate3rdVehicle(timeMgr, refCar, ElemMgr(), SceneLoader());
    }
  }

  if (const_ref_m_ditw_3rd_traffic.datalist_size() > 0) {
    ElemMgr()->GenerateAllTrafficElement();
  }

  return true;
}

Base::txBool CloudTrafficElementSystem::PostOpDITWData(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  Base::txBool need_update_all_element = false;
  auto allVehicles = ElemMgr()->GetAllVehiclePtr();
  std::atomic<bool> atomic_need_update_all_element(false);

  tbb::parallel_for(static_cast<std::size_t>(0), allVehicles.size(), [&](const std::size_t idx) {
    if (CallFail(allVehicles[idx]->CheckAlive(timeMgr))) {
      allVehicles[idx]->Kill();
      atomic_need_update_all_element = true;
    }
  }); /*lamda function*/
      /* parallel_for */

  if (atomic_need_update_all_element) {
    ElemMgr()->ResortKillElement();
  }
  return true;
}

Base::txBool CloudTrafficElementSystem::CheckL2W(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
#if USE_TBB
  const auto& vec_all_vehicles = ElemMgr()->GetAllVehiclePtr();
  const Base::txSize nSize = vec_all_vehicles.size();

  tbb::parallel_for(static_cast<std::size_t>(0), vec_all_vehicles.size(), [&](const std::size_t idx) {
    DITW_TAD_Cloud_AI_VehicleElementPtr cloud_ditw_veh_ptr =
        std::dynamic_pointer_cast<DITW_TAD_Cloud_AI_VehicleElement>(vec_all_vehicles[idx]);
    if (NonNull_Pointer(cloud_ditw_veh_ptr) && CallSucc(cloud_ditw_veh_ptr->IsLogSim())) {
      const Base::txFloat veh_s = cloud_ditw_veh_ptr->DistanceAlongCurve();
      const Base::Info_Lane_t& veh_laneInfo = cloud_ditw_veh_ptr->GetCurrentLaneInfo();
      if ((veh_laneInfo.onLaneUid.roadId == m_l2w_laneInfo.onLaneUid.roadId) &&
          (veh_laneInfo.onLaneUid.sectionId == m_l2w_laneInfo.onLaneUid.sectionId) && (veh_s > m_l2w_s)) {
        cloud_ditw_veh_ptr->SwitchLog2World();
      }
    }
  });  /*lamda function*/
       /* parallel_for */
#endif /*USE_TBB*/
  return true;
}

Base::txBool CloudTrafficElementSystem::ResetForecastSimScene(
    const its::txStatusRecord& status_pb_record /* size = 1 */,
    const sim_msg::TrafficRecords& traffic_pb_record /* size = 1 */, const std::string _event_json) TX_NOEXCEPT {
  /*CloudElemMgr()->ClearAllElement();
  CloudAssemblerCtx()->ResetContext();
  if (FLAGS_need_generate_scene_on_forecast) {
          CloudAssemblerCtx()->GenerateScene(ElemMgr(), SceneLoader());
  }

  ElemMgr()->Initialize(SceneLoader());*/
  EnableVehicleInput(true);

  if (status_pb_record.status_size() > 0) {
#if USE_TBB
    const its::txSimStatus& refKeyFrame = status_pb_record.status(0);
    const Base::txSize nSize = refKeyFrame.vehicle_size();
    LOG(INFO) << TX_VARS_NAME(refKeyFrame.vehicle_size(), nSize);
    tbb::parallel_for(static_cast<std::size_t>(0), nSize, [&](const std::size_t idx) {
      const its::txVehicle& refVeh = refKeyFrame.vehicle(idx);
      TAD_Cloud_AI_VehicleElementPtr new_cloud_veh_ptr = std::make_shared<TAD_Cloud_AI_VehicleElement>();
      if (NonNull_Pointer(new_cloud_veh_ptr) &&
          CallSucc(new_cloud_veh_ptr->Initialize_Cloud_MultiRegion(refVeh, SceneLoader()))) {
        ElemMgr()->AddVehiclePtr(new_cloud_veh_ptr);
        CloudDitwInfo << "create vehicle success. " << TX_VARS_NAME(veh_id, new_cloud_veh_ptr->Id());
      } else {
        LogWarn << " Create Vehicle Element Failure." << TX_VARS(idx);
      }
    }); /*lamda function*/
        /* parallel_for */
#endif  /*USE_TBB*/
  }

  if (traffic_pb_record.traffic_record_size() > 0) {
#if USE_TBB
    const sim_msg::Traffic& refKeyFrameTraffic = traffic_pb_record.traffic_record(0);
    const Base::txSize nSize = refKeyFrameTraffic.cars_size();
    LOG(INFO) << TX_VARS_NAME(refKeyFrameTraffic.cars_size(), nSize);
    tbb::parallel_for(static_cast<std::size_t>(0), nSize, [&](const std::size_t idx) {
      const sim_msg::Car& refCar = refKeyFrameTraffic.cars(idx);
      TAD_Cloud_AI_VehicleElementPtr new_cloud_veh_ptr = std::make_shared<TAD_Cloud_AI_VehicleElement>();
      if (NonNull_Pointer(new_cloud_veh_ptr) &&
          CallSucc(new_cloud_veh_ptr->Initialize_Cloud_MultiRegion(refCar, SceneLoader()))) {
        ElemMgr()->AddVehiclePtr(new_cloud_veh_ptr);
        CloudDitwInfo << "create vehicle success. " << TX_VARS_NAME(veh_id, new_cloud_veh_ptr->Id());
      } else {
        LogWarn << " Create Vehicle Element Failure." << TX_VARS(idx);
      }
    }); /*lamda function*/
        /* parallel_for */
#endif  /*USE_TBB*/
  }
  /*2. virtual city event */
  LOG(INFO) << "[virtual_city_event] " << TX_VARS(_event_json);
  Utils::VirtualCityEvent eventInfo = Utils::parser_virtual_city_event(_event_json);
  LOG(INFO) << "[virtual_city_event][parse_result] " << TX_VARS(_event_json) << TX_VARS(eventInfo.Str());
  Utils::BehaviorCfgDB::SetVehicleCfg(eventInfo.event_type);
  if ("virtual_city_crash" == eventInfo.get_event_type()) {
    m_event_type = event_type::eCrash;
    m_crash_point.FromWGS84(eventInfo.get_event_location_lon(), eventInfo.get_event_location_lat());
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
        m_crash_point, m_crash_laneInfo, m_crash_s);
    CloudDitwInfo << TX_VARS(m_crash_point) << TX_VARS(m_crash_laneInfo) << TX_VARS(m_crash_s);
    if (m_crash_s > 15.0) {
      const auto& ref_influnce_lanes = eventInfo.get_mutable_event_influnce_lane();

      if (2 == ref_influnce_lanes.size() && ref_influnce_lanes[0] != ref_influnce_lanes[1]) {
        Base::txFloat angle[] = {-37.0, 40.0};
        for (Base::txSize idx = 0; idx < ref_influnce_lanes.size(); ++idx) {
          const Base::txLaneID laneId = ref_influnce_lanes[idx];
          const Base::txLaneUId influnceLaneUid(m_crash_laneInfo.onLaneUid.roadId, m_crash_laneInfo.onLaneUid.sectionId,
                                                laneId);
          const Base::Info_Lane_t influnceLocInfo(influnceLaneUid);
          Base::txSysId new_obs_id = eventInfo.get_event_id() * 10 - laneId;
          {
            TAD_Cloud_Obstacle_VehicleElementPtr new_obstacle_veh_ptr =
                std::make_shared<TAD_Cloud_Obstacle_VehicleElement>();
            if (NonNull_Pointer(new_obstacle_veh_ptr) &&
                CallSucc(new_obstacle_veh_ptr->Initialize_Obstacle(new_obs_id, influnceLocInfo, m_crash_s, angle[idx],
                                                                   eventInfo.get_event_duration(), SceneLoader()))) {
              ElemMgr()->AddVehiclePtr(new_obstacle_veh_ptr);
              CloudDitwInfo << "create vehicle success. " << TX_VARS_NAME(veh_id, new_obstacle_veh_ptr->Id());
            } else {
              LogWarn << " Create Vehicle Element Failure.";
            }
          }
        } /*for*/
        const std::vector<Utils::EventInflunceLaneRule>& refEventLaneRule = eventInfo.event_influnce_lane_rule;
        for (Base::txSize idx = 0; idx < refEventLaneRule.size(); ++idx) {
          const auto& refRule = refEventLaneRule[idx];
          const Base::txLaneID laneId = refRule.laneId;
          const Base::txLaneUId influnceLaneUid(m_crash_laneInfo.onLaneUid.roadId, m_crash_laneInfo.onLaneUid.sectionId,
                                                laneId);
          const Base::Info_Lane_t influnceLocInfo(influnceLaneUid);
          const Base::txInt upstream_block_cnt = refRule.upstream_block;
          LOG(INFO) << TX_VARS(eventInfo.event_id) << TX_VARS(laneId) << TX_VARS(upstream_block_cnt);
          Geometry::SpatialQuery::HashedLaneInfo curInflunceHashNode = Geometry::SpatialQuery::GenerateHashedLaneInfo(
              HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(influnceLaneUid), m_crash_s, influnceLocInfo);
          std::list<Geometry::SpatialQuery::HashedLaneInfoPtr> back_hashed;
          Geometry::SpatialQuery::GetBackHashedLaneInfoList(curInflunceHashNode, back_hashed, upstream_block_cnt);
          for (Geometry::SpatialQuery::HashedLaneInfoPtr refHashedNodePtr : back_hashed) {
            const Geometry::SpatialQuery::HashedLaneInfo& refHashedNode = *refHashedNodePtr;
            if (CallFail(refRule.turn_left)) {
              HdMap::HadmapCacheConCurrent::AddDstRefuseLaneChange_Left(refHashedNode.LaneInfo().onLaneUid);
              LOG(INFO) << "[crash_influnce_lane][left] " << Utils::ToString(refHashedNode.LaneInfo().onLaneUid);
            }
            if (CallFail(refRule.turn_right)) {
              HdMap::HadmapCacheConCurrent::AddDstRefuseLaneChange_Right(refHashedNode.LaneInfo().onLaneUid);
              LOG(INFO) << "[crash_influnce_lane][right] " << Utils::ToString(refHashedNode.LaneInfo().onLaneUid);
            }
          }
        }
      } else if (2 == ref_influnce_lanes.size() && ref_influnce_lanes[0] == ref_influnce_lanes[1]) {
        Base::txFloat angle[] = {0.0, 0.0};
        Base::txFloat cur_crash_s = m_crash_s;
        for (Base::txSize idx = 0; idx < ref_influnce_lanes.size(); ++idx) {
          const Base::txLaneID laneId = ref_influnce_lanes[idx];
          const Base::txLaneUId influnceLaneUid(m_crash_laneInfo.onLaneUid.roadId, m_crash_laneInfo.onLaneUid.sectionId,
                                                laneId);
          const Base::Info_Lane_t influnceLocInfo(influnceLaneUid);
          Base::txSysId new_obs_id = eventInfo.get_event_id() * 10 - idx;

          TAD_Cloud_Obstacle_VehicleElementPtr new_obstacle_veh_ptr =
              std::make_shared<TAD_Cloud_Obstacle_VehicleElement>();
          if (NonNull_Pointer(new_obstacle_veh_ptr) &&
              CallSucc(new_obstacle_veh_ptr->Initialize_Obstacle(new_obs_id, influnceLocInfo, cur_crash_s, angle[idx],
                                                                 eventInfo.get_event_duration(), SceneLoader()))) {
            ElemMgr()->AddVehiclePtr(new_obstacle_veh_ptr);
            LOG(INFO) << "create vehicle success. " << TX_VARS_NAME(veh_id, new_obstacle_veh_ptr->Id())
                      << TX_VARS(cur_crash_s) << TX_VARS(new_obstacle_veh_ptr->GeomCenter());
          } else {
            LogWarn << " Create Vehicle Element Failure.";
          }
          LOG(INFO) << "[virtual_city_event][event_location_longitudinal_offset] "
                    << TX_VARS(eventInfo.get_event_location_longitudinal_offset());
          cur_crash_s -= (FLAGS_EGO_Length + eventInfo.get_event_location_longitudinal_offset());
        } /*for*/

        const std::vector<Utils::EventInflunceLaneRule>& refEventLaneRule = eventInfo.event_influnce_lane_rule;
        for (Base::txSize idx = 0; idx < refEventLaneRule.size(); ++idx) {
          const auto& refRule = refEventLaneRule[idx];
          const Base::txLaneID laneId = refRule.laneId;
          const Base::txLaneUId influnceLaneUid(m_crash_laneInfo.onLaneUid.roadId, m_crash_laneInfo.onLaneUid.sectionId,
                                                laneId);
          const Base::Info_Lane_t influnceLocInfo(influnceLaneUid);
          const Base::txInt upstream_block_cnt = refRule.upstream_block;
          LOG(INFO) << TX_VARS(eventInfo.event_id) << TX_VARS(laneId) << TX_VARS(upstream_block_cnt);
          Geometry::SpatialQuery::HashedLaneInfo curInflunceHashNode = Geometry::SpatialQuery::GenerateHashedLaneInfo(
              HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(influnceLaneUid), m_crash_s, influnceLocInfo);
          std::list<Geometry::SpatialQuery::HashedLaneInfoPtr> back_hashed;
          Geometry::SpatialQuery::GetBackHashedLaneInfoList(curInflunceHashNode, back_hashed, upstream_block_cnt);
          for (Geometry::SpatialQuery::HashedLaneInfoPtr refHashedNodePtr : back_hashed) {
            const Geometry::SpatialQuery::HashedLaneInfo& refHashedNode = *refHashedNodePtr;
            if (CallFail(refRule.turn_left)) {
              HdMap::HadmapCacheConCurrent::AddDstRefuseLaneChange_Left(refHashedNode.LaneInfo().onLaneUid);
              LOG(INFO) << "[crash_influnce_lane][left] " << Utils::ToString(refHashedNode.LaneInfo().onLaneUid);
            }
            if (CallFail(refRule.turn_right)) {
              HdMap::HadmapCacheConCurrent::AddDstRefuseLaneChange_Right(refHashedNode.LaneInfo().onLaneUid);
              LOG(INFO) << "[crash_influnce_lane][right] " << Utils::ToString(refHashedNode.LaneInfo().onLaneUid);
            }
          }
        }
      }
    } else {
      LogWarn << TX_VARS(m_crash_s) << " error.";
      return false;
    }

    m_crash_max_speed = eventInfo.get_event_influnce_roads_speed();

    m_crash_speed_limit_start_hashedNodeList.clear();
    m_crash_speed_limit_end_hashedNodeList.clear();

    Coord::txWGS84 speed_limit_start;
    speed_limit_start.FromWGS84(eventInfo.get_event_speed_limit_start_lon(),
                                eventInfo.get_event_speed_limit_start_lat());
    Coord::txWGS84 speed_limit_end;
    speed_limit_end.FromWGS84(eventInfo.get_event_speed_limit_end_lon(), eventInfo.get_event_speed_limit_end_lat());

    LOG(INFO) << TX_VARS(speed_limit_start) << TX_VARS(speed_limit_end);
    Base::Info_Lane_t speed_limit_start_laneInfo, speed_limit_end_laneInfo;
    Base::txFloat speed_limit_start_s, speed_limit_end_s;
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
        speed_limit_start, speed_limit_start_laneInfo, speed_limit_start_s);
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
        speed_limit_end, speed_limit_end_laneInfo, speed_limit_end_s);

    hadmap::txLanes speed_limit_start_lanes_with_invalid;
    hadmap::getLanes(txMapHdr,
                     hadmap::txLaneId(speed_limit_start_laneInfo.onLaneUid.roadId,
                                      speed_limit_start_laneInfo.onLaneUid.sectionId, LANE_PKID_INVALID),
                     speed_limit_start_lanes_with_invalid);

    for (Base::txInt i = 0; i < speed_limit_start_lanes_with_invalid.size(); ++i) {
      const auto laneType = speed_limit_start_lanes_with_invalid[i]->getLaneType();
      if ((Utils::IsDrivingLane(laneType))) {
        const Base::txLaneUId& influnceLaneUid = speed_limit_start_lanes_with_invalid[i]->getTxLaneId();
        const Base::Info_Lane_t influnceLocInfo(influnceLaneUid);
        Geometry::SpatialQuery::HashedLaneInfo speed_limit_start_lane_HashNode =
            Geometry::SpatialQuery::GenerateHashedLaneInfo(
                HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(influnceLaneUid), m_crash_s, influnceLocInfo);

        std::list<Geometry::SpatialQuery::HashedLaneInfoPtr> front_hashed;
        Geometry::SpatialQuery::GetFrontHashedLaneInfoList(speed_limit_start_lane_HashNode, front_hashed, 4);
        m_crash_speed_limit_start_hashedNodeList.push_back(speed_limit_start_lane_HashNode);
        for (Geometry::SpatialQuery::HashedLaneInfoPtr refHashedNodePtr : front_hashed) {
          const Geometry::SpatialQuery::HashedLaneInfo& refHashedNode = *refHashedNodePtr;
          m_crash_speed_limit_start_hashedNodeList.push_back(refHashedNode);
        }
      } else {
        LOG(WARNING) << "unsupport laneType " << laneType
                     << TX_VARS_NAME(srcLaneUid, speed_limit_start_lanes_with_invalid[i]->getTxLaneId());
      }
    }

    hadmap::txLanes speed_limit_end_lanes_with_invalid;
    hadmap::getLanes(txMapHdr,
                     hadmap::txLaneId(speed_limit_end_laneInfo.onLaneUid.roadId,
                                      speed_limit_end_laneInfo.onLaneUid.sectionId, LANE_PKID_INVALID),
                     speed_limit_end_lanes_with_invalid);
    for (Base::txInt i = 0; i < speed_limit_end_lanes_with_invalid.size(); ++i) {
      const auto laneType = speed_limit_end_lanes_with_invalid[i]->getLaneType();
      if ((Utils::IsDrivingLane(laneType))) {
        const Base::txLaneUId& influnceLaneUid = speed_limit_end_lanes_with_invalid[i]->getTxLaneId();
        const Base::Info_Lane_t influnceLocInfo(influnceLaneUid);
        Geometry::SpatialQuery::HashedLaneInfo speed_limit_start_lane_HashNode =
            Geometry::SpatialQuery::GenerateHashedLaneInfo(
                HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(influnceLaneUid), m_crash_s, influnceLocInfo);

        std::list<Geometry::SpatialQuery::HashedLaneInfoPtr> front_hashed;
        Geometry::SpatialQuery::GetFrontHashedLaneInfoList(speed_limit_start_lane_HashNode, front_hashed, 4);
        /*m_crash_speed_limit_end_hashedNodeList.push_back(speed_limit_start_lane_HashNode);*/
        for (Geometry::SpatialQuery::HashedLaneInfoPtr refHashedNodePtr : front_hashed) {
          const Geometry::SpatialQuery::HashedLaneInfo& refHashedNode = *refHashedNodePtr;
          m_crash_speed_limit_end_hashedNodeList.push_back(refHashedNode);
        }
      } else {
        LOG(WARNING) << "unsupport laneType " << laneType
                     << TX_VARS_NAME(srcLaneUid, speed_limit_end_lanes_with_invalid[i]->getTxLaneId());
      }
    }

    for (const auto& refNode : m_crash_speed_limit_start_hashedNodeList) {
      LOG(WARNING) << "[speed_limit_start] " << refNode;
    }

    for (const auto& refNode : m_crash_speed_limit_end_hashedNodeList) {
      LOG(WARNING) << "[speed_limit_end] " << refNode;
    }
  } else if ("virtual_city_fog" == eventInfo.get_event_type()) {
    m_event_type = event_type::eFog;
    m_fog_max_speed = eventInfo.get_event_influnce_roads_speed();
    m_fog_roads_set.clear();
    for (const auto roadId : eventInfo.get_event_influnce_roads()) {
      m_fog_roads_set.insert(roadId);
    }
  } else {
    m_event_type = event_type::eNone;
    LogWarn << "[virtual_city_event] un support event type : " << eventInfo.get_event_type();
  }
  return true;
}

Base::txBool CloudTrafficElementSystem::Fog() TX_NOEXCEPT {
  const auto& vec_all_vehicles = ElemMgr()->GetAllVehiclePtr();
  const Base::txSize nSize = vec_all_vehicles.size();

  tbb::parallel_for(static_cast<std::size_t>(0), vec_all_vehicles.size(), [&](const std::size_t idx) {
    auto veh_ptr = vec_all_vehicles[idx];
    if (NonNull_Pointer(veh_ptr)) {
      const Base::txRoadID rid = veh_ptr->GetCurrentLaneInfo().onLaneUid.roadId;
      if (m_fog_roads_set.count(rid) > 0) {
        veh_ptr->SetVelocityDesired(m_fog_max_speed);
      }
    }
  }); /*lamda function*/
      /* parallel_for */
  return true;
}

Base::txBool CloudTrafficElementSystem::Crash() TX_NOEXCEPT {
#if 0

  const auto & vec_all_vehicles = ElemMgr()->GetAllVehiclePtr();
  const Base::txSize nSize = vec_all_vehicles.size();

  tbb::parallel_for(static_cast<std::size_t>(0), vec_all_vehicles.size(),
    [&](const std::size_t idx) {
    auto veh_ptr = vec_all_vehicles[idx];
    if (NonNull_Pointer(veh_ptr)) {
      const Base::txRoadID rid = veh_ptr->GetCurrentLaneInfo().onLaneUid.roadId;
      const Base::txSectionID sid = veh_ptr->GetCurrentLaneInfo().onLaneUid.sectionId;
      if (9 == rid && sid >= 2) {
        if (sid <= 6) {
          veh_ptr->SetVelocityDesired(16.0);
        } else {
          veh_ptr->ResetVelocityDesired();
        }
      }
    }
  }); /*lamda function*/
      /* parallel_for */
#else
  HdMap::HashedRoadCacheConCurrent::VehicleContainer res_need_speed_limit;
  HdMap::HashedRoadCacheConCurrent::QueryRegisterVehicles(m_crash_speed_limit_start_hashedNodeList,
                                                          res_need_speed_limit);
  for (auto& ref_pair : res_need_speed_limit) {
    if (NonNull_Pointer(ref_pair.second)) {
      ref_pair.second->SetVelocityDesired(m_crash_max_speed);
    }
  }

  HdMap::HashedRoadCacheConCurrent::VehicleContainer res_need_speed_reset;
  HdMap::HashedRoadCacheConCurrent::QueryRegisterVehicles(m_crash_speed_limit_end_hashedNodeList, res_need_speed_reset);
  for (auto& ref_pair : res_need_speed_reset) {
    if (NonNull_Pointer(ref_pair.second)) {
      ref_pair.second->ResetVelocityDesired();
    }
  }

#endif
  return true;
}

Base::txBool CloudTrafficElementSystem::UpdatePlanningCarData(Base::TimeParamManager const& timeMgr,
                                                              const Base::Enums::EgoSubType _egoSubType,
                                                              const Base::txString& egoInfoStr) TX_NOEXCEPT {
  if (_NonEmpty_(egoInfoStr)) {
    auto retEgoVec = (ElemMgr()->GetEgoArray());
    // LogWarn << TX_VARS(timeMgr.PassTime()) << TX_VARS(retEgoVec.size());
    for (auto refEgo : retEgoVec) {
      if (IsSupportSceneType(SceneLoader()->GetSceneType())) {
        TrafficFlow::TAD_EgoVehicleElementPtr curEgoPtr =
            std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(refEgo);
        if (NonNull_Pointer(curEgoPtr) && _egoSubType == (curEgoPtr->GetSubType())) {
          curEgoPtr->SetProtoBufferStr(egoInfoStr);
          curEgoPtr->SetValid(true);
          return true;
        }
      }
    }
  }
  return false;
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogPerfStatistic
#undef CloudDitwInfo
#undef InjectEventInfo
#undef LogWarn
