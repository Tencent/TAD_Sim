/* ===================================================
 *  file:       crgEvalpk.c
 * ---------------------------------------------------
 *  purpose:	compute heading and curvature at a
 *              given position
 * ---------------------------------------------------
 *  based on routines by Dr. Jochen Rauh, Daimler AG
 * ---------------------------------------------------
 *  first edit:	27.11.2008 by M. Dupuis @ VIRES GmbH
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
#include "crgBaseLibPrivate.h"

/* ====== DEFINITIONS ====== */

/* ====== TYPE DEFINITIONS ====== */

/* ====== LOCAL METHODS ====== */

/* ====== IMPLEMENTATION ====== */
int crgEvaluv2pk(int cpId, double u, double v, double* phi, double* curv) {
  CrgContactPointStruct* cp;
  int retVal = 0;

  if (!(cp = crgContactPointGetFromId(cpId))) return 0;

  /* --- compute the fallback solution --- */
  cp->u = u;
  cp->v = v;
  cp->phi = 0.0;
  cp->curv = 0.0;

  retVal = crgDataEvaluv2pk(cp->crgData, &(cp->options), cp->u, cp->v, &(cp->phi), &(cp->curv));

  /* --- transfer the result --- */
  *phi = cp->phi;
  *curv = cp->curv;

  return retVal;
}

int crgEvalxy2pk(int cpId, double x, double y, double* phi, double* curv) {
  double u;
  double v;

  if (!crgEvalxy2uv(cpId, x, y, &u, &v)) return 0;

  return crgEvaluv2pk(cpId, u, v, phi, curv);
}

int crgDataEvaluv2pk(CrgDataStruct* crgData, CrgOptionsStruct* optionList, double u, double v, double* phi,
                     double* curv) {
  size_t indexU = 0;
  size_t nU;
  double fracU;

  /* --- compute the fallback solution --- */
  *phi = 0.0;
  *curv = 0.0;

  if (!crgData) return 1;

  /* --- center line available as x/y data? --- */
  if (!crgData->channelX.info.size) return 1;

  /* --- incoming u value might have to be clipped to correct range --- */
  /* --- if a closed reference line is to be used                   --- */
  if (crgData->util.uIsClosed) crgEvalu2uvalid(crgData, optionList, &u);

  /* find u interval in constantly spaced u axis */
  fracU = (u - crgData->channelU.info.first) / crgData->channelU.info.inc;

  if (fracU < 0.0)
    indexU = 0;
  else {
    indexU = (size_t)fracU;

    /* data dimension is at least 2x2 */
    if (indexU >= crgData->channelU.info.size - 1) indexU = crgData->channelU.info.size - 2;
  }

  fracU -= indexU;

  /* get curvature on road sections of 0.5m length - if possible */
  nU = (size_t)(0.5 / crgData->channelU.info.inc);

  if (nU < 1) nU = 1;

  if (nU > indexU) { /* evaluation before start of road -> return phi(0) and k=0 */
    *phi = crgData->channelPhi.info.first;
    *curv = 0.0;
  } else if (indexU + nU >=
             crgData->channelPhi.info.size) { /* evaluation behind end of road -> return phi(end) and k=0 */
    *phi = crgData->channelPhi.info.last;
    *curv = 0.0;
  } else {
    /*
     *     calculate curvature by using cross product of two
     *     consecutive road sections defined by three points
     *     P0: iu0 = iu - nu: (X0, Y0)
     *     P1: iu1 = iu     : (X1, Y1)
     *     P2: iu2 = iu + nu: (X2, Y2)
     *     curv = dphi/ds = (P1-P0)x(P2-P1) / |P1-P0|**3
     */
    double hd = 1.0 / pow(crgData->channelU.info.inc * nU, 3.0);
    double dx0 = crgData->channelX.data[indexU] - crgData->channelX.data[indexU - nU];
    double dx1 = crgData->channelX.data[indexU + nU] - crgData->channelX.data[indexU];
    double dy0 = crgData->channelY.data[indexU] - crgData->channelY.data[indexU - nU];
    double dy1 = crgData->channelY.data[indexU + nU] - crgData->channelY.data[indexU];

    *curv = (dx0 * dy1 - dy0 * dx1) * hd;

    /* now take v into account if the corresponding option is set */
    if (crgOptionHasValueInt(optionList, dCrgCpOptionCurvMode, dCrgCurvLateral) && fabs(*curv) > 1.0e-10) {
      double radius = 1.0 / (*curv) - v;

      /* --- hitting center of circle describing curve? --- */
      if (fabs(radius) < 1.0e-6)
        *curv = 1.0e6; /* maximum curvature */
      else
        *curv = 1.0 / radius;
    }

    /* calculate phi from actual stored value */
    /* NOTE: phi will not be interpolated, since the data set consists
             of a sequence of straight lines with discrete change of
             direction at the respective end points                   */
    *phi = crgData->channelPhi.data[indexU];
  }

  return 1;
}
