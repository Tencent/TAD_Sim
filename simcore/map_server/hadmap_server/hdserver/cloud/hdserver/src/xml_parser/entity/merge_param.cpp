/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "merge_param.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <cmath>
#include <vector>
#include "common/engine/constant.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/parser_tools.h"

void CMergeParam::Reset() {
  CMerge::Reset();
  m_paramMerges.clear();
}

uint64_t CMergeParam::ListVariations(SequencedParamizedMergeNodes& sequencedContainer,
                                     bool onlyCalcualte /* = false */) {
  sequencedContainer.clear();

  MergeParamUnits::iterator itr = m_paramMerges.begin();
  for (; itr != m_paramMerges.end(); ++itr) {
    ParamizedMergeNodes paramizedNodes;

    double dTriggerValueDelta = std::fabs(itr->dTrigValueEnd - itr->dTrigValueStart);
    double dOffsetDelta = std::fabs(itr->dOffsetEnd - itr->dOffsetStart);
    double dDurationDelta = std::fabs(itr->dDurationEnd - itr->dDurationStart);

    double dTriggerStartValue = itr->dTrigValueStart > itr->dTrigValueEnd ? itr->dTrigValueEnd : itr->dTrigValueStart;
    double dOffsetStartValue = itr->dOffsetStart > itr->dOffsetEnd ? itr->dOffsetEnd : itr->dOffsetStart;
    double dDurationStartValue = itr->dDurationStart > itr->dDurationEnd ? itr->dDurationEnd : itr->dDurationStart;

    double dTriggerValue = dTriggerStartValue;
    double dOffsetValue = dOffsetStartValue;
    double dDurationValue = dDurationStartValue;

    int nTriggerValueCount = 0;
    int nOffsetCount = 0;
    int nDurationCount = 0;
    if (itr->dTrigValueSep > 0) {
      nTriggerValueCount = dTriggerValueDelta / itr->dTrigValueSep;
    }
    if (itr->dOffsetSep > 0) {
      nOffsetCount = dOffsetDelta / itr->dOffsetSep;
    }
    if (itr->dDurationSep > 0) {
      nDurationCount = dDurationDelta / itr->dDurationSep;
    }

    if (onlyCalcualte) {
      nTriggerValueCount++;
      nOffsetCount++;
      nDurationCount++;
      uint64_t nllCount = nTriggerValueCount * nOffsetCount * nDurationCount;
      return nllCount;
    }

    for (int i = 0; i <= nTriggerValueCount; ++i) {
      dTriggerValue = dTriggerStartValue + i * itr->dTrigValueSep;
      for (int j = 0; j <= nOffsetCount; ++j) {
        dOffsetValue = dOffsetStartValue + j * itr->dOffsetSep;
        for (int k = 0; k <= nDurationCount; ++k) {
          dDurationValue = dDurationStartValue + j * itr->dDurationSep;
          tagMergeUnit mu = (*itr);
          mu.dTrigValue = dTriggerValue;
          mu.dOffset = dOffsetValue;
          mu.dDuration = dDurationValue;
          paramizedNodes.push_back(mu);
        }
      }
    }

    sequencedContainer.push_back(paramizedNodes);
  }

  return 0;
}

int CMergeParam::Generate(Merges& merges) {
  SequencedParamizedMergeNodes sequencedContainer;
  ListVariations(sequencedContainer, false);

  MergeDatus inDatus;
  MergeDatus outDatus;

  Compose(sequencedContainer, inDatus, outDatus);

  merges.clear();
  MergeDatus::iterator mItr = outDatus.begin();
  for (; mItr != outDatus.end(); ++mItr) {
    CMerge merge;
    merge = (*this);
    merge.m_merges = (*mItr);
    merge.ConvertToEventStr();
    merge.ConvertToTimeStr();
    merges.push_back(merge);
  }

  return 0;
}

int CMergeParam::Compose(SequencedParamizedMergeNodes& containers, MergeDatus& inDatus, MergeDatus& outDatus) {
  MergeDatus curRecursiveOutDatus;

  if (containers.size() == 0) {
    outDatus = inDatus;

  } else {
    SequencedParamizedMergeNodes::iterator first = containers.begin();
    ParamizedMergeNodes::iterator pItr = first->begin();

    if (inDatus.size() == 0) {
      for (; pItr != first->end(); ++pItr) {
        MergeUnits mus;
        mus.push_back((*pItr));
        curRecursiveOutDatus.push_back(mus);
      }

    } else {
      for (; pItr != first->end(); ++pItr) {
        MergeDatus::iterator inMItr = inDatus.begin();
        for (; inMItr != inDatus.end(); ++inMItr) {
          MergeUnits mus;
          mus = (*inMItr);
          mus.push_back((*pItr));
          curRecursiveOutDatus.push_back(mus);
        }
      }
    }

    containers.pop_front();

    return Compose(containers, curRecursiveOutDatus, outDatus);
  }
  return 0;
}
