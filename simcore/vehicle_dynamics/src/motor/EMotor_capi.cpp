/*
 * EMotor_capi.cpp
 *
 * Code generation for model "EMotor".
 *
 * Model version              : 1.31
 * Simulink Coder version : 9.7 (R2022a) 13-Nov-2021
 * C++ source code generated on : Thu Sep 15 20:22:54 2022
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "rtw_capi.h"
#ifdef HOST_CAPI_BUILD
#  include "EMotor_capi_host.h"
#  define sizeof(s) ((size_t)(0xFFFF))
#  undef rt_offsetof
#  define rt_offsetof(s, el) ((uint16_T)(0xFFFF))
#  define TARGET_CONST
#  define TARGET_STRING(s) (s)
#else /* HOST_CAPI_BUILD */
#  include "EMotor.h"
#  include "EMotor_capi.h"
#  include "EMotor_private.h"
#  include "builtin_typeid_types.h"
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
    {0, TARGET_STRING("EMotor/GainPRM"), TARGET_STRING("Gain"), 0, 0, 0},

    {1, TARGET_STRING("EMotor/Mapped Motor/Mapped Motor Core Speed 4/Electrical Current/Saturation"),
     TARGET_STRING("UpperLimit"), 0, 0, 0},

    {2, TARGET_STRING("EMotor/Mapped Motor/Mapped Motor Core Speed 4/Electrical Current/Saturation"),
     TARGET_STRING("LowerLimit"), 0, 0, 0},

    {3, TARGET_STRING("EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabular Power Loss Data/2-D Lookup Table"),
     TARGET_STRING("Table"), 0, 1, 0},

    {4, TARGET_STRING("EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabular Power Loss Data/2-D Lookup Table"),
     TARGET_STRING("BreakpointsForDimension1"), 0, 2, 0},

    {5, TARGET_STRING("EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabular Power Loss Data/2-D Lookup Table"),
     TARGET_STRING("BreakpointsForDimension2"), 0, 3, 0},

    {6, TARGET_STRING("EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabular Power Loss Data/2-D Lookup Table"),
     TARGET_STRING("maxIndex"), 1, 4, 0},

    {7, TARGET_STRING("EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabulated Torque-speed Envelope/Integrator"),
     TARGET_STRING("InitialCondition"), 0, 0, 0},

    {8,
     TARGET_STRING(
         "EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabulated Torque-speed Envelope/Interpolated zero-crossing"),
     TARGET_STRING("Table"), 0, 5, 0},

    {9,
     TARGET_STRING(
         "EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabulated Torque-speed Envelope/Interpolated zero-crossing"),
     TARGET_STRING("BreakpointsForDimension1"), 0, 5, 0},

    {10,
     TARGET_STRING("EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabulated Torque-speed Envelope/Max Torque Limit"),
     TARGET_STRING("Table"), 0, 3, 0},

    {11,
     TARGET_STRING("EMotor/Mapped Motor/Mapped Motor Core Speed 4/Tabulated Torque-speed Envelope/Max Torque Limit"),
     TARGET_STRING("BreakpointsForDimension1"), 0, 3, 0},

    {0, (nullptr), (nullptr), 0, 0, 0}};

/* Tunable variable parameters */
static rtwCAPI_ModelParameters rtModelParameters[]{/* addrMapIndex, varName, dataTypeIndex, dimIndex, fixPtIndex */
                                                   {12, TARGET_STRING("mot_tc_s"), 0, 0, 0},

                                                   {0, (nullptr), 0, 0, 0}};

#ifndef HOST_CAPI_BUILD

/* Initialize Data Address */
static void EMotor_InitializeDataAddr(void* dataAddr[], P_EMotor_T* EMotor_P) {
  dataAddr[0] = (void*)(&EMotor_P->GainPRM_Gain);
  dataAddr[1] = (void*)(&EMotor_P->Saturation_UpperSat);
  dataAddr[2] = (void*)(&EMotor_P->Saturation_LowerSat);
  dataAddr[3] = (void*)(&EMotor_P->uDLookupTable_tableData[0]);
  dataAddr[4] = (void*)(&EMotor_P->uDLookupTable_bp01Data[0]);
  dataAddr[5] = (void*)(&EMotor_P->uDLookupTable_bp02Data[0]);
  dataAddr[6] = (void*)(&EMotor_P->uDLookupTable_maxIndex[0]);
  dataAddr[7] = (void*)(&EMotor_P->Integrator_IC);
  dataAddr[8] = (void*)(&EMotor_P->Interpolatedzerocrossing_tableData[0]);
  dataAddr[9] = (void*)(&EMotor_P->Interpolatedzerocrossing_bp01Data[0]);
  dataAddr[10] = (void*)(&EMotor_P->MaxTorqueLimit_tableData[0]);
  dataAddr[11] = (void*)(&EMotor_P->MaxTorqueLimit_bp01Data[0]);
  dataAddr[12] = (void*)(&EMotor_P->mot_tc_s);
}

#endif

/* Initialize Data Run-Time Dimension Buffer Address */
#ifndef HOST_CAPI_BUILD

static void EMotor_InitializeVarDimsAddr(int32_T* vardimsAddr[]) { vardimsAddr[0] = (nullptr); }

#endif

#ifndef HOST_CAPI_BUILD

/* Initialize logging function pointers */
static void EMotor_InitializeLoggingFunctions(RTWLoggingFcnPtr loggingPtrs[]) {
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

                                             {rtwCAPI_MATRIX_COL_MAJOR, 2, 2, 0},

                                             {rtwCAPI_VECTOR, 4, 2, 0},

                                             {rtwCAPI_VECTOR, 6, 2, 0},

                                             {rtwCAPI_VECTOR, 8, 2, 0},

                                             {rtwCAPI_VECTOR, 10, 2, 0}};

/* Dimension Array- use dimArrayIndex to access elements of this array */
static uint_T rtDimensionArray[]{
    1, /* 0 */
    1, /* 1 */
    5, /* 2 */
    7, /* 3 */
    1, /* 4 */
    5, /* 5 */
    1, /* 6 */
    7, /* 7 */
    2, /* 8 */
    1, /* 9 */
    1, /* 10 */
    2  /* 11 */
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

    {rtBlockParameters, 12, rtModelParameters, 1},

    {(nullptr), 0},

    {rtDataTypeMap, rtDimensionMap, rtFixPtMap, rtElementMap, rtSampleTimeMap, rtDimensionArray},
    "float",

    {1217384341U, 1683732273U, 789349597U, 71760144U},
    (nullptr),
    0,
    (boolean_T)0};

/* Function to get C API Model Mapping Static Info */
const rtwCAPI_ModelMappingStaticInfo* EMotor_GetCAPIStaticMap(void) { return &mmiStatic; }

/* Cache pointers into DataMapInfo substructure of RTModel */
#ifndef HOST_CAPI_BUILD

void EMotor_InitializeDataMapInfo(RT_MODEL_EMotor_T* const EMotor_M, P_EMotor_T* EMotor_P) {
  /* Set C-API version */
  rtwCAPI_SetVersion(EMotor_M->DataMapInfo.mmi, 1);

  /* Cache static C-API data into the Real-time Model Data structure */
  rtwCAPI_SetStaticMap(EMotor_M->DataMapInfo.mmi, &mmiStatic);

  /* Cache static C-API logging data into the Real-time Model Data structure */
  rtwCAPI_SetLoggingStaticMap(EMotor_M->DataMapInfo.mmi, (nullptr));

  /* Cache C-API Data Addresses into the Real-Time Model Data structure */
  EMotor_InitializeDataAddr(EMotor_M->DataMapInfo.dataAddress, EMotor_P);
  rtwCAPI_SetDataAddressMap(EMotor_M->DataMapInfo.mmi, EMotor_M->DataMapInfo.dataAddress);

  /* Cache C-API Data Run-Time Dimension Buffer Addresses into the Real-Time Model Data structure */
  EMotor_InitializeVarDimsAddr(EMotor_M->DataMapInfo.vardimsAddress);
  rtwCAPI_SetVarDimsAddressMap(EMotor_M->DataMapInfo.mmi, EMotor_M->DataMapInfo.vardimsAddress);

  /* Set Instance specific path */
  rtwCAPI_SetPath(EMotor_M->DataMapInfo.mmi, (nullptr));
  rtwCAPI_SetFullPath(EMotor_M->DataMapInfo.mmi, (nullptr));

  /* Cache C-API logging function pointers into the Real-Time Model Data structure */
  EMotor_InitializeLoggingFunctions(EMotor_M->DataMapInfo.loggingPtrs);
  rtwCAPI_SetLoggingPtrs(EMotor_M->DataMapInfo.mmi, EMotor_M->DataMapInfo.loggingPtrs);

  /* Cache the instance C-API logging pointer */
  rtwCAPI_SetInstanceLoggingInfo(EMotor_M->DataMapInfo.mmi, (nullptr));

  /* Set reference to submodels */
  rtwCAPI_SetChildMMIArray(EMotor_M->DataMapInfo.mmi, (nullptr));
  rtwCAPI_SetChildMMIArrayLen(EMotor_M->DataMapInfo.mmi, 0);
}

#else /* HOST_CAPI_BUILD */
#  ifdef __cplusplus

extern "C" {

#  endif

void EMotor_host_InitializeDataMapInfo(EMotor_host_DataMapInfo_T *dataMap, const char *path) {
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

/* EOF: EMotor_capi.cpp */
