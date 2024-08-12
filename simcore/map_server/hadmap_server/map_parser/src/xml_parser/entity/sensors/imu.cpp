/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "imu.h"

CSensorIMU::CSensorIMU() { Reset(); }

void CSensorIMU::Reset() {
  CSensorBase::Reset();

  m_strQuaternion = "0,0,0,0";
}
