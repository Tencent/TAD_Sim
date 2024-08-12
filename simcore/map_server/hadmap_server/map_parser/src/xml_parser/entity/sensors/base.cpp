/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "base.h"

CSensorBase::CSensorBase() { Reset(); }

void CSensorBase::Reset() {
  m_strID = "-1";
  // m_strEnabled = "true";
  m_bEnabled = true;
  m_strLocationY = "0";
  m_strLocationZ = "0";
  m_strRotationX = "0";
  m_strRotationY = "0";
  m_strLocationX = "0";
  m_strRotationZ = "0";
  m_strInstallSlot = "";
}
