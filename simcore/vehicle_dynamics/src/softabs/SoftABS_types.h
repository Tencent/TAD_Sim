/*
 * SoftABS_types.h
 *
 * Code generation for model "SoftABS".
 *
 * Model version              : 2.108
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Fri Jul 21 17:32:21 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_SoftABS_types_h_
#define RTW_HEADER_SoftABS_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_Ctrl_Input_
#  define DEFINED_TYPEDEF_FOR_Ctrl_Input_

struct Ctrl_Input {
  uint8_T acc_control_mode;
  uint8_T brake_control_mode;
  uint8_T steer_control_mode;
  real_T target_acc_m_s2;
  real_T request_torque;
  real_T brake_pressure_01;
  real_T steer_request_torque;
  real_T steer_cmd_rad;
  uint8_T gear_cmd__0N1D2R3P;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_powecu_in_
#  define DEFINED_TYPEDEF_FOR_powecu_in_

struct powecu_in {
  real_T acc_feedback_m_s2;
  real_T eng_spd_rad_s;
  real_T batt_soc_0_1;
  real_T diff_trq_req_Nm;
  real_T slip_ratio[4];
  real_T veh_speed_vx;
  Ctrl_Input driver_input;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_powecu_out_
#  define DEFINED_TYPEDEF_FOR_powecu_out_

struct powecu_out {
  real_T pt_trq_cmd_Nm;
  real_T brake_pressure_0_1;
  real_T brake_pressure_Pa[4];
  real_T abs_flag[4];
  real_T throttle_01;
};

#endif

/* Parameters (default storage) */
typedef struct P_SoftABS_T_ P_SoftABS_T;

/* Forward declaration for rtModel */
typedef struct tag_RTM_SoftABS_T RT_MODEL_SoftABS_T;

#endif /* RTW_HEADER_SoftABS_types_h_ */
