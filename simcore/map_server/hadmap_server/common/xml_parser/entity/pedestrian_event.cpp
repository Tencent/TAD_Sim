/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/pedestrian_event.h"
#include <cassert>
#include <vector>

#include <boost/algorithm/string.hpp>
#include "common/engine/constant.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/parser_tools.h"

bool PedestrianEventNode::operator<(const PedestrianEventNode& other) {
  if (dTriggerValue < other.dTriggerValue) {
    return true;
  }

  return false;
}

bool PedestrianEventNode::operator==(const PedestrianEventNode& other) {
  const double epsil = 0.000000001;
  double delta = dTriggerValue - other.dTriggerValue;
  if (delta > -1 * epsil && delta < epsil) {
    return true;
  }

  return false;
}

bool PedestrianEventNode::operator!=(const PedestrianEventNode& other) { return !((*this) == other); }

void CPedestrianEvent::Reset() {
  m_ID = 0;
  m_TriggerType = TCT_TIME_RELATIVE;

  m_strID = "";
  m_strTriggerType = "";
  m_strProfiles = "";

  m_nodes.clear();
}

CPedestrianEvent& CPedestrianEvent::operator=(const CPedestrianEvent& other) {
  m_ID = other.m_ID;
  m_TriggerType = other.m_TriggerType;

  m_strID = other.m_strID;
  m_strTriggerType = other.m_strTriggerType;
  m_strProfiles = other.m_strProfiles;

  m_nodes = other.m_nodes;

  return (*this);
}

void CPedestrianEvent::ConvertToStr() {
  m_strID = std::to_string(m_ID);
  if (m_TriggerType == TCT_TIME_RELATIVE) {
    m_strTriggerType = "ttc";
  } else if (m_TriggerType == TCT_POSITION_RELATIVE) {
    m_strTriggerType = "egoDistance";
  } else {
    SYSTEM_LOGGER_ERROR("pedestrian event type error!");
    m_strTriggerType = "error";
    return;
  }

  if (m_nodes.size() > 0) {
    m_strProfiles = m_strTriggerType + " ";
  }

  PedestrianEventNodes::iterator itr = m_nodes.begin();
  int nSize = m_nodes.size();
  int i = 0;

  for (; itr != m_nodes.end(); ++itr) {
    std::string strTrigValue = CUtils::doubleToStringDot3((*itr).dTriggerValue);
    std::string strDirection = CUtils::doubleToStringDot3((*itr).dDirection);
    std::string strVelocity = CUtils::doubleToStringDot3((*itr).dVelocity);
    std::string strCount = std::to_string((*itr).nCount);
    std::string strOneNode = strTrigValue + "," + strDirection + "," + strVelocity + " [" + strCount + "]";
    if (i + 1 != nSize) {
      strOneNode.append(";");
    }

    m_strProfiles.append(strOneNode);
    i++;
  }
}

void CPedestrianEvent::ConvertToValue() {
  m_ID = atoi(m_strID.c_str());

  if (m_strProfiles.size() > 0) {
    std::vector<std::string> strs;
    CParseTools::SplitString(m_strProfiles, strs, ";");

    if (strs.size() > 0) {
      std::vector<std::string> strFirst;
      CParseTools::SplitString2(strs[0], strFirst, " ");
      // strs[0] = strFirst[1];
      m_strTriggerType = strFirst[0];
      if (boost::algorithm::iequals(m_strTriggerType, "ttc")) {
        m_TriggerType = TCT_TIME_RELATIVE;
      } else if (boost::algorithm::iequals(m_strTriggerType, "egoDistance")) {
        m_TriggerType = TCT_POSITION_RELATIVE;
      } else {
        assert(false);
        SYSTEM_LOGGER_ERROR("pedestrian trig condition type error!");
        m_TriggerType = TCT_TIME_RELATIVE;
      }

      for (int i = 0; i < strs.size(); ++i) {
        std::string strEvent = strs[i];
        std::vector<std::string> strNodes;
        CParseTools::SplitString2(strEvent, strNodes, " ,");
        assert(strNodes.size() == 3 || strNodes.size() == 4 || strNodes.size() == 5);
        int nStartIndex = 0;
        if (i == 0) {
          nStartIndex = 1;
        }

        if (strNodes.size() == nStartIndex + 3) {
          PedestrianEventNode pen;

          pen.dTriggerValue = atof(strNodes[nStartIndex].c_str());
          pen.dDirection = atof(strNodes[nStartIndex + 1].c_str());
          pen.dVelocity = atof(strNodes[nStartIndex + 2].c_str());
          pen.nCount = 1;

          m_nodes.push_back(pen);

        } else if (strNodes.size() == nStartIndex + 4) {
          PedestrianEventNode pen;

          pen.dTriggerValue = atof(strNodes[nStartIndex].c_str());
          pen.dDirection = atof(strNodes[nStartIndex + 1].c_str());
          pen.dVelocity = atof(strNodes[nStartIndex + 2].c_str());

          std::string strCountProfile = strNodes[nStartIndex + 3];
          std::string strCount = strCountProfile.substr(1, strCountProfile.length() - 2);
          pen.nCount = atoi(strCount.c_str());

          m_nodes.push_back(pen);

        } else {
          SYSTEM_LOGGER_ERROR("traffic file pedestrian event profile error!");
          continue;
        }
      }
    }
  }
  m_nodes.sort();
}
