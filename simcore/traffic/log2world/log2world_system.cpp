// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "log2world_system.h"
#include "l2w_vehicle_element.h"
#include "tad_ego_vehicle_element.h"
#include "tad_linear_element_manager.h"
#include "tx_parallel_def.h"
#include "tx_protobuf_utils.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_L2W)
#define LogL2WEgoInfo LOG_IF(INFO, FLAGS_LogLevel_L2W_EgoInfo)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool Log2WorldSystem::Initialize(Base::ISceneLoaderPtr loader) TX_NOEXCEPT {
  if (NonNull_Pointer(loader)) {
    m_l2w_SceneLoader = std::dynamic_pointer_cast<SceneLoader::Simrec_SceneLoader>(loader);
    return ParentClass::Initialize(SimrecSceneLoader());
  } else {
    LOG(WARNING) << "param loader is nullptr.";
    return false;
  }
}

void Log2WorldSystem::CreateAssemblerCtx() TX_NOEXCEPT {
  m_l2w_AssemblerContextPtr = std::make_shared<Scene::L2W_AssemblerContext>();
  m_AssemblerCtx = m_l2w_AssemblerContextPtr;
  if (NonNull_Pointer(AssemblerCtx())) {
    AssemblerCtx()->Init(SceneLoader());
  }
}

sim_msg::Location Log2WorldSystem::GetEgoLocation(const EgoSubType _egoSubType) const TX_NOEXCEPT {
  auto retEgoVec = (ElemMgr()->GetEgoArray());
  for (auto refEgo : retEgoVec) {
    TrafficFlow::TAD_EgoVehicleElementPtr curEgoPtr =
        std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(refEgo);
    if (NonNull_Pointer(curEgoPtr) && _egoSubType == (curEgoPtr->GetSubType())) {
      return std::move(curEgoPtr->GetProtoBufferNode());
    }
  }
  return std::move(sim_msg::Location());
}

Base::txBool Log2WorldSystem::UpdatePlanningCarData(Base::TimeParamManager const& timeMgr,
                                                    const Base::Enums::EgoSubType _egoSubType) TX_NOEXCEPT {
  if (NonNull_Pointer(SimrecSceneLoader())) {
    sim_msg::Location newEgoLocation;
    if (SimrecSceneLoader()->InterpEgoLocation(timeMgr, newEgoLocation)) {
      LogL2WEgoInfo << timeMgr.str() << " [Logsim Ego Location] " << Utils::ProtobufDebugJson(&newEgoLocation);
      auto retEgoVec = (ElemMgr()->GetEgoArray());
      for (auto refEgo : retEgoVec) {
        if (IsSupportSceneType(SceneLoader()->GetSceneType())) {
          TrafficFlow::TAD_EgoVehicleElementPtr curEgoPtr =
              std::dynamic_pointer_cast<TrafficFlow::TAD_EgoVehicleElement>(refEgo);
          if (NonNull_Pointer(curEgoPtr) && _egoSubType == (curEgoPtr->GetSubType())) {
            Base::txString egoInfoStr;
            newEgoLocation.SerializeToString(&egoInfoStr);
            curEgoPtr->SetProtoBufferStr(egoInfoStr);
            curEgoPtr->SetValid(true);
            return true;
          }
        }
      }
    } else {
      TX_MARK("see TAD_EgoVehicleElement::FillingElement set valid false.");
    }
  }
  return false;
}

void Log2WorldSystem::SwitchSim(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  {
    auto refAllVehicleArray = ElemMgr()->GetAllVehiclePtr();
    tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray.size(), [&](const std::size_t idx) {
      TrafficFlow::L2W_VehicleElementPtr l2w_veh_ptr =
          std::dynamic_pointer_cast<TrafficFlow::L2W_VehicleElement>(refAllVehicleArray[idx]);
      if (NonNull_Pointer(l2w_veh_ptr)) {
        l2w_veh_ptr->SwitchLog2World();
      }
    }); /*lamda function*/
        /* parallel_for */
  }
}

void Log2WorldSystem::CreateElemMgr() TX_NOEXCEPT {
  m_l2w_elemMgr = std::make_shared<TrafficFlow::L2W_LinearElementManager>();
  m_ElementMgr = m_l2w_elemMgr;
}

Base::txBool Log2WorldSystem::Update_CheckLifeCycle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  Base::txBool bRet = ParentClass::Update_CheckLifeCycle(timeMgr);

  if (CallSucc(bRet)) {
    if (NonNull_Pointer(MultiLayerElemMgr()) && NonNull_Pointer(MultiLayerElemMgr()->ShadowLayer())) {
      auto& ref_All_shadow_Element = MultiLayerElemMgr()->ShadowLayer()->GetAllElement();
      tbb::parallel_for(static_cast<std::size_t>(0), ref_All_shadow_Element.size(), [&](const std::size_t idx) {
        auto& elemPtr = ref_All_shadow_Element[idx];
        if (NonNull_Pointer(elemPtr)) {
          elemPtr->CheckStart(timeMgr);
          elemPtr->CheckEnd(timeMgr);
        }
      }); /*lamda function*/
          /* parallel_for */
      return true;
    } else {
      LogWarn << "MultiLayerElemMgr error.";
      return false;
    }
  } else {
    return bRet;
  }
}

Base::txBool Log2WorldSystem::Update_Vehicle_Simulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  Base::txBool bRet = ParentClass::Update_Vehicle_Simulation(timeMgr);
  // return bRet;
  if (CallSucc(bRet)) {
    LOG(WARNING) << "########################### Shadow Update Start #####################################";
    if (NonNull_Pointer(MultiLayerElemMgr()) && NonNull_Pointer(MultiLayerElemMgr()->ShadowLayer())) {
      auto& ref_shadow_VehicleArray =
          MultiLayerElemMgr()->ShadowLayer()->SearchElementByType(Base::Enums::ElementType::TAD_Vehicle);
      tbb::parallel_for(static_cast<std::size_t>(0), ref_shadow_VehicleArray.size(), [&](const std::size_t idx) {
        auto& elemPtr = ref_shadow_VehicleArray[idx];
        if (NonNull_Pointer(elemPtr)) {
          elemPtr->Update(timeMgr);
        }
      }); /*lamda function*/
          /* parallel_for */
      LOG(WARNING) << "########################### Shadow Update End #####################################";
      return true;
    } else {
      LogWarn << "MultiLayerElemMgr error.";
      LOG(WARNING) << "########################### Shadow Update End #####################################";
      return false;
    }
  } else {
    return bRet;
  }
}

Base::txBool Log2WorldSystem::FillingTrafficDataShadow(Base::TimeParamManager const& timeMgr,
                                                       sim_msg::Traffic& outTraffic) TX_NOEXCEPT {
  outTraffic.Clear();

  if (NonNull_Pointer(MultiLayerElemMgr()) && NonNull_Pointer(MultiLayerElemMgr()->ShadowLayer())) {
#if 1
    {
      /*all vehicles*/
      Base::ISimulator::AtomicSize atom_cars_size = {0};
      auto& refAllVehicleArray_shadow = MultiLayerElemMgr()->ShadowLayer()->GetAllVehiclePtr();

      tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray_shadow.size(), [&](const std::size_t idx) {
        Base::ISimulatorPtr simulator_interface_ptr =
            std::dynamic_pointer_cast<Base::ISimulator>(refAllVehicleArray_shadow[idx]);
        if (NonNull_Pointer(simulator_interface_ptr)) {
          simulator_interface_ptr->PreFillingElement(atom_cars_size);
        }
      }); /*lamda function*/
          /* parallel_for */

      const Base::ISimulator::AtomicSizeValueType n_cars_size = atom_cars_size;
      LOG(WARNING) << TX_VARS_NAME(shadow_car_size, n_cars_size);
      ResetPreAllocateCar(n_cars_size);
      *outTraffic.mutable_cars() = {m_PreAllocateCars.begin(), (m_PreAllocateCars.begin() + n_cars_size)};

      tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicleArray_shadow.size(), [&](const std::size_t idx) {
        Base::ISimulatorPtr simulator_interface_ptr =
            std::dynamic_pointer_cast<Base::ISimulator>(refAllVehicleArray_shadow[idx]);
        if (NonNull_Pointer(simulator_interface_ptr)) {
          simulator_interface_ptr->FillingElement(timeMgr, outTraffic);
        }
      }); /*lamda function*/
          /* parallel_for */
    }
    {
      /*all pedestrians*/
      auto& refAllPedestrianArray = MultiLayerElemMgr()->ShadowLayer()->GetAllPedestrianPtr();
      for (auto refPedestrianPtr : refAllPedestrianArray) {
        if (NonNull_Pointer(refPedestrianPtr) && CallSucc(refPedestrianPtr->IsAlive())) {
          refPedestrianPtr->FillingElement(timeMgr, outTraffic);
        }
      }
    }
    {
      /*all obstacles*/
      auto& refAllObstacleArray = MultiLayerElemMgr()->ShadowLayer()->GetAllObstaclePtr();
      for (auto refObstaclePtr : refAllObstacleArray) {
        if (NonNull_Pointer(refObstaclePtr) && CallSucc(refObstaclePtr->IsAlive())) {
          refObstaclePtr->FillingElement(timeMgr, outTraffic);
        }
      }
    }
#else
    MultiLayerElemMgr()->ShadowLayer()->GenerateAllTrafficElement();
    auto& refAll_Shadow_ElementArray = MultiLayerElemMgr()->ShadowLayer()->GetAllElement();
    tbb::parallel_for(static_cast<std::size_t>(0), refAll_Shadow_ElementArray.size(), [&](const std::size_t idx) {
      Base::ISimulatorPtr simulator_interface_ptr =
          std::dynamic_pointer_cast<Base::ISimulator>(refAll_Shadow_ElementArray[idx]);
      if (NonNull_Pointer(simulator_interface_ptr)) {
        simulator_interface_ptr->FillingElement(timeMgr, outTraffic);
      }
    }); /*lamda function*/
        /* parallel_for */
#endif
  }

  auto& refAll_Shadow_Signals = MultiLayerElemMgr()->ShadowLayer()->GetAllSignal();
  if (refAll_Shadow_Signals.size() > 10) {
    tbb::parallel_for(static_cast<std::size_t>(0), refAll_Shadow_Signals.size(), [&](const std::size_t idx) {
      Base::ISimulatorPtr simulator_interface_ptr =
          std::dynamic_pointer_cast<Base::ISimulator>(refAll_Shadow_Signals[idx]);
      if (NonNull_Pointer(simulator_interface_ptr)) {
        simulator_interface_ptr->FillingElement(timeMgr, outTraffic);
      }
    }); /*lamda function*/
        /* parallel_for */
  } else {
    for (auto& refSignal : refAll_Shadow_Signals) {
      if (NonNull_Pointer(refSignal)) {
        refSignal->FillingElement(timeMgr, outTraffic);
      }
    }
  }
  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)
