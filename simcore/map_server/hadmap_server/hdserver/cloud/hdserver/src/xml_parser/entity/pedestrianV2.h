/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>
#include "./pedestrian.h"
#include "./velocity.h"
#include "common/map_data/map_data_predef.h"

class CPedestrianV2 : public CPedestrian {
 public:
  CPedestrianV2();

  void Reset();
  virtual void ConvertToValue();
  virtual void ConvertToStr();

 public:
  std::string m_strStartVelocity;
  std::string m_strMaxVelocity;
  std::string m_strBehavior;

  int m_nBehaviorType;
  float m_fStartVelocity;
  float m_fMaxVelocity;
};
