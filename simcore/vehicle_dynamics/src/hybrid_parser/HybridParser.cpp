/*
 * HybridParser.cpp
 *
 * Code generation for model "HybridParser".
 *
 * Model version              : 2.4
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Fri Jul 28 13:05:33 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "HybridParser.h"

/* Model step function */
void HybridParser::step() {
  /* Outport: '<Root>/EnableP2' incorporates:
   *  Constant: '<S1>/P2'
   */
  HybridParser_Y.EnableP2 = HybridParser_P.EnableP2;

  /* Outport: '<Root>/EnableP3' incorporates:
   *  Constant: '<S1>/P3'
   */
  HybridParser_Y.EnableP3 = HybridParser_P.EnableP3;

  /* Outport: '<Root>/EnableP4' incorporates:
   *  Constant: '<S1>/P4'
   */
  HybridParser_Y.EnableP4 = HybridParser_P.EnableP4;

  /* Outport: '<Root>/EngToGenerator_GearRatio' incorporates:
   *  Constant: '<S1>/ReGen_Motor_GearRatio[]'
   */
  HybridParser_Y.EngToGenerator_GearRatio = HybridParser_P.ReGen_Motor_GearRatio;

  /* Outport: '<Root>/EngClutch_TimeConst[s]' incorporates:
   *  Constant: '<S1>/Engine_Clutch_TimeConst'
   */
  HybridParser_Y.EngClutch_TimeConsts = HybridParser_P.Engine_Clutch_TimeConst;

  /* Outport: '<Root>/FrontMotGearRatio' incorporates:
   *  Constant: '<S1>/front_motor_gear_ratio'
   */
  HybridParser_Y.FrontMotGearRatio = HybridParser_P.front_motor_gear_ratio;
}

/* Model initialize function */
void HybridParser::initialize() { /* (no initialization code required) */
}

/* Model terminate function */
void HybridParser::terminate() { /* (no terminate code required) */
}

/* Constructor */
HybridParser::HybridParser() : HybridParser_U(), HybridParser_Y(), HybridParser_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
HybridParser::~HybridParser() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_HybridParser_T* HybridParser::getRTM() { return (&HybridParser_M); }
