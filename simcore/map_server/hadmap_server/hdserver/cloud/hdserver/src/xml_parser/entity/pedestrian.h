/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>
#include "./pedestrian_event.h"
#include "common/map_data/map_data_predef.h"

struct tagDirection {
  tagDirection();
  void Reset();
  void ConvertToValue();
  void ConvertToStr();

  std::string m_strDir;
  std::string m_strDirDuration;
  std::string m_strDirVelocity;

  float m_fDir;
  float m_fDirDuration;
  float m_fDirVelocity;
};

class CPedestrian {
 public:
  CPedestrian();

  void Reset();
  virtual void ConvertToValue();
  virtual void ConvertToStr();

 public:
  std::string m_strName;

  std::string m_strID;
  std::string m_strRouteID;
  std::string m_strLaneID;
  std::string m_strStartShift;
  std::string m_strStartTime;
  std::string m_strEndTime;
  std::string m_strOffset;
  std::string m_strType;

  uint64_t m_ID;
  uint64_t m_routeID;
  lanepkid m_laneID;
  float m_fStartShift;
  float m_fStartTime;
  float m_fEndTime;
  float m_fStartOffset;

  CPedestrianEvent m_events;
  std::vector<tagDirection> m_directions;
};
