/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>

#include "catalogbase.h"
#include "common/map_data/map_data_predef.h"
class CObstacle {
 public:
  CObstacle();
  void Reset();
  void ConvertToValue();
  bool GetDefaultObstacle(std::string name, std::string xoscpath = "");
  void ConvertToStr();

 public:
  std::string m_strName;
  std::string m_strID;
  std::string m_strRouteID;
  std::string m_strLaneID;
  std::string m_strStartShift;
  std::string m_strOffset;
  // std::string m_strLength;
  // std::string m_strWidth;
  // std::string m_strHeight;
  std::string m_strType;
  std::string m_strDirection;
  std::string m_strStartAngle;

  uint64_t m_ID;
  uint64_t m_routeID;
  lanepkid m_laneID;

  float m_fStartShift;
  float m_fStartOffset;
  // float m_fObstacleLength;
  // float m_fObstacleWidth;
  // float m_fObstacleHeight;
  CBoundingBox m_boundingBox;
  float m_fDirection;
};
