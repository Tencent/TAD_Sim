/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <list>
#include <string>

struct tagMergeParamUnit;
struct tagMergeUnit {
  int nTrigType;
  int nDistanceMode;
  double dTrigValue;
  int nDir;
  double dDuration;
  double dOffset;
  int nCount;

  tagMergeUnit& operator=(const tagMergeParamUnit& other);
  bool operator<(const tagMergeUnit& other);
  bool operator==(const tagMergeUnit& other);
  bool operator!=(const tagMergeUnit& other);
};

typedef std::list<tagMergeUnit> MergeUnits;

class CMerge {
 public:
  std::string m_strID;
  std::string m_strProfile;
  std::string m_strProfileTime;
  std::string m_strProfileEvent;

  void Reset();
  void ClearNodes();
  void SetToDefault();
  void ConvertToValue();
  void ConvertToTimeValue();
  void ConvertToEventValue();
  void ConvertToStr();
  void ConvertToTimeStr();
  void ConvertToEventStr();
  int PushMerge(tagMergeUnit& merge);
  int MergeCount() { return m_merges.size(); }
  int RemoveMerge(int nNum);
  int ModifyMerge(int nNum, tagMergeUnit& merge);
  MergeUnits& Merges() { return m_merges; }

  CMerge& operator=(const CMerge& other);
  // protected:
  uint64_t m_ID;
  MergeUnits m_merges;
};
