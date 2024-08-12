/*
 * softecu.h
 *
 * Code generation for model "softecu".
 *
 * Model version              : 1.169
 * Simulink Coder version : 9.1 (R2019a) 23-Nov-2018
 * C++ source code generated on : Thu May 21 14:36:09 2020
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Linux 64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_softecu_h_
#define RTW_HEADER_softecu_h_
#include <stddef.h>
#include <string.h>
#include <cmath>

#ifndef softecu_COMMON_INCLUDES_
#  define softecu_COMMON_INCLUDES_
#  include "rtwtypes.h"
#  include "rtw_continuous.h"
#  include "rtw_solver.h"


#endif /* softecu_COMMON_INCLUDES_ */

#include "softecu_types.h"

/* Shared type includes */
#include "multiword_types.h"

/* Child system includes */
#include "softecu_PID.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#  define rtmGetErrorStatus(rtm) ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#  define rtmSetErrorStatus(rtm, val) ((rtm)->errorStatus = (val))
#endif

/* Block signals (default storage) */
typedef struct {
  B_PID_softecu_T PID;   /* '<S26>/PID' */
  B_PID_softecu_T PID_d; /* '<S15>/PID' */
  B_PID_softecu_T PID_f; /* '<S8>/PID' */
} B_softecu_T;

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real_T Memory_PreviousInput;      /* '<S1>/Memory' */
  real_T Memory1_PreviousInput;     /* '<S1>/Memory1' */
  real_T Memory2_PreviousInput;     /* '<S1>/Memory2' */
  uint8_T is_active_c3_softecu_lib; /* '<S4>/Chart' */
  DW_PID_softecu_T PID;             /* '<S26>/PID' */
  DW_PID_softecu_T PID_d;           /* '<S15>/PID' */
  DW_PID_softecu_T PID_f;           /* '<S8>/PID' */
} DW_softecu_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T target_accms2;               /* '<Root>/target_acc[m//s2]' */
  real_T cur_accms2;                  /* '<Root>/cur_acc[m//s2]' */
  real_T vxms;                        /* '<Root>/vx[m|s]' */
  int32_T gear_lever_from_controller; /* '<Root>/gear_lever_from_controller[]' */
  real_T target_front_wheel_anglerad;
  /* '<Root>/target_front_wheel_angle[rad]' */
  real_T cur_front_wheel_anglerad; /* '<Root>/cur_front_wheel_angle[rad]' */
  boolean_T reset;                 /* '<Root>/reset' */
} ExtU_softecu_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T gear_lever_out1reverse0netural0drive;
  /* '<Root>/gear_lever_out[-1:reverse,0:netural,>0:drive]' */
  real_T accpedal;            /* '<Root>/accpedal[%]' */
  real_T brakepedal;          /* '<Root>/brakepedal[%]' */
  real_T steerdeg;            /* '<Root>/steer[deg]' */
  real_T softecu_signals[10]; /* '<Root>/softecu_signals' */
} ExtY_softecu_T;

/* Real-time Model Data Structure */
struct tag_RTM_softecu_T {
  const char_T *errorStatus;
};

/* Class declaration for model softecu */
class softecuModelClass {
  /* public data and function members */
 public:
  /* External inputs */
  ExtU_softecu_T softecu_U;

  /* External outputs */
  ExtY_softecu_T softecu_Y;

  /* model initialize function */
  void initialize();

  /* model step function */
  void step();

  /* model terminate function */
  void terminate();

  /* Constructor */
  softecuModelClass();

  /* Destructor */
  ~softecuModelClass();

  /* Real-Time Model get method */
  RT_MODEL_softecu_T *getRTM();

  /* private data and function members */
 private:
  /* Block signals */
  B_softecu_T softecu_B;

  /* Block states */
  DW_softecu_T softecu_DW;

  /* Real-Time Model */
  RT_MODEL_softecu_T softecu_M;
};

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'softecu'
 * '<S1>'   : 'softecu/SoftACC'
 * '<S2>'   : 'softecu/SoftACC/accpedal_pid'
 * '<S3>'   : 'softecu/SoftACC/brakepedal_pid'
 * '<S4>'   : 'softecu/SoftACC/comfort_start_up'
 * '<S5>'   : 'softecu/SoftACC/dummy_gear_lever'
 * '<S6>'   : 'softecu/SoftACC/reverse_gear_adaption'
 * '<S7>'   : 'softecu/SoftACC/steer_pid'
 * '<S8>'   : 'softecu/SoftACC/accpedal_pid/accpedal'
 * '<S9>'   : 'softecu/SoftACC/accpedal_pid/accpedal/PID'
 * '<S10>'  : 'softecu/SoftACC/accpedal_pid/accpedal/first_order_dynamic'
 * '<S11>'  : 'softecu/SoftACC/accpedal_pid/accpedal/PID/acumulator'
 * '<S12>'  : 'softecu/SoftACC/brakepedal_pid/Compare To Constant'
 * '<S13>'  : 'softecu/SoftACC/brakepedal_pid/Compare To Constant1'
 * '<S14>'  : 'softecu/SoftACC/brakepedal_pid/Compare To Zero'
 * '<S15>'  : 'softecu/SoftACC/brakepedal_pid/brakepedal'
 * '<S16>'  : 'softecu/SoftACC/brakepedal_pid/brakepedal/PID'
 * '<S17>'  : 'softecu/SoftACC/brakepedal_pid/brakepedal/first_order_dynamic'
 * '<S18>'  : 'softecu/SoftACC/brakepedal_pid/brakepedal/PID/acumulator'
 * '<S19>'  : 'softecu/SoftACC/comfort_start_up/Chart'
 * '<S20>'  : 'softecu/SoftACC/comfort_start_up/Compare To Constant'
 * '<S21>'  : 'softecu/SoftACC/comfort_start_up/Compare To Constant1'
 * '<S22>'  : 'softecu/SoftACC/comfort_start_up/Compare To Zero'
 * '<S23>'  : 'softecu/SoftACC/dummy_gear_lever/dummy_gear'
 * '<S24>'  : 'softecu/SoftACC/reverse_gear_adaption/Compare To Constant'
 * '<S25>'  : 'softecu/SoftACC/steer_pid/speed_limiter'
 * '<S26>'  : 'softecu/SoftACC/steer_pid/steer'
 * '<S27>'  : 'softecu/SoftACC/steer_pid/unit_convert'
 * '<S28>'  : 'softecu/SoftACC/steer_pid/unit_convert1'
 * '<S29>'  : 'softecu/SoftACC/steer_pid/steer/PID'
 * '<S30>'  : 'softecu/SoftACC/steer_pid/steer/PID/acumulator'
 */
#endif /* RTW_HEADER_softecu_h_ */
