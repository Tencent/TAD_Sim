/*
 * EMotor_types.h
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

#ifndef RTW_HEADER_EMotor_types_h_
#define RTW_HEADER_EMotor_types_h_
#include "rtwtypes.h"

/* Model Code Variants */
#ifndef DEFINED_TYPEDEF_FOR_moter_in_
#  define DEFINED_TYPEDEF_FOR_moter_in_

struct moter_in {
  real_T trq_cmd_Nm;
  real_T mot_spd_RPM;
  real_T batt_volt_V;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_motor_out_
#  define DEFINED_TYPEDEF_FOR_motor_out_

struct motor_out {
  real_T batt_curr_A;
  real_T mot_trq_Nm;
};

#endif

/* Parameters (default storage) */
typedef struct P_EMotor_T_ P_EMotor_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_EMotor_T RT_MODEL_EMotor_T;

#endif /* RTW_HEADER_EMotor_types_h_ */
