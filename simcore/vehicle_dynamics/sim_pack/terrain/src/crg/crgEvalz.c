/* ===================================================
 *  file:       crgEval2z.c
 * ---------------------------------------------------
 *  purpose:	convert a u/v position into an
 *              elevation
 * ---------------------------------------------------
 *  based on routines by Dr. Jochen Rauh, Daimler AG
 * ---------------------------------------------------
 *  first edit:	26.11.2008 by M. Dupuis @ VIRES GmbH
 *  last mod.:  11.01.2016 by H. Helmich @ VIRES GmbH
 * ===================================================
    Copyright 2016 VIRES Simulationstechnologie GmbH

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */
/* ====== INCLUSIONS ====== */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "crgBaseLibPrivate.h"

/* ====== DEFINITIONS ====== */
#define dMaxBorderError 1.0e-8 /* maximum tolerance for position outside a border [m] */

/* ====== TYPE DEFINITIONS ====== */

/* ====== LOCAL METHODS ====== */

/* ====== IMPLEMENTATION ====== */
int crgEvaluv2z(int cpId, double u, double v, double* z) {
  CrgContactPointStruct* cp;

  if (!(cp = crgContactPointGetFromId(cpId))) return 0;

  return crgEvaluv2zPtr(cp, u, v, z);
}

int crgEvaluv2zPtr(CrgContactPointStruct* cp, double u, double v, double* z) {
  int retVal = 0;

  if (!cp) return 0;

  /* --- compute the fallback solution --- */
  cp->u = u;
  cp->v = v;

  retVal = crgDataEvaluv2z(cp->crgData, &(cp->options), cp->u, cp->v, &(cp->z));

  /* --- transfer the result --- */
  *z = cp->z;

  return retVal;
}

int crgDataEvaluv2z(CrgDataStruct* crgData, CrgOptionsStruct* optionList, double u, double v, double* z) {
  size_t indexU = 0;
  size_t indexV = 0;
  int calcIndex = 1;
  int inCoreAreaU = 1; /* indicates u position is in or out of the core area */
  int inCoreAreaV = 1; /* indicates u position is in or out of the core area */
  int calcValue = 1;
  int calcBank = 1;
  int calcSmoothBase = 0;
  int calcSmooth = 0;
  double fracU = 0.0;
  double fracV = 0.0;
  double z00;
  double z01;
  double z10;
  double z11;
  double bank;
  double zOffset = 0.0;
  double smoothScale = 1.0; /* scale from smoothing option        */
  double smoothZone = 0.0;  /* length of smoothing zone           */
  double smoothBase = 0.0;  /* base value against which to smooth */

  int borderModeU = dCrgBorderModeNone;
  int borderModeV = dCrgBorderModeExKeep;

  /* --- compute the fallback solution --- */
  *z = 0.0;

  if (!crgData) return 0;

    /* --- doing performance measurements? --- */
#ifdef dCrgEnableStats
  if (crgData->perfStat.active) crgData->perfStat.noTotalQueries++;
#endif

  /* --- incoming u value might have to be clipped to correct range --- */
  /* --- if a closed reference line is to be used                   --- */

  if (crgData->util.uIsClosed) crgEvalu2uvalid(crgData, optionList, &u);

  /* find u interval in constantly spaced u axis */
  fracU = (u - crgData->channelU.info.first) / crgData->channelU.info.inc;

  /* correct u interval depending on evaluation options */
  if ((u < crgData->channelU.info.first) || (u > crgData->channelU.info.last)) {
#ifdef dCrgEnableStats
    if (crgData->perfStat.active) crgData->perfStat.noCallsBorderU++;
#endif

    /* --- leaving the core area --- */
    inCoreAreaU = 0;

    if (!optionList) return 0;

    if (optionList->entry[dCrgCpOptionBorderModeU].valid)
      borderModeU = optionList->entry[dCrgCpOptionBorderModeU].iValue;

    if (borderModeU == dCrgBorderModeNone) {
      return 0;
    } else if (borderModeU == dCrgBorderModeExKeep) {
      if (fracU < 0.0) {
        fracU = 0.0;
        calcIndex = 0;
        calcSmoothBase = 1; /* --- base value for smoothing needs to be calculated --- */
      } else
        calcSmoothBase = 2; /* --- base value for smoothing needs to be calculated --- */
                            /* --- rest of fracU > 1.0 will be handled automatically by index calculation below --- */
      /* calcBank  = 0; */  /* new policy in rev. 1.0.3 */
    } else if (borderModeU == dCrgBorderModeExZero) {
      /* --- value zero is already initialized, don't perform full calculation --- */
      calcValue = 0;
      calcBank = 0;

      if (fracU < 0.0)
        calcSmoothBase = 1; /* --- base value for smoothing needs to be calculated --- */
      else
        calcSmoothBase = 2; /* --- base value for smoothing needs to be calculated --- */

      /* calcIndex = 0; */ /* new policy in rev. 0.7 */
    } else if (borderModeU == dCrgBorderModeRepeat) {
      double uSize = crgData->channelU.info.last - crgData->channelU.info.first;
      double maxFrac = uSize / crgData->channelU.info.inc;

      fracU = fmod((u - crgData->channelU.info.first) / crgData->channelU.info.inc,
                   maxFrac); /** todo: performance optimization for successive calls */

      if (fracU < 0.0) fracU += maxFrac;

      /* we're back to the core area */
      inCoreAreaU = 1;

      u = crgData->channelU.info.first + fracU * crgData->channelU.info.inc;
    } else if (borderModeU == dCrgBorderModeReflect) {
      double uSize = crgData->channelU.info.last - crgData->channelU.info.first;
      double maxFrac = uSize / crgData->channelU.info.inc;
      int repSeq = (int)((u - crgData->channelU.info.first) / uSize);
      int revert = abs(repSeq) % 2;

      fracU = fabs(fracU);
      fracU -= abs(repSeq) * maxFrac;

      if (revert) fracU = maxFrac - fracU;

      /* we're back to the core area */
      inCoreAreaU = 1;

      u = crgData->channelU.info.first + fracU * crgData->channelU.info.inc;
    }

    /* --- any offset option valid? --- */
    if (optionList)
      zOffset += optionList->entry[dCrgCpOptionBorderOffsetU].valid
                     ? optionList->entry[dCrgCpOptionBorderOffsetU].dValue
                     : 0.0;
  }

  /* --- calculate / correct the u index? --- */
  if (calcIndex) {
    if (fracU < 0.0) fracU = 0.0;

    indexU = (size_t)fracU;

    if (indexU >= crgData->channelU.info.size - 1) {
      indexU = crgData->channelU.info.size - 2;
      fracU = 1.0;
    } else
      fracU -= indexU;
  }

  /* --- reset the index calculation flag --- */
  calcIndex = 1;

  /* find v interval in constantly spaced v axis */
  if (crgData->admin.defMask & dCrgDataDefVIndex) {
    /* find v interval in constantly spaced v range */
    fracV = (v - crgData->channelV.info.first) / crgData->channelV.info.inc;

    /* correct v interval depending on evaluation options */
    if ((v < crgData->channelV.info.first) || (v > crgData->channelV.info.last)) {
      /* --- leaving the core area --- */
      inCoreAreaV = 0;

      if (!optionList) return 0;

      if (optionList->entry[dCrgCpOptionBorderModeV].valid)
        borderModeV = optionList->entry[dCrgCpOptionBorderModeV].iValue;

      if (borderModeV == dCrgBorderModeNone) {
        return 0;
      } else if (borderModeV == dCrgBorderModeExKeep) {
        if (fracV < 0.0) {
          inCoreAreaV = 0;
          fracV = 0.0;
          calcIndex = 0;
        }
        /* --- fracV > 1.0 will be handled automatically by index calculation below --- */
      } else if (borderModeV == dCrgBorderModeExZero) {
        /* --- value zero is already initialized, don't perform full calculation --- */
        calcValue = 0;
        /* calcIndex = 0; */ /* new policy in rev. 0.7 */
      } else if (borderModeV == dCrgBorderModeRepeat) {
        double vSize = crgData->channelV.info.last - crgData->channelV.info.first;
        double maxFrac = vSize / crgData->channelV.info.inc;

        fracV = fmod((v - crgData->channelV.info.first) / crgData->channelV.info.inc,
                     maxFrac); /** @todo: performance optimization for successive calls */

        if (fracV < 0.0) fracV += maxFrac;

        /* --- clamp v to the valid range --- */

        /* we're back to the core area */
        inCoreAreaV = 1;

        v = crgData->channelV.info.first + fracV * crgData->channelV.info.inc;
      } else if (borderModeV == dCrgBorderModeReflect) {
        double vSize = crgData->channelV.info.last - crgData->channelV.info.first;
        double maxFrac = vSize / crgData->channelV.info.inc;
        int repSeq = (int)((v - crgData->channelV.info.first) / vSize);
        int revert = abs(repSeq) % 2;

        fracV = fabs(fracV);
        fracV -= abs(repSeq) * maxFrac; /* was vSize, debugged 29.09.2009 by Marius */

        if (revert) fracV = maxFrac - fracV;

        /* --- clamp v to the valid range --- */

        /* we're back to the core area */
        inCoreAreaV = 1;

        v = crgData->channelV.info.first + fracV * crgData->channelV.info.inc;
      }

      /* --- any offset option valid? --- */
      if (optionList)
        zOffset += optionList->entry[dCrgCpOptionBorderOffsetV].valid
                       ? optionList->entry[dCrgCpOptionBorderOffsetV].dValue
                       : 0.0;
    }

    /* --- calculate / correct the v index? --- */
    if (calcIndex) {
      if (fracV < 0.0) fracV = 0.0;

      indexV = (size_t)fracV;

      if (indexV >= crgData->channelV.info.size - 1) {
        indexV = crgData->channelV.info.size - 2;
        fracV = 1.0;
      } else
        fracV -= indexV;
    }
  } else
  /* find v interval in variably spaced v axis */
  {
    size_t indexCtr;
    size_t index0 = crgData->channelV.info.size - 1;
    double vPos = v;

    indexV = 0;

    /* --- is a border mode active? --- */
    if ((vPos < crgData->channelV.info.first) || (vPos > crgData->channelV.info.last)) {
      /* --- leaving the core area --- */
      inCoreAreaV = 0;

      borderModeV = dCrgBorderModeExKeep;

#ifdef dCrgEnableStats
      if (crgData->perfStat.active) crgData->perfStat.noCallsBorderV++;
#endif

      /* --- compensate for numeric inaccuracies at the original borders --- */
      if (fabs(vPos - crgData->channelV.info.first) < dMaxBorderError)
        vPos = crgData->channelV.info.first;
      else if (fabs(vPos - crgData->channelV.info.last) < dMaxBorderError)
        vPos = crgData->channelV.info.last;
      else {
        if (!optionList) return 0;

        if ((optionList->entry[dCrgCpOptionBorderModeV].valid))
          borderModeV = optionList->entry[dCrgCpOptionBorderModeV].iValue;

        if (borderModeV == dCrgBorderModeNone) return 0;

        if (borderModeV == dCrgBorderModeExZero) {
          /* --- value zero is already initialized, don't perform full calculation --- */
          calcValue = 0;
        }
      }

      if (borderModeV == dCrgBorderModeRepeat) {
        double vRange = crgData->channelV.info.last - crgData->channelV.info.first;

        if (vPos > crgData->channelV.info.last)
          vPos = crgData->channelV.info.first + fmod((vPos - crgData->channelV.info.last), vRange);
        else if (vPos < crgData->channelV.info.first)
          vPos = crgData->channelV.info.last + fmod((vPos - crgData->channelV.info.first), vRange);

        /* we're back to the core area */
        inCoreAreaV = 1;

        /* --- again: correct for numeric inaccuracies at the border --- */
        if (fabs(vPos - crgData->channelV.info.first) < dMaxBorderError)
          vPos = crgData->channelV.info.first;
        else if (fabs(vPos - crgData->channelV.info.last) < dMaxBorderError)
          vPos = crgData->channelV.info.last;
      } else if (borderModeV == dCrgBorderModeReflect) {
        double vRange = crgData->channelV.info.last - crgData->channelV.info.first;
        int repSeq = 0;
        int revert = 0;
        double remainder = 0.0;

        if (vPos > crgData->channelV.info.last) {
          remainder = fmod((vPos - crgData->channelV.info.last), vRange);
          repSeq = (int)((vPos - crgData->channelV.info.last) / vRange);
          revert = !(abs(repSeq) % 2);

          if (revert)
            vPos = crgData->channelV.info.last - remainder;
          else
            vPos = crgData->channelV.info.first + remainder;
        } else if (vPos < crgData->channelV.info.first) {
          remainder = fmod((vPos - crgData->channelV.info.first), vRange);
          repSeq = (int)((vPos - crgData->channelV.info.first) / vRange);
          revert = abs(repSeq) % 2;

          if (revert)
            vPos = crgData->channelV.info.last + remainder;
          else
            vPos = crgData->channelV.info.first - remainder;
        }

        /* we're back to the core area */
        inCoreAreaV = 1;
      }

      /* --- any offset option valid? --- */
      if (optionList)
        zOffset += optionList->entry[dCrgCpOptionBorderOffsetV].valid
                       ? optionList->entry[dCrgCpOptionBorderOffsetV].dValue
                       : 0.0;
    }

    if (calcIndex) {
      /* --- make a better first guess for the v index based on a pre-computed index table --- */
      if (crgData->indexTableV.valid) {
        size_t lookUpIdx = 0;
        if (vPos > crgData->indexTableV.minVal)
          lookUpIdx = (size_t)((vPos - crgData->indexTableV.minVal) / crgData->indexTableV.range *
                               (crgData->indexTableV.size - 1));
        if (lookUpIdx > (crgData->indexTableV.size - 1)) lookUpIdx = crgData->indexTableV.size - 1;

        indexCtr = crgData->indexTableV.refIdx[lookUpIdx];

        /* --- round-off error? --- */
        if (crgData->channelV.data[indexCtr] <= vPos) indexCtr++;

        if (indexCtr > crgData->indexTableV.size - 1) indexCtr = crgData->indexTableV.size - 1;

        if (lookUpIdx > 0) {
          indexV = crgData->indexTableV.refIdx[lookUpIdx - 1];

          if ((indexV >= indexCtr) && (indexV > 0)) indexV--;
        }

        if (lookUpIdx < crgData->indexTableV.size - 1) {
          index0 = crgData->indexTableV.refIdx[lookUpIdx + 1];

          if ((index0 <= indexCtr) && (index0 < crgData->channelV.info.size - 1)) index0++;
        }
      }

      while (1) {
#ifdef dCrgEnableStats
        if (crgData->perfStat.active) crgData->perfStat.noCallsLoopV1++;
#endif

        indexCtr = (index0 + indexV) / 2;

        if (indexCtr <= indexV) break;

        if (vPos < crgData->channelV.data[indexCtr])
          index0 = indexCtr;
        else
          indexV = indexCtr;
      }
      fracV = (vPos - crgData->channelV.data[indexV]) /
              (crgData->channelV.data[indexV + 1] - crgData->channelV.data[indexV]);

      /* correct v interval depending on evaluation options */
      if (fracV > 1.0)
        fracV = 1.0;
      else if (fracV < 0.0)
        fracV = 0.0;
    }
  }

  if (calcValue) {
    /* evaluate z(u, v) by bilinear interpolation */
    z00 = crgData->channelZ[indexV].data[indexU];
    z10 = crgData->channelZ[indexV].data[indexU + 1] - z00;
    z01 = crgData->channelZ[indexV + 1].data[indexU];
    z11 = crgData->channelZ[indexV + 1].data[indexU + 1] - (z10 + z01);
    z01 -= z00;

    *z = (z11 * fracV + z10) * fracU + z01 * fracV + z00;

    /* add mean value which was subtracted during normalization of channel values */
    *z += crgData->channelZ[indexV].info.mean;
  }

  /* --- is a transition (smooth) option set? --- */
  if (optionList) {
    if (optionList->entry[dCrgCpOptionSmoothUBegin].valid || optionList->entry[dCrgCpOptionSmoothUEnd].valid) {
      if (inCoreAreaU || calcSmoothBase) {
        /* NOTE: smoothZone cannot be 0.0; this is checked when setting the option */
        if (optionList->entry[dCrgCpOptionSmoothUBegin].valid) {
          smoothZone = optionList->entry[dCrgCpOptionSmoothUBegin].dValue;

          if ((u - crgData->channelU.info.first) <= smoothZone) {
            if (u < crgData->channelU.info.first)
              smoothScale = 0.0;
            else
              smoothScale = (u - crgData->channelU.info.first) / smoothZone;
            calcSmoothBase = 1;
            calcSmooth = 1;
          }
        }
        if (optionList->entry[dCrgCpOptionSmoothUEnd].valid) {
          smoothZone = optionList->entry[dCrgCpOptionSmoothUEnd].dValue;

          if ((crgData->channelU.info.last - u) <= smoothZone) {
            if (u > crgData->channelU.info.last)
              smoothScale = 0.0;
            else
              smoothScale = (crgData->channelU.info.last - u) / smoothZone;
            calcSmoothBase = 2;
            calcSmooth = 1;
          }
        }
      }

      /* take smoothing base value into account? */
      if (calcSmoothBase) {
        if (calcSmoothBase == 1) {
          if (crgData->channelRefZ.info.valid)
            smoothBase += crgData->channelRefZ.data[0];
          else
            smoothBase += crgData->channelRefZ.info.first;
        } else if (calcSmoothBase == 2) {
          if (crgData->channelRefZ.info.valid)
            smoothBase += crgData->channelRefZ.data[crgData->channelRefZ.info.size - 1];
          else
            smoothBase += crgData->channelRefZ.info.last;
        }
      }
    }
  }

  /* --- clip frac U to maximum valid range --- */
  if (fracU < 0.0)
    fracU = 0.0;
  else if (fracU > 1.0)
    fracU = 1.0;

  /* --- add slope, banking, offsets and smoothing --- */
  /* add z displacement from reference line z data */
  if (crgData->channelRefZ.info.valid)
    *z += crgData->channelRefZ.data[indexU] +
          fracU * (crgData->channelRefZ.data[indexU + 1] - crgData->channelRefZ.data[indexU]);
  else
    *z += crgData->channelRefZ.info.first;

  /* add z displacement from banking */
  if (crgData->util.hasBank && calcBank) {
    if (crgData->channelBank.info.valid)
      bank = crgData->channelBank.data[indexU] +
             fracU * (crgData->channelBank.data[indexU + 1] - crgData->channelBank.data[indexU]);
    else
      bank = crgData->channelBank.info.first;

    /* --- clip v for banking to vmin/vmax --- */
    if (v < crgData->channelV.info.first)
      v = crgData->channelV.info.first;
    else if (v > crgData->channelV.info.last)
      v = crgData->channelV.info.last;

    *z += bank * v;
  }

  /* add any additional offset caused by options etc. */
  if (!inCoreAreaU) {
    if (borderModeU == dCrgBorderModeExZero)
      *z = zOffset;
    else if (borderModeU == dCrgBorderModeExKeep)
      *z += zOffset;

  } else if (!inCoreAreaV) {
    if (borderModeV == dCrgBorderModeExZero)
      *z = zOffset;
    else if (borderModeV == dCrgBorderModeExKeep)
      *z += zOffset;
  }

  if (calcSmooth) {
    /* overall scale may be influenced by the smoothing zone */
    *z = smoothBase + (*z - smoothBase) * smoothScale;
  }

  return 1;
}

int crgEvalxy2z(int cpId, double x, double y, double* z) {
  double u;
  double v;
  CrgContactPointStruct* cp = NULL;

#ifdef dCrgEnableDebug2
  if (crgIsNan(&x) || crgIsNan(&y) || x != x || y != y) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgEvalxy2z: got NaN for x and/or y position. Refusing evaluation.\n");
    return 0;
  } else {
    crgMsgPrint(dCrgMsgLevelNotice, "crgEvalxy2z: cpId = %d, x = %.6f, y = %.6f\n", cpId, x, y);
  }
#endif

  if (!(cp = crgContactPointGetFromId(cpId))) return 0;

  if (!crgEvalxy2uvPtr(cp, x, y, &u, &v)) return 0;

  return crgEvaluv2zPtr(cp, u, v, z);
}

int crgDataEvalu2Refz(CrgDataStruct* crgData, double u, double* z) {
  double fracU;
  size_t index = 0;

  if (!crgData) return 0;

  /* --- set fallback solution --- */
  *z = 0.0;

  /* --- get elevation on reference line --- */
  if (crgData->channelRefZ.info.valid) {
    if (u < crgData->channelU.info.first)
      u = crgData->channelU.info.first;
    else if (u > crgData->channelU.info.last)
      u = crgData->channelU.info.last;

    fracU = (u - crgData->channelU.info.first) / crgData->channelU.info.inc;

    if (fracU >= 0.) index = (size_t)fracU;

    if (index >= crgData->channelRefZ.info.size - 1) index = crgData->channelRefZ.info.size - 2;

    fracU -= index;

    *z = crgData->channelRefZ.data[index] +
         fracU * (crgData->channelRefZ.data[index + 1] - crgData->channelRefZ.data[index]);
  }

  return 1;
}
