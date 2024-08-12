/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

struct sTagEntityScenarioSet {
  sTagEntityScenarioSet() { Reset(); }

  void Reset();
  int64_t m_nID;
  std::string m_strName;
  std::string m_strLabel;
  std::string m_strScenes;
};

typedef std::vector<sTagEntityScenarioSet> ScenarioSets;

class CScenarioSets {
 public:
  void Clear() { m_sets.clear(); }
  ScenarioSets& Sets() { return m_sets; }

 protected:
  ScenarioSets m_sets;
};
