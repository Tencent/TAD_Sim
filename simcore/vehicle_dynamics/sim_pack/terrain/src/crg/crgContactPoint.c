/* ===================================================
 *  file:       crgContactPoint.c
 * ---------------------------------------------------
 *  purpose:	contact point management routines
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crgBaseLibPrivate.h"

/* ====== DEFINITIONS ====== */

/* ====== TYPE DEFINITIONS ====== */

/* ====== LOCAL VARIABLES ====== */
static CrgContactPointStruct** cpTable = NULL;
static int cpTableSize = 0;

/* ====== IMPLEMENTATION ====== */
int crgContactPointCreate(int dataSetId) {
  int i;
  int tgtId = -1;
  int validIds = 0;
  CrgContactPointStruct* cp = NULL;
  CrgDataStruct* crgData = crgDataSetAccess(dataSetId);

  if (!crgData) return -1;

  cp = (CrgContactPointStruct*)crgCalloc(1, sizeof(CrgContactPointStruct));

  /* --- get the maximum ID of existing data sets --- */
  for (i = 0; i < cpTableSize; i++) {
    /* --- found some free space in list of contact points? --- */
    if (!cpTable[i])
      tgtId = i;
    else
      ++validIds;
  }

  /* --- any unused ID available or do we need to exend the data management list? --- */
  if (tgtId < 0) {
    cpTable = (CrgContactPointStruct**)crgRealloc(cpTable, (cpTableSize + 1) * sizeof(CrgContactPointStruct*));
    tgtId = cpTableSize;
    cpTableSize++;
  }

  if (!cpTable || !cp) {
    crgMsgPrint(dCrgMsgLevelFatal, "crgCreateContactPoint: could not allocate new contact point.\n");
    return -1;
  }

  /* --- allocate space for the history --- */
  crgContactPointPtrSetHistory(cp, dCrgHistoryStdSize);

  /* --- now register contact point in table --- */
  cpTable[tgtId] = cp;
  cp->crgData = crgData;

  /* --- allocate the memory for the options --- */
  crgOptionCreateList(&(cp->options));

  /* --- set the default options of the contact point --- */
  crgContactPointSetDefaultOptions(tgtId);

  /* --- get the options defined in the data set --- */
  crgOptionCopyAll(&(cp->options), &(crgData->options));

#ifdef dCrgEnableDebug2
  crgMsgPrint(dCrgMsgLevelNotice, "crgContactPointCreate: created contact point %d. Now have %d contact points.\n",
              tgtId, validIds);
#endif

  /* --- return the contact point ID, i.e. its position in the contact point table --- */
  return tgtId;
}

int crgContactPointDelete(int cpId) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) return 0;

  /* --- free data associated with the contact point --- */
  crgContactPointReset(cp);

  /* --- mark contact point in cp table as unused --- */
  cpTable[cpId] = NULL;

  /* --- free the actual contact point data --- */
  crgFree(cp);

#ifdef dCrgEnableDebug2
  crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointDelete: deleted contact point %d.\n", cpId);
#endif

  return 1;
}

void crgContactPointDeleteAll(int dataSetId) {
  CrgContactPointStruct* cp = NULL;
  int cpId;

  for (cpId = 0; cpId < cpTableSize; cpId++) {
    if (dataSetId == -1)
      crgContactPointDelete(cpId);
    else {
      cp = crgContactPointGetFromId(cpId);

      if (cp) {
        if (cp->crgData) {
          if (cp->crgData->admin.id == dataSetId) crgContactPointDelete(cpId);
        }
      }
    }
  }

  /* --- now release any memory held for the contact point management --- */
  if (dataSetId == -1) {
    crgFree(cpTable);

    cpTable = NULL;
    cpTableSize = 0;
  }
}

void crgContactPointReset(CrgContactPointStruct* cp) {
  if (!cp) return;

  if (cp->history.entry) crgFree(cp->history.entry);

  if (cp->options.entry) crgFree(cp->options.entry);

  cp->options.entry = NULL;
  cp->options.noEntries = 0;

  /* crgMsgPrint( dCrgMsgLevelWarn, "crgContactPointReset: called.\n" );*/
}

int crgContactPointSetCrgData(CrgContactPointStruct* cp, int dataSetId) {
  CrgDataStruct* crgData = crgDataSetAccess(dataSetId);

  if (!cp) return 0;

  cp->crgData = crgData;

  /* --- history etc. won't work anymore! --- */
  crgContactPointReset(cp);

  return 1;
}

CrgContactPointStruct* crgContactPointGetFromId(int cpId) {
  if (cpId < 0 || cpId >= cpTableSize) return NULL;

  return cpTable[cpId];
}

int crgContactPointOptionSetInt(int cpId, unsigned int optionId, int optionValue) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointOptionSetInt: invalid contact point id <%d>.\n", cpId);
    return 0;
  }

  return crgOptionSetInt(&(cp->options), optionId, optionValue);
}

int crgContactPointOptionSetDouble(int cpId, unsigned int optionId, double optionValue) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointOptionSetDouble: invalid contact point id <%d>.\n", cpId);
    return 0;
  }

  /* --- some options have immediate effect on other settings --- */
  /* --- or should be registered at additional places for     --- */
  /* --- higher performance during queries                    --- */
  switch (optionId) {
    case dCrgCpOptionRefLineSearchU:
      crgContactPointPreloadHistoryU(cp, optionValue);
      break;
    case dCrgCpOptionRefLineSearchUFrac:
      crgContactPointPreloadHistoryUFrac(cp, optionValue);
      break;
    case dCrgCpOptionRefLineClose:
      cp->history.closeDist = optionValue * optionValue; /* internally, square of distance is used */
      break;
    case dCrgCpOptionRefLineFar:
      cp->history.farDist = optionValue * optionValue; /* internally, square of distance is used */
      break;
  }

  return crgOptionSetDouble(&(cp->options), optionId, optionValue);
}

int crgContactPointOptionGetInt(int cpId, unsigned int optionId, int* optionValue) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointOptionGetInt: invalid contact point id <%d>.\n", cpId);
    return 0;
  }

  return crgOptionGetInt(&(cp->options), optionId, optionValue);
}

int crgContactPointOptionGetDouble(int cpId, unsigned int optionId, double* optionValue) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointOptionGetDouble: invalid contact point id <%d>.\n", cpId);
    return 0;
  }

  return crgOptionGetDouble(&(cp->options), optionId, optionValue);
}

int crgContactPointOptionRemove(int cpId, unsigned int optionId) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointOptionRemove: invalid contact point id <%d>.\n", cpId);
    return 0;
  }

  return crgOptionRemove(&(cp->options), optionId);
}

int crgContactPointOptionRemoveAll(int cpId) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointOptionRemove: invalid contact point id <%d>.\n", cpId);
    return 0;
  }

  return crgOptionRemoveAll(&(cp->options));
}

void crgContactPointOptionsPrint(int cpId) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointOptionsPrint: invalid contact point id <%d>.\n", cpId);
    return;
  }

  crgOptionsPrint(&(cp->options), "option");
}

void crgContactPointSetDefaultOptions(int cpId) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointSetDefaultOptions: invalid contact point id <%d>.\n", cpId);
    return;
  }

  crgOptionSetDefaultOptions(&(cp->options));

  /* --- copy history options back to contact point's history buffer --- */
  crgContactPointOptionSetDouble(cpId, dCrgCpOptionRefLineClose, 0.3);
  crgContactPointOptionSetDouble(cpId, dCrgCpOptionRefLineFar, 2.2);
}

int crgContactPointOptionIsSet(CrgContactPointStruct* cp, unsigned int optionId) {
  if (!cp) return 0;

  return crgOptionIsSet(&(cp->options), optionId);
}

int crgContactPointOptionHasValueInt(CrgContactPointStruct* cp, unsigned int optionId, int optionValue) {
  if (!cp) return 0;

  return crgOptionHasValueInt(&(cp->options), optionId, optionValue);
}

int crgContactPointSetHistory(int cpId, int histSize) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointSetHistory: invalid contact point id <%d>.\n", cpId);
    return 0;
  }

  crgContactPointPtrSetHistory(cp, histSize);

  return 1;
}

int crgContactPointPtrSetHistory(CrgContactPointStruct* cp, int histSize) {
  if (!cp) return 0;

  if (histSize < 0) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgContactPointPtrSetHistory: invalid history size = %d\n", histSize);
    return 0;
  }

  /* --- free existing history --- */
  if (cp->history.entry) crgFree(cp->history.entry);

  cp->history.entry = NULL;
  cp->history.totalSize = histSize;
  cp->history.usedSize = 0;
  cp->history.entrySize = sizeof(CrgHistoryEntryStruct);

  if (histSize) cp->history.entry = (CrgHistoryEntryStruct*)crgCalloc(histSize, sizeof(CrgHistoryEntryStruct));

  if (!(cp->history.entry)) {
    crgMsgPrint(dCrgMsgLevelNotice, "crgContactPointPtrSetHistory: could not allocate history.\n");
    return 0;
  }

  /* --- pre-calculate some history variables (note: it's the square value!) --- */
  if (cp->crgData) {
    if (cp->crgData->channelU.info.valid) {
      cp->history.closeDist = 100 * cp->crgData->channelU.info.inc * cp->crgData->channelU.info.inc;
      cp->history.farDist = 100 * cp->history.closeDist;
    } else
      crgMsgPrint(dCrgMsgLevelInfo,
                  "crgContactPointPtrSetHistory: u data invalid, incomplete initialization of history.\n");
  }

  return 1;
}

int crgContactPointSetHistoryForDataSet(CrgDataStruct* crgData, int histSize) {
  int i;
  int result = 1;

  if (!crgData) return 0;

  for (i = 0; i < cpTableSize; i++) {
    if (cpTable[i]) {
      if (cpTable[i]->crgData == crgData) result = result && crgContactPointSetHistoryForDataSet(crgData, histSize);
    }
  }

  return result;
}

void crgContactPointPreloadHistoryU(CrgContactPointStruct* cp, double u) {
  double frac;
  size_t index;

  if (!cp) return;

  if (!cp->crgData) {
    crgMsgPrint(
        dCrgMsgLevelWarn,
        "crgContactPointPreloadHistoryU: contact point does not reference CRG data set. Operation is invalid.\n");
    return;
  }

  /* --- remember result in history --- */
  if (!cp->history.totalSize) return;

  /* --- shift previous values --- */
  if (cp->history.totalSize > 1)
    memmove(&(cp->history.entry[1]), cp->history.entry, (cp->history.totalSize - 1) * cp->history.entrySize);

  /* --- compute x and y for u on reference line --- */
  frac = (u - cp->crgData->channelU.info.first) / cp->crgData->channelU.info.inc;

  if (frac < 0.0)
    index = 0;
  else {
    index = (size_t)frac;

    /* data dimension is at least 2x2 */
    if (index >= cp->crgData->channelX.info.size - 1) index = cp->crgData->channelX.info.size - 2;
  }

  frac -= index;

  cp->history.entry[0].x = cp->crgData->channelX.data[index] +
                           frac * (cp->crgData->channelX.data[index + 1] - cp->crgData->channelX.data[index]);
  cp->history.entry[0].y = cp->crgData->channelY.data[index] +
                           frac * (cp->crgData->channelY.data[index + 1] - cp->crgData->channelY.data[index]);
  cp->history.entry[0].index = index;

  /* --- old elements will be deleted from stack --- */
  cp->history.usedSize = 1;

  /** @todo: check whether restricting used size is ok or whether pre-loading of multiple values shall be allowed */
}

void crgContactPointPreloadHistoryUFrac(CrgContactPointStruct* cp, double uFrac) {
  double u;

  if (!cp->crgData) return;

  if (uFrac < 0.0 || uFrac > 1.0) return;

  u = cp->crgData->channelU.info.first + uFrac * (cp->crgData->channelU.info.last - cp->crgData->channelU.info.first);

  crgContactPointPreloadHistoryU(cp, u);
}

void crgContactPointActivatePerfStat(int cpId) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

#ifndef dCrgEnableStats
  crgMsgPrint(dCrgMsgLevelWarn,
              "crgContactPointActivatePerfStat: performance statistics have been disabled\n"
              "         Please provide/activate \"#define dCrgEnableStats\" in \"crgBaseLibPrivate.h\"\n");
  return;
#endif

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointActivatePerfStat: invalid contact point id <%d>.\n", cpId);
    return;
  }

  /* --- reset old contents first --- */
  crgContactPointResetPerfStat(cp);

  cp->history.stat.active = 1;
  if (cp->crgData) cp->crgData->perfStat.active = 1;
}

void crgContactPointDeActivatePerfStat(int cpId) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

#ifndef dCrgEnableStats
  crgMsgPrint(dCrgMsgLevelWarn,
              "crgContactPointDeActivatePerfStat: performance statistics have been disabled\n"
              "Please provide/activate \"#define dCrgEnableStats\" in \"crgBaseLibPrivate.h\"\n");
  return;
#endif

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointDeActivatePerfStat: invalid contact point id <%d>.\n", cpId);
    return;
  }

  cp->history.stat.active = 0;
  if (cp->crgData) cp->crgData->perfStat.active = 0;
}

void crgContactPointResetPerfStat(CrgContactPointStruct* cp) {
#ifndef dCrgEnableStats
  crgMsgPrint(dCrgMsgLevelWarn,
              "crgContactPointResetPerfStat: performance statistics have been disabled\n"
              "Please provide/activate \"#define dCrgEnableStats\" in \"crgBaseLibPrivate.h\"\n");
  return;
#endif

  if (!cp) return;

  memset(&(cp->history.stat), 0, sizeof(CrgHistoryStatStruct));

  if (cp->crgData) memset(&(cp->crgData->perfStat), 0, sizeof(CrgPerformanceStruct));
}

void crgContactPointPrintPerfStat(int cpId) {
  CrgContactPointStruct* cp = crgContactPointGetFromId(cpId);

  if (!cp) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgContactPointOptionRemove: invalid contact point id <%d>.\n", cpId);
    return;
  }

  crgMsgPrint(dCrgMsgLevelNotice, "Performance statistics for contact point <%d>\n", cpId);
  crgMsgPrint(dCrgMsgLevelNotice, "    History\n");
  crgMsgPrint(dCrgMsgLevelNotice, "        total number of queries:          %d\n", cp->history.stat.noTotalQueries);
  crgMsgPrint(dCrgMsgLevelNotice, "        total number of close hits:       %d\n", cp->history.stat.noCloseHits);
  crgMsgPrint(dCrgMsgLevelNotice, "        total number of far hits:         %d\n", cp->history.stat.noFarHits);
  crgMsgPrint(dCrgMsgLevelNotice, "        total number of non-hits:         %d\n", cp->history.stat.noNoHits);
  crgMsgPrint(dCrgMsgLevelNotice, "        total number of iterations:       %d\n", cp->history.stat.noIter);
  crgMsgPrint(dCrgMsgLevelNotice, "        total number of calls to loop 1:  %d\n", cp->history.stat.noCallsLoop1);
  crgMsgPrint(dCrgMsgLevelNotice, "        total number of calls to loop 2:  %d\n", cp->history.stat.noCallsLoop2);

  if (cp->crgData) {
    crgMsgPrint(dCrgMsgLevelNotice, "    Evaluation\n");
    crgMsgPrint(dCrgMsgLevelNotice, "        total number of queries:             %d\n",
                cp->crgData->perfStat.noTotalQueries);
    crgMsgPrint(dCrgMsgLevelNotice, "        total number of calls to loop V1:    %d\n",
                cp->crgData->perfStat.noCallsLoopV1);
    crgMsgPrint(dCrgMsgLevelNotice, "        max. number of calls to loop V1:     %d\n",
                cp->crgData->perfStat.maxCallsLoopV1);
    crgMsgPrint(dCrgMsgLevelNotice, "        total number of calls to border U:   %d\n",
                cp->crgData->perfStat.noCallsBorderU);
    crgMsgPrint(dCrgMsgLevelNotice, "        total number of calls to border V:   %d\n",
                cp->crgData->perfStat.noCallsBorderV);
  }
}

void crgContactPointPrintHistory(CrgContactPointStruct* cp, double x, double y) {
  int i;

  if (!cp) return;

  crgMsgPrint(dCrgMsgLevelNotice, "History for contact point %p during query %d\n", (void*)(cp),
              cp->history.stat.noTotalQueries);

  for (i = 0; i < cp->history.usedSize; i++) {
    double dx = (cp->history.entry[i].x - x);
    double dy = (cp->history.entry[i].y - y);
    double dist2;

    dist2 = dx * dx + dy * dy;

    crgMsgPrint(dCrgMsgLevelNotice, "entry %d: x / y = %.3f / %.3f, dist2 = %.3lf, index = %d\n", i,
                cp->history.entry[i].x, cp->history.entry[i].y, dist2, cp->history.entry[i].index);
  }
}
