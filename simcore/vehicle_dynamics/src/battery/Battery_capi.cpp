/*
 * Battery_capi.cpp
 *
 * Code generation for model "Battery".
 *
 * Model version              : 1.33
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Tue Jun 14 11:38:22 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "rtw_capi.h"
#ifdef HOST_CAPI_BUILD
#  include "Battery_capi_host.h"
#  define sizeof(s) ((size_t)(0xFFFF))
#  undef rt_offsetof
#  define rt_offsetof(s, el) ((uint16_T)(0xFFFF))
#  define TARGET_CONST
#  define TARGET_STRING(s) (s)
#else /* HOST_CAPI_BUILD */
#  include "Battery.h"
#  include "Battery_capi.h"
#  include "Battery_private.h"
#  include "builtin_typeid_types.h"
#  ifdef LIGHT_WEIGHT_CAPI
#    define TARGET_CONST
#    define TARGET_STRING(s) ((nullptr))
#  else
#    define TARGET_CONST const
#    define TARGET_STRING(s) (s)
#  endif
#endif /* HOST_CAPI_BUILD */

namespace tx_car {
namespace power {

static rtwCAPI_BlockParameters rtBlockParameters[]{
    /* addrMapIndex, blockPath,
     * paramName, dataTypeIndex, dimIndex, fixPtIdx
     */
    {0, TARGET_STRING("Battery/Gain"), TARGET_STRING("Gain"), 0, 0, 0},

    {1, TARGET_STRING("Battery/Gain1"), TARGET_STRING("Gain"), 0, 0, 0},

    {2, TARGET_STRING("Battery/Datasheet Battery/Info Bus/Gain1"), TARGET_STRING("Gain"), 0, 0, 0},

    {3,
     TARGET_STRING(
         "Battery/Datasheet Battery/Datasheet Battery Internal/Datasheet Battery/Charge Model/Integrator Limited"),
     TARGET_STRING("LowerSaturationLimit"), 0, 0, 0},

    {4,
     TARGET_STRING(
         "Battery/Datasheet Battery/Datasheet Battery Internal/Datasheet Battery/Voltage and Power Calculation/R"),
     TARGET_STRING("maxIndex"), 1, 1, 0},

    {0, (nullptr), (nullptr), 0, 0, 0}};

/* Tunable variable parameters */
static rtwCAPI_ModelParameters rtModelParameters[]{/* addrMapIndex, varName, dataTypeIndex, dimIndex, fixPtIndex */
                                                   {5, TARGET_STRING("BattChargeMax"), 0, 0, 0},

                                                   {6, TARGET_STRING("BattInitSoc"), 0, 0, 0},

                                                   {7, TARGET_STRING("BattTempBp"), 0, 2, 0},

                                                   {8, TARGET_STRING("CapLUTBp"), 0, 3, 0},

                                                   {9, TARGET_STRING("CapSOCBp"), 0, 4, 0},

                                                   {10, TARGET_STRING("Em"), 0, 3, 0},

                                                   {11, TARGET_STRING("Np"), 0, 0, 0},

                                                   {12, TARGET_STRING("Ns"), 0, 0, 0},

                                                   {13, TARGET_STRING("RInt"), 0, 5, 0},

                                                   {0, (nullptr), 0, 0, 0}};

#ifndef HOST_CAPI_BUILD

/* Initialize Data Address */
static void Battery_InitializeDataAddr(void* dataAddr[], P_Battery_T* Battery_P) {
  dataAddr[0] = (void*)(&Battery_P->Gain_Gain);
  dataAddr[1] = (void*)(&Battery_P->Gain1_Gain_c);
  dataAddr[2] = (void*)(&Battery_P->Gain1_Gain);
  dataAddr[3] = (void*)(&Battery_P->IntegratorLimited_LowerSat);
  dataAddr[4] = (void*)(&Battery_P->R_maxIndex[0]);
  dataAddr[5] = (void*)(&Battery_P->BattChargeMax);
  dataAddr[6] = (void*)(&Battery_P->BattInitSoc);
  dataAddr[7] = (void*)(&Battery_P->BattTempBp[0]);
  dataAddr[8] = (void*)(&Battery_P->CapLUTBp[0]);
  dataAddr[9] = (void*)(&Battery_P->CapSOCBp[0]);
  dataAddr[10] = (void*)(&Battery_P->Em[0]);
  dataAddr[11] = (void*)(&Battery_P->Np);
  dataAddr[12] = (void*)(&Battery_P->Ns);
  dataAddr[13] = (void*)(&Battery_P->RInt[0]);
}

#endif

/* Initialize Data Run-Time Dimension Buffer Address */
#ifndef HOST_CAPI_BUILD

static void Battery_InitializeVarDimsAddr(int32_T* vardimsAddr[]) { vardimsAddr[0] = (nullptr); }

#endif

#ifndef HOST_CAPI_BUILD

/* Initialize logging function pointers */
static void Battery_InitializeLoggingFunctions(RTWLoggingFcnPtr loggingPtrs[]) {
  loggingPtrs[0] = (nullptr);
  loggingPtrs[1] = (nullptr);
  loggingPtrs[2] = (nullptr);
  loggingPtrs[3] = (nullptr);
  loggingPtrs[4] = (nullptr);
  loggingPtrs[5] = (nullptr);
  loggingPtrs[6] = (nullptr);
  loggingPtrs[7] = (nullptr);
  loggingPtrs[8] = (nullptr);
  loggingPtrs[9] = (nullptr);
  loggingPtrs[10] = (nullptr);
  loggingPtrs[11] = (nullptr);
  loggingPtrs[12] = (nullptr);
  loggingPtrs[13] = (nullptr);
}

#endif

/* Data Type Map - use dataTypeMapIndex to access this structure */
static TARGET_CONST rtwCAPI_DataTypeMap rtDataTypeMap[]{
    /* cName, mwName, numElements, elemMapIndex, dataSize, slDataId, *
     * isComplex, isPointer, enumStorageType */
    {"double", "real_T", 0, 0, sizeof(real_T), (uint8_T)SS_DOUBLE, 0, 0, 0},

    {"unsigned int", "uint32_T", 0, 0, sizeof(uint32_T), (uint8_T)SS_UINT32, 0, 0, 0}};

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
                                             {rtwCAPI_SCALAR, 0, 2, 0},

                                             {rtwCAPI_VECTOR, 2, 2, 0},

                                             {rtwCAPI_VECTOR, 4, 2, 0},

                                             {rtwCAPI_VECTOR, 6, 2, 0},

                                             {rtwCAPI_VECTOR, 8, 2, 0},

                                             {rtwCAPI_MATRIX_COL_MAJOR, 10, 2, 0}};

/* Dimension Array- use dimArrayIndex to access elements of this array */
static uint_T rtDimensionArray[]{
    1,   /* 0 */
    1,   /* 1 */
    2,   /* 2 */
    1,   /* 3 */
    1,   /* 4 */
    7,   /* 5 */
    1,   /* 6 */
    101, /* 7 */
    1,   /* 8 */
    6,   /* 9 */
    7,   /* 10 */
    6    /* 11 */
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

    {rtBlockParameters, 5, rtModelParameters, 9},

    {(nullptr), 0},

    {rtDataTypeMap, rtDimensionMap, rtFixPtMap, rtElementMap, rtSampleTimeMap, rtDimensionArray},
    "float",

    {707106905U, 3279474311U, 2480194082U, 1911678820U},
    (nullptr),
    0,
    (boolean_T)0};

/* Function to get C API Model Mapping Static Info */
const rtwCAPI_ModelMappingStaticInfo* Battery_GetCAPIStaticMap(void) { return &mmiStatic; }

/* Cache pointers into DataMapInfo substructure of RTModel */
#ifndef HOST_CAPI_BUILD

void Battery_InitializeDataMapInfo(RT_MODEL_Battery_T* const Battery_M, P_Battery_T* Battery_P) {
  /* Set C-API version */
  rtwCAPI_SetVersion(Battery_M->DataMapInfo.mmi, 1);

  /* Cache static C-API data into the Real-time Model Data structure */
  rtwCAPI_SetStaticMap(Battery_M->DataMapInfo.mmi, &mmiStatic);

  /* Cache static C-API logging data into the Real-time Model Data structure */
  rtwCAPI_SetLoggingStaticMap(Battery_M->DataMapInfo.mmi, (nullptr));

  /* Cache C-API Data Addresses into the Real-Time Model Data structure */
  Battery_InitializeDataAddr(Battery_M->DataMapInfo.dataAddress, Battery_P);
  rtwCAPI_SetDataAddressMap(Battery_M->DataMapInfo.mmi, Battery_M->DataMapInfo.dataAddress);

  /* Cache C-API Data Run-Time Dimension Buffer Addresses into the Real-Time Model Data structure */
  Battery_InitializeVarDimsAddr(Battery_M->DataMapInfo.vardimsAddress);
  rtwCAPI_SetVarDimsAddressMap(Battery_M->DataMapInfo.mmi, Battery_M->DataMapInfo.vardimsAddress);

  /* Set Instance specific path */
  rtwCAPI_SetPath(Battery_M->DataMapInfo.mmi, (nullptr));
  rtwCAPI_SetFullPath(Battery_M->DataMapInfo.mmi, (nullptr));

  /* Cache C-API logging function pointers into the Real-Time Model Data structure */
  Battery_InitializeLoggingFunctions(Battery_M->DataMapInfo.loggingPtrs);
  rtwCAPI_SetLoggingPtrs(Battery_M->DataMapInfo.mmi, Battery_M->DataMapInfo.loggingPtrs);

  /* Cache the instance C-API logging pointer */
  rtwCAPI_SetInstanceLoggingInfo(Battery_M->DataMapInfo.mmi, (nullptr));

  /* Set reference to submodels */
  rtwCAPI_SetChildMMIArray(Battery_M->DataMapInfo.mmi, (nullptr));
  rtwCAPI_SetChildMMIArrayLen(Battery_M->DataMapInfo.mmi, 0);
}

#else /* HOST_CAPI_BUILD */
#  ifdef __cplusplus

extern "C" {

#  endif

void Battery_host_InitializeDataMapInfo(Battery_host_DataMapInfo_T *dataMap, const char *path) {
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

}  // namespace power
}  // namespace tx_car

/* EOF: Battery_capi.cpp */
