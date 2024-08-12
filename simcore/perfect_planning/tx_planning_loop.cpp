// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_planning_loop.h"
#include "tx_sim_ego_template.h"
TX_NAMESPACE_OPEN(TrafficFlow)

void TAD_PlanningLoop::Init(tx_sim::InitHelper& helper) {
  const std::string param_planning_mode = helper.GetParameter("planning_mode");
  LOG(INFO) << TX_VARS_NAME(planning_mode, param_planning_mode);
  m_SimMode = SimulationMode::eSimEgo;
  if ("sim_ego" == param_planning_mode) {
    m_SimMode = SimulationMode::eSimEgo;
  } else if ("sim_virtual_city_ego" == param_planning_mode) {
    m_SimMode = SimulationMode::eVirtualCityEgo;
  } else if ("sim_adas_ego" == param_planning_mode) {
    m_SimMode = SimulationMode::eADASEgo;
  }

  const std::string param_velocity_mode = helper.GetParameter("velocity_mode");
  if ("const" == param_velocity_mode) {
    FLAGS_planning_const_velocity_mode = true;
  }

  const std::string param_speedlimit_mode = helper.GetParameter("speedlimit_mode");
  if ("false" == param_speedlimit_mode) {
    FLAGS_planning_speedlimit_mode = false;
  }
  LOG(INFO) << TX_VARS(param_velocity_mode) << TX_COND(FLAGS_planning_const_velocity_mode)
            << TX_VARS(param_speedlimit_mode) << TX_COND(FLAGS_planning_speedlimit_mode);

  m_SimPtr = nullptr;
  switch (m_SimMode) {
    case SimulationMode::eSimEgo: {
      m_SimPtr = std::make_shared<SimEgoTemplate>();
      break;
    }
    case SimulationMode::eVirtualCityEgo: {
      m_SimPtr = std::make_shared<SimEgoTemplate>();
      break;
    }
    case SimulationMode::eADASEgo: {
      m_SimPtr = std::make_shared<SimEgoTemplate>();
      break;
    }
  }

  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Init(helper);
  } else {
    LOG(ERROR) << "UnSupport Simulation Mode.";
  }
}

void TAD_PlanningLoop::Reset(tx_sim::ResetHelper& helper) {
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Reset(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

void TAD_PlanningLoop::Step(tx_sim::StepHelper& helper) {
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Step(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

void TAD_PlanningLoop::Stop(tx_sim::StopHelper& helper) {
  if (NonNull_Pointer(m_SimPtr)) {
    m_SimPtr->Stop(helper);
  } else {
    LOG(ERROR) << "Sim Template is null.";
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

TXSIM_MODULE(TrafficFlow::TAD_PlanningLoop)
