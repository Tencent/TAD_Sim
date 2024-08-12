/*
 * Code generation for system system '<S8>/PID'
 *
 * Model                      : softecu
 * Model version              : 1.169
 * Simulink Coder version : 9.1 (R2019a) 23-Nov-2018
 * C++ source code generated on : Thu May 21 14:36:09 2020
 *
 * Note that the functions contained in this file are part of a Simulink
 * model, and are not self-contained algorithms.
 */

#include "softecu_PID.h"

/* Include model header file for global data */
#include "softecu.h"
#include "softecu_private.h"

/*
 * System initialize for atomic system:
 *    '<S8>/PID'
 *    '<S15>/PID'
 *    '<S26>/PID'
 */
void softecu_PID_Init(DW_PID_softecu_T *localDW) {
  /* InitializeConditions for Memory: '<S9>/last_error' */
  localDW->last_error_PreviousInput = 0.0;

  /* InitializeConditions for Memory: '<S11>/Memory' */
  localDW->Memory_PreviousInput = 0.0;
}

/*
 * Output and update for atomic system:
 *    '<S8>/PID'
 *    '<S15>/PID'
 *    '<S26>/PID'
 */
void softecu_PID(real_T rtu_error, boolean_T rtu_reset, B_PID_softecu_T *localB, DW_PID_softecu_T *localDW,
                 real_T rtp_kp, real_T rtp_ki, real_T rtp_iemax, real_T rtp_iemin, real_T rtp_kd) {
  real_T rtb_Saturation_m;
  real_T last_error_PreviousInput_tmp;

  /* Sum: '<S11>/Add' incorporates:
   *  Gain: '<S9>/Gain'
   *  Memory: '<S11>/Memory'
   */
  rtb_Saturation_m = rtp_ki * rtu_error + localDW->Memory_PreviousInput;

  /* Saturate: '<S11>/Saturation' */
  if (rtb_Saturation_m > rtp_iemax) {
    rtb_Saturation_m = rtp_iemax;
  } else {
    if (rtb_Saturation_m < rtp_iemin) {
      rtb_Saturation_m = rtp_iemin;
    }
  }

  /* End of Saturate: '<S11>/Saturation' */

  /* Sum: '<S9>/Add1' incorporates:
   *  Gain: '<S9>/Gain1'
   *  Gain: '<S9>/Gain2'
   *  Memory: '<S9>/last_error'
   *  Sum: '<S9>/Add'
   */
  localB->pid_out = (rtp_kp * rtu_error + rtb_Saturation_m) + (rtu_error - localDW->last_error_PreviousInput) * rtp_kd;

  /* Product: '<S9>/Product' incorporates:
   *  Logic: '<S9>/Logical Operator'
   *  Product: '<S11>/Product'
   */
  last_error_PreviousInput_tmp = !rtu_reset;

  /* Update for Memory: '<S9>/last_error' incorporates:
   *  Product: '<S9>/Product'
   */
  localDW->last_error_PreviousInput = rtu_error * last_error_PreviousInput_tmp;

  /* Update for Memory: '<S11>/Memory' incorporates:
   *  Product: '<S11>/Product'
   */
  localDW->Memory_PreviousInput = rtb_Saturation_m * last_error_PreviousInput_tmp;
}
