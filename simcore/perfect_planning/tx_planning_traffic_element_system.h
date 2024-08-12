// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "scene.pb.h"
#include "traffic.pb.h"
#include "trajectory.pb.h"
#include "tx_centripetal_cat_mull.h"
#include "tx_planning_scene_event_loader.h"
#include "tx_traffic_element_system.h"
#include "tx_vehicle_element.h"
#define __TrafficIdx__ (0)
#define __LifeTimeIdx__ (1)
TX_NAMESPACE_OPEN(TrafficFlow)

class PlanningTrafficElementSystem : public Base::TrafficElementSystem {
  using ParentClass = Base::TrafficElementSystem;
  using control_path_node_vec = CentripetalCatMull::control_path_node_vec;
  using control_path_node = Base::ISceneLoader::IRouteViewer::control_path_node;

 public:
  PlanningTrafficElementSystem() TX_DEFAULT;
  virtual ~PlanningTrafficElementSystem() TX_DEFAULT;
  virtual Base::txBool Initialize(Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool Update(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool Release() TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool IsSupportSceneType(const Base::ISceneLoader::ESceneType _sceneType) const TX_NOEXCEPT
      TX_OVERRIDE;
  virtual Base::IVehicleElementPtr GetEgoVehicleElementPtr() TX_NOEXCEPT { return m_ego_vehicle_ptr; }
  virtual Base::txBool HasEgo() const TX_NOEXCEPT TX_OVERRIDE { return NonNull_Pointer(m_ego_vehicle_ptr); }
  virtual Base::txBool FillingLocationData(Base::TimeParamManager const& timeMgr,
                                           sim_msg::Location& outLocation) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool FillingTrajectoryData(Base::TimeParamManager const& timeMgr,
                                             sim_msg::Trajectory& outTrajectory) TX_NOEXCEPT;
  virtual Base::txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE;
  virtual void ClearLastStepData() TX_NOEXCEPT TX_OVERRIDE {}

 public:
  virtual Base::txBool UpdatePlanningCarData(Base::TimeParamManager const& timeMgr,
                                             const Base::Enums::EgoSubType _egoSubType,
                                             const Base::txString& trafficInfoStr) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool UpdateEnvData(Base::TimeParamManager const& timeMgr,
                                     const Base::txString& trafficInfoStr) TX_NOEXCEPT;
  virtual Base::txBool UpdateGroupPlanningCarData(Base::TimeParamManager const& timeMgr,
                                                  const Base::txString strGroupName,
                                                  const Base::txString& trafficInfoStr) TX_NOEXCEPT;
  virtual Base::txBool RegisterPlanningCar() TX_NOEXCEPT TX_OVERRIDE;
  virtual void FlushSceneEvents(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual void ExecuteEnvPerception(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  virtual void CreateAssemblerCtx() TX_NOEXCEPT TX_OVERRIDE {}
  virtual void CreateElemMgr() TX_NOEXCEPT TX_OVERRIDE {}

 protected:
  virtual Base::txBool Update_Ego_PreSimulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual Base::txBool Update_Ego_Simulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual Base::txBool Update_Ego_PostSimulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual Base::txBool Update_Ego_Signal(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual Base::txBool Update_Ego_LifeCycle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual void UpdateSurroundTraffic(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual control_path_node_vec ParserControlPath(const std::vector<sim_msg::Waypoint>& _ControlPathWayPts) const
      TX_NOEXCEPT;

 protected:
  using DummyDriverViewerPtr = SceneLoader::PlanningSceneEvent_SceneLoader::DummyDriverViewerPtr;
  using DummyDriverViewerPtrVec = SceneLoader::PlanningSceneEvent_SceneLoader::DummyDriverViewerPtrVec;
  virtual SceneLoader::PlanningSceneEvent_SceneLoaderPtr SceneEventLoader() const TX_NOEXCEPT {
    return m_scene_event_loader;
  }

 public:
  enum PlanningMode : Base::txInt { ePlanning = 0, eTrajFollow = 1, eBare = 2 };
  static Base::txString Enum2Str(const PlanningMode _mode) TX_NOEXCEPT;
  virtual PlanningMode ComputePlanningMode(const sim_msg::Ego& cur_ego) const TX_NOEXCEPT;
  Base::txBool isPlanningMode() const TX_NOEXCEPT { return (PlanningMode::ePlanning == m_PlanningMode); }
  Base::txBool isBareMode() const TX_NOEXCEPT { return (PlanningMode::eBare == m_PlanningMode); }
  Base::txBool isTrajFollowMode() const TX_NOEXCEPT { return (PlanningMode::eTrajFollow == m_PlanningMode); }

 protected:
  Base::IVehicleElementPtr m_ego_vehicle_ptr = nullptr;
  std::tuple<sim_msg::Traffic, Base::txFloat /*second*/> m_traffic_mgr;
  SceneLoader::PlanningSceneEvent_SceneLoaderPtr m_scene_event_loader = nullptr;
  PlanningMode m_PlanningMode = PlanningMode::ePlanning;
};

using PlanningTrafficElementSystemPtr = std::shared_ptr<PlanningTrafficElementSystem>;

TX_NAMESPACE_CLOSE(TrafficFlow)
