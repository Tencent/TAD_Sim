/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "common/xml_parser/xosc_replay/open_drive_tpl.h"
#include "common/xml_parser/xosc_replay/open_scenario_tpl.h"

TX_NAMESPACE_OPEN(SIM)

TX_NAMESPACE_OPEN(OSC)

using ScenarioObjectPtr = xmlScenarioObject*;

using ManeuverGroupsPtr = xmlManeuverGroups*;

using ManeuverGroupPtr = xmlManeuverGroup*;

using TrajectoryPtr = xmlTrajectory*;

using SpeedActionPtr = xmlSpeedAction*;

using SimulationTimeConditionPtr = xmlSimulationTimeCondition*;

using TeleportActionPtr = xmlTeleportAction*;

// helper function
extern ManeuverGroupsPtr GetManeuverGroupsPtr(const OpenScenarioPtr& osc, size_t act_idx = 0);

extern TrajectoryPtr GetTrajectoryPtr(ManeuverGroupPtr mg);

extern ScenarioObjectPtr GetScenarioObjectPtrByName(const OpenScenarioPtr& osc, const std::string& name);

extern ManeuverGroupPtr GetManeuverGroupPtrByName(const OpenScenarioPtr& osc, const std::string& name);

extern TrajectoryPtr GetTrajectoryPtrByName(const OpenScenarioPtr& osc, const std::string& name);

extern SpeedActionPtr GetSpeedActionPtrByName(const OpenScenarioPtr& osc, const std::string& name);

extern SimulationTimeConditionPtr GetSimulationTimeConditionPtrByName(const OpenScenarioPtr& osc,
                                                                      const std::string& name);

extern TeleportActionPtr GetTeleportActionPtrByName(const OpenScenarioPtr& osc, const std::string& name);

TX_NAMESPACE_CLOSE(OSC)

TX_NAMESPACE_CLOSE(SIM)
