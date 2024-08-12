/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <vector>
#include "acceleration.h"

struct TimeParamNode : public TimeNode {
  double dTriggerValueStart;
  double dTriggerValueEnd;
  double dTriggerValueSep;

  double dAccStart;
  double dAccEnd;
  double dAccSep;
};

typedef std::list<TimeParamNode> TimeParamNodes;

class CAccelerationParam : public CAcceleration {
 public:
  typedef std::list<TimeNodes> AccelerationDatus;
  typedef std::list<TimeNode> ParamizedTimeNodes;
  typedef std::list<ParamizedTimeNodes> SequencedParamizedTimeNodes;
  typedef std::list<CAcceleration> Accelerations;

  void Reset();

  uint64_t ListVariations(SequencedParamizedTimeNodes& sequencedContainer, bool onlyCalculate);

  int Generate(Accelerations& accs);
  Accelerations& GeneratedAccelerations() { return m_generatedAccs; }

  int Compose(SequencedParamizedTimeNodes& containers, AccelerationDatus& inDatus, AccelerationDatus& outDatus);

  TimeParamNodes m_paramNodes;
  Accelerations m_generatedAccs;
};
