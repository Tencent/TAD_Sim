/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <list>
#include <string>

struct PedestrianEventNode {
  double dTriggerValue;
  double dDirection;
  double dVelocity;
  int nCount;

  bool operator<(const PedestrianEventNode& other);
  bool operator==(const PedestrianEventNode& other);
  bool operator!=(const PedestrianEventNode& other);
};

typedef std::list<PedestrianEventNode> PedestrianEventNodes;

class CPedestrianEvent {
 public:
  void Reset();
  void ConvertToValue();

  void ConvertToStr();
  CPedestrianEvent& operator=(const CPedestrianEvent& other);

  std::string m_strID;
  std::string m_strTriggerType;
  std::string m_strProfiles;
  uint64_t m_ID;
  int m_TriggerType;
  PedestrianEventNodes m_nodes;
};
