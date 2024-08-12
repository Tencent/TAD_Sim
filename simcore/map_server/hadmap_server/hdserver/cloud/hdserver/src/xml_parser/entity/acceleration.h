/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <list>
#include <string>

struct TimeParamNode;
class CAccelerationParam;

struct TimeNode {
  int nTriggerType;
  double dTriggerValue;
  int nDistanceMode;
  double dAcc;
  int nCount;
  int nStopType;
  double dStopValue;

  std::string strTriggerValue;

  TimeNode& operator=(const TimeParamNode& other);
  bool operator<(const TimeNode& other);
  bool operator==(const TimeNode& other);
  bool operator!=(const TimeNode& other);
};

typedef std::list<TimeNode> TimeNodes;

class CAcceleration {
 public:
  void Reset();
  void ClearNodes() { m_nodes.clear(); }
  void SetToDefault();
  void ConvertToValue();
  void ConvertToTimeValue();
  void ConvertToEventValue();

  void ConvertToStr();
  void ConvertToTimeStr();
  void ConvertToEventStr();
  CAcceleration& operator=(const CAcceleration& other);
  CAcceleration& operator=(const CAccelerationParam& other);

  std::string m_strID;
  std::string m_strProfiles;
  std::string m_strProfilesTime;
  std::string m_strProfilesEvent;
  std::string m_strEndConditionTime;
  std::string m_strEndConditionEvent;
  uint64_t m_ID;
  TimeNodes m_nodes;
};
