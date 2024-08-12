/* ===================================================
 *  file:       crgBaseLib.h
 * ---------------------------------------------------
 *  purpose:	interface definitions for the
 *              OpenCRG base library
 * ---------------------------------------------------
 *  based on routines by Dr. Jochen Rauh, Daimler AG
 * ---------------------------------------------------
 *  first edit:	20.10.2008 by M. Dupuis @ VIRES GmbH
 *  last mod.:  03.04.2015 by H. Helmich @ VIRES GmbH
 * ===================================================
    Copyright 2015 VIRES Simulationstechnologie GmbH

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
#ifndef _CRG_BASELIB_H
#define _CRG_BASELIB_H

/* ====== INCLUSIONS ====== */
#include <stdlib.h>

/* ====== DEFINITIONS ====== */

/* --- CRG debug options --- */
/* #define dCrgEnableDebug2 */

/* --- CRG message levels --- */
#define dCrgMsgLevelNone 0
#define dCrgMsgLevelFatal 1
#define dCrgMsgLevelWarn 2
#define dCrgMsgLevelNotice 3
#define dCrgMsgLevelInfo 4
#define dCrgMsgLevelDebug 5

/**
 * CRG options for data handling and evaluation
 * ATTENTION: IDs MUST NOT overlap with modifier IDs (see dCrgModxxx)
 */
#define dCrgCpOptionBorderModeU \
  1 /* [integer], extrapolation / repeat mode for z values in u direction    [dCrgBorderModexxx] */
#define dCrgCpOptionBorderModeV \
  2 /* [integer], extrapolation / repeat mode for z values in v direction    [dCrgBorderModexxx] */
#define dCrgCpOptionCurvMode \
  3 /* [integer], curvature calculation mode aside the reference line              [dCrgCurvxxx] */
#define dCrgCpOptionBorderOffsetU \
  5 /* [double],  offset of z values when exceeding core area in u direction                 [m] */
#define dCrgCpOptionBorderOffsetV \
  6 /* [double],  offset of z values when exceeding core area in v direction                 [m] */
#define dCrgCpOptionSmoothUBegin \
  7 /* [double],  smoothly blend the data on-set over the given range                            */
    /*            at the begin of the data set                                               [m] */
#define dCrgCpOptionSmoothUEnd \
  8 /* [double],  smoothly blend the data off-set over the given range                           */
    /*            at the end of the data set                                                 [m] */
#define dCrgCpOptionRefLineSearchU \
  9 /* [double],  pre-load history with given u value for search of                              */
    /*            reference line                                                             [m] */
#define dCrgCpOptionRefLineSearchUFrac \
  10 /* [double],  pre-load history with given relative u value for search                        */
     /*            of reference line                                                          [-] */
#define dCrgCpOptionRefLineClose \
  11 /* [double],  near criteria for finding a point on the refline                           [m] */
#define dCrgCpOptionRefLineFar \
  12 /* [double],  far criteria for finding a point on the refline                            [m] */
#define dCrgCpOptionRefLineContinue \
  13 /* [integer], reference line continuation mode outside primary u range      [dCrgRefLinexxx] */
#define dCrgCpOptionWarnMsgs \
  14 /* [integer], maximum number of warning messages                                         [-] */
#define dCrgCpOptionCheckEps \
  15 /* [double],  expected min. accuracy                                                     [m] */
#define dCrgCpOptionCheckInc \
  16 /* [double],  expected min. increment                                                    [m] */
#define dCrgCpOptionCheckTol \
  17 /* [double],  expected abs. tolerance                                                    [m] */

/**
 * Mode definitions for option: dCrgCpOptionBorderModeU
 *                              dCrgCpOptionBorderModeV
 */
#define dCrgBorderModeNone 0   /* refuse computation, i.e. return error          */
#define dCrgBorderModeExZero 1 /* set z value to 0                               */
#define dCrgBorderModeExKeep 2 /* keep last value on border of data set          */ /* default */
#define dCrgBorderModeRepeat 3  /* repeat the data set and perform interpolation  */
#define dCrgBorderModeReflect 4 /* reflect the data set and perform interpolation */

/**
 * Mode definitions for option: dCrgCpOptionRefLineContinue
 */
#define dCrgRefLineExtrapolate 0 /* extrapolate reference line position  */ /* default */
#define dCrgRefLineCloseTrack 1                                             /* try to close the reference line      */

/**
 * Mode definitions for option: dCrgCpOptionCurvMode
 */
#define dCrgCurvLateral 0 /* compute curvature based on lateral position (v)  */ /* default */
#define dCrgCurvRefLine 1 /* keep curvature value on reference line           */

/**
 * CRG modifiers for modification of data sets
 * ATTENTION: IDs MUST NOT overlap with option IDs (see dCrgCpOptionxxx)
 */
#define dCrgModScaleZ 21            /* [double], scale factor for grid data z values                            [-] */
#define dCrgModScaleSlope 22        /* [double], scale factor for slope values                                  [-] */
#define dCrgModScaleBank 23         /* [double], scale factor for bank values                                   [-] */
#define dCrgModScaleLength 24       /* [double], scale factor for bank values                                   [-] */
#define dCrgModScaleWidth 25        /* [double], scale factor for bank values                                   [-] */
#define dCrgModScaleCurvature 26    /* [double], scale the curvature of the reference line                      [-] */
#define dCrgModGridNaNMode 27       /* [int],    mode for treating NaNs in grid values                          [-] */
#define dCrgModGridNaNOffset 28     /* [double], offset applied to former NaN values in grid                    [-] */
#define dCrgModRefPointV 29         /* [double], absolute v position of reference point                         [m] */
#define dCrgModRefPointVFrac 30     /* [double], relative v position of reference point                         [-] */
#define dCrgModRefPointVOffset 31   /* [double], v offset to relative v position of reference point             [m] */
#define dCrgModRefPointU 32         /* [double], absolute u position of reference point                         [m] */
#define dCrgModRefPointUFrac 33     /* [double], relative u position of reference point                         [-] */
#define dCrgModRefPointUOffset 34   /* [double], u offset to relative u position of reference point             [m] */
#define dCrgModRefPointX 35         /* [double], target x position of reference point                           [m] */
#define dCrgModRefPointY 36         /* [double], target y position of reference point                           [m] */
#define dCrgModRefPointZ 37         /* [double], target z position of reference point                           [m] */
#define dCrgModRefPointPhi 38       /* [double], target orientation of reference point                        [rad] */
#define dCrgModRefLineOffsetX 39    /* [double], offset x applied on reference line                             [m] */
#define dCrgModRefLineOffsetY 40    /* [double], offset y applied on reference line                             [m] */
#define dCrgModRefLineOffsetZ 41    /* [double], offset z applied on reference line                             [m] */
#define dCrgModRefLineOffsetPhi 42  /* [double], rotation applied on reference line                           [rad] */
#define dCrgModRefLineRotCenterX 43 /* [double], rotation center on reference line                              [m] */
#define dCrgModRefLineRotCenterY 44 /* [double], rotation center on reference line                              [m] */

/**
 * define size of option / modifier structure for contact point and data set structure
 * NOTE: this must be at least be " 1 + MAX( dCrgCpOptionXXX, dCrgModXXX ) "
 */
#define dCrgSizeOptList 50 /* size of option / modifier list [-] */

/**
 * Mode definitions for modifier: dCrgModGridNaNMode
 */
#define dCrgGridNaNKeep 0                                              /* keep existing NaNs in grid         */
#define dCrgGridNaNSetZero 1                                           /* replace NaNs with zeros            */
#define dCrgGridNaNKeepLast 2 /* keep last valid boundary value     */ /* default */

/**
 * Mode definitions for modifier: dCrgModRefPointOrient
 */
#define dCrgOrientFwd 0 /* forward orientation                */
#define dCrgOrientRev 1 /* reverse orientation                */

/* ====== TYPE DEFINITIONS ====== */

#ifdef __cplusplus
extern "C" {
#endif

/* ====== METHODS in crgMgr.c ====== */
/**
 * destroy the data of the given data set
 * @param dataSetId    identifier of the applicable dataset
 * @return 1 if successful, 0 if failed
 */
extern int crgDataSetRelease(int dataSetId);

/**
 * print information contained in the CRG file's header
 * @param dataSetId    identifier of the applicable dataset
 */
extern void crgDataPrintHeader(int dataSetId);

/**
 * print information about the CRG file's channels
 * @param dataSetId    identifier of the applicable dataset
 */
extern void crgDataPrintChannelInfo(int dataSetId);

/**
 * print information about the CRG road
 * @param dataSetId    identifier of the applicable dataset
 */
extern void crgDataPrintRoadInfo(int dataSetId);

/**
 * get the u co-ordinate range of a CRG data set
 * @param dataSetId    identifier of the applicable dataset
 * @param uMin         return value of minimum u co-ordinate
 * @param uMax         return value of maximum u co-ordinate
 * @return 1 upon success, otherwise 0
 */
extern int crgDataSetGetURange(int dataSetId, double* uMin, double* uMax);

/**
 * get the v co-ordinate range of a CRG data set
 * @param dataSetId    identifier of the applicable dataset
 * @param vMin         return value of minimum v co-ordinate
 * @param vMax         return value of maximum v co-ordinate
 * @return 1 upon success, otherwise 0
 */
extern int crgDataSetGetVRange(int dataSetId, double* vMin, double* vMax);

/**
 * get the u and v co-ordinate increments
 * @param dataSetId    identifier of the applicable dataset
 * @param uInc         return value of u increment
 * @param vInc         return value of v increment (0 if explicit v section have been defined)
 * @return 1 upon success, otherwise 0
 */
extern int crgDataSetGetIncrements(int dataSetId, double* uInc, double* vInc);

/**
 * get closed track utility data
 * @param dataSetId    identifier of the applicable dataset
 * @param uIsClosed    return remember whether reference line can be closed
 * @param uCloseMin    return minimum u of closed reference line (NaN if uIsClosed == 0)
 * @param uCloseMax    return maximum u of closed reference line (NaN if uIsClosed == 0)
 * @return 1 upon success, otherwise 0
 */
extern int crgDataSetGetUtilityDataClosedTrack(const int dataSetId, int* uIsClosed, double* uCloseMin,
                                               double* uCloseMax);

/**
 * set/add an integer value modifier to be applied to the data set
 * CRG data using the indicated data point
 * @param  dataSetId    identifier of the applicable dataset
 * @param  modId        identifier of the modifier which is to be set/added
 *                      (see defines above: dCrgModXXXX)
 * @param  modValue     (new) value of the indicated modifier (may be a mask,
 *                      depending on the type)
 * @return 1 if successful, otherwise 0
 */
extern int crgDataSetModifierSetInt(int dataSetId, unsigned int optionId, int optionValue);

/**
 * set/add a double value option to be applied while handling / evaluating
 * CRG data using the indicated data point
 * @param  dataSetId    identifier of the applicable dataset
 * @param  modId        identifier of the modifier which is to be set/added
 *                      (see defines above: dCrgModXXXX)
 * @param  modValue     (new) value of the indicated modifier
 * @return 1 if successful, otherwise 0
 */
extern int crgDataSetModifierSetDouble(int dataSetId, unsigned int optionId, double optionValue);

/**
 * get the value of an integer option
 * @param  dataSetId    identifier of the applicable dataset
 * @param  modId        identifier of the modifier which is to be set/added
 *                      (see defines above: dCrgModXXXX)
 * @param  modValue     pointer to memory location for return value
 * @return 1 if option is set, otherwise 0
 */
extern int crgDataSetModifierGetInt(int dataSetId, unsigned int optionId, int* modValue);

/**
 * get the value of a double option
 * @param  dataSetId    identifier of the applicable dataset
 * @param  modId        identifier of the modifier which is to be set/added
 *                      (see defines above: dCrgModXXXX)
 * @param  modValue     pointer to memory location for return value
 * @return 1 if option is set, otherwise 0
 */
extern int crgDataSetModifierGetDouble(int dataSetId, unsigned int optionId, double* modValue);

/**
 * remove a modifier from the data set settings's modifier list and apply the
 * corresponding default behavior
 * @param  dataSetId    identifier of the applicable dataset
 * @param  modId        identifier of the modifier which is to be set/added
 *                      (see defines above: dCrgModXXXX)
 * @return 1 if successful, otherwise 0
 */
extern int crgDataSetModifierRemove(int dataSetId, unsigned int modId);

/**
 * remove all modifiers from the data set's modifier list
 * @param  dataSetId    identifier of the applicable dataset
 * @return 1 if successful, otherwise 0
 */
extern int crgDataSetModifierRemoveAll(int dataSetId);

/**
 * print the data set's current set of modifiers
 * @param  dataSetId    identifier of the applicable dataset
 */
extern void crgDataSetModifiersPrint(int dataSetId);

/**
 * apply all defined modifiers once to the given data set and remove them afterwards
 * @param  dataSetId    identifier of the applicable dataset
 */
extern void crgDataSetModifiersApply(int dataSetId);

/**
 * set the default modifiers for a data set
 * @param  dataSetId    identifier of the applicable dataset
 */
extern void crgDataSetModifierSetDefault(int dataSetId);

/**
 * set the default options for a data set; these will be transfered to
 * contact points which are derived from the data set
 * @param  dataSetId    identifier of the applicable dataset
 */
extern void crgDataSetOptionSetDefault(int dataSetId);

/**
 * release all data held by the crg library
 */
extern void crgMemRelease(void);

/**
 * get the release string indicating the current version
 */
extern const char* crgGetReleaseInfo(void);

/**
 * wrap the isnan() functionality
 * @param dValue	the double value which is to be checked for NaN
 * @return 1 if value is NaN
 */
extern int crgIsNan(double* dValue);

/**
 * wrap the isnan() functionality
 * @param fValue	the float value which is to be checked for NaN
 * @return 1 if value is NaN
 */
extern int crgIsNanf(float* dValue);

/* ====== METHODS in crgMsg.c ====== */
/**
 * set the maximum level of messages that will be handled,
 * example:  if set to "sCrgMsgLevelNotice", messages of the levels
 *           sCrgMsgLevelWarn and  sCrgMsgLevelFatal will be handled
 * @param level  new value for maximum handled criticality
 */
extern void crgMsgSetLevel(int level);

/**
 * set the maximum number of warning / debug messages to be handled
 * @param maxNo  maximum number of messages to be handled (-1 for unlimited)
 */
extern void crgMsgSetMaxWarnMsgs(int maxNo);

/**
 * set the maximum number of log messages to be handled
 * @param maxNo  maximum number of messages to be handled (-1 for unlimited)
 */
extern void crgMsgSetMaxLogMsgs(int maxNo);

/**
 * query whether messages of a certain level are to be printed (this may
 * change during runtime due to a restriction in the number of messages
 * that may be printed)
 * @param level  the desired level
 * @return 0 if message of the given level may be printed
 */
extern int crgMsgIsPrintable(int level);

/* ====== METHODS in crgLoader.c ====== */
/**
 * check CRG data for consistency and accuracy
 * @param  dataSetId    identifier of the applicable dataset
 * @return true         if crgData is valid
 */
extern int crgCheck(int dataSetId);

/**
 * method for loading CRG data from an existing IPL-formatted file
 * @param filename   full filename of the CRG input file including path
 * @return identifier of the resulting data set or 0 if not successful
 */
extern int crgLoaderReadFile(const char* filename);

/* ====== METHODS in crgContactPoint.c ====== */
/**
 * create a new contact point working on the indicated data set
 * @param  dataSetId index of data set which is to be used
 * @return id of new contact point or -1 if error occurred
 */
extern int crgContactPointCreate(int dataSetId);

/**
 * delete a contact point and its associated data (not: crgData)
 * @param  cpId id of the contact point which is to be deleted
 * @return 1 if successful, otherwise 0
 */
extern int crgContactPointDelete(int cpId);

/**
 * delete all contact points and associated data for a given data set (not: crgData)
 * @param dataSetId  the ID of the data set for which to delete
 *                   the contact points; if ID=-1, then all contact points
 *                   of all data sets will be deleted
 */
extern void crgContactPointDeleteAll(int dataSetId);

/**
 * set/add an integer value option to be applied while handling / evaluating
 * CRG data using the indicated data point
 * @param  cpId         id of the contact point which is to be configured
 * @param  optionId     identifier of the option which is to be set/added
 *                      (see defines above: dCrgCpOptionXXXX)
 * @param  optionValue  (new) value of the indicated option (may be a mask,
 *                      depending on the option type)
 * @return 1 if successful, otherwise 0
 */
extern int crgContactPointOptionSetInt(int cpId, unsigned int optionId, int optionValue);

/**
 * set/add a double value option to be applied while handling / evaluating
 * CRG data using the indicated data point
 * @param  cpId         id of the contact point which is to be configured
 * @param  optionId     identifier of the option which is to be set/added
 *                      (see defines above: dCrgCpOptionXXXX)
 * @param  optionValue  (new) value of the indicated option
 * @return 1 if successful, otherwise 0
 */
extern int crgContactPointOptionSetDouble(int cpId, unsigned int optionId, double optionValue);

/**
 * get the value of an integer option
 * @param  cpId         id of the contact point which is to be used
 * @param  optionId     identifier of the option whose value shall be queried
 *                      (see defines above: dCrgCpOptionXXXX)
 * @param  optionValue  pointer to memory location for return value
 * @return 1 if option is set, otherwise 0
 */
extern int crgContactPointOptionGetInt(int cpId, unsigned int optionId, int* optionValue);

/**
 * get the value of a double option
 * @param  cpId         id of the contact point which is to be used
 * @param  optionId     identifier of the option whose value shall be queried
 *                      (see defines above: dCrgCpOptionXXXX)
 * @param  optionValue  pointer to memory location for return value
 * @return 1 if option is set, otherwise 0
 */
extern int crgContactPointOptionGetDouble(int cpId, unsigned int optionId, double* optionValue);

/**
 * remove an option from the contact point's option settings and apply the
 * corresponding default behavior
 * @param  cpId         id of the contact point which is to be configured
 * @param  optionId     identifier of the option which is to be set/added
 *                      (see defines above: dCrgCpOptionXXXX)
 * @return 1 if successful, otherwise 0
 */
extern int crgContactPointOptionRemove(int cpId, unsigned int optionId);

/**
 * remove all options from the contact point's option settings
 * @param  cpId         id of the contact point which is to be configured
 * @return 1 if successful, otherwise 0
 */
extern int crgContactPointOptionRemoveAll(int cpId);

/**
 * print the contact point's current set of options
 * @param  cpId id of the contact point whose options are to be printed
 */
extern void crgContactPointOptionsPrint(int cpId);

/**
 * set the default options to be applied when using a contact point for data
 * evaluation etc.
 * @param  cpId index of the contact point which is to be modified
 */
extern void crgContactPointSetDefaultOptions(int cpId);

/**
 * set the size of a contact point's history
 * @param  cpId         index of the contact point which is to be modified
 * @param  histSize     new size of history stack
 * @return 1 if successful, otherwise 0
 */
extern int crgContactPointSetHistory(int cpId, int histSize);

/* ====== METHODS in crgEvalxy2uv.c ====== */
/**
 * convert a given (x,y) position into the corresponding (u,v) position
 * @param cpId  id of the contact point to use for the query
 * @param x     x co-ordinate
 * @param y     y co-ordinate
 * @param u     pointer to resulting u co-ordinate
 * @param v     pointer to resulting v co-ordinate
 * @return 1 if successful, otherwise 0
 */
extern int crgEvalxy2uv(int cpId, double x, double y, double* u, double* v);

/* ====== METHODS in crgEvaluv2xy.c ====== */
/**
 * convert a given (u,v) position into the corresponding (x,y) position
 * @param cpId  id of the contact point to use for the query
 * @param u     u co-ordinate
 * @param v     v co-ordinate
 * @param x     pointer to resulting x co-ordinate
 * @param y     pointer to resulting y co-ordinate
 * @return 1 if successful, otherwise 0
 */
extern int crgEvaluv2xy(int cpId, double u, double v, double* x, double* y);

/* ====== METHODS in crgEvalz.c ====== */
/**
 * compute the z value at a given (u,v) position using bilinear interpolation
 * @param cpId  id of the contact point to use for the query
 * @param u     u co-ordinate
 * @param v     v co-ordinate
 * @param z     pointer to resulting z co-ordinate
 * @return 1 if successful, otherwise 0
 */
extern int crgEvaluv2z(int cpId, double u, double v, double* z);

/**
 * compute the z value at a given (x,y) position using bilinear interpolation
 * @param cpId  id of the contact point to use for the query
 * @param x     x co-ordinate
 * @param y     y co-ordinate
 * @param z     pointer to resulting z co-ordinate
 * @return 1 if successful, otherwise 0
 */
extern int crgEvalxy2z(int cpId, double x, double y, double* z);

/* ====== METHODS in crgEvalpk.c ====== */
/**
 * compute the heading and curvature value at a given (u,v) position and store
 * it in the contact point structure
 * @param cpId  id of the contact point to use for the query
 * @param u     u co-ordinate
 * @param v     v co-ordinate
 * @param phi   pointer to resulting heading angle
 * @param curv  pointer to resulting curvature
 * @return 1 if successful, otherwise 0
 */
extern int crgEvaluv2pk(int cpId, double u, double v, double* phi, double* curv);

/**
 * compute the heading and curvature value at a given (x,y) position and store
 * it in the contact point structure
 * @param cpId  id of the contact point to use for the query
 * @param x     x co-ordinate
 * @param y     u co-ordinate
 * @param phi   pointer to resulting heading angle
 * @param curv  pointer to resulting curvature
 * @return 1 if successful, otherwise 0
 */
extern int crgEvalxy2pk(int cpId, double x, double y, double* phi, double* curv);

/* ====== METHODS in crgPortability.c ====== */
/**
 * print a message with a defined criticality level
 * @param level  criticality of the message
 * @param format variable argument list as for standard printf
 */
extern void crgMsgPrint(int level, const char* format, ...);

/**
 * allocate some space from memory, usually equal to calloc()
 * @param nmemb  number of members to allocate
 * @param size   size of a single member
 * @return pointer to allocated memory location
 */
extern void* crgCalloc(size_t nmemb, size_t size);

/**
 * set a user-defined callback method to allocate some space from memory
 * @param func   pointer to the user-defined method
 * @param nmemb  number of members to allocate
 * @param size   size of a single member
 * @return pointer to allocated memory location
 */
extern void crgCallocSetCallback(void* (*func)(size_t nmemb, size_t size));

/**
 * re-allocate space from memory, usually equal to realloc()
 * @param ptr    pointer to existing memory location
 * @param size   size of a single member
 * @return pointer to allocated memory location
 */
extern void* crgRealloc(void* ptr, size_t size);

/**
 * set a user-defined callback method to re-allocate space from memory,
 * @param func   pointer to the user-defined method
 * @param ptr    pointer to existing memory location
 * @param size   size of a single member
 * @return pointer to allocated memory location
 */
extern void crgReallocSetCallback(void* (*func)(void* ptr, size_t size));

/**
 * free previously allocated space from memory, usually equal to free()
 * @param ptr    pointer to existing memory location
 */
extern void crgFree(void* ptr);

/**
 * set a user-defined callback method to free previously allocated space from memory,
 * @param func   pointer to the user-defined method
 * @param ptr    pointer to existing memory location
 */
extern void crgFreeSetCallback(void (*func)(void* ptr));

/**
 * set a user-defined callback method that should be called for all messages
 * instead of printing them to the console
 * @param func   pointer to the user-defined method accepting message level and text
 */
extern void crgMsgSetCallback(int (*func)(int level, char* message));

#ifdef __cplusplus
}
#endif

#endif /* _CRG_BASELIB_H */
