/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/xosc_replay/xosc_replay.h"

#include <algorithm>

TX_NAMESPACE_OPEN(SIM)
TX_NAMESPACE_OPEN(OSC)

ManeuverGroupsPtr GetManeuverGroupsPtr(const OpenScenarioPtr& osc, size_t act_idx) {
  if (osc->Storyboard.Story.Act.empty() && act_idx == 0) {
    osc->Storyboard.Story.Act.insert(osc->Storyboard.Story.Act.end(), xmlAct());
  }
  if (act_idx >= osc->Storyboard.Story.Act.size()) {
    return nullptr;
  }
  return &(osc->Storyboard.Story.Act[act_idx].ManeuverGroup);
}

TrajectoryPtr GetTrajectoryPtr(ManeuverGroupPtr mg) {
  if (!mg || mg->Maneuver.Event.empty() || mg->Maneuver.Event[0].Action.empty()) {
    return nullptr;
  }

  for (auto& event : mg->Maneuver.Event) {
    for (auto& action : event.Action) {
      if (!action.PrivateAction.RoutingAction.FollowTrajectoryAction.Trajectory.Shape.Polyline.Vertexs.empty()) {
        return &(action.PrivateAction.RoutingAction.FollowTrajectoryAction.Trajectory);
      }
    }
  }
  auto& trajectory = mg->Maneuver.Event[0].Action[0].PrivateAction.RoutingAction.FollowTrajectoryAction.Trajectory;
  return &trajectory;
}

ScenarioObjectPtr GetScenarioObjectPtrByName(const OpenScenarioPtr& osc, const std::string& name) {
  auto iter = std::find_if(osc->Entities.begin(), osc->Entities.end(), [&](auto&& e) { return e.name == name; });
  return iter == osc->Entities.end() ? nullptr : &(*iter);
}

ManeuverGroupPtr GetManeuverGroupPtrByName(const OpenScenarioPtr& osc, const std::string& name) {
  for (size_t i = 0; i < osc->Storyboard.Story.Act.size(); ++i) {
    auto mgs = GetManeuverGroupsPtr(osc, i);
    if (!mgs) {
      continue;
    }
    auto iter = std::find_if(mgs->begin(), mgs->end(), [&](auto&& e) { return e.Actors.EntityRef.entityRef == name; });
    if (iter != mgs->end()) {
      return &(*iter);
    }
  }
  return nullptr;
}

TrajectoryPtr GetTrajectoryPtrByName(const OpenScenarioPtr& osc, const std::string& name) {
  auto mg = GetManeuverGroupPtrByName(osc, name);
  if (!mg) {
    return nullptr;
  }
  return GetTrajectoryPtr(mg);
}

SpeedActionPtr GetSpeedActionPtrByName(const OpenScenarioPtr& osc, const std::string& name) {
  auto mg = GetManeuverGroupPtrByName(osc, name);
  if (!mg || mg->Maneuver.Event.empty() || mg->Maneuver.Event[0].Action.empty()) {
    return nullptr;
  }

  for (auto& event : mg->Maneuver.Event) {
    for (auto& action : event.Action) {
      if (!action.PrivateAction.LongitudinalAction.SpeedAction.SpeedActionDynamics.dynamicsDimension.empty()) {
        return &(action.PrivateAction.LongitudinalAction.SpeedAction);
      }
    }
  }
  return nullptr;
}

SimulationTimeConditionPtr GetSimulationTimeConditionPtrByName(const OpenScenarioPtr& osc, const std::string& name) {
  auto mg = GetManeuverGroupPtrByName(osc, name);
  if (!mg) {
    return nullptr;
  }

  thread_local std::map<std::string, int> valid_rules = {{"greaterThan", 1}, {"lessThan", -1}, {"equalTo", 1}};
  for (auto& event : mg->Maneuver.Event) {
    bool is_trajectory_event = false;
    for (auto& action : event.Action) {
      if (!action.PrivateAction.RoutingAction.FollowTrajectoryAction.Trajectory.Shape.Polyline.Vertexs.empty()) {
        is_trajectory_event = true;
        break;
      }
    }
    if (!is_trajectory_event) {
      continue;
    }
    for (auto& cg : event.StartTrigger.ConditionGroup) {
      for (auto& c : cg.Condition) {
        std::string rule = c.ByValueCondition.SimulationTimeCondition.rule;
        if (valid_rules.count(rule) <= 0) {
          continue;
        }
        c.ByValueCondition.SimulationTimeCondition.sign = valid_rules.at(rule);
        return &c.ByValueCondition.SimulationTimeCondition;
      }
    }
  }
  return nullptr;
}

TeleportActionPtr GetTeleportActionPtrByName(const OpenScenarioPtr& osc, const std::string& name) {
  for (auto& iter : osc->Storyboard.Init.Actions.Private) {
    if (iter.entityRef == name) {
      return &iter.PrivateAction.TeleportAction;
    }
  }
  return nullptr;
}

TX_NAMESPACE_CLOSE(OSC)
TX_NAMESPACE_CLOSE(SIM)
