/*
 * TX_DriveLine_EV_Front.cpp
 *
 * Code generation for model "TX_DriveLine_EV_Front".
 *
 * Model version              : 1.211
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Wed Jul 19 10:42:00 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TX_DriveLine_EV_Front.h"
#include <emmintrin.h>
#include <cmath>
#include "TX_DriveLine_EV_Front_private.h"
#include "rtwtypes.h"
#include "zero_crossing_types.h"

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void TX_DriveLine_EV_Front::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
  time_T t{rtsiGetT(si)};

  time_T tnew{rtsiGetSolverStopTime(si)};

  time_T h{rtsiGetStepSize(si)};

  real_T *x{rtsiGetContStates(si)};

  ODE4_IntgData *id{static_cast<ODE4_IntgData *>(rtsiGetSolverData(si))};

  real_T *y{id->y};

  real_T *f0{id->f[0]};

  real_T *f1{id->f[1]};

  real_T *f2{id->f[2]};

  real_T *f3{id->f[3]};

  real_T temp;
  int_T i;
  int_T nXc{7};

  rtsiSetSimTimeStep(si, MINOR_TIME_STEP);

  /* Save the state values at time t in y, we'll use x as ynew. */
  (void)std::memcpy(y, x, static_cast<uint_T>(nXc) * sizeof(real_T));

  /* Assumes that rtsiSetT and ModelOutputs are up-to-date */
  /* f0 = f(t,y) */
  rtsiSetdX(si, f0);
  TX_DriveLine_EV_Front_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  TX_DriveLine_EV_Front_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  TX_DriveLine_EV_Front_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  TX_DriveLine_EV_Front_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Function for MATLAB Function: '<S7>/Limited Slip Differential' */
void TX_DriveLine_EV_Front::TX_DriveLine_EV_Front_automldiffls(const real_T u[4], real_T bw1, real_T bd, real_T bw2,
                                                               real_T Ndiff, real_T shaftSwitch, real_T Jd, real_T Jw1,
                                                               real_T Jw2, const real_T x[2], real_T y[4],
                                                               real_T xdot[2]) {
  real_T diffDir_0[8];
  real_T diffDir_1[8];
  real_T invJ_0[8];
  real_T invJ[4];
  real_T invJ_1[4];
  real_T invJ_2[2];
  real_T invJ_3[2];
  real_T NbdTerm;
  real_T NbdTerm_tmp;
  real_T term1;
  real_T term2;
  int32_T diffDir;
  NbdTerm_tmp = Ndiff * Ndiff;
  NbdTerm = NbdTerm_tmp * bd / 4.0;
  if (shaftSwitch != 0.0) {
    diffDir = -1;
  } else {
    diffDir = 1;
  }

  term1 = NbdTerm_tmp * Jd;
  term2 = (4.0 * Jw1 * Jw2 + term1 * Jw1) + Jw2 * term1;
  invJ[0] = (Jw2 * 4.0 + term1) / term2;
  NbdTerm_tmp = -term1 / term2;
  invJ[2] = NbdTerm_tmp;
  invJ[1] = NbdTerm_tmp;
  invJ[3] = (Jw1 * 4.0 + term1) / term2;
  NbdTerm_tmp = static_cast<real_T>(diffDir) / 2.0 * Ndiff;
  diffDir_0[0] = NbdTerm_tmp;
  diffDir_0[2] = -1.0;
  diffDir_0[4] = 0.0;
  diffDir_0[6] = -0.5;
  diffDir_0[1] = NbdTerm_tmp;
  diffDir_0[3] = 0.0;
  diffDir_0[5] = -1.0;
  diffDir_0[7] = 0.5;
  term1 = -(bw1 + NbdTerm);
  term2 = -(NbdTerm + bw2);
  diffDir_1[0] = NbdTerm_tmp;
  diffDir_1[4] = NbdTerm_tmp;
  for (diffDir = 0; diffDir < 2; diffDir++) {
    int32_T diffDir_tmp;
    for (diffDir_tmp = 0; diffDir_tmp < 4; diffDir_tmp++) {
      int32_T invJ_tmp;
      int32_T invJ_tmp_tmp;
      invJ_tmp_tmp = diffDir_tmp << 1;
      invJ_tmp = diffDir + invJ_tmp_tmp;
      invJ_0[invJ_tmp] = 0.0;
      invJ_0[invJ_tmp] += diffDir_0[invJ_tmp_tmp] * invJ[diffDir];
      invJ_0[invJ_tmp] += diffDir_0[invJ_tmp_tmp + 1] * invJ[diffDir + 2];
    }

    invJ_1[diffDir] = 0.0;
    invJ_1[diffDir] += invJ[diffDir] * term1;
    NbdTerm_tmp = invJ[diffDir + 2];
    invJ_1[diffDir] += NbdTerm_tmp * -NbdTerm;
    invJ_1[diffDir + 2] = 0.0;
    invJ_1[diffDir + 2] += invJ[diffDir] * -NbdTerm;
    invJ_1[diffDir + 2] += NbdTerm_tmp * term2;
    invJ_2[diffDir] = 0.0;
    invJ_2[diffDir] += invJ_0[diffDir] * u[0];
    invJ_2[diffDir] += invJ_0[diffDir + 2] * u[1];
    invJ_2[diffDir] += invJ_0[diffDir + 4] * u[2];
    invJ_2[diffDir] += invJ_0[diffDir + 6] * u[3];
    invJ_3[diffDir] = 0.0;
    invJ_3[diffDir] += invJ_1[diffDir] * x[0];
    invJ_3[diffDir] += invJ_1[diffDir + 2] * x[1];
    xdot[diffDir] = invJ_2[diffDir] + invJ_3[diffDir];
    diffDir_tmp = diffDir << 2;
    diffDir_1[diffDir_tmp + 1] = static_cast<real_T>(diffDir) - 1.0;
    diffDir_1[diffDir_tmp + 2] = 0.0 - static_cast<real_T>(diffDir);
    diffDir_1[diffDir_tmp + 3] = 0.5 - static_cast<real_T>(diffDir);
  }

  for (diffDir = 0; diffDir <= 2; diffDir += 2) {
    __m128d tmp;
    __m128d tmp_0;
    tmp = _mm_loadu_pd(&diffDir_1[diffDir]);
    tmp_0 = _mm_loadu_pd(&diffDir_1[diffDir + 4]);
    _mm_storeu_pd(&y[diffDir], _mm_add_pd(_mm_add_pd(_mm_mul_pd(tmp_0, _mm_set1_pd(x[1])),
                                                     _mm_add_pd(_mm_mul_pd(tmp, _mm_set1_pd(x[0])), _mm_set1_pd(0.0))),
                                          _mm_set1_pd(0.0 * u[3] + (0.0 * u[2] + (0.0 * u[1] + 0.0 * u[0])))));
  }
}

/* Model step function */
void TX_DriveLine_EV_Front::step() {
  real_T a__2[4];
  real_T rtb_Gain_c[4];
  real_T rtb_Gain_a;
  real_T rtb_Gain_p;
  real_T rtb_Product3_g;
  real_T rtb_Product5_n;
  real_T rtb_Switch_j;
  real_T rtb_UnaryMinus_a;
  real_T rtb_UnaryMinus_p;
  real_T rtb_VectorConcatenate_i_idx_0;
  real_T rtb_mu;
  real_T tmp;
  ZCEventType zcEvent;
  if (rtmIsMajorTimeStep((&TX_DriveLine_EV_Front_M))) {
    /* set solver stop time */
    if (!((&TX_DriveLine_EV_Front_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(&(&TX_DriveLine_EV_Front_M)->solverInfo,
                            (((&TX_DriveLine_EV_Front_M)->Timing.clockTickH0 + 1) *
                             (&TX_DriveLine_EV_Front_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(
          &(&TX_DriveLine_EV_Front_M)->solverInfo,
          (((&TX_DriveLine_EV_Front_M)->Timing.clockTick0 + 1) * (&TX_DriveLine_EV_Front_M)->Timing.stepSize0 +
           (&TX_DriveLine_EV_Front_M)->Timing.clockTickH0 * (&TX_DriveLine_EV_Front_M)->Timing.stepSize0 *
               4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&TX_DriveLine_EV_Front_M))) {
    (&TX_DriveLine_EV_Front_M)->Timing.t[0] = rtsiGetT(&(&TX_DriveLine_EV_Front_M)->solverInfo);
  }

  if (rtmIsMajorTimeStep((&TX_DriveLine_EV_Front_M))) {
    /* Memory: '<S42>/Memory' */
    TX_DriveLine_EV_Front_B.Memory = TX_DriveLine_EV_Front_DW.Memory_PreviousInput;

    /* Constant: '<S41>/domega_o' */
    TX_DriveLine_EV_Front_B.domega_o = TX_DriveLine_EV_Front_P.TorsionalCompliance2_domega_o;
  }

  /* Integrator: '<S42>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_EV_Front_M)->solverInfo)) {
    zcEvent = rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_EV_Front_PrevZCX.Integrator_Reset_ZCE,
                       (TX_DriveLine_EV_Front_B.Memory));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_EV_Front_DW.Integrator_IWORK != 0)) {
      TX_DriveLine_EV_Front_X.Integrator_CSTATE = TX_DriveLine_EV_Front_B.domega_o;
    }
  }

  /* Sum: '<S41>/Subtract1' incorporates:
   *  Gain: '<S41>/Gain1'
   *  Gain: '<S41>/Gain2'
   *  Integrator: '<S41>/Integrator'
   *  Integrator: '<S42>/Integrator'
   */
  rtb_UnaryMinus_p = TX_DriveLine_EV_Front_P.TorsionalCompliance2_b * TX_DriveLine_EV_Front_X.Integrator_CSTATE +
                     TX_DriveLine_EV_Front_P.TorsionalCompliance2_k * TX_DriveLine_EV_Front_X.Integrator_CSTATE_g;

  /* SignalConversion generated from: '<S3>/Vector Concatenate' incorporates:
   *  Outport: '<Root>/DrivelineOut'
   */
  TX_DriveLine_EV_Front_Y.DrivelineOut.wheel_drive_trq_Nm[0] = rtb_UnaryMinus_p;
  if (rtmIsMajorTimeStep((&TX_DriveLine_EV_Front_M))) {
    /* Memory: '<S34>/Memory' */
    TX_DriveLine_EV_Front_B.Memory_j = TX_DriveLine_EV_Front_DW.Memory_PreviousInput_m;

    /* Constant: '<S33>/domega_o' */
    TX_DriveLine_EV_Front_B.domega_o_a = TX_DriveLine_EV_Front_P.TorsionalCompliance1_domega_o;
  }

  /* Integrator: '<S34>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_EV_Front_M)->solverInfo)) {
    zcEvent = rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_EV_Front_PrevZCX.Integrator_Reset_ZCE_k,
                       (TX_DriveLine_EV_Front_B.Memory_j));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_EV_Front_DW.Integrator_IWORK_o != 0)) {
      TX_DriveLine_EV_Front_X.Integrator_CSTATE_f = TX_DriveLine_EV_Front_B.domega_o_a;
    }
  }

  /* Sum: '<S33>/Subtract1' incorporates:
   *  Gain: '<S33>/Gain1'
   *  Gain: '<S33>/Gain2'
   *  Integrator: '<S33>/Integrator'
   *  Integrator: '<S34>/Integrator'
   */
  rtb_UnaryMinus_a = TX_DriveLine_EV_Front_P.TorsionalCompliance1_b * TX_DriveLine_EV_Front_X.Integrator_CSTATE_f +
                     TX_DriveLine_EV_Front_P.TorsionalCompliance1_k * TX_DriveLine_EV_Front_X.Integrator_CSTATE_m;

  /* SignalConversion generated from: '<S3>/Vector Concatenate' incorporates:
   *  Outport: '<Root>/DrivelineOut'
   */
  TX_DriveLine_EV_Front_Y.DrivelineOut.wheel_drive_trq_Nm[1] = rtb_UnaryMinus_a;

  /* SignalConversion generated from: '<S3>/Vector Concatenate' incorporates:
   *  Outport: '<Root>/DrivelineOut'
   */
  TX_DriveLine_EV_Front_Y.DrivelineOut.wheel_drive_trq_Nm[2] = 0.0;

  /* SignalConversion generated from: '<S3>/Vector Concatenate' incorporates:
   *  Outport: '<Root>/DrivelineOut'
   */
  TX_DriveLine_EV_Front_Y.DrivelineOut.wheel_drive_trq_Nm[3] = 0.0;
  if (rtmIsMajorTimeStep((&TX_DriveLine_EV_Front_M))) {
    /* Switch: '<S13>/Switch' incorporates:
     *  Constant: '<S13>/Constant'
     */
    if (TX_DriveLine_EV_Front_P.Constant_Value_n > TX_DriveLine_EV_Front_P.Switch_Threshold) {
      /* Switch: '<S13>/Switch' incorporates:
       *  Constant: '<S13>/Constant1'
       */
      TX_DriveLine_EV_Front_B.diffDir = TX_DriveLine_EV_Front_P.Constant1_Value_h;
    } else {
      /* Switch: '<S13>/Switch' incorporates:
       *  Constant: '<S13>/Constant1'
       *  UnaryMinus: '<S13>/Unary Minus'
       */
      TX_DriveLine_EV_Front_B.diffDir = -TX_DriveLine_EV_Front_P.Constant1_Value_h;
    }

    /* End of Switch: '<S13>/Switch' */

    /* Constant: '<S7>/Constant' incorporates:
     *  Concatenate: '<S7>/Vector Concatenate'
     */
    TX_DriveLine_EV_Front_B.VectorConcatenate[0] =
        -TX_DriveLine_EV_Front_P.VEH.InitialLongVel / TX_DriveLine_EV_Front_P.StatLdWhlR[0];

    /* Constant: '<S7>/Constant1' incorporates:
     *  Concatenate: '<S7>/Vector Concatenate'
     */
    TX_DriveLine_EV_Front_B.VectorConcatenate[1] =
        -TX_DriveLine_EV_Front_P.VEH.InitialLongVel / TX_DriveLine_EV_Front_P.StatLdWhlR[0];
  }

  /* Integrator: '<S7>/Integrator' */
  /* Limited  Integrator  */
  if (TX_DriveLine_EV_Front_DW.Integrator_IWORK_c != 0) {
    TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] = TX_DriveLine_EV_Front_B.VectorConcatenate[0];
    TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] = TX_DriveLine_EV_Front_B.VectorConcatenate[1];
  }

  /* Gain: '<S13>/Gain' */
  rtb_Gain_a = TX_DriveLine_EV_Front_P.ratio_diff_front / 2.0;

  /* Integrator: '<S7>/Integrator' */
  if (TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] >= TX_DriveLine_EV_Front_P.Integrator_UpperSat) {
    TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] = TX_DriveLine_EV_Front_P.Integrator_UpperSat;
  } else if (TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] <= TX_DriveLine_EV_Front_P.Integrator_LowerSat) {
    TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] = TX_DriveLine_EV_Front_P.Integrator_LowerSat;
  }

  if (TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] >= TX_DriveLine_EV_Front_P.Integrator_UpperSat) {
    TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] = TX_DriveLine_EV_Front_P.Integrator_UpperSat;
  } else if (TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] <= TX_DriveLine_EV_Front_P.Integrator_LowerSat) {
    TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] = TX_DriveLine_EV_Front_P.Integrator_LowerSat;
  }

  /* Sum: '<S13>/Sum of Elements' incorporates:
   *  Gain: '<S13>/Gain'
   *  Integrator: '<S7>/Integrator'
   *  Product: '<S13>/Product'
   */
  rtb_mu = TX_DriveLine_EV_Front_B.diffDir * TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] * rtb_Gain_a +
           TX_DriveLine_EV_Front_B.diffDir * TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] * rtb_Gain_a;

  /* Outport: '<Root>/DrivelineOut' incorporates:
   *  Constant: '<S1>/Constant'
   *  Constant: '<S1>/Constant1'
   *  Constant: '<S1>/Constant2'
   *  Constant: '<S3>/Constant'
   *  Gain: '<S3>/Gain1'
   */
  TX_DriveLine_EV_Front_Y.DrivelineOut.gear_engaged = TX_DriveLine_EV_Front_P.Constant_Value;
  TX_DriveLine_EV_Front_Y.DrivelineOut.eng_spd_rad_s = TX_DriveLine_EV_Front_P.Constant1_Value;
  TX_DriveLine_EV_Front_Y.DrivelineOut.trans_out_shaft_spd_rad_s = TX_DriveLine_EV_Front_P.Constant2_Value;
  TX_DriveLine_EV_Front_Y.DrivelineOut.front_mot_spd_rad_s = TX_DriveLine_EV_Front_P.front_motor_gear_ratio * rtb_mu;
  TX_DriveLine_EV_Front_Y.DrivelineOut.rear_mot_spd_rad_s = TX_DriveLine_EV_Front_P.Constant_Value_b;
  if (rtmIsMajorTimeStep((&TX_DriveLine_EV_Front_M))) {
    /* Gain: '<S7>/2*pi' incorporates:
     *  Constant: '<S7>/Constant3'
     *  Product: '<S7>/Product1'
     */
    TX_DriveLine_EV_Front_B.upi =
        1.0 / TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_tauC * TX_DriveLine_EV_Front_P.upi_Gain;
  }

  /* Gain: '<S3>/Gain' incorporates:
   *  Inport: '<Root>/DriveLineIn'
   */
  rtb_Gain_a = TX_DriveLine_EV_Front_P.front_motor_gear_ratio * TX_DriveLine_EV_Front_U.DriveLineIn.mot_front_trq_Nm;

  /* SignalConversion generated from: '<S13>/Vector Concatenate' */
  rtb_VectorConcatenate_i_idx_0 = rtb_mu;

  /* Sum: '<S13>/Add' incorporates:
   *  Gain: '<S13>/Gain1'
   *  Integrator: '<S7>/Integrator'
   */
  rtb_mu = TX_DriveLine_EV_Front_P.Gain1_Gain * TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] -
           TX_DriveLine_EV_Front_P.Gain1_Gain * TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1];

  /* Gain: '<S31>/Gain' incorporates:
   *  Abs: '<S30>/Abs'
   *  Constant: '<S31>/Constant1'
   *  Gain: '<S31>/Gain1'
   *  Gain: '<S31>/Gain2'
   *  Sum: '<S31>/Subtract1'
   *  Sum: '<S31>/Subtract2'
   *  Trigonometry: '<S31>/Trigonometric Function'
   */
  rtb_Switch_j = (std::tanh((TX_DriveLine_EV_Front_P.Gain1_Gain_c * std::abs(rtb_VectorConcatenate_i_idx_0) -
                             TX_DriveLine_EV_Front_P.Constant1_Value_hc) *
                            TX_DriveLine_EV_Front_P.Gain2_Gain) +
                  TX_DriveLine_EV_Front_P.Constant1_Value_hc) *
                 TX_DriveLine_EV_Front_P.Gain_Gain;

  /* Gain: '<S27>/Gain' incorporates:
   *  Abs: '<S26>/Abs'
   *  Constant: '<S27>/Constant1'
   *  Gain: '<S27>/Gain1'
   *  Gain: '<S27>/Gain2'
   *  Integrator: '<S7>/Integrator'
   *  Sum: '<S27>/Subtract1'
   *  Sum: '<S27>/Subtract2'
   *  Trigonometry: '<S27>/Trigonometric Function'
   *  UnaryMinus: '<S13>/Unary Minus1'
   */
  rtb_Gain_p =
      (std::tanh((TX_DriveLine_EV_Front_P.Gain1_Gain_d * std::abs(-TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0]) -
                  TX_DriveLine_EV_Front_P.Constant1_Value_o) *
                 TX_DriveLine_EV_Front_P.Gain2_Gain_d) +
       TX_DriveLine_EV_Front_P.Constant1_Value_o) *
      TX_DriveLine_EV_Front_P.Gain_Gain_n;

  /* Switch: '<S22>/Switch' incorporates:
   *  Constant: '<S22>/Constant'
   *  Constant: '<S32>/Constant'
   *  Integrator: '<S7>/Integrator'
   *  Product: '<S22>/Product1'
   *  UnaryMinus: '<S13>/Unary Minus1'
   *  UnaryMinus: '<S41>/Unary Minus'
   */
  if (-rtb_UnaryMinus_p * -TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] >
      TX_DriveLine_EV_Front_P.Switch_Threshold_e) {
    tmp = TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_eta;
  } else {
    tmp = TX_DriveLine_EV_Front_P.Constant_Value_hv;
  }

  /* Product: '<S26>/Product5' incorporates:
   *  Switch: '<S22>/Switch'
   */
  rtb_Product5_n = tmp * rtb_Gain_p;

  /* Product: '<S26>/Product3' incorporates:
   *  Constant: '<S26>/Constant'
   *  Constant: '<S27>/Constant1'
   *  Sum: '<S27>/Subtract'
   */
  rtb_Product3_g = (TX_DriveLine_EV_Front_P.Constant1_Value_o - rtb_Gain_p) * TX_DriveLine_EV_Front_P.Constant_Value_i;

  /* Gain: '<S29>/Gain' incorporates:
   *  Abs: '<S28>/Abs'
   *  Constant: '<S29>/Constant1'
   *  Gain: '<S29>/Gain1'
   *  Gain: '<S29>/Gain2'
   *  Integrator: '<S7>/Integrator'
   *  Sum: '<S29>/Subtract1'
   *  Sum: '<S29>/Subtract2'
   *  Trigonometry: '<S29>/Trigonometric Function'
   *  UnaryMinus: '<S13>/Unary Minus1'
   */
  rtb_Gain_p =
      (std::tanh((TX_DriveLine_EV_Front_P.Gain1_Gain_l * std::abs(-TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1]) -
                  TX_DriveLine_EV_Front_P.Constant1_Value_p) *
                 TX_DriveLine_EV_Front_P.Gain2_Gain_ds) +
       TX_DriveLine_EV_Front_P.Constant1_Value_p) *
      TX_DriveLine_EV_Front_P.Gain_Gain_k;

  /* MATLAB Function: '<S7>/Limited Slip Differential' incorporates:
   *  Constant: '<S7>/Jd'
   *  Constant: '<S7>/Jw1'
   *  Constant: '<S7>/Jw3'
   *  Constant: '<S7>/Ndiff2'
   *  Constant: '<S7>/bd'
   *  Constant: '<S7>/bw1'
   *  Constant: '<S7>/bw2'
   *  Integrator: '<S7>/Integrator'
   *  Product: '<S24>/Product1'
   *  Product: '<S24>/Product4'
   *  Switch: '<S24>/Switch'
   */
  if (TX_DriveLine_EV_Front_P.LimitedSlipDifferential_shaftSwitchMask == 1.0) {
    /* Product: '<S24>/Product4' incorporates:
     *  Constant: '<S24>/Constant'
     *  Constant: '<S32>/Constant'
     *  Product: '<S24>/Product1'
     *  Switch: '<S24>/Switch'
     */
    if (rtb_Gain_a * rtb_VectorConcatenate_i_idx_0 > TX_DriveLine_EV_Front_P.Switch_Threshold_f) {
      tmp = TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_eta;
    } else {
      tmp = TX_DriveLine_EV_Front_P.Constant_Value_g;
    }

    /* SignalConversion generated from: '<S12>/ SFunction ' incorporates:
     *  Constant: '<S30>/Constant'
     *  Constant: '<S31>/Constant1'
     *  Product: '<S22>/Product4'
     *  Product: '<S24>/Product4'
     *  Product: '<S30>/Product3'
     *  Product: '<S30>/Product5'
     *  Sum: '<S26>/Subtract1'
     *  Sum: '<S30>/Subtract1'
     *  Sum: '<S31>/Subtract'
     *  Switch: '<S24>/Switch'
     *  UnaryMinus: '<S41>/Unary Minus'
     */
    rtb_Gain_c[0] =
        ((TX_DriveLine_EV_Front_P.Constant1_Value_hc - rtb_Switch_j) * TX_DriveLine_EV_Front_P.Constant_Value_h +
         tmp * rtb_Switch_j) *
        rtb_Gain_a;
    rtb_Gain_c[1] = (rtb_Product5_n + rtb_Product3_g) * -rtb_UnaryMinus_p;

    /* Product: '<S23>/Product4' incorporates:
     *  Constant: '<S23>/Constant'
     *  Constant: '<S32>/Constant'
     *  Integrator: '<S7>/Integrator'
     *  Product: '<S23>/Product1'
     *  Switch: '<S23>/Switch'
     *  UnaryMinus: '<S13>/Unary Minus1'
     *  UnaryMinus: '<S33>/Unary Minus'
     */
    if (-rtb_UnaryMinus_a * -TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] >
        TX_DriveLine_EV_Front_P.Switch_Threshold_l) {
      tmp = TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_eta;
    } else {
      tmp = TX_DriveLine_EV_Front_P.Constant_Value_h2;
    }

    /* SignalConversion generated from: '<S12>/ SFunction ' incorporates:
     *  Constant: '<S28>/Constant'
     *  Constant: '<S29>/Constant1'
     *  Integrator: '<S9>/Integrator'
     *  Product: '<S23>/Product4'
     *  Product: '<S28>/Product3'
     *  Product: '<S28>/Product5'
     *  Sum: '<S28>/Subtract1'
     *  Sum: '<S29>/Subtract'
     *  Switch: '<S23>/Switch'
     *  UnaryMinus: '<S33>/Unary Minus'
     */
    rtb_Gain_c[2] =
        ((TX_DriveLine_EV_Front_P.Constant1_Value_p - rtb_Gain_p) * TX_DriveLine_EV_Front_P.Constant_Value_iv +
         tmp * rtb_Gain_p) *
        -rtb_UnaryMinus_a;
    rtb_Gain_c[3] = TX_DriveLine_EV_Front_X.Integrator_CSTATE_i;
    TX_DriveLine_EV_Front_automldiffls(rtb_Gain_c, TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_bw1,
                                       TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_bd,
                                       TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_bw2,
                                       TX_DriveLine_EV_Front_P.ratio_diff_front, 1.0,
                                       TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_Jd,
                                       TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_Jw1,
                                       TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_Jw2,
                                       TX_DriveLine_EV_Front_X.Integrator_CSTATE_j, a__2, TX_DriveLine_EV_Front_B.xdot);
  } else {
    if (rtb_Gain_a * rtb_VectorConcatenate_i_idx_0 > TX_DriveLine_EV_Front_P.Switch_Threshold_f) {
      /* Product: '<S24>/Product4' incorporates:
       *  Constant: '<S32>/Constant'
       *  Switch: '<S24>/Switch'
       */
      tmp = TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_eta;
    } else {
      /* Product: '<S24>/Product4' incorporates:
       *  Constant: '<S24>/Constant'
       *  Switch: '<S24>/Switch'
       */
      tmp = TX_DriveLine_EV_Front_P.Constant_Value_g;
    }

    /* SignalConversion generated from: '<S12>/ SFunction ' incorporates:
     *  Constant: '<S30>/Constant'
     *  Constant: '<S31>/Constant1'
     *  Product: '<S22>/Product4'
     *  Product: '<S24>/Product4'
     *  Product: '<S30>/Product3'
     *  Product: '<S30>/Product5'
     *  Sum: '<S26>/Subtract1'
     *  Sum: '<S30>/Subtract1'
     *  Sum: '<S31>/Subtract'
     *  Switch: '<S24>/Switch'
     *  UnaryMinus: '<S41>/Unary Minus'
     */
    rtb_Gain_c[0] =
        ((TX_DriveLine_EV_Front_P.Constant1_Value_hc - rtb_Switch_j) * TX_DriveLine_EV_Front_P.Constant_Value_h +
         tmp * rtb_Switch_j) *
        rtb_Gain_a;
    rtb_Gain_c[1] = (rtb_Product5_n + rtb_Product3_g) * -rtb_UnaryMinus_p;

    /* Product: '<S23>/Product4' incorporates:
     *  Constant: '<S23>/Constant'
     *  Constant: '<S32>/Constant'
     *  Integrator: '<S7>/Integrator'
     *  Product: '<S23>/Product1'
     *  Switch: '<S23>/Switch'
     *  UnaryMinus: '<S13>/Unary Minus1'
     *  UnaryMinus: '<S33>/Unary Minus'
     */
    if (-rtb_UnaryMinus_a * -TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] >
        TX_DriveLine_EV_Front_P.Switch_Threshold_l) {
      tmp = TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_eta;
    } else {
      tmp = TX_DriveLine_EV_Front_P.Constant_Value_h2;
    }

    /* SignalConversion generated from: '<S12>/ SFunction ' incorporates:
     *  Constant: '<S28>/Constant'
     *  Constant: '<S29>/Constant1'
     *  Integrator: '<S9>/Integrator'
     *  Product: '<S23>/Product4'
     *  Product: '<S28>/Product3'
     *  Product: '<S28>/Product5'
     *  Sum: '<S28>/Subtract1'
     *  Sum: '<S29>/Subtract'
     *  Switch: '<S23>/Switch'
     *  UnaryMinus: '<S33>/Unary Minus'
     */
    rtb_Gain_c[2] =
        ((TX_DriveLine_EV_Front_P.Constant1_Value_p - rtb_Gain_p) * TX_DriveLine_EV_Front_P.Constant_Value_iv +
         tmp * rtb_Gain_p) *
        -rtb_UnaryMinus_a;
    rtb_Gain_c[3] = TX_DriveLine_EV_Front_X.Integrator_CSTATE_i;
    TX_DriveLine_EV_Front_automldiffls(rtb_Gain_c, TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_bw1,
                                       TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_bd,
                                       TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_bw2,
                                       TX_DriveLine_EV_Front_P.ratio_diff_front, 0.0,
                                       TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_Jd,
                                       TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_Jw1,
                                       TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_Jw2,
                                       TX_DriveLine_EV_Front_X.Integrator_CSTATE_j, a__2, TX_DriveLine_EV_Front_B.xdot);
  }

  /* End of MATLAB Function: '<S7>/Limited Slip Differential' */
  if (rtmIsMajorTimeStep((&TX_DriveLine_EV_Front_M))) {
    /* Switch: '<S16>/Switch' incorporates:
     *  Constant: '<S16>/Constant'
     */
    if (TX_DriveLine_EV_Front_P.Constant_Value_hw > TX_DriveLine_EV_Front_P.Switch_Threshold_p) {
      /* Switch: '<S16>/Switch' incorporates:
       *  Constant: '<S16>/Constant2'
       */
      TX_DriveLine_EV_Front_B.diffDir_j = TX_DriveLine_EV_Front_P.Constant2_Value_e;
    } else {
      /* Switch: '<S16>/Switch' incorporates:
       *  Constant: '<S16>/Constant2'
       *  UnaryMinus: '<S16>/Unary Minus'
       */
      TX_DriveLine_EV_Front_B.diffDir_j = -TX_DriveLine_EV_Front_P.Constant2_Value_e;
    }

    /* End of Switch: '<S16>/Switch' */
  }

  /* Product: '<S9>/Product' incorporates:
   *  Abs: '<S21>/Abs'
   *  Constant: '<S21>/Constant'
   *  Constant: '<S21>/Constant1'
   *  Constant: '<S21>/Constant2'
   *  Gain: '<S21>/Gain'
   *  Integrator: '<S9>/Integrator'
   *  Lookup_n-D: '<S21>/mu Table'
   *  Product: '<S21>/Product'
   *  Sum: '<S9>/Sum'
   *  Trigonometry: '<S21>/Trigonometric Function'
   */
  TX_DriveLine_EV_Front_B.Product =
      (TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_Fc *
           TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_Ndisks *
           TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_Reff *
           std::tanh(TX_DriveLine_EV_Front_P.Gain_Gain_p * rtb_mu) *
           look1_binlxpw(std::abs(rtb_mu), TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_dw,
                         TX_DriveLine_EV_Front_P.EVFrontLimitedSlipDifferential_muc, 7U) -
       TX_DriveLine_EV_Front_X.Integrator_CSTATE_i) *
      TX_DriveLine_EV_Front_B.upi;

  /* Sum: '<S33>/Subtract' incorporates:
   *  Inport: '<Root>/DriveLineIn'
   *  Integrator: '<S7>/Integrator'
   *  UnaryMinus: '<S13>/Unary Minus1'
   */
  TX_DriveLine_EV_Front_B.Subtract =
      -TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] - TX_DriveLine_EV_Front_U.DriveLineIn.wheel_spd_rad_s[1];

  /* Switch: '<S34>/Switch' incorporates:
   *  Integrator: '<S34>/Integrator'
   */
  if (TX_DriveLine_EV_Front_B.Memory_j != 0.0) {
    tmp = TX_DriveLine_EV_Front_X.Integrator_CSTATE_f;
  } else {
    tmp = TX_DriveLine_EV_Front_B.domega_o_a;
  }

  /* Product: '<S34>/Product' incorporates:
   *  Constant: '<S33>/omega_c'
   *  Sum: '<S34>/Sum'
   *  Switch: '<S34>/Switch'
   */
  TX_DriveLine_EV_Front_B.Product_n =
      (TX_DriveLine_EV_Front_B.Subtract - tmp) * TX_DriveLine_EV_Front_P.TorsionalCompliance1_omega_c;

  /* Sum: '<S41>/Subtract' incorporates:
   *  Inport: '<Root>/DriveLineIn'
   *  Integrator: '<S7>/Integrator'
   *  UnaryMinus: '<S13>/Unary Minus1'
   */
  TX_DriveLine_EV_Front_B.Subtract_c =
      -TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] - TX_DriveLine_EV_Front_U.DriveLineIn.wheel_spd_rad_s[0];

  /* Switch: '<S42>/Switch' incorporates:
   *  Integrator: '<S42>/Integrator'
   */
  if (TX_DriveLine_EV_Front_B.Memory != 0.0) {
    tmp = TX_DriveLine_EV_Front_X.Integrator_CSTATE;
  } else {
    tmp = TX_DriveLine_EV_Front_B.domega_o;
  }

  /* Product: '<S42>/Product' incorporates:
   *  Constant: '<S41>/omega_c'
   *  Sum: '<S42>/Sum'
   *  Switch: '<S42>/Switch'
   */
  TX_DriveLine_EV_Front_B.Product_c =
      (TX_DriveLine_EV_Front_B.Subtract_c - tmp) * TX_DriveLine_EV_Front_P.TorsionalCompliance2_omega_c;
  if (rtmIsMajorTimeStep((&TX_DriveLine_EV_Front_M))) {
    if (rtmIsMajorTimeStep((&TX_DriveLine_EV_Front_M))) {
      /* Update for Memory: '<S42>/Memory' incorporates:
       *  Constant: '<S42>/Reset'
       */
      TX_DriveLine_EV_Front_DW.Memory_PreviousInput = TX_DriveLine_EV_Front_P.Reset_Value_k;

      /* Update for Memory: '<S34>/Memory' incorporates:
       *  Constant: '<S34>/Reset'
       */
      TX_DriveLine_EV_Front_DW.Memory_PreviousInput_m = TX_DriveLine_EV_Front_P.Reset_Value;
    }

    /* Update for Integrator: '<S42>/Integrator' */
    TX_DriveLine_EV_Front_DW.Integrator_IWORK = 0;

    /* Update for Integrator: '<S34>/Integrator' */
    TX_DriveLine_EV_Front_DW.Integrator_IWORK_o = 0;

    /* Update for Integrator: '<S7>/Integrator' */
    TX_DriveLine_EV_Front_DW.Integrator_IWORK_c = 0;
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&TX_DriveLine_EV_Front_M))) {
    rt_ertODEUpdateContinuousStates(&(&TX_DriveLine_EV_Front_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&TX_DriveLine_EV_Front_M)->Timing.clockTick0)) {
      ++(&TX_DriveLine_EV_Front_M)->Timing.clockTickH0;
    }

    (&TX_DriveLine_EV_Front_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&TX_DriveLine_EV_Front_M)->solverInfo);

    {
      /* Update absolute timer for sample time: [0.001s, 0.0s] */
      /* The "clockTick1" counts the number of times the code of this task has
       * been executed. The resolution of this integer timer is 0.001, which is the step size
       * of the task. Size of "clockTick1" ensures timer will not overflow during the
       * application lifespan selected.
       * Timer of this task consists of two 32 bit unsigned integers.
       * The two integers represent the low bits Timing.clockTick1 and the high bits
       * Timing.clockTickH1. When the low bit overflows to 0, the high bits increment.
       */
      (&TX_DriveLine_EV_Front_M)->Timing.clockTick1++;
      if (!(&TX_DriveLine_EV_Front_M)->Timing.clockTick1) {
        (&TX_DriveLine_EV_Front_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void TX_DriveLine_EV_Front::TX_DriveLine_EV_Front_derivatives() {
  XDot_TX_DriveLine_EV_Front_T *_rtXdot;
  boolean_T lsat;
  boolean_T usat;
  _rtXdot = ((XDot_TX_DriveLine_EV_Front_T *)(&TX_DriveLine_EV_Front_M)->derivs);

  /* Derivatives for Integrator: '<S42>/Integrator' */
  _rtXdot->Integrator_CSTATE = TX_DriveLine_EV_Front_B.Product_c;

  /* Derivatives for Integrator: '<S41>/Integrator' */
  _rtXdot->Integrator_CSTATE_g = TX_DriveLine_EV_Front_B.Subtract_c;

  /* Derivatives for Integrator: '<S34>/Integrator' */
  _rtXdot->Integrator_CSTATE_f = TX_DriveLine_EV_Front_B.Product_n;

  /* Derivatives for Integrator: '<S33>/Integrator' */
  _rtXdot->Integrator_CSTATE_m = TX_DriveLine_EV_Front_B.Subtract;

  /* Derivatives for Integrator: '<S7>/Integrator' */
  lsat = (TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] <= TX_DriveLine_EV_Front_P.Integrator_LowerSat);
  usat = (TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] >= TX_DriveLine_EV_Front_P.Integrator_UpperSat);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_EV_Front_B.xdot[0] > 0.0)) ||
      (usat && (TX_DriveLine_EV_Front_B.xdot[0] < 0.0))) {
    _rtXdot->Integrator_CSTATE_j[0] = TX_DriveLine_EV_Front_B.xdot[0];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_j[0] = 0.0;
  }

  lsat = (TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] <= TX_DriveLine_EV_Front_P.Integrator_LowerSat);
  usat = (TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] >= TX_DriveLine_EV_Front_P.Integrator_UpperSat);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_EV_Front_B.xdot[1] > 0.0)) ||
      (usat && (TX_DriveLine_EV_Front_B.xdot[1] < 0.0))) {
    _rtXdot->Integrator_CSTATE_j[1] = TX_DriveLine_EV_Front_B.xdot[1];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_j[1] = 0.0;
  }

  /* End of Derivatives for Integrator: '<S7>/Integrator' */

  /* Derivatives for Integrator: '<S9>/Integrator' */
  _rtXdot->Integrator_CSTATE_i = TX_DriveLine_EV_Front_B.Product;
}

/* Model initialize function */
void TX_DriveLine_EV_Front::initialize() {
  /* Registration code */
  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&TX_DriveLine_EV_Front_M)->solverInfo, &(&TX_DriveLine_EV_Front_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&TX_DriveLine_EV_Front_M)->solverInfo, &rtmGetTPtr((&TX_DriveLine_EV_Front_M)));
    rtsiSetStepSizePtr(&(&TX_DriveLine_EV_Front_M)->solverInfo, &(&TX_DriveLine_EV_Front_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&TX_DriveLine_EV_Front_M)->solverInfo, &(&TX_DriveLine_EV_Front_M)->derivs);
    rtsiSetContStatesPtr(&(&TX_DriveLine_EV_Front_M)->solverInfo, (real_T **)&(&TX_DriveLine_EV_Front_M)->contStates);
    rtsiSetNumContStatesPtr(&(&TX_DriveLine_EV_Front_M)->solverInfo, &(&TX_DriveLine_EV_Front_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&TX_DriveLine_EV_Front_M)->solverInfo,
                                    &(&TX_DriveLine_EV_Front_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&TX_DriveLine_EV_Front_M)->solverInfo,
                                       &(&TX_DriveLine_EV_Front_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&TX_DriveLine_EV_Front_M)->solverInfo,
                                      &(&TX_DriveLine_EV_Front_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&TX_DriveLine_EV_Front_M)->solverInfo, (&rtmGetErrorStatus((&TX_DriveLine_EV_Front_M))));
    rtsiSetRTModelPtr(&(&TX_DriveLine_EV_Front_M)->solverInfo, (&TX_DriveLine_EV_Front_M));
  }

  rtsiSetSimTimeStep(&(&TX_DriveLine_EV_Front_M)->solverInfo, MAJOR_TIME_STEP);
  (&TX_DriveLine_EV_Front_M)->intgData.y = (&TX_DriveLine_EV_Front_M)->odeY;
  (&TX_DriveLine_EV_Front_M)->intgData.f[0] = (&TX_DriveLine_EV_Front_M)->odeF[0];
  (&TX_DriveLine_EV_Front_M)->intgData.f[1] = (&TX_DriveLine_EV_Front_M)->odeF[1];
  (&TX_DriveLine_EV_Front_M)->intgData.f[2] = (&TX_DriveLine_EV_Front_M)->odeF[2];
  (&TX_DriveLine_EV_Front_M)->intgData.f[3] = (&TX_DriveLine_EV_Front_M)->odeF[3];
  (&TX_DriveLine_EV_Front_M)->contStates = ((X_TX_DriveLine_EV_Front_T *)&TX_DriveLine_EV_Front_X);
  rtsiSetSolverData(&(&TX_DriveLine_EV_Front_M)->solverInfo,
                    static_cast<void *>(&(&TX_DriveLine_EV_Front_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&TX_DriveLine_EV_Front_M)->solverInfo, false);
  rtsiSetSolverName(&(&TX_DriveLine_EV_Front_M)->solverInfo, "ode4");
  rtmSetTPtr((&TX_DriveLine_EV_Front_M), &(&TX_DriveLine_EV_Front_M)->Timing.tArray[0]);
  (&TX_DriveLine_EV_Front_M)->Timing.stepSize0 = 0.001;
  rtmSetFirstInitCond((&TX_DriveLine_EV_Front_M), 1);

  /* Start for Constant: '<S41>/domega_o' */
  TX_DriveLine_EV_Front_B.domega_o = TX_DriveLine_EV_Front_P.TorsionalCompliance2_domega_o;

  /* Start for Constant: '<S33>/domega_o' */
  TX_DriveLine_EV_Front_B.domega_o_a = TX_DriveLine_EV_Front_P.TorsionalCompliance1_domega_o;
  TX_DriveLine_EV_Front_PrevZCX.Integrator_Reset_ZCE = UNINITIALIZED_ZCSIG;
  TX_DriveLine_EV_Front_PrevZCX.Integrator_Reset_ZCE_k = UNINITIALIZED_ZCSIG;

  /* InitializeConditions for Memory: '<S42>/Memory' */
  TX_DriveLine_EV_Front_DW.Memory_PreviousInput = TX_DriveLine_EV_Front_P.Memory_InitialCondition;

  /* InitializeConditions for Integrator: '<S42>/Integrator' incorporates:
   *  Integrator: '<S34>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_EV_Front_M))) {
    TX_DriveLine_EV_Front_X.Integrator_CSTATE = 0.0;
    TX_DriveLine_EV_Front_X.Integrator_CSTATE_f = 0.0;
  }

  TX_DriveLine_EV_Front_DW.Integrator_IWORK = 1;

  /* End of InitializeConditions for Integrator: '<S42>/Integrator' */

  /* InitializeConditions for Integrator: '<S41>/Integrator' */
  TX_DriveLine_EV_Front_X.Integrator_CSTATE_g = TX_DriveLine_EV_Front_P.TorsionalCompliance2_theta_o;

  /* InitializeConditions for Memory: '<S34>/Memory' */
  TX_DriveLine_EV_Front_DW.Memory_PreviousInput_m = TX_DriveLine_EV_Front_P.Memory_InitialCondition_f;

  /* InitializeConditions for Integrator: '<S34>/Integrator' */
  TX_DriveLine_EV_Front_DW.Integrator_IWORK_o = 1;

  /* InitializeConditions for Integrator: '<S33>/Integrator' */
  TX_DriveLine_EV_Front_X.Integrator_CSTATE_m = TX_DriveLine_EV_Front_P.TorsionalCompliance1_theta_o;

  /* InitializeConditions for Integrator: '<S7>/Integrator' */
  if (rtmIsFirstInitCond((&TX_DriveLine_EV_Front_M))) {
    TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[0] = 0.0;
    TX_DriveLine_EV_Front_X.Integrator_CSTATE_j[1] = 0.0;
  }

  TX_DriveLine_EV_Front_DW.Integrator_IWORK_c = 1;

  /* End of InitializeConditions for Integrator: '<S7>/Integrator' */

  /* InitializeConditions for Integrator: '<S9>/Integrator' */
  TX_DriveLine_EV_Front_X.Integrator_CSTATE_i = TX_DriveLine_EV_Front_P.Integrator_IC;

  /* set "at time zero" to false */
  if (rtmIsFirstInitCond((&TX_DriveLine_EV_Front_M))) {
    rtmSetFirstInitCond((&TX_DriveLine_EV_Front_M), 0);
  }
}

/* Model terminate function */
void TX_DriveLine_EV_Front::terminate() { /* (no terminate code required) */
}

/* Constructor */
TX_DriveLine_EV_Front::TX_DriveLine_EV_Front()
    : TX_DriveLine_EV_Front_U(),
      TX_DriveLine_EV_Front_Y(),
      TX_DriveLine_EV_Front_B(),
      TX_DriveLine_EV_Front_DW(),
      TX_DriveLine_EV_Front_X(),
      TX_DriveLine_EV_Front_PrevZCX(),
      TX_DriveLine_EV_Front_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
TX_DriveLine_EV_Front::~TX_DriveLine_EV_Front() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_TX_DriveLine_EV_Front_T *TX_DriveLine_EV_Front::getRTM() { return (&TX_DriveLine_EV_Front_M); }
