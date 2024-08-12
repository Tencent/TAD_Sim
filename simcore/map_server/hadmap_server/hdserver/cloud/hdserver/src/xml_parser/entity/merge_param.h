/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "merge.h"

struct tagMergeParamUnit : public tagMergeUnit {
  double dTrigValueStart;
  double dTrigValueEnd;
  double dTrigValueSep;

  double dDurationStart;
  double dDurationEnd;
  double dDurationSep;

  double dOffsetStart;
  double dOffsetEnd;
  double dOffsetSep;
};

typedef std::list<tagMergeParamUnit> MergeParamUnits;

class CMergeParam : public CMerge {
 public:
  typedef std::list<MergeUnits> MergeDatus;
  typedef std::list<tagMergeUnit> ParamizedMergeNodes;
  typedef std::list<ParamizedMergeNodes> SequencedParamizedMergeNodes;
  typedef std::list<CMerge> Merges;

  void Reset();
  uint64_t ListVariations(SequencedParamizedMergeNodes& sequencedContainer, bool onlyCalcualte = false);

  int Generate(Merges& merges);
  MergeParamUnits& ParamMerges() { return m_paramMerges; }
  Merges& GeneratedMerges() { return m_generatedMerges; }

  int Compose(SequencedParamizedMergeNodes& containers, MergeDatus& inDatus, MergeDatus& outDatus);

  Merges m_generatedMerges;
  MergeParamUnits m_paramMerges;
};
