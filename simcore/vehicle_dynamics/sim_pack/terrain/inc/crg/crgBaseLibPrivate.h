/* ===================================================
 *  file:       crgBaseLibPrivate.h
 * ---------------------------------------------------
 *  purpose:	interface definitions for the
 *              OpenCRG base library;
 *              this is the part with the more or
 *              less private definitions used
 *              internally by the CRG routines
 * ---------------------------------------------------
 *  based on routines by Dr. Jochen Rauh, Daimler AG
 * ---------------------------------------------------
 *  first edit:	03.12.2008 by M. Dupuis @ VIRES GmbH
 *  last mod.:  08.04.2014 by H. Helmich @ VIRES GmbH
 * ===================================================
    Copyright 2014 VIRES Simulationstechnologie GmbH

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
#ifndef _CRG_BASELIB_PRIVATE_H
#define _CRG_BASELIB_PRIVATE_H

/* ====== INCLUSIONS ====== */
/* include the public part */
#include "crgBaseLib.h"

/* ====== DEFINITIONS ====== */

/**
 * enable CRG statistics calculations?
 */
/* #define dCrgEnableStats */

/**
 * CRG history, default size
 */
#define dCrgHistoryStdSize 50

/**
 * CRG v index table, default size
 */
#define dCrgVTableStdSize 200

/**
 * CRG options data type
 */
#define dCrgOptionDataTypeInt 0
#define dCrgOptionDataTypeDouble 1

/**
 * maximum tolerated relative error
 */
#define dCrgEps 1.0e-6

/**
 * limit for determination whether lines are parallel
 */
#define dCrgParallelLimit 1.0e-6

/**
 * data definitions for the loader and for modificators
 */
#define dCrgDataDefNone 0x0000 /* this was wrong in 0.3 */
#define dCrgDataDefXEnd 0x0001
#define dCrgDataDefYEnd 0x0002
#define dCrgDataDefVPos 0x0004
#define dCrgDataDefVIndex 0x0008
#define dCrgDataDefSlopeEnd 0x0010
#define dCrgDataDefBankEnd 0x0020
#define dCrgDataDefZEnd 0x0040
#define dCrgDataDefZStart 0x0080

/* ====== TYPE DEFINITIONS ====== */
/**
 * this structure stores administrative information about a single CRG file
 */
typedef struct {
  int id;               /* id of the data set                             [-] */
  int dataFormat;       /* the format of the data in the CRG file         [-] */
  char* fileBuffer;     /* buffer for CRG file data                       [-] */
  double* recordBuffer; /* buffer for a single data record (n channels)   [-] */
  char* dataSection;    /* pointer to the data section                    [-] */
  size_t dataSize;      /* size of the data section                    [byte] */
  int defMask;          /* mask of defined data in header section         [-] */
  size_t recordSize;    /* size of a single data record                [byte] */
  int sectionType;      /* temporarily used while reading file            [-] */
} CrgAdminStruct;

/**
 * this structure stores statistical information about a single CRG file
 */
typedef struct {
  double zMin;        /* minimum z value                                  [m] */
  double zMax;        /* maximum z value                                  [m] */
  double zMeanBeg;    /* mean z value at beginning                        [m] */
  double zMeanEnd;    /* mean z value at end                              [m] */
  int uIsClosed;      /* remember whether reference line can be closed    [-] */
  double uCloseMin;   /* minimum u of closed reference line               [m] */
  double uCloseMax;   /* maximum u of closed reference line               [m] */
  int hasBank;        /* is any banking information available?            [-] */
  double phiFirstSin; /* sine   of first phi value                        [-] */
  double phiFirstCos; /* cosine of first phi value                        [-] */
  double phiLastSin;  /* sine   of last phi value                         [-] */
  double phiLastCos;  /* cosine of last phi value                         [-] */
} CrgUtilityStruct;

/**
 * this structure stores the information about a single CRG channel
 */
typedef struct {
  char valid;      /* validity of the channel                         [0/1] */
  char defined;    /* flag whether channel is defined in file         [0/1] */
  char singlePrec; /* flag whether channel is single precision        [0/1] */
  char spare;      /* a spare for future use                            [-] */
  size_t index;    /* index of the channel                              [-] */
  size_t size;     /* number of entries in the channel                  [-] */
  double first;    /* first (minimum) value in this channel             [m] */
  double last;     /* last (maximum) value in this channel              [m] */
  double inc;      /* increment between two values in this channel      [m] */
  double mean;     /* mean value used for normalizing a channel         [m] */
} CrgChannelInfoStruct;

typedef struct /* channel for double precision data */
{
  CrgChannelInfoStruct info; /* common channel information                      [0/1] */
  double* data;              /* the data of the channel, dynamically allocated    [m] */
} CrgChannelStruct;

typedef struct /* channel for single precision data */
{
  CrgChannelInfoStruct info; /* common channel information                      [0/1] */
  float* data;               /* the data of the channel, dynamically allocated    [m] */
} CrgChannelFStruct;

typedef struct /* channel for single precision data */
{
  CrgChannelInfoStruct info; /* common channel information                      [0/1] */
  void* data;                /* the data of the channel, dynamically allocated        */
} CrgChannelBaseStruct;

/**
 * structure of a single entry in the history
 */
typedef struct {
  double x;     /* inertial x position                                    [m] */
  double y;     /* inertial y position                                    [m] */
  size_t index; /* index of the inertial position in x/y data channels    [-] */
} CrgHistoryEntryStruct;

/**
 * structure for statistics about history use etc.
 */
typedef struct {
  short active;                /* is history statistics active?          [0/1] */
  unsigned int noTotalQueries; /* total number of queries to history       [-] */
  unsigned int noCloseHits;    /* total hits in close distance             [-] */
  unsigned int noFarHits;      /* total hits in far distance               [-] */
  unsigned int noNoHits;       /* total tests without hit                  [-] */
  unsigned int noIter;         /* total number of iterations in history    [-] */
  unsigned int noCallsLoop1;   /* total number of calls to loop 1          [-] */
  unsigned int noCallsLoop2;   /* total number of calls to loop 2          [-] */
} CrgHistoryStatStruct;

/**
 * structure for information about query history for faster access
 */
typedef struct {
  int totalSize;                /* total size of the history                                      [-] */
  int usedSize;                 /* size of valid data in the history                              [-] */
  int entrySize;                /* size of a history entry                                     [byte] */
  double closeDist;             /* square of a distance considered 'close' to a point in history [m2] */
  double farDist;               /* square of a distance considered 'far' to a point in history   [m2] */
  CrgHistoryEntryStruct* entry; /* entries of the history, dynamically allocated                  [-] */
  CrgHistoryStatStruct stat;    /* statistics information about history use                       [-] */
} CrgHistoryStruct;

/**
 * structure for performance measurement data
 */
typedef struct {
  short active;                /* is performance statistics active?          [0/1] */
  unsigned int noTotalQueries; /* total number of queries to history           [-] */
  unsigned int noCallsLoopV1;  /* total number of calls to a given loop        [-] */
  unsigned int maxCallsLoopV1; /* maximum number of calls to a given loop      [-] */
  unsigned int noCallsBorderU; /* total number of calls to borderU mode        [-] */
  unsigned int noCallsBorderV; /* total number of calls to borderV mode        [-] */
} CrgPerformanceStruct;

/**
 * a structure holding settings for one option
 * @todo: maybe, we need generic option data for various types, currently only double and int are foreseen
 */
typedef struct {
  unsigned int id;       /* option identifier                                              [-] */
  unsigned int dataType; /* option data type                           [dCrgOptionDataTypeXXX] */
  double dValue;         /* double value of the option, if applicable                      [-] */
  int iValue;            /* integer value of the option, if applicable                     [-] */
  int valid;             /* validity of the option                                         [-] */
} CrgOptionEntryStruct;

/**
 * a structure holding all option settings for data evaluation etc.
 */
typedef struct {
  unsigned int noEntries;      /* number of available options (size of option entry list)        [-] */
  CrgOptionEntryStruct* entry; /* list of option entries                                         [-] */
} CrgOptionsStruct;

/**
 * a structure for an index table of a non-equally spaced data series
 */
typedef struct {
  short valid;                      /* validity of the index table                                      [0/1] */
  double minVal;                    /* minimum physical value represented in index table                  [-] */
  double maxVal;                    /* maximum physical value represented in index table                  [-] */
  double range;                     /* range of physical values                                           [-] */
  size_t size;                      /* size of the table                                                  [-] */
  size_t refIdx[dCrgVTableStdSize]; /* the index table itself                                             [-] */
} CrgIndexTable;

/**
 * now the complete structure composed of the previous sub-structures
 */
typedef struct {
  CrgAdminStruct admin;          /* administrative data                                                              */
  size_t noChannels;             /* total number of available channels in crg data                               [-] */
  CrgChannelStruct channelV;     /* channels holding v data                                                          */
  CrgChannelStruct channelX;     /* channel holding x data of reference line                                         */
  CrgChannelStruct channelY;     /* channel holding y data of reference line                                         */
  CrgChannelStruct channelU;     /* channel holding u data of reference line                                         */
  CrgChannelStruct channelPhi;   /* channel holding phi data of reference line                                       */
  CrgChannelStruct channelSlope; /* channel holding sloope data of reference line                                    */
  CrgChannelStruct channelBank;  /* channel holding bank data of reference line                                      */
  CrgChannelStruct channelRefZ;  /* channel holding z data of reference line                                         */
  CrgChannelFStruct* channelZ;   /* channels holding the actual z data, dynamically allocated, single precision      */
  CrgOptionsStruct modifiers;    /* list of modifiers to be applied on the data set                              [-] */
  CrgOptionsStruct options;      /* list of default options for new contact points                               [-] */
  CrgUtilityStruct util;         /* utility information, also used for increased performance                     [-] */
  CrgPerformanceStruct perfStat; /* data for performance statistics                                              [-] */
  CrgIndexTable indexTableV;     /* an index table for faster access to v indices in irregularly spaced v grids  [-] */
} CrgDataStruct;

/**
 * a structure holding contact point information (and providing additional memory for queries)
 */
typedef struct {
  double x;                        /* inertial x position                                             [m] */
  double y;                        /* inertial y position                                             [m] */
  double u;                        /* local u position                                                [m] */
  double v;                        /* local v position                                                [m] */
  double z;                        /* data value (typically: elevation) at the given position         [m] */
  double phi;                      /* heading at the given position                                 [rad] */
  double curv;                     /* curvature at the given position                               [1/m] */
  CrgDataStruct* crgData;          /* pointer to the CRG data on which contact point is working           */
  int useLocalHistory;             /* use local history of contact point instead of global one      [0/1] */
  CrgHistoryEntryStruct histEntry; /* information about the previous query                                */
  CrgOptionsStruct options;        /* list of options to be applied when using the contact point      [-] */
  CrgHistoryStruct history;        /* history for successive queries                                  [-] */
  double smoothBaseBeg;            /* base value for smoothing at the begin of the data set           [m] */
  double smoothBaseEnd;            /* base value for smoothing at the end of the data set             [m] */
} CrgContactPointStruct;

/**
 * unions for retrieving the nice NaNs
 */
typedef union {
  int iVal[2];
  double dVal;
} CrgNanUnionDouble;

typedef union {
  int iVal;
  float fVal;
} CrgNanUnionFloat;

/* ====== GLOBAL VARIABLES ====== */
extern int mCrgBigEndian; /* endian-ness of machine */

/* ====== METHODS in crgLoader.c ====== */
/**
 * handle NaNs in the data set according to the specified options
 * @param  crgData     pointer to the CRG data set which is to be altered
 * @param  mode        NaN handling mode:
 *                       0 = keep NaN
 *                       1 = set to 0 + offset
 *                       2 = set to last value + offset
 * @param  offset      offset to be applied to former NaNs when converting them
 *                     according to mode 1 or 2
 */
extern void crgLoaderHandleNaNs(CrgDataStruct* crgData, int mode, double offset);

/**
 * prepare the data read from file
 * @param  crgData     pointer to the CRG data set which is to be altered
 */
extern void crgLoaderPrepareData(CrgDataStruct* crgData);

/**
 * check CRG option settings for consistency and accuracy
 * @return true        if crgData is valid
 */
extern int crgCheckOpts(CrgDataStruct* crgData);

/**
 * check CRG modifier settings for consistency and accuracy
 * @return true        if crgData is valid
 */
extern int crgCheckMods(CrgDataStruct* crgData);

/* ====== METHODS in crgStatistics.c ====== */
/**
 * calculate some statistics data for a given CRG data set
 * @param crgData    pointer to data set which is to be analyzed
 */
extern void crgCalcStatistics(CrgDataStruct* crgData);

/**
 * calculate some utility data for a given CRG data set; by this, the
 * performance of evaluations shall be increased
 * @param crgData    pointer to data set which is to be analyzed
 */
extern void crgCalcUtilityData(CrgDataStruct* crgData);

/**
 * print the elevation data contained in a CRG file to shell
 * @param crgData    pointer to data set which is to be printed
 */
extern void crgPrintElevData(CrgDataStruct* crgData);

/* ====== METHODS in crgMgr.c ====== */
/**
 * create a new data set for storing CRG data in it
 * @return pointer to a new data set
 */
extern CrgDataStruct* crgDataSetCreate(void);

/**
 * get access to a data set with a certain ID
 * @return pointer to the data set or NULL if ID wasn't found
 */
extern CrgDataStruct* crgDataSetAccess(int id);

/**
 * set the size of a data-set specific history
 * @param dataSetId    ID of the applicable data set
 * @param histSize     new size of history stack
 * @return 1 if successful, otherwise 0
 */
extern int crgDataSetHistory(int dataSetId, int histSize);

/**
 * set a given double variable to NaN
 * @param dValue pointer to the variable that is to be set
 */
void crgSetNan(double* dValue);

/**
 * set a given float variable to NaN
 * @param fValue pointer to the variable that is to be set
 */
void crgSetNanf(float* fValue);

/**
 * build the v index table of a data set
 * @param dataSetId    ID of the applicable data set
 */
void crgDataSetBuildVTable(int dataSetId);

/* ====== METHODS in crgMsg.c ====== */
/**
 * get the currently set maximum level of messages
 * @return current criticality level
 */
extern int crgMsgGetLevel(void);

/**
 * get the name of a criticality level
 * @param level  level whose name is requested
 * @return corresponding name as a character string
 */
extern const char* crgMsgGetLevelName(int level);

/* ====== METHODS in crgContactPoint.c ====== */
/**
 * get the pointer to a contact point from a given contact point ID
 * @param  cpId index of the contact point
 * @return pointer to contact point or NULL if wrong index was defined
 */
extern CrgContactPointStruct* crgContactPointGetFromId(int cpId);

/**
 * reset memory and temporary variables within a contact point structure
 * @param  cp  pointer to the contact point which is to be modified
 */
extern void crgContactPointReset(CrgContactPointStruct* cp);

/**
 * change the data set on which a contact point is supposed to work
 * @param  cp  pointer to the contact point which is to be modified
 * @param  dataSetId  index of data set which is to be used
 * @return 1 if successful, otherwise 0
 */
extern int crgContactPointSetCrgData(CrgContactPointStruct* cp, int dataSetId);

/**
 * check whether a given option is available and valid
 * @param  cp  pointer to the contact point which is to be checked
 * @param  optionId  id of the option which is to be queried
 * @return 1 if option is available and valid, otherwise 0
 */
extern int crgContactPointOptionIsSet(CrgContactPointStruct* cp, unsigned int optionId);

/**
 * check whether a given integer option is available, valid and set to a given value
 * @param  cp           pointer to the contact point which is to be checked
 * @param  optionId     id of the option which is to be queried
 * @param  optionValue  desired value of the indicated option
 * @return 1 if option is available and valid and has the given value, otherwise 0
 */
extern int crgContactPointOptionHasValueInt(CrgContactPointStruct* cp, unsigned int optionId, int optionValue);

/**
 * set the size and basic parameters of a contact point's history
 * @param  cp          pointer to the contact point which is to be modified
 * @param histSize     new size of history stack
 * @return 1 if successful, otherwise 0
 */
extern int crgContactPointPtrSetHistory(CrgContactPointStruct* cp, int histSize);

/**
 * set the size and basic parameters of all contact points referring to a
 * given CRG data set
 * @param  crgData     pointer to the applicable CRG data set
 * @param histSize     new size of history stack
 * @return 1 if successful, otherwise 0
 */
extern int crgContactPointSetHistoryForDataSet(CrgDataStruct* crgData, int histSize);

/**
 * pre-load reference line history with data at given u value
 * @param cp         pointer to the contact point which is to be modified
 * @param u          absolute u value to be used for pre-loading
 */
extern void crgContactPointPreloadHistoryU(CrgContactPointStruct* cp, double u);

/**
 * pre-load reference line history with data at given relative u value
 * @paramcp          pointer to the contact point which is to be modified
 * @param uFrac      relative u value to be used for pre-loading
 */
extern void crgContactPointPreloadHistoryUFrac(CrgContactPointStruct* cp, double uFrac);

/**
 * activate the performance statistics
 * @param  cpId  id of the contact point whose statistics is to be activated
 */
extern void crgContactPointActivatePerfStat(int cpId);

/**
 * deactivate the performance statistics
 * @param  cpId  id of the contact point whose statistics is to be deactivated
 */
extern void crgContactPointDeActivatePerfStat(int cpId);

/**
 * reset the performance statistics
 * @param  cp  pointer to the contact point which is to be modified
 */
extern void crgContactPointResetPerfStat(CrgContactPointStruct* cp);

/**
 * print the performance statistics
 * @param  cpId  id of the contact point whose information is to be printed
 */
extern void crgContactPointPrintPerfStat(int cpId);

/**
 * print the history of a contact oint
 * @param  cp  pointer to the contact point whose history is to be shown
 * @param  x   x position for which to compute distance to points in history
 * @param  y   y position for which to compute distance to points in history
 */
extern void crgContactPointPrintHistory(CrgContactPointStruct* cp, double x, double y);

/* ====== METHODS in crgOptionMgmt.c ====== */
/**
 * returns the name of an option as a string
 * @param id         numeric identifier of the option
 * @return name of the option or "unknown"
 */
extern const char* crgOptionGetName(unsigned int optionId);

/**
 * returns the type of an option as integer value (see symbolic constants)
 * @param id         numeric identifier of the option
 * @return type of the option (fallback: integer option)
 */
extern int crgOptionGetType(unsigned int optionId);

/**
 * set/add an integer value option
 * @param  optionList   pointer to a list holding all applicable options
 * @param  optionId     identifier of the option which is to be set/added
 *                      (see defines above: dCrgCpOptionXXXX)
 * @param  optionValue  (new) value of the indicated option (may be a mask,
 *                      depending on the option type)
 * @return 1 if successful, otherwise 0
 */
extern int crgOptionSetInt(CrgOptionsStruct* optionList, unsigned int optionId, int optionValue);

/**
 * set/add a double value option
 * @param  optionList   pointer to a list holding all applicable options
 * @param  optionId     identifier of the option which is to be set/added
 *                      (see defines above: dCrgCpOptionXXXX)
 * @param  optionValue  (new) value of the indicated option
 * @return 1 if successful, otherwise 0
 */
extern int crgOptionSetDouble(CrgOptionsStruct* optionList, unsigned int optionId, double optionValue);

/**
 * get the value of an integer option
 * @param  optionList   pointer to a list holding all applicable options
 * @param  optionId     identifier of the option whose value shall be queried
 *                      (see defines above: dCrgCpOptionXXXX)
 * @param  optionValue  pointer to memory location for return value
 * @return 1 if option is set, otherwise 0
 */
extern int crgOptionGetInt(CrgOptionsStruct* optionList, unsigned int optionId, int* optionValue);

/**
 * get the value of a double option
 * @param  optionList   pointer to a list holding all applicable options
 * @param  optionId     identifier of the option whose value shall be queried
 *                      (see defines above: dCrgCpOptionXXXX)
 * @param  optionValue  pointer to memory location for return value
 * @return 1 if option is set, otherwise 0
 */
extern int crgOptionGetDouble(CrgOptionsStruct* optionList, unsigned int optionId, double* optionValue);

/**
 * remove an option from the respective option list
 * @param  optionList   pointer to a list holding all applicable options
 * @param  optionId     identifier of the option which is to be set/added
 *                      (see defines above: dCrgCpOptionXXXX)
 * @return 1 if successful, otherwise 0
 */
extern int crgOptionRemove(CrgOptionsStruct* optionList, unsigned int optionId);

/**
 * remove all options from the respective option list
 * @param  optionList   pointer to a list holding all applicable options
 * @return 1 if successful, otherwise 0
 */
extern int crgOptionRemoveAll(CrgOptionsStruct* optionList);

/**
 * print the option list's current set of options
 * @param  optionList  pointer to a list holding all applicable options
 * @param  label       label to be used for printed content
 */
extern void crgOptionsPrint(CrgOptionsStruct* optionList, const char* label);

/**
 * check whether a given option is available and valid
 * @param  optionList  pointer to a list holding all applicable options
 * @param  optionId    id of the option which is to be queried
 * @return 1 if option is available and valid, otherwise 0
 */
extern int crgOptionIsSet(CrgOptionsStruct* optionList, unsigned int optionId);

/**
 * check whether a given integer option is available, valid and set to a given value
 * @param  optionList   pointer to a list holding all applicable options
 * @param  optionId     id of the option which is to be queried
 * @param  optionValue  desired value of the indicated option
 * @return 1 if option is available and valid and has the given value, otherwise 0
 */
extern int crgOptionHasValueInt(CrgOptionsStruct* optionList, unsigned int optionId, int optionValue);

/**
 * set the default options to be applied when using a contact point for data
 * evaluation etc.
 * @param  optionList   pointer to a list holding all applicable options
 */
extern void crgOptionSetDefaultOptions(CrgOptionsStruct* optionList);

/**
 * set the default modifiers to be applied when using crg data
 * @param  optionList   pointer to a list holding all applicable modifiers
 */
extern void crgOptionSetDefaultModifiers(CrgOptionsStruct* optionList);

/**
 * copy options from one list to another
 * @param  dst   destination of options
 * @param  src   source of options
 */
extern void crgOptionCopyAll(CrgOptionsStruct* dst, CrgOptionsStruct* src);

/**
 * create an option list; this will delete an existing list and
 * re-allocate the required space
 * @param  optionList   pointer to the list whose entries shall be allocated
 * @return 1 if successful, otherwise 0
 */
extern int crgOptionCreateList(CrgOptionsStruct* optionList);

/* ====== METHODS in crgEvalxy2uv.c ====== */
/**
 * convert a given (x,y) position into the corresponding (u,v) position
 * @param cp    pointer to contact point which is to be used
 * @param x     x co-ordinate
 * @param y     y co-ordinate
 * @param u     pointer to resulting u co-ordinate
 * @param v     pointer to resulting v co-ordinate
 * @return 1 if successful, otherwise 0
 */
extern int crgEvalxy2uvPtr(CrgContactPointStruct* cp, double x, double y, double* u, double* v);

/**
 * depending on reference line settings (i.e. closing of reference line),
 * this routine will clip an incoming u value to the valid range or leave
 * it unmodified
 * @param crgData    pointer to the CRG data set
 * @param optionList pointer to the applicable options
 * @param u          the u value (may be modified)
 * @return 1 if u-value has been modified, otherwise 0; error = -1
 */
extern int crgEvalu2uvalid(CrgDataStruct* crgData, CrgOptionsStruct* optionList, double* u);

/* ====== METHODS in crgEvaluv2xy.c ====== */
/**
 * convert a given (u,v) position into the corresponding (x,y) position
 * @param crgData    pointer to data set which holds the data
 * @param optionList pointer to a list holding all applicable options
 * @param u          u co-ordinate
 * @param v          v co-ordinate
 * @param x          pointer to resulting x co-ordinate
 * @param y          pointer to resulting y co-ordinate
 * @return 1 if successful, otherwise 0
 */
extern int crgDataEvaluv2xy(CrgDataStruct* crgData, CrgOptionsStruct* optionList, double u, double v, double* x,
                            double* y);

/* ====== METHODS in crgEvalz.c ====== */
/**
 * compute the z value at a given (u,v) position using bilinear interpolation
 * @param crgData    pointer to data set which holds the data
 * @param optionList pointer to a list holding all applicable options
 * @param u          u co-ordinate
 * @param v          v co-ordinate
 * @param z          pointer to resulting z co-ordinate
 * @return 1 if successful, otherwise 0
 */
extern int crgDataEvaluv2z(CrgDataStruct* crgData, CrgOptionsStruct* optionList, double u, double v, double* z);

/**
 * compute the z value at a given (u,v) position using bilinear interpolation
 * @param cp    pointer to contact point which is to be used
 * @param u     u co-ordinate
 * @param v     v co-ordinate
 * @param z     pointer to resulting z co-ordinate
 * @return 1 if successful, otherwise 0
 */
extern int crgEvaluv2zPtr(CrgContactPointStruct* cp, double u, double v, double* z);

/**
 * compute the z value of reference line at given u position
 * @param crgData  pointer to data set which holds the data
 * @param u     u co-ordinate
 * @param z     pointer to resulting z co-ordinate
 * @return 1 if successful, otherwise 0
 */
extern int crgDataEvalu2Refz(CrgDataStruct* crgData, double u, double* z);

/* ====== METHODS in crgEvalpk.c ====== */
/**
 * compute the heading and curvature value at a given (u,v) position
 * @param crgData    pointer to data set which holds the data
 * @param optionList pointer to a list holding all applicable options
 * @param u          u co-ordinate
 * @param v          v co-ordinate
 * @param phi        pointer to resulting heading angle
 * @param curv       pointer to resulting curvature
 * @return 1 if successful, otherwise 0
 */
extern int crgDataEvaluv2pk(CrgDataStruct* crgData, CrgOptionsStruct* optionList, double u, double v, double* phi,
                            double* curv);

/* ====== METHODS in crgPortability.c ====== */
/**
 * set the maximum level of messages that will be handled,
 * example:  if set to "sCrgMsgLevelNotice", messages of the levels
 *           sCrgMsgLevelWarn and  sCrgMsgLevelFatal will be handled
 * @param level  new value for maximum handled criticality
 */
extern void crgPortSetMsgLevel(int level);

/**
 * set the maximum number of warning / debug messages to be handled
 * @param maxNo  maximum number of messages to be handled (-1 for unlimited)
 */
extern void crgPortSetMaxWarnMsgs(int maxNo);

/**
 * set the maximum number of log messages to be handled
 * @param maxNo  maximum number of messages to be handled (-1 for unlimited)
 */
extern void crgPortSetMaxLogMsgs(int maxNo);

/**
 * query whether messages of a certain level are to be printed (this may
 * change during runtime due to a restriction in the number of messages
 * that may be printed)
 * @param level  the desired level
 * @return 0 if message of the given level may be printed
 */
extern int crgPortMsgIsPrintable(int level);

#endif /* _CRG_BASELIB_PRIVATE_H */
