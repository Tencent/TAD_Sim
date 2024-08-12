/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <cassert>
#include <list>
#include <string>
#include "common/engine/constant.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/parser_tools.h"

struct EventNode {
  int nTriggerType;
  double dTriggerValue;
  int nDistanceMode;
  int nCount;

  std::string m_strTimeStr;
  std::string m_strEventStr;

 public:
  void Reset();

  bool operator<(const EventNode& other);
  bool operator==(const EventNode& other);
  bool operator!=(const EventNode& other);
  EventNode& operator=(const EventNode& other);

  virtual int ConvertToTimeValue() = 0;
  virtual int ConvertToEventValue() = 0;

  virtual void ConvertToTimeStr() = 0;
  virtual void ConvertToEventStr() = 0;
};

template <typename T>
class CTrafficEvent {
 public:
  typedef std::list<T> EventNodes;
  void Reset();
  void ConvertToValue();
  void ConvertToTimeValue();
  void ConvertToEventValue();

  void ConvertToStr();
  void ConvertToTimeStr();
  void ConvertToEventStr();
  CTrafficEvent& operator=(const CTrafficEvent& other);

  std::string m_strID;
  std::string m_strProfiles;
  std::string m_strProfilesTime;
  std::string m_strProfilesEvent;
  uint64_t m_ID;
  EventNodes m_nodes;
};

template <typename T>
void CTrafficEvent<T>::Reset() {
  m_strID = "";
  m_strProfiles = "";
  m_strProfilesTime = "";
  m_strProfilesEvent = "";

  m_ID = 0;
  m_nodes.clear();
}

template <typename T>
CTrafficEvent<T>& CTrafficEvent<T>::operator=(const CTrafficEvent<T>& other) {
  m_ID = other.m_ID;
  m_strProfiles = other.m_strProfiles;
  m_strProfilesEvent = other.m_strProfilesEvent;
  m_strProfilesTime = other.m_strProfilesTime;
  m_nodes = other.m_nodes;
  m_strID = other.m_strID;

  return (*this);
}

template <typename T>
void CTrafficEvent<T>::ConvertToTimeStr() {
  m_strID = std::to_string(m_ID);
  typename CTrafficEvent<T>::EventNodes::iterator itr = m_nodes.begin();
  // auto nSize = m_nodes.size();
  int nCount = 0;
  for (; itr != m_nodes.end(); ++itr) {
    if (itr->nTriggerType == TCT_TIME_ABSOLUTE) {
      itr->ConvertToTimeStr();
      if (nCount > 0) {
        m_strProfilesTime.append(";");
      }

      m_strProfilesTime.append(itr->m_strTimeStr);
      nCount++;
    }
  }
}
template <typename T>
void CTrafficEvent<T>::ConvertToEventStr() {
  m_strID = std::to_string(m_ID);
  typename CTrafficEvent<T>::EventNodes::iterator itr = m_nodes.begin();
  // auto nSize = m_nodes.size();
  int nCount = 0;
  for (; itr != m_nodes.end(); ++itr) {
    if (itr->nTriggerType != TCT_TIME_ABSOLUTE) {
      itr->ConvertToEventStr();

      if (nCount > 0) {
        m_strProfilesEvent.append(";");
      }
      m_strProfilesEvent.append(itr->m_strEventStr);
      nCount++;
    }
  }
}
template <typename T>
void CTrafficEvent<T>::ConvertToStr() {
  m_strID = std::to_string(m_ID);
  typename CTrafficEvent<T>::EventNodes::iterator itr = m_nodes.begin();
  int nSize = m_nodes.size();
  int i = 0;
  for (; itr != m_nodes.end(); ++itr) {
    std::string strTrigType = std::to_string((*itr).nTriggerType);
    std::string strTrigValue = std::to_string((*itr).dTriggerValue);
    std::string strVelocity = std::to_string((*itr).dVelocity);
    std::string strOneNode = strTrigType + "," + strTrigValue + "," + strVelocity;
    if (i + 1 != nSize) {
      strOneNode.append(";");
    }

    m_strProfiles.append(strOneNode);
    i++;
  }
}

template <typename T>
void CTrafficEvent<T>::ConvertToTimeValue() {
  m_ID = atoi(m_strID.c_str());

  std::vector<std::string> strs;
  CParseTools::SplitString(m_strProfilesTime, strs, ";");

  for (int i = 0; i < strs.size(); ++i) {
    T tn;
    tn.m_strTimeStr = strs[i];
    int nRet = tn.ConvertToTimeValue();
    if (nRet != 0) {
      continue;
    }
    m_nodes.emplace_back(tn);
  }
  m_nodes.sort();
}

template <typename T>
void CTrafficEvent<T>::ConvertToEventValue() {
  m_ID = atoi(m_strID.c_str());

  std::vector<std::string> strs;
  CParseTools::SplitString(m_strProfilesEvent, strs, ";");
  for (size_t i = 0; i < strs.size(); ++i) {
    T tn;
    tn.m_strEventStr = strs[i];
    int nRet = tn.ConvertToEventValue();
    if (nRet != 0) {
      continue;
    }
    m_nodes.push_back(tn);
  }

  m_nodes.sort();
}

template <typename T>
void CTrafficEvent<T>::ConvertToValue() {
  m_ID = atoi(m_strID.c_str());

  std::vector<std::string> strs;
  CParseTools::SplitString(m_strProfiles, strs, ";");
  for (int i = 0; i < strs.size(); ++i) {
    std::string strVelocity = strs[i];
    std::vector<std::string> strNodes;
    CParseTools::SplitString(strVelocity, strNodes, ",");
    assert(strNodes.size() == 2 || strNodes.size() == 3);
    if (strNodes.size() == 2) {
      T tn;
      tn.nTriggerType = TCT_TIME_ABSOLUTE;
      tn.dTriggerValue = atof(strNodes[0].c_str());
      tn.dVelocity = atof(strNodes[1].c_str());

      m_nodes.push_back(tn);
    } else if (strNodes.size() == 3) {
      T tn;
      tn.nTriggerType = atoi(strNodes[0].c_str());
      if (tn.nTriggerType < TCT_START || tn.nTriggerType > TCT_END) {
        assert(false);
        SYSTEM_LOGGER_ERROR("velocity trig condition type error!");
        continue;
      }
      tn.dTriggerValue = atof(strNodes[1].c_str());
      tn.dVelocity = atof(strNodes[2].c_str());

      m_nodes.push_back(tn);
    } else {
      SYSTEM_LOGGER_ERROR("traffic file velocity profile error!");
      continue;
    }
  }
  m_nodes.sort();
}
