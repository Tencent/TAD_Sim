/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "gps.h"

CSensorGPS::CSensorGPS() { Reset(); }

void CSensorGPS::Reset() {
  CSensorBase::Reset();

  m_strQuaternion = "0,0,0,0";
}
