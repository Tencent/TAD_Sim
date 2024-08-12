// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "control.pb.h"
#include "control_v2.pb.h"
#include "scene_sensor_group.pb.h"
#include "tad_scene_event_1_0_0_0.h"
#include "tx_planning_scene_event_loader.h"
#include "vehicle_interaction.pb.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class txPlanningSceneEventModule : public TAD_SceneEvent_1_0_0_0 {
 public:
  using PlanningSceneEvent_SceneLoader = SceneLoader::PlanningSceneEvent_SceneLoader;
  using PlanningSceneEvent_SceneLoaderPtr = SceneLoader::PlanningSceneEvent_SceneLoaderPtr;
  using DummyDriverViewer = SceneLoader::PlanningSceneEvent_SceneLoader::DummyDriverViewer;
  using DummyDriverViewerPtr = SceneLoader::PlanningSceneEvent_SceneLoader::DummyDriverViewerPtr;

 public:
  virtual Base::txBool InitializeSceneEvent(Base::ISceneLoader::IViewerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;
  virtual Base::txBool HandlerEvent(Base::TimeParamManager const& timeMgr,
                                    ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT TX_OVERRIDE;
  virtual void ClearPB() TX_NOEXCEPT {
    m_pb_vehicle_interaction.Clear();
    m_pb_sensor_group.Clear();
  }
  virtual sim_msg::VehicleInteraction vehicle_interaction_pb(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT {
    return m_pb_vehicle_interaction;
  }
  virtual sim_msg::SensorGroup sensor_group_pb(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT {
    return m_pb_sensor_group;
  }
  virtual PlanningSceneEvent_SceneLoaderPtr SceneEventLoader() TX_NOEXCEPT { return m_dummy_loader_ptr; }
  virtual DummyDriverViewerPtr SceneEventViewPtr() TX_NOEXCEPT { return m_dummy_view_ptr; }

 protected:
  PlanningSceneEvent_SceneLoaderPtr m_dummy_loader_ptr = nullptr;
  DummyDriverViewerPtr m_dummy_view_ptr = nullptr;
  sim_msg::VehicleInteraction m_pb_vehicle_interaction;
  sim_msg::SensorGroup m_pb_sensor_group;
};
using txPlanningSceneEventModulePtr = std::shared_ptr<txPlanningSceneEventModule>;

TX_NAMESPACE_CLOSE(TrafficFlow)
