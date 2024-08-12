/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "acceleration.h"
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <vector>
#include "acceleration_param.h"
#include "common/engine/constant.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/parser_tools.h"

TimeNode& TimeNode::operator=(const TimeParamNode& other) {
  nTriggerType = other.nTriggerType;
  dTriggerValue = other.dTriggerValue;
  nDistanceMode = other.nDistanceMode;
  dAcc = other.dAcc;
  nCount = other.nCount;
  nStopType = other.nStopType;
  dStopValue = other.dStopValue;
  return (*this);
}

bool TimeNode::operator<(const TimeNode& other) {
  if (nTriggerType < other.nTriggerType) {
    return true;
  } else if (nTriggerType == other.nTriggerType && dTriggerValue < other.dTriggerValue) {
    return true;
  }

  return false;
}

bool TimeNode::operator==(const TimeNode& other) {
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

bool TimeNode::operator!=(const TimeNode& other) { return !((*this) == other); }

void CAcceleration::Reset() {
  m_strID = "";
  m_strProfiles = "";
  m_strProfilesTime = "";
  m_strProfilesEvent = "";
  m_strEndConditionTime = "";
  m_strEndConditionEvent = "";

  m_ID = 0;
  m_nodes.clear();
}

void CAcceleration::SetToDefault() {
  m_strProfiles = "";
  m_strProfilesTime = "0,0";
  m_strProfilesEvent = "ttc 0,0";
  m_strEndConditionTime = "None,0";
  m_strEndConditionEvent = "None,0";
  m_nodes.clear();

  ConvertToTimeValue();
  ConvertToEventValue();
}

CAcceleration& CAcceleration::operator=(const CAcceleration& other) {
  m_ID = other.m_ID;
  m_strProfiles = other.m_strProfiles;
  m_strProfilesEvent = other.m_strProfilesEvent;
  m_strProfilesTime = other.m_strProfilesTime;
  m_strEndConditionEvent = other.m_strEndConditionEvent;
  m_strEndConditionTime = other.m_strEndConditionTime;
  m_nodes = other.m_nodes;
  m_strID = other.m_strID;

  return (*this);
}

CAcceleration& CAcceleration::operator=(const CAccelerationParam& other) {
  m_ID = other.m_ID;
  m_strID = other.m_strID;

  return (*this);
}

void CAcceleration::ConvertToTimeStr() {
  m_strID = std::to_string(m_ID);
  TimeNodes::iterator itr = m_nodes.begin();
  int nSize = m_nodes.size();
  int i = 0;
  int nCount = 0;
  for (; itr != m_nodes.end(); ++itr) {
    if (itr->nTriggerType == TCT_TIME_ABSOLUTE) {
      std::string strTrigValue = std::to_string((*itr).dTriggerValue);
      std::string strAcc = std::to_string((*itr).dAcc);
      std::string strOneNode = strTrigValue + "," + strAcc;
      if (nCount > 0) {
        m_strProfilesTime.append(";");
      }

      m_strProfilesTime.append(strOneNode);

      std::string strEndType = AccEndConditionTypeValue2Str((*itr).nStopType);
      std::string strEndValue = std::to_string((*itr).dStopValue);
      std::string strOneCondition = strEndType + "," + strEndValue;
      if (nCount > 0) {
        m_strEndConditionTime.append(";");
      }

      m_strEndConditionTime.append(strOneCondition);

      nCount++;
    }
    i++;
  }
}

void CAcceleration::ConvertToEventStr() {
  m_strID = std::to_string(m_ID);
  TimeNodes::iterator itr = m_nodes.begin();
  int nSize = m_nodes.size();
  int i = 0;
  int nCount = 0;
  for (; itr != m_nodes.end(); ++itr) {
    if (itr->nTriggerType != TCT_TIME_ABSOLUTE) {
      std::string strTrigType = "ttc";
      if (itr->nTriggerType == TCT_POSITION_RELATIVE) {
        strTrigType = "egoDistance";
      }
      std::string strDistanceMode = "laneprojection";
      if (itr->nDistanceMode == TDM_EUCLIDEAN_DISTANCE) {
        strDistanceMode = "euclideandistance";
      }

      std::string strTrigValue = std::to_string((*itr).dTriggerValue);
      std::string strAcc = std::to_string((*itr).dAcc);
      std::string strCount = std::to_string((*itr).nCount);
      std::string strOneNode =
          strTrigType + " " + strDistanceMode + " " + strTrigValue + "," + strAcc + " [" + strCount + "]";

      if (nCount > 0) {
        m_strProfilesEvent.append(";");
      }

      m_strProfilesEvent.append(strOneNode);

      std::string strEndType = AccEndConditionTypeValue2Str((*itr).nStopType);
      std::string strEndValue = std::to_string((*itr).dStopValue);
      std::string strOneCondition = strEndType + "," + strEndValue;
      if (nCount > 0) {
        m_strEndConditionEvent.append(";");
      }

      m_strEndConditionEvent.append(strOneCondition);

      nCount++;
    }
    i++;
  }
}

void CAcceleration::ConvertToStr() {
  m_strID = std::to_string(m_ID);
  TimeNodes::iterator itr = m_nodes.begin();
  int nSize = m_nodes.size();
  int i = 0;
  for (; itr != m_nodes.end(); ++itr) {
    std::string strTrigType = std::to_string((*itr).nTriggerType);
    std::string strTrigValue = std::to_string((*itr).dTriggerValue);
    std::string strAcc = std::to_string((*itr).dAcc);
    std::string strOneNode = strTrigType + "," + strTrigValue + "," + strAcc;
    if (i + 1 != nSize) {
      strOneNode.append(";");
    }

    m_strProfiles.append(strOneNode);
    i++;
  }
}

void CAcceleration::ConvertToTimeValue() {
  m_ID = atoi(m_strID.c_str());

  std::vector<std::string> strs;
  CParseTools::SplitString(m_strProfilesTime, strs, ";");
  std::vector<std::string> strEndConditions;
  CParseTools::SplitString(m_strEndConditionTime, strEndConditions, ";");

  if (strEndConditions.size() != 0 && strs.size() != strEndConditions.size()) {
    SYSTEM_LOGGER_ERROR("time profile and time end condtion size not equal!");
    return;
  }

  for (int i = 0; i < strs.size(); ++i) {
    std::string strOneCondition = "";
    if (strEndConditions.size() > 0) {
      strOneCondition = strEndConditions[i];
    }

    std::string strAcc = strs[i];
    std::vector<std::string> strNodes;
    CParseTools::SplitString(strAcc, strNodes, ",");
    std::vector<std::string> strOneConditionNodes;
    CParseTools::SplitString(strOneCondition, strOneConditionNodes, ",");

    assert(strNodes.size() == 2);
    // assert(strOneConditionNodes.size() == 2);
    if (strNodes.size() == 2) {
      TimeNode tn;
      tn.nTriggerType = TCT_TIME_ABSOLUTE;
      tn.nDistanceMode = TDM_NONE;
      tn.dTriggerValue = atof(strNodes[0].c_str());
      tn.dAcc = atof(strNodes[1].c_str());
      tn.nCount = -1;

      tn.nStopType = AECT_NONE;
      tn.dStopValue = 0;
      if (strOneConditionNodes.size() == 2) {
        tn.nStopType = AccEndConditionTypeStr2Value(strOneConditionNodes[0]);
        tn.dStopValue = atof(strOneConditionNodes[1].c_str());
      }

      m_nodes.push_back(tn);
    } else {
      SYSTEM_LOGGER_ERROR("traffic file acc profile error!");
      continue;
    }
  }
  m_nodes.sort();
}

void CAcceleration::ConvertToEventValue() {
  m_ID = atoi(m_strID.c_str());

  std::vector<std::string> strs;
  CParseTools::SplitString(m_strProfilesEvent, strs, ";");

  std::vector<std::string> strEndConditions;
  CParseTools::SplitString(m_strEndConditionEvent, strEndConditions, ";");

  if (strEndConditions.size() != 0 && strs.size() != strEndConditions.size()) {
    SYSTEM_LOGGER_ERROR("event profile and event acc end condtion size not equal!");
    return;
  }

  for (int i = 0; i < strs.size(); ++i) {
    std::string strOneCondition = "";
    if (strEndConditions.size() > 0) {
      strOneCondition = strEndConditions[i];
    }
    std::vector<std::string> strOneConditionNodes;
    CParseTools::SplitString(strOneCondition, strOneConditionNodes, ",");

    std::string strAcc = strs[i];
    std::vector<std::string> strNodes;
    CParseTools::SplitString2(strAcc, strNodes, " ,");

    assert(strNodes.size() == 3 || strNodes.size() == 4 || strNodes.size() == 5);
    if (strNodes.size() == 3) {
      TimeNode tn;
      std::string strType = strNodes[0];
      if (boost::algorithm::iequals(strType, "ttc")) {
        tn.nTriggerType = TCT_TIME_RELATIVE;
      } else if (boost::algorithm::iequals(strType, "egoDistance")) {
        tn.nTriggerType = TCT_POSITION_RELATIVE;
      } else {
        assert(false);
        SYSTEM_LOGGER_ERROR("acc trig condition type error!");
        continue;
      }
      tn.nDistanceMode = TDM_LANE_PROJECTION;

      tn.dTriggerValue = atof(strNodes[1].c_str());
      tn.dAcc = atof(strNodes[2].c_str());
      tn.nCount = 1;

      tn.nStopType = AECT_NONE;
      tn.dStopValue = 0;
      if (strOneConditionNodes.size() == 2) {
        tn.nStopType = AccEndConditionTypeStr2Value(strOneConditionNodes[0]);
        tn.dStopValue = atof(strOneConditionNodes[1].c_str());
      }

      m_nodes.push_back(tn);
    } else if (strNodes.size() == 4) {
      TimeNode tn;
      std::string strType = strNodes[0];
      if (boost::algorithm::iequals(strType, "ttc")) {
        tn.nTriggerType = TCT_TIME_RELATIVE;
      } else if (boost::algorithm::iequals(strType, "egoDistance")) {
        tn.nTriggerType = TCT_POSITION_RELATIVE;
      } else {
        assert(false);
        SYSTEM_LOGGER_ERROR("acc trig condition type error!");
        continue;
      }

      std::string strDistanceMode = strNodes[1];
      if (boost::algorithm::iequals(strDistanceMode, "laneprojection")) {
        tn.nDistanceMode = TDM_LANE_PROJECTION;
      } else if (boost::algorithm::iequals(strDistanceMode, "euclideandistance")) {
        tn.nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
      } else {
        assert(false);
        SYSTEM_LOGGER_ERROR("acc trig condition distance mode error!");
        continue;
      }
      tn.dTriggerValue = atof(strNodes[2].c_str());
      tn.dAcc = atof(strNodes[3].c_str());
      tn.nCount = 1;

      tn.nStopType = AECT_NONE;
      tn.dStopValue = 0;
      if (strOneConditionNodes.size() == 2) {
        tn.nStopType = AccEndConditionTypeStr2Value(strOneConditionNodes[0]);
        tn.dStopValue = atof(strOneConditionNodes[1].c_str());
      }
      m_nodes.push_back(tn);

    } else if (strNodes.size() == 5) {
      TimeNode tn;
      std::string strType = strNodes[0];
      if (boost::algorithm::iequals(strType, "ttc")) {
        tn.nTriggerType = TCT_TIME_RELATIVE;
      } else if (boost::algorithm::iequals(strType, "egoDistance")) {
        tn.nTriggerType = TCT_POSITION_RELATIVE;
      } else {
        assert(false);
        SYSTEM_LOGGER_ERROR("acc trig condition type error!");
        continue;
      }

      std::string strDistanceMode = strNodes[1];
      if (boost::algorithm::iequals(strDistanceMode, "laneprojection")) {
        tn.nDistanceMode = TDM_LANE_PROJECTION;
      } else if (boost::algorithm::iequals(strDistanceMode, "euclideandistance")) {
        tn.nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
      } else {
        assert(false);
        SYSTEM_LOGGER_ERROR("acc trig condition distance mode error!");
        continue;
      }
      tn.dTriggerValue = atof(strNodes[2].c_str());
      tn.dAcc = atof(strNodes[3].c_str());

      std::string strCountProfile = strNodes[4];
      std::string strCount = strCountProfile.substr(1, strCountProfile.length() - 2);
      tn.nCount = atoi(strCount.c_str());

      tn.nStopType = AECT_NONE;
      tn.dStopValue = 0;
      if (strOneConditionNodes.size() == 2) {
        tn.nStopType = AccEndConditionTypeStr2Value(strOneConditionNodes[0]);
        tn.dStopValue = atof(strOneConditionNodes[1].c_str());
      }

      m_nodes.push_back(tn);

    } else {
      SYSTEM_LOGGER_ERROR("traffic file acc event profile error!");
      continue;
    }
  }

  m_nodes.sort();
}

void CAcceleration::ConvertToValue() {
  m_ID = atoi(m_strID.c_str());

  std::vector<std::string> strs;
  CParseTools::SplitString(m_strProfiles, strs, ";");
  for (int i = 0; i < strs.size(); ++i) {
    std::string strAcc = strs[i];
    std::vector<std::string> strNodes;
    CParseTools::SplitString(strAcc, strNodes, ",");
    assert(strNodes.size() == 2 || strNodes.size() == 3);
    if (strNodes.size() == 2) {
      TimeNode tn;
      tn.nTriggerType = TCT_TIME_ABSOLUTE;
      tn.dTriggerValue = atof(strNodes[0].c_str());
      tn.dAcc = atof(strNodes[1].c_str());

      m_nodes.push_back(tn);
    } else if (strNodes.size() == 3) {
      TimeNode tn;
      tn.nTriggerType = atoi(strNodes[0].c_str());
      if (tn.nTriggerType < TCT_START || tn.nTriggerType > TCT_END) {
        assert(false);
        SYSTEM_LOGGER_ERROR("acc trig condition type error!");
        continue;
      }
      tn.dTriggerValue = atof(strNodes[1].c_str());
      tn.dAcc = atof(strNodes[2].c_str());

      m_nodes.push_back(tn);
    } else {
      SYSTEM_LOGGER_ERROR("traffic file acc profile error!");
      continue;
    }
  }
  m_nodes.sort();
}
