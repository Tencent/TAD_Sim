/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/velocity.h"
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <vector>
#include "common/engine/constant.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/velocity_param.h"

VelocityTimeNode& VelocityTimeNode::operator=(const VelocityTimeNode& other) {
  EventNode::operator=(other);
  dVelocity = other.dVelocity;
  return (*this);
}

VelocityTimeNode& VelocityTimeNode::operator=(const VelocityTimeParamNode& other) {
  return VelocityTimeNode::operator=(static_cast<VelocityTimeNode>(other));
}

int VelocityTimeNode::ConvertToTimeValue() {
  std::vector<std::string> strNodes;
  CParseTools::SplitString(m_strTimeStr, strNodes, ",");
  assert(strNodes.size() == 2);
  if (strNodes.size() == 2) {
    nTriggerType = TCT_TIME_ABSOLUTE;
    nDistanceMode = TDM_NONE;
    dTriggerValue = atof(strNodes[0].c_str());
    dVelocity = atof(strNodes[1].c_str());
    nCount = -1;
  } else {
    SYSTEM_LOGGER_ERROR("traffic file velocity profile error!");
    return -1;
  }
  return 0;
}

int VelocityTimeNode::ConvertToEventValue() {
  std::vector<std::string> strNodes;
  CParseTools::SplitString2(m_strEventStr, strNodes, " ,");
  assert(strNodes.size() == 3 || strNodes.size() == 4 || strNodes.size() == 5);
  if (strNodes.size() == 3) {
    std::string strType = strNodes[0];
    if (boost::algorithm::iequals(strType, "ttc")) {
      nTriggerType = TCT_TIME_RELATIVE;
    } else if (boost::algorithm::iequals(strType, "egoDistance")) {
      nTriggerType = TCT_POSITION_RELATIVE;
    } else {
      assert(false);
      SYSTEM_LOGGER_ERROR("velocity trig condition type error!");
      return -1;
    }

    nDistanceMode = TDM_LANE_PROJECTION;
    dTriggerValue = atof(strNodes[1].c_str());
    dVelocity = atof(strNodes[2].c_str());
    nCount = 1;

  } else if (strNodes.size() == 4) {
    std::string strType = strNodes[0];
    if (boost::algorithm::iequals(strType, "ttc")) {
      nTriggerType = TCT_TIME_RELATIVE;
    } else if (boost::algorithm::iequals(strType, "egoDistance")) {
      nTriggerType = TCT_POSITION_RELATIVE;
    } else {
      assert(false);
      SYSTEM_LOGGER_ERROR("velocity trig condition type error!");
      return -1;
    }

    std::string strDistanceMode = strNodes[1];
    if (boost::algorithm::iequals(strDistanceMode, "laneprojection")) {
      nDistanceMode = TDM_LANE_PROJECTION;
    } else if (boost::algorithm::iequals(strDistanceMode, "euclideandistance")) {
      nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
    } else {
      assert(false);
      SYSTEM_LOGGER_ERROR("velocity trig condition distance mode error!");
      return -1;
    }

    dTriggerValue = atof(strNodes[2].c_str());
    dVelocity = atof(strNodes[3].c_str());
    nCount = 1;

  } else if (strNodes.size() == 5) {
    std::string strType = strNodes[0];
    if (boost::algorithm::iequals(strType, "ttc")) {
      nTriggerType = TCT_TIME_RELATIVE;
    } else if (boost::algorithm::iequals(strType, "egoDistance")) {
      nTriggerType = TCT_POSITION_RELATIVE;
    } else {
      assert(false);
      SYSTEM_LOGGER_ERROR("velocity trig condition type error!");
      return -1;
    }

    std::string strDistanceMode = strNodes[1];
    if (boost::algorithm::iequals(strDistanceMode, "laneprojection")) {
      nDistanceMode = TDM_LANE_PROJECTION;
    } else if (boost::algorithm::iequals(strDistanceMode, "euclideandistance")) {
      nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
    } else {
      assert(false);
      SYSTEM_LOGGER_ERROR("velocity trig condition distance mode error!");
      return -1;
    }

    dTriggerValue = atof(strNodes[2].c_str());
    dVelocity = atof(strNodes[3].c_str());

    std::string strCountProfile = strNodes[4];
    std::string strCount = strCountProfile.substr(1, strCountProfile.length() - 2);
    nCount = atoi(strCount.c_str());

  } else {
    SYSTEM_LOGGER_ERROR("traffic file velocity event profile error!");
    return -1;
  }

  return 0;
}

void VelocityTimeNode::ConvertToTimeStr() {
  if (nTriggerType == TCT_TIME_ABSOLUTE) {
    std::string strTrigValue = CUtils::doubleToStringDot3(dTriggerValue);
    std::string strVelocity = CUtils::doubleToStringDot3(dVelocity);
    m_strTimeStr = strTrigValue + "," + strVelocity;
  } else {
    m_strTimeStr = "";
  }
}

void VelocityTimeNode::ConvertToEventStr() {
  if (nTriggerType != TCT_TIME_ABSOLUTE) {
    std::string strTrigType = "ttc";
    if (nTriggerType == TCT_POSITION_RELATIVE) {
      strTrigType = "egoDistance";
    }

    std::string strDistanceMode = "laneprojection";
    if (nDistanceMode == TDM_EUCLIDEAN_DISTANCE) {
      strDistanceMode = "euclideandistance";
    }
    std::string strTrigValue = CUtils::doubleToStringDot3(dTriggerValue);
    std::string strVelocity = CUtils::doubleToStringDot3(dVelocity);
    std::string strCount = std::to_string(nCount);
    m_strEventStr =
        strTrigType + " " + strDistanceMode + " " + strTrigValue + "," + strVelocity + " [" + strCount + "]";

    // m_strEventStr = std::move(strOneNode);
  }
}

PedestrianVelocityNode& PedestrianVelocityNode::operator=(const PedestrianVelocityNode& other) {
  EventNode::operator=(other);
  dVelocity = other.dVelocity;
  dDirection = other.dDirection;
  return (*this);
}

int PedestrianVelocityNode::ConvertToTimeValue() {
  std::vector<std::string> strNodes;
  CParseTools::SplitString(m_strTimeStr, strNodes, ",");
  assert(strNodes.size() == 3);
  if (strNodes.size() == 3) {
    nTriggerType = TCT_TIME_ABSOLUTE;
    nDistanceMode = TDM_NONE;
    dTriggerValue = atof(strNodes[0].c_str());
    dDirection = atof(strNodes[1].c_str());
    dVelocity = atof(strNodes[2].c_str());
    nCount = -1;
  } else {
    SYSTEM_LOGGER_ERROR("traffic file pedestrian velocity profile error!");
    return -1;
  }
  return 0;
}

int PedestrianVelocityNode::ConvertToEventValue() {
  std::vector<std::string> strNodes;
  CParseTools::SplitString2(m_strEventStr, strNodes, " ,");
  assert(strNodes.size() == 4 || strNodes.size() == 5 || strNodes.size() == 6);
  if (strNodes.size() == 4) {
    std::string strType = strNodes[0];
    if (boost::algorithm::iequals(strType, "ttc")) {
      nTriggerType = TCT_TIME_RELATIVE;
    } else if (boost::algorithm::iequals(strType, "egoDistance")) {
      nTriggerType = TCT_POSITION_RELATIVE;
    } else {
      assert(false);
      SYSTEM_LOGGER_ERROR("velocity trig condition type error!");
      return -1;
    }

    nDistanceMode = TDM_LANE_PROJECTION;
    dTriggerValue = atof(strNodes[1].c_str());
    dDirection = atof(strNodes[2].c_str());
    dVelocity = atof(strNodes[3].c_str());
    nCount = 1;

  } else if (strNodes.size() == 5) {
    std::string strType = strNodes[0];
    if (boost::algorithm::iequals(strType, "ttc")) {
      nTriggerType = TCT_TIME_RELATIVE;
    } else if (boost::algorithm::iequals(strType, "egoDistance")) {
      nTriggerType = TCT_POSITION_RELATIVE;
    } else {
      assert(false);
      SYSTEM_LOGGER_ERROR("velocity trig condition type error!");
      return -1;
    }

    std::string strDistanceMode = strNodes[1];
    if (boost::algorithm::iequals(strDistanceMode, "laneprojection")) {
      nDistanceMode = TDM_LANE_PROJECTION;
    } else if (boost::algorithm::iequals(strDistanceMode, "euclideandistance")) {
      nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
    } else {
      assert(false);
      SYSTEM_LOGGER_ERROR("velocity trig condition distance mode error!");
      return -1;
    }

    dTriggerValue = atof(strNodes[2].c_str());
    dDirection = atof(strNodes[3].c_str());
    dVelocity = atof(strNodes[4].c_str());
    nCount = 1;

  } else if (strNodes.size() == 6) {
    std::string strType = strNodes[0];
    if (boost::algorithm::iequals(strType, "ttc")) {
      nTriggerType = TCT_TIME_RELATIVE;
    } else if (boost::algorithm::iequals(strType, "egoDistance")) {
      nTriggerType = TCT_POSITION_RELATIVE;
    } else {
      assert(false);
      SYSTEM_LOGGER_ERROR("velocity trig condition type error!");
      return -1;
    }

    std::string strDistanceMode = strNodes[1];
    if (boost::algorithm::iequals(strDistanceMode, "laneprojection")) {
      nDistanceMode = TDM_LANE_PROJECTION;
    } else if (boost::algorithm::iequals(strDistanceMode, "euclideandistance")) {
      nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
    } else {
      assert(false);
      SYSTEM_LOGGER_ERROR("velocity trig condition distance mode error!");
      return -1;
    }

    dTriggerValue = atof(strNodes[2].c_str());
    dDirection = atof(strNodes[3].c_str());
    dVelocity = atof(strNodes[4].c_str());

    std::string strCountProfile = strNodes[5];
    std::string strCount = strCountProfile.substr(1, strCountProfile.length() - 2);
    nCount = atoi(strCount.c_str());

  } else {
    SYSTEM_LOGGER_ERROR("traffic file pedestrian velocity event profile error!");
    return -1;
  }

  return 0;
}

void PedestrianVelocityNode::ConvertToTimeStr() {
  if (nTriggerType == TCT_TIME_ABSOLUTE) {
    std::string strTrigValue = CUtils::doubleToStringDot3(dTriggerValue);
    int nDirection = dDirection;
    std::string strDirection = std::to_string(nDirection);
    std::string strVelocity = CUtils::doubleToStringDot3(dVelocity);
    m_strTimeStr = strTrigValue + "," + strDirection + "," + strVelocity;
  } else {
    m_strTimeStr = "";
  }
}

void PedestrianVelocityNode::ConvertToEventStr() {
  if (nTriggerType != TCT_TIME_ABSOLUTE) {
    std::string strTrigType = "ttc";
    if (nTriggerType == TCT_POSITION_RELATIVE) {
      strTrigType = "egoDistance";
    }

    std::string strDistanceMode = "laneprojection";
    if (nDistanceMode == TDM_EUCLIDEAN_DISTANCE) {
      strDistanceMode = "euclideandistance";
    }
    std::string strTrigValue = CUtils::doubleToStringDot3(dTriggerValue);
    int nDirection = dDirection;
    std::string strDirection = std::to_string(nDirection);
    std::string strVelocity = CUtils::doubleToStringDot3(dVelocity);
    std::string strCount = std::to_string(nCount);
    m_strEventStr = strTrigType + " " + strDistanceMode + " " + strTrigValue + "," + strDirection + "," + strVelocity +
                    " [" + strCount + "]";

    // m_strEventStr = std::move(strOneNode);
  }
}
