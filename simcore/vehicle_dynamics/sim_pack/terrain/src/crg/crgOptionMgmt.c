/* ===================================================
 *  file:       crgOptionMgmt.c
 * ---------------------------------------------------
 *  purpose:	option management routines
 * ---------------------------------------------------
 *  first edit:	31.03.2009 by M. Dupuis @ VIRES GmbH
 *  last mod.:  26.02.2010 by M. Dupuis @ VIRES GmbH
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crgBaseLibPrivate.h"

/* ====== DEFINITIONS ====== */

/* ====== TYPE DEFINITIONS ====== */

/* ====== LOCAL METHODS ====== */

/**
 * access/create an option entry for the given option ID
 * @param  optionList pointer to a list holding all applicable options
 * @param optionId    numeric ID of the option whose entry shall be accessed
 * @param optionType  numeric ID of the option's assumed type
 * @return pointer to the option entry in the contact point's option list
 */
static CrgOptionEntryStruct* crgOptionGetEntry(CrgOptionsStruct* optionList, unsigned int optionId,
                                               unsigned int optionType);

/* ====== IMPLEMENTATION ====== */

const char* crgOptionGetName(unsigned int optionId) {
  switch (optionId) {
    case dCrgCpOptionBorderModeU:
      return "border mode u";
      break;

    case dCrgCpOptionBorderModeV:
      return "border mode v";
      break;

    case dCrgCpOptionCurvMode:
      return "curvature evaluation mode";
      break;

    case dCrgCpOptionBorderOffsetU:
      return "offset at u border";
      break;

    case dCrgCpOptionBorderOffsetV:
      return "offset at v border";
      break;

    case dCrgCpOptionSmoothUBegin:
      return "smoothing zone at begin";
      break;

    case dCrgCpOptionSmoothUEnd:
      return "smoothing zone at end";
      break;

    case dCrgCpOptionRefLineSearchU:
      return "refline search value u";
      break;

    case dCrgCpOptionRefLineSearchUFrac:
      return "refline search value uFrac";
      break;

    case dCrgCpOptionRefLineClose:
      return "refline search close criteria";
      break;

    case dCrgCpOptionRefLineFar:
      return "refline search far criteria";
      break;

    case dCrgCpOptionRefLineContinue:
      return "refline continuation mode";
      break;

    case dCrgCpOptionCheckEps:
      return "expected min. accuracy";
      break;

    case dCrgCpOptionCheckInc:
      return "expected min. increment";
      break;

    case dCrgCpOptionCheckTol:
      return "expected abs. tolerance";
      break;

    case dCrgModScaleZ:
      return "modifier z scale";
      break;

    case dCrgModScaleSlope:
      return "modifier slope scale";
      break;

    case dCrgModScaleBank:
      return "modifier bank scale";
      break;

    case dCrgModScaleLength:
      return "modifier length scale";
      break;

    case dCrgModScaleWidth:
      return "modifier width scale";
      break;

    case dCrgModScaleCurvature:
      return "modifier curvature scale";
      break;

    case dCrgModGridNaNMode:
      return "modifier grid NaN mode";
      break;

    case dCrgModGridNaNOffset:
      return "modifier grid NaN offset";
      break;

    case dCrgModRefPointV:
      return "modifier reference point v";
      break;

    case dCrgModRefPointVFrac:
      return "modifier reference point v fraction";
      break;

    case dCrgModRefPointVOffset:
      return "modifier reference point v offset";
      break;

    case dCrgModRefPointU:
      return "modifier reference point u";
      break;

    case dCrgModRefPointUFrac:
      return "modifier reference point u fraction";
      break;

    case dCrgModRefPointUOffset:
      return "modifier reference point u offset";
      break;

    case dCrgModRefPointX:
      return "reference point x";
      break;

    case dCrgModRefPointY:
      return "reference point y";
      break;

    case dCrgModRefPointZ:
      return "reference point z";
      break;

    case dCrgModRefPointPhi:
      return "reference point phi";
      break;

    case dCrgModRefLineRotCenterX:
      return "modifier reference line rotation center x";
      break;

    case dCrgModRefLineRotCenterY:
      return "modifier reference line rotation center y";
      break;

    case dCrgModRefLineOffsetPhi:
      return "modifier reference line offset phi";
      break;

    case dCrgModRefLineOffsetX:
      return "modifier reference line offset x";
      break;

    case dCrgModRefLineOffsetY:
      return "modifier reference line offset y";
      break;

    case dCrgModRefLineOffsetZ:
      return "modifier reference line offset z";
      break;

    default:
      return "unknown";
      break;
  }

  /* --- just to avoid a compiler warning --- */
  return "unknown";
}

int crgOptionGetType(unsigned int optionId) {
  switch (optionId) {
    case dCrgCpOptionBorderOffsetU:
    case dCrgCpOptionBorderOffsetV:
    case dCrgCpOptionSmoothUBegin:
    case dCrgCpOptionSmoothUEnd:
    case dCrgCpOptionRefLineSearchU:
    case dCrgCpOptionRefLineSearchUFrac:
    case dCrgCpOptionRefLineClose:
    case dCrgCpOptionRefLineFar:
    case dCrgCpOptionCheckEps:
    case dCrgCpOptionCheckInc:
    case dCrgCpOptionCheckTol:
      return dCrgOptionDataTypeDouble;
      break;

    case dCrgModScaleZ:
    case dCrgModScaleSlope:
    case dCrgModScaleBank:
    case dCrgModScaleLength:
    case dCrgModScaleWidth:
    case dCrgModScaleCurvature:
    case dCrgModGridNaNOffset:
    case dCrgModRefPointU:
    case dCrgModRefPointV:
    case dCrgModRefPointUFrac:
    case dCrgModRefPointUOffset:
    case dCrgModRefPointVFrac:
    case dCrgModRefPointVOffset:
    case dCrgModRefPointX:
    case dCrgModRefPointY:
    case dCrgModRefPointZ:
    case dCrgModRefPointPhi:
    case dCrgModRefLineRotCenterX:
    case dCrgModRefLineRotCenterY:
    case dCrgModRefLineOffsetPhi:
    case dCrgModRefLineOffsetX:
    case dCrgModRefLineOffsetY:
    case dCrgModRefLineOffsetZ:
      return dCrgOptionDataTypeDouble;
      break;

    default:
      return dCrgOptionDataTypeInt;
      break;
  }

  /* --- just to avoid a compiler warning --- */
  return dCrgOptionDataTypeInt;
}

int crgOptionSetInt(CrgOptionsStruct* optionList, unsigned int optionId, int optionValue) {
  CrgOptionEntryStruct* entry = crgOptionGetEntry(optionList, optionId, dCrgOptionDataTypeInt);

  if (!entry) {
    crgMsgPrint(dCrgMsgLevelWarn,
                "crgOptionSetInt: error accessing option entry for option <%s>. "
                "Wrong type, option identifier or contact point ID\n",
                crgOptionGetName(optionId));
    return 0;
  }
  crgMsgPrint(dCrgMsgLevelDebug, "crgOptionSetInt: setting option <%s> to value <%d>\n", crgOptionGetName(optionId),
              optionValue);

  entry->id = optionId;
  entry->iValue = optionValue;
  entry->dataType = dCrgOptionDataTypeInt;
  entry->valid = 1;

  return 1;
}

int crgOptionSetDouble(CrgOptionsStruct* optionList, unsigned int optionId, double optionValue) {
  CrgOptionEntryStruct* entry;

  /* --- check the validity of the option value --- */
  switch (optionId) {
    case dCrgCpOptionSmoothUBegin:
    case dCrgCpOptionSmoothUEnd:
      if (optionValue <= 0.0) {
        crgMsgPrint(dCrgMsgLevelWarn, "crgOptionSetDouble: value for option <%s> must be greater than zero. Ignoring. ",
                    crgOptionGetName(optionId));
        return 0;
      }
      break;
    default:
      break;
  }

  entry = crgOptionGetEntry(optionList, optionId, dCrgOptionDataTypeDouble);

  if (!entry) {
    crgMsgPrint(dCrgMsgLevelWarn,
                "crgOptionSetDouble: error accessing option entry for option <%s>. "
                "Wrong type, option identifier or contact point ID\n",
                crgOptionGetName(optionId));
    return 0;
  }

  entry->id = optionId;
  entry->dValue = optionValue;
  entry->dataType = dCrgOptionDataTypeDouble;
  entry->valid = 1;

  return 1;
}

int crgOptionGetInt(CrgOptionsStruct* optionList, unsigned int optionId, int* optionValue) {
  if (!crgOptionIsSet(optionList, optionId)) return 0;

  *optionValue = optionList->entry[optionId].iValue;

  return 1;
}

int crgOptionGetDouble(CrgOptionsStruct* optionList, unsigned int optionId, double* optionValue) {
  if (!crgOptionIsSet(optionList, optionId)) return 0;

  *optionValue = optionList->entry[optionId].dValue;

  return 1;
}

int crgOptionRemove(CrgOptionsStruct* optionList, unsigned int optionId) {
  /* --- instead of re-allocating the options, shifting entries etc.,     --- */
  /* --- "old" entries are just marked invalid and may be reused later on --- */

  if (!optionList) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgOptionRemove: invalid option list.\n");
    return 0;
  }

  if (optionId >= optionList->noEntries) return 0;

  optionList->entry[optionId].valid = 0;

  return 1;
}

int crgOptionRemoveAll(CrgOptionsStruct* optionList) {
  unsigned int i;

  if (!optionList) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgOptionRemoveAll: invalid option list.\n");
    return 0;
  }

  for (i = 0; i < optionList->noEntries; i++) optionList->entry[i].valid = 0;

  return 1;
}

void crgOptionsPrint(CrgOptionsStruct* optionList, const char* label) {
  unsigned int i;
  int hasOption = 0;

  if (!optionList) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgOptionsPrint: invalid %s list.\n", label);
    return;
  }

  crgMsgPrint(dCrgMsgLevelNotice, "crgOptionsPrint: available %ss:\n", label);

  for (i = 0; i < optionList->noEntries; i++)
    if (optionList->entry[i].valid) {
      hasOption = 1;
      crgMsgPrint(dCrgMsgLevelNotice, "    %8s = \"%s\"\n", label, crgOptionGetName(optionList->entry[i].id));

      switch (optionList->entry[i].dataType) {
        case dCrgOptionDataTypeInt:
          crgMsgPrint(dCrgMsgLevelNotice, "       value = %d\n", optionList->entry[i].iValue);
          break;

        case dCrgOptionDataTypeDouble:
          crgMsgPrint(dCrgMsgLevelNotice, "       value = %.4f\n", optionList->entry[i].dValue);
          break;
      }
    }

  if (!hasOption) crgMsgPrint(dCrgMsgLevelNotice, "    %ss disabled or none set\n", label);

  crgMsgPrint(dCrgMsgLevelNotice, "\n");
}

int crgOptionIsSet(CrgOptionsStruct* optionList, unsigned int optionId) {
  if (!optionList) return 0;

  if (optionList->noEntries <= optionId) return 0;

  return optionList->entry[optionId].valid;
}

int crgOptionHasValueInt(CrgOptionsStruct* optionList, unsigned int optionId, int optionValue) {
  if (!crgOptionIsSet(optionList, optionId)) return 0;

  return optionList->entry[optionId].iValue == optionValue;
}

static CrgOptionEntryStruct* crgOptionGetEntry(CrgOptionsStruct* optionList, unsigned int optionId,
                                               unsigned int optionType) {
  if (!optionList) {
    crgMsgPrint(dCrgMsgLevelDebug, "crgOptionGetEntry: invalid option set.\n");
    return NULL;
  }

  /* --- perform type check --- */
  if (crgOptionGetType(optionId) != optionType) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgOptionGetEntry: wrong type of option <%s>.\n", crgOptionGetName(optionId));
    return NULL;
  }

  /* --- for faster computation, the options are kept in a list with one entry for each --- */
  /* --- possible optionId, so that options can be accessed directly by index           --- */
  if (optionId < optionList->noEntries) return &(optionList->entry[optionId]);

  /* --- new option, so add to list of options --- */
  optionList->entry =
      (CrgOptionEntryStruct*)crgRealloc(optionList->entry, (optionId + 1) * sizeof(CrgOptionEntryStruct));

  if (!optionList->entry) return NULL;

  /* --- initialize the new data --- */
  memset(&(optionList->entry[optionList->noEntries]), 0,
         (optionId - optionList->noEntries + 1) * sizeof(CrgOptionEntryStruct));

  optionList->noEntries = optionId + 1;

  return &(optionList->entry[optionId]);
}

void crgOptionSetDefaultOptions(CrgOptionsStruct* optionList) {
  if (!optionList) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgOptionSetDefault: invalid option set.\n");
    return;
  }

  /* remove all existing options */
  crgOptionRemoveAll(optionList);

  /* add the default options, one at a time */
  crgOptionSetInt(optionList, dCrgCpOptionCurvMode, dCrgCurvLateral);
  crgOptionSetInt(optionList, dCrgCpOptionBorderModeU, dCrgBorderModeExKeep);
  crgOptionSetInt(optionList, dCrgCpOptionBorderModeV, dCrgBorderModeExKeep);

  crgOptionSetDouble(optionList, dCrgCpOptionCheckEps, 1e-6);
  crgOptionSetDouble(optionList, dCrgCpOptionCheckInc, 1e-3);
  crgOptionSetDouble(optionList, dCrgCpOptionCheckTol, 1e-4); /* default: 0.1 * dCrgCpOptionCheckInc */
}

void crgOptionSetDefaultModifiers(CrgOptionsStruct* optionList) {
  if (!optionList) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgOptionSetDefaultModifiers: invalid modifier set.\n");
    return;
  }

  /* remove all existing modifiers */
  crgOptionRemoveAll(optionList);

  /* add the default modifiers, one at a time */
  crgOptionSetInt(optionList, dCrgModGridNaNMode, dCrgGridNaNKeepLast);
  crgOptionSetDouble(optionList, dCrgModRefPointX, 0.0);
  crgOptionSetDouble(optionList, dCrgModRefPointY, 0.0);
  crgOptionSetDouble(optionList, dCrgModRefPointZ, 0.0);
  crgOptionSetDouble(optionList, dCrgModRefPointPhi, 0.0);
}

void crgOptionCopyAll(CrgOptionsStruct* dst, CrgOptionsStruct* src) {
  if (!dst || !src) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgOptionCopyAll: have NULL source or destination. Ignoring request.\n");
    return;
  }

  /* --- delete default initialisation --- */
  if (dst->entry) crgFree(dst->entry);

  /* --- first copy administration data --- */
  memcpy(dst, src, sizeof(CrgOptionsStruct));

  /* --- now copy the contents --- */
  dst->entry = (CrgOptionEntryStruct*)crgCalloc(src->noEntries, sizeof(CrgOptionEntryStruct));

  if (!(dst->entry)) {
    crgMsgPrint(dCrgMsgLevelWarn, "crgOptionCopyAll: could not allocate space for options. Ignoring request.\n");
    dst->noEntries = 0;
    return;
  }

  /* --- copy the actual entries --- */
  memcpy(dst->entry, src->entry, src->noEntries * sizeof(CrgOptionEntryStruct));
}

int crgOptionCreateList(CrgOptionsStruct* optionList) {
  if (!optionList) return 0;

  if (optionList->entry) crgFree(optionList->entry);

  optionList->noEntries = 0;
  optionList->entry = (CrgOptionEntryStruct*)crgCalloc(dCrgSizeOptList + 1, sizeof(CrgOptionEntryStruct));

  if (!optionList->entry) return 0;

  optionList->noEntries = dCrgSizeOptList + 1;

  return 1;
}
