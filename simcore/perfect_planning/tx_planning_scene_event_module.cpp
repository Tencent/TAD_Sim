// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_planning_scene_event_module.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_SceneEvent)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

// sim_msg::VehicleGeometory_VehicleCoordType ReferencePoint2VehicleCoordType(const sim_msg::ReferencePoint ref_pt)
// TX_NOEXCEPT
// {
//  switch (ref_pt)
//  {
//  case sim_msg::ReferencePoint::REFERENCE_POINT_REAR_AXIS:
//    return
// sim_msg::VehicleGeometory_VehicleCoordType::VehicleGeometory_VehicleCoordType_RearAxle_SnappedToGround;
// break;   default:     return
// sim_msg::VehicleGeometory_VehicleCoordType::VehicleGeometory_VehicleCoordType_RearAxle_SnappedToGround;
//  }
// }

Base::txBool txPlanningSceneEventModule::InitializeSceneEvent(Base::ISceneLoader::IViewerPtr _view_ptr,
                                                              Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  ClearPB();
  m_dummy_view_ptr = std::dynamic_pointer_cast<DummyDriverViewer>(_view_ptr);
  m_dummy_loader_ptr = std::dynamic_pointer_cast<PlanningSceneEvent_SceneLoader>(_loader);
  if (NonNull_Pointer(SceneEventLoader()) && NonNull_Pointer(SceneEventViewPtr())) {
    m_use_scene_event_1_0_0_0 = true;
    _elemAccViewPtr = nullptr;
    _elemAccEventViewPtr = nullptr;
    _elemMergeViewPtr = nullptr;
    _elemMergeEventViewPtr = nullptr;
    _elemVelocityViewPtr = nullptr;
    _elemVelocityEventViewPtr = nullptr;

    const auto phyEgoLeader = SceneEventViewPtr()->phyEgoLeader();
    const auto initEgo = SceneEventViewPtr()->initEgo();
    m_pb_vehicle_interaction.mutable_assign()->CopyFrom(initEgo.assign().Raw());
    m_pb_vehicle_interaction.mutable_activate()->CopyFrom(initEgo.activate().Raw());
    m_pb_sensor_group.CopyFrom(SceneEventViewPtr()->sensorGroup().Raw());
    return true;
  } else {
    LogWarn << TX_COND(NonNull_Pointer(SceneEventLoader())) << TX_COND(NonNull_Pointer(SceneEventViewPtr()));
    return false;
  }
}

Base::txBool txPlanningSceneEventModule::HandlerEvent(Base::TimeParamManager const& timeMgr,
                                                      ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(_eventPtr)) {
    const Base::txSysId evId = _eventPtr->id();
    const Base::txString elemType_elemId =
        TAD_SceneEvent_1_0_0_0::Make_ElemType_ElemId(_plus_(Base::Enums::ElementType::TAD_DummyDriver), 0);

    SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewerPtr dummy_scene_event_ptr =
        std::dynamic_pointer_cast<SceneLoader::PlanningSceneEvent_SceneLoader::IDummySceneEventViewer>(_eventPtr);
    if (NonNull_Pointer(dummy_scene_event_ptr)) {
      const auto& ref_action_vec = dummy_scene_event_ptr->actions();
      LogInfo << TX_VARS_NAME(action_size, ref_action_vec.size());
      for (const auto& ref_cur_action : ref_action_vec) {
#if __TX_Mark__("element_state")
        const Base::txInt actionId = ref_cur_action.action_id();
        const Base::txString evId_actionId = TAD_SceneEvent_1_0_0_0::Make_evId_actId(evId, actionId);
        const SceneEventStateInfo sceneEventStateInfo_complete = TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
            elemType_elemId, _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::completeState),
            timeMgr.PassTime());
#endif /*__TX_Mark__("element_state")*/

        using ActionType = SceneLoader::PlanningSceneEvent_SceneLoader::action_ego::ActionType;
        if (ref_cur_action.IsValid(ActionType::activate)) {
          SceneLoader::PlanningSceneEvent_SceneLoader::Activate _a;
          if (ref_cur_action.activate(_a)) {
            m_pb_vehicle_interaction.mutable_activate()->CopyFrom(_a.Raw());
            TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_complete);
          } else {
            m_pb_vehicle_interaction.mutable_activate()->Clear();
            LogWarn << "activate failure, clear";
          }
        } else if (ref_cur_action.IsValid(ActionType::assign)) {
          SceneLoader::PlanningSceneEvent_SceneLoader::Assign _a;
          if (ref_cur_action.assign(_a)) {
            m_pb_vehicle_interaction.mutable_assign()->CopyFrom(_a.Raw());
            TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_complete);
          } else {
            m_pb_vehicle_interaction.mutable_assign()->Clear();
            LogWarn << "assign failure, clear";
          }
        } else if (ref_cur_action.IsValid(ActionType::command)) {
          SceneLoader::PlanningSceneEvent_SceneLoader::Command _c;
          if (ref_cur_action.command(_c)) {
            m_pb_vehicle_interaction.mutable_command()->CopyFrom(_c.m_command);
            if (m_pb_vehicle_interaction.command().has_lane_change()) {
              if (m_pb_vehicle_interaction.command().lane_change().action() == sim_msg::ONOFF_ON) {
                if (m_pb_vehicle_interaction.command().lane_change().value() == sim_msg::Dir::DIR_LEFT) {
                  m_lanechange = 1;
                } else {
                  m_lanechange = -1;
                }
                LogWarn << TX_VARS(m_lanechange);
              }
            }
            TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_complete);
          } else {
            m_pb_vehicle_interaction.mutable_command()->Clear();
            LogWarn << "command failure, clear";
          }
        } else if (ref_cur_action.IsValid(ActionType::override)) {
          SceneLoader::PlanningSceneEvent_SceneLoader::Override _o;
          if (ref_cur_action.override_(_o)) {
            m_pb_vehicle_interaction.mutable_override()->CopyFrom(_o.m_override);
            TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_complete);
          } else {
            m_pb_vehicle_interaction.mutable_override()->Clear();
            LogWarn << "override failure, clear";
          }
        } else if (ref_cur_action.IsValid(ActionType::status)) {
          SceneLoader::PlanningSceneEvent_SceneLoader::Status _s;
          if (ref_cur_action.status(_s)) {
            m_pb_vehicle_interaction.mutable_status()->CopyFrom(_s.m_status);
            TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(evId_actionId, sceneEventStateInfo_complete);
          } else {
            m_pb_vehicle_interaction.mutable_status()->Clear();
            LogWarn << "status failure, clear";
          }
        } else {
          LogWarn << "un known action type.";
        }
      }
      return true;
    } else {
      LogWarn << "[HandlerEvent] dummy_scene_event_ptr is nullptr.";
    }
  } else {
    LogWarn << "[HandlerEvent] _eventPtr is nullptr.";
  }
  return false;
}

TX_NAMESPACE_CLOSE(TrafficFlow)
