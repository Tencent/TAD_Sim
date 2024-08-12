/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "scenario_set_params.h"
#include "common/engine/constant.h"

void sTagSearchScenarioParam::Reset() {
  m_strType = "";
  m_strKeys = "";
}

void sTagGetScenarioInfoListParam::Reset() {
  m_ids.clear();
  m_names.clear();
}

void sTagGetScenarioListParam::Reset() {
  m_nOffset = -1;
  m_nLimit = -1;
}

void sTagGetScenarioSetListParam::Reset() {
  m_nOffset = -1;
  m_nLimit = -1;
}

void sTagCreateScenarioSetParam::Reset() {
  m_nID = 0;
  m_strName = "";
  m_strLabel = "";
  m_strScenes = "";
}

void sTagUpdateScenarioSetParam::Reset() {
  m_nID = 0;
  m_strName = "";
  m_strLabel = "";
  m_strScenes = "";
}

void sTagDeleteScenarioSetParam::Reset() { m_nID = 0; }
