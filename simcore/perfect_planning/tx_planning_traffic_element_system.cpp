// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_planning_traffic_element_system.h"
#include "tx_dummy_event_dispatcher_1_0_0_0.h"
#include "tx_planning_flags.h"
#include "tx_planning_scene_loader.h"
#include "tx_planning_vehicle_element.h"
#include "tx_planning_vehicle_element_trajectory_follow.h"
#include "tx_trajectory_planning_vehicle_element.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_System)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool PlanningTrafficElementSystem::IsSupportSceneType(const Base::ISceneLoader::ESceneType _sceneType) const
    TX_NOEXCEPT {
  if (_plus_(Base::ISceneLoader::ESceneType::eTAD_Ego) == (_sceneType) ||
      _plus_(Base::ISceneLoader::ESceneType::eTAD_DummyDriver) == (_sceneType)) {
    return true;
  } else {
    return false;
  }
}

Base::txBool PlanningTrafficElementSystem::Release() TX_NOEXCEPT {
  m_ego_vehicle_ptr = nullptr;
  m_scene_event_loader = nullptr;
  m_PlanningMode = PlanningMode::ePlanning;
  return ParentClass::Release();
}

Base::txBool PlanningTrafficElementSystem::Initialize(Base::ISceneLoaderPtr loader) TX_NOEXCEPT {
  m_scene_event_loader = std::dynamic_pointer_cast<SceneLoader::PlanningSceneEvent_SceneLoader>(loader);
  if (NonNull_Pointer(SceneEventLoader()) && NonNull_Pointer(loader)) {
    m_SceneDataSource = loader;
    m_PlanningMode = ComputePlanningMode(SceneEventLoader()->GetEgoFromGroupName());
    if (CallSucc(RegisterPlanningCar())) {
      LogInfo << "GenerateScene Success.";
      m_isAlive = true;
    } else {
      LogWarn << "GenerateScene Failure.";
      m_isAlive = false;
    }
  } else {
    LogWarn << "GenerateScene Failure. Data is Null.";
    m_isAlive = false;
  }
  return IsAlive();
}

CentripetalCatMull::control_path_node_vec PlanningTrafficElementSystem::ParserControlPath(
    const std::vector<sim_msg::Waypoint>& _ControlPathWayPts) const TX_NOEXCEPT {
  using txString = Base::txString;
  control_path_node_vec ret_conrtol_path;
  if (_NonEmpty_(_ControlPathWayPts)) {
    TX_MARK(R"(message Waypoint {
                            Position position = 1;
                            string lane_id = 2;
                            double start_s = 3;
                            double l_offset = 4;
                            double speed = 5;  // m/s
                            double accel = 6;  // m/s2
                            double heading_angle = 7;  // unit: degree
                            double front_wheel_steer_angle = 8;  // unit: degree
                            GearSts gear = 9;
                        })");
    for (const auto& way_pt : _ControlPathWayPts) {
      if (CallSucc(way_pt.position().has_world())) {
        control_path_node cp_node;
        __Lon__(cp_node.waypoint) = way_pt.position().world().x();
        __Lat__(cp_node.waypoint) = way_pt.position().world().y();
        __Alt__(cp_node.waypoint) = way_pt.position().world().z();
        cp_node.speed_m_s = way_pt.speed().value();
        cp_node.gear =
            ((sim_msg::GearSts::GEAR_STS_R == way_pt.gear()) ? (_plus_(Base::Enums::ControlPathGear::reverse))
                                                             : (_plus_(Base::Enums::ControlPathGear::drive)));
        ret_conrtol_path.emplace_back(std::move(cp_node));

      } else {
        LOG(WARNING) << "control path point is not world type." << way_pt.position().DebugString();
      }
    }
  }
  return std::move(ret_conrtol_path);
}

PlanningTrafficElementSystem::PlanningMode PlanningTrafficElementSystem::ComputePlanningMode(
    const sim_msg::Ego& cur_ego) const TX_NOEXCEPT {
  if (CallSucc(cur_ego.initial().trajectory_enabled())) {
    LogWarn << "PlanningMode::eTrajFollow, scene config";
    return PlanningMode::eTrajFollow;
  } else {
    LogWarn << "PlanningMode::ePlanning, scene config";
    return PlanningMode::ePlanning;
  }
}

Base::txString PlanningTrafficElementSystem::Enum2Str(const PlanningMode _mode) TX_NOEXCEPT {
  switch (_mode) {
    case PlanningMode::eBare:
      return "eBare";
    case PlanningMode::ePlanning:
      return "ePlanning";
    case PlanningMode::eTrajFollow:
      return "eTrajFollow";
    default:
      break;
  }
  return "unknown";
}

Base::txBool PlanningTrafficElementSystem::RegisterPlanningCar() TX_NOEXCEPT {
  LogInfo << "Call RegisterPlanningCar";
  m_isAlive = false;
  if (NonNull_Pointer(SceneLoader()) && NonNull_Pointer(SceneEventLoader()) &&
      CallSucc(IsSupportSceneType(SceneLoader()->GetSceneType()))) {
    SimPlanningVehicleElementPtr egoVehiclePtr = nullptr;
    SimPlanningVehicleElement::control_path_node_vec control_path;

    switch (m_PlanningMode) {
      case PlanningMode::eBare: {
        egoVehiclePtr = std::make_shared<SimPlanningVehicleElement_BareMode>();
        break;
      }
      case PlanningMode::eTrajFollow: {
        control_path = ParserControlPath(SceneEventLoader()->ControlPathPoints());
        egoVehiclePtr = std::make_shared<SimPlanningVehicleElement_TrajectoryFollow>();
        break;
      }
      case PlanningMode::ePlanning:
      default: {
        if (FLAGS_use_trajectory_planning_mode) {
          egoVehiclePtr = std::make_shared<PlanningVechicleElement>();
        } else {
          egoVehiclePtr = std::make_shared<SimPlanningVehicleElement>();
        }
        break;
      }
    }

    m_ego_vehicle_ptr = egoVehiclePtr;
    if (NonNull_Pointer(m_ego_vehicle_ptr)) {
      sim_msg::Location ego_start;
      SceneLoader()->GetRoutingInfo(ego_start);
      if (CallSucc(
              egoVehiclePtr->InitializeEgo(ego_start, control_path, SceneLoader()->GetEgoRouteData(), SceneLoader()))) {
        txPlanningSceneEventModulePtr scene_event_moduler_ptr =
            std::dynamic_pointer_cast<txPlanningSceneEventModule>(egoVehiclePtr);
        TAD_SceneEvent_1_0_0_0_Ptr sceneEventPtr = std::dynamic_pointer_cast<TAD_SceneEvent_1_0_0_0>(egoVehiclePtr);
        if (NonNull_Pointer(scene_event_moduler_ptr) && NonNull_Pointer(scene_event_moduler_ptr->SceneEventViewPtr()) &&
            NonNull_Pointer(sceneEventPtr) && CallSucc(sceneEventPtr->UseSceneEvent_1_0_0_0())) {
          LogInfo << "[dummy_event][scene_event_1.1.0.0] dummy_ego_id:" << (*egoVehiclePtr).Id()
                  << TX_VARS_NAME(sceneEventCnt,
                                  scene_event_moduler_ptr->SceneEventViewPtr()->DummySceneEventViewerPtrVec().size());
          std::vector<TrafficFlow::TAD_SceneEvent_1_0_0_0::ISceneEventViewerPtr> SceneEvent;
          for (auto dummy_scene_event_ptr :
               scene_event_moduler_ptr->SceneEventViewPtr()->DummySceneEventViewerPtrVec()) {
            SceneEvent.emplace_back(dummy_scene_event_ptr);
          }
          TrafficFlow::txDummyEventDispatcher_v_1_0_0_0::getInstance().RegisterVehicleSceneEvent(SceneEvent,
                                                                                                 sceneEventPtr);
        } else {
          LogWarn << "current mode unsupport scene event." << TX_VARS_NAME(m_PlanningMode, Enum2Str(m_PlanningMode))
                  << TX_COND(NonNull_Pointer(scene_event_moduler_ptr)) << TX_COND(NonNull_Pointer(sceneEventPtr));
        }
        m_isAlive = true;
        LogWarn << "InitializeEgo success.";
        return IsAlive();
      } else {
        LogWarn << "InitializeEgo failure.";
        return IsAlive();
      }
    } else {
      LOG(WARNING) << "GenerateEgo Failure.";
      return IsAlive();
    }
  } else {
    LogWarn << TX_COND(NonNull_Pointer(SceneLoader())) << TX_COND(NonNull_Pointer(SceneEventLoader()))
            << TX_COND(CallSucc(IsSupportSceneType(SceneLoader()->GetSceneType())));
    return IsAlive();
  }
}

void PlanningTrafficElementSystem::ExecuteEnvPerception(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (IsAlive() && NonNull_Pointer(GetEgoVehicleElementPtr())) {
    Base::IEnvPerceptionPtr env_perception_interface_ptr =
        std::dynamic_pointer_cast<Base::IEnvPerception>(GetEgoVehicleElementPtr());
    if (NonNull_Pointer(env_perception_interface_ptr)) {
      env_perception_interface_ptr->ExecuteEnvPerception(timeMgr);
    } else {
      LogWarn << "IEnvPerception is nullptr.";
    }
  } else {
    LogWarn << "ExecuteEnvPerception failure." << TX_COND(IsAlive())
            << TX_COND(NonNull_Pointer(GetEgoVehicleElementPtr()));
  }
}

void PlanningTrafficElementSystem::UpdateSurroundTraffic(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  std::get<__LifeTimeIdx__>(m_traffic_mgr) -= timeMgr.RelativeTime();
}

Base::txBool PlanningTrafficElementSystem::Update_Ego_PreSimulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  if (FLAGS_ego_location_closed_loop) {
    SimPlanningVehicleElementPtr egoPtr =
        std::dynamic_pointer_cast<SimPlanningVehicleElement>(GetEgoVehicleElementPtr());
    if (NonNull_Pointer(egoPtr)) {
      return egoPtr->UpdateLocation(timeMgr);
    } else {
      LogWarn << "HasEgo false.";
      return false;
    }
  } else {
    return true;
  }
}

Base::txBool PlanningTrafficElementSystem::Update_Ego_Simulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(GetEgoVehicleElementPtr())) {
    return GetEgoVehicleElementPtr()->Update(timeMgr);
  } else {
    return false;
  }
}

Base::txBool PlanningTrafficElementSystem::Update_Ego_PostSimulation(const Base::TimeParamManager& timeMgr)
    TX_NOEXCEPT {
  if (NonNull_Pointer(GetEgoVehicleElementPtr())) {
    return GetEgoVehicleElementPtr()->Post_Update(timeMgr);
  } else {
    return false;
  }
}

Base::txBool PlanningTrafficElementSystem::Update_Ego_Signal(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  return true;
}

Base::txBool PlanningTrafficElementSystem::Update_Ego_LifeCycle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  return GetEgoVehicleElementPtr()->CheckStart(timeMgr);
}

Base::txBool PlanningTrafficElementSystem::Update(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  if (IsAlive()) {
    Update_Ego_LifeCycle(timeMgr);

    Update_Ego_Signal(timeMgr);

    Update_Ego_PreSimulation(timeMgr);

    FlushSceneEvents(timeMgr);

    ExecuteEnvPerception(timeMgr);

    Update_Ego_Simulation(timeMgr);

    Update_Ego_PostSimulation(timeMgr);

    if (timeMgr.PassTime() > 0.0 &&
        (CallFail(GetEgoVehicleElementPtr()->IsAlive()) || CallSucc(GetEgoVehicleElementPtr()->IsStop())) &&
        CallSucc(FLAGS_ego_stop_system_unlive)) {
      m_isAlive = false;
    }

    return IsAlive();
  } else {
    LogWarn << "Update failure." << TX_COND(IsAlive());
    return IsAlive();
  }
}

Base::txBool PlanningTrafficElementSystem::UpdatePlanningCarData(Base::TimeParamManager const& timeMgr,
                                                                 const Base::Enums::EgoSubType _egoSubType,
                                                                 const Base::txString& egoLocationInfoStr) TX_NOEXCEPT {
  SimPlanningVehicleElementPtr egoPtr = std::dynamic_pointer_cast<SimPlanningVehicleElement>(GetEgoVehicleElementPtr());
  if (NonNull_Pointer(egoPtr)) {
    return egoPtr->InjectLoc(timeMgr, egoLocationInfoStr);
  } else {
    return false;
  }
}

Base::txBool PlanningTrafficElementSystem::UpdateEnvData(Base::TimeParamManager const& timeMgr,
                                                         const Base::txString& trafficInfoStr) TX_NOEXCEPT {
  SimPlanningVehicleElementPtr egoPtr = std::dynamic_pointer_cast<SimPlanningVehicleElement>(GetEgoVehicleElementPtr());
  if (NonNull_Pointer(egoPtr)) {
    return egoPtr->InjectEnv(timeMgr, trafficInfoStr);
  } else {
    return false;
  }
}

Base::txBool PlanningTrafficElementSystem::UpdateGroupPlanningCarData(
    Base::TimeParamManager const& timeMgr, const Base::txString strGroupName,
    const Base::txString& egoLocationInfoStr) TX_NOEXCEPT {
  SimPlanningVehicleElementPtr egoPtr = std::dynamic_pointer_cast<SimPlanningVehicleElement>(GetEgoVehicleElementPtr());
  if (NonNull_Pointer(egoPtr)) {
    return egoPtr->InjectUnionLoc(timeMgr, strGroupName, egoLocationInfoStr);
  } else {
    return false;
  }
  return true;
}

Base::txBool PlanningTrafficElementSystem::FillingLocationData(Base::TimeParamManager const& timeMgr,
                                                               sim_msg::Location& outLocation) TX_NOEXCEPT {
  SimPlanningVehicleElementPtr fillingPtr =
      std::dynamic_pointer_cast<SimPlanningVehicleElement>(GetEgoVehicleElementPtr());
  if (NonNull_Pointer(fillingPtr)) {
    return fillingPtr->FillingElement(timeMgr, outLocation);
  } else {
    return false;
  }
}

Base::txBool PlanningTrafficElementSystem::FillingTrajectoryData(Base::TimeParamManager const& timeMgr,
                                                                 sim_msg::Trajectory& outTrajectory) TX_NOEXCEPT {
  SimPlanningVehicleElementPtr fillingPtr =
      std::dynamic_pointer_cast<SimPlanningVehicleElement>(GetEgoVehicleElementPtr());
  if (NonNull_Pointer(fillingPtr)) {
    return fillingPtr->FillingTrajectory(timeMgr, outTrajectory);
  } else {
    return false;
  }
}

Base::txBool PlanningTrafficElementSystem::FillingSpatialQuery() TX_NOEXCEPT {
  if (IsAlive() && GetEgoVehicleElementPtr()) {
    return GetEgoVehicleElementPtr()->FillingSpatialQuery();
  } else {
    return false;
  }
}

void PlanningTrafficElementSystem::FlushSceneEvents(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (timeMgr.PassTime() > 0.0) {
    TrafficFlow::txDummyEventDispatcher_v_1_0_0_0::getInstance().FlushAllEvents(timeMgr, ElemMgr());
    SimPlanningVehicleElementPtr fillingPtr =
        std::dynamic_pointer_cast<SimPlanningVehicleElement>(GetEgoVehicleElementPtr());
    if (NonNull_Pointer(fillingPtr)) {
      fillingPtr->EventChangeLane(timeMgr,
                                  TrafficFlow::txDummyEventDispatcher_v_1_0_0_0::getInstance().GetLaneChange());
    }
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
