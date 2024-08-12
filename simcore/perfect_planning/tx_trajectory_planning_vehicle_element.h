// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_planning_vehicle_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)
class PlanningVechicleElement : public SimPlanningVehicleElement {
 public:
  using ParentClass = SimPlanningVehicleElement;

 public:
  virtual Base::txBool MoveStraightOnS(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return true; }
  virtual Base::txBool Update(TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool FillingTrajectory(Base::TimeParamManager const&, sim_msg::Trajectory&) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool UpdateLocation(const Base::TimeParamManager&) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  virtual Base::txBool Need_Longitudinal_Activate() const TX_NOEXCEPT TX_OVERRIDE { return true; }
  virtual Base::txBool Need_Lateral_Activate() const TX_NOEXCEPT TX_OVERRIDE { return false; }
  virtual Base::txBool Need_Autopilot_Activate() const TX_NOEXCEPT { return true; }

 protected:
  TrafficFlow::CentripetalCatMullPtr m_trajectory_ptr = nullptr;
};
using PlanningVechicleElementPtr = std::shared_ptr<PlanningVechicleElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)
