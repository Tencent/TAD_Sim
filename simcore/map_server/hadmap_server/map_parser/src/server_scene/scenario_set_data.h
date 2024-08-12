/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

class CSimulation;

class CScenarioSetData {
 public:
  static CScenarioSetData& Instance();

  const wchar_t* GetScenarioSetList(const wchar_t* wstrParams);
  const wchar_t* CreateScenarioSet(const wchar_t* strParams);
  const wchar_t* DeleteScenarioSet(const wchar_t* strParams);
  const wchar_t* UpdateScenarioSet(const wchar_t* strParams);

 protected:
  std::wstring m_wstrScenarioSetList;
};
