/* ===================================================
 *  file:       crgMgr.c
 * ---------------------------------------------------
 *  purpose:	top level data manager
 * ---------------------------------------------------
 *  first edit:	13.11.2008 by M. Dupuis @ VIRES GmbH
 *  last mod.:  17.01.2017 by H. Helmich @ VIRES GmbH
 * ===================================================
    Copyright 2017 VIRES Simulationstechnologie GmbH

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
#include <stdlib.h>
#include <string.h>
#include "crgBaseLibPrivate.h"

/* ====== LOCAL VARIABLES ====== */
static CrgDataStruct **sDataSetList = NULL;
static int sNoDataSets = 0;

/* ====== LOCAL METHODS ====== */
/**
 * scale the data of a given channel
 * @param channel    pointer to the channel
 * @param factor     scale factor
 * @param dataOnly   flag to scale the data entries only, not the info values
 */
static void crgDataScaleChannel(CrgChannelBaseStruct *channel, double factor, int dataOnly);

/**
 * offset the data of a given channel
 * @param channel    pointer to the channel
 * @param factor     offset value
 */
static void crgDataOffsetChannel(CrgChannelStruct *channel, double offset);

/**
 * apply any transformation specified by modifiers
 * @param crgData    pointer to data set which is to be modified
 */
static void crgDataApplyTransformations(CrgDataStruct *crgData);

/**
 * rotate a given point around a given center for a given angle
 * @param x      pointer to point's x co-ordinate           [m]
 * @param y      pointer to point's u co-ordinate           [m]
 * @param ctrX   x co-ordinate of rotation center           [m]
 * @param ctrY   y co-ordinate of rotation center           [m]
 * @param angle  the rotation angle                       [rad]
 */
static void rotatePoint(double *x, double *y, double ctrX, double ctrY, double angle);

/* ====== IMPLEMENTATION ====== */
int crgDataSetRelease(int dataSet) {
  size_t i;

  CrgDataStruct *crgData = crgDataSetAccess(dataSet);

  if (!crgData) return 0;

  /* --- release all contact points referring to this data set --- */
  crgContactPointDeleteAll(dataSet);

  /* --- release all dynamically allocated data of the data set --- */
  for (i = 0; i < crgData->channelV.info.size; i++)
    if (crgData->channelZ[i].data) crgFree(crgData->channelZ[i].data);

  crgFree(crgData->channelZ);

  if (crgData->channelX.data) crgFree(crgData->channelX.data);

  if (crgData->channelY.data) crgFree(crgData->channelY.data);

  if (crgData->channelPhi.data) crgFree(crgData->channelPhi.data);

  if (crgData->channelV.data) crgFree(crgData->channelV.data);

  if (crgData->channelSlope.data) crgFree(crgData->channelSlope.data);

  if (crgData->channelBank.data) crgFree(crgData->channelBank.data);

  if (crgData->channelRefZ.data) crgFree(crgData->channelRefZ.data);

  /* --- get rid of modifiers and options --- */
  if (crgData->modifiers.entry) crgFree(crgData->modifiers.entry);

  if (crgData->options.entry) crgFree(crgData->options.entry);

  /* --- invalidate the data set in the master list --- */
  for (i = 0; i < (size_t)sNoDataSets; i++)
    if (sDataSetList[i] == crgData) {
      sDataSetList[i] = NULL;
      break;
    }

  /* --- finally: free the crgData struct --- */
  crgFree(crgData);

  crgMsgPrint(dCrgMsgLevelNotice, "crgDataSetRelease: released data set no. %d\n", dataSet);

  return 1;
}

CrgDataStruct *crgDataSetCreate(void) {
  int i;
  int maxId = 0;
  int tgtId = -1;

  /* --- get the maximum ID of existing data sets --- */
  for (i = 0; i < sNoDataSets; i++) {
    /* --- found some free space in list of data sets? --- */
    if (!sDataSetList[i])
      tgtId = i;
    else if (sDataSetList[i]->admin.id > maxId)
      maxId = sDataSetList[i]->admin.id;
  }

  /* --- next available ID is maximum ID plus one --- */
  maxId++;

  crgMsgPrint(dCrgMsgLevelInfo, "crgDataSetCreate: creating data set with id %d\n", maxId);

  /* --- any unused ID available or do we need to exend the data management list? --- */
  if (tgtId < 0) {
    sDataSetList = (CrgDataStruct **)crgRealloc(sDataSetList, (sNoDataSets + 1) * sizeof(CrgDataStruct *));
    tgtId = sNoDataSets;
    sNoDataSets++;
  }

  /* --- now allocate the space for the dataset itself --- */
  sDataSetList[tgtId] = (CrgDataStruct *)crgCalloc(1, sizeof(CrgDataStruct));
  sDataSetList[tgtId]->admin.id = maxId;

  /* --- allocate the memory for the options and modifiers --- */
  crgOptionCreateList(&(sDataSetList[tgtId]->options));
  crgOptionCreateList(&(sDataSetList[tgtId]->modifiers));

  /* --- set the default options of the data set --- */
  crgDataSetModifierSetDefault(maxId);
  crgDataSetOptionSetDefault(maxId);

  return sDataSetList[tgtId];
}

CrgDataStruct *crgDataSetAccess(int id) {
  int i;

  for (i = 0; i < sNoDataSets; i++) {
    if (sDataSetList[i]) {
      if (sDataSetList[i]->admin.id == id) return sDataSetList[i];
    }
  }

  return NULL;
}

void crgDataPrintHeader(int dataSetId) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) return;

  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: uMin     = %10.4f\n", crgData->channelU.info.first);
  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: xBegin   = %10.4f\n", crgData->channelX.info.first);
  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: yBegin   = %10.4f\n", crgData->channelY.info.first);
  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: phiBegin = %10.4f\n", crgData->channelPhi.info.first);
  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: uMax     = %10.4f\n", crgData->channelU.info.last);
  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: xEnd     = %10.4f\n", crgData->channelX.info.last);
  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: yEnd     = %10.4f\n", crgData->channelY.info.last);
  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: phiEnd   = %10.4f\n", crgData->channelPhi.info.last);
  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: uInc     = %10.4f\n", crgData->channelU.info.inc);

  if (!crgData->channelV.data) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgDataPrintHeader: no v channel definition available.\n");
    return;
  }

  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: vMin     = %10.4f\n", crgData->channelV.info.first);
  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: vMax     = %10.4f\n", crgData->channelV.info.last);
  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintHeader: vInc     = %10.4f\n", crgData->channelV.info.inc);
}

void crgDataPrintChannelInfo(int dataSetId) {
  size_t i;
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) return;

  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintChannelInfo: overview of channel information\n");
  crgMsgPrint(dCrgMsgLevelNotice, "    total number of channels  = %4d\n", crgData->noChannels);
  crgMsgPrint(dCrgMsgLevelNotice, "    number of v channels      = %4ld\n", crgData->channelV.info.size);

  for (i = 0; i < crgData->channelV.info.size; ++i)
    crgMsgPrint(dCrgMsgLevelNotice, "    v channel[%4ld].pos   = %.4f\n", i, crgData->channelV.data[i]);

  crgMsgPrint(dCrgMsgLevelNotice, "    number of z channels      = %4ld\n", crgData->channelV.info.size);

  for (i = 0; i < crgData->channelV.info.size; ++i) {
    crgMsgPrint(dCrgMsgLevelNotice, "    z channel[%4ld].info.index = %4d\n", i, crgData->channelZ[i].info.index);
    crgMsgPrint(dCrgMsgLevelDebug, "    z channel[%4ld].info.size  = %4ld\n", i, crgData->channelZ[i].info.size);
  }

  crgMsgPrint(dCrgMsgLevelNotice, "additional channels:\n");

  if (crgData->channelX.info.defined)
    crgMsgPrint(dCrgMsgLevelNotice, "    channelX.info.index   = %d\n", crgData->channelX.info.index);
  if (crgData->channelX.info.valid)
    crgMsgPrint(dCrgMsgLevelDebug, "    channelX.info.size    = %ld\n", crgData->channelX.info.size);

  if (crgData->channelY.info.defined)
    crgMsgPrint(dCrgMsgLevelNotice, "    channelY.info.index   = %d\n", crgData->channelY.info.index);
  if (crgData->channelY.info.valid)
    crgMsgPrint(dCrgMsgLevelDebug, "    channelY.info.size    = %ld\n", crgData->channelY.info.size);

  if (crgData->channelU.info.defined)
    crgMsgPrint(dCrgMsgLevelNotice, "    channelU.info.index   = %d\n", crgData->channelU.info.index);
  if (crgData->channelU.info.valid)
    crgMsgPrint(dCrgMsgLevelDebug, "    channelU.info.size    = %ld\n", crgData->channelU.info.size);

  if (crgData->channelPhi.info.defined)
    crgMsgPrint(dCrgMsgLevelNotice, "    channelPhi.info.index = %d\n", crgData->channelPhi.info.index);
  if (crgData->channelPhi.info.valid)
    crgMsgPrint(dCrgMsgLevelDebug, "    channelPhi.info.size  = %ld\n", crgData->channelPhi.info.size);

  if (crgData->channelSlope.info.defined)
    crgMsgPrint(dCrgMsgLevelNotice, "    channelSlope.info.index = %d\n", crgData->channelSlope.info.index);
  if (crgData->channelSlope.info.valid)
    crgMsgPrint(dCrgMsgLevelDebug, "    channelSlope.info.size  = %ld\n", crgData->channelSlope.info.size);

  if (crgData->channelBank.info.defined)
    crgMsgPrint(dCrgMsgLevelNotice, "    channelBank.info.index = %d\n", crgData->channelBank.info.index);
  if (crgData->channelBank.info.valid)
    crgMsgPrint(dCrgMsgLevelDebug, "    channelBank.info.size  = %ld\n", crgData->channelBank.info.size);

  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintChannelInfo: end of channel overview\n");
}

void crgDataPrintRoadInfo(int dataSetId) {
  size_t i;
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) return;

  crgMsgPrint(dCrgMsgLevelNotice, "crgDataPrintRoadInfo: Summary of crg basic information\n");
  crgMsgPrint(dCrgMsgLevelNotice, "                      length of road [m]:          %10.4f\n",
              crgData->channelU.info.last - crgData->channelU.info.first);
  crgMsgPrint(dCrgMsgLevelNotice, "                      cross section increment [m]: %10.4f\n",
              crgData->channelU.info.inc);
  crgMsgPrint(dCrgMsgLevelNotice, "                      number of cross sections:       %ld\n",
              crgData->channelU.info.size);

  crgMsgPrint(dCrgMsgLevelNotice, "                      width of crg road[m]:        %10.4f\n",
              crgData->channelV.info.last - crgData->channelV.info.first);
  crgMsgPrint(dCrgMsgLevelNotice, "                      width right of refline [m]:  %10.4f\n",
              crgData->channelV.info.first < 0.0 ? crgData->channelV.info.first : 0.0);
  crgMsgPrint(dCrgMsgLevelNotice, "                      width left of refline [m]:   %10.4f\n",
              crgData->channelV.info.last > 0.0 ? crgData->channelV.info.last : 0.0);

  for (i = 0; i < crgData->channelV.info.size; ++i)
    crgMsgPrint(dCrgMsgLevelNotice, "                      long section at [m]:         %10.4f\n",
                crgData->channelV.data[i]);

  crgMsgPrint(dCrgMsgLevelNotice, "                      number of v channels:         %4ld\n",
              crgData->channelV.info.size);
}

int crgDataSetHistory(int dataSetId, int histSize) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgDataSetHistory: unknown data set %d\n", dataSetId);
    return 0;
  }

  if (histSize < 0) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgDataSetHistory: invalid history size = %d\n", histSize);
    return 0;
  }

  /* --- set the history size of all contact points referring to the given data set --- */
  return crgContactPointSetHistoryForDataSet(crgData, histSize);
}

int crgDataSetGetURange(int dataSetId, double *uMin, double *uMax) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgDataSetGetURange: unknown data set %d\n", dataSetId);
    return 0;
  }

  *uMin = crgData->channelU.info.first;
  *uMax = crgData->channelU.info.last;

  return 1;
}

int crgDataSetGetVRange(int dataSetId, double *vMin, double *vMax) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgDataSetGetVRange: unknown data set %d\n", dataSetId);
    return 0;
  }

  *vMin = crgData->channelV.data[0];
  *vMax = crgData->channelV.data[crgData->channelV.info.size - 1];

  return 1;
}

int crgDataSetGetIncrements(int dataSetId, double *uInc, double *vInc) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgDataSetGetIncrements: unknown data set %d\n", dataSetId);
    return 0;
  }

  *uInc = crgData->channelU.info.inc;
  *vInc = crgData->channelV.info.inc;

  return 1;
}

int crgDataSetGetUtilityDataClosedTrack(const int dataSetId, int *uIsClosed, double *uCloseMin, double *uCloseMax) {
  const CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgDataSetGetUtilityDataClosedTrack: unknown data set %d\n", dataSetId);
    return 0;
  }

  *uIsClosed = crgData->util.uIsClosed;

  if (uIsClosed) {
    *uCloseMax = crgData->util.uCloseMax;
    *uCloseMin = crgData->util.uCloseMin;
  } else {
    crgSetNan(uCloseMax);
    crgSetNan(uCloseMin);
  }

  return 1;
}

int crgDataSetModifierSetInt(int dataSetId, unsigned int optionId, int optionValue) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetModifierSetInt: invalid data set id <%d>.\n", dataSetId);
    return 0;
  }

  return crgOptionSetInt(&(crgData->modifiers), optionId, optionValue);
}

int crgDataSetModifierSetDouble(int dataSetId, unsigned int optionId, double optionValue) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetModifierSetDouble: invalid data set id <%d>.\n", dataSetId);
    return 0;
  }

  return crgOptionSetDouble(&(crgData->modifiers), optionId, optionValue);
}

int crgDataSetModifierGetInt(int dataSetId, unsigned int optionId, int *optionValue) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetModifierGetInt: invalid data set id <%d>.\n", dataSetId);
    return 0;
  }

  return crgOptionGetInt(&(crgData->modifiers), optionId, optionValue);
}

int crgDataSetModifierGetDouble(int dataSetId, unsigned int optionId, double *optionValue) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetModifierGetDouble: invalid data set id <%d>.\n", dataSetId);
    return 0;
  }

  return crgOptionGetDouble(&(crgData->modifiers), optionId, optionValue);
}

int crgDataSetModifierRemove(int dataSetId, unsigned int optionId) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetModifierRemove: invalid data set id <%d>.\n", dataSetId);
    return 0;
  }

  return crgOptionRemove(&(crgData->modifiers), optionId);
}

int crgDataSetModifierRemoveAll(int dataSetId) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetModifierRemoveAll: invalid data set id <%d>.\n", dataSetId);
    return 0;
  }

  return crgOptionRemoveAll(&(crgData->modifiers));
}

void crgDataSetModifiersPrint(int dataSetId) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetModifiersPrint: invalid data set id <%d>.\n", dataSetId);
    return;
  }

  crgOptionsPrint(&(crgData->modifiers), "modifier");
}

void crgDataSetModifiersApply(int dataSetId) {
  double dValue;
  int iValue;
  size_t i;
  int needPrepare = 0; /* per default, data doesn't have to be re-prepared */

  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetModifiersApply: invalid data set id <%d>.\n", dataSetId);
    return;
  }

  /* --- is z scaling defined? --- */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModScaleZ, &dValue)) {
    needPrepare = 1;

    for (i = 0; i < crgData->channelV.info.size; i++)
      crgDataScaleChannel((CrgChannelBaseStruct *)&(crgData->channelZ[i]), dValue, 0);
  }

  /* --- is slope scaling defined? --- */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModScaleSlope, &dValue)) {
    needPrepare = 1;

    crgDataScaleChannel((CrgChannelBaseStruct *)&(crgData->channelSlope), dValue, 0);

    /* --- z end value of header is no longer valid! --- */
    crgData->admin.defMask &= ~dCrgDataDefZEnd;
  }

  /* --- is bank scaling defined? --- */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModScaleBank, &dValue)) {
    /* --- re-preparation is not required --- */
    crgDataScaleChannel((CrgChannelBaseStruct *)&(crgData->channelBank), dValue, 0);

    /* --- bank end value of header is no longer valid! --- */
    crgData->admin.defMask &= ~dCrgDataDefBankEnd;
  }

  /* --- is length scaling defined? --- */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModScaleLength, &dValue)) {
    double uRange = crgData->channelU.info.last - crgData->channelU.info.first;

    needPrepare = 1;

    crgDataScaleChannel((CrgChannelBaseStruct *)&(crgData->channelU), dValue, 1);

    crgData->channelU.info.last = crgData->channelU.info.first + dValue * uRange;
    crgData->channelU.info.inc *= dValue;

    /* --- x and y end values of header are no longer valid! --- */
    crgData->admin.defMask &= ~dCrgDataDefXEnd;
    crgData->admin.defMask &= ~dCrgDataDefYEnd;

    /* --- z end value of header is no longer valid! --- */
    crgData->admin.defMask &= ~dCrgDataDefZEnd;
  }

  /* --- is width scaling defined? --- */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModScaleWidth, &dValue)) {
    needPrepare = 1;

    crgDataScaleChannel((CrgChannelBaseStruct *)&(crgData->channelV), dValue, 0);
  }

  /* --- is curvature scaling defined? --- */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModScaleCurvature, &dValue)) {
    needPrepare = 1;
    {
      size_t i;

      for (i = 1; i < crgData->channelPhi.info.size; i++)
        crgData->channelPhi.data[i] =
            crgData->channelPhi.info.first + dValue * (crgData->channelPhi.data[i] - crgData->channelPhi.info.first);

      if (crgData->channelPhi.info.size)
        crgData->channelPhi.info.last = crgData->channelPhi.data[crgData->channelPhi.info.size - 1];
      else
        crgData->channelPhi.info.last = crgData->channelPhi.info.first;

      /* --- re-compute some utility data --- */
      crgData->util.phiLastSin = sin(crgData->channelPhi.info.last);
      crgData->util.phiLastCos = cos(crgData->channelPhi.info.last);
    }

    /* --- x and y end values of header are no longer valid! --- */
    crgData->admin.defMask &= ~dCrgDataDefXEnd;
    crgData->admin.defMask &= ~dCrgDataDefYEnd;
  }

  /* --- treat NaNs in grid  --- */
  if (crgOptionGetInt(&(crgData->modifiers), dCrgModGridNaNMode, &iValue)) {
    needPrepare = 1;

    dValue = 0.0;

    /* --- any offset defined? --- */
    if (iValue != dCrgGridNaNKeep) crgOptionGetDouble(&(crgData->modifiers), dCrgModGridNaNOffset, &dValue);

    crgLoaderHandleNaNs(crgData, iValue, dValue);
  }

  /* --- does the data prepare stage need to be re-called? --- */
  if (needPrepare) crgLoaderPrepareData(crgData);

  /* --- transform data to a different location? --- */
  crgDataApplyTransformations(crgData);
}

static void crgDataScaleChannel(CrgChannelBaseStruct *channel, double factor, int valuesOnly) {
  size_t i;

  if (!channel) return;

  if (channel->data) {
    if (channel->info.singlePrec) {
      float *data = (float *)(channel->data);
      for (i = 0; i < channel->info.size; i++)
        if (!crgIsNanf(&data[i])) data[i] *= (float)factor;
    } else {
      double *data = (double *)(channel->data);
      for (i = 0; i < channel->info.size; i++)
        if (!crgIsNan(&data[i])) data[i] *= factor;
    }
  }

  if (valuesOnly) return;

  channel->info.first *= factor;
  channel->info.last *= factor;
  channel->info.inc *= factor;
  channel->info.mean *= factor;
}

static void crgDataOffsetChannel(CrgChannelStruct *channel, double offset) {
  size_t i;

  if (!channel) return;

  for (i = 0; i < channel->info.size; i++) channel->data[i] += offset;

  channel->info.first += offset;
  channel->info.last += offset;
}

void crgDataSetModifierSetDefault(int dataSetId) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetModifierSetDefault: invalid data set id <%d>.\n", dataSetId);
    return;
  }

  crgOptionSetDefaultModifiers(&(crgData->modifiers));
}

void crgDataSetOptionSetDefault(int dataSetId) {
  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetOptionSetDefault: invalid data set id <%d>.\n", dataSetId);
    return;
  }

  crgOptionSetDefaultOptions(&(crgData->options));
}

void crgMemRelease(void) {
  int i;

  /* --- delete all data sets --- */
  for (i = 0; i < sNoDataSets; i++) {
    if (sDataSetList[i]) crgDataSetRelease(sDataSetList[i]->admin.id);
  }

  /* --- finally: release the list holding all data sets --- */
  crgFree(sDataSetList);

  sDataSetList = NULL;
  sNoDataSets = 0;
}

const char *crgGetReleaseInfo(void) { return "OpenCRG C-API release 1.1.2, April 17, 2018"; }

int crgIsNan(double *dValue) {
  CrgNanUnionDouble checkVal;

  if (!dValue) return 0;

  /* --- assign the value that is to be checked --- */
  memcpy(&checkVal.dVal, dValue, sizeof(double));

  if (mCrgBigEndian) return (checkVal.iVal[0] & 0x7ff80000) >= 0x7ff80000;

  return (checkVal.iVal[1] & 0x7ff80000) >= 0x7ff80000;
}

void crgSetNan(double *dValue) {
  CrgNanUnionDouble checkVal;

  if (!dValue) return;

  if (mCrgBigEndian) {
    int myNan[2] = {0x7ff80000, 0x00000000};
    memcpy(dValue, myNan, sizeof(double));
  } else {
    int myNan[2] = {0x00000000, 0x7ff80000};
    memcpy(dValue, myNan, sizeof(double));
  }

  /* --- assign the value that is to be checked --- */
  memcpy(&checkVal.dVal, dValue, sizeof(double));

  crgMsgPrint(dCrgMsgLevelDebug, "crgSetNan: checkVal = 0x%x / 0x%x, isNan = %d\n", checkVal.iVal[0], checkVal.iVal[1],
              crgIsNan(dValue));
}

void crgSetNanf(float *fValue) {
  int myNan = 0x7fc00000;

  if (!fValue) return;

  memcpy(fValue, &myNan, sizeof(float));
}

int crgIsNanf(float *fValue) {
  CrgNanUnionFloat checkVal;

  if (!fValue) return 0;

  /* --- assign the value that is to be checked --- */
  memcpy(&checkVal.fVal, fValue, sizeof(float));

  return (checkVal.iVal & 0x7fc00000) >= 0x7fc00000;
}

void crgDataSetBuildVTable(int dataSetId) {
  int i;

  CrgDataStruct *crgData = crgDataSetAccess(dataSetId);

  if (!crgData) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgDataSetBuildVTable: invalid data set id <%d>.\n", dataSetId);
    return;
  }
  if (!crgData->channelV.info.size) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgDataSetBuildVTable: no cross section for data set id <%d>.\n", dataSetId);
    return;
  }

  crgData->indexTableV.minVal = crgData->channelV.info.first;
  crgData->indexTableV.maxVal = crgData->channelV.info.last;
  crgData->indexTableV.range = crgData->indexTableV.maxVal - crgData->indexTableV.minVal;

  if (fabs(crgData->indexTableV.range) == 0.0) {
    crgData->indexTableV.size = 0;
    crgData->indexTableV.valid = 0;
    return;
  }

  crgData->indexTableV.size = dCrgVTableStdSize;
  crgData->indexTableV.valid = 1;

  for (i = 0; i < dCrgVTableStdSize; i++) {
    double vPos = crgData->indexTableV.minVal + i * crgData->indexTableV.range / (dCrgVTableStdSize - 1);

    size_t indexV = 0;
    size_t indexCtr;
    size_t index0 = crgData->channelV.info.size - 1;

    while (1) {
      indexCtr = (index0 + indexV) / 2;

      if (indexCtr <= indexV) break;

      if (vPos < crgData->channelV.data[indexCtr])
        index0 = indexCtr;
      else
        indexV = indexCtr;
    }

    crgData->indexTableV.refIdx[i] = indexV;
  }
}

static void crgDataApplyTransformations(CrgDataStruct *crgData) {
  /* --- store the transformation in the following variables --- */
  double fromXYZ[3] = {0.0, 0.0, 0.0};
  double toXYZ[3] = {0.0, 0.0, 0.0};
  double rotCenter[2] = {0.0, 0.0};
  double rotAngle = 0.0;
  double fromPhi = 0.0;
  double fromCurv = 0.0;
  int applyXform = 0;

  double uPos = 0.0;
  double vPos = 0.0;
  double offset = 0.0;
  double dValue = 0.0;

  if (!crgData) return;

  /* --- find the "from" point --- */

  /* --- default reference point is always the begin of the line --- */
  uPos = crgData->channelU.info.first;
  vPos = 0.0;

  /* --- transform data to a different locaction using an arbitrary reference point ? --- */
  /* make this code a bit more robust in terms of optimization */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModRefPointX, &dValue)) {
    toXYZ[0] = dValue;
    applyXform = 1;
  }

  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModRefPointY, &dValue)) {
    toXYZ[1] = dValue;
    applyXform = 1;
  }

  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModRefPointZ, &dValue)) {
    toXYZ[2] = dValue;
    applyXform = 1;
  }

  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModRefPointPhi, &dValue)) {
    rotAngle = dValue;
    applyXform = 1;
  }

  /* u by absolute position? */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModRefPointU, &uPos)) applyXform = 1;

  /* u by relative position? */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModRefPointUFrac, &uPos)) {
    uPos = crgData->channelU.info.first + uPos * (crgData->channelU.info.last - crgData->channelU.info.first);

    /* any offset defined? */
    crgOptionGetDouble(&(crgData->modifiers), dCrgModRefPointUOffset, &offset);

    uPos += offset;

    applyXform = 1;
  }

  /* v by absolute position? */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModRefPointV, &vPos)) applyXform = 1;

  /* v by relative position? */
  if (crgOptionGetDouble(&(crgData->modifiers), dCrgModRefPointVFrac, &vPos)) {
    vPos = crgData->channelV.info.first + vPos * (crgData->channelV.info.last - crgData->channelV.info.first);

    /* any offset defined? */
    offset = 0.0;

    crgOptionGetDouble(&(crgData->modifiers), dCrgModRefPointVOffset, &offset);

    vPos += offset;

    applyXform = 1;
  }

  if (applyXform) {
    /* --- compute FROM point --- */
    crgDataEvaluv2xy(crgData, &(crgData->options), uPos, vPos, &(fromXYZ[0]), &(fromXYZ[1]));
    crgDataEvaluv2z(crgData, &(crgData->options), uPos, vPos, &(fromXYZ[2]));
    crgDataEvaluv2pk(crgData, &(crgData->options), uPos, vPos, &(fromPhi), &(fromCurv));

    /* correct rotation angle */
    rotAngle -= fromPhi;

    rotCenter[0] = fromXYZ[0];
    rotCenter[1] = fromXYZ[1];
  }

  /* --- if transformation is not defined by reference point, then check for relative offsets --- */
  if (!applyXform) {
    int transform = 0;

    /* --- per default, transformation point is begin of reference line --- */
    fromXYZ[0] = crgData->channelX.info.first;
    fromXYZ[1] = crgData->channelY.info.first;

    /* --- default: rotate around xbeg, ybeg */
    rotCenter[0] = fromXYZ[0];
    rotCenter[1] = fromXYZ[1];

    crgOptionGetDouble(&(crgData->modifiers), dCrgModRefLineRotCenterX, &(rotCenter[0]));
    crgOptionGetDouble(&(crgData->modifiers), dCrgModRefLineRotCenterY, &(rotCenter[1]));

    /* --- transform data to a different location using a plain offset? --- */
    /* make this code a bit more robust in terms of optimization */
    transform = crgOptionGetDouble(&(crgData->modifiers), dCrgModRefLineOffsetX, &(toXYZ[0]));
    transform |= crgOptionGetDouble(&(crgData->modifiers), dCrgModRefLineOffsetY, &(toXYZ[1]));
    transform |= crgOptionGetDouble(&(crgData->modifiers), dCrgModRefLineOffsetZ, &(toXYZ[2]));
    transform |= crgOptionGetDouble(&(crgData->modifiers), dCrgModRefLineOffsetPhi, &rotAngle);

    crgDataEvaluv2z(crgData, NULL, 0.0, 0.0, &(fromXYZ[2]));

    toXYZ[0] += fromXYZ[0];
    toXYZ[1] += fromXYZ[1];
    toXYZ[2] += fromXYZ[2];

    /* reset the temporarily used values */
    fromPhi = 0.0;
    fromCurv = 0.0;

    /* --- is there already a need for applying a transformation? --- */
    applyXform = transform;
  }

  if (applyXform) /* temporarily disabled, for debugging only */
    crgMsgPrint(
        dCrgMsgLevelDebug,
        "crgDataApplyTransformations: rotCx / rotCy / dphi / dx / dy / dz  = %.6f %.6f %.6f / %.6f / %.6f / %.6f\n",
        rotCenter[0], rotCenter[1], rotAngle, toXYZ[0] - fromXYZ[0], toXYZ[1] - fromXYZ[1], toXYZ[2] - fromXYZ[2]);

  if (applyXform) {
    size_t i;

    /* --- first rotate --- */
    /* phi on center line */

    crgMsgPrint(dCrgMsgLevelDebug,
                "crgDataApplyTransformations: rotating data by %.3lf deg around %.3f / %.3f fromPhi = %.3lf \n",
                rotAngle * 180 / 3.14159265, rotCenter[0], rotCenter[1], fromPhi * 180 / 3.14159265);

    crgDataOffsetChannel(&(crgData->channelPhi), rotAngle);

    /* --- compute sine and cosine of direction at either end of reference line */
    crgData->util.phiFirstSin = sin(crgData->channelPhi.info.first);
    crgData->util.phiFirstCos = cos(crgData->channelPhi.info.first);
    crgData->util.phiLastSin = sin(crgData->channelPhi.info.last);
    crgData->util.phiLastCos = cos(crgData->channelPhi.info.last);

    /* x,y data of center line */
    rotatePoint(&(crgData->channelX.info.first), &(crgData->channelY.info.first), rotCenter[0], rotCenter[1], rotAngle);
    rotatePoint(&(crgData->channelX.info.last), &(crgData->channelY.info.last), rotCenter[0], rotCenter[1], rotAngle);

    for (i = 0; i < crgData->channelX.info.size; i++)
      rotatePoint(&(crgData->channelX.data[i]), &(crgData->channelY.data[i]), rotCenter[0], rotCenter[1], rotAngle);

    /* --- translation --- */
    crgDataOffsetChannel(&(crgData->channelX), toXYZ[0] - fromXYZ[0]);
    crgDataOffsetChannel(&(crgData->channelY), toXYZ[1] - fromXYZ[1]);

    if (crgData->channelRefZ.info.valid) {
      for (i = 0; i < crgData->channelRefZ.info.size; i++) crgData->channelRefZ.data[i] += toXYZ[2] - fromXYZ[2];

      crgData->channelRefZ.info.first += toXYZ[2] - fromXYZ[2];
      crgData->channelRefZ.info.last += toXYZ[2] - fromXYZ[2];
    } else if (crgData->admin.defMask & dCrgDataDefZStart) {
      crgData->channelRefZ.info.first += toXYZ[2] - fromXYZ[2];
      crgData->channelRefZ.info.last += toXYZ[2] - fromXYZ[2];
    } else {
      for (i = 0; i < crgData->channelV.info.size; i++) {
        crgData->channelZ[i].info.mean += toXYZ[2] - fromXYZ[2];
        crgData->channelZ[i].info.first += toXYZ[2] - fromXYZ[2];
        crgData->channelZ[i].info.last += toXYZ[2] - fromXYZ[2];
      }
    }
  }
}

static void rotatePoint(double *x, double *y, double ctrX, double ctrY, double rotAngle) {
  double dx;
  double dy;
  double angle;
  double dist;

  /* x,y data of center line */
  dx = *x - ctrX;
  dy = *y - ctrY;

  angle = atan2(dy, dx);
  angle += rotAngle;
  dist = sqrt(dx * dx + dy * dy);

  *x = ctrX + dist * cos(angle);
  *y = ctrY + dist * sin(angle);
}
