/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

#include "common/map_data/map_data_predef.h"
#include "common/xml_parser/entity/pedestrian.h"
#include "common/xml_parser/entity/velocity.h"

class CPedestrianV2 : public CPedestrian {
 public:
  CPedestrianV2();

  bool isDynamicObsctale();

  void Reset();
  virtual void ConvertToValue();
  virtual void ConvertToStr();

 public:
  std::string m_strStartVelocity;
  std::string m_strMaxVelocity;
  std::string m_strBehavior;
  std::string m_strAngle;
  std::string m_strStartAngle;
  std::string m_controlPath;

  int m_nBehaviorType;
  float m_fStartVelocity;
  float m_fMaxVelocity;
};
