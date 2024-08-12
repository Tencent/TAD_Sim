/* ===================================================
 *  file:       crgEvalxy2uv.c
 * ---------------------------------------------------
 *  purpose:	convert an x/y position into a u/v
 *              position
 * ---------------------------------------------------
 *  based on routines by Dr. Jochen Rauh, Daimler AG
 * ---------------------------------------------------
 *  first edit:	18.11.2008 by M. Dupuis @ VIRES GmbH
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
#include <string.h>
#include "crgBaseLibPrivate.h"

/* ====== DEFINITIONS ====== */

/* ====== TYPE DEFINITIONS ====== */

/* ====== LOCAL METHODS ====== */

/* ====== IMPLEMENTATION ====== */

int crgEvalxy2uv(int cpId, double x, double y, double* u, double* v) {
  CrgContactPointStruct* cp;

  if (!(cp = crgContactPointGetFromId(cpId))) return 0;

  return crgEvalxy2uvPtr(cp, x, y, u, v);
}

int crgEvalxy2uvPtr(CrgContactPointStruct* cp, double x, double y, double* u, double* v) {
  size_t indexMin = 0;
  int useHist = 0;
  CrgDataStruct* crgData;
  double x0;
  double x1;
  double x2;
  double x3;
  double y0;
  double y1;
  double y2;
  double y3;
  double xxx1;
  double xxx2;
  double x2xx;
  double x2x0;
  double x2x1;
  double x3x1;
  double yyy1;
  double yyy2;
  double y2y0;
  double y2y1;
  double y3y1;
  double y2yy;
  double dProd;
  double t_dProd;
  double ta;
  double tb;
  double du;
  double dist2Min = 0;
  size_t indexP1;
  size_t i;
  int j;
  size_t lastIdx;

  if (!cp) return 0;

  /* --- remember the input and compute the fallback solution --- */
  cp->x = x;
  cp->y = y;
  cp->u = cp->x;
  cp->v = cp->y;
  *u = cp->u;
  *v = cp->v;

  crgData = cp->crgData;

  if (!crgData)
    return 1;
  else if (!(crgData->channelX.info.valid))
    return 1;

  /* --- check for the information in the history  --- */
  /* --- look for search start interval in history --- */

  for (j = 0; j < cp->history.usedSize; j++) {
    double dist2;
    double dx;
    double dy;

#ifdef dCrgEnableStats
    if (cp->history.stat.active) cp->history.stat.noIter++;
#endif

    dx = cp->x - cp->history.entry[j].x;
    dy = cp->y - cp->history.entry[j].y;

    dist2 = dx * dx + dy * dy;

    /* ---  first choice: closer than 10*DUINC to history points (fast) --- */
    if (dist2 < cp->history.closeDist) {
      useHist = 1;
      indexMin = cp->history.entry[j].index;

#ifdef dCrgEnableStats
      if (cp->history.stat.active) cp->history.stat.noCloseHits++;
#endif
      break;
    }
    /* --- second choice: find closest point in history which is not too far away (still fairly fast) --- */
    else if (dist2 < cp->history.farDist) {
      if (!useHist || (dist2 < dist2Min)) {
        dist2Min = dist2;
        indexMin = cp->history.entry[j].index;
        useHist = 1;

#ifdef dCrgEnableStats
        if (cp->history.stat.active) cp->history.stat.noFarHits++;
#endif

        /* --- code runs faster if using first fairly good point instead of waiting for point within closeDist --- */
        /* --- therefore: stop search and go ahead immediately                                                 --- */
        break;
      }
    }
  }

  /* --- did not find close enough point in history               --- */
  /* --- third choice: find globally closest reference line point --- */
  if (!useHist) {
    i = 0;

    while (i < crgData->channelX.info.size) {
      double dx = cp->x - crgData->channelX.data[i];
      double dy = cp->y - crgData->channelY.data[i];

      double dist2 = dx * dx + dy * dy;

      if ((dist2 < dist2Min) || !i) {
        indexMin = i;
        dist2Min = dist2;
      }

      /* --- make sure last point of a closed reference line is tested --- */
      if (crgData->util.uIsClosed) {
        if (i < crgData->channelX.info.size - 10)
          i += 10;
        else if (i < crgData->channelX.info.size - 1)
          i = crgData->channelX.info.size - 1;
        else
          break;
      } else if (i < crgData->channelX.info.size - 10)
        i += 10;
      else if (i < crgData->channelX.info.size - 1)
        i = crgData->channelX.info.size - 1;
      else
        break;
    }
#ifdef dCrgEnableStats
    if (cp->history.stat.active) cp->history.stat.noNoHits++;
#endif
  }

#ifdef dCrgEnableStats
  if (cp->history.stat.active) cp->history.stat.noTotalQueries++;
#endif

  /* -- found the start? --- */
  if (indexMin < 1) indexMin = 1;

  /*  search for relevant reference line interval iu
   *  looking at some points
   *  P0: iu0 = iu - 2: (X0, Y0)
   *  P1: iu1 = iu - 1: (X1, Y1)
   *  P2: iu2 = iu    : (X2, Y2)
   *  P3: iu3 = iu + 1: (X3, Y3)
   *  P :               (X , Y ) (current input from subroutine call)
   */
  t_dProd = 0.0;

  for (;;) {
    indexP1 = (indexMin + 1 < crgData->channelX.info.size) ? indexMin + 1 : crgData->channelX.info.size - 1;

    /*
     *  evaluate dot product (P -P2).(P3-P1):
     *  - unnormalized projection of (P -P2) on (P3-P1)
     *  - hd is negative for P "left"  of normal on P3-P1 through P2
     *  - hd is positive for P "right" of normal on P3-P1 through P2
     */
    xxx2 = cp->x - crgData->channelX.data[indexMin];
    x3x1 = crgData->channelX.data[indexP1] - crgData->channelX.data[indexMin - 1];
    yyy2 = cp->y - crgData->channelY.data[indexMin];
    y3y1 = crgData->channelY.data[indexP1] - crgData->channelY.data[indexMin - 1];

    dProd = xxx2 * x3x1 + yyy2 * y3y1;

    /*
     *  look if P belongs to current interval iu:
     *   update interval upwards as long as necessary/possible
     *   to make hd negative
     */

#ifdef dCrgEnableStats
    if (cp->history.stat.active) cp->history.stat.noCallsLoop1++;
#endif

    if (dProd > 0.0) {
      if (indexMin < (crgData->channelX.info.size - 1))
        indexMin++;
      else if (crgData->util.uIsClosed) {
        t_dProd = (cp->x - crgData->channelX.data[0]) * (crgData->channelX.data[1] - crgData->channelX.data[0]) +
                  (cp->y - crgData->channelY.data[0]) * (crgData->channelY.data[1] - crgData->channelY.data[0]);

        if (t_dProd <= 0.0) break; /* crgData->channelU.data[0] may be closer */

        indexMin = 1;
      } else
        break;
    } else
      break;
  }

  /*
   *  at this point we have gone to the last interval or
   *  at least one interval too far in upwards direction
   *  so let's correct that
   */

  for (;;) {
    size_t indexM2 = 0;

    if (indexMin > 1) indexM2 = indexMin - 2;

    x0 = crgData->channelX.data[indexM2];
    x1 = crgData->channelX.data[indexMin - 1];
    x2 = crgData->channelX.data[indexMin];

    y0 = crgData->channelY.data[indexM2];
    y1 = crgData->channelY.data[indexMin - 1];
    y2 = crgData->channelY.data[indexMin];

    /*
     *  evaluate dot product (P -P1).(P2- P0)
     *  - unnormalized projection of (P -P1) on (P2-P0)
     *  - hd is negative for P "left"  of normal on P2-P0 through P1
     *  - hd is positive for P "right" of normal on P2-P0 through P1
     */
    xxx1 = cp->x - x1;
    x2x0 = x2 - x0;
    yyy1 = cp->y - y1;
    y2y0 = y2 - y0;

    dProd = xxx1 * x2x0 + yyy1 * y2y0;

    /*
     *  look if P belongs to current interval:
     *   update interval downwards as long as necessary/possible
     *   to make hd positive
     */

#ifdef dCrgEnableStats
    if (cp->history.stat.active) cp->history.stat.noCallsLoop2++;
#endif

    if (dProd < 0.0) {
      if (indexMin > 1)
        indexMin--;
      else if (crgData->util.uIsClosed) {
        if (t_dProd != 0.0) break; /* last reference point already checked */

        lastIdx = crgData->channelX.info.size - 1;
        t_dProd = (cp->x - crgData->channelX.data[lastIdx]) *
                      (crgData->channelX.data[lastIdx] - crgData->channelX.data[lastIdx - 1]) +
                  (cp->y - crgData->channelY.data[lastIdx]) *
                      (crgData->channelY.data[lastIdx] - crgData->channelY.data[lastIdx - 1]);

        if (t_dProd >= 0.0) break; /* crgData->channelU.data[end] may be closer */

        indexMin = lastIdx;
      } else
        break;
    } else
      break;
  }

  /*
   * at this point we have got the right interval.
   * evaluate v as distance between P and line trough P2-P1
   * by calculating the normalized cross product
   * (P2-P1)x(P-P1) / |P2-P1|
   */
  x2x1 = x2 - x1;
  y2y1 = y2 - y1;

  cp->v = (x2x1 * yyy1 - y2y1 * xxx1) / sqrt(x2x1 * x2x1 + y2y1 * y2y1);

  /*
   * here we could check distance related to curvature:
   *  warn  if not closer than max/min curvature
   *  abort if not closer than current curvature
   *?
   *
   * now we need a line parallel to P2-P1 through P,
   * cut this line with the normal on P2-P0 through P1 in A
   * cut this line with the normal on P3-P1 through P2 in B
   * evaluate
   *  du = |P-A|/|B-A|*|P2-P1|
   * which is (after some vector calculus) identical to
   *  du = ta/(ta+tb) * |P2-P1| with
   *  ta = ((P2-P0).(P -P1))/((P2-P0).(P2-P1))
   *  tb = ((P3-P1).(P2-P ))/((P3-P1).(P2-P1))
   */

  indexP1 = indexMin + 1;

  if (indexP1 >= crgData->channelX.info.size) indexP1 = crgData->channelX.info.size - 1;

  x3 = crgData->channelX.data[indexP1];
  y3 = crgData->channelY.data[indexP1];

  x3x1 = x3 - x1;
  y3y1 = y3 - y1;

  x2xx = x2 - cp->x;
  y2yy = y2 - cp->y;

  ta = dProd / (x2x0 * x2x1 + y2y0 * y2y1);
  tb = (x3x1 * x2xx + y3y1 * y2yy) / (x3x1 * x2x1 + y3y1 * y2y1);
  du = ta / (ta + tb) * crgData->channelU.info.inc;

  cp->u = (indexMin - 1) * crgData->channelU.info.inc + du + crgData->channelU.info.first;

  /* --- test again if point is closest to begin or end of reference line    --- */
  /* --- and reference line is closed, then check whether point             --- */
  /* --- is within extended section of reference line and correct           --- */
  /* --- co-ordinates as required                                           --- */
  if (crgOptionHasValueInt(&(cp->options), dCrgRefLineCloseTrack, 1) && crgData->util.uIsClosed) {
    if (cp->u > crgData->util.uCloseMax) {
      double dx1 = cp->x - crgData->channelX.info.first;
      double dy1 = cp->y - crgData->channelY.info.first;

      cp->u = crgData->channelU.info.first + dx1 * crgData->util.phiFirstCos + dy1 * crgData->util.phiFirstSin;
      cp->v = +dy1 * crgData->util.phiFirstCos - dx1 * crgData->util.phiFirstSin;
    } else if (cp->u < crgData->util.uCloseMin) {
      double dx1 = cp->x - crgData->channelX.info.last;
      double dy1 = cp->y - crgData->channelY.info.last;

      cp->u = crgData->channelU.info.last + dx1 * crgData->util.phiLastCos + dy1 * crgData->util.phiLastSin;
      cp->v = +dy1 * crgData->util.phiLastCos - dx1 * crgData->util.phiLastSin;
    }
  } else {
    if (cp->u < crgData->channelU.info.first) {
      double dx1 = cp->x - crgData->channelX.info.first;
      double dy1 = cp->y - crgData->channelY.info.first;

      cp->u = crgData->channelU.info.first + dx1 * crgData->util.phiFirstCos + dy1 * crgData->util.phiFirstSin;
      cp->v = +dy1 * crgData->util.phiFirstCos - dx1 * crgData->util.phiFirstSin;
    } else if (cp->u > crgData->channelU.info.last) {
      double dx1 = cp->x - crgData->channelX.info.last;
      double dy1 = cp->y - crgData->channelY.info.last;

      cp->u = crgData->channelU.info.last + dx1 * crgData->util.phiLastCos + dy1 * crgData->util.phiLastSin;
      cp->v = +dy1 * crgData->util.phiLastCos - dx1 * crgData->util.phiLastSin;
    }
  }

  /* --- remember result in history --- */
  if (cp->history.totalSize > 1) {
    /* --- avoid registering twice for the same index --- */
    if (cp->history.entry[0].index != indexP1) {
      memmove(&(cp->history.entry[1]), cp->history.entry, (cp->history.totalSize - 1) * cp->history.entrySize);

      if (cp->history.usedSize < cp->history.totalSize) cp->history.usedSize++;
    }

    cp->history.entry[0].x = cp->x;
    cp->history.entry[0].y = cp->y;
    cp->history.entry[0].index = indexP1;
  }

  *u = cp->u;
  *v = cp->v;

  return 1;
}

int crgEvalu2uvalid(CrgDataStruct* crgData, CrgOptionsStruct* optionList, double* u) {
  if (!crgData || !optionList || !u) return -1;

  if (!(crgData->util.uIsClosed)) return 0;

  if (*u < crgData->util.uCloseMin || *u > crgData->util.uCloseMax) {
    if (crgOptionHasValueInt(optionList, dCrgCpOptionRefLineContinue, dCrgRefLineCloseTrack)) {
      /* --- clip incoming u value to the correct range --- */
      *u = fmod(*u - crgData->util.uCloseMin, crgData->util.uCloseMax - crgData->util.uCloseMin);

      if (*u > 0.0)
        *u += crgData->util.uCloseMin;
      else
        *u += crgData->util.uCloseMax;

      return 1;
    }
  }
  return 0;
}
