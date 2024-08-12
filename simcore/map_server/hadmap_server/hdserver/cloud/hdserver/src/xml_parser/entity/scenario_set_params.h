/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

struct sTagSearchScenarioParam {
  sTagSearchScenarioParam() { Reset(); }

  void Reset();

  std::string m_strType;
  std::string m_strKeys;
};

struct sTagGetScenarioInfoListParam {
  sTagGetScenarioInfoListParam() { Reset(); }

  void Reset();

  std::vector<std::string> m_ids;
  std::vector<std::string> m_names;
};

struct sTagGetScenarioListParam {
  sTagGetScenarioListParam() { Reset(); }

  void Reset();

  int64_t m_nOffset;
  int64_t m_nLimit;
};

struct sTagGetScenarioSetListParam {
  sTagGetScenarioSetListParam() { Reset(); }

  void Reset();

  int64_t m_nOffset;
  int64_t m_nLimit;
};

struct sTagCreateScenarioSetParam {
  sTagCreateScenarioSetParam() { Reset(); }

  void Reset();

  int64_t m_nID;
  std::string m_strName;
  std::string m_strLabel;
  std::string m_strScenes;
};

struct sTagUpdateScenarioSetParam {
  sTagUpdateScenarioSetParam() { Reset(); }

  void Reset();

  int64_t m_nID;
  std::string m_strName;
  std::string m_strLabel;
  std::string m_strScenes;
};

struct sTagDeleteScenarioSetParam {
  sTagDeleteScenarioSetParam() { Reset(); }

  void Reset();

  int64_t m_nID;
};
