/*
 * carTypeParsing_capi.cpp
 *
 * Code generation for model "carTypeParsing".
 *
 * Model version              : 1.11
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Fri Jul  8 17:01:31 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "rtw_capi.h"
#ifdef HOST_CAPI_BUILD
#  include "carTypeParsing_capi_host.h"
#  define sizeof(s) ((size_t)(0xFFFF))
#  undef rt_offsetof
#  define rt_offsetof(s, el) ((uint16_T)(0xFFFF))
#  define TARGET_CONST
#  define TARGET_STRING(s) (s)
#else /* HOST_CAPI_BUILD */
#  include "builtin_typeid_types.h"
#  include "carTypeParsing.h"
#  include "carTypeParsing_capi.h"
#  include "carTypeParsing_private.h"
#  ifdef LIGHT_WEIGHT_CAPI
#    define TARGET_CONST
#    define TARGET_STRING(s) ((nullptr))
#  else
#    define TARGET_CONST const
#    define TARGET_STRING(s) (s)
#  endif
#endif /* HOST_CAPI_BUILD */

static rtwCAPI_BlockParameters rtBlockParameters[]{
    /* addrMapIndex, blockPath,
     * paramName, dataTypeIndex, dimIndex, fixPtIdx
     */
    {0, TARGET_STRING("carTypeParsing/parsingbck_01"), TARGET_STRING("Value"), 0, 0, 0},

    {1, TARGET_STRING("carTypeParsing/parsingbck_02"), TARGET_STRING("Value"), 0, 0, 0},

    {2, TARGET_STRING("carTypeParsing/parsingbck_03"), TARGET_STRING("Value"), 0, 0, 0},

    {0, (nullptr), (nullptr), 0, 0, 0}};

/* Tunable variable parameters */
static rtwCAPI_ModelParameters rtModelParameters[]{/* addrMapIndex, varName, dataTypeIndex, dimIndex, fixPtIndex */
                                                   {3, TARGET_STRING("drivetype"), 0, 0, 0},

                                                   {4, TARGET_STRING("proplutiontype"), 0, 0, 0},

                                                   {0, (nullptr), 0, 0, 0}};

#ifndef HOST_CAPI_BUILD

/* Initialize Data Address */
static void carTypeParsing_InitializeDataAddr(void* dataAddr[], P_carTypeParsing_T* carTypeParsing_P) {
  dataAddr[0] = (void*)(&carTypeParsing_P->parsingbck_01_Value);
  dataAddr[1] = (void*)(&carTypeParsing_P->parsingbck_02_Value);
  dataAddr[2] = (void*)(&carTypeParsing_P->parsingbck_03_Value);
  dataAddr[3] = (void*)(&carTypeParsing_P->drivetype);
  dataAddr[4] = (void*)(&carTypeParsing_P->proplutiontype);
}

#endif

/* Initialize Data Run-Time Dimension Buffer Address */
#ifndef HOST_CAPI_BUILD

static void carTypeParsing_InitializeVarDimsAddr(int32_T* vardimsAddr[]) { vardimsAddr[0] = (nullptr); }

#endif

#ifndef HOST_CAPI_BUILD

/* Initialize logging function pointers */
static void carTypeParsing_InitializeLoggingFunctions(RTWLoggingFcnPtr loggingPtrs[]) {
  loggingPtrs[0] = (nullptr);
  loggingPtrs[1] = (nullptr);
  loggingPtrs[2] = (nullptr);
  loggingPtrs[3] = (nullptr);
  loggingPtrs[4] = (nullptr);
}

#endif

/* Data Type Map - use dataTypeMapIndex to access this structure */
static TARGET_CONST rtwCAPI_DataTypeMap rtDataTypeMap[]{
    /* cName, mwName, numElements, elemMapIndex, dataSize, slDataId, *
     * isComplex, isPointer, enumStorageType */
    {"double", "real_T", 0, 0, sizeof(real_T), (uint8_T)SS_DOUBLE, 0, 0, 0}};

#ifdef HOST_CAPI_BUILD
#  undef sizeof
#endif

/* Structure Element Map - use elemMapIndex to access this structure */
static TARGET_CONST rtwCAPI_ElementMap rtElementMap[]{
    /* elementName, elementOffset, dataTypeIndex, dimIndex, fxpIndex */
    {(nullptr), 0, 0, 0, 0},
};

/* Dimension Map - use dimensionMapIndex to access elements of ths structure*/
static rtwCAPI_DimensionMap rtDimensionMap[]{/* dataOrientation, dimArrayIndex, numDims, vardimsIndex */
                                             {rtwCAPI_SCALAR, 0, 2, 0}};

/* Dimension Array- use dimArrayIndex to access elements of this array */
static uint_T rtDimensionArray[]{
    1, /* 0 */
    1  /* 1 */
};

/* Fixed Point Map */
static rtwCAPI_FixPtMap rtFixPtMap[]{
    /* fracSlopePtr, biasPtr, scaleType, wordLength, exponent, isSigned */
    {(nullptr), (nullptr), rtwCAPI_FIX_RESERVED, 0, 0, (boolean_T)0},
};

/* Sample Time Map - use sTimeIndex to access elements of ths structure */
static rtwCAPI_SampleTimeMap rtSampleTimeMap[]{/* samplePeriodPtr, sampleOffsetPtr, tid, samplingMode */
                                               {(nullptr), (nullptr), 0, 0}};

static rtwCAPI_ModelMappingStaticInfo mmiStatic{
    /* Signals:{signals, numSignals,
     *           rootInputs, numRootInputs,
     *           rootOutputs, numRootOutputs},
     * Params: {blockParameters, numBlockParameters,
     *          modelParameters, numModelParameters},
     * States: {states, numStates},
     * Maps:   {dataTypeMap, dimensionMap, fixPtMap,
     *          elementMap, sampleTimeMap, dimensionArray},
     * TargetType: targetType
     */
    {(nullptr), 0, (nullptr), 0, (nullptr), 0},

    {rtBlockParameters, 3, rtModelParameters, 2},

    {(nullptr), 0},

    {rtDataTypeMap, rtDimensionMap, rtFixPtMap, rtElementMap, rtSampleTimeMap, rtDimensionArray},
    "float",

    {3231349998U, 3390977152U, 111004639U, 1758738351U},
    (nullptr),
    0,
    (boolean_T)0};

/* Function to get C API Model Mapping Static Info */
const rtwCAPI_ModelMappingStaticInfo* carTypeParsing_GetCAPIStaticMap(void) { return &mmiStatic; }

/* Cache pointers into DataMapInfo substructure of RTModel */
#ifndef HOST_CAPI_BUILD

void carTypeParsing_InitializeDataMapInfo(RT_MODEL_carTypeParsing_T* const carTypeParsing_M,
                                          P_carTypeParsing_T* carTypeParsing_P) {
  /* Set C-API version */
  rtwCAPI_SetVersion(carTypeParsing_M->DataMapInfo.mmi, 1);

  /* Cache static C-API data into the Real-time Model Data structure */
  rtwCAPI_SetStaticMap(carTypeParsing_M->DataMapInfo.mmi, &mmiStatic);

  /* Cache static C-API logging data into the Real-time Model Data structure */
  rtwCAPI_SetLoggingStaticMap(carTypeParsing_M->DataMapInfo.mmi, (nullptr));

  /* Cache C-API Data Addresses into the Real-Time Model Data structure */
  carTypeParsing_InitializeDataAddr(carTypeParsing_M->DataMapInfo.dataAddress, carTypeParsing_P);
  rtwCAPI_SetDataAddressMap(carTypeParsing_M->DataMapInfo.mmi, carTypeParsing_M->DataMapInfo.dataAddress);

  /* Cache C-API Data Run-Time Dimension Buffer Addresses into the Real-Time Model Data structure */
  carTypeParsing_InitializeVarDimsAddr(carTypeParsing_M->DataMapInfo.vardimsAddress);
  rtwCAPI_SetVarDimsAddressMap(carTypeParsing_M->DataMapInfo.mmi, carTypeParsing_M->DataMapInfo.vardimsAddress);

  /* Set Instance specific path */
  rtwCAPI_SetPath(carTypeParsing_M->DataMapInfo.mmi, (nullptr));
  rtwCAPI_SetFullPath(carTypeParsing_M->DataMapInfo.mmi, (nullptr));

  /* Cache C-API logging function pointers into the Real-Time Model Data structure */
  carTypeParsing_InitializeLoggingFunctions(carTypeParsing_M->DataMapInfo.loggingPtrs);
  rtwCAPI_SetLoggingPtrs(carTypeParsing_M->DataMapInfo.mmi, carTypeParsing_M->DataMapInfo.loggingPtrs);

  /* Cache the instance C-API logging pointer */
  rtwCAPI_SetInstanceLoggingInfo(carTypeParsing_M->DataMapInfo.mmi, (nullptr));

  /* Set reference to submodels */
  rtwCAPI_SetChildMMIArray(carTypeParsing_M->DataMapInfo.mmi, (nullptr));
  rtwCAPI_SetChildMMIArrayLen(carTypeParsing_M->DataMapInfo.mmi, 0);
}

#else /* HOST_CAPI_BUILD */
#  ifdef __cplusplus

extern "C" {

#  endif

void carTypeParsing_host_InitializeDataMapInfo(carTypeParsing_host_DataMapInfo_T *dataMap, const char *path) {
  /* Set C-API version */
  rtwCAPI_SetVersion(dataMap->mmi, 1);

  /* Cache static C-API data into the Real-time Model Data structure */
  rtwCAPI_SetStaticMap(dataMap->mmi, &mmiStatic);

  /* host data address map is NULL */
  rtwCAPI_SetDataAddressMap(dataMap->mmi, (nullptr));

  /* host vardims address map is NULL */
  rtwCAPI_SetVarDimsAddressMap(dataMap->mmi, (nullptr));

  /* Set Instance specific path */
  rtwCAPI_SetPath(dataMap->mmi, path);
  rtwCAPI_SetFullPath(dataMap->mmi, (nullptr));

  /* Set reference to submodels */
  rtwCAPI_SetChildMMIArray(dataMap->mmi, (nullptr));
  rtwCAPI_SetChildMMIArrayLen(dataMap->mmi, 0);
}

#  ifdef __cplusplus
}
#  endif
#endif /* HOST_CAPI_BUILD */

/* EOF: carTypeParsing_capi.cpp */
