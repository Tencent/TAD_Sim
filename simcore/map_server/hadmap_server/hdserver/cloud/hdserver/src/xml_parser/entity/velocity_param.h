/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "velocity.h"

struct VelocityTimeParamNode : public VelocityTimeNode {
  double dTriggerValueStart;
  double dTriggerValueEnd;
  double dTriggerValueSep;
  double dVelocityStart;
  double dVelocityEnd;
  double dVelocitySep;
};

typedef std::list<VelocityTimeParamNode> VelocityTimeParamNodes;

class CVelocityParam : public CVelocity {
 public:
  typedef std::list<VelocityTimeNodes> VelocityDatus;
  typedef std::list<VelocityTimeNode> ParamizedVelocityTimeNodes;
  typedef std::list<ParamizedVelocityTimeNodes> SequencedParamizedVelocityTimeNodes;
  typedef std::list<CVelocity> Velocitys;

  void Reset();
  int Generate(Velocitys& velocities);

  uint64_t ListVariations(SequencedParamizedVelocityTimeNodes& sequencedContainer, bool onlyCalculate);

  Velocitys& GeneratedVelocitys() { return m_generatedVelocities; }

  int Compose(SequencedParamizedVelocityTimeNodes& containers, VelocityDatus& inDatus, VelocityDatus& outDatus);

  VelocityTimeParamNodes m_paramNodes;
  Velocitys m_generatedVelocities;
};
