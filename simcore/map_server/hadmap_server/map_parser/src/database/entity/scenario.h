/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

struct sTagEntityScenario {
  sTagEntityScenario() { Reset(); }

  void Reset();
  int64_t m_nID = 0;
  std::string m_strName;
  std::string m_strType;
  std::string m_strMap;
  std::string m_strPath;
  std::string m_strInfo;
  std::string m_strLabel;
  std::string m_strSets;
  std::string m_strContent;
  std::string m_strTrafficType;  // xml or simrec
  std::string m_strDataSource;
  std::string ToJson();
};

typedef std::vector<sTagEntityScenario> ScenarioEntities;

class CScenarioEntities {
 public:
  void Clear() { m_entities.clear(); }
  ScenarioEntities& Entities() { return m_entities; }

  std::string ToJson();

 protected:
  ScenarioEntities m_entities;
};
