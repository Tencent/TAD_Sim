/* ===================================================
 *  file:       crgEvaluv2xy.c
 * ---------------------------------------------------
 *  purpose:	convert a u/v position into an x/y
 *              position
 * ---------------------------------------------------
 *  based on routines by Dr. Jochen Rauh, Daimler AG
 * ---------------------------------------------------
 *  first edit:	18.11.2008 by M. Dupuis @ VIRES GmbH
 *  last mod.:  12.01.2016 by H. Helmich @ VIRES GmbH
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
#include "crgBaseLibPrivate.h"

/* ====== DEFINITIONS ====== */

/* ====== TYPE DEFINITIONS ====== */

/* ====== LOCAL METHODS ====== */
/**
 * normlize a 2d vector
 * @param vec    pointer to the co-ordinate array
 */
static void normalizeVector2(double* vec);

/* ====== IMPLEMENTATION ====== */
int crgEvaluv2xy(int cpId, double u, double v, double* x, double* y) {
  CrgContactPointStruct* cp;
  int retVal = 0;

  if (!(cp = crgContactPointGetFromId(cpId))) return 0;

  /* --- remember the input and compute the fallback solution --- */
  cp->u = u;
  cp->v = v;

  retVal = crgDataEvaluv2xy(cp->crgData, &(cp->options), cp->u, cp->v, &(cp->x), &(cp->y));

  /* --- transfer the result --- */
  *x = cp->x;
  *y = cp->y;

  return retVal;
}

int crgDataEvaluv2xy(CrgDataStruct* crgData, CrgOptionsStruct* optionList, double u, double v, double* x, double* y) {
  size_t index = 0;
  double frac;
  double p0[2];
  double p1[2];
  double p2[2];
  double p3[2];
  double a[2];
  double b[2];
  double ab[2];
  double n1[2];
  double n2[2];
  double n12[2];
  double dotProd;

  /* --- compute the fallback solution --- */
  *x = u;
  *y = v;

  if (!crgData)
    return 1;
  else if (!(crgData->channelX.info.valid))
    return 1;

  /* on closed reference lines, u must be adapted */
  crgEvalu2uvalid(crgData, optionList, &u);

  /* find u interval in constantly spaced u axis */
  frac = (u - crgData->channelU.info.first) / crgData->channelU.info.inc;

  if (frac < 0.0)
    index = 0;
  else {
    index = (size_t)frac;

    /* data dimension is at least 2x2 */
    if (index >= crgData->channelX.info.size - 1) index = crgData->channelX.info.size - 2;
  }

  /* --- remaining u fraction --- */
  frac = frac - index;

  /* extrapolation beyond ubeg: simple transformation */
  if (frac < 0) {
    double du = frac * crgData->channelU.info.inc;
    *x = crgData->channelX.info.first + du * crgData->util.phiFirstCos - v * crgData->util.phiFirstSin;
    *y = crgData->channelY.info.first + du * crgData->util.phiFirstSin + v * crgData->util.phiFirstCos;
    return 1;
  }

  /* extrapolation beyond ubeg: simple transformation */
  if (frac > 1) {
    double du = (frac - 1) * crgData->channelU.info.inc;
    *x = crgData->channelX.info.last + du * crgData->util.phiLastCos - v * crgData->util.phiLastSin;
    *y = crgData->channelY.info.last + du * crgData->util.phiLastSin + v * crgData->util.phiLastCos;
    return 1;
  }

  /* --- get endpoints at center line --- */
  p1[0] = crgData->channelX.data[index];
  p1[1] = crgData->channelY.data[index];

  p2[0] = crgData->channelX.data[index + 1];
  p2[1] = crgData->channelY.data[index + 1];

  /* --- normal on P1P2 --- */
  n12[0] = -(p2[1] - p1[1]);
  n12[1] = p2[0] - p1[0];
  normalizeVector2(n12);

  /* --- calculate additional points A and B --- */
  /* --- normal n1 through P1, default is same as n12 --- */
  n1[0] = n12[0];
  n1[1] = n12[1];

  /* if previous point P0 exists, then take P0 into account */
  if (index > 0) {
    p0[0] = crgData->channelX.data[index - 1];
    p0[1] = crgData->channelY.data[index - 1];

    n1[0] = -(p2[1] - p0[1]);
    n1[1] = p2[0] - p0[0];
    normalizeVector2(n1);
  }
  /* --- now for point A --- */
  dotProd = (n1[0] * n12[0]) + (n1[1] * n12[1]);

  if (fabs(dotProd) > 1.0e-10) {
    n1[0] /= dotProd;
    n1[1] /= dotProd;
  }
  a[0] = p1[0] + v * n1[0];
  a[1] = p1[1] + v * n1[1];

  /* --- normal n2 through P2, default is same as n12 --- */
  n2[0] = n12[0];
  n2[1] = n12[1];

  /* if next point P3 exists, then take P3 into account */
  if (index < crgData->channelX.info.size - 2) {
    p3[0] = crgData->channelX.data[index + 2];
    p3[1] = crgData->channelY.data[index + 2];

    n2[0] = -(p3[1] - p1[1]);
    n2[1] = p3[0] - p1[0];
    normalizeVector2(n2);
  }
  /* --- now for point B --- */
  dotProd = (n2[0] * n12[0]) + (n2[1] * n12[1]);

  if (fabs(dotProd) > 1.0e-10) {
    n2[0] /= dotProd;
    n2[1] /= dotProd;
  }
  b[0] = p2[0] + v * n2[0];
  b[1] = p2[1] + v * n2[1];

  ab[0] = b[0] - a[0];
  ab[1] = b[1] - a[1];

  *x = a[0] + frac * ab[0];
  *y = a[1] + frac * ab[1];

  return 1;
}

static void normalizeVector2(double* vec) {
  double length = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);

  if (length < 1.0e-10) return;

  vec[0] /= length;
  vec[1] /= length;
}
