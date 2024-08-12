/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "simulation.h"

class CSimulationParam : public CSimulation {
 public:
  CSimulationParam();
  explicit CSimulationParam(CSimulation& sim);
  CSimulationParam& operator=(const CSimulation& sim);

  int ParseTemplate(const char* strFile);

  int ParsePlanner(TiXmlElement* elemPlanner, CPlannerParam& planner);

  CPlannerParam m_plannerparam;
};
