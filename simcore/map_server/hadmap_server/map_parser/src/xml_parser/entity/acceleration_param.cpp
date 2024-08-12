/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "acceleration_param.h"
#include <boost/algorithm/string.hpp>
#include "common/engine/constant.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/parser_tools.h"

void CAccelerationParam::Reset() {
  CAcceleration::Reset();
  m_paramNodes.clear();
}

uint64_t CAccelerationParam::ListVariations(SequencedParamizedTimeNodes& sequencedContainer, bool onlyCalculate) {
  sequencedContainer.clear();

  TimeParamNodes::iterator itr = m_paramNodes.begin();
  for (; itr != m_paramNodes.end(); ++itr) {
    ParamizedTimeNodes paramizedNodes;

    double dTriggerValueDelta = std::fabs(itr->dTriggerValueEnd - itr->dTriggerValueStart);
    double dAccDelta = std::fabs(itr->dAccEnd - itr->dAccStart);

    double dTriggerStartValue =
        itr->dTriggerValueStart > itr->dTriggerValueEnd ? itr->dTriggerValueEnd : itr->dTriggerValueStart;
    double dAccStart = itr->dAccStart > itr->dAccEnd ? itr->dAccEnd : itr->dAccStart;

    double dTriggerValue = dTriggerStartValue;
    double dAcc = dAccStart;

    int nTriggerValueCount = 0;
    int nAccCount = 0;
    if (itr->dTriggerValueSep > 0) {
      nTriggerValueCount = dTriggerValueDelta / itr->dTriggerValueSep;
    }

    if (itr->dAccSep > 0) {
      nAccCount = dAccDelta / itr->dAccSep;
    }

    if (onlyCalculate) {
      nTriggerValueCount++;
      nAccCount++;
      uint64_t ullCount = nTriggerValueCount * nAccCount;
      return ullCount;
    }

    for (int i = 0; i <= nTriggerValueCount; ++i) {
      dTriggerValue = dTriggerStartValue + i * itr->dTriggerValueSep;
      for (int j = 0; j <= nAccCount; ++j) {
        dAcc = dAccStart + j * itr->dAccSep;
        TimeNode tn = (*itr);
        tn.dTriggerValue = dTriggerValue;
        tn.dAcc = dAcc;
        paramizedNodes.push_back(tn);
      }
    }

    sequencedContainer.push_back(paramizedNodes);
  }
  return 1;
}

int CAccelerationParam::Generate(Accelerations& accs) {
  SequencedParamizedTimeNodes sequencedContainer;
  ListVariations(sequencedContainer, false);

  accs.clear();

  AccelerationDatus inDataus;
  AccelerationDatus outDataus;

  Compose(sequencedContainer, inDataus, outDataus);

  AccelerationDatus::iterator aitr = outDataus.begin();
  for (; aitr != outDataus.end(); ++aitr) {
    CAcceleration acc;
    acc = (*this);
    acc.m_nodes = (*aitr);
    acc.ConvertToEventStr();
    acc.ConvertToTimeStr();

    accs.push_back(acc);
  }

  return 0;
}

int CAccelerationParam::Compose(SequencedParamizedTimeNodes& containers, AccelerationDatus& inDatus,
                                AccelerationDatus& outDatus) {
  AccelerationDatus curRecursiveOutDatus;

  if (containers.size() == 0) {
    outDatus = inDatus;

  } else {
    SequencedParamizedTimeNodes::iterator first = containers.begin();
    ParamizedTimeNodes::iterator pItr = first->begin();
    if (inDatus.size() == 0) {
      for (; pItr != first->end(); ++pItr) {
        TimeNodes acc;
        acc.push_back((*pItr));
        curRecursiveOutDatus.push_back(acc);
      }

    } else {
      for (; pItr != first->end(); ++pItr) {
        AccelerationDatus::iterator aItr = inDatus.begin();
        for (; aItr != inDatus.end(); ++aItr) {
          TimeNodes tns;
          tns = (*aItr);
          tns.push_back((*pItr));
          curRecursiveOutDatus.push_back(tns);
        }
      }
    }

    containers.pop_front();
    return Compose(containers, curRecursiveOutDatus, outDatus);
  }

  return 0;
}
