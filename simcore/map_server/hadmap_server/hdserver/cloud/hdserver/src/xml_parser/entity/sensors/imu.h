/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "base.h"

class CSensorIMU : public CSensorBase {
 public:
  CSensorIMU();
  void Reset();

  std::string m_strQuaternion;
};
