/*
 * TX_DriveLine_ICE.cpp
 *
 * Code generation for model "TX_DriveLine_ICE".
 *
 * Model version              : 1.200
 * Simulink Coder version : 9.8 (R2022b) 13-May-2022
 * C++ source code generated on : Tue Jul 18 21:36:16 2023
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "TX_DriveLine_ICE.h"
#include <emmintrin.h>
#include <cmath>
#include "TX_DriveLine_ICE_private.h"
#include "rtwtypes.h"
#include "zero_crossing_types.h"

/*
 * This function updates continuous states using the ODE4 fixed-step
 * solver algorithm
 */
void TX_DriveLine_ICE::rt_ertODEUpdateContinuousStates(RTWSolverInfo *si) {
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
  int_T nXc{34};

  rtsiSetSimTimeStep(si, MINOR_TIME_STEP);

  /* Save the state values at time t in y, we'll use x as ynew. */
  (void)std::memcpy(y, x, static_cast<uint_T>(nXc) * sizeof(real_T));

  /* Assumes that rtsiSetT and ModelOutputs are up-to-date */
  /* f0 = f(t,y) */
  rtsiSetdX(si, f0);
  TX_DriveLine_ICE_derivatives();

  /* f1 = f(t + (h/2), y + (h/2)*f0) */
  temp = 0.5 * h;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f0[i]);
  }

  rtsiSetT(si, t + temp);
  rtsiSetdX(si, f1);
  this->step();
  TX_DriveLine_ICE_derivatives();

  /* f2 = f(t + (h/2), y + (h/2)*f1) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (temp * f1[i]);
  }

  rtsiSetdX(si, f2);
  this->step();
  TX_DriveLine_ICE_derivatives();

  /* f3 = f(t + h, y + h*f2) */
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + (h * f2[i]);
  }

  rtsiSetT(si, tnew);
  rtsiSetdX(si, f3);
  this->step();
  TX_DriveLine_ICE_derivatives();

  /* tnew = t + h
     ynew = y + (h/6)*(f0 + 2*f1 + 2*f2 + 2*f3) */
  temp = h / 6.0;
  for (i = 0; i < nXc; i++) {
    x[i] = y[i] + temp * (f0[i] + 2.0 * f1[i] + 2.0 * f2[i] + f3[i]);
  }

  rtsiSetSimTimeStep(si, MAJOR_TIME_STEP);
}

/* Function for MATLAB Function: '<S8>/Limited Slip Differential' */
void TX_DriveLine_ICE::TX_DriveLine_ICE_automldiffls(const real_T u[4], real_T bw1, real_T bd, real_T bw2, real_T Ndiff,
                                                     real_T shaftSwitch, real_T Jd, real_T Jw1, real_T Jw2,
                                                     const real_T x[2], real_T y[4], real_T xdot[2]) {
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

/*
 * Output and update for atomic system:
 *    '<S8>/Limited Slip Differential'
 *    '<S59>/Limited Slip Differential'
 *    '<S85>/Limited Slip Differential'
 *    '<S184>/Limited Slip Differential'
 */
void TX_DriveLine_ICE::TX_DriveLine_ICE_LimitedSlipDifferential(real_T rtu_u, real_T rtu_u_j, real_T rtu_u_m,
                                                                real_T rtu_u_n, real_T rtu_bw1, real_T rtu_bd,
                                                                real_T rtu_bw2, real_T rtu_Ndiff, real_T rtu_Jd,
                                                                real_T rtu_Jw1, real_T rtu_Jw2, const real_T rtu_x[2],
                                                                B_LimitedSlipDifferential_TX_DriveLine_ICE_T *localB,
                                                                P_LimitedSlipDifferential_TX_DriveLine_ICE_T *localP) {
  real_T a__2[4];
  real_T rtu_u_0[4];
  if (localP->LimitedSlipDifferential_shaftSwitchMask == 1.0) {
    /* SignalConversion generated from: '<S13>/ SFunction ' */
    rtu_u_0[0] = rtu_u;
    rtu_u_0[1] = rtu_u_j;
    rtu_u_0[2] = rtu_u_m;
    rtu_u_0[3] = rtu_u_n;
    TX_DriveLine_ICE_automldiffls(rtu_u_0, rtu_bw1, rtu_bd, rtu_bw2, rtu_Ndiff, 1.0, rtu_Jd, rtu_Jw1, rtu_Jw2, rtu_x,
                                  a__2, localB->xdot);
  } else {
    /* SignalConversion generated from: '<S13>/ SFunction ' */
    rtu_u_0[0] = rtu_u;
    rtu_u_0[1] = rtu_u_j;
    rtu_u_0[2] = rtu_u_m;
    rtu_u_0[3] = rtu_u_n;
    TX_DriveLine_ICE_automldiffls(rtu_u_0, rtu_bw1, rtu_bd, rtu_bw2, rtu_Ndiff, 0.0, rtu_Jd, rtu_Jw1, rtu_Jw2, rtu_x,
                                  a__2, localB->xdot);
  }
}

/* Model step function */
void TX_DriveLine_ICE::step() {
  /* local block i/o variables */
  real_T rtb_Integrator[2];
  real_T rtb_Integrator_p[2];
  real_T rtb_Product4;
  real_T rtb_Product4_k;
  real_T rtb_Product4_o;
  real_T rtb_Integrator_fd;
  real_T rtb_Integrator_g[2];
  real_T rtb_Product4_n;
  real_T rtb_Product4_f;
  real_T rtb_Product4_g;
  real_T rtb_Integrator_h;
  real_T rtb_Integrator_pn[2];
  real_T rtb_Product4_i;
  real_T rtb_Product4_e;
  real_T rtb_Product4_gf;
  real_T rtb_Integrator_c;
  real_T rtb_Product4_dr;
  real_T rtb_Product4_p;
  real_T rtb_Product4_l;
  real_T rtb_Integrator_k;
  real_T invJ_0[6];
  real_T tmp[6];
  real_T invJ[4];
  real_T invJ_1[4];
  real_T invJ_2[2];
  real_T invJ_3[2];
  real_T rtb_Add1;
  real_T rtb_Gain_c;
  real_T rtb_Gain_o_tmp;
  real_T rtb_Subtract1;
  real_T rtb_Subtract1_co;
  real_T rtb_Subtract1_d;
  real_T rtb_Subtract1_d1;
  real_T rtb_Subtract1_d3;
  real_T rtb_Subtract1_e;
  real_T rtb_Subtract1_fg;
  real_T rtb_Subtract1_fp;
  real_T rtb_Subtract1_g;
  real_T rtb_Subtract1_n;
  real_T rtb_SumofElements;
  real_T rtb_SumofElements_l;
  ZCEventType zcEvent;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* set solver stop time */
    if (!((&TX_DriveLine_ICE_M)->Timing.clockTick0 + 1)) {
      rtsiSetSolverStopTime(
          &(&TX_DriveLine_ICE_M)->solverInfo,
          (((&TX_DriveLine_ICE_M)->Timing.clockTickH0 + 1) * (&TX_DriveLine_ICE_M)->Timing.stepSize0 * 4294967296.0));
    } else {
      rtsiSetSolverStopTime(
          &(&TX_DriveLine_ICE_M)->solverInfo,
          (((&TX_DriveLine_ICE_M)->Timing.clockTick0 + 1) * (&TX_DriveLine_ICE_M)->Timing.stepSize0 +
           (&TX_DriveLine_ICE_M)->Timing.clockTickH0 * (&TX_DriveLine_ICE_M)->Timing.stepSize0 * 4294967296.0));
    }
  } /* end MajorTimeStep */

  /* Update absolute time of base rate at minor time step */
  if (rtmIsMinorTimeStep((&TX_DriveLine_ICE_M))) {
    (&TX_DriveLine_ICE_M)->Timing.t[0] = rtsiGetT(&(&TX_DriveLine_ICE_M)->solverInfo);
  }

  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Memory: '<S43>/Memory' */
    TX_DriveLine_ICE_B.Memory = TX_DriveLine_ICE_DW.Memory_PreviousInput;

    /* Constant: '<S42>/domega_o' */
    TX_DriveLine_ICE_B.domega_o = TX_DriveLine_ICE_P.TorsionalCompliance2_domega_o;
  }

  /* Integrator: '<S43>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo)) {
    zcEvent =
        rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE, (TX_DriveLine_ICE_B.Memory));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_ICE_DW.Integrator_IWORK != 0)) {
      TX_DriveLine_ICE_X.Integrator_CSTATE = TX_DriveLine_ICE_B.domega_o;
    }
  }

  /* Sum: '<S42>/Subtract1' incorporates:
   *  Gain: '<S42>/Gain1'
   *  Gain: '<S42>/Gain2'
   *  Integrator: '<S42>/Integrator'
   *  Integrator: '<S43>/Integrator'
   */
  rtb_Subtract1 = TX_DriveLine_ICE_P.TorsionalCompliance2_b * TX_DriveLine_ICE_X.Integrator_CSTATE +
                  TX_DriveLine_ICE_P.TorsionalCompliance2_k * TX_DriveLine_ICE_X.Integrator_CSTATE_j;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Memory: '<S35>/Memory' */
    TX_DriveLine_ICE_B.Memory_e = TX_DriveLine_ICE_DW.Memory_PreviousInput_b;

    /* Constant: '<S34>/domega_o' */
    TX_DriveLine_ICE_B.domega_o_e = TX_DriveLine_ICE_P.TorsionalCompliance1_domega_o;
  }

  /* Integrator: '<S35>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo)) {
    zcEvent =
        rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_h, (TX_DriveLine_ICE_B.Memory_e));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_ICE_DW.Integrator_IWORK_d != 0)) {
      TX_DriveLine_ICE_X.Integrator_CSTATE_c = TX_DriveLine_ICE_B.domega_o_e;
    }
  }

  /* Sum: '<S34>/Subtract1' incorporates:
   *  Gain: '<S34>/Gain1'
   *  Gain: '<S34>/Gain2'
   *  Integrator: '<S34>/Integrator'
   *  Integrator: '<S35>/Integrator'
   */
  rtb_Subtract1_fg = TX_DriveLine_ICE_P.TorsionalCompliance1_b * TX_DriveLine_ICE_X.Integrator_CSTATE_c +
                     TX_DriveLine_ICE_P.TorsionalCompliance1_k * TX_DriveLine_ICE_X.Integrator_CSTATE_n;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Memory: '<S211>/Memory' */
    TX_DriveLine_ICE_B.Memory_f = TX_DriveLine_ICE_DW.Memory_PreviousInput_o;

    /* Constant: '<S210>/domega_o' */
    TX_DriveLine_ICE_B.domega_o_eq = TX_DriveLine_ICE_P.TorsionalCompliance_domega_o;
  }

  /* Integrator: '<S211>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo)) {
    zcEvent =
        rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_p, (TX_DriveLine_ICE_B.Memory_f));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_ICE_DW.Integrator_IWORK_e != 0)) {
      TX_DriveLine_ICE_X.Integrator_CSTATE_d = TX_DriveLine_ICE_B.domega_o_eq;
    }
  }

  /* Sum: '<S210>/Subtract1' incorporates:
   *  Gain: '<S210>/Gain1'
   *  Gain: '<S210>/Gain2'
   *  Integrator: '<S210>/Integrator'
   *  Integrator: '<S211>/Integrator'
   */
  rtb_Subtract1_fp = TX_DriveLine_ICE_P.TorsionalCompliance_b * TX_DriveLine_ICE_X.Integrator_CSTATE_d +
                     TX_DriveLine_ICE_P.TorsionalCompliance_k * TX_DriveLine_ICE_X.Integrator_CSTATE_h;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Memory: '<S219>/Memory' */
    TX_DriveLine_ICE_B.Memory_j = TX_DriveLine_ICE_DW.Memory_PreviousInput_h;

    /* Constant: '<S218>/domega_o' */
    TX_DriveLine_ICE_B.domega_o_b = TX_DriveLine_ICE_P.TorsionalCompliance1_domega_o_f;
  }

  /* Integrator: '<S219>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo)) {
    zcEvent =
        rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_d, (TX_DriveLine_ICE_B.Memory_j));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_ICE_DW.Integrator_IWORK_o != 0)) {
      TX_DriveLine_ICE_X.Integrator_CSTATE_k = TX_DriveLine_ICE_B.domega_o_b;
    }
  }

  /* Sum: '<S218>/Subtract1' incorporates:
   *  Gain: '<S218>/Gain1'
   *  Gain: '<S218>/Gain2'
   *  Integrator: '<S218>/Integrator'
   *  Integrator: '<S219>/Integrator'
   */
  rtb_Subtract1_d1 = TX_DriveLine_ICE_P.TorsionalCompliance1_b_b * TX_DriveLine_ICE_X.Integrator_CSTATE_k +
                     TX_DriveLine_ICE_P.TorsionalCompliance1_k_b * TX_DriveLine_ICE_X.Integrator_CSTATE_b;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Memory: '<S144>/Memory' */
    TX_DriveLine_ICE_B.Memory_i = TX_DriveLine_ICE_DW.Memory_PreviousInput_i;

    /* Constant: '<S143>/domega_o' */
    TX_DriveLine_ICE_B.domega_o_br = TX_DriveLine_ICE_P.TorsionalCompliance4_domega_o;
  }

  /* Integrator: '<S144>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo)) {
    zcEvent =
        rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_e, (TX_DriveLine_ICE_B.Memory_i));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_ICE_DW.Integrator_IWORK_m != 0)) {
      TX_DriveLine_ICE_X.Integrator_CSTATE_c4 = TX_DriveLine_ICE_B.domega_o_br;
    }
  }

  /* Sum: '<S143>/Subtract1' incorporates:
   *  Gain: '<S143>/Gain1'
   *  Gain: '<S143>/Gain2'
   *  Integrator: '<S143>/Integrator'
   *  Integrator: '<S144>/Integrator'
   */
  rtb_Subtract1_d = TX_DriveLine_ICE_P.TorsionalCompliance4_b * TX_DriveLine_ICE_X.Integrator_CSTATE_c4 +
                    TX_DriveLine_ICE_P.TorsionalCompliance4_k * TX_DriveLine_ICE_X.Integrator_CSTATE_c3;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Memory: '<S152>/Memory' */
    TX_DriveLine_ICE_B.Memory_el = TX_DriveLine_ICE_DW.Memory_PreviousInput_e;

    /* Constant: '<S151>/domega_o' */
    TX_DriveLine_ICE_B.domega_o_a = TX_DriveLine_ICE_P.TorsionalCompliance5_domega_o;
  }

  /* Integrator: '<S152>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo)) {
    zcEvent = rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_de,
                       (TX_DriveLine_ICE_B.Memory_el));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_ICE_DW.Integrator_IWORK_c != 0)) {
      TX_DriveLine_ICE_X.Integrator_CSTATE_nl = TX_DriveLine_ICE_B.domega_o_a;
    }
  }

  /* Sum: '<S151>/Subtract1' incorporates:
   *  Gain: '<S151>/Gain1'
   *  Gain: '<S151>/Gain2'
   *  Integrator: '<S151>/Integrator'
   *  Integrator: '<S152>/Integrator'
   */
  rtb_Subtract1_co = TX_DriveLine_ICE_P.TorsionalCompliance5_b * TX_DriveLine_ICE_X.Integrator_CSTATE_nl +
                     TX_DriveLine_ICE_P.TorsionalCompliance5_k * TX_DriveLine_ICE_X.Integrator_CSTATE_g;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Memory: '<S128>/Memory' */
    TX_DriveLine_ICE_B.Memory_b = TX_DriveLine_ICE_DW.Memory_PreviousInput_d;

    /* Constant: '<S127>/domega_o' */
    TX_DriveLine_ICE_B.domega_o_p = TX_DriveLine_ICE_P.TorsionalCompliance2_domega_o_j;
  }

  /* Integrator: '<S128>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo)) {
    zcEvent =
        rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_c, (TX_DriveLine_ICE_B.Memory_b));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_ICE_DW.Integrator_IWORK_i != 0)) {
      TX_DriveLine_ICE_X.Integrator_CSTATE_e = TX_DriveLine_ICE_B.domega_o_p;
    }
  }

  /* Sum: '<S127>/Subtract1' incorporates:
   *  Gain: '<S127>/Gain1'
   *  Gain: '<S127>/Gain2'
   *  Integrator: '<S127>/Integrator'
   *  Integrator: '<S128>/Integrator'
   */
  rtb_Subtract1_g = TX_DriveLine_ICE_P.TorsionalCompliance2_b_f * TX_DriveLine_ICE_X.Integrator_CSTATE_e +
                    TX_DriveLine_ICE_P.TorsionalCompliance2_k_i * TX_DriveLine_ICE_X.Integrator_CSTATE_ch;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Memory: '<S136>/Memory' */
    TX_DriveLine_ICE_B.Memory_g = TX_DriveLine_ICE_DW.Memory_PreviousInput_g;

    /* Constant: '<S135>/domega_o' */
    TX_DriveLine_ICE_B.domega_o_g = TX_DriveLine_ICE_P.TorsionalCompliance3_domega_o;
  }

  /* Integrator: '<S136>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo)) {
    zcEvent =
        rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_o, (TX_DriveLine_ICE_B.Memory_g));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_ICE_DW.Integrator_IWORK_iy != 0)) {
      TX_DriveLine_ICE_X.Integrator_CSTATE_jc = TX_DriveLine_ICE_B.domega_o_g;
    }
  }

  /* Sum: '<S135>/Subtract1' incorporates:
   *  Gain: '<S135>/Gain1'
   *  Gain: '<S135>/Gain2'
   *  Integrator: '<S135>/Integrator'
   *  Integrator: '<S136>/Integrator'
   */
  rtb_Subtract1_n = TX_DriveLine_ICE_P.TorsionalCompliance3_b * TX_DriveLine_ICE_X.Integrator_CSTATE_jc +
                    TX_DriveLine_ICE_P.TorsionalCompliance3_k * TX_DriveLine_ICE_X.Integrator_CSTATE_l;

  /* MultiPortSwitch generated from: '<S1>/Multiport Switch' incorporates:
   *  Constant: '<Root>/DriveType[1-FD;2-RD;3-4WD]'
   */
  switch (static_cast<int32_T>(TX_DriveLine_ICE_P.drive_type)) {
    case 1:
      /* SignalConversion generated from: '<S2>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[3] = 0.0;

      /* SignalConversion generated from: '<S2>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[2] = 0.0;

      /* SignalConversion generated from: '<S2>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[1] = rtb_Subtract1_fg;

      /* SignalConversion generated from: '<S2>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[0] = rtb_Subtract1;
      break;

    case 2:
      /* SignalConversion generated from: '<S4>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[3] = rtb_Subtract1_d1;

      /* SignalConversion generated from: '<S4>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[2] = rtb_Subtract1_fp;

      /* SignalConversion generated from: '<S4>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[1] = 0.0;

      /* SignalConversion generated from: '<S4>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[0] = 0.0;
      break;

    default:
      /* SignalConversion generated from: '<S3>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[3] = rtb_Subtract1_n;

      /* SignalConversion generated from: '<S3>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[2] = rtb_Subtract1_g;

      /* SignalConversion generated from: '<S3>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[1] = rtb_Subtract1_co;

      /* SignalConversion generated from: '<S3>/Vector Concatenate' incorporates:
       *  Outport: '<Root>/AxlTrq[Nm]'
       */
      TX_DriveLine_ICE_Y.AxlTrqNm[0] = rtb_Subtract1_d;
      break;
  }

  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Switch: '<S14>/Switch' incorporates:
     *  Constant: '<S14>/Constant'
     */
    if (TX_DriveLine_ICE_P.Constant_Value > TX_DriveLine_ICE_P.Switch_Threshold) {
      /* Switch: '<S14>/Switch' incorporates:
       *  Constant: '<S14>/Constant1'
       */
      TX_DriveLine_ICE_B.diffDir = TX_DriveLine_ICE_P.Constant1_Value;
    } else {
      /* Switch: '<S14>/Switch' incorporates:
       *  Constant: '<S14>/Constant1'
       *  UnaryMinus: '<S14>/Unary Minus'
       */
      TX_DriveLine_ICE_B.diffDir = -TX_DriveLine_ICE_P.Constant1_Value;
    }

    /* End of Switch: '<S14>/Switch' */

    /* Constant: '<S8>/Constant' incorporates:
     *  Concatenate: '<S8>/Vector Concatenate'
     */
    TX_DriveLine_ICE_B.VectorConcatenate[0] = -TX_DriveLine_ICE_P.VEH.InitialLongVel / TX_DriveLine_ICE_P.StatLdWhlR[0];

    /* Constant: '<S8>/Constant1' incorporates:
     *  Concatenate: '<S8>/Vector Concatenate'
     */
    TX_DriveLine_ICE_B.VectorConcatenate[1] = -TX_DriveLine_ICE_P.VEH.InitialLongVel / TX_DriveLine_ICE_P.StatLdWhlR[0];
  }

  /* Integrator: '<S8>/Integrator' */
  /* Limited  Integrator  */
  if (TX_DriveLine_ICE_DW.Integrator_IWORK_ok != 0) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_hh[0] = TX_DriveLine_ICE_B.VectorConcatenate[0];
    TX_DriveLine_ICE_X.Integrator_CSTATE_hh[1] = TX_DriveLine_ICE_B.VectorConcatenate[1];
  }

  /* Gain: '<S14>/Gain' incorporates:
   *  Gain: '<S65>/Gain'
   */
  rtb_Gain_c = TX_DriveLine_ICE_P.ratio_diff_front / 2.0;

  /* Integrator: '<S8>/Integrator' */
  if (TX_DriveLine_ICE_X.Integrator_CSTATE_hh[0] >= TX_DriveLine_ICE_P.Integrator_UpperSat) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_hh[0] = TX_DriveLine_ICE_P.Integrator_UpperSat;
  } else if (TX_DriveLine_ICE_X.Integrator_CSTATE_hh[0] <= TX_DriveLine_ICE_P.Integrator_LowerSat) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_hh[0] = TX_DriveLine_ICE_P.Integrator_LowerSat;
  }

  /* Integrator: '<S8>/Integrator' */
  rtb_Integrator[0] = TX_DriveLine_ICE_X.Integrator_CSTATE_hh[0];

  /* Integrator: '<S8>/Integrator' */
  if (TX_DriveLine_ICE_X.Integrator_CSTATE_hh[1] >= TX_DriveLine_ICE_P.Integrator_UpperSat) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_hh[1] = TX_DriveLine_ICE_P.Integrator_UpperSat;
  } else if (TX_DriveLine_ICE_X.Integrator_CSTATE_hh[1] <= TX_DriveLine_ICE_P.Integrator_LowerSat) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_hh[1] = TX_DriveLine_ICE_P.Integrator_LowerSat;
  }

  /* Integrator: '<S8>/Integrator' */
  rtb_Integrator[1] = TX_DriveLine_ICE_X.Integrator_CSTATE_hh[1];

  /* Sum: '<S14>/Sum of Elements' incorporates:
   *  Gain: '<S14>/Gain'
   *  Product: '<S14>/Product'
   */
  rtb_SumofElements = TX_DriveLine_ICE_B.diffDir * rtb_Integrator[0] * rtb_Gain_c +
                      TX_DriveLine_ICE_B.diffDir * rtb_Integrator[1] * rtb_Gain_c;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Switch: '<S190>/Switch' incorporates:
     *  Constant: '<S190>/Constant'
     */
    if (TX_DriveLine_ICE_P.Constant_Value_e > TX_DriveLine_ICE_P.Switch_Threshold_f) {
      /* Switch: '<S190>/Switch' incorporates:
       *  Constant: '<S190>/Constant1'
       */
      TX_DriveLine_ICE_B.diffDir_p = TX_DriveLine_ICE_P.Constant1_Value_h;
    } else {
      /* Switch: '<S190>/Switch' incorporates:
       *  Constant: '<S190>/Constant1'
       *  UnaryMinus: '<S190>/Unary Minus'
       */
      TX_DriveLine_ICE_B.diffDir_p = -TX_DriveLine_ICE_P.Constant1_Value_h;
    }

    /* End of Switch: '<S190>/Switch' */

    /* Constant: '<S184>/Constant' incorporates:
     *  Concatenate: '<S184>/Vector Concatenate'
     */
    TX_DriveLine_ICE_B.VectorConcatenate_m[0] =
        -TX_DriveLine_ICE_P.VEH.InitialLongVel / TX_DriveLine_ICE_P.StatLdWhlR[0];

    /* Constant: '<S184>/Constant1' incorporates:
     *  Concatenate: '<S184>/Vector Concatenate'
     */
    TX_DriveLine_ICE_B.VectorConcatenate_m[1] =
        -TX_DriveLine_ICE_P.VEH.InitialLongVel / TX_DriveLine_ICE_P.StatLdWhlR[0];
  }

  /* Integrator: '<S184>/Integrator' */
  /* Limited  Integrator  */
  if (TX_DriveLine_ICE_DW.Integrator_IWORK_ec != 0) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_a[0] = TX_DriveLine_ICE_B.VectorConcatenate_m[0];
    TX_DriveLine_ICE_X.Integrator_CSTATE_a[1] = TX_DriveLine_ICE_B.VectorConcatenate_m[1];
  }

  /* Gain: '<S190>/Gain' incorporates:
   *  Gain: '<S91>/Gain'
   */
  rtb_Gain_o_tmp = TX_DriveLine_ICE_P.ratio_diff_rear / 2.0;

  /* Integrator: '<S184>/Integrator' */
  if (TX_DriveLine_ICE_X.Integrator_CSTATE_a[0] >= TX_DriveLine_ICE_P.Integrator_UpperSat_d) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_a[0] = TX_DriveLine_ICE_P.Integrator_UpperSat_d;
  } else if (TX_DriveLine_ICE_X.Integrator_CSTATE_a[0] <= TX_DriveLine_ICE_P.Integrator_LowerSat_g) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_a[0] = TX_DriveLine_ICE_P.Integrator_LowerSat_g;
  }

  /* Integrator: '<S184>/Integrator' */
  rtb_Integrator_p[0] = TX_DriveLine_ICE_X.Integrator_CSTATE_a[0];

  /* Integrator: '<S184>/Integrator' */
  if (TX_DriveLine_ICE_X.Integrator_CSTATE_a[1] >= TX_DriveLine_ICE_P.Integrator_UpperSat_d) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_a[1] = TX_DriveLine_ICE_P.Integrator_UpperSat_d;
  } else if (TX_DriveLine_ICE_X.Integrator_CSTATE_a[1] <= TX_DriveLine_ICE_P.Integrator_LowerSat_g) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_a[1] = TX_DriveLine_ICE_P.Integrator_LowerSat_g;
  }

  /* Integrator: '<S184>/Integrator' */
  rtb_Integrator_p[1] = TX_DriveLine_ICE_X.Integrator_CSTATE_a[1];

  /* Sum: '<S190>/Sum of Elements' incorporates:
   *  Gain: '<S190>/Gain'
   *  Product: '<S190>/Product'
   */
  rtb_SumofElements_l = TX_DriveLine_ICE_B.diffDir_p * rtb_Integrator_p[0] * rtb_Gain_o_tmp +
                        TX_DriveLine_ICE_B.diffDir_p * rtb_Integrator_p[1] * rtb_Gain_o_tmp;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Switch: '<S162>/Switch' incorporates:
     *  Constant: '<S162>/Constant'
     */
    if (TX_DriveLine_ICE_P.TransferCase_shaftSwitchMask > TX_DriveLine_ICE_P.Switch_Threshold_l) {
      /* Switch: '<S162>/Switch' incorporates:
       *  Constant: '<S162>/Constant1'
       */
      TX_DriveLine_ICE_B.diffDir_k = TX_DriveLine_ICE_P.Constant1_Value_m;
    } else {
      /* Switch: '<S162>/Switch' incorporates:
       *  Constant: '<S162>/Constant1'
       *  UnaryMinus: '<S162>/Unary Minus'
       */
      TX_DriveLine_ICE_B.diffDir_k = -TX_DriveLine_ICE_P.Constant1_Value_m;
    }

    /* End of Switch: '<S162>/Switch' */

    /* Constant: '<S58>/Constant' incorporates:
     *  Concatenate: '<S58>/Vector Concatenate'
     */
    TX_DriveLine_ICE_B.VectorConcatenate_n[0] =
        -TX_DriveLine_ICE_P.VEH.InitialLongVel / TX_DriveLine_ICE_P.StatLdWhlR[0] * TX_DriveLine_ICE_P.ratio_diff_front;

    /* Constant: '<S58>/Constant1' incorporates:
     *  Concatenate: '<S58>/Vector Concatenate'
     */
    TX_DriveLine_ICE_B.VectorConcatenate_n[1] =
        -TX_DriveLine_ICE_P.VEH.InitialLongVel / TX_DriveLine_ICE_P.StatLdWhlR[0] * TX_DriveLine_ICE_P.ratio_diff_rear;
  }

  /* Integrator: '<S58>/Integrator' */
  /* Limited  Integrator  */
  if (TX_DriveLine_ICE_DW.Integrator_IWORK_ecw != 0) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] = TX_DriveLine_ICE_B.VectorConcatenate_n[0];
    TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1] = TX_DriveLine_ICE_B.VectorConcatenate_n[1];
  }

  if (TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] >= TX_DriveLine_ICE_P.Integrator_UpperSat_a) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] = TX_DriveLine_ICE_P.Integrator_UpperSat_a;
  } else if (TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] <= TX_DriveLine_ICE_P.Integrator_LowerSat_e) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] = TX_DriveLine_ICE_P.Integrator_LowerSat_e;
  }

  if (TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1] >= TX_DriveLine_ICE_P.Integrator_UpperSat_a) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1] = TX_DriveLine_ICE_P.Integrator_UpperSat_a;
  } else if (TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1] <= TX_DriveLine_ICE_P.Integrator_LowerSat_e) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1] = TX_DriveLine_ICE_P.Integrator_LowerSat_e;
  }

  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Sum: '<S162>/Add2' incorporates:
     *  Constant: '<S162>/Constant2'
     *  Constant: '<S58>/TrqSplitRatioConstantConstant'
     */
    TX_DriveLine_ICE_B.Add2 = TX_DriveLine_ICE_P.Constant2_Value - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio;
  }

  /* Sum: '<S162>/Add1' incorporates:
   *  Constant: '<S58>/TrqSplitRatioConstantConstant'
   *  Gain: '<S162>/Gain'
   *  Integrator: '<S58>/Integrator'
   *  Product: '<S162>/Product'
   *  Product: '<S162>/Product1'
   *  Product: '<S162>/Product2'
   */
  rtb_Add1 = TX_DriveLine_ICE_B.diffDir_k * TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] *
                 TX_DriveLine_ICE_P.TransferCase_Ndiff * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio +
             TX_DriveLine_ICE_B.diffDir_k * TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1] *
                 TX_DriveLine_ICE_P.TransferCase_Ndiff * TX_DriveLine_ICE_B.Add2;

  /* MultiPortSwitch generated from: '<S1>/Multiport Switch' incorporates:
   *  Constant: '<Root>/DriveType[1-FD;2-RD;3-4WD]'
   */
  switch (static_cast<int32_T>(TX_DriveLine_ICE_P.drive_type)) {
    case 1:
      /* Outport: '<Root>/PropShaftSpd[Nm]' */
      TX_DriveLine_ICE_Y.PropShaftSpdNm = rtb_SumofElements;
      break;

    case 2:
      /* Outport: '<Root>/PropShaftSpd[Nm]' */
      TX_DriveLine_ICE_Y.PropShaftSpdNm = rtb_SumofElements_l;
      break;

    default:
      /* Outport: '<Root>/PropShaftSpd[Nm]' */
      TX_DriveLine_ICE_Y.PropShaftSpdNm = rtb_Add1;
      break;
  }

  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Gain: '<S8>/2*pi' incorporates:
     *  Constant: '<S8>/Constant3'
     *  Product: '<S8>/Product1'
     */
    TX_DriveLine_ICE_B.upi =
        1.0 / TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_tauC * TX_DriveLine_ICE_P.upi_Gain;
  }

  /* Sum: '<S14>/Add' incorporates:
   *  Gain: '<S14>/Gain1'
   */
  rtb_Subtract1_d3 =
      TX_DriveLine_ICE_P.Gain1_Gain * rtb_Integrator[0] - TX_DriveLine_ICE_P.Gain1_Gain * rtb_Integrator[1];

  /* Gain: '<S32>/Gain' incorporates:
   *  Abs: '<S31>/Abs'
   *  Constant: '<S32>/Constant1'
   *  Gain: '<S32>/Gain1'
   *  Gain: '<S32>/Gain2'
   *  SignalConversion generated from: '<S14>/Vector Concatenate'
   *  Sum: '<S32>/Subtract1'
   *  Sum: '<S32>/Subtract2'
   *  Trigonometry: '<S32>/Trigonometric Function'
   */
  rtb_Subtract1_e = (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_e * std::abs(rtb_SumofElements) -
                                TX_DriveLine_ICE_P.Constant1_Value_b) *
                               TX_DriveLine_ICE_P.Gain2_Gain) +
                     TX_DriveLine_ICE_P.Constant1_Value_b) *
                    TX_DriveLine_ICE_P.Gain_Gain;

  /* Switch: '<S25>/Switch' incorporates:
   *  Constant: '<S25>/Constant'
   *  Constant: '<S33>/Constant'
   *  Inport: '<Root>/DrvShfTrq[Nm]'
   *  Product: '<S25>/Product1'
   *  SignalConversion generated from: '<S14>/Vector Concatenate'
   */
  if (TX_DriveLine_ICE_U.DrvShfTrqNm * rtb_SumofElements > TX_DriveLine_ICE_P.Switch_Threshold_d) {
    rtb_SumofElements = TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_d;
  }

  /* Product: '<S25>/Product4' incorporates:
   *  Constant: '<S31>/Constant'
   *  Constant: '<S32>/Constant1'
   *  Inport: '<Root>/DrvShfTrq[Nm]'
   *  Product: '<S31>/Product3'
   *  Product: '<S31>/Product5'
   *  Sum: '<S31>/Subtract1'
   *  Sum: '<S32>/Subtract'
   *  Switch: '<S25>/Switch'
   */
  rtb_Product4 = ((TX_DriveLine_ICE_P.Constant1_Value_b - rtb_Subtract1_e) * TX_DriveLine_ICE_P.Constant_Value_p +
                  rtb_SumofElements * rtb_Subtract1_e) *
                 TX_DriveLine_ICE_U.DrvShfTrqNm;

  /* Gain: '<S28>/Gain' incorporates:
   *  Abs: '<S27>/Abs'
   *  Constant: '<S28>/Constant1'
   *  Gain: '<S28>/Gain1'
   *  Gain: '<S28>/Gain2'
   *  Sum: '<S28>/Subtract1'
   *  Sum: '<S28>/Subtract2'
   *  Trigonometry: '<S28>/Trigonometric Function'
   *  UnaryMinus: '<S14>/Unary Minus1'
   */
  rtb_Subtract1_e = (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_h * std::abs(-rtb_Integrator[0]) -
                                TX_DriveLine_ICE_P.Constant1_Value_a) *
                               TX_DriveLine_ICE_P.Gain2_Gain_h) +
                     TX_DriveLine_ICE_P.Constant1_Value_a) *
                    TX_DriveLine_ICE_P.Gain_Gain_i;

  /* Switch: '<S23>/Switch' incorporates:
   *  Constant: '<S23>/Constant'
   *  Constant: '<S33>/Constant'
   *  Product: '<S23>/Product1'
   *  UnaryMinus: '<S14>/Unary Minus1'
   *  UnaryMinus: '<S42>/Unary Minus'
   */
  if (-rtb_Subtract1 * -rtb_Integrator[0] > TX_DriveLine_ICE_P.Switch_Threshold_b) {
    rtb_SumofElements = TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_eq;
  }

  /* Product: '<S23>/Product4' incorporates:
   *  Constant: '<S27>/Constant'
   *  Constant: '<S28>/Constant1'
   *  Product: '<S27>/Product3'
   *  Product: '<S27>/Product5'
   *  Sum: '<S27>/Subtract1'
   *  Sum: '<S28>/Subtract'
   *  Switch: '<S23>/Switch'
   *  UnaryMinus: '<S42>/Unary Minus'
   */
  rtb_Product4_k = ((TX_DriveLine_ICE_P.Constant1_Value_a - rtb_Subtract1_e) * TX_DriveLine_ICE_P.Constant_Value_h +
                    rtb_SumofElements * rtb_Subtract1_e) *
                   -rtb_Subtract1;

  /* Gain: '<S30>/Gain' incorporates:
   *  Abs: '<S29>/Abs'
   *  Constant: '<S30>/Constant1'
   *  Gain: '<S30>/Gain1'
   *  Gain: '<S30>/Gain2'
   *  Sum: '<S30>/Subtract1'
   *  Sum: '<S30>/Subtract2'
   *  Trigonometry: '<S30>/Trigonometric Function'
   *  UnaryMinus: '<S14>/Unary Minus1'
   */
  rtb_Subtract1 = (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_m * std::abs(-rtb_Integrator[1]) -
                              TX_DriveLine_ICE_P.Constant1_Value_p) *
                             TX_DriveLine_ICE_P.Gain2_Gain_f) +
                   TX_DriveLine_ICE_P.Constant1_Value_p) *
                  TX_DriveLine_ICE_P.Gain_Gain_b;

  /* Switch: '<S24>/Switch' incorporates:
   *  Constant: '<S24>/Constant'
   *  Constant: '<S33>/Constant'
   *  Product: '<S24>/Product1'
   *  UnaryMinus: '<S14>/Unary Minus1'
   *  UnaryMinus: '<S34>/Unary Minus'
   */
  if (-rtb_Subtract1_fg * -rtb_Integrator[1] > TX_DriveLine_ICE_P.Switch_Threshold_h) {
    rtb_SumofElements = TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_ps;
  }

  /* Product: '<S24>/Product4' incorporates:
   *  Constant: '<S29>/Constant'
   *  Constant: '<S30>/Constant1'
   *  Product: '<S29>/Product3'
   *  Product: '<S29>/Product5'
   *  Sum: '<S29>/Subtract1'
   *  Sum: '<S30>/Subtract'
   *  Switch: '<S24>/Switch'
   *  UnaryMinus: '<S34>/Unary Minus'
   */
  rtb_Product4_o = ((TX_DriveLine_ICE_P.Constant1_Value_p - rtb_Subtract1) * TX_DriveLine_ICE_P.Constant_Value_j +
                    rtb_SumofElements * rtb_Subtract1) *
                   -rtb_Subtract1_fg;

  /* Integrator: '<S10>/Integrator' */
  rtb_Integrator_fd = TX_DriveLine_ICE_X.Integrator_CSTATE_ca;

  /* MATLAB Function: '<S8>/Limited Slip Differential' incorporates:
   *  Constant: '<S8>/Jd'
   *  Constant: '<S8>/Jw1'
   *  Constant: '<S8>/Jw3'
   *  Constant: '<S8>/Ndiff2'
   *  Constant: '<S8>/bd'
   *  Constant: '<S8>/bw1'
   *  Constant: '<S8>/bw2'
   */
  TX_DriveLine_ICE_LimitedSlipDifferential(
      rtb_Product4, rtb_Product4_k, rtb_Product4_o, rtb_Integrator_fd,
      TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_bw1, TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_bd,
      TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_bw2, TX_DriveLine_ICE_P.ratio_diff_front,
      TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_Jd, TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_Jw1,
      TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_Jw2, rtb_Integrator,
      &TX_DriveLine_ICE_B.sf_LimitedSlipDifferential, &TX_DriveLine_ICE_P.sf_LimitedSlipDifferential);
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Switch: '<S17>/Switch' incorporates:
     *  Constant: '<S17>/Constant'
     */
    if (TX_DriveLine_ICE_P.Constant_Value_b > TX_DriveLine_ICE_P.Switch_Threshold_p) {
      /* Switch: '<S17>/Switch' incorporates:
       *  Constant: '<S17>/Constant2'
       */
      TX_DriveLine_ICE_B.diffDir_d = TX_DriveLine_ICE_P.Constant2_Value_m;
    } else {
      /* Switch: '<S17>/Switch' incorporates:
       *  Constant: '<S17>/Constant2'
       *  UnaryMinus: '<S17>/Unary Minus'
       */
      TX_DriveLine_ICE_B.diffDir_d = -TX_DriveLine_ICE_P.Constant2_Value_m;
    }

    /* End of Switch: '<S17>/Switch' */
  }

  /* Product: '<S10>/Product' incorporates:
   *  Abs: '<S22>/Abs'
   *  Constant: '<S22>/Constant'
   *  Constant: '<S22>/Constant1'
   *  Constant: '<S22>/Constant2'
   *  Gain: '<S22>/Gain'
   *  Lookup_n-D: '<S22>/mu Table'
   *  Product: '<S22>/Product'
   *  Sum: '<S10>/Sum'
   *  Trigonometry: '<S22>/Trigonometric Function'
   */
  TX_DriveLine_ICE_B.Product =
      (TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_Fc *
           TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_Ndisks *
           TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_Reff *
           std::tanh(TX_DriveLine_ICE_P.Gain_Gain_g * rtb_Subtract1_d3) *
           look1_binlxpw(std::abs(rtb_Subtract1_d3), TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_dw,
                         TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_muc, 7U) -
       rtb_Integrator_fd) *
      TX_DriveLine_ICE_B.upi;

  /* Sum: '<S34>/Subtract' incorporates:
   *  Inport: '<Root>/OmegaAxle[rad|s]'
   *  UnaryMinus: '<S14>/Unary Minus1'
   */
  TX_DriveLine_ICE_B.Subtract = -rtb_Integrator[1] - TX_DriveLine_ICE_U.OmegaAxlerads[1];

  /* Switch: '<S35>/Switch' incorporates:
   *  Integrator: '<S35>/Integrator'
   */
  if (TX_DriveLine_ICE_B.Memory_e != 0.0) {
    rtb_SumofElements = TX_DriveLine_ICE_X.Integrator_CSTATE_c;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_B.domega_o_e;
  }

  /* Product: '<S35>/Product' incorporates:
   *  Constant: '<S34>/omega_c'
   *  Sum: '<S35>/Sum'
   *  Switch: '<S35>/Switch'
   */
  TX_DriveLine_ICE_B.Product_c =
      (TX_DriveLine_ICE_B.Subtract - rtb_SumofElements) * TX_DriveLine_ICE_P.TorsionalCompliance1_omega_c;

  /* Sum: '<S42>/Subtract' incorporates:
   *  Inport: '<Root>/OmegaAxle[rad|s]'
   *  UnaryMinus: '<S14>/Unary Minus1'
   */
  TX_DriveLine_ICE_B.Subtract_k = -rtb_Integrator[0] - TX_DriveLine_ICE_U.OmegaAxlerads[0];

  /* Switch: '<S43>/Switch' incorporates:
   *  Integrator: '<S43>/Integrator'
   */
  if (TX_DriveLine_ICE_B.Memory != 0.0) {
    rtb_SumofElements = TX_DriveLine_ICE_X.Integrator_CSTATE;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_B.domega_o;
  }

  /* Product: '<S43>/Product' incorporates:
   *  Constant: '<S42>/omega_c'
   *  Sum: '<S43>/Sum'
   *  Switch: '<S43>/Switch'
   */
  TX_DriveLine_ICE_B.Product_a =
      (TX_DriveLine_ICE_B.Subtract_k - rtb_SumofElements) * TX_DriveLine_ICE_P.TorsionalCompliance2_omega_c;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Gain: '<S59>/2*pi' incorporates:
     *  Constant: '<S59>/Constant3'
     *  Product: '<S59>/Product1'
     */
    TX_DriveLine_ICE_B.upi_f =
        1.0 / TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_tauC * TX_DriveLine_ICE_P.upi_Gain_o;

    /* Memory: '<S112>/Memory' */
    TX_DriveLine_ICE_B.Memory_i2 = TX_DriveLine_ICE_DW.Memory_PreviousInput_h0;

    /* Constant: '<S111>/domega_o' */
    TX_DriveLine_ICE_B.domega_o_b2 = TX_DriveLine_ICE_P.TorsionalCompliance_domega_o_m;
  }

  /* Integrator: '<S112>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo)) {
    zcEvent = rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_i,
                       (TX_DriveLine_ICE_B.Memory_i2));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_ICE_DW.Integrator_IWORK_mo != 0)) {
      TX_DriveLine_ICE_X.Integrator_CSTATE_p = TX_DriveLine_ICE_B.domega_o_b2;
    }
  }

  /* Sum: '<S111>/Subtract1' incorporates:
   *  Gain: '<S111>/Gain1'
   *  Gain: '<S111>/Gain2'
   *  Integrator: '<S111>/Integrator'
   *  Integrator: '<S112>/Integrator'
   */
  rtb_Subtract1_e = TX_DriveLine_ICE_P.TorsionalCompliance_b_o * TX_DriveLine_ICE_X.Integrator_CSTATE_p +
                    TX_DriveLine_ICE_P.TorsionalCompliance_k_h * TX_DriveLine_ICE_X.Integrator_CSTATE_px;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Switch: '<S65>/Switch' incorporates:
     *  Constant: '<S65>/Constant'
     */
    if (TX_DriveLine_ICE_P.Constant_Value_o > TX_DriveLine_ICE_P.Switch_Threshold_a) {
      /* Switch: '<S65>/Switch' incorporates:
       *  Constant: '<S65>/Constant1'
       */
      TX_DriveLine_ICE_B.diffDir_h = TX_DriveLine_ICE_P.Constant1_Value_ms;
    } else {
      /* Switch: '<S65>/Switch' incorporates:
       *  Constant: '<S65>/Constant1'
       *  UnaryMinus: '<S65>/Unary Minus'
       */
      TX_DriveLine_ICE_B.diffDir_h = -TX_DriveLine_ICE_P.Constant1_Value_ms;
    }

    /* End of Switch: '<S65>/Switch' */

    /* Constant: '<S59>/Constant' incorporates:
     *  Concatenate: '<S59>/Vector Concatenate'
     */
    TX_DriveLine_ICE_B.VectorConcatenate_l[0] =
        -TX_DriveLine_ICE_P.VEH.InitialLongVel / TX_DriveLine_ICE_P.StatLdWhlR[0];

    /* Constant: '<S59>/Constant1' incorporates:
     *  Concatenate: '<S59>/Vector Concatenate'
     */
    TX_DriveLine_ICE_B.VectorConcatenate_l[1] =
        -TX_DriveLine_ICE_P.VEH.InitialLongVel / TX_DriveLine_ICE_P.StatLdWhlR[0];
  }

  /* Integrator: '<S59>/Integrator' */
  /* Limited  Integrator  */
  if (TX_DriveLine_ICE_DW.Integrator_IWORK_j != 0) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_i[0] = TX_DriveLine_ICE_B.VectorConcatenate_l[0];
    TX_DriveLine_ICE_X.Integrator_CSTATE_i[1] = TX_DriveLine_ICE_B.VectorConcatenate_l[1];
  }

  if (TX_DriveLine_ICE_X.Integrator_CSTATE_i[0] >= TX_DriveLine_ICE_P.Integrator_UpperSat_j) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_i[0] = TX_DriveLine_ICE_P.Integrator_UpperSat_j;
  } else if (TX_DriveLine_ICE_X.Integrator_CSTATE_i[0] <= TX_DriveLine_ICE_P.Integrator_LowerSat_a) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_i[0] = TX_DriveLine_ICE_P.Integrator_LowerSat_a;
  }

  /* Integrator: '<S59>/Integrator' */
  rtb_Integrator_g[0] = TX_DriveLine_ICE_X.Integrator_CSTATE_i[0];

  /* Integrator: '<S59>/Integrator' */
  if (TX_DriveLine_ICE_X.Integrator_CSTATE_i[1] >= TX_DriveLine_ICE_P.Integrator_UpperSat_j) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_i[1] = TX_DriveLine_ICE_P.Integrator_UpperSat_j;
  } else if (TX_DriveLine_ICE_X.Integrator_CSTATE_i[1] <= TX_DriveLine_ICE_P.Integrator_LowerSat_a) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_i[1] = TX_DriveLine_ICE_P.Integrator_LowerSat_a;
  }

  /* Integrator: '<S59>/Integrator' */
  rtb_Integrator_g[1] = TX_DriveLine_ICE_X.Integrator_CSTATE_i[1];

  /* Sum: '<S65>/Sum of Elements' incorporates:
   *  Gain: '<S65>/Gain'
   *  Product: '<S65>/Product'
   */
  rtb_Subtract1 = TX_DriveLine_ICE_B.diffDir_h * rtb_Integrator_g[0] * rtb_Gain_c +
                  TX_DriveLine_ICE_B.diffDir_h * rtb_Integrator_g[1] * rtb_Gain_c;

  /* Sum: '<S65>/Add' incorporates:
   *  Gain: '<S65>/Gain1'
   */
  rtb_Gain_c =
      TX_DriveLine_ICE_P.Gain1_Gain_c * rtb_Integrator_g[0] - TX_DriveLine_ICE_P.Gain1_Gain_c * rtb_Integrator_g[1];

  /* Gain: '<S83>/Gain' incorporates:
   *  Abs: '<S82>/Abs'
   *  Constant: '<S83>/Constant1'
   *  Gain: '<S83>/Gain1'
   *  Gain: '<S83>/Gain2'
   *  SignalConversion generated from: '<S65>/Vector Concatenate'
   *  Sum: '<S83>/Subtract1'
   *  Sum: '<S83>/Subtract2'
   *  Trigonometry: '<S83>/Trigonometric Function'
   */
  rtb_Subtract1_d3 =
      (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_k * std::abs(rtb_Subtract1) - TX_DriveLine_ICE_P.Constant1_Value_be) *
                 TX_DriveLine_ICE_P.Gain2_Gain_d) +
       TX_DriveLine_ICE_P.Constant1_Value_be) *
      TX_DriveLine_ICE_P.Gain_Gain_c;

  /* Switch: '<S76>/Switch' incorporates:
   *  Constant: '<S76>/Constant'
   *  Constant: '<S84>/Constant'
   *  Product: '<S76>/Product1'
   *  SignalConversion generated from: '<S65>/Vector Concatenate'
   *  UnaryMinus: '<S111>/Unary Minus'
   */
  if (-rtb_Subtract1_e * rtb_Subtract1 > TX_DriveLine_ICE_P.Switch_Threshold_fj) {
    rtb_SumofElements = TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_l;
  }

  /* Product: '<S76>/Product4' incorporates:
   *  Constant: '<S82>/Constant'
   *  Constant: '<S83>/Constant1'
   *  Product: '<S82>/Product3'
   *  Product: '<S82>/Product5'
   *  Sum: '<S82>/Subtract1'
   *  Sum: '<S83>/Subtract'
   *  Switch: '<S76>/Switch'
   *  UnaryMinus: '<S111>/Unary Minus'
   */
  rtb_Product4_n = ((TX_DriveLine_ICE_P.Constant1_Value_be - rtb_Subtract1_d3) * TX_DriveLine_ICE_P.Constant_Value_g +
                    rtb_SumofElements * rtb_Subtract1_d3) *
                   -rtb_Subtract1_e;

  /* Gain: '<S79>/Gain' incorporates:
   *  Abs: '<S78>/Abs'
   *  Constant: '<S79>/Constant1'
   *  Gain: '<S79>/Gain1'
   *  Gain: '<S79>/Gain2'
   *  Sum: '<S79>/Subtract1'
   *  Sum: '<S79>/Subtract2'
   *  Trigonometry: '<S79>/Trigonometric Function'
   *  UnaryMinus: '<S65>/Unary Minus1'
   */
  rtb_Subtract1_fg = (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_g * std::abs(-rtb_Integrator_g[0]) -
                                 TX_DriveLine_ICE_P.Constant1_Value_j) *
                                TX_DriveLine_ICE_P.Gain2_Gain_hy) +
                      TX_DriveLine_ICE_P.Constant1_Value_j) *
                     TX_DriveLine_ICE_P.Gain_Gain_j;

  /* Switch: '<S74>/Switch' incorporates:
   *  Constant: '<S74>/Constant'
   *  Constant: '<S84>/Constant'
   *  Product: '<S74>/Product1'
   *  UnaryMinus: '<S143>/Unary Minus'
   *  UnaryMinus: '<S65>/Unary Minus1'
   */
  if (-rtb_Subtract1_d * -rtb_Integrator_g[0] > TX_DriveLine_ICE_P.Switch_Threshold_fh) {
    rtb_SumofElements = TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_l2;
  }

  /* Product: '<S74>/Product4' incorporates:
   *  Constant: '<S78>/Constant'
   *  Constant: '<S79>/Constant1'
   *  Product: '<S78>/Product3'
   *  Product: '<S78>/Product5'
   *  Sum: '<S78>/Subtract1'
   *  Sum: '<S79>/Subtract'
   *  Switch: '<S74>/Switch'
   *  UnaryMinus: '<S143>/Unary Minus'
   */
  rtb_Product4_f = ((TX_DriveLine_ICE_P.Constant1_Value_j - rtb_Subtract1_fg) * TX_DriveLine_ICE_P.Constant_Value_hc +
                    rtb_SumofElements * rtb_Subtract1_fg) *
                   -rtb_Subtract1_d;

  /* Gain: '<S81>/Gain' incorporates:
   *  Abs: '<S80>/Abs'
   *  Constant: '<S81>/Constant1'
   *  Gain: '<S81>/Gain1'
   *  Gain: '<S81>/Gain2'
   *  Sum: '<S81>/Subtract1'
   *  Sum: '<S81>/Subtract2'
   *  Trigonometry: '<S81>/Trigonometric Function'
   *  UnaryMinus: '<S65>/Unary Minus1'
   */
  rtb_Subtract1_fg = (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_hg * std::abs(-rtb_Integrator_g[1]) -
                                 TX_DriveLine_ICE_P.Constant1_Value_o) *
                                TX_DriveLine_ICE_P.Gain2_Gain_n) +
                      TX_DriveLine_ICE_P.Constant1_Value_o) *
                     TX_DriveLine_ICE_P.Gain_Gain_h;

  /* Switch: '<S75>/Switch' incorporates:
   *  Constant: '<S75>/Constant'
   *  Constant: '<S84>/Constant'
   *  Product: '<S75>/Product1'
   *  UnaryMinus: '<S151>/Unary Minus'
   *  UnaryMinus: '<S65>/Unary Minus1'
   */
  if (-rtb_Subtract1_co * -rtb_Integrator_g[1] > TX_DriveLine_ICE_P.Switch_Threshold_au) {
    rtb_SumofElements = TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_dv;
  }

  /* Product: '<S75>/Product4' incorporates:
   *  Constant: '<S80>/Constant'
   *  Constant: '<S81>/Constant1'
   *  Product: '<S80>/Product3'
   *  Product: '<S80>/Product5'
   *  Sum: '<S80>/Subtract1'
   *  Sum: '<S81>/Subtract'
   *  Switch: '<S75>/Switch'
   *  UnaryMinus: '<S151>/Unary Minus'
   */
  rtb_Product4_g = ((TX_DriveLine_ICE_P.Constant1_Value_o - rtb_Subtract1_fg) * TX_DriveLine_ICE_P.Constant_Value_a +
                    rtb_SumofElements * rtb_Subtract1_fg) *
                   -rtb_Subtract1_co;

  /* Integrator: '<S61>/Integrator' */
  rtb_Integrator_h = TX_DriveLine_ICE_X.Integrator_CSTATE_f;

  /* MATLAB Function: '<S59>/Limited Slip Differential' incorporates:
   *  Constant: '<S59>/Jd'
   *  Constant: '<S59>/Jw1'
   *  Constant: '<S59>/Jw3'
   *  Constant: '<S59>/Ndiff2'
   *  Constant: '<S59>/bd'
   *  Constant: '<S59>/bw1'
   *  Constant: '<S59>/bw2'
   */
  TX_DriveLine_ICE_LimitedSlipDifferential(
      rtb_Product4_n, rtb_Product4_f, rtb_Product4_g, rtb_Integrator_h,
      TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_bw1,
      TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_bd,
      TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_bw2, TX_DriveLine_ICE_P.ratio_diff_front,
      TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_Jd,
      TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_Jw1,
      TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_Jw2, rtb_Integrator_g,
      &TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_i, &TX_DriveLine_ICE_P.sf_LimitedSlipDifferential_i);
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Switch: '<S68>/Switch' incorporates:
     *  Constant: '<S68>/Constant'
     */
    if (TX_DriveLine_ICE_P.Constant_Value_i > TX_DriveLine_ICE_P.Switch_Threshold_e) {
      /* Switch: '<S68>/Switch' incorporates:
       *  Constant: '<S68>/Constant2'
       */
      TX_DriveLine_ICE_B.diffDir_n = TX_DriveLine_ICE_P.Constant2_Value_b;
    } else {
      /* Switch: '<S68>/Switch' incorporates:
       *  Constant: '<S68>/Constant2'
       *  UnaryMinus: '<S68>/Unary Minus'
       */
      TX_DriveLine_ICE_B.diffDir_n = -TX_DriveLine_ICE_P.Constant2_Value_b;
    }

    /* End of Switch: '<S68>/Switch' */

    /* Gain: '<S85>/2*pi' incorporates:
     *  Constant: '<S85>/Constant3'
     *  Product: '<S85>/Product1'
     */
    TX_DriveLine_ICE_B.upi_e =
        1.0 / TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_tauC * TX_DriveLine_ICE_P.upi_Gain_i;

    /* Memory: '<S120>/Memory' */
    TX_DriveLine_ICE_B.Memory_n = TX_DriveLine_ICE_DW.Memory_PreviousInput_hy;

    /* Constant: '<S119>/domega_o' */
    TX_DriveLine_ICE_B.domega_o_d = TX_DriveLine_ICE_P.TorsionalCompliance1_domega_o_p;
  }

  /* Product: '<S61>/Product' incorporates:
   *  Abs: '<S73>/Abs'
   *  Constant: '<S73>/Constant'
   *  Constant: '<S73>/Constant1'
   *  Constant: '<S73>/Constant2'
   *  Gain: '<S73>/Gain'
   *  Lookup_n-D: '<S73>/mu Table'
   *  Product: '<S73>/Product'
   *  Sum: '<S61>/Sum'
   *  Trigonometry: '<S73>/Trigonometric Function'
   */
  TX_DriveLine_ICE_B.Product_h =
      (TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_Fc *
           TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_Ndisks *
           TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_Reff *
           std::tanh(TX_DriveLine_ICE_P.Gain_Gain_n * rtb_Gain_c) *
           look1_binlcpw(std::abs(rtb_Gain_c), TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_dw,
                         TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_muc, 6U) -
       rtb_Integrator_h) *
      TX_DriveLine_ICE_B.upi_f;

  /* Integrator: '<S120>/Integrator' */
  if (rtsiIsModeUpdateTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo)) {
    zcEvent =
        rt_ZCFcn(RISING_ZERO_CROSSING, &TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_j, (TX_DriveLine_ICE_B.Memory_n));

    /* evaluate zero-crossings */
    if ((zcEvent != NO_ZCEVENT) || (TX_DriveLine_ICE_DW.Integrator_IWORK_k != 0)) {
      TX_DriveLine_ICE_X.Integrator_CSTATE_d0 = TX_DriveLine_ICE_B.domega_o_d;
    }
  }

  /* Sum: '<S119>/Subtract1' incorporates:
   *  Gain: '<S119>/Gain1'
   *  Gain: '<S119>/Gain2'
   *  Integrator: '<S119>/Integrator'
   *  Integrator: '<S120>/Integrator'
   */
  rtb_Subtract1_d3 = TX_DriveLine_ICE_P.TorsionalCompliance1_b_j * TX_DriveLine_ICE_X.Integrator_CSTATE_d0 +
                     TX_DriveLine_ICE_P.TorsionalCompliance1_k_p * TX_DriveLine_ICE_X.Integrator_CSTATE_dx;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Switch: '<S91>/Switch' incorporates:
     *  Constant: '<S91>/Constant'
     */
    if (TX_DriveLine_ICE_P.Constant_Value_psd > TX_DriveLine_ICE_P.Switch_Threshold_db) {
      /* Switch: '<S91>/Switch' incorporates:
       *  Constant: '<S91>/Constant1'
       */
      TX_DriveLine_ICE_B.diffDir_dx = TX_DriveLine_ICE_P.Constant1_Value_b1;
    } else {
      /* Switch: '<S91>/Switch' incorporates:
       *  Constant: '<S91>/Constant1'
       *  UnaryMinus: '<S91>/Unary Minus'
       */
      TX_DriveLine_ICE_B.diffDir_dx = -TX_DriveLine_ICE_P.Constant1_Value_b1;
    }

    /* End of Switch: '<S91>/Switch' */

    /* Constant: '<S85>/Constant' incorporates:
     *  Concatenate: '<S85>/Vector Concatenate'
     */
    TX_DriveLine_ICE_B.VectorConcatenate_o[0] =
        TX_DriveLine_ICE_P.VEH.InitialLongVel / TX_DriveLine_ICE_P.StatLdWhlR[0];

    /* Constant: '<S85>/Constant1' incorporates:
     *  Concatenate: '<S85>/Vector Concatenate'
     */
    TX_DriveLine_ICE_B.VectorConcatenate_o[1] =
        TX_DriveLine_ICE_P.VEH.InitialLongVel / TX_DriveLine_ICE_P.StatLdWhlR[0];
  }

  /* Integrator: '<S85>/Integrator' */
  /* Limited  Integrator  */
  if (TX_DriveLine_ICE_DW.Integrator_IWORK_f != 0) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_f3[0] = TX_DriveLine_ICE_B.VectorConcatenate_o[0];
    TX_DriveLine_ICE_X.Integrator_CSTATE_f3[1] = TX_DriveLine_ICE_B.VectorConcatenate_o[1];
  }

  if (TX_DriveLine_ICE_X.Integrator_CSTATE_f3[0] >= TX_DriveLine_ICE_P.Integrator_UpperSat_b) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_f3[0] = TX_DriveLine_ICE_P.Integrator_UpperSat_b;
  } else if (TX_DriveLine_ICE_X.Integrator_CSTATE_f3[0] <= TX_DriveLine_ICE_P.Integrator_LowerSat_l) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_f3[0] = TX_DriveLine_ICE_P.Integrator_LowerSat_l;
  }

  /* Integrator: '<S85>/Integrator' */
  rtb_Integrator_pn[0] = TX_DriveLine_ICE_X.Integrator_CSTATE_f3[0];

  /* Integrator: '<S85>/Integrator' */
  if (TX_DriveLine_ICE_X.Integrator_CSTATE_f3[1] >= TX_DriveLine_ICE_P.Integrator_UpperSat_b) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_f3[1] = TX_DriveLine_ICE_P.Integrator_UpperSat_b;
  } else if (TX_DriveLine_ICE_X.Integrator_CSTATE_f3[1] <= TX_DriveLine_ICE_P.Integrator_LowerSat_l) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_f3[1] = TX_DriveLine_ICE_P.Integrator_LowerSat_l;
  }

  /* Integrator: '<S85>/Integrator' */
  rtb_Integrator_pn[1] = TX_DriveLine_ICE_X.Integrator_CSTATE_f3[1];

  /* Sum: '<S91>/Sum of Elements' incorporates:
   *  Gain: '<S91>/Gain'
   *  Product: '<S91>/Product'
   */
  rtb_Subtract1_fg = TX_DriveLine_ICE_B.diffDir_dx * rtb_Integrator_pn[0] * rtb_Gain_o_tmp +
                     TX_DriveLine_ICE_B.diffDir_dx * rtb_Integrator_pn[1] * rtb_Gain_o_tmp;

  /* Sum: '<S91>/Add' incorporates:
   *  Gain: '<S91>/Gain1'
   */
  rtb_Subtract1_d =
      TX_DriveLine_ICE_P.Gain1_Gain_k2 * rtb_Integrator_pn[0] - TX_DriveLine_ICE_P.Gain1_Gain_k2 * rtb_Integrator_pn[1];

  /* Gain: '<S109>/Gain' incorporates:
   *  Abs: '<S108>/Abs'
   *  Constant: '<S109>/Constant1'
   *  Gain: '<S109>/Gain1'
   *  Gain: '<S109>/Gain2'
   *  SignalConversion generated from: '<S91>/Vector Concatenate'
   *  Sum: '<S109>/Subtract1'
   *  Sum: '<S109>/Subtract2'
   *  Trigonometry: '<S109>/Trigonometric Function'
   */
  rtb_Subtract1_co =
      (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_o * std::abs(rtb_Subtract1_fg) - TX_DriveLine_ICE_P.Constant1_Value_l) *
                 TX_DriveLine_ICE_P.Gain2_Gain_l) +
       TX_DriveLine_ICE_P.Constant1_Value_l) *
      TX_DriveLine_ICE_P.Gain_Gain_ge;

  /* Switch: '<S102>/Switch' incorporates:
   *  Constant: '<S102>/Constant'
   *  Constant: '<S110>/Constant'
   *  Product: '<S102>/Product1'
   *  SignalConversion generated from: '<S91>/Vector Concatenate'
   *  UnaryMinus: '<S119>/Unary Minus'
   */
  if (-rtb_Subtract1_d3 * rtb_Subtract1_fg > TX_DriveLine_ICE_P.Switch_Threshold_dw) {
    rtb_SumofElements = TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_o0;
  }

  /* Product: '<S102>/Product4' incorporates:
   *  Constant: '<S108>/Constant'
   *  Constant: '<S109>/Constant1'
   *  Product: '<S108>/Product3'
   *  Product: '<S108>/Product5'
   *  Sum: '<S108>/Subtract1'
   *  Sum: '<S109>/Subtract'
   *  Switch: '<S102>/Switch'
   *  UnaryMinus: '<S119>/Unary Minus'
   */
  rtb_Product4_i = ((TX_DriveLine_ICE_P.Constant1_Value_l - rtb_Subtract1_co) * TX_DriveLine_ICE_P.Constant_Value_jf +
                    rtb_SumofElements * rtb_Subtract1_co) *
                   -rtb_Subtract1_d3;

  /* Gain: '<S105>/Gain' incorporates:
   *  Abs: '<S104>/Abs'
   *  Constant: '<S105>/Constant1'
   *  Gain: '<S105>/Gain1'
   *  Gain: '<S105>/Gain2'
   *  Sum: '<S105>/Subtract1'
   *  Sum: '<S105>/Subtract2'
   *  Trigonometry: '<S105>/Trigonometric Function'
   *  UnaryMinus: '<S91>/Unary Minus1'
   */
  rtb_Gain_c = (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_l * std::abs(-rtb_Integrator_pn[0]) -
                           TX_DriveLine_ICE_P.Constant1_Value_n) *
                          TX_DriveLine_ICE_P.Gain2_Gain_g) +
                TX_DriveLine_ICE_P.Constant1_Value_n) *
               TX_DriveLine_ICE_P.Gain_Gain_p;

  /* Switch: '<S100>/Switch' incorporates:
   *  Constant: '<S100>/Constant'
   *  Constant: '<S110>/Constant'
   *  Product: '<S100>/Product1'
   *  UnaryMinus: '<S127>/Unary Minus'
   *  UnaryMinus: '<S91>/Unary Minus1'
   */
  if (-rtb_Subtract1_g * -rtb_Integrator_pn[0] > TX_DriveLine_ICE_P.Switch_Threshold_d2) {
    rtb_SumofElements = TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_pz;
  }

  /* Product: '<S100>/Product4' incorporates:
   *  Constant: '<S104>/Constant'
   *  Constant: '<S105>/Constant1'
   *  Product: '<S104>/Product3'
   *  Product: '<S104>/Product5'
   *  Sum: '<S104>/Subtract1'
   *  Sum: '<S105>/Subtract'
   *  Switch: '<S100>/Switch'
   *  UnaryMinus: '<S127>/Unary Minus'
   */
  rtb_Product4_e = ((TX_DriveLine_ICE_P.Constant1_Value_n - rtb_Gain_c) * TX_DriveLine_ICE_P.Constant_Value_f +
                    rtb_SumofElements * rtb_Gain_c) *
                   -rtb_Subtract1_g;

  /* Gain: '<S107>/Gain' incorporates:
   *  Abs: '<S106>/Abs'
   *  Constant: '<S107>/Constant1'
   *  Gain: '<S107>/Gain1'
   *  Gain: '<S107>/Gain2'
   *  Sum: '<S107>/Subtract1'
   *  Sum: '<S107>/Subtract2'
   *  Trigonometry: '<S107>/Trigonometric Function'
   *  UnaryMinus: '<S91>/Unary Minus1'
   */
  rtb_Gain_c = (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_hj * std::abs(-rtb_Integrator_pn[1]) -
                           TX_DriveLine_ICE_P.Constant1_Value_o3) *
                          TX_DriveLine_ICE_P.Gain2_Gain_m) +
                TX_DriveLine_ICE_P.Constant1_Value_o3) *
               TX_DriveLine_ICE_P.Gain_Gain_e;

  /* Switch: '<S101>/Switch' incorporates:
   *  Constant: '<S101>/Constant'
   *  Constant: '<S110>/Constant'
   *  Product: '<S101>/Product1'
   *  UnaryMinus: '<S135>/Unary Minus'
   *  UnaryMinus: '<S91>/Unary Minus1'
   */
  if (-rtb_Subtract1_n * -rtb_Integrator_pn[1] > TX_DriveLine_ICE_P.Switch_Threshold_ez) {
    rtb_SumofElements = TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_bt;
  }

  /* Product: '<S101>/Product4' incorporates:
   *  Constant: '<S106>/Constant'
   *  Constant: '<S107>/Constant1'
   *  Product: '<S106>/Product3'
   *  Product: '<S106>/Product5'
   *  Sum: '<S106>/Subtract1'
   *  Sum: '<S107>/Subtract'
   *  Switch: '<S101>/Switch'
   *  UnaryMinus: '<S135>/Unary Minus'
   */
  rtb_Product4_gf = ((TX_DriveLine_ICE_P.Constant1_Value_o3 - rtb_Gain_c) * TX_DriveLine_ICE_P.Constant_Value_pp +
                     rtb_SumofElements * rtb_Gain_c) *
                    -rtb_Subtract1_n;

  /* Integrator: '<S87>/Integrator' */
  rtb_Integrator_c = TX_DriveLine_ICE_X.Integrator_CSTATE_n3;

  /* MATLAB Function: '<S85>/Limited Slip Differential' incorporates:
   *  Constant: '<S85>/Jd'
   *  Constant: '<S85>/Jw1'
   *  Constant: '<S85>/Jw3'
   *  Constant: '<S85>/Ndiff2'
   *  Constant: '<S85>/bd'
   *  Constant: '<S85>/bw1'
   *  Constant: '<S85>/bw2'
   */
  TX_DriveLine_ICE_LimitedSlipDifferential(
      rtb_Product4_i, rtb_Product4_e, rtb_Product4_gf, rtb_Integrator_c,
      TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_bw1,
      TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_bd,
      TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_bw2, TX_DriveLine_ICE_P.ratio_diff_rear,
      TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_Jd,
      TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_Jw1,
      TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_Jw2, rtb_Integrator_pn,
      &TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_d, &TX_DriveLine_ICE_P.sf_LimitedSlipDifferential_d);
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Switch: '<S94>/Switch' incorporates:
     *  Constant: '<S94>/Constant'
     */
    if (TX_DriveLine_ICE_P.Constant_Value_op > TX_DriveLine_ICE_P.Switch_Threshold_f5) {
      /* Switch: '<S94>/Switch' incorporates:
       *  Constant: '<S94>/Constant2'
       */
      TX_DriveLine_ICE_B.diffDir_o = TX_DriveLine_ICE_P.Constant2_Value_h;
    } else {
      /* Switch: '<S94>/Switch' incorporates:
       *  Constant: '<S94>/Constant2'
       *  UnaryMinus: '<S94>/Unary Minus'
       */
      TX_DriveLine_ICE_B.diffDir_o = -TX_DriveLine_ICE_P.Constant2_Value_h;
    }

    /* End of Switch: '<S94>/Switch' */
  }

  /* Product: '<S87>/Product' incorporates:
   *  Abs: '<S99>/Abs'
   *  Constant: '<S99>/Constant'
   *  Constant: '<S99>/Constant1'
   *  Constant: '<S99>/Constant2'
   *  Gain: '<S99>/Gain'
   *  Lookup_n-D: '<S99>/mu Table'
   *  Product: '<S99>/Product'
   *  Sum: '<S87>/Sum'
   *  Trigonometry: '<S99>/Trigonometric Function'
   */
  TX_DriveLine_ICE_B.Product_d =
      (TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_Fc *
           TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_Ndisks *
           TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_Reff *
           std::tanh(TX_DriveLine_ICE_P.Gain_Gain_k * rtb_Subtract1_d) *
           look1_binlcpw(std::abs(rtb_Subtract1_d), TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_dw,
                         TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_muc, 6U) -
       rtb_Integrator_c) *
      TX_DriveLine_ICE_B.upi_e;

  /* Sum: '<S111>/Subtract' incorporates:
   *  Integrator: '<S58>/Integrator'
   *  UnaryMinus: '<S162>/Unary Minus1'
   */
  TX_DriveLine_ICE_B.Subtract_i = rtb_Subtract1 - (-TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0]);

  /* Switch: '<S112>/Switch' incorporates:
   *  Integrator: '<S112>/Integrator'
   */
  if (TX_DriveLine_ICE_B.Memory_i2 != 0.0) {
    rtb_SumofElements = TX_DriveLine_ICE_X.Integrator_CSTATE_p;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_B.domega_o_b2;
  }

  /* Product: '<S112>/Product' incorporates:
   *  Constant: '<S111>/omega_c'
   *  Sum: '<S112>/Sum'
   *  Switch: '<S112>/Switch'
   */
  TX_DriveLine_ICE_B.Product_e =
      (TX_DriveLine_ICE_B.Subtract_i - rtb_SumofElements) * TX_DriveLine_ICE_P.TorsionalCompliance_omega_c;

  /* Sum: '<S119>/Subtract' incorporates:
   *  Integrator: '<S58>/Integrator'
   *  UnaryMinus: '<S162>/Unary Minus1'
   */
  TX_DriveLine_ICE_B.Subtract_kz = rtb_Subtract1_fg - (-TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1]);

  /* Switch: '<S120>/Switch' incorporates:
   *  Integrator: '<S120>/Integrator'
   */
  if (TX_DriveLine_ICE_B.Memory_n != 0.0) {
    rtb_SumofElements = TX_DriveLine_ICE_X.Integrator_CSTATE_d0;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_B.domega_o_d;
  }

  /* Product: '<S120>/Product' incorporates:
   *  Constant: '<S119>/omega_c'
   *  Sum: '<S120>/Sum'
   *  Switch: '<S120>/Switch'
   */
  TX_DriveLine_ICE_B.Product_k =
      (TX_DriveLine_ICE_B.Subtract_kz - rtb_SumofElements) * TX_DriveLine_ICE_P.TorsionalCompliance1_omega_c_g;

  /* Sum: '<S127>/Subtract' incorporates:
   *  Inport: '<Root>/OmegaAxle[rad|s]'
   *  UnaryMinus: '<S91>/Unary Minus1'
   */
  TX_DriveLine_ICE_B.Subtract_b = -rtb_Integrator_pn[0] - TX_DriveLine_ICE_U.OmegaAxlerads[2];

  /* Switch: '<S128>/Switch' incorporates:
   *  Integrator: '<S128>/Integrator'
   */
  if (TX_DriveLine_ICE_B.Memory_b != 0.0) {
    rtb_SumofElements = TX_DriveLine_ICE_X.Integrator_CSTATE_e;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_B.domega_o_p;
  }

  /* Product: '<S128>/Product' incorporates:
   *  Constant: '<S127>/omega_c'
   *  Sum: '<S128>/Sum'
   *  Switch: '<S128>/Switch'
   */
  TX_DriveLine_ICE_B.Product_he =
      (TX_DriveLine_ICE_B.Subtract_b - rtb_SumofElements) * TX_DriveLine_ICE_P.TorsionalCompliance2_omega_c_i;

  /* Sum: '<S135>/Subtract' incorporates:
   *  Inport: '<Root>/OmegaAxle[rad|s]'
   *  UnaryMinus: '<S91>/Unary Minus1'
   */
  TX_DriveLine_ICE_B.Subtract_h = -rtb_Integrator_pn[1] - TX_DriveLine_ICE_U.OmegaAxlerads[3];

  /* Switch: '<S136>/Switch' incorporates:
   *  Integrator: '<S136>/Integrator'
   */
  if (TX_DriveLine_ICE_B.Memory_g != 0.0) {
    rtb_SumofElements = TX_DriveLine_ICE_X.Integrator_CSTATE_jc;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_B.domega_o_g;
  }

  /* Product: '<S136>/Product' incorporates:
   *  Constant: '<S135>/omega_c'
   *  Sum: '<S136>/Sum'
   *  Switch: '<S136>/Switch'
   */
  TX_DriveLine_ICE_B.Product_b =
      (TX_DriveLine_ICE_B.Subtract_h - rtb_SumofElements) * TX_DriveLine_ICE_P.TorsionalCompliance3_omega_c;

  /* Sum: '<S143>/Subtract' incorporates:
   *  Inport: '<Root>/OmegaAxle[rad|s]'
   *  UnaryMinus: '<S65>/Unary Minus1'
   */
  TX_DriveLine_ICE_B.Subtract_g = -rtb_Integrator_g[0] - TX_DriveLine_ICE_U.OmegaAxlerads[0];

  /* Switch: '<S144>/Switch' incorporates:
   *  Integrator: '<S144>/Integrator'
   */
  if (TX_DriveLine_ICE_B.Memory_i != 0.0) {
    rtb_SumofElements = TX_DriveLine_ICE_X.Integrator_CSTATE_c4;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_B.domega_o_br;
  }

  /* Product: '<S144>/Product' incorporates:
   *  Constant: '<S143>/omega_c'
   *  Sum: '<S144>/Sum'
   *  Switch: '<S144>/Switch'
   */
  TX_DriveLine_ICE_B.Product_j =
      (TX_DriveLine_ICE_B.Subtract_g - rtb_SumofElements) * TX_DriveLine_ICE_P.TorsionalCompliance4_omega_c;

  /* Sum: '<S151>/Subtract' incorporates:
   *  Inport: '<Root>/OmegaAxle[rad|s]'
   *  UnaryMinus: '<S65>/Unary Minus1'
   */
  TX_DriveLine_ICE_B.Subtract_p = -rtb_Integrator_g[1] - TX_DriveLine_ICE_U.OmegaAxlerads[1];

  /* Switch: '<S152>/Switch' incorporates:
   *  Integrator: '<S152>/Integrator'
   */
  if (TX_DriveLine_ICE_B.Memory_el != 0.0) {
    rtb_SumofElements = TX_DriveLine_ICE_X.Integrator_CSTATE_nl;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_B.domega_o_a;
  }

  /* Product: '<S152>/Product' incorporates:
   *  Constant: '<S151>/omega_c'
   *  Sum: '<S152>/Sum'
   *  Switch: '<S152>/Switch'
   */
  TX_DriveLine_ICE_B.Product_a3 =
      (TX_DriveLine_ICE_B.Subtract_p - rtb_SumofElements) * TX_DriveLine_ICE_P.TorsionalCompliance5_omega_c;

  /* Gain: '<S179>/Gain' incorporates:
   *  Abs: '<S178>/Abs'
   *  Constant: '<S179>/Constant1'
   *  Gain: '<S179>/Gain1'
   *  Gain: '<S179>/Gain2'
   *  SignalConversion generated from: '<S162>/Vector Concatenate'
   *  Sum: '<S179>/Subtract1'
   *  Sum: '<S179>/Subtract2'
   *  Trigonometry: '<S179>/Trigonometric Function'
   */
  rtb_Subtract1_g =
      (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_gv * std::abs(rtb_Add1) - TX_DriveLine_ICE_P.Constant1_Value_d) *
                 TX_DriveLine_ICE_P.Gain2_Gain_h0) +
       TX_DriveLine_ICE_P.Constant1_Value_d) *
      TX_DriveLine_ICE_P.Gain_Gain_bj;

  /* Switch: '<S172>/Switch' incorporates:
   *  Constant: '<S172>/Constant'
   *  Constant: '<S180>/Constant'
   *  Inport: '<Root>/DrvShfTrq[Nm]'
   *  Product: '<S172>/Product1'
   *  SignalConversion generated from: '<S162>/Vector Concatenate'
   */
  if (TX_DriveLine_ICE_U.DrvShfTrqNm * rtb_Add1 > TX_DriveLine_ICE_P.Switch_Threshold_j) {
    rtb_SumofElements = TX_DriveLine_ICE_P.TransferCase_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_dr;
  }

  /* Sum: '<S178>/Subtract1' incorporates:
   *  Constant: '<S178>/Constant'
   *  Constant: '<S179>/Constant1'
   *  Product: '<S178>/Product3'
   *  Product: '<S178>/Product5'
   *  Sum: '<S179>/Subtract'
   *  Switch: '<S172>/Switch'
   */
  rtb_Add1 = (TX_DriveLine_ICE_P.Constant1_Value_d - rtb_Subtract1_g) * TX_DriveLine_ICE_P.Constant_Value_gv +
             rtb_SumofElements * rtb_Subtract1_g;

  /* Gain: '<S175>/Gain' incorporates:
   *  Abs: '<S174>/Abs'
   *  Constant: '<S175>/Constant1'
   *  Gain: '<S175>/Gain1'
   *  Gain: '<S175>/Gain2'
   *  Integrator: '<S58>/Integrator'
   *  Sum: '<S175>/Subtract1'
   *  Sum: '<S175>/Subtract2'
   *  Trigonometry: '<S175>/Trigonometric Function'
   *  UnaryMinus: '<S162>/Unary Minus1'
   */
  rtb_Subtract1_g =
      (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_b * std::abs(-TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0]) -
                  TX_DriveLine_ICE_P.Constant1_Value_f) *
                 TX_DriveLine_ICE_P.Gain2_Gain_c) +
       TX_DriveLine_ICE_P.Constant1_Value_f) *
      TX_DriveLine_ICE_P.Gain_Gain_nw;

  /* Switch: '<S170>/Switch' incorporates:
   *  Constant: '<S170>/Constant'
   *  Constant: '<S180>/Constant'
   *  Integrator: '<S58>/Integrator'
   *  Product: '<S170>/Product1'
   *  UnaryMinus: '<S162>/Unary Minus1'
   */
  if (rtb_Subtract1_e * -TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] > TX_DriveLine_ICE_P.Switch_Threshold_bl) {
    rtb_SumofElements = TX_DriveLine_ICE_P.TransferCase_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_m;
  }

  /* Sum: '<S174>/Subtract1' incorporates:
   *  Constant: '<S174>/Constant'
   *  Constant: '<S175>/Constant1'
   *  Product: '<S174>/Product3'
   *  Product: '<S174>/Product5'
   *  Sum: '<S175>/Subtract'
   *  Switch: '<S170>/Switch'
   */
  rtb_Subtract1_n = (TX_DriveLine_ICE_P.Constant1_Value_f - rtb_Subtract1_g) * TX_DriveLine_ICE_P.Constant_Value_fb +
                    rtb_SumofElements * rtb_Subtract1_g;

  /* Gain: '<S177>/Gain' incorporates:
   *  Abs: '<S176>/Abs'
   *  Constant: '<S177>/Constant1'
   *  Gain: '<S177>/Gain1'
   *  Gain: '<S177>/Gain2'
   *  Integrator: '<S58>/Integrator'
   *  Sum: '<S177>/Subtract1'
   *  Sum: '<S177>/Subtract2'
   *  Trigonometry: '<S177>/Trigonometric Function'
   *  UnaryMinus: '<S162>/Unary Minus1'
   */
  rtb_Subtract1_g =
      (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_kz * std::abs(-TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1]) -
                  TX_DriveLine_ICE_P.Constant1_Value_bz) *
                 TX_DriveLine_ICE_P.Gain2_Gain_fr) +
       TX_DriveLine_ICE_P.Constant1_Value_bz) *
      TX_DriveLine_ICE_P.Gain_Gain_jk;

  /* SignalConversion generated from: '<S161>/ SFunction ' incorporates:
   *  Inport: '<Root>/DrvShfTrq[Nm]'
   *  MATLAB Function: '<S58>/TransferCase'
   *  Product: '<S170>/Product4'
   *  Product: '<S172>/Product4'
   */
  rtb_Subtract1 = rtb_Add1 * TX_DriveLine_ICE_U.DrvShfTrqNm;
  rtb_Subtract1_n *= rtb_Subtract1_e;

  /* Switch: '<S171>/Switch' incorporates:
   *  Constant: '<S171>/Constant'
   *  Constant: '<S180>/Constant'
   *  Integrator: '<S58>/Integrator'
   *  Product: '<S171>/Product1'
   *  UnaryMinus: '<S162>/Unary Minus1'
   */
  if (rtb_Subtract1_d3 * -TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1] > TX_DriveLine_ICE_P.Switch_Threshold_pr) {
    rtb_SumofElements = TX_DriveLine_ICE_P.TransferCase_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_jc;
  }

  /* SignalConversion generated from: '<S161>/ SFunction ' incorporates:
   *  Constant: '<S176>/Constant'
   *  Constant: '<S177>/Constant1'
   *  MATLAB Function: '<S58>/TransferCase'
   *  Product: '<S171>/Product4'
   *  Product: '<S176>/Product3'
   *  Product: '<S176>/Product5'
   *  Sum: '<S176>/Subtract1'
   *  Sum: '<S177>/Subtract'
   *  Switch: '<S171>/Switch'
   */
  rtb_Subtract1_g = ((TX_DriveLine_ICE_P.Constant1_Value_bz - rtb_Subtract1_g) * TX_DriveLine_ICE_P.Constant_Value_pa +
                     rtb_SumofElements * rtb_Subtract1_g) *
                    rtb_Subtract1_d3;

  /* MATLAB Function: '<S58>/TransferCase' incorporates:
   *  Constant: '<S58>/Jd'
   *  Constant: '<S58>/Jw1'
   *  Constant: '<S58>/Jw3'
   *  Constant: '<S58>/Ndiff2'
   *  Constant: '<S58>/SpdLockConstantConstant'
   *  Constant: '<S58>/TrqSplitRatioConstantConstant'
   *  Constant: '<S58>/bd'
   *  Constant: '<S58>/bw1'
   *  Constant: '<S58>/bw2'
   *  Integrator: '<S58>/Integrator'
   */
  if (TX_DriveLine_ICE_P.TransferCase_SpdLock == 1.0) {
    if (TX_DriveLine_ICE_P.TransferCase_shaftSwitchMask == 1.0) {
      rtb_Subtract1_fg = TX_DriveLine_ICE_P.TransferCase_Ndiff * TX_DriveLine_ICE_P.TransferCase_Ndiff;
      rtb_Subtract1_d3 = rtb_Subtract1_fg * TX_DriveLine_ICE_P.TransferCase_Jd;
      rtb_Subtract1_d3 = 1.0 / ((rtb_Subtract1_d3 * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio +
                                 (TX_DriveLine_ICE_P.TransferCase_Jw1 + TX_DriveLine_ICE_P.TransferCase_Jw2)) +
                                (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio) * rtb_Subtract1_d3);
      rtb_Subtract1_d3 =
          -((rtb_Subtract1_fg * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio * TX_DriveLine_ICE_P.TransferCase_bd +
             (TX_DriveLine_ICE_P.TransferCase_bw1 + TX_DriveLine_ICE_P.TransferCase_bw2)) +
            (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio) * rtb_Subtract1_fg *
                TX_DriveLine_ICE_P.TransferCase_bd) *
              rtb_Subtract1_d3 * TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] +
          ((rtb_Subtract1_d3 * -TX_DriveLine_ICE_P.TransferCase_Ndiff * rtb_Subtract1 +
            -rtb_Subtract1_d3 * rtb_Subtract1_n) +
           -rtb_Subtract1_d3 * rtb_Subtract1_g);
      TX_DriveLine_ICE_B.xdot[0] = rtb_Subtract1_d3;
      TX_DriveLine_ICE_B.xdot[1] = rtb_Subtract1_d3;
    } else {
      rtb_Subtract1_e = TX_DriveLine_ICE_P.TransferCase_Ndiff * TX_DriveLine_ICE_P.TransferCase_Ndiff;
      rtb_Subtract1_d3 = rtb_Subtract1_e * TX_DriveLine_ICE_P.TransferCase_Jd;
      rtb_Subtract1_d3 = 1.0 / ((rtb_Subtract1_d3 * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio +
                                 (TX_DriveLine_ICE_P.TransferCase_Jw1 + TX_DriveLine_ICE_P.TransferCase_Jw2)) +
                                (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio) * rtb_Subtract1_d3);
      rtb_Subtract1_d3 =
          -((rtb_Subtract1_e * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio * TX_DriveLine_ICE_P.TransferCase_bd +
             (TX_DriveLine_ICE_P.TransferCase_bw1 + TX_DriveLine_ICE_P.TransferCase_bw2)) +
            (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio) * rtb_Subtract1_e *
                TX_DriveLine_ICE_P.TransferCase_bd) *
              rtb_Subtract1_d3 * TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] +
          ((rtb_Subtract1_d3 * TX_DriveLine_ICE_P.TransferCase_Ndiff * rtb_Subtract1 +
            -rtb_Subtract1_d3 * rtb_Subtract1_n) +
           -rtb_Subtract1_d3 * rtb_Subtract1_g);
      TX_DriveLine_ICE_B.xdot[0] = rtb_Subtract1_d3;
      TX_DriveLine_ICE_B.xdot[1] = rtb_Subtract1_d3;
    }
  } else if (TX_DriveLine_ICE_P.TransferCase_shaftSwitchMask == 1.0) {
    rtb_Subtract1_co = TX_DriveLine_ICE_P.TransferCase_Ndiff * TX_DriveLine_ICE_P.TransferCase_Ndiff;
    rtb_Gain_c =
        (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio) * (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio);
    rtb_Subtract1_fg = TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio;
    rtb_Subtract1_d =
        (TX_DriveLine_ICE_P.TransferCase_Jw2 * TX_DriveLine_ICE_P.TransferCase_Jd * rtb_Subtract1_co *
             rtb_Subtract1_fg +
         TX_DriveLine_ICE_P.TransferCase_Jw1 * TX_DriveLine_ICE_P.TransferCase_Jd * rtb_Subtract1_co * rtb_Gain_c) +
        TX_DriveLine_ICE_P.TransferCase_Jw1 * TX_DriveLine_ICE_P.TransferCase_Jw2;
    rtb_SumofElements = TX_DriveLine_ICE_P.TransferCase_Ndiff * TX_DriveLine_ICE_P.TransferCase_Ndiff *
                        TX_DriveLine_ICE_P.TransferCase_Jd;
    invJ[0] = (rtb_SumofElements * rtb_Gain_c + TX_DriveLine_ICE_P.TransferCase_Jw2) / rtb_Subtract1_d;
    rtb_Subtract1_d3 = -(TX_DriveLine_ICE_P.TransferCase_Ndiff * TX_DriveLine_ICE_P.TransferCase_Ndiff *
                         TX_DriveLine_ICE_P.TransferCase_Jd * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio *
                         (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio)) /
                       rtb_Subtract1_d;
    invJ[2] = rtb_Subtract1_d3;
    invJ[1] = rtb_Subtract1_d3;
    invJ[3] = (rtb_SumofElements * rtb_Subtract1_fg + TX_DriveLine_ICE_P.TransferCase_Jw1) / rtb_Subtract1_d;
    tmp[0] = -(TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio * TX_DriveLine_ICE_P.TransferCase_Ndiff);
    tmp[2] = -1.0;
    tmp[4] = 0.0;
    tmp[1] = -((1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio) * TX_DriveLine_ICE_P.TransferCase_Ndiff);
    tmp[3] = 0.0;
    tmp[5] = -1.0;
    rtb_SumofElements = -(rtb_Subtract1_fg * rtb_Subtract1_co * TX_DriveLine_ICE_P.TransferCase_bd +
                          TX_DriveLine_ICE_P.TransferCase_bw1);
    rtb_Subtract1_fg =
        -((1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio) * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio *
          rtb_Subtract1_co * TX_DriveLine_ICE_P.TransferCase_bd);
    rtb_Subtract1_d =
        -(rtb_Subtract1_co * TX_DriveLine_ICE_P.TransferCase_bd * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio *
          (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio));
    rtb_Subtract1_e =
        -(rtb_Gain_c * rtb_Subtract1_co * TX_DriveLine_ICE_P.TransferCase_bd + TX_DriveLine_ICE_P.TransferCase_bw2);
    for (int32_T i{0}; i < 2; i++) {
      for (int32_T i_0{0}; i_0 < 3; i_0++) {
        int32_T invJ_tmp;
        int32_T invJ_tmp_tmp;
        invJ_tmp_tmp = i_0 << 1;
        invJ_tmp = i + invJ_tmp_tmp;
        invJ_0[invJ_tmp] = 0.0;
        invJ_0[invJ_tmp] += tmp[invJ_tmp_tmp] * invJ[i];
        invJ_0[invJ_tmp] += tmp[invJ_tmp_tmp + 1] * invJ[i + 2];
      }

      invJ_1[i] = 0.0;
      invJ_1[i] += invJ[i] * rtb_SumofElements;
      rtb_Subtract1_d3 = invJ[i + 2];
      invJ_1[i] += rtb_Subtract1_d3 * rtb_Subtract1_d;
      invJ_1[i + 2] = 0.0;
      invJ_1[i + 2] += invJ[i] * rtb_Subtract1_fg;
      invJ_1[i + 2] += rtb_Subtract1_d3 * rtb_Subtract1_e;
      invJ_2[i] = 0.0;
      invJ_2[i] += invJ_0[i] * rtb_Subtract1;
      invJ_2[i] += invJ_0[i + 2] * rtb_Subtract1_n;
      invJ_2[i] += invJ_0[i + 4] * rtb_Subtract1_g;
      invJ_3[i] = 0.0;
      invJ_3[i] += invJ_1[i] * TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0];
      invJ_3[i] += invJ_1[i + 2] * TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1];
      TX_DriveLine_ICE_B.xdot[i] = invJ_2[i] + invJ_3[i];
    }
  } else {
    rtb_Subtract1_d3 = TX_DriveLine_ICE_P.TransferCase_Ndiff * TX_DriveLine_ICE_P.TransferCase_Ndiff;
    rtb_Subtract1_e =
        (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio) * (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio);
    rtb_SumofElements = TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio;
    rtb_Subtract1_fg = (TX_DriveLine_ICE_P.TransferCase_Jw2 * TX_DriveLine_ICE_P.TransferCase_Jd * rtb_Subtract1_d3 *
                            rtb_SumofElements +
                        TX_DriveLine_ICE_P.TransferCase_Jw1 * TX_DriveLine_ICE_P.TransferCase_Jd * rtb_Subtract1_d3 *
                            rtb_Subtract1_e) +
                       TX_DriveLine_ICE_P.TransferCase_Jw1 * TX_DriveLine_ICE_P.TransferCase_Jw2;
    rtb_Subtract1_d = TX_DriveLine_ICE_P.TransferCase_Ndiff * TX_DriveLine_ICE_P.TransferCase_Ndiff *
                      TX_DriveLine_ICE_P.TransferCase_Jd;
    invJ[0] = (rtb_Subtract1_d * rtb_Subtract1_e + TX_DriveLine_ICE_P.TransferCase_Jw2) / rtb_Subtract1_fg;
    rtb_Subtract1_co = -(TX_DriveLine_ICE_P.TransferCase_Ndiff * TX_DriveLine_ICE_P.TransferCase_Ndiff *
                         TX_DriveLine_ICE_P.TransferCase_Jd * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio *
                         (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio)) /
                       rtb_Subtract1_fg;
    invJ[2] = rtb_Subtract1_co;
    invJ[1] = rtb_Subtract1_co;
    invJ[3] = (rtb_Subtract1_d * rtb_SumofElements + TX_DriveLine_ICE_P.TransferCase_Jw1) / rtb_Subtract1_fg;
    tmp[0] = TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio * TX_DriveLine_ICE_P.TransferCase_Ndiff;
    tmp[2] = -1.0;
    tmp[4] = 0.0;
    tmp[1] = (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio) * TX_DriveLine_ICE_P.TransferCase_Ndiff;
    tmp[3] = 0.0;
    tmp[5] = -1.0;
    rtb_SumofElements = -(rtb_SumofElements * rtb_Subtract1_d3 * TX_DriveLine_ICE_P.TransferCase_bd +
                          TX_DriveLine_ICE_P.TransferCase_bw1);
    rtb_Subtract1_fg =
        -((1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio) * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio *
          rtb_Subtract1_d3 * TX_DriveLine_ICE_P.TransferCase_bd);
    rtb_Subtract1_d =
        -(rtb_Subtract1_d3 * TX_DriveLine_ICE_P.TransferCase_bd * TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio *
          (1.0 - TX_DriveLine_ICE_P.TransferCase_TrqSplitRatio));
    rtb_Subtract1_e = -(rtb_Subtract1_e * rtb_Subtract1_d3 * TX_DriveLine_ICE_P.TransferCase_bd +
                        TX_DriveLine_ICE_P.TransferCase_bw2);
    for (int32_T i{0}; i < 2; i++) {
      for (int32_T i_0{0}; i_0 < 3; i_0++) {
        int32_T invJ_tmp;
        int32_T invJ_tmp_tmp;
        invJ_tmp_tmp = i_0 << 1;
        invJ_tmp = i + invJ_tmp_tmp;
        invJ_0[invJ_tmp] = 0.0;
        invJ_0[invJ_tmp] += tmp[invJ_tmp_tmp] * invJ[i];
        invJ_0[invJ_tmp] += tmp[invJ_tmp_tmp + 1] * invJ[i + 2];
      }

      invJ_1[i] = 0.0;
      invJ_1[i] += invJ[i] * rtb_SumofElements;
      rtb_Subtract1_d3 = invJ[i + 2];
      invJ_1[i] += rtb_Subtract1_d3 * rtb_Subtract1_d;
      invJ_1[i + 2] = 0.0;
      invJ_1[i + 2] += invJ[i] * rtb_Subtract1_fg;
      invJ_1[i + 2] += rtb_Subtract1_d3 * rtb_Subtract1_e;
      invJ_2[i] = 0.0;
      invJ_2[i] += invJ_0[i] * rtb_Subtract1;
      invJ_2[i] += invJ_0[i + 2] * rtb_Subtract1_n;
      invJ_2[i] += invJ_0[i + 4] * rtb_Subtract1_g;
      invJ_3[i] = 0.0;
      invJ_3[i] += invJ_1[i] * TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0];
      invJ_3[i] += invJ_1[i + 2] * TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1];
      TX_DriveLine_ICE_B.xdot[i] = invJ_2[i] + invJ_3[i];
    }
  }

  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Switch: '<S165>/Switch1' incorporates:
     *  Constant: '<S165>/Constant'
     */
    if (TX_DriveLine_ICE_P.TransferCase_shaftSwitchMask > TX_DriveLine_ICE_P.Switch1_Threshold) {
      /* Switch: '<S165>/Switch1' incorporates:
       *  Constant: '<S165>/Constant6'
       */
      TX_DriveLine_ICE_B.diffDir_ks = TX_DriveLine_ICE_P.Constant6_Value;
    } else {
      /* Switch: '<S165>/Switch1' incorporates:
       *  Constant: '<S165>/Constant6'
       *  UnaryMinus: '<S165>/Unary Minus'
       */
      TX_DriveLine_ICE_B.diffDir_ks = -TX_DriveLine_ICE_P.Constant6_Value;
    }

    /* End of Switch: '<S165>/Switch1' */

    /* Gain: '<S184>/2*pi' incorporates:
     *  Constant: '<S184>/Constant3'
     *  Product: '<S184>/Product1'
     */
    TX_DriveLine_ICE_B.upi_i =
        1.0 / TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_tauC * TX_DriveLine_ICE_P.upi_Gain_h;
  }

  /* Sum: '<S190>/Add' incorporates:
   *  Gain: '<S190>/Gain1'
   */
  rtb_Subtract1_co =
      TX_DriveLine_ICE_P.Gain1_Gain_cp * rtb_Integrator_p[0] - TX_DriveLine_ICE_P.Gain1_Gain_cp * rtb_Integrator_p[1];

  /* Gain: '<S208>/Gain' incorporates:
   *  Abs: '<S207>/Abs'
   *  Constant: '<S208>/Constant1'
   *  Gain: '<S208>/Gain1'
   *  Gain: '<S208>/Gain2'
   *  SignalConversion generated from: '<S190>/Vector Concatenate'
   *  Sum: '<S208>/Subtract1'
   *  Sum: '<S208>/Subtract2'
   *  Trigonometry: '<S208>/Trigonometric Function'
   */
  rtb_Subtract1_d = (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_mc * std::abs(rtb_SumofElements_l) -
                                TX_DriveLine_ICE_P.Constant1_Value_p5) *
                               TX_DriveLine_ICE_P.Gain2_Gain_gc) +
                     TX_DriveLine_ICE_P.Constant1_Value_p5) *
                    TX_DriveLine_ICE_P.Gain_Gain_ka;

  /* Switch: '<S201>/Switch' incorporates:
   *  Constant: '<S201>/Constant'
   *  Constant: '<S209>/Constant'
   *  Inport: '<Root>/DrvShfTrq[Nm]'
   *  Product: '<S201>/Product1'
   *  SignalConversion generated from: '<S190>/Vector Concatenate'
   */
  if (TX_DriveLine_ICE_U.DrvShfTrqNm * rtb_SumofElements_l > TX_DriveLine_ICE_P.Switch_Threshold_af) {
    rtb_SumofElements = TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_b3;
  }

  /* Product: '<S201>/Product4' incorporates:
   *  Constant: '<S207>/Constant'
   *  Constant: '<S208>/Constant1'
   *  Inport: '<Root>/DrvShfTrq[Nm]'
   *  Product: '<S207>/Product3'
   *  Product: '<S207>/Product5'
   *  Sum: '<S207>/Subtract1'
   *  Sum: '<S208>/Subtract'
   *  Switch: '<S201>/Switch'
   */
  rtb_Product4_dr = ((TX_DriveLine_ICE_P.Constant1_Value_p5 - rtb_Subtract1_d) * TX_DriveLine_ICE_P.Constant_Value_ea +
                     rtb_SumofElements * rtb_Subtract1_d) *
                    TX_DriveLine_ICE_U.DrvShfTrqNm;

  /* Gain: '<S204>/Gain' incorporates:
   *  Abs: '<S203>/Abs'
   *  Constant: '<S204>/Constant1'
   *  Gain: '<S204>/Gain1'
   *  Gain: '<S204>/Gain2'
   *  Sum: '<S204>/Subtract1'
   *  Sum: '<S204>/Subtract2'
   *  Trigonometry: '<S204>/Trigonometric Function'
   *  UnaryMinus: '<S190>/Unary Minus1'
   */
  rtb_Subtract1_d3 = (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_f * std::abs(-rtb_Integrator_p[0]) -
                                 TX_DriveLine_ICE_P.Constant1_Value_pt) *
                                TX_DriveLine_ICE_P.Gain2_Gain_f1) +
                      TX_DriveLine_ICE_P.Constant1_Value_pt) *
                     TX_DriveLine_ICE_P.Gain_Gain_jx;

  /* Switch: '<S199>/Switch' incorporates:
   *  Constant: '<S199>/Constant'
   *  Constant: '<S209>/Constant'
   *  Product: '<S199>/Product1'
   *  UnaryMinus: '<S190>/Unary Minus1'
   *  UnaryMinus: '<S210>/Unary Minus'
   */
  if (-rtb_Subtract1_fp * -rtb_Integrator_p[0] > TX_DriveLine_ICE_P.Switch_Threshold_g) {
    rtb_SumofElements = TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_f1;
  }

  /* Product: '<S199>/Product4' incorporates:
   *  Constant: '<S203>/Constant'
   *  Constant: '<S204>/Constant1'
   *  Product: '<S203>/Product3'
   *  Product: '<S203>/Product5'
   *  Sum: '<S203>/Subtract1'
   *  Sum: '<S204>/Subtract'
   *  Switch: '<S199>/Switch'
   *  UnaryMinus: '<S210>/Unary Minus'
   */
  rtb_Product4_p = ((TX_DriveLine_ICE_P.Constant1_Value_pt - rtb_Subtract1_d3) * TX_DriveLine_ICE_P.Constant_Value_lw +
                    rtb_SumofElements * rtb_Subtract1_d3) *
                   -rtb_Subtract1_fp;

  /* Gain: '<S206>/Gain' incorporates:
   *  Abs: '<S205>/Abs'
   *  Constant: '<S206>/Constant1'
   *  Gain: '<S206>/Gain1'
   *  Gain: '<S206>/Gain2'
   *  Sum: '<S206>/Subtract1'
   *  Sum: '<S206>/Subtract2'
   *  Trigonometry: '<S206>/Trigonometric Function'
   *  UnaryMinus: '<S190>/Unary Minus1'
   */
  rtb_Subtract1_d3 = (std::tanh((TX_DriveLine_ICE_P.Gain1_Gain_ol * std::abs(-rtb_Integrator_p[1]) -
                                 TX_DriveLine_ICE_P.Constant1_Value_hv) *
                                TX_DriveLine_ICE_P.Gain2_Gain_fs) +
                      TX_DriveLine_ICE_P.Constant1_Value_hv) *
                     TX_DriveLine_ICE_P.Gain_Gain_pj;

  /* Switch: '<S200>/Switch' incorporates:
   *  Constant: '<S200>/Constant'
   *  Constant: '<S209>/Constant'
   *  Product: '<S200>/Product1'
   *  UnaryMinus: '<S190>/Unary Minus1'
   *  UnaryMinus: '<S218>/Unary Minus'
   */
  if (-rtb_Subtract1_d1 * -rtb_Integrator_p[1] > TX_DriveLine_ICE_P.Switch_Threshold_lz) {
    rtb_SumofElements = TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_eta;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_P.Constant_Value_c;
  }

  /* Product: '<S200>/Product4' incorporates:
   *  Constant: '<S205>/Constant'
   *  Constant: '<S206>/Constant1'
   *  Product: '<S205>/Product3'
   *  Product: '<S205>/Product5'
   *  Sum: '<S205>/Subtract1'
   *  Sum: '<S206>/Subtract'
   *  Switch: '<S200>/Switch'
   *  UnaryMinus: '<S218>/Unary Minus'
   */
  rtb_Product4_l = ((TX_DriveLine_ICE_P.Constant1_Value_hv - rtb_Subtract1_d3) * TX_DriveLine_ICE_P.Constant_Value_eg +
                    rtb_SumofElements * rtb_Subtract1_d3) *
                   -rtb_Subtract1_d1;

  /* Integrator: '<S186>/Integrator' */
  rtb_Integrator_k = TX_DriveLine_ICE_X.Integrator_CSTATE_ba;

  /* MATLAB Function: '<S184>/Limited Slip Differential' incorporates:
   *  Constant: '<S184>/Jd'
   *  Constant: '<S184>/Jw1'
   *  Constant: '<S184>/Jw3'
   *  Constant: '<S184>/Ndiff2'
   *  Constant: '<S184>/bd'
   *  Constant: '<S184>/bw1'
   *  Constant: '<S184>/bw2'
   */
  TX_DriveLine_ICE_LimitedSlipDifferential(
      rtb_Product4_dr, rtb_Product4_p, rtb_Product4_l, rtb_Integrator_k,
      TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_bw1, TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_bd,
      TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_bw2, TX_DriveLine_ICE_P.ratio_diff_rear,
      TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_Jd, TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_Jw1,
      TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_Jw2, rtb_Integrator_p,
      &TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_c, &TX_DriveLine_ICE_P.sf_LimitedSlipDifferential_c);
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    /* Switch: '<S193>/Switch' incorporates:
     *  Constant: '<S193>/Constant'
     */
    if (TX_DriveLine_ICE_P.Constant_Value_n > TX_DriveLine_ICE_P.Switch_Threshold_di) {
      /* Switch: '<S193>/Switch' incorporates:
       *  Constant: '<S193>/Constant2'
       */
      TX_DriveLine_ICE_B.diffDir_m = TX_DriveLine_ICE_P.Constant2_Value_j;
    } else {
      /* Switch: '<S193>/Switch' incorporates:
       *  Constant: '<S193>/Constant2'
       *  UnaryMinus: '<S193>/Unary Minus'
       */
      TX_DriveLine_ICE_B.diffDir_m = -TX_DriveLine_ICE_P.Constant2_Value_j;
    }

    /* End of Switch: '<S193>/Switch' */
  }

  /* Product: '<S186>/Product' incorporates:
   *  Abs: '<S198>/Abs'
   *  Constant: '<S198>/Constant'
   *  Constant: '<S198>/Constant1'
   *  Constant: '<S198>/Constant2'
   *  Gain: '<S198>/Gain'
   *  Lookup_n-D: '<S198>/mu Table'
   *  Product: '<S198>/Product'
   *  Sum: '<S186>/Sum'
   *  Trigonometry: '<S198>/Trigonometric Function'
   */
  TX_DriveLine_ICE_B.Product_cv =
      (TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_Fc * TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_Ndisks *
           TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_Reff *
           std::tanh(TX_DriveLine_ICE_P.Gain_Gain_m * rtb_Subtract1_co) *
           look1_binlxpw(std::abs(rtb_Subtract1_co), TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_dw,
                         TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_muc, 7U) -
       rtb_Integrator_k) *
      TX_DriveLine_ICE_B.upi_i;

  /* Sum: '<S210>/Subtract' incorporates:
   *  Inport: '<Root>/OmegaAxle[rad|s]'
   *  UnaryMinus: '<S190>/Unary Minus1'
   */
  TX_DriveLine_ICE_B.Subtract_pe = -rtb_Integrator_p[0] - TX_DriveLine_ICE_U.OmegaAxlerads[2];

  /* Switch: '<S211>/Switch' incorporates:
   *  Integrator: '<S211>/Integrator'
   */
  if (TX_DriveLine_ICE_B.Memory_f != 0.0) {
    rtb_SumofElements = TX_DriveLine_ICE_X.Integrator_CSTATE_d;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_B.domega_o_eq;
  }

  /* Product: '<S211>/Product' incorporates:
   *  Constant: '<S210>/omega_c'
   *  Sum: '<S211>/Sum'
   *  Switch: '<S211>/Switch'
   */
  TX_DriveLine_ICE_B.Product_l =
      (TX_DriveLine_ICE_B.Subtract_pe - rtb_SumofElements) * TX_DriveLine_ICE_P.TorsionalCompliance_omega_c_k;

  /* Sum: '<S218>/Subtract' incorporates:
   *  Inport: '<Root>/OmegaAxle[rad|s]'
   *  UnaryMinus: '<S190>/Unary Minus1'
   */
  TX_DriveLine_ICE_B.Subtract_n = -rtb_Integrator_p[1] - TX_DriveLine_ICE_U.OmegaAxlerads[3];

  /* Switch: '<S219>/Switch' incorporates:
   *  Integrator: '<S219>/Integrator'
   */
  if (TX_DriveLine_ICE_B.Memory_j != 0.0) {
    rtb_SumofElements = TX_DriveLine_ICE_X.Integrator_CSTATE_k;
  } else {
    rtb_SumofElements = TX_DriveLine_ICE_B.domega_o_b;
  }

  /* Product: '<S219>/Product' incorporates:
   *  Constant: '<S218>/omega_c'
   *  Sum: '<S219>/Sum'
   *  Switch: '<S219>/Switch'
   */
  TX_DriveLine_ICE_B.Product_ag =
      (TX_DriveLine_ICE_B.Subtract_n - rtb_SumofElements) * TX_DriveLine_ICE_P.TorsionalCompliance1_omega_c_n;
  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
      /* Update for Memory: '<S43>/Memory' incorporates:
       *  Constant: '<S43>/Reset'
       */
      TX_DriveLine_ICE_DW.Memory_PreviousInput = TX_DriveLine_ICE_P.Reset_Value_g;

      /* Update for Memory: '<S35>/Memory' incorporates:
       *  Constant: '<S35>/Reset'
       */
      TX_DriveLine_ICE_DW.Memory_PreviousInput_b = TX_DriveLine_ICE_P.Reset_Value;
    }

    /* Update for Integrator: '<S43>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK = 0;

    /* Update for Integrator: '<S35>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_d = 0;
    if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
      /* Update for Memory: '<S211>/Memory' incorporates:
       *  Constant: '<S211>/Reset'
       */
      TX_DriveLine_ICE_DW.Memory_PreviousInput_o = TX_DriveLine_ICE_P.Reset_Value_ck;

      /* Update for Memory: '<S219>/Memory' incorporates:
       *  Constant: '<S219>/Reset'
       */
      TX_DriveLine_ICE_DW.Memory_PreviousInput_h = TX_DriveLine_ICE_P.Reset_Value_j5;
    }

    /* Update for Integrator: '<S211>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_e = 0;

    /* Update for Integrator: '<S219>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_o = 0;
    if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
      /* Update for Memory: '<S144>/Memory' incorporates:
       *  Constant: '<S144>/Reset'
       */
      TX_DriveLine_ICE_DW.Memory_PreviousInput_i = TX_DriveLine_ICE_P.Reset_Value_c;

      /* Update for Memory: '<S152>/Memory' incorporates:
       *  Constant: '<S152>/Reset'
       */
      TX_DriveLine_ICE_DW.Memory_PreviousInput_e = TX_DriveLine_ICE_P.Reset_Value_jd;
    }

    /* Update for Integrator: '<S144>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_m = 0;

    /* Update for Integrator: '<S152>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_c = 0;
    if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
      /* Update for Memory: '<S128>/Memory' incorporates:
       *  Constant: '<S128>/Reset'
       */
      TX_DriveLine_ICE_DW.Memory_PreviousInput_d = TX_DriveLine_ICE_P.Reset_Value_l;

      /* Update for Memory: '<S136>/Memory' incorporates:
       *  Constant: '<S136>/Reset'
       */
      TX_DriveLine_ICE_DW.Memory_PreviousInput_g = TX_DriveLine_ICE_P.Reset_Value_jc;
    }

    /* Update for Integrator: '<S128>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_i = 0;

    /* Update for Integrator: '<S136>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_iy = 0;

    /* Update for Integrator: '<S8>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_ok = 0;

    /* Update for Integrator: '<S184>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_ec = 0;

    /* Update for Integrator: '<S58>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_ecw = 0;
    if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
      /* Update for Memory: '<S112>/Memory' incorporates:
       *  Constant: '<S112>/Reset'
       */
      TX_DriveLine_ICE_DW.Memory_PreviousInput_h0 = TX_DriveLine_ICE_P.Reset_Value_n;

      /* Update for Memory: '<S120>/Memory' incorporates:
       *  Constant: '<S120>/Reset'
       */
      TX_DriveLine_ICE_DW.Memory_PreviousInput_hy = TX_DriveLine_ICE_P.Reset_Value_j;
    }

    /* Update for Integrator: '<S112>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_mo = 0;

    /* Update for Integrator: '<S59>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_j = 0;

    /* Update for Integrator: '<S120>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_k = 0;

    /* Update for Integrator: '<S85>/Integrator' */
    TX_DriveLine_ICE_DW.Integrator_IWORK_f = 0;
  } /* end MajorTimeStep */

  if (rtmIsMajorTimeStep((&TX_DriveLine_ICE_M))) {
    rt_ertODEUpdateContinuousStates(&(&TX_DriveLine_ICE_M)->solverInfo);

    /* Update absolute time for base rate */
    /* The "clockTick0" counts the number of times the code of this task has
     * been executed. The absolute time is the multiplication of "clockTick0"
     * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
     * overflow during the application lifespan selected.
     * Timer of this task consists of two 32 bit unsigned integers.
     * The two integers represent the low bits Timing.clockTick0 and the high bits
     * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
     */
    if (!(++(&TX_DriveLine_ICE_M)->Timing.clockTick0)) {
      ++(&TX_DriveLine_ICE_M)->Timing.clockTickH0;
    }

    (&TX_DriveLine_ICE_M)->Timing.t[0] = rtsiGetSolverStopTime(&(&TX_DriveLine_ICE_M)->solverInfo);

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
      (&TX_DriveLine_ICE_M)->Timing.clockTick1++;
      if (!(&TX_DriveLine_ICE_M)->Timing.clockTick1) {
        (&TX_DriveLine_ICE_M)->Timing.clockTickH1++;
      }
    }
  } /* end MajorTimeStep */
}

/* Derivatives for root system: '<Root>' */
void TX_DriveLine_ICE::TX_DriveLine_ICE_derivatives() {
  XDot_TX_DriveLine_ICE_T *_rtXdot;
  boolean_T lsat;
  boolean_T usat;
  _rtXdot = ((XDot_TX_DriveLine_ICE_T *)(&TX_DriveLine_ICE_M)->derivs);

  /* Derivatives for Integrator: '<S43>/Integrator' */
  _rtXdot->Integrator_CSTATE = TX_DriveLine_ICE_B.Product_a;

  /* Derivatives for Integrator: '<S42>/Integrator' */
  _rtXdot->Integrator_CSTATE_j = TX_DriveLine_ICE_B.Subtract_k;

  /* Derivatives for Integrator: '<S35>/Integrator' */
  _rtXdot->Integrator_CSTATE_c = TX_DriveLine_ICE_B.Product_c;

  /* Derivatives for Integrator: '<S34>/Integrator' */
  _rtXdot->Integrator_CSTATE_n = TX_DriveLine_ICE_B.Subtract;

  /* Derivatives for Integrator: '<S211>/Integrator' */
  _rtXdot->Integrator_CSTATE_d = TX_DriveLine_ICE_B.Product_l;

  /* Derivatives for Integrator: '<S210>/Integrator' */
  _rtXdot->Integrator_CSTATE_h = TX_DriveLine_ICE_B.Subtract_pe;

  /* Derivatives for Integrator: '<S219>/Integrator' */
  _rtXdot->Integrator_CSTATE_k = TX_DriveLine_ICE_B.Product_ag;

  /* Derivatives for Integrator: '<S218>/Integrator' */
  _rtXdot->Integrator_CSTATE_b = TX_DriveLine_ICE_B.Subtract_n;

  /* Derivatives for Integrator: '<S144>/Integrator' */
  _rtXdot->Integrator_CSTATE_c4 = TX_DriveLine_ICE_B.Product_j;

  /* Derivatives for Integrator: '<S143>/Integrator' */
  _rtXdot->Integrator_CSTATE_c3 = TX_DriveLine_ICE_B.Subtract_g;

  /* Derivatives for Integrator: '<S152>/Integrator' */
  _rtXdot->Integrator_CSTATE_nl = TX_DriveLine_ICE_B.Product_a3;

  /* Derivatives for Integrator: '<S151>/Integrator' */
  _rtXdot->Integrator_CSTATE_g = TX_DriveLine_ICE_B.Subtract_p;

  /* Derivatives for Integrator: '<S128>/Integrator' */
  _rtXdot->Integrator_CSTATE_e = TX_DriveLine_ICE_B.Product_he;

  /* Derivatives for Integrator: '<S127>/Integrator' */
  _rtXdot->Integrator_CSTATE_ch = TX_DriveLine_ICE_B.Subtract_b;

  /* Derivatives for Integrator: '<S136>/Integrator' */
  _rtXdot->Integrator_CSTATE_jc = TX_DriveLine_ICE_B.Product_b;

  /* Derivatives for Integrator: '<S135>/Integrator' */
  _rtXdot->Integrator_CSTATE_l = TX_DriveLine_ICE_B.Subtract_h;

  /* Derivatives for Integrator: '<S8>/Integrator' */
  lsat = (TX_DriveLine_ICE_X.Integrator_CSTATE_hh[0] <= TX_DriveLine_ICE_P.Integrator_LowerSat);
  usat = (TX_DriveLine_ICE_X.Integrator_CSTATE_hh[0] >= TX_DriveLine_ICE_P.Integrator_UpperSat);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential.xdot[0] > 0.0)) ||
      (usat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential.xdot[0] < 0.0))) {
    _rtXdot->Integrator_CSTATE_hh[0] = TX_DriveLine_ICE_B.sf_LimitedSlipDifferential.xdot[0];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_hh[0] = 0.0;
  }

  lsat = (TX_DriveLine_ICE_X.Integrator_CSTATE_hh[1] <= TX_DriveLine_ICE_P.Integrator_LowerSat);
  usat = (TX_DriveLine_ICE_X.Integrator_CSTATE_hh[1] >= TX_DriveLine_ICE_P.Integrator_UpperSat);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential.xdot[1] > 0.0)) ||
      (usat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential.xdot[1] < 0.0))) {
    _rtXdot->Integrator_CSTATE_hh[1] = TX_DriveLine_ICE_B.sf_LimitedSlipDifferential.xdot[1];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_hh[1] = 0.0;
  }

  /* End of Derivatives for Integrator: '<S8>/Integrator' */

  /* Derivatives for Integrator: '<S184>/Integrator' */
  lsat = (TX_DriveLine_ICE_X.Integrator_CSTATE_a[0] <= TX_DriveLine_ICE_P.Integrator_LowerSat_g);
  usat = (TX_DriveLine_ICE_X.Integrator_CSTATE_a[0] >= TX_DriveLine_ICE_P.Integrator_UpperSat_d);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_c.xdot[0] > 0.0)) ||
      (usat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_c.xdot[0] < 0.0))) {
    _rtXdot->Integrator_CSTATE_a[0] = TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_c.xdot[0];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_a[0] = 0.0;
  }

  lsat = (TX_DriveLine_ICE_X.Integrator_CSTATE_a[1] <= TX_DriveLine_ICE_P.Integrator_LowerSat_g);
  usat = (TX_DriveLine_ICE_X.Integrator_CSTATE_a[1] >= TX_DriveLine_ICE_P.Integrator_UpperSat_d);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_c.xdot[1] > 0.0)) ||
      (usat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_c.xdot[1] < 0.0))) {
    _rtXdot->Integrator_CSTATE_a[1] = TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_c.xdot[1];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_a[1] = 0.0;
  }

  /* End of Derivatives for Integrator: '<S184>/Integrator' */

  /* Derivatives for Integrator: '<S58>/Integrator' */
  lsat = (TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] <= TX_DriveLine_ICE_P.Integrator_LowerSat_e);
  usat = (TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] >= TX_DriveLine_ICE_P.Integrator_UpperSat_a);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_ICE_B.xdot[0] > 0.0)) ||
      (usat && (TX_DriveLine_ICE_B.xdot[0] < 0.0))) {
    _rtXdot->Integrator_CSTATE_kh[0] = TX_DriveLine_ICE_B.xdot[0];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_kh[0] = 0.0;
  }

  lsat = (TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1] <= TX_DriveLine_ICE_P.Integrator_LowerSat_e);
  usat = (TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1] >= TX_DriveLine_ICE_P.Integrator_UpperSat_a);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_ICE_B.xdot[1] > 0.0)) ||
      (usat && (TX_DriveLine_ICE_B.xdot[1] < 0.0))) {
    _rtXdot->Integrator_CSTATE_kh[1] = TX_DriveLine_ICE_B.xdot[1];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_kh[1] = 0.0;
  }

  /* End of Derivatives for Integrator: '<S58>/Integrator' */

  /* Derivatives for Integrator: '<S10>/Integrator' */
  _rtXdot->Integrator_CSTATE_ca = TX_DriveLine_ICE_B.Product;

  /* Derivatives for Integrator: '<S112>/Integrator' */
  _rtXdot->Integrator_CSTATE_p = TX_DriveLine_ICE_B.Product_e;

  /* Derivatives for Integrator: '<S111>/Integrator' */
  _rtXdot->Integrator_CSTATE_px = TX_DriveLine_ICE_B.Subtract_i;

  /* Derivatives for Integrator: '<S59>/Integrator' */
  lsat = (TX_DriveLine_ICE_X.Integrator_CSTATE_i[0] <= TX_DriveLine_ICE_P.Integrator_LowerSat_a);
  usat = (TX_DriveLine_ICE_X.Integrator_CSTATE_i[0] >= TX_DriveLine_ICE_P.Integrator_UpperSat_j);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_i.xdot[0] > 0.0)) ||
      (usat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_i.xdot[0] < 0.0))) {
    _rtXdot->Integrator_CSTATE_i[0] = TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_i.xdot[0];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_i[0] = 0.0;
  }

  lsat = (TX_DriveLine_ICE_X.Integrator_CSTATE_i[1] <= TX_DriveLine_ICE_P.Integrator_LowerSat_a);
  usat = (TX_DriveLine_ICE_X.Integrator_CSTATE_i[1] >= TX_DriveLine_ICE_P.Integrator_UpperSat_j);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_i.xdot[1] > 0.0)) ||
      (usat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_i.xdot[1] < 0.0))) {
    _rtXdot->Integrator_CSTATE_i[1] = TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_i.xdot[1];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_i[1] = 0.0;
  }

  /* End of Derivatives for Integrator: '<S59>/Integrator' */

  /* Derivatives for Integrator: '<S61>/Integrator' */
  _rtXdot->Integrator_CSTATE_f = TX_DriveLine_ICE_B.Product_h;

  /* Derivatives for Integrator: '<S120>/Integrator' */
  _rtXdot->Integrator_CSTATE_d0 = TX_DriveLine_ICE_B.Product_k;

  /* Derivatives for Integrator: '<S119>/Integrator' */
  _rtXdot->Integrator_CSTATE_dx = TX_DriveLine_ICE_B.Subtract_kz;

  /* Derivatives for Integrator: '<S85>/Integrator' */
  lsat = (TX_DriveLine_ICE_X.Integrator_CSTATE_f3[0] <= TX_DriveLine_ICE_P.Integrator_LowerSat_l);
  usat = (TX_DriveLine_ICE_X.Integrator_CSTATE_f3[0] >= TX_DriveLine_ICE_P.Integrator_UpperSat_b);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_d.xdot[0] > 0.0)) ||
      (usat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_d.xdot[0] < 0.0))) {
    _rtXdot->Integrator_CSTATE_f3[0] = TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_d.xdot[0];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_f3[0] = 0.0;
  }

  lsat = (TX_DriveLine_ICE_X.Integrator_CSTATE_f3[1] <= TX_DriveLine_ICE_P.Integrator_LowerSat_l);
  usat = (TX_DriveLine_ICE_X.Integrator_CSTATE_f3[1] >= TX_DriveLine_ICE_P.Integrator_UpperSat_b);
  if (((!lsat) && (!usat)) || (lsat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_d.xdot[1] > 0.0)) ||
      (usat && (TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_d.xdot[1] < 0.0))) {
    _rtXdot->Integrator_CSTATE_f3[1] = TX_DriveLine_ICE_B.sf_LimitedSlipDifferential_d.xdot[1];
  } else {
    /* in saturation */
    _rtXdot->Integrator_CSTATE_f3[1] = 0.0;
  }

  /* End of Derivatives for Integrator: '<S85>/Integrator' */

  /* Derivatives for Integrator: '<S87>/Integrator' */
  _rtXdot->Integrator_CSTATE_n3 = TX_DriveLine_ICE_B.Product_d;

  /* Derivatives for Integrator: '<S186>/Integrator' */
  _rtXdot->Integrator_CSTATE_ba = TX_DriveLine_ICE_B.Product_cv;
}

/* Model initialize function */
void TX_DriveLine_ICE::initialize() {
  /* Registration code */
  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&(&TX_DriveLine_ICE_M)->solverInfo, &(&TX_DriveLine_ICE_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&TX_DriveLine_ICE_M)->solverInfo, &rtmGetTPtr((&TX_DriveLine_ICE_M)));
    rtsiSetStepSizePtr(&(&TX_DriveLine_ICE_M)->solverInfo, &(&TX_DriveLine_ICE_M)->Timing.stepSize0);
    rtsiSetdXPtr(&(&TX_DriveLine_ICE_M)->solverInfo, &(&TX_DriveLine_ICE_M)->derivs);
    rtsiSetContStatesPtr(&(&TX_DriveLine_ICE_M)->solverInfo, (real_T **)&(&TX_DriveLine_ICE_M)->contStates);
    rtsiSetNumContStatesPtr(&(&TX_DriveLine_ICE_M)->solverInfo, &(&TX_DriveLine_ICE_M)->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&(&TX_DriveLine_ICE_M)->solverInfo,
                                    &(&TX_DriveLine_ICE_M)->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&(&TX_DriveLine_ICE_M)->solverInfo,
                                       &(&TX_DriveLine_ICE_M)->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&(&TX_DriveLine_ICE_M)->solverInfo,
                                      &(&TX_DriveLine_ICE_M)->periodicContStateRanges);
    rtsiSetErrorStatusPtr(&(&TX_DriveLine_ICE_M)->solverInfo, (&rtmGetErrorStatus((&TX_DriveLine_ICE_M))));
    rtsiSetRTModelPtr(&(&TX_DriveLine_ICE_M)->solverInfo, (&TX_DriveLine_ICE_M));
  }

  rtsiSetSimTimeStep(&(&TX_DriveLine_ICE_M)->solverInfo, MAJOR_TIME_STEP);
  (&TX_DriveLine_ICE_M)->intgData.y = (&TX_DriveLine_ICE_M)->odeY;
  (&TX_DriveLine_ICE_M)->intgData.f[0] = (&TX_DriveLine_ICE_M)->odeF[0];
  (&TX_DriveLine_ICE_M)->intgData.f[1] = (&TX_DriveLine_ICE_M)->odeF[1];
  (&TX_DriveLine_ICE_M)->intgData.f[2] = (&TX_DriveLine_ICE_M)->odeF[2];
  (&TX_DriveLine_ICE_M)->intgData.f[3] = (&TX_DriveLine_ICE_M)->odeF[3];
  (&TX_DriveLine_ICE_M)->contStates = ((X_TX_DriveLine_ICE_T *)&TX_DriveLine_ICE_X);
  rtsiSetSolverData(&(&TX_DriveLine_ICE_M)->solverInfo, static_cast<void *>(&(&TX_DriveLine_ICE_M)->intgData));
  rtsiSetIsMinorTimeStepWithModeChange(&(&TX_DriveLine_ICE_M)->solverInfo, false);
  rtsiSetSolverName(&(&TX_DriveLine_ICE_M)->solverInfo, "ode4");
  rtmSetTPtr((&TX_DriveLine_ICE_M), &(&TX_DriveLine_ICE_M)->Timing.tArray[0]);
  (&TX_DriveLine_ICE_M)->Timing.stepSize0 = 0.001;
  rtmSetFirstInitCond((&TX_DriveLine_ICE_M), 1);

  /* Start for Constant: '<S42>/domega_o' */
  TX_DriveLine_ICE_B.domega_o = TX_DriveLine_ICE_P.TorsionalCompliance2_domega_o;

  /* Start for Constant: '<S34>/domega_o' */
  TX_DriveLine_ICE_B.domega_o_e = TX_DriveLine_ICE_P.TorsionalCompliance1_domega_o;

  /* Start for Constant: '<S210>/domega_o' */
  TX_DriveLine_ICE_B.domega_o_eq = TX_DriveLine_ICE_P.TorsionalCompliance_domega_o;

  /* Start for Constant: '<S218>/domega_o' */
  TX_DriveLine_ICE_B.domega_o_b = TX_DriveLine_ICE_P.TorsionalCompliance1_domega_o_f;

  /* Start for Constant: '<S143>/domega_o' */
  TX_DriveLine_ICE_B.domega_o_br = TX_DriveLine_ICE_P.TorsionalCompliance4_domega_o;

  /* Start for Constant: '<S151>/domega_o' */
  TX_DriveLine_ICE_B.domega_o_a = TX_DriveLine_ICE_P.TorsionalCompliance5_domega_o;

  /* Start for Constant: '<S127>/domega_o' */
  TX_DriveLine_ICE_B.domega_o_p = TX_DriveLine_ICE_P.TorsionalCompliance2_domega_o_j;

  /* Start for Constant: '<S135>/domega_o' */
  TX_DriveLine_ICE_B.domega_o_g = TX_DriveLine_ICE_P.TorsionalCompliance3_domega_o;

  /* Start for Constant: '<S111>/domega_o' */
  TX_DriveLine_ICE_B.domega_o_b2 = TX_DriveLine_ICE_P.TorsionalCompliance_domega_o_m;

  /* Start for Constant: '<S119>/domega_o' */
  TX_DriveLine_ICE_B.domega_o_d = TX_DriveLine_ICE_P.TorsionalCompliance1_domega_o_p;
  TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE = UNINITIALIZED_ZCSIG;
  TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_h = UNINITIALIZED_ZCSIG;
  TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_p = UNINITIALIZED_ZCSIG;
  TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_d = UNINITIALIZED_ZCSIG;
  TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_e = UNINITIALIZED_ZCSIG;
  TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_de = UNINITIALIZED_ZCSIG;
  TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_c = UNINITIALIZED_ZCSIG;
  TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_o = UNINITIALIZED_ZCSIG;
  TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_i = UNINITIALIZED_ZCSIG;
  TX_DriveLine_ICE_PrevZCX.Integrator_Reset_ZCE_j = UNINITIALIZED_ZCSIG;

  /* InitializeConditions for Memory: '<S43>/Memory' */
  TX_DriveLine_ICE_DW.Memory_PreviousInput = TX_DriveLine_ICE_P.Memory_InitialCondition;

  /* InitializeConditions for Integrator: '<S43>/Integrator' incorporates:
   *  Integrator: '<S35>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_ICE_M))) {
    TX_DriveLine_ICE_X.Integrator_CSTATE = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_c = 0.0;
  }

  TX_DriveLine_ICE_DW.Integrator_IWORK = 1;

  /* End of InitializeConditions for Integrator: '<S43>/Integrator' */

  /* InitializeConditions for Integrator: '<S42>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_j = TX_DriveLine_ICE_P.TorsionalCompliance2_theta_o;

  /* InitializeConditions for Memory: '<S35>/Memory' */
  TX_DriveLine_ICE_DW.Memory_PreviousInput_b = TX_DriveLine_ICE_P.Memory_InitialCondition_n;

  /* InitializeConditions for Integrator: '<S35>/Integrator' */
  TX_DriveLine_ICE_DW.Integrator_IWORK_d = 1;

  /* InitializeConditions for Integrator: '<S34>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_n = TX_DriveLine_ICE_P.TorsionalCompliance1_theta_o;

  /* InitializeConditions for Memory: '<S211>/Memory' */
  TX_DriveLine_ICE_DW.Memory_PreviousInput_o = TX_DriveLine_ICE_P.Memory_InitialCondition_b;

  /* InitializeConditions for Integrator: '<S211>/Integrator' incorporates:
   *  Integrator: '<S219>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_ICE_M))) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_d = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_k = 0.0;
  }

  TX_DriveLine_ICE_DW.Integrator_IWORK_e = 1;

  /* End of InitializeConditions for Integrator: '<S211>/Integrator' */

  /* InitializeConditions for Integrator: '<S210>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_h = TX_DriveLine_ICE_P.TorsionalCompliance_theta_o;

  /* InitializeConditions for Memory: '<S219>/Memory' */
  TX_DriveLine_ICE_DW.Memory_PreviousInput_h = TX_DriveLine_ICE_P.Memory_InitialCondition_k;

  /* InitializeConditions for Integrator: '<S219>/Integrator' */
  TX_DriveLine_ICE_DW.Integrator_IWORK_o = 1;

  /* InitializeConditions for Integrator: '<S218>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_b = TX_DriveLine_ICE_P.TorsionalCompliance1_theta_o_e;

  /* InitializeConditions for Memory: '<S144>/Memory' */
  TX_DriveLine_ICE_DW.Memory_PreviousInput_i = TX_DriveLine_ICE_P.Memory_InitialCondition_bc;

  /* InitializeConditions for Integrator: '<S144>/Integrator' incorporates:
   *  Integrator: '<S152>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_ICE_M))) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_c4 = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_nl = 0.0;
  }

  TX_DriveLine_ICE_DW.Integrator_IWORK_m = 1;

  /* End of InitializeConditions for Integrator: '<S144>/Integrator' */

  /* InitializeConditions for Integrator: '<S143>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_c3 = TX_DriveLine_ICE_P.TorsionalCompliance4_theta_o;

  /* InitializeConditions for Memory: '<S152>/Memory' */
  TX_DriveLine_ICE_DW.Memory_PreviousInput_e = TX_DriveLine_ICE_P.Memory_InitialCondition_c;

  /* InitializeConditions for Integrator: '<S152>/Integrator' */
  TX_DriveLine_ICE_DW.Integrator_IWORK_c = 1;

  /* InitializeConditions for Integrator: '<S151>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_g = TX_DriveLine_ICE_P.TorsionalCompliance5_theta_o;

  /* InitializeConditions for Memory: '<S128>/Memory' */
  TX_DriveLine_ICE_DW.Memory_PreviousInput_d = TX_DriveLine_ICE_P.Memory_InitialCondition_h;

  /* InitializeConditions for Integrator: '<S128>/Integrator' incorporates:
   *  Integrator: '<S136>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_ICE_M))) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_e = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_jc = 0.0;
  }

  TX_DriveLine_ICE_DW.Integrator_IWORK_i = 1;

  /* End of InitializeConditions for Integrator: '<S128>/Integrator' */

  /* InitializeConditions for Integrator: '<S127>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_ch = TX_DriveLine_ICE_P.TorsionalCompliance2_theta_o_j;

  /* InitializeConditions for Memory: '<S136>/Memory' */
  TX_DriveLine_ICE_DW.Memory_PreviousInput_g = TX_DriveLine_ICE_P.Memory_InitialCondition_p;

  /* InitializeConditions for Integrator: '<S136>/Integrator' */
  TX_DriveLine_ICE_DW.Integrator_IWORK_iy = 1;

  /* InitializeConditions for Integrator: '<S135>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_l = TX_DriveLine_ICE_P.TorsionalCompliance3_theta_o;

  /* InitializeConditions for Integrator: '<S8>/Integrator' incorporates:
   *  Integrator: '<S184>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_ICE_M))) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_hh[0] = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_hh[1] = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_a[0] = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_a[1] = 0.0;
  }

  TX_DriveLine_ICE_DW.Integrator_IWORK_ok = 1;

  /* End of InitializeConditions for Integrator: '<S8>/Integrator' */

  /* InitializeConditions for Integrator: '<S184>/Integrator' */
  TX_DriveLine_ICE_DW.Integrator_IWORK_ec = 1;

  /* InitializeConditions for Integrator: '<S58>/Integrator' incorporates:
   *  Integrator: '<S112>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_ICE_M))) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_kh[0] = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_kh[1] = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_p = 0.0;
  }

  TX_DriveLine_ICE_DW.Integrator_IWORK_ecw = 1;

  /* End of InitializeConditions for Integrator: '<S58>/Integrator' */

  /* InitializeConditions for Integrator: '<S10>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_ca = TX_DriveLine_ICE_P.Integrator_IC;

  /* InitializeConditions for Memory: '<S112>/Memory' */
  TX_DriveLine_ICE_DW.Memory_PreviousInput_h0 = TX_DriveLine_ICE_P.Memory_InitialCondition_i;

  /* InitializeConditions for Integrator: '<S112>/Integrator' */
  TX_DriveLine_ICE_DW.Integrator_IWORK_mo = 1;

  /* InitializeConditions for Integrator: '<S111>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_px = TX_DriveLine_ICE_P.TorsionalCompliance_theta_o_k;

  /* InitializeConditions for Integrator: '<S59>/Integrator' incorporates:
   *  Integrator: '<S120>/Integrator'
   */
  if (rtmIsFirstInitCond((&TX_DriveLine_ICE_M))) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_i[0] = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_i[1] = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_d0 = 0.0;
  }

  TX_DriveLine_ICE_DW.Integrator_IWORK_j = 1;

  /* End of InitializeConditions for Integrator: '<S59>/Integrator' */

  /* InitializeConditions for Integrator: '<S61>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_f = TX_DriveLine_ICE_P.Integrator_IC_p;

  /* InitializeConditions for Memory: '<S120>/Memory' */
  TX_DriveLine_ICE_DW.Memory_PreviousInput_hy = TX_DriveLine_ICE_P.Memory_InitialCondition_nn;

  /* InitializeConditions for Integrator: '<S120>/Integrator' */
  TX_DriveLine_ICE_DW.Integrator_IWORK_k = 1;

  /* InitializeConditions for Integrator: '<S119>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_dx = TX_DriveLine_ICE_P.TorsionalCompliance1_theta_o_d;

  /* InitializeConditions for Integrator: '<S85>/Integrator' */
  if (rtmIsFirstInitCond((&TX_DriveLine_ICE_M))) {
    TX_DriveLine_ICE_X.Integrator_CSTATE_f3[0] = 0.0;
    TX_DriveLine_ICE_X.Integrator_CSTATE_f3[1] = 0.0;
  }

  TX_DriveLine_ICE_DW.Integrator_IWORK_f = 1;

  /* End of InitializeConditions for Integrator: '<S85>/Integrator' */

  /* InitializeConditions for Integrator: '<S87>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_n3 = TX_DriveLine_ICE_P.Integrator_IC_pr;

  /* InitializeConditions for Integrator: '<S186>/Integrator' */
  TX_DriveLine_ICE_X.Integrator_CSTATE_ba = TX_DriveLine_ICE_P.Integrator_IC_c;

  /* set "at time zero" to false */
  if (rtmIsFirstInitCond((&TX_DriveLine_ICE_M))) {
    rtmSetFirstInitCond((&TX_DriveLine_ICE_M), 0);
  }
}

/* Model terminate function */
void TX_DriveLine_ICE::terminate() { /* (no terminate code required) */
}

/* Constructor */
TX_DriveLine_ICE::TX_DriveLine_ICE()
    : TX_DriveLine_ICE_U(),
      TX_DriveLine_ICE_Y(),
      TX_DriveLine_ICE_B(),
      TX_DriveLine_ICE_DW(),
      TX_DriveLine_ICE_X(),
      TX_DriveLine_ICE_PrevZCX(),
      TX_DriveLine_ICE_M() {
  /* Currently there is no constructor body generated.*/
}

/* Destructor */
TX_DriveLine_ICE::~TX_DriveLine_ICE() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_TX_DriveLine_ICE_T *TX_DriveLine_ICE::getRTM() { return (&TX_DriveLine_ICE_M); }
