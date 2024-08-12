/* Copyright 1994-2022 The MathWorks, Inc. */

/*
 * File: rtw_modelmap.h
 *
 * Abstract:
 *   Model tuning information.  Use the provided structure access methods
 *   whenever possible.
 *
 *   For details about these structures see Simulink Coder User's guide.
 */

#ifndef __RTW_MODELMAP__
#  define __RTW_MODELMAP__

#  ifdef SL_INTERNAL

#    include "simstruct/simstruc_types.h"
#    include "simulinkcoder_capi_export.hpp"
#    include "util/memmgr/memalloc.hpp"
#    include "version.h"

#  else

#    include <stdlib.h>

#    ifdef HOST_CAPI_BUILD
#      include "simstruc_types.h"
#    else
/* Macro SIMPLIFIED_RTWTYPES_COMPATIBILITY declares compatibility with simplified version of
 * rtwtypes.h */
#      define SIMPLIFIED_RTWTYPES_COMPATIBILITY
#      include "rtwtypes.h"
#      undef SIMPLIFIED_RTWTYPES_COMPATIBILITY
#    endif

#    ifndef __RTW_UTFREE__
#      define __RTW_UTFREE__
#      define utFree(arg) \
        if (arg) free(arg)
#      define utMalloc(arg) malloc(arg)
#    endif

#    define SIMULINKCODER_CAPI_API extern
#  endif

#  include "rtw_capi.h"
#  include "rtw_modelmap_logging.h"

typedef struct rtwCAPI_ModelMappingInfo_tag rtwCAPI_ModelMappingInfo;
typedef struct rtwCAPI_ModelMappingStaticInfo_tag rtwCAPI_ModelMappingStaticInfo;

/* ModelMappingStaticInfo */
struct rtwCAPI_ModelMappingStaticInfo_tag {
  /* signals */
  struct {
    rtwCAPI_Signals const* signals;     /* Signals Array */
    uint_T numSignals;                  /* Num Signals   */
    rtwCAPI_Signals const* rootInputs;  /* Root Inputs array */
    uint_T numRootInputs;               /* Num Root Inputs  */
    rtwCAPI_Signals const* rootOutputs; /* Root Outputs array */
    uint_T numRootOutputs;              /* Num Root Outputs  */
  } Signals;

  /* parameters */
  struct {
    rtwCAPI_BlockParameters const* blockParameters; /* Block parameters Array   */
    uint_T numBlockParameters;                      /* Num block parameters     */
    rtwCAPI_ModelParameters const* modelParameters; /* Model parameters Array*/
    uint_T numModelParameters;                      /* Num Model parameters  */
  } Params;

  /* states */
  struct {
    rtwCAPI_States const* states; /* States array */
    uint_T numStates;             /* Num States   */
  } States;

  /* Static maps */
  /* datatypes, dimensions, fixed point, structure elements, sample times  */
  struct {
    rtwCAPI_DataTypeMap const* dataTypeMap;     /* Data Type Map          */
    rtwCAPI_DimensionMap const* dimensionMap;   /* Data Dimension Map     */
    rtwCAPI_FixPtMap const* fixPtMap;           /* Fixed Point Map        */
    rtwCAPI_ElementMap const* elementMap;       /* Structure Element map  */
    rtwCAPI_SampleTimeMap const* sampleTimeMap; /* Sample Times Map       */
    uint_T const* dimensionArray;               /* Dimension Array        */
  } Maps;

  /* TargetType - string specifying the intended target of the generated  *
   * C-API.                                                               *
   * targetType = "float"   - target supports floats and integer code     *
   *            = "integer" - target supports integer only code           */
  char_T const* targetType;

  /* for internal use */
  uint32_T modelChecksum[4];
  rtwCAPI_ModelMapLoggingStaticInfo const* staticLogInfo;
  size_t rtpSize;
  /* If this instance in a protected model*/
  boolean_T isProtectedModel;
};

/* ModelMappingInfo */
struct rtwCAPI_ModelMappingInfo_tag {
  /* ModelMappingInfo version */
  uint8_T versionNum;

  /* Reference to static model data, all model instances share this map   */
  rtwCAPI_ModelMappingStaticInfo* staticMap;

  /* Instance specific Maps, each model instance has a unique InstanceMap */
  struct {
    const char* path; /* Path to this instance */
    char* fullPath;
    void** dataAddrMap;                       /* Data Address map      */
    rtwCAPI_ModelMappingInfo** childMMIArray; /* array of child MMI    */
    uint_T childMMIArrayLen;                  /* Number of child MMIs  */
    int_T contStateStartIndex;

    /* for internal use */
    rtwCAPI_ModelMapLoggingInstanceInfo* instanceLogInfo;
    int32_T** vardimsAddrMap; /* Vardims Address map   */
    void* rtpAddress;
    RTWLoggingFcnPtr* RTWLoggingPtrs; /* MatFile logging information */
  } InstanceMap;
};

/* Macros for accessing ModelMappingStaticInfo fields */
#  define rtwCAPI_GetSignalsFromStaticMap(SM) ((SM)->Signals.signals)
#  define rtwCAPI_GetNumSignalsFromStaticMap(SM) ((SM)->Signals.numSignals)
#  define rtwCAPI_GetLogSignalsArrayFromStaticMap(SM) ((SM)->Signals.logSignalsArray)
#  define rtwCAPI_GetNumLogSignalsFromStaticMap(SM) ((SM)->Signals.numLogSignals)

#  define rtwCAPI_GetBlockParametersFromStaticMap(SM) ((SM)->Params.blockParameters)
#  define rtwCAPI_GetNumBlockParametersFromStaticMap(SM) ((SM)->Params.numBlockParameters)
#  define rtwCAPI_GetModelParametersFromStaticMap(SM) ((SM)->Params.modelParameters)
#  define rtwCAPI_GetNumModelParametersFromStaticMap(SM) ((SM)->Params.numModelParameters)

#  define rtwCAPI_GetStatesFromStaticMap(SM) ((SM)->States.states)
#  define rtwCAPI_GetNumStatesFromStaticMap(SM) ((SM)->States.numStates)

#  define rtwCAPI_GetRootInputsFromStaticMap(SM) ((SM)->Signals.rootInputs)
#  define rtwCAPI_GetNumRootInputsFromStaticMap(SM) ((SM)->Signals.numRootInputs)
#  define rtwCAPI_GetRootOutputsFromStaticMap(SM) ((SM)->Signals.rootOutputs)
#  define rtwCAPI_GetNumRootOutputsFromStaticMap(SM) ((SM)->Signals.numRootOutputs)

#  define rtwCAPI_GetDataTypeMapFromStaticMap(SM) ((SM)->Maps.dataTypeMap)
#  define rtwCAPI_GetDimensionMapFromStaticMap(SM) ((SM)->Maps.dimensionMap)
#  define rtwCAPI_GetFixPtMapFromStaticMap(SM) ((SM)->Maps.fixPtMap)
#  define rtwCAPI_GetElementMapFromStaticMap(SM) ((SM)->Maps.elementMap)
#  define rtwCAPI_GetSampleTimeMapFromStaticMap(SM) ((SM)->Maps.sampleTimeMap)
#  define rtwCAPI_GetDimensionArrayFromStaticMap(SM) ((SM)->Maps.dimensionArray)

/* Macros for accessing ModelMappingInfo fields */
#  define rtwCAPI_GetSignals(MMI) rtwCAPI_GetSignalsFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetNumSignals(MMI) rtwCAPI_GetNumSignalsFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetLogSignalsArray(MMI) rtwCAPI_GetLogSignalsArrayFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetNumLogSignals(MMI) rtwCAPI_GetNumLogSignalsFromStaticMap((MMI)->staticMap)

#  define rtwCAPI_GetBlockParameters(MMI) rtwCAPI_GetBlockParametersFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetNumBlockParameters(MMI) rtwCAPI_GetNumBlockParametersFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetModelParameters(MMI) rtwCAPI_GetModelParametersFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetNumModelParameters(MMI) rtwCAPI_GetNumModelParametersFromStaticMap((MMI)->staticMap)

#  define rtwCAPI_GetStates(MMI) rtwCAPI_GetStatesFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetNumStates(MMI) rtwCAPI_GetNumStatesFromStaticMap((MMI)->staticMap)

#  define rtwCAPI_GetRootInputs(MMI) rtwCAPI_GetRootInputsFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetNumRootInputs(MMI) rtwCAPI_GetNumRootInputsFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetRootOutputs(MMI) rtwCAPI_GetRootOutputsFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetNumRootOutputs(MMI) rtwCAPI_GetNumRootOutputsFromStaticMap((MMI)->staticMap)

#  define rtwCAPI_GetDataTypeMap(MMI) rtwCAPI_GetDataTypeMapFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetDimensionMap(MMI) rtwCAPI_GetDimensionMapFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetFixPtMap(MMI) rtwCAPI_GetFixPtMapFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetElementMap(MMI) rtwCAPI_GetElementMapFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetSampleTimeMap(MMI) rtwCAPI_GetSampleTimeMapFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_GetDimensionArray(MMI) rtwCAPI_GetDimensionArrayFromStaticMap((MMI)->staticMap)
#  define rtwCAPI_IsProtectedModel(MMI) ((MMI)->staticMap->isProtectedModel)

#  define rtwCAPI_GetStaticLoggingInfo(MMI) ((MMI)->staticMap->staticLogInfo)

#  define rtwCAPI_GetVersion(MMI) ((MMI)->versionNum)
#  define rtwCAPI_GetDataAddressMap(MMI) ((MMI)->InstanceMap.dataAddrMap)
#  define rtwCAPI_GetVarDimsAddressMap(MMI) ((MMI)->InstanceMap.vardimsAddrMap)
#  define rtwCAPI_GetRTWLoggingPtrsMap(MMI) ((MMI)->InstanceMap.RTWLoggingPtrs)
#  define rtwCAPI_GetPath(MMI) ((MMI)->InstanceMap.path)
#  define rtwCAPI_GetFullPath(MMI) ((MMI)->InstanceMap.fullPath)
#  define rtwCAPI_GetChildMMI(MMI, i) ((MMI)->InstanceMap.childMMIArray[i])
#  define rtwCAPI_GetChildMMIArray(MMI) ((MMI)->InstanceMap.childMMIArray)
#  define rtwCAPI_GetChildMMIArrayLen(MMI) ((MMI)->InstanceMap.childMMIArrayLen)
#  define rtwCAPI_MMIGetContStateStartIndex(MMI) ((MMI)->InstanceMap.contStateStartIndex)
#  define rtwCAPI_GetInstanceLoggingInfo(MMI) ((MMI)->InstanceMap.instanceLogInfo)

/* Macros for setting ModelMappingInfo fields */
#  define rtwCAPI_SetVersion(MMI, n) ((MMI).versionNum = (n))
#  define rtwCAPI_SetStaticMap(MMI, statMap) (MMI).staticMap = (statMap)
#  define rtwCAPI_SetLoggingStaticMap(MMI, lStatMap) (MMI).staticMap->staticLogInfo = (lStatMap)
#  define rtwCAPI_SetDataAddressMap(MMI, dAddr) (MMI).InstanceMap.dataAddrMap = (dAddr)
#  define rtwCAPI_SetVarDimsAddressMap(MMI, vAddr) (MMI).InstanceMap.vardimsAddrMap = (vAddr)
#  define rtwCAPI_SetLoggingPtrs(MMI, lAddr) (MMI).InstanceMap.RTWLoggingPtrs = (lAddr)
#  define rtwCAPI_SetPath(MMI, p) (MMI).InstanceMap.path = (p)
#  define rtwCAPI_SetFullPath(MMI, p) (MMI).InstanceMap.fullPath = (p)
#  define rtwCAPI_SetChildMMI(MMI, i, cMMI) (MMI).InstanceMap.childMMIArray[i] = (cMMI)
#  define rtwCAPI_SetChildMMIArray(MMI, cMMIs) (MMI).InstanceMap.childMMIArray = (cMMIs)
#  define rtwCAPI_SetChildMMIArrayLen(MMI, n) (MMI).InstanceMap.childMMIArrayLen = (n)
#  define rtwCAPI_MMISetContStateStartIndex(MMI, i) (MMI).InstanceMap.contStateStartIndex = (i)
#  define rtwCAPI_SetInstanceLoggingInfo(MMI, l) (MMI).InstanceMap.instanceLogInfo = (l)

/* Functions in rtw_modelmap_utils.c */
#  ifdef __cplusplus
extern "C" {
#  endif
SIMULINKCODER_CAPI_API char* rtwCAPI_EncodePath(const char* path);
SIMULINKCODER_CAPI_API boolean_T rtwCAPI_HasStates(const rtwCAPI_ModelMappingInfo* mmi);
SIMULINKCODER_CAPI_API int_T rtwCAPI_GetNumStateRecords(const rtwCAPI_ModelMappingInfo* mmi);
SIMULINKCODER_CAPI_API int_T rtwCAPI_GetNumStateRecordsForRTWLogging(const rtwCAPI_ModelMappingInfo* mmi);
SIMULINKCODER_CAPI_API int_T rtwCAPI_GetNumContStateRecords(const rtwCAPI_ModelMappingInfo* mmi);
SIMULINKCODER_CAPI_API void rtwCAPI_FreeFullPaths(rtwCAPI_ModelMappingInfo* mmi);
SIMULINKCODER_CAPI_API const char_T* rtwCAPI_UpdateFullPaths(rtwCAPI_ModelMappingInfo* mmi, const char_T* path,
                                                             boolean_T isCalledFromTopModel);
SIMULINKCODER_CAPI_API char* rtwCAPI_GetFullStateBlockPath(const char* stateBlockPath, const char* mmiPath,
                                                           size_t mmiPathLen, boolean_T crossingModel);
SIMULINKCODER_CAPI_API uint_T rtwCAPI_GetStateWidth(const rtwCAPI_DimensionMap* dimMap, const uint_T* dimArray,
                                                    const rtwCAPI_States* states, uint_T iState);
SIMULINKCODER_CAPI_API const char_T* rtwCAPI_GetStateRecordInfo(
    const rtwCAPI_ModelMappingInfo* mmi, const char_T** sigBlockName, const char_T** sigLabel, const char_T** sigName,
    int_T* sigWidth, int_T* sigDataType, int_T* logDataType, int_T* sigComplexity, void** sigDataAddr,
    RTWLoggingFcnPtr* RTWLoggingPtrs, boolean_T* sigCrossMdlRef, boolean_T* sigInProtectedMdl,
    const char_T** sigPathAlias, double* sigSampleTime, int_T* sigHierInfoIdx, uint_T* sigFlatElemIdx,
    const rtwCAPI_ModelMappingInfo** sigMMI, int_T* sigIdx, boolean_T crossingModel, boolean_T isInProtectedMdl,
    real_T* stateDerivVector, boolean_T rtwLogging);
SIMULINKCODER_CAPI_API int_T rtwCAPI_GetNumSigLogRecords(const rtwCAPI_ModelMappingInfo* mmi);
SIMULINKCODER_CAPI_API int_T rtwCAPI_GetNumSigLogRecordsForRTWLogging(const rtwCAPI_ModelMappingInfo* mmi);
SIMULINKCODER_CAPI_API const char_T* rtwCAPI_GetSigLogRecordInfo(
    const rtwCAPI_ModelMappingInfo* mmi, const char_T** sigBlockName, const char_T** sigLabel, int_T* sigWidth,
    int_T* sigDataType, int_T* logDataType, int_T* sigComplexity, void** sigDataAddr, boolean_T* sigCrossMdlRef,
    int_T* sigIdx, boolean_T crossingModel, boolean_T rtwLogging);
SIMULINKCODER_CAPI_API void rtwCAPI_CountSysRan(const rtwCAPI_ModelMappingInfo* mmi, int* count);
SIMULINKCODER_CAPI_API void rtwCAPI_FillSysRan(const rtwCAPI_ModelMappingInfo* mmi, sysRanDType** sysRan, int* sysTid,
                                               int* fillIdx);
#  ifdef __cplusplus
}
#  endif

#endif /* __RTW_MODELMAP__ */

/* EOF - rtw_modelmap.h */

/* LocalWords:  MMI Vardims utils
 */
