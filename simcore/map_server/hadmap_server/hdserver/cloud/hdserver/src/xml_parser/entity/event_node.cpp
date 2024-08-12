/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "event_node.h"
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <vector>
#include "common/engine/constant.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/parser_tools.h"

void EventNode::Reset() {
  nTriggerType = TCT_TIME_ABSOLUTE;
  dTriggerValue = 0;
  nDistanceMode = TDM_NONE;
  nCount = -1;
  m_strEventStr = "";
  m_strTimeStr = "";
}

bool EventNode::operator<(const EventNode& other) {
  if (nTriggerType < other.nTriggerType) {
    return true;
  } else if (nTriggerType == other.nTriggerType && dTriggerValue < other.dTriggerValue) {
    return true;
  }

  return false;
}

bool EventNode::operator==(const EventNode& other) {
  if (nTriggerType != other.nTriggerType) {
    return false;
  }

  const double epsil = 0.000000001;
  double delta = dTriggerValue - other.dTriggerValue;
  if (delta > -1 * epsil && delta < epsil) {
    return true;
  }

  return false;
}

bool EventNode::operator!=(const EventNode& other) { return !((*this) == other); }

EventNode& EventNode::operator=(const EventNode& other) {
  if (&other == this) {
    return (*this);
  }

  nTriggerType = other.nTriggerType;
  dTriggerValue = other.dTriggerValue;
  nDistanceMode = other.nDistanceMode;
  nCount = other.nCount;
}
