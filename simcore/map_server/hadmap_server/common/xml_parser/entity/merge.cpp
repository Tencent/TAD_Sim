/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/merge.h"
#include <algorithm>
#include <cassert>
#include <vector>

#include <boost/algorithm/string.hpp>
#include "common/engine/constant.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/merge_param.h"
#include "common/xml_parser/entity/parser_tools.h"

tagMergeUnit& tagMergeUnit::operator=(const tagMergeParamUnit& other) {
  nTrigType = other.nTrigType;
  dTrigValue = other.dTrigValue;
  nDistanceMode = other.nDistanceMode;
  nDir = other.nDir;
  dDuration = other.dDuration;
  dOffset = other.dOffset;
  nCount = other.nCount;
  return (*this);
}

bool tagMergeUnit::operator<(const tagMergeUnit& other) {
  if (nTrigType < other.nTrigType) {
    return true;
  } else if (nTrigType == other.nTrigType && dTrigValue < other.dTrigValue) {
    return true;
  }

  return false;
}

bool tagMergeUnit::operator==(const tagMergeUnit& other) {
  if (nTrigType != other.nTrigType) {
    return false;
  }
  const float epsil = 0.000001;
  float delta = dTrigValue - other.dTrigValue;
  if (delta > -1 * epsil && delta < epsil) {
    return true;
  }

  return false;
}

bool tagMergeUnit::operator!=(const tagMergeUnit& other) { return ((*this) == other); }

void CMerge::ClearNodes() { m_merges.clear(); }

void CMerge::Reset() {
  m_strID = "";
  m_strProfile = "";
  m_strProfileTime = "";
  m_strProfileEvent = "";
  m_ID = 0;
  ClearNodes();
}

void CMerge::SetToDefault() {
  m_strProfile = "";
  m_strProfileTime = "0,0";
  m_strProfileEvent = "ttc 0,0";
  ClearNodes();
  ConvertToTimeValue();
  ConvertToEventValue();
}

void CMerge::ConvertToTimeValue() {
  m_ID = atoi(m_strID.c_str());

  std::vector<std::string> strs;
  CParseTools::SplitString(m_strProfileTime, strs, ";");
  for (int i = 0; i < strs.size(); ++i) {
    std::string strMerge = strs[i];
    if (strMerge.length() == 0) {
      tagMergeUnit tmu;
      tmu.nTrigType = TCT_TIME_ABSOLUTE;
      tmu.nDistanceMode = TDM_NONE;
      tmu.dTrigValue = 0;
      tmu.nDir = 0;
      tmu.dDuration = 4.5;
      tmu.dOffset = 0;
      tmu.nCount = -1;
      m_merges.push_back(tmu);

    } else {
      std::vector<std::string> strUnits;
      CParseTools::SplitString(strMerge, strUnits, ",");
      assert(strUnits.size() == 2 || strUnits.size() == 3 || strUnits.size() == 4);
      if (strUnits.size() == 2) {
        tagMergeUnit tmu;
        tmu.nTrigType = TCT_TIME_ABSOLUTE;
        tmu.nDistanceMode = TDM_NONE;
        tmu.dTrigValue = atof(strUnits[0].c_str());
        tmu.nDir = atoi(strUnits[1].c_str());
        tmu.dDuration = 4.5;
        tmu.dOffset = 0;
        tmu.nCount = -1;
        m_merges.push_back(tmu);
      } else if (strUnits.size() == 3 || strUnits.size() == 4) {
        tagMergeUnit tmu;
        tmu.nTrigType = TCT_TIME_ABSOLUTE;
        tmu.nDistanceMode = TDM_NONE;
        tmu.dTrigValue = atof(strUnits[0].c_str());
        tmu.nDir = atoi(strUnits[1].c_str());
        tmu.dDuration = atof(strUnits[2].c_str());
        tmu.dOffset = 0;
        tmu.nCount = -1;
        if (tmu.nDir == 2 || tmu.nDir == -2) {
          assert(strUnits.size() == 4);
          tmu.dOffset = atof(strUnits[3].c_str());
        }

        m_merges.push_back(tmu);
      } else {
        SYSTEM_LOGGER_ERROR("traffic file merge profile error!");
      }
    }
  }

  m_merges.sort();
}

void CMerge::ConvertToEventValue() {
  m_ID = atoi(m_strID.c_str());

  std::vector<std::string> strs;
  CParseTools::SplitString(m_strProfileEvent, strs, ";");
  for (int i = 0; i < strs.size(); ++i) {
    std::string strMerge = strs[i];
    if (strMerge.length() == 0) {
      continue;
    } else {
      std::vector<std::string> strUnits;
      CParseTools::SplitString2(strMerge, strUnits, " ,");
      assert(strUnits.size() == 3 || strUnits.size() == 4 || strUnits.size() == 6 || strUnits.size() == 7);
      if (strUnits.size() == 3) {
        tagMergeUnit tmu;
        std::string strType = strUnits[0];
        if (boost::algorithm::iequals(strType, "ttc")) {
          tmu.nTrigType = TCT_TIME_RELATIVE;
        } else if (boost::algorithm::iequals(strType, "egoDistance")) {
          tmu.nTrigType = TCT_POSITION_RELATIVE;
        } else {
          assert(false);
          SYSTEM_LOGGER_ERROR("merge trig condition type error!");
          continue;
        }

        tmu.nDistanceMode = TDM_LANE_PROJECTION;
        tmu.dTrigValue = atof(strUnits[1].c_str());
        tmu.nDir = atoi(strUnits[2].c_str());
        tmu.dDuration = 4.5;
        tmu.dOffset = 0;
        tmu.nCount = 1;
        m_merges.push_back(tmu);

      } else if (strUnits.size() == 4) {
        tagMergeUnit tmu;
        std::string strType = strUnits[0];
        if (boost::algorithm::iequals(strType, "ttc")) {
          tmu.nTrigType = TCT_TIME_RELATIVE;
        } else if (boost::algorithm::iequals(strType, "egoDistance")) {
          tmu.nTrigType = TCT_POSITION_RELATIVE;
        } else {
          assert(false);
          SYSTEM_LOGGER_ERROR("merge trig condition type error!");
          continue;
        }

        std::string strDistanceMode = strUnits[1];
        if (boost::algorithm::iequals(strDistanceMode, "laneprojection")) {
          tmu.nDistanceMode = TDM_LANE_PROJECTION;
        } else if (boost::algorithm::iequals(strDistanceMode, "euclideandistance")) {
          tmu.nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
        } else {
          assert(false);
          SYSTEM_LOGGER_ERROR("merge trig condition distance mode error!");
          continue;
        }
        tmu.dTrigValue = atof(strUnits[2].c_str());
        tmu.nDir = atoi(strUnits[3].c_str());
        tmu.dDuration = 4.5;
        tmu.dOffset = 0;
        tmu.nCount = 1;
        m_merges.push_back(tmu);

      } else if (strUnits.size() == 7 || strUnits.size() == 6) {
        tagMergeUnit tmu;
        std::string strType = strUnits[0];
        if (boost::algorithm::iequals(strType, "ttc")) {
          tmu.nTrigType = TCT_TIME_RELATIVE;
        } else if (boost::algorithm::iequals(strType, "egoDistance")) {
          tmu.nTrigType = TCT_POSITION_RELATIVE;
        } else {
          assert(false);
          SYSTEM_LOGGER_ERROR("merge trig condition type error!");
          continue;
        }

        std::string strDistanceMode = strUnits[1];
        if (boost::algorithm::iequals(strDistanceMode, "laneprojection")) {
          tmu.nDistanceMode = TDM_LANE_PROJECTION;
        } else if (boost::algorithm::iequals(strDistanceMode, "euclideandistance")) {
          tmu.nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
        } else {
          assert(false);
          SYSTEM_LOGGER_ERROR("merge trig condition distance mode error!");
          continue;
        }
        tmu.dTrigValue = atof(strUnits[2].c_str());
        tmu.nDir = atoi(strUnits[3].c_str());
        tmu.dDuration = atof(strUnits[4].c_str());
        tmu.dOffset = 0;
        if (tmu.nDir == 2 || tmu.nDir == -2) {
          tmu.dOffset = atof(strUnits[5].c_str());
        }

        std::string strCountProfile = strUnits[6];
        std::string strCount = strCountProfile.substr(1, strCountProfile.length() - 2);
        tmu.nCount = atoi(strCount.c_str());

        m_merges.push_back(tmu);

      } else {
        SYSTEM_LOGGER_ERROR("traffic file merge event profile error!");
      }
    }
  }

  m_merges.sort();
}

void CMerge::ConvertToValue() {
  m_ID = atoi(m_strID.c_str());

  std::vector<std::string> strs;
  CParseTools::SplitString(m_strProfile, strs, ";");
  for (int i = 0; i < strs.size(); ++i) {
    std::string strMerge = strs[i];
    if (strMerge.length() == 0) {
      tagMergeUnit tmu;
      tmu.nTrigType = TCT_TIME_ABSOLUTE;
      tmu.nDistanceMode = TDM_NONE;
      tmu.dTrigValue = 0;
      tmu.nDir = 0;
      m_merges.push_back(tmu);

    } else {
      std::vector<std::string> strUnits;
      CParseTools::SplitString(strMerge, strUnits, ",");
      assert(strUnits.size() == 2 || strUnits.size() == 3);
      if (strUnits.size() == 2) {
        tagMergeUnit tmu;
        tmu.nTrigType = TCT_TIME_ABSOLUTE;
        tmu.nDistanceMode = TDM_NONE;
        tmu.dTrigValue = atof(strUnits[0].c_str());
        tmu.nDir = atoi(strUnits[1].c_str());
        m_merges.push_back(tmu);
      } else if (strUnits.size() == 3) {
        tagMergeUnit tmu;
        tmu.nTrigType = atoi(strUnits[0].c_str());
        tmu.nDistanceMode = TDM_NONE;
        tmu.dTrigValue = atof(strUnits[1].c_str());
        tmu.nDir = atoi(strUnits[2].c_str());
        m_merges.push_back(tmu);
      } else {
        SYSTEM_LOGGER_ERROR("traffic file acc profile error!");
      }
    }
  }

  m_merges.sort();
}

void CMerge::ConvertToTimeStr() {
  m_strProfileTime.clear();

  m_strID = std::to_string(m_ID);
  MergeUnits::iterator itr = m_merges.begin();
  int nSize = m_merges.size();
  int i = 0;
  int nCount = 0;
  for (; itr != m_merges.end(); ++itr) {
    if (itr->nTrigType == TCT_TIME_ABSOLUTE) {
      std::string strValue = CUtils::doubleToStringDot3((*itr).dTrigValue);
      std::string strDir = std::to_string((*itr).nDir);
      std::string strDuration = CUtils::doubleToStringDot3((*itr).dDuration);
      std::string strOneNode;
      if ((*itr).nDir == 2 || (*itr).nDir == -2) {
        std::string strOffset = CUtils::doubleToStringDot3((*itr).dOffset);
        strOneNode = strValue + "," + strDir + "," + strDuration + "," + strOffset;
      } else {
        strOneNode = strValue + "," + strDir + "," + strDuration;
      }
      if (nCount > 0) {
        m_strProfileTime.append(";");
      }
      m_strProfileTime.append(strOneNode);
      nCount++;
    }

    i++;
  }
}

void CMerge::ConvertToEventStr() {
  m_strProfileEvent.clear();

  m_strID = std::to_string(m_ID);
  MergeUnits::iterator itr = m_merges.begin();
  int nSize = m_merges.size();
  int i = 0;
  int nCount = 0;
  for (; itr != m_merges.end(); ++itr) {
    if (itr->nTrigType != TCT_TIME_ABSOLUTE) {
      std::string strType = "ttc";
      if (itr->nTrigType == TCT_POSITION_RELATIVE) {
        strType = "egoDistance";
      }

      std::string strDistanceMode = "laneprojection";
      if (itr->nDistanceMode == TDM_EUCLIDEAN_DISTANCE) {
        strDistanceMode = "euclideandistance";
      }
      std::string strValue = CUtils::doubleToStringDot3((*itr).dTrigValue);
      std::string strDir = std::to_string((*itr).nDir);
      std::string strDuration = CUtils::doubleToStringDot3((*itr).dDuration);
      std::string strOffset = CUtils::doubleToStringDot3((*itr).dOffset);
      std::string strCount = std::to_string((*itr).nCount);

      std::string strOneNode;

      if ((*itr).nDir == 2 || (*itr).nDir == -2) {
        strOneNode = strType + " " + strDistanceMode + " " + strValue + "," + strDir + "," + strDuration + "," +
                     strOffset + " [" + strCount + "]";
      } else {
        strOneNode = strType + " " + strDistanceMode + " " + strValue + "," + strDir + "," + strDuration + ",v [" +
                     strCount + "]";
      }
      if (nCount > 0) {
        m_strProfileEvent.append(";");
      }
      m_strProfileEvent.append(strOneNode);
      nCount++;
    }

    i++;
  }
}

void CMerge::ConvertToStr() {
  m_strProfile.clear();

  m_strID = std::to_string(m_ID);
  MergeUnits::iterator itr = m_merges.begin();
  int nSize = m_merges.size();
  int i = 0;
  for (; itr != m_merges.end(); ++itr) {
    std::string strType = std::to_string((*itr).nTrigType);
    std::string strValue = CUtils::doubleToStringDot3((*itr).dTrigValue);
    std::string strDir = std::to_string((*itr).nDir);
    std::string strOneNode = strType + "," + strValue + "," + strDir;
    if (i + 1 != nSize) {
      strOneNode.append(";");
    }
    m_strProfile.append(strOneNode);
    i++;
  }
}

int CMerge::PushMerge(tagMergeUnit& merge) {
  MergeUnits::iterator itr = std::find(m_merges.begin(), m_merges.end(), merge);
  if (itr != m_merges.end()) {
    return -1;
  }

  m_merges.push_back(merge);
  m_merges.sort();
  return 0;
}

int CMerge::RemoveMerge(int nNum) {
  if (nNum >= m_merges.size()) {
    assert(false);
    return -1;
  }
  int i = 0;
  MergeUnits::iterator itr = m_merges.begin();
  for (; itr != m_merges.end(); ++itr) {
    if (i == nNum) {
      break;
    }
    i++;
  }

  m_merges.erase(itr);
  // m_merges.sort();
  return 0;
}

int CMerge::ModifyMerge(int nNum, tagMergeUnit& merge) {
  if (nNum >= m_merges.size()) {
    assert(false);
    return -1;
  }
  int i = 0;
  MergeUnits::iterator itr1 = m_merges.begin();
  for (; itr1 != m_merges.end(); ++itr1) {
    if (i == nNum) {
      break;
    }
    i++;
  }

  MergeUnits::iterator itr2 = std::find(m_merges.begin(), m_merges.end(), merge);

  if (itr2 != m_merges.end() && itr1 != itr2) {
    return -1;
  }

  (*itr1) = merge;

  m_merges.sort();
  return 0;
}

CMerge& CMerge::operator=(const CMerge& other) {
  m_ID = other.m_ID;
  m_strProfile = other.m_strProfile;
  m_strProfileEvent = other.m_strProfileEvent;
  m_strProfileTime = other.m_strProfileTime;
  m_merges = other.m_merges;
  m_strID = other.m_strID;

  return (*this);
}
