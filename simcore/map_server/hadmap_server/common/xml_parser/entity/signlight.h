/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

class CSignLight {
 public:
  CSignLight();
  void Reset();
  void ConvertToValue();
  void ConvertToStr();
  // 由于traffic还没支持双向道路解析，暂时由场景编辑器进行车道编号的转换
  void ConvertLaneId();

  std::string m_strID;
  std::string m_strRouteID;
  std::string m_strStartShift;
  std::string m_strStartTime;
  std::string m_strOffset;
  std::string m_strTimeGreen;
  std::string m_strTimeYellow;
  std::string m_strTimeRed;
  std::string m_strDirection;
  std::string m_strCompliance;
  std::string m_strPhase;
  std::string m_strStatus;
  std::string m_strLane;
  // add
  std::string m_strEventId;
  std::string m_strPlan;
  std::string m_strJunction;
  std::string m_strPhasenumber;
  std::vector<std::string> m_strSignalheadVec;
  // Route
  double m_frouteLon;
  double m_frouteLat;
  double m_frouteAlt;
  // double m_currentLaneWidth = 0;
  // parse lanelink
  std::map<std::string, std::vector<int>> m_parsetolanes;
  std::map<std::string, std::vector<int>> m_parsetolanelinks;
  uint64_t m_ID;
  uint64_t m_routeID;
  float m_fStartShift;
  float m_fStartTime;
  float m_fStartOffset;
  float m_fTimeGreen;
  float m_fTimeYellow;
  float m_fTimeRed;
  float m_fDirection;
  float m_fCompliance;
  //
  //
};

using LogicLightVec = std::vector<std::shared_ptr<CSignLight>>;
