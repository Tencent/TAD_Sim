// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "control.pb.h"
#include "location.pb.h"
#include "scene.pb.h"
#include "tx_planning_traffic_element_system.h"
#include "tx_simulation_loop.h"
#include "tx_traffic_element_system.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class SimEgoTemplate : public Base::txSimulationTemplate {
  using txString = Base::txString;
  using txFloat = Base::txFloat;
  using TimeParamManager = Base::TimeParamManager;

 public:
  virtual void Init(tx_sim::InitHelper& helper) TX_NOEXCEPT TX_OVERRIDE;
  virtual void Reset(tx_sim::ResetHelper& helper) TX_NOEXCEPT TX_OVERRIDE;
  virtual void Step(tx_sim::StepHelper& helper) TX_NOEXCEPT TX_OVERRIDE;
  virtual void Stop(tx_sim::StopHelper& helper) TX_NOEXCEPT TX_OVERRIDE {}

  virtual void CreateEnv(tx_sim::ResetHelper& helper) TX_NOEXCEPT TX_OVERRIDE;
  virtual void CreateSystem() TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::TrafficElementSystemPtr TrafficSystemPtr() TX_NOEXCEPT TX_OVERRIDE {
    return m_TrafficElementSystem_Ptr;
  }
  virtual TrafficFlow::PlanningTrafficElementSystemPtr PlanningTrafficSystemPtr() TX_NOEXCEPT {
    return m_PlanningTrafficElementSystemPtr;
  }
  virtual void Simulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  virtual void ResetVars() TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::TimeParamManager MakeTimeMgr(const Base::txFloat time_stamp) TX_NOEXCEPT TX_OVERRIDE;
  virtual void ParseInitParameter(tx_sim::InitHelper& helper) TX_NOEXCEPT;
  virtual void ParseResetParameter(tx_sim::ResetHelper& helper) TX_NOEXCEPT;
  virtual void PreSimulation(tx_sim::StepHelper& helper,
                             const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {}
  virtual void PostSimulation(tx_sim::StepHelper& helper,
                              const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {}
  Base::txBool GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams) TX_NOEXCEPT;
  virtual void ReceiveEgoInfo(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual void SimulationTraffic(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  virtual void VerifyRouteEndBehavior(tx_sim::ResetHelper& helper) TX_NOEXCEPT;

 protected:
  txFloat m_lastStepTime = 0.0;
  txFloat m_passTime = 0.0;
  Base::txString m_groupName = "";
  std::vector<tx_sim::Vector3d> m_ego_path;
  sim_msg::Location m_ego_start_location;
  Base::TrafficElementSystemPtr m_TrafficElementSystem_Ptr = nullptr;
  TrafficFlow::PlanningTrafficElementSystemPtr m_PlanningTrafficElementSystemPtr = nullptr;
  txString m_traffic_payload;
  sim_msg::Location m_output_ego_location;
  sim_msg::Trajectory m_output_ego_trajectory;
  sim_msg::Control m_output_ego_control;
  sim_msg::Scene m_input_dummy_driver_scene;
};

using SimEgoTemplatePtr = std::shared_ptr<SimEgoTemplate>;

TX_NAMESPACE_CLOSE(TrafficFlow)
