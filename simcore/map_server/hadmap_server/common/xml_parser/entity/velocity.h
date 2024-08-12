/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <list>
#include <string>
#include "common/xml_parser/entity/event_node.h"

struct VelocityTimeParamNode;
class CVelocityParam;

struct VelocityTimeNode : public EventNode {
  double dVelocity;
  VelocityTimeNode& operator=(const VelocityTimeNode& other);
  VelocityTimeNode& operator=(const VelocityTimeParamNode& other);

  virtual int ConvertToTimeValue();
  virtual int ConvertToEventValue();

  virtual void ConvertToTimeStr();
  virtual void ConvertToEventStr();
};

typedef std::list<VelocityTimeNode> VelocityTimeNodes;

template <>
inline CTrafficEvent<VelocityTimeNode>& CTrafficEvent<VelocityTimeNode>::operator=(
    const CTrafficEvent<VelocityTimeNode>& other) {
  m_ID = other.m_ID;
  m_strID = other.m_strID;

  m_strProfiles = other.m_strProfiles;
  m_strProfilesEvent = other.m_strProfilesEvent;
  m_strProfilesTime = other.m_strProfilesTime;
  m_nodes = other.m_nodes;

  return (*this);
}

typedef CTrafficEvent<VelocityTimeNode> CVelocity;

struct PedestrianVelocityNode : public EventNode {
  double dDirection;
  double dVelocity;
  PedestrianVelocityNode& operator=(const PedestrianVelocityNode& other);

  virtual int ConvertToTimeValue();
  virtual int ConvertToEventValue();

  virtual void ConvertToTimeStr();
  virtual void ConvertToEventStr();
};

typedef std::list<PedestrianVelocityNode> PedestrianVelocityNodes;

typedef CTrafficEvent<PedestrianVelocityNode> CPedestrianVelocity;
