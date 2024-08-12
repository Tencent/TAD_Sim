/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "sensor_truth.h"

CSensorTruth::CSensorTruth() { Reset(); }

void CSensorTruth::Reset() {
  m_bSaveData = true;
  m_strDevice = ".0";
  m_strVFov = "0";
  m_strHFov = "0";
  m_strDRange = "200";
}
