/*
 * carTypeParsing.cpp
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

#include "carTypeParsing.h"
#include "carTypeParsing_capi.h"

/* Model step function */
void carTypeParsing::step() {
  /* BusCreator generated from: '<Root>/CarType' incorporates:
   *  Constant: '<Root>/Constant'
   *  Constant: '<Root>/Constant1'
   *  Constant: '<Root>/parsingbck_01'
   *  Constant: '<Root>/parsingbck_02'
   *  Constant: '<Root>/parsingbck_03'
   *  Outport: '<Root>/CarType'
   */
  carTypeParsing_Y.CarType.drivetype = carTypeParsing_P.drivetype;
  carTypeParsing_Y.CarType.proplutiontype = carTypeParsing_P.proplutiontype;
  carTypeParsing_Y.CarType.parsingbck_01 = carTypeParsing_P.parsingbck_01_Value;
  carTypeParsing_Y.CarType.parsingbck_02 = carTypeParsing_P.parsingbck_02_Value;
  carTypeParsing_Y.CarType.parsingbck_03 = carTypeParsing_P.parsingbck_03_Value;
}

/* Model initialize function */
void carTypeParsing::initialize() {
  /* Registration code */

  /* Initialize DataMapInfo substructure containing ModelMap for C API */
  carTypeParsing_InitializeDataMapInfo((&carTypeParsing_M), &carTypeParsing_P);
}

/* Model terminate function */
void carTypeParsing::terminate() { /* (no terminate code required) */
}

/* Constructor */
carTypeParsing::carTypeParsing() : carTypeParsing_Y(), carTypeParsing_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
carTypeParsing::~carTypeParsing() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_carTypeParsing_T* carTypeParsing::getRTM() { return (&carTypeParsing_M); }
