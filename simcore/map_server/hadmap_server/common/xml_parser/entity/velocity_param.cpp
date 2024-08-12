/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/velocity_param.h"
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <cmath>
#include <vector>
#include "common/engine/constant.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/parser_tools.h"

void CVelocityParam::Reset() { m_paramNodes.clear(); }

uint64_t CVelocityParam::ListVariations(SequencedParamizedVelocityTimeNodes& sequencedContainer, bool onlyCalculate) {
  sequencedContainer.clear();

  VelocityTimeParamNodes::iterator itr = m_paramNodes.begin();
  for (; itr != m_paramNodes.end(); ++itr) {
    ParamizedVelocityTimeNodes paramizedNodes;

    double dTriggerValueDelta = std::fabs(itr->dTriggerValueEnd - itr->dTriggerValueStart);
    double dVelocityDelta = std::fabs(itr->dVelocityEnd - itr->dVelocityStart);

    double dTriggerStartValue =
        itr->dTriggerValueStart > itr->dTriggerValueEnd ? itr->dTriggerValueEnd : itr->dTriggerValueStart;
    double dVelocityStartValue = itr->dVelocityStart > itr->dVelocityEnd ? itr->dVelocityEnd : itr->dVelocityStart;

    double dTriggerValue = itr->dTriggerValueStart;
    double dVelocityValue = itr->dVelocityStart;

    int nTriggerValueCount = 0;
    int nVelocityCount = 0;
    if (itr->dTriggerValueSep > 0) {
      nTriggerValueCount = dTriggerValueDelta / itr->dTriggerValueSep;
    }

    if (itr->dVelocitySep > 0) {
      nVelocityCount = dVelocityDelta / itr->dVelocitySep;
    }

    if (onlyCalculate) {
      nTriggerValueCount++;
      nVelocityCount++;
      uint64_t ullCount = nTriggerValueCount * nVelocityCount;
      return ullCount;
    }

    for (int i = 0; i <= nTriggerValueCount; ++i) {
      dTriggerValue = dTriggerStartValue + i * itr->dTriggerValueSep;
      for (int j = 0; j <= nVelocityCount; ++j) {
        dVelocityValue = dVelocityStartValue + j * itr->dVelocitySep;
        VelocityTimeNode tn = (*itr);
        tn.dTriggerValue = dTriggerValue;
        tn.dVelocity = dVelocityValue;
        paramizedNodes.push_back(tn);
      }
    }

    sequencedContainer.push_back(paramizedNodes);
  }

#ifdef __CLOUD_HADMAP_SERVER__
  return 0;
#else
  return 1;
#endif
}

int CVelocityParam::Generate(Velocitys& velocities) {
  SequencedParamizedVelocityTimeNodes sequencedContainer;
  ListVariations(sequencedContainer, false);

  velocities.clear();

  VelocityDatus inDataus;
  VelocityDatus outDataus;

  Compose(sequencedContainer, inDataus, outDataus);

  VelocityDatus::iterator vitr = outDataus.begin();
  for (; vitr != outDataus.end(); ++vitr) {
    CVelocity vel;
    vel = (*this);
    vel.m_nodes = (*vitr);
    vel.ConvertToEventStr();
    vel.ConvertToTimeStr();

    velocities.push_back(vel);
  }
  return 0;
}

int CVelocityParam::Compose(SequencedParamizedVelocityTimeNodes& containers, VelocityDatus& inDatus,
                            VelocityDatus& outDatus) {
  VelocityDatus curRecursiveOutDatus;

  if (containers.size() == 0) {
    outDatus = inDatus;

  } else {
    SequencedParamizedVelocityTimeNodes::iterator first = containers.begin();
    ParamizedVelocityTimeNodes::iterator pItr = first->begin();
    if (inDatus.size() == 0) {
      for (; pItr != first->end(); ++pItr) {
        VelocityTimeNodes vel;
        vel.push_back((*pItr));
        curRecursiveOutDatus.push_back(vel);
      }

    } else {
      for (; pItr != first->end(); ++pItr) {
        VelocityDatus::iterator vItr = inDatus.begin();
        for (; vItr != inDatus.end(); ++vItr) {
          VelocityTimeNodes vtns;
          vtns = (*vItr);
          vtns.push_back((*pItr));
          curRecursiveOutDatus.push_back(vtns);
        }
      }
    }

    containers.pop_front();
    return Compose(containers, curRecursiveOutDatus, outDatus);
  }

  return 0;
}
