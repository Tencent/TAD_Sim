/* ===================================================
 *  file:       crgStatistics.c
 * ---------------------------------------------------
 *  purpose:	calculate and print statistics info
 *              about a set of CRG ddata
 * ---------------------------------------------------
 *  based on routines by Dr. Jochen Rauh, Daimler AG
 * ---------------------------------------------------
 *  first edit:	10.11.2008 by M. Dupuis @ VIRES GmbH
 *  last mod.:  02.11.2009 by M. Dupuis @ VIRES GmbH
 * ===================================================
    Copyright 2013 VIRES Simulationstechnologie GmbH

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
#include "crgBaseLibPrivate.h"

/* ====== DEFINITIONS ====== */

/* ====== TYPE DEFINITIONS ====== */

/* ====== LOCAL METHODS ====== */

/* ====== LOCAL VARIABLES ====== */

/* ====== GLOBAL VARIABLES ====== */

/* ====== IMPLEMENTATION ====== */
void crgCalcStatistics(CrgDataStruct *crgData) {
  double zMeanF = 0.0;
  double zMeanL = 0.0;
  double zMin = 0.0;
  double zMax = 0.0;
  double dx0;
  double dy0;
  double dx1;
  double dy1;
  double cMax = 0.0;
  double cMin = 0.0;
  double val;
  double curv = 0.0;
  size_t i;
  size_t j;
  size_t maxIndex;
  size_t nPtsF = 0;
  size_t nPtsL = 0;

  if (!crgData) return;

  crgMsgPrint(dCrgMsgLevelNotice, "\n");
  crgMsgPrint(dCrgMsgLevelNotice, "crgCalcStatistics: statistical information about data set:\n");
  crgMsgPrint(dCrgMsgLevelNotice, "    road elevation:\n");

  /* --- mean elevation at start and end of road --- */
  for (i = 0; i < crgData->channelV.info.size; i++) {
    if (crgData->channelZ[i].data) {
      if (!crgIsNanf(&(crgData->channelZ[i].data[0]))) {
        zMeanF += crgData->channelZ[i].data[0];
        nPtsF++;

        if (crgData->channelZ[i].info.size == 0) {
          zMeanL = zMeanF;
          nPtsL++;
        }
      }

      if (crgData->channelZ[i].info.size > 0) {
        if (!crgIsNanf(&(crgData->channelZ[i].data[crgData->channelZ[i].info.size - 1]))) {
          zMeanL += crgData->channelZ[i].data[crgData->channelZ[i].info.size - 1];
          nPtsL++;
        }
      }
    } else
      crgMsgPrint(dCrgMsgLevelWarn, "        v-channel with index %ld has no data!\n", i);
  }

  if (nPtsF) zMeanF /= 1.0 * nPtsF;

  if (nPtsL) zMeanL /= 1.0 * nPtsL;

  /* --- add the mean value which was used for normalizing the data --- */
  zMeanF += crgData->channelZ[0].info.mean;
  zMeanL += crgData->channelZ[0].info.mean;

  crgData->util.zMeanBeg = zMeanF;
  crgData->util.zMeanEnd = zMeanL;

  crgMsgPrint(dCrgMsgLevelNotice, "        mean elevation at start [m]: %10.4f\n", zMeanF);
  crgMsgPrint(dCrgMsgLevelNotice, "        mean elevation at end   [m]: %10.4f\n", zMeanL);

  /* --- minimum and maximum elevation --- */
  zMin = zMeanF;
  zMax = zMin;

  for (i = 0; i < crgData->channelV.info.size; i++) {
    for (j = 0; j < crgData->channelZ[i].info.size; j++) {
      if (!crgIsNanf(&(crgData->channelZ[i].data[j]))) {
        if (crgData->channelZ[i].data[j] < zMin) zMin = crgData->channelZ[i].data[j];
        if (crgData->channelZ[i].data[j] > zMax) zMax = crgData->channelZ[i].data[j];
      }
    }
  }

  /* --- add the mean value which was used for normalizing the data --- */
  zMin += crgData->channelZ[0].info.mean;
  zMax += crgData->channelZ[0].info.mean;

  crgMsgPrint(dCrgMsgLevelNotice, "        min. road elevation     [m]: %10.4f\n", zMin);
  crgMsgPrint(dCrgMsgLevelNotice, "        max. road elevation     [m]: %10.4f\n", zMax);

  crgData->util.zMin = zMin;
  crgData->util.zMax = zMax;

  /* --- reference line information --- */
  crgMsgPrint(dCrgMsgLevelNotice, "    reference line information:\n");
  crgMsgPrint(dCrgMsgLevelNotice, "        min. u      [m]:   %10.4f\n", crgData->channelU.info.first);
  crgMsgPrint(dCrgMsgLevelNotice, "        max. u      [m]:   %10.4f\n", crgData->channelU.info.last);

  if (!crgData->channelX.info.valid) {
    crgMsgPrint(dCrgMsgLevelNotice, "        no reference line data available\n");
    crgMsgPrint(dCrgMsgLevelNotice, "        => reference line is u axis.\n");
  } else {
    crgMsgPrint(dCrgMsgLevelNotice, "        start at x  [m]:   %10.4f\n", crgData->channelX.data[0]);
    crgMsgPrint(dCrgMsgLevelNotice, "        start at y  [m]:   %10.4f\n", crgData->channelY.data[0]);
    dy1 = crgData->channelY.data[1] - crgData->channelY.data[0];
    dx1 = crgData->channelX.data[1] - crgData->channelX.data[0];
    crgMsgPrint(dCrgMsgLevelNotice, "        with phi    [rad]: %10.4f\n", atan2(dy1, dx1));

    maxIndex = crgData->channelX.info.size - 1;
    crgMsgPrint(dCrgMsgLevelNotice, "        end at x    [m]:   %10.4f\n", crgData->channelX.data[maxIndex]);
    crgMsgPrint(dCrgMsgLevelNotice, "        end at y    [m]:   %10.4f\n", crgData->channelY.data[maxIndex]);
    dy1 = crgData->channelY.data[maxIndex] - crgData->channelY.data[maxIndex - 1];
    dx1 = crgData->channelX.data[maxIndex] - crgData->channelX.data[maxIndex - 1];
    crgMsgPrint(dCrgMsgLevelNotice, "        with phi    [rad]: %10.4f\n", atan2(dy1, dx1));
  }

  crgMsgPrint(dCrgMsgLevelNotice, "    v range information:\n");
  crgMsgPrint(dCrgMsgLevelNotice, "        min. v      [m]:   %10.4f\n", crgData->channelV.info.first);
  crgMsgPrint(dCrgMsgLevelNotice, "        max. v      [m]:   %10.4f\n", crgData->channelV.info.last);

  /* --- look at curvature --- */
  val = 1.0 / pow(crgData->channelU.info.inc, 3.0);

  for (i = 1; i < crgData->channelX.info.size - 1; i++) {
    dx0 = crgData->channelX.data[i] - crgData->channelX.data[i - 1];
    dx1 = crgData->channelX.data[i + 1] - crgData->channelX.data[i];
    dy0 = crgData->channelY.data[i] - crgData->channelY.data[i - 1];
    dy1 = crgData->channelY.data[i + 1] - crgData->channelY.data[i];
    curv = (dx0 * dy1 - dy0 * dx1) * val;

    if (curv < cMin) cMin = curv;

    if (curv > cMax) cMax = curv;
  }
  crgMsgPrint(dCrgMsgLevelNotice, "    curvature information at reference line:\n");
  crgMsgPrint(dCrgMsgLevelNotice, "        max right curvature [1/m]:   %10.4f\n", cMin);
  crgMsgPrint(dCrgMsgLevelNotice, "        max left curvature  [1/m]:   %10.4f\n", cMax);

  /* --- look at banking --- */
  if (crgData->channelBank.info.size) {
    crgMsgPrint(dCrgMsgLevelNotice, "    banking information at reference line:\n");
    crgMsgPrint(dCrgMsgLevelNotice, "        start bank [m/m]:   %10.4f\n", crgData->channelBank.data[0]);
    crgMsgPrint(dCrgMsgLevelNotice, "        end bank   [m/m]:   %10.4f\n",
                crgData->channelBank.data[crgData->channelBank.info.size - 1]);
  }

  /* --- look at slope data --- */
  crgMsgPrint(dCrgMsgLevelNotice, "    reference line elevation:\n");
  if (crgData->channelRefZ.info.size) {
    crgMsgPrint(dCrgMsgLevelNotice, "        start elevation [m]:   %10.4f\n", crgData->channelRefZ.data[0]);
    crgMsgPrint(dCrgMsgLevelNotice, "        end elevation   [m]:   %10.4f\n",
                crgData->channelRefZ.data[crgData->channelRefZ.info.size - 1]);
  } else {
    crgMsgPrint(dCrgMsgLevelNotice, "        start elevation [m]:   %10.4f\n", 0.0);
    crgMsgPrint(dCrgMsgLevelNotice, "        end elevation   [m]:   %10.4f\n", 0.0);
  }

  crgMsgPrint(dCrgMsgLevelNotice, "crgCalcStatistics: end of statistical information\n");
  crgMsgPrint(dCrgMsgLevelNotice, "\n");
}

void crgPrintElevData(CrgDataStruct *crgData) {
  size_t i;
  size_t j;

  if (!crgData) return;

  crgMsgPrint(dCrgMsgLevelNotice, "\n");
  crgMsgPrint(dCrgMsgLevelNotice, "crgPrintElevData: elevation data contained in data set:\n");

  for (i = 0; i < crgData->channelV.info.size; i++) {
    fprintf(stderr, "channel %ld at offset %.3f\n", (unsigned long)i, crgData->channelV.data[i]);
    for (j = 0; j < crgData->channelZ[i].info.size; j++) fprintf(stderr, "%10.5f ", crgData->channelZ[i].data[j]);
    fprintf(stderr, "\n");
  }
}

void crgCalcUtilityData(CrgDataStruct *crgData) {
  double a1;
  double a2;
  double b1;
  double b2;
  double m1;
  double m2;
  double n1;
  double n2;
  double k;
  double l;
  double divisor;
  int valid = 0;

  if (!crgData) return;

  /* --- compute sine and cosine of directoin at either end of reference line */
  crgData->util.phiFirstSin = sin(crgData->channelPhi.info.first);
  crgData->util.phiFirstCos = cos(crgData->channelPhi.info.first);
  crgData->util.phiLastSin = sin(crgData->channelPhi.info.last);
  crgData->util.phiLastCos = cos(crgData->channelPhi.info.last);

  /* --- reset the "closed" feature --- */
  crgData->util.uIsClosed = 0;

  /* --- calculate intersection point between extrapolated directions of start and end point --- */
  /* line extending before begin of data (x,y) = (a1,a2) + k * (m1,m2) */
  a1 = crgData->channelX.info.first;
  a2 = crgData->channelY.info.first;

  m1 = crgData->util.phiFirstCos;
  m2 = crgData->util.phiFirstSin;

  /* line extending after end of data (x,y) = (b1,b2) + l * (n1,n2) */
  b1 = crgData->channelX.info.last;
  b2 = crgData->channelY.info.last;

  n1 = crgData->util.phiLastCos;
  n2 = crgData->util.phiLastSin;

  /* compute cosine of angle between lines */
  divisor = m1 * n1 + m2 * n2;

  /* --- rule: angle between the extensions of begin and start --- */
  /* --- must not be larger than 60 deg                        --- */
  if (divisor > 0.5) {
    /* prevent overlap of data areas */

    /* test 1: line perpendicular to begin of data set must intersect with */
    /*         extension on end of data set within valid area              */
    l = ((a2 - b2) * m2 + (a1 - b1) * m1) / divisor;
    valid = l > 0.0;

    if (valid) {
      /* test 2: line perpendicular to end of data set must intersect with */
      /*         extension on begin of data set within valid area          */
      k = ((b2 - a2) * n2 + (b1 - a1) * n1) / divisor;
      valid = k < 0.0;
    }

    if (valid) {
      /* --- lines are almost parallel; also: fallback solution --- */
      /* --- compute min/max U on closed center line --- */
      crgData->util.uCloseMin = crgData->channelU.info.first + 0.5 * k * divisor;
      crgData->util.uCloseMax = crgData->channelU.info.last + 0.5 * l * divisor;
      crgData->util.uIsClosed = 1;

      /* --- now check for valid intersection of lines, i.e. not parallel --- */
      divisor = m1 * n2 - m2 * n1;

      /* --- do lines intersect at all or are they parallel? --- */
      if (fabs(divisor) > dCrgParallelLimit) {
        l = (m1 * (a2 - b2) + m2 * (b1 - a1)) / divisor;

        /* --- intersection on correct side of end point? --- */
        if (l >= 0.0) {
          k = (n1 * (a2 - b2) + n2 * (b1 - a1)) / divisor;

          /* --- intersection on correct side of start point? --- */
          if (k <= 0.0) {
            /* --- compute min/max U on closed center line --- */
            crgData->util.uCloseMin = crgData->channelU.info.first + k;
            crgData->util.uCloseMax = crgData->channelU.info.last + l;
          }
        }
      } else
        crgMsgPrint(dCrgMsgLevelNotice, "crgCalcUtilityData: lines are (almost) parallel\n");
    }
  }

  if (crgData->util.uIsClosed) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgCalcUtilityData: reference line may be closed:\n");
    crgMsgPrint(dCrgMsgLevelDebug, "crgCalcUtilityData: xFirst = %.3f, yFirst = %.3f\n", crgData->channelX.info.first,
                crgData->channelY.info.first);
    crgMsgPrint(dCrgMsgLevelDebug, "crgCalcUtilityData: xLast  = %.3f, yLast = %.3f\n", crgData->channelX.info.last,
                crgData->channelY.info.last);
    crgMsgPrint(dCrgMsgLevelDebug, "crgCalcUtilityData: phiFirst = %.3f, phiLast = %.3f\n",
                crgData->channelPhi.info.first, crgData->channelPhi.info.last);
    crgMsgPrint(dCrgMsgLevelNotice, "crgCalcUtilityData: uMin = %.3f, uMinClosed = %.3f\n",
                crgData->channelU.info.first, crgData->util.uCloseMin);
    crgMsgPrint(dCrgMsgLevelNotice, "crgCalcUtilityData: uMax = %.3f, uMaxClosed = %.3f\n", crgData->channelU.info.last,
                crgData->util.uCloseMax);
  } else
    crgMsgPrint(dCrgMsgLevelNotice, "crgCalcUtilityData: reference line may not be closed.\n");

  /* --- calculate a v-index table for faster data access --- */
  crgDataSetBuildVTable(crgData->admin.id);

  crgMsgPrint(dCrgMsgLevelDebug, "crgCalcUtilityData: created hash table for v position.\n");
}
